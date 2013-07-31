//! Program that traces the contours of colored regions for display.
/*!
@page contours contours.x

 This program takes a color map in fits format, extracts the contours and writes them to a png/fits image.
 The background will be transparent, so that the contours can be overlayed on another image. 
 By default the contours will be written to a png image named like the input fits file with "contours" appended.
 
 <BR>N.B.: In color maps, colors are represented as a numbers, because there is no rgb channels in fits files, 
 When creating the png image, a mapping is done from a number to a color.
 That mapping is consistent between images and calls so that a region that has been tracked keep the same color in the successive images.
 
 @section usage Usage
 
 <tt> contours.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> contours.x [-option optionvalue, ...] fitsFileName </tt>
 
@param width	The width of the contour in pixels.

@param internal	Set this flag if you want the contours inside the regions.
<BR>Choose this for example if the regions may touch each other.
 
@param external	Set this flag if you want the contours outside the regions.
<BR> Choose this if you want to see exactly wich pixels are part of the region.

@param mastic	Set this flag if you want to fill holes in the connected components before tracing the contours.

@param Label	The label to write in the lower left corner.
<BR>You can use keywords from the fits file of the color map by specifying them between {}. e.g. Wavelength: {WAVELNTH}

@param size The size of the image written. i.e. "1024x1024" See <a href="http://www.imagemagick.org/script/command-line-processing.php#geometry" target="_blank">ImageMagick Image Geometry</a>  for specification.

@param straightenUp	Set this flag if you want to have the solar north up.

@param recenter	Recenter the sun on the specified position

@param scaling	Scaling factor to resize the image

@param output	The name for the output file/directory.

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

	// The name of the fits file to process
	string fitsFileName;

	// Options for the contours
	unsigned width = 0;
	bool external = false;
	bool internal = false;
	
	// Option for the preprocessing
	bool mastic = false;
	
	// Options for the labeling
	string Label = "{CLASTYPE} {CPREPROC}";
	
	// Option for the output size
	string size;
	
	// Options for the transformation
	bool straightenUp = false;
	string recenter;
	double scaling = 1;
	
	// Option for the output file/directory
	string output = ".";

	
	string programDescription = "This program plots region contours.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nColorType: " + string(typeid(ColorType).name());

	ArgumentHelper arguments;
	arguments.new_named_unsigned_int('w', "width", "positive integer", "\n\tThe width of the contour.\n\t", width);
	arguments.new_flag('i', "internal", "\n\tSet this flag if you want the contours inside the regions.\n\t", internal);
	arguments.new_flag('e', "external", "\n\tSet this flag if you want the contours outside the regions.\n\t", external);
	arguments.new_flag('m', "mastic", "\n\tSet this flag if you want to fill holes before taking the contours.\n\t", mastic);
	arguments.new_flag('u', "straightenUp", "\n\tSet this flag if you want to have the solar north up.\n\t", straightenUp);
	arguments.new_named_string('r', "recenter", "2 positive real separated by a comma (no spaces)", "\n\tThe position of the new center\n\t", recenter);
	arguments.new_named_double('s', "scaling", "positive real", "\n\tThe scaling factor.\n\t", scaling);
	arguments.new_named_string('L', "Label", "string", "\n\tThe label for the contours.\n\tYou can use keywords from the color map fits file by specifying them between {}\n\t", Label);
	arguments.new_named_string('S', "size", "string", "\n\tThe size of the image written. i.e. \"1024x1024\"\n\tSee ImageMagick Image Geometry for specification.\n\t", size);
	arguments.new_named_string('O', "output","file/directory name", "\n\tThe name for the output file/directory.\n\t", output);
	arguments.new_string("fitsFileName", "\n\tThe name of the fits files to process.\n\t", fitsFileName);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);
	
	bool outputFileAsfits = false;
	
	// We check if the output is a directory or a file
	string outputDirectory, outputFileName;
	if (isDir(output))
	{
		outputDirectory = output;
		filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(fitsFileName)) + ".";
		outputFileName = filenamePrefix + "contours.png";
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

	
	// We parse the size option
	Magick::Geometry size_geometry(size);
	if(!outputFileAsfits && ! size.empty() && !size_geometry.isValid())
	{
		cerr << "Error parsing size argument: "<<size<<" is not a valid specification."<< endl;
		return 2;
	}
	
	// We read the file
	ColorMap* inputImage = getImageFromFile(fitsFileName);
	
	if(mastic)
		inputImage->removeHoles();
	
	if(width == 0)
	{
		width = inputImage->Xaxes()/256;
	}
	
	if(internal)
		inputImage->drawInternContours(width, 0);
	else if(external)
		inputImage->drawExternContours(width, 0);
	else
		inputImage->drawContours(width, 0);
		
	#if defined DEBUG
	inputImage->writeFits(filenamePrefix + "contours.fits");
	#endif
	
	// We transform the image
	if(straightenUp or !recenter.empty() or scaling != 1.)
	{
		// We correct for the roll
		Real rotationAngle = 0;
		if (straightenUp)
		{
			rotationAngle = - inputImage->Crota2();
		}
		
		// We recenter the image
		RealPixLoc newCenter = inputImage->SunCenter();
		if(!recenter.empty() and !readCoordinate(newCenter, recenter))
		{
			return EXIT_FAILURE;
		}
		inputImage->transform(rotationAngle, RealPixLoc(newCenter.x - inputImage->SunCenter().x, newCenter.y - inputImage->SunCenter().y), scaling);
		#if defined DEBUG
		inputImage->writeFits(filenamePrefix + "transformed.fits");
		#endif
		
	}
	
	MagickImage outputImage = inputImage->magick();
	if(!Label.empty())
	{
		string text = expand(Label, inputImage->header);
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
