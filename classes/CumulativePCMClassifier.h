
#ifndef CumulativePCMClassifier_H
#define CumulativePCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "SunImage.h"
#include "HistogramFeatureVector.h"
#include "CumulativeFCMClassifier.h"
#include "HistogramPCMClassifier.h"

class CumulativePCMClassifier : public virtual HistogramPCMClassifier, public CumulativeFCMClassifier
{
	protected :
		using HistogramPCMClassifier::computeB;
		using HistogramPCMClassifier::computeU;
		using HistogramPCMClassifier::computeEta;

		//Asses & Merge functions for the sursegmentation
		using HistogramPCMClassifier::assess;
		using HistogramPCMClassifier::merge;

	public :
		CumulativePCMClassifier(Real fuzzifier = 2);
		CumulativePCMClassifier(const RealFeature& binSize, Real fuzzifier = 2.);
		CumulativePCMClassifier(const std::string& histogramFilename, Real fuzzifier = 2.);
		using CumulativeFCMClassifier::addImages;

		//Classification functions
		using HistogramPCMClassifier::classification;
		using HistogramPCMClassifier::attribution;
			
		using HistogramPCMClassifier::computeJ;
		
		//Function to initialise the centers
		using CumulativeFCMClassifier::init;
		using CumulativeFCMClassifier::randomInit;

};
#endif
