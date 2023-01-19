#!/usr/bin/env python3

import argparse
import logging
import pathlib
import subprocess
import shlex

# Path to ffmpeg executable
FFMPEG_EXECUTABLE = 'ffmpeg'
# Size of the buffer in bytes
FFMPEG_BUFFER_SIZE = 100 * 1024 * 1024
# Polling frequency if ffmpeg has terminated encoding the video in seconds
FFMPEG_EXIT_POLLING = 1

def run_ffmpeg(ffmpeg_cmd, input_filenames = []):
	'''Execute the ffmpeg command, writing the input files one by one to the stdin and returning the return code, sdtout and stderr'''
	
	logging.debug('Executing "%s"', shlex.join(ffmpeg_cmd))
	
	process = subprocess.Popen(
		ffmpeg_cmd,
		bufsize = FFMPEG_BUFFER_SIZE,
		stdin = subprocess.PIPE,
		stdout = subprocess.PIPE,
		stderr = subprocess.PIPE
	)
	
	# Write the input files to the input pipe of ffmpeg one by one
	# And at the same time read the stdout and stderr pipes
	process_output = b''
	process_error = b''
	for input_filename in input_filenames:
		logging.info('Adding file %s to video', input_filename)
		
		try:
			with open(input_filename, 'rb') as input_file:
				process.stdin.write(input_file.read())
		except Exception as why:
			logging.exception('Could not add file %s to video: %s', input_filename, why)
	
	process.stdin.close()
	
	# Wait for ffmpeg to terminate
	# it is important to read the stdout and stderr frequently to avoid a deadlock (as described in the documentation)
	# so wait with a timeout, read stdout and stderr and restart the wait if it exited because of the timeout
	while True:
		try:
			process.wait(timeout = FFMPEG_EXIT_POLLING)
		except subprocess.TimeoutExpired:
			pass
		else:
			break
		finally:
			process_output += process.stdout.read()
			process_error += process.stderr.read()
	
	logging.debug('ffmpeg stdout:\n%s', process_output.decode('utf-8'))
	logging.debug('ffmpeg stderr:\n%s', process_error.decode('utf-8'))
	
	if process.returncode != 0:
		raise RuntimeError('ffmpeg exited with return code: %s\nffmpeg stdout:\n%s\nffmpeg stderr:\n%s', process.returncode, process_output.decode('utf-8'), process_error.decode('utf-8'))


def images_to_mp4_video(input_filenames, output_filename, frame_rate = 24, title = None, size = None):
	'''Concat a list of images into a mp4 video using ffmpeg'''
	
	# Typical parameters for converting images to an mp4 video with an x264 video codec
	# the images are piped to ffmpeg stdin, ffmpeg should deduce the image type
	ffmpeg = [FFMPEG_EXECUTABLE, '-y', '-r', str(frame_rate), '-f', 'image2pipe', '-i', '-', '-an', '-vcodec', 'libx264', '-preset', 'slow', '-vprofile', 'baseline', '-pix_fmt', 'yuv420p']
	
	if title:
		ffmpeg.extend(['-metadata', 'title=%s' % title])
	
	if size:
		ffmpeg.extend(['-s', size])
	
	ffmpeg.append(output_filename)
	
	logging.info('Encoding video %s', output_filename)
	
	try:
		run_ffmpeg(ffmpeg, input_filenames)
	except Exception as why:
		logging.exception('Error making video %s: %s', output_filename, why)
	else:
		logging.info('Wrote video %s', output_filename)


# Start point of the script
if __name__ == '__main__':
	
	# Get the arguments
	parser = argparse.ArgumentParser(description = 'Make a video from images using ffmpeg (needs to be installed and in your path)')
	parser.add_argument('--verbose', '-v', choices = ['DEBUG', 'INFO', 'ERROR'], default = 'INFO', help = 'Set the logging level (default is INFO)')
	parser.add_argument('--overwrite', '-f', action = 'store_true', help = 'Overwrite the video if it already exists')
	parser.add_argument('--frame-rate', '-r', default = 24, type = float, help = 'Frame rate for the video')
	parser.add_argument('--title', '-t', help = 'A title for the video')
	parser.add_argument('--size', '-s', help = 'The size of the video. Must be specified like widthxheight in pixels')
	parser.add_argument('--output', '-o', required = True, help = 'The filename for the video, must end in mp4')
	parser.add_argument('sources', nargs = '+', help = 'The paths of the source images')
	
	args = parser.parse_args()
	
	# Setup the logging
	logging.basicConfig(level = getattr(logging, args.verbose), format = '%(asctime)s %(levelname)-8s: %(message)s')
	
	output_path = pathlib.Path(args.output)
	if output_path.suffix != '.mp4':
		parser.error('Unsupported video suffix for output file %s' % args.output)
	
	if output_path.exists() and not args.overwrite:
		parser.error('Output file %s already exists, not overwriting' % args.output)
	
	images_to_mp4_video(args.sources, args.output, frame_rate = args.frame_rate, title = args.title, size = args.size)
