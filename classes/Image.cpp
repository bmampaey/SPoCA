#include "Image.h"

//!@file Image.cpp

using namespace std;

template<class T>
Image<T>::Image(const long xAxes, const long yAxes)
:xAxes(xAxes),yAxes(yAxes),numberPixels(xAxes * yAxes),pixels(NULL)
{
	nullvalue_ = numeric_limits<T>::has_infinity?numeric_limits<T>::infinity():numeric_limits<T>::max();
	if(numberPixels > 0)
		pixels = new T[numberPixels];

}

template<class T>
Image<T>::Image(const Image<T>& i)
:xAxes(i.xAxes),yAxes(i.yAxes),numberPixels(i.numberPixels),nullvalue_(i.nullvalue_)
{
	pixels = new T[numberPixels];
	memcpy(pixels, i.pixels, numberPixels * sizeof(T));
}


template<class T>
Image<T>::Image(const Image<T>* i)
:xAxes(i->xAxes),yAxes(i->yAxes),numberPixels(i->numberPixels),nullvalue_(i->nullvalue_)
{
	pixels = new T[numberPixels];
	memcpy(pixels, i->pixels, numberPixels * sizeof(T));
}


template<class T>
Image<T>::~Image()
{
	delete[] pixels;
	pixels = NULL;
	#if DEBUG >= 3
		cerr<<"Destructor for Image called (pixels = "<<pixels<<" to "<< numberPixels * sizeof(T)<<")"<<endl;
	#endif
}

template<class T>
inline unsigned Image<T>::Xaxes() const{return xAxes;}
template<class T>
inline unsigned Image<T>::Yaxes() const{return yAxes;}
template<class T>
inline unsigned Image<T>::NumberPixels() const{return numberPixels;}
template<class T>
inline Coordinate Image<T>::coordinate (const unsigned j)const{return Coordinate(j%xAxes, unsigned(j/xAxes));}
template<class T>
inline T& Image<T>::pixel(const unsigned& j)
{return pixels[j];}
template<class T>
inline const T& Image<T>::pixel(const unsigned& j)const
{return pixels[j];}
template<class T>
inline T& Image<T>::pixel(const unsigned& x, const unsigned& y)
{return pixels[x+(y*xAxes)];}
template<class T>
inline const T& Image<T>::pixel(const unsigned& x, const unsigned& y)const
{return pixels[x+(y*xAxes)];}
template<class T>
inline T& Image<T>::pixel(const Coordinate& c)
{return pixels[c.x+(c.y*xAxes)];};
template<class T>
inline const T& Image<T>::pixel(const Coordinate& c)const
{return pixels[c.x+(c.y*xAxes)];};




template<class T>
Image<T>* Image<T>::resize(const unsigned xAxes, const unsigned yAxes)
{
	if(xAxes * yAxes != numberPixels)
	{
		numberPixels = xAxes * yAxes;
		delete[] pixels;
		pixels = new T[numberPixels];
	}
	this->xAxes = xAxes;
	this->yAxes = yAxes;
	
	return this;
}

template<class T>
Image<T>* Image<T>::zero(T value)
{
	fill(pixels, pixels + numberPixels, value);
	return this;
}


template<class T>
Image<T>* Image<T>::dilateDiamond(unsigned size, T pixelValueToDilate)
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


template<class T>
Image<T>* Image<T>::erodeDiamond(unsigned size, T pixelValueToErode)
{

	T fillPixelValue = nullvalue_;
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


template<class T>
Image<T>* Image<T>::dilateCircular(const unsigned size, const T unsetValue)
{
	T * newPixels = new T[numberPixels];
	fill(newPixels, newPixels + numberPixels, unsetValue);
	vector<unsigned> shape;
	shape.reserve(size*size*3);
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -size; x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y *y) <= size)
				shape.push_back(y * xAxes + x);
	
				
	int j;
	unsigned y, x;
	for(y = size; y < yAxes - size; ++y)
	{		
		j = 	y * xAxes + size;
		for(x = size; x < xAxes - size; ++x)
		{
			
			if(pixels[j] != unsetValue)
			{
				newPixels[j] = pixels[j];
				if(pixels[j-1] == unsetValue || pixels[j+1] == unsetValue || pixels[j-xAxes] == unsetValue || pixels[j+xAxes] == unsetValue)
				{
					
					for(unsigned s = 0; s < shape.size(); ++s)
					{
						#if DEBUG >= 1
							if(j + shape[s] >= numberPixels || j - shape[s] < 0)
							{
								cerr<<"Error : trying to access pixel out of image in drawContours"<<endl;
								exit(EXIT_FAILURE);
							}	
						#endif
						newPixels[j + shape[s]] = newPixels[j - shape[s]] = pixels[j];
					}
				}

			}
			++j;
		}
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}


