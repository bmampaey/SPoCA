#include "AIAImage.h"

using namespace std;


AIAImage::~AIAImage()
{}

AIAImage::AIAImage(const long xAxes, const long yAxes, const double radius, const double wavelength)
:SunImage(xAxes, yAxes, radius, wavelength)
{}


AIAImage::AIAImage(const string& filename)
:SunImage()
{
	readFitsImage(filename);
}



AIAImage::AIAImage(const SunImage& i)
:SunImage(i)
{}


AIAImage::AIAImage(const SunImage* i)
:SunImage(i)
{}



int AIAImage::readFitsImageP(fitsfile* fptr)
{
	int   status  = 0;
	char * comment = NULL  ;					  /**<By specifying NULL we say that we don't want the comments	*/

	status = SunImage::readFitsImageP(fptr);
	if(status)
		return status;
		
	if (fits_read_key(fptr, TDOUBLE, "R_SUN", &radius, comment, &status))
	{
		
		cerr<<"Error reading key R_SUN from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
		
		//HACK for bad AIA fits header, remove when R_SUN is corrected
		if (fits_read_key(fptr, TDOUBLE, "RSUN_OBS", &radius, comment, &status))
		{
			
			cerr<<"Error reading key RSUN_OBS from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
			fits_report_error(stderr, status);
			status = 0;
		}
		radius/=cdelt[0];
		//END of HACK
	}

	/* We remove it for Ryan
	if (fits_read_key(fptr, datatype, "DATAP01", &datap01,comment, &status))
	{
		
		cerr<<"Error reading key DATAP01 from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	if (fits_read_key(fptr, datatype, "DATAP95", &datap95,comment, &status))
	{
		
		cerr<<"Error reading key DATAP95 from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	*/

	if (fits_read_key(fptr, datatype, "DATAMEDN", &median,comment, &status))
	{
		
		cerr<<"Error reading key DATAPMEDN from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}

	if (fits_read_key(fptr, TDOUBLE, "EXPTIME", &exposureTime,comment, &status))
	{
		
		cerr<<"Error reading key EXPTIME from file "<<fptr->Fptr->filename<<" :"<< status <<endl;
		fits_report_error(stderr, status);
		status = 0;
	}
	
	return status;


}



inline Real AIAImage::percentCorrection(const Real r)const
{

	const Real r1 = AIA_SINE_CORR_R1 / 100.;
	const Real r2 = AIA_SINE_CORR_R2 / 100.;
	const Real r3 = AIA_SINE_CORR_R3 / 100.;
	const Real r4 = AIA_SINE_CORR_R4 / 100.;
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



