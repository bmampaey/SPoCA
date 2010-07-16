
#ifndef PCMClassifier_H
#define PCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <limits>

#include "SunImage.h"
#include "FeatureVector.h"
#include "FCMClassifier.h"

class PCMClassifier : public virtual FCMClassifier
{
	protected :
		std::vector<Real> eta;

		void computeB(){FCMClassifier::computeB();}
		void computeU();
		Real computeJ() const;
		virtual void computeEta();
		virtual void computeEta(Real alpha);

		Real assess(std::vector<Real>& V);

	public :
		// Constructors & Destructors
		PCMClassifier(Real fuzzifier = 1.5);

		// Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);

		// Function to initialise the centers
		using Classifier::init;
		void init(const std::vector<RealFeature>& initB, const std::vector<Real>& initEta);
		virtual void FCMinit(Real precision = 0.00001, unsigned maxNumberIteration = 100, Real FCMfuzzifier = 2);

		// Accessors
		std::vector<Real> getEta();

};
#endif
