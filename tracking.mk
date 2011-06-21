CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3
LFLAGS=-lcfitsio  $(TRACKINGLFLAGS)
DFLAGS=

all:bin/tracking.x
clean: rm bin/tracking.x objects/tracking.o objects/Header.o objects/FitsFile.o objects/trackable.o objects/Region.o objects/Coordinate.o objects/ColorMap.o objects/SunImage.o objects/Image.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/tools.o


bin/tracking.x : tracking.mk objects/tracking.o objects/Header.o objects/FitsFile.o objects/trackable.o objects/Region.o objects/Coordinate.o objects/ColorMap.o objects/SunImage.o objects/Image.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/tracking.o objects/Header.o objects/FitsFile.o objects/trackable.o objects/Region.o objects/Coordinate.o objects/ColorMap.o objects/SunImage.o objects/Image.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/tools.o $(LFLAGS) -o bin/tracking.x

objects/tracking.o : tracking.mk programs/tracking.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/ArgumentHelper.h classes/ColorMap.h classes/Region.h classes/trackable.h classes/TrackingRelation.h cgt/graph.h classes/FitsFile.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/tracking.cpp -o objects/tracking.o

objects/Header.o : tracking.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/FitsFile.o : tracking.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/trackable.o : tracking.mk classes/trackable.cpp classes/tools.h classes/constants.h classes/ColorMap.h classes/Region.h classes/gradient.h cgt/graph.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/trackable.cpp -o objects/trackable.o

objects/Region.o : tracking.mk classes/Region.cpp classes/constants.h classes/Coordinate.h classes/ColorMap.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/Coordinate.o : tracking.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/ColorMap.o : tracking.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/SunImage.o : tracking.mk classes/SunImage.cpp classes/Image.h classes/Coordinate.h classes/Header.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/Image.o : tracking.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/ArgumentHelper.o : tracking.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/mainutilities.o : tracking.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/HMIImage.o : tracking.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : tracking.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : tracking.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : tracking.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : tracking.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/EUVImage.o : tracking.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/FeatureVector.o : tracking.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/tools.o : tracking.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
