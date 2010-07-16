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


//Because we need to use the value fund for B to classify the normal images
void HistogramPCMClassifier::saveAllResults(SunImage* outImage)
{
	PCMClassifier::computeU();
	PCMClassifier::saveAllResults(outImage);
}


void HistogramPCMClassifier::saveARmap(SunImage* outImage)
{

	PCMClassifier::computeU();
	PCMClassifier::saveARmap(outImage);
}

void HistogramPCMClassifier::saveCHmap(SunImage* outImage)
{

	PCMClassifier::computeU();
	PCMClassifier::saveCHmap(outImage);
}


void HistogramPCMClassifier::computeU()
{
	U.resize(numberBins * numberClasses);
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		for (unsigned j = 0 ; j < numberBins ; ++j)
		{

			U[i*numberBins+j] = d2(HistoX[j],B[i]) / eta[i] ;
			if(fuzzifier == 1.5)
			{
				U[i*numberBins+j] *=  U[i*numberBins+j];
			}
			else if(fuzzifier != 2)
			{
				U[i*numberBins+j] = pow( U[i*numberBins+j] , 1./(fuzzifier-1.) );
			}
			
			U[i*numberBins+j] = 1. / (1. + U[i*numberBins+j]);

		}

	}

}


void HistogramPCMClassifier::computeEta()
{

	eta = vector<Real>(numberClasses,0.);

	Real sum, uij_m;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		sum = 0;

		for (unsigned j = 0 ; j < numberBins ; ++j)
		{

			if(fuzzifier == 2)
				uij_m = U[i*numberBins+j] * HistoX[j].c * U[i*numberBins+j] ;
			else
				uij_m = pow(U[i*numberBins+j],fuzzifier) * HistoX[j].c;

			eta[i] += uij_m*d2(HistoX[j],B[i]);
			sum += uij_m;

		}
		eta[i] /= sum;
	}

}


void HistogramPCMClassifier::computeEta(Real alpha)
{
	//This is the other method to calculate eta, descibed by Krishnapuram and Keller. It is a little faster by the way
	eta = vector<Real>(numberClasses,0.);

	Real sum;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		sum = 0;
		for (unsigned j = 0 ; j < numberBins ; ++j)
		{
			if (U[i*numberBins+j]>alpha)
			{
				eta[i] += d2(HistoX[j],B[i]) * HistoX[j].c;
				sum +=  HistoX[j].c;
			}

		}

		eta[i] /= sum;

	}
}


Real HistogramPCMClassifier::computeJ() const
{
	Real result = 0;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		Real sum1 = 0, sum2 = 0;

		for (unsigned j = 0 ; j < numberBins ; ++j)
		{

			if(fuzzifier == 2)
				sum1 +=  U[i*numberBins+j] * U[i*numberBins+j] * d2(HistoX[j],B[i]) * HistoX[j].c;
			else
				sum1 +=  pow(U[i*numberBins+j], fuzzifier) * d2(HistoX[j],B[i]) * HistoX[j].c;

			if(fuzzifier == 2)
				sum2 += (1 - U[i*numberBins+j]) * (1 - U[i*numberBins+j]) * HistoX[j].c;
			else
				sum2 +=  pow(1 - U[i*numberBins+j], fuzzifier) * HistoX[j].c;

		}
		result += sum1 + (eta[i] * sum2);
	}
	return result;

}



// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE

void HistogramPCMClassifier::classification(Real precision, unsigned maxNumberIteration)
{

	#if defined(DEBUG) && DEBUG >= 1
	if(HistoX.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	#endif

	#if defined(DEBUG) && DEBUG >= 3
	cout<<"--HistogramPCMClassifier::classification--START--"<<endl;
	#endif

	const Real maxFactor = ETA_MAXFACTOR;

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
		for (unsigned j = 0 ; j < numberBins ; ++j)
		{
			Real max_uij = U[j];
			unsigned belongsTo = 0;
			for (unsigned i = 1 ; i < numberClasses ; ++i)
			{
				if (U[i*numberBins+j] > max_uij)
				{
					max_uij = U[i*numberBins+j];
					belongsTo = i;
				}
			}
			class_average[belongsTo] += HistoX[j];
			cardinal[belongsTo]+=HistoX[j].c;

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
	cout<<"--HistogramPCMClassifier::classification--END--"<<endl;
	#endif
}




Real HistogramPCMClassifier::assess(vector<Real>& V)
{
	V = vector<Real>(numberClasses, 0.);
	Real score = 0;

	//This is the vector of the min distances between the centers Bi and all the others centers Bii with ii!=i
	vector<Real> minDist(numberClasses, numeric_limits<Real>::max());
	//The min distance between all centers
	Real minDistBiBii = numeric_limits<Real>::max() ;

	Real distBiBii;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		for (unsigned ii = i + 1 ; ii < numberClasses ; ++ii)
	{
		distBiBii = d2(B[i],B[ii]);
		if(distBiBii < minDist[i])
			minDist[i] = distBiBii;
		if(distBiBii < minDist[ii])
			minDist[ii] = distBiBii;
	}

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		Real sum1 = 0, sum2 = 0;

		for (unsigned j = 0 ; j < numberBins ; ++j)
		{

			if(fuzzifier == 2)
				sum1 +=  U[i*numberBins+j] * U[i*numberBins+j] * d2(HistoX[j],B[i]) * HistoX[j].c;
			else
				sum1 +=  pow(U[i*numberBins+j], fuzzifier) * d2(HistoX[j],B[i]) * HistoX[j].c;

			if(fuzzifier == 2)
				sum2 += (1 - U[i*numberBins+j]) * (1 - U[i*numberBins+j]) * HistoX[j].c;
			else
				sum2 +=  pow(1 - U[i*numberBins+j], fuzzifier) * HistoX[j].c;

		}

		V[i] = sum1 + (eta[i] * sum2);
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberBins);

	}

	score /= (minDistBiBii * numberBins);
	return score;

}




void HistogramPCMClassifier::FCMinit(Real precision, unsigned maxNumberIteration, Real FCMfuzzifier)
{

	#if defined(DEBUG) && DEBUG >= 1
	if(HistoX.size() == 0)
	{
		cerr<<"Error : The vector of FeatureVector must be initialized before doing a centers only init."<<endl;
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
	sort(B.begin(), B.end());
	HistogramFCMClassifier::computeU();
	fuzzifier = temp;
	//We initialise eta
	computeEta();
	
	// We output the FCM segementation for comparison with PCM 
	#if defined(DEBUG) && DEBUG >= 2
	string tempName = outputFileName;
	outputFileName += "HFCM.";
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



