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

	// The list of names of the sun images to process
	string imageType = "UNKNOWN";
	vector<string> imagesFilenames;

	string programDescription = "This Program is for testing image operations.\n";
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
	arguments.new_named_string('I', "imageType","string", "\n\tThe type of the images.\n\tPossible values are : EIT, EUVI, AIA, SWAP\n\t", imageType);
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
	

	EUVImage* image = getImageFromFile(imageType, imagesFilenames[0]);
	
	vector<HGS> ll = image->HGSmap();
	vector<HPC> hpc = image->HPCmap();
	vector<HCC> hcc = image->HCCmap();
	
	for (unsigned j = 0; j < ll.size(); ++j)
		if(!ll[j])
			image->pixel(j) = image->null();
		else
			image->pixel(j) = ll[j].longitude*180./PI;
	
	image->writeFits(stripPath(stripSuffix(imagesFilenames[0])) + ".longitude.fits", 0, "longitude");
	
	for (unsigned j = 0; j < ll.size(); ++j)
		if(!ll[j])
			image->pixel(j) = image->null();
		else
			image->pixel(j) = ll[j].latitude*180./PI;

	image->writeFits(stripPath(stripSuffix(imagesFilenames[0])) + ".latitude.fits", 0, "latitude");
	
	
	for (unsigned j = 0; j < hpc.size(); ++j)
		if(!hpc[j])
			image->pixel(j) = image->null();
		else
			image->pixel(j) = hpc[j].x;
	
	image->writeFits(stripPath(stripSuffix(imagesFilenames[0])) + ".hpc_x.fits", 0, "hpc_x");
	for (unsigned j = 0; j < hpc.size(); ++j)
		if(!hpc[j])
			image->pixel(j) = image->null();
		else
			image->pixel(j) = hpc[j].y;
	
	image->writeFits(stripPath(stripSuffix(imagesFilenames[0])) + ".hpc_y.fits", 0, "hcc_y");
	
	for (unsigned j = 0; j < hcc.size(); ++j)
		if(!hcc[j])
			image->pixel(j) = image->null();
		else
			image->pixel(j) = hcc[j].x;
	
	image->writeFits(stripPath(stripSuffix(imagesFilenames[0])) + ".hcc_x.fits", 0, "hcc_x");
	for (unsigned j = 0; j < hcc.size(); ++j)
		if(!hcc[j])
			image->pixel(j) = image->null();
		else
			image->pixel(j) = hcc[j].y;
	
	image->writeFits(stripPath(stripSuffix(imagesFilenames[0])) + ".hcc_y.fits", 0, "hcc_y");
	
	for (unsigned j = 0; j < hcc.size(); ++j)
		if(!hcc[j])
			image->pixel(j) = image->null();
		else
			image->pixel(j) = hcc[j].z;
	
	image->writeFits(stripPath(stripSuffix(imagesFilenames[0])) + ".hcc_z.fits", 0, "hcc_z");
	
	
	vector<RealPixLoc> dots;
	Real miradius = image->SunRadius()/2;
	for (Real y = image->SunCenter().y-miradius; y <= image->SunCenter().y+miradius+1; y+=miradius)
	{
		for (Real x = image->SunCenter().x-miradius; x <= image->SunCenter().x+miradius+1; x+=miradius)
		{
			RealPixLoc a = RealPixLoc(x,y);
			dots.push_back(a);
		}
	}
	cout<<fixed<<"Pixloc\thpc\thcc\thgs\thcc\thpc\tpixloc"<<endl;
	for (unsigned d = 0; d < dots.size(); ++d)
	{
		HPC hpc = image->toHPC(dots[d]);
		HCC hcc = image->toHCC(hpc);
		HGS hgs = image->toHGS(hcc);
		cout<<dots[d]<<"\t"<<hpc<<"\t"<<hcc<<"\t"<<hgs;
		hcc = image->toHCC(hgs);
		hpc = image->toHPC(hcc);
		dots[d] = image->toRealPixLoc(hpc);
		cout<<"\t"<<hcc<<"\t"<<hpc<<"\t"<<dots[d]<<endl;
	}

	
	return EXIT_SUCCESS;
}
