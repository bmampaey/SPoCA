from datetime import datetime, timedelta
import dateutil.parser

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
		if key.lower() in ['event_npixels', 'area_raw', 'area_uncert', 'area_atdiskcenter','area_atdiskcenteruncert', 'ar_intensmin', 'ar_intensmax', 'ar_intensmean','ar_intensvar', 'ar_intensskew', 'ar_intenskurt', 'ar_intenstotal', 'ar_intenskurt']:
			try:
				value = float(value)
			except Exception:
				pass
		setattr(self,key.lower(),value)
	
	def get_str(self, key):
		try:
			value = getattr(self, key.lower())
		except AttributeError, why:
			print "Event does not have the attribute " + str(key)
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
			seen_keys = []
			for key in keys:
				if key.lower() in seen_keys:
					continue
				else:
					seen_keys.append(key.lower())
				
				if result:
					result += sep + str(key) + ': ' + self.get_str(key)
				else:
					result = str(key) + ': ' + self.get_str(key)
		else:
			result = str(keys)+ ': ' + self.get_str(keys)
		
		return result

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
			print "Color of event differ from MetaEvent color"
		
		self.events.append(event)
		return self
	
	
	def lifetime(self):
		if not self.events:
			return timedelta(seconds=0)
		else:
			self.events.sort()
			return self.events[-1].time - self.events[0].time
	
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
	events = list()
	try:
		hdulist = pyfits.open(filename)
		for hdu in hdulist:
			if isinstance(hdu, pyfits.BinTableHDU):
				attributes = [column.name.lower() for column in hdu.columns]
				for row in hdu.data:
					event = Event()
					for i, attribute in enumerate(attributes):
						event.set(attribute, row[i])
					if 'xcenter' in attributes and 'xcenter' in attributes:
						event.set('center', (event.xcenter, event.ycenter))
					if 'tracked_color' in attributes:
						event.set('color', event.tracked_color)
					events.append(event)
		
		hdulist.close()
	except IOError, why:
		log.warning("Error reading file " + fitsfile + ": "+ str(why))
	return events

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
		print "Error parsing voevent from file " + str(filename) + ": " + str(why)
	
	event_links = dict()
	for p in dom.getElementsByTagName('Reference'):
		if p.getAttribute("name") == "Edge":
			event_links[get_uri(p.getAttribute("uri"))] = p.getAttribute("type")
	
	return event, event_links
