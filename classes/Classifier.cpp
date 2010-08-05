#include "Classifier.h"

using namespace std;

Classifier::Classifier()
:numberClasses(0),numberValidPixels(0),Xaxes(0),Yaxes(0),channels(0)
{}

void Classifier::checkImages(const vector<SunImage*>& images)
{

	#if defined(DEBUG) && DEBUG >= 1
	if(images.size()!=NUMBERWAVELENGTH)
	{
		cerr<<"Error : The number of images to initialize the Classifier must be equal to "<<NUMBERWAVELENGTH<<endl;
		exit(EXIT_FAILURE);
	}
	
	Coordinate sunCenter = images[0]->SunCenter();
	for (unsigned p = 1; p <  NUMBERWAVELENGTH; ++p)
	{
		if( sunCenter.d2(images[p]->SunCenter()) > 2 )
		{
			cerr<<"Warning : Image "<<images[p]->Wavelength()<<" does not have the same sun centre than image "<<images[0]->Wavelength()<<endl;
		}
		if( abs(1. - (images[p]->SunRadius() / images[0]->SunRadius())) > 0.01 )
		{
			cerr<<"Warning : Image "<<images[p]->Wavelength()<<" does not have the same sun radius than image "<<images[0]->Wavelength()<<endl;
			//exit(EXIT_FAILURE);
		}
	}
	#endif

}


void Classifier::addImages(vector<SunImage*>& images)
{

	checkImages(images);
	ordonateImages(images);
	unsigned numberValidPixelsEstimate = images[0]->numberValidPixelsEstimate();
	Xaxes = images[0]->Xaxes();
	Yaxes = images[0]->Yaxes();
	for (unsigned p = 1; p <  NUMBERWAVELENGTH; ++p)
	{
		Xaxes = images[p]->Xaxes() < Xaxes ? images[p]->Xaxes() : Xaxes;
		Yaxes = images[p]->Yaxes() < Yaxes ? images[p]->Yaxes() : Yaxes;
		numberValidPixelsEstimate = images[p]->numberValidPixelsEstimate() > numberValidPixelsEstimate ? images[p]->numberValidPixelsEstimate() : numberValidPixelsEstimate;
	}

	//We initialise the valid pixels vector X
	X.reserve(numberValidPixelsEstimate);
	coordinates.reserve(numberValidPixelsEstimate);

	PixelFeature xj;
	bool validPixel;
	for (unsigned y = 0; y < Yaxes; ++y)
	{
		for (unsigned x = 0; x < Xaxes; ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERWAVELENGTH && validPixel; ++p)
			{
				xj.v[p] = images[p]->pixel(x, y);
				if(xj.v[p] == images[p]->nullvalue)
					validPixel=false;
			}
			if(validPixel)
			{
				coordinates.push_back(Coordinate(x,y));
				X.push_back(xj);
			}

		}
	}

	numberValidPixels = X.size();

}

void Classifier::attribution()
{
	//Initialisation of U
	computeU();
}


unsigned Classifier::sursegmentation(unsigned Cmin)
{
	vector<Real> V;
	Real newScore = assess(V);
	Real oldScore = numeric_limits<Real>::max();
	vector<RealFeature> oldB; 

	#if defined(DEBUG) && DEBUG >= 3
	cout<<"--Classifier::sursegmentation--START--"<<endl;
	cout<<"B :\t"<<B<<endl;
	cout<<"V :\t"<<V<<"\tscore :"<<newScore<<endl;
	#endif
	
	#if defined(DEBUG) && DEBUG >= 2
	string filename;
	Image<unsigned> * segmentedMap;
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

		#if defined(DEBUG) && DEBUG >= 2
		filename = outputFileName + "segmented." + itos(numberClasses) + "classes.fits" ;
		segmentedMap = segmentedMap_maxUij();
		segmentedMap->writeFitsImage(filename);
		delete segmentedMap;
		#endif
		#if defined(DEBUG) && DEBUG >= 3
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

	#if defined(DEBUG) && DEBUG >= 3
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


// The merge function takes 2 centers and merge them into 1

//Merging by taking the mean value of the pixels Xj belonging to the classes Bi1 or Bi2
void Classifier::merge(unsigned i1, unsigned i2)
{

	#if defined(DEBUG) && DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2];
	#endif

	Real max_uij, sum = 0;
	unsigned max_i;
	RealFeature newB = 0;
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		max_uij = 0;
		max_i = 0;
		for (unsigned i = 0 ; i < numberClasses ; ++i)
		{
			if (U[i*numberValidPixels+j] > max_uij)
			{
				max_uij = U[i*numberValidPixels+j];
				max_i = i;
			}
		}
		if(max_i == i1 || max_i == i2)
		{
			newB += X[j] ;
			sum += 1;

		}

	}

	B[i1] = newB / sum;

	#if defined(DEBUG) && DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;

	computeU();

}


