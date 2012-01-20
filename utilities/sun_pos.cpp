#include "classes/constants.h"
#include "classes/FitsFile.h"
#include <string>
#include <ctime>
#include <iostream>
#include <cmath>
using namespace std;

void sun_position(const Real& jdn, Real& longitude, Real& ra, Real& dec, Real& app_longitude, Real& obliquity)
{
	double t = jdn/36525.0;
	#if DEBUG >= 3
		cout<<fixed;
		cout<<"Julian centuries: "<<t<<endl;
	#endif
	app_longitude = (279.696678 + fmod(36000.768925*t,360.0))*3600.0;
	#if DEBUG >= 3
		cout<<"Sun mean longitude: "<<app_longitude<<endl;
	#endif
	double me = 358.475844 + fmod(35999.049750 * t, 360.0);
	double ellcor = (6910.1 - 17.2*t)*sin(me * DEGREE2RADIAN) + 72.3 * sin(2.0 * me* DEGREE2RADIAN);
	app_longitude = app_longitude + ellcor;
	#if DEBUG >= 3
		cout<<"Earth ellipticity correction: "<<app_longitude<<endl;
	#endif

	double mv = 212.603219 + fmod(58517.803875*t, 360.0) ;
	double vencorr = 4.8 * cos((299.1017 + mv - me)* DEGREE2RADIAN) + 5.5 * cos((148.3133 + 2.0 * mv - 2.0 * me)* DEGREE2RADIAN) + 2.5 * cos((315.9433 + 2.0 * mv - 3.0 * me)* DEGREE2RADIAN) + 1.6 * cos((345.2533 + 3.0 * mv - 4.0 * me)* DEGREE2RADIAN) + 1.0 * cos((318.15 + 3.0 * mv - 5.0 * me)* DEGREE2RADIAN);
	app_longitude = app_longitude + vencorr;
	#if DEBUG >= 3
		cout<<"Venus perturbation correction: "<<app_longitude<<endl;
	#endif

	double mm = 319.529425 + fmod(19139.858500 * t, 360.0);
	double marscorr = 2.0 * cos((343.8883 - 2.0 * mm + 2.0 * me)* DEGREE2RADIAN) + 1.8 * cos((200.4017 - 2.0 * mm + me) * DEGREE2RADIAN);
	app_longitude = app_longitude + marscorr;
	#if DEBUG >= 3
		cout<<"Mars perturbation correction: "<<app_longitude<<endl;
	#endif

	double mj = 225.328328 + fmod(3034.6920239 * t, 360.0);
	double jupcorr = 7.2 * cos((179.5317 - mj + me)* DEGREE2RADIAN) + 2.6 * cos((263.2167 - mj) * DEGREE2RADIAN) + 2.7 * cos((87.1450 - 2.0 * mj + 2.0 * me) * DEGREE2RADIAN) + 1.6 * cos((109.4933 - 2.0 * mj + me) * DEGREE2RADIAN);
	app_longitude = app_longitude + jupcorr;
	#if DEBUG >= 3
		cout<<"Jupiter perturbation correction: "<<app_longitude<<endl;
	#endif
	double d = 350.7376814 + fmod(445267.11422 * t, 360.0);
	double mooncorr = 6.5 * sin(d* DEGREE2RADIAN);
	app_longitude = app_longitude + mooncorr;
	#if DEBUG >= 3
		cout<<"Moon perturbation correction: "<<app_longitude<<endl;
	#endif
	double longterm = + 6.4 * sin((231.19 + 20.20 * t)* DEGREE2RADIAN);
	app_longitude = app_longitude + longterm;
	app_longitude = fmod(app_longitude + 2592000.0, 1296000.0);
	#if DEBUG >= 3
		cout<<"Long period terms correction: "<<app_longitude<<endl;
	#endif
	longitude = app_longitude/3600.0;
	
	app_longitude = app_longitude - 20.5;
	#if DEBUG >= 3
		cout<<"Aberration correction: "<<app_longitude<<endl;
	#endif
	double omega = 259.183275 - fmod(1934.142008 * t , 360.0);
	app_longitude = app_longitude - 17.2 * sin(omega* DEGREE2RADIAN);
	#if DEBUG >= 3
		cout<<"Nutation correction: "<<app_longitude<<endl;
	#endif
	obliquity = 23.452294 - 0.0130125*t + (9.2*cos(omega* DEGREE2RADIAN))/3600.0;
	app_longitude = app_longitude/3600.0;
	ra = atan2(sin(app_longitude* DEGREE2RADIAN) * cos(obliquity* DEGREE2RADIAN) , cos(app_longitude* DEGREE2RADIAN)) * RADIAN2DEGREE;
	if (ra < 0.0)
		ra += 360.0;
	dec = asin(sin(app_longitude* DEGREE2RADIAN) * sin(obliquity* DEGREE2RADIAN)) * RADIAN2DEGREE;
}

