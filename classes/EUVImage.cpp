#include "EUVImage.h"
#include "colortables.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<EUVPixelType>.quiet_NaN())
#endif
#ifndef RealMAX
#define RealMAX (numeric_limits<EUVPixelType>::max())
#endif

EUVImage::~EUVImage()
{}


EUVImage::EUVImage(const unsigned& xAxes, const unsigned& yAxes)
:SunImage<EUVPixelType>(xAxes,yAxes),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap95(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const unsigned& xAxes, const unsigned& yAxes, const RealPixLoc& suncenter, const Real& radius)
:SunImage<EUVPixelType>(xAxes,yAxes,suncenter,radius),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap95(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:SunImage<EUVPixelType>(header, xAxes, yAxes),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap95(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:SunImage<EUVPixelType>(wcs, xAxes, yAxes),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap95(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const EUVImage& i)
:SunImage<EUVPixelType>(i)
{
	wavelength = i.wavelength;
	median = i.median;
	mode = i.mode;
	datap01 = i.datap01;
	datap95 = i.datap95;
	exposureTime = i.exposureTime;
	ALCParameters = i.ALCParameters;
}


EUVImage::EUVImage(const EUVImage* i)
:SunImage<EUVPixelType>(i)
{
	wavelength = i->wavelength;
	median = i->median;
	mode = i->mode;
	datap01 = i->datap01;
	datap95 = i->datap95;
	exposureTime = i->exposureTime;
	ALCParameters = i->ALCParameters;
}

vector<Real> EUVImage::getALCParameters()
{
	Real parameters[] = EUV_ALC_PARAMETERS;
	vector<Real> temp(parameters, parameters + (sizeof(parameters)/sizeof(parameters[0])));
	for(unsigned p = 0; p < temp.size(); ++p)
		temp[p] /= 100.;
	return temp;
}

void EUVImage::setALCParameters(vector<Real> ALCParameters)
{
	if (ALCParameters.size() < 4)
	{
		cerr<<"Error setting the ALC parameters, at least 4 must be provided."<<endl;
		exit(EXIT_FAILURE);
	}
	this->ALCParameters = ALCParameters;
}

void EUVImage::parseHeader()
{
	#if DEBUG >= 1
	//If pixel values are negative this can cause problems in some functions
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullpixelvalue;
		}
	}
	#endif
	
	SunImage<EUVPixelType>::parseHeader();
	
	if (header.has("WAVELNTH"))
		wavelength = header.get<Real>("WAVELNTH");
	if (header.has("DATAMEDN"))
		median = header.get<Real>("DATAMEDN");
	if (header.has("DATAP01"))
		datap01 = header.get<EUVPixelType>("DATAP01");
	if (header.has("DATAP95"))
		datap95 = header.get<EUVPixelType>("DATAP95");
	
	if (header.has("EXPTIME"))
		exposureTime = header.get<Real>("EXPTIME");
	else
		exposureTime = 1;
}


void EUVImage::fillHeader()
{
	SunImage<EUVPixelType>::fillHeader();
	header.set<Real>("EXPTIME", exposureTime);
	header.set<Real>("DATAMEDN", median);
	header.set<EUVPixelType>("DATAP01",datap01);
	header.set<EUVPixelType>("DATAP95", datap95);
}

inline string EUVImage::Channel() const
{
	return Instrument() + "_" + itos(int(Wavelength()));
}

inline string EUVImage::Label() const
{
		return Instrument() + " " + itos(int(Wavelength())) + "Å " + ObservationDate();
}

inline Real EUVImage::Wavelength() const
{
	return wavelength;
}

inline Real EUVImage::Median() const
{
	if (median != nullpixelvalue)
		return median;
	else
		return percentiles(0.5);
}

inline Real EUVImage::Mode() const
{
	if (mode != nullpixelvalue)
		return mode;
	else
		return static_cast<const Image<EUVPixelType>*>(this)->mode();
}

