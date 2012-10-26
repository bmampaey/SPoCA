#!/usr/bin/env python2.6
import os, os.path, sys
from datetime import datetime
import pyfits
import dateutil.parser
import subprocess
import glob

class segmentation:
	bin = "bin/classification.x"
	args = dict()
	extra = None
	output_dir = "results"
	map_types = {'A': 'ARMap', 'C': 'CHMap', 'S': 'SegmentedMap', 'M': 'MixedMap'}
	
	@classmethod
	def set_parameters(cls, configfile, output_dir = "results", extra = None):
		cls.args = parse_configfile(configfile)
		if "bin" in cls.args:
			cls.bin = cls.args["bin"]
			del cls.args["bin"]
		
		cls.bin = os.path.abspath(cls.bin)
		
		if output_dir:
			cls.output_dir = output_dir
		
		cls.output_dir = os.path.abspath(cls.output_dir)
		
		if extra:
			cls.extra = extra
	
	
	@classmethod
	def test_parameters(cls):
		if not os.path.isdir(cls.output_dir):
			return False, "Output dir does not exists: " + str(cls.output_dir)
		if not os.access(cls.output_dir, os.W_OK):
			return False, "Output dir is not writable: " + str(cls.output_dir)
		if not os.path.exists(cls.bin):
			return False, "Could not find executable: " + str(cls.bin)
		
		for m in cls.args["maps"].split(','):
			if m not in cls.map_types:
				return False, "Unknown map type " + m
				
		arguments = cls.build_arguments(["testfile"], "test")
		if not arguments:
			return False, "Could not create arguments"
		
		test_args = [cls.bin] + arguments + ['--help']
		process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		if process.returncode != 0:
			return False, "Arguments could be wrong :"+ ' '.join(test_args) + "\treturned error:" + error  
		else:
			return True, output
	
	@classmethod
	def build_arguments(cls, fitsfiles, name):
		
		arguments = list()
		for key, value in cls.args.items():
			if len(key) > 1:
				arguments.append("--"+key)
			else:
				arguments.append("-"+key)
			if value:
				arguments.append(value)
		
		arguments.extend(['-O', os.path.join(cls.output_dir, name)])
		
		for fitsfile in fitsfiles:
			arguments.append(os.path.abspath(fitsfile))
		
		return arguments
	
	@classmethod
	def result_files(cls, name):
		results = list()
		for m, suffix in cls.map_types.items():
			if cls.args["maps"].find(m) > -1:
				results.append(os.path.join(cls.output_dir, '.'.join([name, suffix, 'fits'])))
		return results
	
	def __init__(self, name, fitsfiles, previous=None, force=False):
		from dagman_job import Job
		self.results = segmentation.result_files(name)
		make_job = force
		if not force:
			for result in self.results:
				 if not os.path.exists(result):
				 	make_job = True
				 	break
		if make_job:
			arguments = ' '.join(self.build_arguments(fitsfiles, name))
			self.job = Job(name, self.bin, arguments, require=previous)
		else:
			self.job = None