/* Older merging functions kept for historical reasons

//Merging according to Vincent's method
void Classifier::merge(unsigned i1, unsigned i2)
{
	#if defined(DEBUG) && DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2];
	#endif

	Real alpha = MERGEVINCENT_ALPHA;
	RealFeature newB[] = {0., 0.};
	Real caardinal[] = {0., 0.};

	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		if( U[i1*numberValidPixels+j] > alpha)
		{
			++caardinal[0];
			newB[0] += X[j]*U[i1*numberValidPixels+j];

		}
		if( U[i2*numberValidPixels+j] > alpha)
		{
			++caardinal[1];
			newB[1] += X[j]*U[i2*numberValidPixels+j];

		}

	}
	newB[0] /= caardinal[0];
	newB[1] /= caardinal[1];
	B[i1] = newB[0] + newB[1];

	#if defined(DEBUG) && DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;

	computeU();
}

//Merging by taking the mean value of the 2 centers, and recalculate U acordingly
void Classifier::merge(unsigned i1, unsigned i2)
{

	#if defined(DEBUG) && DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2];
	#endif

	B[i1] += B[i2];
	B[i1] /= 2.;

	#if defined(DEBUG) && DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;

	computeU();

}

*/

Image<unsigned>* Classifier::segmentedMap_maxUij()
{

	#if defined(DEBUG) && DEBUG >= 1
	if (U.size() != numberClasses*numberValidPixels)
		cerr<<"The membership matrix U has not yet been calculated"<<endl;
	#endif

	Image<unsigned>* segmentedMap = new Image<unsigned>(Xaxes, Yaxes);
	segmentedMap->zero();

	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		Real max_uij = U[j];
		segmentedMap->pixel(coordinates[j]) = 1;
		for (unsigned i = 1 ; i < numberClasses ; ++i)
		{
			if (U[i*numberValidPixels+j] > max_uij)
			{
				max_uij = U[i*numberValidPixels+j];
				segmentedMap->pixel(coordinates[j]) = i + 1;
			}
		}

	}
	return segmentedMap;

}

Image<unsigned>* Classifier::segmentedMap_closestCenter()
{

	#if defined(DEBUG) && DEBUG >= 1
	if (U.size() != numberClasses*numberValidPixels)
		cerr<<"The membership matrix U has not yet been calculated"<<endl;
	#endif

	Image<unsigned>* segmentedMap = new Image<unsigned>(Xaxes, Yaxes);
	segmentedMap->zero();
	Real d2XjBi;
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		Real minDistance = d2(X[j], B[0]);
		segmentedMap->pixel(coordinates[j]) = 1;
		for (unsigned i = 1 ; i < numberClasses ; ++i)
		{
			d2XjBi = d2(X[j], B[i]);
			if (d2XjBi < minDistance)
			{
				minDistance = d2XjBi;
				segmentedMap->pixel(coordinates[j]) = i + 1;
			}
		}

	}
	return segmentedMap;

}

