#!/usr/bin/env python3

import logging
import argparse
import matplotlib.colors
import skimage.io

from plot_region_map_contours import get_contours_image, overlay_contours_image, save_image
from gradient import gradient

# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Create an image with the contours of several SPoCA region maps, eventually displayed on top of a background')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	group = parser.add_mutually_exclusive_group()
	group.add_argument('--hdu-index', metavar = 'NUMBER', type = int, default = 1, dest = 'image_hdu_name_or_index', help = 'The index of the table HDU containing the image (default 1)')
	group.add_argument('--hdu-name', metavar = 'NAME', dest = 'image_hdu_name_or_index', help = 'The name of the table HDU containing the image')
	parser.add_argument('--background', '-b', metavar = 'FILEPATH', help = 'The path to a background image, if not specified background will be transparent')
	parser.add_argument('--output', '-o', required = True, help = 'Path of the output image')
	parser.add_argument('--image-size', '-s', nargs = 2, metavar = ('WIDTH', 'HEIGHT'), type = int, help = 'The size in pixels of the output image')
	parser.add_argument('--contour-width', '-w', default = 3, type = int, help = 'The width in pixels (before resizing) of the contours')
	parser.add_argument('--colors', '-c', nargs = '*', help = 'A list of CSS colors to use, one for each region map (order is respected)')
	parser.add_argument('region_maps', nargs = '+', metavar = 'FILEPATH', help = 'The path to a region map FITS file')
	args = parser.parse_args()
	
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	# Convert the color names to RGB
	if args.colors is None:
		# The first color of gradient is the background color, so skip it
		colors = [matplotlib.colors.to_rgb(color) for color in gradient[1:]]
	elif len(args.colors) != len(args.region_maps):
		raise ValueError('The number of colors provided do not match the number of regions maps')
	else:
		colors = [matplotlib.colors.to_rgb(color) for color in args.colors]
	
	if args.background :
		image = skimage.io.imread(args.background)
	else:
		image = None
	
	for color, region_map in zip(colors, args.region_maps):
		try:
			contours_image = get_contours_image(region_map, args.image_hdu_name_or_index, [color], contour_width = args.contour_width, image_size = args.image_size)
		except Exception as why:
			logging.exception('Could not get contours image from %s: %s', args.region_map, why)
			raise
		
		if image is None:
			image = contours_image
		else:
			try:
				image = overlay_contours_image(contours_image, image)
			except Exception as why:
				logging.exception('Could not overlay contours over image: %s', why)
				raise
		
	save_image(image, args.output)
