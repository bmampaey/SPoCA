#include "PFCMClassifier.h"

using namespace std;

PFCMClassifier::PFCMClassifier(Real fuzzifier, Real nfuzzifier, Real a, Real b)
:PCMClassifier(fuzzifier),nfuzzifier(nfuzzifier),a(a),b(b)
{}


void PFCMClassifier::computeT()
{
	T.resize(numberFeatureVectors * numberClasses);
	vector<Real> beta(numberClasses);
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		beta[i] = b / eta[i];
	
	TipicalitySet::iterator tij = T.begin();
	if(nfuzzifier == 1.5)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij)
			{
				*tij = distance_squared(*xj,B[i]) * beta[i] ;
				*tij *=  *tij;
				*tij = 1. / (1. + *tij);
			}
		}
	}
	else if(nfuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij)
			{
				*tij = distance_squared(*xj,B[i]) * beta[i] ;
				*tij = 1. / (1. + *tij);
			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij)
			{
				*tij = distance_squared(*xj,B[i]) * beta[i] ;
				*tij = pow( *tij , Real(1./(nfuzzifier-1.)));
				*tij = 1. / (1. + *tij);
			}
		}
	}
}

void PFCMClassifier::computeUT()
{
	vector<Real> d2XjB(numberClasses);
	unsigned i;
	U.resize(numberFeatureVectors * numberClasses);
	T.resize(numberFeatureVectors * numberClasses);
	vector<Real> beta(numberClasses);
	for (i = 0 ; i < numberClasses ; ++i)
		beta[i] = b / eta[i];
	
	TipicalitySet::iterator tij = T.begin();
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
			for (unsigned ii = 0 ; ii < numberClasses ; ++ii, ++tij, ++uij)
			{
				*uij = i != ii? 0. : 1.;
				*tij = i != ii? 0. : 1.;
			}
		}
		else
		{
			for (i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				Real sum = 0;
				for (unsigned ii = 0 ; ii < numberClasses ; ++ii)
				{
					if (fuzzifier == 2)
					{
						sum += (d2XjB[i]/d2XjB[ii]);
					}
					else
					{
						sum += pow(d2XjB[i]/d2XjB[ii],Real(1./(fuzzifier-1.)));
					}
				}
				*uij = 1./sum;
				
				*tij = d2XjB[i] * beta[i] ;

				if(nfuzzifier == 1.5)
				{
					*tij *=  *tij;

				}
				else if(nfuzzifier != 2)
				{
					*tij = pow( *tij , Real(1./(nfuzzifier-1.)));
				}
				
				*tij = 1. / (1. + *tij);
			}
		}
	}
}

void PFCMClassifier::computeB()
{
	B.assign(numberClasses, 0.);
	vector<Real> sum(numberClasses, 0.);

	TipicalitySet::iterator tij = T.begin();
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if(fuzzifier == 2 && nfuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				Real aubt = (a * *uij * *uij) + (b * *tij * *tij);
				B[i] += *xj * aubt;
				sum[i] += aubt;
			}
		}
	}
	else if(fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				Real aubt = (a * *uij * *uij) + (b * pow(*tij,nfuzzifier));
				B[i] += *xj * aubt;
				sum[i] += aubt;
			}
		}
	}
	else if(nfuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				Real aubt = (a * pow(*uij,fuzzifier)) + (b * *tij * *tij);
				B[i] += *xj * aubt;
				sum[i] += aubt;
			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				Real aubt = (a * pow(*uij,fuzzifier)) + (b * pow(*tij,nfuzzifier));
				B[i] += *xj * aubt;
				sum[i] += aubt;
			}
		}
	}
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		B[i] /= sum[i];
}



// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE
void PFCMClassifier::classification(Real precision, unsigned maxNumberIteration)
{	
	const Real maxFactor = ETA_MAXFACTOR;


	#if DEBUG >= 1
	if(X.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if DEBUG >= 3
	cout<<"--PFCMClassifier::classification--START--"<<endl;
	#endif
	
	#if DEBUG >= 2
		stepinit(filenamePrefix+"iterations.txt");
		unsigned decimals = unsigned(1 - log10(precision));;
	#endif
	
	//Initialisation of precision & U
	this->precision = precision;

	Real precisionReached = numeric_limits<Real>::max();
	vector<RealFeature> oldB = B;
	vector<Real> start_eta = eta;
	bool recomputeEta = FIXETA != true;
	for (unsigned iteration = 0; iteration < maxNumberIteration && precisionReached > precision ; ++iteration)
	{

		if (recomputeEta)	//eta is to be recalculated each iteration.
		{
			computeEta();
			for (unsigned i = 0 ; i < numberClasses && recomputeEta ; ++i)
			{
				if ( (start_eta[i] / eta[i] > maxFactor) || (start_eta[i] / eta[i] < 1. / maxFactor) )
				{
					recomputeEta = false;
				}
			}
		}

		computeUT();
		computeB();

		precisionReached = variation(oldB,B);

		oldB = B;

		#if DEBUG >= 2
			stepout(iteration, precisionReached, decimals);
		#endif
	}

	
	#if DEBUG >= 3
	cout<<endl<<"--PFCMClassifier::classification--END--"<<endl;
	#endif
	#if DEBUG >= 1
	feenableexcept(excepts);
	#endif
}


Real PFCMClassifier::computeJ() const
{
	Real result = 0;
	TipicalitySet::const_iterator tij = T.begin();
	MembershipSet::const_iterator uij = U.begin();
	vector<Real> sum(numberClasses,0.);
	
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if(fuzzifier == 2 && nfuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				result += (a * *uij * *uij) + (b * *tij * *tij) * distance_squared(*xj,B[i]);
				sum[i] += (1. - *tij) * (1. - *tij);
			}
		}
	}
	else if(fuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				result += (a * *uij * *uij) + (b * pow(*tij,nfuzzifier)) * distance_squared(*xj,B[i]);
				sum[i] += pow(Real(1. - *tij), nfuzzifier);
			}
		}
	}
	else if(nfuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				result += (a * pow(*uij,fuzzifier)) + (b * *tij * *tij) * distance_squared(*xj,B[i]);
				sum[i] += (1. - *tij) * (1. - *tij);
			}
		}
	}
	else
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				result += (a * pow(*uij,fuzzifier)) + (b * pow(*tij,nfuzzifier)) * distance_squared(*xj,B[i]);
				sum[i] += pow(Real(1. - *tij), nfuzzifier);
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		result += eta[i] * sum[i];
	}
	return result;

}

//! TO BE DONE
Real PFCMClassifier::assess(vector<Real>& V)
{
	V.assign(numberClasses, 0.);
	Real score = 0;
/*
	//This is the vector of the min distances between the centers Bi and all the others centers Bii with ii!=i
	vector<Real> minDist(numberClasses, numeric_limits<Real>::max());
	//The min distance between all centers
	Real minDistBiBii = numeric_limits<Real>::max() ;

	Real distBiBii;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		for (unsigned ii = i + 1 ; ii < numberClasses ; ++ii)
		{
			distBiBii = distance_squared(B[i],B[ii]);
			if(distBiBii < minDist[i])
				minDist[i] = distBiBii;
			if(distBiBii < minDist[ii])
				minDist[ii] = distBiBii;
		}
	}

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		Real sum1 = 0, sum2 = 0;

		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{

			if(fuzzifier == 2)
				sum1 +=  *uij * *uij * distance_squared(*xj,B[i]);
			else
				sum1 +=  pow(*uij, fuzzifier) * distance_squared(*xj,B[i]);

			if(fuzzifier == 2)
				sum2 += (1 - *uij) * (1 - *uij);
			else
				sum2 +=  pow(Real(1 - *uij), fuzzifier);

		}

		V[i] = sum1 + (eta[i] * sum2);
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberFeatureVectors);

	}

	score /= (minDistBiBii * numberFeatureVectors);
	*/
	return score;

}



void PFCMClassifier::stepinit(const string filename)
{
		Classifier::stepinit(filename);
		ostringstream out;
		out<<"\t"<<"J";
		out<<"\t"<<"eta";
		if(stepfile.good())
			stepfile<<out.str();
		
		#if DEBUG >= 3
			cout<<out.str();
		#endif
	
}


void PFCMClassifier::stepout(const unsigned iteration, const Real precisionReached, const int decimals)
{
		Classifier::stepout(iteration, precisionReached, decimals);
		ostringstream out;
		out.setf(ios::fixed);
		out.precision(decimals);
		out<<"\t"<<computeJ();
		out<<"\t"<<eta;

		if(stepfile.good())
			stepfile<<out.str();
		
		#if DEBUG >= 3
			cout<<out.str();
		#endif
		
}

