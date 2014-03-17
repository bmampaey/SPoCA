
#ifndef HistogramPCMClassifier_H
#define HistogramPCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <limits>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "HistogramFCMClassifier.h"
#include "PCMClassifier.h"

class HistogramPCMClassifier : public virtual PCMClassifier, public HistogramFCMClassifier
{
	protected :

		using HistogramFCMClassifier::computeB;
		
		//! Computation of the probability
		void computeU();
		
		//! Computation of J the total intracluster variance
		Real computeJ() const;
		
		//! Function to compute eta
		virtual void computeEta();
		
		//! Function to compute eta
		virtual void computeEta(Real alpha);

	public :
		//! Constructor
		HistogramPCMClassifier(Real fuzzifier = 2., unsigned numberClasses = 0, Real precision = 0.0015, unsigned maxNumberIteration = 100, const RealFeature& binSize = 0.);
		
		//! Constructor
		HistogramPCMClassifier(ParameterSection& parameters);
		
		//! Classification function
		void classification();
		
		//! Function to do attribution (Fix center classification)
		void attribution();

		//Function to initialise the centers
		using HistogramFCMClassifier::initB;
		using HistogramFCMClassifier::randomInitB;
		
		//! Function to initialise the centers and eta by doing an FCM
		void FCMinit();

		//Utilities functions for outputing results
		using HistogramFCMClassifier::classAverage;

};
#endif