template<class T>
Image<T>* Image<T>::erodeCircular(const unsigned size, const T unsetValue)
{
	T * newPixels = new T[numberPixels];
	memcpy(newPixels, pixels, numberPixels * sizeof(T));
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
			
			if(pixels[j] != unsetValue && (pixels[j-1] != pixels[j] || pixels[j+1] != pixels[j] || pixels[j-xAxes] != pixels[j] || pixels[j+xAxes] != pixels[j]))
			{
				newPixels[j] = unsetValue;
				for(unsigned s = 0; s < shape.size(); ++s)
				{
					#if DEBUG >= 1
						if(j + shape[s] >= numberPixels || j - shape[s] < 0)
						{
							cerr<<"Error : trying to access pixel out of image in drawContours"<<endl;
							exit(EXIT_FAILURE);
						}	
					#endif
					newPixels[j + shape[s]] = newPixels[j - shape[s]] = unsetValue;
				}
				
			}
			++j;
		}
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}



template<class T>
Image<T>* Image<T>::drawBox(const T color, Coordinate min, Coordinate max)
{

	if (min.x >= xAxes || min.y >= yAxes)	  //The box is out of the picture
		return this;
	if (max.x >= xAxes)
		max.x = xAxes - 1;
	if (max.y >= yAxes)
		max.y = yAxes - 1;

	for (unsigned x=min.x; x <= max.x; ++x)
	{
		pixel(x,min.y) = pixel(x,max.y) = color;
	}
	for (unsigned y=min.y; y <= max.y; ++y)
	{
		pixel(min.x,y) = pixel(max.x,y) = color;
	}

	return this;

}


template<class T>
Image<T>* Image<T>::drawCross(const T color, Coordinate c, const unsigned size)
{
	unsigned min, max;
	min = c.x < size + 1 ? 0 : c.x - size - 1;
	max = c.x + size + 1 < xAxes  ? c.x + size + 1 : xAxes - 1;

	for (unsigned x=min; x <= max; ++x)
	{
		pixel(x,c.y) = color;
	}
	min = c.y < size + 1 ? 0 : c.y - size - 1;
	max = c.y + size + 1 < yAxes  ? c.y + size + 1 : yAxes - 1;
	for (unsigned y=min; y <= max; ++y)
	{
		pixel(c.x,y) = color;
	}

	return this;
}

template<class T>
Image<T>* Image<T>::drawInternContours(const unsigned width, const T unsetValue)
{

	Image<T> * eroded = new Image<T> (this);
	eroded->erodeCircular(width, unsetValue);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(eroded->pixels[j] != eroded->nullvalue_)
			pixels[j] = unsetValue;
	}
	delete eroded;
	return this;

}

template<class T>
Image<T>* Image<T>::drawExternContours(const unsigned width, const T unsetValue)
{

	Image<T> * copy = new Image<T> (this);
	this->dilateCircular(width, unsetValue);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == copy->pixels[j])
			pixels[j] = unsetValue;
	}
	delete copy;
	return this;

}

template<class T>
Image<T>* Image<T>::drawContours(const unsigned width, const T unsetValue)
{
	unsigned size = width/2;
	T * newPixels = new T[numberPixels];
	memcpy(newPixels, pixels, numberPixels * sizeof(T));
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
			T maxColor = pixels[j-1];
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

template<class T>
void Image<T>::diff(const Image<T> * img)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(img->pixels[j] == nullvalue_)
			pixels[j] = nullvalue_;
		else if (pixels[j] != nullvalue_)
			pixels[j] -= img->pixels[j];
		
	}
}


