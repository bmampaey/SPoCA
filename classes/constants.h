#pragma once
#ifndef Constants_H
#define Constants_H

/*!
@file constants.h

See @ref Compilation_Options "Compilation Options" for the description of each define

@page Compilation_Options Compilation Options

These are all the compilation options and constants of SPoCA.

See constants.h for the actual value 

*/


// Modify the following to adapt to your program

/*!
@page Compilation_Options
@param REAL The precision you want for the float numbers
<BR>Acceptable values are float or double (gives better precision, but increase computation time)
*/
#if ! defined(REAL)
#define REAL float
#endif

/*!
@page Compilation_Options
@param PIXELTYPE The type of the pixel values for the EUVImage. 
<BR> Acceptable values are float or double (gives better precision, but increase computation time)
 Be carrefull that depending on the type of the image you may loose precision.
 It is best to use the same type for PixelType and Real
*/
#if ! defined(PIXELTYPE)
#define PIXELTYPE float
#endif

/*!
@page Compilation_Options
@param COLORTYPE The type of the pixel color for the ColorMap 
<BR> It should be an integer, preferably a unsigned
*/
#if ! defined(COLORTYPE)
#define COLORTYPE short unsigned 
#endif

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
@param NUMBERWAVELENGTH The number of channels for the classifier.
*/
#if ! defined(NUMBERWAVELENGTH)
#define NUMBERWAVELENGTH 2
#endif

/*!
@page Compilation_Options
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
@param MIN_AR_SIZE The minimal size for an AR in (arc/sec)square (otherwise it is a bright point)
*/
#if ! defined(MIN_AR_SIZE)
#define MIN_AR_SIZE 1500
#endif

/*!
@page Compilation_Options
@param MIN_CH_SIZE The minimal size for an CH in (arc/sec)square (otherwise it is a dark point)
*/
#if ! defined(MIN_CH_SIZE)
#define MIN_CH_SIZE 3000
#endif

/*!
@page Compilation_Options
@param AR_AGGREGATION The aggregation factor for AR in (arc/sec)square (dilation)
*/
#if ! defined(AR_AGGREGATION)
#define AR_AGGREGATION 32 // Equivalent to 12 EIT pixels
#endif

/*!
@page Compilation_Options
@param CH_AGGREGATION The aggregation factor for CH in (arc/sec)square (dilation)
*/
#if ! defined(CH_AGGREGATION)
#define CH_AGGREGATION 32 // Equivalent to 60 EIT pixels
#endif

/*!
@page Compilation_Options
@param FIXETA If the computaion of eta is to be fixed or not
<BR> Acceptables values are TRUE or FALSE
*/
#define FIXETA FALSE

/*!
@page Compilation_Options
@param ETA_MAXFACTOR The maximum deviation in the computation of eta
*/
#define ETA_MAXFACTOR 100

/*!
@page Compilation_Options
@param ETA2_REDUCTION_FACTOR The reduction factor in the computation of eta2
*/
#define ETA2_REDUCTION_FACTOR 95

/*!
@page Compilation_Options
@param MERGE Type of merge function to use
<BR> Possible values are MERGEMAX , MERGECIS
*/
#define MERGE MERGECIS

/*!
@page Compilation_Options
@param MERGEVINCENT_ALPHA alpha value of Vincent's merge function
*/
#define MERGEVINCENT_ALPHA 20./255.

/*!
@page Compilation_Options
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
@param SSW_PATH Path to solarsoft for CoordinateConvertor
*/
#define SSW_PATH "/usr/local/ssw/"
#define WCS_ROUTINES_SAV "./idl/wcs_routines.sav"

/*!
@page Compilation_Options
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
@section disk_center_area Parameters to compute the area at disk center
@param SUNRADIUS The radius of the sun in Mmeters (R0)
*/
#define SUNRADIUS 695.508
/*!
@page Compilation_Options
@param DR0 Something. CIS, what is it ?
*/
#define DR0 0.026
/*!
@page Compilation_Options
@param DR Something else.
*/
#define DR 2.
/*!
@page Compilation_Options
@param HIGGINS_FACTOR The higgins_factor
*/
#define HIGGINS_FACTOR  16

/*!
@page Compilation_Options
@section region_stats Constants for the region stats
@param distance_observer_sun 
@param earth_orbit_eccentricity 
@param yearly_maximal_error 
@param rad2arcsec 
@param RADEG 
@param DEGRA 
*/

#define distance_observer_sun 149597.871
#define earth_orbit_eccentricity 0.0167
#define yearly_maximal_error (distance_observer_sun * earth_orbit_eccentricity)
#define rad2arcsec 206264.806247096
#define RADEG 57.295779513
#define DEGRA 0.017453293

/*---------------- Do NOT modify below please ------------------*/


#if REAL==float
#define Real float
#elif REAL==double
#define Real double
#else
#warning "REAL not defined or badly defined, using default type double"
#define Real double
#endif

#if PIXELTYPE==float
#define PixelType float
#elif PIXELTYPE==double
#define PixelType double
#else
#warning "PIXELTYPE not defined or badly defined, using default type double"
#define PixelType double
#endif


#ifndef COLORTYPE
#warning "COLORTYPE not defined or badly defined, using default type unsigned short"
#define ColorType short unsigned 
#else
#define ColorType COLORTYPE
#endif

#define PI 3.14159265358979323846
#define MIPI 1.57079632679489661923
#define BIPI 6.28318530717958647692

#endif
