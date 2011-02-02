#!/usr/bin/python2.6
# -*- coding: iso-8859-15 -*-
# Problems to be solved : Thread failing to be created, problem with the movie
import subprocess, shlex
from operator import itemgetter, attrgetter
import pyfits
import sys, os, os.path
import string
import shutil
from collections import deque
import threading
import logging, pprint
import dateutil.parser, datetime
import re
import argparse
import types

threading.stack_size(32768*2)

tracking_args = dict()
movie_args = dict()
background_args = dict()
overlay_args = dict()
contours_args = dict()
frame_args = dict()

##### PLEASE SET PARAMETERS HERE UNDER #####

# Name of the log file
log_filename = 'makemovie.log'

# Numbers of cpu
cpu_max = 3

# Parameters for the data sets
wavelengths = [171,193]
max_deltatime = 60 * 60 # If files are more than that many second appart in a set, it will be discarded

# Parameters for the movie

movie_name = 'HFCM_C3_ALCDivMedian'
movie_args['scale']     = '720:720'
movie_args['bitrate']   = '1200'
movie_args['framerate'] = '4'
movie_args['title']     = 'AR detected by SPoCA on AIA data May 2010'

# Parameters for backgrounds
background_args['label_schema']           = r'SOHO/EIT {wavelength}Å {time}'
background_args['pointsize']              = '25'
background_args['contrast_preprocessing'] = '-contrast-stretch 0%x0.1%'
background_args['wavelength'] = None

# Parameters for overlays
overlay_args['pointsize'] = background_args['pointsize']
overlay_args['title'] = 'SPoCA AR (HFCM, C4, ALC)'

# Parameters for contours
contours_args['width'] = 5
contours_args['side'] = "-e"
contours_args['mastic'] = "-m"

# Parameters for the frames
frame_args['framesize'] = '1024x1024>'

# Parameters for the tracking
no_tracking = False
tracking_args['max_delta_t'] = 1 * 60 * 60
tracking_args['derotate'] = "-D"
tracking_args['max_files'] = 9
tracking_args['overlap'] = 3
tracking_color = 0


# Parameter for the segmentation
segmentation_params = '-T HFCM -C 4 -I AIA -P ALC,DivMedian -z 0.01,0.01 -B centers.txt'
map_type = "ARmap"

# Binaries locations
uncompress_bin = '/usr/bin/imcopy'
convert_bin = '/usr/bin/convert'
mencoder_bin = '/usr/bin/mencoder'
contours_bin = 'bin/contours.x'
segmentation_bin = 'bin/classification.x'
tracking_bin = 'bin/tracking.x'

# Repo locations 
frames_repo = None
overlays_repo = None
results_repo = 'FCM_test'
backgrounds_repo = 'backgrounds'
data_repo = '/data/aia/lev0/all/'
tmp_folder = '/tmp'

# If you want to force the remaking of all files
force = False

# Set up of the logging
logging.basicConfig(filename=log_filename, level = logging.DEBUG, format='%(asctime)s %(levelname)s %(message)s', datefmt='%Y-%m-%d %H:%M:%S')

# Set up of the synchronisation semaphores
cpus = threading.Semaphore(cpu_max)
printer = threading.Semaphore()

# Start point of the script
def main(argv):
	logging.debug('{0:_>20}'.format("START"))
	args = get_arguments(argv)
	if not args:
		print "Missing or incorrect arguments, exiting"
		return None
	
	# We get the fits files
	fitsfiles = get_fitsfiles(data_repo)
	
	# We make the sets
	fitsfiles_sets = make_sets(fitsfiles, wavelengths, max_deltatime)
	
	# We create all the jobs
	all_jobs = create_jobs(fitsfiles_sets)
	
	# I run all my jobs
	thread_list = list()
	for job in all_jobs:
		thread_list.append(threading.Thread(group=None, name='run_job', target=run_job, args=(job,), kwargs={}))
		thread_list[-1].start()

	# I wait that they complete 
	for thread in thread_list:
		thread.join()
	
	logging.debug('{0:_>20}'.format("END"))

