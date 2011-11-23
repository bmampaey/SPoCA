#include "Classifier.h"

using namespace std;

Classifier::Classifier(Real fuzzifier)
:fuzzifier(fuzzifier),numberClasses(0),numberFeatureVectors(0),Xaxes(0),Yaxes(0)
{}

Classifier::~Classifier()
{
	if(stepfile.is_open())
		stepfile.close();
}

void Classifier::addImages(vector<EUVImage*> images)
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
	
	ordonateImages(images);
	unsigned numberPixelsEstimate = images[0]->NumberPixels();
	Xaxes = images[0]->Xaxes();
	Yaxes = images[0]->Yaxes();
	for (unsigned p = 1; p < NUMBERCHANNELS; ++p)
	{
		Xaxes = images[p]->Xaxes() < Xaxes ? images[p]->Xaxes() : Xaxes;
		Yaxes = images[p]->Yaxes() < Yaxes ? images[p]->Yaxes() : Yaxes;
		numberPixelsEstimate = images[p]->NumberPixels() > numberPixelsEstimate ? images[p]->NumberPixels() : numberPixelsEstimate;
	}

	//We initialise the valid pixels vector X
	X.reserve(numberPixelsEstimate);
	coordinates.reserve(numberPixelsEstimate);

	RealFeature f;
	bool validPixel;
	for (unsigned y = 0; y < Yaxes; ++y)
	{
		for (unsigned x = 0; x < Xaxes; ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERCHANNELS && validPixel; ++p)
			{
				f.v[p] = images[p]->pixel(x, y);
				if(f.v[p] == images[p]->null())
					validPixel=false;
			}
			if(validPixel)
			{
				coordinates.push_back(PixLoc(x,y));
				X.push_back(f);
			}

		}
	}

	numberFeatureVectors = X.size();

}

void Classifier::attribution()
{
	sortB();
	computeU();
}

unsigned Classifier::sursegmentation(unsigned Cmin)
{
	vector<Real> V;
	Real newScore = assess(V);
	Real oldScore = numeric_limits<Real>::max();
	vector<RealFeature> oldB; 

	#if DEBUG >= 3
	cout<<"--Classifier::sursegmentation--START--"<<endl;
	cout<<"B :\t"<<B<<endl;
	cout<<"V :\t"<<V<<"\tscore :"<<newScore<<endl;
	#endif
	

	for (unsigned C = numberClasses - 1; C >= Cmin; --C)
	{

		//We search for the max validity index (<=> worst center)
		unsigned indMax1 = 0, indMax2 = 0;
		Real max1Vi = 0;
		for (unsigned i = 0 ; i < numberClasses ; ++i)
		{
			if (V[i] > max1Vi)
			{
				indMax1 = i;
				max1Vi = V[i];
			}
		}
		//We look for it's worst neighboor
		if(indMax1 == 0)
		{
			indMax2 = 1;
		}
		else if(indMax1 == numberClasses -1)
		{
			indMax2 = numberClasses -2;
		}
		else
		{
			indMax2 = V[indMax1 - 1] > V[indMax1 + 1] ? indMax1 - 1 : indMax1 + 1;
		}

		oldB = B;
		//We merge
		merge(indMax1, indMax2);

		//And we re-asses
		oldScore = newScore;
		newScore = assess(V);

		#if DEBUG >= 2
		ColorMap segmentedMap;
		segmentedMap_maxUij(&segmentedMap);
		segmentedMap.writeFits(filenamePrefix + "segmented." + itos(numberClasses) + "classes.fits");
		#endif
		
		#if DEBUG >= 3
		cout<<"new B :"<<B<<endl;
		cout<<"V :"<<V<<"\tscore :"<<newScore<<endl;
		#endif

		/*	If we don't specify a min number of class to reach(i.e. Cmin == 0)
			then we stop when the oldScore is smaller than the newScore	*/
		if( Cmin == 0 && oldScore < newScore )
		{
			//We put back the old center, and we stop
			initB(oldB);
			attribution();
			break;

		}

	}

	#if DEBUG >= 3
	cout<<"--Classifier::sursegmentation--END--"<<endl;
	#endif

	return numberClasses;
}

unsigned Classifier::sursegmentation(vector<RealFeature>& B, unsigned Cmin)
{

	
	//We must be sure that some classification has been done
	initB(B);
	attribution();

	return sursegmentation(Cmin);

}

#if MERGE_TYPE==MERGEMAX
/*!
Compute the new center by computing the mean value of the featurevector belonging to one of the 2 centers to be merged.
A featurevector belong to a class if it's memebership is maximal for that class. 
The values of the membership are computed using the regular method for computing membership with the new centers.
*/
void Classifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	
	for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
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
			newB += *xj;
			sum += 1;
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


