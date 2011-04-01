#include "CoronalHole.h"

using namespace std;


unsigned CHclass(const vector<RealFeature>& B)
{
	// The CoronalHole class has the smallest center
	unsigned CHclass = 1;
	RealFeature minB = B[0];
	
	for (unsigned i = 1; i < B.size(); ++i)
	{
		if (minB > B[i])
		{
			minB = B[i];
			CHclass = i + 1;
		}
	}
	return CHclass;
}


// Function that saves the CH map for tracking
// You pass it a ColorMap that has already all the keywords correctly set
ColorMap* CoronalHoleMap(const ColorMap* segmentedMap, unsigned CHclass, bool tresholdRawArea)
{
	ColorMap* CHMap = new ColorMap(segmentedMap);

	//We create a map of the class CHclass
	CHMap->bitmap(segmentedMap, CHclass);

	#if DEBUG >= 2
	CHMap->writeFits(outputFileName + "CHmap.pure.fits");
	#endif

	// We clean the colormap to remove very small components (like protons)
	CHMap->erodeCircular(2,0)->dilateCircular(2,0);
	
	#if DEBUG >= 2
	CHMap->writeFits(outputFileName + "CHmap.opened.fits");
	#endif
	
	
	// We remove the parts off limb (Added by Cis to avoid large off-limbs CH to combine 2 well separated CH on-disk to be aggregated as one)
	CHMap->nullifyAboveRadius(1.);
	
	// We agregate the blobs together
	blobsIntoCH(CHMap);
	
	#if DEBUG >= 2
	CHMap->writeFits(outputFileName + "CHmap.aggregated.fits");
	#endif
	
	// We don't need the CH post limb
	CHMap->nullifyAboveRadius(1.); 

	// We erase small regions
	if(tresholdRawArea)
		CHMap->tresholdRegionsByRawArea(MIN_CH_SIZE);
	else
		CHMap->tresholdRegionsByRealArea(MIN_CH_SIZE);

	return CHMap;
}

/*
		// I need the chain code
		vector<Coordinate> chainCode = CHMap->chainCode(coronalHoles[i]->FirstPixel(), maxChainCodepoints);
		// Oysh, I have only 70 chars to write the chaincode, each pair is taking up to 12 char
		//CHMap->header.set<string>(id+"CHAIN", chainCode, "Coronal Hole Chain Code");
	}
*/

/*
void blobsIntoCH (ColorMap* CHmap)
{
	//We create  a map by dilation 
	unsigned dilateFactor = unsigned(CH_AGGREGATION  / sqrt(CHmap->PixelArea() ));
	ColorMap* dilated = new ColorMap(CHmap);
	dilated->dilateCircular(dilateFactor,CHmap->nullvalue());
	dilated->colorizeConnectedComponents(1);
	#if DEBUG >= 2
	dilated->writeFits(outputFileName + "CHmap.dilated.fits");
	#endif
	//We color the blobs using the dilated map 
	for (unsigned j=0; j < CHmap->NumberPixels(); ++j)
	{
		if (CHmap->pixel(j) != CHmap->nullvalue())
			CHmap->pixel(j) = dilated->pixel(j);
	}
	delete dilated;
	
}
*/

void blobsIntoCH (ColorMap* CHmap)
{

	// We agregate the blobs together by closing 
	unsigned dilateFactor = unsigned(CH_AGGREGATION  / sqrt(CHmap->PixelArea() ) );
	
	CHmap->dilateCircular(dilateFactor, 0)->erodeCircular(dilateFactor, 0);
	
	CHmap->colorizeConnectedComponents(1);
	
}





