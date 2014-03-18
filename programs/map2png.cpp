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

@param straightenUp	Set this flag if you want to have the solar north up.

@param recenter	Recenter the sun on the specified position

@param scaling	Scaling factor to resize the image

@param output	The name for the output file/directory.

<BR>N.B.: In the color maps, because there is no colors in fits files, they are represented as a number.
 When creating the png image, a mapping is done from a number to a color.
 That mapping is consistent between images and calls so that a region that has been tracked keep the same color in the successive images. 

See @ref Compilation_Options for constants and parameters at compilation time.

*/

#include <vector>
#include <iostream>
#include <string>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgParser.h"

#include "../classes/ColorMap.h"
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
	string programDescription = "This program convert a color map to a png image\n";
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
	args["transparent"] = ArgParser::Parameter('t', "If you want the null values to be transparent.");
	args["Label"] = ArgParser::Parameter('L', "The label to write on the lower left corner. You can use keywords from the color map fits file by specifying them between {}");
	args["straightenUp"] = ArgParser::Parameter('u', "Set if you want to rotate the image so the solar north is up.");
	args["recenter"] = ArgParser::Parameter('r', "Set to the position of the new sun center ifyou want to translate the image");
	args["scaling"] = ArgParser::Parameter('s', "Set to the scaling factor if you want to rescale the image.");
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
	
	// We parse the background color
	Color backgroundColor(0, 0 ,0, 0);
	if(args["transparent"])
	{
		backgroundColor.alpha(1);
	}
	else
	{
		backgroundColor.alpha(0);
	}
	
	// We convert the images
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		filenamePrefix = makePath(args["output"], stripPath(stripSuffix(imagesFilenames[p]))) + ".";
		ColorMap* inputImage = getColorMapFromFile(imagesFilenames[p]);
	
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
		
		// We make the png
		MagickImage outputImage = inputImage->magick(backgroundColor);
		
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
		
		// We label the image
		if(args["Label"].is_set())
		{
			string text = inputImage->getHeader().expand(args["label"]);
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
