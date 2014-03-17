#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <utility>
#include <ctype.h>

#include "../classes/tools.h"
#include "../classes/constants.h"

#include "../classes/Image.h"
#include "../classes/SunImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"
#include "../classes/Region.h"
#include "../classes/Coordinate.h"


#ifndef Real
#define Real float
#endif

using namespace std;
using namespace dsr;

string filenamePrefix;

extern template class Image<Real>;

// Function to parse the color LUT
std::map<ColorType,ColorType> parse_LUT(const string& filename)
{
	std::map<ColorType,ColorType> LUT;
	ifstream file(filename.c_str());
	ColorType from, to;
	while(file.good())
	{
		while(file.good() && !isdigit(char(file.peek())))
		{
			file.get();
		}
		if (file.good())
			file>>from;
		while(file.good() && !isdigit(char(file.peek())))
		{
			file.get();
		}
		if (file.good())
			file>>to;
		if (file.good())
			LUT[from] = to;
	}
	return LUT;
}


int main(int argc, const char **argv)
{
	#if defined EXTRA_SAFE
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif
	
	// Options for colorLUT
	string colorLUT = "none";
	
	// option for the output directory
	string outputDirectory = ".";
	
	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	string programDescription = "This Program recolor a map using a color LUT.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + toString(NUMBERCHANNELS);
	#if defined DEBUG
	programDescription+="\nDEBUG: ON";
	#endif
	#if defined EXTRA_SAFE
	programDescription+="\nEXTRA_SAFE: ON";
	#endif
	#if defined VERBOSE
	programDescription+="\nVERBOSE: ON";
	#endif
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('c', "colorLUT", "filename", "\n\tThe color LUT\n\t", colorLUT);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of color maps.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);
	
	// Parse the color LUT
	std::map<ColorType,ColorType> LUT = parse_LUT(colorLUT);
	
	// We read and recolor the files
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		filenamePrefix = outputDirectory+"/"+stripPath(stripSuffix(imagesFilenames[p]))+".";
		ColorMap* map = getImageFromFile(imagesFilenames[p]);
		
		// We recolor the map
		map->recolorizeConnectedComponents(LUT);
		
		// We write the map
		map->writeFits(outputDirectory+"/"+stripPath(stripSuffix(imagesFilenames[p]))+ ".recolored.fits");
	}
	return EXIT_SUCCESS;
}
