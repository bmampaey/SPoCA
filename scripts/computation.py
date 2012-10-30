from sqlalchemy import create_engine, ForeignKey
from sqlalchemy import Table, Column, DateTime, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship, backref, sessionmaker
from sqlalchemy.schema import UniqueConstraint
import threading
import sys
import datetime
import time
Base = declarative_base()

class Computation(Base, threading.Thread):
	__tablename__ = 'computations'
	id = Column(Integer, primary_key=True)
	
	type = Column('type', String(50), nullable=False)
	__mapper_args__ = {'polymorphic_on': type}
	
	date_start = Column(DateTime, nullable=False)
	date_end = Column(DateTime, nullable=True)
	
	error = Column(String, nullable=True)
	
	def __init__(self, session, function, *args, **kwargs):
		Base.__init__(self)
		threading.Thread.__init__(self)
		self.session = session
		self.function = function
		self.args = args
		self.kwargs = kwargs
	
	def run(self):
		self.date_start = datetime.datetime.utcnow()
		self.session.add(self)
		self.session.commit()
		try:
			self.function(*self.args, **self.kwargs)
		except Exception as e:
			self.error = str(e)
		self.date_end = datetime.datetime.utcnow()
		self.session.add(self)
		self.session.commit()

class AdditionComputation(Computation):
	__tablename__ = 'testcomputations'
	__mapper_args__ = {'polymorphic_identity': 'addition'}
	
	id = Column(Integer, ForeignKey('computations.id'), primary_key=True)
	operand1 = Column(Integer, nullable=False)
	operand2 = Column(Integer, nullable=False)

	def __init__(self, session, operand1, operand2):
		self.operand1 = operand1
		self.operand2 = operand2
		
		def addition(operand1, operand2):
			time.sleep(20)
			return self.operand1 + self.operand2	

		Computation.__init__(self, session, addition, operand1, operand2)

		

if __name__ == '__main__':
	engine = create_engine(sys.argv[1], echo=False)
	Base.metadata.create_all(engine)
	Session = sessionmaker(bind=engine)
	
	c1 = AdditionComputation(Session(), 1, 1)
	c2 = AdditionComputation(Session(), 2, 2)
	c3 = AdditionComputation(Session(), 3, 3)
	
	c1.start()
	time.sleep(5)
	c2.start()
	time.sleep(5)
	c3.start()
	time.sleep(5)
	c1.join()
	c2.join()
	c3.join()




