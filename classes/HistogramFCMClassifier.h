
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
		void computeB();
		void computeU();
		Real computeJ() const;


		//Asses & Merge functions for the sursegmentation
		Real assess(std::vector<Real>& V);
		void merge(unsigned i1, unsigned i2);

	public :
		//Constructors & Destructors
		HistogramFCMClassifier(Real fuzzifier = 2.);
		HistogramFCMClassifier(const RealFeature& binSize, Real fuzzifier = 2.);
		HistogramFCMClassifier(const std::string& histogramFilename, Real fuzzifier = 2.);
		
		//Function to add images to the Histogram
		virtual void addImages(std::vector<EUVImage*> images);

		//Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);
		void attribution();

		//Utilities functions for outputing results	
		std::vector<RealFeature> classAverage() const;
		
		//Function to initialise the centers
		void initB(const std::vector<RealFeature>& B, const std::vector<std::string>& channels);
		void randomInitB(unsigned C);

};
#endif
