#include "Region.h"
#include <map>

using namespace std;

Region::Region(const unsigned id)
:id(id),observationTime(0), color(0), first(PixLoc::null()), boxmin(PixLoc::null()), boxmax(PixLoc::null()), firstObservationTime(observationTime), numberPixels(0), center(0,0), centerxError(0), centeryError(0), areaProjected(0), areaProjectedUncertainity(0), areaDeprojected(0), areaDeprojectedUncertainity(0)
{}

Region::Region(const time_t& observationTime)
:id(0),observationTime(observationTime), color(0), first(PixLoc::null()), boxmin(PixLoc::null()), boxmax(PixLoc::null()), firstObservationTime(observationTime), numberPixels(0), center(0,0), centerxError(0), centeryError(0), areaProjected(0), areaProjectedUncertainity(0), areaDeprojected(0), areaDeprojectedUncertainity(0)
{}

Region::Region(const time_t& observationTime, const unsigned id, const ColorType color)
:id(id),observationTime(observationTime), color(color), first(PixLoc::null()), boxmin(PixLoc::null()), boxmax(PixLoc::null()), firstObservationTime(observationTime), numberPixels(0), center(0,0), centerxError(0), centeryError(0), areaProjected(0), areaProjectedUncertainity(0), areaDeprojected(0), areaDeprojectedUncertainity(0)
{}

bool Region::operator==(const Region& r)const
{
	return observationTime == r.observationTime && id == r.id;
}

unsigned  Region::Id() const
{
	return id;
}

void Region::setId(const unsigned& id)
{
	this->id = id;
}

ColorType Region::Color() const
{
	return color;
}

void Region::setColor(const ColorType& color)
{
	this->color = color;
}

PixLoc Region::Boxmin() const
{
	return boxmin;
}

PixLoc Region::Boxmax() const
{
	return boxmax;
}

PixLoc Region::FirstPixel() const
{
	return first;
}

time_t Region::ObservationTime() const
{
	return observationTime;
}

time_t Region::FirstObservationTime() const
{
	return firstObservationTime;
}

void Region::setFirstObservationTime(const time_t& t)
{
	firstObservationTime = t;
}

string Region::ObservationDate() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
	return ss.str();
}

string Region::FirstObservationDate() const
{
	tm* date_obs;
	date_obs = gmtime(&firstObservationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
	return ss.str();
}

unsigned Region::NumberPixels() const
{
	return numberPixels;
}

RealPixLoc Region::Center() const
{
	if (numberPixels > 0)
		return RealPixLoc(center.x/numberPixels, center.y/numberPixels);
	else
		return center;
}

Real Region::CenterxError() const
{
	return 1 + EARTH_ORBIT_ECCENTRICITY * centerxError;
}

Real Region::CenteryError() const
{
	return 1 + EARTH_ORBIT_ECCENTRICITY * centeryError;
}

Real Region::Area_Projected() const
{
	return areaProjected;
}

Real Region::Area_Projected_Uncertainity() const
{
	return areaProjectedUncertainity;
}

Real Region::Area_Deprojected() const
{
	return areaDeprojected;
}

Real Region::Area_Deprojected_Uncertainity() const
{
	return areaDeprojectedUncertainity;
}

string Region::HekLabel() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<setw(2)<<date_obs->tm_mon + 1<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<setw(2)<<date_obs->tm_min<<setw(2)<<date_obs->tm_sec<<"_"<<id;
	return ss.str();
}

string Region::toString(const string& separator, bool header) const
{
	if (header)
	{
		return "Id"+separator+"Color"+separator+"ObservationDate"+separator+"FirstObservationDate"+separator+"Boxmin"+separator+"Boxmax";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<Id()<<separator<<Color()<<separator<<ObservationDate()<<separator<<FirstObservationDate()<<separator<<Boxmin()<<separator<<Boxmax();
		return out.str();
	}
}

