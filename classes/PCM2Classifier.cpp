#include "PCM2Classifier.h"

using namespace std;

PCM2Classifier::PCM2Classifier(Real fuzzifier)
:PCMClassifier(fuzzifier)
{}

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
	
	#if DEBUG >= 3
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
void PCM2Classifier::classification(Real precision, unsigned maxNumberIteration)
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
	cout<<"--PCM2Classifier::classification--START--"<<endl;
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

		#if DEBUG >= 2
			stepout(iteration, precisionReached, decimals);
		#endif

	}

	#if DEBUG >= 3
	cout<<endl<<"--PCM2Classifier::classification--END--"<<endl;
	#endif
	#if DEBUG >= 1
	feenableexcept(excepts);
	#endif
}

void PCM2Classifier::initEta(const vector<Real>& eta)
{
	this->eta = eta;
	reduceEta();
}




/*
// BAD RESULTS, BUT KEEP FOR NOW
// ALTERNATIVE VERSION: INCREASING ETAS
void PCM2Classifier::computeEta(ofstream* iterationsFile)
{
	PCMClassifier::computeEta(iterationsFile);

	signed i, j, p;
	const Real dilation = 1.02;
	Real max;

	vector< vector<Real> > alpha;
	vector< vector<Real> > beta;
	vector<Real>           gamma;

	alpha.resize(numberClasses);
	beta .resize(numberClasses);
	gamma.resize(numberClasses);
	for (i = 0; i < numberClasses; ++i)
	{
		alpha[i].resize(numberClasses);
		beta [i].resize(numberClasses);
	}

	for (i = numberClasses - 2; i >= 0; i--)
	{
		for (j = i + 1; j < numberClasses; ++j)
		{	
			max = numeric_limits<Real>::min();
			for(p = 0; p < NUMBERCHANNELS; ++p)
				if (B[i].v[p]/B[j].v[p] > max)
					max = B[i].v[p]/B[j].v[p];

			alpha[i][j] = max;
			beta [i][j] = eta[j] * alpha[i][j];
		}

		max = numeric_limits<Real>::min();
		for (j = i + 1; j < numberClasses; ++j)
			if (beta[i][j] > max)
				max = beta[i][j];

		gamma[i] = max;
		if (eta[i] <= gamma[i])
		{
			eta[i] = dilation * gamma[i];
		}
	}

	#if DEBUG >= 3
	cout<<"eta2:\t"<<eta<<endl;
	#endif	
}
*/

