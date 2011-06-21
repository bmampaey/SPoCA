CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3
LFLAGS=-lcfitsio
DFLAGS=

all:bin/preprocessing.x
clean: rm bin/preprocessing.x objects/preprocessing.o objects/mainutilities.o objects/Header.o objects/ColorMap.o objects/SunImage.o objects/FitsFile.o objects/Image.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/EUVImage.o objects/Coordinate.o objects/tools.o


bin/preprocessing.x : preprocessing.mk objects/preprocessing.o objects/mainutilities.o objects/Header.o objects/ColorMap.o objects/SunImage.o objects/FitsFile.o objects/Image.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/EUVImage.o objects/Coordinate.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/preprocessing.o objects/mainutilities.o objects/Header.o objects/ColorMap.o objects/SunImage.o objects/FitsFile.o objects/Image.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/EUVImage.o objects/Coordinate.o objects/tools.o $(LFLAGS) -o bin/preprocessing.x

objects/preprocessing.o : preprocessing.mk utilities/preprocessing.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/EUVImage.h classes/ArgumentHelper.h classes/mainutilities.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/preprocessing.cpp -o objects/preprocessing.o

objects/mainutilities.o : preprocessing.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/Header.o : preprocessing.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/ColorMap.o : preprocessing.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/SunImage.o : preprocessing.mk classes/SunImage.cpp classes/Image.h classes/Coordinate.h classes/Header.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsFile.o : preprocessing.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Image.o : preprocessing.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/HMIImage.o : preprocessing.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : preprocessing.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : preprocessing.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : preprocessing.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : preprocessing.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/FeatureVector.o : preprocessing.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/ArgumentHelper.o : preprocessing.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/EUVImage.o : preprocessing.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/Coordinate.o : preprocessing.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/tools.o : preprocessing.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
