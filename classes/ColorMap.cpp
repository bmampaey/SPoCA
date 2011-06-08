#include "ColorMap.h"

using namespace std;

ColorMap::~ColorMap()
{}

ColorMap::ColorMap(const long xAxes, const long yAxes)
:SunImage<ColorType>(xAxes, yAxes)
{
	nullvalue_ = 0;

}


ColorMap::ColorMap(const SunImage<ColorType>& i)
:SunImage<ColorType>(i)
{
	nullvalue_ = 0;

}


ColorMap::ColorMap(const SunImage<ColorType>* i)
:SunImage<ColorType>(i)
{
	nullvalue_ = 0;

}


ColorMap::ColorMap(const Header& header)
:SunImage<ColorType>(header)
{
	postRead();
}

void ColorMap::postRead()
{
	suncenter.x = header.get<int>("CRPIX1") - 1;
	suncenter.y = header.get<int>("CRPIX2") - 1;
	cdelt1 = header.get<double>("CDELT1");
	cdelt2 = header.get<double>("CDELT2");
	
	// We read the radius
	radius = header.get<double>("RADIUS");
	b0 = (header.get<double>("SOLAR_B0")/180.)*PI;
	date_obs = header.get<string>("DATE-OBS");
	//Sometimes the date is appended with a z
	if(date_obs.find_first_of("Zz") != string::npos)
		date_obs.erase(date_obs.find_first_of("Zz"));
	observationTime = iso2ctime(date_obs);
}

void ColorMap::preWrite()
{
	header.set<string>("INSTRUME", "SPoCA");
	header.set<double>("RADIUS", radius);
	header.set<int>("CRPIX1", suncenter.x + 1);
	header.set<int>("CRPIX2", suncenter.y + 1);
	header.set<double>("CDELT1", cdelt1);
	header.set<double>("CDELT2",cdelt2);
	header.set<string>("DATE-OBS", date_obs);
	header.set<double>("SOLAR_B0", (b0 * 180)/PI);
}


bool isColorMap(const Header& header)
{
	return header.get<bool>("INSTRUME") && header.get<string>("INSTRUME").find("SPoCA") != string::npos;	
}



void ColorMap::thresholdRegionsByRawArea(const double minSize)
{
	const double pixelarea = PixelArea();
	
	//First we compute the area for each color
	map<ColorType,double> areas;
	ColorType* p = pixels;
	
	for (unsigned y = 0; y < yAxes; ++y)
	{
		for (unsigned x = 0 ; x < xAxes; ++x)
		{
			if(*p != nullvalue_)
			{
				if (areas.count(*p) == 0)
					areas[*p] = pixelarea;
				else
					areas[*p] += pixelarea;
			}
			++p;
		}
	}
	
	//Now we nullify those that are too small
	p = pixels;
	
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(*p != nullvalue_ && areas[*p] < minSize)
		{
			*p = nullvalue_;
		}
		++p;
	}

}

void ColorMap::thresholdRegionsByRealArea(const double minSize)
{
	const double R0 = radius * PixelArea();
	const double R2 = radius * radius;
	
	//First we compute the area for each color
	map<ColorType,double> areas;

	ColorType* p = pixels;
	
	const int xmax = xAxes - suncenter.x;
	const int ymax = yAxes - suncenter.y;
	
	for (int y = - suncenter.y; y < ymax; ++y)
	{
		for (int x = - suncenter.x ; x < xmax; ++x)
		{
			if(*p != nullvalue_)
			{
				if (areas.count(*p) == 0)
					areas[*p] = 0;
				
				double pixelArea2 = R2 - (x * x) - (y * y);			
				if(pixelArea2 > 0)
					areas[*p] += R0 / sqrt(pixelArea2);
				else
					areas[*p] = numeric_limits<double>::infinity();
			}
			++p;
		}
	}
	
	//Now we nullify those that are too small
	p = pixels;
	
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(*p != nullvalue_ && areas[*p] < minSize)
		{
			*p = nullvalue_;
		}
		++p;
	}

}


