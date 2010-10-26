#include "SunImage.h"

const double PI = 3.14159265358979323846;
const double MIPI = 1.57079632679489661923;
const double BIPI = 6.28318530717958647692;

using namespace std;


SunImage::~SunImage()
{

}

SunImage::SunImage(const long xAxes, const long yAxes)
:Image<PixelType>(xAxes,yAxes),radius(0),wavelength(0),observationTime(0),cdelt1(0),cdelt2(0),median(0),datap01(0), datap95(numeric_limits<PixelType>::max()), exposureTime(0), b0(0)
{
	suncenter.x = xAxes/2;
	suncenter.y = yAxes/2;
}

SunImage::SunImage(const long xAxes, const long yAxes,  const Coordinate suncenter, const double radius, const double cdelt1, const double cdelt2, const double wavelength)
:Image<PixelType>(xAxes,yAxes),radius(radius),wavelength(wavelength),observationTime(0),suncenter(suncenter),cdelt1(cdelt1),cdelt2(cdelt2),median(0),datap01(0), datap95(numeric_limits<PixelType>::max()), exposureTime(0), b0(0)
{

}


SunImage::SunImage(const string& filename)
:Image<PixelType>(),median(0),datap01(0), datap95(numeric_limits<PixelType>::max()), exposureTime(0)
{
	readFitsImage(filename);
}


SunImage::SunImage(const SunImage& i)
:Image<PixelType>(i),radius(i.radius),wavelength(i.wavelength),observationTime(i.observationTime),suncenter(i.suncenter),cdelt1(i.cdelt1),cdelt2(i.cdelt2),median(i.median),datap01(i.datap01),datap95(i.datap95),date_obs(i.date_obs),exposureTime(i.exposureTime),b0(i.b0),header(i.header)
{
}


SunImage::SunImage(const SunImage* i)
:Image<PixelType>(i),radius(i->radius),wavelength(i->wavelength),observationTime(i->observationTime),suncenter(i->suncenter),cdelt1(i->cdelt1),cdelt2(i->cdelt2),median(i->median),datap01(i->datap01),datap95(i->datap95), date_obs(i->date_obs), exposureTime(i->exposureTime),b0(i->b0),header(i->header)
{

}


int SunImage::readFitsImageP(fitsfile* fptr)
{
	int status = Image<PixelType>::readFitsImageP(fptr);
	if(status)
		return status;

	readHeader(fptr);
	#if DEBUG >= 1
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullvalue_;
		}
	}
	#endif
	return 0;
}


int SunImage::writeFitsImageP(fitsfile* fptr)
{
	int status = Image<PixelType>::writeFitsImageP(fptr);
	if(status)
		return status;
	
	writeHeader(fptr);
	if (fits_write_date(fptr, &status) )
	{
		cerr<<"Error : writing date to file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		status = 0;
	} 

	return status;
}

void SunImage::readHeader(fitsfile* fptr)
{
	header.readKeywords(fptr);
}

void SunImage::writeHeader(fitsfile* fptr)
{
	header.writeKeywords(fptr);
}


double SunImage::Wavelength() const
{return wavelength;}
double SunImage::Median() const
{return median;}
Coordinate SunImage::SunCenter() const
{return suncenter;}
double SunImage::SunRadius() const
{return radius;}
double SunImage::B0() const
{return b0;}

time_t SunImage::ObservationTime()const
{
	if(observationTime != 0)
		return observationTime;
	else
	{
		//We convert observationTime to time
		int status = 0;
		tm time;
		double seconds;
		int month;
		if (fits_str2time(const_cast<char *>(date_obs.c_str()), &(time.tm_year), &(month), &(time.tm_mday), &(time.tm_hour), &(time.tm_min), &seconds, &status))
		{
			cerr<<"Error converting date_obs to time : "<< status <<endl;
			fits_report_error(stderr, status);
		}
		else
		{
			time.tm_sec = int(seconds);
			time.tm_mon = month -1;	//Because stupid c++ standard lib has the month going from 0-11
			time.tm_isdst = 0;
		}
		return timegm(&time);
	}


}
string SunImage::ObservationDate() const
{return date_obs;}
double SunImage::PixelArea() const
{return cdelt1 * cdelt2;}
unsigned SunImage::numberValidPixelsEstimate() const
{return unsigned(PI*radius*radius);}


