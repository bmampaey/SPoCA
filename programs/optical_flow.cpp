// This program will call optical flow on a pair of sun images
// Written by Benjamin Mampaey on 15 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/SunImage.h"
#include "../classes/klt.h"



using namespace std;
using namespace dsr;


string filenamePrefix;



int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// The list of names of the sun images to process
	vector<string> imagesFilenames;
	
	
	// The parameters for the optical flow
	unsigned window_width = 13;
	unsigned window_height = 13;
	unsigned pyramidLevels = 4;
	unsigned subsampling = 2;
	double pyramid_sigma_fact = 0.9f;
	double grad_sigma = 1.0f;
	double smooth_sigma_fact = 0.1f;
	unsigned max_iterations = 50;
	double max_residue = 100.0f;
	double step_factor = 1.0f;
	double min_determinant = 0.01f;
	double min_displacement = 0.1f;

	string programDescription = "This Program will track points between 2 or more images, using Lucas-Kanade Pyramidal Optical Flow.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the maps of the regions to track.\n\t", imagesFilenames);
	arguments.new_named_unsigned_int('L', "pyramidLevels", "positive integer", "\n\tThe number levels for the pyramid.\n\t", pyramidLevels);
	arguments.new_named_unsigned_int('w', "windowwidth", "positive integer", "\n\tThe width of the search window.\n\t", window_width);
	arguments.new_named_unsigned_int('h', "windowheight", "positive integer", "\n\tThe heigth of the search window.\n\t", window_height);
	arguments.new_named_unsigned_int('S', "subsampling", "positive integer", "\n\tThe subsampling for the pyramid levels.\n\t", subsampling);
	arguments.new_named_unsigned_int('M', "maxiterations", "positive integer", "\n\tThe maximal number of search iteartions.\n\t", max_iterations);
	arguments.new_named_double('p',"pyramidsigma", "positive real", "\n\tThe value of the sigma for the convolution kernel of the pyramid..\n\t",pyramid_sigma_fact);
	arguments.new_named_double('g',"gradsigma", "positive real", "\n\tThe value of the sigma for the convolution kernel of the gradient..\n\t",grad_sigma);
	arguments.new_named_double('s',"smoothsigma", "positive real", "\n\tThe value of the sigma for the convolution kernel of the smoothing..\n\t",smooth_sigma_fact);
	arguments.new_named_double('m',"maxresidue", "positive real", "\n\tThe maximal acceptable residue between the window in the first and the second image.\n\t",pyramid_sigma_fact);
	arguments.new_named_double('f',"step_factor", "positive real", "\n\tThe step factor.\n\t",step_factor);
	arguments.new_named_double('D',"mindeterminant", "positive real", "\n\tThe minimum value of the determinant to reject the existence of a solution.\n\t",min_determinant);
	arguments.new_named_double('d',"mindisplacement", "positive real", "\n\tThe minimum displacement to stop the search.\n\t",min_displacement);

	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	if(imagesFilenames.size() < 2)
	{
		cerr<<"Error : "<<imagesFilenames.size()<<" fits image file given as parameter, at leat 2 must be given!"<<endl;
		return EXIT_FAILURE;
	}

	SunImage* image1 = getImageFromFile("UNKNOWN", imagesFilenames[0]);
	for(unsigned i = 0; i < image1->NumberPixels(); ++i)
			if(image1->pixel(i) == image1->nullvalue())
				image1->pixel(i) = 0;

	vector<Coordinate> points_oldlocation;
	
	// This is for testing only
	unsigned numberofdots = 10;
	unsigned deltax = image1->Xaxes() / numberofdots;
	unsigned deltay = image1->Yaxes() / numberofdots;
	/*for(unsigned y = deltay; y < image1->Yaxes(); y+=deltay)
	{
		for(unsigned x = deltax; x < image1->Xaxes(); x+=deltax)
		{
			points_oldlocation.push_back(Coordinate(x,y));
		}
	}*/
	for(unsigned y = deltay, x = deltax; y < image1->Yaxes() && x < image1->Xaxes(); y+=deltay, x+=deltax)
	{
		points_oldlocation.push_back(Coordinate(x,y));
		points_oldlocation.push_back(Coordinate(x,image1->Yaxes() - y));
	}

	vector<Coordinate> points_newlocation = points_oldlocation;


				
	for (unsigned p = 1; p < imagesFilenames.size(); ++p)
	{
		SunImage* image2 = getImageFromFile("UNKNOWN", imagesFilenames[p]);
		for(unsigned i = 0; i < image2->NumberPixels(); ++i)
			if(image2->pixel(i) == image2->nullvalue())
				image2->pixel(i) = 0;
				

		KLTTrackFeatures(image1, image2, points_newlocation, window_width, window_height, pyramidLevels, subsampling, pyramid_sigma_fact, grad_sigma, smooth_sigma_fact = 0.1f, max_iterations, max_residue, step_factor, min_determinant, min_displacement);
	  
		for(unsigned c = 0; c < points_oldlocation.size(); ++c)
		{
			if(points_oldlocation[c] != Coordinate::Max)
			{
				image1->drawCross(c, points_oldlocation[c], 10);
			}
			
			if(points_newlocation[c] == Coordinate::Max)
			{
				cout<<c<<": "<<points_oldlocation[c]<<" was lost in second image"<<endl;
			}
			else
			{
				cout<<c<<": "<<points_oldlocation[c]<<" -> "<<points_newlocation[c]<<endl;
			}
		}
		
		image1->writeFitsImage(filenamePrefix + "features." +  stripPath(imagesFilenames[p-1]));
		delete image1;
		
		points_oldlocation = points_newlocation;
		image1 = image2;
	}
	
	for(unsigned c = 0; c < points_oldlocation.size(); ++c)
	{
		if(points_oldlocation[c] != Coordinate::Max)
		{
			image1->drawCross(c, points_oldlocation[c], 10);
		}
	}
	image1->writeFitsImage(filenamePrefix + "features." +  stripPath(imagesFilenames[imagesFilenames.size()-1]));
	delete image1;
	
	
	return EXIT_SUCCESS;
}
