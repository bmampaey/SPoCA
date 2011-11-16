#include "Image.h"
#include <deque>
#include <assert.h>

//!@file Image.cpp

using namespace std;

template<class T>
Image<T>::Image(const unsigned& xAxes, const unsigned& yAxes)
:xAxes(xAxes),yAxes(yAxes),numberPixels(xAxes * yAxes),pixels(NULL)
{
	nullpixelvalue = numeric_limits<T>::has_infinity?numeric_limits<T>::infinity():numeric_limits<T>::max();
	if(numberPixels > 0)
		pixels = new T[numberPixels];

}

template<class T>
Image<T>::Image(const Image<T>& i)
:xAxes(i.xAxes),yAxes(i.yAxes),numberPixels(i.numberPixels),nullpixelvalue(i.nullpixelvalue)
{
	pixels = new T[numberPixels];
	memcpy(pixels, i.pixels, numberPixels * sizeof(T));
}


template<class T>
Image<T>::Image(const Image<T>* i)
:xAxes(i->xAxes),yAxes(i->yAxes),numberPixels(i->numberPixels),nullpixelvalue(i->nullpixelvalue)
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
inline unsigned Image<T>::Xaxes() const
{
	return xAxes;
}

template<class T>
inline unsigned Image<T>::Yaxes() const
{
	return yAxes;
}

template<class T>
inline unsigned Image<T>::NumberPixels() const
{
	return numberPixels;
}

template<class T>
inline PixLoc Image<T>::coordinate(const unsigned j) const
{
	if (j < numberPixels)
		return PixLoc(j%xAxes, unsigned(j/xAxes));
	else
		return PixLoc::null();
}

template<class T>
inline T& Image<T>::pixel(const unsigned& j)
{
	assert(j < numberPixels);
	return pixels[j];
}

template<class T>
inline const T& Image<T>::pixel(const unsigned& j)const
{
	assert(j < numberPixels);
	return pixels[j];
}

template<class T>
inline T& Image<T>::pixel(const unsigned& x, const unsigned& y)
{return pixels[x+(y*xAxes)];}

template<class T>
inline const T& Image<T>::pixel(const unsigned& x, const unsigned& y)const
{
	assert(x < xAxes && y < yAxes);
	return pixels[x+(y*xAxes)];
}

template<class T>
inline T& Image<T>::pixel(const PixLoc& c)
{
	assert(c.x < xAxes && c.y < yAxes);
	return pixels[c.x+(c.y*xAxes)];
}

template<class T>
inline const T& Image<T>::pixel(const PixLoc& c)const
{
	assert(c.x < xAxes && c.y < yAxes);
	return pixels[c.x+(c.y*xAxes)];
}




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
Image<T>* Image<T>::drawBox(const T color, PixLoc min, PixLoc max)
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
Image<T>* Image<T>::drawCross(const T color, PixLoc c, const unsigned size)
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
Image<T>* Image<T>::drawCircle(PixLoc center, double radius, T color)
{
	unsigned x0 = center.x;
	unsigned y0 = center.y;
	for(Real y = 0; y <= radius; ++y)
	{
		unsigned dx = unsigned((radius*cos(asin(y/radius)))+0.5);
		pixel(x0+dx,unsigned(y0+y)) = color;
		pixel(x0-dx,unsigned(y0+y)) = color;
		pixel(x0+dx,unsigned(y0-y)) = color;
		pixel(x0-dx,unsigned(y0-y)) = color;
	}
	return this;
}



template<class T>
void Image<T>::diff(const Image<T> * img)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(img->pixels[j] == nullpixelvalue)
			pixels[j] = nullpixelvalue;
		else if (pixels[j] != nullpixelvalue)
			pixels[j] -= img->pixels[j];
		
	}
}


template<class T>
void Image<T>::div(const Image<T> * img)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(img->pixels[j] == nullpixelvalue || img->pixels[j] == 0)
			pixels[j] = nullpixelvalue;
		else if (pixels[j] != nullpixelvalue)
			pixels[j] /= img->pixels[j];
	}
}

template<class T>
void Image<T>::div(const T value)
{
	if (value == 0 )
	{
		cerr<<"Error: Trying to divide pixels by 0"<<endl;
	}
	else
	{
		for (unsigned j = 0; j < numberPixels; ++j)
		{
			if(pixels[j] != nullpixelvalue)
				pixels[j] /= value;
		}
	}
}

template<class T>
void Image<T>::mul(const T value)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
			pixels[j] *= value;
	}
	
}
		
