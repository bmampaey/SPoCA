//! Program to help study the annulus limb correction.
/*!
@page limb_correction limb_correction.x

This program will apply the annulus limb correction on the provided EUV images to help study the good parameters to use for @ref ALC_corr

The program will generate the following files:
 - line.txt		: The values along the horizontal line passing through the middle of the image
 - preprocessed.line.txt	: The values along the horizontal line passing through the middle of the image after processing
 - preprocessed.fits	: The image processed
 - function.fits	: The value of the percent correction function see @ref EUVImage.h
 
 @section usage Usage
 
 <tt> limb_correction.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> limb_correction.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
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
 
@param radiusratio	The ratio of the radius of the sun that will be processed.

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

using namespace std;
using namespace dsr;

string filenamePrefix;

bool circleAt(Real pixelRadius, string imageType)
{
	if(pixelRadius > 1. - 0.001 &&  pixelRadius < 1. + 0.001)
		return true;
	if(pixelRadius > sineCorrectionParameters[0]/100. - 0.001 &&  pixelRadius < sineCorrectionParameters[0]/100. + 0.001)
		return true;
	if(pixelRadius > sineCorrectionParameters[1]/100. - 0.001 &&  pixelRadius < sineCorrectionParameters[1]/100. + 0.001)
		return true;
	if(pixelRadius > sineCorrectionParameters[2]/100. - 0.001 &&  pixelRadius < sineCorrectionParameters[2]/100. + 0.001)
		return true;
	if(pixelRadius > sineCorrectionParameters[3]/100. - 0.001 &&  pixelRadius < sineCorrectionParameters[3]/100. + 0.001)
		return true;
	return false;

}



int main(int argc, const char **argv)
{
	#if DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// The list of names of the images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;
	
	string programDescription = "This Program helps you study the annulus limb correction.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_description(programDescription.c_str());
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", filenamePrefix);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);


	//Let's set the name of output files
	if(filenamePrefix.empty())
	{
		filenamePrefix = "limb_correction";
	}
	filenamePrefix += ".";

	ofstream lineFile;

	
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read  the sun image
		EUVImage* image  = getImageFromFile(imageType, imagesFilenames[p]);
		image->nullifyAboveRadius(radiusRatio);
		string filename = filenamePrefix + stripPath(stripSuffix(imagesFilenames[p])) + ".";
		
		//We output the middle line of the image
		lineFile.open((filename + "line.txt").c_str());
		if (lineFile)
		{
			unsigned y = image->Yaxes() / 2;
			for (unsigned x = 0; x < image->Xaxes(); ++x)
				lineFile<<image->pixel(x, y) <<endl;
		}
		else
		{
			cerr<<"Error : Could not open file "<<filename<<" for writing."<<endl;
		}

		lineFile.close();
	
		//We preprocess the sun image
		image->preprocessing(preprocessingSteps, radiusRatio);
		
		//We output the middle line of the image
		lineFile.open((filename + "preprocessed.line.txt").c_str());
		if (lineFile)
		{
			unsigned y = image->Yaxes() / 2;
			for (unsigned x = 0; x < image->Xaxes(); ++x)
				lineFile<<image->pixel(x, y) <<endl;
		}
		else
		{
			cerr<<"Error : Could not open file "<<filename<<" for writing."<<endl;
		}

		lineFile.close();

	
		//We draw circles on the image
		double sunRadius = image->SunRadius();
		Coordinate sunCenter = image->SunCenter();
		for (unsigned y = 0; y < image->Yaxes(); ++y)
		{
			for (unsigned x = 0; x < image->Xaxes(); ++x)
			{
				if(circleAt(sunCenter.d(Coordinate(x,y)) / sunRadius, imageType))
					image->pixel(x,y) = image->nullvalue();				
			}
		}
		image->writeFits(filename + "preprocessed.fits");
		delete image;
	}
	
	//We create the function image
	EUVImage* image  = getImageFromFile(imageType, imagesFilenames[0]);
	image->zero();
	double sunRadius = image->SunRadius();
	Coordinate sunCenter = image->SunCenter();
	for (unsigned y=0; y < image->Yaxes(); ++y)
	{
		for (unsigned x=0; x < image->Xaxes(); ++x)
		{

			double pixelRadius = sunCenter.d(Coordinate(x,y)) / sunRadius;
			if(circleAt(pixelRadius, imageType))
				image->pixel(x,y) = 2;
			else
				image->pixel(x,y) = image->percentCorrection(pixelRadius);
			
		}
	}
	image->writeFits(filenamePrefix + "function.fits");
	
	delete image;
	return EXIT_SUCCESS;
}

