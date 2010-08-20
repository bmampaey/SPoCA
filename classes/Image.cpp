#include "Image.h"

using namespace std;

template<class T>
Image<T>::~Image()
{
	delete[] pixels;
}


template<class T>
Image<T>::Image(const long xAxes, const long yAxes)
:naxis(2),numberPixels(xAxes * yAxes),pixels(NULL),nullvalue(numeric_limits<T>::has_infinity?numeric_limits<T>::infinity():numeric_limits<T>::max())
{

	axes[0] = xAxes;
	axes[1] = yAxes;

	if(typeid(T) == typeid(double))
		datatype = TDOUBLE;
	else if(typeid(T) == typeid(float))
		datatype = TFLOAT;
	else if(typeid(T) == typeid(long))
		datatype = TLONG;
	else if(typeid(T) == typeid(unsigned long))
		datatype = TLONG;
	else if(typeid(T) == typeid(short))
		datatype = TSHORT;
	else if(typeid(T) == typeid(unsigned short))
		datatype = TSHORT;						  //Because imagemagick does not understand BSCALE and BZERO
	//	datatype = TUSHORT;
	else if(typeid(T) == typeid(int))
		datatype = TINT;
	else if(typeid(T) == typeid(unsigned int))
		datatype = TINT;						  //Because imagemagick does not understand BSCALE and BZERO
	//	datatype = TUINT;
	else if(typeid(T) == typeid(char))
		datatype = TBYTE;
	else if(typeid(T) == typeid(signed char))
		datatype = TSBYTE;
	else
		datatype = TDOUBLE;

	switch(datatype)
	{
		case TDOUBLE:
			bitpix = DOUBLE_IMG;
			break;
		case TFLOAT:
			bitpix = FLOAT_IMG;
			break;
		case TLONG:
			bitpix = LONGLONG_IMG;
			break;
		case TULONG:
			bitpix = ULONG_IMG;					  //There is no ULONGLONG_IMG
			break;
		case TINT:
			bitpix = LONGLONG_IMG;
			break;
		case TUINT:
			bitpix = ULONG_IMG;
			break;
		case TSHORT:
			bitpix = SHORT_IMG;
			break;
		case TUSHORT:
			bitpix = SHORT_IMG;
			break;
		case TBYTE:
			bitpix = BYTE_IMG;
			break;
		case TSBYTE:
			bitpix = SBYTE_IMG;
			break;
		default:
			bitpix = DOUBLE_IMG;

	}

	if(numberPixels > 0)
		pixels = new T[numberPixels];

}

template<class T>
Image<T>::Image(const string& filename)
:nullvalue(numeric_limits<T>::has_infinity?numeric_limits<T>::infinity():numeric_limits<T>::max())
{

	//We determine the datatype from the template

	if(typeid(T) == typeid(double))
		datatype = TDOUBLE;
	else if(typeid(T) == typeid(float))
		datatype = TFLOAT;
	else if(typeid(T) == typeid(long))
		datatype = TLONG;
	else if(typeid(T) == typeid(unsigned long))
		datatype = TLONG;
	else if(typeid(T) == typeid(short))
		datatype = TSHORT;
	else if(typeid(T) == typeid(unsigned short))
		datatype = TSHORT;					  //Because imagemagick does not understand BSCALE and BZERO
	//	datatype = TUSHORT;
	else if(typeid(T) == typeid(int))
		datatype = TINT;
	else if(typeid(T) == typeid(unsigned int))
		datatype = TINT;					  //Because imagemagick does not understand BSCALE and BZERO
	//	datatype = TUINT;
	else if(typeid(T) == typeid(char))
		datatype = TBYTE;
	else if(typeid(T) == typeid(signed char))
		datatype = TSBYTE;
	else
		datatype = TDOUBLE;

	//We read the image from the fits file

	readFitsImage(filename);
	
	#if DEBUG >= 1
	// We check that there is no lost of precision

	switch(bitpix)
	{

		case BYTE_IMG:
			if(datatype != TBYTE)				  //Can't put a unsigned into a signed of same size
				break;
		case USHORT_IMG:
												  //Can't put a unsigned into a signed of same size or smaller
			if(datatype != TBYTE && datatype != TSHORT)
				break;
		case ULONG_IMG:
												  //Can't put a unsigned into a signed of same size or smaller
			if(datatype != TBYTE && datatype != TSHORT && datatype != TLONG && datatype != TINT)
				break;
		case SBYTE_IMG:
		case SHORT_IMG:
			if(datatype == TSHORT)
				break;
		case LONG_IMG:
			if(datatype == TLONG || datatype == TINT)
				break;
		case LONGLONG_IMG:
			if(datatype == TLONGLONG)
				break;
		case FLOAT_IMG:
			if(datatype == TFLOAT)
				break;
		case DOUBLE_IMG:
			if(datatype != TDOUBLE)
			{
				cerr<<"Warning : Fits File image data type and Image data type mismatch, you may lose precision or sign."<<endl;
			}
			break;
		default:
			cerr<<"Error : Unknown fits File image data type."<<endl;

	}
	
	#endif
	

}


