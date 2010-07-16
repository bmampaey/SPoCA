#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/AIAImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/MainUtilities.h"
#include "../classes/RegionStats.h"
#include "../classes/Coordinate.h"
#include "../classes/FeatureVector.h"
#include "../classes/CoordinateConvertor.h"

using namespace std;
using namespace dsr;

string outputFileName;



int main(int argc, const char **argv)
{
	#if defined(DEBUG) && DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif
	
	string preprocessingSteps = "NAR";
	double radiusRatio = 0.95;
	string colorizedComponentsMapFileName;
	vector<string> sunImagesFileNames;
	
	vector<SunImage*> images;
	AIAImage* colorizedComponentsMap;
	
	string programDescription = "This Programm output regions info and statistics.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_double('r',"radiusratio","radiusratio","The ratio of the radius of the sun that will be processed",radiusRatio);
	arguments.new_named_string('M',"colorizedComponentsMap","colorizedComponentsMap", "The name of the file containing a colorizedComponentsMap of regions (i.e. each one must have a different color).", colorizedComponentsMapFileName);
	arguments.new_named_string('P', "preprocessingSteps", "preprocessingSteps", "The step of preprocessing to apply to the sun images.\n\tNullify above radius : NAR, ALC : Annulus Limb Correction, Division median : DivMedian, Take the square root : TakeSqrt, Take the log : TakeLog, Division by the mode : DivMode, Division by the Exposure Time : DivExpTime", preprocessingSteps);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "The name of the fits files containing the images of the sun.", sunImagesFileNames);

	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	if(sunImagesFileNames.size() < 1 )
	{
		cerr<<"No fits image file given as parameter!"<<endl;
		return EXIT_FAILURE;
	}
	
	//We read and preprocess the sun images
	images = getImagesFromFiles("AIA", sunImagesFileNames, false);
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing(preprocessingSteps, radiusRatio);
		#if defined(DEBUG) && DEBUG >= 2
		images[p]->writeFitsImage(outputFileName + "preprocessed."+sunImagesFileNames[p].substr(sunImagesFileNames[p].rfind('/')!=string::npos?sunImagesFileNames[p].rfind('/')+1:0));
		#endif
	}

	//We do the same for the colorizedComponentsMap
	#if defined(DEBUG) && DEBUG >= 1
		if(colorizedComponentsMapFileName.find(".fits")==string::npos && colorizedComponentsMapFileName.find(".fts")==string::npos)
		{
			cerr<<colorizedComponentsMapFileName<<" is not a fits file! (must end in .fits or .fts)"<<endl;
		}
	#endif
	
	colorizedComponentsMap = new AIAImage(colorizedComponentsMapFileName);
	
	#if defined(DEBUG) && DEBUG >= 1

		
	Coordinate sunCenter = colorizedComponentsMap->SunCenter();
	double sunRadius = colorizedComponentsMap->SunRadius();

	for (unsigned p = 0; p < sunImagesFileNames.size(); ++p)
	{
		if( sunCenter.d2(images[p]->SunCenter()) > 2 )
		{
			cerr<<"Warning : Image "<<sunImagesFileNames[p]<<" will be recentered to have the same sun centre than image "<<colorizedComponentsMapFileName<<endl;
			images[p]->recenter(sunCenter);
		}
		if( abs(1. - (images[p]->SunRadius() / sunRadius)) > 0.01 )
		{
			cerr<<"Error : Image "<<sunImagesFileNames[p]<<" does not have the same sun radius than image "<<colorizedComponentsMapFileName<<endl;
			exit(EXIT_FAILURE);
		}
	}
	#endif

	colorizedComponentsMap->nullifyAboveRadius(1);

	for (unsigned p = 0; p < sunImagesFileNames.size(); ++p)
	{
		vector<RegionStats*> regions = getRegions(colorizedComponentsMap, images[p]);
		for (unsigned r = 0; r < regions.size(); ++r)
		{
			cout<<*(regions[r])<<endl;
			delete regions[r];
		}
		delete images[p];
	}

	return EXIT_SUCCESS;
}
