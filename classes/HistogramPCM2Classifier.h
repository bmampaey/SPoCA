
#ifndef HistogramPCM2Classifier_H
#define HistogramPCM2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <limits>

#include "SunImage.h"
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

		//Function to initialise the centers
		using HistogramPCMClassifier::init;
		using HistogramPCMClassifier::randomInit;
		using HistogramPCMClassifier::FCMinit;

		//Utilities functions for outputing results
		void saveAllResults(SunImage* outImage);
		void saveARmap(SunImage* outImage);
		void saveCHmap(SunImage* outImage);
		using HistogramFCMClassifier::classAverage;

};
#endif
