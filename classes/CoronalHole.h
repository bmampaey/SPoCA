#pragma once
#ifndef CoronalHole_H
#define CoronalHole_H

#include <vector>
#include <string>
#include "EUVImage.h"
#include "ColorMap.h"
#include "Region.h"
#include "RegionStats.h"
#include "FitsFile.h"
#include "ArgParser.h"

/*!
@file CoronalHole.h
List of function to transform the results of the segmentation into a map of Active Region (CH)
*/

//! Return a map of aggregated CH 
ColorMap* getAggregatedCHMap(const ColorMap* map, Real cleaningFactor, Real aggregationFactor, const std::string& projection = "none");

//! Fill a header with all the information about CH map creation
void fillHeaderCH(Header& header, ParameterSection& parameters);

//! Method to write a CH map to a fits file
void writeCHMap(ColorMap*& map, const std::string& filename, const std::vector<EUVImage*>& images, ParameterSection& parameters, bool compressed = true);

//! Parameters for the extraction of CH maps
ParameterSection CHMapParameters();

#endif