template<class T>
void Image<T>::div(const Image<T> * img)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(img->pixels[j] == nullvalue_ || img->pixels[j] == 0)
			pixels[j] = nullvalue_;
		else if (pixels[j] != nullvalue_)
			pixels[j] /= img->pixels[j];
	}
}


template<class T>
unsigned Image<T>::colorizeConnectedComponents(const T setValue)
{
	T color = setValue;
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


template<class T>
unsigned Image<T>::propagateColor(const T color, const Coordinate& firstPixel)
{
	return propagateColor(color, firstPixel.x + firstPixel.y * xAxes);
}


template<class T>
unsigned Image<T>::propagateColor(const T color, const unsigned firstPixel)
{
	vector<unsigned> pixelList;
	T setValue = pixels[firstPixel];
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


template<class T>
unsigned Image<T>::tresholdConnectedComponents(const unsigned minSize, const T setValue)
{
	vector<unsigned> treatedPixels;
	T color = setValue + 1;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == setValue)
		{
			if (propagateColor(color, j) < minSize)
				propagateColor(nullvalue_, j);
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


template<class T>
Image<T>* Image<T>::bitmap(const Image<T>* bitMap, T setValue)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
			pixels[j] = bitMap->pixel(j) == setValue ? 1 : nullvalue_;
	}
	return this;

}

template<class T>
Image<T>* Image<T>::removeHoles(T unusedColor)
{
	propagateColor(unusedColor, 0);
	T lastColor = nullvalue_;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullvalue_)
		{
			lastColor = pixels[j];
		}
		else
		{
			pixels[j] = lastColor;
		}
	}
	propagateColor(nullvalue_, 0);
	return this;
}

template<class T>
void Image<T>::minmax(T& min, T& max) const
{
	min = numeric_limits<T>::max();
	max = 0;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullvalue_)
		{
			min = pixels[j] < min ? pixels[j] : min;
			max = pixels[j] > max ? pixels[j] : max;
				
		}
	}
}


template<class T>
Real Image<T>::mean() const
{
	Real sum = 0;
	Real card = 0;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullvalue_)
		{
			sum += pixels[j];
			++card;
		}
	}
	if(card == 0)
		return 0;
	else
		return sum / card;

}


template<class T>
Real Image<T>::variance() const
{
	Real m2 = 0, temp = 0;
	Real card = 0;
	Real meanValue = mean();
	for (unsigned j = 0; j < numberPixels; ++j)
		if(pixels[j] != nullvalue_)
	{
		temp = (pixels[j] - meanValue);
		m2 += temp * temp;
		++card;
	}
	if(card == 0)
		return 0;
	else
		return m2 / card;

}


template<class T>
Real Image<T>::skewness() const
{
	Real m2 = 0, m3 = 0;
	Real card = 0;
	Real meanValue = mean(), temp;
	for (unsigned j = 0; j < numberPixels; ++j)
		if(pixels[j] != nullvalue_)
	{
		temp = (pixels[j] - meanValue);
		temp *= temp;
		m2 += temp;
		temp *= (pixels[j] - meanValue);
		m3 += temp;
		++card;
	}

	if(card == 0)
		return 0;

	m2 /= card;
	m3 /= card;

	return m3 / sqrt(m2 * m2 * m2);

}


template<class T>
Real Image<T>::kurtosis() const
{
	Real m2 = 0, m4 = 0;
	Real card = 0;
	Real meanValue = mean(), temp;
	for (unsigned j = 0; j < numberPixels; ++j)
		if(pixels[j] != nullvalue_)
	{
		temp = (pixels[j] - meanValue);
		temp *= temp;
		m2 += temp;
		temp *= temp;
		m4 += temp;
		++card;
	}

	if(card == 0)
		return 0;

	m2 /= card;
	m4 /= card;

	return ( m4 / (m2 * m2) ) - 3;

}


