// This programm will make a fits file readable by Imagemagick
// Written by Benjamin Mampaey on 14 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/SunImage.h"
#include "../classes/gradient.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"

using namespace std;
using namespace dsr;

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
	bool modulo = false;

	string programDescription = "This Program makes the fits files usable with ImageMagick utilities.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_flag('M', "modulo", "\n\tIf you want the colors to be between 1 and gradientMax\n\t" , modulo);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	SunImage* image = NULL;

	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		image = new SunImage(imagesFilenames[p]);

		for (unsigned j = 0; j < image->NumberPixels(); ++j)
		{
			if(image->pixel(j) == image->nullvalue() )
			{
				image->pixel(j) = 0;
			}
			else if(modulo)
			{
				image->pixel(j) = 1 + (int(image->pixel(j)) % gradientMax);
			}
			else
			{
				image->pixel(j) += 1;
			}
		}
		image->pixel(0) = 0;
		image->pixel(1) = gradientMax;
		outputFileName =  stripSuffix(imagesFilenames[p]);
		image->writeFitsImage(outputFileName + ".magick.fits");
		delete image;
	}
	return EXIT_SUCCESS;
}
