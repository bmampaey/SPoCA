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

string outputFileName;

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
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

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
		outputFileName =  stripSuffix(imagesFilenames[p]) + ".contours.";
		
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
			image.writeFits(outputFileName + "fits");
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
						unsigned indice = (unsigned(image.pixel(x, y)) % gradientMax) + 1 ;
						pngImage.pixelColor(x, image.Yaxes() - y - 1, Color(magick_gradient[indice]));
					}
				}
			}
			pngImage.write(outputFileName + "png");
		}
		
	}
	return EXIT_SUCCESS;
}