template<class T>
void Image<T>::neighboorhoodMean(const Image<T>* image, int Nradius)
{
	if(numberPixels != image->numberPixels)
	{
		delete pixels;
		pixels = new T[image->numberPixels];
		numberPixels = image->numberPixels;
	}

	xAxes = image->xAxes;
	yAxes = image->yAxes;

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * xAxes + x);
		}
	}

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real m1 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if(neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue_ )
			{
				m1 += image->pixels[neighboor];
				++card;
			}

		}
		if(card == 0)
		{
			pixels[j] = nullvalue_;
		}
		else
		{
			pixels[j] = T(m1 / card);
		}

	}

}


template<class T>
void Image<T>::neighboorhoodVariance(const Image<T>* image, int Nradius)
{

	if(numberPixels != image->numberPixels)
	{
		delete pixels;
		pixels = new T[image->numberPixels];
		numberPixels = image->numberPixels;
	}
	xAxes = image->xAxes;
	yAxes = image->yAxes;

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * xAxes + x);
		}
	}

	Image<T>* meanImage = new Image<T>(0,0);
	meanImage->neighboorhoodMean(image, Nradius);

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real temp = 0, m2 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue_ )
			{
				temp = (image->pixels[neighboor] - meanImage->pixels[j]);
				m2 += temp * temp;
				++card;
			}

		}
		if(card == 0)
		{
			pixels[j] = nullvalue_;
		}
		else
		{
			pixels[j] = T(m2 / card);
		}

	}

	delete meanImage;

}


template<class T>
void Image<T>::neighboorhoodSkewness(const Image<T>* image, int Nradius)
{

	if(numberPixels != image->numberPixels)
	{
		delete pixels;
		pixels = new T[image->numberPixels];
		numberPixels = image->numberPixels;
	}
	xAxes = image->xAxes;
	yAxes = image->yAxes;

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * xAxes + x);
		}
	}

	Image<T>* meanImage = new Image<T>(0,0);
	meanImage->neighboorhoodMean(image, Nradius);

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real temp = 0, m2 = 0, m3 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue_ )
			{
				temp = (image->pixels[neighboor] - meanImage->pixels[j]);
				m2 += temp * temp;
				m3 += temp * temp * temp;
				++card;
			}

		}
		if(card == 0 || m2 == 0)
		{
			pixels[j] = nullvalue_;
		}
		else
		{
			m2 /= card;
			m3 /= card;
			m2 = m2 * m2 * m2;
			if(m2 != 0)
				pixels[j] = T(m3 / sqrt(m2));
			else
				pixels[j] = nullvalue_;

		}

	}

	delete meanImage;

}


template<class T>
void Image<T>::neighboorhoodKurtosis(const Image<T>* image, int Nradius)
{

	if(numberPixels != image->numberPixels)
	{
		delete pixels;
		pixels = new T[image->numberPixels];
		numberPixels = image->numberPixels;
	}
	xAxes = image->xAxes;
	yAxes = image->yAxes;

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * xAxes + x);
		}
	}

	Image<T>* meanImage = new Image<T>(0,0);
	meanImage->neighboorhoodMean(image, Nradius);

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real temp = 0, m2 = 0, m4 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue_ )
			{
				temp = (image->pixels[neighboor] - meanImage->pixels[j]);
				temp *= temp;
				m2 += temp;
				temp *= temp;
				m4 += temp;
				++card;
			}

		}
		if(card == 0)
		{
			pixels[j] = nullvalue_;
		}
		else
		{
			m2 /= card;
			m4 /= card;
			if(m2 != 0)
				pixels[j] = T(( m4 / (m2 * m2) ) - 3);
			else
				pixels[j] = nullvalue_;
		}

	}

	delete meanImage;

}


Real normal (Real mu, Real sigma)
{
	Real result = Real(sqrt(-2*log((1 + Real(rand() %10000))/10000.))*cos(2*3.14159265*((1 + Real(rand() %10000))/10000.)));
	return Real(mu + (result * sigma));
}


