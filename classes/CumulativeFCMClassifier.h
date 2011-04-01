
#ifndef CumulativeFCMClassifier_H
#define CumulativeFCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "EUVImage.h"
#include "HistogramFeatureVector.h"
#include "HistogramFCMClassifier.h"
#include "CumulativeClassifier.h"

class CumulativeFCMClassifier : public virtual HistogramFCMClassifier, public CumulativeClassifier
{
	protected :
		using HistogramFCMClassifier::computeB;
		using HistogramFCMClassifier::computeU;
		
		//Asses & Merge functions for the sursegmentation
		using HistogramFCMClassifier::assess;
		using HistogramFCMClassifier::merge;
	public :
		CumulativeFCMClassifier(Real fuzzifier = 2.);
		CumulativeFCMClassifier(const RealFeature& binSize, Real fuzzifier = 2.);
		CumulativeFCMClassifier(const std::string& histogramFilename, Real fuzzifier = 2.);
		void addImages(std::vector<EUVImage*> images);	

		//Classification functions
		using HistogramFCMClassifier::classification;
		using HistogramFCMClassifier::attribution;
		
		using HistogramFCMClassifier::computeJ;
		
		//Function to initialise the centers
		using HistogramFCMClassifier::initB;
		using HistogramFCMClassifier::randomInitB;
		

};
#endif
