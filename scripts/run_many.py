#!/usr/bin/env python2.6
import os, os.path, sys
import string
import threading
from Queue import Queue
import logging
from collections import deque
import argparse
import subprocess, shlex
import time
from datetime import datetime

import pyfits
import get_data



# Return a time in the form yyyymmdd_hhmmss 
def pretty_date(date):
	return date.strftime("%Y%m%d_%H%M%S")

# Return the list of desired keywords from the headers of a fitsfile
def get_keywords(fitsfile, keywords):
	result = [None]*len(keywords)
	try:
		hdulist = pyfits.open(fitsfile)
		for hdu in hdulist:
			for k,keyword in enumerate(keywords):
				if keyword in hdu.header:
					result[k] = hdu.header[keyword]
		
		hdulist.close()
	except IOError, why:
		log.warning("Error reading file " + fitsfile + ": "+ str(why))
	
	return result


def make_jobs(files_queue, output_queue):
	
	counter = 0
	
	filename = files_queue.get()
	# None is provided when the queue is empty
	while filename != None:
		
		log.debug("Received filename " + str(filename))
		
		counter += 1
		job_name = "j"+str(counter)
		
		arguments=jobs_info['args'] + ' ' + filename
		
		job = async.Job(job_name, executable=jobs_info['bin'], arguments=arguments, job_input=jobs_info['input'], extra=jobs_info['extra'], require=None, pre_submit=None, call_back=None, auto_start=True)
			
		log.debug("Submitted job " + str(job_name))
		
		output_queue.put(job)
		filename = files_queue.get()
	
	log.debug("No more files to treat, exiting thread")
	output_queue.put(None)


# Start point of the script
if __name__ == "__main__":
	
	# Default name for the log file
	log_filename = os.path.splitext(os.path.basename(sys.argv[0]))[0] + '.log'
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Test the get_data module.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='set the file where to log')
	parser.add_argument('--output_filename', '-o', default='run_many.out', help='set the file where to write job output')
	parser.add_argument('--force', '-f', default=False, action='store_true', help='force the remaking of all files')
	parser.add_argument('--stop_on_error', '-s', default=False, action='store_true', help='terminate the script if there is an error')
	parser.add_argument('--retry_on_error', '-r', default=False, action='store_true', help='retry to submit a job if it fails. has precedence over stop_on_error')
	parser.add_argument('--batch_mode', '-b', default=False, action='store_true', help='do not ask before resubmitting a job if it fails. if stop_on_error is set and you answer no, the program will terminate')
	parser.add_argument('--data', '-D', default='get_data.config', help='Config file for the data')
	parser.add_argument('--jobs_info', '-J', default='run_many.config', help='Config file for the job')
	parser.add_argument('--condor', '-c', default=False, action='store_true', help='Run the jobs with condor')
	
	args = parser.parse_args()
	
	# Set up of the logging
	log_filename = args.log_filename
	if args.debug :
		logging.basicConfig(filename=log_filename, level = logging.DEBUG, format='%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
	else:
		logging.basicConfig(filename=log_filename, level = logging.INFO, format='%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
	
	console = logging.StreamHandler()
	console.setFormatter(logging.Formatter('%(levelname)-8s: %(name)-12s: %(message)s'))
	if args.verbose:
		console.setLevel(logging.INFO)
	else:
		console.setLevel(logging.CRITICAL)
	
	# Create logger
	log = logging.getLogger('run_job')
	log.addHandler(console)
	
	# set up of the global variables
	force = args.force
	stop_on_error = args.stop_on_error
	retry_on_error = args.retry_on_error
	batch_mode = args.batch_mode
	
	# Set up of the get_data options
	data = dict()
	try :
		with open(args.data) as f:
			exec(f)
	except (IOError) , why:
		log.critical("Could not open data config file: "+ args.data + ": "+ str(why))
		sys.exit(2)
	
	# Set up of the jobs_info options
	jobs_info = dict()
	
	try :
		with open(args.jobs_info) as f:
			exec(f)
	except (IOError) , why:
		log.critical("Could not open jobs_info config file: "+ args.spoca + ": "+ str(why))
		sys.exit(2)
	
	# We check if we run with condor or simple process
	if args.condor:
		import condor_job as async
	else:
		import process_job as async

	threads = list()
	
	# We get the files
	log.debug("Starting to get data")
	files_queue = Queue()
	get_files_thread = threading.Thread(group=None, name='get_files', target=get_data.get_files, args=(data, files_queue), kwargs={})
	get_files_thread.start()
	threads.append(get_files_thread)
	
	# We make the jobs
	input_queue = files_queue
	output_queue = Queue()
	make_jobs_thread = threading.Thread(group=None, name='make_jobs', target=make_jobs, args=(input_queue, output_queue), kwargs={})
	make_jobs_thread.start()
	threads.append(make_jobs_thread)
	
	# We write the output of the jobs to the file
	
	with open(args.output_filename, 'w+') as f:
		
		job = output_queue.get()
		# None is provided when the queue is empty
		while job != None:
			while not job.isTerminated():
				log.debug(str(job.name) + " has not terminated yet, sleeping a little.")
				time.sleep(5)
			if job.return_code == 0:
				log.debug(str(job.name) + " has succesfuly terminated, writing output to file.")
				f.write(job.output)
				f.flush()
			else:
				log.critical(str(job.name) + " has failed with return code "+ str(job.return_code)+". Error:\n" + str(job.error))
			job = output_queue.get()
	
	for thread in threads:
		thread.join()
		log.info(thread.name + " thread has terminated")

	async.Job.wait()

