
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
		
		//Function to output classification steps
		virtual void stepinit(const std::string filename);
		virtual void stepout(const unsigned iteration, const Real precisionReached, const int precision = 5);

	public :
		// Constructors & Destructors
		PCMClassifier(Real fuzzifier = 1.5);

		// Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);

		// Function to initialise the centers & eta
		using Classifier::initB;
		virtual void initEta(const std::vector<Real>& eta);
		void initBEta(const std::vector<RealFeature>& B, const std::vector<Real>& eta);
		virtual void FCMinit(Real precision = 0.00001, unsigned maxNumberIteration = 100, Real FCMfuzzifier = 2);

		// Accessors
		std::vector<Real> getEta();
		void saveEta(const std::string& filename);

};
#endif
