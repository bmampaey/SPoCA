#include "ActiveRegion.h"

using namespace std;


#if defined(AGGREGATE_DILATE_BENTEST)
// Function that saves the AR map for tracking
// You pass it a SunImage that has already all the keywords correctly set
SunImage* ActiveRegionMap(SunImage* segmentedMap, unsigned ARclass)
{
	segmentedMap->setNullvalue(0);
	
	//We create a map of the class ARclass
	segmentedMap->bitmap(segmentedMap, ARclass);

	//We smooth the edges
	segmentedMap->erodeCircular(2,0)->dilateCircular(2,0);

	//We don't need the AR post limb anymore
	segmentedMap->nullifyAboveRadius(1.); 
	
	//We agregate the blobs together
	blobsIntoAR(segmentedMap);

	return segmentedMap;

}

#else
// Function that saves the AR map for tracking
// You pass it a SunImage that has already all the keywords correctly set
SunImage* ActiveRegionMap(SunImage* segmentedMap, unsigned ARclass)
{
	segmentedMap->setNullvalue(0);

	//We create a map of the class ARclass
	segmentedMap->bitmap(segmentedMap, ARclass);

	//We smooth the edges
	segmentedMap->dilateCircular(2,0)->erodeCircular(2,0);

	//We erase small regions
	unsigned minSize = unsigned(MIN_AR_SIZE / segmentedMap->PixelArea());
	segmentedMap->tresholdConnectedComponents(minSize, 1);
	
	//We agregate the blobs together
	blobsIntoAR(segmentedMap);
	
	//We don't need the AR post limb anymore
	segmentedMap->nullifyAboveRadius(1.); 

	return segmentedMap;

}
#endif

// Work in progress
// Function that saves the CH map for tracking
// You pass it a SunImage that has already all the keywords correctly set
SunImage* CoronalHoleMap(SunImage* segmentedMap, unsigned CHclass)
{

	segmentedMap->setNullvalue(0);

	//We create a map of the class CHclass
	segmentedMap->bitmap(segmentedMap, CHclass);

	//We smooth the edges
	segmentedMap->dilateCircular(2,0)->erodeCircular(2,0);

	//We erase small regions
	unsigned minSize = unsigned(MIN_CH_SIZE / segmentedMap->PixelArea());
	segmentedMap->tresholdConnectedComponents(minSize, 0);
	
	//We agregate the blobs together
	blobsIntoAR(segmentedMap); //TODO
	
	//We don't need the CH post limb
	segmentedMap->nullifyAboveRadius(1.); 

	return segmentedMap;

}

unsigned ARclass(const vector<RealFeature>& B)
{
	// The Active Regions class has the biggest center
	unsigned ARclass = 0;
	RealFeature maxB = 0;
	
	for (unsigned i = 0; i < B.size(); ++i)
	{
		if (maxB < B[i])
		{
			maxB = B[i];
			ARclass = i + 1;
		}
	}
	return ARclass;
}

unsigned CHclass(const vector<RealFeature>& B)
{
	// The Coronal Hole class has the smallest center
	unsigned CHclass = 0;
	RealFeature minB = numeric_limits<Real>::max();
	
	for (unsigned i = 0; i < B.size(); ++i)
	{
		if (minB < B[i])
		{
			minB = B[i];
			CHclass = i + 1;
		}
	}
	return CHclass;
}

#if defined(AGGREGATE_DILATE)
void blobsIntoAR (SunImage* ARmap)
{

	//We agregate the blobs together by dilation 
	unsigned dilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ) );
	ARmap->dilateCircular(dilateFactor, 0);
	
	ARmap->colorizeConnectedComponents(1);
	
	return this;
}


