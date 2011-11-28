#include "ColorMap.h"
#include <assert.h>
#include <map>
#include <deque>

extern std::string filenamePrefix;

using namespace std;

ColorMap::~ColorMap()
{}

ColorMap::ColorMap(const unsigned& xAxes, const unsigned& yAxes)
:SunImage<ColorType>(xAxes, yAxes)
{
	nullpixelvalue = 0;

}

ColorMap::ColorMap(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:SunImage<ColorType>(header, xAxes, yAxes)
{
	nullpixelvalue = 0;
}

ColorMap::ColorMap(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:SunImage<ColorType>(wcs, xAxes, yAxes)
{
	nullpixelvalue = 0;
}

ColorMap::ColorMap(const SunImage<ColorType>& i)
:SunImage<ColorType>(i)
{
	nullpixelvalue = 0;

}

ColorMap::ColorMap(const SunImage<ColorType>* i)
:SunImage<ColorType>(i)
{
	nullpixelvalue = 0;
}

inline ColorType ColorMap::interpolate(float x, float y) const
{
	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;
	if(x > xAxes-1.)
		x = xAxes-1.;
	if(y > yAxes-1.)
		y = yAxes-1.;
	
	unsigned ix = unsigned(x), iy = unsigned(y);
	
	float dx = x - ix, dy = y - iy;
	Real cdx = 1. - dx, cdy = 1. - dy;
	ColorType colors[] = {pixel(ix, iy), pixel(ix+1, iy), pixel(ix, iy+1), pixel(ix+1, iy+1)};
	float quantity[] = {cdx*cdy, dx*cdy, cdx*dy, dx*dy};
	for(unsigned i = 0; i < 3; ++i)
		for(unsigned j = i+1; j < 4; ++j)
			if(colors[i] == colors[j])
			{
				quantity[i] += quantity[j];
				quantity[j] = 0;
			}
	unsigned max = 0;
	for(unsigned i = 1; i < 4; ++i)
		max = quantity[i] > quantity[max] ? i : max;
	
	return colors[max];
}

inline ColorType ColorMap::interpolate(const RealPixLoc& c) const
{
	return interpolate(c.x, c.y);
}

void ColorMap::parseHeader()
{
	// We parse the header to extract the wcs coordinate system
	if(header.has("CRPIX1") and header.has("CRPIX2"))
		wcs.setSunCenter(header.get<Real>("CRPIX1") - 1, header.get<Real>("CRPIX2") - 1);
	else
		cerr<<"Error: Fits header not conform: No CRPIX1 or CRPIX2 keyword."<<endl;
	
	if (header.has("T_OBS"))
		wcs.setDateObs(header.get<string>("T_OBS"));
	else if (header.has("DATE_OBS"))
		wcs.setDateObs(header.get<string>("DATE_OBS"));
	else if (header.has("DATE-OBS"))
		wcs.setDateObs(header.get<string>("DATE-OBS"));
	
	if(header.has("CDELT1") and header.has("CDELT2"))
		wcs.setCDelt(header.get<Real>("CDELT1"), header.get<Real>("CDELT2"));
	else
		cerr<<"Error: Fits header not conform: No CDELT1 or CDELT2 keyword."<<endl;
	
	if (header.has("HGLT_OBS"))
		wcs.setB0(header.get<Real>("HGLT_OBS"));
	else if (header.has("CRLT_OBS"))
		wcs.setB0(header.get<Real>("CRLT_OBS"));
	else if(header.has("SOLAR_B0"))
		wcs.setB0(header.get<Real>("SOLAR_B0"));
	else if (wcs.time_obs != 0)
		wcs.setB0(earth_latitude(wcs.time_obs));
	
	if (header.has("HGLN_OBS"))
		wcs.setL0(header.get<Real>("HGLN_OBS"));
	else
		wcs.setL0(0);
	
	if (header.has("CRLN_OBS"))
		wcs.setCarringtonL0(header.get<Real>("CRLN_OBS"));
	
	if (header.has("DSUN_OBS"))
		wcs.setDistanceSunObs(header.get<double>("DSUN_OBS")/1000000.);
	else if (wcs.time_obs != 0)
		wcs.setDistanceSunObs(distance_sun_earth(wcs.time_obs));
	
	if (header.has("CD1_1") and header.has("CD1_2") and header.has("CD2_1") and header.has("CD2_2"))
	{
		wcs.setCD(header.get<Real>("CD1_1"), header.get<Real>("CD1_2"), header.get<Real>("CD2_1"), header.get<Real>("CD2_2"));
	}
	else if (header.has("PC1_1") and header.has("PC1_2") and header.has("PC2_1") and header.has("PC2_2"))
	{
		wcs.setPC(header.get<Real>("PC1_1"), header.get<Real>("PC1_2"), header.get<Real>("PC2_1"), header.get<Real>("PC2_2"));
	}
	else if (header.has("CROTA2"))
	{
		wcs.setCrota2(header.get<Real>("CROTA2"));
	}
	
	// We read the radius
	if (header.has("R_SUN"))
		wcs.setSunradius(header.get<Real>("R_SUN"));
}

void ColorMap::fillHeader()
{
	header.set<Real>("CRPIX1", wcs.sun_center.x + 1);
	header.set<Real>("CRPIX2", wcs.sun_center.y + 1);
	header.set<Real>("CDELT1", wcs.cdelt1);
	header.set<Real>("CDELT2", wcs.cdelt2);
	header.set<Real>("HGLT_OBS", wcs.b0 * RADIAN2DEGREE);
	header.set<Real>("HGLN_OBS", wcs.l0 * RADIAN2DEGREE);
	header.set<Real>("CRLN_OBS", wcs.carrington_l0 * RADIAN2DEGREE);
	header.set<Real>("DSUN_OBS", wcs.dsun_obs*1000000.);
	header.set<Real>("CD1_1", wcs.cd[0][0]);
	header.set<Real>("CD1_2", wcs.cd[0][1]);
	header.set<Real>("CD2_1", wcs.cd[1][0]);
	header.set<Real>("CD2_2", wcs.cd[1][1]);
	header.set<string>("DATE_OBS",  wcs.date_obs);
	header.set<Real>("R_SUN", wcs.sun_radius);
}


bool isColorMap(const Header& header)
{
	return header.has("INSTRUME") && header.get<string>("INSTRUME").find("SPoCA") != string::npos;	
}



void ColorMap::thresholdRegionsByRawArea(const double minSize)
{
	const double pixelarea = PixelArea();
	
	//First we compute the area for each color
	map<ColorType,double> areas;
	ColorType* p = pixels;
	
	for (unsigned y = 0; y < yAxes; ++y)
	{
		for (unsigned x = 0 ; x < xAxes; ++x)
		{
			if(*p != nullpixelvalue)
			{
				if (areas.count(*p) == 0)
					areas[*p] = pixelarea;
				else
					areas[*p] += pixelarea;
			}
			++p;
		}
	}
	
	//Now we nullify those that are too small
	p = pixels;
	
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(*p != nullpixelvalue && areas[*p] < minSize)
		{
			*p = nullpixelvalue;
		}
		++p;
	}

}

void ColorMap::thresholdRegionsByRealArea(const double minSize)
{
	const Real R0 = wcs.sun_radius * PixelArea();
	const Real R2 =  wcs.sun_radius *  wcs.sun_radius;
	
	//First we compute the area for each color
	map<ColorType,Real> areas;

	ColorType* p = pixels;
	
	const Real xmax = Real(xAxes) - wcs.sun_center.x;
	const Real ymax = Real(yAxes) - wcs.sun_center.y;
	
	for (Real y = - wcs.sun_center.y; y < ymax; ++y)
	{
		for (Real x = - wcs.sun_center.x ; x < xmax; ++x)
		{
			if(*p != nullpixelvalue)
			{
				if (areas.count(*p) == 0)
					areas[*p] = 0;
				
				Real pixelArea2 = R2 - (x * x) - (y * y);
				if(pixelArea2 > 0)
					areas[*p] += R0 / sqrt(pixelArea2);
				else
					areas[*p] = numeric_limits<Real>::infinity();
			}
			++p;
		}
	}
	
	//Now we nullify those that are too small
	p = pixels;
	
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(*p != nullpixelvalue && areas[*p] < minSize)
		{
			*p = nullpixelvalue;
		}
		++p;
	}

}


