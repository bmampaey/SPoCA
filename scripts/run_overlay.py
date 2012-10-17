#!/usr/bin/python2.6
import os, os.path, sys, glob
import logging
import argparse
import threading
from Queue import Queue
import spoca_job
import time
import re
from datetime import datetime, timedelta

def setup_logging(filename = None, quiet = False, verbose = False, debug = False):
	global logging
	if debug:
		logging.basicConfig(level = logging.DEBUG, format='%(levelname)-8s: %(message)s')
	elif verbose:
		logging.basicConfig(level = logging.INFO, format='%(levelname)-8s: %(message)s')
	else:
		logging.basicConfig(level = logging.CRITICAL, format='%(levelname)-8s: %(message)s')
	
	if quiet:
		logging.root.handlers[0].setLevel(logging.CRITICAL + 10)
	elif verbose:
		logging.root.handlers[0].setLevel(logging.INFO)
	else:
		logging.root.handlers[0].setLevel(logging.CRITICAL)
	
	if filename:
		import logging.handlers
		fh = logging.FileHandler(filename, delay=True)
		fh.setFormatter(logging.Formatter('%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S'))
		if debug:
			fh.setLevel(logging.DEBUG)
		else:
			fh.setLevel(logging.INFO)
		
		logging.root.addHandler(fh)


timedelta_regex = re.compile(r'((?P<days>\d+?)d)?((?P<hours>\d+?)h)?((?P<minutes>\d+?)m)?')

def parse_duration(duration):
	
	parts = timedelta_regex.match(duration)
	if not parts:
		logging.critical("Cannot convert duration to timedelta : " + str(duration))
		return None
	
	parts = parts.groupdict()
	time_params = {}
	for (name, param) in parts.iteritems():
		if param:
			time_params[name] = int(param)
	return timedelta(**time_params)

def glob_files(filenames):
	
	filelist = list()
	
	for filename in filenames:
		if os.path.exists(filename):
			filelist.append(filename)
		else:
			files = sorted(glob.glob(filename))
			if files:
				filelist.extend(files)
			else:
				logging.warning("File %s not found, skipping!", filename)
	return filelist

date_regex = re.compile(r'\d[^.]*\d')
def file_date(filename):
	import dateutil.parser
	filename = os.path.basename(filename)
	m = date_regex.search(filename)
	if m:
		try:
			filedate = dateutil.parser.parse(m.group(0).replace('_', ''))
			return filedate
		
		except Exception:
			return None
	
	else:
		return None

def align_files(filelists, files_queue, max_delta = None):
	
	def delta_time(fileanddate_set):
		dates = [fileanddate['date'] for fileanddate in fileanddate_set]
		return max(dates) - min(dates)
	
	def file_set(fileanddate_set):
		return [filelists[fileanddate['index'][0]][fileanddate['index'][1]] for fileanddate in fileanddate_set]
	
	filesanddates = list()
	for channel in range(len(filelists)):
		for j in range(len(filelists[channel])):
			filedate = file_date(filelists[channel][j])
			if not filedate:
				log.critical("Cannot extract date of file %s, exiting", filelists[channel][j])
				sys.exit(1)
			filesanddates.append({'date': filedate, 'index': (channel,j)})
	
	filesanddates.sort(key=lambda k: k['date'], reverse = True)
	
	best_set = [None] * len(filelists)
	while filesanddates:
		fileanddate = filesanddates.pop()
		channel = fileanddate['index'][0]
		if any([b == None for b in best_set]):
			best_set[channel] = fileanddate
		else:
			current_set = best_set[:]
			current_set[channel] = fileanddate
			# If is is worse then we had our best_set
			best_delta = delta_time(best_set)
			current_delta = delta_time(current_set)
			if (max_delta == None or best_delta < max_delta) and (best_delta < current_delta):
				files_queue.put(file_set(best_set))
				best_set = [None] * len(filelists)
				best_set[channel] = fileanddate
			else:
				logging.debug("Skipping file %s with best_delta %s too big or worse than %s", filelists[channel][best_set[channel]['index'][1]], best_delta, current_delta)
				best_set = current_set

def zip_files(filelists, files_queue):
	
	filesets = zip(*filelists)
	for fileset in filesets:
		files_queue.put(fileset)



