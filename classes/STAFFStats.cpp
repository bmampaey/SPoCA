#include "STAFFStats.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif

#ifndef INF
#define INF (numeric_limits<Real>::infinity())
#endif


// CONSTRUCTOR INITIALIZATION EXTENDED BY CIS, Nov 28, 2012
STAFFStats::STAFFStats(const time_t& observationTime, const unsigned id)
:id(id),observationTime(observationTime), m2(NAN), m3(NAN), m4(NAN), minIntensity(NAN), maxIntensity(NAN), totalIntensity(0), area_Raw(0), area_AtDiskCenter(0), fillingFactor(0), numberPixels(0)
{}

// CALL ADAPTED BY CIS TO INCLUDE CENTER AREA AND FILLING FACTOR (Nov 28, 2012)
void STAFFStats::add(const PixLoc& coordinate, const EUVPixelType& pixelIntensity, const RealPixLoc& sunCenter, const Real& sun_radius)
{
	// If the intensity is not a number we omit it
	if(not (isnan(pixelIntensity) || isinf(pixelIntensity)))
	{
		if( isnan(maxIntensity) || maxIntensity < pixelIntensity )
			maxIntensity = pixelIntensity;
		
		if( isnan(minIntensity) || pixelIntensity < minIntensity)
			minIntensity = pixelIntensity;
		
		totalIntensity += pixelIntensity;
		
		// We keep a vector of intensities to compute the variance, the skewness and the kurtosis
		intensities.push_back(pixelIntensity);
		
		// If I add an intensity, the previously computed moment of order > 1 are invalid
		m2 = NAN;
		m3 = NAN;
		m4 = NAN;
	}

	// ADAPTED BY CIS (October 9, 2012)
	Real dx = fabs(coordinate.x - sunCenter.x);
	Real dy = fabs(coordinate.y - sunCenter.y);
	Real radius_squared = sun_radius * sun_radius;
	Real sigma = radius_squared - (dx * dx) - (dy * dy);
	
	++numberPixels;
	
	// We compute the contribution of the pixel to the raw area in Mm²
	const Real raw_pixel_area = (SUN_RADIUS) * (SUN_RADIUS) / radius_squared;
	
	area_Raw += raw_pixel_area;
	
	// We compute the contribution of the pixel to the area at disk center in Mm²
	Real area_correction_factor = HIGGINS_FACTOR + 1;
	if(sigma > 0)
	{
		area_correction_factor = sun_radius/sqrt(sigma);
	}

	// If the area correction factor is more than some value (i.e. the pixel is near the limb)
	if (area_correction_factor <= HIGGINS_FACTOR)
	{
		area_AtDiskCenter += raw_pixel_area * area_correction_factor;
	}
}

unsigned STAFFStats::Id() const
{
	return id;
}

void STAFFStats::setId(const unsigned& id)
{
	this->id = id;
}

time_t STAFFStats::ObservationTime() const
{
	return observationTime;
}

string STAFFStats::ObservationDate() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
	return ss.str();
}

Real STAFFStats::MinIntensity() const
{
	if (isinf(minIntensity))
		return NAN;
	else
		return minIntensity;
}

Real STAFFStats::MaxIntensity() const
{
	if (isinf(maxIntensity))
		return NAN;
	else
		return maxIntensity;
}


// ADDED BY CIS, Nov 28, 2012.
unsigned STAFFStats::NumberPixels() const
{
	return numberPixels;
}


Real STAFFStats::Mean() const
{
	if (intensities.size() == 0 || isinf(totalIntensity) || isnan(totalIntensity))
		return NAN;
	else
		return totalIntensity / intensities.size();
}

Real STAFFStats::Median() const
{
	if (intensities.size() == 0)
		return NAN;
	else
		return quickselect(intensities, 0.5);
}

void STAFFStats::computeMoments()
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
Real STAFFStats::Variance() const
{
	if (intensities.size() == 0)
		return NAN;
	if(isnan(m2))
	{
		const_cast<STAFFStats*>(this)->computeMoments();
	}

	if (isinf(m2) || isnan(m2))
		return NAN;
	else
		return m2 / intensities.size();

}

Real STAFFStats::Skewness() const
{
	if (intensities.size() == 0)
		return NAN;
	if(isnan(m2) || isnan(m3))
	{
		const_cast<STAFFStats*>(this)->computeMoments();
	}

	if (isinf(m3) || isnan(m3) || isinf(m2) || isnan(m2) || m2 <= 0)
		return NAN;
	else
		return sqrt(intensities.size()) * m3 / sqrt(m2 * m2 * m2);

}

Real STAFFStats::Kurtosis() const
{
	if(intensities.size() == 0)
		return NAN;
	if(isnan(m2) || isnan(m4))
	{
		const_cast<STAFFStats*>(this)->computeMoments();
	}

	if (isinf(m4) || isnan(m4) || isinf(m2) || isnan(m2) || m2 <= 0)
		return NAN;
	else
		return (intensities.size() * m4 / (m2 * m2) ) - 3;

}

Real STAFFStats::TotalIntensity() const
{
	if (isinf(totalIntensity))
		return NAN;
	else
		return totalIntensity;
}

Real STAFFStats::Area_Raw() const
{
	return area_Raw;
}		

Real STAFFStats::Area_AtDiskCenter() const
{
	return area_AtDiskCenter;
}		

