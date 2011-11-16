#pragma once
#ifndef Image_H
#define Image_H

#include <iostream>
#include <limits>
#include <typeinfo>
#include <vector>
#include <cmath>
#include <string.h>
#include <stdio.h>

#include "tools.h"
#include "constants.h"
#include "Coordinate.h"
#include "FitsFile.h"

//! Class image and base class of all other image classes
/*!
Simple mono channel 2 dimensions image.

It is implemented as a single dimension array of pixels.
The pixels are ordonated from the first pixel in the lower left corner of the image until the last pixel in the uper right corner of the image, row by row.
This is similar to the way fits files store the pixels.

It implements also some common routines on images.
*/

//! @tparam T Type of a single pixel
template<class T>
class Image
{

	protected :
		//! Size of the X axes of the Image
		unsigned xAxes;
		
		//! Size of the Y axes of the Image
		unsigned yAxes;
		
		//! Number of pixels of the Image
		unsigned  numberPixels;
		
		//! Pointer to the array of pixels
		T * pixels;
		
		//! null is the value of a non significatif pixel
		/*! It is set by default to the max value of a pixel.
		May be a problem if the picture is saturated */
		T nullpixelvalue;  
		
		//! Computes the percentil value of the array arr
		T quickselect(std::vector<T>& arr, Real percentil = 0.5) const;

	public :
		//! Constructor for an Image of size xAxes x yAxes
		Image(const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		/*! Allocate memory for the pixels and copy them*/
		Image(const Image<T>& i);
		
		//! Copy Constructor
		/*! Allocate memory for the pixels and copy them*/
		Image(const Image<T>* i);
		
		//! Destructors
		/*! Deallocate the reserved memory for the pixels*/
		virtual ~Image();

		//! Accessor to retrieve the Xaxes
		unsigned Xaxes() const;
		
		//! Accessor to retrieve the Yaxes
		unsigned Yaxes() const;
		
		//! Accessor to retrieve the number of pixels
		unsigned NumberPixels() const;
		
		//! Accessor to retrieve a reference to a pixel
		T& pixel(const unsigned& j);
		
		//! Accessor to retrieve a const reference to a pixel
		const T& pixel(const unsigned& j)const;
		
		//! Accessor to retrieve a reference to a pixel
		T& pixel(const unsigned& x, const unsigned& y);
		
		//! Accessor to retrieve a const reference to a pixel
		const T& pixel(const unsigned& x, const unsigned& y)const;
		
		//! Accessor to retrieve a reference to a pixel
		T& pixel(const PixLoc& c);
		
		//! Accessor to retrieve a const reference to a pixel
		const T& pixel(const PixLoc& c)const;
		
		//! Accessor to retrieve the interpolated value of the image in x, y
		virtual T interpolate(float x, float y) const;
		
		//! Accessor to retrieve the interpolated value of the image in c
		virtual T interpolate(const RealPixLoc& c) const;
		
		//! Accessor to retrieve the coordinate of a pixel
		PixLoc coordinate(const unsigned j)const;
		
		//! Accessor to retrieve the null pixel value
		T null() const
		{return nullpixelvalue;}
		
		//! Accessor to set the null pixel value
		void setNullValue(T null)
		{nullpixelvalue = null;}
		
		//! Test if a pixel is null
		bool isNull(const unsigned& j)const
		{return pixels[j] == nullpixelvalue;}
		
		//! Test if a pixel is null
		bool isNull(const unsigned& x, const unsigned& y)const
		{return pixel(x,y)  == nullpixelvalue;}
		
		//! Test if a pixel is null
		bool isNull(const PixLoc& c)const
		{return pixel(c)  == nullpixelvalue;}

		//! Routine to resize the Image
		Image<T>* resize(const unsigned xAxes, const unsigned yAxes = 1);
		
		//! Routine to set all pixels to a certain value
		Image<T>* zero(T value = 0);
		
		//! Routine to draw a box
		Image<T>* drawBox(const T color, PixLoc min, PixLoc max);
		
		//! Routine to draw a cross
		Image<T>* drawCross(const T color, PixLoc c, const unsigned size = 5);
		
		//! Routine to draw a circle
		Image<T>* drawCircle(PixLoc center, double radius, T color);
		
		//! Routine to substract each pixel by the corresponding pixel of img
		void diff(const Image<T> * img);
		
		//! Routine to divide each pixel by the corresponding pixel of img
		void div(const Image<T> * img);
		
		//! Routine to divide each pixel by the value
		void div(const T value);
		
		//! Routine to divide each pixel by the value
		void mul(const T value);
		
		//! Routine to threshold the image betwenn min and max
		void threshold(const T min, const T max);
		
		//! Routine that set all pixels to 1 if the corresponding pixel has a value of setValue
		Image<T>* bitmap(T setValue = 1);
		
		//! Routine that set all pixels to 1 if the corresponding pixel in bitMap has a value of setValue
		Image<T>* bitmap(const Image<T>* bitMap, T setValue = 1);
		
		//! Computes the min and max of the Image
		void minmax(T& min, T& max) const;
		
		//! Computes the mean value of the Image
		Real mean() const;
		
		//! Computes the variance value of the Image
		Real variance() const;
		//! Computes the skewness of the Image
		Real skewness() const;
		//! Computes the kurtosis of the Image
		Real kurtosis() const;
		
		//! Computes the median of the Image
		T median() const;
		
		//! Computes the percentiles of the Image
		/*! Note that the algorithm perform much faster if the percentiles are in ascending order */
		std::vector<T> percentiles(const std::vector<Real>& p) const;

		//! Computes a percentile of the Image
		T percentiles(const Real& p) const;
		
		//! Computes the mode of the Image
		/*! If the binSize is not provided, it will be taken as NUMBER_BINS (See @ref Compilation_Options) in 2 times the variance of the image */
		Real mode(Real binSize = 0) const;
		
		//! Routine that Replace each pixel by the mean of its neighboors (in circle of radius Nradius)
		void localMean(const Image<T>* image, int Nradius);
		//! Routine that Replace each pixel by the variance of its neighboors (in circle of radius Nradius)
		void localVariance(const Image<T>* image, int Nradius);
		//! Routine that Replace each pixel by the skewness of its neighboors (in circle of radius Nradius)
		void localSkewness(const Image<T>* image, int Nradius);
		//! Routine that Replace each pixel by the kurtosis of its neighboors (in circle of radius Nradius)
		void localKurtosis(const Image<T>* image, int Nradius);
		
		Image<T>* convolution(const Image<T> * img, const float kernel[3][3]);
		Image<T>* sobel_approx(const Image<T> * img);
		Image<T>* sobel(const Image<T> * img);
		
		// For the optical flow
		Image<T>* horizontal_convolution(const Image<T>* img,  const std::vector<float>& kernel);
		Image<T>* vertical_convolution(const Image<T>* img,  const std::vector<float>& kernel);
		Image<T>* convolution(const Image<T>* img, const std::vector<float>& horizontal_kernel, const std::vector<float>& vertical_kernel);
		
		Image<T>* binomial_smoothing(unsigned width, const Image<T>* img = NULL);
		
		//! Routine to write the Image to a fits files
		virtual FitsFile& writeFits(FitsFile& file, int mode = 0, const std::string imagename = "");
		
		//! Routine to read an Image from fits files
		virtual FitsFile& readFits(FitsFile& file);
		
		//! Routine to write the Image to a fits files
		bool writeFits(const std::string& filename, int mode = 0, const std::string imagename = "");
		
		//! Routine to read an Image from a fits files
		bool readFits(const std::string& filename);
		

};


#endif
