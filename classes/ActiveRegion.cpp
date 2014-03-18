#include "ActiveRegion.h"

using namespace std;
extern std::string filenamePrefix;

ColorMap* getAggregatedARMap(const ColorMap* ARMap, const int projection)
{

	string filename = filenamePrefix + "ARMap.";

	Real cleaningFactor = Real(AR_CLEANING) / sqrt(ARMap->PixelArea());
	Real aggregationFactor = Real(AR_AGGREGATION) / sqrt(ARMap->PixelArea());
	
	ColorMap* aggregated = new ColorMap(ARMap);

	#if defined DEBUG
	aggregated->writeFits(filename + "pure.fits");
	#endif
	
	/*! Apply the projection */
	switch(projection)
	{
		case(SunImage<ColorType>::equirectangular):
			
			aggregated->equirectangular_projection(ARMap, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "equirectangular_projection.fits");
			#endif
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			aggregated->Lambert_cylindrical_projection(ARMap, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "Lambert_cylindrical_projection.fits");
			#endif
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		break;
		
		case(SunImage<ColorType>::sinusoidal):
			aggregated->sinusoidal_projection(ARMap, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "sinusoidal_projection.fits");
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
	
	#if defined DEBUG
	aggregated->writeFits(filename + "eroded.fits");
	#endif
	
	/*! Aggregate the blobs together */
	aggregated->dilateCircular(cleaningFactor + aggregationFactor, 0);
	
	#if defined DEBUG
	aggregated->writeFits(filename + "dilated.fits");
	#endif
	
	/*! Give back the original size */
	aggregated->erodeCircular(aggregationFactor, 0);
	
	#if defined DEBUG
	aggregated->writeFits(filename + "closed.fits");
	#endif
	
	/* Apply the deprojection */
	ColorMap* projeted = NULL;
	switch(projection)
	{
		case(SunImage<ColorType>::equirectangular):
			projeted = new ColorMap(aggregated);
			aggregated->equirectangular_deprojection(projeted, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "equirectangular_deprojection.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			projeted = new ColorMap(aggregated);
			aggregated->Lambert_cylindrical_deprojection(projeted, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "Lambert_cylindrical_deprojection.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::sinusoidal):
			projeted = new ColorMap(aggregated);
			aggregated->sinusoidal_deprojection(projeted, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "sinusoidal_deprojection.fits");
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
	
	#if defined DEBUG
	aggregated->writeFits(filename + "aggregated.fits");
	#endif

	return aggregated;
}

void fillHeaderAR(Header& header)
{
	string projection;
	switch(AR_PROJECTION)
	{
		case(SunImage<ColorType>::equirectangular):
			projection = "equirectangular";
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			projection = "Lambert cylindrical";
		break;
		
		case(SunImage<ColorType>::sinusoidal):
			projection = "sinusoidal";
		break;
		
		case(SunImage<ColorType>::distance_transform):
			projection = "Distance transform";
		break;
		
		default:
			projection = "None";
	}
	
	header.set("CLEANING", AR_CLEANING, "Cleaning factor in arcsec");
	header.set("AGGREGAT", AR_AGGREGATION, "Aggregation factor in arcsec");
	header.set("PROJECTN", projection, "Projection used for the aggregation");
	header.set("MINSIZE", MIN_AR_SIZE, "Min size of regions in arcsec²");
	header.set("THRAWAR", AR_TRA, "Min size threshold on raw area");
	header.set("FRAGGMTD", AR_FRAGMENTED, "Regions are fragmented");
}

void writeARMap(ColorMap*& ARMap, const string& filename, vector<EUVImage*> images, bool compressed, unsigned chaincodeMinPoints, unsigned chaincodeMaxPoints, Real chaincodeMaxDeviation)
{
	/*! Clean everything above the radius */
	ARMap->nullifyAboveRadius(1.);
	
	/*! We get the map of aggregated AR */
	ColorMap* aggregatedMap = getAggregatedARMap(ARMap, AR_PROJECTION);
	
	if(AR_FRAGMENTED)
	{
		/*! Color the pixels using the aggregated map */
		for (unsigned j = 0; j < aggregatedMap->NumberPixels(); ++j)
		{
			if(ARMap->pixel(j) == 1)
			{
				ARMap->pixel(j) = aggregatedMap->pixel(j);
			}
		}
	}
	else
	{
		delete ARMap;
		ARMap = aggregatedMap;
	}

	#if defined DEBUG
	ARMap->writeFits(filenamePrefix + "ARMap.all.fits");
	#endif

	/*! Erase the small regions */
	if(AR_TRA)
		ARMap->thresholdRegionsByRawArea(MIN_AR_SIZE);
	else
		ARMap->thresholdRegionsByRealArea(MIN_AR_SIZE);

	#if defined DEBUG
	ARMap->writeFits(filenamePrefix + "ARMap.large.fits");
	#endif
	
	/*! We write the map of AR to the fits file */
	FitsFile file(filename, FitsFile::overwrite);
	ARMap->writeFits(file, compressed ? FitsFile::compress : 0, "ActiveRegionMap");
	
	/*! We write some info about the ARMap creation */
	Header header = ARMap->getHeader();
	fillHeaderAR(header);
	file.writeHeader(header);
	
	/*! We get the regions */
	vector<Region*> regions = getRegions(ARMap);
	
	/*! We write the regions to the fits file */
	file.writeTable("Regions");
	writeRegions(file, regions);
	
	/*! We get the chaincode and write them to the fits file */
	if(chaincodeMaxPoints > 0)
	{
		#if defined DEBUG
		ColorMap chaincode_map(ARMap);
		#endif
		
		file.writeTable("ChainCodes");
		for (unsigned r = 0; r < regions.size(); ++r)
		{
			vector<PixLoc> chaincode = regions[r]->chainCode(aggregatedMap, chaincodeMinPoints, chaincodeMaxPoints, chaincodeMaxDeviation);
			file.writeColumn(toString(regions[r]->Id(),7), chaincode);
		
			#if defined DEBUG
			for (unsigned c = 0; c < chaincode.size(); ++c)
			{
				chaincode_map.drawCircle(chaincode[c], 3, r+10);
			}
			#endif
		}
		#if defined DEBUG
		chaincode_map.writeFits(filenamePrefix + "ARMap.chaincodes.fits");
		#endif
	}
	
	/*! We write intensities statistics relative to image */
	for(unsigned i = 0; i < images.size(); ++i)
	{
		EUVImage* image = images[i];
		/*! We get the stats of the AR */
		vector<RegionStats*> regions_stats =  getRegionStats(ARMap, image, regions);
	
		/*! We write the AR Stats into the fits */
		file.writeTable(image->Channel()+"_ActiveRegionStats");
		writeRegions(file, regions_stats);
		
		/*! We write some info bout the image in the header of the table */
		Header tableHeader;
		if(dynamic_cast<EUVImage*>(image))
			tableHeader.set("WAVELNTH", dynamic_cast<EUVImage*>(image)->Wavelength(), "Wavelength of the intensity image.");
		
		const Header& imageHeader = image->getHeader();
		if(imageHeader.has("LVL_NUM"))
			tableHeader.set("LVL_NUM", imageHeader.get<float>("LVL_NUM"), "Level number of the intensity image.");
		if(imageHeader.has("DATE"))
			tableHeader.set("DATE", imageHeader.get<string>("DATE"), "Creation date of the intensity image.");
		if(imageHeader.has("QUALITY"))
			tableHeader.set("QUALITY", imageHeader.get<unsigned>("QUALITY"), "Quality level of the intensity image.");
		
		file.writeHeader(tableHeader);
		
		#if defined VERBOSE
		cout<<"ActiveRegionStats Table"<<endl;
		if(regions_stats.size() > 0)
			cout<<regions_stats[0]->toString("|", true)<<endl;
		else
			cout<<"Empty"<<endl;
		for (unsigned r = 0; r < regions_stats.size(); ++r)
		{
			cout<<regions_stats[r]->toString("|")<<endl;
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
	
	if(ARMap != aggregatedMap)
		delete aggregatedMap;
}


