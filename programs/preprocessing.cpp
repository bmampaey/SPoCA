//! Program to help study the effects of the preprocessing.
/*!
@page preprocessing_x preprocessing.x

This program serves to study the effects of the preprocessing and in particular the  @ref ALC on EUV images.


The program will generate the following files:
 - line.txt		: The intensities of the pixels of the original image along the horizontal lines passing by the specified percentage of the height the image.
 - preprocessed.line.txt	: The intensities of the pixels of the preprocessed image along the horizontal lines passing by the specified percentage of the height the image.
 - preprocessed.fits	: The image preprocessed, an image with circles along the ALC parameters, and an image with the values of the function for the ALC. See @ref ALC.
 
 @section usage Usage
 
 <tt> preprocessing.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> preprocessing.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
@param imageType	The type of the images.
<BR>Possible values are : 
 - EIT
 - EUVI
 - AIA
 - SWAP

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
 
@param radiusratio	The ratio of the radius of the sun that will be processed.

@param ALCparameters	The parameters for the Annulus Limb Correction. See @ref ALC for more information.
<BR>List of 4 percentages of the radius of the sun, in increasing order. For example 80,100,110,120.

@param lines	List of percentages of the height of the image for the horizontal lines to be printed in the 2 line.txt files


See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

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
#include "../classes/Coordinate.h"
#include "../classes/EUVImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"

using std::string; using std::cout; using std::cerr; using std::endl;
using std::vector; 
using std::ofstream;
using std::ios;
using namespace dsr;

string filenamePrefix;

// Routine that draws the limits of the ALC percentCorrection function on an image
void ALCLimitsImage(EUVImage* image, vector<Real> ALCParameters)
{
	//image->zero(0);
	if (ALCParameters.size() == 0)
		ALCParameters = image->getALCParameters();
	
	for(unsigned p = 0; p < ALCParameters.size(); ++p)
		image->drawCircle(PixLoc(image->SunCenter()), image->SunRadius() * ALCParameters[p], 1);
}

// Routine that draws the ALC percentCorrection function on an image
void percentCorrectionImage(EUVImage* image)
{
	image->zero(0);
	double sunRadius = image->SunRadius();
	RealPixLoc sunCenter = image->SunCenter();
	
	for (unsigned y=0; y < image->Yaxes(); ++y)
	{
		for (unsigned x=0; x < image->Xaxes(); ++x)
		{
			double pixelRadius = distance(sunCenter, RealPixLoc(x,y)) / sunRadius;
			image->pixel(x,y) = image->percentCorrection(pixelRadius);
		}
	}
}

// Routine that output the horizontal lines 
void printLines(const string& filename, const EUVImage* image, const vector<Real>& lines)
{
		if(lines.size() > 0)
		{
			ofstream lineFile(filename.c_str(), std::ios::app);
			if (lineFile.good())
			{
				// We write the column header
				for (unsigned l = 0; l < lines.size(); ++l)
					lineFile<<";"<<unsigned(image->Yaxes()*(lines[l]/100.));
				lineFile<<endl;
				
				for (unsigned x = 0; x < image->Xaxes(); ++x)
				{
					// We write the row header
					lineFile<<x;
					for (unsigned l = 0; l < lines.size(); ++l)
						lineFile<<";"<<image->pixel(x, image->Yaxes()*(lines[l]/100.));
					lineFile<<endl;
				}
			}
			else
			{
				cerr<<"Error : Could not open file "<<filename<<" for writing."<<endl;
			}

			lineFile.close();
		}
}

int main(int argc, const char **argv)
{
	#if defined EXTRA_SAFE
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
        cout<<setiosflags(ios::fixed);
	#endif

	// The list of names of the images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;
	string ALCparameters;
	
	// Options for the output
	string lines;
	
	string programDescription = "Program to help study the effects of the preprocessing.\n";
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
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_description(programDescription.c_str());
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('A', "ALCparameters","comma separated list of positive real (no spaces)", "\n\tThe parameters for the Annulus Limb Correction.\n\tSet this option to override the default values.\n\t", ALCparameters);
	arguments.new_named_string('L', "lines","comma separated list of percentages (no spaces)", "\n\tThe intensities values along the lines corresponding to the percentages of the height of the images will be written to a text file.\n\tFor example specifying -L 25,50,75 will provide the intensities of the lines passing by the first quarter of the image, the middle, and the last quarter;\n\t", lines);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);


	// We set the name of output files prefix
	if(!preprocessingSteps.empty())
	{
		filenamePrefix = "R" + dtos(radiusRatio);
		string preprocessingStepsCopy(preprocessingSteps);
		vector<string> steps;
		preprocessingStepsCopy>>steps;
		for (unsigned s = 0; s < steps.size(); ++s)
			filenamePrefix += "_" + steps[s];
		filenamePrefix += ".";
	}
	

	// We parse the requested lines
	vector<Real> lineValues;
	if(!lines.empty())
	{
		lines>>lineValues;
	}
	
	// We parse the ALC parameters if any
	vector<Real> ALCparams;
	if(!ALCparameters.empty())
	{
		ALCparameters>>ALCparams;
		for(unsigned p = 0; p < ALCparams.size(); ++p)
			ALCparams[p] /= 100.;
	}

	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read the sun image
		EUVImage* image  = getImageFromFile(imageType, imagesFilenames[p]);
		
		string filename = filenamePrefix + stripPath(stripSuffix(imagesFilenames[p])) + ".";
		
		// We set up the ALC parameters if any
		if(ALCparams.size() > 0)
		{
			image->setALCParameters(ALCparams);
		}
		
		image->nullifyAboveRadius(radiusRatio);
		
		// We print the intensities of the chosen lines of the original image
		printLines(filename + "line.txt", image, lineValues);
	
		// We preprocess the sun image
		image->preprocessing(preprocessingSteps, radiusRatio);
		
		// We print the intensities of the chosen lines of the preprocessed image
		printLines(filename + "preprocessed.line.txt", image, lineValues);

		// We write the preprocessed image
		FitsFile file(filename + "preprocessed.fits", FitsFile::overwrite);
		file.writeImage(&(image->pixel(0)), image->Xaxes(), image->Yaxes(), 0, "Processed image");
	
		// We create the image of the ALC limits (it will draw circles on it)
		ALCLimitsImage(image, ALCparams);
		file.writeImage(&(image->pixel(0)), image->Xaxes(), image->Yaxes(), 0, "Processed image with limits");
		
		// We create the function image
		percentCorrectionImage(image);
		file.writeImage(&(image->pixel(0)), image->Xaxes(), image->Yaxes(), 0, "Percent correction function");
		
		delete image;
	}
	return EXIT_SUCCESS;
}

