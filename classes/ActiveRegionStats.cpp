#include "ActiveRegionStats.h"
#include <map>

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif

#ifndef INF
#define INF (numeric_limits<Real>::infinity())
#endif


ActiveRegionStats::ActiveRegionStats()
:Region(), m1(0), m2(NAN), m3(NAN), m4(NAN), minIntensity(NAN), maxIntensity(NAN), totalIntensity(0), centerxError(0), centeryError(0), area_Raw(0), area_RawUncert(0), area_AtDiskCenter(0), area_AtDiskCenterUncert(0), numberContourPixels(0), barycenter_x(0), barycenter_y(0)
{}

ActiveRegionStats::ActiveRegionStats(const time_t& observationTime)
:Region(observationTime), m1(0), m2(NAN), m3(NAN), m4(NAN), minIntensity(NAN), maxIntensity(NAN), totalIntensity(0), centerxError(0), centeryError(0), area_Raw(0), area_RawUncert(0), area_AtDiskCenter(0), area_AtDiskCenterUncert(0), numberContourPixels(0), barycenter_x(0), barycenter_y(0)
{}

ActiveRegionStats::ActiveRegionStats(const time_t& observationTime, const unsigned id, const ColorType color)
:Region(observationTime, id, color), m1(0), m2(NAN), m3(NAN), m4(NAN), minIntensity(NAN), maxIntensity(NAN), totalIntensity(0), centerxError(0), centeryError(0), area_Raw(0), area_RawUncert(0), area_AtDiskCenter(0), area_AtDiskCenterUncert(0), numberContourPixels(0), barycenter_x(0), barycenter_y(0)
{}


void ActiveRegionStats::add(const Coordinate& pixelCoordinate, const EUVPixelType& pixelIntensity, const Coordinate sunCenter, const bool atBorder, const double R)
{
	Region::add(pixelCoordinate);
	m1 += pixelIntensity;
	if( isnan(maxIntensity) || maxIntensity < pixelIntensity )
		maxIntensity = pixelIntensity;
	if( isnan(minIntensity) || pixelIntensity < minIntensity)
		minIntensity = pixelIntensity;
	totalIntensity += pixelIntensity;
	
	const double R0R2 = (SUNRADIUS / R) * (SUNRADIUS / R);
	const double DR0R0DRR = (DR0 / SUNRADIUS) + (DR / R);
	int relativePixelCoordinatex = pixelCoordinate.x - sunCenter.x;
	int relativePixelCoordinatey = pixelCoordinate.y - sunCenter.y;
	double pixelArea2 = (R * R) - (relativePixelCoordinatex * relativePixelCoordinatex) - (relativePixelCoordinatey * relativePixelCoordinatey);
	double modifiedPixelArea2 = (2 * R * R) - (relativePixelCoordinatex * relativePixelCoordinatex) - (relativePixelCoordinatey * relativePixelCoordinatey);
	double pixelArea;
	if(pixelArea2 > 0)
		pixelArea = R / sqrt(pixelArea2);
	else
		pixelArea = HIGGINS_FACTOR + 1;
	
	area_Raw       += R0R2;
	area_RawUncert += 2 * R0R2 * DR0R0DRR;

	if(atBorder)
	{
		area_RawUncert          += R0R2;
		area_AtDiskCenterUncert += R0R2 * pixelArea;
	}

	if (pixelArea <= HIGGINS_FACTOR)
	{
		area_AtDiskCenter       += R0R2 * pixelArea;
		area_AtDiskCenterUncert += R0R2 * ( (2 * (DR0 / SUNRADIUS) * pixelArea2 + (DR / R) * modifiedPixelArea2 + abs(relativePixelCoordinatex) + abs(relativePixelCoordinatey) ) * 
					   (pixelArea * pixelArea * pixelArea)) / (R * R);	
	}
	#ifndef NONAN_HIGGINS_FACTOR
	else 
	{
		area_AtDiskCenter = NAN;
		area_AtDiskCenterUncert = NAN;
	}
	#endif
	
	centerxError += relativePixelCoordinatex;
	centeryError += relativePixelCoordinatey;
	
	barycenter_x += pixelCoordinate.x * pixelIntensity;
	barycenter_y += pixelCoordinate.y * pixelIntensity;
	// We keep a vector of intensities to compute the variance, the skewness and the kurtosis
	if(intensities.capacity() == intensities.size())
		intensities.reserve(intensities.size() + 100);
	intensities.push_back(pixelIntensity);
	// If I add an intensity, the previously computed moment of order > 1 are invalid
	m2 = NAN;
	m3 = NAN;
	m4 = NAN;
}