def create_jobs(fitsfiles_sets):
	# I create the list of all the jobs to be done
	all_jobs = list()
	
	# I create the list of all the jobs to make the backgrounds
	logging.info("Macking backgrounds jobs")
	backgrounds_jobs = list()
	for fitsfiles_set in fitsfiles_sets:
		job = dict(target = make_background, args = (background_args, fitsfiles_set), result = None, done = threading.Event(), requirements = None)
		backgrounds_jobs.append(job)
	all_jobs.extend(backgrounds_jobs)
	
	# I create the list of all the jobs to make segmented maps
	logging.info( "Macking segmentation jobs")
	segmentation_jobs = list()
	for fitsfiles_set in fitsfiles_sets:
		# I need the date for reference
		ref_date = get_ref_date(fitsfiles_set)
		result = os.path.join(results_repo, ref_date)
		job = dict(target = make_segmentation, args = (segmentation_params, fitsfiles_set, map_type), result = None, done = threading.Event(), requirements = None)
		segmentation_jobs.append(job)
	all_jobs.extend(segmentation_jobs)
	
	
	tracking_jobs = list()
	contours_jobs = list()
	
	if(not no_tracking):
	# I create the list of all the jobs to do tracking and the contours
		logging.info( "Macking tracking & contours jobs")
		# This is the first tracking, only new segmented maps
		i = 0
		segmentation_result = list()
		while len(segmentation_result) <= tracking_args['max_files'] and i < len(segmentation_jobs):
			segmentation_result.append(segmentation_jobs[i])
			i+=1
		tracking_job = dict(target = make_tracking, args = (tracking_args, 0), result = None, done = threading.Event(), requirements = segmentation_result)
		tracking_jobs.append(tracking_job)
		for segmentation_job in segmentation_result:
			contours_job = dict(target = make_contours, args = (contours_args, ), result = None, done = threading.Event(), requirements = [segmentation_job, tracking_job])
			contours_jobs.append(contours_job)
		
		
		# Then the following ones, previous segmented maps and new segmented maps
		while i < len(segmentation_jobs):
			segmentation_result = [tracking_jobs[-1]]
			while len(segmentation_result) - 1 <= (tracking_args['max_files'] - tracking_args['overlap']) and i < len(segmentation_jobs):
				segmentation_result.append(segmentation_jobs[i])
				i+=1
			tracking_job = dict(target = make_tracking, args = (tracking_args, tracking_args['overlap']), result = None, done = threading.Event(), requirements = segmentation_result)
			tracking_jobs.append(tracking_job)
			for segmentation_job in segmentation_result[1:]:
				contours_job = dict(target = make_contours, args = (contours_args, ), result = None, done = threading.Event(), requirements = [segmentation_job, tracking_job])
				contours_jobs.append(contours_job)
		
		all_jobs.extend(tracking_jobs)
	else:
		# I create only the list of the jobs to make the contours
		logging.info( "Macking contours jobs")
		for segmentation_job in segmentation_jobs:
			job = dict(target = make_contours, args = (contours_args, ), result = None, done = threading.Event(), requirements = [segmentation_job])
			contours_jobs.append(job)
	
	all_jobs.extend(contours_jobs)
	
	# I create the list of the jobs to make the overlays
	logging.info( "Macking overlay jobs")
	overlay_jobs = list()
	for (contours_job,background_job) in zip(contours_jobs, backgrounds_jobs):
		job = dict(target = make_overlay, args = (overlay_args, ), result = None, done = threading.Event(), requirements = [contours_job, background_job])
		overlay_jobs.append(job)
	
	all_jobs.extend(overlay_jobs)
	
	# I create the list of the jobs to make the frames
	logging.info( "Macking frame jobs")
	frame_jobs = list()
	for overlay_job in overlay_jobs:
		job = dict(target = make_frame, args = (frame_args, ), result = None, done = threading.Event(), requirements = [overlay_job])
		frame_jobs.append(job)
	
	all_jobs.extend(frame_jobs)
	
	# I create the job to make the movie
	logging.info( "Macking movie jobs")
	movie_job = dict(target = make_movie, args = (movie_args, movie_name), result = None, done = None, requirements = [frame_jobs])
	all_jobs.append(movie_job)
	
	return all_jobs

