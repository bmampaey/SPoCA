#include "ActiveRegion.h"

using namespace std;

extern std::string filenamePrefix;

/*! Return the indice of the biggest class center */
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


/*!
@param segmentedMap ColorMap of the segmentation that has already all the keywords correctly set
@param ARclass The color in the segmentedMap that correspond to the class of AR
@param thresholdRawArea If set, the threshold fro removing small AR will be computed onthe Raw Area. Otherwise on the Area at disc center
*/
ColorMap* ActiveRegionMap(const ColorMap* segmentedMap, unsigned ARclass, bool thresholdRawArea)
{
	ColorMap* ARMap = new ColorMap(segmentedMap);

	/*! Create a map of the class ARclass */
	ARMap->bitmap(segmentedMap, ARclass);

	#if DEBUG >= 2
	ARMap->writeFits(filenamePrefix + "ARmap.pure.fits");
	#endif

	/*! Clean the colormap to remove very small components (like protons)*/
	unsigned cleaningFactor = unsigned(AR_CLEANING / sqrt(ARMap->PixelArea() ));
	ARMap->erodeCircular(cleaningFactor,0)->dilateCircular(cleaningFactor,0);
	
	#if DEBUG >= 2
	ARMap->writeFits(filenamePrefix + "ARmap.opened.fits");
	#endif
	
	
	/*! Remove the parts off limb (Added by Cis to avoid large off-limbs AR to combine 2 well separated AR on-disk to be aggregated as one)*/
	ARMap->nullifyAboveRadius(1.);
	
	/*! Aggregate the blobs together*/
	blobsIntoAR(ARMap);
	
	#if DEBUG >= 2
	ARMap->writeFits(filenamePrefix + "ARmap.aggregated.fits");
	#endif
	
	/*! Remove the AR post limb*/
	ARMap->nullifyAboveRadius(1.); 

	/*! Erase the small regions*/
	if(thresholdRawArea)
		ARMap->thresholdRegionsByRawArea(MIN_AR_SIZE);
	else
		ARMap->thresholdRegionsByRealArea(MIN_AR_SIZE);

	return ARMap;
}

#if !defined(AR_AGGREGATION_TYPE) || (AR_AGGREGATION_TYPE == AR_AGGREGATION_FRAGMENTED)
void blobsIntoAR (ColorMap* ARmap)
{
	/*! Create a map by dilation */ 
	unsigned dilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ));
	ColorMap* dilated = new ColorMap(ARmap);
	dilated->dilateCircular(dilateFactor,ARmap->nullvalue());
	dilated->colorizeConnectedComponents(1);
	#if DEBUG >= 2
	dilated->writeFits(filenamePrefix + "ARmap.dilated.fits");
	#endif
	
	/*! Color the blobs using the dilated map */
	for (unsigned j=0; j < ARmap->NumberPixels(); ++j)
	{
		if (ARmap->pixel(j) != ARmap->nullvalue())
			ARmap->pixel(j) = dilated->pixel(j);
	}
	delete dilated;

}
#elif AR_AGGREGATION_TYPE == AR_AGGREGATION_CLOSING
/*! Aggregate the blobs together by closing */
void blobsIntoAR (ColorMap* ARmap)
{
	unsigned dilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ) );
	
	ARmap->dilateCircular(dilateFactor, 0)->erodeCircular(dilateFactor, 0);
	
	ARmap->colorizeConnectedComponents(1);
}

#elif AR_AGGREGATION_TYPE == AR_AGGREGATION_DILATE

/*! Aggregate the blobs together by dilation */
void blobsIntoAR (ColorMap* ARmap)
{
	unsigned dilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ) );
	
	ARmap->dilateCircular(dilateFactor, 0);
	
	ARmap->colorizeConnectedComponents(1);
}

#else
#warning "Unknown AR_AGGREGATION_TYPE"
#endif 





