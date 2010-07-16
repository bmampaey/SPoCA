CC=g++
CFLAGS=-Wall -fkeep-inline-functions -O3
LFLAGS=-lcfitsio
TRACKINGFLAGS=-lpthread
IDLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
DFLAGS=

all:bin/tracking.x
clean: rm bin/tracking.x objects/tracking.o objects/CoordinateConvertor.o objects/Coordinate.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/Region.o objects/SunImage.o objects/Image.o objects/tools.o


bin/tracking.x : tracking.mk objects/tracking.o objects/CoordinateConvertor.o objects/Coordinate.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/Region.o objects/SunImage.o objects/Image.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/tracking.o objects/CoordinateConvertor.o objects/Coordinate.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/Region.o objects/SunImage.o objects/Image.o objects/tools.o $(LFLAGS) -o bin/tracking.x

objects/tracking.o : tracking.mk programs/tracking.cpp classes/tools.h classes/constants.h classes/SunImage.h classes/Region.h classes/gradient.h classes/ArgumentHelper.h classes/MainUtilities.h classes/CoordinateConvertor.h cgt/graph.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/tracking.cpp -o objects/tracking.o

objects/CoordinateConvertor.o : tracking.mk classes/CoordinateConvertor.cpp classes/idl_export.h classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CoordinateConvertor.cpp -o objects/CoordinateConvertor.o

objects/Coordinate.o : tracking.mk classes/Coordinate.cpp classes/idl_export.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/MainUtilities.o : tracking.mk classes/MainUtilities.cpp classes/FeatureVector.h classes/SunImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/MainUtilities.cpp -o objects/MainUtilities.o

objects/SWAPImage.o : tracking.mk classes/SWAPImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : tracking.mk classes/AIAImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : tracking.mk classes/EUVIImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : tracking.mk classes/EITImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/FeatureVector.o : tracking.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/ArgumentHelper.o : tracking.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/Region.o : tracking.mk classes/Region.cpp classes/constants.h classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/SunImage.o : tracking.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/Image.o : tracking.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/tools.o : tracking.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
