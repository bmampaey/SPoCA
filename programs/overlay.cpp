//! This program plots a map regions contours overlayed on a background image.
/*!
@page overlay overlay.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/overlay.x [-option optionvalue ...]  mapFile fitsFile [ fitsFile ... ] </tt>

@param mapFile	Path to the map
@param fitsFile	Path to a fits file to be converted

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param colors	The list of color of the regions to plot separated by commas. All regions will be selected if ommited.

@param fill	Set this flag if you want to fill holes in the regions before ploting the contours.

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
<BR> Smooth=zz.z (Binomial smoothing of zz.z arcsec)

@param internal	Set this flag if you want the contours inside the regions.
<BR>Will be outside otherwise.

@param upperLabel	The label to write on the upper left corner.
<BR>If set but no value is passed, a default label will be written.
<BR>You can use keywords from the color map fits file by specifying them between {}

@param lowerLabel	The label to write on the lower left corner.
<BR>You can use keywords from the color map fits file by specifying them between {}

@param output	The path of the the output directory.

@param recenter	Set to the position of the new sun center if you want to translate the image

@param registerImages	Set to register/align the images to the map.

@param scaling	Set to the scaling factor if you want to rescale the image.

@param size	The size of the image written. i.e. "1024x1024". See ImageMagick Image Geometry for specification.
<BR>If not set the output image will have the same dimension as the input image.

@param straightenUp	Set if you want to rotate the image so the solar north is up.

@param type	The type of image to write.

@param uniqueColor	Set to a color if you want all contours to be plotted in that color.
<BR>See gradient image for the color number.

@param width	The width of the contour in pixels.

See @ref Compilation_Options for constants and parameters at compilation time.

*/


#include <vector>
#include <iostream>
#include <string>
#include <set>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgParser.h"

#include "../classes/ColorMap.h"
#include "../classes/MagickImage.h"
#include "../classes/EUVImage.h"




using namespace std;

using Magick::Color;
using Magick::ColorGray;
using Magick::Geometry;
using Magick::Quantum;

