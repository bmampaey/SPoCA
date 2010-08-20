#include "CoordinateConvertor.h"

using namespace std;
	
inline void IDL_do(const string command, bool reset = true)
{
	#if DEBUG >= 1
		cout<<";IDL command: \n"<<command<<endl;
	#endif
	if(IDL_ExecuteStr(const_cast<char *>(command.c_str())))
	{
		cerr<<"ERROR executing command: "<<command<<endl
		<<"\n\terror_state.msg: "<<IDL_STRING_STR(IDL_SysvErrStringFunc())
		<<"\n\terror_state.sys_msg: "<<IDL_STRING_STR(IDL_SysvSyserrStringFunc())
		<<"\n\terror_state: "<<IDL_SysvErrorCodeValue()<<endl;
		if(reset)
		{
			cerr<<"Will try to cleanup."<<endl;
			string cleanup = "MESSAGE, /RESET";
			IDL_do(cleanup, false);
		}
		
	}
	#if DEBUG >= 1
	else
	{
		cout<<";SUCCESS"<<endl;
	}
	#endif

}	


CoordinateConvertor::CoordinateConvertor(SunImage* image, string coordinateType)
:coordinateType(coordinateType)
{
	if(instances == 0)
	{
		if (!IDL_Init(IDL_INIT_BACKGROUND | IDL_INIT_QUIET, NULL, NULL))
		{
			cerr<<"ERROR initializing IDL"
			<<"\n\terror_state.msg: "<<IDL_STRING_STR(IDL_SysvErrStringFunc())
			<<"\n\terror_state.sys_msg: "<<IDL_STRING_STR(IDL_SysvSyserrStringFunc())
			<<"\n\terror_state: "<<IDL_SysvErrorCodeValue()<<endl;
		}
		#if DEBUG >= 1
			cout<<"IDL initialized correctly"<<endl;
		#endif
		string set_ssw_sysv = "defsysv,'!SSW',1b,1";
		IDL_do(set_ssw_sysv);
		string load_wcs_routines = "RESTORE, '"WCS_ROUTINES_SAV"'";
		IDL_do(load_wcs_routines);
		//setenv("ANCIL_DATA", SSW_PATH,1);
		setenv("DLM_PATH", "<IDL_DEFAULT>:"SSW_PATHSSW_PATH"/stereo/gen/exe/icy/linux_x86/lib/",1);
	}
	
	string create_header = "header = STRARR("+itos(3 + image->header.size())+")";
	IDL_do(create_header);
	unsigned h;
	for (h = 0; h < image->header.size(); ++h)
	{
		string add_key = "header["+ itos(h) +"] = \""+ image->header[h] + "\"";
		IDL_do(add_key);
	}
	// We need the naxis in the header
	string add_key = "header["+ itos(h) +"] = \"NAXIS   =                    2 /\"";
	IDL_do(add_key);
	++h;
	add_key = "header["+ itos(h) +"] = \"NAXIS1  =                 "+ itos(image->Xaxes())  + " / Number of columns\"";
	IDL_do(add_key);
	++h;
	add_key = "header["+ itos(h) +"] = \"NAXIS2  =                 "+ itos(image->Yaxes())  + " / Number of rows\"";
	IDL_do(add_key);
	
	#if DEBUG >= 3
		string help_header = "help, header, /struct";
		IDL_do(help_header);
	#endif

	wcs = "wcs" + itos(lastId);
	string create_wcs = "get_wcs, header=header, wcs=" + wcs;
	IDL_do(create_wcs);
	
	#if DEBUG >= 3
		string help_wcs = "help, " + wcs + ", /struct";
		IDL_do(help_wcs);
	#endif
	
	/*
	// To test IDL, and enter commands at the prompt
	cout<<"Enter command at the prompt (no space allowed). Type ctrl+d when you are finished"<<endl; 
	string cmd;
	cout<<"IDL > ";
	while(cin>>cmd)
	{
		IDL_do(cmd);
		cout<<"IDL > ";
	}
	*/

	++lastId;
	++instances;
} 

