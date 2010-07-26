#include "Region.h"

using namespace std;

Region::Region()
:id(0),observationTime(0),color(0), first(Coordinate::Max), boxmin(Coordinate::Max), boxmax(0), center(0), numberPixels(0)
{}

Region::Region(const time_t& observationTime)
:id(0),observationTime(observationTime),color(0),first(Coordinate::Max), boxmin(Coordinate::Max), boxmax(0), center(0), numberPixels(0)
{}

Region::Region(const time_t& observationTime, const unsigned id, const unsigned long color)
:id(id),observationTime(observationTime),color(color),first(Coordinate::Max), boxmin(Coordinate::Max), boxmax(0), center(0), numberPixels(0)
{}

bool Region::operator==(const Region& r)const
{return observationTime == r.observationTime && id == r.id;}

unsigned  Region::Id() const
{return id;}

void Region::setId(const unsigned& id)
{this->id = id;}

unsigned long Region::Color() const
{
	return color;
}


void Region::setColor(const unsigned long& color)
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
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year<<"-"<<setw(2)<<date_obs->tm_mon + 1<<"-"<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<":"<<setw(2)<<date_obs->tm_min<<":"<<setw(2)<<date_obs->tm_sec;
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
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year<<setw(2)<<date_obs->tm_mon + 1<<setw(2)<<date_obs->tm_mday<<"T"<<setw(2)<<date_obs->tm_hour<<setw(2)<<date_obs->tm_min<<setw(2)<<date_obs->tm_sec<<"_"<<id;
	return ss.str();
}

string Region::Visu3DLabel() const
{
	tm* date_obs;
	date_obs = gmtime(&observationTime);
	ostringstream ss;
	ss<<setfill('0')<<setw(4)<<date_obs->tm_year<<setw(2)<<date_obs->tm_mon + 1<<setw(2)<<date_obs->tm_mday<<setw(2)<<date_obs->tm_hour<<setw(2)<<date_obs->tm_min;
	return ss.str();
}


const string Region::header = "Id\tColor\tObservationDate\t(Center.x,Center.y)\t(Boxmin.x,Boxmin.y)\t(Boxmax.x,Boxmax.y)\tNumberPixels";

string Region::toString() const
{
	ostringstream out;
	out<<setiosflags(ios::fixed)<<Id()<<"\t"<<Color()<<"\t"<<ObservationDate()<<"\t"<<Center()<<"\t"<<Boxmin()<<"\t"<<Boxmax()<<"\t"<<NumberPixels();
	return out.str();
}

#ifdef CoordinateConvertor_H
string Region::toString(const CoordinateConvertor& coco) const
{
	ostringstream out;
	out<<setiosflags(ios::fixed)<<Id()<<"\t"<<Color()<<"\t"<<ObservationDate();

	float fx, fy;
	out<<setiosflags(ios::fixed);
	coco.convert(Center(), fx, fy);
	out<<"\t"<<"("<<fx<<","<<fy<<")";
	coco.convert(Boxmin(), fx, fy);
	out<<"\t"<<"("<<fx<<","<<fy<<")";
	coco.convert(Boxmax(), fx, fy);
	out<<"\t"<<"("<<fx<<","<<fy<<")";
	
	out<<"\t"<<NumberPixels();
	return out.str();
}
#endif

// Extraction of the regions from a connected component colored Map
vector<Region*> getRegions(const SunImage* colorizedComponentsMap)
{
	vector<Region*> regions;

	unsigned id = 0;
	
	//Let's get the connected regions
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
					regions[color] = new Region(colorizedComponentsMap->ObservationTime(),id, 0);
					++id;
				}
				
				// We add the pixel to the region
				regions[color]->add(Coordinate(x,y));
			}
		}

	}
	

	//We cleanup the null regions
	vector<Region*>::iterator r1 = regions.begin();
	while (r1 != regions.end())
	{
		if(!(*r1))
		{
			vector<Region*>::iterator r2 = r1;
			while( r2 != regions.end() && !(*r2))
				++r2;
			r1 = regions.erase(r1,r2);
		}
		else
			++r1;
	}

	return regions;

}