#elif defined(AGGREGATE_DILATE_BENTEST)
// The idea is that we dilate more the smaller blobs than the bigger ones
void blobsIntoAR (SunImage* ARmap)
{

	const double R0 = ARmap->SunRadius() * ARmap->PixelArea();
	const double R2 = ARmap->SunRadius() * ARmap->SunRadius();
	unsigned setValue = 1;
	vector<int> neighboors;
	neighboors.push_back(1);
	neighboors.push_back(-1);
	neighboors.push_back(Xaxes());
	neighboors.push_back(-Xaxes());
	
	neighboors.push_back(Xaxes()+1);
	neighboors.push_back(Xaxes()-1);
	neighboors.push_back(-Xaxes()+1);
	neighboors.push_back(-Xaxes()-1);
	
	//We create a list of blobs and their areas
	unsigned newColor = setValue;
	vector<unsigned> blobList;
	vector<double> areas(newColor,0);
	for (unsigned j=0; j < numberPixels; ++j)
	{
		if(ARmap->pixel(j) != setValue)
			continue;
			
		blobList.push_back(j);
		++newColor;
		areas.push_back(0);
		vector<unsigned> pixelList(1,j);
		while ( ! pixelList.empty())
		{
			unsigned h = pixelList.back();
			pixelList.pop_back();
			ARmap->pixel(h) = newColor;
			Coordinate c = ARmap->coordinate(h);
			int cx = c.x > ARmap->SunCenter().x ? c.x - ARmap->SunCenter().x : ARmap->SunCenter().x - c.x;
			int cy = c.y > ARmap->SunCenter().y ? c.y - ARmap->SunCenter().y : ARmap->SunCenter().y - c.y;
			double pixelArea2 = R2 - (cx * cx) - (cy * cy);
			if(pixelArea2 > 0)
				areas[blobList.size()-1] += R0 / sqrt(pixelArea2);
			else
				areas[blobList.size()-1] = numeric_limits<double>::infinity();
			for(unsigned n = 0; n < neighboors.size(); ++n)
				if(ARmap->pixel(h+neighboors[n]) == setValue)
					pixelList.push_back(h+neighboors[n]);
		}
	}
	//ARmap->writeFitsImage("undilated.fits");
	
	vector<unsigned> dilation_size (areas.size(), 0);
	// We transform the areas in dilation size
	#define AR_DILATION_FACTOR 12
	const double AR_area_factor = 1 / (AR_DILATION_FACTOR * ARmap->PixelArea());
	unsigned maxDilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ));
	for (unsigned s=0; s < areas.size(); ++s)
	{
		if(areas[s] != numeric_limits<double>::infinity())
		{
			dilation_size[s] =  areas[s] * AR_area_factor;
			dilation_size[s] = dilation_size[s] > maxDilateFactor ? 0 : maxDilateFactor -  dilation_size[s];
		}
		//cout<<ARmap->pixel(blobList[s])<<" a:"<<areas[s]<<" ds:"<<dilation_size[s]<<endl;
	}
	
	


	//We create the map of dilation 
	SunImage * dilated = new SunImage(ARmap->Xaxes(), ARmap->Yaxes());
	dilated->zero();
	for (unsigned b=0; b < blobList.size(); ++b)
	{
		// Xe create a disc shape of radius dilation_size 
		vector<unsigned> shape;
		shape.reserve(dilation_size[b]*dilation_size[b]*3);
		for(unsigned x = 1; x <= dilation_size[b]; ++x)
			shape.push_back(x);
		for(int x = -dilation_size[b]; x <= int(dilation_size[b]); ++x)
			for(unsigned y = 1; y <= dilation_size[b]; ++y)
				if(sqrt(x * x + y *y) <= dilation_size[b])
					shape.push_back(y * ARmap->Xaxes() + x);
					
		vector<unsigned> pixelList(1,blobList[b]);
		while ( ! pixelList.empty())
		{
			unsigned h = pixelList.back();
			pixelList.pop_back();
			dilated->pixel(h) = 1;
			ARmap->pixel(h) = setValue;
			bool propagate = false;
			for(unsigned n = 0; n < neighboors.size(); ++n)
				if(ARmap->pixel(h+neighboors[n]) != ARmap->nullvalue() && ARmap->pixel(h+neighboors[n]) > setValue)
					pixelList.push_back(h+neighboors[n]);
				else
					propagate=true;
							
			if(propagate)
			{
			
				for(unsigned s = 0; s < shape.size(); ++s)
				{
					#if DEBUG >= 1
						if(h + shape[s] >= numberPixels || h - shape[s] < 0)
						{
							cerr<<"Error : trying to access pixel out of image in drawContours"<<endl;
							exit(EXIT_FAILURE);
						}	
					#endif
					dilated->pixel(h + shape[s]) = dilated->pixel(h - shape[s]) = 1;
				}
			}

		}

	}
	
	unsigned numberRegions = dilated->colorizeConnectedComponents(1);
	//dilated->writeFitsImage("dilated.fits");
	areas.clear();
	areas.resize(numberRegions + 2, 0);
	//We color the image using the dilated map and recompute the new areas
	for (unsigned j=0; j < numberPixels; ++j)
	{
		if (ARmap->pixel(j) != ARmap->nullvalue())
		{
			ARmap->pixel(j) = dilated->pixel(j);
			Coordinate c = coordinate(j);
			int cx = c.x > ARmap->SunCenter().x ? c.x - ARmap->SunCenter().x : ARmap->SunCenter().x - c.x;
			int cy = c.y > ARmap->SunCenter().y ? c.y - ARmap->SunCenter().y : ARmap->SunCenter().y - c.y;
			double pixelArea2 = R2 - (cx * cx) - (cy * cy);
			if(pixelArea2 > 0)
				areas[ARmap->pixel(j)] += R0 / sqrt(pixelArea2);
			else
				areas[ARmap->pixel(j)] = numeric_limits<double>::infinity();
		}
	}		
	//ARmap->writeFitsImage("recolored.fits");
	
	//We remove the AR too small
	for (unsigned j=0; j < numberPixels; ++j)
	{
		if (ARmap->pixel(j) != ARmap->nullvalue() && areas[ARmap->pixel(j)]  < MIN_AR_SIZE)
		{
			ARmap->pixel(j) = ARmap->nullvalue();
		}
	}

	/*
	// Test of the size of each pixel
	for (unsigned j=0; j < numberPixels; ++j)
	{
			Coordinate c = coordinate(j);
			int cx = c.x > ARmap->SunCenter().x ? c.x - ARmap->SunCenter().x : ARmap->SunCenter().x - c.x;
			int cy = c.y > ARmap->SunCenter().y ? c.y - ARmap->SunCenter().y : ARmap->SunCenter().y - c.y;
			double pixelArea2 = R2 - (cx * cx) - (cy * cy);
			if(pixelArea2 > 0)
				dilated->pixel(j)= R0 / sqrt(pixelArea2);
			else
				dilated->pixel(j)= numeric_limits<double>::infinity();
		
	}
	dilated->writeFitsImage("pixel_real_size.fits");
	*/

	delete dilated;
	
	return this;
}

#else

void blobsIntoAR (SunImage* ARmap)
{
	//We create  a map by dilation 
	unsigned dilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ));
	SunImage* dilated = new SunImage(ARmap);
	dilated->dilateCircular(dilateFactor,ARmap->nullvalue());
	dilated->colorizeConnectedComponents(1);
	
	//We color the blobs using the dilated map 
	for (unsigned j=0; j < ARmap->NumberPixels(); ++j)
	{
		if (ARmap->pixel(j) != ARmap->nullvalue())
			ARmap->pixel(j) = dilated->pixel(j);
	}
	delete dilated;
	
}



#endif

