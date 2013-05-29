#!/usr/bin/env python2.6
# -*- coding: iso-8859-15 -*-
import sys
import subprocess, shlex
import os
import os.path
import string
import logging
import argparse
import tempfile

# parameters for mencoder
ffmpeg_bin = '/pool/software/ffmpeg/bin/ffmpeg'

def run_ffmpeg(ffmpeg_cmd, input_filenames = []):
	
	# We redirect the stdout and stderr to temporary files
	try:
		stdout_file = tempfile.TemporaryFile()
		stderr_file = tempfile.TemporaryFile()
	except Exception, why:
		logging.critical('Failed creating temporary files for stout and stderr : %s', str(why))
		return False
	
	try:
		# We start ffmpeg
		logging.debug("About to execute: %s", ' '.join(ffmpeg_cmd))
		process = subprocess.Popen(ffmpeg_cmd, bufsize = 100 * 1024 * 1024,shell=False, stdin=subprocess.PIPE, stdout=stdout_file, stderr=stderr_file, close_fds = True)
		#logging.debug("Created process for: %s", ' '.join(ffmpeg_cmd))
		
		# If we have input files, we write their content to the input pipe of ffmpeg
		for input_filename in input_filenames:
			try:
				with open(input_filename, 'rb') as input_file:
					logging.debug("Adding image %s to %s", input_filename, ffmpeg_cmd[-1])
					process.stdin.write(input_file.read())
			except Exception, why:
				logging.error("Could not add image %s to video: %s", input_filename, str(why))
		
		process.stdin.close()
		
		# We wait for ffmpeg to terminate
		logging.debug("Waiting to terminate process for: %s", ' '.join(ffmpeg_cmd))
		return_code = process.wait()
		if return_code != 0:
			stdout_file.seek(0)
			stdout = stdout_file.read()
			stderr_file.seek(0)
			stderr = stderr_file.read()
			logging.error('Failed running command %s :\nReturn code : %d\n StdOut: %s\n StdErr: %s', ' '.join(ffmpeg_cmd), return_code, stdout, stderr)
	except Exception, why:
		logging.critical('Failed running command %s : %s', ' '.join(ffmpeg_cmd), str(why))
		return False
	else:
		if logging.root.isEnabledFor(logging.DEBUG):
			stdout_file.seek(0)
			stdout = stdout_file.read()
			logging.debug("ffmpeg stdout:\n%s\n", stdout)
			stderr_file.seek(0)
			stderr = stderr_file.read()
			logging.debug("ffmpeg stderr:\n%s\n", stderr)
		return True


def png_to_mp4_video(input_filenames, output_filename, frame_rate = 24, video_title = None, video_size = None, video_bitrate = None):
	
	# We verify the extension
	output_path, extension = os.path.splitext(output_filename)
	if extension != ".mp4":
		output_filename = output_path + ".mp4"
		logging.warning("Output filename for mp4 video does not have extension .mp4, changing output filename to %s", output_filename)
	
	# We set up ffmpeg for the creation of mp4
	ffmpeg = [ffmpeg_bin, '-y', '-r', str(frame_rate), '-f', 'image2pipe', '-vcodec', 'png', '-i', '-', '-an', '-vcodec', 'libx264', '-preset', 'slow', '-vprofile', 'baseline', '-pix_fmt', 'yuv420p']
	if video_bitrate:
		ffmpeg.extend(['-maxrate', str(video_bitrate) + 'k'])
	
	if video_title:
		ffmpeg.extend(['-metadata', 'title=' + str(video_title)])
	
	if video_size:
		ffmpeg.extend(['-s', video_size])
	
	ffmpeg.append(output_filename)
	
	logging.info("Making video %s", output_filename)
	return run_ffmpeg(ffmpeg, input_filenames)