void fillRandomDots(Image<PixelType>* image, unsigned numberClasses, const vector<Real>& classesFeatures, const vector<Real>& backgroundFeatures)
{

	srand(unsigned(time(0)));
	//First we fill the entire picture with a normal distribution with mu = classesFeatures[0] and sigma = classesFeatures[1]
	for(unsigned j = 0; j < image->NumberPixels(); ++j)
		image->pixel(j) = normal(backgroundFeatures[0],backgroundFeatures[1]);

	//Then, for each class we put five dots of color classesFeatures[2*i] that we dilate
	for(unsigned j = 0; j < 5; ++j)
		for (unsigned i = 0; i < numberClasses; ++i)
	{
		image->pixel(rand()%image->NumberPixels()) = classesFeatures[4*i+2];
		image->dilateCircular(unsigned (classesFeatures[4*i] + (rand()%int(classesFeatures[4*i+1]))), classesFeatures[4*i+2]);
	}

	//Last we fill these dots with normal distribution with mu = classesFeatures[2*i] and sigma = classesFeatures[2*i+1]
	for (unsigned i = 0; i < numberClasses; ++i)
	{
		for(unsigned j = 0; j < image->NumberPixels(); ++j)
			if(image->pixel(j) == classesFeatures[4*i+2])
				do
		image->pixel(j) = normal(classesFeatures[4*i+2],classesFeatures[4*i+3]);
		while(image->pixel(j) < 0);
	}

}

template<class T>
Image<T>* Image<T>::convolution(const Image<T> * img, const float kernel[3][3])
{
	resize(img->xAxes, img->yAxes);
	unsigned j = xAxes + 1;

	T* p0 = img->pixels;
	T* p1 = p0+xAxes;
	T* p2 = p1+xAxes;

	for(unsigned y = 1; y < yAxes-1; ++y)
	{
		for(unsigned x = 1; x < xAxes-1; ++x)
		{
			pixels[j++] =	kernel[0][0] * p0[0] + kernel[0][1] * p0[1] + kernel[0][2] * p0[2] +
						kernel[1][0] * p1[0] + kernel[1][1] * p1[1] + kernel[1][2] * p1[2] +
						kernel[2][0] * p2[0] + kernel[2][1] * p2[1] + kernel[2][2] * p2[2] ;
			++p0;
			++p1;
			++p2;
		}

	}
	return this;
}

template<class T>
Image<T>* Image<T>::convolution(const Image<T> * img, const float kernel[5][5])
{
	resize(img->xAxes, img->yAxes);
	return this;
}

unsigned abs(unsigned x)
{return x;}

template<class T>
Image<T>* Image<T>::sobel_approx(const Image<T> * img)
{
	resize(img->xAxes, img->yAxes);
	unsigned j = 0;

	T* p0 = img->pixels;
	T* p1 = p0+xAxes;
	T* p2 = p1+xAxes;

	for(unsigned y = 0; y < yAxes-2; ++y)
	{
		for(unsigned x = 0; x < xAxes-2; ++x)
		{
			pixels[j++] = ( abs((p0[0] + 2*p0[1] + p0[2]) - (p2[0] + 2*p2[1]+ p2[2])) + abs((p0[2] + 2*p1[2] + p2[2]) - (p0[0] + 2*p1[0] + p2[0])) ) / 6;
			++p0;
			++p1;
			++p2;
		}

	}
	return this;
}

