import computation
import sys
import pickle
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

if __name__ == '__main__':
	database = sys.argv[1]
	c = pickle.load(sys.stdin)
	engine = create_engine(database, echo=False)
	computation.Base.metadata.create_all(engine)
	Session = sessionmaker(bind=engine)
	c.run(Session())
