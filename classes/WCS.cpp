#include <assert.h>
#include <limits>
#include <cmath>

#include "WCS.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif



WCS::WCS()
:sun_center(NAN), sun_radius(NAN), date_obs(""), cdelt1(0), cdelt2(0), b0(NAN), l0(NAN), carrington_l0(NAN), dsun_obs(NAN), sunradius_Mm(SUN_RADIUS), time_obs(0), cos_b0(NAN), sin_b0(NAN) 
{
	cd[0][0] = cd[0][1] = cd[1][0] = cd[1][1] = NAN;
	icd[0][0] = icd[0][1] = icd[1][0] = icd[1][1] = NAN;
}

WCS::WCS(const RealPixLoc& sun_center, const Real& sun_radius)
:sun_center(sun_center), sun_radius(sun_radius), date_obs(""), cdelt1(0), cdelt2(0), b0(NAN), l0(NAN), carrington_l0(NAN), dsun_obs(NAN), sunradius_Mm(SUN_RADIUS), time_obs(0), cos_b0(NAN), sin_b0(NAN) 
{
	cd[0][0] = cd[0][1] = cd[1][0] = cd[1][1] = NAN;
	icd[0][0] = icd[0][1] = icd[1][0] = icd[1][1] = NAN;
}

void WCS::setSunCenter(const Real& crpix1, const Real& crpix2)
{
	sun_center = RealPixLoc(crpix1, crpix2);
}

void WCS::setSunradius(const double& sun_radius)
{
	this->sun_radius = sun_radius;
}

void WCS::setDateObs(string date_obs)
{
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	
	this->date_obs = date_obs;
	if(! date_obs.empty())
		time_obs = iso2ctime(date_obs);
	else
		time_obs = 0;
}

void WCS:: setCDelt(const Real& cdelt1, const Real& cdelt2)
{
	this->cdelt1 = cdelt1;
	this->cdelt2 = cdelt2;
}

void WCS:: setB0(const Real& b0)
{
	this->b0 = b0*DEGREE2RADIAN;
	sin_b0 = sin(this->b0);
	cos_b0 = cos(this->b0);
}

void WCS:: setL0(const Real& l0)
{
	this->l0 = l0 * DEGREE2RADIAN;
}

void WCS:: setCarringtonL0(const Real& l0)
{
	this->carrington_l0 = l0 * DEGREE2RADIAN;
}

void WCS:: setDistanceSunObs(const double& dsun_obs)
{
	this->dsun_obs = dsun_obs;
}

//! Set the CD matrix from a CROTA2
void WCS:: setCrota2(const Real& crota2)
{
	assert(cdelt1 != 0 && cdelt2 != 0);
	Real sin_crota2 = sin(crota2*DEGREE2RADIAN);
	Real cos_crota2 = cos(crota2*DEGREE2RADIAN);
	setPC(cos_crota2, -sin_crota2 * (cdelt2/cdelt1), sin_crota2 * (cdelt2/cdelt1), cos_crota2);
}

//! Set the CD matrix from a PC matrix
void WCS:: setPC(const Real& pc1_1, const Real& pc1_2, const Real& pc2_1, const Real& pc2_2)
{
	assert(cdelt1 != 0 && cdelt2 != 0);
	setCD(cdelt1 * pc1_1, cdelt2 * pc1_2, cdelt1 * pc2_1, cdelt2 * pc2_2);
}

//! Set the CD matrix and the inverse CD matrix
void WCS:: setCD(const Real& cd1_1, const Real& cd1_2, const Real& cd2_1, const Real& cd2_2)
{
	cd[0][0] = cd1_1;
	cd[0][1] = cd1_2;
	cd[1][0] = cd2_1;
	cd[1][1] = cd2_2;
	
	assert(cd[0][0]*cd[1][1] != cd[0][1]*cd[1][0]);
	
	Real det_cd = 1./(cd[0][0]*cd[1][1] - cd[0][1]*cd[1][0]);
	icd[0][0] = det_cd * cd[1][1];
	icd[0][1] = - det_cd * cd[0][1];
	icd[1][0] = - det_cd * cd[1][0];
	icd[1][1] = det_cd * cd[0][0];
}