#elif MERGE_TYPE==MERGEVINCENT
/*!
Compute the new center by a strange method.
The values of the membership are computed using the regular method for computing membership with the new centers.
*/
void Classifier::merge(unsigned i1, unsigned i2)
{
	Real alpha = MERGEVINCENT_ALPHA;
	RealFeature newB[] = {0., 0.};
	Real caardinal[] = {0., 0.};
	MembershipSet::iterator uij = U.begin();
	for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj)
	{
		if(*(uij + i1) > alpha)
		{
			++caardinal[0];
			newB[0] += *xj * *(uij + i1);

		}
		if( *(uij + i2) > alpha)
		{
			++caardinal[1];
			newB[1] += *xj * *(uij + i2);

		}
		uij+=numberClasses;
	}

	newB[0] /= caardinal[0];
	newB[1] /= caardinal[1];

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB[0] + newB[1]<<endl;
	#endif

	B[i1] = newB[0] + newB[1];
	B.erase(B.begin()+i2);
	--numberClasses;
	
	computeU();
}

#else
/*!
Compute the new center by a taking the mean value of the 2 centers to be merged.
The values of the membership are computed using the regular method for computing membership with the new centers.
*/
void Classifier::merge(unsigned i1, unsigned i2)
{
	RealFeature newB = (B[i1] + B[i2]) / 2.;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif
	
	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;

	computeU();

}
#endif

ColorMap* Classifier::segmentedMap_maxUij(ColorMap* segmentedMap)
{

	#if DEBUG >= 1
	if (U.size() != numberClasses*numberFeatureVectors)
		cerr<<"The membership matrix U has not yet been calculated"<<endl;
	#endif
	
	if(segmentedMap)
	{
		segmentedMap->resize(Xaxes, Yaxes);
	}
	else
	{
		segmentedMap = new ColorMap(Xaxes, Yaxes);
	}
	
	segmentedMap->zero();
	segmentedMap->setNullValue(0);
	
	MembershipSet::iterator uij = U.begin();
	for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
	{
		Real max_uij = 0;
		ColorType color = 0;
		for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
		{
			if (*uij > max_uij)
			{
				max_uij = *uij;
				color = i + 1;
			}
		}
		segmentedMap->pixel(coordinates[j]) = color;
	}
	return segmentedMap;

}

ColorMap* Classifier::segmentedMap_closestCenter(ColorMap* segmentedMap)
{

	#if DEBUG >= 1
	if (U.size() != numberClasses*numberFeatureVectors)
		cerr<<"The membership matrix U has not yet been calculated"<<endl;
	#endif

	if(segmentedMap)
	{
		segmentedMap->resize(Xaxes, Yaxes);
	}
	else
	{
		segmentedMap = new ColorMap(Xaxes, Yaxes);
	}
	
	segmentedMap->zero();
	segmentedMap->setNullValue(0);
	
	for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
	{
		Real minDistance = distance_squared(X[j], B[0]);
		segmentedMap->pixel(coordinates[j]) = 1;
		for (unsigned i = 1 ; i < numberClasses ; ++i)
		{
			Real d2XjBi = distance_squared(X[j], B[i]);
			if (d2XjBi < minDistance)
			{
				minDistance = d2XjBi;
				segmentedMap->pixel(coordinates[j]) = i + 1;
			}
		}

	}
	return segmentedMap;

}

ColorMap* Classifier::segmentedMap_classThreshold(unsigned middleClass, Real lowerIntensity_minMembership, Real higherIntensity_minMembership, ColorMap* segmentedMap)
{

	#if DEBUG >= 1
	if (U.size() != numberClasses*numberFeatureVectors)
		cerr<<"The membership matrix U has not yet been calculated."<<endl;
	if (middleClass == 0 || middleClass > numberClasses)
	{
		cerr<<"The class number for threshold segmentation should be between 1 and numberClasses. It was set to: "<<middleClass<<endl;
		exit(EXIT_FAILURE);
	}
	if(lowerIntensity_minMembership < 0 || lowerIntensity_minMembership > 1)
	{
		cerr<<"The lowerIntensity minMembership must be a real between 0 and 1. It was set to: "<<lowerIntensity_minMembership<<endl;
		exit(EXIT_FAILURE);
	}
	if(higherIntensity_minMembership < 0 || higherIntensity_minMembership > 1)
	{
		cerr<<"The higherIntensity minMembership must be a real between 0 and 1. It was set to: "<<higherIntensity_minMembership<<endl;
		exit(EXIT_FAILURE);
	}
	#endif
	
	--middleClass;
	
	if(segmentedMap)
	{
		segmentedMap->resize(Xaxes, Yaxes);
	}
	else
	{
		segmentedMap = new ColorMap(Xaxes, Yaxes);
	}
	
	segmentedMap->zero();
	segmentedMap->setNullValue(0);
	
	for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
	{
		if(X[j] < B[middleClass])
		{
			if(U[j*numberClasses+middleClass] < lowerIntensity_minMembership)
				segmentedMap->pixel(coordinates[j]) = 1;
			else
				segmentedMap->pixel(coordinates[j]) = 2;
		}
		else
		{
			if(U[j*numberClasses+middleClass] < higherIntensity_minMembership)
				segmentedMap->pixel(coordinates[j]) = 3;
			else
				segmentedMap->pixel(coordinates[j]) = 2;
		}
	}
	return segmentedMap;

}


