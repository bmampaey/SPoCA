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

string outputFileName;

extern template class Image<Real>;

int main(int argc, const char **argv)
{
	#if DEBUG >= 1
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;
		
	// Option for the derotation
	int delta_t = 3600;
	string reference;

	string programDescription = "This Program use differential derotation to project a Sun Image in time.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERWAVELENGTH: " + itos(NUMBERWAVELENGTH);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nPixelType: " + string(typeid(PixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
	arguments.new_named_string('R', "reference","string", "\n\tThe image of reference for the derotation.\n\tThe image will be derotated so it is comparable to the reference image\n\t", reference);
	arguments.new_named_int('d',"delta_t","integer","\n\tThe delta time in seconds to derotate the image\n\t",delta_t);
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
	EUVImage* reference_image = NULL;
	if(!reference.empty())
	{
		reference_image = getImageFromFile(imageType, reference);
	}
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		EUVImage* image  = getImageFromFile(imageType, imagesFilenames[p]);
		image->preprocessing("NAR", 1);
		if(reference_image)
		{
			image->rotate_like(reference_image);
			image->writeFits(stripSuffix(stripPath(imagesFilenames[p])) + "_rotated2_" + stripSuffix(stripPath(reference))+".fits");
		}
		else
		{
			image->rotate(delta_t);
			image->writeFits(stripSuffix(stripPath(imagesFilenames[p])) + "_rotatedby_" + itos(delta_t) +"s.fits");
		}
		delete image;

	}
	delete reference_image;
	return EXIT_SUCCESS;
}
