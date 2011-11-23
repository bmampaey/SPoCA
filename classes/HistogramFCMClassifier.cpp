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
	FCMClassifier::sortB();
	FCMClassifier::computeU();
}

void HistogramFCMClassifier::addImages(vector<EUVImage*> images)
{

	if(images.size() < NUMBERCHANNELS)
	{
		cerr<<"Error : The number of images is not equal to "<<NUMBERCHANNELS<<endl;
		exit(EXIT_FAILURE);
	}
	else if(images.size() > NUMBERCHANNELS)
	{
		cerr<<"Warning : The number of images is not equal to "<<NUMBERCHANNELS<<". Only using the first ones."<<endl;
	}
	
	// We must verify that the channels of the histogram are the same as the channels of the classifier
	if(channels.size() > 0)
	{
		if(histoChannels.empty())
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
	}
	else 
	{
		if(histoChannels.empty())
		{
			histoChannels.resize(NUMBERCHANNELS);
			for (unsigned p = 0; p< NUMBERCHANNELS; ++p)
				histoChannels[p] = images[p]->Channel();
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
		HistogramFCMClassifier::stepinit(filenamePrefix+"iterations.txt");
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

		precisionReached = variation(oldB,B);
		oldB = B;

		#if DEBUG >= 2
			HistogramFCMClassifier::stepout(iteration, precisionReached, decimals);
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
	V.assign(numberClasses, 0.);
	Real score = 0;
	unsigned numberElements = 0;
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
				numberElements += xj->c;
			}
		}
	}
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberElements);

	}

	score /= (minDistBiBii * numberElements);

	return score;

}



#if MERGE_TYPE==MERGEMAX
/*!
Compute the new center by computing the mean value of the featurevector belonging to one of the 2 centers to be merged, weighted by the max value of the membership of the 2 centers.
A featurevector belong to a class if it's memebership is maximal for that class. 
The values of the membership are computed using the regular method for computing memebership with the new centers.
*/

void HistogramFCMClassifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			Real max_uij = 0;
			unsigned max_i = 0;
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				if (*uij > max_uij)
				{
					max_uij = *uij;
					max_i = i;
				}
			}
			// If it belongs to one of the 2 class I am merging, I update it's B
			if(max_i == i1 || max_i == i2)
			{
				Real uij_m = max_uij * max_uij * xj->c;
				newB += *xj * uij_m;
				sum += uij_m;
			}
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			Real max_uij = 0;
			unsigned max_i = 0;
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				if (*uij > max_uij)
				{
					max_uij = *uij;
					max_i = i;
				}
			}
			// If it belongs to one of the 2 class I am merging, I update it's B
			if(max_i == i1 || max_i == i2)
			{
				Real uij_m = pow(max_uij,fuzzifier) * xj->c;
				newB +=*xj * uij_m;
				sum += uij_m;

			}
		}
	}

	

	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif

	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;
	
	computeU();
}


#elif MERGE_TYPE==MERGECIS
/*!
The values of the membership are computed by taking the maximal membership value for the 2 classes to be merged.
The new center is computed using the regular method for computing centers with the new membership.

N.B. This method invalidate the constarint that the sum of membership for a feature vector is equal to 1 
*/
void HistogramFCMClassifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			if(*(uij+i1) < *(uij+i2))
				*(uij+i1) = *(uij+i2);
			
			Real uij_m = *(uij+i1) **(uij+i1) * xj->c;
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			// We search to which class belongs the featureVector 
			if(*(uij+i1) < *(uij+i2))
				*(uij+i1) = *(uij+i2);
			
			Real uij_m = pow(*(uij+i1),fuzzifier) * xj->c;
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	
	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif
	
	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;

}
#elif MERGE_TYPE==MERGESUM
/*!
The values of the membership are computed by taking the sum of the membership value for the 2 classes to be merged.
The new center is computed using the regular method for computing centers with the new membership.
*/
void HistogramFCMClassifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{ 
			*(uij+i1) += *(uij+i2);
			Real uij_m = *(uij+i1) **(uij+i1) * xj->c;
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	else
	{
		for (HistoFeatureVectorSet::iterator xj = HistoX.begin(); xj != HistoX.end(); ++xj)
		{
			*(uij+i1) += *(uij+i2);
			Real uij_m = pow(*(uij+i1),fuzzifier) * xj->c;
			newB += *xj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	
	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif

	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;

}
#endif

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

void HistogramFCMClassifier::initB(const vector<RealFeature>& B, const vector<string>& channels)
{
	if(histoChannels.empty())
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
	FCMClassifier::initB(B, channels);
}

/*! B is actually not randomly initialized, but is initialized with values spred over the range of the histogram. */
void HistogramFCMClassifier::randomInitB(unsigned C)
{
	#if DEBUG >= 1
	if(HistoX.size() == 0)
	{
		cerr<<"Error : The set of FeatureVector must be initialized before doing a random init."<<endl;
		exit(EXIT_FAILURE);

	}
	#endif
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



