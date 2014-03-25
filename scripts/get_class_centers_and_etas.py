#!/usr/bin/env python2.6

import string
import os, os.path
import logging
import sys
import argparse
import re
import dateutil.parser
from glob import glob
from datetime import datetime

find_numbers = re.compile(r'[-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?')
find_channels = re.compile(r'\w+')


def parse_map(filename):
	import pyfits
	channels = None
	class_centers = None
	etas = None
	time = None
	try:
		hdulist = pyfits.open(filename)
		for hdu in hdulist:
			if isinstance(hdu, (pyfits.CompImageHDU, pyfits.ImageHDU)):
				if 'DATE_OBS' in hdu.header:
					time = dateutil.parser.parse(hdu.header['DATE_OBS'])
				
				if 'CHANNELS' in hdu.header:
					channels = find_channels.findall(hdu.header['CHANNELS'])
				
				if 'CLSCTR01' in hdu.header:
					class_centers = list()
					for i in range(1,100):
						center = 'CLSCTR' + ('%02d' % i)
						if center in hdu.header:
							class_centers.extend(find_numbers.findall(hdu.header[center]))
						else:
							break
				
				if 'CETA01' in hdu.header:
					etas = list()
					for i in range(1,100):
						eta = 'CETA' + ('%02d' % i)
						if eta in hdu.header:
							etas.extend(find_numbers.findall(hdu.header[eta]))
						else:
							break
		
		hdulist.close()
	
	except Exception, why:
		logging.critical("Error getting class_centers from file " + filename + ": "+ str(why))
	return time, class_centers, etas, channels

find_voevent_channels = re.compile(r'spocaChannels(?:[^;\w]*)(\w[^;]*\w)(?:[^;\w]*)')
find_voevent_class_centers = re.compile(r'spocaCenters(?:[^;\d]*)(\d[^;]*\d)(?:[^;\d]*)')
find_voevent_etas = re.compile(r'spocaEtas(?:[^;\d]*)(\d[^;]*\d)(?:[^;\d]*)')

def parse_voevent(filename):
	import re
	import xml.dom.minidom
	channels = None
	class_centers = None
	etas = None
	time = None
	try:
		dom = xml.dom.minidom.parse(filename)
		FRM_ParamSet = dom.getElementsByTagName('lmsal:FRM_ParamSet')[0].firstChild.data
		
		time = dateutil.parser.parse(dom.getElementsByTagName('TimeInstant')[0].getElementsByTagName('ISOTime')[0].firstChild.data)
		
		match = find_voevent_channels.search(FRM_ParamSet)
		if match:
			channels = find_channels.findall(match.groups(0)[0])
		
		match = find_voevent_class_centers.search(FRM_ParamSet)
		if match:
			class_centers = find_numbers.findall(match.groups(0)[0])
		
		match = find_voevent_etas.search(FRM_ParamSet)
		if match:
			etas = find_numbers.findall(match.groups(0)[0])
	
	except Exception, why:
		logging.critical("Error getting class_centers and etas from file %s, %s", filename, str(why))
	
	return time, class_centers, etas, channels

find_filename_time = re.compile(r'(?P<time>\d\d\d\d\d\d\d\d_\d\d\d\d\d\d)')
def parse_class_centers_file(filename):
	channels = None
	class_centers = None
	etas = None
	time = None
	file_time = find_filename_time.search(filename)
	if file_time:
		time = datetime.strptime(file_time.group('time'), '%Y%m%d_%H%M%S')
	else:
		logging.critical("Error getting time from file " + filename)
	try:
		with open (filename, 'r') as centers_file:
			line = centers_file.readline()
		
		fields = line.split("\t", 1)
		channels = find_channels.findall(fields[0])
		class_centers = find_numbers.findall(fields[1])
		if len(fields) > 2:
			etas = find_numbers.findall(fields[2])
	except Exception, why:
		logging.critical("Error getting class_centers from file " + filename + ": "+ str(why))
	return time, class_centers, etas, channels