string nextStep(string& preprocessingList)
{
	size_t pos = preprocessingList.find(',');
	string preprocessingStep;
	if (pos != string::npos)
	{
		preprocessingStep = preprocessingList.substr(0, pos);
		preprocessingList = preprocessingList.substr(pos + 1);
	}
	else
	{
		preprocessingStep = preprocessingList;
		preprocessingList.clear();
	}
	return preprocessingStep;
}

void SunImage::preprocessing(string preprocessingList, const Real radiusRatio)
{

	string preprocessingStep = nextStep(preprocessingList);
	while(!preprocessingStep.empty())
	{
		if(preprocessingStep == "NAR")
		{
			nullifyAboveRadius(radiusRatio);
		}
		else if( preprocessingStep == "ALC")
		{
			//The successive operations ALC + DivMode and ALC + DivMedian have been optimized
			preprocessingStep = nextStep(preprocessingList);
			if( preprocessingStep == "DivMedian")
			{
				ALCDivMedian(radiusRatio, MINRADIUS());
			}
			else if( preprocessingStep == "DivMode")
			{
				ALCDivMode(radiusRatio, MINRADIUS());
			}
			else
			{
				annulusLimbCorrection(radiusRatio, MINRADIUS());
				preprocessingList = preprocessingStep + "," + preprocessingList;
			}
		}
		
		else if( preprocessingStep == "DivMedian")
		{
			if(median == 0)
			{
				cerr<<"Error during preprocessing step DivMedian : median = 0."<<endl;
				exit(EXIT_FAILURE);
			}
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue_)
					pixels[j] = pixels[j] / median;
			}
		}	
		else if( preprocessingStep == "DivMode")
		{
			if(mode == 0)
			{
				cerr<<"Error during preprocessing step DivMode : mode = 0."<<endl;
				exit(EXIT_FAILURE);
			}
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue_)
					pixels[j] = pixels[j] / mode;
			}
		}	
		else if( preprocessingStep == "TakeSqrt")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue_)
					pixels[j] = pixels[j] >= 0 ? sqrt(pixels[j]) : nullvalue_;
			}
		}	
		else if( preprocessingStep == "TakeLog")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue_)
					pixels[j] = pixels[j] > 0 ? log(pixels[j]) : nullvalue_;
			}
		}
		else if( preprocessingStep == "DivExpTime")
		{
			if(exposureTime == 0)
			{
				cerr<<"Error during preprocessing step DivExpTime : exposureTime = 0."<<endl;
				exit(EXIT_FAILURE);
			}
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue_)
					pixels[j] = pixels[j] / exposureTime;
			}
		}
		else
		{
			cerr<<"Error during preprocessing : Unknown preprocessing step "<<preprocessingStep<<endl;
		}
		
		preprocessingStep = nextStep(preprocessingList);
			
	}
	
}


void SunImage::nullifyAboveRadius(const Real radiusRatio)
{
	Real radius2 = radiusRatio*radiusRatio*radius*radius;
	for (unsigned y=0; y < Yaxes(); ++y)
	{
		for (unsigned x=0; x < Xaxes(); ++x)
		{
			if ((x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y)> radius2)
				pixel(x,y) = nullvalue_;
		}
	}
}


/* Function that returns the percentage of correction necessary for an annulus, given the percentage of the annulus radius to the radius of the sun */

/*Method proposed by Benjamin
No correction before r1 and after r2, Full correction between r3 and r4,
progressive correction following the ascending phase of the sine between r1 and r2
progressive correction following the descending phase of the sine between r3 and r4 */