template<class T>
Image<T>::Image(const Image<T>& i)
:naxis(i.naxis),numberPixels(i.numberPixels),datatype(i.datatype),anynull(i.anynull),bitpix(i.bitpix),nullvalue(i.nullvalue)
{
	axes[0] = i.axes[0];
	axes[1] = i.axes[1];
	pixels = new T[numberPixels];
	memcpy(pixels, i.pixels, numberPixels * sizeof(T));
}


template<class T>
Image<T>::Image(const Image<T>* i)
:naxis(i->naxis),numberPixels(i->numberPixels),datatype(i->datatype),anynull(i->anynull),bitpix(i->bitpix),nullvalue(i->nullvalue)
{
	axes[0] = i->axes[0];
	axes[1] = i->axes[1];
	pixels = new T[numberPixels];
	memcpy(pixels, i->pixels, numberPixels * sizeof(T));
}


template<class T>
inline unsigned Image<T>::Xaxes() const{return unsigned(axes[0]);}
template<class T>
inline unsigned Image<T>::Yaxes() const{return unsigned(axes[1]);}
template<class T>
inline unsigned Image<T>::NumberPixels() const{return numberPixels;}
template<class T>
inline Coordinate Image<T>::coordinate (const unsigned j)const{return Coordinate(j%Xaxes(), unsigned(j/Xaxes()));}
template<class T>
inline T& Image<T>::pixel(const unsigned& j)
{return pixels[j];}
template<class T>
inline const T& Image<T>::pixel(const unsigned& j)const
{return pixels[j];}
template<class T>
inline T& Image<T>::pixel(const unsigned& x, const unsigned& y)
{return pixels[x+(y*Xaxes())];}
template<class T>
inline const T& Image<T>::pixel(const unsigned& x, const unsigned& y)const
{return pixels[x+(y*Xaxes())];}
template<class T>
inline T& Image<T>::pixel(const Coordinate& c)
{return pixels[c.x+(c.y*Xaxes())];};
template<class T>
inline const T& Image<T>::pixel(const Coordinate& c)const
{return pixels[c.x+(c.y*Xaxes())];};


template<class T>
int Image<T>::readFitsImage(const std::string& filename)
{

	int   status  = 0;
	fitsfile  *fptr;

	if (fits_open_image(&fptr, filename.c_str(), READONLY, &status))
	{
		cerr<<"Error : opening file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return status;
	} 
	int readStatus = readFitsImageP(fptr);
	if ( fits_close_file(fptr, &status) )
	{
		cerr<<"Error : closing file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
	} 
	return readStatus;

}

template<class T>
int Image<T>::readFitsImageP(fitsfile* fptr)
{
	int   status  = 0;
	if (fits_get_img_param(fptr, 2, &bitpix, &naxis, axes, &status))
	{
		cerr<<"Error : reading image parameters from file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return status;
	}
	
	numberPixels = Xaxes() * Yaxes();
	pixels = new T[numberPixels];

	if (fits_read_img(fptr, datatype, 1, numberPixels,const_cast<T*>(&nullvalue),pixels,&anynull, &status))
	{
		cerr<<"Error : reading image from file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
	} 
	

	return status;
}




