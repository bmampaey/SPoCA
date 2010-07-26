#include "CoordinateConvertor.h"

using namespace std;
	
CoordinateConvertor::CoordinateConvertor(SunImage* image, string coordinateType)
:coordinateType(coordinateType)
{
	//IDL_INIT_BACKGROUND | IDL_INIT_QUIET;
	if(instances == 0)
	{
		if (!IDL_Init(IDL_INIT_BACKGROUND, NULL, NULL))
		{
			//Do some error message
		}
	}
	wcs = "wcs" + lastId;
	string create_header = "header = FLOAT("+itos(image->header.size())+")";
	IDL_ExecuteStr(const_cast<char *>(create_header.c_str()));
	for (unsigned h = 0; h < image->header.size(); ++h)
	{
		string add_key = "header["+ itos(h) +"] = "+ image->header[h];
		IDL_ExecuteStr(const_cast<char *>(add_key.c_str()));
	}
	
	string create_wcs = wcs + " = fitshead2wcs(header)";
	IDL_ExecuteStr(const_cast<char *>(create_wcs.c_str()));
	
	++lastId;
	++instances;
} 

CoordinateConvertor::CoordinateConvertor(SunImage* image)
{
	//IDL_INIT_BACKGROUND | IDL_INIT_QUIET;
	if(instances == 0)
	{
		if (!IDL_Init(IDL_INIT_BACKGROUND, NULL, NULL))
		{
			//Do some error message
		}
	}
	wcs = "wcs" + lastId;
	string create_header = "header = FLOAT("+itos(image->header.size())+")";
	IDL_ExecuteStr(const_cast<char *>(create_header.c_str()));
	for (unsigned h = 0; h < image->header.size(); ++h)
	{
		string add_key = "header["+ itos(h) +"] = "+ image->header[h];
		IDL_ExecuteStr(const_cast<char *>(add_key.c_str()));
	}
	
	string create_wcs = wcs + " = fitshead2wcs(header)";
	IDL_ExecuteStr(const_cast<char *>(create_wcs.c_str()));
	
	++lastId;
	++instances;
} 


CoordinateConvertor::CoordinateConvertor(const CoordinateConvertor& cc)
:wcs(cc.wcs), coordinateType(cc.coordinateType)
{
	++instances;
} 

CoordinateConvertor::~CoordinateConvertor()
{
	--instances;
	if(instances == 0)
	{
		IDL_Cleanup(IDL_FALSE);
	}
}

void CoordinateConvertor::convert(Coordinate c, float& x, float& y, bool arcsec) const
{
	this->convert(coordinateType, c, x, y, arcsec);
}


void CoordinateConvertor::convert(string coord_type, Coordinate c, float& x, float& y, bool arcsec) const
{
	
	if(coord_type.empty())
	{
		x = c.x;
		y = c.y;
		return;
	}
	
	string arcsecond = arcsec ? "1" : "0";
	
	string carrington = coord_type == "HGC" ? "1" : "0";
	coord_type = coord_type == "HGC" ? "HG" : coord_type;
	
	string declare_cartesian = "cartesian = [" + itos(c.x) + "," + itos(c.y) + "]";
	IDL_ExecuteStr(const_cast<char *>(declare_cartesian.c_str()));
	
	// We convert the cartesian pixel coodinates into WCS
	string convert2wcs = "wcs_coord = WCS_GET_COORD("+ wcs +", cartesian)";
	IDL_ExecuteStr(const_cast<char *>(convert2wcs.c_str()));
	
	// We convert the WCS coodinates into helioprojective cartesian
	string convert2coord = "WCS_CONVERT_FROM_COORD, "+ wcs +", wcs_coord, '" + coord_type + "', ARCSECONDS = " + arcsecond  + ", CARRINGTON = " + carrington + ", x, y";
	IDL_ExecuteStr(const_cast<char *>(convert2coord.c_str()));
	IDL_VPTR v;
	v = IDL_FindNamedVariable((char *)"x", IDL_FALSE);
	x = v->value.f ;
	v = IDL_FindNamedVariable((char *)"y", IDL_FALSE);
	y = v->value.f ;
}

unsigned CoordinateConvertor::instances = 0;
unsigned CoordinateConvertor::lastId = 0;
