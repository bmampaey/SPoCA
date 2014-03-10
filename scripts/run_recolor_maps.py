#!/usr/bin/python2.6
import os, os.path, sys, glob
import logging
import argparse
from condor_job import Job
import time
import re
from datetime import datetime, timedelta
import dateutil.parser
import shutil

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


# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	# Default name for the log file
	log_filename = os.path.join('.', script_name+'.log')
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Run recolor_maps in parralel on many maps.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='Set the file where to log')
	parser.add_argument('--extra', '-e', default=None, help='Any extra parameter for condor')
	parser.add_argument('--bin', '-b', default="recolor_map.x", help='The path to the executable')
	parser.add_argument('--outdir', '-o', default='images', help='Set the output directory for the stats')
	parser.add_argument('--force', '-f', default=False, action='store_true', help='force the remaking of all files')
	parser.add_argument('--LUTs', '-L', nargs='+', help='The paths of the LUT files')
	parser.add_argument('filenames', nargs='+', help='The paths of the fits files.')

	
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(filename = args.log_filename, quiet = False, verbose = args.verbose, debug = args.debug)
	# Create a logger
	log = logging.getLogger(script_name)
	
	# Parsing and checking of the parameters
	# Output directory
	outdir = os.path.abspath(args.outdir)
	if not os.path.isdir(outdir):
		try:
			os.mkdir(outdir)
		except OSError, why:
			log.critical("Output directory %s is not a directory and could not be created: %s", outdir, why)
			sys.exit(2)
		else:
			log.info("Created output directory %s", outdir)
	
	if not os.path.exists(args.bin):
		log.critical("Executable %s not found, exiting", args.bin)
		sys.exit(2)
	
	# We glob the maps
	maps = glob_files(args.filenames)
	
	if not maps:
		log.critical("No maps found to process, exiting.")
		sys.exit(2)
		
	# LUT Files
	LUTs =  glob_files(args.LUTs)
	if not LUTs:
		log.critical("No LUTs found to process, exiting.")
		sys.exit(2)
	
	LUTs_dates = dict()
	for filename in LUTs:
		LUTs_dates[file_date(filename)] = filename
	
	jobs = list()
	for filename in maps:
		map_date = file_date(filename)
		result_file = os.path.join(args.outdir, os.path.splitext(os.path.basename(filename))[0] + ".recolored.fits")
		if map_date not in LUTs_dates:
			log.warning("Map %s does not have a LUT, copying the file to %s", filename, result_file)
			shutil.copyfile(filename, result_file)
		elif args.force or not os.path.exists(result_file):
			log.info("Recoloring map %s using LUT %s", filename, LUTs_dates[map_date])
			arguments = "-O {outdir} -c {lut} {filename}".format(outdir=args.outdir, lut=LUTs_dates[map_date], filename=filename)
			job = Job(map_date.strftime("%Y%m%d_%H%M%S"), args.bin, arguments, extra=args.extra, auto_start=True)
			jobs.append(job)
		else:
			log.info("Map %s has already been recolored", filename)
			del LUTs_dates[map_date]
	
	if LUTs_dates:
		log.info("The following LUT files were not used:")
		for filename in LUTs_dates.values():
			log.info(filename)
	
	jobs.reverse()
	
	# We wait for all jobs to terminate
	while jobs:
		job = jobs.pop()
		log.debug("Waiting for job %s to terminate", job.name)
		while not job.isTerminated():
			time.sleep(1)
		log.info("Job %s has terminated successfully.", job.name)
		if job.return_code != 0:
			log.warning("Job %s terminated with return code %s. Error: %s", job.name, job.return_code, job.error)
