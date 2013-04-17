#include "MagickImage.h"

using namespace std;

using Magick::Color;
using Magick::ColorGray;
using Magick::ColorRGB;
using Magick::Geometry;
using Magick::Quantum;
using Magick::StorageType;


StorageType MagickImage::magickDataType(const type_info& t)
{
	StorageType datatype = MagickCore::UndefinedPixel;
	if(t == typeid(double))
		datatype = Magick::DoublePixel;
	else if(t == typeid(float))
		datatype = Magick::FloatPixel;
	else if(t == typeid(long))
		datatype = Magick::IntegerPixel;
	else if(t == typeid(unsigned long))
		datatype = Magick::IntegerPixel;
	else if(t == typeid(short))
		datatype = Magick::ShortPixel;
	else if(t == typeid(unsigned short))
		datatype = Magick::ShortPixel;
	else if(t == typeid(int))
		datatype = Magick::IntegerPixel;
	else if(t == typeid(unsigned int))
		datatype = Magick::IntegerPixel;
	else if(t == typeid(char))
		datatype = Magick::CharPixel;
	else if(t == typeid(signed char))
		datatype = Magick::CharPixel;
	else
	{
		cerr<<"Error: unknown magick data type for type "<<t.name()<<endl;
	}
	return datatype;
}

MagickImage::MagickImage()
:Magick::Image()
{
	MagickCore::SetImageColorspace( this->image(), MagickCore::RGBColorspace );
	depth(8);
}

MagickImage::MagickImage(const string& filename)
:Magick::Image(filename)
{
	MagickCore::SetImageColorspace( this->image(), MagickCore::RGBColorspace );
	depth(8);
}

template<class T>
MagickImage::MagickImage(T* image, const unsigned X, const unsigned Y, const string channel)
:Magick::Image(X, Y, channel, magickDataType(typeid(T)), image) 
{
	MagickCore::SetImageColorspace( this->image(), MagickCore::RGBColorspace );
	depth(8);
}

MagickImage::MagickImage(const Color& background, const unsigned X, const unsigned Y)
:Magick::Image(Geometry(X, Y), background)
{
	MagickCore::SetImageColorspace( this->image(), MagickCore::RGBColorspace );
	depth(8);
}

template MagickImage::MagickImage(ColorType* image, const unsigned X, const unsigned Y, const string channel);
template MagickImage::MagickImage(EUVPixelType* image, const unsigned X, const unsigned Y, const string channel);
template MagickImage::MagickImage(char* image, const unsigned X, const unsigned Y, const string channel);





