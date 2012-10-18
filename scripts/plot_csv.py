import pandas
import matplotlib.pyplot as plt
import matplotlib.dates
import numpy as np
from datetime import datetime
import os
import glob
import sys

OBSERVATIONDATE_FORMAT = "%Y-%m-%dT%H:%M:%S"

def load_csv(directory):
	files = glob.glob(os.path.join(directory, "*CHMap*.csv"))

	frame = pandas.read_csv(files[0])
	for i in xrange(1, len(files)):
		sys.stdout.write("Reading CSV files in \"%s\"... %3d%%\r" % (directory, (i/float(len(files))*100)))
		sys.stdout.flush()
		frame = frame.append(pandas.read_csv(files[i]))
	sys.stdout.write("Reading CSV files in \"%s\"... %3d%%\r\n" % (directory, 100))
	
	frame.ObservationDate = frame.ObservationDate.map(lambda x: datetime.strptime(x, OBSERVATIONDATE_FORMAT))

	frame = frame.groupby('ObservationDate').mean()

	return frame

def plot_column(column, *args, **kwargs):
	plt.plot_date(column.dropna().index.map(lambda x: matplotlib.dates.date2num(x.to_pydatetime())), column.dropna(), *args, **kwargs)

def plot_EUVI_statistic(frame_a, frame_b, statistic):
	if type(frame_a) == str:
		frame_a = load_csv(frame_a)
	if type(frame_b) == str:
		frame_b = load_csv(frame_b)
	
	plot_column(frame_a[statistic], fmt='r-', label="STEREO-A")
	plot_column(frame_b[statistic], fmt='b-', label="STEREO-B")
	plt.xlabel("Observation date")
	plt.ylabel(statistic)
	plt.legend(loc='best')
	plt.show()

	return (frame_a, frame_b)
