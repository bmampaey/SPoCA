CC=g++
CFLAGS=-Wall -fkeep-inline-functions -g
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --cppflags --cxxflags --ldflags --libs`
MAGICKCFLAGS=-I /usr/include/ImageMagick/
DFLAGS=
LFLAGS=-lcfitsio

all:bin/image_derotate.x
clean: rm bin/image_derotate.x objects/image_derotate.o objects/mainutilities.o objects/ColorMap.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/SunImage.o objects/FitsHeader.o objects/Coordinate.o objects/Image.o objects/tools.o


bin/image_derotate.x : image_derotate.mk objects/image_derotate.o objects/mainutilities.o objects/ColorMap.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/SunImage.o objects/FitsHeader.o objects/Coordinate.o objects/Image.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/image_derotate.o objects/mainutilities.o objects/ColorMap.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/FeatureVector.o objects/ArgumentHelper.o objects/SunImage.o objects/FitsHeader.o objects/Coordinate.o objects/Image.o objects/tools.o $(LFLAGS) -o bin/image_derotate.x

objects/image_derotate.o : image_derotate.mk utilities/image_derotate.cpp classes/tools.h classes/constants.h classes/Image.h classes/SunImage.h classes/ArgumentHelper.h classes/mainutilities.h
	$(CC) -c $(CFLAGS) $(DFLAGS) utilities/image_derotate.cpp -o objects/image_derotate.o

objects/mainutilities.o : image_derotate.mk classes/mainutilities.cpp classes/FeatureVector.h classes/SunImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/ColorMap.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/ColorMap.o : image_derotate.mk classes/ColorMap.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/SWAPImage.o : image_derotate.mk classes/SWAPImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : image_derotate.mk classes/AIAImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : image_derotate.mk classes/EUVIImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : image_derotate.mk classes/EITImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/FeatureVector.o : image_derotate.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/ArgumentHelper.o : image_derotate.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/SunImage.o : image_derotate.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h classes/FitsHeader.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsHeader.o : image_derotate.mk classes/FitsHeader.cpp classes/fitsio.h classes/longnam.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsHeader.cpp -o objects/FitsHeader.o

objects/Coordinate.o : image_derotate.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Image.o : image_derotate.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/tools.o : image_derotate.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o