#include "PCMClassifier.h"

using namespace std;

PCMClassifier::PCMClassifier(Real fuzzifier)
:FCMClassifier(fuzzifier)
{}

void PCMClassifier::computeU()
{
	U.resize(numberValidPixels * numberClasses);
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			U[i*numberValidPixels+j] = d2(X[j],B[i]) / eta[i] ;
			if(fuzzifier == 1.5)
			{
				U[i*numberValidPixels+j] *=  U[i*numberValidPixels+j];
			}
			else if(fuzzifier != 2)
			{
				U[i*numberValidPixels+j] = pow( U[i*numberValidPixels+j] , 1./(fuzzifier-1.) );
			}
			U[i*numberValidPixels+j] = 1. / (1. + U[i*numberValidPixels+j]);
		}

	}

}


void PCMClassifier::computeEta()
{
	eta = vector<Real>(numberClasses,0.);

	Real sum, uij_m;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		sum = 0;
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{

			if(fuzzifier == 2)
				uij_m = U[i*numberValidPixels+j] * U[i*numberValidPixels+j];
			else
				uij_m = pow(U[i*numberValidPixels+j],fuzzifier);

			eta[i] += uij_m*d2(X[j],B[i]);
			sum += uij_m;
		}
		eta[i] /= sum;
	}
}

//This is the other method to calculate eta, descibed by Krishnapuram and Keller. It is a little faster by the way
void PCMClassifier::computeEta(Real alpha)
{
	
	eta = vector<Real>(numberClasses,0.);

	Real sum;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		sum = 0;
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			if (U[i*numberValidPixels+j]>alpha)
			{
				eta[i] += d2(X[j],B[i]);
				++sum;
			}

		}

		eta[i] /= sum;

	}

}

// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE
void PCMClassifier::classification(Real precision, unsigned maxNumberIteration)
{	
	const Real maxFactor = ETA_MAXFACTOR;


	#if defined(DEBUG) && DEBUG >= 1
	if(X.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if defined(DEBUG) && DEBUG >= 3
	cout<<"--PCMClassifier::classification--START--"<<endl;
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
				break;
		}

		oldB = B;

		#if defined(DEBUG) && DEBUG >= 3
		cout<<"iteration :"<<iteration;
		cout<<"\tprecisionReached :"<<precisionReached;
		#if DEBUG >= 4
			cout<<"\tJPCM :"<<computeJ();
		#endif
		cout<<"\tB :"<<B;
		cout<<"\teta :"<<eta;
		
		// We compute the real average of each class
		vector<RealFeature> class_average(numberClasses, 0.);
		vector<Real> cardinal(numberClasses, 0.);
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			Real max_uij = U[j];
			unsigned belongsTo = 0;
			for (unsigned i = 1 ; i < numberClasses ; ++i)
			{
				if (U[i*numberValidPixels+j] > max_uij)
				{
					max_uij = U[i*numberValidPixels+j];
					belongsTo = i;
				}
			}
			class_average[belongsTo] += X[j];
			++cardinal[belongsTo];

		}
		cout<<"\tclass_average :";
		for (unsigned i = 0 ; i < numberClasses ; ++i)
		{
			cout<<class_average[i]/cardinal[i]<<"\t";
		}
		cout<<endl;
	
		#endif
	}

	
	#if defined(DEBUG) && DEBUG >= 3
	cout<<"--PCMClassifier::classification--END--"<<endl;
	#endif
	#if defined(DEBUG) && DEBUG >= 1
	feenableexcept(excepts);
	#endif
}


Real PCMClassifier::computeJ() const
{
	Real result = 0;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		Real sum1 = 0, sum2 = 0;

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{

			if(fuzzifier == 2)
				sum1 +=  U[i*numberValidPixels+j] * U[i*numberValidPixels+j] * d2(X[j],B[i]);
			else
				sum1 +=  pow(U[i*numberValidPixels+j], fuzzifier) * d2(X[j],B[i]);

			if(fuzzifier == 2)
				sum2 += (1 - U[i*numberValidPixels+j]) * (1 - U[i*numberValidPixels+j]);
			else
				sum2 +=  pow(1 - U[i*numberValidPixels+j], fuzzifier);

		}
		result += sum1 + (eta[i] * sum2);
	}
	return result;

}



Real PCMClassifier::assess(vector<Real>& V)
{
	V = vector<Real>(numberClasses, 0.);
	Real score = 0;

	//This is the vector of the min distances between the centers Bi and all the others centers Bii with ii!=i
	vector<Real> minDist(numberClasses, numeric_limits<Real>::max());
	//The min distance between all centers
	Real minDistBiBii = numeric_limits<Real>::max() ;

	Real distBiBii;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		for (unsigned ii = i + 1 ; ii < numberClasses ; ++ii)
		{
			distBiBii = d2(B[i],B[ii]);
			if(distBiBii < minDist[i])
				minDist[i] = distBiBii;
			if(distBiBii < minDist[ii])
				minDist[ii] = distBiBii;
		}
	}

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		Real sum1 = 0, sum2 = 0;

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{

			if(fuzzifier == 2)
				sum1 +=  U[i*numberValidPixels+j] * U[i*numberValidPixels+j] * d2(X[j],B[i]);
			else
				sum1 +=  pow(U[i*numberValidPixels+j], fuzzifier) * d2(X[j],B[i]);

			if(fuzzifier == 2)
				sum2 += (1 - U[i*numberValidPixels+j]) * (1 - U[i*numberValidPixels+j]);
			else
				sum2 +=  pow(1 - U[i*numberValidPixels+j], fuzzifier);

		}

		V[i] = sum1 + (eta[i] * sum2);
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberValidPixels);

	}

	score /= (minDistBiBii * numberValidPixels);
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
	#if defined(DEBUG) && DEBUG >= 1
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

	#if defined(DEBUG) && DEBUG >= 1
	if(X.size() == 0)
	{
		cerr<<"Error : The vector of FeatureVector must be initialized before doing a FCM init."<<endl;
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
	sort(B.begin(), B.end());
	FCMClassifier::computeU();
	fuzzifier = temp;
	//We initialise eta
	computeEta();

	// We output the FCM segementation for comparison with PCM 
	#if defined(DEBUG) && DEBUG >= 2
	string tempName = outputFileName;
	outputFileName += "FCM.";
	saveAllResults(NULL);
	outputFileName = tempName;
	#endif


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
			precisionReached = abs(oldEta[i] - eta[i]);
			if (precisionReached > precision)
				break;
		}
		cout<<"eta :"<<eta<<endl;
		oldEta = eta;
	}
	#endif
}



