#include "EUVImage.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<PixelType>.quiet_NaN())
#endif
#ifndef RealMAX
#define RealMAX (numeric_limits<PixelType>::max())
#endif

EUVImage::~EUVImage()
{

}

EUVImage::EUVImage(const long xAxes, const long yAxes)
:SunImage<PixelType>(xAxes,yAxes),wavelength(0),median(0),mode(0),datap01(0), datap95(RealMAX), exposureTime(1)
{
	sineCorrectionParameters[0] = SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SINE_CORR_R4 / 100.;
}

EUVImage::EUVImage(const long xAxes, const long yAxes, const Coordinate suncenter, const double radius)
:SunImage<PixelType>(xAxes,yAxes,suncenter,radius),wavelength(0),median(0),mode(0),datap01(0), datap95(RealMAX), exposureTime(1)
{
	sineCorrectionParameters[0] = SINE_CORR_R1 / 100.;
	sineCorrectionParameters[1] = SINE_CORR_R2 / 100.;
	sineCorrectionParameters[2] = SINE_CORR_R3 / 100.;
	sineCorrectionParameters[3] = SINE_CORR_R4 / 100.;
}


EUVImage::EUVImage(const EUVImage& i)
:SunImage<PixelType>(i),wavelength(i.wavelength),median(i.median),datap01(i.datap01),datap95(i.datap95),exposureTime(i.exposureTime)
{
}


EUVImage::EUVImage(const EUVImage* i)
:SunImage<PixelType>(i),wavelength(i->wavelength),median(i->median),datap01(i->datap01),datap95(i->datap95), exposureTime(i->exposureTime)
{
}

EUVImage::EUVImage(const Header& header)
:SunImage<PixelType>(header)
{
	postRead();
}

void EUVImage::postRead()
{
	#if DEBUG >= 1
	//If pixel values are negative this can cause problems in some functions
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullvalue_;
		}
	}
	#endif
}


void EUVImage::preWrite()
{

}

double EUVImage::Wavelength() const
{return wavelength;}
double EUVImage::Median() const
{return median;}
double EUVImage::ExposureTime() const
{return exposureTime;}

void EUVImage::copySunParameters(const EUVImage* i)
{
	SunImage<PixelType>::copySunParameters(i);
	wavelength = i->wavelength;
	median = i->median;
	mode = i->mode;
	datap01 = i->datap01;
	datap95 = i->datap95;
	exposureTime = i->exposureTime;
}

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

void EUVImage::preprocessing(string preprocessingList, const Real radiusRatio)
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
				median /= median;
				datap01 /= median;
				datap95 /= median;
				mode /= median;
			}
			else if( preprocessingStep == "DivMode")
			{
				ALCDivMode(radiusRatio, MINRADIUS());
				median /= mode;
				datap01 /= mode;
				datap95 /= mode;
				mode /= mode;
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
			median /= median;
			datap01 /= median;
			datap95 /= median;
			mode /= median;
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
			median /= mode;
			datap01 /= mode;
			datap95 /= mode;
			mode /= mode;
		}	
		else if( preprocessingStep == "TakeSqrt")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue_)
					pixels[j] = pixels[j] >= 0 ? sqrt(pixels[j]) : nullvalue_;
			}
			median = sqrt(median);
			datap01 = sqrt(datap01);
			datap95 = sqrt(datap95);
			mode = NAN;
		}	
		else if( preprocessingStep == "TakeLog")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullvalue_)
					pixels[j] = pixels[j] > 0 ? log(pixels[j]) : nullvalue_;
			}
			median = log(median);
			datap01 = log(datap01);
			datap95 = log(datap95);
			mode = NAN;
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
			median /= exposureTime;
			datap01 /= exposureTime;
			datap95 /= exposureTime;
			mode /= exposureTime;
		}
		else
		{
			cerr<<"Error during preprocessing : Unknown preprocessing step "<<preprocessingStep<<endl;
		}
		
		preprocessingStep = nextStep(preprocessingList);
			
	}
	
}

/* Function that returns the percentage of correction necessary for an annulus, given the percentage of the annulus radius to the radius of the sun */

