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

//! \tparam T Type of a single pixel
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
		
		//! nullvalue is the value of a non significatif pixel
		/*! It is set by default to the max value of a pixel.
		May be a problem if the picture is saturated */
		T nullvalue_;  

	public :
		//! Constructor for an Image of size xAxes x yAxes
		Image(const long xAxes = 0, const long yAxes = 0);
		
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
		T& pixel(const Coordinate& c);
		
		//! Accessor to retrieve a const reference to a pixel
		const T& pixel(const Coordinate& c)const;
		
		//! Accessor to retrieve the coordinate of a pixel
		Coordinate coordinate (const unsigned j)const;
		
		//! Accessor to retrieve the nullvalue
		T nullvalue() const
		{return nullvalue_;}
		
		//! Accessor to set the nullvalue
		void setNullvalue(T nullvalue)
		{nullvalue_ = nullvalue;}

		//! Routine to resize the Image
		Image<T>* resize(const unsigned xAxes, const unsigned yAxes = 1);
		
		//! Routine to set all pixels to a certain value
		Image<T>* zero(T value = 0);
		
		//! Routine to draw a box
		Image<T>* drawBox(const T color, Coordinate min, Coordinate max);
		
		//! Routine to draw a cross
		Image<T>* drawCross(const T color, Coordinate c, const unsigned size = 5);
		
		//! Routine to draw the contours 
		Image<T>* drawContours(const unsigned width, const T unsetValue );
		
		//! Routine to draw the internal contours
		Image<T>* drawInternContours(const unsigned width, const T unsetValue);
		
		//! Routine to draw the external contours
		Image<T>* drawExternContours(const unsigned width, const T unsetValue);
		
		//! Routine to do dilation with the shape of a diamond
		Image<T>* dilateDiamond(const unsigned size, const T pixelValueToDilate);
		
		//! Routine to do erosion with the shape of a diamond
		Image<T>* erodeDiamond(const unsigned size, const T pixelValueToErode);
		
		//! Routine to do dilation with the shape of a disc
		/*! Much slower than dilateDiamond */
		Image<T>* dilateCircular(const unsigned size, const T unsetValue);
		
		//! Routine to do erosion with the shape of a disc
		/*! Much slower than erodeDiamond */
		Image<T>* erodeCircular(const unsigned size, const T unsetValue);
		
		//! Routine to substract each pixel by the corresponding pixel of img
		void diff(const Image<T> * img);
		
		//! Routine to divide each pixel by the corresponding pixel of img
		void div(const Image<T> * img);
		
		//! Routine to propagate a color in the connected component specified by firstPixel
		unsigned propagateColor(const T color, const Coordinate& firstPixel);
		
		//! Routine to propagate a color in the connected component specified by firstPixel
		unsigned propagateColor(const T color, const unsigned firstPixel);
		
		//! Routine that gives a different color to each connected component
		unsigned colorizeConnectedComponents(const T setValue = 0);
		
		//! Routine that removes connected component of a size smaller than minSize
		unsigned tresholdConnectedComponents(const unsigned minSize, const T setValue = 0);
		
		//! Routine that set all pixels to 1 if the corresponding pixel in bitMap has a value of setValue
		Image<T>* bitmap(const Image<T>* bitMap, T setValue = 1);
		
		//! Routine that tries to remove holes in connected components
		Image<T>* removeHoles(T unusedColor = std::numeric_limits<T>::max() - 1);
		
		//! Routine that generate a chaincode for the connected component indicated by firstPixel
		std::vector<Coordinate> chainCode(const Coordinate firstPixel, const unsigned max_points) const;
		
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
		
		//! Routine that Replace each pixel by the mean of its neighboors (in circle of radius Nradius)
		void neighboorhoodMean(const Image<T>* image, int Nradius);
		//! Routine that Replace each pixel by the variance of its neighboors (in circle of radius Nradius)
		void neighboorhoodVariance(const Image<T>* image, int Nradius);
		//! Routine that Replace each pixel by the skewness of its neighboors (in circle of radius Nradius)
		void neighboorhoodSkewness(const Image<T>* image, int Nradius);
		//! Routine that Replace each pixel by the kurtosis of its neighboors (in circle of radius Nradius)
		void neighboorhoodKurtosis(const Image<T>* image, int Nradius);
		
		Image<T>* convolution(const Image<T> * img, const float kernel[3][3]);
		Image<T>* convolution(const Image<T> * img, const float kernel[5][5]);
		Image<T>* sobel_approx(const Image<T> * img);
		Image<T>* sobel(const Image<T> * img);
		
		// For the optical flow
		Image<T>* convolveHoriz(const Image<T>* img,  const std::vector<float>& kernel);
		Image<T>* convolveVert(const Image<T>* img,  const std::vector<float>& kernel);
		Image<T>* convolveSeparate(const Image<T>* img, const std::vector<float>& horiz_kernel, const std::vector<float>& vert_kernel);
		T interpolate(const float x, const float y) const;
		
		//! Routine to write the Image to a fits files
		virtual FitsFile& writeFits(FitsFile& file, int mode = 0);
		
		//! Routine to read an Image from fits files
		virtual FitsFile& readFits(FitsFile& file);
		
		//! Routine to write the Image to a fits files
		bool writeFits(const std::string& filename, int mode = 0);
		
		//! Routine to read an Image from a fits files
		bool readFits(const std::string& filename);

};

void fillRandomDots(Image<PixelType>* image, unsigned numberClasses, const std::vector<Real>& classesFeatures, const std::vector<Real>& backgroundFeatures);
#endif
