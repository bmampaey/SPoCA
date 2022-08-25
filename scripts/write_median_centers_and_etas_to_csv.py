#!/usr/bin/env python3

import logging
import pandas

from argparse import ArgumentParser
from datetime import datetime, timedelta

def parse_interval(hours):
	return timedelta(hours=float(hours))


def get_rolling_median_dataframe(dataframe, interval_before, interval_after):
	'''Return a dataframe of the rolling median of each column'''
	
	records = list()
	
	for time in dataframe['time']:
		before = time - interval_before
		after = time + interval_after
		median = dataframe.loc[(dataframe['time'] >= before) & (dataframe['time'] <= after)].median(numeric_only=True)
		records.append({'time': time, **median})
	
	return pandas.DataFrame.from_records(records, index = 'time').sort_values('time')

def get_fixed_median_dataframe(dataframe):
	'''Return a dataframe of the fixed median of each column'''
	
	median = dataframe.median(numeric_only=True)
	
	records = [
		{ 'time': dataframe['time'].min(), **median },
		{ 'time': dataframe['time'].max(), **median },
	]
	
	return pandas.DataFrame.from_records(records, index = 'time').sort_values('time')

# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = ArgumentParser(description='Calculate the median (or rolling median) of SPoCA class centers and etas from a CSV file and write them to a new CSV file')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	parser.add_argument('--output', '-o', required = True, help = 'Path of the output CSV file')
	parser.add_argument('--start-date', '-s', type = datetime.fromisoformat, help = 'Discard all centers before this date (ISO 8601 format)')
	parser.add_argument('--end-date', '-e', type = datetime.fromisoformat, help = 'Discard all centers after this date (ISO 8601 format)')
	parser.add_argument('--rolling-before', '-b', type = parse_interval, default = timedelta(0), help = 'For a rolling median, specify the interval to use before the time (in hours)')
	parser.add_argument('--rolling-after', '-a', type = parse_interval, default = timedelta(0), help = 'For a rolling median, specify the interval to use after the time (in hours)')
	parser.add_argument('filepath', metavar = 'FILEPATH', help = 'The path to CSV file written by the aggregate_class_centers_and_etas.py script')
	args = parser.parse_args()
	
	# Setup the logging
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	# Create a dataframe from the csv
	try:
		input_dataframe = pandas.read_csv(args.filepath, index_col = 'filepath', parse_dates = ['time'])
	except Exception as why:
		logging.critical('Could not read CSV file %s: %s', args.filepath, why)
		raise
	
	if args.start_date:
		logging.info('Removing all rows before %s', args.start_date)
		logging.debug(input_dataframe.loc[input_dataframe['time'] < args.start_date])
		input_dataframe = input_dataframe.loc[input_dataframe['time'] >= args.start_date]
	
	if args.end_date:
		logging.info('Removing all rows after %s', args.end_date)
		logging.debug(input_dataframe.loc[input_dataframe['time'] >= args.end_date])
		input_dataframe = input_dataframe.loc[input_dataframe['time'] < args.end_date]
	
	if args.rolling_before or args.rolling_after:
		logging.info('Computing rolling median dataframe')
		output_dataframe = get_rolling_median_dataframe(input_dataframe, interval_before = args.rolling_before, interval_after = args.rolling_after)
	
	else:
		logging.info('Computing fix median dataframe')
		output_dataframe = get_fixed_median_dataframe(input_dataframe)
	
	try:
		output_dataframe.to_csv(args.output)
	except Exception as why:
		logging.critical('Could not write CSV file %s: %s', args.output, why)
		raise
	else:
		logging.info('Wrote CSV file %s', args.output)