Image<unsigned>* Classifier::segmentedMap_classTreshold(unsigned middleClass, Real lowerIntensity_minMembership, Real higherIntensity_minMembership)
{

	#if defined(DEBUG) && DEBUG >= 1
	if (U.size() != numberClasses*numberValidPixels)
		cerr<<"The membership matrix U has not yet been calculated."<<endl;
	if (middleClass == 0 || middleClass > numberClasses)
	{
		cerr<<"The class number for treshold segmentation should be between 1 and numberClasses. It was set to: "<<middleClass<<endl;
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
	
	#if defined(DEBUG) && DEBUG >= 2
	Image<Real>* map = fuzzyMap(middleClass);
	map->writeFitsImage(outputFileName + "fuzzymap.fits");
	delete map;
	#endif


	Image<unsigned>* segmentedMap = new Image<unsigned>(Xaxes, Yaxes);
	segmentedMap->zero();
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		if(X[j] < B[middleClass])
		{
			if(U[middleClass*numberValidPixels+j] < lowerIntensity_minMembership)
				segmentedMap->pixel(coordinates[j]) = 1;
			else
				segmentedMap->pixel(coordinates[j]) = 2;
		}
		else
		{
			if(U[middleClass*numberValidPixels+j] < higherIntensity_minMembership)
				segmentedMap->pixel(coordinates[j]) = 3;
			else
				segmentedMap->pixel(coordinates[j]) = 2;
		}
	}
	return segmentedMap;

}


