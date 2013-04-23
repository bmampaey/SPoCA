#!/usr/bin/env python2.6

from datetime import datetime, timedelta
import dateutil.parser
import logging
from Queue import Queue
import os.path
import sys
import argparse
from glob import glob

class Event:
	def __init__(self, type = None, time = None, color = None):
		self.type = type
		self.time = time
		self.color = color
	
	def __str__(self):
		return self.to_str(['color', 'time'], '\t')
	
	def __cmp__(self, other):
		return cmp(self.time, other.time)
	
	def set(self, key, value):
		if key.lower() in ['event_npixels', 'area_raw', 'area_uncert', 'area_atdiskcenter','area_atdiskcenteruncert', 'intensmin', 'intensmax', 'intensmean','intensvar', 'intensskew', 'intenskurt', 'intenstotal', 'intensmedian']:
			try:
				value = float(value)
			except Exception:
				pass
		setattr(self,key.lower(),value)
	
	def get_str(self, key):
		try:
			value = getattr(self, key.lower())
		except AttributeError, why:
			logging.error("Event does not have the attribute %s", key)
			return 'NA'
		
		if isinstance(value, datetime):
			return value.strftime('%Y-%m-%d %H:%M:%S')
		elif isinstance(value, float):
			return '%.2f' % value
		elif isinstance(value, tuple):
			return '(%.2f, %.2f)' % value
		else:
			return str(value)
	
	def to_str(self, keys = None, sep = '\n'):
		result = ''
		if not keys:
			return result
		
		if isinstance(keys, (list, tuple)):
			for key in keys:
				if result:
					result += sep + self.get_str(key)
				else:
					result = self.get_str(key)
		else:
			result = self.get_str(keys)
		
		return result
	
	def attributes(self):
		return self.__dict__.keys()

class MetaEvent:
	def __init__(self, color=None, events=None):
		self.color = color
		self.events = []
		if events:
			for event in events:
				self.add(event)
	
	def __iadd__(self, event):
		self.add(event)
		return self
	
	def add(self, event):
		if self.color == None:
			self.color = event.color
		elif self.color != event.color:
			logging.critical("Color of event %s differ from MetaEvent color %s", event.color, self.color)
		
		self.events.append(event)
		return self
	
	
	def lifetime(self):
		if not self.events:
			return timedelta(seconds=0)
		else:
			self.events.sort()
			return self.events[-1].time - self.events[0].time
	
	def estimated_lifetime(self):
		if not self.events:
			return timedelta(seconds=0)
		else:
			self.events.sort()
			return self.events[-1].time - min(self.get("first_time"))
	
	def start_time(self):
		if not self.events:
			return None
		else:
			self.events.sort()
			return self.events[0].time
	
	def end_time(self):
		if not self.events:
			return None
		else:
			self.events.sort()
			return self.events[-1].time
	
	def get(self, attributes, start_time=None, end_time=None):
		self.events.sort()
		
		values = list()
		
		if isinstance(attributes, (list, tuple)):
			for attribute in attributes:
				attribute_values = list()
				for e in self.events:
					if (start_time == None or e.time >= start_time) and (end_time == None or e.time < end_time):
						try:
							attribute_values.append(getattr(e, attribute.lower()))
				
						except AttributeError, why:
							#print "Event does not have the attribute " + str(attribute)
							attribute_values.append(None)
				values.append(attribute_values)
		
		else:
			for e in self.events:
				if (start_time == None or e.time >= start_time) and (end_time == None or e.time < end_time):
					try:
						values.append(getattr(e, attributes.lower()))
				
					except AttributeError, why:
						#print "Event does not have the attribute " + str(attribute)
						values.append(None)
		
		return values

def parse_map(filename):
	import pyfits
	events = dict()
	try:
		hdulist = pyfits.open(filename)
		for hdu in hdulist:
			if type(hdu) is pyfits.BinTableHDU:
				attributes = [column.name.lower() for column in hdu.columns]
				try:
					id_column = attributes.index('id')
				except ValueError:
					logging.debug("%s is not a region table, skipping", hdu.name)
					continue
				
				event_type = 'NA'
				if 'EXTNAME' in hdu.header:
					event_type = hdu.header['EXTNAME']
				
				for row in hdu.data:
					event_id = row[id_column]
					if (event_type, event_id) not in events:
						events[(event_type, event_id)] = Event(type = event_type, color = event_id)
					
					event = events[(event_type, event_id)]
					for i, attribute in enumerate(attributes):
						event.set(attribute, row[i])
					if 'date_obs' in attributes:
						event.set('time', dateutil.parser.parse(event.date_obs))
					if 'first_date_obs' in attributes:
						event.set('first_time', dateutil.parser.parse(event.first_date_obs))
					if 'raw_area' in attributes:
						event.set('area_raw', event.raw_area)
					if 'xcenter' in attributes and 'ycenter' in attributes:
						event.set('center', (event.xcenter, event.ycenter))
					if 'xboxmin' in attributes and 'yboxmin' in attributes:
						event.set('boxmin', (event.xboxmin, event.yboxmin))
					if 'xboxmax' in attributes and 'yboxmax' in attributes:
						event.set('boxmax', (event.xboxmax, event.yboxmax))
					if 'tracked_color' in attributes:
						event.set('color', event.tracked_color)
		
		hdulist.close()
	except IOError, why:
		logging.warning("Error reading file %s: %s", filename ,why)
	return events.values()