inline Real SunImage::percentCorrection(const Real r)const
{

	const Real r1 = SINE_CORR_R1 / 100.;
	const Real r2 = SINE_CORR_R2 / 100.;
	const Real r3 = SINE_CORR_R3 / 100.;
	const Real r4 = SINE_CORR_R4 / 100.;
	if (r <= r1 || r >= r4)
		return 0;
	else if (r >= r2 && r <= r3)
		return 1;
	else if (r <= r2)
	{
		Real T = - 2*(r1-r2);
		Real phi = MIPI*(r1+r2)/(r1-r2);
		return (sin((BIPI/T)*r + phi) + 1)/2;
	}
	else // (r => r3)
	{
		Real T = 2*(r3-r4);
		Real phi = - MIPI*(r3+r4)/(r3-r4);
		return (sin((BIPI/T)*r + phi) + 1)/2;
	}

}

/* Older methods kept for historical reasons

// Method proposed by Vincent
// No Correction before r1, Full correction after r1 
inline Real SunImage::percentCorrection(const Real r)const
{
	const Real r1 = DISCRETE_CORR_R1 / 100.;
	if (r < r1)
		return 0;
	else
		return 1;
}




// Method proposed by Cis
// No Correction before r1, Full correction after r2, progressive correction in between
inline Real SunImage::percentCorrection(const Real r)const
{

	const Real r1 = SLOPE_CORR_R1 / 100.;
	const Real r2 = SLOPE_CORR_R2 / 100.;
	if (r < r1)
		return 0;
	else if (r >= r1 && r <= r2)
	{
		return (r - r1)/(r2 - r1);
	}
	else
		return 1;

}


//2nd Method of Cis
inline Real SunImage::percentCorrection(const Real r)const
{
	return exp(-0.01 * pow(100 * (r-1),4));
}

*/

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */

#define ELEM_SWAP(a,b) { register PixelType t=(a);(a)=(b);(b)=t; }

PixelType quick_select(vector<PixelType>& arr, int n)
{
	int low, high ;
	int median;
	int middle, ll, hh;

	low = 0 ; high = n-1 ; median = (low + high) / 2;
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
		middle = (low + high) / 2;
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

void SunImage::annulusLimbCorrection(Real maxLimbRadius, Real minLimbRadius)
{						  
	minLimbRadius *= radius;
	maxLimbRadius *= radius;
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = 1.0;	//This means that we consider the width of an annulus to be the size of a third of a pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	PixelType* pixelValue;
	Real pixelRadius2 = 0;
	unsigned indice = 0;
	if (median == 0) // I don't know the median value yet
	{
		vector<PixelType> onDiscList;
		onDiscList.reserve(numberValidPixelsEstimate());
		for (unsigned y=0; y < Yaxes(); ++y)
		{
			for (unsigned x=0; x < Xaxes(); ++x)
			{

				pixelValue = &pixel(x,y);

				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);

					if (pixelRadius2 <=  minLimbRadius2)
					{
						onDiscList.push_back((*pixelValue));
					}
					else if (pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) = annulusMean.at(indice) + (*pixelValue);
						annulusNbrPixels.at(indice) = annulusNbrPixels.at(indice) + 1;
					}

				}
			}
		}

		median = quick_select(onDiscList, onDiscList.size());
		#if DEBUG >= 3
		cout<<"Image preprocessing found median: "<<median<<endl;
		#endif
	}
	else
	{
		for (unsigned y=0; y < Yaxes(); ++y)
		{
			for (unsigned x=0; x < Xaxes(); ++x)
			{
				pixelValue = &pixel(x,y);
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);

					if (pixelRadius2 >  minLimbRadius2 && pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) = annulusMean.at(indice) + (*pixelValue);
						annulusNbrPixels.at(indice) = annulusNbrPixels.at(indice) + 1;
					}

				}
			}
		}
	}

	// We calculate the mean value of each annulus
	for (unsigned i=0; i<annulusMean.size(); ++i)
	{
		if(annulusNbrPixels.at(i)>0)
			annulusMean.at(i) = annulusMean.at(i) / Real(annulusNbrPixels.at(i));
	}
	// We correct the limb
	for (unsigned y=0; y < Yaxes(); ++y)
	{
		for (unsigned x=0; x < Xaxes(); ++x)
		{
			pixelValue = &pixel(x,y);
			if ((*pixelValue) != nullvalue_)
			{
				pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue_;
				}
				else if (pixelRadius2 > minLimbRadius2)	  //We correct the limb
				{
					Real pixelRadius = sqrt(pixelRadius2);
					Real fraction = percentCorrection(pixelRadius/radius);
					indice = unsigned((pixelRadius-minLimbRadius)/deltaR);
					(*pixelValue) = (1. - fraction) * (*pixelValue) + (fraction * (*pixelValue) * median) / annulusMean.at(indice);

				}

			}
		}
	}

}

