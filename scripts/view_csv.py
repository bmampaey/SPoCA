#!/usr/bin/python
import matplotlib.pyplot as plt
import matplotlib.colorbar as clb
import pandas as pd
import numpy as np
import pyfits
import argparse
import time
from glob import glob
import re
from datetime import datetime, timedelta
import os.path
import datacursor
import matplotlib.dates as mdates


def plot_images(date):
	for axe, images_set in zip(image_axes, images_sets):
		try:
			image_path = images_set[date]
		except Exception:
			print "No image found for date ", date
		else:
			image_filename = os.path.basename(image_path)
			image_title, ext = os.path.splitext(image_filename)
			if ext == ".fits" or ext == ".fts":
				image = np.nan_to_num(pyfits.open(image_path)[0].data)
				minv, maxv = np.percentile(image, [1, 99])
				np.clip(image, minv, maxv, out = image)
				axe.imshow(image, cmap = plt.cm.gray)
			else:
				image = plt.imread(image_path)
				axe.imshow(image)
			axe.set_title(image_title)



def formatter(**kwargs):
	date = datetime.strptime(mdates.DateFormatter('%Y%m%d%H%M%S')(kwargs['x']), '%Y%m%d%H%M%S')
	print "Selected date and value : ", date, date.strftime('EIT.%Y%m%d_%H%M%S.0195.fits'), kwargs['y']
	plot_images(date)
	return mdates.DateFormatter('%d/%m/%Y %H:%M')(kwargs['x'])

if __name__ == "__main__":
	
	parser = argparse.ArgumentParser(description='View an interactive plot of a csv file.')
	parser.add_argument('csv_file', help='The csv file')
	parser.add_argument('--time_column', '-t', default = "date_obs", help='The column to use for the time.')
	parser.add_argument('--columns', '-c', nargs='+', help='The columns to plot.')
	parser.add_argument('--images', '-I', nargs='*', action='append', default = [], help='The paths to images to plot. Must be specified once per channel.')
	args = parser.parse_args()
	
	# Parse the csv file
	csv = pd.read_csv(args.csv_file, parse_dates=args.time_column, index_col = args.time_column)
	
	# Parse the images
	images_sets = list()
	
	find_date = re.compile(r'(?P<time>\d\d\d\d\d\d\d\d_\d\d\d\d\d\d)')
	for image_paterns in args.images:
		images_set = dict()
		for image_patern in image_paterns:
			images_filenames = glob(image_patern)
			for image_filename in images_filenames:
					file_date = find_date.search(image_filename)
					if file_date:
						images_set[datetime.strptime(file_date.group('time'), '%Y%m%d_%H%M%S')] = image_filename
					else:
						print "Error getting date from file " + image_filename
		images_sets.append(images_set)
	
	# Create the figure and the axis
	figure = plt.figure()
	number_rows = 2 * len(args.columns) if args.images else len(args.columns)
	
	# Plot the columns
	plot_axes = list()
	last_axe = None
	for row, column in enumerate(args.columns):
		axe = figure.add_subplot(number_rows, 1, row + 1, ylabel = column, sharex = last_axe)
		axe.scatter(csv.index, csv[column], marker='.', c='k', edgecolors = 'none')
		# We hide the x axes
		plt.setp(axe.get_xmajorticklabels() + axe.get_xminorticklabels(), visible=False)
		last_axe = axe
		plot_axes.append(axe)
	
	# We show the x axes for the last plot
	if last_axe:
		plt.setp(last_axe.get_xmajorticklabels() + last_axe.get_xminorticklabels(), visible=True)
	
	
	# Make the axis for the images
	image_axes = list()
	for column, images_set in enumerate(images_sets):
		axe = figure.add_subplot(2, len(images_sets), len(images_sets) + column + 1)
		# We hide the axis
		axe.get_xaxis().set_visible(False)
		axe.get_yaxis().set_visible(False)
		image_axes.append(axe)
	
	# Plot the very first image
	plot_images(datetime(csv.index[0].year, csv.index[0].month, csv.index[0].day, csv.index[0].hour, csv.index[0].minute, csv.index[0].second))
	
	figure.subplots_adjust(left=0.05, right=0.95, bottom=0.05, top=0.95, hspace=0.1, wspace=0.1)
	figure.set_size_inches(19,12)
	datacursor.datacursor(axes = plot_axes, formatter = formatter, display='multiple', draggable=True, tolerance = 20)
	plt.show()

