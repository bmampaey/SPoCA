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
from operator import *

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


def make_jobs(sets_queue, output_queue):
	
	counter = 0
	
	set = sets_queue.get()
	# None is provided when the queue is empty
	while set != None:
		
		files = set['files']
		
		filename = os.path.join(os.getcwd(), '_'.join([os.path.splitext(os.path.basename(f['path']))[0] for f in files])) + '.png'
		
		job = None
		
		if args.force or not os.path.exists(filename):
			counter += 1
			job_name = "j"+str(counter)
			arguments= '-pointsize 25'
			for f in files :
				arguments += ' -label ' + str(f['channel']) + ' ' + f['path']
			
			if args.tile:
				arguments += ' ' + args.tile + ' -geometry +0+0 ' + filename
			else:
				arguments += ' -geometry +0+0 ' + filename
		
			job = async.Job(job_name, executable='montage', arguments=arguments, job_input=None, extra=None, require=None, pre_submit=None, call_back=None, auto_start=True)
			
			log.debug("Submitted job " + str(job_name))
		
		output_queue.put(job)
		set = sets_queue.get()
	
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
	parser.add_argument('--force', '-f', default=False, action='store_true', help='force the remaking of all files')
	parser.add_argument('--data', '-D', nargs='+', default=None, help='Config files for the data')
	parser.add_argument('--frequency', '-F', default=None, help='Frequency of the sets')
	parser.add_argument('--max_deltatime', '-M', default=None, help='Max deltatime of the sets')
	parser.add_argument('--tile', '-T', default=None, help='Arrangement of the images. I.e. 2x3 will make 2 columns and 3 rows.')
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
	log = logging.getLogger('run_tile_images')
	log.addHandler(console)
	
	# set up of the global variables
	force = args.force
	
	# We check if we run with condor or simple process
	if args.condor:
		import condor_job as async
	else:
		import process_job as async
	
	# Check get_data options
	if args.data == None:
		log.critical("No config data file specified")
		sys.exit(2)
	
	# We get the files
	log.debug("Starting to get data")
	files = list()
	channels = list()
	data = dict()
	for d in args.data:
		try :
			with open(d) as f:
				exec(f)
		except (IOError) , why:
			log.critical("Could not open data config file: "+ d + ": "+ str(why))
			sys.exit(2)
		files.extend(get_data.get_files(data))
		channels.extend(data['channels'])
	

	threads = list()
	
	# We make the sets
	log.debug("Starting to make sets")
	files_queue = Queue()
	sets_queue = Queue()
	get_sets_thread = threading.Thread(group=None, name='get_sets', target=get_data.get_sets, args=({'max_deltatime':args.max_deltatime, 'channels':channels}, sets_queue, files_queue), kwargs={})
	get_sets_thread.start()
	threads.append(get_sets_thread)
	
	if args.frequency:
		# We make the reduced sets
		log.debug("Starting to make reduced sets")
		input_queue = sets_queue
		sets_queue = Queue()
		get_sets(data, sets_queue = None, files_queue = None)
		get_reduced_sets_thread = threading.Thread(group=None, name='get_reduced_sets', target=get_data.get_reduced_sets, args=({'frequency':args.args.frequency}, sets_queue, input_queue), kwargs={})
		get_reduced_sets_thread.start()
		threads.append(get_reduced_sets_thread)
	
	# We make the jobs
	output_queue = Queue()
	make_jobs_thread = threading.Thread(group=None, name='make_jobs', target=make_jobs, args=(sets_queue, output_queue), kwargs={})
	make_jobs_thread.start()
	threads.append(make_jobs_thread)
	
	# We sort the files by date
	files.sort(key=itemgetter('date'))
	# We feed the files to the treads 
	for f in files:
		files_queue.put(f)
	
	files_queue.put(None)
	
	job = output_queue.get()
	# None is provided when the queue is empty
	while job != None:
		job = output_queue.get()
	
	for thread in threads:
		thread.join()
		log.info(thread.name + " thread has terminated")

	async.Job.wait()

