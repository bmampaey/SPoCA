#include "SunImage.h"

using namespace std;


SunImage::~SunImage()
{
	for (unsigned k = 0; k < header.size(); ++k)
		delete header[k];
}

SunImage::SunImage(const long xAxes, const long yAxes, const double radius, const double wavelength)
:Image<PixelType>(xAxes,yAxes),radius(radius),wavelength(wavelength),observationTime(0),median(0),datap01(0), datap95(numeric_limits<PixelType>::max()), exposureTime(0)
{
	suncenter.x = xAxes/2;
	suncenter.y = yAxes/2;
	cdelt[0] = cdelt[1] = 1;
}


SunImage::SunImage(const string& filename)
:Image<PixelType>(),median(0),datap01(0), datap95(numeric_limits<PixelType>::max()), exposureTime(0)
{
	readFitsImage(filename);
}


SunImage::SunImage(const SunImage& i)
:Image<PixelType>(i),radius(i.radius),wavelength(i.wavelength),observationTime(i.observationTime),suncenter(i.suncenter),median(i.median),datap01(i.datap01),datap95(i.datap95), exposureTime(i.exposureTime)
{
	strncpy (date_obs, i.date_obs, 80);
	cdelt[0] = i.cdelt[0];
	cdelt[1] = i.cdelt[1];
	header.resize(i.header.size(), NULL);
	for (unsigned k = 0; k < i.header.size(); ++k)
	{
		header[k] = new char[81];
		strncpy (header[k], i.header[k], 80);
	}
}


SunImage::SunImage(const SunImage* i)
:Image<PixelType>(i),radius(i->radius),wavelength(i->wavelength),observationTime(i->observationTime),suncenter(i->suncenter),median(i->median),datap01(i->datap01),datap95(i->datap95), exposureTime(i->exposureTime)
{
	strncpy (date_obs, i->date_obs, 80);
	cdelt[0] = i->cdelt[0];
	cdelt[1] = i->cdelt[1];
	header.resize(i->header.size(), NULL);
	for (unsigned k = 0; k < i->header.size(); ++k)
	{
		header[k] = new char[81];
		strncpy (header[k], i->header[k], 80);
	}
}


