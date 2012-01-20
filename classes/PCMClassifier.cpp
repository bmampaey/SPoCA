#include "PCMClassifier.h"

using namespace std;

PCMClassifier::PCMClassifier(Real fuzzifier)
:FCMClassifier(fuzzifier)
{}

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
void PCMClassifier::classification(Real precision, unsigned maxNumberIteration)
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
	cout<<"--PCMClassifier::classification--START--"<<endl;
	#endif
	
	#if DEBUG >= 2
		stepinit(filenamePrefix+"iterations.txt");
		unsigned decimals = unsigned(1 - log10(precision));;
	#endif
	
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
	cout<<endl<<"--PCMClassifier::classification--END--"<<endl;
	#endif
	#if DEBUG >= 1
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

Real PCMClassifier::assess(vector<Real>& V)
{
	V.assign(numberClasses, 0.);
	Real score = 0;
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
		for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				V[i] += distance_squared(*xj,B[i]) * *uij * *uij;
				sum[i] += (1 - *uij) * (1 - *uij); 
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
				sum[i] += pow(Real(1. - *uij), fuzzifier); 
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		V[i] += eta[i] * sum[i];
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberFeatureVectors);

	}

	score /= (minDistBiBii * numberFeatureVectors);

	return score;

}


vector<Real> PCMClassifier::getEta()
{
	return eta;
}

void PCMClassifier::saveEta(const string& filename)
{
	ofstream etaFile(filename.c_str());
	if (etaFile.good())
	{
		etaFile<<eta<<endl;
		etaFile.close();
	}
}

void PCMClassifier::initBEta(const vector<RealFeature>& B, const vector<Real>& eta)
{
	#if DEBUG >= 1
	if(B.size() != eta.size())
	{
		cerr<<"Error : The size of initB is different than the size of initEta"<<endl;
		exit(EXIT_FAILURE);

	}
	#endif

	initB(B);
	this->eta = eta;
}

void PCMClassifier::initEta(const vector<Real>& eta)
{
	this->eta = eta;
}


void PCMClassifier::FCMinit(Real precision, unsigned maxNumberIteration, Real FCMfuzzifier)
{

	#if DEBUG >= 1
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
	#endif

	numberClasses = B.size();
	Real temp = fuzzifier;
	fuzzifier = FCMfuzzifier;
	FCMClassifier::classification(precision, maxNumberIteration);
	
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
		ostringstream out;
		out<<"\t"<<"eta";
		if(stepfile.good())
			stepfile<<out.str();
		
		#if DEBUG >= 3
			cout<<out.str();
		#endif
	
}


void PCMClassifier::stepout(const unsigned iteration, const Real precisionReached, const int decimals)
{
		Classifier::stepout(iteration, precisionReached, decimals);
		ostringstream out;
		out.setf(ios::fixed);
		out.precision(decimals);
		out<<"\t"<<eta;

		if(stepfile.good())
			stepfile<<out.str();
		
		#if DEBUG >= 3
			cout<<out.str();
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
