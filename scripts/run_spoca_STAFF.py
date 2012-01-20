#!/usr/bin/python2.6
import os, os.path, sysimport pickle
import subprocess
import logging
import smtplib
from email.mime.text import MIMEText
import argparse   
import glob
import time
from datetime import datetime, timedelta
from spoca_job import segmentation, get_STAFF_stats

# Directory where the AIA files are located
aia_directory = "/pool/data/aia.lev1"

# The date of the first data to process
start_date = datetime(2010, 05, 13, 00, 00)

# Directory to store prepped AIA files
prepped_directory = "/pool/staff/AIA/data"

# Path to the solar soft installation directory
solar_soft_path = "/pool/ssw"

# Path to the idl bin
idl_bin = "/usr/local/bin/idl"

# Path to the runtime idl programm to prep the AIA data
runtime_aia_prep = "/pool/staff/AIA/runtime_aia_prep.sav"

# Path to the spoca_ar classification program
spoca_ar_bin = "/pool/staff/AIA/spoca_ar/bin/classification.x"

# Path to the config of spoca_ar classification program
spoca_ar_config = "/pool/staff/AIA/spoca_ar/scripts/configs/AIA_AR.segmentation.config"

# Wavelengths for spoca_ar
spoca_ar_wavelengths = [171, 193]

# Directory to output the CH maps
ARmaps_directory = "/pool/staff/AIA/ARmaps"

# Path to the spoca_ch classification program
spoca_ch_bin = "/pool/staff/AIA/spoca_ch/bin/classification.x"

# Path to the config of spoca_ch classification program
spoca_ch_config = "/pool/staff/AIA/spoca_ch/scripts/configs/AIA_CH.segmentation.config"

# Wavelengths for spoca_ch
spoca_ch_wavelengths = [193]

# Directory to output the AR maps
CHmaps_directory = "/pool/staff/AIA/CHmaps"

# Path to the get_STAFF_stats program
get_STAFF_stats_bin = "/pool/staff/AIA/spoca_ch/bin/get_STAFF_stats.x"

# Path to the config of get_STAFF_stats program
get_STAFF_stats_config = "/pool/staff/AIA/spoca_ch/scripts/configs/get_AIA_STAFF_stats.config"

# Wavelengths for get_STAFF_stats
get_STAFF_stats_wavelengths = [171, 193]

# Directory to output the STAFF stats results
stats_directory = "/pool/staff/AIA/stats"

# The cadence of the desired results
run_cadence = timedelta(hours = 6)

# The time that must be waited before processing data
run_delay = timedelta(days = 60)

# The maximal number of errors before someone must be emailed
max_number_errors = 5

# The emails of the people to warn in case of too many errors
emails = ["cis.verbeeck@oma.be", "benjamin.mampaey@oma.be"]

# The smtp server to send the emails
smtp_server = 'smtp.oma.be'

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
		import logging.handlers
		fh = logging.FileHandler(filename, delay=True)
		fh.setFormatter(logging.Formatter('%(asctime)s %(name)-12s %(levelname)-8s %(funcName)-12s %(message)s', datefmt='%Y-%m-%d %H:%M:%S'))
		if debug:
			fh.setLevel(logging.DEBUG)
		else:
			fh.setLevel(logging.INFO)
		
		logging.root.addHandler(fh)

def send_email(sender, adresses, subject, messages = []):
	
	mail_body = "\n".join(messages)
	msg = MIMEText(mail_body)
	msg['Subject'] = subject
	msg['From'] = sender
	msg['To'] = ';'.join(adresses)
	log.debug("Sending email: %s", msg.as_string())
	try:
		s = smtplib.SMTP(smtp_server)
		s.sendmail(sender, adresses, msg.as_string())
		s.quit()
	except Exception, why:
		log.critical("Could not send mail %s to smtp server %s: %s", msg.as_string(), smtp_server, str(why))


