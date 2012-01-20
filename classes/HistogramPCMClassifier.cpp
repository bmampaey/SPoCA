#include "HistogramPCMClassifier.h"

using namespace std;

HistogramPCMClassifier::HistogramPCMClassifier(Real fuzzifier)
:PCMClassifier(), HistogramFCMClassifier()
{
	this->fuzzifier = fuzzifier;
}

HistogramPCMClassifier::HistogramPCMClassifier(const RealFeature& binSize, Real fuzzifier)
:PCMClassifier(), HistogramFCMClassifier()
{
	this->fuzzifier = fuzzifier;
	initBinSize(binSize);
}

HistogramPCMClassifier::HistogramPCMClassifier(const std::string& histogramFilename, Real fuzzifier)
:PCMClassifier(), HistogramFCMClassifier()
{
	this->fuzzifier = fuzzifier;
	initHistogram(histogramFilename);
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

void HistogramPCMClassifier::classification(Real precision, unsigned maxNumberIteration)
{

	#if DEBUG >= 1
	if(HistoX.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if DEBUG >= 3
	cout<<"--HistogramPCMClassifier::classification--START--"<<endl;
	#endif

	#if DEBUG >= 2
		stepinit(filenamePrefix+"iterations.txt");
		unsigned decimals = unsigned(1 - log10(precision));;
	#endif

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

		#if DEBUG >= 2
			stepout(iteration, precisionReached, decimals);
		#endif
	}

	#if DEBUG >= 3
	cout<<endl<<"--HistogramPCMClassifier::classification--END--"<<endl;
	#endif
	#if DEBUG >= 1
	feenableexcept(excepts);
	#endif
}


Real HistogramPCMClassifier::assess(vector<Real>& V)
{
	V.assign(numberClasses, 0.);
	Real score = 0;
	unsigned numberElements = 0;
	vector<Real> sum(numberClasses,0.);
	
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
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				V[i] += distance_squared(*xj,B[i]) * *uij * *uij * xj->c;
				sum[i] += (1 - *uij) * (1 - *uij) * xj->c; 
				numberElements += xj->c;
			}
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				V[i] += distance_squared(*xj,B[i]) * pow(*uij, fuzzifier) * xj->c;
				sum[i] += pow(Real(1. - *uij), fuzzifier) * xj->c; 
				numberElements += xj->c;
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		V[i] += eta[i] * sum[i];
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberElements);

	}

	score /= (minDistBiBii * numberElements);

	return score;

}


void HistogramPCMClassifier::FCMinit(Real precision, unsigned maxNumberIteration, Real FCMfuzzifier)
{

	#if DEBUG >= 1
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
	#endif

	numberClasses = B.size();
	Real temp = fuzzifier;
	fuzzifier = FCMfuzzifier;
	HistogramFCMClassifier::classification(precision, maxNumberIteration);

	
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



