#pragma once
#ifndef TrackingRelation_H
#define TrackingRelation_H

class TrackingRelation
{
	public:
		ColorType past_color;
		std::string type;
		ColorType present_color;

		TrackingRelation(const ColorType& past_color, const std::string& type, const ColorType& present_color)
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
