//! Program that traces the contours of colored regions for display.
/*!
@page contours contours.x

 This program takes color maps in fits format, and for each one extract the contours and writes them to a png image.
 The background will be transparent, so that the contours can be overlayed on another image. 
 The name of the png image will be the name of the original file with "contours" appended.
 
 N.B.: In fits files, there is no colors, so they are represented as a number in the color maps.
 When creating the png image, a mapping is done from a number to a color.
 That mapping is consistent between images and calls so that a region that has been tracked keep the same color in the successive images. 
 
 @section usage Usage
 
 <tt> contours.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> contours.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
@param width	The width of the contour in pixels.

@param internal	Set this flag if you want the contours inside the regions.
<BR>Choose this for example if the regions may touch each other.
 
@param external	Set this flag if you want the contours outside the regions.
<BR> Choose this if you want to see exactly wich pixels are part of the region.

@param fits	Set this flag if you want the output saved as fits instead of png.

@param mastic	Set this flag if you want to fill holes in the connected components before tracing the contours.


See @ref Compilation_Options for constants and parameters at compilation time.

*/



#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <Magick++.h>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/ColorMap.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/gradient.h"

using namespace std;
using namespace dsr;

using Magick::Color;
using Magick::ColorGray;
using Magick::Geometry;
using Magick::Quantum;

string filenamePrefix;

int main(int argc, const char **argv)
{

	// The list of names of the images to process
	vector<string> imagesFilenames;

	// Options for the contours
	unsigned width = 5;
	bool external = false;
	bool internal = false;
	
	// Option for the output
	bool fits = false;
	
	// Option for the preprocessing
	bool mastic = false;

	
	string programDescription = "This Program makes contours out off color regions.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nColorType: " + string(typeid(ColorType).name());

	ArgumentHelper arguments;
	arguments.new_named_unsigned_int('w', "width", "positive integer", "\n\tThe width of the contour.\n\t", width);
	arguments.new_flag('i', "internal", "\n\tSet this flag if you want the contours inside the regions.\n\t", internal);
	arguments.new_flag('e', "external", "\n\tSet this flag if you want the contours outside the regions.\n\t", external);
	arguments.new_flag('f', "fits", "\n\tSet this flag if you want the output saved as fits.\n\t", fits);
	arguments.new_flag('m', "mastic", "\n\tSet this flag if you want to fill holes before taking the contours.\n\t", mastic);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files to draw the contours.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	Color background(0, 0 ,0, MaxRGB);

	ColorMap image;
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		image.readFits(imagesFilenames[p]);
		filenamePrefix =  stripSuffix(imagesFilenames[p]) + ".contours.";
		
		if(mastic)
			image.removeHoles();
		
		if(internal)
			image.drawInternContours(width, 0);
		else if(external)
			image.drawExternContours(width, 0);
		else
			image.drawContours(width, 0);
			
		if(fits)
		{
			image.writeFits(filenamePrefix + "fits");
		}
		else //png
		{
			Magick::Image pngImage( Geometry(image.Xaxes(), image.Yaxes()), background );
			for (unsigned y = 0; y < image.Yaxes(); ++y)
			{
				for (unsigned x = 0; x < image.Xaxes(); ++x)
				{	
					if(image.pixel(x, y) != image.nullvalue() )
					{
						ColorType indice = (image.pixel(x, y) % gradientMax) + 1 ;
						pngImage.pixelColor(x, image.Yaxes() - y - 1, Color(magick_gradient[indice]));
					}
				}
			}
			pngImage.write(filenamePrefix + "png");
		}
		
	}
	return EXIT_SUCCESS;
}