ColorMap* ColorMap::dilateDiamond(unsigned size, ColorType pixelValueToDilate)
{

	unsigned *manthanDistance = new unsigned[xAxes * yAxes];
	unsigned maxDistance = xAxes + yAxes;

	for (unsigned y=0; y < yAxes; ++y)
	{
		for (unsigned x=0; x < xAxes; ++x)
		{
			if (pixel(x,y) == pixelValueToDilate)
			{

				manthanDistance[x+y*xAxes] = 0;
			}
			else
			{

				manthanDistance[x+y*xAxes] = maxDistance;

				if (x>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x-1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x-1+y*xAxes]+1);

				if (y>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y-1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y-1)*xAxes]+1);
			}
		}
	}

	for (unsigned y=yAxes; y >0; )
	{
		--y;
		for (unsigned x=xAxes; x >0; )
		{

			--x;
			if (x+1<xAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+1+y*xAxes]+1);

			if (y+1<yAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y+1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y+1)*xAxes]+1);

		}
	}

	for (unsigned y=0; y < yAxes; ++y)
		for (unsigned x=0; x < xAxes; ++x)
			if(manthanDistance[x+y*xAxes] <= size) pixel(x,y) = pixelValueToDilate;

	delete[] manthanDistance;
	return this;

}



ColorMap* ColorMap::erodeDiamond(unsigned size, ColorType pixelValueToErode)
{

	ColorType fillPixelValue = nullvalue_;
	unsigned *manthanDistance = new unsigned[xAxes * yAxes];
	unsigned maxDistance = xAxes + yAxes;

	for (unsigned y=0; y < yAxes; ++y)
	{
		for (unsigned x=0; x < xAxes; ++x)
		{
			if (pixel(x,y) != pixelValueToErode)
			{

				manthanDistance[x+y*xAxes] = 0;
			}
			else
			{

				manthanDistance[x+y*xAxes] = maxDistance;

				if (x>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x-1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x-1+y*xAxes]+1);

				if (y>0) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y-1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y-1)*xAxes]+1);
			}
		}
	}

	for (unsigned y=yAxes; y >0; )
	{
		--y;
		for (unsigned x=xAxes; x >0;)
		{
			--x;
			if (x+1<xAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+1+y*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+1+y*xAxes]+1);

			if (y+1<xAxes) manthanDistance[x+y*xAxes] = manthanDistance[x+y*xAxes] < (manthanDistance[x+(y+1)*xAxes]+1) ? manthanDistance[x+y*xAxes] : (manthanDistance[x+(y+1)*xAxes]+1);

		}
	}

	for (unsigned y=0; y < yAxes; ++y)
		for (unsigned x=0; x < xAxes; ++x)
			pixel(x,y) = manthanDistance[x+y*xAxes] <= size? fillPixelValue : pixelValueToErode;

	delete[] manthanDistance;
	return this;

}



ColorMap* ColorMap::dilateCircular(const unsigned size, const ColorType unsetValue)
{
	ColorType * newPixels = new ColorType[numberPixels];
	fill(newPixels, newPixels + numberPixels, unsetValue);
	vector<unsigned> shape;
	shape.reserve(size*size*3);
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -size; x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y *y) <= size)
				shape.push_back(y * xAxes + x);
	
				
	int j;
	unsigned y, x;
	for(y = size; y < yAxes - size; ++y)
	{		
		j = 	y * xAxes + size;
		for(x = size; x < xAxes - size; ++x)
		{
			
			if(pixels[j] != unsetValue)
			{
				newPixels[j] = pixels[j];
				if(pixels[j-1] == unsetValue || pixels[j+1] == unsetValue || pixels[j-xAxes] == unsetValue || pixels[j+xAxes] == unsetValue)
				{
					
					for(unsigned s = 0; s < shape.size(); ++s)
					{
						#if DEBUG >= 1
							if(j + shape[s] >= numberPixels || j - shape[s] < 0)
							{
								cerr<<"Error : trying to access pixel out of image in drawContours"<<endl;
								exit(EXIT_FAILURE);
							}	
						#endif
						newPixels[j + shape[s]] = newPixels[j - shape[s]] = pixels[j];
					}
				}

			}
			++j;
		}
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}



ColorMap* ColorMap::erodeCircular(const unsigned size, const ColorType unsetValue)
{
	ColorType * newPixels = new ColorType[numberPixels];
	memcpy(newPixels, pixels, numberPixels * sizeof(ColorType));
	vector<unsigned> shape;
	shape.reserve(size*size*3);
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -size; x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y *y) <= size)
				shape.push_back(y * xAxes + x);
	
				
	int j;
	for(unsigned y = size; y < yAxes - size; ++y)
	{		
		j = 	y * xAxes + size;
		for(unsigned x = size; x < xAxes - size; ++x)
		{
			
			if(pixels[j] != unsetValue && (pixels[j-1] != pixels[j] || pixels[j+1] != pixels[j] || pixels[j-xAxes] != pixels[j] || pixels[j+xAxes] != pixels[j]))
			{
				newPixels[j] = unsetValue;
				for(unsigned s = 0; s < shape.size(); ++s)
				{
					#if DEBUG >= 1
						if(j + shape[s] >= numberPixels || j - shape[s] < 0)
						{
							cerr<<"Error : trying to access pixel out of image in drawContours"<<endl;
							exit(EXIT_FAILURE);
						}	
					#endif
					newPixels[j + shape[s]] = newPixels[j - shape[s]] = unsetValue;
				}
				
			}
			++j;
		}
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}


