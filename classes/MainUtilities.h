// Functions to help main programs

#pragma once
#ifndef MainUtilities_H
#define MainUtilities_H

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "FeatureVector.h"
#include "SunImage.h"
#include "EITImage.h"
#include "EUVIImage.h"
#include "AIAImage.h"
#include "SWAPImage.h"

extern std::string outputFileName;
extern const char * instruments[];
extern const char * limb_corrections[];

//Read the centers and the wavelength from a file
void readCentersFromFile(std::vector<RealFeature>& B, RealFeature& wavelengths, const std::string& centersFileName);


//Read the images from fits files
std::vector<SunImage*> getImagesFromFiles(const std::string type, const std::vector<std::string>& sunImagesFileNames, bool align = false);
#endif
