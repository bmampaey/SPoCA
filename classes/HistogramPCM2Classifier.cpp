#include "HistogramPCM2Classifier.h"

using namespace std;

HistogramPCM2Classifier::HistogramPCM2Classifier(Real fuzzifier)
:HistogramPCMClassifier(), PCM2Classifier()
{
	this->fuzzifier = fuzzifier;
}

HistogramPCM2Classifier::HistogramPCM2Classifier(const RealFeature& binSize, Real fuzzifier)
:HistogramPCMClassifier(), PCM2Classifier()
{
	this->fuzzifier = fuzzifier;
	initBinSize(binSize);
}

HistogramPCM2Classifier::HistogramPCM2Classifier(const std::string& histogramFilename, Real fuzzifier)
:HistogramPCMClassifier(), PCM2Classifier()
{
	this->fuzzifier = fuzzifier;
	initHistogram(histogramFilename);
}


//Because we need to use the value fund for B to classify the normal images
void HistogramPCM2Classifier::saveAllResults(SunImage* outImage)
{
	PCM2Classifier::computeU();
	PCM2Classifier::saveAllResults(outImage);
}


void HistogramPCM2Classifier::saveARmap(SunImage* outImage)
{
	PCM2Classifier::computeU();
	PCM2Classifier::saveARmap(outImage);
}

void HistogramPCM2Classifier::saveCHmap(SunImage* outImage)
{
	PCM2Classifier::computeU();
	PCM2Classifier::saveCHmap(outImage);
}



void HistogramPCM2Classifier::computeU()
{
	U.resize(numberBins * numberClasses);
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		for (unsigned j = 0 ; j < numberBins ; ++j)
		{
			U[i*numberBins+j] = d2(HistoX[j],B[i]) / eta[i];
			if(fuzzifier == 1.5)
			{
				U[i*numberBins+j] *= U[i*numberBins+j];
				U[i*numberBins+j] *= U[i*numberBins+j];
			}
			else if(fuzzifier == 2)
			{
				U[i*numberBins+j] *= U[i*numberBins+j];
				
			}
			else
			{
				U[i*numberBins+j] = pow( U[i*numberBins+j] , 2./(fuzzifier-1.) ) ;
			}
				
			U[i*numberBins+j] = 1. / (1. + U[i*numberBins+j] ) ;

		}

	}


}


// VERSION WITH LIMITED VARIATION OF ETA W.R.T. ITS INITIAL VALUE

void HistogramPCM2Classifier::classification(Real precision, unsigned maxNumberIteration)
{

	#if defined(DEBUG) && DEBUG >= 1
	if(HistoX.size() == 0 || B.size() == 0 || B.size() != eta.size())
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if defined(DEBUG) && DEBUG >= 3
	cout<<"--HistogramPCM2Classifier::classification--START--"<<endl;
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

		#if defined(DEBUG) && DEBUG >= 3
		cout<<"iteration :"<<iteration;
		cout<<"\tprecisionReached :"<<precisionReached;
		#if DEBUG >= 4
			cout<<"\tJPCM :"<<computeJ();
		#endif
		cout<<"\tB :"<<B;
		cout<<"\teta :"<<eta;		
		cout<<"\tclass_average :"<<classAverage();
		cout<<endl;
	
		#endif
	}
	


	#if defined(DEBUG) && DEBUG >= 3
	cout<<"--HistogramPCM2Classifier::classification--END--"<<endl;
	#endif
	#if defined(DEBUG) && DEBUG >= 1
	feenableexcept(excepts);
	#endif
}



void HistogramPCM2Classifier::computeEta()
{
	HistogramPCMClassifier::computeEta();

	#if defined(DEBUG) && DEBUG >= 3
		cout<<"pre_eta:\t"<<eta<<"\t";
	#endif

	PCM2Classifier::reduceEta();

}
