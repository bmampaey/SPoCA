#include "RegionStats.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif

#ifndef INF
#define INF (numeric_limits<Real>::infinity())
#endif



RegionStats::RegionStats(const time_t& observationTime, const unsigned id)
:id(id),observationTime(observationTime), numberPixels(0), m2(NAN), m3(NAN), m4(NAN), minIntensity(NAN), maxIntensity(NAN), totalIntensity(0), centerxError(0), centeryError(0), area_Raw(0), area_RawUncert(0), area_AtDiskCenter(0), area_AtDiskCenterUncert(0), numberContourPixels(0), center(0,0), barycenter(0,0), clipped_spatial(false)
{}


void RegionStats::add(const PixLoc& coordinate, const EUVPixelType& pixelIntensity, const RealPixLoc& sunCenter, const bool& atBorder, const Real& sun_radius)
{

	
	// If the intensity is not a number, the event is said to be clipped spatially
	if(isnan(pixelIntensity) || isinf(pixelIntensity))
	{
		clipped_spatial = true;
	}
	else
	{
		if( isnan(maxIntensity) || maxIntensity < pixelIntensity )
			maxIntensity = pixelIntensity;
		
		if( isnan(minIntensity) || pixelIntensity < minIntensity)
			minIntensity = pixelIntensity;
		
		totalIntensity += pixelIntensity;
		
		barycenter.x += coordinate.x * pixelIntensity;
		barycenter.y += coordinate.y * pixelIntensity;
		
		// We keep a vector of intensities to compute the variance, the skewness and the kurtosis
		intensities.push_back(pixelIntensity);
		
		// If I add an intensity, the previously computed moment of order > 1 are invalid
		m2 = NAN;
		m3 = NAN;
		m4 = NAN;
	}
	
	Real dx = fabs(coordinate.x - sunCenter.x);
	Real dy = fabs(coordinate.y - sunCenter.y);
	Real radius_squared = sun_radius * sun_radius;
	Real sigma = radius_squared - (dx * dx) - (dy * dy);
	
	++numberPixels;
	
	// We compute the center
	center.x += coordinate.x;
	center.y += coordinate.y;

	// We compute the center error
	centerxError = fabs(center.x/numberPixels - sunCenter.x);
	centeryError = fabs(center.y/numberPixels - sunCenter.y);
	
	// We compute the contribution of the pixel to the raw area in Mm², and it's uncertainity
	const Real raw_pixel_area = (SUN_RADIUS) * (SUN_RADIUS) / radius_squared;
	
	area_Raw += raw_pixel_area;
	area_RawUncert += 2 * raw_pixel_area * ((SUN_RADIUS_VARIATION / SUN_RADIUS) + (SUN_RADIUS_VARIATION_PIXELS / sun_radius));
	if(atBorder)
		area_RawUncert += raw_pixel_area;
	
	// We compute the contribution of the pixel to the area at disk center in Mm², and it's uncertainity
	Real area_correction_factor = HIGGINS_FACTOR + 1;
	if(sigma > 0)
		area_correction_factor = sun_radius/sqrt(sigma);

	// If the area correction factor is more than some value (i.e. the pixel is near the limb)
	// we mark that the area at disk center will be invalid
	if (area_correction_factor <= HIGGINS_FACTOR)
	{
		area_AtDiskCenter += raw_pixel_area * area_correction_factor;
		area_AtDiskCenterUncert += (raw_pixel_area / sqrt(sigma*sigma*sigma)) * (dx + dy + (2 * (SUN_RADIUS_VARIATION / SUN_RADIUS) * sigma) + ((SUN_RADIUS_VARIATION_PIXELS / sun_radius) * (radius_squared + sigma)));
		if(atBorder)
			area_AtDiskCenterUncert += raw_pixel_area * area_correction_factor;
	}
	else
	{
		clipped_spatial = true;
	}
}

unsigned RegionStats::Id() const
{
	return id;
}

void RegionStats::setId(const unsigned& id)
{
	this->id = id;
}

time_t RegionStats::ObservationTime() const
{
	return observationTime;
}

string RegionStats::ObservationDate() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
	return ss.str();
}

unsigned RegionStats::NumberPixels() const
{
	return numberPixels;
}

RealPixLoc RegionStats::Center() const
{
	if (numberPixels > 0)
		return RealPixLoc(center.x/numberPixels, center.y/numberPixels);
	else
		return center;
}