def get_requirements_results(requirements):
	
	requirement_results = list()
	for requirement in requirements:
		if type(requirement) == types.ListType:
			results = get_requirements_results(requirement)
			if results:
				requirement_results.append(results)
			else:
				return None
		else:
			requirement['done'].wait()
			if requirement['result']:
				requirement_results.append(requirement['result'])
			else:
				printer.acquire()
				logging.critical("Requirement failed: " + pprint.pformat(requirement['target']) ) 
				printer.release()
				return None
	
	return requirement_results

def run_job(job):
	
	# If I have some requirements I wait that they are done, and I collect their results
	requirement_results = None
	if job['requirements']:
		requirement_results = get_requirements_results(job['requirements'])
		if not requirement_results:
			printer.acquire()
			logging.critical("Missing requirement for job : " + pprint.pformat(job['target'])) 
			printer.release()
			return
	
	# I run my job
	cpus.acquire()
	printer.acquire()
	logging.debug("Starting job: " + pprint.pformat(job['target']))
	logging.debug("With arguments:\n" + pprint.pformat(job['args']))
	printer.release()
	if requirement_results:
		job['result'] = job['target'](*job['args'], results = requirement_results)
	else:
		job['result'] = job['target'](*job['args'])
	
	cpus.release()
	
	# The job is done if I have some results
	if job['result'] and job['done']:
		job['done'].set()
	
	return


def make_movie(movie_args, movie_name, results):
	
	# I check if the movie already exists
	
	if os.path.exists(movie_name):
		if not force:
			logging.info(movie_name + ' already exists, nothing to be done')
			return movie_name
		else:
			logging.info(movie_name + ' already exists, forcing remaking')
			os.remove(movie_name)
	frames = ','.join(results[0])
	# I call mencoder
	mencoder_log = os.path.join(tmp_folder, os.path.basename(movie_name)+'.mencoder.log')
	if os.path.exists(mencoder_log):
		os.remove(mencoder_log)
	
	# We run the first pass
	mencoder = [mencoder_bin, 'mf://' + frames ,'-mf', 'fps=' + str(movie_args['framerate']), '-noskip', '-info', 'name=' + str(movie_args['title']), '-o', movie_name, '-ovc', 'lavc', '-lavcopts', 'vcodec=msmpeg4v2:vbitrate=' + str(movie_args['bitrate'])+':mbd=2:trell:cmp=3:subcmp=3:autoaspect:vpass=1','-vf', 'scale='+str(movie_args['scale']), '-passlogfile', mencoder_log]
	logging.debug("about to execute: " + ' '.join(mencoder))
	
	process = subprocess.Popen(mencoder, shell=False, stdout=open(os.devnull, 'w'), stderr=subprocess.PIPE)
	if process.wait() != 0:
		logging.critical(' '.join(mencoder) + ' failed!')
		logging.critical('Error : ' + str(process.communicate()[1]))
		return None
	
	# We run the second pass
	mencoder = [mencoder_bin, 'mf://' + frames ,'-mf', 'fps=' + str(movie_args['framerate']), '-noskip', '-info', 'name=' + str(movie_args['title']), '-o', movie_name, '-ovc', 'lavc', '-lavcopts', 'vcodec=msmpeg4v2:vbitrate=' + str(movie_args['bitrate'])+':mbd=2:trell:cmp=3:subcmp=3:autoaspect:vpass=2','-vf', 'scale='+str(movie_args['scale']), '-passlogfile', mencoder_log]
	logging.debug("about to execute: " + ' '.join(mencoder))
	
	process = subprocess.Popen(mencoder, shell=False, stdout=open(os.devnull, 'w'), stderr=subprocess.PIPE)
	if process.wait() != 0:
		logging.critical(' '.join(mencoder) + ' failed!')
		logging.critical('Error : ' + str(process.communicate()[1]))
		return None
	
	return movie_name

