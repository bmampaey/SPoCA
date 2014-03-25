#include "PFCMClassifier.h"

using namespace std;

PFCMClassifier::PFCMClassifier(Real fuzzifier, unsigned numberClasses, Real precision, unsigned maxNumberIteration, Real FCMweight, Real PCMweight)
:PCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration), FCMweight(FCMweight), PCMweight(PCMweight)
{
	#if defined DEBUG
	cout<<"Called PFCM constructor"<<endl;
	#endif
}

PFCMClassifier::PFCMClassifier(ParameterSection& parameters)
:PCMClassifier(parameters), FCMweight(parameters["FCMweight"]), PCMweight(parameters["PCMweight"])
{
	#if defined DEBUG
	cout<<"Called PFCM constructor with parameter section"<<endl;
	#endif
}

void PFCMClassifier::computeT()
{
	T.resize(numberFeatureVectors * numberClasses);
	vector<Real> beta(numberClasses);
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		beta[i] = PCMweight / eta[i];
	
	TipicalitySet::iterator tij = T.begin();
	if(fuzzifier == 1.5)
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
	else if(fuzzifier == 2)
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
				*tij = pow( *tij , Real(1./(fuzzifier-1.)));
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
		beta[i] = PCMweight / eta[i];
	
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
					if (FCMfuzzifier == 2)
					{
						sum += (d2XjB[i]/d2XjB[ii]);
					}
					else
					{
						sum += pow(d2XjB[i]/d2XjB[ii],Real(1./(FCMfuzzifier-1.)));
					}
				}
				*uij = 1./sum;
				
				*tij = d2XjB[i] * beta[i] ;

				if(fuzzifier == 1.5)
				{
					*tij *=  *tij;

				}
				else if(fuzzifier != 2)
				{
					*tij = pow( *tij , Real(1./(fuzzifier-1.)));
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
	// If the FCMfuzzifier is 2 we can optimise by avoiding the call to the pow function
	if(FCMfuzzifier == 2 && fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				Real aubt = (FCMweight * *uij * *uij) + (PCMweight * *tij * *tij);
				B[i] += *xj * aubt;
				sum[i] += aubt;
			}
		}
	}
	else if(FCMfuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				Real aubt = (FCMweight * *uij * *uij) + (PCMweight * pow(*tij,fuzzifier));
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
				Real aubt = (FCMweight * pow(*uij,FCMfuzzifier)) + (PCMweight * *tij * *tij);
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
				Real aubt = (FCMweight * pow(*uij,FCMfuzzifier)) + (PCMweight * pow(*tij,fuzzifier));
				B[i] += *xj * aubt;
				sum[i] += aubt;
			}
		}
	}
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		B[i] /= sum[i];
}



// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE
void PFCMClassifier::classification()
{
	const Real maxFactor = ETA_MAXFACTOR;
	if(X.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);
	}
	
	#if defined EXTRA_SAFE
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if defined VERBOSE
	cout<<"--PFCMClassifier::classification--START--"<<endl;
	#endif
	
	stepinit(filenamePrefix+"iterations.txt");
	
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
		
		stepout(iteration, precisionReached, precision);
	}
	
	#if defined VERBOSE
	cout<<endl<<"--PFCMClassifier::classification--END--"<<endl;
	#endif
	
	#if defined EXTRA_SAFE
	feenableexcept(excepts);
	#endif
}


Real PFCMClassifier::computeJ() const
{
	Real result = 0;
	TipicalitySet::const_iterator tij = T.begin();
	MembershipSet::const_iterator uij = U.begin();
	vector<Real> sum(numberClasses,0.);
	
	// If the FCMfuzzifier is 2 we can optimise by avoiding the call to the pow function
	if(FCMfuzzifier == 2 && fuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				result += (FCMweight * *uij * *uij) + (PCMweight * *tij * *tij) * distance_squared(*xj,B[i]);
				sum[i] += (1. - *tij) * (1. - *tij);
			}
		}
	}
	else if(FCMfuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				result += (FCMweight * *uij * *uij) + (PCMweight * pow(*tij,fuzzifier)) * distance_squared(*xj,B[i]);
				sum[i] += pow(Real(1. - *tij), fuzzifier);
			}
		}
	}
	else if(fuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++tij, ++uij)
			{
				result += (FCMweight * pow(*uij,FCMfuzzifier)) + (PCMweight * *tij * *tij) * distance_squared(*xj,B[i]);
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
				result += (FCMweight * pow(*uij,FCMfuzzifier)) + (PCMweight * pow(*tij,fuzzifier)) * distance_squared(*xj,B[i]);
				sum[i] += pow(Real(1. - *tij), fuzzifier);
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		result += eta[i] * sum[i];
	}
	return result;

}


void PFCMClassifier::stepinit(const string filename)
{
	Classifier::stepinit(filename);
	#if defined DEBUG || defined VERBOSE
		ostringstream out;
		out<<"\t"<<"J";
		out<<"\t"<<"eta";
		
		#if defined VERBOSE
			cout<<out.str();
		#endif
		
		#if defined DEBUG
			if(stepfile.good())
				stepfile<<out.str();
		#endif
	#endif
}

void PFCMClassifier::stepout(const unsigned iteration, const Real precisionReached, const Real precision)
{
	Classifier::stepout(iteration, precisionReached, precision);
	#if defined DEBUG || defined VERBOSE
		ostringstream out;
		out.setf(ios::fixed);
		out.precision(1 - log10(precision));
		out<<"\t"<<computeJ();
		out<<"\t"<<eta;
		
		#if defined VERBOSE
			cout<<out.str();
		#endif
		
		#if defined DEBUG
			if(stepfile.good())
				stepfile<<out.str();
		#endif
	#endif
}

void PFCMClassifier::fillHeader(Header& header)
{
	PCMClassifier::fillHeader(header);
	header.set("CFCMW8", FCMweight, "PFCM classifier FCM Weight");
	header.set("CPCMW8", PCMweight, "PFCM classifier PCM Weight");
}

