//! This program convert a fits image to a png image, applying some contrast enhancement.
/*!
@page fits2png png.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/fits2png.x [-option optionvalue ...]  fitsFile [ fitsFile ... ] </tt>

@param fitsFile	Path to a fits file to be converted

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param color	Set if you want the output images to be colorized.

@param colorTable	Set to an image to use as a color table if you want to colorize the image.
<BR>If not set the default color table for the instrument/wavelength will be used.

@param imagePreprocessing	The steps of preprocessing to apply to the sun images.
<BR>Can be any combination of the following:
<BR> NAR=zz.z (Nullify pixels above zz.z*radius)
<BR> ALC (Annulus Limb Correction)
<BR> DivMedian (Division by the median)
<BR> TakeSqrt (Take the square root)
<BR> TakeLog (Take the log)
<BR> TakeAbs (Take the absolute value)
<BR> DivMode (Division by the mode)
<BR> DivExpTime (Division by the Exposure Time)
<BR> ThrMin=zz.z (Threshold intensities to minimum zz.z)
<BR> ThrMax=zz.z (Threshold intensities to maximum zz.z)
<BR> ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)
<BR> ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)
<BR> ThrMinMode (Threshold intensities to minimum the mode of pixel intensities)
<BR> ThrMaxMode (Threshold intensities to maximum the mode of pixel intensities)
<BR> Smooth=zz.z (Binomial smoothing of zz.z arcsec)

@param upperLabel	The label to write on the upper left corner.
<BR>If set but no value is passed, a default label will be written.
<BR>You can use keywords from the color map fits file by specifying them between {}

@param output	The path of the output directory.

@param recenter	Set to the position of the new sun center ifyou want to translate the image

@param scaling	Set to the scaling factor if you want to rescale the image.

@param size	The size of the image written. i.e. "1024x1024". See ImageMagick Image Geometry for specification.
<BR>If not set the output image will have the same dimension as the input image.

@param straightenUp	Set if you want to rotate the image so the solar north is up.

@param type	The type of image to write.
See @ref Compilation_Options for constants and parameters at compilation time.

*/


#include <vector>
#include <iostream>
#include <string>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgParser.h"

#include "../classes/EUVImage.h"
#include "../classes/MagickImage.h"

using namespace std;

using Magick::Color;
using Magick::ColorGray;
using Magick::Geometry;
using Magick::Quantum;