void SunImage::ALCDivMedian(Real maxLimbRadius, Real minLimbRadius)
{						  
	minLimbRadius *= radius;
	maxLimbRadius *= radius;
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = 1.0;	//This means that we consider the width of an annulus to be the size of a third of a pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	PixelType* pixelValue;
	Real pixelRadius2 = 0;
	unsigned indice = 0;
	if (median == 0) // I don't know the median value yet
	{
		vector<PixelType> onDiscList;
		onDiscList.reserve(numberValidPixelsEstimate());
		for (unsigned y=0; y < Yaxes(); ++y)
		{
			for (unsigned x=0; x < Xaxes(); ++x)
			{

				pixelValue = &pixel(x,y);
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);

					if (pixelRadius2 <=  minLimbRadius2)
					{
						onDiscList.push_back((*pixelValue));
					}
					else if (pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) = annulusMean.at(indice) + (*pixelValue);
						annulusNbrPixels.at(indice) = annulusNbrPixels.at(indice) + 1;
					}

				}
			}
		}

		median = quick_select(onDiscList, onDiscList.size());
		#if DEBUG >= 3
		cout<<"Image preprocessing found median: "<<median<<endl;
		#endif
	}
	else
	{
		for (unsigned y=0; y < Yaxes(); ++y)
		{
			for (unsigned x=0; x < Xaxes(); ++x)
			{
				pixelValue = &pixel(x,y);
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);

					if (pixelRadius2 >  minLimbRadius2 && pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) = annulusMean.at(indice) + (*pixelValue);
						annulusNbrPixels.at(indice) = annulusNbrPixels.at(indice) + 1;
					}

				}
			}
		}
	}

	// We calculate the mean value of each annulus
	for (unsigned i=0; i<annulusMean.size(); ++i)
	{
		if(annulusNbrPixels.at(i)>0)
			annulusMean.at(i) = annulusMean.at(i) / Real(annulusNbrPixels.at(i));
	}
	// We correct the limb
	for (unsigned y=0; y < Yaxes(); ++y)
	{
		for (unsigned x=0; x < Xaxes(); ++x)
		{
			pixelValue = &pixel(x,y);
			if ((*pixelValue) != nullvalue_)
			{
				pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue_;
				}
				else if (pixelRadius2 > minLimbRadius2)	  //We correct the limb
				{
					Real pixelRadius = sqrt(pixelRadius2);
					Real fraction = percentCorrection(pixelRadius/radius);
					indice = unsigned((pixelRadius-minLimbRadius)/deltaR);
					(*pixelValue) = (1. - fraction) * (*pixelValue) / median + (fraction * (*pixelValue)) / annulusMean.at(indice);

				}
				else
				{
					(*pixelValue) = (*pixelValue) / median;
				}

			}
		}
	}

}

