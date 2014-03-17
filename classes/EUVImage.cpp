#include "EUVImage.h"
#include "colortables.h"
#include <algorithm>

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
:SunImage<EUVPixelType>(xAxes,yAxes),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap99(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const unsigned& xAxes, const unsigned& yAxes, const RealPixLoc& suncenter, const Real& radius)
:SunImage<EUVPixelType>(xAxes,yAxes,suncenter,radius),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap99(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const Header& header, const unsigned& xAxes, const unsigned& yAxes)
:SunImage<EUVPixelType>(header, xAxes, yAxes),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap99(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const WCS& wcs, const unsigned& xAxes, const unsigned& yAxes)
:SunImage<EUVPixelType>(wcs, xAxes, yAxes),wavelength(NAN),median(nullpixelvalue),mode(nullpixelvalue),datap01(nullpixelvalue), datap99(nullpixelvalue), exposureTime(1), ALCParameters(getALCParameters())
{}


EUVImage::EUVImage(const EUVImage& i)
:SunImage<EUVPixelType>(i)
{
	wavelength = i.wavelength;
	median = i.median;
	mode = i.mode;
	datap01 = i.datap01;
	datap99 = i.datap99;
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
	datap99 = i->datap99;
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
	#if defined EXTRA_SAFE
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
	if (header.has("DATAP99"))
		datap99 = header.get<EUVPixelType>("DATAP99");
	
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
	header.set<EUVPixelType>("DATAP99", datap99);
}

inline string EUVImage::Channel() const
{
	return Instrument() + "_" + toString(int(Wavelength()));
}

inline string EUVImage::Label() const
{
		return Instrument() + " " + toString(int(Wavelength())) + "Å " + ObservationDate();
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


void EUVImage::preprocessing(string preprocessingList)
{
	double maxRadius = MAXRADIUS();
	vector<string> preprocessingSteps = split(preprocessingList);
	for(unsigned s = 0; s < preprocessingSteps.size(); ++s)
	{
		vector<string> stepParameters = split(preprocessingSteps[s], '=');
		string stepType = trimWhites(stepParameters[0]);
		if(stepType == "NAR")
		{
			if(stepParameters.size() > 1)
			{
				double radiusRatio = toDouble(stepParameters[1]);
				if(radiusRatio < maxRadius)
				{
					nullifyAboveRadius(radiusRatio);
					maxRadius = radiusRatio;
				}
			}
			else
			{
				nullifyAboveRadius();
				maxRadius = 1;
			}
		}
		else if(stepType == "ALC")
		{
			annulusLimbCorrection(maxRadius, MINRADIUS());
		}
		else if(stepType.find("Div") == 0)
		{
			double denominator = 1.;
			while(true)
			{
				if(stepType == "DivMedian")
					denominator *= Median();
				else if(stepType == "DivMode")
					denominator *= Mode();
				else if(stepType == "DivExpTime")
					denominator *= exposureTime;
				else
				{
					cerr<<"Error : Unknown preprocessing step "<<stepType<<endl;
					exit(EXIT_FAILURE);
				}
				if(s + 1 < preprocessingSteps.size() && preprocessingSteps[s+1].find("Div") == 0)
				{
					++s;
					stepParameters = split(preprocessingSteps[s], '=');
					stepType = trimWhites(stepParameters[0]);
				}
			}
			if(denominator == 0)
			{
				cerr<<"Error: In preprocessing step "<<preprocessingSteps[s]<<". Division by 0."<<endl;
				exit(EXIT_FAILURE);
			}
			else if(denominator != 1.)
			{
				for (unsigned j=0; j < numberPixels; ++j)
				{
					if (pixels[j] != nullpixelvalue)
						pixels[j] = pixels[j] / denominator;
				}
				median /= denominator;
				datap01 /= denominator;
				datap99 /= denominator;
				mode /= denominator;
			}
		}
		else if(stepType == "TakeSqrt")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullpixelvalue)
					pixels[j] = pixels[j] >= 0 ? sqrt(pixels[j]) : nullpixelvalue;
			}
			median = sqrt(median);
			mode = nullpixelvalue;
			datap01 = sqrt(datap01);
			datap99 = sqrt(datap99);
		}	
		else if(stepType == "TakeLog")
		{
			for (unsigned j=0; j < numberPixels; ++j)
			{
				if (pixels[j] != nullpixelvalue)
					pixels[j] = pixels[j] > 0 ? log(pixels[j]) : nullpixelvalue;
			}
			median = log(median);
			mode = nullpixelvalue;
			datap01 = log(datap01);
			datap99 = log(datap99);
			
		}
		else if(stepType.find("Thr") == 0)
		{
			double upperPercentile = 100, lowerPercentile = 0;
			double upperValue = std::numeric_limits<double>::max(), lowerValue = std::numeric_limits<double>::min();
			while(true)
			{
				if(stepParameters.size() < 2)
				{
					cerr<<"Error: No value specified for threshold preprocessing step"<<preprocessingSteps[s]<<endl;
					exit(EXIT_FAILURE);
				}
				if(stepType == "ThrMinPer")
				{
					lowerPercentile = max(toDouble(stepParameters[1]), lowerPercentile);
				}
				else if(stepType == "ThrMaxPer")
				{
					upperPercentile = min(toDouble(stepParameters[1]), upperPercentile);
				}
				else if(stepType == "ThrMin")
				{
					lowerValue = max(toDouble(stepParameters[1]), lowerValue);
				}
				else if(stepType == "ThrMax")
				{
					upperValue = min(toDouble(stepParameters[1]), upperValue);
				}
				else
				{
					cerr<<"Error: Unknown preprocessing step "<<preprocessingSteps[s]<<endl;
					exit(EXIT_FAILURE);
				}
				if(s + 1 < preprocessingSteps.size() && preprocessingSteps[s+1].find("Thr") == 0)
				{
					++s;
					stepParameters = split(preprocessingSteps[s], '=');
					stepType = trimWhites(stepParameters[0]);
				}
			}
			
			vector<Real> percents;
			if(lowerPercentile > 0)
				percents.push_back(lowerPercentile/100.);
			if(upperPercentile < 100)
				percents.push_back(upperPercentile/100.);
				
			if(percents.size() > 0)
			{
				vector<EUVPixelType> limits = percentiles(percents);
				
				if(upperPercentile < 100)
				{
					upperValue = min(double(limits.back()), upperValue);
					limits.pop_back();
				}
				
				if(lowerPercentile > 0)
				{
					lowerValue = max(double(limits.back()), lowerValue);
					limits.pop_back();
				}
			}
			threshold(lowerValue, upperValue);
		}
		else if(stepType == "Smooth")
		{
			binomial_smoothing(int(toDouble(stepParameters[1])/PixelWidth()+0.5));
		}
		else
		{
			cerr<<"Error: Unknown preprocessing step "<<preprocessingSteps[s]<<endl;
		}
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
	#if defined VERBOSE
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
				// If we are in the limb, we apply the correction
				if (minLimbRadius2 < pixelRadius2 && pixelRadius2 <= maxLimbRadius2)
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

void EUVImage::enhance_contrast()
{
	if (datap01 == nullpixelvalue || datap99 == nullpixelvalue)
	{
		vector<Real> values(2, 0.01);
		values[1] = 0.99;
		vector<EUVPixelType> p = percentiles(values);
		datap01 = p[0];
		datap99 = p[1];
	}
	threshold(datap01, datap99);
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

