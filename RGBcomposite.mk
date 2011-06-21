CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3 $(MAGICKCFLAGS) 
LFLAGS=-lcfitsio $(MAGICKLFLAGS) 
DFLAGS= -DMAGICK 

all:bin/RGBcomposite.x
clean: rm bin/RGBcomposite.x objects/RGBcomposite.o objects/MagickImage.o objects/EUVImage.o objects/SunImage.o objects/FitsFile.o objects/Header.o objects/Image.o objects/Coordinate.o objects/ArgumentHelper.o objects/mainutilities.o objects/ColorMap.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/tools.o


bin/RGBcomposite.x : RGBcomposite.mk objects/RGBcomposite.o objects/MagickImage.o objects/EUVImage.o objects/SunImage.o objects/FitsFile.o objects/Header.o objects/Image.o objects/Coordinate.o objects/ArgumentHelper.o objects/mainutilities.o objects/ColorMap.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/RGBcomposite.o objects/MagickImage.o objects/EUVImage.o objects/SunImage.o objects/FitsFile.o objects/Header.o objects/Image.o objects/Coordinate.o objects/ArgumentHelper.o objects/mainutilities.o objects/ColorMap.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/tools.o $(LFLAGS) -o bin/RGBcomposite.x

objects/RGBcomposite.o : RGBcomposite.mk utilities/RGBcomposite.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/ArgumentHelper.h classes/EUVImage.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/RGBcomposite.cpp -o objects/RGBcomposite.o

objects/MagickImage.o : RGBcomposite.mk classes/MagickImage.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/MagickImage.cpp -o objects/MagickImage.o

objects/EUVImage.o : RGBcomposite.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/SunImage.o : RGBcomposite.mk classes/SunImage.cpp classes/Image.h classes/Coordinate.h classes/Header.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsFile.o : RGBcomposite.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Header.o : RGBcomposite.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/Image.o : RGBcomposite.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/Coordinate.o : RGBcomposite.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/ArgumentHelper.o : RGBcomposite.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/mainutilities.o : RGBcomposite.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/ColorMap.o : RGBcomposite.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h classes/MagickImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/HMIImage.o : RGBcomposite.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : RGBcomposite.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : RGBcomposite.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : RGBcomposite.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : RGBcomposite.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/FeatureVector.o : RGBcomposite.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/tools.o : RGBcomposite.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
