#!/usr/bin/env python2.6
import os, os.path, sys
import string
import threading
from Queue import Queue
import logging
from collections import deque
import argparse
import subprocess, shlex
import time
from datetime import datetime

import pyfits
import get_data



# Return a time in the form yyyymmdd_hhmmss 
def pretty_date(date):
	return date.strftime("%Y%m%d_%H%M%S")

# Return the list of desired keywords from the headers of a fitsfile
def get_keywords(fitsfile, keywords):
	result = [None]*len(keywords)
	try:
		hdulist = pyfits.open(fitsfile)
		for hdu in hdulist:
			for k,keyword in enumerate(keywords):
				if keyword in hdu.header:
					result[k] = hdu.header[keyword]
		
		hdulist.close()
	except IOError, why:
		log.warning("Error reading file " + fitsfile + ": "+ str(why))
	
	return result


def check_options():
	
	def which(program):
		def is_exe(fpath):
			return os.path.exists(fpath) and os.access(fpath, os.X_OK)
		
		fpath, fname = os.path.split(program)
		if fpath:
			if is_exe(program):
				return program
		else:
			for path in os.environ["PATH"].split(os.pathsep):
				exe_file = os.path.join(path, program)
				if is_exe(exe_file):
					return exe_file

		return None
	
	
	# Check options for the segmenation
	if not segmentation:
		log.critical("No segmentation option found in file :" + args.spoca)
		return False
	
	if not which(segmentation['bin']): 
		log.critical("segmentation executable "+ segmentation['bin']+ " is missing" )
		return False
	
	test_args = [segmentation['bin']] + shlex.split(segmentation['args']) + ["file"]*9 + ['--help']
	process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	(output, error) = process.communicate()
	if process.returncode != 0:
		log.warning("segmentation arguments could be wrong :"+ segmentation['args'])
		log.warning("tested with command: "+ ' '.join(test_args))
		log.warning("output: "+ output+ "\nerror: "+ error)
	else:
		log.info("Parameters for the segmentation:\n"+ output)

	if not os.path.isdir(segmentation['repo']):
		if os.path.exists(segmentation['repo']):
			log.critical("segmentation repository "+ segmentation['repo']+ " exists but is not a directory")
			return False
		else:
			try:
				os.mkdir(segmentation['repo'])
			except OSError, why:
				log.critical("segmentation repository "+ segmentation['repo']+ " does not exist and could not be created")
				return False
			else:
				log.warning("segmentation repository "+ segmentation['repo']+ " did not exist and was created")
	
	# Check options for the tracking
	global do_tracking
	if not tracking:
		if do_tracking:
			log.critical("do_tracking is set, but no tracking option found in file :" + args.spoca)
			return False
		do_tracking = False
	
	if do_tracking:
		if not which(tracking['bin']): 
			log.critical("tracking executable "+ tracking['bin']+ " is missing" )
			return False
		
		test_args = [tracking['bin']] + shlex.split(tracking['args']) + ["file"]*9 + ['--help']
		process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		if process.returncode != 0:
			log.warning("tracking arguments could be wrong :"+ segmentation['args'])
			log.warning("tested with command: "+ ' '.join(test_args))
			log.warning("output: "+ output+ "\nerror: "+ error)
		else:
			log.info("Parameters for the tracking:\n"+ output)

		try: 
	        	tracking['nbrimages'] = int(tracking['nbrimages'])
		except ValueError:
			log.critical("tracking nbrimages is not an integer: "+ str(tracking['nbrimages']))
			return False
		
		try: 
	        	tracking['overlap'] = int(tracking['overlap'])
		except ValueError:
			log.critical("tracking overlap is not an integer: "+ str(tracking['overlap']))
			return False
		
		if tracking['nbrimages'] <= tracking['overlap']:
			tracking['overlap'] = tracking['nbrimages'] - 1
			log.warning("tracking overlap is bigger than tracking nbrimages. Setting it to : " + str(tracking['overlap']))
	
	# Check options for the overlay
	global do_overlay
	if not overlay:
		if do_overlay:
			log.critical("do_overlay is set, but no overlay option found in file :" + args.spoca)
			return False
		do_overlay = False
	
	if do_overlay:
		if not which(overlay['bin']): 
			log.critical("overlay executable "+ overlay['bin']+ " is missing" )
			return False
		
		test_args = [overlay['bin']] + shlex.split(overlay['args']) + ["file"]*9 + ['--help']
		process = subprocess.Popen(test_args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(output, error) = process.communicate()
		if process.returncode != 0:
			log.warning("overlay arguments could be wrong :"+ overlay['args'])
			log.warning("tested with command: "+ ' '.join(test_args))
			log.warning("output: "+ output+ "\nerror: "+ error)
		else:
			log.info("Parameters for the overlay:\n"+ output)

		if not os.path.isdir(overlay['repo']):
			if os.path.exists(overlay['repo']):
				log.critical("overlay repository "+ overlay['repo']+ " exists but is not a directory")
				return False
			else:
				try:
					os.mkdir(overlay['repo'])
				except OSError, why:
					log.critical("overlay repository "+ overlay['repo']+ " does not exist and could not be created")
					return False
				else:
					log.warning("overlay repository "+ overlay['repo']+ " did not exist and was created")
	
	# Check options for the frame
	global do_frame
	if not frame:
		if do_frame:
			log.critical("do_frame is set, but no frame option found in file :" + args.spoca)
			return False
		do_frame = False
	
	if do_frame:
		if not which(frame['bin']): 
			log.critical("frame executable "+ frame['bin']+ " is missing" )
			return False
		
		if not os.path.isdir(frame['repo']):
			if os.path.exists(frame['repo']):
				log.critical("frame repository "+ frame['repo']+ " exists but is not a directory")
				return False
			else:
				try:
					os.mkdir(frame['repo'])
				except OSError, why:
					log.critical("frame repository "+ frame['repo']+ " does not exist and could not be created")
					return False
				else:
					log.warning("frame repository "+ frame['repo']+ " did not exist and was created")
	
	# Check options for the movie
	global do_movie
	if not movie:
		if do_movie:
			log.critical("do_movie is set, but no movie option found in file :" + args.spoca)
			return False
		do_movie = False
	
	if do_movie:
		if not which(movie['bin']): 
			log.critical("movie executable "+ movie['bin']+ " is missing" )
			return False
		
		try: 
	        	movie['number_passes'] = int(movie['number_passes'])
		except ValueError:
			log.warning("movie number passes is not an integer: "+ movie['number_passes']+ ", setting it to 2")
			movie['number_passes'] = 2
			
		if os.path.exists(movie['filename']) and not force:
			log.critical("movie filename "+ movie['filename']+ " already exists and force is not set")
			return False
		
		if not os.access(os.path.dirname(movie['filename']), os.W_OK):
			log.critical("cannot write in directory for movie"+ os.path.dirname(movie['filename']))
			return False
	
	return True


def make_segmentation_jobs(sets_queue, output_queue):
	
	def check_fitsfiles(job):
		fitsfiles = ""
		for fitsfile in job.arguments['fitsfiles']:
			if os.path.exists(fitsfile):
				fitsfiles += ' ' + fitsfile
			else:
				return False
		
		job.arguments = job.arguments['args'] + fitsfiles
	
	
	counter = 0
	
	set = sets_queue.get()
	# None is provided when the queue is empty
	while set != None:
		
		log.debug("Received set " + str(set))
		
		reference_string = pretty_date(set['date'])
		filename = os.path.join(segmentation['repo'], reference_string + segmentation['map_suffix'])
		
		job = None
		
		if force or not os.path.exists(filename):
			
			counter += 1
			job_name = "s"+str(counter)
			
			arguments = {'args':'-O ' + segmentation['repo'] + ' ' + segmentation['args'] + ' ', 'fitsfiles':set['filenames']}
			job = async.Job(job_name, executable=segmentation['bin'], arguments=arguments, job_input=None, extra=segmentation['extra'], require=None, pre_submit=check_fitsfiles, call_back=None, auto_start=True)
			
			log.debug("Submitted job " + str(job_name))
		else :
			log.debug("Not doing segmentation for file " + filename)
		
		output_queue.put({'job': job, 'data': filename})
		set = sets_queue.get()
	
	log.debug("No more  files for segmentation, exiting thread")
	output_queue.put(None)


def make_tracking_jobs(input_queue, output_queue):
	
	def check_fitsfiles(job):
		fitsfiles = ""
		for fitsfile in job.arguments['fitsfiles']:
			if os.path.exists(fitsfile):
				fitsfiles += ' ' + fitsfile
		
		if fitsfiles:
			job.arguments = job.arguments['args'] + fitsfiles
		else:
			return False
	
	def call_back(job):
		log.debug("Tracking job "+ str(job.name) + " terminated\noutput:\n" + str(job.output) + "\nerror:\n" + str(job.error))
	
	overlap_files = list()
	tracking_files = list()
	required_jobs = list()
	run_tracking = force
	counter = 0
	
	combo = input_queue.get()
	# None is provided when the queue is empty
	while combo != None:
		tracking_files.append(combo['data'])
		required_jobs.append(combo['job'])
		
		# If i have enough file to do the tracking, I add a job
		if len(tracking_files) + len(overlap_files) >= tracking['nbrimages']:
			
			job = None
						
			# Do we need to actually run the tracking?
			if not run_tracking:
				for filename in tracking_files:
					if os.path.exists(filename):
						tracked = get_keywords(filename, ['TRACKED'])[0]
						if tracked and tracked.lower().find("yes"):
							log.debug(filename + " has been tracked already")
							continue
					run_tracking = True
					break
			
			if run_tracking:
				counter += 1
				job_name = "t"+str(counter)
				arguments = {'args': tracking['args'], 'fitsfiles': overlap_files + tracking_files}
				job = async.Job(job_name, executable=tracking['bin'], arguments=arguments, job_input=None, extra=tracking['extra'], require=required_jobs, pre_submit=check_fitsfiles, call_back=call_back, auto_start=True)
				log.debug("Submitted job " + str(job_name))
			else :
				log.debug("Not doing traking for files " + str(tracking_files))
			
			for filename in tracking_files:
				output_queue.put({'job': job, 'data':filename})
			
			# We prepare for the next tracking job
			required_jobs = [job]
			overlap_files = tracking_files[-tracking['overlap']:]
			tracking_files = list()
		
		combo = input_queue.get()
	
	# It is possible that some files are left to do tracking
	if tracking_files:
		
		job = None
						
		# Do we need to actually run the tracking?
		if not run_tracking:
			for filename in tracking_files:
				if os.path.exists(filename) and get_keywords(filename, ['TRACKED'])[0]:
					log.debug(filename + " has been tracked already")
				else:
					run_tracking = True
					break
		if run_tracking:
			counter += 1
			job_name = "t"+str(counter)
			arguments = {'args': tracking['args'], 'fitsfiles': overlap_files + tracking_files}
			job = async.Job(job_name, executable=tracking['bin'], arguments=arguments, job_input=None, extra=tracking['extra'], require=required_jobs, pre_submit=check_fitsfiles, call_back=call_back, auto_start=True)
			log.debug("Submitted job " + str(job_name))
		else :
			log.debug("Not doing traking for files " + str(tracking_files))
		
		for filename in tracking_files:
			output_queue.put({'job': job, 'data':filename})
		
		# We prepare for the next tracking job
		required_jobs = [job]
		overlap_files = tracking_files[-tracking['overlap']:]
		tracking_files = list()
	
	
	log.debug("No more files for tracking, exiting thread")
	output_queue.put(None)

	
def make_overlay_jobs(input_queue, output_queue):
	
	def check_fitsfile(job):
		if os.path.exists(job.arguments['fitsfile']):
			job.arguments = job.arguments['args'] + ' ' + job.arguments['fitsfile']
		else:
			return False
	
	counter = 0
	
	combo = input_queue.get()
	# None is provided when the queue is empty
	while combo != None:
		
		reference_string = os.path.splitext(os.path.basename(combo['data']))[0]
		filename = os.path.join(overlay['repo'], reference_string + '.png')
		
		job = None
		
		if force or not os.path.exists(filename):
			counter += 1
			job_name = "o"+str(counter)
			
			arguments = {'args': '-O ' + overlay['repo'] + ' ' + overlay['args'], 'fitsfile': combo['data']}
			job = async.Job(job_name, executable=overlay['bin'], arguments=arguments, job_input=None, extra=overlay['extra'], require=combo['job'], pre_submit=check_fitsfile, call_back=None, auto_start=True)
			log.debug("Submitted job " + str(job_name))
		else :
			log.debug("Not doing overlay for file " + filename)
		
		output_queue.put({'job': job, 'data':filename})
		
		combo = input_queue.get()
	
	log.debug("No more  files for overlay, exiting thread")
	output_queue.put(None)


def make_frame_jobs(input_queue, output_queue):
	
	def check_image(job):
		if os.path.exists(job.arguments['image']):
			job.arguments = job.arguments['image'] + ' ' + job.arguments['args']
		else:
			return False
	
	
	counter = 0
	
	combo = input_queue.get()
	# None is provided when the queue is empty
	while combo != None:
		
		reference_string = os.path.basename(combo['data'])
		filename = os.path.join(frame['repo'], reference_string)
		
		job = None
		
		if force or not os.path.exists(filename):
			counter += 1
			job_name = "f"+str(counter)
			
			arguments = {'args': frame['args'] + ' ' + filename, 'image': combo['data']}
			job = async.Job(job_name, executable=frame['bin'], arguments=arguments, job_input=None, extra=frame['extra'], require=combo['job'], pre_submit=check_image, call_back=None, auto_start=True)
			log.debug("Submitted job " + str(job_name))
		else :
			log.debug("Not doing frame for file " + filename)
		
		output_queue.put({'job': job, 'data':filename})
		
		combo = input_queue.get()
	
	log.debug("No more files for frame, exiting thread")
	output_queue.put(None)


def make_movie_jobs(input_queue):
	
	
	def check_frames(job):
			frames = ''
			for f in job.arguments['frames']:
				if os.path.exists(f):
					frames += 'mf://' + f.strip() + ' '
			if not frames:
				return False
			else:
				job.arguments = frames + ' ' + job.arguments['args'] + ' -lavcopts vpass=' + str(job.arguments['pass']) + ' -passlogfile ' + job.arguments['scene_name'] + '.log' +' -o ' + job.arguments['scene_name'] + '.avi'
	
	def check_scenes(job):
			scenes = ''
			for s in job.arguments['scenes']:
				if os.path.exists(s):
					scenes += s.strip() + ' '
			if not scenes:
				return False
			else:
				job.arguments = scenes + ' ' + job.arguments['args'] + ' -o ' + job.arguments['movie_name']
	
	
	if force or not os.path.exists(movie['filename']):
		
		scene = 0
		scenes = list()
		movie_jobs = list()
		required_jobs = list()
		frames = list()
		job = None
		
		combo = input_queue.get()
		# None is provided when the queue is empty
		while combo != None:
			
			required_jobs.append(combo['job'])
			frames.append(combo['data'])
			
			# If I have enough frames I can make a movie scene
			if len(frames) >= movie['nbrframes']:
				scene += 1
				scene_name = "scene" + str(scene)
				scenes.append(scene_name)
				# We do several pass to improve the movie quality
				for p in range(movie['number_passes']):
					job_name = scene_name + "pass" + str(p+1)
					arguments = {'frames': frames, 'args': movie['args'], 'pass' : p+1, 'scene_name' : scene_name}
					job = async.Job(job_name, executable=movie['bin'], arguments=arguments, job_input=None, extra=movie['extra'], require=required_jobs, pre_submit=check_frames, call_back=None, auto_start=True)
					required_jobs = [job]
				
				movie_jobs.append(job)
				required_jobs = list()
			
			combo = input_queue.get()
		
		# If some frames are left, we make a last scene
		if len(frames) > 0:
			scene += 1
			scene_name = "scene" + str(scene)
			scenes.append(scene_name)
			# We do several pass to improve the movie quality
			for p in range(movie['number_passes']):
				job_name = scene_name + "pass" + str(p+1)
				arguments = {'frames': frames, 'args': movie['args'], 'pass' : p+1, 'scene_name' : scene_name}
				job = async.Job(job_name = scene_name + "pass" + str(p+1), executable=movie['bin'], arguments=arguments, job_input=None, extra=movie['extra'], require=required_jobs, pre_submit=check_frames, call_back=None, auto_start=True)
				required_jobs = [job]
			
			movie_jobs.append(job)
			required_jobs = list()
				
		# We join all the scenes together	
		arguments = {'scenes': [s+'.avi' for s in scenes], 'args': '-oac copy -ovc copy', 'movie_name' : movie['filename']}
		async.Job(job_name="movie", executable=movie['bin'], arguments=arguments, job_input=None, extra=movie['extra'], require=movie_jobs, pre_submit=check_scenes, call_back=None, auto_start=True)




# Start point of the script
if __name__ == "__main__":
	
	# Default name for the log file
	log_filename = os.path.splitext(os.path.basename(sys.argv[0]))[0] + '.log'
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Test the get_data module.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='set the file where to log')
	parser.add_argument('--force', '-f', default=False, action='store_true', help='force the remaking of all files')
	parser.add_argument('--stop_on_error', '-s', default=False, action='store_true', help='terminate the script if there is an error')
	parser.add_argument('--retry_on_error', '-r', default=False, action='store_true', help='retry to submit a job if it fails. has precedence over stop_on_error')
	parser.add_argument('--batch_mode', '-b', default=False, action='store_true', help='do not ask before resubmitting a job if it fails. if stop_on_error is set and you answer no, the program will terminate')
	parser.add_argument('--data', '-D', default='get_data.config', help='Config file for the data')
	parser.add_argument('--spoca', '-S', default='run_spoca.config', help='Config file for the SPoCA')
	parser.add_argument('--condor', '-c', default=False, action='store_true', help='Run the jobs with condor')
	
	args = parser.parse_args()
	
	# Set up of the logging
	log_filename = args.log_filename
	if args.debug :
		logging.basicConfig(filename=log_filename, level = logging.DEBUG, format='%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
	else:
		logging.basicConfig(filename=log_filename, level = logging.INFO, format='%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
	
	console = logging.StreamHandler()
	console.setFormatter(logging.Formatter('%(levelname)-8s: %(name)-12s: %(message)s'))
	if args.verbose:
		console.setLevel(logging.INFO)
	else:
		console.setLevel(logging.CRITICAL)
	
	# Create logger
	log = logging.getLogger('run_spoca')
	log.addHandler(console)
	
	# set up of the global variables
	force = args.force
	stop_on_error = args.stop_on_error
	retry_on_error = args.retry_on_error
	batch_mode = args.batch_mode
	
	# Set up of the get_data options
	data = dict()
	try :
		with open(args.data) as f:
			exec(f)
	except (IOError) , why:
		log.critical("Could not open data config file: "+ args.data + ": "+ str(why))
		sys.exit(2)
	
	# Set up of the run_spoca options
	segmentation = dict()
	tracking = dict()
	overlay = dict()
	frame = dict()
	movie = dict()
	do_tracking = False
	do_overlay = False
	do_frame = False
	do_movie = False
	
	try :
		with open(args.spoca) as f:
			exec(f)
	except (IOError) , why:
		log.critical("Could not open spoca config file: "+ args.spoca + ": "+ str(why))
		sys.exit(2)
	
	if not check_options():
		print "Missing or incorrect spoca options, exiting."
		sys.exit(2)
	
	# We check if we run with condor or simple process
	if args.condor:
		import condor_job as async
	else:
		import process_job as async

	threads = list()
	
	# We get the set
	log.debug("Starting to get data")
	sets_queue = Queue()
	get_sets_thread = threading.Thread(group=None, name='get_reduced_sets', target=get_data.get_reduced_sets, args=(data, sets_queue), kwargs={})
	get_sets_thread.start()
	threads.append(get_sets_thread)
	
	# We make the segmentation jobs
	input_queue = sets_queue
	output_queue = Queue()
	segmentation_thread = threading.Thread(group=None, name='make_segmentation_jobs', target=make_segmentation_jobs, args=(input_queue, output_queue), kwargs={})
	segmentation_thread.start()
	threads.append(segmentation_thread)
	
	if do_tracking:
		# We make the tracking jobs
		input_queue = output_queue
		output_queue = Queue()
		tracking_thread = threading.Thread(group=None, name='make_tracking_jobs', target=make_tracking_jobs, args=(input_queue, output_queue), kwargs={})
		tracking_thread.start()
		threads.append(tracking_thread)
	
	if do_overlay: 
		# We make the overlay jobs
		input_queue = output_queue
		output_queue = Queue()
		overlay_thread = threading.Thread(group=None, name='make_overlay_jobs', target=make_overlay_jobs, args=(input_queue, output_queue), kwargs={})
		overlay_thread.start()
		threads.append(overlay_thread)
		
	if do_frame:
		# We make the frames
		input_queue = output_queue
		output_queue = Queue()
		frame_thread = threading.Thread(group=None, name='make_frame_jobs', target=make_frame_jobs, args=(input_queue, output_queue), kwargs={})
		frame_thread.start()
		threads.append(frame_thread)
	
	if do_movie:
		# We make the movie
		input_queue = output_queue
		movie_thread = threading.Thread(group=None, name='make_movie_jobs', target=make_movie_jobs, args=(input_queue,), kwargs={})
		movie_thread.start()
		threads.append(movie_thread)

	# We wait that all jobs are done

	for thread in threads:
		thread.join()
		log.info(thread.name + " thread has terminated")

	async.Job.wait()

