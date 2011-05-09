#include "CoronalHole.h"

using namespace std;
extern std::string filenamePrefix;

/*! Return the indice of the smallest class center */
unsigned CHclass(const vector<RealFeature>& B)
{
	// The CoronalHole class has the smallest center
	unsigned CHclass = 1;
	RealFeature minB = B[0];
	
	for (unsigned i = 1; i < B.size(); ++i)
	{
		if (B[i] < minB)
		{
			minB = B[i];
			CHclass = i + 1;
		}
	}
	return CHclass;
}



/*!
@param segmentedMap ColorMap of the segmentation that has already all the keywords correctly set
@param CHclass The color in the segmentedMap that correspond to the class of CH
@param thresholdRawArea If set, the threshold fro removing small CH will be computed onthe Raw Area. Otherwise on the Area at disc center
*/
ColorMap* CoronalHoleMap(const ColorMap* segmentedMap, unsigned CHclass, bool thresholdRawArea)
{
	ColorMap* CHMap = new ColorMap(segmentedMap);

	/*! Create a map of the class CHclass */
	CHMap->bitmap(segmentedMap, CHclass);

	#if DEBUG >= 2
	CHMap->writeFits(filenamePrefix + "CHmap.pure.fits");
	#endif

	/*! Clean the colormap to remove very small/thin components (like filaments)*/
	unsigned cleaningFactor = unsigned(CH_CLEANING / sqrt(CHMap->PixelArea() ));
	CHMap->erodeCircular(cleaningFactor,0)->dilateCircular(cleaningFactor,0);
	
	#if DEBUG >= 2
	CHMap->writeFits(filenamePrefix + "CHmap.opened.fits");
	#endif
	
	
	/*! Remove the parts off limb (Added by Cis to avoid large off-limbs CH to combine 2 well separated CH on-disk to be aggregated as one)*/
	CHMap->nullifyAboveRadius(1.);
	
	/*! Aggregate the blobs together*/
	blobsIntoCH(CHMap);
	
	#if DEBUG >= 2
	CHMap->writeFits(filenamePrefix + "CHmap.aggregated.fits");
	#endif
	
	/*! Remove the CH post limb*/
	CHMap->nullifyAboveRadius(1.); 

	/*! Erase the small regions*/
	if(thresholdRawArea)
		CHMap->thresholdRegionsByRawArea(MIN_CH_SIZE);
	else
		CHMap->thresholdRegionsByRealArea(MIN_CH_SIZE);

	return CHMap;
}


#if !defined(CH_AGGREGATION_TYPE) || (CH_AGGREGATION_TYPE == CH_AGGREGATION_FRAGMENTED)
void blobsIntoCH (ColorMap* CHmap)
{
	/*! Create a map by dilation */
	unsigned dilateFactor = unsigned(CH_AGGREGATION  / sqrt(CHmap->PixelArea() ));
	ColorMap* dilated = new ColorMap(CHmap);
	dilated->dilateCircular(dilateFactor,CHmap->nullvalue());
	dilated->colorizeConnectedComponents(1);
	#if DEBUG >= 2
	dilated->writeFits(filenamePrefix + "CHmap.dilated.fits");
	#endif
	/*! Color the blobs using the dilated map */
	for (unsigned j=0; j < CHmap->NumberPixels(); ++j)
	{
		if (CHmap->pixel(j) != CHmap->nullvalue())
			CHmap->pixel(j) = dilated->pixel(j);
	}
	delete dilated;
	
}

#elif CH_AGGREGATION_TYPE == CH_AGGREGATION_CLOSING
/*! Aggregate the blobs together by closing */
void blobsIntoCH (ColorMap* CHmap)
{
	unsigned dilateFactor = unsigned(CH_AGGREGATION  / sqrt(CHmap->PixelArea() ) );
	
	CHmap->dilateCircular(dilateFactor, 0)->erodeCircular(dilateFactor, 0);
	
	CHmap->colorizeConnectedComponents(1);
}

#elif CH_AGGREGATION_TYPE == CH_AGGREGATION_DILATE

/*! Aggregate the blobs together by dilation */
void blobsIntoCH (ColorMap* CHmap)
{
	unsigned dilateFactor = unsigned(CH_AGGREGATION  / sqrt(CHmap->PixelArea() ) );
	
	CHmap->dilateCircular(dilateFactor, 0);
	
	CHmap->colorizeConnectedComponents(1);
}

#else
#warning "Unknown CH_AGGREGATION_TYPE"
#endif

