#include "SPoCA2Classifier.h"

using namespace std;

SPoCA2Classifier::SPoCA2Classifier(unsigned neighboorhoodRadius, Real fuzzifier)
:SPoCAClassifier(neighboorhoodRadius), PCM2Classifier(fuzzifier)
{}

void SPoCA2Classifier::computeU()
{

	Real sumNeighboors;
	vector<Real> d2BiX(numberValidPixels);
	vector<unsigned>::iterator lastneighboor;
	U.resize(numberValidPixels * numberClasses);
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		//We precalculate all the distances from each pixel Xj to the center Bi
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			d2BiX[j] = d2(X[j],B[i]);
		}

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			sumNeighboors = 0;
			lastneighboor = N[j].end();
			for (vector<unsigned>::iterator k = N[j].begin(); k!=lastneighboor; ++k)
				sumNeighboors +=  d2BiX[(*k)];

			sumNeighboors = (sumNeighboors * beta[j]) + d2BiX[j];

			if(fuzzifier == 2)
				U[i*numberValidPixels+j] = sumNeighboors / eta[i] ;
			else
				U[i*numberValidPixels+j] = pow( sumNeighboors / eta[i], 1./(fuzzifier-1.) );

			U[i*numberValidPixels+j] *= U[i*numberValidPixels+j];
			U[i*numberValidPixels+j] = 1. / (1. + U[i*numberValidPixels+j]);

		}
	}

}
