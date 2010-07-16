
#ifndef PCM2Classifier_H
#define PCM2Classifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "SunImage.h"
#include "FeatureVector.h"
#include "PCMClassifier.h"

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

};
#endif
