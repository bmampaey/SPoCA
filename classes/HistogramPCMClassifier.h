
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

class HistogramPCMClassifier : public virtual PCMClassifier,  public virtual HistogramFCMClassifier
{
	protected :

		void computeB(){HistogramFCMClassifier::computeB();}
		void computeU();
		Real computeJ() const;
		virtual void computeEta();
		virtual void computeEta(Real alpha);

		//Asses & Merge functions for the sursegmentation
		Real assess(std::vector<Real>& V);
		using HistogramFCMClassifier::merge;

	public :
		//Constructors & Destructors
		HistogramPCMClassifier(Real fuzzifier = 1.5);
		HistogramPCMClassifier(const RealFeature& binSize, Real fuzzifier = 2.);
		HistogramPCMClassifier(const std::string& histogramFilename, Real fuzzifier = 2.);
		
		//Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);
		void attribution();

		//Function to initialise the centers
		using HistogramFCMClassifier::initB;
		using HistogramFCMClassifier::randomInitB;
		void FCMinit(Real precision = 0.00001, unsigned maxNumberIteration = 100, Real FCMfuzzifier = 2);

		//Utilities functions for outputing results
		using HistogramFCMClassifier::classAverage;

};
#endif
