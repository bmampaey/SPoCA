
#ifndef PCM2Classifier_H
#define PCM2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "PCMClassifier.h"

//! Modified Possibilistic C-Means Classifier
/*!
The class implements a modified version of the PCM clustering algorithm.

The modification have been made by Vincent Barra and Cis Verbeeck, to reduce the coincident clusters
problem and he derivation of the eta factors to zero.

*/

class PCM2Classifier : public virtual PCMClassifier
{
	protected :

		using PCMClassifier::computeB;
		void computeU();
		void computeEta();
		void reduceEta();
		
		using PCMClassifier::computeJ;

	public :
		
		//Constructors & Destructors
		PCM2Classifier(Real fuzzifier = 2);

		//Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);
		
		// Function to initialise the eta
		void initEta(const std::vector<Real>& eta);

};
#endif
