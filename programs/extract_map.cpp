#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <fenv.h>
#include <iomanip>
#include <utility>

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

// Function to parse the preprocessing steps
vector<pair<string, double> > get_preprocesing_steps(const string& preprocessingSteps)
{
	vector<pair<string, double> > preprocessing;
	vector<string> preprocessing_steps = split(preprocessingSteps, ',');
	for (vector<string>::iterator step = preprocessing_steps.begin() ; step != preprocessing_steps.end(); ++step)
	{
		vector<string> name_value = split(*step, '=');
		if (name_value.size() > 2)
		{
			cerr<<"Error : preprocessing step "<<*step<<" is malformed!"<<endl;
		}
		if (name_value[0] == "erode" || name_value[0] == "dilate")
		{
			preprocessing.push_back(pair<string, double>(name_value[0], stod(name_value[1])));
		}
		else if (name_value[0] == "NAR")
		{
			if (name_value.size() < 2)
				preprocessing.push_back(pair<string, double>(name_value[0], 1));
			else
				preprocessing.push_back(pair<string, double>(name_value[0], stod(name_value[1])));
		}
		else if (name_value[0] != "none" && !name_value[0].empty())
		{
			cerr<<"Error : Unknown preprocessing step "<<name_value[0]<<"!"<<endl;
		}
	}
	return preprocessing;
}

ColorMap* get_projected_map(ColorMap* map, const string& projection, bool exact = false)
{
	ColorMap * projected_map = new ColorMap(map->getWCS(), map->Xaxes(), map->Yaxes());
	if(projection == "equirectangular")
	{
		projected_map->equirectangular_projection(map, exact);
	}
	else if(projection == "lambert")
	{
		projected_map->Lambert_cylindrical_projection(map, exact);
	}
	else if(projection == "sinusoidal")
	{
		projected_map->sinusoidal_projection(map, exact);
	}
	#if defined DEBUG
		filenamePrefix += projection+".";
		projected_map->writeFits(filenamePrefix + "fits");
	#endif
	return projected_map;
}

ColorMap* get_deprojected_map(ColorMap* map, const string& projection, bool exact = false)
{
	ColorMap * deprojected_map = new ColorMap(map->getWCS(), map->Xaxes(), map->Yaxes());
	if(projection == "equirectangular")
	{
		deprojected_map->equirectangular_deprojection(map, exact);
	}
	else if(projection == "lambert")
	{
		deprojected_map->Lambert_cylindrical_deprojection(map, exact);
	}
	else if(projection == "sinusoidal")
	{
		deprojected_map->sinusoidal_deprojection(map, exact);
	}
	#if defined DEBUG
		filenamePrefix += "deprojected.";
		deprojected_map->writeFits(filenamePrefix + "fits");
	#endif
	return deprojected_map;
}

