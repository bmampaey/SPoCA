#include "HistogramFCMClassifier.h"

using namespace std;

HistogramFCMClassifier::HistogramFCMClassifier(Real fuzzifier, unsigned numberClasses, Real precision, unsigned maxNumberIteration, const RealFeature& binSize)
:FCMClassifier(fuzzifier, numberClasses, precision, maxNumberIteration), HistogramClassifier(binSize)
{
	#if defined DEBUG
	cout<<"Called HFCM constructor"<<endl;
	#endif
}

HistogramFCMClassifier::HistogramFCMClassifier(ParameterSection& parameters)
:FCMClassifier(parameters)
{
	if (parameters["histogramFilename"].is_set())
		initHistogram(parameters["histogramFilename"]);
	else
		initBinSize(parameters["binSize"]);
	
	
	#if defined DEBUG
	cout<<"Called HFCM constructor with parameter section"<<endl;
	#endif
}

void HistogramFCMClassifier::attribution()
{
	FCMClassifier::sortB();
	FCMClassifier::computeU();
}

void HistogramFCMClassifier::addImages(vector<EUVImage*> images)
{
	if(images.size() != NUMBERCHANNELS)
	{
		cerr<<"Error : The number of images is not equal to "<<NUMBERCHANNELS<<endl;
		exit(EXIT_FAILURE);
	}
	
	// We must verify that the channels of the histogram are the same as the channels of the classifier
	if(channels.empty() && histoChannels.empty())
	{
		for(unsigned p = 0; p < images.size(); ++p)
			channels.push_back(images[p]->Channel());
		histoChannels = channels;
	}
	else if(histoChannels.empty())
	{
		histoChannels = channels;
	}
	else if(channels.empty())
	{
		channels = histoChannels;
	}
	else if(histoChannels.size() != channels.size())
	{
		cerr<<"Error : number of channels of the histogram differ from the number of channels of the classifier, check centers file or histogram file."<<endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		for(unsigned p = 0; p < channels.size(); ++p)
		{
			if(histoChannels[p] != channels[p])
			{
				cerr<<"Error : channel "<<p<<" of the histogram ("<<histoChannels[p]<<") differ from classifier ("<<channels[p]<<"), check centers file or histogram file."<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	
	// I will need the images in the end to show the classification
	// so I add them to the FCM Classifier, and use it's Feture vectors to build the histogram
	FCMClassifier::addImages(images);
	addFeatures(X);

}


void HistogramFCMClassifier::computeB()
{
	B.assign(numberClasses, 0.);
	vector<Real> sum(numberClasses, 0.);
	
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = *uij * *uij * xj->c;
				B[i] += *xj * uij_m;
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
				B[i] += *xj * uij_m;
				sum[i] += uij_m;
			}
		}
	}
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		B[i] /= sum[i];
}


void HistogramFCMClassifier::computeU()
{
	vector<Real> d2XjB(numberClasses);
	U.resize(numberBins * numberClasses);
	
	unsigned i;
	MembershipSet::iterator uij = U.begin();
	for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
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
				*uij = i != ii ? 0. : 1.;
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


Real HistogramFCMClassifier::computeJ() const
{
	Real result = 0;
	MembershipSet::const_iterator uij = U.begin();
	if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result +=  *uij * *uij * distance_squared(*xj,B[i]);
			}
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				result +=  pow(*uij, fuzzifier) * distance_squared(*xj,B[i]);
			}
		}
	}

	return result;
}

void HistogramFCMClassifier::classification()
{
	if(HistoX.size() == 0 || B.size() == 0)
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);
	
	}
	
	#if defined EXTRA_SAFE
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if defined VERBOSE
	cout<<"--HistogramFCMClassifier::classification--START--"<<endl;
	#endif
	
	HistogramFCMClassifier::stepinit(filenamePrefix+"iterations.txt");
	
	//Initialisation of precision

	this->precision = precision;

	Real precisionReached = numeric_limits<Real>::max();
	vector<RealFeature> oldB = B;
	for (unsigned iteration = 0; iteration < maxNumberIteration && precisionReached > precision ; ++iteration)
	{
		HistogramFCMClassifier::computeU();
		HistogramFCMClassifier::computeB();

		precisionReached = variation(oldB,B);
		oldB = B;
	
		HistogramFCMClassifier::stepout(iteration, precisionReached, precision);
	}
	
	#if defined VERBOSE
	cout<<endl<<"--HistogramFCMClassifier::classification--END--"<<endl;
	#endif
	
	#if defined EXTRA_SAFE
	feenableexcept(excepts);
	#endif
}

// Computes the real average of each class
vector<RealFeature> HistogramFCMClassifier::classAverage() const
{
	
	vector<RealFeature> class_average(numberClasses, 0.);
	vector<Real> cardinal(numberClasses, 0.);
	
	MembershipSet::const_iterator uij = U.begin();
	for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
	{
		Real max_uij = *uij;
		unsigned belongsTo = 0;
		for (unsigned i = 1 ; i < numberClasses ; ++i, ++uij)
		{
			if (*uij > max_uij)
			{
				max_uij = *uij;
				belongsTo = i;
			}
		}
		class_average[belongsTo] += *xj;
		cardinal[belongsTo]+=xj->c;
	}
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		class_average[i]/=cardinal[i];
	}
	return class_average;
}

void HistogramFCMClassifier::initB(const vector<string>& channels, const vector<RealFeature>& B)
{
	// We verify and set the classifier channels
	if(channels.size() != NUMBERCHANNELS)
	{
		cerr<<"Error : The number of channels is not correct."<<endl;
		exit(EXIT_FAILURE);
	}
	if(HistoX.empty())
	{
		histoChannels = channels;
	}
	else if(histoChannels.size() != channels.size())
	{
		cerr<<"Error : number of channels of the histogram differ from the number of channels of the classifier, check centers file or histogram file."<<endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		for(unsigned p = 0; p < channels.size(); ++p)
		{
			if(histoChannels[p] != channels[p])
			{
				cerr<<"Error : channel "<<p<<" of the histogram ("<<histoChannels[p]<<") differ from classifier ("<<channels[p]<<"), check centers file or histogram file."<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	FCMClassifier::initB(channels, B);
}

/*! B is actually not randomly initialized, but is initialized with values spred over the range of the histogram. */
void HistogramFCMClassifier::randomInitB(unsigned C)
{
	if(HistoX.size() == 0)
	{
		cerr<<"Error : The set of FeatureVector must be initialized before doing a random init."<<endl;
		exit(EXIT_FAILURE);
	}
	
	numberClasses = C;
	srand(unsigned(time(0)));
	B.resize(numberClasses);
	HistoFeatureVectorSet::iterator xj = HistoX.begin();
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		unsigned delta = unsigned((i+0.5)*(Real(numberBins)/numberClasses));
		for (unsigned j = 0; j < delta; ++j)
			++xj;
		B[i]=*xj;
	}
	
	//We like our centers to be sorted
	FCMClassifier::sortB();
}

void HistogramFCMClassifier::fillHeader(Header& header)
{
	FCMClassifier::fillHeader(header);
	header.set("CBINSIZE", binSize, "Histogram bin size");
	header.set("CNBRBINS", numberBins, "Histogram number of bin");
}


