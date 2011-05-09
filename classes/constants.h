#pragma once
#ifndef Constants_H
#define Constants_H

/*!
@file constants.h

See @ref Compilation_Options "Compilation Options" for the description of each define

@page Compilation_Options Compilation Options

These are all the compilation options and constants of SPoCA.

See constants.h for the actual values.

*/


// Modify the following to adapt to your program

/*!
@page Compilation_Options

@section data_types Types of data for the different container

@param Real The precision you want for the float numbers
<BR>Acceptable values are float or double (gives better precision, but increase computation time)
*/
#if ! defined(Real)
#define Real float
#endif

/*!
@page Compilation_Options
@param EUVPixelType The type of the pixel values for the EUVImage. 
<BR> Acceptable values are float or double (gives better precision, but increase computation time)
 Be carrefull that depending on the type of the image you may loose precision.
 It is best to use the same type for EUVPixelType and Real
*/
#if ! defined(EUVPixelType)
#define EUVPixelType float
#endif

/*!
@page Compilation_Options
@param ColorType The type of the pixel color for the ColorMap 
<BR> It should be an integer, preferably a unsigned
*/
#if ! defined(ColorType)
#define ColorType short unsigned 
#endif

/*!
@page Compilation_Options

@section behavior_options Modification of the behavior of the program

@param NUMBERCHANNELS The number of channels for the classifier.
*/
#if ! defined(NUMBERCHANNELS)
#define NUMBERCHANNELS 2
#endif

/*!
@page Compilation_Options

@subsection possibilistic_options Possibilistic Classifiers

@param FIXETA If the computaion of eta is to be fixed or not
<BR> Acceptables values are true or false
*/

#if ! defined(FIXETA)
#define FIXETA false
#endif

/*!
@page Compilation_Options
@param ETA_MAXFACTOR The maximum deviation in the computation of eta
*/
#if ! defined(ETA_MAXFACTOR)
#define ETA_MAXFACTOR 100
#endif

/*!
@page Compilation_Options
@param ETA2_REDUCTION_FACTOR The reduction factor in the computation of eta2
*/
#if ! defined(ETA2_REDUCTION_FACTOR)
#define ETA2_REDUCTION_FACTOR 95
#endif

/*!
@page Compilation_Options

@subsection sursegmentation_options Sursegmentation
See Classifier::merge and Classifier::sursegmentation
@param MERGE_TYPE Type of merge function to use
<BR> Possible values are:
 -# MERGEMAX	The 2 centers are merged by taking the mean value of the pixels belonging to those classes (i.e. their membership is maximal for that class)
 <BR>		The membership are updated by using the formula to compute the membership of the corresponding Classifier
 -# MERGECIS	The membership is updated by taking the maximal membership of the 2 classes
 <BR>		The 2 centers are merged by using the formula to compute the centers of the corresponding Classifier

*/
#if ! defined(MERGE_TYPE)
#define MERGE_TYPE MERGECIS
#endif
/*!
@page Compilation_Options
@param MERGEVINCENT_ALPHA alpha value of Vincent's merge function
*/
#define MERGEVINCENT_ALPHA 20./255.


/*!
@page Compilation_Options

@subsection various_options Various

@param DEBUG The level of debugging
<BR>  This manage the quantity of output to screen and files, and verification to be done
 - 0 is to disable debugging
 - 1 is for verification and output of the results only
 - 2 is 1 + lots of output to files
 - 3 is 2 + lots of output to screen
*/
#if ! defined(DEBUG)
#define DEBUG 0
#endif

/*!
@page Compilation_Options
@param SSW_PATH Path to solarsoft for CoordinateConvertor
*/
#define SSW_PATH "/usr/local/ssw/"
#define WCS_ROUTINES_SAV "./idl/wcs_routines.sav"

/*!
@page Compilation_Options

@param STRICTFITS Cfitsio defines additional unsigned type that are not part of the strict FITS standart data format
<BR> If you want to read and manipulate Fits files with software not written with the cfitsio (e.g. ImageMagick and IDL), you should set STRICTFITS
*/
#ifndef STRICTFITS
#define STRICTFITS
#endif


