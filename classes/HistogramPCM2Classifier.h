
#ifndef HistogramPCM2Classifier_H
#define HistogramPCM2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <limits>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "HistogramPCMClassifier.h"
#include "PCM2Classifier.h"

class HistogramPCM2Classifier : public PCM2Classifier, public virtual HistogramPCMClassifier
{
	protected :

		using HistogramPCMClassifier::computeB;
		void computeU();
		void computeEta();

		
		//We don't know how to compute J for PCM2H
		using HistogramPCMClassifier::computeJ;

		//Asses & Merge functions for the sursegmentation
		using HistogramPCMClassifier::assess;
		using HistogramPCMClassifier::merge;

	public :
		//Constructors & Destructors
		HistogramPCM2Classifier(Real fuzzifier = 2);
		HistogramPCM2Classifier(const RealFeature& binSize, Real fuzzifier = 2.);
		HistogramPCM2Classifier(const std::string& histogramFilename, Real fuzzifier = 2.);

		//Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);
		void attribution();

		//Function to initialise the centers & eta
		using HistogramPCMClassifier::initB;
		using HistogramPCMClassifier::randomInitB;
		using HistogramPCMClassifier::FCMinit;
		using PCM2Classifier::initEta;

		//Utilities functions for outputing results
		using HistogramFCMClassifier::classAverage;

};
#endif
