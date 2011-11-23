#!/usr/bin/env python2.6

from datetime import datetime, timedelta
import dateutil.parser
import re
import sys
import os.path
import argparse
import matplotlib.dates as mdates
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import logging



map_centers = re.compile(r'(?P<channels>\(\d+\.?\d*(, *\d+\.?\d*)*\)) *(?P<class_centers>\[\(\d+\.?\d*(, *\d+\.?\d*)*\)(, *\(\d+\.?\d*(, *\d+\.?\d*)*\))*\])')

def parse_map(filename):
	import pyfits
	channels = None
	class_centers = None
	time = None
	try:
		hdulist = pyfits.open(filename)
		for hdu in hdulist:
			if isinstance(hdu, (pyfits.CompImageHDU, pyfits.ImageHDU)):
				if 'T_OBS' in hdu.header:
					time = dateutil.parser.parse(hdu.header['t_obs'])
				if 'CHANNELS' in hdu.header:
					channels = hdu.header['CHANNELS']
					class_centers = list()
					for i in range(100):
						center = 'CLSCTR' + ('%02d' % i)
					if center in hdu.header:
						class_centers.append(eval(hdu.header[center]))
					else:
						break
		
		hdulist.close()
	
	except Exception, why:
		log.critical("Error getting class_centers from file " + filename + ": "+ str(why))
	return time, class_centers, channels

find_voevent_channels = re.compile(r'spocaChannels *= *(?P<channels>\(\d+\.?\d*(, *\d+\.?\d*)*\));')
find_voevent_centers = re.compile(r'spocaCenters *= *(?P<class_centers>\[\(\d+\.?\d*(, *\d+\.?\d*)*\)(, *\(\d+\.?\d*(, *\d+\.?\d*)*\))*\]);')
def parse_voevent(filename):
	import re
	import xml.dom.minidom
	channels = None
	class_centers = None
	time = None
	try:
		dom = xml.dom.minidom.parse(filename)
		FRM_ParamSet = dom.getElementsByTagName('lmsal:FRM_ParamSet')[0].firstChild.data
		
		spocaCenters = find_voevent_centers.search(FRM_ParamSet)
		if spocaCenters:
			class_centers = eval(spocaCenters.group('class_centers'))
		
		spocaChannels = find_voevent_channels.search(FRM_ParamSet)
		if spocaChannels:
			channels = eval(spocaChannels.group('channels'))
		
		time = dateutil.parser.parse(dom.getElementsByTagName('TimeInstant')[0].getElementsByTagName('ISOTime')[0].firstChild.data)
	except Exception, why:
		log.critical("Error getting class_centers from file " + filename + ": "+ str(why))
	
	return time, class_centers, channels


timedelta_regex = re.compile(r'((?P<days>\d+?)d)?((?P<hours>\d+?)h)?((?P<minutes>\d+?)m)?')
def parse_duration(duration):
	""" Parse a duration string to a timedelta """
	parts = timedelta_regex.match(duration)
	if not parts:
		log.critical("Cannot convert duration to timedelta : " + str(duration))
		return None
	
	parts = parts.groupdict()
	time_params = {}
	for (name, param) in parts.iteritems():
		if param:
			time_params[name] = int(param)
	return timedelta(**time_params)

def pretty_date(date):
	""" Format a datetime to yyyymmdd_hhmmss """
	return date.strftime("%Y%m%d_%H%M%S")


linestyles = ['-' , '--' , '-.' , ':']

