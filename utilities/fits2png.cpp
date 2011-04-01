// This programm will make a fits file readable by Imagemagick
// Written by Benjamin Mampaey on 14 July 2010

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

string outputFileName;

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
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

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
				if(image.pixel(x, y) != image.nullvalue())
				{
					unsigned indice = (unsigned(image.pixel(x, y)) % gradientMax) + 1 ;
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

		outputFileName =  stripSuffix(imagesFilenames[p]);
		pngImage.write(outputFileName + ".png");
	}
	return EXIT_SUCCESS;
}