string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This program convert a fits image to a png image, applying some contrast enhancement.\n";
	programDescription+="\nVersion: 3.0";
	programDescription+="\nAuthor: Benjamin Mampaey, benjamin.mampaey@sidc.be";
	
	programDescription+="\nCompiled on "  __DATE__  " with options :";
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

	// We define our program parameters
	ArgParser args(programDescription);
	
	args["config"] = ArgParser::ConfigurationFile('C');
	args["help"] = ArgParser::Help('h');
	
	args["type"] = ArgParser::Parameter("png", 'T', "The type of image to write.");
	args["imagePreprocessing"] = ArgParser::Parameter('P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n TakeAbs (Take the absolute value)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile\n ThrMinMode (Threshold intensities to minimum the mode)\n ThrMaxMode (Threshold intensities to maximum the mode)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["upperLabel"] = ArgParser::Parameter("", 'L', "The label to write on the upper left corner.\nIf set but no value is passed, a default label will be written.\nYou can use keywords from the color map fits file by specifying them between {}");
	args["color"] = ArgParser::Parameter(false, 'c', "Set if you want the output images to be colorized.");
	args["straightenUp"] = ArgParser::Parameter(false, 'u', "Set if you want to rotate the image so the solar north is up.");
	args["recenter"] = ArgParser::Parameter("", 'R', "Set to the position of the new sun center ifyou want to translate the image");
	args["scaling"] = ArgParser::Parameter(1, 's', "Set to the scaling factor if you want to rescale the image.");
	args["colorTable"] = ArgParser::Parameter("", 'C', "Set to an image to use as a color table if you want to colorize the image.\nIf not set the default color table for the instrument/wavelength will be used.");
	args["size"] = ArgParser::Parameter("100%x100%", 'S', "The size of the image written. i.e. \"1024x1024\". See ImageMagick Image Geometry for specification.\nIf not set the output image will have the same dimension as the input image.");
	args["output"] = ArgParser::Parameter(".", 'O', "The path of the output directory.");
	args["fitsFile"] = ArgParser::RemainingPositionalParameters("Path to a fits file to be converted", 1);
	
	// We parse the arguments
	try
	{
		args.parse(argc, argv);
	}
	catch ( const invalid_argument& error)
	{
		cerr<<"Error : "<<error.what()<<endl;
		cerr<<args.help_message(argv[0])<<endl;
		return EXIT_FAILURE;
	}
	
	if (! isDir(args["output"]))
	{
		cerr<<"Error : "<<args["output"]<<" is not a directory!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We parse the size option
	Magick::Geometry size_geometry(args["size"].as<string>());
	if(!size_geometry.isValid())
	{
		cerr << "Error: Size parameter "<<args["size"]<<" is not a valid specification."<< endl;
		return EXIT_FAILURE;
	}
	
	// We create the colorTable
	MagickImage colorTable;
	if(args["color"] && args["colorTable"].is_set())
	{
		if(!isFile(args["colorTable"]))
		{
			cerr<<"Error : Cannot find color table image "<<args["colorTable"]<<endl;
			return EXIT_FAILURE;
		}
		else
		{
			colorTable = MagickImage(args["colorTable"].as<string>());
		}
	}
	
	// We convert the images
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		filenamePrefix = makePath(args["output"], stripPath(stripSuffix(imagesFilenames[p]))) + ".";
		
		EUVImage* inputImage = getImageFromFile("Unknown", imagesFilenames[p]);
		
		// We improve the contrast
		if(args["imagePreprocessing"].is_set())
		{
			inputImage->preprocessing(args["imagePreprocessing"]);
		}
		else
		{
			inputImage->enhance_contrast();
		}
	
		#if defined DEBUG
		inputImage->writeFits(filenamePrefix + "preprocessed.fits");
		#endif
	
		// We transform the image
		if(args["straightenUp"] || args["recenter"].is_set() || args["scaling"].is_set())
		{
			// We correct for the roll
			Real rotationAngle = 0;
			if (args["straightenUp"])
			{
				rotationAngle = - inputImage->Crota2();
			}
		
			// We recenter the image
			RealPixLoc newCenter = inputImage->SunCenter();
			if(args["recenter"].is_set() && !readCoordinate(newCenter, args["recenter"]))
			{
				cerr<<"Error : Cannot convert "<<args["recenter"]<<" to coordinates"<<endl;
				return EXIT_FAILURE;
			}
		
			// We scale the image
			Real scaling = 1.;
			if(args["scaling"].is_set())
				scaling = args["scaling"];
		
			inputImage->transform(rotationAngle, RealPixLoc(newCenter.x - inputImage->SunCenter().x, newCenter.y - inputImage->SunCenter().y), scaling);
		
			#if defined DEBUG
			inputImage->writeFits(filenamePrefix + "transformed.fits");
			#endif
		}
		
		MagickImage outputImage = inputImage->magick();
		
		// We color the image
		if(args["color"])
		{
			if(args["colorTable"].is_set())
			{
				// We use the color table from the parameters
				MagickCore::ClutImage(outputImage.image(), colorTable.image());
			}
			else
			{
				vector<unsigned char> intrumentColorTable = inputImage->color_table();
				colorTable = MagickImage(&(intrumentColorTable[0]), 1, intrumentColorTable.size()/3, "RGB");
				#if defined DEBUG
				colorTable.write(filenamePrefix + "colortable." + args["type"]);
				#endif
				MagickCore::ClutImage(outputImage.image(), colorTable.image());
			}
		}
		
		// We label the image
		if(args["upperLabel"].is_set())
		{
			string text = args["upperLabel"];
			if(text.empty())
				text = inputImage->Label();
			else
				text = inputImage->getHeader().expand(text);
			
			size_t text_size = inputImage->Xaxes()/40;
			outputImage.fillColor("white");
			outputImage.fontPointsize(text_size);
			outputImage.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::NorthWestGravity);
		}
		
		delete inputImage;
		
		// We resize the image
		outputImage.scale(size_geometry);
		
		// We write down the image
		outputImage.write(filenamePrefix + args["type"]);
	}
	
	return EXIT_SUCCESS;
}