def class_centers_figure(class_centers, channels, joined=False, size=None, legend=False):
	
	if size:
		figure = plt.figure(figsize=(size[0]/100., size[1]/100.), dpi=100)
	else:
		figure = plt.figure()
	
	if legend:
		figure.subplots_adjust(bottom=.1, hspace=.18, top=.95, left = .07, right = .85)
	else:
		figure.subplots_adjust(bottom=.1, hspace=.18, top=.95, left = .07, right = .95)
	
	number_classes = 0
	number_channels = len(channels)
	for class_center in class_centers.values():
		if class_center != None:
			number_classes = len(class_center)
			break
	
	
	# We precreate all the axes
	# Could be more easily done with plt.subplots but it is only available from version 1.0.1
	if joined:
		last_axes = figure.add_subplot(1,1,1)
		axes = number_classes * [last_axes]
	else:
		last_axes = None
		axes = list()
		for c in range(number_classes):
			axes.append(figure.add_subplot(number_classes, 1, c+1, title='Center ' + str(c+1), sharex=last_axes))
			last_axes = axes[-1]
	
	# We create the list of values of the class centers
	ylimits = dict()
	values = dict()
	for c in range(number_classes):
		values[c] = list()
		ylimits[c] = (float('inf'), float('-inf'))
		
	times = class_centers.keys()
	times.sort()
	for t in times:
		if not class_centers[t]:
			for c in range(number_classes):
				values[c].append(tuple(number_channels*[None]))
		else:
			for c, class_center in enumerate(class_centers[t]):
				values[c].append(class_center)
				ylimits[c] = min(ylimits[c][0], min(class_center)), max(ylimits[c][1], max(class_center)) 
	
	# We stretch a little the ylimits
	for c in range(number_classes):
		ymin, ymax = ylimits[c]
		stretch = (ymax - ymin) * 0.05
		
		if ymin >= 0 and ymin < stretch:
			ymin = 0
		else:
			ymin -= stretch
		ylimits[c] = (ymin, ymax+stretch)
	
	lines = list()
	labels = list()
	
	# We make the plots
	for c in range(number_classes):
		
		color = cm.jet(float(c)/number_classes)
		line = axes[c].plot(times, values[c], color=color)
		
		for i, l in enumerate(line):
			l.set_linestyle(linestyles[i])
			lines.append(l)
			labels.append('Class ' + str(c) + ' ' + str(channels[i]))
	
	# We set the ylimits
	if joined:
		ymin, ymax = ylimits[0]
		for c in range(1,number_classes):
			ymin = min(ymin, ylimits[c][0])
			ymax = max(ymax, ylimits[c][1])
			last_axes.set_ylim((ymin, ymax), auto=False)
	else:
		for c in range(number_classes):
			axes[c].set_ylim(ylimits[c], auto=False)
	
	# We set up the ticks labels on the last axes
	last_axes.xaxis.set_major_locator(mdates.HourLocator(byhour=[0]))
	last_axes.xaxis.set_major_formatter(mdates.DateFormatter('%b %d'))
	for tick in last_axes.xaxis.get_major_ticks():
		tick.label.set_rotation(45)
		tick.label.set_fontweight(700)
		tick.label.set_horizontalalignment('right')
	
	last_axes.xaxis.set_minor_locator(mdates.HourLocator(byhour=[6,12,18]))
	last_axes.xaxis.set_minor_formatter(mdates.DateFormatter('%H:%M'))
	for tick in last_axes.xaxis.get_minor_ticks():
		tick.label.set_rotation(45)
		tick.label.set_horizontalalignment('right')
	
	# We hide the tick labels on the previous axes
	if not joined:
		for c in range(number_classes-1):
			plt.setp(axes[c].get_xmajorticklabels() + axes[c].get_xminorticklabels(), visible=False)
	
	if legend:
		figure.legend(lines, labels, loc='center right', fancybox=True, ncol=1)
	
	
	#figure.autofmt_xdate()
	return figure


def save_figure(suffix, figure, time_ticks, max_delta, start_time, end_time): 
	
	# For each time tick we save a figure
	while not terminate_threads:
		
		time_tick = time_ticks.get()
		
		# We draw lines on the figure axes at the time tick
		for axes in figure.axes:
			axes.axvline(x=time_tick, linewidth=4, color='black', alpha=0.3)
	
		# We set up the time boundaries
		if time_tick - max_delta/2 < start_time:
			tmin = start_time
		elif time_tick + max_delta/2 > end_time:
			tmin = max(end_time - max_delta, start_time)
		else:
			tmin = time_tick - max_delta/2
	
		tmax = tmin + max_delta
		for axes in figure.axes:
			axes.set_xlim((tmin, tmax), auto=False)
	
		filename = pretty_date(time_tick) + '.' + suffix
		log.info("Saving partial figure for time " + str(time_tick) + " as " +  filename)
		figure.savefig(filename)
		
		time_ticks.task_done()
		
		# We erase the lines
		for axes in figure.axes:
			del axes.lines[-1]


# Return a time in the form yyyymmdd_hhmmss 
def pretty_date(date):
	return date.strftime("%Y%m%d_%H%M%S")