def find_files(base_directory, date, wavelengths):
	'''Search and return the filepath for a date, and the specified wavelengths'''
	path = os.path.join(base_directory, date.strftime('%Y/%m/%d/H%H00/'))
	files = dict()
	for wavelength in wavelengths:
		filename = '*.%04d.fits' % wavelength
		filepaths = glob.glob(os.path.join(path, filename))
		if filepaths:
			files[wavelength] = filepaths[0]
	
	log.debug("Found following files in folder %s for date %s: %s", base_directory, start_date, files)
	return files


def aia_prep_files(filepaths, prepped_directory, retries = 10):
	
	# select the files not already prepped
	to_prep = list()
	for filepath in filepaths:
		prepped_file = os.path.join(prepped_directory, os.path.basename(filepath))
		if not os.path.exists(prepped_file):
			to_prep.append(filepath)
		else:
			log.debug("File %s is already present, not doing aia prep for file %s", prepped_file, filepath)
	
	if to_prep:
		log.info("Running aia_prep on files %s", to_prep)
		try:
			# Set up the environ
			os.environ['SSW'] = solar_soft_path
			os.environ['SSW_ONTOLOGY'] = os.path.join(solar_soft_path, 'vobs/ontology')
			os.environ['AIA_CALIBRATION'] = os.path.join(solar_soft_path, 'sdo/aia/calibration')
			os.environ['DISPLAY'] = ''
			
		except OSError, why:
			log.critical("Could set up environnement to run idl: %s", str(why))
			return False
		
		# Call idl with the runtime_aia_prep
		idl_command = [idl_bin, '-queue', '-quiet', '-rt='+runtime_aia_prep, '-args', prepped_directory]
		idl_command.extend(to_prep)
		idl_process = subprocess.Popen(idl_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		output, errors = idl_process.communicate()
		if idl_process.returncode == 0:
			log.debug("Sucessfully ran idl command %s, output: %s, errors: %s", ' '.join(idl_command), str(output), str(errors))
		else:
			log.error("Error running idl command %s, output: %s, errors: %s", ' '.join(idl_command), str(output), str(errors))
	
	# Return the path to the prepped files
	prepped_files = list()
	for filepath in filepaths:
		prepped_file = os.path.join(prepped_directory, os.path.basename(filepath))
		if os.path.exists(prepped_file):
			prepped_files.append(prepped_file)
		else:
			log.error("Prepped file %s is missing", prepped_file)
			prepped_files.append(None)
	
	# If the prepping didn't succeed, we retry to do it
	if not all(prepped_file) and retries > 0:
		log.info("Some files were not aia prepped succesfully, retrying.")
		# The main reason the prepping don't succeed is a license shortage. So we wait 5 minute hoping that one will be free and try again.
		time.sleep(300) 
		return aia_prep_files(filepaths, prepped_directory, retries - 1)
	else:
		return prepped_files


def setup_spoca(spoca_bin, configfile, output_dir):
	
	class segmentation_instance(segmentation):
		pass
	
	segmentation_instance.set_parameters(configfile, output_dir)
	segmentation_instance.bin = spoca_bin
	ok, reason = segmentation_instance.test_parameters()
	if ok:
		log.info("Spoca parameters in file %s seem ok", configfile)
		log.debug(reason)
	else:
		log.warn("Spoca parameters in file %s could be wrong", configfile)
		log.warn(reason)
	
	return segmentation_instance


def run_spoca(spoca, fitsfiles, name):
	
	log.info("Running spoca on files %s", fitsfiles)
	spoca_command = [spoca.bin] + spoca.build_arguments(fitsfiles, name)
	spoca_process = subprocess.Popen(spoca_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	output, errors = spoca_process.communicate()
	if spoca_process.returncode == 0:
		log.debug("Sucessfully ran spoca command %s, output: %s, errors: %s", ' '.join(spoca_command), str(output), str(errors))
		return True
	else:
		log.error("Error running spoca command %s, output: %s, errors: %s", ' '.join(spoca_command), str(output), str(errors))
		return False

def run_get_STAFF_stats(get_STAFF_stats, ARmap, CHmap, fitsfiles, outfile):
	
	log.info("Running get_STAFF_stats on CHmap %s, ARmap %s, and files %s", CHmap, ARmap, fitsfiles)
	get_STAFF_stats_command = [get_STAFF_stats.bin] + get_STAFF_stats.build_arguments(CHmap, ARmap, fitsfiles)
	get_STAFF_stats_process = subprocess.Popen(get_STAFF_stats_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	output, errors = get_STAFF_stats_process.communicate()
	if get_STAFF_stats_process.returncode == 0:
		log.debug("Sucessfully ran get_STAFF_stats command %s, output: %s, errors: %s", ' '.join(get_STAFF_stats_command), str(output), str(errors))
		try:
			with open(outfile, "w") as f:
				f.write(str(output))
		except IOError, why:
			log.error("Could not write get_STAFF_stats results in file %s: %s", outfile, str(why))
			return False
		else:
			return True
	else:
		log.error("Error running get_STAFF_stats command %s, output: %s, errors: %s", ' '.join(get_STAFF_stats_command), str(output), str(errors))
		return False

# Start point of the script
if __name__ == "__main__":
	
	script_name = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	
	# Default path for the log file
	log_filename = os.path.join('.', script_name+'.log')
	
	# Sender email address of the script
	sender = "%s@oma.be" % script_name
	
	# Default path for the status file
	status_filename = os.path.join('.', script_name+'_status.pickle')
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Run spoca on many fits files.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='set the logging level to debug for the log file')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='set the logging level to verbose at the screen')
	parser.add_argument('--log_filename', '-l', default=log_filename, help='set the file where to log')
	parser.add_argument('--status_filename', '-s', default=status_filename, help='set the file where to save the status')
	
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(filename = args.log_filename, quiet = False, verbose = args.verbose, debug = args.debug)
	
	# Create a logger
	log = logging.getLogger(script_name)
	
	# Set the status filename
	if args.status_filename:
		status_filename = args.status_filename
	
	# Restore the previous status if any 
	status = dict()
	try: 
		pickle_file = open(status_filename, 'rb')
	except IOError, why:
		log.info("Could not open status file %s for restoring status: %s", status_filename, str(why))
	else:
		log.info("Restoring previous status from status file %s", status_filename)
		try:
			status = pickle.load(pickle_file)
			pickle_file.close()
			log.info("Status restored: %s", status)
		except Exception, why:
			log.critical("Could not restore status from file %s: %s", status_filename, str(why))
			sys.exit(2)
	
	# Parse the status
	if status:
		try:
			start_date = status['start_date']
		except ValueError, why:
			log.warning("Could not restore start_date from status, skipping")
	
	# Setup the spoca_ar segmentation parameters
	spoca_ar = setup_spoca(spoca_ar_bin, spoca_ar_config, ARmaps_directory)
	
	# Setup the spoca_ch segmentation parameters
	spoca_ch = setup_spoca(spoca_ch_bin, spoca_ch_config, CHmaps_directory)
	
	# Setup the get_STAFF_stats
	get_STAFF_stats.set_parameters(get_STAFF_stats_config, stats_directory)
	get_STAFF_stats.bin = get_STAFF_stats_bin
	ok, reason = get_STAFF_stats.test_parameters()
	if ok:
		log.info("get_STAFF_stats parameters in file %s seem ok", get_STAFF_stats_config)
		log.debug(reason)
	else:
		log.warn("get_STAFF_stats parameters in file %s could be wrong", get_STAFF_stats_config)
		log.warn(reason)
	
	# Make the list of all needed wavelengths
	wavelengths = sorted(list(set(spoca_ar_wavelengths + spoca_ch_wavelengths + get_STAFF_stats_wavelengths)))
	
	# Counter for the number of failure
	number_errors = 0
	
	# Start the big loop
	while(True):
		
		stop = False
		
		# We check if the files we need are already available or if we need to wait
		if datetime.now() <= start_date + run_delay :
			time_to_wait = run_delay - (datetime.now() - start_date)
			log.info("Waiting %s hours for data to be available", time_to_wait)
			seconds_to_wait = (time_to_wait.microseconds + (time_to_wait.seconds + time_to_wait.days * 24 * 3600) * 10**6) / 10**6
			if seconds_to_wait > 0:
				log.debug("Sleeping for %s seconds", seconds_to_wait)
				time.sleep(seconds_to_wait)
		
		# Find the files we need
		files = find_files(aia_directory, start_date, wavelengths)
		
		# Check if we have all the files we need
		filepaths = list()
		for w in wavelengths:
			if w in files:
				filepaths.append(files[w])
			else:
				log.warning("Missing aia files for date %s wavelength %s. Not running spoca.", start_date, w)
				stop = True
		
		# Prep the files
		if not stop:
			filepaths = aia_prep_files(filepaths, prepped_directory)
			for i, w in enumerate(wavelengths):
				if i < len(filepaths) and filepaths[i]:
					files[w] = filepaths[i]
				else:
					log.warning("Missing prepped file for file %s, skipping spoca run for date %s", files[w], start_date)
					stop = True
					break
		
		
		map_name = start_date.strftime('%Y%m%d_%H%M%S')
		
		# We run spoca for AR
		if not stop:
			stop = not run_spoca(spoca_ar, [files[w] for w in spoca_ar_wavelengths], map_name)
		
		# We check if the ARmap exists
		if not stop:
			ARmap = os.path.join(ARmaps_directory, map_name+'.SegmentedMap.fits')
			if not os.path.exists(ARmap):
				log.error("Could not find ARmap %s", ARmap)
				stop = True
		
		# We run spoca for CH
		if not stop:
			stop = not run_spoca(spoca_ch, [files[w] for w in spoca_ch_wavelengths], map_name)
		
		# We check if the CHmap exists
		if not stop:
			CHmap = os.path.join(CHmaps_directory, map_name+'.SegmentedMap.fits')
			if not os.path.exists(CHmap):
				log.error("Could not find CHmap %s", CHmap)
				stop = True
		
		# Run get_STAFF_stats
		if not stop:
			outfile = os.path.join(stats_directory, map_name+'.csv')
			stop = not run_get_STAFF_stats(get_STAFF_stats, ARmap, CHmap, [files[w] for w in get_STAFF_stats_wavelengths], outfile)
		
		# Check if there was an error
		if stop:
			# If stop is set there was an error, we increase the counter
			number_errors += 1
		else:
			# If stop is not set there was no error, we decrease the counter
			number_errors = max(number_errors - 1, 0)
		
		# If the number of errors is too high, tell someone
		if number_errors > max_number_errors:
			send_email(sender, emails, "Too many errors in %s" % script_name, ["There has been %d errors in the script %s" % (number_errors, script_name), "Please check the log file %s for reasons, and take any neccesary action" % log_filename])
			# We reset the counter to avoid sendinf too many emails
			number_errors = 0
		
		# We update the start_date for the next run
		start_date += run_cadence
		
		# We update the status
		status['start_date'] = start_date
		
		# We save the status
		try: 
			pickle_file = open(status_filename, 'wb')
		except IOError, why:
			log.error("Could not open status file %s for saving status: %s", status_filename, str(why))
		else:
			log.debug("Saving status to status file %s", status_filename)
			try:
				pickle.dump(status, pickle_file, -1)
				pickle_file.close()
			except Exception, why:
				log.error("Could not save status from file %s: %s", status_filename, str(why))
			pickle_file.close()
