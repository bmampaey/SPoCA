
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

class SPoCA2Classifier :  public PCM2Classifier, public SPoCAClassifier
{
	protected :
		
		using SPoCAClassifier::computeB;
		
		//! Computation of the probability
		void computeU();
		
		using PCM2Classifier::computeEta;
		
		//We don't know how to compute J for SPoCA2
		using SPoCAClassifier::computeJ;

	public :
		//! Constructor
		SPoCA2Classifier(Real fuzzifier = 2., unsigned numberClasses = 0, Real precision = 0.0015, unsigned maxNumberIteration = 100, unsigned neighborhoodRadius = 1);
		
		//! Constructor
		SPoCA2Classifier(ParameterSection& parameters);

		//! Classification function
		using PCM2Classifier::classification;
		
		//! Function to initialise the eta
		using PCM2Classifier::initEta;
};
#endif
