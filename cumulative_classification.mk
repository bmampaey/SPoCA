CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3
LFLAGS=-lcfitsio
DFLAGS=

all:bin/cumulative_classification.x
clean: rm bin/cumulative_classification.x objects/cumulative_classification.o objects/ArgumentHelper.o objects/FeatureVector.o objects/CumulativeSPoCAClassifier.o objects/SPoCAClassifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/Coordinate.o objects/Region.o objects/FitsFile.o objects/Header.o objects/ColorMap.o objects/SunImage.o objects/Image.o objects/CumulativeSPoCA2Classifier.o objects/SPoCA2Classifier.o objects/PCM2Classifier.o objects/CumulativePCMClassifier.o objects/HistogramPCMClassifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/CumulativePCM2Classifier.o objects/HistogramPCM2Classifier.o objects/CumulativeFCMClassifier.o objects/CumulativeClassifier.o objects/EUVImage.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o


bin/cumulative_classification.x : cumulative_classification.mk objects/cumulative_classification.o objects/ArgumentHelper.o objects/FeatureVector.o objects/CumulativeSPoCAClassifier.o objects/SPoCAClassifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/Coordinate.o objects/Region.o objects/FitsFile.o objects/Header.o objects/ColorMap.o objects/SunImage.o objects/Image.o objects/CumulativeSPoCA2Classifier.o objects/SPoCA2Classifier.o objects/PCM2Classifier.o objects/CumulativePCMClassifier.o objects/HistogramPCMClassifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/CumulativePCM2Classifier.o objects/HistogramPCM2Classifier.o objects/CumulativeFCMClassifier.o objects/CumulativeClassifier.o objects/EUVImage.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/cumulative_classification.o objects/ArgumentHelper.o objects/FeatureVector.o objects/CumulativeSPoCAClassifier.o objects/SPoCAClassifier.o objects/PCMClassifier.o objects/FCMClassifier.o objects/Classifier.o objects/Coordinate.o objects/Region.o objects/FitsFile.o objects/Header.o objects/ColorMap.o objects/SunImage.o objects/Image.o objects/CumulativeSPoCA2Classifier.o objects/SPoCA2Classifier.o objects/PCM2Classifier.o objects/CumulativePCMClassifier.o objects/HistogramPCMClassifier.o objects/HistogramFCMClassifier.o objects/HistogramClassifier.o objects/CumulativePCM2Classifier.o objects/HistogramPCM2Classifier.o objects/CumulativeFCMClassifier.o objects/CumulativeClassifier.o objects/EUVImage.o objects/mainutilities.o objects/HMIImage.o objects/SWAPImage.o objects/AIAImage.o objects/EUVIImage.o objects/EITImage.o objects/tools.o $(LFLAGS) -o bin/cumulative_classification.x

objects/cumulative_classification.o : cumulative_classification.mk programs/cumulative_classification.cpp classes/tools.h classes/constants.h classes/mainutilities.h classes/EUVImage.h classes/CumulativeClassifier.h classes/CumulativeFCMClassifier.h classes/CumulativePCM2Classifier.h classes/CumulativePCMClassifier.h classes/CumulativeSPoCA2Classifier.h classes/CumulativeSPoCAClassifier.h classes/FeatureVector.h classes/ArgumentHelper.h
	$(CC) -c $(CFLAGS) $(DFLAGS) programs/cumulative_classification.cpp -o objects/cumulative_classification.o

objects/ArgumentHelper.o : cumulative_classification.mk classes/ArgumentHelper.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ArgumentHelper.cpp -o objects/ArgumentHelper.o

objects/FeatureVector.o : cumulative_classification.mk classes/FeatureVector.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FeatureVector.cpp -o objects/FeatureVector.o

objects/CumulativeSPoCAClassifier.o : cumulative_classification.mk classes/CumulativeSPoCAClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/CumulativeClassifier.h classes/SPoCAClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CumulativeSPoCAClassifier.cpp -o objects/CumulativeSPoCAClassifier.o

objects/SPoCAClassifier.o : cumulative_classification.mk classes/SPoCAClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCAClassifier.cpp -o objects/SPoCAClassifier.o

objects/PCMClassifier.o : cumulative_classification.mk classes/PCMClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/FCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCMClassifier.cpp -o objects/PCMClassifier.o

objects/FCMClassifier.o : cumulative_classification.mk classes/FCMClassifier.cpp classes/Image.h classes/EUVImage.h classes/FeatureVector.h classes/Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FCMClassifier.cpp -o objects/FCMClassifier.o

objects/Classifier.o : cumulative_classification.mk classes/Classifier.cpp classes/tools.h classes/constants.h classes/Image.h classes/EUVImage.h classes/ColorMap.h classes/FeatureVector.h classes/Region.h classes/Coordinate.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Classifier.cpp -o objects/Classifier.o

objects/Coordinate.o : cumulative_classification.mk classes/Coordinate.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Coordinate.cpp -o objects/Coordinate.o

objects/Region.o : cumulative_classification.mk classes/Region.cpp classes/constants.h classes/Coordinate.h classes/ColorMap.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Region.cpp -o objects/Region.o