void SunImage::ALCDivMode(Real maxLimbRadius, Real minLimbRadius)
{						  
	minLimbRadius *= radius;
	maxLimbRadius *= radius;
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = 1.0;	//This means that we consider the width of an annulus to be the size of a third of a pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	// I need to construct a quick histogram to find the mode
	const Real binSize = 5;
	vector<unsigned> histo(1024, 0);

	PixelType* pixelValue;
	Real pixelRadius2 = 0;
	unsigned indice = 0;
	if (median == 0) // I don't know the median value yet
	{
		vector<PixelType> onDiscList;
		onDiscList.reserve(numberValidPixelsEstimate());
		for (unsigned y=0; y < Yaxes(); ++y)
		{
			for (unsigned x=0; x < Xaxes(); ++x)
			{

				pixelValue = &pixel(x,y);
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);

					if (pixelRadius2 <=  minLimbRadius2)
					{
						onDiscList.push_back((*pixelValue));
						unsigned bin = unsigned((*pixelValue)/binSize);
						if (bin >= histo.size())
							histo.resize(bin + 1024, 0);
						++histo[bin];
					}
					else if (pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) = annulusMean.at(indice) + (*pixelValue);
						annulusNbrPixels.at(indice) = annulusNbrPixels.at(indice) + 1;
					}

				}
			}
		}

		median = quick_select(onDiscList, onDiscList.size());
		#if DEBUG >= 3
		cout<<"Image preprocessing found median: "<<median<<endl;
		#endif
	}
	else
	{
		for (unsigned y=0; y < Yaxes(); ++y)
		{
			for (unsigned x=0; x < Xaxes(); ++x)
			{
				pixelValue = &pixel(x,y);
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);

					if (pixelRadius2 <=  minLimbRadius2)
					{
						unsigned bin = unsigned((*pixelValue)/binSize);
						if (bin >= histo.size())
							histo.resize(bin + 1024, 0);
						++histo[bin];
					}
					else if (pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) = annulusMean.at(indice) + (*pixelValue);
						annulusNbrPixels.at(indice) = annulusNbrPixels.at(indice) + 1;
					}

				}
			}
		}
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
	#if DEBUG >= 3
	cout<<"Image preprocessing found mode: "<<mode<<endl;
	#endif

	// We calculate the mean value of each annulus
	for (unsigned i=0; i<annulusMean.size(); ++i)
	{
		if(annulusNbrPixels.at(i)>0)
			annulusMean.at(i) = annulusMean.at(i) / Real(annulusNbrPixels.at(i));
	}

	for (unsigned y=0; y < Yaxes(); ++y)
	{
		for (unsigned x=0; x < Xaxes(); ++x)
		{
			pixelValue = &pixel(x,y);
			if ((*pixelValue) != nullvalue_)
			{
				pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue_;
				}
				else
				{
					if (pixelRadius2 > minLimbRadius2)	  //We correct the limb
					{
						Real pixelRadius = sqrt(pixelRadius2);
						Real fraction = percentCorrection(pixelRadius/radius);
						indice = unsigned((pixelRadius-minLimbRadius)/deltaR);
						(*pixelValue) = (1. - fraction) * (*pixelValue) + (fraction * (*pixelValue) * median) / annulusMean.at(indice);

					}
					(*pixelValue) /= mode;
				}

			}
		}
	}

}


void SunImage::recenter(const Coordinate& newCenter)
{
	int delta = (suncenter.x - newCenter.x) + (suncenter.y - newCenter.y) * Xaxes();
	if(delta < 0)
	{
		memmove(pixels - delta, pixels, (numberPixels + delta + 1) * sizeof(PixelType));
		fill(pixels, pixels-delta, nullvalue_);
	}
	else if (delta > 0)
	{
		memmove(pixels, pixels + delta, (numberPixels - delta + 1) * sizeof(PixelType));
		fill(pixels + numberPixels - delta, pixels + numberPixels, nullvalue_);
	}
	suncenter = newCenter;
}