def make_overlay_jobs(files_queue, output_queue, force=False):
	
	counter = 0
	
	files = files_queue.get()
	# None is provided when the queue is empty
	while files:
		
		job_name = "overlay_%s" % counter; counter += 1
		job = spoca_job.overlay(job_name, files[0], fitsfiles=files[1:], force = force)
		if job.job:
			logging.info("Running overlay job %s for map %s on files %s", job_name, files[0], str(files[1:]))
			output_queue.put(job.job)
		else:
			logging.info("Skipping overlay job %s for map %s on files %s", job_name, files[0], str(files[1:]))
		files = files_queue.get()
	
	logging.info("No more files to process, exiting thread")
	output_queue.put(None)


# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	# Default name for the log file
	log_filename = os.path.join('.', script_name+'.log')
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Run ovelay in parralel on many maps.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='Set the file where to log')
	parser.add_argument('--extra', '-e', default=None, help='Any extra parameter for condor')
	parser.add_argument('--output_directory', '-o', default='images', help='Set the output directory for the images')
	parser.add_argument('--force', '-f', default=False, action='store_true', help='force the remaking of all files')
	parser.add_argument('--align_files', '-a', default=False, action='store_true', help='Try to align files on filename')
	parser.add_argument('--max_delta', '-m', default=None, help='If align files is set, max delta time between the files of the set. Must be specified like 1h (one hour) or 3m (3 minutes)')
	parser.add_argument('--overlay_config', '-O', default=None, help='Config file for the overlay')
	parser.add_argument('--maps', '-M', nargs='+', help='The paths of the maps')
	parser.add_argument('--files', '-F', nargs='*', action='append', help='The paths of the fits files. Must be specified once per channel.')

	
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(filename = args.log_filename, quiet = False, verbose = args.verbose, debug = args.debug)
	# Create a logger
	log = logging.getLogger(script_name)
	
	# Parsing and checking of the parameters
	# Output directory
	output_directory = os.path.abspath(args.output_directory)
	if not os.path.isdir(output_directory):
		try:
			os.mkdir(output_directory)
		except OSError, why:
			log.critical("Output directory %s is not a directory and could not be created: %s", output_directory, why)
			sys.exit(2)
		else:
			log.info("Created output directory %s", output_directory)
	
	# Overlay config
	if args.overlay_config == None or not os.path.exists(args.overlay_config):
		log.critical("Config file %s does not exists", args.overlay_config)
		sys.exit(2)
	else:
		spoca_job.overlay.set_parameters(args.overlay_config, output_directory, args.extra)
	
	ok, reason = spoca_job.overlay.test_parameters()
	if ok:
		log.info("Overlay parameters seem ok")
		log.debug(reason)
	else:
		log.warn("Overlay parameters could be wrong")
		log.warn(reason)	
	
	
	# Maps
	if not args.maps:
		log.critical("No maps specified, exiting.")
		sys.exit(2)
	
	# We glob the maps
	maplist = glob_files(args.maps)
	
	number_files = len(maplist)
	
	if number_files == 0:
		log.critical("No maps found to process, exiting.")
		sys.exit(2)
		
	# Files
	filelists = list()
	if not args.files:
		log.info("No fits files specified, using IMAGE001 in maps.")
	else:
		# We glob the files
		for channel in range(len(args.files)):
			filelist = glob_files(args.files[channel])
			if filelist:
				filelists.append(filelist)
			else:
				filelists.append([args.files[channel]] * number_files)
		
	
	threads = list()
	# We make the overlay jobs
	files_queue = Queue()
	output_queue = Queue()
	overlay_jobs = []
	overlay_thread = threading.Thread(group=None, name='make_overlay_jobs', target=make_overlay_jobs, args=(files_queue, output_queue, overlay_jobs, args.force), kwargs={})
	overlay_thread.start()
	threads.append(overlay_thread)
	
	# We feed the files to the threads
	log.debug("Starting to feed files")
	filelists.insert(0, maplist)
	if(args.align_files):
		max_delta = None
		if args.max_delta:
			max_delta = parse_duration(args.max_delta)
			if max_delta == None:
				log.critical("Error parsing max_delta %s, exiting.", args.max_delta)
				sys.exit(2)
		align_files(filelists, files_queue, max_delta)
	else:
		zip_files(filelists, files_queue)
	
	files_queue.put(None)
	
	# We wait that all threads are done
	for thread in threads:
		log.debug("Waiting for thread %s to terminate", thread.name)
		thread.join()
		log.info("Thread %s has terminated", thread.name)
	
	DAG(segmentation_jobs+tracking_jobs+overlay_jobs).submit()