RealPixLoc RegionStats::Barycenter() const
{
	if (! clipped_spatial && totalIntensity > 0)
		return RealPixLoc(barycenter.x/totalIntensity, barycenter.y/totalIntensity);
	else
		return Center();
}

Real RegionStats::CenterxError() const
{
	return 1 + EARTH_ORBIT_ECCENTRICITY * centerxError;
}

Real RegionStats::CenteryError() const
{
	return 1 + EARTH_ORBIT_ECCENTRICITY * centeryError;
}

Real RegionStats::MinIntensity() const
{
	if (isinf(minIntensity))
		return NAN;
	else
		return minIntensity;
}

Real RegionStats::MaxIntensity() const
{
	if (isinf(maxIntensity))
		return NAN;
	else
		return maxIntensity;
}

Real RegionStats::Mean() const
{
	if (intensities.size() == 0 || isinf(totalIntensity) || isnan(totalIntensity))
		return NAN;
	else
		return totalIntensity / intensities.size();
}

Real RegionStats::Median() const
{
	if (intensities.size() == 0)
		return NAN;
	else
		return quickselect(intensities, 0.5);
}

void RegionStats::computeMoments()
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
Real RegionStats::Variance() const
{
	if (intensities.size() == 0)
		return NAN;
	if(isnan(m2))
	{
		const_cast<RegionStats*>(this)->computeMoments();
	}

	if (isinf(m2) || isnan(m2))
		return NAN;
	else
		return m2 / intensities.size();

}

Real RegionStats::Skewness() const
{
	if (intensities.size() == 0)
		return NAN;
	if(isnan(m2) || isnan(m3))
	{
		const_cast<RegionStats*>(this)->computeMoments();
	}

	if (isinf(m3) || isnan(m3) || isinf(m2) || isnan(m2) || m2 <= 0)
		return NAN;
	else
		return sqrt(intensities.size()) * m3 / sqrt(m2 * m2 * m2);

}

Real RegionStats::Kurtosis() const
{
	if(intensities.size() == 0)
		return NAN;
	if(isnan(m2) || isnan(m4))
	{
		const_cast<RegionStats*>(this)->computeMoments();
	}

	if (isinf(m4) || isnan(m4) || isinf(m2) || isnan(m2) || m2 <= 0)
		return NAN;
	else
		return (intensities.size() * m4 / (m2 * m2) ) - 3;

}

Real RegionStats::TotalIntensity() const
{
	if (isinf(totalIntensity))
		return NAN;
	else
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

bool RegionStats::ClippedSpatial() const
{
	return clipped_spatial;
}

string RegionStats::toString(const string& separator, bool header) const
{
	if (header)
	{
		return "Id"+separator+"ObservationDate"+separator+"NumberPixels"+separator+"Center"+separator+"Barycenter"+separator+"MinIntensity"+separator+"MaxIntensity"+separator+"Mean"+separator+"Median"+separator+"Variance"+separator+"Skewness"+separator+"Kurtosis"+separator+"TotalIntensity"+separator+"CenterxError"+separator+"CenteryError"+separator+"Area_Raw"+separator+"Area_RawUncert"+separator+"Area_AtDiskCenter"+separator+"Area_AtDiskCenterUncert"+separator+"ClippedSpatial";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<Id()<<separator<<ObservationDate()<<separator<<NumberPixels()<<separator<<Center()<<separator<<Barycenter()<<separator<<MinIntensity()<<separator<<MaxIntensity()<<separator<<Mean()<<separator<<Median()<<separator<<Variance()<<separator<<Skewness()<<separator<<Kurtosis()<<separator<<TotalIntensity()<<separator<<CenterxError()<<separator<<CenteryError()<<separator<<Area_Raw()<<separator<<Area_RawUncert()<<separator<<Area_AtDiskCenter()<<separator<<Area_AtDiskCenterUncert()<<separator<<ClippedSpatial();
		return out.str();
	}
}


vector<RegionStats*> getRegionStats(const ColorMap* coloredMap, const EUVImage* image, const vector<Region*>& regions)
{
	map<ColorType,RegionStats*> regions_stats;
	for(unsigned r = 0; r < regions.size(); ++r)
	{
		if (regions_stats.count(regions[r]->Color()) == 0)
			regions_stats[regions[r]->Color()] = new RegionStats(image->ObservationTime(), regions[r]->Id());
	}
	
	RealPixLoc sunCenter = image->SunCenter();
	Real sunRadius = image->SunRadius();
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			if(coloredMap->pixel(x,y) != coloredMap->null())
			{
				const ColorType& color = coloredMap->pixel(x,y);
				
				// We only compute the region stats for the given regions
				if (regions_stats.count(color) > 0)
				{
					//Is the pixel in the contour (<=> there is a neighboor pixel != pixel color)
					bool atBorder = coloredMap->pixel(x-1,y) != color || coloredMap->pixel(x+1,y) != color || coloredMap->pixel(x,y-1) != color || coloredMap->pixel(x,y+1) != color;
				
					// We add the pixel to the region
					regions_stats[color]->add(PixLoc(x,y), image->pixel(x, y), sunCenter, atBorder, sunRadius);
				}
			}
		}
	}
	
	return values(regions_stats);
}