void SunImage::copyKeywords(const SunImage* i)
{

	radius = i->radius;
	suncenter = i->suncenter;
	wavelength = i->wavelength;
	median = i->median;
	mode = i->mode;
	observationTime = i->observationTime;
	datap01 = i->datap01;
	datap95 = i->datap95;
	cdelt1 = i->cdelt1;
	cdelt2 = i->cdelt2;
	date_obs =  i->date_obs;
	exposureTime = i->exposureTime;
	b0 = i->b0;
	
}

// Calculates the differential solar rotation speed for a given pixel
// Formula coming from Rotation of Doppler features in the solar photosphere by	Snodgrass, Herschel B. and Ulrich, Roger K.
inline Real SunImage::angularSpeed(Real latitude) const
{
	const Real A = 14.713;
	const Real B = -2.396;
	const Real C =  -1.787;
	latitude *= latitude;
	Real result = A + B * latitude + C * latitude * latitude;
	return result * (PI / (24. * 3600. * 180.));
}

// Rotate an image by delta_t seconds
void SunImage::rotate(const int delta_t)
{
	PixelType* new_pixels = new PixelType[numberPixels];
	fill(new_pixels, new_pixels + numberPixels, nullvalue_);
	
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);

	//Compute for each point of the new image what is the equivalent in the current image
	
	unsigned j = 0;
	for(Real y = 0; y < Yaxes(); ++y)
	{
		Real ry = (y - suncenter.y) / radius;
		for(Real x = 0; x < Xaxes(); ++x)
		{
			Real rx = (x - suncenter.x) / radius;
			Real z = 1. - rx * rx - ry * ry;
			// If we are within the disk
			if(z >= 0)
			{ 
				z = sqrt(z);
				Real cur_latitude = asin(cos_b0 * ry + sin_b0 * z); // The new latitude equals the current latitude
				Real new_longitude = atan(rx / (cos_b0 * z - sin_b0 * ry));
				Real cur_longitude = new_longitude - delta_t * angularSpeed(cur_latitude);
				// We need to be sure that the current longitude is visible on the disk
				if(cur_longitude > -MIPI &&  cur_longitude < MIPI)
				{
					unsigned cur_x = suncenter.x + radius * (cos(cur_latitude) * sin(cur_longitude));
					unsigned cur_y = suncenter.y + radius * (cos_b0 * sin(cur_latitude) - sin_b0 * cos(cur_latitude) * cos(cur_longitude));
					new_pixels[j] = pixel(cur_x, cur_y);
				}
			}
 			++j;
		}
	}
	delete pixels;
	pixels = new_pixels;
}

// Return a new image = to teh image rotated to be comparable to img
SunImage* SunImage::rotated_like(const SunImage* img) const
{
	SunImage * rotated = new SunImage(img->Xaxes(), img->Yaxes());
	rotated->copyKeywords(img);
	rotated->nullvalue_ = img->nullvalue_;
	rotated->zero(nullvalue_);
	
	int delta_t = difftime(img->ObservationTime(),ObservationTime());
	
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);
	Real cos_newb0 = cos(rotated->b0);
	Real sin_newb0 = sin(rotated->b0);
	
	//Compute for each point of the new image what is the equivalent in the current image
	for(Real new_y = 0; new_y < rotated->Yaxes(); ++new_y)
	{
		Real ry = (new_y - rotated->suncenter.y) / rotated->radius;
		for(Real new_x = 0; new_x < rotated->Xaxes(); ++new_x)
		{
			Real rx = (new_x - rotated->suncenter.x) / rotated->radius;
			Real z = 1. - rx * rx - ry * ry;
			// If we are within the disk
			if(z >= 0)
			{ 
				z = sqrt(z);
				Real cur_latitude = asin(cos_newb0 * ry + sin_newb0 * z); // The new latitude equals the current latitude
				Real new_longitude = atan(rx / (cos_newb0 * z - sin_newb0 * ry));
				Real cur_longitude = new_longitude - delta_t * angularSpeed(cur_latitude);
				// We need to be sure that the current longitude is visible on the disk
				if(cur_longitude > -MIPI &&  cur_longitude < MIPI)
				{
					unsigned cur_x = suncenter.x + radius * (cos(cur_latitude) * sin(cur_longitude));
					unsigned cur_y = suncenter.y + radius * (cos_b0 * sin(cur_latitude) - sin_b0 * cos(cur_latitude) * cos(cur_longitude));
					rotated->pixel(new_x,new_y) = pixel(cur_x, cur_y);
				}
 			}
 			
		}
	}

	return rotated;

}

