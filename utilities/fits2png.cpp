//! Program that converts a color map in fits format to a png image.
/*!
@page fits2png fits2png.x

 This program takes color maps in fits format, and for each one writes it to a png image.
 The name of the png image will be the name of the original file with the suffix png.
 
 @section usage Usage
 
 <tt> fits2png.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> fits2png.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
@param transparent	If you want the null values to be transparent.

@param colorize	If you want to colorize the regions.
<BR>N.B.: In fits files, there is no colors, so they are represented as a number in the color maps.
 When creating the png image, a mapping is done from a number to a color.
 That mapping is consistent between images and calls so that a region that has been tracked keep the same color in the successive images. 

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
#include "../classes/gradient.h"
#include "../classes/ArgumentHelper.h"


using namespace std;
using namespace dsr;
using Magick::Color;
using Magick::ColorGray;
using Magick::Geometry;
using Magick::Quantum; 

string filenamePrefix;

int main(int argc, const char **argv)
{
	#if DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif
	
	// The list of names of the sun images to process
	vector<string> imagesFilenames;
	
	// Options for the colorisation
	bool transparent = false;
	bool colorize = false;

	string programDescription = "This Program makes the fits files usable with ImageMagick utilities.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nColorType: " + string(typeid(ColorType).name());

	ArgumentHelper arguments;
	arguments.new_flag('T', "transparent", "\n\tIf you want the null values to be transparent\n\t" , transparent);
	arguments.new_flag('C', "colorize", "\n\tIf you want to colorize the image\n\t" , colorize);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	Color background(0, 0 ,0, 0);
	if(transparent)
	{
	    background.alphaQuantum(MaxRGB);
	}
	else
	{
	    background.alphaQuantum(0);
	}

	ColorMap image;
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		
		image.readFits(imagesFilenames[p]);
		unsigned Xaxes = image.Xaxes();
		unsigned Yaxes = image.Yaxes();

		Magick::Image pngImage( Geometry(Xaxes, Yaxes), background );
		//pngImage.type(Magick::TrueColorMatteType);
    
		for (unsigned y = 0; y < Yaxes; ++y)
		{
		    for (unsigned x = 0; x < Xaxes; ++x)
		    {	
				if(image.pixel(x, y) != image.nullvalue() )
				{
					unsigned indice = (image.pixel(x, y) % gradientMax) + 1 ;
					if(colorize)
					{
						pngImage.pixelColor(x, Yaxes - y - 1, Color(magick_gradient[indice]));
					}
					else
					{
						pngImage.pixelColor(x, Yaxes - y - 1, ColorGray(double(indice * MaxRGB)/gradientMax));
					}
				}
			}
		}

		filenamePrefix =  stripSuffix(imagesFilenames[p]);
		pngImage.write(filenamePrefix + ".png");
	}
	return EXIT_SUCCESS;
}
