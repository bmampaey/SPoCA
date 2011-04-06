#include "HistogramFCMClassifier.h"

using namespace std;

HistogramFCMClassifier::HistogramFCMClassifier(Real fuzzifier)
:FCMClassifier(), HistogramClassifier()
{
	this->fuzzifier = fuzzifier;
}

HistogramFCMClassifier::HistogramFCMClassifier(const RealFeature& binSize, Real fuzzifier)
:FCMClassifier(), HistogramClassifier()
{
	this->fuzzifier = fuzzifier;
	initBinSize(binSize);
}

HistogramFCMClassifier::HistogramFCMClassifier(const std::string& histogramFilename, Real fuzzifier)
:FCMClassifier(), HistogramClassifier()
{
	this->fuzzifier = fuzzifier;
	initHistogram(histogramFilename);
}

void HistogramFCMClassifier::attribution()
{
	FCMClassifier::computeU();
}

void HistogramFCMClassifier::addImages(vector<EUVImage*> images)
{

	// We must verify that the channels of the histogram are the same as the channels of the classifier
	if(channels)
	{
		if(!histoChannels)
		{
			histoChannels = channels;
		}
		else if(histoChannels != channels)
		{
			cerr<<"Error : channels in the histogram file do not correspond to channels of the classifier (check centers file or order of the images)."<<endl;
			exit(EXIT_FAILURE);
		}
	}
	else 
	{
		if(!histoChannels)
		{
			for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
				histoChannels.v[p] = images[p]->Wavelength();
		}
		channels = histoChannels;
	}
		

	// I will need the images in the end to show the classification
	// so I add them to the FCM Classifier, and use it's Feture vectors to build the histogram
	FCMClassifier::addImages(images);
	addFeatures(X);

}



void HistogramFCMClassifier::computeB()
{

	Real sum, uij_m;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		B[i] = 0.;
		sum = 0;
		unsigned j = 0;
		for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj, ++j)
		{
			if (fuzzifier == 2)
				uij_m = U[i*numberBins+j] * U[i*numberBins+j] * xj->c;
			else
				uij_m = pow(U[i*numberBins+j],fuzzifier) * xj->c;

			B[i] += *xj * uij_m ;
			sum += uij_m;

		}

		B[i] /= sum;

	}
}


void HistogramFCMClassifier::computeU()
{

	Real sum;
	vector<Real> d2XjB(numberClasses);
	unsigned i;
	U.resize(numberBins * numberClasses);
	unsigned j = 0;
	for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj, ++j)
	{
		for (i = 0 ; i < numberClasses ; ++i)
		{
			d2XjB[i] = d2(*xj,B[i]);
			if (d2XjB[i] < precision)
				break;
		}
		if(i < numberClasses)	// The pixel is very close to B[i]
		{
			for (unsigned ii = 0 ; ii < numberClasses ; ++ii)
			{
				U[ii*numberBins+j] = 0.;
			}
			U[i*numberBins+j] = 1.;
		}
		else
		{
			for (i = 0 ; i < numberClasses ; ++i)
			{
				sum = 0;
				for (unsigned ii = 0 ; ii < numberClasses ; ++ii)
				{
					if (fuzzifier == 2)
						sum += (d2XjB[i]/d2XjB[ii]);
					else
						sum += pow(d2XjB[i]/d2XjB[ii],Real(1./(fuzzifier-1.)));

				}
				U[i*numberBins+j] = 1./sum;
			}

		}

	}

}


Real HistogramFCMClassifier::computeJ() const
{
	Real result = 0;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		unsigned j = 0;
		for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj, ++j)
		{

			if (fuzzifier == 2)
				result +=  U[i*numberBins+j] * xj->c * U[i*numberBins+j] * d2(*xj,B[i]) ;
			else
				result +=  pow(U[i*numberBins+j], fuzzifier) * xj->c * d2(*xj,B[i]) ;

		}
	}
	return result;

}


void HistogramFCMClassifier::classification(Real precision, unsigned maxNumberIteration)
{
	#if DEBUG >= 1
	if(HistoX.size() == 0 || B.size() == 0)
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if DEBUG >= 3
	cout<<"--HistogramFCMClassifier::classification--START--"<<endl;
	#endif
	
	#if DEBUG >= 2
		stepinit(outputFileName+"iterations.txt");
		unsigned decimals = unsigned(1 - log10(precision));;
	#endif

	//Initialisation of precision

	this->precision = precision;

	Real precisionReached = numeric_limits<Real>::max();
	vector<RealFeature> oldB = B;
	for (unsigned iteration = 0; iteration < maxNumberIteration && precisionReached > precision ; ++iteration)
	{
		HistogramFCMClassifier::computeU();
		HistogramFCMClassifier::computeB();

		for (unsigned i = 0 ; i < numberClasses ; ++i)
		{
			precisionReached = d2(oldB[i],B[i]);
			if (precisionReached > precision)
				break;

		}
		oldB = B;

		#if DEBUG >= 2
			stepout(iteration, precisionReached, decimals);
		#endif

	}

	#if DEBUG >= 3
	cout<<endl<<"--HistogramFCMClassifier::classification--END--"<<endl;
	#endif
	#if DEBUG >= 1
	feenableexcept(excepts);
	#endif
}


