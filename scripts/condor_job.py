#!/usr/bin/env python2.6
import subprocess
import os, os.path, sys, tempfile
import string, re
import threading
import time
import xml.dom.minidom
import logging
import argparse

CONDOR_JOB_STATUS = {
0:'U', #	Unexpanded
1:'I', #	Idle
2:'R', #	Running
3:'X', #	Removed
4:'C', #	Completed
5:'H', #	Held
6:'E', #	Submission_err
}

CONDOR_SUBMIT_EXTRA = ["arguments", "environment", "getenv", "notification", "notify_user", "priority", "rank", "requirements", "should_transfer_files", "stream_error", "transfer_executable", "transfer_input_files", "transfer_output_files", "transfer_output_remaps"]

#Some condor constants
CONDOR_SUBMIT = 'condor_submit'
CONDOR_SUBMIT_TIMEOUT = 20
CONDOR_RM = 'condor_rm'
CONDOR_Q = 'condor_q'
CONDOR_Q_TIMEOUT = 60
CONDOR_MAX_RUNNING_JOBS = 4
SUBMIT_EXPRESSION = r"Proc\s*(?P<cluster_number>\d+)"
RUNNING_STATUS = [0, 1, 2]

CONDOR_JOB_DESCRIPTION_TEMPLATE="""
####################
##
## Condor job description file created by the Condor.py module
##
####################


# Check the Condor primer on _which universe to choose
# (standard / vanilla)
Universe        = vanilla

# The absolute path to Executable
Executable      = {executable}

# Arguments for the executable
Arguments	= "{arguments}"

# Working directory
InitialDir      = {initialdir}

# Input/Output files
Input          =  {input_file}
Output          = {output_file}
Error           = {error_file}
Log             = {log_file}

# The filsesytem is shared
requirements		= (HasPOOL =?= True)

# Parameters for condor
getenv = True
log_xml = True
should_transfer_files	= IF_NEEDED
when_to_transfer_output = ON_EXIT
transfer_executable	= NO

# Tell condor to run the job
Queue 1

"""

def _parse_dom(data_text):
	dom = xml.dom.minidom.parseString(data_text)
	data_info = dict()
	for a in dom.getElementsByTagName('a'):
		if a.hasAttribute('n') and a.firstChild != None:
			name = a.getAttribute('n')
			value = a.firstChild
			if value.nodeName == 's':
				data_info[name] = value.firstChild.data
			elif value.nodeName == 'i':
				data_info[name] = int(value.firstChild.data)
			elif value.nodeName == 'r':
				data_info[name] = float(value.firstChild.data)
			elif value.nodeName == 'b' and value.hasAttribute('v'):
				data_info[name] = value.getAttribute('v').lower() == 't'
	
	return data_info


def _which(program):
	def is_exe(fpath):
		return os.path.exists(fpath) and os.access(fpath, os.X_OK)
	
	fpath, fname = os.path.split(program)
	if fpath:
		if is_exe(program):
			return program
	else:
		for path in os.environ["PATH"].split(os.pathsep):
			exe_file = os.path.join(path, program)
			if is_exe(exe_file):
				return exe_file
	
	return None


