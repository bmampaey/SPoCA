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

ColorMap* getAggregatedCHMap(const ColorMap* CHMap, const int projection)
{

	string filename = filenamePrefix + "CHMap.";

	Real cleaningFactor = Real(CH_CLEANING) / sqrt(CHMap->PixelArea());
	Real aggregationFactor = Real(CH_AGGREGATION) / sqrt(CHMap->PixelArea());
	
	ColorMap* aggregated = new ColorMap(CHMap);
	
	#if DEBUG >= 2
	aggregated->writeFits(filename + "pure.fits");
	#endif
	
	/*! Apply the projection */
	switch(projection)
	{
		case(SunImage<ColorType>::equirectangular):
			
			aggregated->equirectangular_projection(CHMap, false);
			#if DEBUG >= 2
			aggregated->writeFits(filename + "equirectangular_projection.fits");
			#endif
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			aggregated->Lambert_cylindrical_projection(CHMap, false);
			#if DEBUG >= 2
			aggregated->writeFits(filename + "Lambert_cylindrical_projection.fits");
			#endif
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		break;
		
		case(SunImage<ColorType>::sinuosidal):
			aggregated->sinuosidal_projection(CHMap, false);
			#if DEBUG >= 2
			aggregated->writeFits(filename + "sinuosidal_projection.fits");
			#endif
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		break;
		
		case(SunImage<ColorType>::distance_transform):
			cerr<<"Distance transform is not yet implemented"<<endl;
		break;
	}
	
	/*! Clean the color map to remove very small components (like protons)*/
	aggregated->erodeCircular(cleaningFactor, 0);
	
	#if DEBUG >= 2
	aggregated->writeFits(filename + "eroded.fits");
	#endif
	
	/*! Aggregate the blobs together */
	aggregated->dilateCircular(cleaningFactor + aggregationFactor, 0);
	
	#if DEBUG >= 2
	aggregated->writeFits(filename + "dilated.fits");
	#endif
	
	/*! Give back the original size */
	aggregated->erodeCircular(aggregationFactor, 0);
	
	#if DEBUG >= 2
	aggregated->writeFits(filename + "closed.fits");
	#endif
	
	/* Apply the deprojection */
	ColorMap* projeted = NULL;
	switch(projection)
	{
		case(SunImage<ColorType>::equirectangular):
			projeted = new ColorMap(aggregated);
			aggregated->equirectangular_deprojection(projeted, false);
			#if DEBUG >= 2
			aggregated->writeFits(filename + "equirectangular_deprojection.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			projeted = new ColorMap(aggregated);
			aggregated->Lambert_cylindrical_deprojection(projeted, false);
			#if DEBUG >= 2
			aggregated->writeFits(filename + "Lambert_cylindrical_deprojection.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::sinuosidal):
			projeted = new ColorMap(aggregated);
			aggregated->sinuosidal_deprojection(projeted, false);
			#if DEBUG >= 2
			aggregated->writeFits(filename + "sinuosidal_deprojection.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::distance_transform):
			cerr<<"Distance transform is not yet implemented"<<endl;
		break;
	}
	delete projeted;
	
	/*! Remove the parts off limb and colorize */
	aggregated->nullifyAboveRadius(1.); 
	aggregated->colorizeConnectedComponents(1);
	
	#if DEBUG >= 2
	aggregated->writeFits(filename + "aggregated.fits");
	#endif

	return aggregated;
}

Header getCHMapHeader()
{
	Header map_header;
	string projection;
	switch(CH_PROJECTION)
	{
		case(SunImage<ColorType>::equirectangular):
			projection = "equirectangular";
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			projection = "Lambert cylindrical";
		break;
		
		case(SunImage<ColorType>::sinuosidal):
			projection = "sinuosidal";
		break;
		
		case(SunImage<ColorType>::distance_transform):
			projection = "Distance transform";
		break;
		
		default:
			projection = "None";
	}
	
	map_header.set("CLEANING", CH_CLEANING, "Cleaning factor in arcsec");
	map_header.set("AGGREGAT", CH_AGGREGATION, "Aggregation factor in arcsec");
	map_header.set("PROJECTN", projection, "Projection used for the aggregation");
	map_header.set("MINSIZE", MIN_CH_SIZE, "Min size of regions in arcsec²");
	map_header.set("THRAWAR", CH_TRA, "Min size threshold on raw area");
	map_header.set("FRAGGMTD", CH_FRAGMENTED, "Regions are fragmented");
	return map_header;
}

void writeCHMap(ColorMap*& CHMap, const string& filename, bool compressed, unsigned chaincodeMinPoints, unsigned chaincodeMaxPoints, Real chaincodeMaxDeviation, EUVImage* image)
{
	/*! Clean everything above the radius */
	CHMap->nullifyAboveRadius(1.);
	
	/*! We get the map of aggregated CH */
	ColorMap* aggregatedMap = getAggregatedCHMap(CHMap, CH_PROJECTION);
	
	if(CH_FRAGMENTED)
	{
		/*! Color the pixels using the aggregated map */
		for (unsigned j = 0; j < aggregatedMap->NumberPixels(); ++j)
		{
			if(CHMap->pixel(j) == 1)
			{
				CHMap->pixel(j) = aggregatedMap->pixel(j);
			}
		}
	}
	else
	{
		delete CHMap;
		CHMap = aggregatedMap;
	}

	#if DEBUG >= 2
	CHMap->writeFits(filenamePrefix + "CHMap.all.fits");
	#endif

	/*! Erase the small regions */
	if(CH_TRA)
		CHMap->thresholdRegionsByRawArea(MIN_CH_SIZE);
	else
		CHMap->thresholdRegionsByRealArea(MIN_CH_SIZE);

	#if DEBUG >= 2
	CHMap->writeFits(filenamePrefix + "CHMap.large.fits");
	#endif
	
	/*! We write the map of CH to the fits file */
	FitsFile file(filename, FitsFile::overwrite);
	CHMap->writeFits(file, compressed ? FitsFile::compress : 0, "CoronalHoleMap");
	
	/*! We write some info about the CHMap creation */ 
	Header CHMapHeader = getCHMapHeader();
	file.writeHeader(CHMapHeader);
	
	/*! We get the regions */
	vector<Region*> regions = getRegions(CHMap);
	
	/*! We write the regions to the fits file */
	file.writeTable("Regions");
	writeRegions(file, regions);
	
	/*! We get the chaincode and write them to the fits file */
	if(chaincodeMaxPoints > 0)
	{
		#if DEBUG>= 2
		ColorMap chaincode_map(CHMap);
		#endif
		
		file.writeTable("ChainCodes");
		for (unsigned r = 0; r < regions.size(); ++r)
		{
			vector<PixLoc> chaincode = regions[r]->chainCode(aggregatedMap, chaincodeMinPoints, chaincodeMaxPoints, chaincodeMaxDeviation);
			file.writeColumn(itos(regions[r]->Id(),7), chaincode);
		
			#if DEBUG>= 2
			for (unsigned c = 0; c < chaincode.size(); ++c)
			{
				chaincode_map.drawCircle(chaincode[c], 3, r+10);
			}
			#endif
		}
		#if DEBUG>= 2
		chaincode_map.writeFits(filenamePrefix + "CHMap.chaincodes.fits");
		#endif
	}
	
	/*! We write intensities statistics relative to image */
	if(image)
	{
		/*! We get the stats of the CH */
		vector<RegionStats*> regions_stats =  getRegionStats(CHMap, image, regions);
	
		/*! We write the CH Stats into the fits */
		file.writeTable(image->Channel()+"_CoronalHoleStats");
		writeRegions(file, regions_stats);
		
		/*! We write some info bout the image in the header of the table */
		Header tableHeader;
		tableHeader.set("WAVELNTH", image->Wavelength(), "Wavelength of the intensity image.");
		
		const Header& imageHeader = image->getHeader();
		if(imageHeader.has("LVL_NUM"))
			tableHeader.set("LVL_NUM", imageHeader.get<float>("LVL_NUM"), "Level number of the intensity image.");
		if(imageHeader.has("DATE"))
			tableHeader.set("DATE", imageHeader.get<string>("DATE"), "Creation date of the intensity image.");
		if(imageHeader.has("QUALITY"))
			tableHeader.set("QUALITY", imageHeader.get<unsigned>("QUALITY"), "Quality level of the intensity image.");
		
		file.writeHeader(tableHeader);
		
		#if DEBUG>= 3
		cerr<<"CoronalHoleStats Table"<<endl;
		if(regions_stats.size() > 0)
			cerr<<regions_stats[0]->toString("|", true)<<endl;
		else
			cerr<<"Empty"<<endl;
		for (unsigned r = 0; r < regions_stats.size(); ++r)
		{
			cerr<<regions_stats[r]->toString("|")<<endl;
		}
		#endif
		for (unsigned r = 0; r < regions_stats.size(); ++r)
		{
			delete regions_stats[r];
		}
	}
	
	/*! We cleanup */
	for (unsigned r = 0; r < regions.size(); ++r)
	{
		delete regions[r];
	}
	
	if(CHMap != aggregatedMap)
		delete aggregatedMap;
}


