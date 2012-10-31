import subprocess
import tempfile
import os
import shutil
import glob
import sys
from sqlalchemy import create_engine, ForeignKey
from sqlalchemy import Table, Column, DateTime, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship, backref, sessionmaker
from sqlalchemy.schema import UniqueConstraint
import sunpy
import datetime

root = "/pool/rdv/git/spoca"

Base = declarative_base()
Session = None

def setdb(db):
	global Session
	
	engine = create_engine(db, echo=False)
	Base.metadata.create_all(engine)
	Session = sessionmaker(bind=engine)

class SPoCAError(Exception):
	pass
class ClassificationError(SPoCAError):
	pass
class OverlayError(SPoCAError):
	pass

class Computation(Base):
	__tablename__ = 'computations'
	id = Column(Integer, primary_key=True)
	
	type = Column(String(50), nullable=False)
	__mapper_args__ = {'polymorphic_on': type}
	
	date_start = Column(DateTime, nullable=False)
	date_end = Column(DateTime, nullable=True)
	
	error = Column(String, nullable=True)
	
	def update(self, session):
		session.add(self)
		session.commit()

	def run(self, session=None):
		if session is None:
			if Session is not None:
				session = Session()
		
		self.date_start = datetime.datetime.utcnow()
		if session is not None:
			self.update(session)

		#try:
		self.compute(session)
		#except Exception as e:
		#	self.error = str(e)

		self.date_end = datetime.datetime.utcnow()
		if session is not None:
			self.update(session)
	
	def compute(self, session):
		pass

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

def index(fitsfiles, extra=None, progress=False, session=None):
	if session is None:
		session = Session()
	
	fitsfiles = map(os.path.abspath, fitsfiles)
	
	fitsfile_objs = []

	for i in xrange(len(fitsfiles)):
		queryresults = session.query(FitsFile).filter(FitsFile.path == fitsfiles[i]).all()

		if len(queryresults) == 0:
			fitsfile_obj = FitsFile(fitsfiles[i], extra)
			fitsfile_objs.append(fitsfile_obj)
			session.add(fitsfile_obj)
		else:
			fitsfile_objs.extend(queryresults)
		
		if progress:
			sys.stdout.write("Indexing %d FITS files... %6.2f%%\r" % (len(fitsfiles), (float(i+1)/float(len(fitsfiles))*100)))
			sys.stdout.flush()
	
	session.commit()
	return fitsfile_objs

classification_parameters = {
	# parameter name: (flag name, to string conversion function, default value)
	"centers_input_file": ("B", os.path.abspath, None),
	"number_classes": ("C", str, 4),
	"intensities_stats_preprocessing": ("G", ",".join, ["NAR"]),
	"histogram_input_file": ("H", os.path.abspath, None),
	"image_type": ("I", str, None),
	"max_limits_file": ("L", os.path.abspath, None),
	"maps": ("M", ",".join, ["A","C","S"]),
	"neighbourhood_radius": ("N", str, None),
	"output_directory": ("O", os.path.abspath, "."),
	"preprocessing": ("P", ",".join, ["NAR"]),
	"intensities_stats_radiusratio": ("R", str, 0.95),
	"segmentation": ("S", str, "max"),
	"classifier": ("T", str, "HFCM"),
	"chaincode_max_deviation": ("X", str, 0),
	"classes_ar": ("a", lambda l: ",".join(map(str, l)), None),
	"classes_ch": ("c", lambda l: ",".join(map(str, l)), None),
	"classes_qs": ("q", lambda l: ",".join(map(str, l)), None),
	"fuzzifier": ("f", str, 2),
	"max_iterations": ("i", str, 100),
	"precision": ("p", str, 0.0015),
	"radiusratio": ("r", str, 1.31),
	"thresholds": ("t", lambda l: ",".join(map(str, l)), None),
	"uncompressed": ("u", None, None),
	"chaincode_max_points": ("x", str, 0),
	"bin_size": ("z", lambda l: ",".join(map(str, l)), [0.01,0.01])
}

class Classification(Computation):
	__tablename__ = 'classifications'
	__mapper_args__ = {'polymorphic_identity': 'classification'}

	id = Column(Integer, ForeignKey('computations.id'), primary_key=True)
	ar_map = Column(String, nullable=True)
	ch_map = Column(String, nullable=True)
	segmented_map = Column(String, nullable=True)
	centers_output_file = Column(String, nullable=True)
	eta_output_file = Column(String, nullable=True)

	classifications_fitsfiles = Table('classifications_inputfiles', Base.metadata,
		Column('classification_id', Integer, ForeignKey('classifications.id')),
		Column('fitsfile_id', Integer, ForeignKey('fitsfiles.id'))
	)

	fitsfiles = relationship("FitsFile", secondary=classifications_fitsfiles, backref='classifications')

	def __init__(self, *fitsfiles, **parameters):
		Computation.__init__(self)

		self.compute_fitsfiles = fitsfiles
		self.compute_parameters = parameters
		
		for (k, v) in convert_to_string(classification_parameters, parameters).iteritems():
			vars(self)[k] = v
		
		
	def compute(self, session):
		if session is not None:
			self.fitsfiles = index(self.compute_fitsfiles, session=session)
			self.update(session)	

		results = classification(*self.compute_fitsfiles, **self.compute_parameters)
		
		for result in results:
			if result.endswith('.ARMap.fits'):
				self.ar_map = result
			elif result.endswith('.CHMap.fits'):
				self.ch_map = result
			elif result.endswith('.SegmentedMap.fits'):
				self.segmented_map = result
			elif result.endswith('.centers.txt'):
				self.centers_output_file = result
			elif result.endswith('.eta.txt'):
				self.eta_output_file = result
			else:
				raise ClassificationError('Unknown output file: \"%s\"' % result)

