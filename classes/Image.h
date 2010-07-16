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

#include "fitsio.h"
#include "longnam.h"
#include "tools.h"
#include "constants.h"
#include "Coordinate.h"

template<class T>
class Image
{

	protected :
		int naxis;
		long axes[2];
		unsigned  numberPixels;
		T * pixels;
		int datatype;
		int anynull, bitpix;

	public :
		const T nullvalue;						  //nullvalue is the value of a non significatif pixel, it is set to the max. May be a problem if the picture is saturated
		
		//Constructors and destructors
		Image(const std::string& filename);
		Image(const long xAxes = 0, const long yAxes = 0);
		Image(const Image<T>& i);
		Image(const Image<T>* i);
		virtual ~Image();
		
		//Routines to read and write a fits file
		virtual int writeFitsImage(const std::string& filename);
		virtual int writeFitsImageP(fitsfile* fptr);
		virtual int readFitsImage(const std::string& filename);
		virtual int readFitsImageP(fitsfile* fptr);

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


		//Various routines to work on images
		Image<T>* zero(T value = 0);
		Image<T>* drawBox(const T color, Coordinate min, Coordinate max);
		Image<T>* drawCross(const T color, Coordinate c, const unsigned size = 5);
		Image<T>* drawContours(const unsigned width = 5);
		Image<T>* dilateDiamond(const unsigned size, const T pixelValueToDilate);
		Image<T>* erodeDiamond(const unsigned size, const T pixelValueToErode);
		Image<T>* dilateCircular(const unsigned size, const T pixelValueToDilate);
		Image<T>* erodeCircular(const unsigned size, const T pixelValueToErode);
		Image<T>* erodeCircularColor(const unsigned size, const T unsetValue);
		void diff(const Image<T> * img);
		void div(const Image<T> * img);
		unsigned propagateColor(const T color, const Coordinate& firstPixel);
		unsigned propagateColor(const T color, const unsigned firstPixel);
		unsigned colorizeConnectedComponents(const T setValue = 0);
		unsigned tresholdConnectedComponents(const unsigned minSize, const T setValue = 0);

		//returns the image with all the pixels set to nullvalue if bitMap != setValue;
		Image<T>* bitmap(const Image<unsigned>* bitMap, unsigned setValue = 1);

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

};

void fillRandomDots(Image<PixelType>* image, unsigned numberClasses, const std::vector<Real>& classesFeatures, const std::vector<Real>& backgroundFeatures);
#endif