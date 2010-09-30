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

void HistogramFCMClassifier::addImages(std::vector<SunImage*>& images)
{

	for (unsigned p = 0; p <  NUMBERWAVELENGTH; ++p)
	{
		if( binSize.v[p] == 0 )
		{
			cerr<<"binSize cannot be 0."<<endl;
			exit(EXIT_FAILURE);
		}
	}
	
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
			for (unsigned p = 0; p< NUMBERWAVELENGTH; ++p)
				histoChannels.v[p] = images[p]->Wavelength();
		}
		channels = histoChannels;
	}
		

	//I will need the images in the end to show the classification anyway
	FCMClassifier::addImages(images);
	HistoPixelFeature xj;
	for (unsigned j = 0; j < numberValidPixels; ++j)
	{
		for (unsigned p = 0; p <  NUMBERWAVELENGTH; ++p)
		{
			xj.v[p] = (int(X[j].v[p]/binSize.v[p]) * binSize.v[p]) + ( binSize.v[p] / 2 );
		}

		unsigned pos = insert(xj);
		++HistoX[pos].c;
	}

	numberBins = HistoX.size();
	
	#if DEBUG >= 2
	saveHistogram(outputFileName + "histogram.txt");
	#endif

}



void HistogramFCMClassifier::computeB()
{

	Real sum, uij_m;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		B[i] = 0.;
		sum = 0;
		for (unsigned j = 0 ; j < numberBins ; ++j)
		{
			if (fuzzifier == 2)
				uij_m = U[i*numberBins+j] * U[i*numberBins+j] * HistoX[j].c;
			else
				uij_m = pow(U[i*numberBins+j],fuzzifier) * HistoX[j].c;

			B[i] += HistoX[j] * uij_m ;
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
	for (unsigned j = 0 ; j < numberBins ; ++j)
	{
		for (i = 0 ; i < numberClasses ; ++i)
		{
			d2XjB[i] = d2(HistoX[j],B[i]);
			if (d2XjB[i] < precision)
				break;
		}
		if(i < numberClasses)					  // The pixel is very close to B[i]
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
						sum += pow(d2XjB[i]/d2XjB[ii],1./(fuzzifier-1.));

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

		for (unsigned j = 0 ; j < numberBins ; ++j)
		{

			if (fuzzifier == 2)
				result +=  U[i*numberBins+j] * HistoX[j].c * U[i*numberBins+j] * d2(HistoX[j],B[i]) ;
			else
				result +=  pow(U[i*numberBins+j], fuzzifier) * HistoX[j].c * d2(HistoX[j],B[i]) ;

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
		for (unsigned j = 0 ; j < numberBins ; ++j)
		{
			if (fuzzifier == 2)
				V[i] += d2(HistoX[j],B[i]) * U[i*numberBins+j] * U[i*numberBins+j] * HistoX[j].c;
			else
				V[i] += d2(HistoX[j],B[i]) * pow(U[i*numberBins+j],fuzzifier) * HistoX[j].c;

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
	for (unsigned j = 0 ; j < numberBins ; ++j)
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
				uij_m = max_uij * max_uij * HistoX[j].c;
			else
				uij_m = pow(max_uij,fuzzifier) * HistoX[j].c;

			B[i1] += HistoX[j] * uij_m;
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
	for (unsigned j = 0 ; j < numberBins ; ++j)
	{
		if(U[i1*numberBins+j] < U[i2*numberBins+j])
			U[i1*numberBins+j] = U[i2*numberBins+j];

		if (fuzzifier == 2)
			uij_m = U[i1*numberBins+j] * U[i1*numberBins+j] * HistoX[j].c;
		else
			uij_m = pow(U[i1*numberBins+j],fuzzifier) * HistoX[j].c;

		B[i1] += HistoX[j] * uij_m;
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
	//We initialise the centers by setting each one randomly to one of the actual pixel. This is vincent's method!
	srand(unsigned(time(0)));
	B.resize(numberClasses);
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		B[i]=HistoX[rand() % numberBins];

	}
	//We like our centers to be sorted
	sort(B.begin(), B.end());
}



