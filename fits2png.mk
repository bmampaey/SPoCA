CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3 $(MAGICKCFLAGS) 
LFLAGS=-lcfitsio $(MAGICKLFLAGS) 
DFLAGS= -DMAGICK 

all:bin/fits2png.x
clean: rm bin/fits2png.x objects/fits2png.o objects/ArgumentHelper.o objects/MagickImage.o objects/EUVImage.o objects/SunImage.o objects/FitsFile.o objects/Header.o objects/Image.o objects/Coordinate.o objects/ColorMap.o objects/gradient.o objects/tools.o


bin/fits2png.x : fits2png.mk objects/fits2png.o objects/ArgumentHelper.o objects/MagickImage.o objects/EUVImage.o objects/SunImage.o objects/FitsFile.o objects/Header.o objects/Image.o objects/Coordinate.o objects/ColorMap.o objects/gradient.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/fits2png.o objects/ArgumentHelper.o objects/MagickImage.o objects/EUVImage.o objects/SunImage.o objects/FitsFile.o objects/Header.o objects/Image.o objects/Coordinate.o objects/ColorMap.o objects/gradient.o objects/tools.o $(LFLAGS) -o bin/fits2png.x

objects/fits2png.o : fits2png.mk utilities/fits2png.cpp classes/tools.h classes/constants.h classes/ColorMap.h classes/EUVImage.h classes/MagickImage.h classes/ArgumentHelper.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/fits2png.cpp -o objects/fits2png.o

objects/ArgumentHelper.o : fits2png.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/MagickImage.o : fits2png.mk classes/MagickImage.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/MagickImage.cpp -o objects/MagickImage.o

objects/EUVImage.o : fits2png.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/SunImage.o : fits2png.mk classes/SunImage.cpp classes/Image.h classes/Coordinate.h classes/Header.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsFile.o : fits2png.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Header.o : fits2png.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/Image.o : fits2png.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/Coordinate.o : fits2png.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/ColorMap.o : fits2png.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/gradient.o : fits2png.mk classes/gradient.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/gradient.cpp -o objects/gradient.o

objects/tools.o : fits2png.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
