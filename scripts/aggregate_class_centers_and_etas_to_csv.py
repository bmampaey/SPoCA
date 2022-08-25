#!/usr/bin/env python3

import logging
import re
from argparse import ArgumentParser
from xml.dom import minidom
from dateutil import parser as parse_date
from glob import iglob
from datetime import datetime
from astropy.io import fits
from pathlib import Path
from pandas import DataFrame


float_regex = re.compile(r'[-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?')
channel_regex = re.compile(r'\w+')


def get_info_from_map(filepath):
	'''Extract the time, class centers and etas from a SPoCA map FITS file'''
	
	info = { 'time': None, 'class_centers': dict(), 'etas': dict() }
	
	with fits.open(filepath) as hdulist:
		for hdu in hdulist:
			if 'DATE_OBS' in hdu.header:
				info['time'] = parse_date(hdu.header['DATE_OBS'])
			
			if 'CHANNELS' not in hdu.header:
				continue
			
			else:
				channels = channel_regex.findall(hdu.header['CHANNELS'])
				
				# Assume that there is no more than 100 class centers
				for i in range(1, 100):
					class_center_keyword = 'CLSCTR' + ('%02d' % i)
					if class_center_keyword in hdu.header:
						class_centers = float_regex.findall(hdu.header[class_center_keyword])
						if len(class_centers) != len(channels):
							raise ValueError('Number of class centers is different than number of channels for keyword %s' % class_center_keyword)
						for channel in channels:
							info['class_centers']['center_%02d_%s' % (i, channel)] = float(class_centers.pop(0))
					else:
						break
				
				# Assume that there is no more than 100 etas
				for i in range(1, 100):
					eta_keyword = 'CETA' + ('%02d' % i)
					if eta_keyword in hdu.header:
						info['etas']['eta_%02d' % i] = float(hdu.header[eta_keyword])
					else:
						break
				
				break
		else:
			raise ValueError('CHANNELS keyword not found in file')
	
	return info

voevent_channels_regex = re.compile(r'spocaChannels(?:[^;\w]*)(\w[^;]*\w)(?:[^;\w]*)')
voevent_class_centers_regex = re.compile(r'spocaCenters(?:[^;\d]*)(\d[^;]*\d)(?:[^;\d]*)')
voevent_etas_regex = re.compile(r'spocaEtas(?:[^;\d]*)(\d[^;]*\d)(?:[^;\d]*)')

def get_info_from_voevent(filepath):
	'''Extract the time, class centers and etas from a SPoCA VO event file'''
	
	info = { 'time': None, 'class_centers': dict(), 'etas': dict() }
	
	dom = minidom.parse(str(filepath))
	
	info['time'] = parse_date(dom.getElementsByTagName('TimeInstant')[0].getElementsByTagName('ISOTime')[0].firstChild.data)
	
	# The SPoCA channels, class centers and etas are sored in the FRM_ParamSet element
	FRM_ParamSet = dom.getElementsByTagName('lmsal:FRM_ParamSet')[0].firstChild.data
	
	match = voevent_channels_regex.search(FRM_ParamSet)
	if match:
		channels = channel_regex.findall(match.groups(0)[0])
	else:
		raise ValueError('spocaChannels not found in file')
	
	match = voevent_class_centers_regex.search(FRM_ParamSet)
	if match:
		class_centers = float_regex.findall(match.groups(0)[0])
		number_classes, remainder =  divmod(len(class_centers), len(channels))
		if remainder != 0:
			raise ('Number of class centers is different than number of channels for parameter spocaCenters')
		for i in range(1, number_classes + 1):
			for channel in channels:
				info['class_centers']['center_%02d_%s' % (i, channel)] = float(class_centers.pop(0))
	else:
		raise ValueError('spocaCenters not found in file')
	
	match = voevent_etas_regex.search(FRM_ParamSet)
	if match:
		etas = float_regex.findall(match.groups(0)[0])
		for i, eta in enumerate(etas):
			info['etas']['eta_%02d' % i] = float(etas.pop(0))
	
	return info

time_regex = re.compile(r'(?P<time>\d\d\d\d\d\d\d\d_\d\d\d\d\d\d)')

def get_info_from_centers_file(filepath):
	'''Extract the time, class centers and etas from a SPoCA class centers text file'''
	
	info = { 'time': None, 'class_centers': dict(), 'etas': dict() }
	
	# The time is only stored in the filename
	file_time = time_regex.search(str(filepath))
	
	if file_time:
		info['time'] = datetime.strptime(file_time.group('time'), '%Y%m%d_%H%M%S')
	else:
		raise ValueError('Could not extract time from file name')
	
	with open(filepath) as centers_file:
		line = centers_file.readline()
	
	if '\t' not in line:
		raise ValueError('Format of file is wrong')
	
	fields = line.split('\t')
	channels = channel_regex.findall(fields[0])
	class_centers = float_regex.findall(fields[1])
	
	number_classes, remainder =  divmod(len(class_centers), len(channels))
	
	if remainder != 0:
		raise ('Number of class centers is different than number of channels')
	
	for i in range(1, number_classes + 1):
		for channel in channels:
			info['class_centers']['center_%02d_%s' % (i, channel)] = float(class_centers.pop(0))

	if len(fields) > 2:
		etas = float_regex.findall(fields[2])
		for i, eta in enumerate(etas):
			info['etas']['eta_%02d' % i] = float(etas.pop(0))
	
	return info

def get_dataframe_from_files(filepaths):
	'''Function that parses SPoCA generated files and returns a DataFrame with the filepaths as index and the times, class centers and etas as columns'''
	infos = list()
	
	for filepath in filepaths:
		logging.info('Extracting infos from file %s', filepath)
		
		extension = filepath.suffix.lower()
		
		if extension == '.xml':
			try:
				info = get_info_from_voevent(filepath)
			except Exception as why:
				logging.error('Could not extract info from VO event file %s: %s', filepath, why)
				continue
		
		elif extension == '.fits':
			try:
				info = get_info_from_map(filepath)
			except Exception as why:
				logging.error('Could not extract info from map FITS file %s: %s', filepath, why)
				continue
		
		elif extension == '.txt':
			try:
				info = get_info_from_centers_file(filepath)
			except Exception as why:
				logging.error('Could not extract info from class centers text file %s: %s', filepath, why)
				continue
		
		else:
			logging.error('Unknown file type for file %s, skipping!', filepath)
			continue
		
		logging.debug('Parsed file %s:\n%s', filepath, info)
		
		infos.append({'filepath': filepath, 'time' : info['time'], **info['class_centers'], **info['etas']})
	
	return DataFrame.from_records(infos, index = 'filepath')


def iter_filepaths(glob_patterns):
	'''Return an iterator of file paths'''
	for glob_pattern in glob_patterns:
		for filepath in iglob(glob_pattern, recursive=True):
			yield Path(filepath)

# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = ArgumentParser(description='Extract the time, class centers and etas from SPoCA generated files (FITS map, VO event, class center text file) and write them to a csv file')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	parser.add_argument('--output', '-o', default='class_centers_and_eta.csv', help = 'Path of the output CSV file')
	parser.add_argument('filepaths', nargs = '+', metavar = 'FILEPATH', help = 'The path to a SPoCA fits maps, voevents or, centers and etas text file (accept also a glob pattern)')
	args = parser.parse_args()
	
	# Setup the logging
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	# We get the dataframe from the files and write it as CSV to the output file
	try:
		dataframe = get_dataframe_from_files(iter_filepaths(args.filepaths))
	except Exception as why:
		logging.critical('Could not extract info from files: %s', why)
	else:
		dataframe.to_csv(args.output)
