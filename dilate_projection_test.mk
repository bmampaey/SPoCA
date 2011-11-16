CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3
LFLAGS=-lcfitsio
DFLAGS=

all:bin/dilate_projection_test.x
clean: rm bin/dilate_projection_test.x objects/dilate_projection_test.o objects/Coordinate.o objects/Region.o objects/FitsFile.o objects/Header.o objects/ColorMap.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/SunImage.o objects/WCS.o objects/Image.o objects/tools.o


bin/dilate_projection_test.x : dilate_projection_test.mk objects/dilate_projection_test.o objects/Coordinate.o objects/Region.o objects/FitsFile.o objects/Header.o objects/ColorMap.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/SunImage.o objects/WCS.o objects/Image.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/dilate_projection_test.o objects/Coordinate.o objects/Region.o objects/FitsFile.o objects/Header.o objects/ColorMap.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/EUVImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/SunImage.o objects/WCS.o objects/Image.o objects/tools.o $(LFLAGS) -o bin/dilate_projection_test.x

objects/dilate_projection_test.o : dilate_projection_test.mk utilities/dilate_projection_test.cpp classes/tools.h classes/constants.h classes/Image.h classes/SunImage.h classes/ArgumentHelper.h classes/mainutilities.h classes/Region.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/dilate_projection_test.cpp -o objects/dilate_projection_test.o

objects/Coordinate.o : dilate_projection_test.mk classes/Coordinate.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Region.o : dilate_projection_test.mk classes/Region.cpp classes/constants.h classes/Coordinate.h classes/ColorMap.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/FitsFile.o : dilate_projection_test.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Header.o : dilate_projection_test.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/ColorMap.o : dilate_projection_test.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/mainutilities.o : dilate_projection_test.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/HMIImage.o : dilate_projection_test.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : dilate_projection_test.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : dilate_projection_test.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : dilate_projection_test.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : dilate_projection_test.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/EUVImage.o : dilate_projection_test.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/FeatureVector.o : dilate_projection_test.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/ArgumentHelper.o : dilate_projection_test.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/SunImage.o : dilate_projection_test.mk classes/SunImage.cpp classes/Image.h classes/WCS.h classes/Header.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/WCS.o : dilate_projection_test.mk classes/WCS.cpp classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/WCS.cpp -o objects/WCS.o

objects/Image.o : dilate_projection_test.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/tools.o : dilate_projection_test.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
