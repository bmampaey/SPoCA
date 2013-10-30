#!/usr/bin/python
import pandas
import argparse
import re

class_center_regex = re.compile("class_center_(?P<class_number>\d+)_(?P<channel>\w+)")

# Start point of the script
if __name__ == "__main__":
	# Get the arguments
	parser = argparse.ArgumentParser(description='Compute the median of the class centers and the etas, and write files for SPoCA.')
	parser.add_argument('filename', help='A csv file as generetad by the script get_class_centers_and_etas.py')
	args = parser.parse_args()
	
	# We look into the header of the file what are the time_index, the channels and the number of classes
	channels = list()
	number_classes = 0
	time_index = "time"
	with open(args.filename) as csv_file:
		headers = csv_file.readline().split(",")
		time_index = headers[0]
		for header in headers[1:]:
			parts = class_center_regex.match(header)
			if parts:
				class_number, channel = parts.group("class_number", "channel")
				number_classes = int(class_number) if int(class_number) > number_classes else number_classes
				if channel not in channels: channels.append(channel)
	number_classes += 1
	
	# Parse the csv file
	csv = pandas.read_csv(args.filename, parse_dates = time_index, index_col = time_index)
	
	median_centers = list()
	for header in csv.columns[0:number_classes*len(channels)]:
		median_centers.append(csv[header].median())
	
	with open("median_centers.txt", 'w') as outfile:
		outfile.write("[" + ",".join(channels) + "]\t")
		outfile.write(str([tuple(median_centers[i * len(channels):(i+1) * len(channels)]) for i in range(number_classes)]))
	
	median_etas = list()
	for header in csv.columns[number_classes*len(channels):]:
		median_etas.append(csv[header].median())
	with open("median_etas.txt", 'w') as outfile:
		outfile.write(str(median_etas))

