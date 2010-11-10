#pragma once
#ifndef Constants_H
#define Constants_H

#include "fitsio.h"

/* Definition of constants*/
/* Modify the following to adapt to your program */

// The precision you want for the float numbers
// Acceptable values are TFLOAT (fast) TDOUBLE (more precise)
#if ! defined(REALTYPE)
#define REALTYPE TDOUBLE
#endif

// The type of the pixel values of the images. 
// Acceptable values are TBYTE TSBYTE TUSHORT TSHORT TUINT TINT TULONG TLONG TFLOAT TLONGLONG TDOUBLE
// Be carrefull that depending on the type of the image you may loose precision.
#if ! defined(PIXELTYPE)
#define PIXELTYPE TFLOAT
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
#define AR_AGGREGATION 31.44 // Equivalent to 12 EIT pixels
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
#define MAX_QUOTIENT_FACTOR 13


/*---------------- Do NOT modify below please ------------------*/


#if ! defined(REALTYPE)
#warning "REALTYPE not defined, using default type double"
#define REALTYPE TDOUBLE
#endif

#if REALTYPE==TFLOAT
#define Real float
#else
#define Real double
#endif



#if ! defined(PIXELTYPE)
#warning "PIXELTYPE not defined, using default TDOUBLE"
#define PIXELTYPE TDOUBLE
#endif

#if PIXELTYPE==TBYTE
#define PixelType signed char
#elif PIXELTYPE==TSBYTE
#define PixelType unsigned char
#elif PIXELTYPE==TSHORT
#define PixelType short
#elif PIXELTYPE==TUSHORT
#define PixelType unsigned short
#elif PIXELTYPE==TINT
#define PixelType int
#elif PIXELTYPE==TUINT
#define PixelType unsigned int
#elif PIXELTYPE==TLONG
#define PixelType long
#elif PIXELTYPE==TULONG
#define PixelType unsigned long
#elif PIXELTYPE==TLONGLONG
#define PixelType long long
#elif PIXELTYPE==TFLOAT
#define PixelType float
#elif PIXELTYPE==TDOUBLE
#define PixelType double
#endif


#endif
