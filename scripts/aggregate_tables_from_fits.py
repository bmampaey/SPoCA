#!/usr/bin/env python3

import logging
import pandas
from argparse import ArgumentParser
from glob import iglob
from astropy.io import fits

def get_dataframe_from_files(filepaths, hdu_name_or_index):
	'''Function that reads SPoCA generated map FITS files and returns a DataFrame with the aggregated tables'''
	tables = dict()
	
	for filepath in filepaths:
		logging.info('Extracting table from file %s', filepath)
		
		try:
			with fits.open(filepath) as hdulist:
				data = hdulist[hdu_name_or_index].data
		except Exception as why:
			logging.error('Could not read HDU %s from FITS file %s: %s', hdu_name_or_index, filepath, why)
		else:
			tables[filepath] = pandas.DataFrame(data.tolist(), columns=[c.name for c in data.columns])
	
	return pandas.concat(tables.values(), keys = tables.keys(), names=['Filepath', 'Row'])


def iter_filepaths(glob_patterns):
	'''Return an iterator of file paths'''
	for glob_pattern in glob_patterns:
		for filepath in iglob(glob_pattern, recursive=True):
			yield filepath


# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = ArgumentParser(description='Aggregate all the data from tables contained in SPoCA map FITS files and write them to a CSV file')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	parser.add_argument('--output', '-o', required = True, help = 'Path of the output CSV file')
	group = parser.add_mutually_exclusive_group(required=True)
	group.add_argument('--hdu-name', metavar = 'NAME', dest = 'hdu_name_or_index', help = 'The name of the table HDU to read the table from')
	group.add_argument('--hdu-index', metavar = 'NUMBER', type = int, dest = 'hdu_name_or_index', help = 'The index of the table HDU to read the table from')
	parser.add_argument('filepaths', nargs = '+', metavar = 'FILEPATH', help = 'The path to a SPoCA fits map (accept also a glob pattern)')
	args = parser.parse_args()
	
	# Setup the logging
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	# We get the dataframe from the files and write it as CSV to the output file
	try:
		dataframe = get_dataframe_from_files(iter_filepaths(args.filepaths), args.hdu_name_or_index)
	except Exception as why:
		logging.critical('Could not extract info from files: %s', why)
	else:
		dataframe.to_csv(args.output)
