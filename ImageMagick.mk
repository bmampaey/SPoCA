CC=g++
CFLAGS=-Wall -fkeep-inline-functions -O3
LFLAGS=-lcfitsio
TRACKINGFLAGS=-lpthread
IDLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
DFLAGS=

all:bin/ImageMagick.x
clean: rm bin/ImageMagick.x objects/ImageMagick.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/SunImage.o objects/Coordinate.o objects/FeatureVector.o objects/ArgumentHelper.o objects/Image.o objects/tools.o


bin/ImageMagick.x : ImageMagick.mk objects/ImageMagick.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/SunImage.o objects/Coordinate.o objects/FeatureVector.o objects/ArgumentHelper.o objects/Image.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/ImageMagick.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/SunImage.o objects/Coordinate.o objects/FeatureVector.o objects/ArgumentHelper.o objects/Image.o objects/tools.o $(LFLAGS) -o bin/ImageMagick.x

objects/ImageMagick.o : ImageMagick.mk programs/ImageMagick.cpp classes/tools.h classes/constants.h classes/Image.h classes/gradient.h classes/ArgumentHelper.h classes/MainUtilities.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/ImageMagick.cpp -o objects/ImageMagick.o

objects/MainUtilities.o : ImageMagick.mk classes/MainUtilities.cpp classes/FeatureVector.h classes/SunImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/MainUtilities.cpp -o objects/MainUtilities.o

objects/SWAPImage.o : ImageMagick.mk classes/SWAPImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : ImageMagick.mk classes/AIAImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : ImageMagick.mk classes/EUVIImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : ImageMagick.mk classes/EITImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/SunImage.o : ImageMagick.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/Coordinate.o : ImageMagick.mk classes/Coordinate.cpp classes/idl_export.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/FeatureVector.o : ImageMagick.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/ArgumentHelper.o : ImageMagick.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/Image.o : ImageMagick.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/tools.o : ImageMagick.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
