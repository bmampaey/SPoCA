#include "RegionStats.h"

using namespace std;

RegionStats::RegionStats()
:Region(), m1(0), m2(0), m3(0), m4(0), minIntensity(numeric_limits<PixelType>::max()), maxIntensity(0), totalIntensity(0), centerxError(0), centeryError(0), area_Raw(0), area_RawUncert(0), area_AtDiskCenter(0), area_AtDiskCenterUncert(0), numberContourPixels(0)
{}

RegionStats::RegionStats(const time_t& observationTime)
:Region(observationTime), m1(0), m2(0), m3(0), m4(0), minIntensity(numeric_limits<PixelType>::max()), maxIntensity(0), totalIntensity(0), centerxError(0), centeryError(0), area_Raw(0), area_RawUncert(0), area_AtDiskCenter(0), area_AtDiskCenterUncert(0), numberContourPixels(0)
{}

RegionStats::RegionStats(const time_t& observationTime, const unsigned id, const unsigned long color)
:Region(observationTime, id, color), m1(0), m2(0), m3(0), m4(0), minIntensity(numeric_limits<PixelType>::max()), maxIntensity(0), totalIntensity(0), centerxError(0), centeryError(0), area_Raw(0), area_RawUncert(0), area_AtDiskCenter(0), area_AtDiskCenterUncert(0), numberContourPixels(0)
{}

//The radius of the sun in Mmeters (R0)
const double R0 = 695.508;
//Something. CIS, what is it ?
const double DR0 = 0.026;
//Something else.
const double DR = 2.;
//The higgins_factor

const double HIGGINS_FACTOR = 31;




void RegionStats::add(const Coordinate& pixelCoordinate, const PixelType& pixelIntensity, const Coordinate sunCenter, const bool atBorder, const double R)
{
	Region::add(pixelCoordinate);
	m1 += pixelIntensity;
	if( maxIntensity < pixelIntensity )
		maxIntensity = pixelIntensity;
	if( pixelIntensity < minIntensity)
		minIntensity = pixelIntensity;
	totalIntensity += pixelIntensity;
	
	const double R0R2 = (R0 / R) * (R0 / R);
	double DR0R0DRR= (DR0 / R0) + (DR / R); 
	
	area_Raw += R0R2;
	area_RawUncert += 2 * R0R2 * DR0R0DRR;
	if(atBorder)
	{
		area_RawUncert += R0R2;
	}
	int relativePixelCoordinatex = pixelCoordinate.x - sunCenter.x;
	int relativePixelCoordinatey = pixelCoordinate.y - sunCenter.y;
	double pixelArea2 = (R * R) - (relativePixelCoordinatex * relativePixelCoordinatex) - (relativePixelCoordinatey * relativePixelCoordinatey);
	double pixelArea = R / sqrt(pixelArea2);
	if (pixelArea <= HIGGINS_FACTOR)
	{
		area_AtDiskCenter += R0R2 * pixelArea;
		area_AtDiskCenterUncert += R0R2 * (( 2 *  DR0R0DRR * pixelArea2 + abs(relativePixelCoordinatex) + abs(relativePixelCoordinatey) ) * (pixelArea * pixelArea * pixelArea)) / (R * R);	
	}
	else 
	{
		area_AtDiskCenter = numeric_limits<Real>::infinity();
		area_AtDiskCenterUncert = numeric_limits<Real>::infinity();
	}
	
	centerxError += relativePixelCoordinatex;
	centeryError += relativePixelCoordinatey; 
	
}


void RegionStats::update(const PixelType& pixelIntensity)
{
	Real delta = pixelIntensity - (m1 / numberPixels);
	Real delta2 = delta * delta;
	m2 += delta2;
	m4 += delta2 * delta2;
	m3 += delta2 * delta;

}

const double distance_observer_sun = 149597.871;
const double earth_orbit_eccentricity = 0.0167;
const double yearly_maximal_error = distance_observer_sun * earth_orbit_eccentricity;
const double rad2arcsec = 206264.806247096;

Real RegionStats::CenterxError() const
{
	if (numberPixels > 0)
	{
		return rad2arcsec / distance_observer_sun * (1 + (yearly_maximal_error / distance_observer_sun) * abs(centerxError / numberPixels));
	}
	else
	{
		return numeric_limits<Real>::infinity();
	}
}

Real RegionStats::CenteryError() const
{
	if (numberPixels > 0)
	{
		return rad2arcsec / distance_observer_sun * (1 + (yearly_maximal_error / distance_observer_sun) * abs(centeryError / numberPixels));
	}
	else
	{
		return numeric_limits<Real>::infinity();
	}
}


Real RegionStats::MinIntensity() const
{
	return minIntensity;
}

