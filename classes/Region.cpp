#include "Region.h"
#include <map>

using namespace std;

Region::Region(const unsigned id)
:id(id),observationTime(0),color(0), first(Coordinate::Max), boxmin(Coordinate::Max), boxmax(0), center(0), numberPixels(0)
{}

Region::Region(const time_t& observationTime)
:id(0),observationTime(observationTime),color(0),first(Coordinate::Max), boxmin(Coordinate::Max), boxmax(0), center(0), numberPixels(0)
{}

Region::Region(const time_t& observationTime, const unsigned id, const ColorType color)
:id(id),observationTime(observationTime),color(color),first(Coordinate::Max), boxmin(Coordinate::Max), boxmax(0), center(0), numberPixels(0)
{}

bool Region::operator==(const Region& r)const
{return observationTime == r.observationTime && id == r.id;}

unsigned  Region::Id() const
{return id;}

void Region::setId(const unsigned& id)
{this->id = id;}

ColorType Region::Color() const
{
	return color;
}


void Region::setColor(const ColorType& color)
{
	this->color = color;
}


Coordinate Region::Boxmin() const
{
	return boxmin;
}

Coordinate Region::Boxmax() const
{
	return boxmax;
}

Coordinate Region::Center() const
{
	if (numberPixels > 0)
		return Coordinate(center.x/numberPixels, center.y/numberPixels);
	else
		return Coordinate::Max;
}

Coordinate Region::FirstPixel() const
{
	return first;
}


unsigned Region::NumberPixels() const
{
	return numberPixels;
}

time_t Region::ObservationTime() const
{
	return observationTime;
}

string Region::ObservationDate() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
	return ss.str();
}

void Region::add(const unsigned& x, const unsigned& y)
{
	if( y < first.y || (y == first.y && x < first.x))
	{
		first.y = y;
		first.x = x;
	}
	boxmin.x = x < boxmin.x ? x : boxmin.x;
	boxmin.y = y < boxmin.y ? y : boxmin.y;
	boxmax.x = x > boxmax.x ? x : boxmax.x;
	boxmax.y = y > boxmax.y ? y : boxmax.y;
	center.x += x;
	center.y += y;
	++ numberPixels;
}


void Region::add(const Coordinate& pixelCoordinate)
{
	this->add(pixelCoordinate.x, pixelCoordinate.y);
}


string Region::HekLabel() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<setw(2)<<date_obs->tm_mon + 1<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<setw(2)<<date_obs->tm_min<<setw(2)<<date_obs->tm_sec<<"_"<<id;
	return ss.str();
}

string Region::Visu3DLabel() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<setw(2)<<date_obs->tm_mon + 1<<setw(2)<<date_obs->tm_mday<<setw(2)<<date_obs->tm_hour<<setw(2)<<date_obs->tm_min;
	return ss.str();
}


string Region::toString(const string& separator, bool header) const
{
	string result;
	if (header)
	{
		result = "Id"+separator+"Color"+separator+"ObservationDate"+separator+"Center"+separator+"Boxmin"+separator+"Boxmax"+separator+"NumberPixels";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<Id()<<separator<<Color()<<separator<<ObservationDate()<<separator<<Center()<<separator<<Boxmin()<<separator<<Boxmax()<<separator<<NumberPixels();
		result = out.str();
	}
	return result;
}

// Extraction of the regions from a connected component colored Map
vector<Region*> getRegions(const ColorMap* colorizedComponentsMap)
{
	map<ColorType,Region*> regions_table;
	unsigned id = 0;
	
	//Let's get the connected regions
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
					regions_table[color] = new Region(colorizedComponentsMap->ObservationTime(),id, color);
					++id;
				}
				
				// We add the pixel to the region
				regions_table[color]->add(Coordinate(x,y));
			}
		}

	}
	
	//We create the vector of regions
	vector<Region*> regions;
	regions.reserve(regions_table.size());
	for(map<ColorType,Region*>::const_iterator r = regions_table.begin(); r != regions_table.end(); ++r)
		regions.push_back(r->second);
	
	return regions;

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
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Center().x + 1;
		file.writeColumn("XCENTER", data);
	}
	
	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Center().y + 1;
		file.writeColumn("YCENTER", data);
	}

	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Boxmin().x + 1;
		file.writeColumn("XBOXMIN", data);
	}

	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Boxmin().y + 1;
		file.writeColumn("YBOXMIN", data);
	}
	
	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Boxmax().x + 1;
		file.writeColumn("XBOXMAX", data);
	}
	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Boxmax().y + 1;
		file.writeColumn("YBOXMAX", data);
	}

	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->NumberPixels();
		file.writeColumn("NUMBER_PIXELS", data);
	}

	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->FirstPixel().x + 1;
		file.writeColumn("XFIRST", data);
	}
	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->FirstPixel().y + 1;
		file.writeColumn("YFIRST", data);
	}

	return file;

}

FitsFile& readRegions(FitsFile& file, vector<Region*>& regions)
{
	// We augment the regions vector
	int firstRegion = regions.size();
	{
		vector<unsigned> data;
		file.readColumn("ID", data);
		for(unsigned i = 0; i < data.size(); ++i)
			regions.push_back(new Region(data[i]));
		
	}

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
		vector<Real> xdata;
		vector<Real> ydata;
		file.readColumn("XCENTER", xdata);
		file.readColumn("YCENTER", ydata);
		if(xdata.size() != ydata.size())
		{
			cerr<<"Error reading region center, number of x coordinate is different than y coordinate!"<<endl;
		}

		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < xdata.size() && i < ydata.size(); ++r, ++i)
			regions[r]->center = Coordinate(xdata[i]-1,ydata[i]-1);
	}

	{
		vector<unsigned> xdata;
		vector<unsigned> ydata;
		file.readColumn("XBOXMIN", xdata);
		file.readColumn("YBOXMIN", ydata);
		if(xdata.size() != ydata.size())
		{
			cerr<<"Error reading region boxmin, number of x coordinate is different than y coordinate!"<<endl;
		}

		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < xdata.size() && i < ydata.size(); ++r, ++i)
			regions[r]->boxmin = Coordinate(xdata[i]-1,ydata[i]-1);
	}

	{
		vector<unsigned> xdata;
		vector<unsigned> ydata;
		file.readColumn("XBOXMAX", xdata);
		file.readColumn("YBOXMAX", ydata);
		if(xdata.size() != ydata.size())
		{
			cerr<<"Error reading region boxmax, number of x coordinate is different than y coordinate!"<<endl;
		}

		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < xdata.size() && i < ydata.size(); ++r, ++i)
			regions[r]->boxmax = Coordinate(xdata[i]-1,ydata[i]-1);
	}


	{
		vector<unsigned> data;
		file.readColumn("NUMBER_PIXELS", data);
		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < data.size(); ++r, ++i)
			regions[r]->numberPixels = data[i];
	}

	{
		vector<unsigned> xdata;
		vector<unsigned> ydata;
		file.readColumn("XFIRST", xdata);
		file.readColumn("YFIRST", ydata);
		if(xdata.size() != ydata.size())
		{
			cerr<<"Error reading region first, number of x coordinate is different than y coordinate!"<<endl;
		}

		for(unsigned r = firstRegion, i = 0; r < regions.size() && i < xdata.size() && i < ydata.size(); ++r, ++i)
			regions[r]->first = Coordinate(xdata[i]-1,ydata[i]-1);
	}

	return file;

}

