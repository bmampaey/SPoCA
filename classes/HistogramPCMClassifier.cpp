#include "HistogramPCMClassifier.h"

using namespace std;


HistogramPCMClassifier::HistogramPCMClassifier(Real fuzzifier, unsigned numberClasses, Real precision, unsigned maxNumberIteration, const RealFeature& binSize)
:FCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration), PCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration), HistogramFCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration, binSize)
{
	#if defined DEBUG
	cout<<"Called HPCM constructor"<<endl;
	#endif
}

HistogramPCMClassifier::HistogramPCMClassifier(ParameterSection& parameters)
:FCMClassifier(parameters), PCMClassifier(parameters), HistogramFCMClassifier(parameters)
{
	#if defined DEBUG
	cout<<"Called HPCM constructor with parameter section"<<endl;
	#endif
}


void HistogramPCMClassifier::attribution()
{
	PCMClassifier::sortB();
	PCMClassifier::computeU();
}

void HistogramPCMClassifier::computeU()
{
	U.resize(numberBins * numberClasses);
	
	MembershipSet::iterator uij = U.begin();
	if (fuzzifier == 1.5)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij = distance_squared(*xj,B[i]) / eta[i] ;
				*uij = 1. / (1. + *uij * *uij);
			}
		}
	}
	else if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij = distance_squared(*xj,B[i]) / eta[i] ;
				*uij = 1. / (1. + *uij);
			}
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij = distance_squared(*xj,B[i]) / eta[i] ;
				*uij = 1. / (1. + pow(*uij , Real(1./(fuzzifier-1.))));
			}
		}
	}
}


void HistogramPCMClassifier::computeEta()
{
	// U must be initialized before computing eta 
	if(HistoX.size() * numberClasses != U.size())
	{
		if(numberClasses > 0 && eta.size() == numberClasses)
		{
			// We have centers and eta we can initialized U
			computeU();
		}
		else
		{
			cerr<<"Error: U must be initialized before computing eta."<<endl;
			exit(EXIT_FAILURE);
		}
	}
	eta.assign(numberClasses,0.);
	vector<Real> sum(numberClasses,0.);
	
	MembershipSet::iterator uij = U.begin();
	if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = *uij * *uij * xj->c;
				eta[i] += uij_m * distance_squared(*xj,B[i]);
				sum[i] += uij_m;
			}
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = pow(*uij,fuzzifier) * xj->c;
				eta[i] += uij_m * distance_squared(*xj,B[i]);
				sum[i] += uij_m;
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		eta[i] /= sum[i];
	}
}

/*!
This is another method to compute eta.
It was described by Krishnapuram and Keller and is faster than the other computeEta method.
*/
void HistogramPCMClassifier::computeEta(Real alpha)
{
	eta.assign(numberClasses,0.);
	vector<Real> sum(numberClasses,0.);
	MembershipSet::iterator uij = U.begin();
	for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
	{
		for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
		{
			if (*uij > alpha)
			{
				eta[i] += distance_squared(*xj,B[i]);
				sum[i] +=  xj->c;
			}
		}
	}

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		if(sum[i] != 0)
			eta[i] /= sum[i];
		else
		{
			cerr<<"Error : Computation of Eta failed for class "<<i<<endl;
			exit(EXIT_FAILURE);
		}
	}
}


Real HistogramPCMClassifier::computeJ() const
{
	Real result = 0;
	vector<Real> sum(numberClasses,0.);
	MembershipSet::const_iterator uij = U.begin();
	if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result += *uij * *uij * distance_squared(*xj,B[i]) * xj->c;
				sum[i] += (1 - *uij) * (1 - *uij) * xj->c; 
			}
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result += pow( *uij, fuzzifier) * distance_squared(*xj,B[i]) * xj->c;
				sum[i] += pow(Real(1. - *uij), fuzzifier) * xj->c; 
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		result += eta[i] * sum[i];
	}
	return result;

}


// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE

void HistogramPCMClassifier::classification()
{
	if(HistoX.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);
	}
	
	#if defined EXTRA_SAFE
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif
	
	#if defined VERBOSE
	cout<<"--HistogramPCMClassifier::classification--START--"<<endl;
	#endif
	
	stepinit(filenamePrefix+"iterations.txt");
	
	const Real maxFactor = ETA_MAXFACTOR;
	
	//Initialisation of precision
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
		
		computeU();
		computeB();
		
		precisionReached = variation(oldB,B);
		
		oldB = B;
		
		stepout(iteration, precisionReached, precision);
	}
	
	#if defined VERBOSE
	cout<<endl<<"--HistogramPCMClassifier::classification--END--"<<endl;
	#endif
	
	#if defined EXTRA_SAFE
	feenableexcept(excepts);
	#endif
}

void HistogramPCMClassifier::FCMinit()
{
	if(HistoX.size() == 0)
	{
		cerr<<"Error : The set of FeatureVector must be initialized before doing a centers only init."<<endl;
		exit(EXIT_FAILURE);
	}
	if(B.size() == 0)
	{
		cerr<<"Error : The centers must be initialised before doing FCM."<<endl;
		exit(EXIT_FAILURE);
	}
	
	numberClasses = B.size();
	Real temp = fuzzifier;
	fuzzifier = FCMfuzzifier;
	HistogramFCMClassifier::classification();
	
	//We like our centers to be sorted 
	HistogramFCMClassifier::sortB();
	HistogramFCMClassifier::computeU();
	fuzzifier = temp;
	//We initialise eta
	computeEta();
	
	#ifdef ETA_BEN
	//This is just a test
	//We try to stabilize eta before starting the classification
	vector<Real> oldEta = eta;
	Real precisionReached = numeric_limits<Real>::max();
	for (unsigned iteration = 0; iteration < maxNumberIteration && precisionReached > precision ; ++iteration)
	{		
		computeU();
		computeEta();
		
		for (unsigned i = 0 ; i < numberClasses ; ++i)
		{
			precisionReached = abs((eta[i] - oldEta[i])/oldEta[i]);
			if (precisionReached > precision)
				break;
		}
		cout<<"eta :"<<eta<<endl;
		oldEta = eta;
	}
	#endif
}



