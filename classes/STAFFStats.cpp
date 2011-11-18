#include "STAFFStats.h"

using namespace std;

#ifndef NAN
#define NAN (numeric_limits<Real>::quiet_NaN())
#endif

#ifndef INF
#define INF (numeric_limits<Real>::infinity())
#endif



STAFFStats::STAFFStats(const unsigned id)
:id(id), m2(NAN), m3(NAN), m4(NAN), minIntensity(NAN), maxIntensity(NAN), totalIntensity(0), area_Raw(0)
{}


void STAFFStats::add(const EUVPixelType& pixelIntensity, const Real& sun_radius)
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
	
	// We compute the contribution of the pixel to the raw area in Mm²
	area_Raw += (SUN_RADIUS * SUN_RADIUS) / (sun_radius * sun_radius);

}

unsigned STAFFStats::Id() const
{
	return id;
}

void STAFFStats::setId(const unsigned& id)
{
	this->id = id;
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


string STAFFStats::toString(const string& separator, bool header) const
{
	if (header)
	{
		return "Id"+separator+"MinIntensity"+separator+"MaxIntensity"+separator+"Mean"+separator+"Median"+separator+"Variance"+separator+"Skewness"+separator+"Kurtosis"+separator+"TotalIntensity"+separator+"Area_Raw";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<Id()<<separator<<MinIntensity()<<separator<<MaxIntensity()<<separator<<Mean()<<separator<<Median()<<separator<<Variance()<<separator<<Skewness()<<separator<<Kurtosis()<<separator<<TotalIntensity()<<separator<<Area_Raw();
		return out.str();
	}
}


vector<STAFFStats*> getSTAFFStats(const ColorMap* coloredMap, const EUVImage* image, const vector<Region*>& regions)
{
	map<ColorType,STAFFStats*> regions_stats;
	for(unsigned r = 0; r < regions.size(); ++r)
	{
		if (regions_stats.count(regions[r]->Color()) == 0)
			regions_stats[regions[r]->Color()] = new STAFFStats(regions[r]->Id());
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
					regions_stats[color]->add(image->pixel(x, y), sunRadius);
				}
			}
		}
	}
	
	return values(regions_stats);
}

STAFFStats getSTAFFStats(const ColorMap* coloredMap, ColorType color, const EUVImage* image)
{
	STAFFStats stats;
	
	Real sunRadius = image->SunRadius();
	
	for (unsigned j = 0; j < coloredMap->NumberPixels(); ++j)
	{
		if(coloredMap->pixel(j) == color)
		{
			// We add the pixel to the region
			stats.add(image->pixel(j), sunRadius);
		}
	}
	
	return stats;
}

vector<STAFFStats> getSTAFFStats(const ColorMap* CHMap, ColorType CHClass, const ColorMap* ARMap, ColorType ARClass, const EUVImage* image)
{
	vector<STAFFStats> stats(3);
	
	Real sunRadius = image->SunRadius();
	
	for (unsigned j = 0; j < CHMap->NumberPixels(); ++j)
	{
		if(CHMap->pixel(j) == CHClass)
		{
			// We add the pixel to the CH stats
			stats[0].add(image->pixel(j), sunRadius);
		}
		else if(ARMap->pixel(j) == ARClass)
		{
			// We add the pixel to the AR stats
			stats[1].add(image->pixel(j), sunRadius);
		}
		else
		{
			// We add remaining pixels to the QS stats
			stats[2].add(image->pixel(j), sunRadius);
		}
	}
	
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

	return file;

}
