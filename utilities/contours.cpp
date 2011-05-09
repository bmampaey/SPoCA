//! Program that traces the contours of colored regions for display.
/*!
@page contours contours.x

 This program takes color maps in fits format, and for each one extract the contours and writes them to a png image.
 The background will be transparent, so that the contours can be overlayed on another image. 
 The name of the png image will be the name of the original file with "contours" appended.
 
 <BR>N.B.: In the color maps, because there is no colors in fits files, they are represented as a number.
 When creating the png image, a mapping is done from a number to a color.
 That mapping is consistent between images and calls so that a region that has been tracked keep the same color in the successive images.
 
 @section usage Usage
 
 <tt> contours.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> contours.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
@param width	The width of the contour in pixels.

@param internal	Set this flag if you want the contours inside the regions.
<BR>Choose this for example if the regions may touch each other.
 
@param external	Set this flag if you want the contours outside the regions.
<BR> Choose this if you want to see exactly wich pixels are part of the region.

@param fits	Set this flag if you want the output saved as fits instead of png.

@param mastic	Set this flag if you want to fill holes in the connected components before tracing the contours.

@param Label	The label to write in the upper left corner.
<BR>You can use keywords from the fits file of the color map by specifying them between {}. e.g. Wavelength: {WAVELNTH}

@param size The size of the image written. i.e. "1024x1024" See <a href="http://www.imagemagick.org/script/command-line-processing.php#geometry" target="_blank">ImageMagick Image Geometry</a>  for specification.

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

	// The list of names of the images to process
	vector<string> imagesFilenames;

	// Options for the contours
	unsigned width = 5;
	bool external = false;
	bool internal = false;
	
	// Option for the output
	bool fits = false;
	
	// Option for the preprocessing
	bool mastic = false;
	
	// Options for the labeling
	string Label = "{CLASTYPE} {CPREPROC}";
	
	// Option for the output size
	string size;
	
	// option for the output directory
	string outputDirectory = ".";

	
	string programDescription = "This Program makes contours out off color regions.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nColorType: " + string(typeid(ColorType).name());

	ArgumentHelper arguments;
	arguments.new_named_unsigned_int('w', "width", "positive integer", "\n\tThe width of the contour.\n\t", width);
	arguments.new_flag('i', "internal", "\n\tSet this flag if you want the contours inside the regions.\n\t", internal);
	arguments.new_flag('e', "external", "\n\tSet this flag if you want the contours outside the regions.\n\t", external);
	arguments.new_flag('f', "fits", "\n\tSet this flag if you want the output saved as fits.\n\t", fits);
	arguments.new_flag('m', "mastic", "\n\tSet this flag if you want to fill holes before taking the contours.\n\t", mastic);
	arguments.new_named_string('L', "Label", "string", "\n\tThe label for the contours.\n\tYou can use keywords from the color map fits file by specifying them between {}\n\t", Label);
	arguments.new_named_string('S', "size", "string", "\n\tThe size of the image written. i.e. \"1024x1024\"\n\tSee ImageMagick Image Geometry for specification.\n\t", size);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files to draw the contours.\n\t", imagesFilenames);
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
	if(!fits && ! size.empty() && !size_geometry.isValid())
	{
		cerr << "Error parsing size argument: "<<size<<" is not a valid specification."<< endl;
		return 2;
	}
	
	ColorMap image;
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		image.readFits(imagesFilenames[p]);
		filenamePrefix =  stripSuffix(imagesFilenames[p]) + ".contours.";
		
		if(mastic)
			image.removeHoles();
		
		if(internal)
			image.drawInternContours(width, 0);
		else if(external)
			image.drawExternContours(width, 0);
		else
			image.drawContours(width, 0);
			
		if(fits)
		{
			image.writeFits(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[p])) + ".contours.fits");
		}
		else //png
		{
			MagickImage contours = image.magick();
			if(!Label.empty())
			{
				string text = expand(Label, image.header);
				size_t text_size = image.Xaxes()/40;
				contours.fillColor("white");
				contours.fontPointsize(text_size);
				contours.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::NorthWestGravity);
				contours.label(text);
			}
			if(size_geometry.isValid())
				contours.scale(size_geometry);
			
			contours.write(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[p])) + ".contours.png");
		}
		
	}
	return EXIT_SUCCESS;
}