ColorMap* ColorMap::dilateDiamond(unsigned size, ColorType pixelValueToDilate)
{

	unsigned *manthanDistance = new unsigned[xAxes * yAxes];
	unsigned maxDistance = xAxes + yAxes;

	for (unsigned y=0; y < yAxes; ++y)
	{
		for (unsigned x=0; x < xAxes; ++x)
		{
			if (pixel(x,y) == pixelValueToDilate)
			{

				manthanDistance[x+y*xAxes] = 0;
			}
			else
			{

				manthanDistance[x+y*xAxes] = maxDistance;

				if (x>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x-1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x-1+y*xAxes]+1);

				if (y>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y-1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y-1)*xAxes]+1);
			}
		}
	}

	for (unsigned y=yAxes; y >0; )
	{
		--y;
		for (unsigned x=xAxes; x >0; )
		{

			--x;
			if (x+1<xAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+1+y*xAxes]+1);

			if (y+1<yAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y+1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y+1)*xAxes]+1);

		}
	}

	for (unsigned y=0; y < yAxes; ++y)
		for (unsigned x=0; x < xAxes; ++x)
			if(manthanDistance[x+y*xAxes] <= size) pixel(x,y) = pixelValueToDilate;

	delete[] manthanDistance;
	return this;

}



ColorMap* ColorMap::erodeDiamond(unsigned size, ColorType pixelValueToErode)
{

	ColorType fillPixelValue = nullpixelvalue;
	unsigned *manthanDistance = new unsigned[xAxes * yAxes];
	unsigned maxDistance = xAxes + yAxes;

	for (unsigned y=0; y < yAxes; ++y)
	{
		for (unsigned x=0; x < xAxes; ++x)
		{
			if (pixel(x,y) != pixelValueToErode)
			{

				manthanDistance[x+y*xAxes] = 0;
			}
			else
			{

				manthanDistance[x+y*xAxes] = maxDistance;

				if (x>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x-1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x-1+y*xAxes]+1);

				if (y>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y-1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y-1)*xAxes]+1);
			}
		}
	}

	for (unsigned y=yAxes; y >0; )
	{
		--y;
		for (unsigned x=xAxes; x >0;)
		{
			--x;
			if (x+1<xAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+1+y*xAxes]+1);

			if (y+1<xAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y+1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y+1)*xAxes]+1);

		}
	}

	for (unsigned y=0; y < yAxes; ++y)
		for (unsigned x=0; x < xAxes; ++x)
			pixel(x,y) = manthanDistance[x+y*xAxes] <= size? fillPixelValue : pixelValueToErode;

	delete[] manthanDistance;
	return this;

}



