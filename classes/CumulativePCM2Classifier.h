
#ifndef CumulativePCM2Classifier_H
#define CumulativePCM2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "EUVImage.h"
#include "HistogramFeatureVector.h"
#include "CumulativePCMClassifier.h"
#include "HistogramPCM2Classifier.h"

class CumulativePCM2Classifier : public HistogramPCM2Classifier, public CumulativePCMClassifier
{
	protected :
		using HistogramPCM2Classifier::computeB;
		using HistogramPCM2Classifier::computeU;
		using HistogramPCM2Classifier::computeEta;

		//Asses & Merge functions for the sursegmentation
		using HistogramPCM2Classifier::assess;
		using HistogramPCM2Classifier::merge;

	public :
		CumulativePCM2Classifier(Real fuzzifier = 2);
		CumulativePCM2Classifier(const RealFeature& binSize, Real fuzzifier = 2.);
		CumulativePCM2Classifier(const std::string& histogramFilename, Real fuzzifier = 2.);
		using CumulativeFCMClassifier::addImages;

		//Classification functions
		using HistogramPCM2Classifier::classification;
		using HistogramPCM2Classifier::attribution;
			
		using HistogramPCM2Classifier::computeJ;
		
		//Function to initialise the centers & eta
		using CumulativePCMClassifier::initB;
		using CumulativePCMClassifier::randomInitB;
		using HistogramPCM2Classifier::initEta;

};
#endif