int main(int argc, const char **argv)
{
	#if defined EXTRA_SAFE
	feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif
	
	// Option for the color to extract
	unsigned color = 1;
	
	// Options for the projections
	string projection = "none";
	
	// Option for the recoloring
	bool recolor = false;
	
	// Options for the preprocessing of images
	string preprocessingSteps = "none";
	
	// option for the output directory
	string outputDirectory = ".";
	
	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	string programDescription = "This Program extract a binary map from a segmentation map.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	#if defined DEBUG
	programDescription+="\nDEBUG: ON";
	#endif
	#if defined EXTRA_SAFE
	programDescription+="\nEXTRA_SAFE: ON";
	#endif
	#if defined VERBOSE
	programDescription+="\VERBOSE: ON";
	#endif
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('P', "preprocessingSteps", "comma separated list of string (no spaces)", "\n\tThe steps of preprocessing to apply to the sun images.\n\tPossible values :\n\t\tnone\n\t\terode\n\t\tdilate\n\t\tNAR (Nullify above radius)\n\t", preprocessingSteps);
	arguments.new_named_string('p', "projection", "string", "\n\tSet the projection that you want to apply before the closing.\n\tPossible values are : none, equirectangular, lambert, sinusoidal, exact\n\t", projection);
	arguments.new_flag('r', "recolor", "\n\tSet this flag if you want each blob to get a different color.\n\t", recolor);
	arguments.new_named_unsigned_int('c', "color", "positive integer", "\n\tThe color to extract from the segmentation map.\n\t", color);
	arguments.new_named_string('O', "outputDirectory","directory name", "\n\tThe name for the output directory.\n\t", outputDirectory);
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of color maps.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);
	
	// We verify the projection option
	if(projection != "none" && projection != "equirectangular" && projection != "lambert" && projection != "sinusoidal" && projection != "exact")
	{
		cerr<<"Unknown projection type!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We check if the outputDirectory is a directory 
	if (! isDir(outputDirectory))
	{
		cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We get the preprocessing step
	vector<pair<string, double> > preprocesing_steps = get_preprocesing_steps(preprocessingSteps);
	
	// We read and process the files
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		filenamePrefix = outputDirectory+"/"+stripPath(stripSuffix(imagesFilenames[p]))+".";
		ColorMap* map = getImageFromFile(imagesFilenames[p]);
		map->bitmap(color);
		ColorMap * projected_map = NULL;
		for (vector<pair<string, double> >::iterator step = preprocesing_steps.begin() ; step != preprocesing_steps.end(); ++step)
		{
			if (step->first == "NAR")
			{
				if (projected_map != NULL)
				{
					// We need to deproject first
					map = get_deprojected_map(projected_map, projection);
					delete projected_map;
					projected_map = NULL;
				}
				map->nullifyAboveRadius(step->second);
				#if defined DEBUG
					filenamePrefix += "NAR.";
					map->writeFits(filenamePrefix + "fits");
				#endif
			}
			else if (step->first == "erode")
			{
				if (projection == "none" || projection.empty())
				{
					map->erodeCircular(step->second/map->PixelWidth(), 0);
				}
				else if (projection == "exact")
				{
					filenamePrefix += "exact.";
					map->erodeCircularProjected(step->second/map->PixelWidth(), 0);
				}
				else
				{
					if (projected_map == NULL)
					{
						projected_map = get_projected_map(map, projection);
						delete map;
						map = NULL;
					}
					projected_map->erodeCircular(step->second/projected_map->PixelWidth(), 0);
				}
				#if defined DEBUG
					filenamePrefix += "eroded.";
					if (projected_map != NULL)
						projected_map->writeFits(filenamePrefix + "fits");
					else
						map->writeFits(filenamePrefix + "fits");
				#endif
			}
			else if (step->first == "dilate")
			{
				if (projection == "none" || projection.empty())
				{
					map->dilateCircular(step->second/map->PixelWidth(), 0);
				}
				else if (projection == "exact")
				{
					filenamePrefix += "exact.";
					map->dilateCircularProjected(step->second/map->PixelWidth(), 0);
				}
				else
				{
					if (projected_map == NULL)
					{
						projected_map = get_projected_map(map, projection);
						delete map;
						map = NULL;
					}
					projected_map->dilateCircular(step->second/projected_map->PixelWidth(), 0);
				}
				#if defined DEBUG
					filenamePrefix += "dilated.";
					if (projected_map != NULL)
						projected_map->writeFits(filenamePrefix + "fits");
					else
						map->writeFits(filenamePrefix + "fits");
				#endif
			}
		}
		// We deproject the map if necessary
		if (projected_map != NULL)
		{
			map = get_deprojected_map(projected_map, projection);
			delete projected_map;
			projected_map = NULL;
		}
		
		// We recolor eventually
		if (recolor)
			map->colorizeConnectedComponents(1);
		
		// We write the map
		map->writeFits(outputDirectory+"/"+stripPath(stripSuffix(imagesFilenames[p]))+ ".map" + itos(color) + ".fits");
	}
	return EXIT_SUCCESS;
}