template<class T>
int Image<T>::writeFitsImage(const string& filename)
{
	fitsfile *fptr;
	int status = 0;

	remove(filename.c_str());
	
	if (fits_create_file(&fptr, filename.c_str(), &status))
	{
		cerr<<"Error : creating file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return status;
	} 
	int writeSatus = writeFitsImageP(fptr);
	if (fits_close_file(fptr, &status))
	{
		cerr<<"Error : closing file "<<filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
	} 

	return writeSatus;

}

template<class T>
int Image<T>::writeFitsImageP(fitsfile *fptr)
{
	int status = 0;
	//char* comment = NULL;

	//We correct bitpix so we write with the correct type
	
	switch(datatype)
	{
		case TDOUBLE:
			bitpix = DOUBLE_IMG;
			break;
		case TFLOAT:
			bitpix = FLOAT_IMG;
			break;
		case TLONG:
			bitpix = LONGLONG_IMG;
			break;
		case TULONG:
			bitpix = ULONG_IMG;					  //There is no ULONGLONG_IMG
			break;
		case TINT:
			bitpix = LONGLONG_IMG;
			break;
		case TUINT:
			bitpix = ULONG_IMG;
			break;
		case TSHORT:
			bitpix = SHORT_IMG;
			break;
		case TUSHORT:
			bitpix = SHORT_IMG;
			break;
		case TBYTE:
			bitpix = BYTE_IMG;
			break;
		case TSBYTE:
			bitpix = SBYTE_IMG;
			break;
		default:
			bitpix = DOUBLE_IMG;
	}


/*	if ( fits_set_compression_type(fptr, RICE_1, &status) )
	{
		cerr<<"Error : could not set image compression :"<< status <<endl;			
		fits_report_error(stderr, status);
		status = 0;
	}*/
	if ( fits_create_img(fptr,  bitpix, naxis, axes, &status) )
	{
		cerr<<"Error : creating image in file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return status;
	} 

	if ( fits_write_img(fptr, datatype, 1, numberPixels, pixels, &status) )
	{
		cerr<<"Error : writing pixels to file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		return status;
	} 

	return status;

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

	unsigned *manthanDistance = new unsigned[Xaxes() * Yaxes()];
	unsigned maxDistance = Xaxes() + Yaxes();

	for (unsigned y=0; y < Yaxes(); ++y)
	{
		for (unsigned x=0; x < Xaxes(); ++x)
		{
			if (pixel(x,y) == pixelValueToDilate)
			{

				manthanDistance[x+y*Xaxes()] = 0;
			}
			else
			{

				manthanDistance[x+y*Xaxes()] = maxDistance;

				if (x>0) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x-1+y*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x-1+y*Xaxes()]+1);

				if (y>0) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x+(y-1)*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x+(y-1)*Xaxes()]+1);
			}
		}
	}

	for (unsigned y=Yaxes(); y >0; )
	{
		--y;
		for (unsigned x=Xaxes(); x >0; )
		{

			--x;
			if (x+1<Xaxes()) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x+1+y*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x+1+y*Xaxes()]+1);

			if (y+1<Yaxes()) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x+(y+1)*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x+(y+1)*Xaxes()]+1);

		}
	}

	for (unsigned y=0; y < Yaxes(); ++y)
		for (unsigned x=0; x < Xaxes(); ++x)
			if(manthanDistance[x+y*Xaxes()] <= size) pixel(x,y) = pixelValueToDilate;

	delete[] manthanDistance;
	return this;

}


