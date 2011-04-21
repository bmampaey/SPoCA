
#ifndef SPoCAClassifier_H
#define SPoCAClassifier_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <string>

#include "EUVImage.h"
#include "FeatureVector.h"
#include "PCMClassifier.h"


//! Spatial Possibilistic C-Means Classifier
/*!
The class implements a multi channel Spatialy constrained Possibilistic C-Means clustering algorithm.

The SPoCA Classifier has been described in Barra V., Delouille V., Hochedez J.-F.:2008 `Segmentation of extreme ultraviolet solar images via multichannel fuzzy clustering', Advances in Space Research, 42, 917--925.

*/

//! The type for neighbors indices
typedef unsigned NeighborIndex;

//! The type for the vector of neighbors
typedef std::vector<NeighborIndex> Neighborhood;

//! The type for the vector of the vector of neighbors
typedef std::vector<Neighborhood> NeighborhoodVector;


class SPoCAClassifier : public virtual PCMClassifier
{
	protected :
		//! Vector of precalculated neighbors smoothing (Xj + (betaj * sum Xn) for n belonging to Nj)
		FeatureVectorSet smoothedX;

		//! Vector of the beta function (1/Nj)
		std::vector<Real> beta;

		//! Vector of neighbors indices (Nj are the strict neighbors of pixel j)
		NeighborhoodVector N;
		
		//! The neighborhoodRadius <=> half the size of the square of neighbors. i.e. The square of neighbors has a side of (2 * Nradius) + 1
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
		SPoCAClassifier(unsigned neighborhoodRadius = 1, Real fuzzifier = 2);

		//! Function to add images to the classifier 
		void addImages(std::vector<EUVImage*> images);

		//Classification functions
		using PCMClassifier::classification;


};
#endif