def parse_voevent(filename):
	import re
	import xml.dom.minidom
	def get_coordinates(element):
		return (float(element.getElementsByTagName('C1')[0].firstChild.data), float(element.getElementsByTagName('C2')[0].firstChild.data))
	
	def get_date(element):
		return dateutil.parser.parse(element.getElementsByTagName('ISOTime')[0].firstChild.data)
	
	def get_uri(value):
		uri = re.search(r'([a-zA-Z0-9_]+)$', value)
		if uri:
			return uri.group(1)
		else:
			raise Exception("uri not found")
			return None
	
	try:
		dom = xml.dom.minidom.parse(filename)
		event = Event()
		for p in dom.getElementsByTagName('Param'):
			event.set(p.getAttribute("name").lower(), p.getAttribute("value"))
		event.set("time", get_date(dom.getElementsByTagName('TimeInstant')[0]))
		event.set("start_time", get_date(dom.getElementsByTagName('StartTime')[0]))
		event.set("stop_time", get_date(dom.getElementsByTagName('StopTime')[0]))
		event.set("center", get_coordinates(dom.getElementsByTagName('Value2')[0]))
		event.set("center_error", get_coordinates(dom.getElementsByTagName('Error2')[0]))
		event.set("box_center", get_coordinates(dom.getElementsByTagName('Center')[0]))
		event.set("box_size", get_coordinates(dom.getElementsByTagName('Size')[0]))
		event.set('type', dom.getElementsByTagName('Concept')[0].firstChild.data)
		event.set('uri', get_uri(dom.getElementsByTagName('AuthorIVORN')[0].firstChild.data))
		event.set('color', int(re.search(r"(\d+)$", event.frm_specificid).group(1)))
		
	except Exception, why:
		logging.critical("Error parsing voevent from file %s: %s", filename, why)
	
	event_links = dict()
	for p in dom.getElementsByTagName('Reference'):
		if p.getAttribute("name") == "Edge":
			event_links[get_uri(p.getAttribute("uri"))] = p.getAttribute("type")
	
	return event, event_links

def get_events(filenames):
	'''Function that parses files to extract events'''
	events = list()
	
	for filename in filenames:
		
		if os.path.splitext(filename)[1].lower() == ".xml":
			# We parse the xml file
			event, links = parse_voevent(filename)
			logging.info("Event from file %s: %s", filename, event)
			events.append(event)
		
		elif os.path.splitext(filename)[1].lower() == ".fits":
			# We parse the map
			new_events = parse_map(filename)
			logging.info("Parsed %s events from file %s", len(new_events), filename)
			logging.debug("\n".join([str(e) for e in new_events]))
			events.extend(new_events)
		
		else:
			logging.critical("Unknown file type %s", filename)
	
	return events

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

# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Gather all events stats into a csv file.')
	parser.add_argument('--quiet', '-q', default=False, action='store_true', help='Do not display any error message.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='Set the logging level to debug')
	parser.add_argument('--output', '-o', default='events_stats.csv', help='Name of the csv file for the events stats.')
	parser.add_argument('filename', nargs='+', help='The paths of the files. Can be SPoCA fits maps or voevents')
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(quiet = args.quiet, verbose = True, debug = args.debug)
	
	# We glob the filenames
	filenames = list()
	for filename in args.filename:
		if os.path.exists(filename):
			filenames.append(filename)
		else:
			files = sorted(glob(filename))
			if files:
				filenames.extend(files)
			else:
				logging.warning("File %s not found, skipping!", filename)
	
	# Get the events from the files
	events = get_events(filenames)
	
	# We make the set of column names
	column_names = set()
	for event in events:
		column_names.update(set(event.attributes()))
	column_names = list(column_names)
	
	# We write the csv file
	try:
		with open(args.output, 'w') as csv_file:
			csv_file.write(','.join(column_names))
			csv_file.write("\n")
			for event in events:
				csv_file.write(event.to_str(column_names, sep = ','))
				csv_file.write("\n")
	except Exception, why:
		logging.critical("Could not write csv file %s: %s", str(args.output), str(why))

