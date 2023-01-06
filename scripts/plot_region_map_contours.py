#!/usr/bin/env python3

import logging
import argparse
import os
import astropy.io.fits
import numpy
import matplotlib.colors
import skimage.transform
import skimage.io
import skimage.color
import skimage.segmentation
from gradient import gradient

def get_contours_image(fits_file, image_hdu_name_or_index, colors, background_color = (0, 0, 0), contour_width = 3, image_size = None):
	'''Return a RGBA image with the region contours of a region map FITS file,
		colors must be a list of RGB triplets to represent the contours
		background_color is a single RGB triplet to represent everything but the contours'''
	
	colors = numpy.array(colors)
	
	if len(colors.shape) != 2 or colors.shape[1] != 3:
		raise ValueError('Parameter colors must be a list of RGB colors')
	
	if len(background_color) != 3:
		raise ValueError('Parameter background_color must be a triplet of RGB color')
	
	with astropy.io.fits.open(fits_file) as hdulist:
		image = hdulist[image_hdu_name_or_index].data
	
	# FITS file arrays coordinate start at the bottom left, while images coordinates start at the top left
	# so it is necessary to flip the image
	image = numpy.flipud(image)
	
	# Use dilation to increase the size of the regions (pixels with a value > 0)
	# then substract the original image so only the external contours remain, and the rest is set to 0
	contours = skimage.segmentation.expand_labels(image, contour_width) - image
	
	# The coloring of the contours is done by using the pixel value of the contour as the index in the array of colors
	# This ensure that tracked region maps keep the same color over time
	# Because indexing start at 0, and 0 is the value of the background, put the last color first
	# And add an alpha channel, so the background is transparent but the contours are solid
	colors = numpy.roll(colors, 1, axis = 0)
	colors = numpy.pad(colors, ((0,0), (0,1)), mode = 'constant', constant_values = 1.)
	image = colors.take(contours, axis = 0, mode = 'wrap')
	image[contours == 0] = (*background_color, 0.)
	
	if image_size is not None:
		# Do not apply antialisaing (smoothing), the pixel values are discrete
		image = skimage.transform.resize(image, image_size, anti_aliasing = False)
	
	return image


def overlay_contours_image(contours_image, background_image, alpha = 1.):
	'''Return the contours image overlayed on top of a background image'''
	
	if len(contours_image.shape) != 3 or contours_image.shape[2] != 4:
		raise ValueError('Parameter contours_image must be a 2D RGBA image, i.e. a 3D numpy array')
	
	if alpha < 0 or alpha > 1:
		raise ValueError('Parameter alpha must be a value between 0 and 1')
	
	# For simplicity we work with floats instead of integers
	contours_image = skimage.util.img_as_float(contours_image)
	background_image = skimage.util.img_as_float(background_image)
	
	# The background image needs to be RGBA for the steps below
	# so convert it to RGB if it is monochannel
	# and add the α channel if it is missing, with the value of 1 to make the image opaque
	if len(background_image.shape) < 3:
		background_image = skimage.color.gray2rgba(background_image)
	elif background_image.shape[2] < 4:
		background_image = numpy.pad(background_image, ((0,0), (0,0), (0,1)), mode = 'constant', constant_values = 1.)
		
	# Resize the background image to be the same size as of the contours image
	# don't specify anti aliasing, skimage will apply it if needed
	background_image = skimage.transform.resize(background_image, contours_image.shape)
	
	# Combine the 2 images by summing them using the α channels of each image as a weight:
	# 1. Use the alpha parameter to change the transparency of the contours image
	# contours image have a totally transparent background (α channel = 0) and opaque contours (α channel = 1)
	# so an alpha parameter < 1 make the contours more transparent
	# 2. Compute the remaining α channel for the background image as (1 - the α channel of the contours)
	# but weighted by it's original value, so if the pixel in the background image was transparent (α channel = 0) it stays transparent
	# 3. Use the α channel of each image as a weight for the R, G and B channels
	# 4. Sum the 2 images
	
	contours_image[:, :, 3] *= alpha
	background_image[:, :, 3] *= (1. - contours_image[:, :, 3])
	
	# Make the α channel the same size as the RGB channels by repeating it 3 times
	contours_image[: , :, :3] *= numpy.repeat(contours_image[:, :, 3][:,:,None], 3, axis = 2)
	background_image[: , :, :3] *= numpy.repeat(background_image[:, :, 3][:,:,None], 3, axis = 2)
	
	return contours_image + background_image


def save_image(image, filepath):
	'''Save an image to file'''
	
	# Only png allow to have an α channel, so remove it for other image format
	if os.path.splitext(filepath)[1].lower() != '.png' and image.shape[1] > 3:
		image = skimage.color.rgba2rgb(image)
	
	skimage.io.imsave(filepath, image)


# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Create an image with the contours of a SPoCA region map, eventually displayed on top of a background')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	group = parser.add_mutually_exclusive_group()
	group.add_argument('--hdu-index', metavar = 'NUMBER', type = int, default = 1, dest = 'image_hdu_name_or_index', help = 'The index of the table HDU containing the image (default 1)')
	group.add_argument('--hdu-name', metavar = 'NAME', dest = 'image_hdu_name_or_index', help = 'The name of the table HDU containing the image')
	parser.add_argument('--background', '-b', metavar = 'FILEPATH', help = 'The path to a background image, if not specified background will be transparent')
	parser.add_argument('--output', '-o', required = True, help = 'Path of the output image')
	parser.add_argument('--image-size', '-s', nargs = 2, metavar = ('WIDTH', 'HEIGHT'), type = int, help = 'The size in pixels of the output image')
	parser.add_argument('--contour-width', '-w', default = 3, type = int, help = 'The width in pixels (before resizing) of the contours')
	parser.add_argument('--colors', '-c', nargs = '*', help = 'A list of CSS colors to use')
	parser.add_argument('region_map', metavar = 'FILEPATH', help = 'The path to a region map FITS file')
	args = parser.parse_args()
	
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	# Convert the color names to RGB
	if args.colors is None:
		# The first color of gradient is the background color, so skip it
		colors = [matplotlib.colors.to_rgb(color) for color in gradient[1:]]
	else:
		colors = [matplotlib.colors.to_rgb(color) for color in args.colors]
	
	try:
		contours_image = get_contours_image(args.region_map, args.image_hdu_name_or_index, colors, contour_width = args.contour_width, image_size = args.image_size)
	except Exception as why:
		logging.exception('Could not get contours image from %s: %s', args.region_map, why)
		raise
	
	if args.background :
		background_image = skimage.io.imread(args.background)
		try:
			image = overlay_contours_image(contours_image, background_image)
		except Exception as why:
			logging.exception('Could not overlay contours over image %s: %s', args.background, why)
			raise
	else:
		image = contours_image
	
	save_image(image, args.output)
