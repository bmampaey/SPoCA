#pragma once
#ifndef TrackingRelation_H
#define TrackingRelation_H

class TrackingRelation
{
	public:
		unsigned long past_color;
		std::string type;
		unsigned long present_color;

		TrackingRelation(const unsigned long& past_color, const std::string& type, const unsigned long& present_color)
		:past_color(past_color), type(type), present_color(present_color){}
		
		bool operator<(const TrackingRelation& r) const
		{
			if(past_color < r.past_color)
				return true;
			if(present_color < r.present_color)
				return true;
			if(type < r.type)
				return true;
			return false;
		}
		bool operator==(const TrackingRelation& r) const
		{
			if(past_color != r.past_color)
				return false;
			if(present_color != r.present_color)
				return false;
			if(type != r.type)
				return false;
			return true;
		}
};

#endif
