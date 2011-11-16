#include "FCMClassifier.h"

using namespace std;

FCMClassifier::FCMClassifier(Real fuzzifier)
:Classifier(fuzzifier)
{
	#if DEBUG >= 1
	if (fuzzifier == 1)
	{
		cerr<<"Error : Fuzzifier must not equal 1.";
		exit(EXIT_FAILURE);
	}
	#endif
	

}


void FCMClassifier::computeB()
{
	B.assign(numberClasses, 0.);
	vector<Real> sum(numberClasses, 0.);
	
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = *uij * *uij;
				B[i] += *xj * uij_m;
				sum[i] += uij_m;
			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = pow(*uij,fuzzifier);
				B[i] += *xj * uij_m;
				sum[i] += uij_m;
			}
		}
	}
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		B[i] /= sum[i];
}



void FCMClassifier::computeU()
{
	vector<Real> d2XjB(numberClasses);
	U.resize(numberFeatureVectors * numberClasses);
	
	unsigned i;
	MembershipSet::iterator uij = U.begin();
	
	for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
	{
		for (i = 0 ; i < numberClasses ; ++i)
		{
			d2XjB[i] = distance_squared(*xj,B[i]);
			if (d2XjB[i] < precision)
				break;
		}
		// The pixel is very close to B[i]
		if(i < numberClasses)
		{
			for (unsigned ii = 0 ; ii < numberClasses ; ++ii, ++uij)
			{
				*uij = i != ii? 0. : 1.;
			}
		}
		// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
		else if (fuzzifier == 2)
		{
			for (i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real sum = 0;
				for (unsigned ii = 0 ; ii < numberClasses ; ++ii)
					sum += (d2XjB[i]/d2XjB[ii]);
				*uij = 1./sum;
			}
		}
		else
		{
			for (i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real sum = 0;
				for (unsigned ii = 0 ; ii < numberClasses ; ++ii)
					sum += pow(d2XjB[i]/d2XjB[ii],Real(1./(fuzzifier-1.)));
				*uij = 1./sum;
			}
		}

	}

}


Real FCMClassifier::computeJ() const
{
	Real result = 0;
	MembershipSet::const_iterator uij = U.begin();
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result +=  *uij * *uij * distance_squared(*xj,B[i]);
			}
		}
	}
	else
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result +=  pow(*uij, fuzzifier) * distance_squared(*xj,B[i]);
			}
		}
	}

	return result;
}


void FCMClassifier::classification(Real precision, unsigned maxNumberIteration)
{

	#if DEBUG >= 1
	if(X.size() == 0 || B.size() == 0)
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if DEBUG >= 3
	cout<<"--FCMClassifier::classification--START--"<<endl;
	#endif
	
	#if DEBUG >= 2
		FCMClassifier::stepinit(filenamePrefix+"iterations.txt");
		unsigned decimals = unsigned(1 - log10(precision));
	#endif
	
	//Initialisation of precision
	this->precision = precision;

	Real precisionReached = numeric_limits<Real>::max();
	vector<RealFeature> oldB = B;
	for (unsigned iteration = 0; iteration < maxNumberIteration && precisionReached > precision ; ++iteration)
	{
		FCMClassifier::computeU();
		FCMClassifier::computeB();
		
		precisionReached = variation(oldB,B);
		oldB = B;

		#if DEBUG >= 2
			FCMClassifier::stepout(iteration, precisionReached, decimals);
		#endif

	}

	#if DEBUG >= 3
	cout<<"--FCMClassifier::classification--END--"<<endl;
	#endif
	
	#if DEBUG >= 1
	feenableexcept(excepts);
	#endif

}


Real FCMClassifier::assess(vector<Real>& V)
{
	V.assign(numberClasses, 0.);
	Real score = 0;

	//This is the vector of the min distances between the centers Bi and all the others centers Bii with ii!=i
	vector<Real> minDist(numberClasses, numeric_limits<Real>::max());
	//The min distance between any 2 centers
	Real minDistBiBii = numeric_limits<Real>::max() ;

	Real distBiBii;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		for (unsigned ii = i + 1 ; ii < numberClasses ; ++ii)
		{
			distBiBii = distance_squared(B[i],B[ii]);
			if(distBiBii < minDist[i])
				minDist[i] = distBiBii;
			if(distBiBii < minDist[ii])
				minDist[ii] = distBiBii;
		}
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				V[i] += distance_squared(*xj,B[i]) * *uij * *uij;
			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				V[i] += distance_squared(*xj,B[i]) * pow(*uij, fuzzifier);
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberFeatureVectors);

	}

	score /= (minDistBiBii * numberFeatureVectors);

	return score;

}


#if MERGE_TYPE==MERGEMAX
/*!
Compute the new center by computing the mean value of the featurevector belonging to one of the 2 centers to be merged, weighted by the max value of the membership of the 2 centers.
A featurevector belong to a class if it's memebership is maximal for that class. 
The values of the membership are computed using the regular method for computing memebership with the new centers.
*/

void FCMClassifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			Real max_uij = 0;
			unsigned max_i = 0;
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				if (*uij > max_uij)
				{
					max_uij = *uij;
					max_i = i;
				}
			}
			// If it belongs to one of the 2 class I am merging, I update it's B
			if(max_i == i1 || max_i == i2)
			{
				Real uij_m = max_uij * max_uij;
				newB += *xj * uij_m;
				sum += uij_m;

			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			Real max_uij = 0;
			unsigned max_i = 0;
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				if (*uij > max_uij)
				{
					max_uij = *uij;
					max_i = i;
				}
			}
			// If it belongs to one of the 2 class I am merging, I update it's B
			if(max_i == i1 || max_i == i2)
			{
				Real uij_m = pow(max_uij,fuzzifier);
				newB += *xj * uij_m;
				sum += uij_m;

			}
		}
	}

	

	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif
	
	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;
	
	computeU();
}


#elif MERGE_TYPE==MERGECIS
/*!
The values of the membership are computed by taking the maximal membership value for the 2 classes to be merged.
The new center is computed using the regular method for computing centers with the new membership.

N.B. This method invalidate the constraint that the sum of membership for a feature vector is equal to 1 
*/
void FCMClassifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			if(*(uij+i1) < *(uij+i2))
				*(uij+i1) = *(uij+i2);
			
			Real uij_m = *(uij+i1) **(uij+i1);
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			if(*(uij+i1) < *(uij+i2))
				*(uij+i1) = *(uij+i2);
			
			Real uij_m = pow(*(uij+i1),fuzzifier);
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	
	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif

	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;
	

}
#elif MERGE_TYPE==MERGESUM
/*!
The values of the membership are computed by taking the sum of the membership value for the 2 classes to be merged.
The new center is computed using the regular method for computing centers with the new membership.
*/
void FCMClassifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{ 
			*(uij+i1) += *(uij+i2);
			Real uij_m = *(uij+i1) **(uij+i1);
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			*(uij+i1) += *(uij+i2);
			Real uij_m = pow(*(uij+i1),fuzzifier);
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	
	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif
	
	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;

}
#endif
