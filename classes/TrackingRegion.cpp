#include "TrackingRegion.h"

using namespace std;

TrackingRegion::TrackingRegion(const time_t& observationTime, const unsigned id)
:id(id),observationTime(observationTime), numberPixels(0), area_arcsec2(0), area_Mm2(0), center(0,0), HGcenter(0, 0), first(PixLoc::null()), boxmin(PixLoc::null()), boxmax(PixLoc::null())
{}


void TrackingRegion::add(const PixLoc& coordinate, const HGS& longlat, const Real& pixel_area_arcsec2, const Real& pixel_area_Mm2)
{
	// We compute the areas
	++numberPixels;
	area_arcsec2 += pixel_area_arcsec2;
	if (isfinite(pixel_area_Mm2))
		area_Mm2 += pixel_area_Mm2;
	
	// We compute the center
	center.x += coordinate.x;
	center.y += coordinate.y;
	HGcenter += longlat;
	
	// We compute the positions
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
}

unsigned TrackingRegion::Id() const
{
	return id;
}

void TrackingRegion::setId(const unsigned& id)
{
	this->id = id;
}

time_t TrackingRegion::ObservationTime() const
{
	return observationTime;
}

string TrackingRegion::ObservationDate() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
	return ss.str();
}

unsigned TrackingRegion::NumberPixels() const
{
	return numberPixels;
}

Real TrackingRegion::Area_arcsec2() const
{
	return area_arcsec2;
}

Real TrackingRegion::Area_Mm2() const
{
	return area_Mm2;
}

RealPixLoc TrackingRegion::Center() const
{
	if (numberPixels > 0)
		return RealPixLoc(center.x/numberPixels, center.y/numberPixels);
	else
		return RealPixLoc(NAN, NAN);
}

HGS TrackingRegion::HGCenter() const
{
	if (numberPixels > 0)
		return HGS(HGcenter.longitude / numberPixels, HGcenter.latitude / numberPixels);
	else
		return HGS(NAN, NAN);
}


PixLoc TrackingRegion::FirstPixel() const
{
	return first;
}

PixLoc TrackingRegion::Boxmin() const
{
	return boxmin;
}

PixLoc TrackingRegion::Boxmax() const
{
	return boxmax;
}

string TrackingRegion::HekLabel() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year+1900<<setw(2)<<date_obs->tm_mon + 1<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<setw(2)<<date_obs->tm_min<<setw(2)<<date_obs->tm_sec<<"_"<<id;
	return ss.str();
}


string TrackingRegion::toString(const string& separator, bool header) const
{
	if (header)
	{
		return "Id"+separator+"ObservationDate"+separator+"Center"+separator+"HGCenter"+separator+"NumberPixels"+separator+"Area_arcsec2"+separator+"Area_Mm2";
	}
	else
	{
		ostringstream out;
		out<<setiosflags(ios::fixed)<<Id()<<separator<<ObservationDate()<<separator<<Center()<<separator<<HGCenter()<<separator<<NumberPixels()<<separator<<Area_arcsec2()<<separator<<Area_Mm2();
		return out.str();
	}
}

string TrackingRegion::toJSON() const
{
	ostringstream out;
	out<<"{\n";
	out<<"\"Label\": \""<<HekLabel()<<"\",\n";
	out<<"\"Id\": "<<Id()<<",\n";
	out<<"\"ObservationDate\": \""<<ObservationDate()<<"\",\n";
	out<<"\"Center\": \""<<Center()<<"\",\n";
	out<<"\"HGCenter\": \""<<HGCenter()<<"\",\n";
	out<<"\"NumberPixels\" : "<<NumberPixels()<<",\n";
	out<<"\"Area_arcsec2\": "<<Area_arcsec2()<<",\n";
	out<<"\"Area_Mm2\" :"<<Area_Mm2()<<",\n";
	out<<"\"FirstPixel\": \""<<FirstPixel()<<"\",\n";
	out<<"\"BoxMin\": \""<<Boxmin()<<"\",\n";
	out<<"\"BoxMax\": \""<<Boxmax()<<"\"\n";
	out<<"}";
	return out.str();
}

vector<TrackingRegion*> getTrackingRegions(const ColorMap* coloredMap)
{
	map<ColorType, TrackingRegion*> regions;
	Real pixelArea = coloredMap->PixelArea();
	
	for (unsigned y = 0; y < coloredMap->Yaxes(); ++y)
	{
		for (unsigned x = 0; x < coloredMap->Xaxes(); ++x)
		{
			const ColorType& color = coloredMap->pixel(x,y);
			if(color != coloredMap->null())
			{
				// If no region of that color exist we create it
				if(regions.count(color) == 0)
				{
					regions[color] = new TrackingRegion(coloredMap->ObservationTime(), color);
				}
				
				// We add the pixel to the region
				regions[color]->add(PixLoc(x,y), coloredMap->toHGS(RealPixLoc(x,y)), pixelArea, coloredMap->RealPixelArea(PixLoc(x,y)));
			}
		}
	}
	return values(regions);
}

FitsFile& writeRegions(FitsFile& file, const vector<TrackingRegion*>& regions)
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
			data[r] = regions[r]->ObservationDate();
		file.writeColumn("DATE_OBS", data);
	}
	
	{
		vector<unsigned> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->NumberPixels();
		file.writeColumn("NUMBER_PIXELS", data);
	}
	
	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Area_arcsec2();
		file.writeColumn("AREA_ARCSEC2", data);
	}
	
	{
		vector<Real> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Area_Mm2();
		file.writeColumn("AREA_MM2", data);
	}
	
	{
		vector<RealPixLoc> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->Center();
		file.writeColumn("CENTER", data);
	}
	
	{
		vector<HGS> data(regions.size());
		for(unsigned r = 0; r < regions.size(); ++r)
			data[r] = regions[r]->HGCenter();
		file.writeColumn("HGCENTER", data);
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
	
	return file;

}