inline Real EUVImage::ExposureTime() const
{
	return exposureTime;
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
			//The successive operations ALC + DivMedian have been optimized
			preprocessingStep = nextStep(preprocessingList);
			if( preprocessingStep == "DivMedian")
			{
				ALCDivMedian(radiusRatio, MINRADIUS());
				median /= median;
				datap01 /= median;
				datap95 /= median;
				mode /= median;
			}
			else
			{
				annulusLimbCorrection(radiusRatio, MINRADIUS());
				preprocessingList = preprocessingStep + "," + preprocessingList;
			}
		}
		else if(preprocessingStep.find("Div") == 0)
		{
			Real denominator = 1.;
			while(preprocessingStep.find("Div") == 0)
			{
				if(preprocessingStep == "DivMedian")
					denominator *= Median();
				else if(preprocessingStep == "DivMode")
					denominator *= Mode();
				else if(preprocessingStep == "DivExpTime")
					denominator *= exposureTime;
				else
					cerr<<"Error during preprocessing : Unknown preprocessing step "<<preprocessingStep<<endl;
				
				if(denominator == 0)
				{
					cerr<<"Error during preprocessing step "<<preprocessingStep<<". Division by 0."<<endl;
					exit(EXIT_FAILURE);
				}
				preprocessingStep = nextStep(preprocessingList);
			}
			
			preprocessingList = preprocessingStep + "," + preprocessingList;
			
			if(denominator != 1)
			{
				for (unsigned j=0; j < numberPixels; ++j)
				{
					if (pixels[j] != nullpixelvalue)
						pixels[j] = pixels[j] / denominator;
				}
				median /= denominator;
				datap01 /= denominator;
				datap95 /= denominator;
				mode /= denominator;
			}
		}	
		else if( preprocessingStep == "TakeSqrt")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullpixelvalue)
					pixels[j] = pixels[j] >= 0 ? sqrt(pixels[j]) : nullpixelvalue;
			}
			median = sqrt(median);
			mode = nullpixelvalue;
			datap01 = sqrt(datap01);
			datap95 = sqrt(datap95);
		}	
		else if( preprocessingStep == "TakeLog")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullpixelvalue)
					pixels[j] = pixels[j] > 0 ? log(pixels[j]) : nullpixelvalue;
			}
			median = log(median);
			mode = nullpixelvalue;
			datap01 = log(datap01);
			datap95 = log(datap95);
			
		}
		else if(preprocessingStep.find("Thr") == 0)
		{
			double upper = 100, lower = 0;
			while(preprocessingStep.find("Thr") == 0)
			{
				if(preprocessingStep.find("ThrMin") == 0)
				{
					lower = stod(preprocessingStep.substr(6));
				}
				else if (preprocessingStep.find("ThrMax") == 0)
				{
					upper = stod(preprocessingStep.substr(6));
				}
				else
					cerr<<"Error during preprocessing : Unknown preprocessing step "<<preprocessingStep<<endl;
				preprocessingStep = nextStep(preprocessingList);
			}
			
			preprocessingList = preprocessingStep + "," + preprocessingList;
			
			vector<Real> percents;
			if(lower > 0)
				percents.push_back(lower/100.);
			if(upper < 100)
				percents.push_back(upper/100.);
				
			
			if(percents.size() > 0)
			{
				vector<EUVPixelType> temp(pixels, pixels+numberPixels);
				vector<EUVPixelType> limits = percentiles(percents);
				
				EUVPixelType max = RealMAX;
				if(upper < 100)
				{
					max = limits.back();
					limits.pop_back();
					#if DEBUG >= 3
					cerr<<"Percentile "<<upper<<": "<<max<<endl;
					#endif
				}
				
				EUVPixelType min = -RealMAX;
				if(lower > 0)
				{
					min = limits.back();
					limits.pop_back();
					#if DEBUG >= 3
					cerr<<"Percentile "<<lower<<": "<<min<<endl;
					#endif
				}
				
				threshold(min, max);
			}
		}
		else if(preprocessingStep.find("Smooth") == 0)
		{
			double factor = stod(preprocessingStep.substr(6));
			binomial_smoothing(int(factor/PixelWidth()+0.5));
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

	const Real r1 = ALCParameters[0];
	const Real r2 = ALCParameters[1];
	const Real r3 = ALCParameters[2];
	const Real r4 = ALCParameters[3];
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



void EUVImage::annulusLimbCorrection(Real maxLimbRadius, Real minLimbRadius)
{
	minLimbRadius *= SunRadius();
	maxLimbRadius *= SunRadius();
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = ANNULUS_WIDTH;	//This is the width of an annulus in pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	EUVPixelType* pixelValue = pixels;
	const Real xmax = Real(Xaxes()) - SunCenter().x;
	const Real ymax = Real(Yaxes()) - SunCenter().y;
	Real pixelRadius2 = 0;
	unsigned indice = 0;
	
	vector<EUVPixelType> onDiscList;
	onDiscList.reserve(unsigned(BIPI * minLimbRadius * minLimbRadius));

	// During the first pass we compute the total value of the annulus post limb, and the median value of the ante limb disc
	for (Real y = - SunCenter().y; y < ymax; ++y)
	{
		for (Real x = - SunCenter().x ; x < xmax; ++x)
		{
			if ((*pixelValue) != nullpixelvalue)
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

	median = quickselect(onDiscList);
	#if DEBUG >= 3
	cout<<"Image preprocessing found median: "<<median<<endl;
	#endif


	// We calculate the mean value of each annulus
	for (unsigned i=0; i<annulusMean.size(); ++i)
	{
		if(annulusNbrPixels.at(i)>0)
			annulusMean.at(i) = annulusMean.at(i) / Real(annulusNbrPixels.at(i));
	}
	
	// We correct the limb
	pixelValue = pixels;
	for (Real y = - SunCenter().y; y < ymax; ++y)
	{
		for (Real x = - SunCenter().x ; x < xmax; ++x)
		{
			if ((*pixelValue) != nullpixelvalue)
			{
				pixelRadius2 = x*x + y*y;
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullpixelvalue;
				}
				else if (pixelRadius2 > minLimbRadius2)	  //We are in the limb
				{
					Real pixelRadius = sqrt(pixelRadius2);
					Real fraction = percentCorrection(pixelRadius/SunRadius());
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
	minLimbRadius *= SunRadius();
	maxLimbRadius *= SunRadius();
	Real minLimbRadius2 = minLimbRadius*minLimbRadius;
	Real maxLimbRadius2 = maxLimbRadius*maxLimbRadius;
	const Real deltaR = ANNULUS_WIDTH;	//This is the width of an annulus in pixel
	vector<Real> annulusMean(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);
	vector<unsigned> annulusNbrPixels(unsigned((maxLimbRadius-minLimbRadius)/deltaR)+2,0);

	EUVPixelType* pixelValue = pixels;
	const Real xmax = Real(Xaxes()) - SunCenter().x;
	const Real ymax = Real(Yaxes()) - SunCenter().y;
	Real pixelRadius2 = 0;
	unsigned indice = 0;

	vector<EUVPixelType> onDiscList;
	onDiscList.reserve(unsigned(BIPI * minLimbRadius * minLimbRadius));

	// During the first pass we compute the total value of the annulus post limb, and the median value of the ante limb disc
	for (Real y = - SunCenter().y; y < ymax; ++y)
	{
		for (Real x = - SunCenter().x ; x < xmax; ++x)
		{
			if ((*pixelValue) != nullpixelvalue)
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

	median = quickselect(onDiscList);
	#if DEBUG >= 3
	cout<<"Image preprocessing found median: "<<median<<endl;
	#endif


	// We calculate the mean value of each annulus
	for (unsigned i=0; i<annulusMean.size(); ++i)
	{
		if(annulusNbrPixels.at(i)>0)
			annulusMean.at(i) = annulusMean.at(i) / Real(annulusNbrPixels.at(i));
	}
	// We correct the limb
	pixelValue = pixels;
	for (Real y = - SunCenter().y; y < ymax; ++y)
	{
		for (Real x = - SunCenter().x ; x < xmax; ++x)
		{
			if ((*pixelValue) != nullpixelvalue)
			{
				pixelRadius2 = x*x + y*y;
				if (maxLimbRadius2 < pixelRadius2)
				{
					(*pixelValue) = nullpixelvalue;
				}
				else if (pixelRadius2 > minLimbRadius2)	  //We are in the limb
				{
					Real pixelRadius = sqrt(pixelRadius2);
					Real fraction = percentCorrection(pixelRadius/SunRadius());
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


void EUVImage::enhance_contrast()
{
	if (datap01 == nullpixelvalue || datap95 == nullpixelvalue)
	{
		vector<Real> values(2, 0.01);
		values[1] = 0.99;
		vector<EUVPixelType> p = percentiles(values);
		datap01 = p[0];
		datap95 = p[1];
	}
	threshold(datap01, datap95);
}

vector<char> EUVImage::color_table() const
{
	char colorTable[][3] = CT_BLUE_RED;
	return vector<char>(colorTable[0], colorTable[0] + 3*(sizeof(colorTable)/sizeof(colorTable[0])));
}

#ifdef MAGICK
MagickImage EUVImage::magick()
{
	EUVPixelType* rescaledPixels = new EUVPixelType[numberPixels];
	EUVPixelType min, max;
	minmax(min,max);
	Real scale = 1./ (max - min);
	for(unsigned j = 0; j < numberPixels; ++j)
	{
		rescaledPixels[j] = pixels[j] == nullpixelvalue ? 0 : (pixels[j] - min) * scale;
	}
	MagickImage rescaledImage(rescaledPixels, xAxes, yAxes, "I");
	rescaledImage.flip();
	return rescaledImage;
}

#endif


