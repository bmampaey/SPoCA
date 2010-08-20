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

bool circleAt(Real pixelRadius)
{
	if(pixelRadius > 1. - 0.001 &&  pixelRadius < 1. + 0.001)
		return true;
		
#if LIMB_CORRECTION == VINCE_CORR
	if(pixelRadius > Real(VINCE_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(VINCE_CORR_R1)/100. + 0.001)
		return true;
#endif

#if LIMB_CORRECTION == CIS1_CORR
	if(pixelRadius > Real(CIS1_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(CIS1_CORR_R1)/100. + 0.001)
		return true;
	if(pixelRadius > Real(CIS1_CORR_R2)/100. - 0.001 &&  pixelRadius < Real(CIS1_CORR_R2)/100. + 0.001)
		return true;
#endif

#if LIMB_CORRECTION == BEN_CORR
	if(pixelRadius > Real(BEN_CORR_R1)/100. - 0.001 &&  pixelRadius < Real(BEN_CORR_R1)/100. + 0.001)
		return true;
	if(pixelRadius > Real(BEN_CORR_R2)/100. - 0.001 &&  pixelRadius < Real(BEN_CORR_R2)/100. + 0.001)
		return true;
	if(pixelRadius > Real(BEN_CORR_R3)/100. - 0.001 &&  pixelRadius < Real(BEN_CORR_R3)/100. + 0.001)
		return true;
	if(pixelRadius > Real(BEN_CORR_R4)/100. - 0.001 &&  pixelRadius < Real(BEN_CORR_R4)/100. + 0.001)
		return true;
#endif

	return false;

}



int main(int argc, const char **argv)
{
	#if DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// The list of names of the images to process
	string imageType = "AIA";
	vector<string> sunImagesFileNames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 1.31;
	
	string programDescription = "This Program helps you study the annulus limb correction.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_description(programDescription.c_str());
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('O', "outputFile","file name", "\n\tThe name for the output file(s).\n\t", outputFileName);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the images of the sun.\n\t", sunImagesFileNames);
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);


	//Let's set the name of output files
	//If none as been provided as a program argument, we set it to the executable file name minus the last .extension
	if(outputFileName.empty())
	{
		outputFileName = argv[0];
		outputFileName = outputFileName.substr(outputFileName.rfind('/')!=string::npos?outputFileName.rfind('/')+1:0);
		outputFileName = outputFileName.substr(0, outputFileName.rfind('.'));
	}
	outputFileName += ".";

	//We output the middle line of the images
	for (unsigned p = 0; p < sunImagesFileNames.size(); ++p)
	{
		SunImage* image  = getImageFromFile(imageType, sunImagesFileNames[p]);
		image->nullifyAboveRadius(radiusRatio);
		string filename = outputFileName + string(limb_corrections[LIMB_CORRECTION]) + ".";
		filename +=  sunImagesFileNames[p].substr(sunImagesFileNames[p].rfind('/')!=string::npos?sunImagesFileNames[p].rfind('/')+1:0);
		filename += ".line.txt";
		ofstream lineFile(filename.c_str());
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
		delete image;

	}

	//We read and preprocess the sun images
	fetchImagesFromFile(images, sunImagesFileNames, preprocessingSteps, radiusRatio);
	
	//We output the middle line of the images
	for (unsigned p = 0; p < images.size(); ++p)
	{
		string filename = outputFileName + string(limb_corrections[LIMB_CORRECTION]) + ".";
		filename +=  sunImagesFileNames[p].substr(sunImagesFileNames[p].rfind('/')!=string::npos?sunImagesFileNames[p].rfind('/')+1:0);
		filename += ".preprocessed.line.txt";
		ofstream lineFile(filename.c_str());
		if (lineFile)
		{
			unsigned y = images[p]->Yaxes() / 2;
			for (unsigned x = 0; x < images[p]->Xaxes(); ++x)
				lineFile<<images[p]->pixel(x, y) <<endl;
		}
		else
		{
			cerr<<"Error : Could not open file "<<filename<<" for writing."<<endl;

		}
		

		lineFile.close();

	}
	//We draw circles on the images
	Real pixelRadius = 0;
	for (unsigned p = 0; p < images.size(); ++p)
	{
		unsigned Xaxes = images[p]->Xaxes(), Yaxes = images[p]->Yaxes();
		double sunRadius = images[p]->SunRadius();
		Coordinate sunCenter = images[p]->SunCenter();
		for (unsigned y=0; y < Yaxes; ++y)
		{
			for (unsigned x=0; x < Xaxes; ++x)
			{

				pixelRadius = sqrt(sunCenter.d2(Coordinate(x,y))) / sunRadius;
				if(circleAt(pixelRadius))
					images[p]->pixel(x,y) = images[p]->nullvalue;
				
			}
		}
		string filename = outputFileName + string(limb_corrections[LIMB_CORRECTION]) + ".";
		filename +=  sunImagesFileNames[p].substr(sunImagesFileNames[p].rfind('/')!=string::npos?sunImagesFileNames[p].rfind('/')+1:0);
		images[p]->writeFitsImage(filename);
	}
	//We create the function image
	images[0]->zero();
	unsigned Xaxes = images[0]->Xaxes(), Yaxes = images[0]->Yaxes();
	double sunRadius = images[0]->SunRadius();
	Coordinate sunCenter = images[0]->SunCenter();
	for (unsigned y=0; y < Yaxes; ++y)
	{
		for (unsigned x=0; x < Xaxes; ++x)
		{

			pixelRadius = sqrt(sunCenter.d2(Coordinate(x,y))) / sunRadius;
			if(circleAt(pixelRadius))
				images[0]->pixel(x,y) = 2;
			else
				images[0]->pixel(x,y) = images[0]->percentCorrection(pixelRadius);
			
		}
	}
	string filename = outputFileName + string(limb_corrections[LIMB_CORRECTION]) + ".function.fits";
	images[0]->writeFitsImage(filename);

	return EXIT_SUCCESS;
}