ColorMap* Classifier::segmentedMap_limits(vector<RealFeature>& limits,ColorMap* segmentedMap)
{

	segmentedMap_maxUij(segmentedMap);
	
	#if DEBUG >= 2
	segmentedMap->writeFits(filenamePrefix + "max.segmented.fits");
	#endif

	//We create a vector of transformation telling wich class must be merged to what class
	vector<ColorType> transfo(numberClasses + 1, 1);

	sort(limits.begin(), limits.end());
		
	for (unsigned i = 0; i < numberClasses; ++i)
		for (unsigned l = 0; l < limits.size() && limits[l] < B[i]; ++l)
			transfo[i + 1] = l + 2;

	transfo[0] = 0;
	
	for (unsigned j = 0 ; j < segmentedMap->NumberPixels() ; ++j)
	{
		segmentedMap->pixel(j) = transfo[segmentedMap->pixel(j)];
	}
	return segmentedMap;

}

ColorMap* Classifier::segmentedMap_fixed(vector<unsigned>& ch, vector<unsigned>& qs, vector<unsigned>& ar, ColorMap* segmentedMap)
{

	segmentedMap_maxUij(segmentedMap);

	#if DEBUG >= 2
	segmentedMap->writeFits(filenamePrefix + "max.segmented.fits");
	#endif

	//We create a vector of transformation telling wich class must be merged to what class
	vector<ColorType> transfo(numberClasses + 1, 0);

	for (unsigned i = 0; i < ch.size(); ++i)
		transfo[ch[i]] = 1;

	for (unsigned i = 0; i < qs.size(); ++i)
		transfo[qs[i]] = 2;
		
	for (unsigned i = 0; i < ar.size(); ++i)
		transfo[ar[i]] = 3;
		
	for (unsigned j = 0 ; j < segmentedMap->NumberPixels() ; ++j)
	{
		segmentedMap->pixel(j) = transfo[segmentedMap->pixel(j)];
	}
	return segmentedMap;

}



EUVImage* Classifier::fuzzyMap(const unsigned i, EUVImage* fuzzyMap)
{
	if(fuzzyMap)
	{
		fuzzyMap->resize(Xaxes, Yaxes);
	}
	else
	{
		fuzzyMap = new EUVImage(Xaxes, Yaxes);
	}
	
	fuzzyMap->zero();
	unsigned j = 0;
	for (MembershipSet::iterator uij = U.begin()+i; uij != U.end(); uij += numberClasses, ++j)
		fuzzyMap->pixel(coordinates[j]) = *uij;

	return fuzzyMap;
}


EUVImage* Classifier::normalizedFuzzyMap(const unsigned i, EUVImage* fuzzyMap)
{
	if(fuzzyMap)
	{
		fuzzyMap->resize(Xaxes, Yaxes);
	}
	else
	{
		fuzzyMap = new EUVImage(Xaxes, Yaxes);
	}
	
	fuzzyMap->zero();

	MembershipSet::iterator uij = U.begin();

	for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
	{
		Real sum = 0;
		for (unsigned k = 0 ; k < numberClasses ; ++k, ++uij)
		{
			sum += *uij;
		}
		fuzzyMap->pixel(coordinates[j]) = U[j*numberClasses+i] / sum;
	}

	return fuzzyMap;
}


void Classifier::saveB(const string& filename)
{
	ofstream centersFile(filename.c_str());
	if (centersFile.good())
	{
		centersFile<<channels<<"\t"<<B<<endl;
		centersFile.close();
	}
}


vector<RealFeature> Classifier::percentiles(vector<Real> percentileValues)
{

	vector<RealFeature> sortedX = X;
	vector<RealFeature> result;
	sort(sortedX.begin(), sortedX.end());
	for (unsigned h = 0; h < percentileValues.size(); ++h)
	{
		#if DEBUG >= 1
		if(percentileValues[h] < 0 || percentileValues[h] > 1)
		{
			cerr<<"Percentile values must be between 0 and 1"<<endl;
			exit(EXIT_FAILURE);
		}
		#endif
		result.push_back(sortedX[unsigned(percentileValues[h] * sortedX.size())]);

	}
	return result;
}


vector<RealFeature> Classifier::getB()
{
	return B;
}

