
#ifndef SPoCA2Classifier_H
#define SPoCA2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <list>

#include "SunImage.h"
#include "FeatureVector.h"
#include "PCM2Classifier.h"
#include "SPoCAClassifier.h"

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
		SPoCA2Classifier(unsigned neighboorhoodRadius = 1, Real fuzzifier = 2.);

		//Classification functions
		using PCM2Classifier::classification;
		
		// Function to initialise the eta
		using PCM2Classifier::initEta;
		

			
};
#endif
