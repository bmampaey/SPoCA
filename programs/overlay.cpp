//! Program that generates a png to visualize the results of segmentation.
/*!
@page overlay overlay.x

 This program takes one color map in fits format and extract the contours.
 Then for each sun image given it will generate a background image and overlay the contours on top of it.
 
 <BR>N.B.: In the color maps, because there is no colors in fits files, they are represented as a number.
 When creating the png image, a mapping is done from a number to a color.
 That mapping is consistent between images and calls so that a region that has been tracked keep the same color in the successive images.
 
 @section usage Usage
 
 <tt> overlay.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> overlay.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
@param colorizedMap	The name of a colorized Map of regions (i.e. each one must have a different color).
 
@param width	The width of the contour in pixels.
<BR>If not specified an optimal value will be chosen. 

@param internal	Set this flag if you want the contours inside the regions.
<BR>Choose this for example if the regions may touch each other.
 
@param external	Set this flag if you want the contours outside the regions.
<BR> Choose this if you want to see exactly wich pixels are part of the region.

@param mastic	Set this flag if you want to fill holes in the connected components before tracing the contours.

@param Label	The label to write in the upper left corner.
<BR>You can use keywords from the fits file of the color map by specifying them between {}. e.g. Wavelength: {WAVELNTH}

@param label	Set this flag if you want a label on the image.
<BR> The label will state the instrument of observation, wavelength and date of observation of the EUV image.

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

@param colors The list of colors to select separated by commas (no spaces)
<BR>All colors will be selected if ommited.

@param colorsFilename A file containing a list of colors to select separated by commas
<BR>All colors will be selected if ommited.

@param outputDirectory	The name for the output directory.

See @ref Compilation_Options for constants and parameters at compilation time.

*/



