CC=g++
CFLAGS=-Wall -fkeep-inline-functions -O3
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --cppflags --cxxflags --ldflags --libs`
MAGICKCFLAGS=-I /usr/include/ImageMagick/
DFLAGS=
LFLAGS=-lcfitsio

all:bin/get_regions_stats.x
clean: rm bin/get_regions_stats.x objects/get_regions_stats.o objects/FeatureVector.o objects/Coordinate.o objects/RegionStats.o objects/Region.o objects/ArgumentHelper.o objects/SunImage.o objects/FitsHeader.o objects/Image.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o


bin/get_regions_stats.x : get_regions_stats.mk objects/get_regions_stats.o objects/FeatureVector.o objects/Coordinate.o objects/RegionStats.o objects/Region.o objects/ArgumentHelper.o objects/SunImage.o objects/FitsHeader.o objects/Image.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/get_regions_stats.o objects/FeatureVector.o objects/Coordinate.o objects/RegionStats.o objects/Region.o objects/ArgumentHelper.o objects/SunImage.o objects/FitsHeader.o objects/Image.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o $(LFLAGS) -o bin/get_regions_stats.x

objects/get_regions_stats.o : get_regions_stats.mk programs/get_regions_stats.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/SunImage.h classes/ArgumentHelper.h classes/RegionStats.h classes/Coordinate.h classes/FeatureVector.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/get_regions_stats.cpp -o objects/get_regions_stats.o

objects/FeatureVector.o : get_regions_stats.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/Coordinate.o : get_regions_stats.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/RegionStats.o : get_regions_stats.mk classes/RegionStats.cpp classes/constants.h classes/Region.h classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/RegionStats.cpp -o objects/RegionStats.o

objects/Region.o : get_regions_stats.mk classes/Region.cpp classes/constants.h classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/ArgumentHelper.o : get_regions_stats.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/SunImage.o : get_regions_stats.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h classes/FitsHeader.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsHeader.o : get_regions_stats.mk classes/FitsHeader.cpp classes/fitsio.h classes/longnam.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsHeader.cpp -o objects/FitsHeader.o

objects/Image.o : get_regions_stats.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/mainutilities.o : get_regions_stats.mk classes/mainutilities.cpp classes/FeatureVector.h classes/SunImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/SWAPImage.o : get_regions_stats.mk classes/SWAPImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : get_regions_stats.mk classes/AIAImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : get_regions_stats.mk classes/EUVIImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : get_regions_stats.mk classes/EITImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/tools.o : get_regions_stats.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