class resegmentation(segmentation):
	bin = "bin/attribution.x"
	
	@classmethod
	def get_info(cls, mapname):
		try:
			hdulist = pyfits.open(mapname)
		except Exception, why:
			raise Exception("Error reading file " + mapname + ": "+ str(why))
		
		params = dict()
		for hdu in hdulist:
			if isinstance(hdu, (pyfits.CompImageHDU, pyfits.ImageHDU)) and 'CHANNELS' in hdu.header:
				params["channels"] = eval(hdu.header['CHANNELS'])
				params["class_centers"] = list()
				for i in range(100):
					center = 'CLSCTR' + ('%02d' % i)
					if center in hdu.header:
						params["class_centers"].append(eval(hdu.header[center]))
					else:
						break
				
				if 'CETA' in hdu.header:
					params["eta"] = eval(hdu.header['CETA'])
				
				params["fitsfiles"] = list()
				for i in range(1, 1000):
					image = 'IMAGE' + ('%03d' % i)
					if image in hdu.header:
						params["fitsfiles"].append(os.path.abspath(hdu.header[image]))
					else:
						break
		return params
	
	@classmethod
	def build_arguments(cls, fitsfiles, name, params = dict()):
		
		filename_prefix = os.path.join(cls.output_dir, name)
		if "eta" in params:
			filename = filename_prefix+".eta.txt"
			try:
				with open(filename, "w") as f:
					f.write(str(params["eta"]))
				cls.args["etaFile"] = filename
			except IOError, why:
				raise Exception("Could not write eta file " + filename + " for resegmentation job " + name)
		
		if "class_centers" in params and "channels" in params:
			filename = filename_prefix+".centers.txt"
			try:
				with open(filename, "w") as f:
					f.write(str(params["channels"]) + "\t" + str(params["class_centers"]))
				cls.args["centersFile"] = filename
			except IOError, why:
				raise Exception("Could not write centers file " + filename + " for resegmentation job " + name)
		
		arguments = list()
		for key, value in cls.args.items():
			if len(key) > 1:
				arguments.append("--"+key)
			else:
				arguments.append("-"+key)
			if value:
				arguments.append(value)
		
		arguments.extend(['-O', os.path.join(cls.output_dir, name)])
		
		for fitsfile in fitsfiles:
			arguments.append(os.path.abspath(fitsfile))
		
		return arguments
	
	def __init__(self, name, mapname, auto_start=True, force=False):
		from dagman_job import Job
		params = get_info(mapname)
		self.results = resegmentation.result_files(name)
		make_job = force
		if not force:
			for result in self.results:
				 if not os.path.exists(result):
				 	make_job = True
				 	break
		if make_job:
			arguments = ' '.join(self.build_arguments(fitsfiles, name, params))
			self.job = Job(name, self.bin, arguments)
		else:
			self.job = None
	


class tracking:
	bin = "bin/tracking.x"
	args = dict()
	extra = None
	
	@classmethod
	def set_parameters(cls, configfile, extra = None):
		cls.args = parse_configfile(configfile)
		if "bin" in cls.args:
			cls.bin = cls.args["bin"]
			del cls.args["bin"]
		
		cls.overlap = None
		if "overlap" in cls.args:
			try:
				cls.overlap = int(cls.args["overlap"])
			except Exception:
				pass
		
		cls.max_files = None
		if "max_files" in cls.args:
			try:
				cls.max_files = int(cls.args["max_files"])
				del cls.args["max_files"]
			except Exception:
				pass
		# If max_files has not been set, we set it to 3 times the overlap
		elif cls.overlap:
			cls.max_files = 3 * cls.overlap
		
		cls.bin = os.path.abspath(cls.bin)
		
		if extra:
			cls.extra = extra
	
	
	@classmethod
	def test_parameters(cls):
		if not os.path.exists(cls.bin):
			return False, "Could not find executable: " + str(cls.bin)
		
		arguments = cls.build_arguments(["testfile1", "testfile2"])
		if not arguments:
			return False, "Could not create arguments"
		
		if not cls.overlap:
			return False, "Overlap was not set correctly"
		
		if not cls.max_files:
			return False, "Max_files was not set correctly"
		
		test_args = [cls.bin] + arguments + ['--help']
		process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		if process.returncode != 0:
			return False, "Arguments could be wrong :"+ ' '.join(test_args) + "\treturned error:" + error  
		else:
			return True, output
	
	@classmethod
	def build_arguments(cls, fitsfiles):
		
		arguments = list()
		for key, value in cls.args.items():
			if len(key) > 1:
				arguments.append("--"+key)
			else:
				arguments.append("-"+key)
			if value:
				arguments.append(value)
		
		for fitsfile in fitsfiles:
			arguments.append(os.path.abspath(fitsfile))
		
		return arguments
	
	@classmethod
	def _has_been_tracked(cls, fitsfile):
		try:
			hdulist = pyfits.open(fitsfile)
			for hdu in hdulist:
				if 'TRACKED' in hdu.header:
					return hdu.header['TRACKED'] == '1'
			
			hdulist.close()
		except IOError, why:
			return False
		return False
		
	def __init__(self, name, fitsfiles, previous=None, auto_start=True, force=False):
		from dagman_job import Job
		self.results = fitsfiles
		make_job = force
		if not force:
			for fitsfile in fitsfiles:
				 if not tracking._has_been_tracked(fitsfile):
				 	make_job = True
				 	break
		if make_job:
			arguments = ' '.join(self.build_arguments(fitsfiles))
			if previous:
				self.job = Job(name, self.bin, arguments, require=previous)
			else:
				self.job = Job(name, self.bin, arguments)
		else:
			self.job = None

