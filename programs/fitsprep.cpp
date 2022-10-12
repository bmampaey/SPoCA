//! This program applying some contrast enhancement to a sun image FITS file.
/*!
@page fitsprep.x

Version: 3.0

Author: Benjamin Mampaey, benjamin.mampaey@sidc.be

@section usage Usage
<tt> bin/fitsprep.x [-option optionvalue ...] fitsFile</tt>

@param fitsFile	Path to the sun image FITS file to be enhanced

global parameters:

@param help	Print a help message and exit.
<BR>If you pass the value doxygen, the help message will follow the doxygen convention.
<BR>If you pass the value config, the help message will write a configuration file template.

@param config	Program option configuration file.


@param imagePreprocessing	The steps of preprocessing to apply to the sun images.
<BR>Can be any combination of the following:
<BR> NAR=zz.z (Nullify pixels above zz.z*radius)
<BR> ALC (Annulus Limb Correction)
<BR> DivMedian (Division by the median)
<BR> TakeSqrt (Take the square root)
<BR> TakeLog (Take the log)
<BR> TakeAbs (Take the absolute value)
<BR> DivMode (Division by the mode)
<BR> DivExpTime (Division by the Exposure Time)
<BR> ThrMin=zz.z (Threshold intensities to minimum zz.z)
<BR> ThrMax=zz.z (Threshold intensities to maximum zz.z)
<BR> ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)
<BR> ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile)
<BR> ThrMinMode (Threshold intensities to minimum the mode of pixel intensities)
<BR> ThrMaxMode (Threshold intensities to maximum the mode of pixel intensities)
<BR> Smooth=zz.z (Binomial smoothing of zz.z arcsec)

@param uncompressed	Set this to true if you want results maps to be uncompressed.

@param output	The path of the output file or of a directory.

*/


#include <vector>
#include <iostream>
#include <string>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgParser.h"

#include "../classes/EUVImage.h"

using namespace std;


string filenamePrefix;

int main(int argc, const char **argv)
{
	// We declare our program description
	string programDescription = "This program applying some contrast enhancement to a sun image FITS file.";
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
	
	args["imagePreprocessing"] = ArgParser::Parameter('P', "The steps of preprocessing to apply to the sun images.\nCan be any combination of the following:\n NAR=zz.z (Nullify pixels above zz.z*radius)\n ALC (Annulus Limb Correction)\n DivMedian (Division by the median)\n TakeSqrt (Take the square root)\n TakeLog (Take the log)\n TakeAbs (Take the absolute value)\n DivMode (Division by the mode)\n DivExpTime (Division by the Exposure Time)\n ThrMin=zz.z (Threshold intensities to minimum zz.z)\n ThrMax=zz.z (Threshold intensities to maximum zz.z)\n ThrMinPer=zz.z (Threshold intensities to minimum the zz.z percentile)\n ThrMaxPer=zz.z (Threshold intensities to maximum the zz.z percentile\n ThrMinMode (Threshold intensities to minimum the mode)\n ThrMaxMode (Threshold intensities to maximum the mode)\n Smooth=zz.z (Binomial smoothing of zz.z arcsec)");
	args["uncompressed"] = ArgParser::Parameter(false, 'u', "Set this to true if you want results image to be uncompressed.");
	args["output"] = ArgParser::Parameter(".", 'O', "The path of the output file or of a directory.");
	args["fitsFile"] = ArgParser::PositionalParameter("Path to the sun image FITS file to be enhanced");
	
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
	
	// We setup the filenamePrefix
	if (isDir(args["output"]))
	{
		filenamePrefix = stripSuffix(stripPath(args["fitsFile"]));
		filenamePrefix = makePath(args["output"], filenamePrefix + ".");
	}
	else
	{
		string outputDirectory = getPath(args["output"]);
		if (! isDir(outputDirectory))
		{
			cerr<<"Error : "<<outputDirectory<<" is not a directory!"<<endl;
			return EXIT_FAILURE;
		}
		filenamePrefix = stripSuffix(args["output"]) + ".";
	}
	
	EUVImage* inputImage = getImageFromFile("Unknown", args["fitsFile"]);
	
	// We improve the contrast
	if(args["imagePreprocessing"].is_set())
	{
		inputImage->preprocessing(args["imagePreprocessing"]);
		inputImage->getHeader().set("IPREPROC", args["imagePreprocessing"], "Image Preprocessing");
	}
	else
	{
		inputImage->enhance_contrast();
	}
	
	inputImage->writeFits(filenamePrefix + "fits", args["uncompressed"] ? 0 : FitsFile::compress);
	return EXIT_SUCCESS;
}
