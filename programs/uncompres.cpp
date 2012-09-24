// This program will decompress a fits file
// Written by Benjamin Mampaey on 14 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
# include <algorithm>

#include "../classes/EUVImage.h"
#include "../classes/ColorMap.h"
#include "../classes/FitsFile.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"


using namespace std;
using namespace dsr;

string filenamePrefix;


int main(int argc, const char **argv)
{
	#if DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	string programDescription = "This Program will uncompres a fits files.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());

	ArgumentHelper arguments;
	arguments.set_string_vector("input_fitsFileName output_fitsFileName", "", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	if(imagesFilenames.size() != 2)
	{
		cerr<<"Error : "<<imagesFilenames.size()<<" fits image file name given as parameter, 2 must be given!"<<endl;
		return EXIT_FAILURE;
	}
	FitsFile file(imagesFilenames[0]);
	Header header;
	file.readHeader(header);
	if(isColorMap(header))
	{
		ColorMap image;
		image.readFits(file);
		image.writeFits(imagesFilenames[1]);
	}
	else
	{
		EUVImage image;
		image.readFits(file);
		image.writeFits(imagesFilenames[1]);
	}

	return EXIT_SUCCESS;
}

