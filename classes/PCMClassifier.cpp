#include "PCMClassifier.h"

using namespace std;

PCMClassifier::PCMClassifier(Real fuzzifier, unsigned numberClasses, Real precision, unsigned maxNumberIteration)
:FCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration), FCMfuzzifier(fuzzifier)
{
	#if defined DEBUG
	cout<<"Called PCM constructor"<<endl;
	#endif
}

PCMClassifier::PCMClassifier(ParameterSection& parameters)
:FCMClassifier(parameters)
{
	FCMfuzzifier = fuzzifier;
	if(parameters["PCMfuzzifier"].is_set())
	{
		fuzzifier = parameters["PCMfuzzifier"];
	}
	
	if (fuzzifier == 1 )
	{
		cerr<<"Error : Fuzzifier must not equal 1.";
		exit(EXIT_FAILURE);
	}
	
	#if defined DEBUG
	cout<<"Called PCM constructor with parameter section"<<endl;
	#endif
}

void PCMClassifier::computeU()
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
				*uij = 1. / (1. + *uij);
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
				*uij = 1. / (1. + pow(*uij , Real(1./(fuzzifier-1.))));
			}
		}
	}
}


void PCMClassifier::computeEta()
{

	// U must be initialized before computing eta 
	if(X.size() * numberClasses != U.size())
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
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = *uij * *uij;
				eta[i] += uij_m * distance_squared(*xj,B[i]);
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
void PCMClassifier::computeEta(Real alpha)
{
	eta.assign(numberClasses,0.);
	vector<Real> sum(numberClasses,0.);
	MembershipSet::iterator uij = U.begin();
	for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
	{
		for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
		{
			if (*uij > alpha)
			{
				eta[i] += distance_squared(*xj,B[i]);
				sum[i] += 1;
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

// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE
void PCMClassifier::classification()
{	
	const Real maxFactor = ETA_MAXFACTOR;


	#if defined EXTRA_SAFE
	if(X.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if defined VERBOSE
	cout<<"--PCMClassifier::classification--START--"<<endl;
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
		
		FCMClassifier::stepout(iteration, precisionReached, precision);
	}

	
	#if defined VERBOSE
	cout<<endl<<"--PCMClassifier::classification--END--"<<endl;
	#endif
	#if defined EXTRA_SAFE
	feenableexcept(excepts);
	#endif
}


Real PCMClassifier::computeJ() const
{
	Real result = 0;
	vector<Real> sum(numberClasses,0.);
	MembershipSet::const_iterator uij = U.begin();
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result += *uij * *uij * distance_squared(*xj,B[i]);
				sum[i] += (1 - *uij) * (1 - *uij); 
			}
		}
	}
	else
	{
		for (FeatureVectorSet::const_iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result += pow( *uij, fuzzifier) * distance_squared(*xj,B[i]);
				sum[i] += pow(Real(1. - *uij), fuzzifier); 
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		result += eta[i] * sum[i];
	}
	return result;

}


vector<Real> PCMClassifier::getEta()
{
	return eta;
}

void PCMClassifier::fillHeader(Header& header)
{
	FCMClassifier::fillHeader(header);
	header.set("CFCMFUZ", FCMfuzzifier, "FCM Fuzzifier");
	for (unsigned i = 0; i < eta.size(); ++i)
		header.set("CETA"+toString(i+1,2), toString(eta[i]), "Classification eta " + toString(i+1,2));
}

void PCMClassifier::initBEta(const std::vector<std::string>& channels, const std::vector<RealFeature>& B, const vector<Real>& eta)
{
	if(B.size() != eta.size())
	{
		cerr<<"Error : The number of class centers is different than the number of eta"<<endl;
		exit(EXIT_FAILURE);
	}
	
	initB(channels, B);
	this->eta = eta;
}

void PCMClassifier::initEta(const vector<Real>& eta)
{
	this->eta = eta;
}


void PCMClassifier::FCMinit()
{
	if(X.size() == 0)
	{
		cerr<<"Error : The set of FeatureVector must be initialized before doing a FCM init."<<endl;
		exit(EXIT_FAILURE);
	}
	if(B.size() == 0)
	{
		cerr<<"Error : The centers must be initialised before doing a FCM init."<<endl;
		exit(EXIT_FAILURE);
	}
	
	numberClasses = B.size();
	Real temp = fuzzifier;
	fuzzifier = FCMfuzzifier;
	FCMClassifier::classification();
	
	//We like our centers to be sorted 
	FCMClassifier::sortB();
	FCMClassifier::computeU();
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

void PCMClassifier::stepinit(const string filename)
{
	Classifier::stepinit(filename);
	#if defined DEBUG || defined VERBOSE
		ostringstream out;
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


void PCMClassifier::stepout(const unsigned iteration, const Real precisionReached, const Real precision)
{
	Classifier::stepout(iteration, precisionReached, precision);
	#if defined DEBUG || defined VERBOSE
		ostringstream out;
		out.setf(ios::fixed);
		out.precision(1 - log10(precision));
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

void PCMClassifier::sortB()
{
	/*! When sorting B, the eta need to follow the same order. */
	vector<RealFeature> sortedB = B;
	sort(sortedB.begin(), sortedB.end());
	vector<unsigned> indice(numberClasses);
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		for(unsigned ii = 0; ii < numberClasses; ++ii)
		{
			if(sortedB[i] == B[ii])
			{
				indice[i] = ii;
				break;
			}
		}
	}
	
	vector<Real> sortedEta(numberClasses);
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		sortedEta[i] = eta[indice[i]];
	}
	eta = sortedEta;
	B = sortedB;
}