def png_to_webm_video(input_filenames, output_filename, frame_rate = 24, video_title = None, video_size = None, video_bitrate = None):
	
	# We verify the extension
	output_path, extension = os.path.splitext(output_filename)
	if extension != ".webm":
		output_filename = output_path + ".webm"
		logging.warning("Output filename for webm video does not have extension .webm, changing output filename to %s", output_filename)
	
	# We set up ffmpeg for the creation of webm
	ffmpeg = [ffmpeg_bin, '-y', '-r', str(frame_rate), '-f', 'image2pipe', '-vcodec', 'png', '-i', '-', '-an', '-vcodec', 'libvpx', '-quality', 'good', '-cpu-used', '0', '-qmin', '10', '-qmax', '42', '-threads', '2']
	if video_bitrate:
		ffmpeg.extend(['-maxrate', str(video_bitrate) + 'k', '-bufsize', str(2*video_bitrate) + 'k'])
	else:
		ffmpeg.extend(['-bufsize', '1000k'])
	
	if video_title:
		ffmpeg.extend(['-metadata', 'title=' + str(video_title)])
	
	ffmpeg.append(output_filename)
	
	logging.info("Making video %s", output_filename)
	return run_ffmpeg(ffmpeg, input_filenames)


def png_to_ogv_video(input_filenames, output_filename, frame_rate = 24, video_title = None, video_size = None, video_bitrate = None):
	
	# We verify the extension
	output_path, extension = os.path.splitext(output_filename)
	if extension != ".ogv":
		output_filename = output_path + ".ogv"
		logging.warning("Output filename for ogv video does not have extension .ogv, changing output filename to %s", output_filename)
	
	# We set up ffmpeg for the creation of ogv
	ffmpeg = [ffmpeg_bin, '-y', '-r', str(frame_rate), '-f', 'image2pipe', '-vcodec', 'png', '-i', '-', '-an', '-vcodec', 'libtheora', '-q:v', '7']
	if video_bitrate:
		ffmpeg.extend(['-maxrate', str(video_bitrate) + 'k'])
	
	if video_title:
		ffmpeg.extend(['-metadata', 'title=' + str(video_title)])
	
	ffmpeg.append(output_filename)
	
	logging.info("Making video %s", output_filename)
	return run_ffmpeg(ffmpeg, input_filenames)


def png_to_ts_video(input_filenames, output_filename, frame_rate = 24, video_title = None, video_size = None, video_bitrate = None, video_preset='ultrafast'):
	
	# We verify the extension
	output_path, extension = os.path.splitext(output_filename)
	if extension != ".ts":
		output_filename = output_path + ".ts"
		logging.warning("Output filename for ts video does not have extension .ts, changing output filename to %s", output_filename)
	
	# We set up ffmpeg for the creation of ts
	ffmpeg = [ffmpeg_bin, '-y', '-loglevel', 'debug', '-r', str(frame_rate), '-f', 'image2pipe', '-vcodec', 'png', '-i', '-', '-an', '-vcodec', 'libx264', '-preset', video_preset, '-qp', '0']
	if video_bitrate:
		ffmpeg.extend(['-maxrate', str(video_bitrate) + 'k'])
	
	if video_title:
		ffmpeg.extend(['-metadata', 'title=' + str(video_title)])
	
	if video_size:
		ffmpeg.extend(['-s', video_size])
	
	ffmpeg.append(output_filename)
	
	logging.info("Making video %s", output_filename)
	return run_ffmpeg(ffmpeg, input_filenames)


def video_to_mp4_video(input_filenames, output_filename, frame_rate = 24, video_title = None, video_size = None, video_bitrate = None):
	
	# We verify the extension
	output_path, extension = os.path.splitext(output_filename)
	if extension != ".mp4":
		output_filename = output_path + ".mp4"
		logging.warning("Output filename for mp4 video does not have extension .mp4, changing output filename to %s", output_filename)
	
	# We set up ffmpeg for the creation of mp4
	ffmpeg = [ffmpeg_bin, '-y', '-i']
	if isinstance(input_filenames, basestring):
		 ffmpeg.append(input_filenames)
	elif len(input_filenames) == 1:
		 ffmpeg.append(input_filenames[0])
	else:
		ffmpeg.append("concat:"+'|'.join(input_filenames))
	
	ffmpeg.extend(['-an', '-vcodec', 'libx264', '-preset', 'slow', '-vprofile', 'baseline', '-pix_fmt', 'yuv420p', '-r', str(frame_rate)])
	
	if video_bitrate:
		ffmpeg.extend(['-maxrate', str(video_bitrate) + 'k'])
	
	if video_title:
		ffmpeg.extend(['-metadata', 'title=' + str(video_title)])
	
	if video_size:
		ffmpeg.extend(['-s', video_size])
	
	ffmpeg.append(output_filename)
	
	logging.info("Making video %s", output_filename)
	return run_ffmpeg(ffmpeg)

