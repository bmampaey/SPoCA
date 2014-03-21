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
#define ColorType unsigned 
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
@param NUMBER_BINS The default number of bins to use in 2 times the variance when constructing an histogram
<BR> It should be a positive integer
*/

#if ! defined(NUMBER_BINS)
#define NUMBER_BINS 100
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

@subsection various_options Various
@param DEBUG Enables debugging, a lot of debug files will be written to disk
@param VERBOSE Enables verbose output, a lot of information will be printed to screen. This can be disruptive to script parsing the output of the programs.
@param EXTRA_SAFE Enables some additional checks during program execution. Can make the program slower.
@param WRITE_LIMITED_MAP Will write the maps with the limit applied in program get_region_stats
@param WRITE_MEMBERSHIP_FILES Will write the membership values to fits files during classification or attribution.
@param WRITE_CENTERS_FILE Will write the class centers values to a text file after classification.

*/

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

@section ALC_corr Annulus Limb Correction
See @ref ALC.

@param ANNULUS_WIDTH Parameters for the width of an annulus in the Annulus Limb Correction
*/

#define ANNULUS_WIDTH (2.)

/*
@page Compilation_Options
@param EIT_ALC_PARAMETERS Parameters for Annulus Limb Correction of EIT images

*/

#define EIT_ALC_PARAMETERS {80, 95, 107, 112}

/*!
@page Compilation_Options
@param EUVI_ALC_PARAMETERS Parameters for Annulus Limb Correction of EUVI images

*/

#define EUVI_ALC_PARAMETERS {70, 95, 108, 112}

/*!
@page Compilation_Options
@param AIA_ALC_PARAMETERS Parameters for Annulus Limb Correction of AIA images

*/

#define AIA_ALC_PARAMETERS {70, 95, 108, 112} 

/*!
@page Compilation_Options
@param SWAP_ALC_PARAMETERS Parameters for Annulus Limb Correction of SWAP images

*/

#define SWAP_ALC_PARAMETERS {70, 75, 107, 112} 

/*!
@page Compilation_Options
@param EUV_ALC_PARAMETERS Parameters for Annulus Limb Correction of EUV images

*/

#define EUV_ALC_PARAMETERS {70, 95, 108, 112} 


/*!
@page Compilation_Options

@section solar_constants Solar Constants

@param SUN_RADIUS The average radius of the sun in Mmeters
*/
#define SUN_RADIUS 695.508

/*!
@page Compilation_Options

@param DISTANCE_EARTH_SUN The average distance between the earth and the sun in Mmeters
*/
#define DISTANCE_EARTH_SUN 149597.870691

/*!
@page Compilation_Options
@param EARTH_ORBIT_ECCENTRICITY  
*/
#define EARTH_ORBIT_ECCENTRICITY 0.0167

/*!
@page Compilation_Options
@param YEARLY_MAXIMAL_ERROR 
*/
#define YEARLY_MAXIMAL_ERROR (DISTANCE_EARTH_SUN * EARTH_ORBIT_ECCENTRICITY)

/*!
@page Compilation_Options
@param SUN_RADIUS_VARIATION Maximal variation of the radius of the sun, in Mm.
*/
#define SUN_RADIUS_VARIATION 0.026
/*!
@page Compilation_Options
@param SUN_RADIUS_VARIATION_PIXELS Maximal variation of the radius of the sun, in pixels.
*/
#define SUN_RADIUS_VARIATION_PIXELS 2.

/*!
@page Compilation_Options
@param HIGGINS_FACTOR Limit of correction for projection effects in the computation of the area at disk center
*/
#define HIGGINS_FACTOR 16

/*!
@page Compilation_Options
@section mathematical_constants Mathematical Constants
@param PI Value of PI
@param MIPI Value of PI/2
@param BIPI Value of PI*2
*/
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679
#define MIPI (PI/2.)
#define BIPI (2.*PI)

/*!
@page Compilation_Options
@param RADIAN2ARCSEC Conversion between radians and arcsec
*/

#define RADIAN2ARCSEC (648000./PI)

/*!
@page Compilation_Options
@param ARCSEC2RADIAN Conversion between arcsec and radians
*/

#define ARCSEC2RADIAN (PI/648000.)

/*!
@page Compilation_Options
@param RADIAN2DEGREE Conversion between radians and degrees
*/
#define RADIAN2DEGREE (180./PI)

/*!
@page Compilation_Options
@param DEGREE2RADIAN Conversion between degrees and radians 
*/
#define DEGREE2RADIAN (PI/180.)



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

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif

#ifndef INF
#define INF (numeric_limits<Real>::infinity())
#endif

#ifndef RealMAX
#define RealMAX (numeric_limits<EUVPixelType>::max())
#endif

#endif //end of Constants_H