Real distance_sun_earth(Real jdn, Real& p, Real& b)
{
	// Number of Julian days since 2415020.0
	jdn -= 2415020.0;
	
	// Compute the sun position
	Real longitude, ra, dec, app_longitude, obliquity;
	sun_position(jdn, longitude, ra, dec, app_longitude, obliquity);
	
	double lambda = longitude - (20.5/3600.0);
	double node = 73.666666 + (50.25/3600.0)*((jdn/365.25) + 50.0);
	double arg = lambda - node;
	p = (atan(-tan(obliquity*DEGREE2RADIAN) * cos(app_longitude*DEGREE2RADIAN)) + atan(-0.12722 * cos(arg*DEGREE2RADIAN))) * RADIAN2DEGREE;
	b = asin(0.12620 * sin(arg*DEGREE2RADIAN)) * RADIAN2DEGREE;
	double t = jdn/36525.0;
	double mv = 212.6 + fmod(58517.80 * t, 360.0);
	double me = 358.476 + fmod(35999.0498 * t, 360.0);
	double mm = 319.5 + fmod(19139.86 * t, 360.0);
	double mj = 225.3 + fmod(3034.69 * t, 360.0);
	double d = 350.7 + fmod(445267.11 * t, 360.0);
	double r = 1.000141 - (0.016748 - 0.0000418*t) * cos(me*DEGREE2RADIAN) - 0.000140 * cos(2.0*me*DEGREE2RADIAN) + 0.000016 * cos((58.3 + 2.0*mv - 2.0*me)*DEGREE2RADIAN) + 0.000005 * cos((209.1 + mv - me)*DEGREE2RADIAN) + 0.000005 * cos((253.8 - 2.0*mm + 2.0*me)*DEGREE2RADIAN) + 0.000016 * cos((89.5 - mj + me)*DEGREE2RADIAN) + 0.000009 * cos((357.1 - 2.0*mj + 2.0*me)*DEGREE2RADIAN) + 0.000031 * cos(d*DEGREE2RADIAN);
	// Distance to the sun in arcmin
	double distance = asin((695508000.0 / 149597870691.0)/r)*10800./PI;
	#if DEBUG >= 3
		cout<<"distance_sun_earth (arcmin): "<<distance<<endl;
	#endif
	// We convert in meters
	return 695508000.0 * (60. * 180. / PI) / distance;
}

Real julian_day(time_t observationTime)
{
	// Transform observationTime to year/month/day
	tm* date = gmtime(&observationTime);
	int year = date->tm_year + 1900;
	int month = date->tm_mon + 1;
	
	// Compute the julian day
	int a = (14 - month)/12;
	int y = year + 4800 - a;
	int m = month + 12 * a - 3;
	
	Real jdn = int(date->tm_mday + (153*m+2)/5 + y*365 + y/4 - y/100 + y/400 - 32045) + ((date->tm_hour - 12.) / 24. + (date->tm_min / 1440.) + (date->tm_sec/86400.));
	return jdn;
}

int main(int argc, char**argv)
{
	Real jdn, longitude, ra, dec, app_longitude, obliquity;
	string date;
	if (argc < 2)
	{
		cout<<"Enter day :";
		cin>>date;
	}
	else
	{
		date = argv[1];
	}
	time_t time = iso2ctime(date);
	jdn = julian_day(time);
	cout<<fixed<<"Julian day: "<<jdn<<endl;
	sun_position(jdn, longitude, ra, dec, app_longitude, obliquity);
	cout<<fixed<<" "<<longitude<<" "<<ra<<" "<<dec<<" "<<app_longitude<<" "<<obliquity<<endl;
	Real p, b;
	cout<<"distance_sun_earth: "<<distance_sun_earth(jdn, p, b);
	cout<<" p: "<<p<<" b: "<<b<<endl;
}