def video_to_webm_video(input_filenames, output_filename, frame_rate = 24, video_title = None, video_size = None, video_bitrate = None):
	
	# We verify the extension
	output_path, extension = os.path.splitext(output_filename)
	if extension != ".webm":
		output_filename = output_path + ".webm"
		logging.warning("Output filename for webm video does not have extension .webm, changing output filename to %s", output_filename)
	
	# We set up ffmpeg for the creation of webm
	ffmpeg = [ffmpeg_bin, '-y', '-i']
	if isinstance(input_filenames, basestring):
		 ffmpeg.append(input_filenames)
	elif len(input_filenames) == 1:
		 ffmpeg.append(input_filenames[0])
	else:
		ffmpeg.append("concat:"+'|'.join(input_filenames))
	
	ffmpeg.extend(['-an', '-vcodec', 'libvpx', '-quality', 'good', '-cpu-used', '0', '-qmin', '10', '-qmax', '42', '-threads', '2', '-r', str(frame_rate)])
	
	if video_bitrate:
		ffmpeg.extend(['-maxrate', str(video_bitrate) + 'k'])
	
	if video_title:
		ffmpeg.extend(['-metadata', 'title=' + str(video_title)])
	
	if video_size:
		ffmpeg.extend(['-s', video_size])
	
	ffmpeg.append(output_filename)
	
	logging.info("Making video %s", output_filename)
	return run_ffmpeg(ffmpeg)