string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This program plots a map regions contours overlayed on a background image.";
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
	args["imagePreprocessing"] = ArgParser::Parameter('P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n TakeAbs (Take the absolute value)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["upperLabel"] = ArgParser::Parameter("", 'L', "The label to write on the upper left corner.\nIf set but no value is passed, a default label will be written.\nYou can use keywords from the color map fits file by specifying them between {}");
	args["lowerLabel"] = ArgParser::Parameter("{CLASTYPE} {CPREPROC}", 'l', "The label to write on the lower left corner.\nYou can use keywords from the color map fits file by specifying them between {}");
	args["width"] = ArgParser::Parameter(1, 'w', "The width of the contour in pixels.");
	args["internal"] = ArgParser::Parameter(false, 'i', "Set this flag if you want the contours inside the regions.\nWill be outside otherwise.");
	args["fill"] = ArgParser::Parameter(false, 'f', "Set this flag if you want to fill holes in the regions before ploting the contours.");
	args["colors"] = ArgParser::Parameter("", 'C', "The list of color of the regions to plot separated by commas. All regions will be selected if ommited.");
	args["uniqueColor"] = ArgParser::Parameter(7, 'U', "Set to a color if you want all contours to be plotted in that color.\nSee gradient image for the color number.");
	args["registerImages"] = ArgParser::Parameter(false, 'r', "Set to register/align the images to the map.");
	args["straightenUp"] = ArgParser::Parameter(false, 'u', "Set if you want to rotate the image so the solar north is up.");
	args["recenter"] = ArgParser::Parameter("", 'R', "Set to the position of the new sun center if you want to translate the image");
	args["scaling"] = ArgParser::Parameter(1, 's', "Set to the scaling factor if you want to rescale the image.");
	args["size"] = ArgParser::Parameter("100%x100%", 'S', "The size of the image written. i.e. \"1024x1024\". See ImageMagick Image Geometry for specification.\nIf not set the output image will have the same dimension as the input image.");
	args["output"] = ArgParser::Parameter(".", 'O', "The path of the the output directory.");
	args["mapFile"] = ArgParser::PositionalParameter("Path to the map");
	args["fitsFile"] = ArgParser::RemainingPositionalParameters("Path to a fits file to be converted", 1);
	
	// We parse the arguments
	try
	{
		args.parse(argc, argv);
	}
	catch(const invalid_argument& error)
	{
		cerr<<"Error : "<<error.what()<<endl;
		cerr<<args.help_message(argv[0])<<endl;
		return EXIT_FAILURE;
	}
	
	if(!isDir(args["output"]))
	{
		cerr<<"Error : "<<args["output"]<<" is not a directory!"<<endl;
		return EXIT_FAILURE;
	}
	
	filenamePrefix = makePath(args["output"], stripPath(stripSuffix(args["mapFile"]))) + ".";
	
	// We parse the size option
	Magick::Geometry size_geometry(args["size"].as<string>());
	if(!size_geometry.isValid())
	{
		cerr << "Error: Size parameter "<<args["size"]<<" is not a valid specification."<< endl;
		return EXIT_FAILURE;
	}
	
	// We create the contour image
	ColorMap* inputImage = getColorMapFromFile(args["mapFile"]);
	
	// We fill holes if requested
	if(args["fill"])
	{
		inputImage->removeHoles();
	
		#if defined DEBUG
		inputImage->writeFits(filenamePrefix + "filled.fits");
		#endif
	}
	
	// We parse the colors of the regions to plot
	set<ColorType> colors;
	if(args["colors"].is_set())
	{
		vector<ColorType> tmp = toVector<ColorType>(args["colors"]);
		colors.insert(tmp.begin(),tmp.end());
	}
	
	// We erase any colors that is not to be kept
	if(colors.size() > 0 && args["uniqueColor"].is_set())
	{
		ColorType uniqueColor = args["uniqueColor"];
		for(unsigned j = 0; j < inputImage->NumberPixels(); ++j)
		{
			if(inputImage->pixel(j) != inputImage->null())
			{
				if(colors.count(inputImage->pixel(j)) == 0)
				{
					inputImage->pixel(j) = inputImage->null();
				}
				else
				{
					inputImage->pixel(j) = uniqueColor;
				}
			}
		}
	}
	else if(args["uniqueColor"].is_set())
	{
		ColorType uniqueColor = args["uniqueColor"];
		for(unsigned j = 0; j < inputImage->NumberPixels(); ++j)
		{
			if(inputImage->pixel(j) != inputImage->null())
			{
				inputImage->pixel(j) = uniqueColor;
			}
		}
	}
	else if(colors.size() > 0)
	{
		for(unsigned j = 0; j < inputImage->NumberPixels(); ++j)
		{
			if(inputImage->pixel(j) != inputImage->null() && colors.count(inputImage->pixel(j)) == 0)
			{
				inputImage->pixel(j) = inputImage->null();
			}
		}
	}
	
	#if defined DEBUG
	if(colors.size() > 0 || args["uniqueColor"].is_set())
		inputImage->writeFits(filenamePrefix + "recolored.fits");
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
	
	// We plot the contours
	unsigned width = inputImage->Xaxes()/256;
	if(args["width"].is_set())
	{
		width = toUnsigned(args["width"]);
	}
	
	if(args["internal"])
		inputImage->drawInternContours(width, 0);
	else
		inputImage->drawExternContours(width, 0);
	
	#if defined DEBUG
	inputImage->writeFits(filenamePrefix + "contours.fits");
	#endif
	
	// We make the png contours and label it if necessary
	MagickImage contours = inputImage->magick();
	if(args["lowerLabel"].is_set())
	{
		string text = inputImage->getHeader().expand(args["lowerLabel"]);
		size_t text_size = inputImage->Xaxes()/40;
		contours.fillColor("white");
		contours.fontPointsize(text_size);
		contours.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::SouthWestGravity);
	}
	
	#if defined DEBUG
	contours.write(filenamePrefix + "contours." + args["type"]);
	#endif
	
	// We convert the images
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We expand the name of the background fits image with the header of the inputImage
		string imageFilename = inputImage->getHeader().expand(imagesFilenames[p]);
		string outputFilename = makePath(args["output"], stripPath(stripSuffix(imageFilename))) + ".";
		
		if(!isFile(imageFilename))
		{
			cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
			continue;
		}
		
		// We read the sun image for the background
		EUVImage* image = getImageFromFile("Unknown", imageFilename);
		
		// We improve the contrast
		if(args["imagePreprocessing"].is_set())
		{
			image->preprocessing(args["imagePreprocessing"]);
		}
		else
		{
			image->enhance_contrast();
		}
		
		#if defined DEBUG
		image->writeFits(outputFilename + "preprocessed.fits");
		#endif
		
		// We transform the image to align it with the inputImage
		if(args["registerImages"])
		{
			image->align(inputImage);
			#if defined DEBUG
			image->writeFits(outputFilename + "registered.fits");
			#endif
		}

		
		// We make the png background and label it if necessary
		MagickImage outputImage = image->magick();
		if(args["upperLabel"].is_set())
		{
			string text = args["upperLabel"];
			if(text.empty())
				text = image->Label();
			else
				text = image->getHeader().expand(text);
			
			size_t text_size = image->Xaxes()/40;
			outputImage.fillColor("white");
			outputImage.fontPointsize(text_size);
			outputImage.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::NorthWestGravity);
		}
		#if defined DEBUG
		outputImage.write(outputFilename + "background." + args["type"]);
		#endif
		
		delete image;
		
		// We overlay the 2 images
		outputImage.composite(contours, Magick::CenterGravity, Magick::OverCompositeOp);
		
		// We resize the image
		outputImage.scale(size_geometry);
		
		// We write down the image
		outputImage.write(filenamePrefix + "_on_." + stripSuffix(stripPath(imageFilename)) + "." + args["type"]);
	}
	
	delete inputImage;
	return EXIT_SUCCESS;
}


