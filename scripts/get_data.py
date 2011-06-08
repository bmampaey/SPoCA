#!/usr/bin/env python2.6
import os, os.path, sys
import string
import threading
from  Queue import Queue
import logging
from datetime import datetime, timedelta
from collections import deque
import dateutil.parser
import argparse



# create logger
module_logger = logging.getLogger('get_data')


def check_get_files_options(data):
	
	if not data:
		return False
	
	if not os.path.isdir(data['repo']):
		module_logger.critical("get_files error: " + data['repo'] + " is not a folder.")
		return False
	
	if 'suffix' not in data:
		data['suffix'] = None
	
	if 'earliest_date' not in data:
		data['earliest_date'] = None
	
	elif data['earliest_date'] != None and not isinstance(data['earliest_date'], datetime):
		try:
			data['earliest_date'] = dateutil.parser.parse(data['earliest_date'])
		except Exception, why:
			module_logger.critical("get_files error: earliest_date is not a valid date: " + str(why))
			return False
	
	if 'latest_date' not in data:
		data['latest_date'] = None
	
	elif data['latest_date'] != None and not isinstance(data['latest_date'], datetime):
		try:
			data['latest_date'] = dateutil.parser.parse(data['latest_date'])
		except Exception, why:
			module_logger.critical("get_files error: latest_date is not a valid date: " + str(why))
			return False
	
	if 'folder_date' not in data:
		data['folder_date'] = None
	
	elif data['folder_date'] != None and not hasattr(data['folder_date'], '__call__'):
		module_logger.critical("get_files error: folder_date is not a routine")
		return False
	
	if 'check_folder' not in data:
		data['check_folder'] = None
	
	elif data['check_folder'] != None and not hasattr(data['check_folder'], '__call__'):
		module_logger.critical("get_files error: check_folder is not a routine")
		return False
	
	if 'file_date' not in data:
		data['file_date'] = None
	
	elif data['file_date'] != None and not hasattr(data['file_date'], '__call__'):
		module_logger.critical("get_files error: file_date is not a routine")
		return False

	if 'check_file' not in data:
		data['check_file'] = None
	
	elif data['check_file'] != None and not hasattr(data['check_file'], '__call__'):
		module_logger.critical("get_files error: check_file is not a routine")
		return False
	
	return True

# Return a list of files in a folder and it's subfolders
def get_files(data, files_queue = None):
	
	if not check_get_files_options(data):
		if files_queue:
			files_queue.put(None)
			return False
		else:
			return []
	
	files = list()
	folder_list = deque([data['repo']])
	
	while folder_list:
		folder = folder_list.popleft()
		for entry in sorted(os.listdir(folder)):
			
			path = os.path.join(folder,entry)
			
			if os.path.isdir(path): # The path is a directory
				# We check it has a date and it is valid
				if data['folder_date'] != None:
					folder_date = data['folder_date'](path)
					
					if	(folder_date != None and
						data['earliest_date'] != None and folder_date < data['earliest_date'] and
						data['latest_date'] != None and data['latest_date'] <= folder_date):
						continue
				
				# We check it is valid
				if data['check_folder'] != None and not data['check_folder'](path):
					continue
				
				# We add the folder to the list
				folder_list.append(path)
						
			else: #The path is a file
				# We check the suffix
				if data['suffix'] != None and os.path.splitext(path)[1] != data['suffix']:
					continue
				
				# We check if it has a channel
				if data['channels'] != None:
					file_channel = data['file_channel'](path)
					if file_channel == None or file_channel not in data['channels']:
						continue
				
				# We check it has a date and it is valid
				if data['file_date'] != None:
					file_date = data['file_date'](path)

					if	(file_date == None or
						(data['earliest_date'] != None and file_date < data['earliest_date']) or
						(data['latest_date'] != None and data['latest_date'] <= file_date)):
						continue
				
				# We check it is valid
				if data['check_file'] != None and not data['check_file'](path):
					continue
				
				# We add the file to the list
				if files_queue:
					files_queue.put(path)
				else:
					files.append(path)
	
	module_logger.debug("No more files, exiting thread")
	
	if files_queue:
		files_queue.put(None)
		return True
	else:
		return files


