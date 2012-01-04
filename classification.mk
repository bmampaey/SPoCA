CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3
LFLAGS=-lcfitsio
DFLAGS=

all:bin/classification.x
clean: rm bin/classification.x objects/classification.o objects/FitsFile.o objects/Coordinate.o objects/Header.o objects/CoronalHole.o objects/Region.o objects/ActiveRegion.o objects/SegmentationStats.o objects/RegionStats.o objects/FeatureVector.o objects/HistogramPCMClassifier.o objects/HistogramPCM2Classifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/Image.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PFCMClassifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/EUVImage.o objects/SunImage.o objects/WCS.o objects/ColorMap.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o


bin/classification.x : classification.mk objects/classification.o objects/FitsFile.o objects/Coordinate.o objects/Header.o objects/CoronalHole.o objects/Region.o objects/ActiveRegion.o objects/SegmentationStats.o objects/RegionStats.o objects/FeatureVector.o objects/HistogramPCMClassifier.o objects/HistogramPCM2Classifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/Image.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PFCMClassifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/EUVImage.o objects/SunImage.o objects/WCS.o objects/ColorMap.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/classification.o objects/FitsFile.o objects/Coordinate.o objects/Header.o objects/CoronalHole.o objects/Region.o objects/ActiveRegion.o objects/SegmentationStats.o objects/RegionStats.o objects/FeatureVector.o objects/HistogramPCMClassifier.o objects/HistogramPCM2Classifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/Image.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PFCMClassifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/EUVImage.o objects/SunImage.o objects/WCS.o objects/ColorMap.o objects/ArgumentHelper.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o $(LFLAGS) -o bin/classification.x

objects/classification.o : classification.mk programs/classification.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/ArgumentHelper.h classes/ColorMap.h classes/EUVImage.h classes/Classifier.h classes/FCMClassifier.h classes/PCMClassifier.h classes/PFCMClassifier.h classes/PCM2Classifier.h classes/SPoCAClassifier.h classes/SPoCA2Classifier.h classes/HistogramFCMClassifier.h classes/HistogramPCM2Classifier.h classes/HistogramPCMClassifier.h classes/FeatureVector.h classes/RegionStats.h classes/SegmentationStats.h classes/ActiveRegion.h classes/CoronalHole.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/classification.cpp -o objects/classification.o

objects/FitsFile.o : classification.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Coordinate.o : classification.mk classes/Coordinate.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Header.o : classification.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/CoronalHole.o : classification.mk classes/CoronalHole.cpp classes/FeatureVector.h classes/EUVImage.h classes/ColorMap.h classes/Region.h classes/RegionStats.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CoronalHole.cpp -o objects/CoronalHole.o

objects/Region.o : classification.mk classes/Region.cpp classes/constants.h classes/tools.h classes/Coordinate.h classes/ColorMap.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/ActiveRegion.o : classification.mk classes/ActiveRegion.cpp classes/FeatureVector.h classes/EUVImage.h classes/ColorMap.h classes/Region.h classes/RegionStats.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ActiveRegion.cpp -o objects/ActiveRegion.o

objects/SegmentationStats.o : classification.mk classes/SegmentationStats.cpp classes/constants.h classes/tools.h classes/Coordinate.h classes/EUVImage.h classes/ColorMap.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SegmentationStats.cpp -o objects/SegmentationStats.o

objects/RegionStats.o : classification.mk classes/RegionStats.cpp classes/constants.h classes/tools.h classes/Coordinate.h classes/EUVImage.h classes/ColorMap.h classes/FitsFile.h classes/Region.h classes/SegmentationStats.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/RegionStats.cpp -o objects/RegionStats.o

objects/FeatureVector.o : classification.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/HistogramPCMClassifier.o : classification.mk classes/HistogramPCMClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/HistogramFCMClassifier.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramPCMClassifier.cpp -o objects/HistogramPCMClassifier.o

objects/HistogramPCM2Classifier.o : classification.mk classes/HistogramPCM2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/HistogramPCMClassifier.h classes/PCM2Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramPCM2Classifier.cpp -o objects/HistogramPCM2Classifier.o

objects/HistogramFCMClassifier.o : classification.mk classes/HistogramFCMClassifier.cpp classes/Image.h classes/EUVImage.h classes/HistogramFeatureVector.h classes/FeatureVector.h classes/FCMClassifier.h classes/HistogramClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramFCMClassifier.cpp -o objects/HistogramFCMClassifier.o

objects/HistogramClassifier.o : classification.mk classes/HistogramClassifier.cpp classes/EUVImage.h classes/HistogramFeatureVector.h classes/FeatureVector.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramClassifier.cpp -o objects/HistogramClassifier.o

objects/Image.o : classification.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/SPoCA2Classifier.o : classification.mk classes/SPoCA2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCM2Classifier.h classes/SPoCAClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCA2Classifier.cpp -o objects/SPoCA2Classifier.o

objects/SPoCAClassifier.o : classification.mk classes/SPoCAClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCAClassifier.cpp -o objects/SPoCAClassifier.o

objects/PCM2Classifier.o : classification.mk classes/PCM2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCM2Classifier.cpp -o objects/PCM2Classifier.o

objects/PFCMClassifier.o : classification.mk classes/PFCMClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PFCMClassifier.cpp -o objects/PFCMClassifier.o

objects/PCMClassifier.o : classification.mk classes/PCMClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/FCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCMClassifier.cpp -o objects/PCMClassifier.o

objects/FCMClassifier.o : classification.mk classes/FCMClassifier.cpp classes/Image.h classes/EUVImage.h classes/FeatureVector.h classes/Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FCMClassifier.cpp -o objects/FCMClassifier.o

objects/Classifier.o : classification.mk classes/Classifier.cpp classes/tools.h classes/constants.h classes/Image.h classes/EUVImage.h classes/ColorMap.h classes/FeatureVector.h classes/Region.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Classifier.cpp -o objects/Classifier.o

objects/EUVImage.o : classification.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/SunImage.o : classification.mk classes/SunImage.cpp classes/Image.h classes/WCS.h classes/Header.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/WCS.o : classification.mk classes/WCS.cpp classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/WCS.cpp -o objects/WCS.o

objects/ColorMap.o : classification.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/ArgumentHelper.o : classification.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/mainutilities.o : classification.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/HMIImage.o : classification.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : classification.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : classification.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : classification.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : classification.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/tools.o : classification.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