template<class T>
Image<T>* Image<T>::sobel(const Image<T> * img)
{
	resize(img->xAxes, img->yAxes);
	const float sobel_kernelx[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
	const float sobel_kernely[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

	Image<T> Cx (img->xAxes, img->xAxes);
	Image<T> Cy (img->xAxes, img->xAxes);
	Cx.convolution(img, sobel_kernelx);
	Cy.convolution(img, sobel_kernely);
	for (unsigned j= 0; j < numberPixels; ++j) 
	{
		pixels[j] = sqrt(Cx.pixels[j] * Cx.pixels[j] + Cy.pixels[j] * Cy.pixels[j]);
	}
	return this;
}

template<class T>
Image<T>* Image<T>::convolveHoriz(const Image<T>* img,  const vector<float>& kernel)
{

	const T* ptrrow = img->pixels;
	const T* ppp;	
	T* ptrout;
	
	// I can't convolve myself
	if(img != this)
	{
		resize(img->xAxes, img->yAxes);
		ptrout = pixels;
	}
	else
	{
		ptrout = new T[img->NumberPixels()];
	}
	

	const unsigned radius = kernel.size() / 2;

	/* Kernel width must be odd */
	if(kernel.size() % 2 != 1)
	{
		cerr<<"Kernel width must be odd"<<endl;
		exit(EXIT_FAILURE);
	}

	/* For each row, do ... */

	for (unsigned y = 0 ; y < img->yAxes ; y++)
	{
		unsigned x = 0;
		/* Zero leftmost columns */
		for ( ; x < radius ; x++)
			*ptrout++ = 0.0;

		/* Convolve middle columns with kernel */
		for ( ; x < img->xAxes - radius ; x++)
		{
			ppp = ptrrow + x - radius;
			register T sum = 0.0;
			for (int k = kernel.size()-1 ; k >= 0 ; k--)
				sum += *ppp++ * kernel[k];
			*ptrout++ = sum;
		}

		/* Zero rightmost columns */
		for ( ; x < img->xAxes; x++)
			*ptrout++ = 0.0;

		ptrrow += img->xAxes;
	}
	if(img == this)
	{
		delete[] pixels;
		pixels = ptrout;
	}
	return this;
}



template<class T>
Image<T>* Image<T>::convolveVert(const Image<T>* img, const vector<float>& kernel)
{

	
	const T* ptrcol = img->pixels;	 			
	const T* ppp;												 
	T* ptrout;
	
	// I can't convolve myself
	if(img != this)
	{
		resize(img->xAxes, img->yAxes);
		ptrout = pixels;
	}
	else
	{
		ptrout = new T[img->NumberPixels()];
	}

	const unsigned radius = kernel.size() / 2;

	/* Kernel width must be odd */
	if(kernel.size() % 2 != 1)
	{
		cerr<<"Kernel width must be odd"<<endl;
		exit(EXIT_FAILURE);
	}

	/* For each column, do ... */

	for (unsigned x = 0 ; x < img->xAxes; x++)
	{
		unsigned y = 0;
		/* Zero leftmost columns */
		for ( ; y < radius ; y++)
		{
			*ptrout = 0.0;
			ptrout += img->xAxes;
		}

		/* Convolve middle rows with kernel */
		for ( ; y < img->yAxes - radius ; y++)
		{
			ppp = ptrcol + img->xAxes* (y - radius);
			register T sum = 0.0;
			for (int k = kernel.size()-1 ; k >= 0 ; k--)
			{
				sum += *ppp * kernel[k];
				ppp += img->xAxes;
			}
			*ptrout = sum;
			ptrout += img->xAxes;
		}

		/* Zero rightmost columns */
		for ( ; y < img->yAxes ; y++)
		{
			*ptrout = 0.0;
			ptrout += img->xAxes;
		}

		ptrcol++;
		ptrout -= img->yAxes * img->xAxes- 1;
	}
	if(img == this)
	{
		delete[] pixels;
		pixels = ptrout;
	}
	return this;
}

template<class T>
Image<T>* Image<T>::convolveSeparate(const Image<T>* img,  const vector<float>& horiz_kernel, const vector<float>& vert_kernel)
{

	Image<T> imgtmp;
	imgtmp.convolveHoriz(img, horiz_kernel);
	convolveVert(&imgtmp, vert_kernel);
	return this;
}

template<class T>
T Image<T>::interpolate(const float x, const float y) const
{
	int xt = (int) x;/* coordinates of top-left corner */
	int yt = (int) y;
	float ax = x - xt;
	float ay = y - yt;
	return ( (1-ax)*(1-ay)*pixel(xt,yt) + ax*(1-ay)*pixel(xt+1,yt) + (1-ax)*ay*pixel(xt,yt+1) + ax*ay*pixel(xt+1,yt+1));
}

/*!
To extract the chain code of a connected component, we first list all the points along the boundary starting at the firstPixel.
Once The list is made, it is reduced by trying to find the most relevant points.
First we take the firstpixel point and it's furthest point in the list, and add them to the chain code.
Then we search the point that is the furthest from the line passing by each pair of consecutive point in the chain code, and add it to the chain code.
We repeat that last step until we have enough points  
*/
template<class T>
vector<Coordinate> Image<T>::chainCode(const Coordinate firstPixel, const unsigned max_points) const
{
	vector<Coordinate> chain;
	chain.reserve(1000);
	
	// We list all the directions
	int directions[] = {	0 + xAxes, //Norh
				1 + xAxes, //NE
				1 + 0, //East
				1 - xAxes, //SE
				0 - xAxes, //South
				-1 - xAxes, //SW
				-1 + 0, //West
				-1 + xAxes, //NW
				//We repeat for simplicity 
				0 + xAxes, 1 + xAxes, 1 + 0, 1 - xAxes, 0 - xAxes, -1 - xAxes, -1 + 0, -1 + xAxes
			};
	
	// We start at the first pixel, and we search for the first direction
	chain.push_back(firstPixel);
	unsigned cur_p = firstPixel.x + firstPixel.y * xAxes;
	int first_direction = 0;
	bool found = false;
	for (unsigned i = 0; i <= 8; ++i)
	{
		int next_p = cur_p + directions[first_direction];
		if(next_p >= 0 && next_p < int(numberPixels))
		{
			if(pixels[next_p] == nullvalue_)
			{
				found=true;
			}
			else if (found)
			{
				break;
			}
		}
		++first_direction;
	}
	first_direction%=8;
	// If we are a single pixel, we return
	if(!found)
	{
		return chain;
	}

	// We create a classical chaincode of all pixels locations along the boundary
	// until we come back to the first pixel, with the same direction
	Coordinate current_pixel = firstPixel;
	int next_direction = first_direction;
	
	// At the same time I search for the pixel the furthest away from the first_pixel
	// See below
	Real biggest_distance = 0.;
	unsigned furthest_pixel_indice = 0;
	
	do
	{
		// We move the current pixel into the next direction
		cur_p = cur_p + directions[next_direction];
		// I add the current pixel to the chain
		current_pixel.x = cur_p%xAxes;
		current_pixel.y = unsigned(cur_p/xAxes);
		chain.push_back(current_pixel);
		// I check if it is the furthest pixel
		Real distance = firstPixel.d2(current_pixel);
		if(distance > biggest_distance)
		{
			biggest_distance = distance;
			furthest_pixel_indice = chain.size() - 1;
		}
		// We search the direction of the following pixel on the border by looking at all directions
		// Starting at the opposite direction +1
		next_direction = (next_direction + 5) % 8;
		for (unsigned i = 0; i < 8; ++i)
		{
			int next_p = cur_p + directions[next_direction];
			if(next_p >= 0 && next_p < int(numberPixels) && pixels[next_p] != nullvalue_)
			{
				break;
			}
			++next_direction;
		}
		next_direction %= 8;
		
	}while (!((firstPixel == current_pixel) && (next_direction == first_direction)));


	if(chain.size() <= max_points)
	{
		return chain;
	}

	// Now we reduce the chain code to max_points
	// The good_indices is a sorted list of the indices of most important points in the chain code
	vector<unsigned> good_indices;
	good_indices.reserve(max_points);
	good_indices.push_back(0);
	// The tmp_indices is a list of the indices of some important points in the chain code
	// The corresponding distances list specify for each tmp_indice the distance to the current reduced chain code 
	vector<unsigned> tmp_indices;
	vector<Real> distances;
	tmp_indices.reserve(max_points);
	distances.reserve(max_points);
	tmp_indices.push_back(furthest_pixel_indice);
	distances.push_back(biggest_distance);
	while(good_indices.size() < max_points)
	{
		// I search in the tmp_indices for the worst point, i.e. with the biggest distance
		unsigned worst_indice = 0;
		Real max_distance = distances[worst_indice];
		for(unsigned i = 1; i < tmp_indices.size(); ++i)
		{
			if(max_distance < distances[i])
			{
				max_distance = distances[i];
				worst_indice = i;
			}
		}
		
		// I remove the worst point from the tmp_indices
		unsigned worst_point_indice = tmp_indices[worst_indice];
		tmp_indices.erase(tmp_indices.begin()+worst_indice);
		distances.erase(distances.begin()+worst_indice);
		// I add the worst point indice to the good_indices
		vector<unsigned>::iterator it;
		for(it = good_indices.begin(); it != good_indices.end() && worst_point_indice > *it; ++it){}
		it = good_indices.insert(it, worst_point_indice);

		// I search for the worst point between the worst_point_indice and the previous point from the good_indices
		unsigned previous_indice = *(it-1);
		// We compute the line equation ax+by+c=0 passing between points chain[previous_indice] and chain[worst_point_indice]
		Real a = Real(chain[worst_point_indice].y) - chain[previous_indice].y;
		Real b = Real(chain[previous_indice].x) - chain[worst_point_indice].x;
		Real c = - (b * chain[previous_indice].y + a * chain[previous_indice].x);
		// We search the pixel in the chain that is the furthest to the line
		max_distance = 0;
		worst_indice = previous_indice+1;
		for(unsigned i = previous_indice+1; i < worst_point_indice; ++i)
		{
			Real d =  abs(a * chain[i].x + b * chain[i].y + c);
			if(d >= max_distance)
			{
				max_distance = d;
				worst_indice = i;
			}
		}
		max_distance/=sqrt(a*a+b*b);
		// We add that new worst point to the tmp_indices
		if(worst_indice != worst_point_indice)
		{
			tmp_indices.push_back(worst_indice);
			distances.push_back(max_distance);
		}
		// I search for the worst point between the worst_point_indice and the next point from the good_indices
		unsigned next_indice = (it+1 != good_indices.end()) ? *(it+1) : chain.size()-1;
		// We compute the line equation ax+by+c=0 passing between points chain[worst_point_indice] and chain[next_indice]
		a = Real(chain[next_indice].y) - chain[worst_point_indice].y;
		b = Real(chain[worst_point_indice].x) - chain[next_indice].x;
		c = - (b * chain[worst_point_indice].y + a * chain[worst_point_indice].x);
		// We search the pixel in the chain that is the furthest to the line
		max_distance = 0;
		worst_indice = worst_point_indice+1;
		for(unsigned i = worst_point_indice+1; i < next_indice; ++i)
		{
			Real d =  abs(a * chain[i].x + b * chain[i].y + c);
			if(d >= max_distance)
			{
				max_distance = d;
				worst_indice = i;
			}
		}
		max_distance/=sqrt(a*a+b*b);
		// We add that new worst point to the tmp_indices
		if(worst_indice != next_indice)
		{
			tmp_indices.push_back(worst_indice);
			distances.push_back(max_distance);
		}
	}
	// I compute the reduced chain by using the good_indices
	vector<Coordinate> reduced_chain(good_indices.size());
	for(unsigned i=0; i < good_indices.size(); ++i)
	{
		reduced_chain[i]=chain[good_indices[i]];
	}
	return reduced_chain;
}


template<class T>
FitsFile& Image<T>::writeFits(FitsFile& file, int mode)
{
	return file.writeImage(pixels, xAxes, yAxes, mode);
}

template<class T>
FitsFile& Image<T>::readFits(FitsFile& file)
{
	file.readImage(pixels, xAxes, yAxes, &(nullvalue_));
	numberPixels = xAxes * yAxes;
	return file;
}

template<class T>
bool Image<T>::writeFits(const std::string& filename, int mode)
{
	FitsFile file(filename, FitsFile::overwrite);
	this->writeFits(file, mode);
	return file.isGood();
}

template<class T>
bool Image<T>::readFits(const std::string& filename)
{
	FitsFile file(filename);
	this->readFits(file);
	return file.isGood();
}


/*! @file Image.cpp
Instantiation of the template class Image for PixelType
See @ref Compilation_Options constants.h */

template class Image<PixelType>;

/*! @file Image.cpp
Instantiation of the template class Image for ColorType
See @ref Compilation_Options constants.h */

template class Image<ColorType>;