def get_class_centers_and_etas(filenames):
	'''Function that parses files to extract the class centers and etas'''
	good_channels = None
	number_centers = 0
	class_centers_and_etas = dict()
	for filename in filenames:
		
		time, class_centers, etas, channels = None, None, None, None
		
		fileprefix, extension = os.path.splitext(filename)
		
		if extension.lower() == ".xml":
			time, class_centers, etas, channels = parse_voevent(filename)
		
		elif extension.lower() == ".fits":
			time, class_centers, etas, channels = parse_map(filename)
		
		elif extension.lower() == ".txt":
			time, class_centers, etas, channels = parse_class_centers_file(filename)
		
		else:
				logging.error("Skipping file %s, unknown file type", str(filename))
				continue
		
		if time:
			logging.info("Found time %s in file %s", str(time), str(filename))
			if time not in class_centers_and_etas:
				class_centers_and_etas[time] = {'class_centers' : list(), 'etas' : list()}
		else:
			logging.error("Time not found in file %s, skipping", str(filename))
			continue
		
		if channels != None:
			logging.info("Found channels %s in file %s", str(channels), str(filename))
			if good_channels == None:
				good_channels = channels
			elif channels != good_channels:
				logging.error("Channels from file %s do not correspond to good channels: %s <> %s", str(filename), str(good_channels), str(channels))
				continue
		
		if class_centers:
			logging.info("Found class centers %s in file %s", str(class_centers), str(filename))
			class_centers_and_etas[time]['class_centers'] = class_centers
			number_centers = max(number_centers, len(class_centers)/len(channels))
		
		if etas:
			logging.info("Found etas %s in file %s", str(etas), str(filename))
			class_centers_and_etas[time]['etas'] = etas
	
	return good_channels, number_centers, class_centers_and_etas


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
	parser = argparse.ArgumentParser(description='Gather all class centers and etas in a csv file.')
	parser.add_argument('--quiet', '-q', default=False, action='store_true', help='Do not display any error message.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='Set the logging level to debug')
	parser.add_argument('--output', '-o', default='class_centers_and_eta.csv', help='Name of the csv file for class centers and etas.')
	parser.add_argument('filename', nargs='+', help='The paths of the files.Can be SPoCA fits maps, voevents or, centers and etas text files')
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(quiet = args.quiet, verbose = True, debug = args.debug)
	
	# We glob the filenames
	filenames = list()
	for filename in args.filename:
		if os.path.exists(filename):
			filenames.append(filename)
		else:
			files = sorted(glob(filename))
			if files:
				filenames.extend(files)
			else:
				logging.warning("File %s not found, skipping!", filename)
	
	# We get the class centers and etas
	good_channels, number_centers, class_centers_and_etas = get_class_centers_and_etas(filenames)
	
	# We write all class centers and etas
	try:
		with open(args.output, 'w') as class_centers_and_eta_file:
			class_centers_and_eta_file.write("time,")
			class_centers_and_eta_file.write(','.join(["class_center_" + str(i+1) + "_" + channel for i in range(number_centers) for channel in good_channels]))
			class_centers_and_eta_file.write(",")
			class_centers_and_eta_file.write(','.join(["eta_" + str(i+1) for i in range(number_centers)]))
			class_centers_and_eta_file.write("\n")
			for time in sorted(class_centers_and_etas.keys()):
				class_centers_and_eta_file.write(time.strftime('%Y-%m-%d %H:%M:%S,'))
				class_centers_and_eta_file.write(','.join([str(class_centers) for class_centers in class_centers_and_etas[time]['class_centers']]))
				class_centers_and_eta_file.write(',')
				class_centers_and_eta_file.write(','.join(class_centers_and_etas[time]['etas']))
				class_centers_and_eta_file.write("\n")
	except Exception, why:
		logging.critical("Could not write class centers ans etas to file %s: %s", str(args.output), str(why))

