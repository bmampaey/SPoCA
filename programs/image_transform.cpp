#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <limits>

#include "../classes/tools.h"
#include "../classes/constants.h"

#include "../classes/Image.h"
#include "../classes/EUVImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"

#ifndef Real
#define Real float
#endif

using namespace std;
using namespace dsr;

string filenamePrefix;

extern template class Image<Real>;

int main(int argc, const char **argv)
{
	#if defined EXTRA_SAFE
	//feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// The name of the sun image to transform
	string imageType = "UNKNOWN";
	string fitsFileName;
	
	// Options for the transformation
	string sTranslation = "0,0";
	double rotationAngle = 0;
	double scaling = 1;
	bool straightenUp = false;
	
	// Option for the output file/directory
	string output = ".";

	string programDescription = "This program operates a transformation on a image, such as translation, scaling and rotation.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('t', "translation","2 positive real separated by a comma (no spaces)", "\n\tThe translation values in x and y direction\n\t", sTranslation);
	arguments.new_named_double('r', "rotationAngle", "positive real", "\n\tThe angle of rotation in degrees.\n\t",rotationAngle);
	arguments.new_named_double('s', "scaling", "positive real", "\n\tThe scaling factor.\n\t",scaling);
	arguments.new_flag('u', "straightenUp", "\n\tSet this flag if you want to have the solar north up.\n\t", straightenUp);
	arguments.new_named_string('O', "output","file/directory name", "\n\tThe name for the output file/directory.\n\t", output);
	arguments.new_string("fitsFileName", "\n\tThe name of the fits file to transform.\n\t", fitsFileName);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// We check if the output is a directory or a file
	string outputDirectory, outputFileName;
	if (isDir(output))
	{
		outputDirectory = output;
		filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(fitsFileName)) + ".";
		outputFileName = filenamePrefix + "transformed.fits";
	}
	else
	{
		outputDirectory = getPath(output);
		filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(fitsFileName)) + ".";
		outputFileName = output;
		// We check if the outputDirectory exists
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
	}
	
	EUVImage* image = getImageFromFile(imageType, fitsFileName);
	
	RealPixLoc translation(0, 0);
	if(!sTranslation.empty() and !readCoordinate(translation, sTranslation))
	{
		return EXIT_FAILURE;
	}
	
	if(straightenUp)
	{
		rotationAngle = - image->Crota2();
	}
	
	// We apply the transformation
	image->transform(rotationAngle, translation, scaling);
	image->writeFits(outputFileName);
	
	delete image;
	return EXIT_SUCCESS;
}