def check_get_sets_options(data):
	
	if not data['channels']:
		module_logger.critical("get_sets error: no channels specified.")
		return False
	# else Test if list
	
	if not data['file_date'] :
		module_logger.critical("get_sets error: no file_date routine specified.")
		return False
	
	if not data['file_channel']:
		module_logger.critical("get_sets error: no file_channel routine specified.")
		return False
	
	if 'max_deltatime' not in data:
		data['max_deltatime'] = None
	
	if 'earliest_date' not in data:
		data['earliest_date'] = None
	
	elif data['earliest_date'] != None and not isinstance(data['earliest_date'], datetime):
		try:
			data['earliest_date'] = dateutil.parser.parse(data['earliest_date'])
		except Exception, why:
			module_logger.critical("get_files error: earliest_date is not a valid date: " + str(why))
			return False
	
	if 'latest_date' not in data:
		data['latest_date'] = None
	
	elif data['latest_date'] != None and not isinstance(data['latest_date'], datetime):
		try:
			data['latest_date'] = dateutil.parser.parse(data['latest_date'])
		except Exception, why:
			module_logger.critical("get_files error: latest_date is not a valid date: " + str(why))
			return False
	
	return True


# Return tuples of files of different channel, but close in time
def get_sets(data, sets_queue = None):
	
	if not check_get_sets_options(data):
		if sets_queue:
			sets_queue.put(None)
			return False
		else:
			return []
	
	# We start a thread to get the fits_files
	files_queue = Queue()
	data_temp = data.copy()
	data_temp['check_file'] = None
	get_files_thread = threading.Thread(group=None, name='get_files', target=get_files, args=(data_temp, files_queue), kwargs={})
	get_files_thread.start()
	
	sets = list()

	# The get_files normally return the files ordered by ascending time
	# So I don't have to wait to get them all to start
	# I will receive an undef as a signal that there is no more files to process
	
	# I make a set template and fill it with the files as they are comming
	set = dict.fromkeys(data['channels'], None)
	set_date = dict.fromkeys(data['channels'], None)
	
	filename = files_queue.get()
	while filename != None:
		
		#I update the set with the file
		file_date = data['file_date'](filename)
		if not file_date:
			continue
		
		file_channel = data['file_channel'](filename)
		if file_channel == None or file_channel not in data['channels']:
			continue
		
		set[file_channel] = filename
		set_date[file_channel] = file_date
		
		# I check if the set is complete and if the max_deltatime is not exceeded
		if all(set.values()) and (data['max_deltatime'] == None or (max(set_date.values()) - min(set_date.values()) <= data['max_deltatime'])):
			
			# We delay the check of the quality at the latest time possible
			good_set = True
			if data['check_file'] != None:
				for filename in set.values():
					if not data['check_file'](filename):
						good_set = False
						break
			if good_set:
				
				if sets_queue:
					sets_queue.put({'date' : set_date[data['channels'][0]], 'filenames' : [set[c] for c in data['channels']]})
				else:
					sets.append({'date' : set_date[data['channels'][0]], 'filenames' : [set[c] for c in data['channels']]})
				
				set = dict.fromkeys(data['channels'], None)
				set_date = dict.fromkeys(data['channels'], None)
			
			else:
				module_logger.debug("Skipping bad quality set")
		
		filename = files_queue.get()
	
	module_logger.debug("No more sets, exiting thread")
	
	if sets_queue:
		sets_queue.put(None)
		return True
	else:
		return sets


def check_get_reduced_sets_options(data):
	
	if 'earliest_date' not in data:
		data['earliest_date'] = None
	
	elif data['earliest_date'] != None and not isinstance(data['earliest_date'], datetime):
		try:
			data['earliest_date'] = dateutil.parser.parse(data['earliest_date'])
		except Exception, why:
			module_logger.critical("get_files error: earliest_date is not a valid date: " + str(why))
			return False
	
	if 'latest_date' not in data:
		data['latest_date'] = None
	
	elif data['latest_date'] != None and not isinstance(data['latest_date'], datetime):
		try:
			data['latest_date'] = dateutil.parser.parse(data['latest_date'])
		except Exception, why:
			module_logger.critical("get_files error: latest_date is not a valid date: " + str(why))
			return False
	
	if 'frequency' not in data or data['frequency'].seconds == 0:
		data['frequency'] = None
	
	return True