/*!
@page Compilation_Options

@section region_extraction Extraction of Coronal Hole (CH) and Active Region (AR)
See ActiveRegion.h and CoronalHole.h for information about how these parameters are used.

@param MIN_AR_SIZE The minimal size for an AR in (arc/sec)² (otherwise it is a bright point)
*/
#if ! defined(MIN_AR_SIZE)
#define MIN_AR_SIZE 1500
#endif

/*!
@page Compilation_Options
@param MIN_CH_SIZE The minimal size for an CH in (arc/sec)² (otherwise it is a dark point)
*/
#if ! defined(MIN_CH_SIZE)
#define MIN_CH_SIZE 3000
#endif

/*!
@page Compilation_Options
@param AR_AGGREGATION The aggregation factor for AR in (arc/sec)
*/
#if ! defined(AR_AGGREGATION)
#define AR_AGGREGATION 32. // Equivalent to 12 EIT pixels
#endif

/*!
@page Compilation_Options
@param CH_AGGREGATION The aggregation factor for CH in (arc/sec)
*/
#if ! defined(CH_AGGREGATION)
#define CH_AGGREGATION 32. // Equivalent to 12 EIT pixels
#endif


/*!
@page Compilation_Options
@param AR_AGGREGATION_TYPE Type of aggregation to use for AR
<BR> Possible values are:
 -# AR_AGGREGATION_DILATE: The AR map is dilated by a disc of radius AR_AGGREGATION.
 -# AR_AGGREGATION_FRAGMENTED: A dilated map like the one described above is used to tell wich blobs correspond to the same AR.
 This means that the resulting AR can be fragmented.
 -# AR_AGGREGATION_CLOSING: A closing of factor AR_AGGREGATION is done on the AR map.
*/

#if ! defined(AR_AGGREGATION_TYPE)
#define AR_AGGREGATION_TYPE AR_AGGREGATION_FRAGMENTED
#endif

/*!
@page Compilation_Options
@param CH_AGGREGATION_TYPE Type of aggregation to use for CH
<BR> Possible values are:
 -# CH_AGGREGATION_DILATE: The CH map is dilated by a disc of radius CH_AGGREGATION.
 -# CH_AGGREGATION_FRAGMENTED: A dilated map like the one described above is used to tell wich blobs correspond to the same CH.
 This means that the resulting CH can be fragmented.
 -# CH_AGGREGATION_CLOSING: A closing of factor CH_AGGREGATION is done on the CH map.
*/

#if ! defined(CH_AGGREGATION_TYPE)
#define CH_AGGREGATION_TYPE CH_AGGREGATION_CLOSING
#endif

/*!
@page Compilation_Options
@param AR_CLEANING The cleaning factor for AR in (arc/sec)
*/
#if ! defined(AR_CLEANING)
#define AR_CLEANING 6. // Equivalent to 2 EIT pixels
#endif

/*!
@page Compilation_Options
@param CH_CLEANING The cleaning factor for CH in (arc/sec)
*/
#if ! defined(CH_CLEANING)
#define CH_CLEANING 6. // Equivalent to 2 EIT pixels
#endif


/*!
@page Compilation_Options

@section ALC_corr Anulus Limb Correction

@param EIT_*_CORR_R* Limits for the limb correction
<BR> For instrument EIT
*/
#define EIT_DISCRETE_CORR_R1  95
#define EIT_SLOPE_CORR_R1 90
#define EIT_SLOPE_CORR_R2 95
#define EIT_SINE_CORR_R1 90
#define EIT_SINE_CORR_R2 95
#define EIT_SINE_CORR_R3 105
#define EIT_SINE_CORR_R4 110

/*!
@page Compilation_Options
@param EUVI_*_CORR_R* Limits for the limb correction
<BR> For instrument EUVI
*/
#define EUVI_DISCRETE_CORR_R1  95
#define EUVI_SLOPE_CORR_R1 90
#define EUVI_SLOPE_CORR_R2 95
#define EUVI_SINE_CORR_R1 95
#define EUVI_SINE_CORR_R2 100
#define EUVI_SINE_CORR_R3 105
#define EUVI_SINE_CORR_R4 110

