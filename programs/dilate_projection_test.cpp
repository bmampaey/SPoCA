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
	bool equirectangular = false;
	bool lambert = false;
	bool sinusoidal = false;
	unsigned size = 2048;
	
	// Options for the dilation
	double dilate_factor = 10;
	
	

	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	string programDescription = "This Program is for testing dilation on Sun Image projection.\n";
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
	arguments.new_named_double('d', "dilation", "positive real", "\n\tThe dilation factor in arcsec.\n\t", dilate_factor);
	arguments.new_named_unsigned_int('s', "size", "positive integer", "\n\tThe size of the projection image.\n\t", size);
	arguments.new_flag('E', "equirectangular", "\n\tSet this flag if you want equirectangular projection.\n\t", equirectangular);
	arguments.new_flag('L', "lambert", "\n\tSet this flag if you want Lambert cylindrical projection.\n\t", lambert);
	arguments.new_flag('S', "sinusoidal", "\n\tSet this flag if you want sinusoidal projection.\n\t", sinusoidal);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of color maps.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	if(dilate_factor <= 0)
	{
		cerr<<"Dilation factor must be a strictly positive number!"<<endl;
		return EXIT_FAILURE;
	}	
	
	if(!equirectangular && !lambert && !sinusoidal)
		equirectangular = lambert = sinusoidal = true;
	
	ColorMap * projection = new ColorMap(size, size);
	
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		ColorMap* image = getImageFromFile(imagesFilenames[p]);
		ColorType nullvalue = image->null();
		
		string filename = stripPath(stripSuffix(imagesFilenames[p]));
	
		image->nullifyAboveRadius();
		image->writeFits(filename + ".nar.fits");
		
		Real dilate_factor_pixels = dilate_factor/image->PixelWidth();
		Real projeted_dilate_factor = dilate_factor_pixels * (Real(projection->Xaxes())/Real(image->Xaxes())) * (2./3.);
		
		ColorMap * deprojection = new ColorMap(image->getWCS(), image->Xaxes(), image->Yaxes());
		
		if(equirectangular)
		{
			projection->equirectangular_projection(image, exact);
			projection->writeFits(filename + ".equirectangular_projection.fits");
		
			projection->dilateCircular(projeted_dilate_factor, nullvalue);
			projection->writeFits(filename + ".equirectangular_projection.dilated.fits");
			
			deprojection->equirectangular_deprojection(projection, exact);
			deprojection->writeFits(filename + ".equirectangular_deprojection.dilated.fits");
		}
		
		if(lambert)
		{
			projection->Lambert_cylindrical_projection(image, exact);
			projection->writeFits(filename + ".Lambert_cylindrical_projection.fits");
		
			projection->dilateCircular(projeted_dilate_factor , nullvalue);
			projection->writeFits(filename + ".Lambert_cylindrical_projection.dilated.fits");
			
			deprojection->Lambert_cylindrical_deprojection(projection, exact);
			deprojection->writeFits(filename + ".Lambert_cylindrical_deprojection.dilated.fits");
		}
		
		if(sinusoidal)
		{
			projection->sinusoidal_projection(image, exact);
			projection->writeFits(filename + ".sinusoidal_projection.fits");
		
			projection->dilateCircular(projeted_dilate_factor, nullvalue);
			projection->writeFits(filename + ".sinusoidal_projection.dilated.fits");
			
			deprojection->sinusoidal_deprojection(projection, exact);
			deprojection->writeFits(filename + ".sinusoidal_deprojection.dilated.fits");
		}

		image->dilateCircular(dilate_factor_pixels, nullvalue);
		image->writeFits(filename + ".dilated.fits");

		delete deprojection;
		delete image;
	}
	
	// We create a grid of equidistant points on the sphere
	ColorMap* image = new ColorMap(size, size);
	image->zero();
	
	Real dy = size / 8.;
	Real dx = size / 8.;
	for(Real y=0; y < size/2.; y=ceil(y + dy*cos(y*PI/size)))
	{
		unsigned yl = ceil(size/2. - y - 0.5), yr = floor(size/2. + y + 0.5);
		for(Real x=0; x < size/2.; x=ceil(x + dx*cos(x*PI/size)))
		{
			unsigned xl = ceil(size/2. - x - 0.5), xr = floor(size/2. + x + 0.5);
			image->pixel(xl, yl) = image->pixel(xl, yr) = image->pixel(xr, yl) = image->pixel(xr, yr) = 1;
		}
	}
	image->nullifyAboveRadius();
	
	// We dilate a little the dots
	ColorType nullvalue = image->null();
	image->dilateCircular(2, nullvalue);
	
	string filename = "grid";
	image->writeFits(filename + ".fits");
	
	ColorMap * deprojection = new ColorMap(image->getWCS(), image->Xaxes(), image->Yaxes());
	
	Real projeted_dilate_factor = dilate_factor * (Real(projection->Xaxes())/Real(image->Xaxes())) * (2./3.);
	
	if(equirectangular)
	{
		projection->equirectangular_projection(image, exact);
		projection->writeFits(filename + ".equirectangular_projection.fits");
	
		projection->dilateCircular(projeted_dilate_factor, nullvalue);
		projection->writeFits(filename + ".equirectangular_projection.dilated.fits");
		
		deprojection->equirectangular_deprojection(projection, exact);
		deprojection->writeFits(filename + ".equirectangular_deprojection.dilated.fits");
	}
	
	if(lambert)
	{
		projection->Lambert_cylindrical_projection(image, exact);
		projection->writeFits(filename + ".Lambert_cylindrical_projection.fits");
	
		projection->dilateCircular(projeted_dilate_factor, nullvalue);
		projection->writeFits(filename + ".Lambert_cylindrical_projection.dilated.fits");
		
		deprojection->Lambert_cylindrical_deprojection(projection, exact);
		deprojection->writeFits(filename + ".Lambert_cylindrical_deprojection.dilated.fits");
	}
	
	if(sinusoidal)
	{
		projection->sinusoidal_projection(image, exact);
		projection->writeFits(filename + ".sinusoidal_projection.fits");
	
		projection->dilateCircular(projeted_dilate_factor, nullvalue);
		projection->writeFits(filename + ".sinusoidal_projection.dilated.fits");
		
		deprojection->sinusoidal_deprojection(projection, exact);
		deprojection->writeFits(filename + ".sinusoidal_deprojection.dilated.fits");
	}
	
	image->dilateCircular(dilate_factor, nullvalue);
	image->writeFits(filename + ".dilated.fits");
		
	return EXIT_SUCCESS;
}