objects/FitsFile.o : cumulative_classification.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Header.o : cumulative_classification.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/ColorMap.o : cumulative_classification.mk classes/ColorMap.cpp classes/Header.h classes/SunImage.h classes/gradient.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/ColorMap.cpp -o objects/ColorMap.o

objects/SunImage.o : cumulative_classification.mk classes/SunImage.cpp classes/Image.h classes/Coordinate.h classes/Header.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SunImage.cpp -o objects/SunImage.o

objects/Image.o : cumulative_classification.mk classes/Image.cpp classes/tools.h classes/constants.h classes/Coordinate.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Image.cpp -o objects/Image.o

objects/CumulativeSPoCA2Classifier.o : cumulative_classification.mk classes/CumulativeSPoCA2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/CumulativeClassifier.h classes/CumulativeSPoCAClassifier.h classes/SPoCA2Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CumulativeSPoCA2Classifier.cpp -o objects/CumulativeSPoCA2Classifier.o

objects/SPoCA2Classifier.o : cumulative_classification.mk classes/SPoCA2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCM2Classifier.h classes/SPoCAClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SPoCA2Classifier.cpp -o objects/SPoCA2Classifier.o

objects/PCM2Classifier.o : cumulative_classification.mk classes/PCM2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/PCM2Classifier.cpp -o objects/PCM2Classifier.o

objects/CumulativePCMClassifier.o : cumulative_classification.mk classes/CumulativePCMClassifier.cpp classes/EUVImage.h classes/HistogramFeatureVector.h classes/CumulativeFCMClassifier.h classes/HistogramPCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CumulativePCMClassifier.cpp -o objects/CumulativePCMClassifier.o

objects/HistogramPCMClassifier.o : cumulative_classification.mk classes/HistogramPCMClassifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/HistogramFCMClassifier.h classes/PCMClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramPCMClassifier.cpp -o objects/HistogramPCMClassifier.o

objects/HistogramFCMClassifier.o : cumulative_classification.mk classes/HistogramFCMClassifier.cpp classes/Image.h classes/EUVImage.h classes/HistogramFeatureVector.h classes/FeatureVector.h classes/FCMClassifier.h classes/HistogramClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramFCMClassifier.cpp -o objects/HistogramFCMClassifier.o

objects/HistogramClassifier.o : cumulative_classification.mk classes/HistogramClassifier.cpp classes/EUVImage.h classes/HistogramFeatureVector.h classes/FeatureVector.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramClassifier.cpp -o objects/HistogramClassifier.o

objects/CumulativePCM2Classifier.o : cumulative_classification.mk classes/CumulativePCM2Classifier.cpp classes/EUVImage.h classes/HistogramFeatureVector.h classes/CumulativePCMClassifier.h classes/HistogramPCM2Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CumulativePCM2Classifier.cpp -o objects/CumulativePCM2Classifier.o

objects/HistogramPCM2Classifier.o : cumulative_classification.mk classes/HistogramPCM2Classifier.cpp classes/EUVImage.h classes/FeatureVector.h classes/HistogramPCMClassifier.h classes/PCM2Classifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HistogramPCM2Classifier.cpp -o objects/HistogramPCM2Classifier.o

objects/CumulativeFCMClassifier.o : cumulative_classification.mk classes/CumulativeFCMClassifier.cpp classes/EUVImage.h classes/HistogramFeatureVector.h classes/HistogramFCMClassifier.h classes/CumulativeClassifier.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CumulativeFCMClassifier.cpp -o objects/CumulativeFCMClassifier.o

objects/CumulativeClassifier.o : cumulative_classification.mk classes/CumulativeClassifier.cpp classes/tools.h classes/constants.h classes/EUVImage.h classes/FeatureVector.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/CumulativeClassifier.cpp -o objects/CumulativeClassifier.o

objects/EUVImage.o : cumulative_classification.mk classes/EUVImage.cpp classes/Coordinate.h classes/SunImage.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVImage.cpp -o objects/EUVImage.o

objects/mainutilities.o : cumulative_classification.mk classes/mainutilities.cpp classes/FeatureVector.h classes/EUVImage.h classes/EITImage.h classes/EUVIImage.h classes/AIAImage.h classes/SWAPImage.h classes/HMIImage.h classes/ColorMap.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/mainutilities.cpp -o objects/mainutilities.o

objects/HMIImage.o : cumulative_classification.mk classes/HMIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/HMIImage.cpp -o objects/HMIImage.o

objects/SWAPImage.o : cumulative_classification.mk classes/SWAPImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/SWAPImage.cpp -o objects/SWAPImage.o

objects/AIAImage.o : cumulative_classification.mk classes/AIAImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/AIAImage.cpp -o objects/AIAImage.o

objects/EUVIImage.o : cumulative_classification.mk classes/EUVIImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EUVIImage.cpp -o objects/EUVIImage.o

objects/EITImage.o : cumulative_classification.mk classes/EITImage.cpp classes/EUVImage.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/EITImage.cpp -o objects/EITImage.o

objects/tools.o : cumulative_classification.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
