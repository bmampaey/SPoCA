CC=g++
CFLAGS=-Wall -fkeep-inline-functions -O3
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --cppflags --cxxflags --ldflags --libs`
MAGICKCFLAGS=-I /usr/include/ImageMagick/
DFLAGS=
LFLAGS=-lcfitsio

all:bin/visualisation_3D.x
clean: rm bin/visualisation_3D.x objects/visualisation_3D.o objects/trackable.o objects/gradient.o objects/Region.o objects/Coordinate.o objects/SunImage.o objects/FitsHeader.o objects/Image.o objects/ArgumentHelper.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/tools.o


bin/visualisation_3D.x : visualisation_3D.mk objects/visualisation_3D.o objects/trackable.o objects/gradient.o objects/Region.o objects/Coordinate.o objects/SunImage.o objects/FitsHeader.o objects/Image.o objects/ArgumentHelper.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/visualisation_3D.o objects/trackable.o objects/gradient.o objects/Region.o objects/Coordinate.o objects/SunImage.o objects/FitsHeader.o objects/Image.o objects/ArgumentHelper.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/tools.o $(LFLAGS) -o bin/visualisation_3D.x

objects/visualisation_3D.o : visualisation_3D.mk programs/visualisation_3D.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/ArgumentHelper.h classes/SunImage.h classes/Region.h classes/trackable.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/visualisation_3D.cpp -o objects/visualisation_3D.o

objects/trackable.o : visualisation_3D.mk classes/trackable.cpp classes/tools.h classes/constants.h classes/SunImage.h classes/Region.h classes/gradient.h cgt/graph.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/trackable.cpp -o objects/trackable.o

objects/gradient.o : visualisation_3D.mk classes/gradient.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/gradient.cpp -o objects/gradient.o

objects/Region.o : visualisation_3D.mk classes/Region.cpp classes/constants.h classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/Coordinate.o : visualisation_3D.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/SunImage.o : visualisation_3D.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h classes/FitsHeader.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsHeader.o : visualisation_3D.mk classes/FitsHeader.cpp classes/fitsio.h classes/longnam.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsHeader.cpp -o objects/FitsHeader.o

objects/Image.o : visualisation_3D.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/ArgumentHelper.o : visualisation_3D.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/mainutilities.o : visualisation_3D.mk classes/mainutilities.cpp classes/FeatureVector.h classes/SunImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/SWAPImage.o : visualisation_3D.mk classes/SWAPImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : visualisation_3D.mk classes/AIAImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : visualisation_3D.mk classes/EUVIImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : visualisation_3D.mk classes/EITImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/FeatureVector.o : visualisation_3D.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/tools.o : visualisation_3D.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
