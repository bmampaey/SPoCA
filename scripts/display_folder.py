#! /usr/bin/env python2.6

import os, sys, argparse

try:
	import cv
except ImportError, why:
	print "You need to have opencv for python installed to use this script"
	sys.exit(2)



def show_image(number):
	image = cv.LoadImage(imagenames[number])
	cv.ShowImage(window, image)


if __name__ == '__main__':
	folder = sys.argv[1]
	window = folder
	imagenames = sorted(os.listdir(folder))
	
	cv.NamedWindow(window, 1);
	cv.CreateTrackbar( "Image", window, 0, len(imagenames)-1, show_image)
	show_image(0)
	
	cv.WaitKey(0)