// Rotate the image to be comparable to img
void SunImage::rotate_like(const SunImage* img)
{
	SunImage * rotated = this->rotated_like(img);
	axes[0] = rotated->axes[0];
	axes[1] = rotated->axes[1];
	numberPixels = rotated->numberPixels;
	nullvalue_ = rotated->nullvalue_;
	delete pixels;
	pixels = rotated->pixels;

}

// Shift a point in the image by delta_t seconds
Coordinate SunImage::shift(const Coordinate c, const int delta_t) const
{

	Real latitude, longitude;
	longlat(c, longitude, latitude);
	longitude += delta_t * angularSpeed(latitude);
	
	Coordinate newc;
	newc.x = suncenter.x + radius * (cos(latitude) * sin(longitude));
	newc.y = suncenter.y + radius * (sin(latitude) * cos(b0) - cos(latitude) * cos(longitude) * sin(b0));
	
	return newc;

}

// Shift a point in the image to the equivalant point in img
Coordinate SunImage::shift_like(const Coordinate c, const SunImage* img) const
{

	int delta_t = difftime(img->ObservationTime(),ObservationTime());
	Real latitude, longitude;
	longlat(c, longitude, latitude);
	longitude += delta_t * angularSpeed(latitude);
	
	Coordinate newc;
	newc.x = img->suncenter.x + img->radius * (cos(latitude) * sin(longitude));
	newc.y = img->suncenter.y + img->radius * (sin(latitude) * cos(img->b0) - cos(latitude) * cos(longitude) * sin(img->b0));
	
	return newc;

}


void SunImage::longlat(const Coordinate c, Real& longitude, Real& latitude) const
{
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);
	Real rx = c.x ; 
	Real ry = c.y;
	rx = (rx - suncenter.x) / radius; 
	ry = (ry - suncenter.y) / radius;
	Real z = 1. - rx * rx - ry * ry;
	if(z >= 0)
	{ 
		z = sqrt(z);
		latitude = asin(cos_b0 * ry + sin_b0 * z);
		longitude = atan(rx / (cos_b0 * z - sin_b0 * ry));
	}
	else
	{
		latitude = numeric_limits<PixelType>::quiet_NaN();
		longitude = numeric_limits<PixelType>::quiet_NaN();
	}
}

void SunImage::longlat_map(vector<Real>& longitude_map, vector<Real>& latitude_map) const
{

	longitude_map.resize(numberPixels, 0);
	latitude_map.resize(numberPixels, 0);
	
	Real cos_b0 = cos(b0);
	Real sin_b0 = sin(b0);
	
	unsigned j = 0;
	for(Real y = 0; y < Yaxes(); ++y)
	{
		Real ry = (y - suncenter.y) / radius;
		for(Real x = 0; x < Xaxes(); ++x)
		{
			Real rx = (x - suncenter.x) / radius;
			Real z = 1. - rx * rx - ry * ry;
			if(z >= 0)
			{ 
				z = sqrt(z);
				latitude_map[j] = asin(cos_b0 * ry + sin_b0 * z);
				longitude_map[j] = atan(rx / (cos_b0 * z - sin_b0 * ry));
 			}
 			else
			{
				latitude_map[j] = numeric_limits<PixelType>::quiet_NaN();
				longitude_map[j] = numeric_limits<PixelType>::quiet_NaN();
			}
			++j;
		}
	}


}
