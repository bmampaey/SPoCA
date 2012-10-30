import os, sys, tempfile
import subprocess

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
Arguments	= {arguments}

# Input/Output files
Input          =  {input_file}
Output          = {output_file}
Error           = {error_file}
Log             = {log_file}

# The filesytem is shared
requirements		= (HasPOOL =?= True)

# Parameters for condor
getenv = True
log_xml = True
should_transfer_files	= IF_NEEDED
when_to_transfer_output = ON_EXIT
transfer_executable	= NO

# Tell condor to run the job
queue

"""

CONDOR_DAGMAN_CONFIGURATION_TEMPLATE="""
DAGMAN_MAX_SUBMITS_PER_INTERVAL = {jobsperinterval}

"""

class Job:
	def __init__(self, name, executable, arguments, require=[], input_file="/dev/null"):
		self.require = require
		self.name = name
		self.executable = executable
		self.arguments = arguments
		self.input_file = input_file
	def write_description(self, path):
		with open(path, "w") as f:
			f.write(CONDOR_JOB_DESCRIPTION_TEMPLATE.format( \
				executable=self.executable, \
				arguments=self.arguments, \
				input_file=self.input_file, \
				output_file="/dev/null", \
				error_file="/dev/null", \
				log_file="/dev/null"))
			

class DAG:
	def __init__(self, jobs):
		self.jobs = jobs
	def submit(self):
		condor_directory = tempfile.mkdtemp()
		
		with open(os.path.join(condor_directory, "dagman.conf"), "w") as f:
			f.write(CONDOR_DAGMAN_CONFIGURATION_TEMPLATE.format(jobsperinterval=1000))

		with open(os.path.join(condor_directory, "dagman.condor"), "w") as f:
			for job in self.jobs:
				path = os.path.join(condor_directory, job.name+".condor")
				job.write_description(path)
				f.write("JOB " + job.name + " " + path + "\n")
			for job in self.jobs:
				if job.require != [] and job.require is not None:
					f.write("PARENT " + " ".join(map(lambda x: x.name, job.require)) + " CHILD " + job.name + "\n")
			f.write("CONFIG " + os.path.join(condor_directory, "dagman.conf") + "\n")

		try:
			subprocess.check_call("condor_submit_dag -maxjobs 1000 '%s'" % os.path.join(condor_directory, "dagman.condor"), shell=True)
		except subprocess.CalledProcessError as e:
			print "Condor job submission failed: %s" % e.output
		else:
			print "The condor jobs in %s have been successfully started." % condor_directory
