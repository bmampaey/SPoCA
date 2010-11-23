#include "CoronalHole.h"

using namespace std;


unsigned CHclass(const vector<RealFeature>& B)
{
	// The CoronalHole class has the smallest center
	unsigned CHclass = 1;
	RealFeature minB = B[0];
	
	for (unsigned i = 1; i < B.size(); ++i)
	{
		if (maxB < B[i])
		{
			maxB = B[i];
			CHclass = i + 1;
		}
	}
	return CHclass;
}


// Function that saves the CH map for tracking
// You pass it a ColorMap that has already all the keywords correctly set
ColorMap* CoronalHoleMap(ColorMap* segmentedMap, unsigned CHclass)
{
	segmentedMap->setNullvalue(0);

	//We create a map of the class CHclass
	segmentedMap->bitmap(segmentedMap, CHclass);

	#if DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "CHmap.pure.fits");
	#endif

	// We clean the colormap to remove very small components (like protons)
	segmentedMap->erodeCircular(2,0)->dilateCircular(2,0);
	
	#if DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "CHmap.opened.fits");
	#endif
	
	
	// We remove the parts off limb (Added by Cis to avoid large off-limbs CH to combine 2 well separated CH on-disk to be aggregated as one)
	segmentedMap->nullifyAboveRadius(1.);
	
	// We agregate the blobs together
	blobsIntoCH(segmentedMap);
	
	#if DEBUG >= 2
	segmentedMap->writeFitsImage(outputFileName + "CHmap.aggregated.fits");
	#endif
	
	// We don't need the CH post limb
	segmentedMap->nullifyAboveRadius(1.); 

	// We erase small regions
	segmentedMap->tresholdRegionsByRawArea(MIN_CH_SIZE);


	return segmentedMap;

}

#if !defined(AGGREGATE_DILATE)
void blobsIntoCH (ColorMap* CHmap)
{
	//We create  a map by dilation 
	unsigned dilateFactor = unsigned(CH_AGGREGATION  / sqrt(CHmap->PixelArea() ));
	ColorMap* dilated = new ColorMap(CHmap);
	dilated->dilateCircular(dilateFactor,CHmap->nullvalue());
	dilated->colorizeConnectedComponents(1);
	
	//We color the blobs using the dilated map 
	for (unsigned j=0; j < CHmap->NumberPixels(); ++j)
	{
		if (CHmap->pixel(j) != CHmap->nullvalue())
			CHmap->pixel(j) = dilated->pixel(j);
	}
	delete dilated;
	
}
#else //defined(AGGREGATE_DILATE)

void blobsIntoCH (ColorMap* CHmap)
{

	// We agregate the blobs together by dilation 
	unsigned dilateFactor = unsigned(CH_AGGREGATION  / sqrt(CHmap->PixelArea() ) );
	
	CHmap->dilateCircular(dilateFactor, 0);
	
	CHmap->colorizeConnectedComponents(1);
	
}
#endif //defined(AGGREGATE_DILATE)




