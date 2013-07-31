//! Program that convert 3 EUV fits file into a RGB png image.
/*!
@page RGBcomposite RGBcomposite.x

 This program takes 3 EUV images in fits format, and compose them as a RGB color image, each fits file corresponding to a color channel.
 By default, the name of the png image will be the composition of the names of the 3 fits files (with the png suffix).
 
 @section usage Usage
 
 <tt> RGBcomposite.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> RGBcomposite.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 fitsFileName3</tt>

fitsFileName1 corresponds to the red channel
fitsFileName2 corresponds to the green channel
fitsFileName3 corresponds to the blue channel

@param label	Set this flag if you want a label on the image.
<BR> The label will state the instrument of observation, wavelength and date of observation of the last EUV image.


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
	// The list of names of the images to process
	vector<string> fitsFileNames;
	
	// Options for the preprocessing of images
	string preprocessingSteps = "";
		
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
	
	string programDescription = "This Program generates a RGB image out of 3 fits files.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());

	ArgumentHelper arguments;
	arguments.new_flag('l', "label", "\n\tSet this flag if you want a label on the background.\n\t", label);
	arguments.new_flag('u', "straightenUp", "\n\tSet this flag if you want to have the solar north up.\n\t", straightenUp);
	arguments.new_named_string('r', "recenter", "2 positive real separated by a comma (no spaces)", "\n\tThe position of the new center\n\t", recenter);
	arguments.new_named_double('s', "scaling", "positive real", "\n\tThe scaling factor.\n\t", scaling);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_string('S', "size", "string", "\n\tThe size of the image written. i.e. \"1024x1024\"\n\tSee ImageMagick Image Geometry for specification.\n\t", size);
	arguments.new_named_string('O', "output","file/directory name", "\n\tThe name for the output file/directory.\n\t", output);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 fitsFileName3", "\n\tThe name of the fits files to use to make the composite, in Red Green Blue order.\n\t", fitsFileNames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// We check that we received 3 files
	if(fitsFileNames.size() != 3)
	{
		cerr << "Error : You must provide exactly 3 files."<< endl;
		return 2;
	}
	
	// We check if the output is a directory or a file
	string outputDirectory, outputFileName;
	if (isDir(output))
	{
		outputDirectory = output;
		outputFileName = outputDirectory + "/" + stripSuffix(stripPath(fitsFileNames[0])) + "_" + stripSuffix(stripPath(fitsFileNames[1])) + "_"+ stripSuffix(stripPath(fitsFileNames[2])) + ".composite.png";
	}
	else
	{
		outputDirectory = getPath(output);
		outputFileName = output;
		// We check if the outputDirectory exists
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : directory "<<outputDirectory<<" does not exists!"<<endl;
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
	
	// We make the gradients
	Magick::Image red_gradient( "1x2", "black" );
	red_gradient.pixelColor( 0, 1, "red" );
	Magick::Image green_gradient( "1x2", "black" );
	green_gradient.pixelColor( 0, 1, "green1" );
	Magick::Image blue_gradient( "1x2", "black" );
	blue_gradient.pixelColor( 0, 1, "blue" ); 
	
	// We make the red channel with the first image
	filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(fitsFileNames[0])) + ".";
	EUVImage* inputImage = getImageFromFile("UNKNOWN", fitsFileNames[0]);
	
	// We improve the contrast
	if(! preprocessingSteps.empty())
	{
		inputImage->preprocessing(preprocessingSteps);
	}
	else 
	{
		inputImage->enhance_contrast();
	}
	
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
	
	RealPixLoc referenceSunCenter = inputImage->SunCenter();
	Real referenceCrota2 = inputImage->Crota2();
	Real referenceSunRadius = inputImage->SunRadius();
	
	// We make the png
	MagickImage red_channel = inputImage->magick();
	delete inputImage;
	MagickCore::ClutImage(red_channel.image(), red_gradient.image());
	#if defined DEBUG
		red_channel.write(filenamePrefix + "red.png");
	#endif
	
	// We make the green channel with the second image
	filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(fitsFileNames[1])) + ".";
	inputImage = getImageFromFile("UNKNOWN", fitsFileNames[1]);
	
	// We improve the contrast
	if(! preprocessingSteps.empty())
	{
		inputImage->preprocessing(preprocessingSteps);
	}
	else 
	{
		inputImage->enhance_contrast();
	}
	
	// We transform the image
	inputImage->transform(referenceCrota2 - inputImage->Crota2(), RealPixLoc(referenceSunCenter.x - inputImage->SunCenter().x, referenceSunCenter.y - inputImage->SunCenter().y), referenceSunRadius/inputImage->SunRadius());
	#if defined DEBUG
	inputImage->writeFits(filenamePrefix + "transformed.fits");
	#endif
	
	// We make the png
	MagickImage green_channel = inputImage->magick();
	delete inputImage;
	MagickCore::ClutImage(green_channel.image(), green_gradient.image());
	#if defined DEBUG
		green_channel.write(filenamePrefix + "green.png");
	#endif
	
	// We make the blue channel with the third image
	filenamePrefix = outputDirectory + "/" + stripSuffix(stripPath(fitsFileNames[2])) + ".";
	inputImage = getImageFromFile("UNKNOWN", fitsFileNames[2]);
	// We improve the contrast
	if(! preprocessingSteps.empty())
	{
		inputImage->preprocessing(preprocessingSteps);
	}
	else 
	{
		inputImage->enhance_contrast();
	}
	
	// We transform the image
	inputImage->transform(referenceCrota2 - inputImage->Crota2(), RealPixLoc(referenceSunCenter.x - inputImage->SunCenter().x, referenceSunCenter.y - inputImage->SunCenter().y), referenceSunRadius/inputImage->SunRadius());
	#if defined DEBUG
	inputImage->writeFits(filenamePrefix + "transformed.fits");
	#endif
	
	// We make the png
	MagickImage blue_channel = inputImage->magick();
	MagickCore::ClutImage(blue_channel.image(), blue_gradient.image());
	#if defined DEBUG
		blue_channel.write(filenamePrefix + "blue.png");
	#endif
	
	// We compose the channels together
	blue_channel.composite(red_channel, Magick::CenterGravity, Magick::PlusCompositeOp);
	blue_channel.composite(green_channel, Magick::CenterGravity, Magick::PlusCompositeOp);
	
	if(label)
	{
		string text = inputImage->Label();
		size_t text_size = inputImage->Xaxes()/40;
		blue_channel.fillColor("white");
		blue_channel.fontPointsize(text_size);
		blue_channel.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::NorthWestGravity);
		blue_channel.label(text);
	}
	delete inputImage;
	
	if(size_geometry.isValid())
		blue_channel.scale(size_geometry);
	
	blue_channel.write(outputFileName);

	return EXIT_SUCCESS;
}