vector<RegionStats*> getRegionStats(const ColorMap* coloredMap, const EUVImage* image)
{
	unsigned id = 0;
	map<ColorType,RegionStats*> regions_stats;
	
	RealPixLoc sunCenter = image->SunCenter();
	Real sunRadius = image->SunRadius();
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			if(coloredMap->pixel(x,y) != coloredMap->null())
			{
				const ColorType& color = coloredMap->pixel(x,y);
				
				// If the regions_stats does not yet exist we create it
				if (regions_stats.count(color) == 0)
				{
					regions_stats[color] = new RegionStats(image->ObservationTime(), id);
					++id;
				}
				
				// Is the pixel in the contour (<=> there is a neighboor pixel != pixel color)
				bool atBorder = coloredMap->pixel(x-1,y) != color || coloredMap->pixel(x+1,y) != color || coloredMap->pixel(x,y-1) != color || coloredMap->pixel(x,y+1) != color;
				
				// We add the pixel to the region
				regions_stats[color]->add(PixLoc(x,y), image->pixel(x, y), sunCenter, atBorder, sunRadius);
			}
		}
	}
	
	return values(regions_stats);
}

vector<SegmentationStats*> getTotalRegionStats(const ColorMap* coloredMap, const EUVImage* image)
{
	vector<SegmentationStats*> regions_stats;
	regions_stats.push_back(new SegmentationStats(image->ObservationTime(), 0));
	regions_stats.push_back(new SegmentationStats(image->ObservationTime(), 1));
	
	RealPixLoc sunCenter = image->SunCenter();
	Real sunRadius = image->SunRadius();
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			if(coloredMap->pixel(x,y) != coloredMap->null())
			{
				regions_stats[1]->add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
			}
			else
			{
				regions_stats[0]->add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
			}
		}
	}
	
	return regions_stats;
}



FitsFile& writeRegions(FitsFile& file, const vector<RegionStats*>& regions_stats)
{
	{
		vector<unsigned> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Id();
		file.writeColumn("ID", data);
	}
	
	{
		vector<string> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->ObservationDate();
		file.writeColumn("DATE_OBS", data);
	}
	
	{
		vector<unsigned> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->NumberPixels();
		file.writeColumn("NUMBER_PIXELS", data);
	}
	
	{
		vector<RealPixLoc> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Center();
		file.writeColumn("CENTER", data);
	}
	
	{
		vector<RealPixLoc> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Barycenter();
		file.writeColumn("BARYCENTER", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->MinIntensity();
		file.writeColumn("MIN_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->MaxIntensity();
		file.writeColumn("MAX_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Mean();
		file.writeColumn("MEAN_INTENSITY", data);
	}
	
	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Median();
		file.writeColumn("MEDIAN_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Variance();
		file.writeColumn("VARIANCE", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Skewness();
		file.writeColumn("SKEWNESS", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Kurtosis();
		file.writeColumn("KURTOSIS", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->TotalIntensity();
		file.writeColumn("TOTAL_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->CenterxError();
		file.writeColumn("XCENTER_ERROR", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->CenteryError();
		file.writeColumn("YCENTER_ERROR", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Area_Raw();
		file.writeColumn("RAW_AREA", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Area_RawUncert();
		file.writeColumn("RAW_AREA_UNCERTAINITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Area_AtDiskCenter();
		file.writeColumn("AREA_ATDISKCENTER", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->Area_AtDiskCenterUncert();
		file.writeColumn("AREA_ATDISKCENTER_UNCERTAINITY", data);
	}
	
	{
		vector<string> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r]->ClippedSpatial() ? "T" : "F";
		file.writeColumn("CLIPPED_SPATIAL", data);
	}
	
	return file;

}