Image<unsigned>* Classifier::segmentedMap_limits(vector<RealFeature>& limits)
{

	Image<unsigned>* segmentedMap = segmentedMap_maxUij();
	
	#if defined(DEBUG) && DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "max.segmented.fits");
	#endif

	//We create a vector of transformation telling wich class must be merged to what class
	vector<unsigned> transfo(numberClasses + 1, 1);

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

Image<unsigned>* Classifier::segmentedMap_fixed(vector<unsigned>& ch, vector<unsigned>& qs, vector<unsigned>& ar)
{

	Image<unsigned>* segmentedMap = segmentedMap_maxUij();

	#if defined(DEBUG) && DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "max.segmented.fits");
	#endif

	//We create a vector of transformation telling wich class must be merged to what class
	vector<unsigned> transfo(numberClasses + 1, 0);

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



Image<Real>* Classifier::fuzzyMap(const unsigned i)
{
	Image<Real>* map = new Image<Real>(Xaxes, Yaxes);
	map->zero();
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		map->pixel(coordinates[j]) = U[i*numberValidPixels+j];

	return map;
}


Image<Real>* Classifier::normalizedFuzzyMap(const unsigned i)
{
	Image<Real>* map = new Image<Real>(Xaxes, Yaxes);
	Real    sum;
	map->zero();

	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		sum = 0;
		for (unsigned k = 0 ; k < numberClasses ; ++k)
		{
			sum += U[k*numberValidPixels+j];
		}
		map->pixel(coordinates[j]) = U[i*numberValidPixels+j] / sum;
	}

	return map;
}


// Function that saves all results possible
// It is not very efficient, can output a LOT of big files, and is only for research and testing
// You pass it a SunImage that has already all the keywords correctly set
void Classifier::saveAllResults(SunImage* outImage)
{
	Image<unsigned> * segmentedMap = segmentedMap_maxUij();
	string filename;

	#if defined(DEBUG) && DEBUG >= 2
	filename = outputFileName + "segmented." + itos(numberClasses) + "classes.fits";
	segmentedMap->writeFitsImage(filename);
	#endif
	#if defined(DEBUG) && DEBUG >= 4
	if(!outImage)
	{
		delete segmentedMap;
		return;
	}
	unsigned numberRegions;
	vector<Region*> regions;
	for (unsigned i = 1; i <= numberClasses; ++i)
	{
		outImage->zero();

		//We create a map of the class i
		outImage->bitmap(segmentedMap, i);
		string baseName = outputFileName + "class" + itos(i) + ".";

		#if defined(DEBUG) && DEBUG >= 2
		filename = baseName + "uncleaned.fits";
		outImage->writeFitsImage(filename);
		#endif

		//We smooth the edges
		outImage->dilateDiamond(2,0)->erodeDiamond(2,0);

		#if defined(DEBUG) && DEBUG >= 2
		filename = baseName + "smoothed.uncleaned.fits";
		outImage->writeFitsImage(filename);
		#endif

		//Let's find the connected regions
		numberRegions = outImage->colorizeConnectedComponents(0);

		#if defined(DEBUG) && DEBUG >= 2
		filename = baseName + "blobs.uncleaned.fits";
		outImage->writeFitsImage(filename);
		#endif

		

		#if defined(DEBUG) && DEBUG >= 2
		//Let's get the connected regions info
		regions = getRegions(outImage);
		filename = baseName + "regions.uncleaned.txt";
		ofstream uncleanedResultsFile(filename.c_str());
		if (uncleanedResultsFile.good())
		{
			uncleanedResultsFile<<Region::header<<endl;
			for(unsigned r = 0; r < regions.size() && uncleanedResultsFile.good(); ++r)
			{
				uncleanedResultsFile<<regions[r]->toString()<<endl;
				delete regions[r];
			}
		}
		uncleanedResultsFile.close();
		#endif

		#if defined(DEBUG) && DEBUG >= 2
		//Let's draw the contours
		outImage->drawContours();
		filename = baseName + "contours.uncleaned.fits";
		outImage->writeFitsImage(filename);
		#endif

		//Let's remove the small regions (i.e. assimilated to bright points )
		unsigned minSize = unsigned(MIN_AR_SIZE / outImage->PixelArea());
		outImage->zero();
		outImage->bitmap(segmentedMap, i);
		outImage->tresholdConnectedComponents(minSize, 0);

		#if defined(DEBUG) && DEBUG >= 2
		filename = baseName + "fits";
		outImage->writeFitsImage(filename);
		#endif

		//We smooth the edges
		outImage->dilateCircular(2,0)->erodeCircular(2,0);

		#if defined(DEBUG) && DEBUG >= 2
		filename = baseName + "smoothed.fits";
		outImage->writeFitsImage(filename);
		#endif


		//Let's find the connected regions
		numberRegions = outImage->colorizeConnectedComponents(0);

		#if defined(DEBUG) && DEBUG >= 2
		filename = baseName + "blobs.fits";
		outImage->writeFitsImage(filename);
		#endif

		//Let's get the connected regions info
		regions = getRegions(outImage);

		filename = baseName + "regions.txt";
		ofstream resultsFile(filename.c_str());
		if (resultsFile.good())
		{
			resultsFile<<Region::header<<endl;
			for(unsigned r = 0; r < regions.size() && resultsFile.good(); ++r)
			{
				resultsFile<<regions[r]->toString()<<endl;
			}
		}
		resultsFile.close();
		
		#if defined(DEBUG) && DEBUG >= 2
		//Let's draw the contours
		outImage->drawContours();
		filename = baseName + "contours.fits";
		outImage->writeFitsImage(filename);
		#endif

		#if defined(DEBUG) && DEBUG >= 2
		//Let's draw the boxes
		outImage->zero();
		for (unsigned r = 1; r < regions.size(); ++r)
		{
			outImage->drawBox(regions[r]->Id(), regions[r]->Boxmin(), regions[r]->Boxmax());
		}

		filename = baseName + "boxes.fits";
		outImage->writeFitsImage(filename);
		#endif

		#if defined(DEBUG) && DEBUG >= 2
		//Let's draw the centers
		outImage->zero();
		for (unsigned r = 1; r < regions.size(); ++r)
		{
			outImage->drawCross(regions[r]->Id(), regions[r]->Center(), 5);
		}

		filename = baseName + "centers.fits";
		outImage->writeFitsImage(filename);
		#endif

		//We cleanup
		for(unsigned r = 0; r < regions.size(); ++r)
			delete regions[r];

		#if defined(DEBUG) && DEBUG >= 2
		//Let's get the fuzzyMaps
		Image<Real>* map = fuzzyMap(i-1);
		filename = baseName + "fuzzy.fits";
		map->writeFitsImage(filename);
		delete map;
		#endif
		#if defined(DEBUG) && DEBUG >= 2
		//Let's get the normalized fuzzyMaps
		Image<Real>* normalizedMap = normalizedFuzzyMap(i-1);
		filename = baseName + "fuzzy.normalized.fits";
		normalizedMap->writeFitsImage(filename);
		delete normalizedMap;
		#endif
	}
	#endif
	delete segmentedMap;

}





// Function that saves the AR map for tracking
// You pass it a SunImage that has already all the keywords correctly set
void Classifier::saveARmap(SunImage* outImage)
{
	Image<unsigned> * segmentedMap = segmentedMap_maxUij();
	string filename;
	unsigned ARclass = 0;
	RealFeature maxB = 0;
	// The Active Regions class has the biggest center
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		if (maxB < B[i])
		{
			maxB = B[i];
			ARclass = i + 1;
		}
	}
	outImage->zero();

	//We create a map of the class ARclass
	outImage->bitmap(segmentedMap, ARclass);

	delete segmentedMap;

	//We erase small regions
	unsigned minSize = unsigned(MIN_AR_SIZE / outImage->PixelArea());
	outImage->tresholdConnectedComponents(minSize, 0);
	
	//We agregate the blobs together
	outImage->blobsIntoAR();
	
	//We don't need the AR post limb anymore
	outImage->nullifyAboveRadius(1.); 

	filename = outputFileName + "ARmap.tracking.fits";
	outImage->writeFitsImage(filename);

}

