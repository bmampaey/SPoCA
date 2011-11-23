//! Program that convert an EUV fits file into a png image suitable for viewing.
/*!
@page background background.x

 This program takes EUV images in fits format, and for each one create a png file suitable for viewing.
 The name of the png image will be the name of the original file (with the png suffix).
  
 @section usage Usage
 
 <tt> background.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> background.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>

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

@param outputDirectory	The name for the output directory.

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
	vector<string> imagesFilenames;
	
	// Options for the preprocessing of images
	string preprocessingSteps = "";
	
	// Options for the colorisation
	bool color = false;
	string colorTableName;
	
	// Options for the labeling
	bool label = false;
	
	// Option for the output size
	string size;
	
	// option for the output directory
	string outputDirectory = ".";
	
	string programDescription = "This Program makes background out off color regions.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());

	ArgumentHelper arguments;
	arguments.new_flag('l', "label", "\n\tSet this flag if you want a label on the background.\n\t", label);
	arguments.new_flag('c', "color", "\n\tSet this flag if you want the background to be colorized.\n\t", color);
	arguments.new_named_string('C', "colorTable", "image name", "\n\tImage to use as a color table if you want to colorize the image.\n\t" , colorTableName);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_string('S', "size", "string", "\n\tThe size of the image written. i.e. \"1024x1024\"\n\tSee ImageMagick Image Geometry for specification.\n\t", size);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files to draw the background.\n\t", imagesFilenames);
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
	

	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		EUVImage* image = getImageFromFile("UNKNOWN", imagesFilenames[p]);
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
		image->writeFits(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[p])) + ".fits");
		#endif
		MagickImage background = image->magick();
		if(color)
		{
			if(!colorTableName.empty())
			{
				MagickCore::ClutImage(background.image(), colorTable.image());
			}
			else
			{
				vector<char> intrumentColorTable = image->color_table();
				colorTable = MagickImage(&(intrumentColorTable[0]), 1, intrumentColorTable.size()/3, "RGB");
				//#if DEBUG >= 2
				colorTable.write(outputDirectory + "/" + "colortable.png");
				//#endif
				MagickCore::ClutImage(background.image(), colorTable.image());
			}
		}

		if(label)
		{
			string text = image->Label();
			size_t text_size = image->Xaxes()/40;
			background.fillColor("white");
			background.fontPointsize(text_size);
			background.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::SouthWestGravity);
			background.label(text);
		}
		if(size_geometry.isValid())
			background.scale(size_geometry);
		
		background.write(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[p])) + ".png");
	}
	return EXIT_SUCCESS;
}

