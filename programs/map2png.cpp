//! Program that converts a color map in fits format to a png image.
/*!
@page fits2png fits2png.x

 This program takes a color map image in fits format and creates an image file suitable for viewing.
 By default the fits file will be converted to a png image named like the input fits file (with the png suffix).
 
 @section usage Usage
 
 <tt> map2png.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> map2png.x [-option optionvalue, ...] fitsFileName </tt>
 
@param transparent	If you want the null values to be transparent.

@param Label	The label to write in the lower left corner.
<BR>You can use keywords from the fits file of the color map by specifying them between {}. e.g. Wavelength: {WAVELNTH}

@param size The size of the image written. i.e. "1024x1024" See <a href="http://www.imagemagick.org/script/command-line-processing.php#geometry" target="_blank">ImageMagick Image Geometry</a>  for specification.

@param output	The name for the output file/directory.

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
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/ColorMap.h"
#include "../classes/MagickImage.h"



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
	
	// The name of the fits file to convert
	string fitsFileName;
	
	// Options for the background of color maps
	bool transparent = false;
	
	// Options for the labeling
	string Label = "{CLASTYPE} {CPREPROC}";
	
	// Option for the output size
	string size;
	
	// option for the output file/directory
	string output = ".";

	string programDescription = "This program transform a color map into a png.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nColorType: " + string(typeid(ColorType).name());

	ArgumentHelper arguments;
	arguments.new_flag('T', "transparent", "\n\tIf you want the null values to be transparent\n\t" , transparent);
	arguments.new_named_string('L', "Label", "string", "\n\tThe label for the contours.\n\tYou can use keywords from the color map fits file by specifying them between {}\n\t", Label);
	arguments.new_named_string('S', "size", "string", "\n\tThe size of the image written. i.e. \"1024x1024\"\n\tSee ImageMagick Image Geometry for specification.\n\t", size);
	arguments.new_named_string('O', "output","file/directory name", "\n\tThe name for the output file/directory.\n\t", output);
	arguments.new_string("fitsFileName", "\n\tThe name of the fits files to convert.\n\t", fitsFileName);
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
		outputFileName = filenamePrefix + "png";
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
	
	Color background(0, 0 ,0, 0);
	if(transparent)
	{
		background.alphaQuantum(MaxRGB);
	}
	else
	{
		background.alphaQuantum(0);
	}
	
	// We parse the size option
	Magick::Geometry size_geometry(size);
	if(! size.empty() && !size_geometry.isValid())
	{
		cerr << "Error parsing size argument: "<<size<<" is not a valid specification."<< endl;
		return 2;
	}
	
	// We read the file
	ColorMap* inputImage = getImageFromFile(fitsFileName);
	// We make the png
	MagickImage outputImage = inputImage->magick();

	if(!Label.empty())
	{
		string text = expand(Label, inputImage->getHeader());
		size_t text_size = inputImage->Xaxes()/40;
		outputImage.fillColor("white");
		outputImage.fontPointsize(text_size);
		outputImage.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::SouthWestGravity);
		outputImage.label(text);
	}
	
	delete inputImage;
	
	if(size_geometry.isValid())
		outputImage.scale(size_geometry);
	
	outputImage.write(outputFileName);
	
	return EXIT_SUCCESS;
}
