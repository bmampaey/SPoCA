#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/Image.h"
#include "../classes/ArgumentHelper.h"

using namespace std;
using namespace dsr;

string outputFileName;

int main(int argc, const char **argv)
{

	// The list of names of the images to process
	vector<string> sunImagesFileNames;

	// Options for the contours
	unsigned width = 5;
	bool external = false;
	bool internal = false;

	
	string programDescription = "This Program makes contours out off color regions.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_unsigned_int('w', "width", "positive integer", "\n\tThe width of the contour..\n\t", width);
	arguments.new_flag('i', "internal", "\n\tSet this flag if you want the contours inside the regions.\n\t", internal);
	arguments.new_flag('e', "external", "\n\tSet this flag if you want the contours outside the regions..\n\t", external);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files to draw the contours.\n\t", sunImagesFileNames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	Image<PixelType>* image = NULL;

	for (unsigned s = 0; s < sunImagesFileNames.size(); ++s)
	{
		image = new Image<PixelType>(sunImagesFileNames[s]);
		
		if(internal)
			image->drawInternContours(width, 0);
		else if(external)
			image->drawExternContours(width, 0);
		else
			image->drawContours(width, 0);
			
		outputFileName =  sunImagesFileNames[s].substr(0, sunImagesFileNames[s].find(".fits"));
		image->writeFitsImage(outputFileName + ".contours.fits");
		delete image;
	}
	return EXIT_SUCCESS;
}
