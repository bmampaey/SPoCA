CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3
LFLAGS=-lcfitsio
DFLAGS=

all:bin/attribution.x
clean: rm bin/attribution.x objects/attribution.o objects/FitsFile.o objects/Coordinate.o objects/Header.o objects/CoronalHole.o objects/Region.o objects/ActiveRegion.o objects/SegmentationStats.o objects/RegionStats.o objects/FeatureVector.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Image.o objects/Classifier.o objects/ColorMap.o objects/SunImage.o objects/WCS.o objects/EUVImage.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o


bin/attribution.x : attribution.mk objects/attribution.o objects/FitsFile.o objects/Coordinate.o objects/Header.o objects/CoronalHole.o objects/Region.o objects/ActiveRegion.o objects/SegmentationStats.o objects/RegionStats.o objects/FeatureVector.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Image.o objects/Classifier.o objects/ColorMap.o objects/SunImage.o objects/WCS.o objects/EUVImage.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/attribution.o objects/FitsFile.o objects/Coordinate.o objects/Header.o objects/CoronalHole.o objects/Region.o objects/ActiveRegion.o objects/SegmentationStats.o objects/RegionStats.o objects/FeatureVector.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Image.o objects/Classifier.o objects/ColorMap.o objects/SunImage.o objects/WCS.o objects/EUVImage.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o $(LFLAGS) -o bin/attribution.x

objects/attribution.o : attribution.mk programs/attribution.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/ArgumentHelper.h classes/EUVImage.h classes/ColorMap.h classes/Classifier.h classes/FCMClassifier.h classes/PCMClassifier.h classes/PCM2Classifier.h classes/SPoCAClassifier.h classes/SPoCA2Classifier.h classes/FeatureVector.h classes/RegionStats.h classes/SegmentationStats.h classes/ActiveRegion.h classes/CoronalHole.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/attribution.cpp -o objects/attribution.o

objects/FitsFile.o : attribution.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Coordinate.o : attribution.mk classes/Coordinate.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Header.o : attribution.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/CoronalHole.o : attribution.mk classes/CoronalHole.cpp classes/FeatureVector.h classes/EUVImage.h classes/ColorMap.h classes/Region.h classes/RegionStats.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CoronalHole.cpp -o objects/CoronalHole.o

objects/Region.o : attribution.mk classes/Region.cpp classes/constants.h classes/tools.h classes/Coordinate.h classes/ColorMap.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/ActiveRegion.o : attribution.mk classes/ActiveRegion.cpp classes/FeatureVector.h classes/EUVImage.h classes/ColorMap.h classes/Region.h classes/RegionStats.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ActiveRegion.cpp -o objects/ActiveRegion.o

objects/SegmentationStats.o : attribution.mk classes/SegmentationStats.cpp classes/constants.h classes/tools.h classes/Coordinate.h classes/EUVImage.h classes/ColorMap.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SegmentationStats.cpp -o objects/SegmentationStats.o

objects/RegionStats.o : attribution.mk classes/RegionStats.cpp classes/constants.h classes/tools.h classes/Coordinate.h classes/EUVImage.h classes/ColorMap.h classes/FitsFile.h classes/Region.h classes/SegmentationStats.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/RegionStats.cpp -o objects/RegionStats.o

objects/FeatureVector.o : attribution.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/SPoCA2Classifier.o : attribution.mk classes/SPoCA2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCM2Classifier.h classes/SPoCAClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCA2Classifier.cpp -o objects/SPoCA2Classifier.o

objects/SPoCAClassifier.o : attribution.mk classes/SPoCAClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCAClassifier.cpp -o objects/SPoCAClassifier.o

objects/PCM2Classifier.o : attribution.mk classes/PCM2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCM2Classifier.cpp -o objects/PCM2Classifier.o

objects/PCMClassifier.o : attribution.mk classes/PCMClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/FCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCMClassifier.cpp -o objects/PCMClassifier.o

objects/FCMClassifier.o : attribution.mk classes/FCMClassifier.cpp classes/Image.h classes/EUVImage.h classes/FeatureVector.h classes/Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FCMClassifier.cpp -o objects/FCMClassifier.o

objects/Image.o : attribution.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/Classifier.o : attribution.mk classes/Classifier.cpp classes/tools.h classes/constants.h classes/Image.h classes/EUVImage.h classes/ColorMap.h classes/FeatureVector.h classes/Region.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Classifier.cpp -o objects/Classifier.o

objects/ColorMap.o : attribution.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/SunImage.o : attribution.mk classes/SunImage.cpp classes/Image.h classes/WCS.h classes/Header.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/WCS.o : attribution.mk classes/WCS.cpp classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/WCS.cpp -o objects/WCS.o

objects/EUVImage.o : attribution.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/ArgumentHelper.o : attribution.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/mainutilities.o : attribution.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/HMIImage.o : attribution.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : attribution.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : attribution.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : attribution.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : attribution.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/tools.o : attribution.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