Real STAFFStats::FillingFactor() const
{
	return fillingFactor;
}

string STAFFStats::toString(const string& separator, bool header) const
{
	if (header)
	{
		return "ObservationDate"+separator+"MinIntensity"+separator+"MaxIntensity"+separator+"Mean"+separator+"Median"+separator+"Variance"+separator+"Skewness"+separator+"Kurtosis"+separator+"TotalIntensity"+separator+"Area_Raw"+separator+"Area_AtDiskCenter"+separator+"FillingFactor";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<ObservationDate()<<separator<<MinIntensity()<<separator<<MaxIntensity()<<separator<<Mean()<<separator<<Median()<<separator<<Variance()<<separator<<Skewness()<<separator<<Kurtosis()<<separator<<TotalIntensity()<<separator<<Area_Raw()<<separator<<Area_AtDiskCenter()<<separator<<FillingFactor();
		return out.str();
	}
}


// METHOD NOT DEFINED IN HEADER FILE!
vector<STAFFStats*> getSTAFFStats(const ColorMap* coloredMap, const EUVImage* image, const vector<Region*>& regions)
{
	map<ColorType,STAFFStats*> regions_stats;
	for(unsigned r = 0; r < regions.size(); ++r)
	{
		if (regions_stats.count(regions[r]->Color()) == 0)
			regions_stats[regions[r]->Color()] = new STAFFStats(image->ObservationTime(), regions[r]->Id());
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
					// We add the pixel to the region
					regions_stats[color]->add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
				}
			}
		}
	}
	
	return values(regions_stats);
}

// CALL ADAPTED BY CIS TO INCLUDE CENTER AREA AND FILLING FACTOR (Nov 28, 2012)
STAFFStats getSTAFFStats(const ColorMap* coloredMap, ColorType color, const EUVImage* image)
{
	STAFFStats stats(image->ObservationTime());
	
	RealPixLoc sunCenter = image->SunCenter();
	Real sunRadius = image->SunRadius();
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			if(coloredMap->pixel(x,y) == color)
			{
				// We add the pixel to the region
				stats.add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
			}
		}
	}
	
	return stats;
}

// CALL ADAPTED BY CIS TO INCLUDE CENTER AREA AND FILLING FACTOR (Nov 28, 2012)
vector<STAFFStats> getSTAFFStats(const ColorMap* CHMap, ColorType CHClass, const ColorMap* ARMap, ColorType ARClass, const EUVImage* image)
{
	vector<STAFFStats> stats(3, image->ObservationTime());
	
	RealPixLoc sunCenter = image->SunCenter();
	Real sunRadius = image->SunRadius();
	
	for (unsigned y = 0; y < CHMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < CHMap->Xaxes(); ++x)
		{
			if(CHMap->pixel(x,y) == CHClass)
			{
				// We add the pixel to the CH stats
				stats[0].add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
			}
			else if(ARMap->pixel(x,y) == ARClass)
			{
				// We add the pixel to the AR stats
				stats[1].add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
			}
			else if(ARMap->pixel(x,y) != ARMap->null() || CHMap->pixel(x,y) != CHMap->null())
			{
				// We add remaining pixels to the QS stats
				stats[2].add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
			}
		}
	}
	
	// Changed by Cis Verbeeck, Nov 28, 2012.
	// Filling factor can only be calculated reliably after the CH+QS+AR area is known.
	// The previous calculation using 1/(pi r^2) produced up to 4% error in filling factor.
	Real areaRaw_CH = stats[0].Area_Raw();
	Real areaRaw_AR = stats[1].Area_Raw();
	Real areaRaw_QS = stats[2].Area_Raw();
	Real totalArea  = areaRaw_CH + areaRaw_AR + areaRaw_QS;
	
	stats[0].fillingFactor = areaRaw_CH / totalArea;
	stats[1].fillingFactor = areaRaw_AR / totalArea;
	stats[2].fillingFactor = areaRaw_QS / totalArea;
	
	return stats;
}

FitsFile& writeRegions(FitsFile& file, const vector<STAFFStats>& regions_stats)
{
	{
		vector<unsigned> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Id();
		file.writeColumn("ID", data);
	}
	
	{
		vector<string> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].ObservationDate();
		file.writeColumn("DATE_OBS", data);
	}
	
	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].MinIntensity();
		file.writeColumn("MIN_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].MaxIntensity();
		file.writeColumn("MAX_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Mean();
		file.writeColumn("MEAN_INTENSITY", data);
	}
	
	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Median();
		file.writeColumn("MEDIAN_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Variance();
		file.writeColumn("VARIANCE", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Skewness();
		file.writeColumn("SKEWNESS", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Kurtosis();
		file.writeColumn("KURTOSIS", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].TotalIntensity();
		file.writeColumn("TOTAL_INTENSITY", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Area_Raw();
		file.writeColumn("RAW_AREA", data);
	}

	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].Area_AtDiskCenter();
		file.writeColumn("AREA_ATDISKCENTER", data);
	}
	
	{
		vector<Real> data(regions_stats.size());
		for(unsigned r = 0; r < regions_stats.size(); ++r)
			data[r] = regions_stats[r].FillingFactor();
		file.writeColumn("FILLING_FACTOR", data);
	}

	return file;
}
