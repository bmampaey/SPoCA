import math
import random
import Image, ImageDraw
import logging
import sys
import argparse

def distance(a, b):
	
	result = 0
	for x in zip(a, b):
		result += (x[0] - x[1])**2
	
	return math.sqrt(result)

def grey_level(color):
	return 100. - (((255.*(color[2]-color[1]))**2 + (255.*(color[0]-color[2]))**2 + (255. *(color[1]-color[0]))**2)/84565012.5)

def draw_gradient(colors, filename):
	image = Image.new("RGB", (1, len(colors)))
	draw = ImageDraw.Draw(image)
	for y in range(len(colors)):
		draw.rectangle([(0, y),(1, y+1)], fill=colors[y])
	image.save(filename)

def draw_gradient_preview(colors, filename):
	width = 100
	height = 20
	image = Image.new("RGB", (width, len(colors)*height))
	draw = ImageDraw.Draw(image)
	for y in range(len(colors)):
		draw.rectangle([(0, y * height),(width, (y+1) * height)], fill=colors[y])
		draw.text((1, (y+0.1) * height), str(colors[y]), fill=(128,128,128))
	image.save(filename)

def write_C_header(filename, colors):
		web_colors = ['"#%02x%02x%02x"' % color for color in colors]
		try:
			with open(filename, 'w+') as f:
				f.write("#ifndef GRADIENT_H\n")
				f.write("#define GRADIENT_H\n")
				f.write("const unsigned gradientMax = "+str(len(colors))+";\n")
				f.write("const char* const gradient[] = {"+','.join(web_colors)+"};\n")
				f.write("#endif\n")
		except IOError:
			log.critical("Error writing to file " + str(filename)+ ": " + str(why))

def write_Python_header(filename, colors):
		web_colors = ['"#%02x%02x%02x"' % color for color in colors]
		try:
			with open(filename, 'w+') as f:
				f.write("gradient = ["+','.join(web_colors)+"]\n")
		except IOError:
			log.critical("Error writing to file " + str(filename)+ ": " + str(why))


# Start point of the script
if __name__ == "__main__":
	# Get the arguments
	parser = argparse.ArgumentParser(description='Generate a gradient of color as a png image, a c include file, a python list of colors.')
	parser.add_argument('--verbose', '-v', default=False, action='store_true', help='Verbose')
	parser.add_argument('--increment', '-i', default=30, type=int, help='The minimal increment between colors. Must be a value between 1 and 255')
	parser.add_argument('--max_grey_level', '-g', default=80, type=int, help='The maximal accepeted grey level in percent. Must be a value between 0 and 100')
	parser.add_argument('--window_size', '-w', default=10, type=int, help='The size of the running window in wich colors must not be similar ')
	args = parser.parse_args()
	
	console = logging.StreamHandler()
	console.setFormatter(logging.Formatter('%(levelname)-8s: %(message)s'))
	if args.verbose:
		console.setLevel(logging.INFO)
	else:
		console.setLevel(logging.CRITICAL)
	
	# Create logger
	log = logging.getLogger('make_gradient')
	log.addHandler(console)
	
	# set up of the global variables
	if args.increment < 1 or args.increment > 255:
		log.critical("increment must be between 1 and 255, has been set to " + str(increment))
		sys.exit(2)
	else:
		increment = args.increment
	
	if args.max_grey_level < 0 or args.max_grey_level > 100:
		log.critical("max_grey_level must be between 0 and 100, has been set to " + str(increment))
		sys.exit(2)
	else:
		max_grey_level = args.max_grey_level
	if args.window_size < 0:
		log.info("window_size must not be smaller than 0, forcing to 0")
		window_size = 0
	else:
		window_size = args.window_size
	
	colors = list()
	# We generate all the non greyish possible colors
	for red in range(0,256,increment):
		for green in range(0,256,increment):
			for blue in range(0,256,increment):
				color = (red,green,blue)
				if grey_level(color) < max_grey_level:
					colors.append(color)
				else:
					log.info("Discarding greyish color " + str(color))

	# We separate the similar colors as much as we can in a running window
	random.shuffle(colors)
	separated_colors = [colors.pop()]
	while len(colors) > 0:
		furthest = None
		max_distance = 0
		for c in colors:
			dist = min([distance(c, s) for s in separated_colors[-window_size:]])
			if dist > max_distance:
				max_distance = dist
				furthest = c
		color = furthest
		colors.remove(color)
		separated_colors.append(color)

	colors = separated_colors
	
	draw_gradient(colors, "gradient.png")
	draw_gradient_preview(colors, "gradient_preview.png")
	
	# We write the C header
	write_C_header("gradient.h", colors)
	# We write the python header
	write_Python_header("gradient.py", colors)