# Start point of the script
if __name__ == "__main__":
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Generate a graph of the voevents.')
	parser.add_argument('--output', '-o', default='events.png', help='Name of the file for graph.')
	parser.add_argument('--size', '-s', default=None, help='The size of the plot. Specify as widthxheight')
	parser.add_argument('--multi', '-M', nargs='?', default=None, const='', help='To have multiple graphs, one for each time. Specify a max duration like 0d0h0m0s')
	parser.add_argument('--joined', '-J', default=False, action='store_true', help='Set to plot all centers into one graph')
	parser.add_argument('--legend', '-L', default=False, action='store_true', help='Set to add a legend')
	parser.add_argument('--threading', '-t', default=1, type=int, help='Set to a value bigger than 1 if you want multithreading.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='Set the logging level to debug')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='Set the logging level to verbose')
	parser.add_argument('filename', nargs='+', help='The names of the xml files')

	args = parser.parse_args()
	
	# Create logger
	if args.debug:
		logging.basicConfig(level = logging.DEBUG, format='%(levelname)-8s: %(message)s')
	elif args.verbose:
		logging.basicConfig(level = logging.INFO, format='%(levelname)-8s: %(message)s')
	else:
		logging.basicConfig(level = logging.CRITICAL, format='%(levelname)-8s: %(message)s')
	
	log = logging.getLogger('plot_class_centers')
	
	# Parse the arguments
	filenames = args.filename
	
	if args.size:
		m = re.search(r'(\d+)\D(\d+)', args.size)
		if not m:
			log.critical("Error in size specification")
			sys.exit(2)
		size = (int(m.group(1)), int(m.group(2)))
	else:
		size=None
	
	# We parse the files to extract the class centers
	channels = None
	class_centers = dict()
	for filename in filenames:
		
		if os.path.splitext(filename)[1].lower() == ".xml":
			time, class_center, channel = parse_voevent(filename)
		
		elif os.path.splitext(filename)[1].lower() == ".fits":
			time, class_center, channel = parse_map(filename)
		
		else:
			log.critical("Unknown file type " + str(filename))
			continue
		
		if channel == None or class_center == None:
			log.critical("Class centers not found in file " + str(filename))
			continue
		
		else:
			log.info("Class centers found in file " + str(filename)+ " : "+ str(class_center))
		
		if channels == None:
			channels = channel
		
		if channel != channels:
			log.critical("Channels in file " + str(filename) + " do not match previous channels : "+ str(channel)+ " != "+ str(channels))
		
		if time not in class_centers or not class_centers[time]:
			class_centers[time] = class_center

	
	# We create the big figure
	figure = class_centers_figure(class_centers, channels, joined=args.joined, size=size, legend=args.legend)
	
	# We save the full figure
	log.info("Saving full figure as " + str(args.output))
	figure.savefig(args.output)
	
	# If the user requested multi plots
	if args.multi != None:
		
		max_delta = parse_duration(args.multi)
		
		# We collect all the time ticks for which we have class centers
		time_ticks = class_centers.keys()
		time_ticks.sort()
		
		if args.threading > 1:
			import threading, Queue, copy
			terminate_threads = False
			time_ticks_queue = Queue.Queue()
			threads = list()
			while len(threads) < args.threading:
				if figure == None:
					figure = class_centers_figure(class_centers, channels, joined=args.joined, size=size, legend=args.legend)
				threads.append(threading.Thread(group=None, target=save_figure, name='save_figure_'+str(len(threads)), args=(args.output, figure, time_ticks_queue, max_delta, time_ticks[0], time_ticks[-1]), kwargs={}))
				threads[-1].daemon = True
				threads[-1].start()
				figure = None
			
			for time_tick in time_ticks:
				time_ticks_queue.put(time_tick)
			try:
				time_ticks_queue.join()
			except Exception, why:
				terminate_threads = True
				log.critical("Stopping after error : " + str(why))
		else:
		
			# For each time tick we save a figure
			for time_tick in time_ticks:
				
				# We draw lines on the figure axes at the time tick
				for axes in figure.axes:
					axes.axvline(x=time_tick, linewidth=4, color='black', alpha=0.3)
				
				# We set up the time boundaries
				if time_tick - max_delta/2 < time_ticks[0]:
					tmin = time_ticks[0]
				elif time_tick + max_delta/2 > time_ticks[-1]:
					tmin = max(time_ticks[-1] - max_delta, time_ticks[0])
				else:
					tmin = time_tick - max_delta/2
				
				tmax = tmin + max_delta
				for axes in figure.axes:
					axes.set_xlim((tmin, tmax), auto=False)
				
				filename = pretty_date(time_tick)+'.'+args.output
				log.info("Saving partial figure for time " + str(time_tick) + " as " +  filename)
				figure.savefig(filename)
				
				# We erase the lines
				for axes in figure.axes:
					del axes.lines[-1]