ColorMap* ColorMap::dilateCircular(const Real size, const ColorType unsetValue)
{
	ColorType * newPixels = new ColorType[numberPixels];
	memcpy(newPixels, pixels, numberPixels * sizeof(ColorType));
	vector<int> shape;
	shape.reserve(unsigned(size*size*3));
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -int(size); x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y * y) <= size)
				shape.push_back(y * xAxes + x);
	
	int offset = xAxes + 1;
	ColorType * j = pixels + offset;
	ColorType * nj = newPixels + offset;
	for(unsigned y = 1; y < yAxes - 1; ++y)
	{
		for(unsigned x = 1; x < xAxes - 1; ++x)
		{
			if(*j != unsetValue)
			{
				if(*(j-1) == unsetValue || *(j+1) == unsetValue || *(j-xAxes) == unsetValue || *(j+xAxes) == unsetValue)
				{
					for(unsigned s = 0; s < shape.size(); ++s)
					{
						if(offset + shape[s] < int(numberPixels))
							*(nj + shape[s]) = *j;
						if(shape[s] <= offset)
							*(nj - shape[s]) = *j;
					}
				}
			}
			++j;
			++nj;
			++offset;
		}
		j+=2;
		nj+=2;
		offset+=2;
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}



ColorMap* ColorMap::erodeCircular(const Real size, const ColorType unsetValue)
{
	ColorType * newPixels = new ColorType[numberPixels];
	memcpy(newPixels, pixels, numberPixels * sizeof(ColorType));
	vector<int> shape;
	shape.reserve(unsigned(size*size*3));
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -int(size); x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y *y) <= size)
				shape.push_back(y * xAxes + x);
	
	int offset = xAxes + 1;
	ColorType * j = pixels + offset;
	ColorType * nj = newPixels + offset;
	for(unsigned y = 1; y < yAxes - 1; ++y)
	{		
		for(unsigned x = 1; x < xAxes - 1; ++x)
		{
			if(*j != unsetValue && (*(j-1) != *j || *(j+1) != *j || *(j-xAxes) != *j || *(j+xAxes) != *j))
			{
				*nj = unsetValue;
				for(unsigned s = 0; s < shape.size(); ++s)
				{
					if(offset + shape[s] < int(numberPixels))
						*(nj + shape[s]) = unsetValue;
					if(shape[s] <= offset)
						*(nj - shape[s]) = unsetValue;
				}
				
			}
			++offset;
			++j;
			++nj;
		}
		j+=2;
		nj+=2;
		offset+=2;
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}


