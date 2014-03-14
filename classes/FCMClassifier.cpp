#include "FCMClassifier.h"

using namespace std;

FCMClassifier::FCMClassifier(Real fuzzifier, unsigned numberClasses, Real precision, unsigned maxNumberIteration)
:Classifier(fuzzifier, numberClasses, precision, maxNumberIteration)
{
	#if defined EXTRA_SAFE
	if (fuzzifier == 1)
	{
		cerr<<"Error : Fuzzifier must not equal 1.";
		exit(EXIT_FAILURE);
	}
	#endif
	#if defined DEBUG
	cout<<"Called FCM constructor"<<endl;
	#endif
}

FCMClassifier::FCMClassifier(ParameterSection& parameters)
:Classifier(parameters)
{
	if(parameters["FCMfuzzifier"].is_set())
		fuzzifier = parameters["FCMfuzzifier"];
	#if defined EXTRA_SAFE
	if (fuzzifier == 1)
	{
		cerr<<"Error : Fuzzifier must not equal 1.";
		exit(EXIT_FAILURE);
	}
	#endif

	#if defined DEBUG
	cout<<"Called FCM constructor with parameter section"<<endl;
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

	#if defined EXTRA_SAFE
	if(X.size() == 0 || B.size() == 0)
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if defined VERBOSE
	cout<<"--FCMClassifier::classification--START--"<<endl;
	#endif
	
	FCMClassifier::stepinit(filenamePrefix+"iterations.txt");
	
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
		
		FCMClassifier::stepout(iteration, precisionReached, precision);
	}

	#if defined VERBOSE
	cout<<"--FCMClassifier::classification--END--"<<endl;
	#endif
	
	#if defined EXTRA_SAFE
	feenableexcept(excepts);
	#endif

}