vector<string> Classifier::getChannels()
{
	return channels;
}

EUVImage* Classifier::getImage(unsigned p)
{

	EUVImage* image = new EUVImage(Xaxes, Yaxes);
	image->zero();
	for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
	{
		image->pixel(coordinates[j]) = X[j].v[p];
	}

	return image;

}


void Classifier::randomInitB(unsigned C)
{
	#if DEBUG >= 1
	if(X.size() == 0)
	{
		cerr<<"Error : The set of FeatureVector must be initialized before doing a random init."<<endl;
		exit(EXIT_FAILURE);

	}
	#endif
	numberClasses = C;
	//We initialise the centers by setting each one randomly to one of the actual pixel. This is vincent's method!
	srand(unsigned(time(0)));
	B.resize(numberClasses);
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		B[i]=X[rand() % numberFeatureVectors];

	}
	//We like our centers to be sorted
	sort(B.begin(), B.end());
}


void Classifier::initB(const vector<RealFeature>& B)
{
	this->B = B;
	numberClasses = B.size();
}

void Classifier::initB(const vector<RealFeature>& B, const vector<string>& channels)
{
	this->B = B;
	numberClasses = B.size();
	this->channels = channels;
}

void Classifier::sortB()
{
	sort(B.begin(), B.end());
}

void Classifier::ordonateImages(vector<EUVImage*>& images)
{
	if(channels.size() == NUMBERCHANNELS)
	{
		for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
		{
			if(channels[p] != images[p]->Channel())
			{
				unsigned pp = p+1;
				while(pp < NUMBERCHANNELS && channels[p] != images[pp]->Channel())
					++pp;
				if(pp < NUMBERCHANNELS)
				{
					EUVImage* temp = images[pp];
					images[pp] = images[p];
					images[p] = temp;
				}
				else
				{

					cerr<<"Error : the wavelengths of the sun images provided do not match the wavelengths of the centers file!"<<endl;
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	else
	{
		channels.resize(NUMBERCHANNELS);
		for (unsigned p = 0; p < NUMBERCHANNELS; ++p)
			channels[p] = images[p]->Channel();
	}
	
}

// Computes the real average of each class
vector<RealFeature> Classifier::classAverage() const
{
	
	vector<RealFeature> class_average(numberClasses, 0.);
	vector<Real> cardinal(numberClasses, 0.);
	
	MembershipSet::const_iterator uij = U.begin();
	for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
	{
		Real max_uij = 0;
		unsigned belongsTo = 0;
		for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
		{
			if (*uij > max_uij)
			{
				max_uij = *uij;
				belongsTo = i;
			}
		}
		class_average[belongsTo] += X[j];
		++cardinal[belongsTo];
	}
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		class_average[i]/=cardinal[i];
	}
	return class_average;
}

void Classifier::stepinit(const string filename)
{
	if(stepfile.is_open())
		stepfile.close();
	
	stepfile.open(filename.c_str(), ios_base::app);
	if(!stepfile)
	{
		cerr<<"Error : could not open iterations file "<<filenamePrefix<<"iterations.txt !"<<endl;
	}
	ostringstream out;
	out<<"iteration"<<"\t"<<"precisionReached";
	for (unsigned i = 0; i < numberClasses; ++i)
		out<<"\t"<<"B"<<i;
	#if DEBUG >= 4
		for (unsigned i = 0; i < numberClasses; ++i)
			out<<"\t"<<"classAvg"<<i;
	#endif
	if(stepfile.good())
		stepfile<<endl<<out.str();
		
	#if DEBUG >= 3
		cout<<endl<<out.str();
	#endif
	
}

void Classifier::stepout(const unsigned iteration, const Real precisionReached, const int decimals)
{
		ostringstream out;
		out.setf(ios::fixed);
		out.precision(decimals);
		out<<iteration<<"\t"<<precisionReached<<"\t"<<B;
		#if DEBUG >= 4
			out<<"\t"<<classAverage();
		#endif
		if(stepfile.good())
			stepfile<<endl<<out.str();
		
		#if DEBUG >= 3
			cout<<endl<<out.str();
		#endif
		
}

Real Classifier::variation(const vector<RealFeature>& oldB, const vector<RealFeature>& newB) const
{
	Real maximalVariation = 0;
	for (unsigned i = 0 ; i < oldB.size() && i < newB.size() ; ++i)
	{
		Real normNewBi = norm(newB[i]);
		Real normOldBi = norm(oldB[i]);
		if(fabs(normOldBi) < numeric_limits<Real>::epsilon() )
			return numeric_limits<Real>::max();
		Real variationI = fabs(normNewBi - normOldBi)/normOldBi;
		if (variationI > maximalVariation)
		{
			maximalVariation = variationI;
		}
	}
	return maximalVariation;
}