/*!
@page Compilation_Options
@param AIA_*_CORR_R* Limits for the limb correction
<BR> For instrument AIA
*/
#define AIA_DISCRETE_CORR_R1  95
#define AIA_SLOPE_CORR_R1 90
#define AIA_SLOPE_CORR_R2 95
#define AIA_SINE_CORR_R1 80
#define AIA_SINE_CORR_R2 102
#define AIA_SINE_CORR_R3 105
#define AIA_SINE_CORR_R4 112

/*!
@page Compilation_Options
@param SWAP_*_CORR_R* Limits for the limb correction
<BR> For instrument SWAP
*/
#define SWAP_DISCRETE_CORR_R1  95
#define SWAP_SLOPE_CORR_R1 90
#define SWAP_SLOPE_CORR_R2 95
#define SWAP_SINE_CORR_R1 95
#define SWAP_SINE_CORR_R2 100
#define SWAP_SINE_CORR_R3 105
#define SWAP_SINE_CORR_R4 115

/*!
@page Compilation_Options
@param *_CORR_R* Limits for the limb correction
<BR> For a default instrument
*/
#define DISCRETE_CORR_R1  95
#define SLOPE_CORR_R1 90
#define SLOPE_CORR_R2 95
#define SINE_CORR_R1 95
#define SINE_CORR_R2 100
#define SINE_CORR_R3 105
#define SINE_CORR_R4 115


/*!
@page Compilation_Options

@section HEK

@param MIN_QUOTIENT_FACTOR The minimum quotient factor 
*/
#define MIN_QUOTIENT_FACTOR 5

/*!
@page Compilation_Options
@param MAX_QUOTIENT_FACTOR The maximum quotient factor 
*/
#define MAX_QUOTIENT_FACTOR 10




/*!
@page Compilation_Options

@section solar_constants Solar Constants

@param SUNRADIUS The radius of the sun in Mmeters (R0)
*/
#define SUNRADIUS 695.508
/*!
@page Compilation_Options
@param DR0 Something. Maximal variation in the distance between satellite and Sun, in Mm.
*/
#define DR0 0.026
/*!
@page Compilation_Options
@param DR Maximal variation in the distance between satellite and Sun, in pixels.
*/
#define DR 2.

/*!
@page Compilation_Options
@param HIGGINS_FACTOR Limit of correction for projection effects in the computation of the area at disk center
*/
#define HIGGINS_FACTOR 16

/*!
@page Compilation_Options
@param distance_observer_sun distance between the earth and the sun
*/
#define distance_observer_sun 149597.871

/*!
@page Compilation_Options
@param earth_orbit_eccentricity 
*/
#define earth_orbit_eccentricity 0.0167

/*!
@page Compilation_Options
@param yearly_maximal_error 
*/
#define yearly_maximal_error (distance_observer_sun * earth_orbit_eccentricity)

/*!
@page Compilation_Options
@section universal_constants Universal Constants
@param rad2arcsec Conversion between radians and arcsec
*/

#define rad2arcsec 206264.806247096

/*!
@page Compilation_Options

@param RADEG Conversion between radians and degrees
*/
#define RADEG 57.295779513

/*!
@page Compilation_Options

@param DEGRA Conversion between degrees and radians 
*/
#define DEGRA 0.017453293

/*!
@page Compilation_Options

@param PI Value of PI
@param MIPI Value of PI/2
@param BIPI Value of PI*2
*/
#define PI 3.14159265358979323846
#define MIPI 1.57079632679489661923
#define BIPI 6.28318530717958647692

/*---------------- Do NOT modify below please ------------------*/


#if !(Real==float || Real==double)
#warning "Real not defined or badly defined, using default type float"
#define Real float
#endif

#if !(EUVPixelType==float || EUVPixelType==double)
#warning "EUVPixelType not defined or badly defined, using default type float"
#define EUVPixelType float
#endif


#ifndef ColorType
#warning "COLORTYPE not defined or badly defined, using default type unsigned short"
#define ColorType short unsigned 
#endif

#define AR_AGGREGATION_DILATE 1
#define AR_AGGREGATION_FRAGMENTED 2
#define AR_AGGREGATION_CLOSING 3
#define CH_AGGREGATION_DILATE 1
#define CH_AGGREGATION_FRAGMENTED 2
#define CH_AGGREGATION_CLOSING 3
#define MERGEMAX 1
#define MERGECIS 2
#define MERGESUM 3
#define MERGEVINCENT 4

#endif //end of Constants_H