# Return tuples of files of different channel, but close in time at a certain frequency
def get_reduced_sets(data, sets_queue = None):
	
	if not check_get_reduced_sets_options(data):
		if sets_queue:
			sets_queue.put(None)
			return False
		else:
			return []
	
	# If I don't need to reduce the sets, I just call get_sets
	if data['frequency'] == None:
		if sets_queue:
			get_sets_thread = threading.Thread(group=None, name='get_sets', target=get_sets, args=(data, sets_queue), kwargs={})
			get_sets_thread.start()
			return True
		else:
			return get_sets(data)
			 
	
	# We start a thread to get the set
	all_sets_queue = Queue()
	data_temp = data.copy()
	data_temp['check_file'] = None
	get_sets_thread = threading.Thread(group=None, name='get_sets', target=get_sets, args=(data_temp, all_sets_queue), kwargs={})
	get_sets_thread.start()
	
	# The get_sets normally return the sets ordered by ascending time
	# So I don't have to wait to get them all to start
	# I will receive an undef as a signal that there is no more sets to process

	set = all_sets_queue.get()

	if data['earliest_date']:
		start_time = data['earliest_date']
	elif set != None:
		start_time = set['date']
	else:
		start_time = None
	
	if start_time:
		next_time = start_time + data['frequency']

	sets = list()
	
	while set != None:
		
		while (next_time <= set['date']) :
			module_logger.info("get_reduced_sets: no set found for timeslot " + str(start_time) + " -> " + str(next_time))
			start_time = next_time
			next_time = start_time + data['frequency']
		
		if start_time <= set['date'] and set['date'] < next_time:
			
			# We delay the check of the quality at the latest time possible
			good_set = True
			if data['check_file'] != None:
				for filename in set['filenames']:
					if not data['check_file'](filename):
						good_set = False
						break
			if good_set:
				
				if sets_queue:
					module_logger.debug("Giving set "+ str(set))
					sets_queue.put(set)
				else:
					sets.append(set)
				
				start_time = next_time
				next_time = start_time + data['frequency']
			
			else:
				module_logger.debug("Skipping bad quality set")
		
		set = all_sets_queue.get()
		
	module_logger.debug("No more reduced sets, exiting thread")
	
	if sets_queue:
		sets_queue.put(None)
		module_logger.debug("Putted None in sets_queue to warn for end of data")
		return True
	else:
		return sets


# Start point of the script
if __name__ == "__main__":
	
	# Get the arguments
	log_filename = os.path.splitext(os.path.basename(sys.argv[0]))[0] + '.log'
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Test the get_data module.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='set the file where to log')
	parser.add_argument('--data', '-D', default='get_data.config', help='Config file for the data')
	parser.add_argument('--output', '-O', default=None, help='Output file to write the results to')
	args = parser.parse_args()

	# Set up of the logging
	log_filename = args.log_filename
	if args.debug :
		logging.basicConfig(filename=log_filename, level = logging.DEBUG, format='%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
	else:
		logging.basicConfig(filename=log_filename, level = logging.INFO, format='%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
	
	console = logging.StreamHandler()
	console.setFormatter(logging.Formatter('Error: %(name)-12s: %(message)s'))
	if args.verbose:
		console.setLevel(logging.INFO)
	else:
		console.setLevel(logging.CRITICAL)
	
	# Create logger
	module_logger.addHandler(console)

	# Set up of the get_data options
	data = dict()
	try:
		with open(args.data) as f:
			exec(f)
	except (IOError) , why:
		log.critical("Could not open data config file: "+ args.data + ": "+ str(why))
		sys.exit(2)

	
	output = None
	if args.output:
		try:
			output = open(args.output, 'w+')
		except (IOError) , why:
			log.critical("Could not open output file: "+ args.output + ": "+ str(why))
			sys.exit(2)
	
	if False:
		
		# We get the fitfile in a non threaded way
		files = get_files(data_repo, None, check_filename, check_folder, check_file)
		print "files:\n", '\n'.join(files)
		
		# We get the sets in a non threaded way
		sets = get_sets(data_repo, channels, max_deltatime, get_date_channel, None, check_filename, check_folder, check_file)
		print "\nsets:\n", '\n'.join([str(set['date']) + "\t" + str(set['filenames']) for set in sets])
		
		# We get the reduced sets in a non threaded way
		reduced_sets = get_reduced_sets(data_repo, channels, max_deltatime, get_date_channel, earliest_date, frequency, None, check_filename, check_folder, check_file)
		print "\nreduced sets:\n", '\n'.join([str(set['date']) + "\t" + str(set['filenames']) for set in reduced_sets])
	
	
	# We get the sets in a threaded way 
	sets_queue = Queue()
	get_sets_thread = threading.Thread(group=None, name='get_reduced_sets', target=get_reduced_sets, args=(data, sets_queue), kwargs={})
	get_sets_thread.start()
	
	set = sets_queue.get()
	while set != None:
		if output:
			output.write('\n'.join(set['filenames']) + '\n')
			output.flush()
		else:
			print '\n'.join(set['filenames'])
		
		set = sets_queue.get()
	
	if output:
		output.close()
		
