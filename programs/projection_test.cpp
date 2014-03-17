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
#include "../classes/SunImage.h"
#include "../classes/ArgumentHelper.h"
#include "../classes/mainutilities.h"
#include "../classes/Region.h"
#include "../classes/Coordinate.h"


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

	// Options for the projections
	bool exact = false;
	bool grid = false;
	bool equirectangular = false;
	bool lambert = false;
	bool sinusoidal = false;

	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	string programDescription = "This Program is for testing Sun images image projections.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + toString(NUMBERCHANNELS);
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
	arguments.new_flag('e', "exact", "\n\tSet this flag if you want exact projection.\n\t", exact);
	arguments.new_flag('g', "grid", "\n\tSet this flag if you want to see the projection of a grid instead of the image.\n\t", grid);
	arguments.new_flag('E', "equirectangular", "\n\tSet this flag if you want equirectangular projection.\n\t", equirectangular);
	arguments.new_flag('L', "lambert", "\n\tSet this flag if you want Lambert cylindrical projection.\n\t", lambert);
	arguments.new_flag('S', "sinusoidal", "\n\tSet this flag if you want sinusoidal projection.\n\t", sinusoidal);
	

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
	
	if(!equirectangular && !lambert &&!sinusoidal)
		equirectangular = lambert = sinusoidal = true;
	
	unsigned xaxes = 1024;
	unsigned yaxes = 1024;

	SunImage<EUVPixelType> * projection = new SunImage<EUVPixelType>(xaxes, yaxes);
	
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		EUVImage* image = getImageFromFile("UNKNOWN", imagesFilenames[p]);
		EUVImage* deprojection = new EUVImage(image);
		
		string filename = stripPath(stripSuffix(imagesFilenames[p]));
		
		if(!grid)
		{
			if(equirectangular)
			{
				projection->equirectangular_projection(image, exact);
				projection->writeFits(filename + ".equirectangular_projection.fits");
			
				deprojection->equirectangular_deprojection(projection, exact);
				deprojection->writeFits(filename + ".equirectangular_deprojection.fits");
			}
		
			if(lambert)
			{
				projection->Lambert_cylindrical_projection(image, exact);
				projection->writeFits(filename + ".Lambert_cylindrical_projection.fits");
		
				deprojection->Lambert_cylindrical_deprojection(projection, exact);
				deprojection->writeFits(filename + ".Lambert_cylindrical_deprojection.fits");
			}
		
			if(sinusoidal)
			{
				projection->sinusoidal_projection(image, exact);
				projection->writeFits(filename + ".sinusoidal_projection.fits");
		
				deprojection->sinusoidal_deprojection(projection, exact);
				deprojection->writeFits(filename + ".sinusoidal_deprojection.fits");
			}
		}
		else
		{
			// We create a grid on a equirectengular projection
			projection->zero();
			unsigned dx = xaxes / 20;
			unsigned dy = yaxes / 20;
			for(unsigned y=dy/2; y < yaxes; y+=dy)
				for(unsigned x=0; x < xaxes; ++x)
					projection->pixel(x,y) = projection->pixel(x,y-1) = projection->pixel(x,y+1) = y;
			for(unsigned x=dy/2; x < xaxes; x+=dx)
				for(unsigned y=0; y < yaxes; ++y)
					projection->pixel(x,y) = projection->pixel(x-1,y) = projection->pixel(x+1,y) = x;
			
			// And we use the deprojection to create the sphere
			image->equirectangular_deprojection(projection, exact);
			image->writeFits(filename + ".grid.fits");
			
			if(equirectangular)
			{
				projection->equirectangular_projection(image, exact);
				projection->writeFits(filename + ".grid.equirectangular_projection.fits");
				
				deprojection->equirectangular_deprojection(projection, exact);
				deprojection->writeFits(filename + ".grid.equirectangular_deprojection.fits");
			}
			
			if(lambert)
			{
				projection->Lambert_cylindrical_projection(image, exact);
				projection->writeFits(filename + ".grid.Lambert_cylindrical_projection.fits");
				
				deprojection->Lambert_cylindrical_deprojection(projection, exact);
				deprojection->writeFits(filename + ".grid.Lambert_cylindrical_deprojection.fits");
			}
			
			if(sinusoidal)
			{
				projection->sinusoidal_projection(image, exact);
				projection->writeFits(filename + ".grid.sinusoidal_projection.fits");
		
				deprojection->sinusoidal_deprojection(projection, exact);
				deprojection->writeFits(filename + ".grid.sinusoidal_deprojection.fits");
			}
		}
		delete image;
		delete deprojection;
	}
	delete projection;
	return EXIT_SUCCESS;
}