Coordinate ActiveRegionStats::Barycenter() const
{
	if (! isnan(totalIntensity) && ! isinf(totalIntensity) && totalIntensity > 0)
		return Coordinate(barycenter_x/totalIntensity, barycenter_y/totalIntensity);
	else
		return Center();
}

Real ActiveRegionStats::CenterxError() const
{
	if (numberPixels > 0)
	{
		return rad2arcsec / distance_observer_sun * (1 + (yearly_maximal_error / distance_observer_sun) * abs(centerxError / numberPixels));
	}
	else
	{
		return INF;
	}
}

Real ActiveRegionStats::CenteryError() const
{
	if (numberPixels > 0)
	{
		return rad2arcsec / distance_observer_sun * (1 + (yearly_maximal_error / distance_observer_sun) * abs(centeryError / numberPixels));
	}
	else
	{
		return INF;
	}
}


Real ActiveRegionStats::MinIntensity() const
{
	if (isinf(minIntensity))
		return NAN;
	else
		return minIntensity;
}

Real ActiveRegionStats::MaxIntensity() const
{
	if (isinf(maxIntensity))
		return NAN;
	else
		return maxIntensity;
}

Real ActiveRegionStats::Mean() const
{
	if(numberPixels == 0)
		return 0;
	else if (isinf(m1) || isnan(m1))
		return NAN;
	else
		return m1 / numberPixels;
}

void ActiveRegionStats::computeMoments()
{
	Real mean = Mean();
	if(isnan(mean) || isinf(mean))
	{
		m2 = m3 = m4 = NAN;
	}
	else
	{
		m2 = m3 = m4 = 0;
		for (unsigned i = 0; i < intensities.size(); ++i)
		{
			Real delta = intensities[i] - mean;
			Real delta2 = delta * delta;
			m2 += delta2;
			m4 += delta2 * delta2;
			m3 += delta2 * delta;
		}
	}
}
Real ActiveRegionStats::Variance() const
{
	if(numberPixels == 0)
		return 0;
	if(isnan(m2))
	{
		const_cast<ActiveRegionStats*>(this)->computeMoments();
	}

	if (isinf(m2) || isnan(m2))
		return NAN;
	else
		return m2 / numberPixels;

}

Real ActiveRegionStats::Skewness() const
{
	if(numberPixels == 0)
		return 0;
	if(isnan(m2) || isnan(m3))
	{
		const_cast<ActiveRegionStats*>(this)->computeMoments();
	}

	if (isinf(m3) || isnan(m3) || isinf(m2) || isnan(m2) || m2 <= 0)
		return NAN;
	else
		return sqrt(numberPixels) * m3 / sqrt(m2 * m2 * m2);

}

Real ActiveRegionStats::Kurtosis() const
{
	if(numberPixels == 0)
		return 0;
	if(isnan(m2) || isnan(m4))
	{
		const_cast<ActiveRegionStats*>(this)->computeMoments();
	}

	if (isinf(m4) || isnan(m4) || isinf(m2) || isnan(m2))
		return NAN;
	else
		return (numberPixels * m4 / (m2 * m2) ) - 3;

}

Real ActiveRegionStats::TotalIntensity() const
{
	if (isinf(totalIntensity))
		return NAN;
	else
		return totalIntensity;
}


Real ActiveRegionStats::Area_Raw() const
{
	return area_Raw;
}		

Real ActiveRegionStats::Area_RawUncert() const
{
	return area_RawUncert;
}

Real ActiveRegionStats::Area_AtDiskCenter() const
{
	return area_AtDiskCenter;
}		

Real ActiveRegionStats::Area_AtDiskCenterUncert() const
{
	return area_AtDiskCenterUncert;
}