#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <set>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/ColorMap.h"
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

	// Options for the contours
	// The map of colored regions
	string colorizedMapFileName;

	// Options for the contours
	unsigned width = 0;
	bool external = false;
	bool internal = false;
	
	// Option for the preprocessing of contours
	bool mastic = false;
	
	// Options for the labeling
	string Label = "{CLASTYPE} {CPREPROC}\ncleaning: {CLEANING} arcsec\naggregation: {AGGREGAT} arcsec\nprojection: sinusoidal\nmin size: {MINSIZE} arcsec²";
	
	// Options for the background
	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;
	
	// Options for the preprocessing of images
	string preprocessingSteps = "";
	
	// Options for the labeling
	bool label = false;
	
	// Option for the output size
	string size;
	
	// Options for the colors to overlay
	string colorsString;
	string colorsFilename;
	bool sameColors = false;
	
	// option for the output directory
	string outputDirectory = ".";

	
	string programDescription = "This program plots region contours overlayed on a background image.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nColorType: " + string(typeid(ColorType).name());
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());

	ArgumentHelper arguments;
	arguments.new_named_unsigned_int('w', "width", "positive integer", "\n\tThe width of the contour.\n\t", width);
	arguments.new_flag('i', "internal", "\n\tSet this flag if you want the contours inside the regions.\n\t", internal);
	arguments.new_flag('e', "external", "\n\tSet this flag if you want the contours outside the regions.\n\t", external);
	arguments.new_flag('m', "mastic", "\n\tSet this flag if you want to fill holes before taking the contours.\n\t", mastic);
	arguments.new_named_string('L', "Label", "string", "\n\tThe label for the contours.\n\tYou can use keywords from the color map fits file by specifying them between {}\n\t", Label);

	arguments.new_named_string('M',"colorizedMap","file name", "\n\tA colorized Map of regions (i.e. each one must have a different color).\n\t", colorizedMapFileName);
	arguments.new_flag('l', "label", "\n\tSet this flag if you want a label on the background.\n\t", label);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);

	arguments.new_named_string('S', "size", "string", "\n\tThe size of the image written. i.e. \"1024x1024\"\n\tSee ImageMagick Image Geometry for specification.\n\t", size);
	
	arguments.new_named_string('c', "colors", "string", "\n\tThe list of colors to select separated by commas (no spaces)\n\tAll colors will be selected if ommited.\n\t", colorsString);
	arguments.new_flag('s', "sameColors", "\n\tSet this flag if want all colors to be the same.\n\t", sameColors);
	arguments.new_named_string('C', "colorsFilename", "string", "\n\tA file containing a list of colors to select separated by commas\n\tAll colors will be selected if ommited.\n\t", colorsFilename);

	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "The name of the fits files containing the images of the sun.", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// We check if the outputDirectory is a directory 
	if (! isDir(outputDirectory))
	{
		cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
		return EXIT_FAILURE;
	}
	
	
	// We parse the size option
	Magick::Geometry size_geometry(size);
	if(! size.empty() && !size_geometry.isValid())
	{
		cerr << "Error parsing size argument: "<<size<<" is not a valid specification."<< endl;
		return 2;
	}
	
	set<ColorType> colors;
	// We parse the colors to overlay
	if(! colorsFilename.empty())
	{
		ifstream colorsFile(colorsFilename.c_str());
		if(colorsFile.good())
		{
			vector<ColorType> tmp;
			colorsFile>>tmp;
			colors.insert(tmp.begin(),tmp.end());
		}
		else
		{
			cerr << "Error reading list of colors to overlay from file: "<<colorsFilename<<endl;
			return 2;
		}
	}
	if(! colorsString.empty())
	{
		vector<ColorType> tmp;
		istringstream ss(colorsString);
		ss>>tmp;
		colors.insert(tmp.begin(),tmp.end());
	}
	
	// We create the contour image
	ColorMap* colorizedMap = getImageFromFile(colorizedMapFileName);
	
	// We erase any colors that is not to be kept
	if(colors.size() > 0)
	{
		for(unsigned j = 0; j < colorizedMap->NumberPixels(); ++j) {
			if(!sameColors) {
				if (colors.count(colorizedMap->pixel(j)) == 0)
					colorizedMap->pixel(j) = colorizedMap->null();
			} else {
				if(colorizedMap->pixel(j) != colorizedMap->null())
					colorizedMap->pixel(j) = *colors.begin();
			}
		}
	}
	
	//We fill holes if requested
	if(mastic)
		colorizedMap->removeHoles();
	
	if(width == 0)
	{
		width = colorizedMap->Xaxes()/256;
	}
	
	if(internal)
		colorizedMap->drawInternContours(width, 0);
	else if(external)
		colorizedMap->drawExternContours(width, 0);
	else
		colorizedMap->drawContours(width, 0);
	
	#if DEBUG >= 2
	colorizedMap->writeFits(outputDirectory + "/" + stripSuffix(stripPath(colorizedMapFileName)) + ".contours.fits");
	#endif
	
	// We make the png contours and label it if necessary
	MagickImage contours = colorizedMap->magick();
	if(!Label.empty())
	{
		string text = expand(Label, colorizedMap->getHeader());
		size_t text_size = colorizedMap->Xaxes()/40;
		contours.fillColor("white");
		contours.fontPointsize(text_size);
		contours.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::NorthWestGravity);
		contours.label(text);
	}
	#if DEBUG >= 2
	contours.write(outputDirectory + "/" + stripSuffix(stripPath(colorizedMapFileName)) + ".contours.png");
	#endif
		
	RealPixLoc sunCenter = colorizedMap->SunCenter();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
	
		// We read the sun image for the background
		string imageFilename = expand(imagesFilenames[p], colorizedMap->getHeader());
		if(! isFile(imageFilename))
		{
			cerr<<"Error : "<<imageFilename<<" is not a regular file!"<<endl;
			continue;
		}
		EUVImage* image = getImageFromFile(imageType, imageFilename);
		image->recenter(sunCenter);
		// We improve the contrast
		if(! preprocessingSteps.empty())
		{
			image->preprocessing(preprocessingSteps);
		}
		else 
		{
			image->enhance_contrast();
		}
		
		#if DEBUG >= 2
		image->writeFits(outputDirectory + "/" + stripSuffix(stripPath(imageFilename)) + ".background.fits");
		#endif
		
		// We make the png background and label it if necessary
		MagickImage background = image->magick();
		if(label)
		{
			string text = image->Label();
			size_t text_size = image->Xaxes()/40;
			background.fillColor("white");
			background.fontPointsize(text_size);
			background.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::SouthWestGravity);
			background.label(text);
		}
		#if DEBUG >= 2
		background.write(outputDirectory + "/" + stripSuffix(stripPath(imageFilename)) + ".background.png");
		#endif
		// We overlay the 2 images and write it down
		background.composite(contours, Magick::CenterGravity, Magick::OverCompositeOp);
		
		if(size_geometry.isValid())
			background.scale(size_geometry);
		
		background.write(outputDirectory + "/" + stripSuffix(stripPath(colorizedMapFileName)) + "_on_" + stripSuffix(stripPath(imageFilename)) + ".png");
	}
	return EXIT_SUCCESS;
}