def video_to_ogv_video(input_filenames, output_filename, frame_rate = 24, video_title = None, video_size = None, video_bitrate = None):
	
	# We verify the extension
	output_path, extension = os.path.splitext(output_filename)
	if extension != ".ogv":
		output_filename = output_path + ".ogv"
		logging.warning("Output filename for ogv video does not have extension .ogv, changing output filename to %s", output_filename)
	
	# We set up ffmpeg for the creation of ogv
	ffmpeg = [ffmpeg_bin, '-y', '-i']
	if isinstance(input_filenames, basestring):
		 ffmpeg.append(input_filenames)
	elif len(input_filenames) == 1:
		 ffmpeg.append(input_filenames[0])
	else:
		ffmpeg.append("concat:"+'|'.join(input_filenames))
	
	ffmpeg.extend(['-an', '-vcodec', 'libtheora', '-q:v', '7', '-r', str(frame_rate)])
	
	if video_bitrate:
		ffmpeg.extend(['-maxrate', str(video_bitrate) + 'k'])
	
	if video_title:
		ffmpeg.extend(['-metadata', 'title=' + str(video_title)])
	
	if video_size:
		ffmpeg.extend(['-s', video_size])
	
	ffmpeg.append(output_filename)
	
	logging.info("Making video %s", output_filename)
	return run_ffmpeg(ffmpeg)


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
	elif debug:
		logging.root.handlers[0].setLevel(logging.DEBUG)
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
	
	known_video_types = ['.ts', '.mp4', '.ogv', '.webm']
	
	# Get the arguments
	parser = argparse.ArgumentParser(description='Make video from png.')
	parser.add_argument('--debug', '-d', default=False, action='store_true', help='Set the logging level to debug')
	parser.add_argument('--quiet', '-q', default=False, action='store_true', help='Do not display any error message.')
	parser.add_argument('--overwrite', '-o', default=False, action='store_true', help='Overwrite the video if it already exists')
	parser.add_argument('--frame_rate', '-r', default=24, type=float, help='Frame rate for the video')
	parser.add_argument('--video_bitrate', '-b', default=0, type=float, help='A maximal bitrate for the video in kb')
	parser.add_argument('--video_title', '-t', default=None, help='A title for the video')
	parser.add_argument('--video_size', '-s', default=None, help='The size of the video. Must be specified like widthxheight in pixels')
	parser.add_argument('--video_filenames', '-f', action='append', help='The filenames for the video. Must end in ' + ', '.join(known_video_types))	
	parser.add_argument('sources', nargs='+', help='The paths of the source png images/video(s).')
	
	args = parser.parse_args()
	
	# Setup the logging
	setup_logging(quiet = args.quiet, verbose = True, debug = args.debug)
	
	if not args.video_filenames:
		logging.error("You must specify at least one video filename")
		sys.exit(2)
	
	videos_to_make = dict()
	for video_filename in args.video_filenames:
		video_path, video_extension = os.path.splitext(video_filename)
		if video_extension in known_video_types:
			if video_extension in videos_to_make:
				logging.warning("Video of type %s has been specfied more than once as argument, using last one %s", video_extension, video_filename)
			videos_to_make[video_extension] = video_filename
		else:
			logging.error("Unknown video type %s for video %s", video_extension, video_filename)
	
	for video_filename in videos_to_make.values():
		if video_filename and os.path.exists(video_filename):
			if not args.overwrite:
				logging.critical("Video %s already exists, not overwriting", video_filename)
				sys.exit(1)
			else:
				logging.info("Video %s will be overwritten", video_filename)
	
	source_path, source_extension = os.path.splitext(args.sources[0])
	
	if source_extension == '.png':
		logging.info("Source type detected is png images")
		# We make a video from png images
		if '.ts' in videos_to_make:
			png_to_ts_video(args.sources, videos_to_make['.ts'], frame_rate = args.frame_rate, video_title = args.video_title, video_size = args.video_size, video_bitrate = args.video_bitrate)
			video_sources = videos_to_make['.ts']
			del videos_to_make['.ts']
		
		elif '.mp4' in videos_to_make:
			png_to_mp4_video(args.sources, videos_to_make['.mp4'], frame_rate = args.frame_rate, video_title = args.video_title, video_size = args.video_size, video_bitrate = args.video_bitrate)
			video_sources = videos_to_make['.mp4']
			del videos_to_make['.mp4']
		
		elif '.webm' in videos_to_make:
			png_to_webm_video(args.sources, videos_to_make['.webm'], frame_rate = args.frame_rate, video_title = args.video_title, video_size = args.video_size, video_bitrate = args.video_bitrate)
			video_sources = videos_to_make['.webm']
			del videos_to_make['.webm']
		
		elif '.ogv' in videos_to_make:
			png_to_ogv_video(args.sources, videos_to_make['.ogv'], frame_rate = args.frame_rate, video_title = args.video_title, video_size = args.video_size, video_bitrate = args.video_bitrate)
			video_sources = videos_to_make['.ogv']
			del videos_to_make['.ogv']
	else:
		logging.info("Source type detected is videos")
		video_sources = args.sources
	
	# We make the remaining videos from video sources
	if '.mp4' in videos_to_make:
		video_to_mp4_video(video_sources, videos_to_make['.mp4'], frame_rate = args.frame_rate, video_title = args.video_title, video_size = args.video_size, video_bitrate = args.video_bitrate)

	elif '.webm' in videos_to_make:
		video_to_webm_video(video_sources, videos_to_make['.webm'], frame_rate = args.frame_rate, video_title = args.video_title, video_size = args.video_size, video_bitrate = args.video_bitrate)

	elif '.ogv' in videos_to_make:
		video_to_ogv_video(video_sources, videos_to_make['.ogv'], frame_rate = args.frame_rate, video_title = args.video_title, video_size = args.video_size, video_bitrate = args.video_bitrate)

