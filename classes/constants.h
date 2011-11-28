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
@param SSW_PATH Path to solarsoft for PixLocConvertor
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
@param AR_PROJECTION The projection to improve the aggregation. (By changing the projection it is possible to improve tha aggragation nar the limb.)
<BR> Possible values are:
 -# NO_PROJECTION	No projection will be made
 -# EQUIRECTANGULAR_PROJECTION	
 -# LAMBERT_CYLINDRICAL_PROJECTION
 -# SINUOSIDAL_PROJECTION
 -# DISTANCE_TRANSFORM	This is not a projection per se, but the exact disance along the sphere will be used in the morphological transformations
*/
#if ! defined(AR_PROJECTION)
#define AR_PROJECTION SINUOSIDAL_PROJECTION
#endif

/*!
@page Compilation_Options
@param CH_PROJECTION The projection to improve the aggregation. (By changing the projection it is possible to improve tha aggragation nar the limb.)
<BR> Possible values are:
 -# NO_PROJECTION	No projection will be made
 -# EQUIRECTANGULAR_PROJECTION	
 -# LAMBERT_CYLINDRICAL_PROJECTION
 -# SINUOSIDAL_PROJECTION
 -# DISTANCE_TRANSFORM	This is not a projection per se, but the exact disance along the sphere will be used in the morphological transformations
*/
#if ! defined(CH_PROJECTION)
#define CH_PROJECTION SINUOSIDAL_PROJECTION
#endif


/*!
@page Compilation_Options
@param AR_FRAGMENTED If want the map of AR to be fragmented set to true, if you want it to be single blobs set to false
*/

#if ! defined(AR_FRAGMENTED)
#define AR_FRAGMENTED true
#endif

/*!
@page Compilation_Options
@param CH_FRAGMENTED If want the map of CH to be fragmented set to true, if you want it to be single blobs set to false
*/

#if ! defined(CH_FRAGMENTED)
#define CH_FRAGMENTED true
#endif

/*!
@page Compilation_Options
@param AR_TRA If set to true, the threshold for removing small AR will be computed on the Raw Area, otherwise on the arae At Disk Center
*/
#if ! defined(AR_TRA)
#define AR_TRA false
#endif

/*!
@page Compilation_Options
@param CH_TRA If set to true, the threshold for removing small CH will be computed on the Raw Area, otherwise on the arae At Disk Center
*/
#if ! defined(CH_TRA)
#define CH_TRA false
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

@section ALC_corr Annulus Limb Correction
See @ref ALC.

@param ANNULUS_WIDTH Parameters for the width of an annulus in the Annulus Limb Correction
*/

#define ANNULUS_WIDTH (2.)

/*
@page Compilation_Options
@param EIT_ALC_PARAMETERS Parameters for Annulus Limb Correction of EIT images

*/

#define EIT_ALC_PARAMETERS {75, 85, 110, 112}

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

#define SWAP_ALC_PARAMETERS {70, 95, 108, 112} 

/*!
@page Compilation_Options
@param EUV_ALC_PARAMETERS Parameters for Annulus Limb Correction of EUV images

*/

#define EUV_ALC_PARAMETERS {95, 100, 105, 110} 

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

@param PI Value of PI
@param MIPI Value of PI/2
@param BIPI Value of PI*2
*/
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679
#define MIPI (PI/2.)
#define BIPI (2.*PI)

/*!
@page Compilation_Options
@section universal_constants Universal Constants
@param RADIAN2ARCSEC Conversion between radians and arcsec
*/

#define RADIAN2ARCSEC (648000./PI)

/*!
@page Compilation_Options
@section universal_constants Universal Constants
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

#define MERGEMAX 1
#define MERGECIS 2
#define MERGESUM 3
#define MERGEVINCENT 4
#define NO_PROJECTION 0 
#define EQUIRECTANGULAR_PROJECTION 1
#define LAMBERT_CYLINDRICAL_PROJECTION 2
#define SINUOSIDAL_PROJECTION 3
#define DISTANCE_TRANSFORM 4

#endif //end of Constants_H
