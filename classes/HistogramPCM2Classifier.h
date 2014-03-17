
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

class HistogramPCM2Classifier :  public PCM2Classifier, public HistogramPCMClassifier
{
	protected :

		using HistogramPCMClassifier::computeB;
		
		//! Computation of the probability
		void computeU();
		
		//! Function to compute eta
		void computeEta();
		
		//We don't know how to compute J for HPCM2
		using HistogramPCMClassifier::computeJ;

	public :
		//! Constructor
		HistogramPCM2Classifier(Real fuzzifier = 2., unsigned numberClasses = 0, Real precision = 0.0015, unsigned maxNumberIteration = 100, const RealFeature& binSize = 0.);
		
		//! Constructor
		HistogramPCM2Classifier(ParameterSection& parameters);

		//! Classification function
		void classification();
		
		//! Function to do attribution (Fix center classification)
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
