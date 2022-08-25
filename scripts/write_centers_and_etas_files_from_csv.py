#!/usr/bin/env python3

import logging
import re
import pandas

from argparse import ArgumentParser
from pathlib import Path

center_regex = re.compile('(?P<center>center_\d+)_(?P<channel>\w+)')
eta_regex = re.compile('(?P<etar>eta_\d+)')

def parse_column_names(column_names):
	'''Parse the CSV column names to extract the channels, centers and etas'''
	channels = set()
	centers = set()
	etas = set()
	
	for column_name in column_names:
		match = center_regex.match(column_name)
		if match:
			center, channel = match.group('center', 'channel')
			channels.add(channel)
			centers.add(center)
		match = eta_regex.match(column_name)
		if match:
			eta = match.group('eta')
			etas.add(eta)
	
	return sorted(channels), sorted(centers), sorted(etas)

# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = ArgumentParser(description='Read the SPoCA class centers and etas from a csv file and write them to text file for input to the attribution program')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	parser.add_argument('--output', '-o', default = './{date}_centers.txt', help = 'Path of the output text file, accept a {date} placeholder (default to ./{date}_centers.txt)')
	parser.add_argument('filepath', metavar = 'FILEPATH', help = 'The path to CSV file written by the write_median_centers_and_etas_to_csv.py script')
	args = parser.parse_args()
	
	# Setup the logging
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	# Create a dataframe from the csv
	try:
		dataframe = pandas.read_csv(args.filepath, index_col = 'time', parse_dates = ['time'])
	except Exception as why:
		logging.critical('Could not read CSV file %s: %s', args.filepath, why)
		raise
	
	channels, centers, etas = parse_column_names(dataframe.columns)
	logging.debug('Found in CSV file channels %s, centers %s and etas %s', channels, centers, etas)
	
	for time, row in dataframe.iterrows():
		filepath = args.output.format(date = time.strftime('%Y%m%d_%H%M%S'))
		
		class_centers = ['(' + ','.join(str(row['%s_%s' % (center, channel)]) for channel in channels) + ')' for center in centers]
		
		if etas:
			class_etas = [str(row[eta]) for eta in etas]
			with open(filepath, 'wt') as file:
				file.write('[%s]\t[%s]\t[%s]' % (','.join(channels), ','.join(class_centers), ','.join(class_etas)))
		else:
			with open(filepath, 'wt') as file:
				file.write('[%s]\t[%s]' % (','.join(channels), ','.join(class_centers)))
		
		logging.info('Wrote centers text file %s', filepath)