string ActiveRegionStats::toString(const string& separator, bool header) const
{
	string result = Region::toString(separator, header);
	if (header)
	{
		result += separator+"Barycenter"+separator+"MinIntensity"+separator+"MaxIntensity"+separator+"Mean"+separator+"Variance"+separator+"Skewness"+separator+"Kurtosis"+separator+"TotalIntensity"+separator+"CenterxError"+separator+"CenteryError"+separator+"Area_Raw"+separator+"Area_RawUncert"+separator+"Area_AtDiskCenter"+separator+"Area_AtDiskCenterUncert";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<separator<<Barycenter()<<separator<<MinIntensity()<<separator<<MaxIntensity()<<separator<<Mean()<<separator<<Variance()<<separator<<Skewness()<<separator<<Kurtosis()<<separator<<TotalIntensity()<<separator<<CenterxError()<<separator<<CenteryError()<<separator<<Area_Raw()<<separator<<Area_RawUncert()<<separator<<Area_AtDiskCenter()<<separator<<Area_AtDiskCenterUncert();
		result += out.str();
	}
	return result;
}


vector<ActiveRegionStats*> getActiveRegionStats(const ColorMap* colorizedComponentsMap, const EUVImage* image)
{
	map<ColorType,ActiveRegionStats*> regions_table;
	Coordinate sunCenter = colorizedComponentsMap->SunCenter();
	double sunRadius = colorizedComponentsMap->SunRadius();
	unsigned id = 0;
	
	//Let's get the connected regions stats
	for (unsigned y = 0; y < colorizedComponentsMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < colorizedComponentsMap->Xaxes(); ++x)
		{
			if(colorizedComponentsMap->pixel(x,y) != colorizedComponentsMap->nullvalue())
			{
				ColorType color = colorizedComponentsMap->pixel(x,y);
				
				// If the regions does not yet exist we create it
				if (regions_table.count(color) == 0)
				{
					regions_table[color] = new ActiveRegionStats(colorizedComponentsMap->ObservationTime(),id, color);
					++id;
				}
				
				//Is the pixel in the contour (<=> there is a neighboor pixel != pixel color)
				bool atBorder = colorizedComponentsMap->pixel(x-1,y) != color || colorizedComponentsMap->pixel(x+1,y) != color || colorizedComponentsMap->pixel(x,y-1) != color || colorizedComponentsMap->pixel(x,y+1) != color;
				
				// We add the pixel to the region
				regions_table[color]->add(Coordinate(x,y), image->pixel(x, y), sunCenter, atBorder, sunRadius);
			}
		}

	}

	
	//We create the vector of regions
	vector<ActiveRegionStats*> regions;
	regions.reserve(regions_table.size());
	for(map<ColorType,ActiveRegionStats*>::const_iterator r = regions_table.begin(); r != regions_table.end(); ++r)
		regions.push_back(r->second);
	
	return regions;

}

FitsFile& writeRegions(FitsFile& file, const vector<ActiveRegionStats*>& ActiveRegionStats)
{

	vector<Region*> regions(ActiveRegionStats.begin(), ActiveRegionStats.end());
	writeRegions(file, regions);

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Barycenter().x + 1;
		file.writeColumn("XBARYCENTER", data);
	}
	
	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Barycenter().y = 1;
		file.writeColumn("YBARYCENTER", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->MinIntensity();
		file.writeColumn("MIN_INTENSITY", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->MaxIntensity();
		file.writeColumn("MAX_INTENSITY", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Mean();
		file.writeColumn("MEAN_INTENSITY", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Variance();
		file.writeColumn("VARIANCE", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Skewness();
		file.writeColumn("SKEWNESS", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Kurtosis();
		file.writeColumn("KURTOSIS", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->TotalIntensity();
		file.writeColumn("TOTAL_INTENSITY", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->CenterxError();
		file.writeColumn("XCENTER_ERROR", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->CenteryError();
		file.writeColumn("YCENTER_ERROR", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Area_Raw();
		file.writeColumn("RAW_AREA", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Area_RawUncert();
		file.writeColumn("RAW_AREA_UNCERTAINITY", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Area_AtDiskCenter();
		file.writeColumn("AREA_ATDISKCENTER", data);
	}

	{
		vector<Real> data(ActiveRegionStats.size());
		for(unsigned r = 0; r < ActiveRegionStats.size(); ++r)
			data[r] = ActiveRegionStats[r]->Area_AtDiskCenterUncert();
		file.writeColumn("AREA_ATDISKCENTER_UNCERTAINITY", data);
	}

	return file;

}
