CC=g++
CFLAGS=-Wall -fkeep-inline-functions -O3
LFLAGS=-l cfitsio
DFLAGS=-DDEBUG=0 -DNUMBERWAVELENGTH=2

all:bin/ImageHistogram.x
clean: rm bin/ImageHistogram.x objects/ImageHistogram.o objects/FeatureVector.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/ArgumentHelper.o objects/SunImage.o objects/Coordinate.o objects/Image.o objects/tools.o


bin/ImageHistogram.x : ImageHistogram.mk objects/ImageHistogram.o objects/FeatureVector.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/ArgumentHelper.o objects/SunImage.o objects/Coordinate.o objects/Image.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/ImageHistogram.o objects/FeatureVector.o objects/MainUtilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/ArgumentHelper.o objects/SunImage.o objects/Coordinate.o objects/Image.o objects/tools.o $(LFLAGS) -o bin/ImageHistogram.x

objects/ImageHistogram.o : ImageHistogram.mk programs/ImageHistogram.cpp classes/SunImage.h classes/ArgumentHelper.h classes/MainUtilities.h classes/HistogramFeatureVector.h classes/FeatureVector.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/ImageHistogram.cpp -o objects/ImageHistogram.o

objects/FeatureVector.o : ImageHistogram.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/MainUtilities.o : ImageHistogram.mk classes/MainUtilities.cpp classes/FeatureVector.h classes/SunImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/MainUtilities.cpp -o objects/MainUtilities.o

objects/SWAPImage.o : ImageHistogram.mk classes/SWAPImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : ImageHistogram.mk classes/AIAImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : ImageHistogram.mk classes/EUVIImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : ImageHistogram.mk classes/EITImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/ArgumentHelper.o : ImageHistogram.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/SunImage.o : ImageHistogram.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/Coordinate.o : ImageHistogram.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Image.o : ImageHistogram.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/tools.o : ImageHistogram.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
