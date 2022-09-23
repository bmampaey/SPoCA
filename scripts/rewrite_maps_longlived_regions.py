#!/usr/bin/env python3

import logging
import pandas
import numpy
from argparse import ArgumentParser
from pathlib import Path
from astropy.io import fits


def rewrite_map(input_filepath, image_hdu_name_or_index, output_filepath, longlived_regions_colors, background_color = 0):
	'''Write a new FITS map with the image but keep only the regions in the image with a value in longlived_regions_colors'''
	
	with fits.open(input_filepath) as hdulist:
		image_hdu = hdulist[image_hdu_name_or_index]
		image = image_hdu.data
		
		# Make the list of region's colors to be erase in the image
		erase_colors = set(numpy.unique(image)) - longlived_regions_colors - set([background_color])
		
		# If there are no regions to erase, rewrite the file anyway
		if erase_colors:
			logging.debug('Erasing regions with colors %s in file %s', erase_colors, input_filepath)
			# numpy.isin does not accept a "set" as a parameter so convert erase_colors to a list
			image[numpy.isin(image, list(erase_colors))] = background_color
		else:
			logging.debug('No regions to erase in file %s', input_filepath)
		
		# Note that if the image HDU is compressed, the image in the output file will also be compressed
		image_hdu.writeto(output_filepath, checksum = True)
		logging.info('Wrote file %s', output_filepath)


# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = ArgumentParser(description='Re-write SPoCA map FITS files but only keep regions that have a minimum lifespan')
	parser.add_argument('--min-lifespan', '-m', required = True, type = pandas.Timedelta, help = 'The minimum lifespan')
	parser.add_argument('--lifespan-csv', metavar = 'FILEPATH', required = True, help = 'The path to the CSV file containing the lifespan of the regions as generated by the script write_regions_lifespan_to_csv.py')
	parser.add_argument('--background-color', metavar = 'NUMBER', default = 0, help = 'The value to set the pixels of the regions that will be erased (default to 0)')
	parser.add_argument('--output', '-o', default = '.', type = Path, help = 'Path of the output directory')
	group = parser.add_mutually_exclusive_group()
	group.add_argument('--hdu-index', metavar = 'NUMBER', type = int, default = 0, dest = 'image_hdu_name_or_index', help = 'The index of the table HDU containing the image (default 0)')
	group.add_argument('--hdu-name', metavar = 'NAME', dest = 'image_hdu_name_or_index', help = 'The name of the table HDU containing the image')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	parser.add_argument('filepaths', nargs = '+', metavar = 'FILEPATH', help = 'The path to a SPoCA FITS map')
	args = parser.parse_args()
	
	# Setup the logging
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	# Create a dataframe from the region csv
	try:
		lifespan_dataframe = pandas.read_csv(args.lifespan_csv, usecols = ['TRACKED_COLOR', 'LIFESPAN'], converters = {'LIFESPAN': pandas.to_timedelta})
	except Exception as why:
		logging.critical('Could not read CSV file %s: %s', args.lifespan_csv, why)
		raise
	
	longlived_regions_colors = set(lifespan_dataframe.loc[lifespan_dataframe['LIFESPAN'] >= args.min_lifespan, 'TRACKED_COLOR'])
	
	for filepath in args.filepaths:
		rewrite_map(filepath, args.image_hdu_name_or_index, args.output / Path(filepath).name, longlived_regions_colors, args.background_color)
