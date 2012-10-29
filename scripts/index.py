from sqlalchemy import create_engine, ForeignKey
from sqlalchemy import Column, DateTime, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship, backref, sessionmaker
from sqlalchemy.schema import UniqueConstraint
import sunpy
import sys
import os

engine = create_engine('sqlite:///spoca.db', echo=False)
Base = declarative_base()

class FitsFile(Base):
	__tablename__ = "fitsfiles"

	id = Column(Integer, primary_key=True)
	path = Column(String, unique=True)
	date = Column(DateTime)
	observatory = Column(String)
	instrument = Column(String)
	detector = Column(String)
	measurement = Column(String)
	extra = Column(String)
	
	UniqueConstraint(date, observatory, instrument, detector, measurement, extra)

	def __init__(self, path, extra=None):
		sunmap = sunpy.make_map(path)

		self.path = os.path.abspath(path)
		self.date = sunpy.time.parse_time(sunmap.get_header()["date-obs"])
		self.observatory = sunmap.observatory
		self.instrument = sunmap.instrument
		self.detector = sunmap.detector
		self.measurement = sunmap.measurement
		self.extra = extra

class Classifications(Base):
	__tablename__ = "classifications"

	id = Column(Integer, primary_key=True)
	date = Column(DateTime)
	parameters = Column(String)

	UniqueConstraint(date, parameters)

class ClassificationInputFiles(Base):
	__tablename__ = "classificationinputfiles"

	id = Column(Integer, primary_key=True)

	inputfile_id = Column(Integer, ForeignKey("fitsfiles.id"))
	classification_id = Column(Integer, ForeignKey("classifications.id"))

	UniqueConstraint(inputfile_id, classification_id)

class ClassificationResults(Base):
	__tablename__ = "classificationresults"

	id = Column(Integer, primary_key=True)
	classification_id = Column(Integer, ForeignKey("classifications.id"), unique=True)
	armap_path = Column(String, unique=True)
	chmap_path = Column(String, unique=True)
	segmentedmap_path = Column(String, unique=True)
	centers_path = Column(String, unique=True)
	eta_path = Column(String, unique=True)	

Base.metadata.create_all(engine)

def index_fitsfiles(fitsfiles, extra=None, progress=True):
	Session = sessionmaker(bind=engine)

	session = Session()

	for i in xrange(len(fitsfiles)):
		session.add(FitsFile(fitsfiles[i], extra))
		
		if progress:
			sys.stdout.write("Indexing %d FITS files... %6.2f%%\r" % (len(fitsfiles), (float(i+1)/float(len(fitsfiles))*100)))
			sys.stdout.flush()
	
	session.commit()



