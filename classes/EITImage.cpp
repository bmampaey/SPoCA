#include "EITImage.h"

using namespace std;


EITImage::~EITImage()
{}

EITImage::EITImage(const long xAxes, const long yAxes, const double radius, const double wavelength)
:SunImage(xAxes, yAxes, radius, wavelength)
{}


EITImage::EITImage(const string& filename)
:SunImage()
{
	readFitsImage(filename);
}



EITImage::EITImage(const SunImage& i)
:SunImage(i)
{}


EITImage::EITImage(const SunImage* i)
:SunImage(i)
{}


int EITImage::readFitsImageP(fitsfile* fptr)
{
	int   status  = 0;
	char * comment = NULL  ;					  /**<By specifying NULL we say that we don't want the comments	*/

	status = SunImage::readFitsImageP(fptr);
	if(status)
		return status;
		
	if (fits_read_key(fptr, TDOUBLE, "SOLAR_R", &radius, comment, &status))
	{
		
		cerr<<"Error reading key SOLAR_R from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	//In EIT images, bad pixels have negative values according to Veronique
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if (pixels[j] < 0)
		{
			pixels[j] = nullvalue;
		}
	}

	return status;
	
}

inline Real EITImage::percentCorrection(const Real r)const
{

	const Real r1 = EIT_SINE_CORR_R1 / 100.;
	const Real r2 = EIT_SINE_CORR_R2 / 100.;
	const Real r3 = EIT_SINE_CORR_R3 / 100.;
	const Real r4 = EIT_SINE_CORR_R4 / 100.;
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

