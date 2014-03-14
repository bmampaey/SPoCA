
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
		//! Computation of the centers of classes
		void computeB();
		
		//! Computation of the membership
		void computeU();
		
		//! Computation of J the total intracluster variance
		Real computeJ() const;
	
	public :
		//! Constructor
		FCMClassifier(Real fuzzifier = 2., unsigned numberClasses = 0, Real precision = 0.0015, unsigned maxNumberIteration = 100);
		
		//! Constructor
		FCMClassifier(ParameterSection& parameters);
		
		//Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);
};
#endif