ColorMap* ColorMap::drawInternContours(const unsigned width, const ColorType unsetValue)
{

	ColorMap* eroded = new ColorMap(this);
	eroded->erodeCircular(width, unsetValue);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(eroded->pixels[j] != eroded->nullpixelvalue)
			pixels[j] = unsetValue;
	}
	delete eroded;
	return this;

}


ColorMap* ColorMap::drawExternContours(const unsigned width, const ColorType unsetValue)
{

	ColorMap * copy = new ColorMap (this);
	this->dilateCircular(width, unsetValue);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == copy->pixels[j])
			pixels[j] = unsetValue;
	}
	delete copy;
	return this;

}


ColorMap* ColorMap::drawContours(const unsigned width, const ColorType unsetValue)
{
	unsigned size = width/2;
	ColorType * newPixels = new ColorType[numberPixels];
	memcpy(newPixels, pixels, numberPixels * sizeof(ColorType));
	vector<unsigned> shape;
	shape.reserve(size*size*3);
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -size; x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y *y) <= size)
				shape.push_back(y * xAxes + x);
	
				
	int j;
	for(unsigned y = size; y < yAxes - size; ++y)
	{		
		j = 	y * xAxes + size;
		for(unsigned x = size; x < xAxes - size; ++x)
		{
			ColorType maxColor = pixels[j-1];
			maxColor = pixels[j+1] > maxColor ? pixels[j+1] : maxColor;
			maxColor = pixels[j-xAxes] > maxColor ? pixels[j-xAxes] : maxColor;
			maxColor = pixels[j+xAxes] > maxColor ? pixels[j+xAxes] : maxColor;
			if(pixels[j] != maxColor)
			{
				newPixels[j] = maxColor;
				for(unsigned s = 0; s < shape.size(); ++s)
				{
					#if DEBUG >= 1
						if(j + shape[s] >= numberPixels || j - shape[s] < 0)
						{
							cerr<<"Error : trying to access pixel out of image in drawContours"<<endl;
							exit(EXIT_FAILURE);
						}	
					#endif
					newPixels[j + shape[s]] = newPixels[j - shape[s]] = maxColor;
				}
				
			}
			else
			{
				newPixels[j] = unsetValue;
			}
			++j;
		}
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}


unsigned ColorMap::colorizeConnectedComponents(const ColorType setValue)
{
	ColorType color = setValue;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == setValue)
		{
			++color;
			propagateColor(color, j);

		}
	}

	return unsigned(color - setValue);

}



unsigned ColorMap::propagateColor(const ColorType color, const PixLoc& firstPixel)
{
	return propagateColor(color, firstPixel.x + firstPixel.y * xAxes);
}



unsigned ColorMap::propagateColor(const ColorType color, const unsigned firstPixel)
{
	deque<unsigned> pixelList;
	ColorType setValue = pixels[firstPixel];
	unsigned h;
	unsigned numberColoredPixels = 0;

	pixelList.push_back(firstPixel);
	while ( ! pixelList.empty())
	{
		h = pixelList.back();
		pixelList.pop_back();
		if(pixels[h] != setValue)
			continue;
		pixels[h] = color;
		++numberColoredPixels;
		if(h+1 < numberPixels && pixels[h+1] == setValue)
			pixelList.push_back(h+1);
		if(h+xAxes < numberPixels && pixels[h+xAxes] == setValue)
			pixelList.push_back(h+xAxes);
		if(h >= 1 && pixels[h-1] == setValue)
			pixelList.push_back(h-1);
		if(h >= xAxes && pixels[h-xAxes] == setValue)
			pixelList.push_back(h-xAxes);

	}
	return numberColoredPixels;
}



unsigned ColorMap::thresholdConnectedComponents(const unsigned minSize, const ColorType setValue)
{
	deque<unsigned> treatedPixels;
	ColorType color = setValue + 1;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == setValue)
		{
			if (propagateColor(color, j) < minSize)
				propagateColor(nullpixelvalue, j);
			else
			{
				++color;
				treatedPixels.push_back(j);
			}
		}
	}
	//We have to give back the original color
	for (unsigned t = 0; t < treatedPixels.size(); ++t)
	{
		propagateColor(setValue, treatedPixels[t]);
	}

	return unsigned(color - 1 - setValue);
}


