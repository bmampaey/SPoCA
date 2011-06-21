//! Program that convert 3 EUV fits file into a RGB png image.
/*!
@page RGBcomposite RGBcomposite.x

 This program takes 3 EUV images in fits format, and compose them as a RGB color image, each fits file corresponding to a color channel.
 The name of the png image will be the composition of the names of the 3 fits files (with the png suffix).
  
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




void AIA_contrast_0304(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(4.99941/exposureTime);
	image->threshold(50, 2000);
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		image->pixel(j) = log10(image->pixel(j));
	}
}

void AIA_contrast_0335(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(6.99734/exposureTime);
	image->threshold(3.5, 1000);
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		image->pixel(j) = log10(image->pixel(j));
	}
}

void AIA_contrast_1600(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(2.99911/exposureTime);
	image->threshold(0, 1000);
}

void AIA_contrast_0193(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(2.99950/exposureTime);
	image->threshold(120, 6000);
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		image->pixel(j) = log10(image->pixel(j));
	}
}

void AIA_contrast_0094(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(4.99803/exposureTime);
	image->threshold(1.5, 50);
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		image->pixel(j) = sqrt(image->pixel(j));
	}
}

void AIA_contrast_0171(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(4.99803/exposureTime);
	image->threshold(10, 6000);
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		image->pixel(j) = sqrt(image->pixel(j));
	}
}

void AIA_contrast_0211(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(4.99801/exposureTime);
	image->threshold(30, 13000);
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		image->pixel(j) = log10(image->pixel(j));
	}
}

void AIA_contrast_0131(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(6.99685/exposureTime);
	image->threshold(7, 1200);
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
	{
		image->pixel(j) = log10(image->pixel(j));
	}
}

void AIA_contrast_1700(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(1.00026/exposureTime);
	image->threshold(0, 2500);
}

void AIA_contrast_4500(EUVImage * image)
{
	Real exposureTime = image->ExposureTime();
	image->mul(1.00026/exposureTime);
	image->threshold(0, 26000);
}
void enhance_contrast(EUVImage * image, string preprocessingSteps)
{
	if(! preprocessingSteps.empty())
	{
		image->preprocessing(preprocessingSteps);
	}
	else if(isAIA(image->header))
	{
		if(image->Wavelength() == 304.)
			AIA_contrast_0304(image);

		else if(image->Wavelength() == 335. )
			AIA_contrast_0335(image);

		else if(image->Wavelength() == 1600. )
			AIA_contrast_1600(image);

		else if(image->Wavelength() == 193. )
			AIA_contrast_0193(image);

		else if(image->Wavelength() == 94. )
			AIA_contrast_0094(image);

		else if(image->Wavelength() == 171. )
			AIA_contrast_0171(image);

		else if(image->Wavelength() == 211. )
			AIA_contrast_0211(image);

		else if(image->Wavelength() == 131. )
			AIA_contrast_0131(image);

		else if(image->Wavelength() == 1700. )
			AIA_contrast_1700(image);

		else if(image->Wavelength() == 4500. )
			AIA_contrast_4500(image);

		else
			cerr<<"Error: unknown wavelength for AIA "<<image->Wavelength()<<endl;

	}
}


int main(int argc, const char **argv)
{

	// The list of names of the images to process
	vector<string> imagesFilenames;
	
	// Options for the preprocessing of images
	string preprocessingSteps = "";
		
	// Options for the labeling
	bool label = false;
	
	// Option for the output size
	string size;
	
	// option for the output directory
	string outputDirectory = ".";
	
	string programDescription = "This Program generates a RGB image out of 3 fits files.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());

	ArgumentHelper arguments;
	arguments.new_flag('l', "label", "\n\tSet this flag if you want a label on the background.\n\t", label);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_string('S', "size", "string", "\n\tThe size of the image written. i.e. \"1024x1024\"\n\tSee ImageMagick Image Geometry for specification.\n\t", size);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 fitsFileName3", "\n\tThe name of the fits files to use to make the composite, in Red Green Blue order.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// We check that we received 3 files
	if(imagesFilenames.size() != 3)
	{
		cerr << "Error : You must provide exactly 3 files."<< endl;
		return 2;
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
	EUVImage* image = getImageFromFile("UNKNOWN", imagesFilenames[0]);
	enhance_contrast(image, preprocessingSteps);
	MagickImage red_channel = image->magick();
	delete image;
	MagickCore::ClutImage(red_channel.image(), red_gradient.image());
	#if DEBUG >= 2
		red_channel.write(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[0])) + "_red.png");
	#endif
	
	
	// We make the green channel with the second image
	image = getImageFromFile("UNKNOWN", imagesFilenames[1]);
	enhance_contrast(image, preprocessingSteps);
	MagickImage green_channel = image->magick();
	delete image;
	MagickCore::ClutImage(green_channel.image(), green_gradient.image());
	#if DEBUG >= 2
		green_channel.write(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[1])) + "_green.png");
	#endif
	
	// We make the blue channel with the third image
	image = getImageFromFile("UNKNOWN", imagesFilenames[2]);
	enhance_contrast(image, preprocessingSteps);
	MagickImage blue_channel = image->magick();
	MagickCore::ClutImage(blue_channel.image(), blue_gradient.image());
	#if DEBUG >= 2
		blue_channel.write(outputDirectory + "/" + stripSuffix(stripPath(imagesFilenames[2])) + "_blue.png");
	#endif
	
	// We compose the channels together
	blue_channel.composite(red_channel, Magick::CenterGravity, Magick::PlusCompositeOp);
	blue_channel.composite(green_channel, Magick::CenterGravity, Magick::PlusCompositeOp);
	
	if(label)
	{
		string text = image->Instrument() + " " + dtos(image->Wavelength()) + "Å " + image->ObservationDate();
		size_t text_size = image->Xaxes()/40;
		blue_channel.fillColor("white");
		blue_channel.fontPointsize(text_size);
		blue_channel.annotate(text, Geometry(0, 0, text_size/2, text_size/2), Magick::SouthWestGravity);
		blue_channel.label(text);
	}
	delete image;
	
	if(size_geometry.isValid())
		blue_channel.scale(size_geometry);
	
	string outputFilename = outputDirectory + "/";
	for(unsigned p = 0; p < 3; ++p)
		outputFilename += stripSuffix(stripPath(imagesFilenames[p])) + "_";
	outputFilename += "composite.png";
	
	blue_channel.write(outputFilename);

	return EXIT_SUCCESS;
}


