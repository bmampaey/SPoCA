#include "CoronalHole.h"

using namespace std;
extern std::string filenamePrefix;

ColorMap* getAggregatedCHMap(const ColorMap* map, Real cleaningFactor, Real aggregationFactor, const string& projection)
{
	// We convert the factors from arcsec to pixels
	cleaningFactor /= sqrt(map->PixelArea());
	aggregationFactor /= sqrt(map->PixelArea());
	
	ColorMap* aggregated = new ColorMap(map);

	#if defined DEBUG
	aggregated->writeFits(filenamePrefix + "pure.fits");
	#endif
	
	if(projection == "exact")
	{
		/*! Clean the color map to remove very small components (like protons)*/
		aggregated->erodeCircularProjected(cleaningFactor, 0);
	
		#if defined DEBUG
		aggregated->writeFits(filenamePrefix + "eroded.fits");
		#endif
	
		/*! Aggregate the blobs together */
		aggregated->dilateCircularProjected(cleaningFactor + aggregationFactor, 0);
	
		#if defined DEBUG
		aggregated->writeFits(filenamePrefix + "dilated.fits");
		#endif
	
		/*! Give back the original size */
		aggregated->erodeCircularProjected(aggregationFactor, 0);
	
		#if defined DEBUG
		aggregated->writeFits(filenamePrefix + "closed.fits");
		#endif
	}
	else
	{
		/*! Apply the projection */
		if(projection == "equirectangular")
		{
			aggregated->equirectangular_projection(map, false);
			#if defined DEBUG
			aggregated->writeFits(filenamePrefix + "equirectangular_projection.fits");
			#endif
			// We adjust the factors because in the projection the pixel size changes
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		}
		else if(projection == "lambert")
		{
			aggregated->Lambert_cylindrical_projection(map, false);
			#if defined DEBUG
			aggregated->writeFits(filenamePrefix + "Lambert_cylindrical_projection.fits");
			#endif
			// We adjust the factors because in the projection the pixel size changes
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		}
		else if(projection == "sinusoidal")
		{
			aggregated->sinusoidal_projection(map, false);
			#if defined DEBUG
			aggregated->writeFits(filenamePrefix + "sinusoidal_projection.fits");
			#endif
			// We adjust the factors because in the projection the pixel size changes
			cleaningFactor *= (2./3.);
			aggregationFactor *= (2./3.);
		}
		else if(projection != "none")
		{
			cerr<<"Unknown projection type "<<projection<<endl;
			exit(EXIT_FAILURE);
		}
	
		/*! Clean the color map to remove very small components (like protons)*/
		aggregated->erodeCircular(cleaningFactor, 0);
	
		#if defined DEBUG
		aggregated->writeFits(filenamePrefix + "eroded.fits");
		#endif
	
		/*! Aggregate the blobs together */
		aggregated->dilateCircular(cleaningFactor + aggregationFactor, 0);
	
		#if defined DEBUG
		aggregated->writeFits(filenamePrefix + "dilated.fits");
		#endif
	
		/*! Give back the original size */
		aggregated->erodeCircular(aggregationFactor, 0);
	
		#if defined DEBUG
		aggregated->writeFits(filenamePrefix + "closed.fits");
		#endif
	
		/* Apply the deprojection */
		if(projection == "equirectangular")
		{
			ColorMap* projeted = new ColorMap(aggregated);
			aggregated->equirectangular_deprojection(projeted, false);
			delete projeted;
			#if defined DEBUG
			aggregated->writeFits(filenamePrefix + "equirectangular_deprojection.fits");
			#endif
		
		}
		else if(projection == "lambert")
		{
			ColorMap* projeted = new ColorMap(aggregated);
			aggregated->Lambert_cylindrical_deprojection(projeted, false);
			delete projeted;
			#if defined DEBUG
			aggregated->writeFits(filenamePrefix + "Lambert_cylindrical_deprojection.fits");
			#endif
		}
		else if(projection == "sinusoidal")
		{
			ColorMap* projeted = new ColorMap(aggregated);
			aggregated->sinusoidal_deprojection(projeted, false);
			delete projeted;
			#if defined DEBUG
			aggregated->writeFits(filenamePrefix + "sinusoidal_deprojection.fits");
			#endif
		}
		else if(projection != "none")
		{
			cerr<<"Unknown projection type "<<projection<<endl;
			exit(EXIT_FAILURE);
		}
	}
	/*! Remove the parts off limb and colorize */
	aggregated->nullifyAboveRadius(1.); 
	aggregated->colorizeConnectedComponents(1);
	
	#if defined DEBUG
	aggregated->writeFits(filenamePrefix + "aggregated.fits");
	#endif

	return aggregated;
}

