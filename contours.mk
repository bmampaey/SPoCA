CC=g++

TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --cppflags --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
DFLAGS=
LFLAGS=-lcfitsio $(MAGICKLFLAGS)
CFLAGS=-Wall -fkeep-inline-functions -O3  $(MAGICKCFLAGS)

all:bin/contours.x
clean: rm bin/contours.x objects/contours.o objects/gradient.o objects/ArgumentHelper.o objects/ColorMap.o objects/SunImage.o objects/FitsHeader.o objects/Coordinate.o objects/Image.o objects/tools.o


bin/contours.x : contours.mk objects/contours.o objects/gradient.o objects/ArgumentHelper.o objects/ColorMap.o objects/SunImage.o objects/FitsHeader.o objects/Coordinate.o objects/Image.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/contours.o objects/gradient.o objects/ArgumentHelper.o objects/ColorMap.o objects/SunImage.o objects/FitsHeader.o objects/Coordinate.o objects/Image.o objects/tools.o $(LFLAGS) -o bin/contours.x

objects/contours.o : contours.mk utilities/contours.cpp classes/tools.h classes/constants.h classes/ColorMap.h classes/ArgumentHelper.h classes/gradient.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/contours.cpp -o objects/contours.o

objects/gradient.o : contours.mk classes/gradient.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/gradient.cpp -o objects/gradient.o

objects/ArgumentHelper.o : contours.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/ColorMap.o : contours.mk classes/ColorMap.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/SunImage.o : contours.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h classes/FitsHeader.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsHeader.o : contours.mk classes/FitsHeader.cpp classes/fitsio.h classes/longnam.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsHeader.cpp -o objects/FitsHeader.o

objects/Coordinate.o : contours.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Image.o : contours.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/tools.o : contours.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
