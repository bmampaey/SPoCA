
#ifndef PCMClassifier_H
#define PCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <limits>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "FCMClassifier.h"

//! Possibilistic C-Means Classifier
/*!
The class implements a multi channel Possibilistic C-Means clustering algorithm.
*/

class PCMClassifier : public virtual FCMClassifier
{
	protected :
		//! Vector of eta values
		std::vector<Real> eta;

		using FCMClassifier::computeB;
		void computeU();
		Real computeJ() const;
		//! Function to compute eta
		virtual void computeEta();
		//! Function to compute eta
		virtual void computeEta(Real alpha);

		Real assess(std::vector<Real>& V);
		
		//! Function to sort the centers
		virtual void sortB();
		
		//Function to output classification steps
		virtual void stepinit(const std::string filename);
		virtual void stepout(const unsigned iteration, const Real precisionReached, const int precision = 5);

	public :
		// Constructors & Destructors
		PCMClassifier(Real fuzzifier = 1.5);

		// Classification functions
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);

		//! Function to initialise the centers
		using Classifier::initB;
		
		//! Function to initialise eta
		virtual void initEta(const std::vector<Real>& eta);
		
		//! Function to initialise the centers and eta
		void initBEta(const std::vector<RealFeature>& B, const std::vector<Real>& eta);
		
		//! Function to initialise the centers and eta by doing an FCM
		virtual void FCMinit(Real precision = 0.00001, unsigned maxNumberIteration = 100, Real FCMfuzzifier = 2);

		//! Accessors to retrieve eta
		std::vector<Real> getEta();
		
		//! Function to save eta to a file
		void saveEta(const std::string& filename);

};
#endif
