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

def what_ring(r):
	for ring, borders in rings.iteritems():
		if r >= borders[0] and r < borders[1]:
			return ring
	return 0

def show_rings(filename):
	
	hdu = pyfits.open(filename)[1]
	header = hdu.header
	center_x = header['CRPIX1'] - 1
	center_y = header['CRPIX2'] - 1
	radius = header['R_SUN']
	data = hdu.data
	for y in range(data.shape[0]):
		for x in range(data.shape[1]):
			try:
				data[y][x] = what_ring(sqrt((x - center_x)**2 + (y - center_y)**2)/radius)
			except Exception, why:
				logging.error("Error getting value of ring: %s", str(why))
	hdu = pyfits.PrimaryHDU(data, header)
	hdulist = pyfits.HDUList([hdu])
	hdulist.writeto(os.path.splitext(filename)+".rings.fits", clobber=True, output_verify='silentfix')


def get_spectra(filename):
	
	hdu = pyfits.open(filename)[1]
	header = hdu.header
	original_image = header['IMAGE001']
	number_classes = header['CNBRCLAS']
	date = datetime.strptime(header['DATE_OBS'].split('.', 1)[0], '%Y-%m-%dT%H:%M:%S')
	center_x = header['CRPIX1'] - 1
	center_y = header['CRPIX2'] - 1
	radius = header['R_SUN']
	data = hdu.data
	results = numpy.zeros((len(rings)+1,number_classes+1), dtype=numpy.int)
	for y in range(data.shape[0]):
		for x in range(data.shape[1]):
			try:
				ring = what_ring(sqrt((x - center_x)**2 + (y - center_y)**2)/radius)
				results[ring][data[y][x]] += 1
			except Exception, why:
				logging.error("Error getting value of ring: %s", str(why))

	return date, pandas.DataFrame(results, index=['other'] + ['ring'+str(r) for r in range(1,len(rings)+1)], columns=['noclass'] + ['class'+str(c) for c in range(1,number_classes+1)])

def get_spectras(filenames):
	results = dict()
	for filename in filenames:
		date, frame = get_spectra(filename)
		results[date] = frame
	
	return pandas.Panel(results)

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Compute the spectra of SPoCA maps.')
	parser.add_argument('maps', nargs='+', help='The paths to the maps.')
	parser.add_argument('--output', '-o', default="output.csv", help='The name of the output csv file.')
	args = parser.parse_args()
	
	try:
		output_file = open(args.output, 'w')
	except Exception, why:
		logging.critical("Error opening output file %s : %s", output_filename, str(why))
		sys.exit(2)
	
	filenames = list()
	for maps in args.maps:
		filenames.extend(glob(maps))
	
	filenames.sort()
	
	write_header = True
	for filename in filenames:
		logging.info("Computing spectra for file %s", filename)
		try:
			date, spectra = get_spectra(filename)
		except Exception, why:
			logging.error("Could not process file %s : %s", filename, str(why))
		else:
			if write_header:
				output_file.write("time")
				for header in itertools.product(spectra.index, spectra.columns):
					output_file.write(", " + str(header))
				output_file.write("\n")
				write_header = False
			output_file.write(str(date) + ", ")
			output_file.write(", ".join(str(v) for v in spectra.stack()) + "\n")
			output_file.flush()
	logging.info("No more files to process")
	output_file.close()



