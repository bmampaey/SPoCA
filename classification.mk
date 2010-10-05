CC=g++
CFLAGS=-Wall -fkeep-inline-functions -O3
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --cppflags --cxxflags --ldflags --libs`
MAGICKCFLAGS=-I /usr/include/ImageMagick/
DFLAGS=
LFLAGS=-lcfitsio

all:bin/classification.x
clean: rm bin/classification.x objects/classification.o objects/ActiveRegion.o objects/ArgumentHelper.o objects/FeatureVector.o objects/HistogramPCMClassifier.o objects/HistogramPCM2Classifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/Image.o objects/Coordinate.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/Region.o objects/SunImage.o objects/FitsHeader.o objects/ColorMap.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o


bin/classification.x : classification.mk objects/classification.o objects/ActiveRegion.o objects/ArgumentHelper.o objects/FeatureVector.o objects/HistogramPCMClassifier.o objects/HistogramPCM2Classifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/Image.o objects/Coordinate.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/Region.o objects/SunImage.o objects/FitsHeader.o objects/ColorMap.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/classification.o objects/ActiveRegion.o objects/ArgumentHelper.o objects/FeatureVector.o objects/HistogramPCMClassifier.o objects/HistogramPCM2Classifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/Image.o objects/Coordinate.o objects/SPoCA2Classifier.o objects/SPoCAClassifier.o objects/PCM2Classifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/Region.o objects/SunImage.o objects/FitsHeader.o objects/ColorMap.o objects/mainutilities.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o $(LFLAGS) -o bin/classification.x

objects/classification.o : classification.mk programs/classification.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/ColorMap.h classes/SunImage.h classes/Classifier.h classes/FCMClassifier.h classes/PCMClassifier.h classes/PCM2Classifier.h classes/SPoCAClassifier.h classes/SPoCA2Classifier.h classes/HistogramFCMClassifier.h classes/HistogramPCM2Classifier.h classes/HistogramPCMClassifier.h classes/FeatureVector.h classes/ArgumentHelper.h classes/ActiveRegion.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/classification.cpp -o objects/classification.o

objects/ActiveRegion.o : classification.mk classes/ActiveRegion.cpp classes/FeatureVector.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ActiveRegion.cpp -o objects/ActiveRegion.o

objects/ArgumentHelper.o : classification.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/FeatureVector.o : classification.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/HistogramPCMClassifier.o : classification.mk classes/HistogramPCMClassifier.cpp classes/SunImage.h classes/FeatureVector.h classes/HistogramFCMClassifier.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramPCMClassifier.cpp -o objects/HistogramPCMClassifier.o

objects/HistogramPCM2Classifier.o : classification.mk classes/HistogramPCM2Classifier.cpp classes/SunImage.h classes/FeatureVector.h classes/HistogramPCMClassifier.h classes/PCM2Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramPCM2Classifier.cpp -o objects/HistogramPCM2Classifier.o

objects/HistogramFCMClassifier.o : classification.mk classes/HistogramFCMClassifier.cpp classes/Image.h classes/SunImage.h classes/HistogramFeatureVector.h classes/FeatureVector.h classes/FCMClassifier.h classes/HistogramClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramFCMClassifier.cpp -o objects/HistogramFCMClassifier.o

objects/HistogramClassifier.o : classification.mk classes/HistogramClassifier.cpp classes/SunImage.h classes/HistogramFeatureVector.h classes/FeatureVector.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramClassifier.cpp -o objects/HistogramClassifier.o

objects/Image.o : classification.mk classes/Image.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/Coordinate.o : classification.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/SPoCA2Classifier.o : classification.mk classes/SPoCA2Classifier.cpp classes/SunImage.h classes/FeatureVector.h classes/PCM2Classifier.h classes/SPoCAClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCA2Classifier.cpp -o objects/SPoCA2Classifier.o

objects/SPoCAClassifier.o : classification.mk classes/SPoCAClassifier.cpp classes/SunImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCAClassifier.cpp -o objects/SPoCAClassifier.o

objects/PCM2Classifier.o : classification.mk classes/PCM2Classifier.cpp classes/SunImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCM2Classifier.cpp -o objects/PCM2Classifier.o

objects/PCMClassifier.o : classification.mk classes/PCMClassifier.cpp classes/SunImage.h classes/FeatureVector.h classes/FCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCMClassifier.cpp -o objects/PCMClassifier.o

objects/FCMClassifier.o : classification.mk classes/FCMClassifier.cpp classes/Image.h classes/SunImage.h classes/FeatureVector.h classes/Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FCMClassifier.cpp -o objects/FCMClassifier.o

objects/Classifier.o : classification.mk classes/Classifier.cpp classes/tools.h classes/constants.h classes/Image.h classes/SunImage.h classes/ColorMap.h classes/FeatureVector.h classes/Region.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Classifier.cpp -o objects/Classifier.o

objects/Region.o : classification.mk classes/Region.cpp classes/constants.h classes/Coordinate.h classes/ColorMap.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/SunImage.o : classification.mk classes/SunImage.cpp classes/fitsio.h classes/longnam.h classes/Image.h classes/Coordinate.h classes/FitsHeader.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/FitsHeader.o : classification.mk classes/FitsHeader.cpp classes/fitsio.h classes/longnam.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsHeader.cpp -o objects/FitsHeader.o

objects/ColorMap.o : classification.mk classes/ColorMap.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/mainutilities.o : classification.mk classes/mainutilities.cpp classes/FeatureVector.h classes/SunImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/ColorMap.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/SWAPImage.o : classification.mk classes/SWAPImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : classification.mk classes/AIAImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : classification.mk classes/EUVIImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : classification.mk classes/EITImage.cpp classes/fitsio.h classes/longnam.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/tools.o : classification.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
