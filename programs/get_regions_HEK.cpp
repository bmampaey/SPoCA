#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/ColorMap.h"
#include "../classes/SunImage.h"
#include "../classes/RegionStats.h"
#include "../classes/Coordinate.h"
#include "../classes/FeatureVector.h"

using namespace std;
using namespace dsr;

string outputFileName;



int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	// Options for the preprocessing of images
	string preprocessingSteps = "NAR,DivExpTime";
	double radiusRatio = 0.95;
	

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
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "The name of the fits files containing the images of the sun.", imagesFilenames);

	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);


	if(imagesFilenames.size() < 1 )
	{
		cerr<<"No fits image file given as parameter!"<<endl;
		return EXIT_FAILURE;
	}
	
	ColorMap* colorizedComponentsMap = dynamic_cast<ColorMap*> (getImageFromFile("ColorMap", colorizedComponentsMapFileName));
	colorizedComponentsMap->nullifyAboveRadius(1);		
	Coordinate sunCenter = colorizedComponentsMap->SunCenter();
	double sunRadius = colorizedComponentsMap->SunRadius();
	

	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
	
		//We read and preprocess the sun image
		SunImage* image = getImageFromFile(imageType, imagesFilenames[p]);
		image->preprocessing(preprocessingSteps, radiusRatio);
		#if DEBUG >= 2
		image->writeFitsImage(outputFileName + "preprocessed."+ stripPath(imagesFilenames[p]) );
		#endif

		if( sunCenter.d2(image->SunCenter()) > 2 )
		{
			cerr<<"Warning : Image "<<imagesFilenames[p]<<" will be recentered to have the same sun centre than image "<<colorizedComponentsMapFileName<<endl;
			image->recenter(sunCenter);
		}
		if( abs(1. - (image->SunRadius() / sunRadius)) > 0.01 )
		{
			cerr<<"Error : Image "<<imagesFilenames[p]<<" does not have the same sun radius than image "<<colorizedComponentsMapFileName<<endl;
			exit(EXIT_FAILURE);
		}
		
		// We get the regions stats and output them
		vector<RegionStats*> regions = getRegions(colorizedComponentsMap, image);
		for (unsigned r = 0; r < regions.size(); ++r)
		{
			cout<<regions[r]->HekLabel()<<"\t"<<regions[r]->toString()<<endl;
			delete regions[r];
		}
		delete image;
	}
	delete colorizedComponentsMap;
	return EXIT_SUCCESS;
}
