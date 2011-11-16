#include "SPoCA2Classifier.h"

using namespace std;

SPoCA2Classifier::SPoCA2Classifier(unsigned neighborhoodRadius, Real fuzzifier)
:SPoCAClassifier(neighborhoodRadius), PCM2Classifier(fuzzifier)
{}


void SPoCA2Classifier::computeU()
{
	U.assign(numberFeatureVectors * numberClasses, 0.);

	// For each feature vector, we compute the distance to each Bi
	// And we had the distance to all the neighbors of j (including myself) multiplied by beta[N[j]]
	MembershipSet::iterator uij = U.begin();
	NeighborhoodVector::iterator Nj = N.begin();
	for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj, ++Nj)
	{
		for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
		{
			Real d2BiXj = distance_squared(*xj,B[i]);
			*uij += d2BiXj;
			for(Neighborhood::iterator k = Nj->begin(); k!=Nj->end(); ++k)
			{
				U[(*k)*numberClasses+i] += beta[*k] * d2BiXj;
			}
		}
	}
	// Now I fuzzify and inverse uij
	if(fuzzifier == 2)
	{
		for (MembershipSet::iterator uij = U.begin(); uij != U.end();)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij /= eta[i];
				*uij = 1. / (1. + *uij * *uij);
			}
		}
	}
	else if(fuzzifier == 1.5)
	{
		for (MembershipSet::iterator uij = U.begin(); uij != U.end();)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij /= eta[i];
				*uij *= *uij;
				*uij = 1. / (1. + *uij * *uij);
			}
		}
	}
	else
	{
		for (MembershipSet::iterator uij = U.begin(); uij != U.end();)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij /= eta[i];
				*uij = pow( *uij, Real(2./(fuzzifier-1.)));
				*uij = 1. / (1. + *uij);
			}
		}
	}

}