inline void Region::add(const PixLoc& coordinate, const bool& atBorder, const RealPixLoc& sunCenter, const Real& sun_radius, const Real pixelLength, const Real pixelWidth)
{
	
	// We compute the positions of the first pixel, and the bounding box
	if(!first)
	{
		first = coordinate;
	}
	else if( coordinate.y < first.y || (coordinate.y == first.y && coordinate.x < first.x))
	{
		first.y = coordinate.y;
		first.x = coordinate.x;
	}
	if(!boxmin)
	{
		boxmin = coordinate;
	}
	else
	{
		boxmin.x = coordinate.x < boxmin.x ? coordinate.x : boxmin.x;
		boxmin.y = coordinate.y < boxmin.y ? coordinate.y : boxmin.y;
	}
	if(!boxmax)
	{
		boxmax = coordinate;
	}
	else
	{
		boxmax.x = coordinate.x > boxmax.x ? coordinate.x : boxmax.x;
		boxmax.y = coordinate.y > boxmax.y ? coordinate.y : boxmax.y;
	}
	
	++numberPixels;
	
	// We compute the center
	center.x += coordinate.x;
	center.y += coordinate.y;
	
	centerxError = fabs(center.x/numberPixels - sunCenter.x);
	centeryError = fabs(center.y/numberPixels - sunCenter.y);
	
	// We compute the contribution of the pixel to the area as seen by the observer (arcsec²)
	const Real pixelAreaArcsec2 = pixelLength * pixelWidth;
	areaProjected += pixelAreaArcsec2;
	areaProjectedUncertainity += 2 * pixelAreaArcsec2 * ((SUN_RADIUS_VARIATION / SUN_RADIUS) + (SUN_RADIUS_VARIATION_PIXELS / sun_radius));
	if(atBorder)
		areaProjectedUncertainity += pixelAreaArcsec2;
	
	// We compute the contribution of the pixel to the area on the solar sphere (Mm²)
	// Compute the area error
	const Real dx = fabs(coordinate.x - sunCenter.x);
	const Real dy = fabs(coordinate.y - sunCenter.y);
	const Real radius_squared = sun_radius * sun_radius;
	const Real sigma = radius_squared - (dx * dx) - (dy * dy);
	const Real pixelAreaMm2 = (SUN_RADIUS * SUN_RADIUS) / radius_squared;
	Real areaCorrectionFactor = HIGGINS_FACTOR;
	
	if(sigma > 0)
		areaCorrectionFactor = sun_radius/sqrt(sigma);
	
	// Limit the correction factor to some max limit in case the pixel is near the limb
	if (areaCorrectionFactor > HIGGINS_FACTOR)
		areaCorrectionFactor = HIGGINS_FACTOR;
	
	areaDeprojected += pixelAreaMm2 * areaCorrectionFactor;
	areaDeprojectedUncertainity += (pixelAreaMm2 / sqrt(sigma*sigma*sigma)) * (dx + dy + (2 * (SUN_RADIUS_VARIATION / SUN_RADIUS) * sigma) + ((SUN_RADIUS_VARIATION_PIXELS / sun_radius) * (radius_squared + sigma)));
	if(atBorder)
		areaDeprojectedUncertainity += pixelAreaMm2 * areaCorrectionFactor;
}

vector<Region*> getRegions(const ColorMap* coloredMap)
{
	unsigned id = 0;
	map<ColorType, Region*> regions;
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			if(coloredMap->pixel(x,y) != coloredMap->null())
			{
				const ColorType& color = coloredMap->pixel(x,y);
				// If no region of that color exist we create it
				if(regions.count(color) == 0)
				{
					regions[color] = new Region(coloredMap->ObservationTime(), id, color);
					++id;
				}
				
				// Is the pixel in the contour (<=> there is a neighboor pixel != pixel color)
				bool atBorder = coloredMap->pixel(x-1,y) != color || coloredMap->pixel(x+1,y) != color || coloredMap->pixel(x,y-1) != color || coloredMap->pixel(x,y+1) != color;
				
				// We add the pixel to the region
				regions[color]->add(PixLoc(x,y), atBorder, coloredMap->SunCenter(), coloredMap->SunRadius(), coloredMap->PixelLength(), coloredMap->PixelWidth());
			}
		}
	}
	return values(regions);
}

