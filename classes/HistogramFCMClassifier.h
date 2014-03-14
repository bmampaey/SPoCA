
#ifndef HistogramFCMClassifier_H
#define HistogramFCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <fstream>

#include "Image.h"
#include "EUVImage.h"
#include "HistogramFeatureVector.h"
#include "FeatureVector.h"
#include "FCMClassifier.h"
#include "HistogramClassifier.h"

class HistogramFCMClassifier : public virtual FCMClassifier, public HistogramClassifier
{

	protected :
		//! Computation of the centers of classes
		void computeB();
		
		//! Computation of the membership
		void computeU();
		
		//! Computation of J the total intracluster variance
		Real computeJ() const;
	
	public :
		//! Constructor
		HistogramFCMClassifier(Real fuzzifier = 2., unsigned numberClasses = 0, Real precision = 0.0015, unsigned maxNumberIteration = 100, const RealFeature& binSize = 0.);
		
		//! Constructor
		HistogramFCMClassifier(ParameterSection& parameters);
		
		//! Function to add images to the Histogram
		virtual void addImages(std::vector<EUVImage*> images);
		
		//! Classification function
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);
		
		//! Function to do attribution (Fix center classification).
		void attribution();
		
		//! Utilities functions for outputing results
		std::vector<RealFeature> classAverage() const;
		
		//! Function to initialise the centers
		void initB(const std::vector<std::string>& channels, const std::vector<RealFeature>& B);
		void randomInitB(unsigned C);

};
#endif