int SunImage::readFitsImageP(fitsfile* fptr)
{
	int   status  = 0;
	char* comment = NULL;					  /**<By specifying NULL we say that we don't want the comments	*/

	status = Image<PixelType>::readFitsImageP(fptr);
	if(status)
		return status;

	if (fits_read_key(fptr, TDOUBLE, "WAVELNTH", &wavelength,comment, &status))
	{
		cerr<<"Error reading key WAVELNTH from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	if (fits_read_key(fptr, TINT, "CRPIX1", &(suncenter.x),comment, &status))
	{
		cerr<<"Error reading key CRPIX1 from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	if (fits_read_key(fptr, TINT, "CRPIX2", &(suncenter.y), comment, &status))
	{
		cerr<<"Error reading key CRPIX2 from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	if (fits_read_key(fptr, TDOUBLE, "CDELT1", &(cdelt[0]), comment, &status))
	{
		cerr<<"Error reading key CDELT1 from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	if (fits_read_key(fptr, TDOUBLE, "CDELT2", &(cdelt[1]), comment, &status))
	{
		cerr<<"Error reading key CDELT2 from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	
		
	//The date of observation can be defined as DATE_OBS ou DATE-OBS
	if (fits_read_key(fptr, TSTRING, "DATE-OBS", date_obs, comment, &status))
	{
		
		status = 0;
		if (fits_read_key(fptr, TSTRING, "DATE_OBS", date_obs, comment, &status))
		{

			cerr<<"Error reading key DATE-OBS from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
			fits_report_error(stderr, status);
			status = 0;
		}
		else
		{
			//Sometimes the date is appended with a z
			char * letter;
			if((letter = strpbrk (date_obs, "zZ")))
				*letter = '\0';
		}
	}
	//We convert observationTime to time
	tm time;
	double seconds;
	int month;
	if (fits_str2time(date_obs, &(time.tm_year), &(month), &(time.tm_mday), &(time.tm_hour), &(time.tm_min), &seconds, &status))
		cerr<<"Error converting date_obs to time : "<< status <<endl;
	time.tm_sec = int(seconds);
	time.tm_mon = month -1;	//Because stupid c++ standard lib has the month going from 0-11
	time.tm_isdst = 0;
	observationTime = timegm(&time);
		


	// We save all keywords for future usage
	status = 0;
	char record[81];
	const char* inclist[] = {"*"};
	const char* exclist[] = {"SIMPLE", "BITPIX", "NAXIS*", "EXTEND", "Z*", "XTENSION", "TTYPE1", "TFORM1", "PCOUNT", "GCOUNT", "TFIELDS"};
	//We first need to reset the fptr to the beginning
	if( fits_read_record (fptr, 0, record, &status))
	{
		cerr<<"Error reseting the fits pointer to the beginning of the header for file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		status = KEY_NO_EXIST;
	}
	while(status != KEY_NO_EXIST)
	{
		status = 0;
		if(fits_find_nextkey(fptr, const_cast<char**>(inclist), sizeof(inclist)/sizeof(char *), const_cast<char**>(exclist), sizeof(exclist)/sizeof(char *), record, &status))
		{
			if(status != KEY_NO_EXIST)
			{
				cerr<<"Error reading keyword from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
				fits_report_error(stderr, status);
				status = KEY_NO_EXIST;
			}
		}
		else
		{
			header.push_back(strdup(record));
		}
	} 
	
	#if defined(DEBUG) && DEBUG >= 1
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullvalue;
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
		
	for (unsigned k = 0; k < header.size(); ++k)
	{
		if(fits_write_record(fptr, header[k], &status))
		{
			cerr<<"Error : writing keyword to file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
			fits_report_error(stderr, status);
			status = 0;
		} 
	}
	
	if (fits_write_date(fptr, &status) )
	{
		cerr<<"Error : writing date to file "<<fptr->Fptr->filename<<" :"<< status <<endl;			
		fits_report_error(stderr, status);
		status = 0;
	} 

	return status;
}



double SunImage::Wavelength() const
{return wavelength;}
double SunImage::Median() const
{return median;}
Coordinate SunImage::SunCenter() const
{return suncenter;}
double SunImage::SunRadius() const
{return radius;}
time_t SunImage::ObservationTime() const
{return observationTime;}
string SunImage::ObservationDate() const
{return string(date_obs);}
double SunImage::PixelArea() const
{return cdelt[0] * cdelt[1];}
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
				if (pixels[j] != nullvalue)
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
				if (pixels[j] != nullvalue)
					pixels[j] = pixels[j] / mode;
			}
		}	
		else if( preprocessingStep == "TakeSqrt")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue)
					pixels[j] = pixels[j] >= 0 ? sqrt(pixels[j]) : nullvalue;
			}
		}	
		else if( preprocessingStep == "TakeLog")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue)
					pixels[j] = pixels[j] > 0 ? log(pixels[j]) : nullvalue;
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
				if (pixels[j] != nullvalue)
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
				pixel(x,y) = nullvalue;
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

				if ((*pixelValue) != nullvalue)
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
		#if defined(DEBUG) && DEBUG >= 3
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
				if ((*pixelValue) != nullvalue)
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
			if ((*pixelValue) != nullvalue)
			{
				pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue;
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
				if ((*pixelValue) != nullvalue)
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
		#if defined(DEBUG) && DEBUG >= 3
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
				if ((*pixelValue) != nullvalue)
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
			if ((*pixelValue) != nullvalue)
			{
				pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue;
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
				if ((*pixelValue) != nullvalue)
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
		#if defined(DEBUG) && DEBUG >= 3
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
				if ((*pixelValue) != nullvalue)
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
	#if defined(DEBUG) && DEBUG >= 3
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
			if ((*pixelValue) != nullvalue)
			{
				pixelRadius2 = (x-suncenter.x)*(x-suncenter.x) + (y-suncenter.y)*(y-suncenter.y);
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue;
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
		fill(pixels, pixels-delta, nullvalue);
	}
	else if (delta > 0)
	{
		memmove(pixels, pixels + delta, (numberPixels - delta + 1) * sizeof(PixelType));
		fill(pixels + numberPixels - delta, pixels + numberPixels, nullvalue);
	}
	suncenter = newCenter;
}


void SunImage::copyKeywords(const SunImage* i)
{
	radius = i->radius;
	suncenter = i->suncenter;
	wavelength = i->wavelength;
	median = i->median;
	observationTime = i->observationTime;
	datap01=i->datap01;
	datap95=i->datap95;
	cdelt[0]=i->cdelt[0];
	cdelt[1]=i->cdelt[1];
}

#if defined(AGGREGATE_DILATE)
SunImage* SunImage::blobsIntoAR ()
{

	//We agregate the blobs together by dilation of 31.44 arcsec (== 12 pixel EIT)
	unsigned dilateFactor = unsigned(31.44 / cdelt[0]);
	this->dilateCircular(dilateFactor,0);
	
	this->colorizeConnectedComponents(0);
	
	return this;
}

#else

SunImage* SunImage::blobsIntoAR ()
{

	//We create  a map by dilation of 31.44 arcsec (== 12 pixel EIT)
	unsigned dilateFactor = unsigned(31.44 / cdelt[0]);
	SunImage* dilated = new SunImage(this);
	dilated->dilateCircular(dilateFactor,0);
	dilated->colorizeConnectedComponents(0);
	
	//We color the blobs using the dilated map 
	for (unsigned j=0; j < numberPixels; ++j)
	{
		if (pixels[j] != nullvalue)
			pixels[j] = dilated->pixel(j);
	}
	delete dilated;
	
	return this;
}

#endif




//calculates the differential solar rotation speed for a given pixel
// Formula coming from Wikipedia, should be verified
inline Real SunImage::angularSpeed(Real latitude)
{
	const Real A = 14.713;
	const Real B = -2.396;
	const Real C =  -1.787;
	latitude *= latitude;
	Real result = A + B * latitude + C * latitude * latitude;
	return result * (PI / (24. * 3600. * 180.));
}


inline unsigned SunImage::newPos(Real x, Real y, const Real t)
{
	x = suncenter.x - x;
	y = suncenter.y - y;
	Real latitude = y / radius;
	Real omega = angularSpeed(latitude);
	Real alpha = omega * t;
	Real r = 1;
	Real teta = asin(x / sqrt(x * x + r * r));
	Real delta = r * sin(teta - alpha);
	return suncenter.x - delta;
}


SunImage* SunImage::rotate(const unsigned t)
{
	SunImage * img = new SunImage(axes[0], axes[1], radius, wavelength);
	vector<Real> radiusAtLatitude (radius + 1, 0);
	vector<unsigned> left ( radius, Xaxes());
	vector<unsigned> right ( radius, 0);

	Image<Real> Phi (Xaxes(), radius + 1);
	Phi.zero(MIPI);
	for (unsigned latitude = 0; latitude < radius; ++latitude)
	{
		radiusAtLatitude[latitude] = sqrt((radius * radius) - (latitude * latitude));
		left[latitude] = suncenter.x - radiusAtLatitude[latitude] ;
		right[latitude] = suncenter.x + radiusAtLatitude[latitude];
		for (int longitude = floor(1. - radiusAtLatitude[latitude]); longitude <=  ceil(radiusAtLatitude[latitude] - 1.); ++longitude)
		{
			Real r1 = Real(latitude) / Real(radius);
			Phi.pixel(suncenter.x + longitude,latitude)=asin(Real(longitude) /(sqrt(1 - r1 * r1) * Real(radius)));
		}
	}
	Phi.writeFitsImage("phi.fits");

	cout<<"left\tright:"<<endl;
	for (unsigned i = 0; i < left.size(); ++i)
		cout<<left[i]<<"\t"<<right[i]<<endl;

	Image<Real> PhiRotated (Xaxes(), radius + 1);
	for (unsigned latitude = 0; latitude < radius; ++latitude)
	{
		for (unsigned longitude = left[latitude]; longitude <=  right[latitude]; ++longitude)
		{
			PhiRotated.pixel(longitude,latitude)=Phi.pixel(longitude,latitude) + angularSpeed(Real(latitude)/radius) * t;
		}
	}
	PhiRotated.writeFitsImage("PhiRotated.fits");
	/*

	img->zero();
	Real newx;
	for(int y = 0; y <= radius ; ++y)
	{
		for(int x = -radiusAtLatitude(y); x <= radiusAtLatitude(r); ++x)
		{
			newx = newPos(x,y,t);
			img->pixel(x,y) = pixel(newx,y);
			img->pixel(x,-y) = pixel(newx,-y);
	}
	}*/

	return img;

}


