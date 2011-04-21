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
Image<T>* Image<T>::bitmap(const Image<T>* bitMap, T setValue)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
			pixels[j] = bitMap->pixel(j) == setValue ? 1 : nullvalue_;
	}
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