class overlay:
	bin = "bin/overlay.x"
	args = dict()
	extra = None
	output_dir = "overlays"
	
	@classmethod
	def set_parameters(cls, configfile, output_dir = "overlays", extra = None):
		cls.args = parse_configfile(configfile)
		if "bin" in cls.args:
			cls.bin = cls.args["bin"]
			del cls.args["bin"]
		
		cls.bin = os.path.abspath(cls.bin)
		
		if output_dir:
			cls.output_dir = output_dir
		
		cls.output_dir = os.path.abspath(cls.output_dir)
		
		if extra:
			cls.extra = extra
	
	
	@classmethod
	def test_parameters(cls):
		if not os.path.isdir(cls.output_dir):
			return False, "Output dir does not exists: " + str(cls.output_dir)
		if not os.access(cls.output_dir, os.W_OK):
			return False, "Output dir is not writable: " + str(cls.output_dir)
		if not os.path.exists(cls.bin):
			return False, "Could not find executable: " + str(cls.bin)
				
		arguments = cls.build_arguments("testmap", ["testfile"])
		if not arguments:
			return False, "Could not create arguments"
		
		test_args = [cls.bin] + arguments + ['--help']
		process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		if process.returncode != 0:
			return False, "Arguments could be wrong :"+ ' '.join(test_args) + "\treturned error:" + error  
		else:
			return True, output
	
	@classmethod
	def build_arguments(cls, mapfile, fitsfiles):
		
		arguments = list()
		for key, value in cls.args.items():
			if len(key) > 1:
				arguments.append("--"+key)
			else:
				arguments.append("-"+key)
			if value:
				arguments.append(value)
		
		arguments.extend(['-M', os.path.abspath(mapfile)])
		arguments.extend(['-O', cls.output_dir])
		
		for fitsfile in fitsfiles:
			if os.path.exists(fitsfile):
				arguments.append(os.path.abspath(fitsfile))
			else:
				arguments.append(fitsfile)
		
		return arguments
	
	@classmethod
	def result_files(cls, mapname):
		
		base = os.path.splitext(os.path.split(mapname)[1])[0]
		return os.path.join(cls.output_dir, base+"*.png")
	
	def __init__(self, name, mapname, fitsfiles=[], previous=None, auto_start=True, force=False):
		from dagman_job import Job
		if not fitsfiles:
			fitsfiles = ['{IMAGE001}']
			raise Exception("No FITS files given")
		make_job = force
		if not force:
			overlays = glob.glob(overlay.result_files(mapname))
			if len(overlays) != len(fitsfiles):
				 make_job = True
		
		if make_job:
			arguments = ' '.join(self.build_arguments(mapname, fitsfiles))
			if previous:
				self.job = Job(name, self.bin, arguments, require=previous)
			else:
				self.job = Job(name, self.bin, arguments)
		else:
			self.job = None


