
#ifndef CumulativeSPoCAClassifier_H
#define CumulativeSPoCAClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "SunImage.h"
#include "FeatureVector.h"
#include "CumulativeClassifier.h"
#include "SPoCAClassifier.h"

class CumulativeSPoCAClassifier : public virtual SPoCAClassifier, public CumulativeClassifier
{
	protected :
		using SPoCAClassifier::computeB;
		using SPoCAClassifier::computeU;
		using SPoCAClassifier::computeJ;
		using SPoCAClassifier::computeEta;

		//Asses & Merge functions for the sursegmentation
		using SPoCAClassifier::assess;
		using SPoCAClassifier::merge;

	public :
		CumulativeSPoCAClassifier(unsigned neighboorhoodRadius = 1, Real fuzzifier = 2);
		void addImages(std::vector<SunImage*>& images);

		//Classification functions
		using SPoCAClassifier::classification;
		using SPoCAClassifier::attribution;
		
		//Function to initialise the centers
		using SPoCAClassifier::initB;
		using SPoCAClassifier::randomInitB;

};
#endif
