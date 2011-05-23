
#ifndef PFCMClassifier_H
#define PFCMClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <limits>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "PCMClassifier.h"

//! Possibilistic C-Means Classifier
/*!
The class implements a multi channel Possibilistic C-Means clustering algorithm.
*/

//! The type for the set of tipicality
typedef std::vector<Real> TipicalitySet;

class PFCMClassifier : public virtual PCMClassifier
{
	protected :
	
		//! Tipicality fuzzifier
		Real nfuzzifier;
		
		//! Probability factor
		Real a;
		
		//! Tipicality factor
		Real b;
		
		//! Set of tipicality
		TipicalitySet T;

		//! Computation of the centers of classes
		void computeB();
		
		//! Computation of the probability
		using FCMClassifier::computeU;
		
		//! Computation of the tipicality
		void computeT();
		
		//! Computation of the tipicality and the probability at the same time
		void computeUT();
		
		//! Computation of J the total intracluster variance
		Real computeJ() const;
		
		Real assess(std::vector<Real>& V);
		
		//! Function to initialize the output of the classification steps
		void stepinit(const std::string filename);
		
		//! Function to output a classification step
		void stepout(const unsigned iteration, const Real precisionReached, const int precision = 5);

	public :
		//! Constructor
		PFCMClassifier(Real fuzzifier = 2, Real nfuzzifier = 2, Real a = 2, Real b = 2);

		//! Function to do the classification
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);


};
#endif