template<class T>
Image<T>* Image<T>::erodeDiamond(unsigned size, T pixelValueToErode)
{

	T fillPixelValue = nullvalue;
	unsigned *manthanDistance = new unsigned[Xaxes() * Yaxes()];
	unsigned maxDistance = Xaxes() + Yaxes();

	for (unsigned y=0; y < Yaxes(); ++y)
	{
		for (unsigned x=0; x < Xaxes(); ++x)
		{
			if (pixel(x,y) != pixelValueToErode)
			{

				manthanDistance[x+y*Xaxes()] = 0;
			}
			else
			{

				manthanDistance[x+y*Xaxes()] = maxDistance;

				if (x>0) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x-1+y*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x-1+y*Xaxes()]+1);

				if (y>0) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x+(y-1)*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x+(y-1)*Xaxes()]+1);
			}
		}
	}

	for (unsigned y=Yaxes(); y >0; )
	{
		--y;
		for (unsigned x=Xaxes(); x >0;)
		{
			--x;
			if (x+1<Xaxes()) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x+1+y*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x+1+y*Xaxes()]+1);

			if (y+1<Xaxes()) manthanDistance[x+y*Xaxes()] = manthanDistance[x+y*Xaxes()] < (manthanDistance[x+(y+1)*Xaxes()]+1) ? manthanDistance[x+y*Xaxes()] : (manthanDistance[x+(y+1)*Xaxes()]+1);

		}
	}

	for (unsigned y=0; y < Yaxes(); ++y)
		for (unsigned x=0; x < Xaxes(); ++x)
			pixel(x,y) = manthanDistance[x+y*Xaxes()] <= size? fillPixelValue : pixelValueToErode;

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
				shape.push_back(y * Xaxes() + x);
	
				
	int j;
	unsigned y, x;
	for(y = size; y < Yaxes() - size; ++y)
	{		
		j = 	y * Xaxes() + size;
		for(x = size; x < Xaxes() - size; ++x)
		{
			
			if(pixels[j] != unsetValue)
			{
				newPixels[j] = pixels[j];
				if(pixels[j-1] == unsetValue || pixels[j+1] == unsetValue || pixels[j-Xaxes()] == unsetValue || pixels[j+Xaxes()] == unsetValue)
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
				shape.push_back(y * Xaxes() + x);
	
				
	int j;
	for(unsigned y = size; y < Yaxes() - size; ++y)
	{		
		j = 	y * Xaxes() + size;
		for(unsigned x = size; x < Xaxes() - size; ++x)
		{
			
			if(pixels[j] != unsetValue && (pixels[j-1] == unsetValue || pixels[j+1] == unsetValue || pixels[j-Xaxes()] == unsetValue || pixels[j+Xaxes()] == unsetValue))
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

	if (min.x >= Xaxes() || min.y >= Yaxes())	  //The box is out of the picture
		return this;
	if (max.x >= Xaxes())
		max.x = Xaxes() - 1;
	if (max.y >= Yaxes())
		max.y = Yaxes() - 1;

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
	max = c.x + size + 1 < Xaxes()  ? c.x + size + 1 : Xaxes() - 1;

	for (unsigned x=min; x <= max; ++x)
	{
		pixel(x,c.y) = color;
	}
	min = c.y < size + 1 ? 0 : c.y - size - 1;
	max = c.y + size + 1 < Yaxes()  ? c.y + size + 1 : Yaxes() - 1;
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
		if(eroded->pixels[j] != eroded->nullvalue)
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
	unsigned size = width / 2;
	T * newPixels = new T[numberPixels];
	fill(newPixels, newPixels + numberPixels, unsetValue);
	vector<unsigned> shape;
	shape.reserve(size*size*3);
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -size; x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y *y) <= size)
				shape.push_back(y * Xaxes() + x);
	
				
	int j;
	for(unsigned y = size; y < Yaxes() - size; ++y)
	{		
		j = 	y * Xaxes() + size;
		for(unsigned x = size; x < Xaxes() - size; ++x)
		{
			
			if(pixels[j] != unsetValue && (pixels[j-1] == unsetValue || pixels[j+1] == unsetValue || pixels[j-Xaxes()] == unsetValue || pixels[j+Xaxes()] == unsetValue))
			{
				newPixels[j] = pixels[j];
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
		if(img->pixels[j] == nullvalue)
			pixels[j] = nullvalue;
		else if (pixels[j] != nullvalue)
			pixels[j] -= img->pixels[j];
		
	}
}


template<class T>
void Image<T>::div(const Image<T> * img)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(img->pixels[j] == nullvalue || img->pixels[j] == 0)
			pixels[j] = nullvalue;
		else if (pixels[j] != nullvalue)
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
	return propagateColor(color, firstPixel.x + firstPixel.y * Xaxes());
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
		pixels[h] = color;
		++numberColoredPixels;
		if(h+1 < numberPixels && pixels[h+1] == setValue)
			pixelList.push_back(h+1);
		if(h+Xaxes() < numberPixels && pixels[h+Xaxes()] == setValue)
			pixelList.push_back(h+Xaxes());
		if(h >= 1 && pixels[h-1] == setValue)
			pixelList.push_back(h-1);
		if(h >= Xaxes() && pixels[h-Xaxes()] == setValue)
			pixelList.push_back(h-Xaxes());

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
				propagateColor(nullvalue, j);
			else
			{
				++color;
				treatedPixels.push_back(j);
			}
		}
	}
	//We have to give back the original color
	for (unsigned t = 0; t < treatedPixels.size(); ++ t)
	{
		propagateColor(setValue, treatedPixels[t]);
	}

	return unsigned(color - 1 - setValue);
}


template<class T>
Image<T>* Image<T>::bitmap(const Image<unsigned>* bitMap, unsigned setValue)
{
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(bitMap->pixel(j) != setValue)
			pixels[j] = nullvalue;
	}
	return this;

}


