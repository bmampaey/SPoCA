//! Program that computes different local statistical values of EUV images
/*!
@page image_stat image_stat.x

 This program takes EUV sun images in fits format, and writes fits files with the following local statistics:
 - local Mean
 - local Variance
 - local Skewness
 - local Kurtosis
 
 It also generates the result of passing the images trough the following filters:
 - sobel
 - sobel approximation
 
 For example, the "local Variance" means that each pixel of the resulting image corresponds to the Variance value
 of a circular region centered at the same pixel location in the original image.
 
 The name of the resulting files will be the name of the original file with the type of statistic appended.
 
  @section usage Usage
 
 <tt> image_stat.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> image_stat.x [-option optionvalue, ...] fitsFileName1 fitsFileName2 </tt>
 
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

@param localRadius	The localRadius is the radius of the disc of neighboors considered for the statistic computation.
<BR>The diameter will be 2 x localRadius + 1

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

#include "../classes/Image.h"
#include "../classes/EUVImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"

#ifndef Real
#define Real float
#endif

using namespace std;
using namespace dsr;

string filenamePrefix;

extern template class Image<Real>;

int main(int argc, const char **argv)
{
	#if defined EXTRA_SAFE
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;

		
	// Option for the size of the neighboorhood
	unsigned localRadius = 1;

	string programDescription = "This Program generates local stats fits files.\n";
	programDescription+="Compiled with options :";
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

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_unsigned_int('N', "localRadius", "positive integer", "\n\tThe localRadius is the radius of the disc of neighboors considered for the statistic computation.\n\tThe diameter will be 2 x localRadius + 1.\n\t", localRadius);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	if(imagesFilenames.size() == 0)
	{
		cerr<<imagesFilenames.size()<<" fits image file given as parameter, at least 1 must be given!"<<endl;
		return EXIT_FAILURE;
	}

	for (unsigned p = 0; p< imagesFilenames.size(); ++p)
	{

		EUVImage* image  = getImageFromFile(imageType, imagesFilenames[p]);
		image->preprocessing(preprocessingSteps,radiusRatio);
		
		EUVImage stat(image->getWCS());
		
		filenamePrefix = stripSuffix(imagesFilenames[p]) + ".";
		
		stat.localMean(image, localRadius);
		stat.writeFits(filenamePrefix + "N" + itos(localRadius) + ".localMean.fits");
		
		stat.localVariance(image, localRadius);
		stat.writeFits(filenamePrefix + "N" + itos(localRadius) + ".localVariance.fits");
		
		stat.localSkewness(image, localRadius);
		stat.writeFits(filenamePrefix + "N" + itos(localRadius) + ".localSkewness.fits");
		
		stat.localKurtosis(image, localRadius);
		stat.writeFits(filenamePrefix + "N" + itos(localRadius) + ".localKurtosis.fits");
		
		stat.sobel(image);
		stat.writeFits(filenamePrefix + "sobel.fits");
		
		stat.sobel_approx(image);
		stat.writeFits(filenamePrefix + "sobel_approx.fits");
		
		delete image;
	

	}

	return EXIT_SUCCESS;
}