/*Method proposed by Benjamin
No correction before r1 and after r2, Full correction between r3 and r4,
progressive correction following the ascending phase of the sine between r1 and r2
progressive correction following the descending phase of the sine between r3 and r4 */

inline Real EUVImage::percentCorrection(const Real r)const
{

	const Real r1 = sineCorrectionParameters[0];
	const Real r2 = sineCorrectionParameters[1];
	const Real r3 = sineCorrectionParameters[2];
	const Real r4 = sineCorrectionParameters[3];
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
inline Real EUVImage::percentCorrection(const Real r)const
{
	const Real r1 = DISCRETE_CORR_R1 / 100.;
	if (r < r1)
		return 0;
	else
		return 1;
}




// Method proposed by Cis
// No Correction before r1, Full correction after r2, progressive correction in between
inline Real EUVImage::percentCorrection(const Real r)const
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
inline Real EUVImage::percentCorrection(const Real r)const
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

void EUVImage::annulusLimbCorrection(Real maxLimbRadius, Real minLimbRadius)
{						  
	minLimbRadius *= radius;
	maxLimbRadius *= radius;
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = 1.0;	//This is the width of an annulus in pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	PixelType* pixelValue = pixels;
	const int xmax = Xaxes() - suncenter.x;
	const int ymax = Yaxes() - suncenter.y;
	Real pixelRadius2 = 0;
	unsigned indice = 0;
	if (median == 0) // I don't know the median value yet
	{
		vector<PixelType> onDiscList;
		onDiscList.reserve(numberValidPixelsEstimate());

	
		for (int y = - suncenter.y; y < ymax; ++y)
		{
			for (int x = - suncenter.x ; x < xmax; ++x)
			{
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = x*x + y*y;

					if (pixelRadius2 <=  minLimbRadius2)
					{
						onDiscList.push_back((*pixelValue));
					}
					else if (pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) += (*pixelValue);
						annulusNbrPixels.at(indice) += 1;
					}

				}
				++pixelValue;
			}
		}

		median = quick_select(onDiscList, onDiscList.size());
		#if DEBUG >= 3
		cout<<"Image preprocessing found median: "<<median<<endl;
		#endif
	}
	else
	{
		for (int y = - suncenter.y; y < ymax; ++y)
		{
			for (int x = - suncenter.x ; x < xmax; ++x)
			{
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = x*x + y*y;

					if (pixelRadius2 >  minLimbRadius2 && pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) += (*pixelValue);
						annulusNbrPixels.at(indice) += 1;
					}

				}
				++pixelValue;
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
	pixelValue = pixels;
	for (int y = - suncenter.y; y < ymax; ++y)
	{
		for (int x = - suncenter.x ; x < xmax; ++x)
		{
			if ((*pixelValue) != nullvalue_)
			{
				pixelRadius2 = x*x + y*y;
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue_;
				}
				else if (pixelRadius2 > minLimbRadius2)	  //We are in the limb
				{
					Real pixelRadius = sqrt(pixelRadius2);
					Real fraction = percentCorrection(pixelRadius/radius);
					indice = unsigned((pixelRadius-minLimbRadius)/deltaR);
					(*pixelValue) = (1. - fraction) * (*pixelValue) + (fraction * (*pixelValue) * median) / annulusMean.at(indice);

				}
			}
			++pixelValue;
		}
	}

}