vector<Region*> getRegions(const ColorMap* coloredMap, const set<ColorType>& colors)
{
	unsigned id = 0;
	
	map<ColorType, Region*> regions;
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			if(coloredMap->pixel(x,y) != coloredMap->null())
			{
				const ColorType& color = coloredMap->pixel(x,y);
				if(colors.count(color) > 1)
				{
					// If no region of that color exist we create it
					if(regions.count(color) == 0)
					{
						regions[color] = new Region(coloredMap->ObservationTime(),id, color);
						++id;
					}
					// Is the pixel in the contour (<=> there is a neighboor pixel != pixel color)
					bool atBorder = coloredMap->pixel(x-1,y) != color || coloredMap->pixel(x+1,y) != color || coloredMap->pixel(x,y-1) != color || coloredMap->pixel(x,y+1) != color;
					
					// We add the pixel to the region
					regions[color]->add(PixLoc(x,y), atBorder, coloredMap->SunCenter(), coloredMap->SunRadius(), coloredMap->PixelLength(), coloredMap->PixelWidth());
				}
			}
		}
	}
	return values(regions);
}

FitsFile& writeRegions(FitsFile& file, const vector<Region*>& regions)
{
	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Id();
		file.writeColumn("ID", data);
	}

	{
		vector<string> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->HekLabel();
		file.writeColumn("HEKID", data);
	}

	{
		vector<ColorType> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Color();
		file.writeColumn("COLOR", data);
	}

	{
		vector<string> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->ObservationDate();
		file.writeColumn("DATE_OBS", data);
	}

	{
		vector<string> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->FirstObservationDate();
		file.writeColumn("FIRST_DATE_OBS", data);
	}

	
	{
		vector<PixLoc> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Boxmin();
		file.writeColumn("BOXMIN", data);
	}

	{
		vector<PixLoc> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Boxmax();
		file.writeColumn("BOXMAX", data);
	}

	{
		vector<PixLoc> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->FirstPixel();
		file.writeColumn("FIRST", data);
	}

	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->NumberPixels();
		file.writeColumn("NUMBER_PIXELS", data);
	}

	{
		vector<RealPixLoc> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Center();
		file.writeColumn("CENTER", data);
	}

	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->CenterxError();
		file.writeColumn("XCENTER_ERROR", data);
	}

	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->CenteryError();
		file.writeColumn("YCENTER_ERROR", data);
	}

	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Area_Projected();
		file.writeColumn("AREA_PROJECTED", data);
	}

	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Area_Projected_Uncertainity();
		file.writeColumn("AREA_PROJECTED_UNCERTAINITY", data);
	}

	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Area_Deprojected();
		file.writeColumn("AREA_DEPROJECTED", data);
	}

	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Area_Deprojected_Uncertainity();
		file.writeColumn("AREA_DEPROJECTED_UNCERTAINITY", data);
	}

	return file;
}

FitsFile& readRegions(FitsFile& file, vector<Region*>& regions, bool getTrackedColors)
{
	// We augment the regions vector
	int firstRegion = regions.size();
	{
		vector<unsigned> data;
		file.readColumn("ID", data);
		for(unsigned i = 0; i < data.size(); ++i)
			regions.push_back(new Region(data[i]));
		
	}
	if(getTrackedColors)
	{
		if(file.find_column("TRACKED_COLOR"))
		{
			vector<unsigned> data;
			file.readColumn("TRACKED_COLOR", data);
			for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
				regions[r]->color = data[i];
		}
		else
		{
			for(unsigned r = firstRegion; r < regions.size(); ++r)
				regions[r]->color = 0;
		}
	}
	else
	{
		vector<unsigned> data;
		file.readColumn("COLOR", data);
		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
			regions[r]->color = data[i];
	}

	{
		vector<string> data;
		file.readColumn("DATE_OBS", data);
		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
			regions[r]->observationTime = iso2ctime(data[i]);
	}
	
	{
		vector<string> data;
		file.readColumn("FIRST_DATE_OBS", data);
		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
			regions[r]->firstObservationTime = iso2ctime(data[i]);
	}

	{
		vector<PixLoc> data;
		file.readColumn("BOXMIN", data);
		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
			regions[r]->boxmin = data[i];
	}
	
	{
		vector<PixLoc> data;
		file.readColumn("BOXMAX", data);
		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
			regions[r]->boxmax = data[i];
	}

	{
		vector<PixLoc> data;
		file.readColumn("FIRST", data);
		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
			regions[r]->first = data[i];
	}

	return file;
}

