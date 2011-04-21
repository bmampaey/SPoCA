
#ifndef SPoCA2Classifier_H
#define SPoCA2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <list>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "PCM2Classifier.h"
#include "SPoCAClassifier.h"

//! Modified Spatial Possibilistic C-Means Classifier
/*!
The class implements a modified version of the SPoCA clustering algorithm.

The modifications have been made by Vincent Barra and Cis Verbeeck, to reduce the coincident clusters
problem and he derivation of the eta factors to zero.

The modifications are identical to the one done to the PCM2Classifier.

*/

class SPoCA2Classifier :  public PCM2Classifier, public virtual SPoCAClassifier
{
	protected :
		//Basic functions
		using SPoCAClassifier::computeB;
		void computeU();
		using PCM2Classifier::computeEta;
		//We don't know how to compute J for SPoCA2
		using SPoCAClassifier::computeJ;

	public :
		SPoCA2Classifier(unsigned neighborhoodRadius = 1, Real fuzzifier = 2.);

		//Classification functions
		using PCM2Classifier::classification;
		
		// Function to initialise the eta
		using PCM2Classifier::initEta;
		

			
};
#endif