template<class T>
void Image<T>::threshold(const T min, const T max = numeric_limits<T>::max())
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
		{
			pixels[j] = pixels[j] < min ? min : pixels[j];
			pixels[j] = pixels[j] > max ? max : pixels[j];
		}
	}
}



template<class T>
Image<T>* Image<T>::bitmap(T setValue)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		pixels[j] = pixels[j] == setValue ? 1 : nullpixelvalue;
	}
	return this;

}

template<class T>
Image<T>* Image<T>::bitmap(const Image<T>* bitMap, T setValue)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		pixels[j] = bitMap->pixel(j) == setValue ? 1 : nullpixelvalue;
	}
	return this;

}


template<class T>
void Image<T>::minmax(T& min, T& max) const
{
	min = numeric_limits<T>::max();
	max = numeric_limits<T>::is_signed ? - numeric_limits<T>::max() : 0;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
		{
			min = pixels[j] < min ? pixels[j] : min;
			max = pixels[j] > max ? pixels[j] : max;
				
		}
	}
}



#define ELEM_SWAP(a,b) { register T t=(a);(a)=(b);(b)=t; }
/* !
This Quickselect routine is based on the algorithm described in
"Numerical recipes in C", Second Edition,
Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
This code by Nicolas Devillard - 1998. Public domain.
 */
template<class T>
T Image<T>::quickselect(vector<T>& arr, Real percentil) const
{

	int low = 0 ;
	int high = arr.size()-1 ;
	int median = low + int((high - low) * percentil);
	
	int middle, ll, hh;
	
	for (;;)
	{
		if (high <= low)						  /* One element only */
			return arr[median] ;

		if (high == low + 1)					  /* Two elements only */
		{
			if (arr[low] > arr[high])
				ELEM_SWAP(arr[low], arr[high]) ;
			return arr[median] ;
		}

		/* Find median of low, middle and high items; swap into position low */
		middle = low + int((high - low) * percentil);
		if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
		if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
		if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

		/* Swap low item (now in position middle) into position (low+1) */
		ELEM_SWAP(arr[middle], arr[low+1]) ;

		/* Nibble from each end towards middle, swapping items when stuck */
		ll = low + 1;
		hh = high;
		for (;;)
		{
			do ll++; while (arr[low] > arr[ll]) ;
			do hh--; while (arr[hh]  > arr[low]) ;

			if (hh < ll)
				break;

			ELEM_SWAP(arr[ll], arr[hh]) ;
		}

		/* Swap middle item (in position low) back into correct position */
		ELEM_SWAP(arr[low], arr[hh]) ;

		/* Re-set active partition */
		if (hh <= median)
			low = ll;
		if (hh >= median)
			high = hh - 1;
	}
}
#undef ELEM_SWAP

template<class T>
T Image<T>::median() const
{
	vector<T> arr;
	arr.reserve(numberPixels);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
		{
			arr.push_back(pixels[j]);
		}
	}
	return quickselect(arr, 0.5);
}

template<class T>
vector<T> Image<T>::percentiles(const vector<Real>& p) const
{
	vector<T> arr;
	arr.reserve(numberPixels);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
		{
			arr.push_back(pixels[j]);
		}
	}
	vector<T> results;
	for (unsigned i = 0; i < p.size(); ++i)
	{
		if (p[i] >= 0 && p[i] <= 1)
		{
			results.push_back(quickselect(arr, p[i]));
		}
		else
		{
			cerr<<"Error: Percentiles must be values between 0 and 1"<<endl;
			exit(EXIT_FAILURE);
		}
	}
	return results;
}

template<class T>
T Image<T>::percentiles(const Real& p) const
{
	if (p < 0 || p > 1)
	{
		cerr<<"Error: Percentiles must be values between 0 and 1"<<endl;
		exit(EXIT_FAILURE);
	}
	
	vector<T> arr;
	arr.reserve(numberPixels);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
		{
			arr.push_back(pixels[j]);
		}
	}
	return quickselect(arr, p);
}