class get_stats:
	bin = "bin/get_segmentation_stats.x"
	args = dict()
	extra = None
	output_dir = "stats"
	
	@classmethod
	def set_parameters(cls, configfile, output_dir = "stats", extra = None):
		cls.args = parse_configfile(configfile)
		if "bin" in cls.args:
			cls.bin = cls.args["bin"]
			del cls.args["bin"]
		
		cls.bin = os.path.abspath(cls.bin)
		
		if output_dir:
			cls.output_dir = output_dir
		
		cls.output_dir = os.path.abspath(cls.output_dir)
		
		if extra:
			cls.extra = extra
	
	
	@classmethod
	def test_parameters(cls):
		if not os.path.isdir(cls.output_dir):
			return False, "Output dir does not exists: " + str(cls.output_dir)
		if not os.access(cls.output_dir, os.W_OK):
			return False, "Output dir is not writable: " + str(cls.output_dir)
		if not os.path.exists(cls.bin):
			return False, "Could not find executable: " + str(cls.bin)
				
		arguments = cls.build_arguments("testmap", ["testfile"])
		if not arguments:
			return False, "Could not create arguments"
		
		test_args = [cls.bin] + arguments + ['--help']
		process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		if process.returncode != 0:
			return False, "Arguments could be wrong :"+ ' '.join(test_args) + "\treturned error:" + error  
		else:
			return True, output
	
	@classmethod
	def build_arguments(cls, mapfile, fitsfiles):
		
		arguments = list()
		for key, value in cls.args.items():
			if len(key) > 1:
				arguments.append("--"+key)
			else:
				arguments.append("-"+key)
			if value:
				arguments.append(value)
		
		arguments.extend(['-M', os.path.abspath(mapfile)])
		
		for fitsfile in fitsfiles:
			if os.path.exists(fitsfile):
				arguments.append(os.path.abspath(fitsfile))
			else:
				arguments.append(fitsfile)
		
		return arguments
	
	@classmethod
	def result_files(cls, mapname):
		
		base = os.path.splitext(os.path.split(mapname)[1])[0]
		return os.path.join(cls.output_dir, base+".csv")
	
	
	def __init__(self, name, mapname, fitsfiles=[], auto_start=True, force=False):
		from dagman_job import Job
		if not fitsfiles:
			fitsfiles = ['{IMAGE001}']
		self.results = get_stats.result_files(mapname)
		
		make_job = force or not os.path.exists(self.results)
		
		if make_job:
			arguments = ' '.join(self.build_arguments(mapname, fitsfiles))
			self.job = Job(name, "/pool/rdv/git/spoca/scripts/get_segmentation_stats.sh", '"\''+self.bin+' '+arguments+'\' \''+self.results+'\'"')
		else:
			self.job = None

class get_STAFF_stats(get_stats):
	
	@classmethod
	def test_parameters(cls):
		if not os.path.isdir(cls.output_dir):
			return False, "Output dir does not exists: " + str(cls.output_dir)
		if not os.access(cls.output_dir, os.W_OK):
			return False, "Output dir is not writable: " + str(cls.output_dir)
		if not os.path.exists(cls.bin):
			return False, "Could not find executable: " + str(cls.bin)
				
		arguments = cls.build_arguments("CHmap", "ARmap", ["testfile"])
		if not arguments:
			return False, "Could not create arguments"
		
		test_args = [cls.bin] + arguments + ['--help']
		process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		if process.returncode != 0:
			return False, "Arguments could be wrong :"+ ' '.join(test_args) + "\treturned error:" + error  
		else:
			return True, output

	@classmethod
	def build_arguments(cls, CHmapfile, ARmapfile, fitsfiles):
		
		arguments = list()
		for key, value in cls.args.items():
			if len(key) > 1:
				arguments.append("--"+key)
			else:
				arguments.append("-"+key)
			if value:
				arguments.append(value)
		
		arguments.extend(['-C', os.path.abspath(CHmapfile), '-A', os.path.abspath(ARmapfile)])
		
		for fitsfile in fitsfiles:
			if os.path.exists(fitsfile):
				arguments.append(os.path.abspath(fitsfile))
			else:
				arguments.append(fitsfile)
		
		return arguments
	
	def __init__(self, name, CHmapfile, ARmapfile, fitsfiles=[], auto_start=True, force=False):
		from condor_job import Job
		if not fitsfiles:
			fitsfiles = ['{IMAGE001}']
		self.results = get_stats.result_files(CHmapfile)
		
		make_job = force or not os.path.exists(self.results)
		
		if make_job:
			def write_stats(job):
				with open(self.results, "w") as f:
					f.write(job.output)
			
			arguments = ' '.join(self.build_arguments(CHmapfile, ARmapfile, fitsfiles))
			self.job = Job(name, self.bin, arguments)
		else:
			self.job = None

# Return a time in the form yyyymmdd_hhmmss 
def pretty_date(date):
	return date.strftime("%Y%m%d_%H%M%S")
	
def parse_configfile(configfile):
	args = dict()
	try:
		with open(configfile) as f:
			for line in f:
				param = line.strip()
				if param and param[0] != '#':
					params = param.split('=', 1)
					if len(params) > 1:
						args[params[0].strip()]=params[1].strip()
					else:
						args[params[0].strip()]=None
	except IOError, why:
		raise Exception("Error parsing configuration file " + str(configfile) + ": " + str(why))
	
	return args