CoordinateConvertor::CoordinateConvertor(SunImage* image)
{

	if(instances == 0)
	{
		if (!IDL_Init(IDL_INIT_BACKGROUND | IDL_INIT_QUIET, NULL, NULL))
		{
			cerr<<"ERROR initializing IDL"
			<<"\n\terror_state.msg: "<<IDL_STRING_STR(IDL_SysvErrStringFunc())
			<<"\n\terror_state.sys_msg: "<<IDL_STRING_STR(IDL_SysvSyserrStringFunc())
			<<"\n\terror_state: "<<IDL_SysvErrorCodeValue()<<endl;
		}
		#if DEBUG >= 1
			cout<<"IDL initialized correctly"<<endl;
		#endif
		string set_ssw_path = "!PATH = !PATH + ':' + EXPAND_PATH('+" SSW_PATH "/gen/idl') + ':' + EXPAND_PATH('+" SSW_PATH "/stereo/') + ':' + EXPAND_PATH('+" SSW_PATH "/soho/eit/idl') + ':' + EXPAND_PATH('+" SSW_PATH "/gen/idl_libs/astron')";
		IDL_do(set_ssw_path);
		string set_ssw_sysv = "defsysv,'!SSW',1b,1";
		IDL_do(set_ssw_sysv);
		string set_ssw_dlm_path = "!DLM_PATH = !DLM_PATH + ':' + '"SSW_PATH"/stereo/gen/exe/icy/linux_x86/lib/'";
		IDL_do(set_ssw_dlm_path);
	}
	wcs = "wcs" + itos(lastId);
	string create_header = "header = STRARR("+itos(3 + image->header.size())+")";
	IDL_do(create_header);
	unsigned h;
	for (h = 0; h < image->header.size(); ++h)
	{
		string add_key = "header["+ itos(h) +"] = \""+ image->header[h] + "\"";
		IDL_do(add_key);
	}
	// We need the naxis in the header
	string add_key = "header["+ itos(h) +"] = \"NAXIS   =                    2 /\"";
	IDL_do(add_key);
	++h;
	add_key = "header["+ itos(h) +"] = \"NAXIS1  =                 "+ itos(image->Xaxes())  + " / Number of columns\"";
	IDL_do(add_key);
	++h;
	add_key = "header["+ itos(h) +"] = \"NAXIS2  =                 "+ itos(image->Yaxes())  + " / Number of rows\"";
	IDL_do(add_key);
	
	#if DEBUG >= 3
		string help_header = "help, header, /struct";
		IDL_do(help_header);
	#endif
	
	string create_wcs = wcs + " = fitshead2wcs(header)";
	IDL_do(create_wcs);
	
	#if DEBUG >= 3
		string help_wcs = "help, " + wcs + ", /struct";
		IDL_do(help_wcs);
	#endif
	/*
	// To test IDL, and enter commands at the prompt
	cout<<"Enter command at the prompt (no space allowed). Type ctrl+d when you are finished"<<endl; 
	string cmd;
	cout<<"IDL > ";
	while(cin>>cmd)
	{
		IDL_do(cmd, false);
		cout<<"IDL > ";
	}
	*/

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
	/*
	string carrington = coord_type == "HGC" ? "1" : "0";
	coord_type = coord_type == "HGC" ? "HG" : coord_type;
	*/
	string declare_cartesian = "cartesian = [" + itos(c.x) + "," + itos(c.y) + "]";
	IDL_do(declare_cartesian);
	/*
	// We convert the cartesian pixel coodinates into WCS
	string convert2wcs = "wcs_coord = WCS_GET_COORD("+ wcs +", cartesian)";
	IDL_do(convert2wcs);
	
	// We convert the WCS coodinates into helioprojective cartesian
	string convert2coord = "WCS_CONVERT_FROM_COORD, "+ wcs +", wcs_coord, '" + coord_type + "', ARCSECONDS = " + arcsecond  + ", CARRINGTON = " + carrington + ", x, y";
	IDL_do(convert2coord);
	*/
	// We convert the WCS coodinates into helioprojective cartesian
	string convert2coord = "convert, wcs="+ wcs +", coord_type = '" + coord_type + "', cartesian = cartesian, ARCSECONDS = " + arcsecond + ", x, y";
	IDL_do(convert2coord);
	
	IDL_VPTR v;
	v = IDL_FindNamedVariable((char *)"x", IDL_FALSE);
	x = v->value.f ;
	v = IDL_FindNamedVariable((char *)"y", IDL_FALSE);
	y = v->value.f ;
	#if DEBUG >= 3
		cout<<coord_type<<" = ["<<x<<","<<y<<"]"<<endl; 
	#endif
}

unsigned CoordinateConvertor::instances = 0;
unsigned CoordinateConvertor::lastId = 0;
