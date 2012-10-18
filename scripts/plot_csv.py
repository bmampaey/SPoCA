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
	files = sorted(glob.glob(os.path.join(directory, "*CHMap*.csv")))

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

def plot_EUVI_filling_factor(directory_a, directory_b):
	plot_column(load_csv(directory_a).FillingFactor, fmt='r-', label="STEREO-A")
	plot_column(load_csv(directory_b).FillingFactor, fmt='b-', label="STEREO-B")
	plt.legend(loc='best')
	plt.show()
