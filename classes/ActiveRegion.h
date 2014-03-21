#pragma once
#ifndef ActiveRegion_H
#define ActiveRegion_H

#include <vector>
#include <string>
#include "EUVImage.h"
#include "ColorMap.h"
#include "Region.h"
#include "RegionStats.h"
#include "FitsFile.h"
#include "ArgParser.h"

/*!
@file ActiveRegion.h
List of function to transform the results of the segmentation into a map of Active Region (AR)
*/

//! Return a map of aggregated AR 
ColorMap* getAggregatedARMap(const ColorMap* map, Real cleaningFactor, Real aggregationFactor, const std::string& projection = "none");

//! Fill a header with all the information about AR map creation
void fillHeaderAR(Header& header, ParameterSection& parameters);

//! Method to write a AR map to a fits file
void writeARMap(ColorMap*& map, const std::string& filename, const std::vector<EUVImage*>& images, ParameterSection& parameters, bool compressed = true);

//! Parameters for the extraction of AR maps
ParameterSection ARMapParameters();

#endif