template<class T>
Real Image<T>::mode(Real binSize) const
{
	// If the binSize was not given or wrong, we compute by taking NUMBER_BINS in 2 times the variance
	if (binSize <= 0)
	{
		binSize = 2. * variance() / NUMBER_BINS;
	}
	// We build an histogram
	deque<unsigned> histo(2 * NUMBER_BINS, 0);
	T* pixelValue = pixels;
	T* endpixel = pixels + numberPixels;
	while(pixelValue != endpixel)
	{
		if ((*pixelValue) != nullpixelvalue)
		{
			unsigned bin = unsigned((*pixelValue)/binSize);
			if (bin >= histo.size())
				histo.resize(bin + 1024, 0);
			++histo[bin];
		}
		++pixelValue;
	}

	// We search for the mode
	unsigned max = 0;
	Real mode = 0;
	for (unsigned h = 0; h < histo.size(); ++h)
	{
		if (max < histo[h])
		{
			max = histo[h];
			mode = h;
		}
	}
	mode = mode * binSize + (binSize / 2);
	return mode;
}

template<class T>
Real Image<T>::mean() const
{
	Real sum = 0;
	Real card = 0;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullpixelvalue)
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
		if(pixels[j] != nullpixelvalue)
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
		if(pixels[j] != nullpixelvalue)
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
	{
		if(pixels[j] != nullpixelvalue)
		{
			temp = (pixels[j] - meanValue);
			temp *= temp;
			m2 += temp;
			temp *= temp;
			m4 += temp;
			++card;
		}
	}
	if(card == 0)
		return 0;

	m2 /= card;
	m4 /= card;

	return ( m4 / (m2 * m2) ) - 3;

}


template<class T>
void Image<T>::localMean(const Image<T>* image, int Nradius)
{
	resize(image->xAxes, image->yAxes);
	
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
			if(neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullpixelvalue )
			{
				m1 += image->pixels[neighboor];
				++card;
			}

		}
		if(card == 0)
		{
			pixels[j] = nullpixelvalue;
		}
		else
		{
			pixels[j] = T(m1 / card);
		}

	}

}


template<class T>
void Image<T>::localVariance(const Image<T>* image, int Nradius)
{

	resize(image->xAxes, image->yAxes);

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
	meanImage->localMean(image, Nradius);

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real temp = 0, m2 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullpixelvalue )
			{
				temp = (image->pixels[neighboor] - meanImage->pixels[j]);
				m2 += temp * temp;
				++card;
			}

		}
		if(card == 0)
		{
			pixels[j] = nullpixelvalue;
		}
		else
		{
			pixels[j] = T(m2 / card);
		}

	}

	delete meanImage;

}


template<class T>
void Image<T>::localSkewness(const Image<T>* image, int Nradius)
{

	resize(image->xAxes, image->yAxes);

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
	meanImage->localMean(image, Nradius);

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real temp = 0, m2 = 0, m3 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullpixelvalue )
			{
				temp = (image->pixels[neighboor] - meanImage->pixels[j]);
				m2 += temp * temp;
				m3 += temp * temp * temp;
				++card;
			}

		}
		if(card == 0 || m2 == 0)
		{
			pixels[j] = nullpixelvalue;
		}
		else
		{
			m2 /= card;
			m3 /= card;
			m2 = m2 * m2 * m2;
			if(m2 != 0)
				pixels[j] = T(m3 / sqrt(m2));
			else
				pixels[j] = nullpixelvalue;

		}

	}

	delete meanImage;

}


template<class T>
void Image<T>::localKurtosis(const Image<T>* image, int Nradius)
{
	resize(image->xAxes, image->yAxes);

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
	meanImage->localMean(image, Nradius);

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real temp = 0, m2 = 0, m4 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullpixelvalue )
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
			pixels[j] = nullpixelvalue;
		}
		else
		{
			m2 /= card;
			m4 /= card;
			if(m2 != 0)
				pixels[j] = T(( m4 / (m2 * m2) ) - 3);
			else
				pixels[j] = nullpixelvalue;
		}

	}

	delete meanImage;

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
			pixels[j++] =	T(kernel[0][0] * p0[0] + kernel[0][1] * p0[1] + kernel[0][2] * p0[2] +
						kernel[1][0] * p1[0] + kernel[1][1] * p1[1] + kernel[1][2] * p1[2] +
						kernel[2][0] * p2[0] + kernel[2][1] * p2[1] + kernel[2][2] * p2[2] );
			++p0;
			++p1;
			++p2;
		}

	}
	return this;
}


// There is no abs function for unsigned
inline unsigned abs(unsigned x)
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
			pixels[j++] = T((abs((p0[0] + 2*p0[1] + p0[2]) - (p2[0] + 2*p2[1]+ p2[2])) + abs((p0[2] + 2*p1[2] + p2[2]) - (p0[0] + 2*p1[0] + p2[0])) ) / 6.);
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
		pixels[j] = T(sqrt(Cx.pixels[j] * Cx.pixels[j] + Cy.pixels[j] * Cy.pixels[j]));
	}
	return this;
}

