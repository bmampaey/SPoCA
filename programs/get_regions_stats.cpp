#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"

#include "../classes/SunImage.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/RegionStats.h"
#include "../classes/Coordinate.h"
#include "../classes/FeatureVector.h"

#ifdef COCO
#include "../classes/CoordinateConvertor.h"
#endif

using namespace std;
using namespace dsr;

string outputFileName;



int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// The list of names of the sun images to process
	string imageType = "AIA";
	vector<string> sunImagesFileNames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR";
	double radiusRatio = 0.95;
	
	// Options for the type of coordinate
	string coordinateType;

	// The map of colored regions
	string colorizedComponentsMapFileName;
	
	
	string programDescription = "This Programm output regions info and statistics.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tNAR (Nullify above radius)\n\t\tALC (Annulus Limb Correction)\n\t\tDivMedian (Division by the median)\n\t\tTakeSqrt (Take the square root)\n\t\tTakeLog (Take the log)\n\t\tDivMode (Division by the mode)\n\t\tDivExpTime (Division by the Exposure Time)\n\t", preprocessingSteps);
	arguments.new_named_double('r', "radiusratio", "positive real", "\n\tThe ratio of the radius of the sun that will be processed.\n\t",radiusRatio);
	arguments.new_named_string('M',"colorizedComponentsMap","file name", "\n\tA colorized Components Map of regions (i.e. each one must have a different color).\n\t", colorizedComponentsMapFileName);
	arguments.new_named_string('C', "coordinateType", "string", "\n\tThe type of coordinates to output positions.\n\tPossible values are : HG, HGC, HPC, HPR, HCC, HCR\n\t", coordinateType);
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
		

	for (unsigned p = 0; p < sunImagesFileNames.size(); ++p)
	{
	
		//We read and preprocess the sun image
		SunImage* image = getImageFromFile(imageType, sunImagesFileNames[p]);
		image->preprocessing(preprocessingSteps, radiusRatio);
		#if DEBUG >= 2
		image->writeFitsImage(outputFileName + "preprocessed."+sunImagesFileNames[p].substr(sunImagesFileNames[p].rfind('/')!=string::npos?sunImagesFileNames[p].rfind('/')+1:0));
		#endif
		SunImage* colorizedComponentsMap = getImageFromFile(imageType, colorizedComponentsMapFileName);
		colorizedComponentsMap->copyKeywords(image);
		colorizedComponentsMap->nullifyAboveRadius(1);		
		
		// We get the regions stats and output them
		vector<RegionStats*> regions = getRegions(colorizedComponentsMap, image);
		#ifdef CoordinateConvertor_H
		CoordinateConvertor coco(image, coordinateType);
		#endif

		for (unsigned r = 0; r < regions.size(); ++r)
		{
			#ifdef CoordinateConvertor_H
			cout<<regions[r]->toString(coco)<<endl;
			#else
			cout<<regions[r]->toString()<<endl;
			#endif
			delete regions[r];
		}
		delete image;
		delete colorizedComponentsMap;
	}
	return EXIT_SUCCESS;
	
}


