#include "ActiveRegion.h"

using namespace std;


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


// Function that saves the AR map for tracking
// You pass it a ColorMap that has already all the keywords correctly set
ColorMap* ActiveRegionMap(ColorMap* segmentedMap, unsigned ARclass)
{
	segmentedMap->setNullvalue(0);

	//We create a map of the class ARclass
	segmentedMap->bitmap(segmentedMap, ARclass);

	#if DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "ARmap.pure.fits");
	#endif

	// We clean the colormap to remove very small components (like protons)
	segmentedMap->erodeCircular(2,0)->dilateCircular(2,0);
	
	#if DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "ARmap.opened.fits");
	#endif
	
	
	// We remove the parts off limb (Added by Cis to avoid large off-limbs to combine 2 well separated AR on-disk regions to be aggregated as one)
	segmentedMap->nullifyAboveRadius(1.);
	
	// We agregate the blobs together
	blobsIntoAR(segmentedMap);
	
	#if DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "ARmap.aggregated.fits");
	#endif
	
	// We don't need the AR post limb
	segmentedMap->nullifyAboveRadius(1.); 

	// We erase small regions
	double minSize = MIN_AR_SIZE / segmentedMap->PixelArea();
	
	segmentedMap->tresholdRegionsByRawArea(minSize);


	return segmentedMap;

}

#if !defined(AGGREGATE_DILATE)
void blobsIntoAR (ColorMap* ARmap)
{
	//We create  a map by dilation 
	unsigned dilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ));
	ColorMap* dilated = new ColorMap(ARmap);
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
#else //defined(AGGREGATE_DILATE)

void blobsIntoAR (ColorMap* ARmap)
{

	// We agregate the blobs together by dilation 
	unsigned dilateFactor = unsigned(AR_AGGREGATION  / sqrt(ARmap->PixelArea() ) );
	
	ARmap->dilateCircular(dilateFactor, 0);
	
	ARmap->colorizeConnectedComponents(1);
	
}
#endif //defined(AGGREGATE_DILATE)




