
#ifndef CumulativeSPoCA2Classifier_H
#define CumulativeSPoCA2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "CumulativeClassifier.h"
#include "CumulativeSPoCAClassifier.h"
#include "SPoCA2Classifier.h"

class CumulativeSPoCA2Classifier : public  SPoCA2Classifier, public CumulativeSPoCAClassifier
{

	protected :
		using SPoCA2Classifier::computeU;
		using SPoCA2Classifier::computeB;

		using SPoCA2Classifier::computeEta;

		//Asses & Merge functions for the sursegmentation
		using SPoCA2Classifier::assess;
		using SPoCA2Classifier::merge;
	public :
		CumulativeSPoCA2Classifier(unsigned neighboorhoodRadius = 1, Real fuzzifier = 2);
		using CumulativeSPoCAClassifier::addImages;

		//Classification functions
		using SPoCA2Classifier::classification;
		using SPoCA2Classifier::attribution;
			
		using SPoCA2Classifier::computeJ;
		
		//Function to initialise the centers
		using CumulativeSPoCAClassifier::initB;
		using CumulativeSPoCAClassifier::randomInitB;


};
#endif
