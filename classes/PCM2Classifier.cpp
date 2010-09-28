#include "PCM2Classifier.h"

using namespace std;

PCM2Classifier::PCM2Classifier(Real fuzzifier)
:PCMClassifier(fuzzifier)
{}

void PCM2Classifier::computeU()
{
	U.resize(numberValidPixels * numberClasses);
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			U[i*numberValidPixels+j] = d2(X[j],B[i]) / eta[i];
			if(fuzzifier == 1.5)
			{
				U[i*numberValidPixels+j] *= U[i*numberValidPixels+j];
				U[i*numberValidPixels+j] *= U[i*numberValidPixels+j];
			}
			else if(fuzzifier == 2)
			{
				U[i*numberValidPixels+j] *= U[i*numberValidPixels+j];
				
			}
			else
			{
				U[i*numberValidPixels+j] = pow( U[i*numberValidPixels+j] , 2./(fuzzifier-1.) ) ;
			}
				
			U[i*numberValidPixels+j] = 1. / (1. + U[i*numberValidPixels+j] ) ;

		}

	}

}

void PCM2Classifier::computeEta()
{
	PCMClassifier::computeEta();
	
	#if DEBUG >= 3
		cout<<"pre_eta:\t"<<eta<<"\t";
	#endif
	
	reduceEta();
}


void PCM2Classifier::reduceEta()
{

	
	const Real reduction = ETA2_REDUCTION_FACTOR / 100.;
	
	// We need to treat the eta in ascending order of B
	// Therefore we create an indice vector of the sorted values
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
	
	for (unsigned i = 1; i < numberClasses; ++i)
	{
		Real min = numeric_limits<Real>::max();
		for(unsigned ii = 0; ii < i; ++ii)
		{
			if (eta.at(indice[ii]) < eta.at(indice[i]))
			{
				Real min_Bi_above_Bii = B.at(indice[i]).v[0]/B.at(indice[ii]).v[0];
				for(unsigned p = 1; p < NUMBERWAVELENGTH; ++p)
					if (B.at(indice[i]).v[p]/B.at(indice[ii]).v[p] < min_Bi_above_Bii )
						min_Bi_above_Bii = B.at(indice[i]).v[p]/B.at(indice[ii]).v[p];

				if( eta.at(indice[ii])*min_Bi_above_Bii < min)
					min = eta.at(indice[ii])*min_Bi_above_Bii;
			}
		}

		if(eta.at(indice[i]) > reduction * min)
		{
			eta.at(indice[i]) = reduction * min;
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
		stepinit(outputFileName+"iterations.txt");
		unsigned decimals = unsigned(1 - log10(precision));;
	#endif
	
	//Initialisation of precision & U
	this->precision = precision;

	Real precisionReached = numeric_limits<Real>::max();
	vector<RealFeature> oldB = B;
	vector<Real> start_eta = eta;
	bool recomputeEta = FIXETA != TRUE;
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

		for (unsigned i = 0 ; i < numberClasses ; ++i)
		{
			precisionReached = d2(oldB[i],B[i]);
			if (precisionReached > precision)
			{
				break;
			}
		}
		
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
			for(p = 0; p < NUMBERWAVELENGTH; ++p)
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

