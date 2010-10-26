// This programm will give the image resulting from the quotien or the difference of 2 images
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
#include "../classes/Image.h"
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
	bool recenter = false;
	
	// Options for the operation
	bool quotient = false;
	bool difference = false;

	string programDescription = "This Program will produce the image resulting from the quotient or the difference of 2 images.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_flag('Q', "quotient", "\n\tIf you want to produce the quotient of the 2 images\n\t" , quotient);
	arguments.new_flag('D', "difference", "\n\tIf you want to produce the difference of the 2 images\n\t" , difference);
	arguments.new_flag('r', "recenter", "\n\tIf you wantthe second image to be shifted to have the same sun center than the first image.\n\t" , recenter);
	arguments.set_string_vector("fitsFileName1 fitsFileName2", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.new_named_string('O',"outputFile","outputFile", "The name for the output file(s).", outputFileName);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	if((!difference && !quotient) || (difference && quotient))
	{
		cerr<<"Error : you must provide one and only one of -Q or -D."<<endl;
		return EXIT_FAILURE;
	}
	vector<SunImage*> images;
	if(recenter)
		images = getImagesFromFiles("UNKNOWN", imagesFilenames, recenter);
	else
		images = getImagesFromFiles("SunImage", imagesFilenames, recenter);
	if(images.size() != 2)
	{
		cerr<<"Error : you must provide exactly 2 fits files."<<endl;
		return EXIT_FAILURE;
	}
	if(outputFileName.empty())
	{
		if(quotient)
			outputFileName =  stripPath(stripSuffix(imagesFilenames[0])) + "_Q_" +  stripPath(stripSuffix(imagesFilenames[1])) + ".fits";
		else
			outputFileName = stripPath(stripSuffix(imagesFilenames[0])) + "_D_" + stripPath(stripSuffix(imagesFilenames[1])) + ".fits";
	}
	
	if(quotient)
	{
		images[0]->div(images[1]);
	}
	else
	{
		images[0]->diff(images[1]);
	}
	
	images[0]->writeFitsImage(outputFileName);

	return EXIT_SUCCESS;
}