// Work in progress
// Function that saves the CH map for tracking
// You pass it a SunImage that has already all the keywords correctly set
void Classifier::saveCHmap(SunImage* outImage)
{
	Image<unsigned> * segmentedMap = segmentedMap_maxUij();
	string filename;
	unsigned CHclass = 0;
	RealFeature minB = numeric_limits<Real>::max();
	// The Active Regions class has the biggest center
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		if (minB < B[i])
		{
			minB = B[i];
			CHclass = i + 1;
		}
	}
	outImage->zero();

	//We create a map of the class CHclass
	outImage->bitmap(segmentedMap, CHclass);

	delete segmentedMap;

	//We erase small regions
	unsigned minSize = unsigned(MIN_CH_SIZE / outImage->PixelArea());
	outImage->tresholdConnectedComponents(minSize, 0);
	
	//We agregate the blobs together
	outImage->blobsIntoAR(); //TODO
	
	//We don't need the CH post limb anymore
	outImage->nullifyAboveRadius(1.); 

	filename = outputFileName + "CHmap.tracking.fits";
	outImage->writeFitsImage(filename);

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


vector<PixelFeature> Classifier::percentiles(vector<Real> percentileValues)
{

	vector<PixelFeature> sortedX = X;
	vector<PixelFeature> result;
	sort(sortedX.begin(), sortedX.end());
	for (unsigned h = 0; h < percentileValues.size(); ++h)
	{
		#if defined(DEBUG) && DEBUG >= 1
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



SunImage* Classifier::getImage(unsigned p)
{

	SunImage* image = new SunImage(Xaxes, Yaxes);
	image->zero();
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		image->pixel(coordinates[j]) = X[j].v[p];
	}

	return image;

}


void Classifier::randomInitB(unsigned C)
{
	#if defined(DEBUG) && DEBUG >= 1
	if(X.size() == 0)
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
		B[i]=X[rand() % numberValidPixels];

	}
	//We like our centers to be sorted
	sort(B.begin(), B.end());
}


void Classifier::initB(const vector<RealFeature>& B)
{
	this->B = B;
	numberClasses = B.size();
}

void Classifier::initB(const vector<RealFeature>& B, const RealFeature& channels)
{
	this->B = B;
	numberClasses = B.size();
	this->channels = channels;
}

void Classifier::ordonateImages(vector<SunImage*>& images)
{
	if(channels)
	{
		for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
		{
			if(channels.v[p] != images[p]->Wavelength())
			{
				unsigned pp = p+1;
				while(pp < NUMBERWAVELENGTH && channels.v[p] != images[pp]->Wavelength())
					++pp;
				if(pp < NUMBERWAVELENGTH)
				{
					SunImage* temp = images[pp];
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
		for (unsigned p = 0; p < NUMBERWAVELENGTH; ++p)
			channels.v[p] = images[p]->Wavelength();
	}
	
}

// Computes the real average of each class
vector<RealFeature> Classifier::classAverage() const
{
	
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
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		class_average[i]/=cardinal[i];
	}
	return class_average;
}
