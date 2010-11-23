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
#include "../classes/SunImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"

using namespace std;
using namespace dsr;

string outputFileName;

bool circleAt(Real pixelRadius, string imageType)
{
	if(pixelRadius > 1. - 0.001 &&  pixelRadius < 1. + 0.001)
		return true;
		
	if (imageType == "EIT")
	{
		if(pixelRadius > Real(EIT_SINE_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(EIT_SINE_CORR_R1)/100. + 0.001)
			return true;
		if(pixelRadius > Real(EIT_SINE_CORR_R2)/100. - 0.001 &&  pixelRadius < Real(EIT_SINE_CORR_R2)/100. + 0.001)
			return true;
		if(pixelRadius > Real(EIT_SINE_CORR_R3)/100. - 0.001 &&  pixelRadius < Real(EIT_SINE_CORR_R3)/100. + 0.001)
			return true;
		if(pixelRadius > Real(EIT_SINE_CORR_R4)/100. - 0.001 &&  pixelRadius < Real(EIT_SINE_CORR_R4)/100. + 0.001)
			return true;
	}
	else if (imageType == "EUVI")
	{
		if(pixelRadius > Real(EUVI_SINE_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(EUVI_SINE_CORR_R1)/100. + 0.001)
			return true;
		if(pixelRadius > Real(EUVI_SINE_CORR_R2)/100. - 0.001 &&  pixelRadius < Real(EUVI_SINE_CORR_R2)/100. + 0.001)
			return true;
		if(pixelRadius > Real(EUVI_SINE_CORR_R3)/100. - 0.001 &&  pixelRadius < Real(EUVI_SINE_CORR_R3)/100. + 0.001)
			return true;
		if(pixelRadius > Real(EUVI_SINE_CORR_R4)/100. - 0.001 &&  pixelRadius < Real(EUVI_SINE_CORR_R4)/100. + 0.001)
			return true;

	}
	else if (imageType == "AIA")
	{
		if(pixelRadius > Real(AIA_SINE_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(AIA_SINE_CORR_R1)/100. + 0.001)
			return true;
		if(pixelRadius > Real(AIA_SINE_CORR_R2)/100. - 0.001 &&  pixelRadius < Real(AIA_SINE_CORR_R2)/100. + 0.001)
			return true;
		if(pixelRadius > Real(AIA_SINE_CORR_R3)/100. - 0.001 &&  pixelRadius < Real(AIA_SINE_CORR_R3)/100. + 0.001)
			return true;
		if(pixelRadius > Real(AIA_SINE_CORR_R4)/100. - 0.001 &&  pixelRadius < Real(AIA_SINE_CORR_R4)/100. + 0.001)
			return true;

	}
	else if (imageType == "SWAP")
	{
		if(pixelRadius > Real(SWAP_SINE_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(SWAP_SINE_CORR_R1)/100. + 0.001)
			return true;
		if(pixelRadius > Real(SWAP_SINE_CORR_R2)/100. - 0.001 &&  pixelRadius < Real(SWAP_SINE_CORR_R2)/100. + 0.001)
			return true;
		if(pixelRadius > Real(SWAP_SINE_CORR_R3)/100. - 0.001 &&  pixelRadius < Real(SWAP_SINE_CORR_R3)/100. + 0.001)
			return true;
		if(pixelRadius > Real(SWAP_SINE_CORR_R4)/100. - 0.001 &&  pixelRadius < Real(SWAP_SINE_CORR_R4)/100. + 0.001)
			return true;

	}
	else 
	{
		if(pixelRadius > Real(SINE_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(SINE_CORR_R1)/100. + 0.001)
			return true;
		if(pixelRadius > Real(SINE_CORR_R2)/100. - 0.001 &&  pixelRadius < Real(SINE_CORR_R2)/100. + 0.001)
			return true;
		if(pixelRadius > Real(SINE_CORR_R3)/100. - 0.001 &&  pixelRadius < Real(SINE_CORR_R3)/100. + 0.001)
			return true;
		if(pixelRadius > Real(SINE_CORR_R4)/100. - 0.001 &&  pixelRadius < Real(SINE_CORR_R4)/100. + 0.001)
			return true;

	}

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
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", outputFileName);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", imagesFilenames);
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);


	//Let's set the name of output files
	if(outputFileName.empty())
	{
		outputFileName = "limb_correction";
	}
	outputFileName += ".";

	ofstream lineFile;

	
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read  the sun image
		SunImage* image  = getImageFromFile(imageType, imagesFilenames[p]);
		image->nullifyAboveRadius(radiusRatio);
		string filename = outputFileName + stripPath(stripSuffix(imagesFilenames[p])) + ".";
		
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
		image->writeFitsImage(filename + "preprocessed.fits");
		delete image;
	}
	
	//We create the function image
	SunImage* image  = getImageFromFile(imageType, imagesFilenames[0]);
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
	image->writeFitsImage(outputFileName + "function.fits");
	
	delete image;
	return EXIT_SUCCESS;
}

