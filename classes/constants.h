#pragma once
#ifndef Constants_H
#define Constants_H


/* Definition of constants*/
/* Modify the following to adapt to your program */

// The precision you want for the float numbers
// Acceptable values are TFLOAT (fast) TDOUBLE (more precise)
#if ! defined(REAL)
#define REAL float
#endif

// The type of the pixel values of the images. 
// Be carrefull that depending on the type of the image you may loose precision.
#if ! defined(PIXELTYPE)
#define PIXELTYPE float
#endif

//Cfitsio defines additional unsigned type that are not part of the strict FITS standart data format
// If you want to read and manipulate Fits files with software not written with the cfitsio (e.g. ImageMagick and IDL), you should set STRICTFITS
#ifndef STRICTFITS
#define STRICTFITS
#endif

// The number of wavelength, or images to process in parralel
#if ! defined(NUMBERWAVELENGTH)
#define NUMBERWAVELENGTH 2
#endif

// The quantity of output to screen and files, and verification to be done
// 1 is for verification and output of the results only
// 2 is 1 + lots of output to files
// 3 is 2 + lots of output to screen
#if ! defined(DEBUG)
#define DEBUG 0
#endif

// The minimal size for an AR in (arc/sec)square (otherwise it is a bright point)
#if ! defined(MIN_AR_SIZE)
#define MIN_AR_SIZE 1500
#endif

// The minimal size for an CH in (arc/sec)square (otherwise it is a dark point)
#if ! defined(MIN_CH_SIZE)
#define MIN_CH_SIZE 1500
#endif

// The aggregation factor for AR in (arc/sec)square (dilation)
#if ! defined(AR_AGGREGATION)
#define AR_AGGREGATION 32 // Equivalent to 12 EIT pixels
#endif

// The aggregation factor for CH in (arc/sec)square (dilation)
#if ! defined(CH_AGGREGATION)
#define CH_AGGREGATION 150 // Equivalent to 60 EIT pixels
#endif

// If the computaion of eta is to be fixed or not
// Acceptables values are TRUE or FALSE
#define FIXETA FALSE

// The maximum deviation in the computation of eta
#define ETA_MAXFACTOR 100

// The reduction factor in the computation of eta2
#define ETA2_REDUCTION_FACTOR 95

// Type of merge function to use
// Possible values are MERGEMAX , MERGECIS
#define MERGE MERGECIS

// alpha value of Vincent's merge function

#define MERGEVINCENT_ALPHA 20./255.

// Limits for the limb correction

// For instrument EIT
#define EIT_DISCRETE_CORR_R1  95
#define EIT_SLOPE_CORR_R1 90
#define EIT_SLOPE_CORR_R2 95
#define EIT_SINE_CORR_R1 90
#define EIT_SINE_CORR_R2 95
#define EIT_SINE_CORR_R3 105
#define EIT_SINE_CORR_R4 110


// For instrument EUVI
#define EUVI_DISCRETE_CORR_R1  95
#define EUVI_SLOPE_CORR_R1 90
#define EUVI_SLOPE_CORR_R2 95
#define EUVI_SINE_CORR_R1 95
#define EUVI_SINE_CORR_R2 100
#define EUVI_SINE_CORR_R3 105
#define EUVI_SINE_CORR_R4 110

// For instrument AIA
#define AIA_DISCRETE_CORR_R1  95
#define AIA_SLOPE_CORR_R1 90
#define AIA_SLOPE_CORR_R2 95
#define AIA_SINE_CORR_R1 80
#define AIA_SINE_CORR_R2 102
#define AIA_SINE_CORR_R3 105
#define AIA_SINE_CORR_R4 112

// For instrument SWAP
#define SWAP_DISCRETE_CORR_R1  95
#define SWAP_SLOPE_CORR_R1 90
#define SWAP_SLOPE_CORR_R2 95
#define SWAP_SINE_CORR_R1 95
#define SWAP_SINE_CORR_R2 100
#define SWAP_SINE_CORR_R3 105
#define SWAP_SINE_CORR_R4 115

// For a default instrument
#define DISCRETE_CORR_R1  95
#define SLOPE_CORR_R1 90
#define SLOPE_CORR_R2 95
#define SINE_CORR_R1 95
#define SINE_CORR_R2 100
#define SINE_CORR_R3 105
#define SINE_CORR_R4 115

// Path of solarsoft for CoordinateConvertor

#define SSW_PATH "/usr/local/ssw/"
#define WCS_ROUTINES_SAV "./idl/wcs_routines.sav"

// The minimum quotient factor 
#define MIN_QUOTIENT_FACTOR 5

// The maximum quotient factor 
#define MAX_QUOTIENT_FACTOR 10

// Parameters to compute the area at disk center
//The radius of the sun in Mmeters (R0)
#define SUNRADIUS 695.508
//Something. CIS, what is it ?
#define DR0 0.026
//Something else.
#define DR 2.
//The higgins_factor
#define HIGGINS_FACTOR  16

// Parameters for the region stats
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



#define PI 3.14159265358979323846
#define MIPI 1.57079632679489661923
#define BIPI 6.28318530717958647692

#endif
