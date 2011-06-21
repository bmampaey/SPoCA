#!/usr/bin/env python2.6
import os, os.path, sys
import logging
import argparse
import dateutil.parser
from datetime import datetime
import pyfits




# Return a time in the form yyyymmdd_hhmmss 
def pretty_date(date):
	return date.strftime("%Y%m%d_%H%M%S")

# Return the list of desired keywords from the headers of a fitsfile
def get_keywords(fitsfile, keywords):
	result = [None]*len(keywords)
	try:
		hdulist = pyfits.open(fitsfile)
		for hdu in hdulist:
			for k,keyword in enumerate(keywords):
				if keyword in hdu.header:
					result[k] = hdu.header[keyword]
		
		hdulist.close()
	except IOError, why:
		log.warning("Error reading file " + fitsfile + ": "+ str(why))
	
	return result

# Start point of the script
if __name__ == "__main__":
	
	# Default name for the log file
	log_filename = os.path.splitext(os.path.basename(sys.argv[0]))[0] + '.log'
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Rename fits files like so prefix.date_time.channels.fits.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='Set the logging level to debug')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='Det the logging level to verbose')
	parser.add_argument('--link', '-l', default=False, action='store_true', help='Make a symbolic link instead of renaming the file.')
	parser.add_argument('--prefix', '-p', default=None, help='The prefix. Leave blank if you don\'t want any.')
	parser.add_argument('--channel', '-c', metavar='keyword', default=["WAVELNTH"], action='append', help='The channel keywords. Leave blank if you don\'t want any.')
	parser.add_argument('filename', nargs='+', help='The names of the fits files')
	
	args = parser.parse_args()
	
	console = logging.StreamHandler()
	console.setFormatter(logging.Formatter('%(levelname)-8s: %(message)s'))
	if args.debug:
		console.setLevel(logging.DEBUG)
	elif args.verbose:
		console.setLevel(logging.INFO)
	else:
		console.setLevel(logging.CRITICAL)
	
	# Create logger
	log = logging.getLogger('rename_fits')
	log.addHandler(console)
	
	# set up of the arguments
	prefix = args.prefix
	filenames = args.filename
	keywords = ['t_obs','date_obs','date-obs']
	keywords.extend(args.channel)
	
	for filename in filenames:
		values = get_keywords(filename, keywords)
		log.debug("Found values for file "+ str(filename) + ": " + ' '.join(values))
		t_obs, date_obs1, date_obs2 = values[:3]
		channels = values[3:]
		try:
			if t_obs:
				date = dateutil.parser.parse(t_obs)
			elif date_obs1:
				date = dateutil.parser.parse(date_obs1)
			elif date_obs2:
				date = dateutil.parser.parse(date_obs2)
			else:
				log.critical("No date or time information found in the fits file " + str(filename))
				continue
		except ValueError:
			log.critical("Date is not well formed, cannot parse it.")
			continue
		
		new_path = ""
		
		if prefix:
			new_path += str(prefix) + '.'
		
		new_path += pretty_date(date) + '.'
		
		for i in range(len(channels)):
			if channels[i] != None:
				new_path += str(channels[i]) + '.'
			else:
				log.critical("No keyword " + str(keywords[i+3]) + " was found in file " + str(filename))
		
		new_path += 'fits'
		
		new_path = os.path.join(os.path.dirname(filename), new_path)
		
		try:
			if not args.link:
				log.info("Renaming file " + str(filename) + " to " + new_path)
				os.rename(filename, new_path)
			else:
				log.info("Linking file " + str(filename) + " to " + new_path)
				os.symlink(os.path.abspath(filename), new_path)
		except OSError, why:
			log.critical("Error renaming/linking file " + str(filename) + " to " + new_path + ": " + str(why))
		