ColorMap* ColorMap::removeHoles(ColorType unusedColor)
{
	propagateColor(unusedColor, 0);
	ColorType lastColor = nullpixelvalue;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
		{
			lastColor = pixels[j];
		}
		else
		{
			pixels[j] = lastColor;
		}
	}
	propagateColor(nullpixelvalue, 0);
	return this;
}



//! Method to aggregates pixels into blobs by perfoming a closing
void ColorMap::aggregateBlobs(const Real& aggregationFactor, const int& projection)
{
	ColorMap* projeted = NULL;
	
	// We do the dilation
	switch(projection)
	{
		case(SunImage<ColorType>::equirectangular):
			projeted = new ColorMap(getWCS(), Xaxes(), Yaxes());
			projeted->equirectangular_projection(this, false);
			#if DEBUG >= 2
			projeted->writeFits(filenamePrefix + "equirectangular_projection.fits");
			#endif
			projeted->dilateCircular((2./3.)*aggregationFactor, null());
			#if DEBUG >= 2
			projeted->writeFits(filenamePrefix + "dilated.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			projeted = new ColorMap(getWCS(), Xaxes(), Yaxes());
			projeted->Lambert_cylindrical_projection(this, false);
			#if DEBUG >= 2
			projeted->writeFits(filenamePrefix + "Lambert_cylindrical_projection.fits");
			#endif
			projeted->dilateCircular((2./3.)*aggregationFactor, null());
			#if DEBUG >= 2
			projeted->writeFits(filenamePrefix + "dilated.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::sinuosidal):
			projeted = new ColorMap(getWCS(), Xaxes(), Yaxes());
			projeted->sinuosidal_projection(this, false);
			#if DEBUG >= 2
			projeted->writeFits(filenamePrefix + "sinuosidal_projection.fits");
			#endif
			projeted->dilateCircular((2./3.)*aggregationFactor, null());
			#if DEBUG >= 2
			projeted->writeFits(filenamePrefix + "dilated.fits");
			#endif
		break;
		
		case(SunImage<ColorType>::distance_transform):
			cerr<<"Distance transform is not yet implemented"<<endl;
			
		case(SunImage<ColorType>::no_projection):
			dilateCircular(aggregationFactor, null());
			#if DEBUG >= 2
			writeFits(filenamePrefix + "dilated.fits");
			#endif
		break;
		
		default:
		cerr<<"Unknown projection type for blob aggregation"<<endl;
		return;
		
	}
	

	
	// We do the erosion
	switch(projection)
	{
		case(SunImage<ColorType>::equirectangular):
			projeted->erodeCircular((2./3.)*aggregationFactor, null());
			this->equirectangular_deprojection(projeted, false);
		break;
		
		case(SunImage<ColorType>::Lambert_cylindrical):
			projeted->erodeCircular((2./3.)*aggregationFactor, null());
			this->Lambert_cylindrical_deprojection(projeted, false);
		break;
		
		case(SunImage<ColorType>::sinuosidal):
			projeted->erodeCircular((2./3.)*aggregationFactor, null());
			this->sinuosidal_deprojection(projeted, false);
		break;
		
		case(SunImage<ColorType>::distance_transform):
			cerr<<"Distance transform is not yet implemented"<<endl;
			
		case(SunImage<ColorType>::no_projection):
			erodeCircular(aggregationFactor, null());
		break;
		
		default:
		cerr<<"Unknown projection type for blob aggregation"<<endl;
		return;
		
	}
	delete projeted;
}

#ifdef MAGICK
MagickImage ColorMap::magick(const Magick::Color background)
{
	MagickImage image(background, xAxes, yAxes);
	for (unsigned y = 0; y < yAxes; ++y)
	{
		for (unsigned x = 0; x < xAxes; ++x)
		{	
			if(pixel(x, y) != nullpixelvalue)
			{
				image.pixelColor(x, yAxes - y - 1, Magick::Color(gradient[pixel(x, y) % gradientMax]));
			}
		}
	}
	return image;
}

MagickImage ColorMap::magick()
{
	return magick(Magick::Color(0, 0 ,0, MaxRGB));
}
#endif



