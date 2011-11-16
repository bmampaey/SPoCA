#pragma once
#ifndef ColorMap_H
#define ColorMap_H

#include "Header.h"
#include "SunImage.h"

#ifdef MAGICK
#include "gradient.h"
#include "MagickImage.h"
#endif


class ColorMap : public SunImage<ColorType>
{
	public :
		//! Constructor
		ColorMap(const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Constructor for an ColorMap from an header
		ColorMap(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an ColorMap from a WCS
		ColorMap(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		ColorMap(const SunImage<ColorType>& i);
		
		//! Copy Constructor
		ColorMap(const SunImage<ColorType>* i);
		
		//! Destructors
		~ColorMap();
		
		//! Accessor to retrieve the interpolated value of the image in x y
		ColorType interpolate(float x, float y) const;
		
		//! Accessor to retrieve the interpolated value of the image in c
		ColorType interpolate(const RealPixLoc& c) const;
		
		//! Routine to read the sun parameters from the header
		void parseHeader();
		
		//! Routine to write the sun parameters to the header
		void fillHeader();
		
		//! Routine to draw the contours 
		ColorMap* drawContours(const unsigned width, const ColorType unsetValue );
		
		//! Routine to draw the internal contours
		ColorMap* drawInternContours(const unsigned width, const ColorType unsetValue);
		
		//! Routine to draw the external contours
		ColorMap* drawExternContours(const unsigned width, const ColorType unsetValue);
		
		//! Routine to do dilation with the shape of a diamond
		ColorMap* dilateDiamond(const unsigned size, const ColorType pixelValueToDilate);
		
		//! Routine to do erosion with the shape of a diamond
		ColorMap* erodeDiamond(const unsigned size, const ColorType pixelValueToErode);
		
		//! Routine to do dilation with the shape of a disc
		/*! Much slower than dilateDiamond */
		ColorMap* dilateCircular(const Real size, const ColorType unsetValue);
		
		//! Routine to do erosion with the shape of a disc
		/*! Much slower than erodeDiamond */
		ColorMap* erodeCircular(const Real size, const ColorType unsetValue);
		
		//! Routine to threshold regions by its raw size
		void thresholdRegionsByRawArea(const double minSize);
		
		//! Routine to threshold regions by its size at disc center
		void thresholdRegionsByRealArea(const double minSize);
		
		//! Routine that removes connected component of a size (number of pixels) smaller than minSize
		unsigned thresholdConnectedComponents(const unsigned minSize, const ColorType setValue = 0);
		
		//! Routine to propagate a color in the connected component specified by firstPixel
		unsigned propagateColor(const ColorType color, const PixLoc& firstPixel);
		
		//! Routine to propagate a color in the connected component specified by firstPixel
		unsigned propagateColor(const ColorType color, const unsigned firstPixel);
		
		//! Routine that gives a different color to each connected component
		unsigned colorizeConnectedComponents(const ColorType setValue = 0);
		
		//! Routine that tries to remove holes in connected components
		ColorMap* removeHoles(ColorType unusedColor = std::numeric_limits<ColorType>::max() - 1);
		
		//! Method to aggregates pixels into blobs by perfoming a closing
		void aggregateBlobs(const Real& aggregationFactor, const int& projection = SunImage<ColorType>::no_projection);
		
		#ifdef MAGICK
		//! Routine that creates and return a MagickImage with the specified background
		MagickImage magick(const Magick::Color background);
		//! Routine that creates and return a MagickImage with a transparent background
		MagickImage magick();
		#endif

};

bool isColorMap(const Header& header);
#endif

