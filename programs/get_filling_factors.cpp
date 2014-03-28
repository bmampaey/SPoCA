//! This Program computes filling factors.
/*!
@page get_filling_factors get_filling_factors.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/get_filling_factors.x [-option optionvalue ...]  mapFile [ mapFile ... ] </tt>

@param mapFile	Path to a segmentation map

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.

@param areaLimit	The type of the limit of the area of the segmentation map taken into account the computation of stats. Possible values:
<BR>NAR=zz.z (pixels below zz.z*radius)
<BR>Long=zz.z (pixels between -zz.z and zz.z degrees of longitude)	Lat=zz.z (pixels between -zz.z and zz.z degrees of latitude)

@param excludeNull	Exclude null pixels in the computation of filling factors.

@param output	The name for the output file.

@param separator	The separator to put between columns.

@param writeHeader	Write the csv header.

See @ref Compilation_Options for constants and parameters for SPoCA at compilation time.

*/


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <map>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgParser.h"

#include "../classes/ColorMap.h"
#include "../classes/EUVImage.h"

#include "../classes/SegmentationStats.h"
#include "../classes/RegionStats.h"

using namespace std;

//! Prefix name for outputing intermediate result files
string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This Program computes filling factors.";
	programDescription+="\nVersion: 3.0";
	programDescription+="\nAuthor: Benjamin Mampaey, benjamin.mampaey@sidc.be";
	
	programDescription+="\nCompiled on "  __DATE__  " with options :";
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
	
	// We define our program parameters
	ArgParser args(programDescription);
	
	args["config"] = ArgParser::ConfigurationFile('C');
	args["help"] = ArgParser::Help('h');
	
	args["writeHeader"] = ArgParser::Parameter(true, 'w', "Write the csv header.");
	args["excludeNull"] = ArgParser::Parameter(true, 'n', "Exclude null pixels in the computation of filling factors.");
	args["areaLimit"] = ArgParser::Parameter("none", 'L', "The type of the limit of the area of the segmentation map taken into account the computation of stats. Possible values:\nNAR=zz.z (pixels below zz.z*radius)\nLong=zz.z (pixels between -zz.z and zz.z degrees of longitude)\tLat=zz.z (pixels between -zz.z and zz.z degrees of latitude)");
	args["separator"] = ArgParser::Parameter(',', 's', "The separator to put between columns.");
	args["output"] = ArgParser::Parameter(".", 'O', "The name for the output file.");
	args["mapFile"] = ArgParser::RemainingPositionalParameters("Path to a segmentation map", 1);
	
	// We parse the arguments
	try
	{
		args.parse(argc, argv);
	}
	catch ( const invalid_argument& error)
	{
		cerr<<"Error : "<<error.what()<<endl;
		cerr<<args.help_message(argv[0])<<endl;
		return EXIT_FAILURE;
	}
	
	// We open the output file
	ofstream csvFile(args["output"].as<string>().c_str(), ios_base::app);
	if(!csvFile)
	{
		cerr<<"Error : Cannot open file "<<args["output"]<<" for writing!"<<endl;
		return EXIT_FAILURE;
	}
	csvFile<<setiosflags(ios::fixed);
	
	// We setup the output directory
	string outputDirectory = getPath(args["output"]);
	if (! isDir(outputDirectory))
	{
		cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
		return EXIT_FAILURE;
	}
	
	string separator = args["separator"];
	bool writeHeader = args["writeHeader"];
	
	// We compute the filling factor for each colorMap
	deque<string> imagesFilenames = args.RemainingPositionalArguments();
	for (unsigned p = 0; p < imagesFilenames.size(); ++p)
	{
		filenamePrefix = makePath(args["output"], stripPath(stripSuffix(imagesFilenames[p]))) + ".";
		ColorMap* colorMap = getColorMapFromFile(imagesFilenames[p]);
		
		// We apply the arealimit if any
		if(args["areaLimit"].is_set())
		{
			colorMap->preprocessing(args["areaLimit"]);
			#if defined DEBUG || defined WRITE_LIMITED_MAP
				colorMap->writeFits(makePath(outputDirectory, stripSuffix(stripPath(imagesFilenames[p])) + "limited.fits"), FitsFile::compress);
			#endif
		}
		
		Header header = colorMap->getHeader();
		
		// We write the csv header
		if(writeHeader)
		{
			csvFile<<"time";
			unsigned numberClasses = 0;
			for(unsigned i = 1; i < 100; ++i)
			{
				if(header.has("CLSCTR" + toString(i,2)))
				{
					csvFile<<separator<<"CLSCTR" + toString(i,2);
					++numberClasses;
				}
				else
				{
					break;
				}
			}
			for(unsigned i = 1; i <= numberClasses; ++i)
				csvFile<<separator<<"FFCLS" + toString(i,2);
			csvFile<<endl;
			writeHeader = false;
		}
			
		// We write the date obs to the file
		csvFile<<colorMap->ObservationDate();
		
		// We write the class centers
		unsigned numberClasses = 0;
		for(unsigned i = 1; i < 100; ++i)
		{
			if(header.has("CLSCTR" + toString(i,2)))
			{
				csvFile<<separator<<replaceAll(replaceAll(header.get<string>("CLSCTR" + toString(i,2)), "(", ""), ")", "");
				++numberClasses;
			}
			else
			{
				break;
			}
		}
		
		// We compute the filling factors
		map<ColorType, unsigned> numberPixels;
		for(unsigned j = 0; j < colorMap->NumberPixels(); ++j)
		{
			if(numberPixels.count(colorMap->pixel(j)) == 0)
			{
				numberPixels[colorMap->pixel(j)] = 0;
			}
			++numberPixels[colorMap->pixel(j)];
		}
		
		float total = 0;
		for (map<ColorType, unsigned>::iterator i = numberPixels.begin(); i != numberPixels.end(); ++i)
			if(!args["excludeNull"] || i->first != 0)
				total += i->second;
		
		// We write the filling factors to the csv
		for (unsigned i = 1; i <= numberClasses; ++i)
		{
			csvFile<<separator<<float(numberPixels[i])/total;
		}
		csvFile<<endl;
		delete colorMap;
	}
	
	return EXIT_SUCCESS;
}



