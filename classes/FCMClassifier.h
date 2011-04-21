
#ifndef FCMClassifier_H
#define FCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "Image.h"
#include "EUVImage.h"
#include "FeatureVector.h"
#include "Classifier.h"

//! Fuzzy C-Means Classifier
/*!
The class implements a multi channel Fuzzy C-Means clustering algorithm.
*/


class FCMClassifier : public Classifier
{
	protected :
		Real precision;

		void computeB();
		void computeU();
		Real computeJ() const;

		//Asses & Merge functions for the sursegmentation
		Real assess(std::vector<Real>& V);
		void merge(unsigned i1, unsigned i2);

	public :
		//Constructors & Destructors
		FCMClassifier(Real fuzzifier = 2.);

		//Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);
};
#endif