void EUVImage::ALCDivMedian(Real maxLimbRadius, Real minLimbRadius)
{						  
	minLimbRadius *= radius;
	maxLimbRadius *= radius;
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = 1.0;	//This is the width of an annulus in pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	PixelType* pixelValue = pixels;
	const int xmax = Xaxes() - suncenter.x;
	const int ymax = Yaxes() - suncenter.y;
	Real pixelRadius2 = 0;
	unsigned indice = 0;
	if (median == 0) // I don't know the median value yet
	{
		vector<PixelType> onDiscList;
		onDiscList.reserve(numberValidPixelsEstimate());

	
		for (int y = - suncenter.y; y < ymax; ++y)
		{
			for (int x = - suncenter.x ; x < xmax; ++x)
			{
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = x*x + y*y;

					if (pixelRadius2 <=  minLimbRadius2)
					{
						onDiscList.push_back((*pixelValue));
					}
					else if (pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) += (*pixelValue);
						annulusNbrPixels.at(indice) += 1;
					}

				}
				++pixelValue;
			}
		}

		median = quick_select(onDiscList, onDiscList.size());
		#if DEBUG >= 3
		cout<<"Image preprocessing found median: "<<median<<endl;
		#endif
	}
	else
	{
		for (int y = - suncenter.y; y < ymax; ++y)
		{
			for (int x = - suncenter.x ; x < xmax; ++x)
			{
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = x*x + y*y;

					if (pixelRadius2 >  minLimbRadius2 && pixelRadius2 <=  maxLimbRadius2)
					{
						indice = unsigned((sqrt(pixelRadius2)-minLimbRadius)/deltaR);
						annulusMean.at(indice) += (*pixelValue);
						annulusNbrPixels.at(indice) += 1;
					}

				}
				++pixelValue;
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
	pixelValue = pixels;
	for (int y = - suncenter.y; y < ymax; ++y)
	{
		for (int x = - suncenter.x ; x < xmax; ++x)
		{
			if ((*pixelValue) != nullvalue_)
			{
				pixelRadius2 = x*x + y*y;
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue_;
				}
				else if (pixelRadius2 > minLimbRadius2)	  //We are in the limb
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
			++pixelValue;
		}
	}

}

void EUVImage::ALCDivMode(Real maxLimbRadius, Real minLimbRadius)
{						  
	minLimbRadius *= radius;
	maxLimbRadius *= radius;
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = 1.0;	//This is the width of an annulus in pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	// I need to construct a quick histogram to find the mode
	const Real binSize = 5;
	vector<unsigned> histo(1024, 0);


	PixelType* pixelValue = pixels;
	const int xmax = Xaxes() - suncenter.x;
	const int ymax = Yaxes() - suncenter.y;
	Real pixelRadius2 = 0;
	unsigned indice = 0;
	if (median == 0) // I don't know the median value yet
	{
		vector<PixelType> onDiscList;
		onDiscList.reserve(numberValidPixelsEstimate());

	
		for (int y = - suncenter.y; y < ymax; ++y)
		{
			for (int x = - suncenter.x ; x < xmax; ++x)
			{
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = x*x + y*y;

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
						annulusMean.at(indice) += (*pixelValue);
						annulusNbrPixels.at(indice) += 1;
					}

				}
				++pixelValue;
			}
		}

		median = quick_select(onDiscList, onDiscList.size());
		#if DEBUG >= 3
		cout<<"Image preprocessing found median: "<<median<<endl;
		#endif
	}
	else
	{
		for (int y = - suncenter.y; y < ymax; ++y)
		{
			for (int x = - suncenter.x ; x < xmax; ++x)
			{
				if ((*pixelValue) != nullvalue_)
				{
					pixelRadius2 = x*x + y*y;

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
						annulusMean.at(indice) += (*pixelValue);
						annulusNbrPixels.at(indice) += 1;
					}

				}
				++pixelValue;
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
	// We correct the limb
	pixelValue = pixels;
	for (int y = - suncenter.y; y < ymax; ++y)
	{
		for (int x = - suncenter.x ; x < xmax; ++x)
		{
			
			if ((*pixelValue) != nullvalue_)
			{
				pixelRadius2 = x*x + y*y;
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullvalue_;
				}
				else
				{
					if (pixelRadius2 > minLimbRadius2)	  //We are in the limb
					{
						Real pixelRadius = sqrt(pixelRadius2);
						Real fraction = percentCorrection(pixelRadius/radius);
						indice = unsigned((pixelRadius-minLimbRadius)/deltaR);
						(*pixelValue) = (1. - fraction) * (*pixelValue) + (fraction * (*pixelValue) * median) / annulusMean.at(indice);

					}
					(*pixelValue) /= mode;
				}
			}
			++pixelValue;
		}
	}

}