for (k, v) in classification_parameters.iteritems():
	setattr(Classification, k, Column(k, String, nullable=(v[2] is None)))

def convert_to_string(table, parameters):
	parameters = dict(parameters)

	for (k, (flag, function, default)) in table.iteritems():
		if k not in parameters.keys() and default is not None:
			parameters[k] = default
		if k in parameters.keys() and function is not None:
			parameters[k] = function(parameters[k])
	
	return parameters


def build_arguments(table, bin, parameters, fitsfiles):
	arguments = [bin]
	
	parameters = convert_to_string(table, parameters)
	
	for (k, v) in parameters.items():
		(flag, function, default) = table[k]
		arguments.append("-"+flag)
		if function is not None and v is not None:
			if type(v) != str:
				v = function(v)
			arguments.append(v)
	
	arguments.extend(fitsfiles)

	return arguments


def classification(*fitsfiles, **parameters):
	if(len(fitsfiles) > 2):
		raise ClassificationError("Number of channels greater than 2 is not supported.")
	elif len(fitsfiles) == 2:
		bin = os.path.join(root, "bin2/classification.x")
	else:
		bin = os.path.join(root, "bin1/classification.x")
	
	if "output_directory" in parameters.keys():
		output_directory = parameters["output_directory"]
	else:
		output_directory = os.path.join(root, "results")
	
	parameters["output_directory"] = tempfile.mkdtemp()
	
	arguments = build_arguments(classification_parameters, bin, parameters, fitsfiles)
	print arguments
	try:
		p = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(out, err) = p.communicate()
		print out
		
		if err != "":
			raise ClassificationError(err)
		
		result_files = glob.glob(os.path.join(parameters["output_directory"], "*"))
		for result_file in result_files:
			shutil.copy(result_file, output_directory)
	finally:
		shutil.rmtree(parameters["output_directory"])
	
	return map(lambda f: os.path.join(output_directory, os.path.basename(f)), result_files)

overlay_parameters = {
	"colors_file": ("C", os.path.abspath),
	"label": ("L", str),
	"map": ("M", os.path.abspath),
	"output_directory": ("O", os.path.abspath),
	"preprocessing": ("P", ",".join),
	"size": ("S", lambda (width, height): str(width)+"x"+str(height)),
	"colors": ("c", lambda l: ",".join(map(str, l))),
	"external": ("e", None),
	"internal": ("i", None),
	"label": ("l", None),
	"mastic": ("m", None),
	"width": ("w", str)
}

class Overlay(Computation):
	__tablename__ = 'overlays'
	__mapper_args__ = {'polymorphic_identity': 'overlay'}

	id = Column(Integer, ForeignKey('computations.id'), primary_key=True)
	map = Column(String, nullable=False)
	fitsfile_id = Column(Integer, ForeignKey('fitsfiles.id'))
	fitsfile = relationship('FitsFile', backref='overlays')
	image = Column(String, nullable=True)

	def __init__(self, fitsfile, **parameters):
		Computation.__init__(self)

		self.compute_fitsfile = fitsfile
		self.compute_parameters = parameters
		
		for (k, v) in convert_to_string(classification_parameters, parameters).iteritems():
			vars(self)[k] = v
		
		
	def compute(self, session):
		if session is not None:
			self.fitsfile = index([self.compute_fitsfile], session=session)[0]
			self.update(session)	

		results = overlay(self.compute_fitsfile, **self.compute_parameters)
		
		for result in results:
			if result.endswith('.png'):
				self.image = result
			else:
				raise OverlayError('Unknown output file: \"%s\"' % result)

def overlay(*fitsfiles, **parameters):
	root = os.path.abspath(".")
	
	bin = os.path.join(root, "bin1/overlay.x")
	
	if "output_directory" in parameters.keys():
		output_directory = parameters["output_directory"]
	else:
		output_directory = os.path.join(root, "results")
	
	parameters["output_directory"] = tempfile.mkdtemp()
	
	arguments = build_arguments(overlay_parameters, bin, parameters, fitsfiles)
	print arguments
	try:
		p = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(out, err) = p.communicate()
		print out
		
		if err != "":
			raise OverlayError(err)
		
		result_files = glob.glob(os.path.join(parameters["output_directory"], "*"))
		for result_file in result_files:
			shutil.copy(result_file, output_directory)
	finally:
		shutil.rmtree(parameters["output_directory"])
	
	return map(lambda f: os.path.join(output_directory, os.path.basename(f)), result_files)



if __name__ == '__main__':
	setdb(sys.argv[1])