def make_frame(frame_args, results):
	
	overlay = results[0]
	frame_name = os.path.join(frames_repo, os.path.basename(overlay) )
	# I check if the frame already exists
	if os.path.exists(frame_name):
		if not force:
			logging.info(frame_name + ' already exists, nothing to be done')
			return frame_name
		else:
			logging.info(frame_name + ' already exists, forcing remaking')
			os.remove(frame_name)
	
	# I call imagemagick
	convert = [convert_bin, '-resize', frame_args['framesize'], overlay, '-depth', '8', frame_name]
	logging.debug("about to execute: " + ' '.join(convert))
	if subprocess.call(convert) != 0:
		logging.critical(' '.join(convert) + ' failed!')
		return None
	
	return frame_name


def make_overlay(overlay_args, results):
	
	contours = results[0]
	background = results[1]
	overlay_name = os.path.join(overlays_repo, os.path.basename(background) )
	# I check if the overlay already exists
	if os.path.exists(overlay_name):
		if not force:
			logging.info(overlay_name + ' already exists, nothing to be done')
			return overlay_name
		else:
			logging.info(overlay_name + ' already exists, forcing remaking')
			os.remove(overlay_name)
	
	# I call imagemagick
	convert = [convert_bin, background, contours, "-fill", "white", "-pointsize", overlay_args['pointsize'], "-gravity", "southeast", "-undercolor", "black", "-annotate", "+20+20", overlay_args['title'], "-composite", overlay_name]
	logging.debug("about to execute: " + ' '.join(convert))
	if subprocess.call(convert) != 0:
		logging.critical(' '.join(convert) + ' failed!')
		return None
	
	return overlay_name

def make_contours(contours_args, results):
	
	segmentedmap = results[0]
	contours_name = os.path.splitext(segmentedmap)[0] + '.contours.png'
	# I check if the overlay already exists
	if os.path.exists(contours_name):
		if not force:
			logging.info(contours_name + ' already exists, nothing to be done')
			return contours_name
		else:
			logging.info(contours_name + ' already exists, forcing remaking')
			os.remove(contours_name)
	
	# I call contours.x
	contoursx = [contours_bin, '-w', str(contours_args['width']), contours_args['side'], contours_args['mastic'], segmentedmap]
	logging.debug("about to execute: " + ' '.join(contoursx))
	if subprocess.call(contoursx) != 0:
		logging.critical(' '.join(contoursx) + ' failed!')
		return None
	
	return contours_name


def make_segmentation(segmentation_params, fitsfiles_set, map_type):
	
	fitsfiles = [f['filename'] for f in fitsfiles_set]
	ref_date = get_ref_date(fitsfiles_set)
	output_file = os.path.join(results_repo, ref_date)
	segmentedmap_name = output_file + '.' + map_type + '.fits'
	# I check if the segmentedmap already exists
	if os.path.exists(segmentedmap_name):
		if not force:
			logging.info(segmentedmap_name + ' already exists, nothing to be done')
			return segmentedmap_name
		else:
			logging.info(segmentedmap_name + ' already exists, forcing remaking')
			os.remove(segmentedmap_name)
	
	# I call the segmentation program (SPoCA)
	segmentation = [segmentation_bin, '-O', output_file] + shlex.split(segmentation_params) + fitsfiles
	
	logging.debug("about to execute: " + ' '.join(segmentation))
	if subprocess.call(segmentation) != 0:
		logging.critical(' '.join(segmentation) + ' failed!')
		return None
	
	return segmentedmap_name


