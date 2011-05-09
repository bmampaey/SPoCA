#pragma once
#ifndef MagickImage_H
#define MagickImage_H

#include <math.h>
#include <typeinfo>
#include <string>
#include <Magick++.h>
#include <iostream>

#include "constants.h"

//! Class that facilitate the usage of ImageMagick Magick++ Library
/*!
Wrapper around the class Magick::Image. It allows to easily write the Images as png files fro example.
All of the method of that subclass are available. See <a href="http://www.imagemagick.org/Magick++/Image.html#Image%20Manipulation%20Methods" target="_blank">Image Manipulation Methods</a> for the list.
*/




class MagickImage : public Magick::Image
{
	protected :
		//! Routine to convert a C++ data type to a magick datatype 
		Magick::StorageType magickDataType(const std::type_info& t);

	public :
	
		//! Constructor
		MagickImage();
		
		//! Constructor
		MagickImage(const std::string& filename);
		
		//! Constructor
		template<class T>
		MagickImage(T* image, const unsigned X, const unsigned Y, const std::string channel = "I");
		
		//! Constructor
		MagickImage(const Magick::Color& background, const unsigned X = 0, const unsigned Y = 0);

};

#endif



