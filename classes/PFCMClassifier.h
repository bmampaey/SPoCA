
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
The class implements a multi channel Possibilistic Fuzzy C-Means clustering algorithm.
See paper A Possibilistic Fuzzy c-Means Clustering Algorithm by Nikhil R. Pal, Kuhu Pal, James M. Keller, and James C. Bezdek

PFCM is a hybridization of possibilistic c-means (PCM) and fuzzy c-means (FCM) that often avoids various problems of PCM, FCM and FPCM.
PFCM solves the noise sensitivity defect of FCM, overcomes the coincident clusters problem of PCM and eliminates the row sum constraints of FPCM.

In PFCM, FCM membership is called probability, and PCM possibility is called tipicality.
 
*/

//! The type for the set of tipicality
typedef std::vector<Real> TipicalitySet;

class PFCMClassifier : public virtual PCMClassifier
{
	protected :
	
		//! Probability factor (FCM)
		Real FCMweight;
		
		//! Tipicality factor (PCM)
		Real PCMweight;
		
		//! Set of tipicality (PCM)
		TipicalitySet T;

		//! Computation of the centers of classes
		void computeB();
		
		//! Computation of the probability (FCM)
		using FCMClassifier::computeU;
		
		//! Computation of the tipicality (PCM)
		void computeT();
		
		//! Computation of the tipicality and the probability at the same time
		void computeUT();
		
		//! Computation of J the total intracluster variance
		Real computeJ() const;
		
		//! Function to initialize the output of the classification steps
		void stepinit(const std::string filename);
		
		//! Function to output a classification step
		void stepout(const unsigned iteration, const Real precisionReached, const Real precision);

	public :
		//! Constructor
		PFCMClassifier(Real fuzzifier = 2., unsigned numberClasses = 0, Real precision = 0.0015, unsigned maxNumberIteration = 100, Real FCMweight = 2, Real PCMweight = 2);

		//! Constructor
		PFCMClassifier(ParameterSection& parameters);

		//! Function to do the classification
		void classification(Real precision = 1., unsigned maxNumberIteration = 100);

		//! Function to fill a fits header with classification information
		void fillHeader(Header& header);
};
#endif