def make_background(background_args, fitsfiles_set):
	
	fitsfiles = [f for f in fitsfiles_set if f['wavelength'] == background_args['wavelength']]
	fitsfile_name = fitsfiles[0]['filename']
	background_name = os.path.join(backgrounds_repo, pretty_date(fitsfiles[0]['time']) + '.png')
	
	# I check if the background already exists
	if os.path.exists(background_name):
		if not force:
			logging.info(background_name + ' already exists, nothing to be done')
			return background_name
		else:
			logging.info(background_name + ' already exists, forcing remaking')
			os.remove(background_name)
	
	decompress = compressed(fitsfile_name)
	print fitsfile_name, " is compressed ? ", decompress
	if decompress:
		uncompressed_fitsfile = os.path.join(tmp_folder, os.path.basename(fitsfile_name))
		if not os.path.exists(uncompressed_fitsfile) or force:
			uncompress = [uncompress_bin, fitsfile_name, uncompressed_fitsfile]
			logging.debug("about to execute: " + ' '.join(uncompress))
			if subprocess.call(uncompress) != 0:
				logging.critical(' '.join(uncompress) + ' failed!')
				return None
	else:
		uncompressed_fitsfile = fitsfile_name
	
	label = background_args['label_schema'].format(wavelength = fitsfiles[0]['wavelength'], time = fitsfiles[0]['date_obs'])
	logging.debug("label for file " + background_name + " is: " + label)
	
	convert = [convert_bin] + shlex.split(background_args['contrast_preprocessing']) + [uncompressed_fitsfile , "-fill", "white", "-pointsize", background_args['pointsize'], "-gravity", "northeast", "-undercolor", "black", "-annotate", "+20+20", label, background_name]
	logging.debug("about to execute: " + ' '.join(convert))
	if subprocess.call(convert) != 0:
		logging.critical(' '.join(convert) + ' failed!')
	if decompress:
		os.remove(uncompressed_fitsfile)
	
	return background_name


