#!/usr/bin/python
import pandas
from glob import glob
import pyfits
from math import sqrt
import numpy
from datetime import datetime, timedelta
import itertools
import sys
import logging
import argparse
from multiprocessing import Pool

rings = {
 1 : (0.00, 0.07),
 2 : (0.07, 0.16),
 3 : (0.16, 0.25),
 4 : (0.25, 0.35),
 5 : (0.35, 0.45),
 6 : (0.45, 0.55),
 7 : (0.55, 0.65),
 8 : (0.65, 0.75),
 9 : (0.75, 0.85),
10 : (0.85, 0.95),
11 : (0.95, 1.05),
12 : (1.05, 1.15),
13 : (1.15, 1.25),
14 : (1.25, 1.35)
}

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
		if debug:
			logging.root.handlers[0].setLevel(logging.DEBUG)
		else:
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


def show_rings(filename):
	'''Write a fits file of the rings for a particularfile'''
	hdu = pyfits.open(filename)[1]
	header = hdu.header
	center_x = header['CRPIX1'] - 1
	center_y = header['CRPIX2'] - 1
	solar_radius = header['R_SUN']
	data = numpy.zeros(hdu.data.shape)
	y, x = numpy.mgrid[-center_y : data.shape[0] - center_y : 1,-center_x : data.shape[1] - center_x : 1]
	distance_map = numpy.sqrt(x*x+y*y) / solar_radius
	for ring, limits in rings.iteritems():
		data[(distance_map >= limits[0]) & (distance_map < limits[1])] = ring
	
	hdu = pyfits.PrimaryHDU(data, header)
	hdulist = pyfits.HDUList([hdu])
	hdulist.writeto(os.path.splitext(filename)+".rings.fits", clobber=True, output_verify='silentfix')


def get_spectra(filename):
	'''Compute the number of pixel in each class and ring'''
	logging.info("Computing spectra for file %s", filename)
	hdu = pyfits.open(filename)[1]
	header = hdu.header
	original_image = header['IMAGE001']
	number_classes = header['CNBRCLAS']
	date = datetime.strptime(header['DATE_OBS'].split('.', 1)[0], '%Y-%m-%dT%H:%M:%S')
	center_x = header['CRPIX1'] - 1
	center_y = header['CRPIX2'] - 1
	solar_radius = header['R_SUN']
	data = hdu.data
	y, x = numpy.mgrid[-center_y : data.shape[0] - center_y : 1,-center_x : data.shape[1] - center_x : 1]
	distance_map = numpy.sqrt(x*x+y*y) / solar_radius
	results = numpy.zeros((len(rings)+1,number_classes+1), dtype=numpy.int)
	# We compute the histogram of each ring
	for ring, limits in rings.iteritems():
		histogram, edges = numpy.histogram(data[(distance_map >= limits[0]) & (distance_map < limits[1])], bins = range(number_classes+2))
		results[ring] = histogram
	# We compute the histogram of everything after the last ring
	histogram, edges = numpy.histogram(data[distance_map >= limits[1]], bins = range(number_classes+2))
	results[0] = histogram
	
	return date, pandas.DataFrame(results, index=['other'] + ['ring'+str(r) for r in range(1,len(rings)+1)], columns=['noclass'] + ['class'+str(c) for c in range(1,number_classes+1)]).T.unstack().rename(lambda x: '(%s;%s)' % x)

def get_spectras(filenames, max_processes = None):
	'''Compute the spectra for many files'''
	pool = Pool(processes = max_processes)
	results = pool.map(get_spectra, filenames)
	return pandas.DataFrame(dict(results))

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Compute the spectra of SPoCA maps.')
	parser.add_argument('--quiet', '-q', default=False, action='store_true', help='Do not display any error message.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='Set the logging level to debug')
	parser.add_argument('maps', nargs='+', help='The paths to the maps.')
	parser.add_argument('--max_processes', '-m', default=None, type=int, help='The maximum number of processes')
	parser.add_argument('--output', '-o', default="output.csv", help='The name of the output csv file.')
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(quiet = args.quiet, verbose = True, debug = args.debug)
	
	filenames = list()
	for maps in args.maps:
		filenames.extend(glob(maps))
	
	if not filenames:
		logging.critical("No file found to process")
		sys.exit(2)
	
	filenames.sort()
	
	spectras = get_spectras(filenames, args.max_processes)
	try:
		spectras.T.to_csv(args.output, index_label = "time")
	except Exception, why:
		logging.error("Could not write spectras to file %s : %s", args.output, str(why))
	else:
		logging.info("Wrote spectras to file %s", args.output)



