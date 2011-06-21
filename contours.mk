CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3 $(MAGICKCFLAGS) 
LFLAGS=-lcfitsio $(MAGICKLFLAGS) 
DFLAGS= -DMAGICK 

all:bin/contours.x
clean: rm bin/contours.x objects/contours.o objects/MagickImage.o objects/ColorMap.o objects/SunImage.o objects/FitsFile.o objects/Coordinate.o objects/Image.o objects/Header.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/tools.o


bin/contours.x : contours.mk objects/contours.o objects/MagickImage.o objects/ColorMap.o objects/SunImage.o objects/FitsFile.o objects/Coordinate.o objects/Image.o objects/Header.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/contours.o objects/MagickImage.o objects/ColorMap.o objects/SunImage.o objects/FitsFile.o objects/Coordinate.o objects/Image.o objects/Header.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/tools.o $(LFLAGS) -o bin/contours.x

objects/contours.o : contours.mk utilities/contours.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/ArgumentHelper.h classes/ColorMap.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/contours.cpp -o objects/contours.o

objects/MagickImage.o : contours.mk classes/MagickImage.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/MagickImage.cpp -o objects/MagickImage.o

objects/ColorMap.o : contours.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/SunImage.o : contours.mk classes/SunImage.cpp classes/Image.h classes/Coordinate.h classes/Header.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsFile.o : contours.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Coordinate.o : contours.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Image.o : contours.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/Header.o : contours.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/ArgumentHelper.o : contours.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/mainutilities.o : contours.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/HMIImage.o : contours.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : contours.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : contours.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : contours.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : contours.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/EUVImage.o : contours.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/FeatureVector.o : contours.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/tools.o : contours.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