ColorMap* ColorMap::drawInternContours(const unsigned width, const ColorType unsetValue)
{

	ColorMap * eroded = new ColorMap (this);
	eroded->erodeCircular(width, unsetValue);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(eroded->pixels[j] != eroded->nullvalue_)
			pixels[j] = unsetValue;
	}
	delete eroded;
	return this;

}


ColorMap* ColorMap::drawExternContours(const unsigned width, const ColorType unsetValue)
{

	ColorMap * copy = new ColorMap (this);
	this->dilateCircular(width, unsetValue);
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == copy->pixels[j])
			pixels[j] = unsetValue;
	}
	delete copy;
	return this;

}


ColorMap* ColorMap::drawContours(const unsigned width, const ColorType unsetValue)
{
	unsigned size = width/2;
	ColorType * newPixels = new ColorType[numberPixels];
	memcpy(newPixels, pixels, numberPixels * sizeof(ColorType));
	vector<unsigned> shape;
	shape.reserve(size*size*3);
	for(unsigned x = 1; x <= size; ++x)
		shape.push_back(x);
	for(int x = -size; x <= int(size); ++x)
		for(unsigned y = 1; y <= size; ++y)
			if(sqrt(x * x + y *y) <= size)
				shape.push_back(y * xAxes + x);
	
				
	int j;
	for(unsigned y = size; y < yAxes - size; ++y)
	{		
		j = 	y * xAxes + size;
		for(unsigned x = size; x < xAxes - size; ++x)
		{
			ColorType maxColor = pixels[j-1];
			maxColor = pixels[j+1] > maxColor ? pixels[j+1] : maxColor;
			maxColor = pixels[j-xAxes] > maxColor ? pixels[j-xAxes] : maxColor;
			maxColor = pixels[j+xAxes] > maxColor ? pixels[j+xAxes] : maxColor;
			if(pixels[j] != maxColor)
			{
				newPixels[j] = maxColor;
				for(unsigned s = 0; s < shape.size(); ++s)
				{
					#if DEBUG >= 1
						if(j + shape[s] >= numberPixels || j - shape[s] < 0)
						{
							cerr<<"Error : trying to access pixel out of image in drawContours"<<endl;
							exit(EXIT_FAILURE);
						}	
					#endif
					newPixels[j + shape[s]] = newPixels[j - shape[s]] = maxColor;
				}
				
			}
			else
			{
				newPixels[j] = unsetValue;
			}
			++j;
		}
	}
	
	delete[] pixels;
	pixels = newPixels;
	return this;
}


unsigned ColorMap::colorizeConnectedComponents(const ColorType setValue)
{
	ColorType color = setValue;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == setValue)
		{
			++color;
			propagateColor(color, j);

		}
	}

	return unsigned(color - setValue);

}



unsigned ColorMap::propagateColor(const ColorType color, const Coordinate& firstPixel)
{
	return propagateColor(color, firstPixel.x + firstPixel.y * xAxes);
}



unsigned ColorMap::propagateColor(const ColorType color, const unsigned firstPixel)
{
	vector<unsigned> pixelList;
	ColorType setValue = pixels[firstPixel];
	unsigned h;
	unsigned numberColoredPixels = 0;

	pixelList.push_back(firstPixel);
	while ( ! pixelList.empty())
	{
		h = pixelList.back();
		pixelList.pop_back();
		if(pixels[h] != setValue)
			continue;
		pixels[h] = color;
		++numberColoredPixels;
		if(h+1 < numberPixels && pixels[h+1] == setValue)
			pixelList.push_back(h+1);
		if(h+xAxes < numberPixels && pixels[h+xAxes] == setValue)
			pixelList.push_back(h+xAxes);
		if(h >= 1 && pixels[h-1] == setValue)
			pixelList.push_back(h-1);
		if(h >= xAxes && pixels[h-xAxes] == setValue)
			pixelList.push_back(h-xAxes);

	}
	return numberColoredPixels;
}



