CC=g++
CFLAGS=-Wall -fkeep-inline-functions -O3
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --cppflags --cxxflags --ldflags --libs`
MAGICKCFLAGS=-I /usr/include/ImageMagick/
DFLAGS=
LFLAGS=-lcfitsio

all:bin/ImageContour.x
clean: rm bin/ImageContour.x objects/ImageContour.o objects/ArgumentHelper.o objects/Image.o objects/Coordinate.o objects/tools.o


bin/ImageContour.x : ImageContour.mk objects/ImageContour.o objects/ArgumentHelper.o objects/Image.o objects/Coordinate.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/ImageContour.o objects/ArgumentHelper.o objects/Image.o objects/Coordinate.o objects/tools.o $(LFLAGS) -o bin/ImageContour.x

objects/ImageContour.o : ImageContour.mk utilities/ImageContour.cpp classes/tools.h classes/constants.h classes/Image.h classes/ArgumentHelper.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/ImageContour.cpp -o objects/ImageContour.o

objects/ArgumentHelper.o : ImageContour.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/Image.o : ImageContour.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/Coordinate.o : ImageContour.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/tools.o : ImageContour.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
