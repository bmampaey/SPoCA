#include "SegmentationStats.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif

#ifndef INF
#define INF (numeric_limits<Real>::infinity())
#endif



SegmentationStats::SegmentationStats(const time_t& observationTime, const unsigned id)
:id(id),observationTime(observationTime), numberPixels(0), m2(NAN), m3(NAN), m4(NAN), minIntensity(NAN), maxIntensity(NAN), totalIntensity(0), area_Raw(0), area_AtDiskCenter(0), fillingFactor(0)
{}


void SegmentationStats::add(const PixLoc& coordinate, const EUVPixelType& pixelIntensity, const RealPixLoc& sunCenter, const Real& sun_radius)
{

	// If the intensity is not a number, the event is said to be clipped spatially
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
	
	Real dx = fabs(coordinate.x - sunCenter.x);
	Real dy = fabs(coordinate.y - sunCenter.y);
	Real radius_squared = sun_radius * sun_radius;
	Real sigma = radius_squared - (dx * dx) - (dy * dy);
	
	++numberPixels;
	
	// We compute the contribution of the pixel to the raw area in Mm², and it's uncertainity
	const Real raw_pixel_area = (SUN_RADIUS) * (SUN_RADIUS) / radius_squared;
	
	area_Raw += raw_pixel_area;
	
	// We compute the contribution of the pixel to the area at disk center in Mm², and it's uncertainity
	Real area_correction_factor = HIGGINS_FACTOR + 1;
	if(sigma > 0)
	{
		area_correction_factor = sun_radius/sqrt(sigma);
		// We compute the filling factor
		fillingFactor += 1./(PI*radius_squared);
	}

	// If the area correction factor is more than some value (i.e. the pixel is near the limb)
	if (area_correction_factor <= HIGGINS_FACTOR)
	{
		area_AtDiskCenter += raw_pixel_area * area_correction_factor;
	}
}

unsigned SegmentationStats::Id() const
{
	return id;
}

void SegmentationStats::setId(const unsigned& id)
{
	this->id = id;
}

time_t SegmentationStats::ObservationTime() const
{
	return observationTime;
}

string SegmentationStats::ObservationDate() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
	return ss.str();
}

unsigned SegmentationStats::NumberPixels() const
{
	return numberPixels;
}

Real SegmentationStats::MinIntensity() const
{
	if (isinf(minIntensity))
		return NAN;
	else
		return minIntensity;
}

Real SegmentationStats::MaxIntensity() const
{
	if (isinf(maxIntensity))
		return NAN;
	else
		return maxIntensity;
}

Real SegmentationStats::Mean() const
{
	if (intensities.size() == 0 || isinf(totalIntensity) || isnan(totalIntensity))
		return NAN;
	else
		return totalIntensity / intensities.size();
}

Real SegmentationStats::Median() const
{
	if (intensities.size() == 0)
		return NAN;
	else
		return quickselect(intensities, 0.5);
}

void SegmentationStats::computeMoments()
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
Real SegmentationStats::Variance() const
{
	if (intensities.size() == 0)
		return NAN;
	if(isnan(m2))
	{
		const_cast<SegmentationStats*>(this)->computeMoments();
	}

	if (isinf(m2) || isnan(m2))
		return NAN;
	else
		return m2 / intensities.size();

}

Real SegmentationStats::Skewness() const
{
	if (intensities.size() == 0)
		return NAN;
	if(isnan(m2) || isnan(m3))
	{
		const_cast<SegmentationStats*>(this)->computeMoments();
	}

	if (isinf(m3) || isnan(m3) || isinf(m2) || isnan(m2) || m2 <= 0)
		return NAN;
	else
		return sqrt(intensities.size()) * m3 / sqrt(m2 * m2 * m2);

}

Real SegmentationStats::Kurtosis() const
{
	if(intensities.size() == 0)
		return NAN;
	if(isnan(m2) || isnan(m4))
	{
		const_cast<SegmentationStats*>(this)->computeMoments();
	}

	if (isinf(m4) || isnan(m4) || isinf(m2) || isnan(m2) || m2 <= 0)
		return NAN;
	else
		return (intensities.size() * m4 / (m2 * m2) ) - 3;

}

Real SegmentationStats::TotalIntensity() const
{
	if (isinf(totalIntensity))
		return NAN;
	else
		return totalIntensity;
}

