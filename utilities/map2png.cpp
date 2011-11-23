//! Program that converts a color map in fits format to a png image.
/*!
@page fits2png fits2png.x

 This program takes fits images and transform them to png images.
 The name of the png image will be the name of the original file with the suffix png.
 
 @section usage Usage
 
 <tt> fits2png.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> fits2png.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
@param transparent	If you want the null values to be transparent.

<BR>N.B.: In the color maps, because there is no colors in fits files, they are represented as a number.
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
#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/ColorMap.h"
#include "../classes/EUVImage.h"
#include "../classes/MagickImage.h"
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
	
	// Options for the background of color maps
	bool transparent = false;
	
	// option for the output directory
	string outputDirectory = ".";

	string programDescription = "This Program makes the fits files usable with ImageMagick utilities.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nColorType: " + string(typeid(ColorType).name());

	ArgumentHelper arguments;
	arguments.new_flag('T', "transparent", "\n\tIf you want the null values to be transparent\n\t" , transparent);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
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
	
	MagickImage pngImage;
	
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		FitsFile file(imagesFilenames[p]);
		Header header;
		file.readHeader(header);
		if(isColorMap(header))
		{
			ColorMap image;
			image.readFits(file);
			pngImage = image.magick(background);
		}
		else
		{
			EUVImage image;
			image.readFits(file);
			pngImage = image.magick();
		}
		pngImage.write(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[p])) + ".png");
	}
	return EXIT_SUCCESS;
}
