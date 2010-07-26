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
#include "../dsr/ArgumentHelper.h"
#include "../classes/MainUtilities.h"

using namespace std;
using namespace dsr;

string outputFileName;

int main(int argc, const char **argv)
{
	#if defined(DEBUG) && DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif


	// The list of names of the images to process
	vector<string> sunImagesFileNames;

	// Options for the contours
	unsigned width = 5;

	
	string programDescription = "This Programm makes contours out off color regions.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_unsigned_int('w', "width", "positive integer", "\n\tThe width of the contour..\n\t", width);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", outputFileName);
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
		image->drawContours(width);
		outputFileName =  sunImagesFileNames[s].substr(0, sunImagesFileNames[s].find(".fits"));
		image->writeFitsImage(outputFileName + ".contours.fits");
		delete image;
	}
	return EXIT_SUCCESS;
}