unsigned ColorMap::thresholdConnectedComponents(const unsigned minSize, const ColorType setValue)
{
	vector<unsigned> treatedPixels;
	ColorType color = setValue + 1;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] == setValue)
		{
			if (propagateColor(color, j) < minSize)
				propagateColor(nullvalue_, j);
			else
			{
				++color;
				treatedPixels.push_back(j);
			}
		}
	}
	//We have to give back the original color
	for (unsigned t = 0; t < treatedPixels.size(); ++t)
	{
		propagateColor(setValue, treatedPixels[t]);
	}

	return unsigned(color - 1 - setValue);
}


ColorMap* ColorMap::removeHoles(ColorType unusedColor)
{
	propagateColor(unusedColor, 0);
	ColorType lastColor = nullvalue_;
	for (unsigned j = 0; j < numberPixels; ++j)
	{
		if(pixels[j] != nullvalue_)
		{
			lastColor = pixels[j];
		}
		else
		{
			pixels[j] = lastColor;
		}
	}
	propagateColor(nullvalue_, 0);
	return this;
}


/*!
To extract the chain code of a connected component, we first list all the points along the boundary starting at the firstPixel.
Once The list is made, it is reduced by trying to find the most relevant points.
First we take the firstpixel point and it's furthest point in the list, and add them to the chain code.
Then we search the point that is the furthest from the line passing by each pair of consecutive point in the chain code, and add it to the chain code.
We repeat that last step until we have enough points  
*/

vector<Coordinate> ColorMap::chainCode(const Coordinate firstPixel, const unsigned max_points) const
{
	vector<Coordinate> chain;
	chain.reserve(1000);
	
	// We list all the directions
	int directions[] = {	0 + xAxes, //Norh
				1 + xAxes, //NE
				1 + 0, //East
				1 - xAxes, //SE
				0 - xAxes, //South
				-1 - xAxes, //SW
				-1 + 0, //West
				-1 + xAxes, //NW
				//We repeat for simplicity 
				0 + xAxes, 1 + xAxes, 1 + 0, 1 - xAxes, 0 - xAxes, -1 - xAxes, -1 + 0, -1 + xAxes
			};
	
	// We start at the first pixel, and we search for the first direction
	chain.push_back(firstPixel);
	unsigned cur_p = firstPixel.x + firstPixel.y * xAxes;
	int first_direction = 0;
	bool found = false;
	for (unsigned i = 0; i <= 8; ++i)
	{
		int next_p = cur_p + directions[first_direction];
		if(next_p >= 0 && next_p < int(numberPixels))
		{
			if(pixels[next_p] == nullvalue_)
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
	first_direction%=8;
	// If we are a single pixel, we return
	if(!found)
	{
		return chain;
	}

	// We create a classical chaincode of all pixels locations along the boundary
	// until we come back to the first pixel, with the same direction
	Coordinate current_pixel = firstPixel;
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
		Real distance = firstPixel.d2(current_pixel);
		if(distance > biggest_distance)
		{
			biggest_distance = distance;
			furthest_pixel_indice = chain.size() - 1;
		}
		// We search the direction of the following pixel on the border by looking at all directions
		// Starting at the opposite direction +1
		next_direction = (next_direction + 5) % 8;
		for (unsigned i = 0; i < 8; ++i)
		{
			int next_p = cur_p + directions[next_direction];
			if(next_p >= 0 && next_p < int(numberPixels) && pixels[next_p] != nullvalue_)
			{
				break;
			}
			++next_direction;
		}
		next_direction %= 8;
		
	}while (!((firstPixel == current_pixel) && (next_direction == first_direction)));


	if(chain.size() <= max_points)
	{
		return chain;
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
	while(good_indices.size() < max_points)
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
	vector<Coordinate> reduced_chain(good_indices.size());
	for(unsigned i=0; i < good_indices.size(); ++i)
	{
		reduced_chain[i]=chain[good_indices[i]];
	}
	return reduced_chain;
}

#ifdef MAGICK
MagickImage ColorMap::magick(const Magick::Color background)
{
	MagickImage image(background, xAxes, yAxes);
	for (unsigned y = 0; y < yAxes; ++y)
	{
		for (unsigned x = 0; x < xAxes; ++x)
		{	
			if(pixel(x, y) != nullvalue_)
			{
				image.pixelColor(x, yAxes - y - 1, Magick::Color(gradient[pixel(x, y) % gradientMax]));
			}
		}
	}
	return image;
}

MagickImage ColorMap::magick()
{
	return magick(Magick::Color(0, 0 ,0, MaxRGB));
}
#endif