Real RegionStats::MaxIntensity() const
{
	return maxIntensity;
}

Real RegionStats::Mean() const
{
	if (numberPixels > 0)
		return m1 / numberPixels;
	else
		return numeric_limits<Real>::infinity();
}

Real RegionStats::Variance() const
{
	if (numberPixels > 0)
		return m2 / numberPixels;
	else
		return numeric_limits<Real>::infinity();
}

Real RegionStats::Skewness() const
{
	if (m2 > 0)
		return sqrt(numberPixels) * m3 / sqrt(m2 * m2 * m2);
	else
		return numeric_limits<Real>::infinity();
}

Real RegionStats::Kurtosis() const
{
	if (m2 > 0)
		return (numberPixels * m4 / (m2 * m2) ) - 3;
	else
		return numeric_limits<Real>::infinity();
}

Real RegionStats::TotalIntensity() const
{
	return totalIntensity;
}


Real RegionStats::Area_Raw() const
{
	return area_Raw;
}		

Real RegionStats::Area_RawUncert() const
{
	return area_RawUncert;
}

Real RegionStats::Area_AtDiskCenter() const
{
	return area_AtDiskCenter;
}		

Real RegionStats::Area_AtDiskCenterUncert() const
{
	return area_AtDiskCenterUncert;
}


const string RegionStats::header = Region::header + "MinIntensity\tMaxIntensity\tMean\tVariance\tSkewness\tKurtosis\tTotalIntensity\tCenterxError\tCenteryError\tArea_Raw\tArea_RawUncert\tArea_AtDiskCenter\tArea_AtDiskCenterUncert";

ostream& operator<<(ostream& out, const RegionStats& r)
{
	out<<setiosflags(ios::fixed)<<Region(r)<<"\t"<<r.MinIntensity()<<"\t"<<r.MaxIntensity()<<"\t"<<r.Mean()<<"\t"<<r.Variance()<<"\t"<<r.Skewness()<<"\t"<<r.Kurtosis()<<"\t"<<r.TotalIntensity()<<"\t"<<r.CenterxError()<<"\t"<<r.CenteryError()<<"\t"<<r.Area_Raw()<<"\t"<<r.Area_RawUncert()<<"\t"<<r.Area_AtDiskCenter()<<"\t"<<r.Area_AtDiskCenterUncert();
	return out;
}



vector<RegionStats*> getRegions(const SunImage* colorizedComponentsMap, const SunImage* image)
{
	vector<RegionStats*> regions;
	
	Coordinate sunCenter = colorizedComponentsMap->SunCenter();
	double sunRadius = colorizedComponentsMap->SunRadius();
	unsigned id = 0;
	
	//Let's get the connected regions stats
	for (unsigned y = 0; y < colorizedComponentsMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < colorizedComponentsMap->Xaxes(); ++x)
		{
			if(colorizedComponentsMap->pixel(x,y) != colorizedComponentsMap->nullvalue)
			{
				unsigned color = unsigned(colorizedComponentsMap->pixel(x,y));
					
				//We check the array size before
				if(color >= regions.size())
					regions.resize(color + 100, NULL);
					
				// If the regions does not yet exist we create it
				if (!regions[color])
				{
					regions[color] = new RegionStats(colorizedComponentsMap->ObservationTime(),id, color);
					++id;
				}
				
				//Is the pixel in the contour (<=> there is a neighboor pixel != pixel color)
				bool atBorder = colorizedComponentsMap->pixel(x-1,y) != color || colorizedComponentsMap->pixel(x+1,y) != color || colorizedComponentsMap->pixel(x,y-1) != color || colorizedComponentsMap->pixel(x,y+1) != color;
				
				// We add the pixel to the region
				regions[color]->add(Coordinate(x,y), image->pixel(x, y), sunCenter, atBorder, sunRadius);
			}
		}

	}
	//We make a second pass to calculate the Variance, Skewness and Kurtosis
	for (unsigned j = 0; j < colorizedComponentsMap->NumberPixels(); ++j)
	{
		if(colorizedComponentsMap->pixel(j) != colorizedComponentsMap->nullvalue)
		{
			unsigned color = unsigned(colorizedComponentsMap->pixel(j));
			regions[color]->update(image->pixel(j));
		}
	}

	//We cleanup the null regions
	vector<RegionStats*>::iterator r1 = regions.begin();
	while (r1 != regions.end())
	{
		if(!(*r1))
		{
			vector<RegionStats*>::iterator r2 = r1;
			while( r2 != regions.end() && !(*r2))
				++r2;
			r1 = regions.erase(r1,r2);
		}
		else
			++r1;
	}

	return regions;

}