def _timeout_call(prog, input=None, timeout=0):
	
	module_logger.debug("About to execute: "+ ' '.join(prog))
	
	process = None
	if input:
		process = subprocess.Popen(prog, shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		process.stdin.write(input)
		process.stdin.close()
	else:
		process = subprocess.Popen(prog, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		
	start_time = time.time()
	while process.poll() == None and time.time() - start_time < timeout:
		time.sleep(0.5)
	
	if process.poll() == None:
		module_logger.debug("Program did not terminate within the timeout, killing it. "+ ' '.join(prog))
		process.kill()
	
	return (process.returncode, process.stdout.read(), process.stderr.read())


class Job:
	# Static constants
	CHECK_INTERVAL = 5
	TEMP_DIR = tempfile.mkdtemp(prefix = 'condor_temp_file_' + str(os.getpid()) + '_')
	MAX_SUBMISSION_RETRIES = 5
	
	# Static variables
	_last_id = 0
	_pending = dict()
	_pending_lock = threading.Lock()
	_running = dict()
	_running_lock = threading.Lock()
	_monitor_thread = None
	_running_slot = threading.BoundedSemaphore(CONDOR_MAX_RUNNING_JOBS)
	
	# Constructor
	def __init__(self, name, executable, arguments="", job_input=None, extra=None, require=None, pre_submit=None, call_back=None, auto_start=True):
		self.name = str(name)
		self.executable = executable
		self.arguments = arguments
		self.input = job_input
		self.extra = extra
		
		if pre_submit != None and not hasattr(pre_submit, '__call__'):
			module_logger.critical("Job "+ self.name + ": The pre_submit routine is not a callable object")
			return "Error: pre_submit routine is not a callable object"
		else:
			self.pre_submit = pre_submit
		
		if call_back != None and not hasattr(call_back, '__call__'):
			module_logger.critical("Job "+ self.name + ": The call_back routine is not a callable object")
			return "Error: call_back routine is not a callable object"
		else:
			self.call_back = call_back
		
		try:
			self.require = set([j for j in require if isinstance(j, Job)])
		except TypeError:
			self.require = set([require] if isinstance(require, Job) else [])
		
		# We take care of the private variables
		self._started = threading.Lock()
		self._terminated = False
		
		self._id = Job._last_id
		Job._last_id += 1
		
		self._cluster_number = -1
		
		self._provide = set([])
		
		# We add our id to the provide set of the required jobs
		if self.require:
			# Ideally I should check that there is no cycle, but we are going to trust the user.
			for j in self.require:
				if j != self:
					j._provide.add(self._id)
				else:
					module_logger.warning("Job "+ self.name + ": Require itself to start, omitting self")
		
		# We create the temporary filenames for input/output/error/logging
		self._input_file = os.path.join(Job.TEMP_DIR, self.name+'.'+str(self._id)+'.in')
		self._output_file = os.path.join(Job.TEMP_DIR, self.name+'.'+str(self._id)+'.out')
		self._error_file = os.path.join(Job.TEMP_DIR, self.name+'.'+str(self._id)+'.err')
		self._log_file = os.path.join(Job.TEMP_DIR, self.name+'.'+str(self._id)+'.log')
		
		module_logger.debug("Created job " + str(self))
		
		if auto_start:
			self.start(False)
	
	# Destructor
	def __del__(self):
		module_logger.debug("Deleting job " + str(self.name))
	
	# Return a string representation of a Job
	def __str__(self):
		result = "name: " + self.name
		result+= "\n executable:" + str(self.executable)
		result+= "\n arguments:" + str(self.arguments)
		result+= "\n input:" + str(self.input)
		result+= "\n extra:" + str(self.extra)
		result+= "\n require:" + str([j.name for j in self.require])
		return result
	
	
	# Submit a job to condor
	def start(self, immediate=True):
		
		module_logger.debug("Job "+ self.name + ": starting")
		
		# We test if all the required jobs are terminated
		missing = [j.name for j in self.require if not j._terminated]
		if missing:
			if immediate:
				module_logger.debug("Job "+ self.name + ": Tried to start job, but some requirements are missing: " + str(missing))
				return False
			else:
				module_logger.debug("Job "+ self.name + ": Tried to start job, but some requirements are missing: " + str(missing))
				with Job._pending_lock:
					Job._pending[self._id] = self
				module_logger.debug("Job "+ self.name + ": has been appended to pending queue")
				return True
		
		# Ok we can start the job
		if not self._started.acquire(False):
			module_logger.debug("Job "+ self.name + ": Trying to start an already started job.")
			return True
		
		
		# We call pre_submit to allow the job to initialise itself correctly
		submitting = None
		if self.pre_submit:
			submitting = self.pre_submit(self)
		
		# If pre_submit return an explicit False value, we don't submit but we suppose it has terminated
		if submitting != None and not submitting:
			self._terminated = True
			return True
		
		# We verify that the executable exists and set it to it's full path for condor
		executable = _which(self.executable)
		
		# We need to make sure that all keys of extra are clean (i.e. in lower case and striped of white spaces)
		extra_clean = dict()
		if self.extra:
			for k, v in self.extra.iteritems():
				extra_clean[str(k).strip().lower()] = str(v)
		
		# We set up the input file
		if extra_clean and 'input' in extra_clean.keys():
			self._input_file = extra_clean['input']
			if self.input:
				module_logger.warning("Job "+ self.name + ": Input file is present in extra parameter. input specified in parameters will be ignored")
		
		else:
			if self.input != None :
				try:
					with open(self._input_file, 'w') as f:
						f.write(self.input)
				
				except (IOError), why:
					module_logger.critical("Job "+ self.name + ": Could not write temporary file " + self._input_file + " : " + str(why))
					return False
			else:
				self._input_file = ""
		
		
		# We create a condor job description
		self._condor_job_description = CONDOR_JOB_DESCRIPTION_TEMPLATE.format(executable=executable, arguments=self.arguments, initialdir=Job.TEMP_DIR, input_file=self._input_file, output_file=self._output_file, error_file=self._error_file, log_file=self._log_file)
		# module_logger.debug("Job"+ self.name + " description file:\n" + self._condor_job_description)
		
		# We submit the condor job description file with condor_submit
		condor_submit = [CONDOR_SUBMIT, '-']
		
		# We add the extra arguments
		for k, v in extra_clean.iteritems():
			# but we only allow some
			if k in CONDOR_SUBMIT_EXTRA:
				condor_submit.extend(['-a', k+'='+v])
		
		# We submit the job
		module_logger.debug("Job "+ self.name + ": Submitting")
		Job._running_slot.acquire()
		returncode, output, error = _timeout_call(condor_submit, input=self._condor_job_description, timeout=CONDOR_SUBMIT_TIMEOUT)
		
		# We allow to retry the submission if it has failed
		retries = 1
		while returncode != 0 and retries <= Job.MAX_SUBMISSION_RETRIES:
			module_logger.warning("Job "+ self.name + ": Failed to submit condor job, return code: " + str(returncode) + error)
			module_logger.debug("Job description: " + str(self._condor_job_description))
			module_logger.warning("Retrying...")
			returncode, output, error = _timeout_call(condor_submit, input=self._condor_job_description, timeout=retries*CONDOR_SUBMIT_TIMEOUT)
			retries += 1
		
		if returncode != 0:
			module_logger.critical("Job "+ self.name + ": Failed to submit condor job, return code: " + str(returncode) + error)
			return False
		else:
			# We retrieve the condor cluster number from the output
			submit_info = submit_regex.search(output)
			
			if not submit_info:
				module_logger.critical("Job "+ self.name + ": Cluster number not found in submit output: " + output)
				return False
		
		# We retrieve the condor cluster number from the output
		self._cluster_number = int(submit_info.group('cluster_number'))
		module_logger.debug("Job "+ self.name + ": Submited with cluster number "+ str(self._cluster_number))
		
		# We add the job to running
		with Job._running_lock:
			Job._running[self._cluster_number] = self
		
		# We delete the job from pending if it is there
		with Job._pending_lock:
			if self._id in Job._pending:
				del Job._pending[self._id]
		
		# If there is no monitor thread we start one 
		if Job._monitor_thread == None or not Job._monitor_thread.isAlive():
			Job._monitor_thread = threading.Thread(group=None, name='monitor', target=Job._monitor, args=(), kwargs={})
			Job._monitor_thread.start()
		
		return True
	
	# Resubmit a job to condor
	def restart(self):
		if self._cluster_number > 0 and self._cluster_number in Job._running:
			module_logger.debug("Job "+ self.name + ": Trying to restart a job that is running.")
			return False
		
		if self._started.acquire(False):
			module_logger.warning("Job "+ self.name + ": Trying to restart a job that was never started.")
			self._started.release()
			return False
		
		# We cleanup the previous run
		self._terminated = False
		self.output = None
		self.error = None
		self.return_code = None
		
		# We restart the job
		self._started.release()
		return self.start()
	
	def isTerminated(self):
		return self._terminated
	
	# Terminate a job
	def _terminate(self):
		
		if self._terminated:
			module_logger.warning("Job "+ self.name + ": Trying to terminate a job that is already terminated.")
			return True
		
		if self._started.acquire(False):
			module_logger.warning("Job "+ self.name + ": Trying to terminate a job that was not started.")
			self._started.release()
			return False
		
		# We check the log_file to know how and if it really terminated
		return_code = None
		log_message = None
		
		try:
			with open(self._log_file) as f:
				log_message = f.read()
		except (IOError) , why:
			# TODO What to do if I cannot find the log ?
			module_logger.critical("Job "+ self.name + ": Could not open log file "+ str(self._log_file) + ": "+ str(why))
		
		while log_message:
			
			# We parse the nodes in reverse order one at a time
			log_message, tag, info = log_message.rpartition('<c>')
			log_info = _parse_dom(tag + info)
			
			module_logger.debug("Job "+ self.name + ": log info: "+ str(log_info))
			
			# We check that we have the correct cluster
			cluster_number = log_info["Cluster"]
			if cluster_number != self._cluster_number:
				module_logger.warning("Job "+ self.name + ": Found information about other cluster " + str(cluster_number) + " in log info. My cluster number is " + str(self._cluster_number))
				if cluster_number not in Job._running.keys():
					module_logger.warning("But cluster " + str(cluster_number) + " is not a known job")
					continue
				else:
					try :
						module_logger.warning("and cluster " + str(cluster_number) + " job name is " + Job._running[cluster_number].name)
					except KeyError:
						pass
			
			elif log_info["MyType"] == "JobTerminatedEvent":
				
				if "ReturnValue" in log_info.keys():
					return_code = log_info["ReturnValue"]
				
				# If it terminated because of a signal, we set return_code to  -(ReturnValue)
				elif "TerminatedBySignal" in log_info.keys():
					return_code = - log_info["TerminatedBySignal"]
				
				break
		
		# Did we really terminated ?
		if return_code == None:
			module_logger.debug("Job "+ self.name + ": No terminated event found in log file, probably still runnning")
			return False
		else:
			self.return_code = return_code
		
		# Ok we have really terminated
		Job._running_slot.release()
		try :
			with Job._running_lock:
				del Job._running[self._cluster_number]
		except KeyError:
			module_logger.warning("Job "+ self.name + ": Job was already removed from running job list")
		
		self._cluster_number = -1
		
		# We read the error and output from the files if they exist
		self.output = None
		try:
			with open(self._output_file) as f:
				self.output = f.read()
		except (IOError) , why:
			module_logger.warning("Job "+ self.name +": Could not open output file "+ self._output_file + ": "+ str(why))
		
		self.error = None
		try:
			with open(self._error_file) as f:
				self.error = f.read()
		except (IOError) , why:
			module_logger.warning("Job "+ self.name +": Could not open error file "+ self._error_file + ": "+ str(why))
		
		# We delete all the temporary file if they exist
		if(self._input_file):
			try:
				os.remove(self._input_file)
			except (OSError) , why:
				module_logger.info("Job "+ self.name + ": Could not cleanup temporary file "+ self._input_file + ": "+ str(why))
		try:
			os.remove(self._output_file)
		except (OSError) , why:
			module_logger.info("Job "+ self.name + ": Could not cleanup temporary file "+ self._output_file + ": "+ str(why))
		try:
			os.remove(self._error_file)
		except (OSError) , why:
			module_logger.info("Job "+ self.name + ": Could not cleanup temporary file "+ self._error_file + ": "+ str(why))
		try:
			os.remove(self._log_file)
		except (OSError) , why:
			module_logger.info("Job "+ self.name + ": Could not cleanup temporary file "+ self._log_file + ": "+ str(why))
		
		
		# We call the call_back
		terminatedok = None
		if self.call_back:
				terminatedok = self.call_back(self)
		
		self._terminated = True
		
		# We try to start the job we provide to and that are pending
		if (terminatedok == None or bool(terminatedok)) and self._provide:
			for j in self._provide:
				if j in Job._pending:
					Job._pending[j].start(True)
		
		return True
	
	@staticmethod
	def _monitor():
		
		# We monitor as long as there are job running
		while (Job._running):
			
			# We start by sleeping a little
			time.sleep(Job.CHECK_INTERVAL)
			
			# We retrieve the state of the processes with condor_q
			condor_q = [CONDOR_Q, '-format', '%i', 'ClusterId', '-format', '|%i\n', 'JobStatus']
			
			returncode, output, error = _timeout_call(condor_q, input=None, timeout=CONDOR_Q_TIMEOUT)
			if not output and returncode != 0:
				module_logger.warning("Call to condor_q failed with return code: "+ str(returncode)+ " error: "+ error)
				continue
			else:
				pass
				#module_logger.debug("Call to condor_q succeeded: "+ output)
			
			# We parse the output and for each cluster determine it's state
			job_statuses = dict()
			for job_status in output.split('\n'):
				if job_status.count('|') > 0:
					(cluster, status) = job_status.split('|')
					job_statuses[int(cluster)] = int(status)
			
			# We make a list of possibly terminated clusters
			terminated_clusters = list()
			for cluster in Job._running.keys():
				if cluster not in job_statuses or job_statuses[cluster] not in RUNNING_STATUS:
					module_logger.debug(str(cluster) + " seems to be terminated")
					terminated_clusters.append(cluster)
			
			# For each cluster in the terminated list we try to terminate the job
			
			for cluster in terminated_clusters:
				
				terminated_succesfully = None
				
				try:
					job_id = Job._running[cluster]._id
					terminated_succesfully = Job._running[cluster]._terminate()
				
				except (KeyError):
					module_logger.warning(str(cluster) + " not in running job list, has it changed?")
					continue
			
			if len(Job._running) > 0 :
				module_logger.debug("Following jobs are running: " + ' '.join([j.name for j in Job._running.values()]))
			else :
				module_logger.debug("No more jobs are running")
			
			if len(Job._pending) > 0 :
				module_logger.debug("Following jobs are pending: " + ' '.join([j.name for j in Job._pending.values()]))
			else :
				module_logger.debug("No more jobs are pending")
			
		
		# The list of running job is empty we terminate the thread
		module_logger.info("No more jobs to monitor, exiting.")
		Job._monitor_thread = None
		return True		
	
	@staticmethod
	def wait():
		while(len(Job._running)) > 0:
			if Job._monitor_thread == None or not Job._monitor_thread.is_alive():
				module_logger.debug("Jobs are running but monitor thread is dead. Respawning.")
				Job._monitor_thread = threading.Thread(group=None, name='monitor', target=Job._monitor, args=(), kwargs={})
				Job._monitor_thread.start()
				Job._monitor_thread.join()
		
		if len(Job._pending) > 0 :
			module_logger.warning("Following jobs have never met their requirements to be started: " + ' '.join([j.name for j in Job._pending.values()]))
		
		module_logger.info("Job monitor thread has exited.")


# We initialise the library

# We test if condor_submit and condor_q (and condor_rm) are known executables
CONDOR_SUBMIT = _which(CONDOR_SUBMIT)
assert CONDOR_SUBMIT != None, "Exectutable condor_submit not found"

CONDOR_Q = _which(CONDOR_Q)
assert CONDOR_Q != None, "Exectutable condor_q not found"

CONDOR_RM = _which(CONDOR_RM)
assert CONDOR_RM != None, "Exectutable condor_rm not found"

# We precompile the regex to go faster
submit_regex = re.compile(SUBMIT_EXPRESSION, re.M | re.I)

# create logger
module_logger = logging.getLogger('condor_plus')

if __name__ == '__main__':
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Test the condor_job module.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	args = parser.parse_args()

	if args.debug:
		log_level = logging.DEBUG
	else:
		log_level = logging.INFO
	
	logging.basicConfig(level = log_level, format='%(levelname)-8s %(funcName)-12s :%(message)s')
	#module_logger.addHandler(logging.StreamHandler())
	
	# We define a call_back function that prints the results
	def display(job):
		print str(job)
		print "return_code: ", job.return_code
		print "output     :\n", job.output
		print "error      :\n", job.error
		if job.return_code != 0:
			print "Job ", job.name, "failed! return code: ", job.return_code
			if not job.extra:
				job.extra = {'retry': 1}
			else:
				job.extra['retry'] = job.extra['retry'] + 1
			if job.extra['retry'] < 2:
				print "Trying to resubmit the job, retry number " + str(job.extra['retry'])
			 	job.restart()
			 	
			else:
				print "Tried 3 times to submit the job. Giving up!"
			return False
	
	
	def modify_input(job):
		for j in job.require:
			job.input += j.output
	
	print "Testing the condor_plus library"
	j1 = Job("test1", "ls", ".", call_back=display)
	j2 = Job("test2", "cat", job_input="oh le le", require=j1, pre_submit=modify_input, call_back=display)
	j3 = Job("test3", "ls", "bin", call_back=display, require=[j1, j2])
	
	Job.wait()
	
