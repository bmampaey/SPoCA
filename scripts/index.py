import sunpy
import sqlite3
import sys
import os

def create_fits_index(database):
	conn = sqlite3.connect(database)
	c = conn.cursor()
	
	c.execute('''CREATE TABLE fitsfiles (obsdate text, obervatory text, instrument text, detector text, measurement text, dimensionx integer, dimensiony integer, path text)''')
	
	conn.commit()
	conn.close()

def index_fits(fitsfiles, database):
	conn = sqlite3.connect(database)
	c = conn.cursor()
	
	for i in xrange(len(fitsfiles)):
		sunmap = sunpy.make_map(fitsfiles[i])
		c.execute('''INSERT INTO fitsfiles VALUES (?, ?, ?, ?, ?, ?, ?, ?)''', ( \
			str(sunmap.date.strftime("%Y-%m-%d %H:%M:%S.%f")), \
			sunmap.observatory, \
			sunmap.instrument, \
			sunmap.detector, \
			sunmap.measurement, \
			sunmap.shape[0], \
			sunmap.shape[1], \
			os.path.abspath(fitsfiles[i]) \
		))

		sys.stdout.write("Indexing %d FITS files... %6.2f%%\r" % (len(fitsfiles), (float(i+1)/float(len(fitsfiles))*100)))
		sys.stdout.flush()

	conn.commit()
	conn.close()
