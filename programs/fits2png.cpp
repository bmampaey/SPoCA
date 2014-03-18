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
	args["imagePreprocessing"] = ArgParser::Parameter('P', "The steps of preprocessing to apply to the sun images. Can be any combination of the following: NAR=zz.z (Nullify pixels above zz.z*radius); ALC (Annulus Limb Correction); DivMedian (Division by the median); TakeSqrt (Take the square root); TakeLog (Take the log); DivMode (Division by the mode); DivExpTime (Division by the Exposure Time); ThrMin=zz.z (Threshold intensities to minimum zz.z); ThrMax=zz.z (Threshold intensities to maximum zz.z); ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile); ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile); Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["label"] = ArgParser::Parameter('l', "The label to write on the upper left corner. If set but no value is passed, a default label will be written.");
	args["color"] = ArgParser::Parameter('c', "Set if you want the output images to be colorized.");
	args["straightenUp"] = ArgParser::Parameter('u', "Set if you want to rotate the image so the solar north is up.");
	args["recenter"] = ArgParser::Parameter('r', "Set to the position of the new sun center ifyou want to translate the image");
	args["scaling"] = ArgParser::Parameter('s', "Set to the scaling factor if you want to rescale the image.");
	args["colorTable"] = ArgParser::Parameter('C', "Set to an image to use as a color table if you want to colorize the image. If not set the default color table for the instrument/wavelength will be used.");
	args["size"] = ArgParser::Parameter("100%x100%", 'S', "The size of the image written. i.e. \"1024x1024\"See ImageMagick Image Geometry for specification. If not set the output image will have the same dimension as the input image.");
	args["output"] = ArgParser::Parameter(".", 'O', "The path of the the output directory.");
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
				vector<char> intrumentColorTable = inputImage->color_table();
				colorTable = MagickImage(&(intrumentColorTable[0]), 1, intrumentColorTable.size()/3, "RGB");
				#if defined DEBUG
				colorTable.write(filenamePrefix + "colortable.png");
				#endif
				MagickCore::ClutImage(outputImage.image(), colorTable.image());
			}
		}
		
		// We label the image
		if(args["label"].is_set())
		{
			string text = args["label"];
			if(text.empty())
				text = inputImage->Label();
			else
				text = inputImage->getHeader().expand(text);
			
			size_t text_size = inputImage->Xaxes()/40;
			outputImage.fillColor("white");
			outputImage.fontPointsize(text_size);
			outputImage.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::NorthWestGravity);
			outputImage.label(text);
		}
		
		delete inputImage;
		
		// We resize the image
		outputImage.scale(size_geometry);
		
		// We write down the image
		outputImage.write(filenamePrefix + args["type"]);
	}
	
	return EXIT_SUCCESS;
}

