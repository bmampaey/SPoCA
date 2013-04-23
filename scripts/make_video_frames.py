#!/usr/bin/env python2.6
# -*- coding: iso-8859-15 -*-
import sys
import subprocess, shlex
import os
import os.path
import string
import logging
import argparse
import threading
from glob import glob

# Parameters to transform fits to png
fits2png_bin = '/pool/bem/BRAIN/vero/fits2png.x'

def run_fits2png(fits2png_cmd):
	
	logging.debug("About to execute: %s", ' '.join(fits2png_cmd))
	try:
		process = subprocess.Popen(fits2png_cmd, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		stdout, stderr = process.communicate()
		return_code = process.poll()
		if return_code != 0:
			logging.error("Failed running command %s : Return code : %d\t Stdout: %s\t StdErr: %s", ' '.join(fits2png_cmd), return_code, stderr, stdout)
			return False
	except Exception, why:
		logging.error('Failed running command %s : %s', ' '.join(fits2png_cmd), str(why))
		return False
	else:
		return True


def fits2images(fits_files, output_directory = '.', size = None, recenter = None, label = None, color = None, straighten_up = False, scale = False, preprocessing_steps = None, force = False):
	
	# We make up the fits2png command 
	fits2png_cmd = [fits2png_bin, '-O', output_directory]
	if size:
		fits2png_cmd.extend(['-S', str(size)])
	if recenter:
		fits2png_cmd.extend(['-r', str(recenter)])
	if label:
		fits2png_cmd.extend(['-l'])
	if color:
		fits2png_cmd.extend(['-c'])
	if straighten_up:
		fits2png_cmd.extend(['-u'])
	if scale and scale != 1:
		fits2png_cmd.extend(['-s', str(scale)])
	if preprocessing_steps:
		fits2png_cmd.extend(['-P', str(preprocessing_steps)])
	
	for fits_file in fits_files:
		if not force:
			fits_filename, trash = os.path.splitext(os.path.basename(fits_file))
			image_filename = os.path.join(output_directory, fits_filename+ '.png')
			if os.path.exists(image_filename):
				continue
		run_fits2png(fits2png_cmd + [fits_file])

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
	elif debug:
		logging.root.handlers[0].setLevel(logging.DEBUG)
	elif verbose:
		logging.root.handlers[0].setLevel(logging.INFO)
	else:
		logging.root.handlers[0].setLevel(logging.CRITICAL)
	
	if filename:
		fh = logging.FileHandler(filename, delay=True)
		fh.setFormatter(logging.Formatter('%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S'))
		if debug:
			fh.setLevel(logging.DEBUG)
		else:
			fh.setLevel(logging.INFO)
		
		logging.root.addHandler(fh)

# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Make video frames from fits files.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='Set the logging level to debug')
	parser.add_argument('--quiet', '-q', default=False, action='store_true', help='Do not display any error message.')
	parser.add_argument('--output_directory', '-o', default='.', help='The directory in which the video frame will be created')
	parser.add_argument('--size', '-s', default=None, help='The size of the video frames. Must be specified like widthxheight in pixels')
	parser.add_argument('--recenter', '-r', default=False, help='Specify to recenter the sun in the frame. Must be specified in pixels as centerX,centerY')
	parser.add_argument('--label', '-l', default=False, action='store_true', help='To write a label in the upper left corner')
	parser.add_argument('--color', '-c', default=False, action='store_true', help='To output the frame in color')
	parser.add_argument('--straighten_up', '-u', default=False, action='store_true', help='To align the solar north upward')
	parser.add_argument('--zoom', '-z', default=1.0, type=float, help='The scaling factor')
	parser.add_argument('--preprocessing_steps', '-p', default=None, help='A list of preprocessing steps to overpass the default ones. See fits2png documentation')
	parser.add_argument('--threads', '-t', default=1, type=int, help='The number of threads to use.')
	parser.add_argument('--force', '-f', default=False, action='store_true', help='Overwrite video frame if it exists already')
	parser.add_argument('fits_files', nargs='+', help='The paths of the fits files')
	
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(quiet = args.quiet, verbose = True, debug = args.debug)
	
	if not args.fits_files:
		logging.error("You must specify at least one fits file")
		sys.exit(2)
	
	# We glob the fits_files
	fits_files = list()
	for fits_file in args.fits_files:
		if os.path.exists(fits_file):
			fits_files.append(fits_file)
		else:
			files = sorted(glob(fits_file))
			if files:
				fits_files.extend(files)
			else:
				logging.warning("File %s not found, skipping!", fits_file)
	
	number_threads = max(1, args.threads)
	
	start_pos = 0
	delta_pos = int(len(fits_files) / number_threads)
	while start_pos + delta_pos < len(fits_files):
		logging.debug("Starting thread for fits files %d to %d", start_pos, start_pos + delta_pos)
		thread = threading.Thread(group=None, name='fits2images', target=fits2images, args=(fits_files[start_pos:start_pos+delta_pos], args.output_directory, args.size, args.recenter, args.label, args.color, args.straighten_up, args.zoom, args.preprocessing_steps, args.force))
		thread.start()
		start_pos += delta_pos
	
	if start_pos < len(fits_files):
		fits2images(fits_files[start_pos:], args.output_directory, args.size, args.recenter, args.label, args.color, args.straighten_up, args.zoom, args.preprocessing_steps, args.force)