template<class T>
Image<T>* Image<T>::horizontal_convolution(const Image<T>* img,  const vector<float>& kernel)
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
			*ptrout++ = 0;

		/* Convolve middle columns with kernel */
		for ( ; x < img->xAxes - radius ; x++)
		{
			ppp = ptrrow + x - radius;
			register float sum = 0;
			for (int k = kernel.size()-1 ; k >= 0 ; k--)
				sum += *ppp++ * kernel[k];
			*ptrout++ = T(sum);
		}

		/* Zero rightmost columns */
		for ( ; x < img->xAxes; x++)
			*ptrout++ = 0;

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
Image<T>* Image<T>::vertical_convolution(const Image<T>* img, const vector<float>& kernel)
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
			*ptrout = 0;
			ptrout += img->xAxes;
		}

		/* Convolve middle rows with kernel */
		for ( ; y < img->yAxes - radius ; y++)
		{
			ppp = ptrcol + img->xAxes* (y - radius);
			register float sum = 0;
			for (int k = kernel.size()-1 ; k >= 0 ; k--)
			{
				sum += *ppp * kernel[k];
				ppp += img->xAxes;
			}
			*ptrout = T(sum);
			ptrout += img->xAxes;
		}

		/* Zero rightmost columns */
		for ( ; y < img->yAxes ; y++)
		{
			*ptrout = 0;
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
Image<T>* Image<T>::convolution(const Image<T>* img,  const vector<float>& horiz_kernel, const vector<float>& vert_kernel)
{

	Image<T> imgtmp;
	imgtmp.horizontal_convolution(img, horiz_kernel);
	vertical_convolution(&imgtmp, vert_kernel);
	return this;
}

template<class T>
Image<T>* Image<T>::binomial_smoothing(unsigned width, const Image<T>* img)
{
	if(width <= 1)
		return this;
	if (img == NULL)
		img = this;
	
	if(width%2 == 0)
		width += 1;
	
	unsigned N = width - 1;
	vector<double> factoriel(width, 1.);
	for(unsigned i = 2; i < width; ++i)
		factoriel[i] = factoriel[i-1]*i;
		
	double den = exp2(double(N));
	
	vector<float> kernel(width, 1./den);

	for(unsigned i = 1; i < N; ++i)
		kernel[i] = (factoriel[N] / (factoriel[i] * factoriel[N-i])) / den;
	#if DEBUG >= 3
		cerr<<"Binomial smoothing kernel: "<<kernel<<endl;
	#endif
	
	convolution(img, kernel, kernel);
	return this;
}

template<class T>
inline T Image<T>::interpolate(float x, float y) const
{
	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;
	if(x > xAxes-1.)
		x = xAxes-1.;
	if(y > yAxes-1.)
		y = yAxes-1.;
	
	unsigned ix = (unsigned) x;
	unsigned iy = (unsigned) y;
	Real dx = x - ix;
	Real dy = y - iy;
	Real cdx = 1. - dx;
	Real cdy = 1. - dy;
	T* p = pixels + iy * xAxes + ix;
	return T(cdx*cdy*Real(*p) + dx*cdy*Real(*(p+1)) + cdx*dy*Real(*(p+xAxes)) + dx*dy*Real(*(p+xAxes+1)));
}

template<class T>
inline T Image<T>::interpolate(const RealPixLoc& c) const
{
	return interpolate(c.x, c.y);
}


template<class T>
FitsFile& Image<T>::writeFits(FitsFile& file, int mode, const string imagename)
{
	return file.writeImage(pixels, xAxes, yAxes, mode, imagename);
}

template<class T>
FitsFile& Image<T>::readFits(FitsFile& file)
{
	file.readImage(pixels, xAxes, yAxes, &(nullpixelvalue));
	numberPixels = xAxes * yAxes;
	return file;
}

template<class T>
bool Image<T>::writeFits(const std::string& filename, int mode, const string imagename)
{
	FitsFile file(filename, FitsFile::overwrite);
	this->writeFits(file, mode, imagename);
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
Instantiation of the template class Image for EUVPixelType
See @ref Compilation_Options constants.h */

template class Image<EUVPixelType>;

/*! @file Image.cpp
Instantiation of the template class Image for ColorType
See @ref Compilation_Options constants.h */

template class Image<ColorType>;

