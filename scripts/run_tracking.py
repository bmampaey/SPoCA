#!/usr/bin/python
import os, os.path, sys
import subprocess
import logging
import argparse
from glob import glob
from spoca_job import tracking

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

def run_tracking(filenames, force):
	
	bin = tracking.bin
	if not os.path.exists(bin):
		return False, "Could not find executable: " + str(bin)
	
	overlap = tracking.overlap
	if not overlap:
		return False, "Overlap was not set correctly"
	
	max_files = tracking.max_files
	if not max_files:
		return False, "Max_files was not set correctly"
	
	log.debug("Tracking max_files %s, overlap %s", max_files, overlap)
	
	files_to_track = list()
	overlap_files = list()
	
	for filename in filenames:
		
		files_to_track.append(filename)
		
		if len(files_to_track) + len(overlap_files) >= max_files:
			arguments = tracking.build_arguments(overlap_files+files_to_track)
			if not arguments:
				return False, "Could not create arguments"
			
			process = subprocess.Popen([bin] + arguments, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			(output, error) = process.communicate()
			
			if process.returncode != 0:
				return False, "Arguments could be wrong :"+ ' '.join(arguments) + "\treturned error:" + error  
			else:
				log.info("Tracking job for files %s to %s ran succesfully", files_to_track[0], files_to_track[-1])
			overlap_files = files_to_track[-overlap:]
			files_to_track = list()
	
	if files_to_track:
		log.debug("Making LAST tracking job.")
		arguments = tracking.build_arguments(overlap_files+files_to_track)
		if not arguments:
			return False, "Could not create arguments"
		
		process = subprocess.Popen([bin] + arguments, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		
		if process.returncode != 0:
			return False, "Arguments could be wrong :"+ ' '.join(arguments) + "\treturned error:" + error  
		else:
			log.info("Tracking job for files %s to %s ran succesfully", files_to_track[0], files_to_track[-1])


# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	# Default name for the log file
	log_filename = os.path.join('./', script_name+'.log')
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Run tracking on many fits files.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='set the file where to log')
	parser.add_argument('--force', '-f', default=False, action='store_true', help='force the remaking of all files')
	parser.add_argument('--extra', '-e', default=None, help='Any extra parameter for condor')
	parser.add_argument('--tracking_config', '-T', default=None, required=True, help='Config file for the tracking')
	parser.add_argument('--files', '-F', nargs='+', help='The paths of the maps.')
	
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(filename = args.log_filename, quiet = False, verbose = args.verbose, debug = args.debug)
	
	# Create a logger
	log = logging.getLogger(script_name)
	
	# Parsing and checking of the parameters
	# Tracking config
	if args.tracking_config != None:
		if not os.path.exists(args.tracking_config):
			log.critical("Config file %s does not exists", args.tracking_config)
			sys.exit(2)
		else:
			tracking.set_parameters(args.tracking_config, args.extra)
		
		ok, reason = tracking.test_parameters()
		if ok:
			log.info("Tracking parameters seem ok")
			log.debug(reason)
		else:
			log.warn("Tracking parameters could be wrong")
			log.warn(reason)
	else:
		log.info("Not doing tracking")
	
	# Files
	if not args.files:
		log.critical("No fits files specified, exiting.")
		sys.exit(2)
	
	# We glob the files
	filenames = list()
	for filepattern in args.files:
		filenames.extend(glob(filepattern))
	
	if len(filenames) == 0:
		log.critical("No maps found to process, exiting.")
		sys.exit(2)
	
	filenames.sort()
	
	# We run the tracking
	run_tracking(filenames, args.force)
	

