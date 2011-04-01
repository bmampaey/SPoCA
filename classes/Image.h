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


template<class T>
class Image
{

	protected :
		unsigned xAxes, yAxes;
		unsigned  numberPixels;
		T * pixels;
		T nullvalue_;  //nullvalue is the value of a non significatif pixel, it is set to the max. May be a problem if the picture is saturated

	public :
		//Constructors and destructors
		Image(const long xAxes = 0, const long yAxes = 0);
		Image(const Image<T>& i);
		Image(const Image<T>* i);
		virtual ~Image();

		//Accessors
		unsigned Xaxes() const;
		unsigned Yaxes() const;
		unsigned NumberPixels() const;
		T& pixel(const unsigned& j);
		const T& pixel(const unsigned& j)const;
		T& pixel(const unsigned& x, const unsigned& y);
		const T& pixel(const unsigned& x, const unsigned& y)const;
		T& pixel(const Coordinate& c);
		const T& pixel(const Coordinate& c)const;
		Coordinate coordinate (const unsigned j)const;
		T nullvalue() const
		{return nullvalue_;}
		void setNullvalue(T nullvalue)
		{nullvalue_ = nullvalue;}


		//Various routines to work on images
		Image<T>* resize(const unsigned xAxes, const unsigned yAxes = 1);
		Image<T>* zero(T value = 0);
		Image<T>* drawBox(const T color, Coordinate min, Coordinate max);
		Image<T>* drawCross(const T color, Coordinate c, const unsigned size = 5);
		Image<T>* drawContours(const unsigned width, const T unsetValue );
		Image<T>* drawInternContours(const unsigned width, const T unsetValue);
		Image<T>* drawExternContours(const unsigned width, const T unsetValue);
		Image<T>* dilateDiamond(const unsigned size, const T pixelValueToDilate);
		Image<T>* erodeDiamond(const unsigned size, const T pixelValueToErode);
		Image<T>* dilateCircular(const unsigned size, const T unsetValue);
		Image<T>* erodeCircular(const unsigned size, const T unsetValue);
		void diff(const Image<T> * img);
		void div(const Image<T> * img);
		unsigned propagateColor(const T color, const Coordinate& firstPixel);
		unsigned propagateColor(const T color, const unsigned firstPixel);
		unsigned colorizeConnectedComponents(const T setValue = 0);
		unsigned tresholdConnectedComponents(const unsigned minSize, const T setValue = 0);
		Image<T>* bitmap(const Image<T>* bitMap, T setValue = 1);
		Image<T>* removeHoles(T unusedColor = std::numeric_limits<T>::max() - 1);
		std::vector<Coordinate> chainCode(const Coordinate firstPixel, const unsigned max_points) const;
		
		//Return the mean value of the image
		Real mean() const;
		//Return the variance value of the image
		Real variance() const;
		//Return the skewness of the image
		Real skewness() const;
		//Return the kurtosis of the image
		Real kurtosis() const;
		//Replace each pixel by the mean of its neighboors (in circle of radius Nradius)
		void neighboorhoodMean(const Image<T>* image, int Nradius);
		//Replace each pixel by the variance of its neighboors (in circle of radius Nradius)
		void neighboorhoodVariance(const Image<T>* image, int Nradius);
		//Replace each pixel by the skewness of its neighboors (in circle of radius Nradius)
		void neighboorhoodSkewness(const Image<T>* image, int Nradius);
		//Replace each pixel by the kurtosis of its neighboors (in circle of radius Nradius)
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
		
		//Routines to read/write fits files
		virtual FitsFile& writeFits(FitsFile& file, int mode = 0);
		virtual FitsFile& readFits(FitsFile& file);
		bool writeFits(const std::string& filename, int mode = 0);
		bool readFits(const std::string& filename);

};

void fillRandomDots(Image<PixelType>* image, unsigned numberClasses, const std::vector<Real>& classesFeatures, const std::vector<Real>& backgroundFeatures);
#endif
