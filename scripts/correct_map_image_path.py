#!/usr/bin/env python
import os.path
import logging
import argparse
from glob import glob
import pyfits


def correct_image_path(fitsfile, directory = "", save_backup = False):
	'''Correct the path of the IMAGE keywords in the header of all the HDU of a fits file'''
	logging.info("Correcting paths in file %s", fitsfile)
	try:
		hdulist = pyfits.open(fitsfile, mode='update', save_backup = save_backup, disable_image_compression = True, do_not_scale_image_data = True)
		for hdu in hdulist:
			for i in range(1,1000):
				keyword = "IMAGE{i:0>3}".format(i=i)
				if keyword in hdu.header:
					hdu.header[keyword] = os.path.join(directory, os.path.basename(hdu.header[keyword]))
					logging.debug("Keyword %s has been updated to %s", keyword, hdu.header[keyword])
				else:
					break
		hdulist.close()
	except IOError, why:
		logging.error("Could not correct file %s: %s", fitsfile, str(why))

# Start point of the script
if __name__ == "__main__":
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Correct the path of the IMAGE keywords in the header of SPoCA maps')
	parser.add_argument('--quiet', '-q', default=False, action='store_true', help='Suppress output to terminal')
	parser.add_argument('--directory', '-d', default='', help='An optional new base directory.')
	parser.add_argument('--backup', '-b', default=False, action='store_true', help='Create a backup of file')
	parser.add_argument('filename', nargs='+', help='The names of the fits files')
	
	args = parser.parse_args()
	
	if args.quiet:
		logging.basicConfig(level = logging.ERROR, format='%(levelname)-8s: %(message)s')
	else:
		logging.basicConfig(level = logging.DEBUG, format='%(levelname)-8s: %(message)s')
	
	filenames = list()
	for filename in args.filename:
		filenames.extend(glob(filename))
	
	for filename in filenames:
		correct_image_path(filename, directory = args.directory, save_backup = args.backup)
