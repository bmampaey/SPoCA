#include "Classifier.h"

using namespace std;

Classifier::Classifier(Real fuzzifier, unsigned numberClasses, Real precision, unsigned maxNumberIteration)
:fuzzifier(fuzzifier),numberClasses(numberClasses),precision(precision), maxNumberIteration(maxNumberIteration),numberFeatureVectors(0),Xaxes(0),Yaxes(0)
{
	#if defined DEBUG
	cout<<"Called Classifier constructor"<<endl;
	#endif
}

Classifier::Classifier(ParameterSection& parameters)
:fuzzifier(parameters["fuzzifier"]),numberClasses(parameters["numberClasses"]),precision(parameters["precision"]), maxNumberIteration(parameters["maxNumberIteration"]),numberFeatureVectors(0),Xaxes(0),Yaxes(0)
{
	#if defined DEBUG
	cout<<"Called Classifier constructor with parameter section"<<endl;
	#endif
}

Classifier::~Classifier()
{
	if(stepfile.is_open())
		stepfile.close();
}

void Classifier::addImages(vector<EUVImage*> images)
{
	// We verify and set the classifier channels
	if(images.size() != NUMBERCHANNELS)
	{
		cerr<<"Error : The number of images is not equal to "<<NUMBERCHANNELS<<endl;
		exit(EXIT_FAILURE);
	}
	if(channels.empty())
	{
		for(unsigned p = 0; p < images.size(); ++p)
			channels.push_back(images[p]->Channel());
	}
	else
	{
		if(images.size() != channels.size())
		{
			cerr<<"Error : The number of images is not equal to "<<NUMBERCHANNELS<<endl;
			exit(EXIT_FAILURE);
		}
		for(unsigned p = 0; p < channels.size(); ++p)
		{
			if(channels[p] != images[p]->Channel())
			{
				cerr<<"Error : The images channels do not correspond to classifier channels."<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
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
	X.reserve(numberPixelsEstimate * 1.1);
	coordinates.reserve(numberPixelsEstimate * 1.1);

	RealFeature f;
	bool validPixel;
	for (unsigned y = 0; y < Yaxes; ++y)
	{
		for (unsigned x = 0; x < Xaxes; ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p < NUMBERCHANNELS && validPixel; ++p)
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
	#if defined DEBUG || defined WRITE_MEMBERSHIP_FILES
	// We write the fits file of Uij
	Image<EUVPixelType> image(Xaxes,Yaxes);
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		image.zero();
		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
			image.pixel(coordinates[j]) = U[j*numberClasses + i];
		image.writeFits(filenamePrefix + "membership.final.class_" + toString(i) + ".fits");
	}
	#endif
}

ColorMap* Classifier::getSegmentedMap(ParameterSection& parameters, ColorMap* segmentedMap)
{
	// We do the segmentation
	if (parameters["type"] == "max")
	{
		segmentedMap_maxUij(segmentedMap);
	}
	else if (parameters["type"] == "closest")
	{
		segmentedMap_closestCenter(segmentedMap);
	}
	else if (parameters["type"] == "threshold")
	{
		if(!parameters["thresholds"].is_set())
		{
			cerr<<"Error : For threshold segmentation the thresholds parameter is mandatory."<<endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			vector<Real> thresholds = parameters["thresholds"];
			segmentedMap_classThreshold(thresholds[0], thresholds[1], thresholds[2], segmentedMap);
		}
	}
	else if (parameters["type"] == "limits")
	{
		if(!parameters["maxLimits"].is_set())
		{
			cerr<<"Error : For limits segmentation the limits parameter is mandatory."<<endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			vector<RealFeature> limits = parameters["limits"];
			segmentedMap_limits(limits, segmentedMap);
		}
	}
	else if (parameters["type"] == "fix")
	{
		vector<unsigned> CH, QS, AR;
		if(parameters["CH"].is_set())
		{
			CH = parameters["CH"];
		}
		if(parameters["QS"].is_set())
		{
			QS = parameters["QS"];
		}
		if(parameters["AR"].is_set())
		{
			AR = parameters["AR"];
		}
		segmentedMap_fixed(CH, QS, AR, segmentedMap);
	}
	else 
	{
		cerr<<"Error : "<<parameters["type"]<<" is not a known segmentation type!"<<endl;
		exit(EXIT_FAILURE);
	}
	
	//We add information about the classification to the header of the segmented map
	Header& header = segmentedMap->getHeader();
	fillHeader(header);
	
	//We add information about the segmentation to the header of the segmented map
	header.set("INSTRUME", "Segmentation Map", "SPoCA segmentation map");
	header.set("SEGMTYPE", parameters["type"], "Segmentation type");
	if (parameters["type"] == "fix")
	{
		if(parameters["CH"].is_set())
			header.set("SFIXCH", str(parameters["CH"]));
		if(parameters["QS"].is_set())
			header.set("SFIXQS", str(parameters["QS"]));
		if(parameters["AR"].is_set())
			header.set("SFIXAR", str(parameters["AR"]));
	}
	else if (parameters["type"] == "threshold")
		header.set("STRSHLD", str(parameters["thresholds"]));
	else if (parameters["type"] == "limits")
		header.set("SLIMITS", str(parameters["limits"]));
	
	return segmentedMap;
}

void Classifier::fillHeader(Header& header)
{
	header.set("TELESCOP", "SPoCA", "This map was computed with SPoCA");
	header.set("CNBRCLAS", numberClasses, "Number Classes");
	header.set("CPRECIS", precision, "Classifier Precision");
	header.set("CMAXITER", maxNumberIteration, "Maximum Number of Iteration");
	header.set("CFUZFIER", fuzzifier, "Classifier Fuzzifier");
	header.set("CHANNELS", toString(getChannels()), "Classification Channels");
	
	B = getB();
	for (unsigned i = 0; i < numberClasses; ++i)
		header.set("CLSCTR"+toString(i+1,2), toString(B[i], 4), "Classification class center " + toString(i+1,2));
}

ColorMap* Classifier::segmentedMap_maxUij(ColorMap* segmentedMap)
{
	if (U.size() != numberClasses*numberFeatureVectors)
	{
		cerr<<"The membership matrix U has not yet been calculated"<<endl;
		exit(EXIT_FAILURE);
	}
	
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
	if (U.size() != numberClasses*numberFeatureVectors)
	{
		cerr<<"The membership matrix U has not yet been calculated"<<endl;
		exit(EXIT_FAILURE);
	}
	
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
	if (U.size() != numberClasses*numberFeatureVectors)
	{
		cerr<<"The membership matrix U has not yet been calculated"<<endl;
		exit(EXIT_FAILURE);
	}
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


ColorMap* Classifier::segmentedMap_limits(vector<RealFeature>& limits, ColorMap* segmentedMap)
{

	segmentedMap_maxUij(segmentedMap);
	
	#if defined DEBUG
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

	#if defined DEBUG
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
	if(X.size() == 0)
	{
		cerr<<"Error : The set of FeatureVector must be initialized before doing a random init."<<endl;
		exit(EXIT_FAILURE);
	}
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

void Classifier::initB(const std::vector<std::string>& channels, const std::vector<RealFeature>& B)
{
	// We verify and set the classifier channels
	if(channels.size() != NUMBERCHANNELS)
	{
		cerr<<"Error : The number of channels is not correct."<<endl;
		exit(EXIT_FAILURE);
	}
	if(X.empty())
	{
		this->channels = channels;
	}
	else if(this->channels.size() != channels.size())
	{
		cerr<<"Error : The number of channels is not correct."<<endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		for(unsigned p = 0; p < channels.size(); ++p)
		{
			if(channels[p] != this->channels[p])
			{
				cerr<<"Error : The class centers channels do not correspond to the classifier channels."<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	this->B = B;
	numberClasses = B.size();
}

void Classifier::sortB()
{
	sort(B.begin(), B.end());
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
	#if defined DEBUG || defined VERBOSE
		ostringstream out;
		out<<"iteration"<<"\t"<<"precisionReached";
		for (unsigned i = 0; i < numberClasses; ++i)
			out<<"\t"<<"B"<<i;
		for (unsigned i = 0; i < numberClasses; ++i)
			out<<"\t"<<"classAvg"<<i;
	
		#if defined VERBOSE
			cout<<endl<<out.str();
		#endif
		
		#if defined DEBUG
			if(stepfile.is_open())
				stepfile.close();
			
			stepfile.open(filename.c_str(), ios_base::app);
			if(!stepfile)
			{
				cerr<<"Error : could not open iterations file "<<filename<<"!"<<endl;
			}
			if(stepfile.good())
				stepfile<<endl<<out.str();
		#endif
	#endif
}

void Classifier::stepout(const unsigned iteration, const Real precisionReached, const Real precision)
{
	#if defined DEBUG || defined VERBOSE
		ostringstream out;
		out.setf(ios::fixed);
		out.precision(1 - log10(precision));
		out<<iteration<<"\t"<<precisionReached<<"\t"<<B;
		out<<"\t"<<classAverage();
	
		#if defined VERBOSE
			cout<<endl<<out.str();
		#endif
		
		#if defined DEBUG
		if(stepfile.good())
			stepfile<<endl<<out.str();
		#endif
	#endif
	
	#if defined DEBUG || defined WRITE_MEMBERSHIP_FILES
	// We write the fits file of Uij
	if(U.size() == numberFeatureVectors * numberClasses)
	{
		Image<EUVPixelType> image(Xaxes,Yaxes);
		for (unsigned i = 0; i < numberClasses; ++i)
		{
			image.zero();
			for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
				image.pixel(coordinates[j]) = U[j*numberClasses + i];
			image.writeFits(filenamePrefix + "membership.iteration_" + toString(iteration) + ".class_" + toString(i) + ".fits");
		}
	}
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



ParameterSection Classifier::classificationParameters()
{
	ParameterSection parameters;
	parameters["maxNumberIteration"] = ArgParser::Parameter(100, 'i', "The maximal number of iteration for the classification.");
	parameters["precision"] = ArgParser::Parameter(0.0015, 'p', "The precision to be reached to stop the classification.");
	parameters["fuzzifier"] = ArgParser::Parameter(2, 'f', "The fuzzifier value");
	parameters["FCMfuzzifier"] = ArgParser::Parameter(2, "The FCM fuzzifier value. Set if you want to override the global fuzzifier value for FCM.");
	parameters["PCMfuzzifier"] = ArgParser::Parameter(2, "The PCM fuzzifier value. Set if you want to override the global fuzzifier value for PCM.");
	parameters["FCMweight"] = ArgParser::Parameter(2, "The FCM weight for PFCM classification.");
	parameters["PCMweight"] = ArgParser::Parameter(2, "The PCM  weight for PFCM classification.");
	parameters["numberClasses"] = ArgParser::Parameter(4, 'C', "The number of classes to classify the sun images into.");
	parameters["neighborhoodRadius"] = ArgParser::Parameter(1, 'N', "Only for spatial classifiers like SPoCA. The neighborhoodRadius is half the size of the square of neighboors.\nFor example with a value of 1, the square has a size of 3x3.");
	parameters["binSize"] = ArgParser::Parameter(RealFeature(1), 'z', "The size of the bins of the histogram.\nNB : Be carreful that the histogram is built after the image preprocessing.");
	return parameters;
}

ParameterSection Classifier::segmentationParameters()
{
	ParameterSection parameters;
	parameters["type"] = ArgParser::Parameter("max", 'T', "The type of segmentation. Possible values are : max, closest, threshold, limits, fix");
	parameters["limits"] = ArgParser::Parameter('L', "Only for limit segmentation. A vector of feature vectors to group class centers.");
	parameters["CH"] = ArgParser::Parameter('c', "Only for fix segmentation. The classes of the Coronal Hole.");
	parameters["AR"] = ArgParser::Parameter('a', "Only for fix segmentation. The classes of the Active Region.");
	parameters["QS"] = ArgParser::Parameter('q', "Only for fix segmentation. The classes of the Quiet Sun.");
	parameters["thresholds"] = ArgParser::Parameter('t', "Only for threshold segmentation. The parameter of the threshold segmentation.\nMust be of the form class_number,lowerIntensity_minMembership,higherIntensity_minMembership");
	return parameters;
}
