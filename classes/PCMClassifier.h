
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
		
		//! Fuzzifier for FCM init
		Real FCMfuzzifier;
		
		using FCMClassifier::computeB;
		
		//! Computation of the probability
		void computeU();
		
		//! Computation of J the total intracluster variance
		Real computeJ() const;
		
		//! Function to compute eta
		virtual void computeEta();
		
		//! Function to compute eta
		virtual void computeEta(Real alpha);
		
		//! Function to initialize the output of the classification steps
		void stepinit(const std::string filename);
		
		//! Function to output a classification step
		void stepout(const unsigned iteration, const Real precisionReached, const Real precision);
	
	public :
		//! Constructor
		PCMClassifier(Real fuzzifier = 2., unsigned numberClasses = 0, Real precision = 0.0015, unsigned maxNumberIteration = 100);
		
		//! Constructor
		PCMClassifier(ParameterSection& parameters);
		
		//! Classification functions
		void classification();
		
		//! Function to initialise the centers
		using Classifier::initB;
		
		//! Function to initialise eta
		virtual void initEta(const std::vector<Real>& eta);
		
		//! Function to initialise the centers and eta
		void initBEta(const std::vector<std::string>& channels, const std::vector<RealFeature>& B, const std::vector<Real>& eta);
		
		//! Function to initialise the centers and eta by doing an FCM
		virtual void FCMinit();
		
		//! Function to initialise eta by doing an FCM attribution
		virtual void FCMinitEta();
		
		//! Function to sort the centers
		virtual void sortB();
		
		//! Accessors to retrieve eta
		std::vector<Real> getEta();
		
		//! Function to fill a fits header with classification information
		void fillHeader(Header& header);
};
#endif