def make_tracking(tracking_args, overlap, results):
	
	global tracking_color
	if overlap > 0:
		ref_files = results[0][-overlap:]
		fitsfiles = results[1:]
	else:
		ref_files = []
		fitsfiles = results
	
	# I call the tracking.x program 
	trackingx = [tracking_bin, '-o' , str(overlap), '-d', str(tracking_args['max_delta_t']), '-n', str(tracking_color), '-A', tracking_args['derotate']] + ref_files + fitsfiles 
	logging.debug("about to execute: " + ' '.join(trackingx))
	process = subprocess.Popen(trackingx, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	if process.wait() != 0:
		logging.critical(' '.join(trackingx) + ' failed!')
		logging.critical('Error : ' + str(process.communicate()[1]))
		return None
	else:
		m = re.search('\d+', process.communicate()[0])
		tracking_color = int(m.group(0))
		logging.debug("tracking_color: " + str(tracking_color))
	return fitsfiles



# Return a list of fits files in a folder, with some attributes
def get_fitsfiles(folder):
	filelist = [f for f in os.listdir(folder) if os.path.splitext(f)[1] == ".fits"]
	filelist = [os.path.join(folder,f) for f in filelist]
	fitsfiles = list()
	for filename in filelist:
		(date_obs, wavelength) = get_info(filename)
		fitsfiles.append({'filename' : filename, 'date_obs' : date_obs, 'wavelength' : wavelength, 'time' : dateutil.parser.parse(date_obs)})
	
	return fitsfiles

# Returns the date and wavelength of a fits file
def get_info(fitsfile):
	hdulist = pyfits.open(fitsfile)
	for hdu in hdulist:
		header = hdu.header
		if 'WAVELNTH' in header:
			hdulist.close()
			if 'INSTRUME' in header and re.search('AIA', header['INSTRUME'], re.I):
				return (header['T_OBS'], header['WAVELNTH'])
			else:
				return (header['DATE-OBS'], header['WAVELNTH'])
	hdulist.close()
	return (None, None)


# Return list of filenames sorted by wavelength and then time
def make_sets(fitsfiles, wavelengths, max_deltatime):
	
	# First I make lists by wavelength
	fitsfiles_lists=dict()
	for wavelength in wavelengths:
		fitsfiles_lists[wavelength] = list()
	for fitsfile in fitsfiles:
		if fitsfile['wavelength'] in wavelengths :
			fitsfiles_lists[fitsfile['wavelength']].append(fitsfile)
	
	# Now I sort each list by time
	for fitsfiles_list in fitsfiles_lists.values():
		fitsfiles_list.sort(key = lambda a:a['time'])
		
	# We create tuples of images of the different wavelength
	fitsfiles_sets = zip(* fitsfiles_lists.values())
	
	# I remove the tuples that are more than max_deltatime appart
	for i, fitsfiles_set in enumerate(fitsfiles_sets[:]):
		if (max(fitsfiles_set, key = lambda a:a['time'])['time'] - min(fitsfiles_set, key = lambda a:a['time'])['time']) > datetime.timedelta(seconds=max_deltatime):
			fitsfiles_sets.remove(fitsfiles_set)
	
	return fitsfiles_sets


# For multiwavelength we need a time reference
def get_ref_date(fitsfiles_set):
	ref_time = fitsfiles_set[0]['time']
	for fitsfile in fitsfiles_set[1:]:
		if fitsfile['time'] < ref_time:
				ref_time = fitsfile['time']
		
	return pretty_date(ref_time)

# Return a time in the form yyyymmdd_hhmmss 
def pretty_date(time):
	return time.strftime("%Y%m%d_%H%M%S")


def get_arguments(argv):
	
	args = True
	#args = parser.parse_args(argv)
	
	# Parameters check
	if not background_args['wavelength']:
		background_args['wavelength'] = wavelengths[0]
	else:
		if background_args['wavelength'] not in wavelengths:
			print "Error background_wavelength must be one of ", wavelengths
	
	global movie_name
	(basename,ext) = os.path.splitext(movie_name)
	if not ext:
		movie_name += '.avi'
	
	if not movie_args['title']:
		movie_args['title'] = os.path.basename(basename)
	
	if tracking_args['overlap'] >= tracking_args['max_files']:
		tracking_args['overlap'] = int(tracking_args['max_files'] / 3)
		print "Warning: setting tracking_overlap to ", tracking_args['overlap']
	
		
	if not overlay_args['pointsize']:
		overlay_args['pointsize'] = background_args['pointsize']
	
	if not overlay_args['title']:
		overlay_args['title'] = movie_args['title']
	
	# Repository check
	
	global results_repo
	if not os.path.isdir(results_repo):
		if os.path.exists(results_repo):
			print results_repo, " exists but is not a directory"
			return None
		else:
			os.mkdir(results_repo)
	
	global overlays_repo
	if not overlays_repo:
		overlays_repo = results_repo+'_O'
	if not os.path.isdir(overlays_repo):
		if os.path.exists(overlays_repo):
			print overlays_repo, " exists but is not a directory"
			return None
		else:
			os.mkdir(overlays_repo)
	
	global frames_repo
	if not frames_repo:
		frames_repo = results_repo+'_F'
	if not os.path.isdir(frames_repo):
		if os.path.exists(frames_repo):
			print frames_repo, " exists but is not a directory"
			return None
		else:
			os.mkdir(frames_repo)
	
	global tmp_folder
	if not tmp_folder:
		tmp_folder = '/tmp'
	if not os.path.isdir(tmp_folder):
		if os.path.exists(tmp_folder):
			print tmp_folder, " exists but is not a directory"
			return None
		else:
			os.mkdir(tmp_folder)
	
	global data_repo
	if not os.path.isdir(data_repo):
		print data_repo, " data_repo does not exists"
		return None
	
	global backgrounds_repo
	if not backgrounds_repo:
		backgrounds_repo = 'backgrounds'
	if not os.path.isdir(backgrounds_repo):
		if os.path.exists(backgrounds_repo):
			print backgrounds_repo, " exists but is not a directory"
			return None
		else:
			os.mkdir(backgrounds_repo)
	
	# Binaries check
	
	if not os.path.exists(uncompress_bin):
		print "Error: ", uncompress_bin , " is missing"
		return None
	if not os.path.exists(convert_bin):
		print "Error: ", convert_bin , " is missing"
		return None
	if not os.path.exists(mencoder_bin):
		print "Error: ", mencoder_bin , " is missing"
		return None
	if not os.path.exists(contours_bin):
		print "Error: ", contours_bin , " is missing"
		return None
	if not os.path.exists(segmentation_bin):
		print "Error: ", segmentation_bin , " is missing"
		return None
	
	if not no_tracking and not os.path.exists(tracking_bin):
		print "Error: ", tracking_bin , " is missing"
		return None
	
	
	return args


def compressed(fitsfile):
	hdulist = pyfits.open(fitsfile)
	for hdu in hdulist:
		header = hdu.header
		if 'XTENSION' in header:
			hdulist.close()
			return True
	hdulist.close()
	return False


if __name__ == "__main__":
	main(sys.argv[1:])