Real HistogramFCMClassifier::assess(vector<Real>& V)
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
		unsigned j = 0;
		for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj, ++j)
		{
			if (fuzzifier == 2)
				V[i] += d2(*xj,B[i]) * U[i*numberBins+j] * U[i*numberBins+j] * xj->c;
			else
				V[i] += d2(*xj,B[i]) * pow(U[i*numberBins+j],fuzzifier) * xj->c;

		}

		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberBins);

	}

	score /= (minDistBiBii * numberBins);

	return score;

}


#if MERGE==MERGEMAX
//We merge according to Benjamin's method
void HistogramFCMClassifier::merge(unsigned i1, unsigned i2)
{

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2];
	#endif

	Real max_uij, uij_m, sum = 0;
	unsigned max_i;
	B[i1] = 0;
	unsigned j = 0;
	for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj, ++j)
	{
		max_uij = 0;
		max_i = 0;
		for (unsigned i = 0 ; i < numberClasses ; ++i)
			if (U[i*numberBins+j] > max_uij)
		{
			max_uij = U[i*numberBins+j];
			max_i = i;
		}
		if(max_i == i1 || max_i == i2)
		{
			if (fuzzifier == 2)
				uij_m = max_uij * max_uij * xj->c;
			else
				uij_m = pow(max_uij,fuzzifier) * xj->c;

			B[i1] += *xj * uij_m;
			sum += uij_m;

		}

	}

	B[i1] /= sum;

	#if DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;

	computeU();
}


#elif MERGE==MERGECIS
//We merge according to Cis's method
void HistogramFCMClassifier::merge(unsigned i1, unsigned i2)
{

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2];
	#endif

	Real uij_m, sum = 0;
	B[i1] = 0;
	unsigned j = 0;
	for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj, ++j)
	{
		if(U[i1*numberBins+j] < U[i2*numberBins+j])
			U[i1*numberBins+j] = U[i2*numberBins+j];

		if (fuzzifier == 2)
			uij_m = U[i1*numberBins+j] * U[i1*numberBins+j] * xj->c;
		else
			uij_m = pow(U[i1*numberBins+j],fuzzifier) * xj->c;

		B[i1] += *xj * uij_m;
		sum += uij_m;

	}

	B[i1] /= sum;

	#if DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;
	U.erase(U.begin() + i2 * numberBins, U.begin() + (i2 + 1)  * numberBins);
}
#endif

// Computes the real average of each class
vector<RealFeature> HistogramFCMClassifier::classAverage() const
{
	
	vector<RealFeature> class_average(numberClasses, 0.);
	vector<Real> cardinal(numberClasses, 0.);
	unsigned j = 0;
	for (set<HistoPixelFeature>::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj, ++j)
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
		class_average[belongsTo] += *xj;
		cardinal[belongsTo]+=xj->c;

	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		class_average[i]/=cardinal[i];
	}
	return class_average;
}

void HistogramFCMClassifier::initB(const vector<RealFeature>& B, const RealFeature& channels)
{
	if(!histoChannels)
	{
		histoChannels = channels;
	}
	else if(histoChannels != channels)
	{
		cerr<<"Error : channels in the histogram file do not correspond to channels of the classifier (check centers file or order of the images)."<<endl;
		exit(EXIT_FAILURE);
	}
	FCMClassifier::initB(B, channels);
}

/*! B is actually not randomly initialized, but is initialized with values spred over the range of the histogram. */
void HistogramFCMClassifier::randomInitB(unsigned C)
{
	#if DEBUG >= 1
	if(HistoX.size() == 0)
	{
		cerr<<"Error : The vector of FeatureVector must be initialized before doing a random init."<<endl;
		exit(EXIT_FAILURE);

	}
	#endif
	numberClasses = C;
	srand(unsigned(time(0)));
	B.resize(numberClasses);
	set<HistoPixelFeature>::iterator xj = HistoX.begin();
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		unsigned delta = unsigned((i+0.5)*(Real(numberBins)/numberClasses));
		for (unsigned j = 0; j < delta; ++j)
			++xj;
		B[i]=*xj;

	}
	//We like our centers to be sorted
	sort(B.begin(), B.end());
}



