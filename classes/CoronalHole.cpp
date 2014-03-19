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
	
	#if defined DEBUG
	aggregated->writeFits(filename + "pure.fits");
	#endif

	/*! Apply the projection */
	switch(projection)
	{
		case(SunImage<ColorType>::equirectangular):
			
			aggregated->equirectangular_projection(CHMap, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "equirectangular_projection.fits");
			#endif
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			aggregated->Lambert_cylindrical_projection(CHMap, false);
			#if defined DEBUG
			aggregated->writeFits(filename + "Lambert_cylindrical_projection.fits");
			#endif
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		break;
		
		case(SunImage<ColorType>::sinusoidal):
			aggregated->sinusoidal_projection(CHMap, false);
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

void fillHeaderCH(Header& header)
{
	string projection;
	switch(CH_PROJECTION)
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
	header.set("INSTRUME", "Coronal Hole Map", "SPoCA coronal hole map");
	header.set("CLEANING", CH_CLEANING, "Cleaning factor in arcsec");
	header.set("AGGREGAT", CH_AGGREGATION, "Aggregation factor in arcsec");
	header.set("PROJECTN", projection, "Projection used for the aggregation");
	header.set("MINSIZE", MIN_CH_SIZE, "Min size of regions in arcsec²");
	header.set("THRAWAR", CH_TRA, "Min size threshold on raw area");
	header.set("FRAGGMTD", CH_FRAGMENTED, "Regions are fragmented");
}

void writeCHMap(ColorMap*& CHMap, const string& filename, vector<EUVImage*> images, bool compressed, unsigned chaincodeMinPoints, unsigned chaincodeMaxPoints, Real chaincodeMaxDeviation)
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

	#if defined DEBUG
	CHMap->writeFits(filenamePrefix + "CHMap.all.fits");
	#endif

	/*! Erase the small regions */
	if(CH_TRA)
		CHMap->thresholdRegionsByRawArea(MIN_CH_SIZE);
	else
		CHMap->thresholdRegionsByRealArea(MIN_CH_SIZE);

	#if defined DEBUG
	CHMap->writeFits(filenamePrefix + "CHMap.large.fits");
	#endif
	
	/*! We write the map of CH to the fits file */
	FitsFile file(filename, FitsFile::overwrite);
	CHMap->writeFits(file, compressed ? FitsFile::compress : 0, "CoronalHoleMap");
	
	/*! We write some info about the CHMap creation */ 
	Header header = CHMap->getHeader();
	fillHeaderCH(header);
	file.writeHeader(header);
	
	/*! We get the regions */
	vector<Region*> regions = getRegions(CHMap);
	
	/*! We write the regions to the fits file */
	file.writeTable("Regions");
	writeRegions(file, regions);
	
	/*! We get the chaincode and write them to the fits file */
	if(chaincodeMaxPoints > 0)
	{
		#if defined DEBUG
		ColorMap chaincode_map(CHMap);
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
		chaincode_map.writeFits(filenamePrefix + "CHMap.chaincodes.fits");
		#endif
	}
	
	/*! We write intensities statistics relative to image */
	for(unsigned i = 0; i < images.size(); ++i)
	{
		EUVImage* image = images[i];
		
		/*! We get the stats of the CH */
		vector<RegionStats*> regions_stats =  getRegionStats(CHMap, image, regions);
	
		/*! We write the CH Stats into the fits */
		file.writeTable(image->Channel()+"_CoronalHoleStats");
		writeRegions(file, regions_stats);
		
		/*! We write some info about the image in the header of the table */
		Header tableHeader;
		tableHeader.set("CHANNEL", image->Channel(), "Channel of the intensity image.");
		tableHeader.set("WAVELNTH", image->Wavelength(), "Wavelength of the intensity image.");
		tableHeader.set("DATE-OBS", image->ObservationDate(), "Observation Date of the intensity image.");
		tableHeader.set("INSTRUME", image->Instrument(), "Instrument of the intensity image.");
		const Header& imageHeader = image->getHeader();
		if(imageHeader.has("LVL_NUM"))
			tableHeader.set("LVL_NUM", imageHeader.get<float>("LVL_NUM"), "Level number of the intensity image.");
		if(imageHeader.has("DATE"))
			tableHeader.set("DATE", imageHeader.get<string>("DATE"), "Creation date of the intensity image.");
		if(imageHeader.has("QUALITY"))
			tableHeader.set("QUALITY", imageHeader.get<unsigned>("QUALITY"), "Quality level of the intensity image.");
		
		file.writeHeader(tableHeader);
		
		#if defined VERBOSE
		cout<<"CoronalHoleStats Table"<<endl;
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
	
	if(CHMap != aggregatedMap)
		delete aggregatedMap;
}


