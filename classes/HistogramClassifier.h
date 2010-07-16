
#ifndef HistogramClassifier_H
#define HistogramClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <fstream>

#include "SunImage.h"
#include "HistogramFeatureVector.h"
#include "FeatureVector.h"

class HistogramClassifier
{
	protected :
		std::vector<HistoPixelFeature> HistoX;
		RealFeature binSize;
		unsigned numberBins;
		RealFeature histoChannels;

	protected :
		unsigned insert(const HistoPixelFeature& xj);

	public :
		// Constructors & Destructors
		HistogramClassifier();
		HistogramClassifier(const RealFeature& binSize);
		HistogramClassifier(const std::string& histogramFilename);
		
		// Function to initialise/save the Histogram
		void initHistogram(const std::string& histogramFilename, bool reset = true);
		void initBinSize(const RealFeature& binSize);
		void saveHistogram(const std::string& histogramFilename);
		
		
};
#endif
