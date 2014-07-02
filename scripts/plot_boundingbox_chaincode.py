#!/usr/bin/env python

from datetime import datetime, timedelta
import logging
import os.path
import sys
import argparse
from get_events_stats import parse_voevent
import Image, ImageDraw

cdelt1, cdelt2 = 0.6, 0.6
xcenter, ycenter = 2047.5, 2047.5

def HPC2pixloc((x, y), xcenter = 2047.5, ycenter = 2047.5, cdelt1 = 0.6, cdelt2 = 0.6, ytrans = 4096):
	if ytrans:
		return (x/cdelt1) + xcenter, ytrans - ((y/cdelt2) + ycenter)
	else:
		return (x/cdelt1) + xcenter, (y/cdelt2) + ycenter

def draw_rectangle(image, center, size = 5, color = None, fillcolor = None, width = 1):
	
	try:
		xsize = size[0]/2.
		ysize = size[1]/2.
	except Exception:
		xsize = size/2.
		ysize = size/2.
	
	lower_left = center[0] - xsize, center[1] - ysize
	upper_rigth = center[0] + xsize, center[1] + ysize
	
	logging.debug("Rectangle coords: %s %s", lower_left, upper_rigth)
	
	draw = ImageDraw.Draw(image)
	
	if fillcolor or width <= 1:
		draw.rectangle([lower_left,upper_rigth],outline=color,fill=fillcolor)
	if width > 1:
		draw.line([lower_left[0], lower_left[1], lower_left[0], upper_rigth[1]], fill = color, width = width)
		draw.line([lower_left[0], upper_rigth[1], upper_rigth[0], upper_rigth[1]], fill = color, width = width)
		draw.line([upper_rigth[0], upper_rigth[1], upper_rigth[0], lower_left[1]], fill = color, width = width)
		draw.line([upper_rigth[0], lower_left[1], lower_left[0], lower_left[1]], fill = color, width = width)

# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	
	logging.basicConfig(level = logging.DEBUG, format='%(levelname)-8s: %(message)s')
	
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Plot the bounding box and chain code from a voevent xml file onto an image')
	parser.add_argument('filenames', nargs=2, help='The paths of the files. One should be the xml file and the other a png file.')
	args = parser.parse_args()
	
	if len(args.filenames) != 2:
		logging.critical("You need to pass exactly 2 filenames")
		sys.exit(2)
	
	event = None
	image = None
	
	for filename in args.filenames:
		trash, ext = os.path.splitext(filename)
		if ext == ".xml":
			event, event_links = parse_voevent(filename)
		elif ext == ".png":
			image = Image.open(filename)
		else:
			logging.critical("Unknown filetype for file %s", filename)
			sys.exit(2)
	
	if not event:
		logging.critical("Missing event")
		sys.eit(2)
	if not image:
		logging.critical("Missing image")
		sys.exit(2)
	
	# We draw the bounding box
	boundingbox_center = HPC2pixloc(event.box_center, xcenter, ycenter, cdelt1, cdelt2)
	logging.info("Found center in HPC: %s, in pixloc: %s", event.box_center, boundingbox_center)
	
	boundingbox_size = event.box_size[0]/cdelt1, event.box_size[1]/cdelt2
	logging.info("Found size in HPC: %s, in pixloc: %s", event.box_size, boundingbox_size)
	
	draw_rectangle(image, boundingbox_center, boundingbox_size, color = 'green', fillcolor = None, width = 10)
	
	# We draw the chain code point
	chaincode = eval(event.bound_chaincode)
	for i in range(0, len(chaincode), 2):
		point = HPC2pixloc((chaincode[i], chaincode[i+1]), xcenter, ycenter, cdelt1, cdelt2)
		logging.info("Found chaincode point in HPC: %s, in pixloc: %s", (chaincode[i], chaincode[i+1]), point)
		draw_rectangle(image, point, 10, color = 'red', fillcolor = 'red')
	
	image.resize((1024,1024)).save("BoundingBox.png")
	

