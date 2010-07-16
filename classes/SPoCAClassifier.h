
#ifndef SPoCAClassifier_H
#define SPoCAClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "SunImage.h"
#include "FeatureVector.h"
#include "PCMClassifier.h"

class SPoCAClassifier : public virtual PCMClassifier
{
	protected :
		//vector of precalculated neighboors smoothing (Xj + (betaj * sum Xn) for n belonging to Nj)
		std::vector<RealFeature> smoothedX;

		//vector of the beta function (1/number of valid neighboors)
		std::vector<Real> beta;

		//vector of neighboors (Nj are the strict neighboors of pixel j)
		std::vector<std::vector<unsigned> > N;
		
		//the neighboorhoodRadius == half the size of the square of neighboors
		unsigned Nradius;

		// Basic functions
		void computeB();
		void computeU();
		Real computeJ() const;
		using PCMClassifier::computeEta;

		//Asses & Merge functions for the sursegmentation
		Real assess(std::vector<Real>& V);
		void merge(unsigned i1, unsigned i2);

	public :
		//Constructors & Destructors
		SPoCAClassifier(unsigned neighboorhoodRadius = 1, Real fuzzifier = 2);

		//Functions to add images
		void addImages(std::vector<SunImage*>& images);

		//Classification functions
		using PCMClassifier::classification;


};
#endif