Real SegmentationStats::Area_Raw() const
{
	return area_Raw;
}		

Real SegmentationStats::Area_AtDiskCenter() const
{
	return area_AtDiskCenter;
}		


Real SegmentationStats::FillingFactor() const
{
	return fillingFactor;
}

string SegmentationStats::toString(const string& separator, bool header) const
{
	if (header)
	{
		return "Id"+separator+"ObservationDate"+separator+"NumberPixels"+separator+"MinIntensity"+separator+"MaxIntensity"+separator+"Mean"+separator+"Median"+separator+"Variance"+separator+"Skewness"+separator+"Kurtosis"+separator+"TotalIntensity"+separator+"Area_Raw"+separator+"Area_AtDiskCenter"+separator+"FillingFactor";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<Id()<<separator<<ObservationDate()<<separator<<NumberPixels()<<separator<<MinIntensity()<<separator<<MaxIntensity()<<separator<<Mean()<<separator<<Median()<<separator<<Variance()<<separator<<Skewness()<<separator<<Kurtosis()<<separator<<TotalIntensity()<<separator<<Area_Raw()<<separator<<Area_AtDiskCenter()<<separator<<FillingFactor();
		return out.str();
	}
}


vector<SegmentationStats*> getSegmentationStats(const ColorMap* coloredMap, const EUVImage* image, const vector<ColorType>& classes)
{
	map<ColorType,SegmentationStats*> segmentation_stats;
	for(unsigned r = 0; r < classes.size(); ++r)
	{
		if (segmentation_stats.count(classes[r]) == 0)
			segmentation_stats[classes[r]] = new SegmentationStats(classes[r]);
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
				
				// We only compute the class stats for the given classes
				if (segmentation_stats.count(color) > 0)
				{
					// We add the pixel to the class
					segmentation_stats[color]->add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
				}
			}
		}
	}
	
	return values(segmentation_stats);
}

vector<SegmentationStats*> getSegmentationStats(const ColorMap* coloredMap, const EUVImage* image)
{
	map<ColorType,SegmentationStats*> segmentation_stats;
	
	RealPixLoc sunCenter = image->SunCenter();
	Real sunRadius = image->SunRadius();
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			if(coloredMap->pixel(x,y) != coloredMap->null())
			{
				const ColorType& color = coloredMap->pixel(x,y);
				
				// If the segmentation_stats does not yet exist we create it
				if (segmentation_stats.count(color) == 0)
				{
					segmentation_stats[color] = new SegmentationStats(image->ObservationTime(), color);
				}
				// We add the pixel to the class
				segmentation_stats[color]->add(PixLoc(x,y), image->pixel(x, y), sunCenter, sunRadius);
			}
		}
	}
	
	return values(segmentation_stats);
}

FitsFile& writeRegions(FitsFile& file, const vector<SegmentationStats*>& segmentation_stats)
{
	{
		vector<unsigned> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Id();
		file.writeColumn("ID", data);
	}
	
	{
		vector<string> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->ObservationDate();
		file.writeColumn("DATE_OBS", data);
	}
	
	{
		vector<unsigned> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->NumberPixels();
		file.writeColumn("NUMBER_PIXELS", data);
	}
	
	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->MinIntensity();
		file.writeColumn("MIN_INTENSITY", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->MaxIntensity();
		file.writeColumn("MAX_INTENSITY", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Mean();
		file.writeColumn("MEAN_INTENSITY", data);
	}
	
	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Median();
		file.writeColumn("MEDIAN_INTENSITY", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Variance();
		file.writeColumn("VARIANCE", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Skewness();
		file.writeColumn("SKEWNESS", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Kurtosis();
		file.writeColumn("KURTOSIS", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->TotalIntensity();
		file.writeColumn("TOTAL_INTENSITY", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Area_Raw();
		file.writeColumn("RAW_AREA", data);
	}

	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->Area_AtDiskCenter();
		file.writeColumn("AREA_ATDISKCENTER", data);
	}
	
	{
		vector<Real> data(segmentation_stats.size());
		for(unsigned r = 0; r < segmentation_stats.size(); ++r)
			data[r] = segmentation_stats[r]->FillingFactor();
		file.writeColumn("FILLING_FACTOR", data);
	}
	
	return file;

}
