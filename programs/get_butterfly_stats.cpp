//! Program that computes the butterfly statistics of a map
/*!
@page get_butterfly_stats get_butterfly_stats.x

 This program takes colorized maps in fits format and computes the area of all the regions latitude by latitude
  
 @section usage Usage
 
 <tt> get_butterfly_stats.x -h </tt>
 
 Calling the programs with -h will provide you with help 
 
 <tt> get_butterfly_stats.x [-option optionvalue, ...] colorizeMap colorizeMap ...</tt>
 
@param separator	The separator to put between columns.

@param colors The list of colors to select separated by commas (no spaces)
<BR>All colors will be selected if ommited.

@param colorsFilename A file containing a list of colors to select separated by commas
<BR>All colors will be selected if ommited.

See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <set>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/ColorMap.h"
#include "../classes/EUVImage.h"


#include "../classes/RegionStats.h"
#include "../classes/Coordinate.h"
#include "../classes/FeatureVector.h"


using namespace std;
using namespace dsr;

string filenamePrefix;



int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// Program version
	string version = "2.0";
	
	// The maps of colored regions
	vector<string> imagesFilenames;
	
	// Options for the colors to select
	string colorsString;
	string colorsFilename;
	
	// Option for the output
	string separator = ",";
	
	// Option for the output file/directory
	string output = "butterfly_stats.csv";
	
	string programDescription = "This Programm output regions info and statistics.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nNUMBERCHANNELS: " + itos(NUMBERCHANNELS);
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.new_named_string('s', "separator", "string", "\n\tThe separator to put between columns.\n\t", separator);
	arguments.new_named_string('c', "colors", "string", "\n\tThe list of colors to select separated by commas (no spaces)\n\tAll colors will be selected if ommited.\n\t", colorsString);
	arguments.new_named_string('C', "colorsFilename", "string", "\n\tA file containing a list of colors to select separated by commas\n\tAll colors will be selected if ommited.\n\t", colorsFilename);
	arguments.new_named_string('O', "output","file name", "\n\tThe name for the output file.\n\t", output);
	arguments.set_string_vector("colorizedMap colorizedMap ...", "The name of the fits files containing the colorized maps.", imagesFilenames);

	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version(version.c_str());
	arguments.process(argc, argv);

	if(imagesFilenames.size() < 1)
	{
		cerr<<"No fits image file given as parameter!"<<endl;
		return EXIT_FAILURE;
	}
	
	// We parse the colors to keep
	set<ColorType> colors;
	if(! colorsFilename.empty())
	{
		ifstream colorsFile(colorsFilename.c_str());
		if(colorsFile.good())
		{
			vector<ColorType> tmp;
			colorsFile>>tmp;
			colors.insert(tmp.begin(),tmp.end());
		}
		else
		{
			cerr << "Error reading list of colors to overlay from file: "<<colorsFilename<<endl;
			return 2;
		}
	}
	if(! colorsString.empty())
	{
		vector<ColorType> tmp;
		istringstream ss(colorsString);
		ss>>tmp;
		colors.insert(tmp.begin(),tmp.end());
	}
	
	// We open the output file
	ofstream outputFile(output.c_str(), ios_base::trunc);
	
	
	vector<float> totalNumberOfPixels;
	vector<float> regionNumberOfPixels;
	vector<float> correctedTotalNumberOfPixels;
	vector<float> correctedRegionNumberOfPixels;
	
	// We write the header of the columns
	outputFile<<"time"<<separator<<"stat";
	for (int latitude = -91; latitude < 0; ++latitude)
		outputFile<<separator<<latitude;
	for (int latitude = 0; latitude < 91; ++latitude)
		outputFile<<separator<<latitude;
	outputFile<<endl;
	
	// We process files one by one
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		// We read the map
		ColorMap* colorizedMap = getImageFromFile(imagesFilenames[p]);
		cout<<imagesFilenames[p]<<endl;
		// We erase any colors that is not to be kept
		if(colors.size() > 0)
		{
			for(unsigned j = 0; j < colorizedMap->NumberPixels(); ++j)
			{
				if (colors.count(colorizedMap->pixel(j)) == 0)
					colorizedMap->pixel(j) = colorizedMap->null();
			}
			#if DEBUG >= 2
				colorizedMap->writeFits(filenamePrefix + "color_cleaned." +  stripPath(colorizedMapFileName));
			#endif
		}
		
		// We compute the butterfly stats
		colorizedMap->computeButterflyStats(totalNumberOfPixels, regionNumberOfPixels, correctedTotalNumberOfPixels, correctedRegionNumberOfPixels);
		
		// We write the absolute number of pixels to the file
		outputFile<<colorizedMap->ObservationDate()<<separator<<"absoluteNumberOfPixels";
		for (unsigned i = 0 ; i < regionNumberOfPixels.size(); ++i)
			outputFile<<separator<<regionNumberOfPixels[i];
		outputFile<<"\n";
		
		// We write the relative number of pixels to the file
		outputFile<<colorizedMap->ObservationDate()<<separator<<"relativeNumberOfPixels";
		for (unsigned i = 0 ; i < regionNumberOfPixels.size(); ++i)
			outputFile<<separator<<regionNumberOfPixels[i]/totalNumberOfPixels[i];
		outputFile<<"\n";
		
		// We write the absolute corrected number of pixels to the file
		outputFile<<colorizedMap->ObservationDate()<<separator<<"absoluteCorrectedNumberOfPixels";
		for (unsigned i = 0 ; i < correctedRegionNumberOfPixels.size(); ++i)
			outputFile<<separator<<correctedRegionNumberOfPixels[i];
		outputFile<<"\n";
		
		// We write the relative corrected number of pixels to the file
		outputFile<<colorizedMap->ObservationDate()<<separator<<"relativeCorrectedNumberOfPixels";
		for (unsigned i = 0 ; i < correctedRegionNumberOfPixels.size(); ++i)
			outputFile<<separator<<correctedRegionNumberOfPixels[i]/correctedTotalNumberOfPixels[i];
		outputFile<<"\n";
		
		delete colorizedMap;
	}
	outputFile.close();
	return EXIT_SUCCESS;
	
}