void fillHeaderCH(Header& header, ParameterSection& parameters)
{
	header.set("INSTRUME", "Coronal Region Map", "SPoCA coronal region map");
	header.set("CLEANING", parameters["cleaning"], "Cleaning factor in arcsec");
	header.set("AGGREGAT", parameters["aggregation"], "Aggregation factor in arcsec");
	header.set("PROJECTN", parameters["projection"], "Projection used for the cleaning and aggregation");
	header.set("MINSIZE", parameters["minimalSize"], "Min size of regions in arcsec²");
	header.set("THRAWAR", parameters["useRawArea"], "Min size threshold on raw area");
	header.set("FRAGGMTD", !parameters["aggregated"], "Regions are fragmented");
}

void writeCHMap(ColorMap*& map, const string& filename, const vector<EUVImage*>& images, ParameterSection& parameters, bool compressed)
{
	/*! Clean everything above the radius */
	map->nullifyAboveRadius(1.);
	
	/*! We get the map of aggregated CH */
	ColorMap* aggregatedMap = parameters["projection"].is_set() ? getAggregatedCHMap(map, parameters["cleaning"], parameters["aggregation"], parameters["projection"]) : getAggregatedCHMap(map, parameters["cleaning"], parameters["aggregation"], "none");
	
	if(parameters["aggregated"])
	{
		delete map;
		map = aggregatedMap;
	}
	else
	{
		/*! Color the pixels using the aggregated map */
		for (unsigned j = 0; j < aggregatedMap->NumberPixels(); ++j)
		{
			if(map->pixel(j) == 1)
			{
				map->pixel(j) = aggregatedMap->pixel(j);
			}
		}
	}
	
	#if defined DEBUG
	map->writeFits(filenamePrefix + "all.fits");
	#endif
	
	/*! Erase the small regions */
	if(parameters["useRawArea"])
		map->thresholdRegionsByRawArea(parameters["minimalSize"]);
	else
		map->thresholdRegionsByRealArea(parameters["minimalSize"]);

	#if defined DEBUG
	map->writeFits(filenamePrefix + "large.fits");
	#endif
	
	/*! We write the map of CH to the fits file */
	FitsFile file(filename, FitsFile::overwrite);
	map->writeFits(file, compressed ? FitsFile::compress : 0, "CoronalHoleMap");
	
	/*! We write some info about the map creation */
	Header header = map->getHeader();
	fillHeaderCH(header, parameters);
	file.writeHeader(header);
	
	/*! We get the regions */
	vector<Region*> regions = getRegions(map);
	
	/*! We write the regions to the fits file */
	file.writeTable("Regions");
	writeRegions(file, regions);
	
	/*! We get the chaincode and write them to the fits file */
	if(parameters["chaincodeMaxPoints"].as<int>() > 0)
	{
		#if defined DEBUG
		ColorMap chaincode_map(map);
		#endif
		
		file.writeTable("ChainCodes");
		for (unsigned r = 0; r < regions.size(); ++r)
		{
			vector<PixLoc> chaincode = regions[r]->chainCode(aggregatedMap, parameters["chaincodeMinPoints"], parameters["chaincodeMaxPoints"], parameters["chaincodeMaxDeviation"]);
			file.writeColumn(toString(regions[r]->Id(),7), chaincode);
		
			#if defined DEBUG
			for (unsigned c = 0; c < chaincode.size(); ++c)
			{
				chaincode_map.drawCircle(chaincode[c], 3, r+10);
			}
			#endif
		}
		#if defined DEBUG
		chaincode_map.writeFits(filenamePrefix + "chaincodes.fits");
		#endif
	}
	
	/*! We write intensities statistics relative to image */
	for(unsigned i = 0; i < images.size(); ++i)
	{
		EUVImage* image = images[i];
		/*! We get the stats of the CH */
		vector<RegionStats*> regions_stats =  getRegionStats(map, image, regions);
	
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
	
	if(map != aggregatedMap)
		delete aggregatedMap;
}

ParameterSection CHMapParameters()
{
	ParameterSection parameters;
	parameters["chaincodeMinPoints"] = ArgParser::Parameter(4, 'x', "The minimal number of points in a chaincode.");
	parameters["chaincodeMaxPoints"] = ArgParser::Parameter(0, 'X', "The maximal number of points in a chaincode.");
	parameters["chaincodeMaxDeviation"] = ArgParser::Parameter(0, 'd', "The maximal deviation of the chaincode curve between 2 points, in arcsec.");
	parameters["cleaning"] = ArgParser::Parameter(6, "Cleaning factor in arcsec.");
	parameters["aggregation"] = ArgParser::Parameter(32, "Aggregation factor in arcsec.");
	parameters["minimalSize"] = ArgParser::Parameter(3000, "Minal size of regions in arcsec². Smaller regions will be discarded");
	parameters["projection"] = ArgParser::Parameter("sinusoidal", "Projection used for the aggregation.");
	parameters["aggregated"] = ArgParser::Parameter(false, "Aggregate regions so that one region correspond to only one connected component");
	parameters["useRawArea"] = ArgParser::Parameter(false, "When discarding small regions, use raw area instead of real area.");
	return parameters;
}


