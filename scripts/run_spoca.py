#!/usr/bin/python2.6
import os, os.path, sys
import string
import threading
from Queue import Queue
import logging
from collections import deque
import argparse
import re
import glob
import spoca_job
import time
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


date_regex = re.compile(r'\d\d\d\d\d\d\d\d[^0-9]*\d+')
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


def make_segmentation_jobs(files_queue, output_queue, sequential, force):
	
	counter = 0
	job = None
	fileset = files_queue.get()
	# None is provided when the queue is empty
	while fileset != None:
		
		filedate = file_date(fileset[0])
		if filedate:
			job_name = filedate.strftime("%Y%m%d_%H%M%S")
		else:
			job_name = "segmentation_%s" % counter
			counter += 1
		
		if sequential and job:
			job = spoca_job.segmentation(job_name, fileset, previous = job.job, force = force)
		else:
			job = spoca_job.segmentation(job_name, fileset, force = force)
		
		if job.job:
			log.info("Running segmentation job for files %s", fileset)
		else:
			log.debug("Not running segmentation job for files %s", fileset)
		
		output_queue.put(job)
		fileset = files_queue.get()
	
	log.debug("No more files for segmentation, exiting thread")
	
	output_queue.put(None)

def make_tracking_jobs(job_queue, output_queue, force):
	
	counter = 0
	
	max_files = spoca_job.tracking.max_files
	overlap = spoca_job.tracking.overlap
	
	log.debug("Tracking max_files %s, overlap %s", max_files, overlap)
	
	files_to_track = list()
	overlap_files = list()
	
	previous_jobs = list()
	
	# None is provided when the queue is empty
	job = job_queue.get()
	while job != None:
		
		files_to_track.append(job.results[0])
		previous_jobs.append(job.job)
		
		if len(files_to_track) + len(overlap_files) >= max_files:
			job_name = "tracking_%s" % counter; counter += 1
			tracking_job = spoca_job.tracking(job_name, overlap_files+files_to_track, previous=previous_jobs, force = force)
			if tracking_job.job:
				log.info("Running tracking job for files %s", tracking_job.results)
			else:
				log.debug("Not running tracking job for files %s", tracking_job.results)
			
			output_queue.put(tracking_job)
			overlap_files = files_to_track[-overlap:]
			files_to_track = list()
			previous_jobs = list()
		
		job = job_queue.get()
	
	if files_to_track:
		log.debug("Making LAST tracking job.")
		job_name = "tracking_%s" % counter; counter += 1
		tracking_job = spoca_job.tracking(job_name, overlap_files+files_to_track, previous=previous_jobs, force = force)
		if tracking_job.job:
			log.info("Running tracking job for files %s", tracking_job.results)
		else:
			log.debug("Not running tracking job for files %s", tracking_job.results)
		output_queue.put(tracking_job)
	
	log.debug("No more files for tracking, exiting thread")
	
	output_queue.put(None)


def make_overlay_jobs(job_queue, output_queue, force):
	
	counter = 0
	
	job = job_queue.get()
	# None is provided when the queue is empty
	while job != None:
		for mapname in job.results:
			job_name = "overlay_%s" % counter; counter += 1
			overlay_job = spoca_job.overlay(job_name, mapname, previous=[job.job], force = force)
			if overlay_job.job:
				log.info("Running overlay job for map %s", mapname)
			else:
				log.debug("Not running overlay job for map %s", mapname)
			output_queue.put(overlay_job)
		job = job_queue.get()
	
	log.debug("No more files for overlay, exiting thread")
	
	output_queue.put(None)

# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	# Default name for the log file
	log_filename = os.path.join('./', script_name+'.log')
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Run spoca on many fits files.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='set the file where to log')
	parser.add_argument('--force', '-f', default=False, action='store_true', help='force the remaking of all files')
	parser.add_argument('--output_directory', '-o', default='results', help='Set the output directory for the maps')
	parser.add_argument('--extra', '-e', default=None, help='Any extra parameter for condor')
	parser.add_argument('--align_files', '-a', default=False, action='store_true', help='Try to align files on filename')
	parser.add_argument('--max_delta', '-m', default=None, help='If align files is set, max delta time between the files of the set. Must be specified like 1h (one hour) or 3m (3 minutes)')
	parser.add_argument('--sequential', '-s', default=False, action='store_true', help='Run segmentation sequentially instead of concurrently')
	parser.add_argument('--segmentation_config', '-S', default='segmentation.config', help='Config file for the segmentation')
	parser.add_argument('--tracking_config', '-T', default=None, help='Config file for the tracking')
	parser.add_argument('--overlay_config', '-O', default=None, help='Config file for the overlay')	
	parser.add_argument('--files', '-F', nargs='+', action='append', help='The paths of the fits files. Must be specified once per channel.')
	
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
	
	# Segmentation config
	if args.segmentation_config == None or not os.path.exists(args.segmentation_config):
		log.critical("Config file %s does not exists", args.segmentation_config)
		sys.exit(2)
	else:
		spoca_job.segmentation.set_parameters(args.segmentation_config, output_directory, args.extra)
	
	ok, reason = spoca_job.segmentation.test_parameters()
	if ok:
		log.info("Segmentation parameters seem ok")
		log.debug(reason)
	else:
		log.warn("Segmentation parameters could be wrong")
		log.warn(reason)	

	# Tracking config
	if args.tracking_config != None:
		if not os.path.exists(args.tracking_config):
			log.critical("Config file %s does not exists", args.tracking_config)
			sys.exit(2)
		else:
			spoca_job.tracking.set_parameters(args.tracking_config, args.extra)
		
		ok, reason = spoca_job.tracking.test_parameters()
		if ok:
			log.info("Tracking parameters seem ok")
			log.debug(reason)
		else:
			log.warn("Tracking parameters could be wrong")
			log.warn(reason)
	else:
		log.info("Not doing tracking")
	
	# Overlay config
	if args.overlay_config != None:
		if not os.path.exists(args.overlay_config):
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
	else:
		log.info("Not doing overlays")
	
	# Files
	if not args.files:
		log.critical("No fits files specified, exiting.")
		sys.exit(2)
	
	# We glob the files
	filelists = list()
	for channel in range(len(args.files)):
		filelists.append(glob_files(args.files[channel]))
	
	number_files = min([len(filelist) for filelist in filelists])
	
	if number_files == 0:
		log.critical("No fits files found to process, exiting.")
		sys.exit(2)
	
	for channel in range(len(filelists)):
		if len(filelists[channel]) != number_files:
			log.warning("Number of files for channel %d is more than %d, ignoring excess.", channel, number_files)
	
	threads = list()
	
	# We make the segmentation jobs
	files_queue = Queue()
	output_queue = Queue()
	segmentation_thread = threading.Thread(group=None, name='make_segmentation_jobs', target=make_segmentation_jobs, args=(files_queue, output_queue, args.sequential, args.force), kwargs={})
	segmentation_thread.start()
	threads.append(segmentation_thread)
	
	if args.tracking_config != None:
		# We make the tracking jobs
		input_queue = output_queue
		output_queue = Queue()
		tracking_thread = threading.Thread(group=None, name='make_tracking_jobs', target=make_tracking_jobs, args=(input_queue, output_queue, args.force), kwargs={})
		tracking_thread.start()
		threads.append(tracking_thread)
	
	if args.overlay_config != None: 
		# We make the overlay jobs
		input_queue = output_queue
		output_queue = Queue()
		overlay_thread = threading.Thread(group=None, name='make_overlay_jobs', target=make_overlay_jobs, args=(input_queue, output_queue, args.force), kwargs={})
		overlay_thread.start()
		threads.append(overlay_thread)
	
		
	# We feed the files to the threads
	log.debug("Starting to feed files")
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
	
	# We wait for all jobs to terminate
	job = output_queue.get()
	while job != None:
		log.debug("Waiting for job %s to terminate", job.name)
		while not job.isTerminated():
			time.sleep(1)
		log.info("Job %s has terminated successfully.", job.name)
		if job.return_code != 0:
			log.warning("Job %s terminated with return code %s. Error: %s", job.name, job.return_code, job.error)
		job = output_queue.get()
