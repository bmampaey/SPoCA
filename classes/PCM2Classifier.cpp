#include "PCM2Classifier.h"

using namespace std;

PCM2Classifier::PCM2Classifier(Real fuzzifier, unsigned numberClasses, Real precision, unsigned maxNumberIteration)
:FCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration), PCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration)
{
	#if defined DEBUG
	cout<<"Called PCM2 constructor"<<endl;
	#endif
}

PCM2Classifier::PCM2Classifier(ParameterSection& parameters)
:FCMClassifier(parameters), PCMClassifier(parameters)
{
	#if defined DEBUG
	cout<<"Called PCM2 constructor with parameter section"<<endl;
	#endif
}

void PCM2Classifier::computeU()
{
	U.resize(numberFeatureVectors * numberClasses);
	
	MembershipSet::iterator uij = U.begin();
	if (fuzzifier == 1.5)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij = distance_squared(*xj,B[i]) / eta[i] ;
				*uij *= *uij;
				*uij = 1. / (1. + *uij * *uij);
			}
		}
	}
	else if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij = distance_squared(*xj,B[i]) / eta[i] ;
				*uij = 1. / (1. + *uij * *uij);
			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij = distance_squared(*xj,B[i]) / eta[i] ;
				*uij = 1. / (1. + pow(*uij , Real(2./(fuzzifier-1.))));
			}
		}
	}
}


void PCM2Classifier::computeEta()
{
	PCMClassifier::computeEta();
	
	#if defined VERBOSE
		cout<<"\npre_eta:\t"<<eta<<"\t";
	#endif
	
	reduceEta();
}


void PCM2Classifier::reduceEta()
{
	const Real reduction = ETA2_REDUCTION_FACTOR / 100.;
	
	// We need to treat the eta in ascending order
	// Therefore we sort them
	sortB();
	
	for (unsigned i = 1; i < numberClasses; ++i)
	{
		Real min = numeric_limits<Real>::max();
		for(unsigned ii = 0; ii < i; ++ii)
		{
			if (eta[ii] < eta[i])
			{
				Real min_Bi_above_Bii = B[i].v[0]/B[ii].v[0];
				for(unsigned p = 1; p < NUMBERCHANNELS; ++p)
					if (B[i].v[p]/B[ii].v[p] < min_Bi_above_Bii )
						min_Bi_above_Bii = B[i].v[p]/B[ii].v[p];

				if( eta[ii]*min_Bi_above_Bii < min)
					min = eta[ii]*min_Bi_above_Bii;
			}
		}

		if(eta[i] > reduction * min)
		{
			eta[i] = reduction * min;
		}
	} 
}



// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE
void PCM2Classifier::classification()
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
	cout<<"--PCM2Classifier::classification--START--"<<endl;
	#endif
	
	stepinit(filenamePrefix+"iterations.txt");
	
	//Initialisation of precision
	this->precision = precision;
	
	Real precisionReached = numeric_limits<Real>::max();
	vector<RealFeature> oldB = B;
	vector<Real> start_eta = eta;
	bool recomputeEta = FIXETA != true;
	for (unsigned iteration = 0; iteration < maxNumberIteration && precisionReached > precision ; ++iteration)
	{
		
		if (recomputeEta)
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
		
		computeU();
		computeB();
		
		precisionReached = variation(oldB,B);
		
		// avoid class cannibalism
		if (precisionReached <= precision)
		{
			reduceEta();
			computeU();
			computeB();
		}
		
		oldB = B;
		
		stepout(iteration, precisionReached, precision);
	}
	
	#if defined VERBOSE
	cout<<endl<<"--PCM2Classifier::classification--END--"<<endl;
	#endif
	
	#if defined EXTRA_SAFE
	feenableexcept(excepts);
	#endif
}

void PCM2Classifier::initEta(const vector<Real>& eta)
{
	this->eta = eta;
	reduceEta();
}