/*!
To extract the chain code of a connected component, we first search the first pixel on the external boundary.
Then we list all the points along the boundary starting from that first pixel.
Once The list is made, it is reduced by trying to find the most relevant points.
First we take the firstpixel point and it's furthest point in the list, and add them to the chain code.
Then we search the point that is the furthest from the line passing by each pair of consecutive point in the chain code, and add it to the chain code.
We repeat that last step until we have enough points

@param image The image to use for the computation of the chaincode
@param max_points The maximal number of chaincode points to list.
@param max_deviation The maximal deviation of the chaincode curve between 2 points, in arcsec.

*/

vector<PixLoc> Region::chainCode(const ColorMap* image, const unsigned min_points, const unsigned max_points, Real max_deviation) const
{
	deque<PixLoc> chain;
	
	max_deviation /= sqrt(image->PixelArea());
	unsigned xAxes = image->Xaxes();
	
	// We list all the directions
	int directions[] = {	0 + int(xAxes), //Norh
				1 + int(xAxes), //NE
				1 + 0, //East
				1 - int(xAxes), //SE
				0 - int(xAxes), //South
				-1 - int(xAxes), //SW
				-1 + 0, //West
				-1 + int(xAxes), //NW
				//We repeat for simplicity
				0 + int(xAxes), 1 + int(xAxes), 1 + 0, 1 - int(xAxes), 0 - int(xAxes), -1 - int(xAxes), -1 + 0, -1 + int(xAxes)
			};
	
	// We search the left most pixel on the external border
	PixLoc firstPixel = PixLoc(boxmin.x, boxmax.y);
	while(image->pixel(firstPixel) != color)
	{
		if(firstPixel.y == boxmin.y)
		{
			if(firstPixel.x == boxmax.x)
			{
				cerr<<"Empty region, no chaincode"<<endl;
				return vector<PixLoc>();
			}
			else
			{
				firstPixel.y = boxmax.y;
				firstPixel.x += 1;
			}
		}
		else
		{
			firstPixel.y -= 1;
		}
	}
	
	// We start at the first pixel, and we search for the first direction
	chain.push_back(firstPixel);
	unsigned cur_p = firstPixel.x + firstPixel.y * xAxes;
	int first_direction = 0;
	bool found = false;
	for (unsigned i = 0; i <= 8; ++i)
	{
		int next_p = cur_p + directions[first_direction];
		if(next_p >= 0 && unsigned(next_p) < image->NumberPixels())
		{
			if(image->pixel(next_p) != color)
			{
				found=true;
			}
			else if (found)
			{
				break;
			}
		}
		++first_direction;
	}
	// If we are a single pixel, we return
	if (first_direction > 8)
	{
		return vector<PixLoc>(chain.begin(), chain.end());
	}
	first_direction%=8;

	// We create a classical chaincode of all pixels locations along the boundary
	// until we come back to the first pixel, with the same direction
	PixLoc current_pixel = firstPixel;
	int next_direction = first_direction;
	
	// At the same time I search for the pixel the furthest away from the first_pixel
	// See below
	Real biggest_distance = 0.;
	unsigned furthest_pixel_indice = 0;
	
	do
	{
		// We move the current pixel into the next direction
		cur_p = cur_p + directions[next_direction];
		// I add the current pixel to the chain
		current_pixel.x = cur_p%xAxes;
		current_pixel.y = unsigned(cur_p/xAxes);
		chain.push_back(current_pixel);
		// I check if it is the furthest pixel
		Real d = distance_squared(firstPixel, current_pixel);
		if(d > biggest_distance)
		{
			biggest_distance = d;
			furthest_pixel_indice = chain.size() - 1;
		}
		// We search the direction of the following pixel on the border by looking at all directions
		// Starting at the opposite direction +1
		next_direction = (next_direction + 5) % 8;
		for (unsigned i = 0; i < 8; ++i)
		{
			int next_p = cur_p + directions[next_direction];
			if(next_p >= 0 && unsigned(next_p) < image->NumberPixels() && image->pixel(next_p) == color)
			{
				break;
			}
			++next_direction;
		}
		next_direction %= 8;
		
	}while (!((firstPixel == current_pixel) && (next_direction == first_direction)));


	if(chain.size() <= max_points && max_deviation <= 0)
	{
		return vector<PixLoc>(chain.begin(), chain.end());
	}

	// Now we reduce the chain code to max_points
	// The good_indices is a sorted list of the indices of most important points in the chain code
	vector<unsigned> good_indices;
	good_indices.reserve(max_points);
	good_indices.push_back(0);
	// The tmp_indices is a list of the indices of some important points in the chain code
	// The corresponding distances list specify for each tmp_indice the distance to the current reduced chain code
	vector<unsigned> tmp_indices;
	vector<Real> distances;
	tmp_indices.reserve(max_points);
	distances.reserve(max_points);
	tmp_indices.push_back(furthest_pixel_indice);
	distances.push_back(biggest_distance);
	while(good_indices.size() < max_points && tmp_indices.size() > 0)
	{
		// I search in the tmp_indices for the worst point, i.e. with the biggest distance
		unsigned worst_indice = 0;
		Real max_distance = distances[worst_indice];
		for(unsigned i = 1; i < tmp_indices.size(); ++i)
		{
			if(max_distance < distances[i])
			{
				max_distance = distances[i];
				worst_indice = i;
			}
		}
		
		if (max_distance <  max_deviation && good_indices.size() >= min_points)
			break;
		
		// I remove the worst point from the tmp_indices
		unsigned worst_point_indice = tmp_indices[worst_indice];
		tmp_indices.erase(tmp_indices.begin()+worst_indice);
		distances.erase(distances.begin()+worst_indice);
		// I add the worst point indice to the good_indices
		vector<unsigned>::iterator it;
		for(it = good_indices.begin(); it != good_indices.end() && worst_point_indice > *it; ++it){}
		it = good_indices.insert(it, worst_point_indice);

		// I search for the worst point between the worst_point_indice and the previous point from the good_indices
		unsigned previous_indice = *(it-1);
		// We compute the line equation ax+by+c=0 passing between points chain[previous_indice] and chain[worst_point_indice]
		Real a = Real(chain[worst_point_indice].y) - chain[previous_indice].y;
		Real b = Real(chain[previous_indice].x) - chain[worst_point_indice].x;
		Real c = - (b * chain[previous_indice].y + a * chain[previous_indice].x);
		// We search the pixel in the chain that is the furthest to the line
		max_distance = 0;
		worst_indice = previous_indice+1;
		for(unsigned i = previous_indice+1; i < worst_point_indice; ++i)
		{
			Real d =  abs(a * chain[i].x + b * chain[i].y + c);
			if(d >= max_distance)
			{
				max_distance = d;
				worst_indice = i;
			}
		}
		max_distance/=sqrt(a*a+b*b);
		// We add that new worst point to the tmp_indices
		if(worst_indice != worst_point_indice)
		{
			tmp_indices.push_back(worst_indice);
			distances.push_back(max_distance);
		}
		// I search for the worst point between the worst_point_indice and the next point from the good_indices
		unsigned next_indice = (it+1 != good_indices.end()) ? *(it+1) : chain.size()-1;
		// We compute the line equation ax+by+c=0 passing between points chain[worst_point_indice] and chain[next_indice]
		a = Real(chain[next_indice].y) - chain[worst_point_indice].y;
		b = Real(chain[worst_point_indice].x) - chain[next_indice].x;
		c = - (b * chain[worst_point_indice].y + a * chain[worst_point_indice].x);
		// We search the pixel in the chain that is the furthest to the line
		max_distance = 0;
		worst_indice = worst_point_indice+1;
		for(unsigned i = worst_point_indice+1; i < next_indice; ++i)
		{
			Real d =  abs(a * chain[i].x + b * chain[i].y + c);
			if(d >= max_distance)
			{
				max_distance = d;
				worst_indice = i;
			}
		}
		max_distance/=sqrt(a*a+b*b);
		// We add that new worst point to the tmp_indices
		if(worst_indice != next_indice)
		{
			tmp_indices.push_back(worst_indice);
			distances.push_back(max_distance);
		}
	}
	// I compute the reduced chain by using the good_indices
	vector<PixLoc> reduced_chain(good_indices.size());
	for(unsigned i=0; i < good_indices.size(); ++i)
	{
		reduced_chain[i]=chain[good_indices[i]];
	}
	return reduced_chain;
}
