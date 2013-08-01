//! Program that convert an EUV fits file into a png image suitable for viewing.
/*!
@page outputImage outputImage.x

 This program takes a EUV image in fits format and creates an image file suitable for viewing.
 By default the fits file will be converted to a png image named like the input fits file (with the png suffix).
  
 @section usage Usage
 
 <tt> outputImage.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> outputImage.x [-option optionvalue, ...] fitsFileName</tt>

@param label	Set this flag if you want a label on the image.
<BR> The label will state the instrument of observation, wavelength and date of observation of the EUV image.

@param colorTable	Image to use as a color table if you want to colorize the image

@param preprocessingSteps	The steps of preprocessing to apply to the sun images.
<BR>Possible values :
 - NAR (Nullify above radius)
 - ALC (Annulus Limb Correction)
 - DivMedian (Division by the median)
 - TakeSqrt (Take the square root)
 - TakeLog (Take the log)
 - DivMode (Division by the mode)
 - DivExpTime (Division by the Exposure Time)
 - ThrMinzz.z (Threshold intensities to minimum the zz.z percentile) 
 - ThrMaxzz.z (Threshold intensities to maximum the zz.z percentile)
 - Smoothzz.z Binomial smoothing of zz.z arcsec

@param size The size of the image written. i.e. "1024x1024" See <a href="http://www.imagemagick.org/script/command-line-processing.php#geometry" target="_blank">ImageMagick Image Geometry</a>  for specification.

@param straightenUp	Set this flag if you want to have the solar north up.

@param recenter	Recenter the sun on the specified position

@param scaling	Scaling factor to resize the image

@param output	The name for the output file/directory.

See @ref Compilation_Options for constants and parameters at compilation time.

*/


#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <map>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/EUVImage.h"
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

	// The name of the fits file to convert
	string fitsFileName;
	
	// Options for the preprocessing of images
	string preprocessingSteps = "";
	
	// Options for the colorisation
	bool color = false;
	string colorTableName;
	
	// Options for the labeling
	bool label = false;
	
	// Option for the output size
	string size;
	
	// Options for the transformation
	bool straightenUp = false;
	string recenter;
	double scaling = 1;
	
	// Option for the output file/directory
	string output = ".";
	
	string programDescription = "This program transform a fits image in a png image, applying some contrast enhancement.\n";
	programDescription+="Compiled with options :";
	#if defined DEBUG
	programDescription+="\nDEBUG: ON";
	#endif
	#if defined EXTRA_SAFE
	programDescription+="\nEXTRA_SAFE: ON";
	#endif
	#if defined VERBOSE
	programDescription+="\VERBOSE: ON";
	#endif
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());

	ArgumentHelper arguments;
	arguments.new_flag('l', "label", "\n\tSet this flag if you want a label on the outputImage.\n\t", label);
	arguments.new_flag('c', "color", "\n\tSet this flag if you want the outputImage to be colorized.\n\t", color);
	arguments.new_flag('u', "straightenUp", "\n\tSet this flag if you want to have the solar north up.\n\t", straightenUp);
	arguments.new_named_string('r', "recenter", "2 positive real separated by a comma (no spaces)", "\n\tThe position of the new center\n\t", recenter);
	arguments.new_named_double('s', "scaling", "positive real", "\n\tThe scaling factor.\n\t", scaling);
	arguments.new_named_string('C', "colorTable", "image name", "\n\tImage to use as a color table if you want to colorize the image.\n\t" , colorTableName);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
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
	
	// We parse the size option
	Magick::Geometry size_geometry(size);
	if(! size.empty() && !size_geometry.isValid())
	{
		cerr << "Error parsing size argument: "<<size<<" is not a valid specification."<< endl;
		return 2;
	}
	
	// We create the colorTable
	MagickImage colorTable;
	if(color && !colorTableName.empty())
	{
		if(!isFile(colorTableName))
		{
			cerr<<"Error : "<<colorTableName<<" is not a file!"<<endl;
			return EXIT_FAILURE;
		}
		else
		{
			colorTable = MagickImage(colorTableName);
		}
	}
	
	EUVImage* inputImage = getImageFromFile("UNKNOWN", fitsFileName);
	
	// We improve the contrast
	if(! preprocessingSteps.empty())
	{
		inputImage->preprocessing(preprocessingSteps);
	}
	else 
	{
		inputImage->enhance_contrast();
	}
	
	#if defined DEBUG
	inputImage->writeFits(filenamePrefix + "preprocessed.fits");
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
	if(color)
	{
		if(!colorTableName.empty())
		{
			MagickCore::ClutImage(outputImage.image(), colorTable.image());
		}
		else
		{
			vector<char> intrumentColorTable = inputImage->color_table();
			colorTable = MagickImage(&(intrumentColorTable[0]), 1, intrumentColorTable.size()/3, "RGB");
			#if defined DEBUG
			colorTable.write(filenamePrefix + "colortable.png");
			#endif
			MagickCore::ClutImage(outputImage.image(), colorTable.image());
		}
	}
	
	if(label)
	{
		string text = inputImage->Label();
		size_t text_size = inputImage->Xaxes()/40;
		outputImage.fillColor("white");
		outputImage.fontPointsize(text_size);
		outputImage.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::NorthWestGravity);
		outputImage.label(text);
	}
	
	delete inputImage;
	
	if(size_geometry.isValid())
		outputImage.scale(size_geometry);
	
	outputImage.write(outputFileName);
	
	return EXIT_SUCCESS;
}