template<class T>
Real Image<T>::mean() const
{
	Real sum = 0;
	Real card = 0;
	for (unsigned j = 0; j < numberPixels; ++j)
		if(pixels[j] != nullvalue)
	{
		sum += pixels[j];
		++card;
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
		if(pixels[j] != nullvalue)
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
		if(pixels[j] != nullvalue)
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
		if(pixels[j] != nullvalue)
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
	naxis = image->naxis;
	axes[0] = image->axes[0];
	axes[1] = image->axes[1];

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * Xaxes() + x);
		}
	}

	for (unsigned j = 0; j < numberPixels; ++j)
	{
		Real m1 = 0, card = 0;

		for (unsigned n = 0; n < neigboorhood.size(); ++n)
		{
			unsigned neighboor = j + neigboorhood[n];
			if(neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue )
			{
				m1 += image->pixels[neighboor];
				++card;
			}

		}
		if(card == 0)
		{
			pixels[j] = nullvalue;
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
	naxis = image->naxis;
	axes[0] = image->axes[0];
	axes[1] = image->axes[1];

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * Xaxes() + x);
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
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue )
			{
				temp = (image->pixels[neighboor] - meanImage->pixels[j]);
				m2 += temp * temp;
				++card;
			}

		}
		if(card == 0)
		{
			pixels[j] = nullvalue;
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
	naxis = image->naxis;
	axes[0] = image->axes[0];
	axes[1] = image->axes[1];

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * Xaxes() + x);
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
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue )
			{
				temp = (image->pixels[neighboor] - meanImage->pixels[j]);
				m2 += temp * temp;
				m3 += temp * temp * temp;
				++card;
			}

		}
		if(card == 0 || m2 == 0)
		{
			pixels[j] = nullvalue;
		}
		else
		{
			m2 /= card;
			m3 /= card;
			m2 = m2 * m2 * m2;
			if(m2 != 0)
				pixels[j] = T(m3 / sqrt(m2));
			else
				pixels[j] = nullvalue;

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
	naxis = image->naxis;
	axes[0] = image->axes[0];
	axes[1] = image->axes[1];

	vector<unsigned> neigboorhood;
	int Nradius2 = Nradius * Nradius;
	//We construct the neighboorhood disc offsets
	for (int y = -Nradius; y <= Nradius; ++y)
	{
		for (int x = -Nradius; x <= Nradius; ++x)
		{
			if((x * x) + (y * y) <= Nradius2)
				neigboorhood.push_back(y * Xaxes() + x);
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
			if( neighboor >= 0 && neighboor < numberPixels && image->pixels[neighboor] != nullvalue )
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
			pixels[j] = nullvalue;
		}
		else
		{
			m2 /= card;
			m4 /= card;
			if(m2 != 0)
				pixels[j] = T(( m4 / (m2 * m2) ) - 3);
			else
				pixels[j] = nullvalue;
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


/* We create the code for the template class we need
   See constants.h */

template class Image<Real>;
template class Image<unsigned>;

#if PIXELTYPE!=REALTYPE && PIXELTYPE!=TUINT
template class Image<PixelType>;
#endif
