#include "ColorMap.h"

using namespace std;

ColorMap::~ColorMap()
{}

ColorMap::ColorMap(const long xAxes, const long yAxes)
:SunImage<unsigned>(xAxes, yAxes)
{
	nullvalue_ = 0;

}


ColorMap::ColorMap(const SunImage<unsigned>& i)
:SunImage<unsigned>(i)
{
	nullvalue_ = 0;

}


ColorMap::ColorMap(const SunImage<unsigned>* i)
:SunImage<unsigned>(i)
{
	nullvalue_ = 0;

}


ColorMap::ColorMap(const Header& header)
:SunImage<unsigned>(header)
{
	postRead();
}

void ColorMap::postRead()
{
	suncenter.x = header.get<int>("CRPIX1") - 1;
	suncenter.y = header.get<int>("CRPIX2") - 1;
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	// We read the radius
	radius = header.get<double>("RADIUS");
	b0 = (header.get<double>("SOLAR_B0")/180.)*PI;
	date_obs = header.get<string>("DATE-OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = iso2ctime(date_obs);
}

void ColorMap::preWrite()
{
	header.set<string>("INSTRUME", "SPoCA");
	header.set<double>("RADIUS", radius);
	header.set<int>("CRPIX1", suncenter.x + 1);
	header.set<int>("CRPIX2", suncenter.y + 1);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("DATE-OBS", date_obs);
	header.set<double>("SOLAR_B0", (b0 * 180)/PI);
}


bool isColorMap(const Header& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SPoCA") != string::npos;	
}



void ColorMap::tresholdRegionsByRawArea(const double minSize)
{
	const double pixelarea = PixelArea();
	
	//First we compute the area for each color
	vector<double> areas(100, 0);

	unsigned* p = pixels;
	
	for (unsigned y = 0; y < yAxes; ++y)
	{
		for (unsigned x = 0 ; x < xAxes; ++x)
		{
			if(*p != nullvalue_)
			{
				if(areas.size() < *p + 1)
				{
					areas.resize(*p + 1, 0);
				}

				areas[*p] += pixelarea;
			}
			++p;
		}
	}
	
	//Now we nullify those that are too small
	p = pixels;
	
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(*p != nullvalue_ && areas[*p] < minSize)
		{
			*p = nullvalue_;
		}
		++p;
	}

}

void ColorMap::tresholdRegionsByRealArea(const double minSize)
{
	const double R0 = radius * PixelArea();
	const double R2 = radius * radius;
	
	//First we compute the area for each color
	vector<double> areas(100, 0);

	unsigned* p = pixels;
	
	const int xmax = xAxes - suncenter.x;
	const int ymax = yAxes - suncenter.y;
	
	for (int y = - suncenter.y; y < ymax; ++y)
	{
		for (int x = - suncenter.x ; x < xmax; ++x)
		{
			if(*p != nullvalue_)
			{
				if(areas.size() < *p + 1)
				{
					areas.resize(*p + 1, 0);
				}

				double pixelArea2 = R2 - (x * x) - (y * y);			
				if(pixelArea2 > 0)
					areas[*p] += R0 / sqrt(pixelArea2);
				else
					areas[*p] = numeric_limits<double>::infinity();
			}
			++p;
		}
	}
	
	//Now we nullify those that are too small
	p = pixels;
	
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(*p != nullvalue_ && areas[*p] < minSize)
		{
			*p = nullvalue_;
		}
		++p;
	}

}
