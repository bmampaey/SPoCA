#include "CumulativeSPoCAClassifier.h"

using namespace std;

CumulativeSPoCAClassifier::CumulativeSPoCAClassifier(unsigned neighborhoodRadius, Real fuzzifier)
:SPoCAClassifier(), CumulativeClassifier()
{
	this->fuzzifier = fuzzifier;
	this->Nradius = neighborhoodRadius;
}

void CumulativeSPoCAClassifier::addImages(vector<EUVImage*> images)
{

	if(images.size() < NUMBERCHANNELS)
	{
		cerr<<"Error : The number of images is not equal to "<<NUMBERCHANNELS<<endl;
		exit(EXIT_FAILURE);
	}
	else if(images.size() > NUMBERCHANNELS)
	{
		cerr<<"Warning : The number of images is not equal to "<<NUMBERCHANNELS<<". Only using the first ones."<<endl;
	}
	
	ordonateImages(images);
	unsigned numberValidPixelsEstimate = images[0]->numberValidPixelsEstimate();
	unsigned xaxes = images[0]->Xaxes();
	unsigned yaxes = images[0]->Yaxes();
	for (unsigned p = 1; p <  NUMBERCHANNELS; ++p)
	{
		xaxes = images[p]->Xaxes() < xaxes ? images[p]->Xaxes() : xaxes;
		yaxes = images[p]->Yaxes() < yaxes ? images[p]->Yaxes() : yaxes;
		numberValidPixelsEstimate = images[p]->numberValidPixelsEstimate() > numberValidPixelsEstimate ? : numberValidPixelsEstimate;
	}
	if(Xaxes == 0)
	{
		Xaxes = xaxes;
	}
	#if DEBUG >= 1
	if(xaxes != Xaxes)
	{
		cerr<<"Error : All the images must have the same Xaxes size."<<endl;
		exit(EXIT_FAILURE);
	}
	#endif

	X.reserve(numberFeatureVectors + numberValidPixelsEstimate);
	smoothedX.reserve(numberFeatureVectors + numberValidPixelsEstimate);
	coordinates.reserve(numberFeatureVectors + numberValidPixelsEstimate);

	//Temporary vectors to build the neihboorhood
	vector<unsigned> caardNeighbors(xaxes * yaxes, 0);
	NeighborhoodVector neighbors(xaxes * yaxes);

	bool validPixel;
	RealFeature f;
	//We initialise the valid pixels vector X, and the neighbors N
	for (unsigned y = 0; y < yaxes; ++y)
	{
		for (unsigned x = 0; x < xaxes; ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERCHANNELS && validPixel; ++p)
			{
				f.v[p] = images[p]->pixel(x,y);
				if(f.v[p] == images[p]->nullvalue())
					validPixel=false;
			}
			
			if(! validPixel)
				continue;
			
			//In a normal situation, we can suppose that the sun disc isn't going to touch the borders of the picture. Otherwise just take a radiusRatio smaller.
			#if DEBUG >= 1
			#define ASSERTBOUNDARIES neighbor > 0 && neighbor < images[0]->NumberPixels()
			#else
			#define ASSERTBOUNDARIES true
			#endif

			//Since I am the neigboor of my neigboors, I can add myself to their neigboorhood
			for (unsigned ny = y - Nradius; ny <= y + Nradius; ++ny)
			{
				NeighborIndex neighbor = ny * Xaxes;
				for (unsigned nx = x - Nradius; nx <= x + Nradius; ++nx)
				{
					if(!(nx == x && ny == y) && ASSERTBOUNDARIES)
					{
						neighbors[neighbor].push_back(X.size());
						++caardNeighbors[neighbor];
					}
					++neighbor;
				}
			}
			X.push_back(x);
			smoothedX.push_back(x);
			coordinates.push_back(Coordinate(x,y + Yaxes));
			
		}
	}

	//Calculation of beta, Initialisation of smoothedX (the picture of the mean intensities) and of N the neighbors
	beta.reserve(X.size());
	RealFeature sum;
	for (unsigned j = numberFeatureVectors; j < X.size(); ++j)
	{
		unsigned coord = coordinates[j].x + coordinates[j].y * Xaxes;
		beta.push_back(caardNeighbors[coord] != 0 ? (1. / caardNeighbors[coord]) : 0);
		N.push_back(neighbors[coord]);
		
		if( N[j].size() == 0)
			continue;

		sum = 0;
		for (Neighborhood::iterator k = N[j].begin(); k!=N[j].end(); ++k)
			sum +=  X[*k];

		smoothedX[j] += sum * beta[j];

	}

	numberFeatureVectors = X.size();
	Yaxes += yaxes;
	++numberImages;

	// We write the fits file of smoothedX for verification
	#if DEBUG >= 2
	Image<EUVPixelType> image(Xaxes,Yaxes);
	for (unsigned p = 0; p <  NUMBERCHANNELS; ++p)
	{
		image.zero();

		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
			image.pixel(coordinates[j]) = smoothedX[j].v[p];

		image.writeFits(filenamePrefix + "smoothed." + itos(int(images[p]->Wavelength())) + ".fits");

	}
	#endif
	
	#if DEBUG >= 3
	#include <fstream>
	ofstream betaFile((filenamePrefix + "betaN.txt").c_str());
	for (unsigned j = 0; j < numberFeatureVectors && betaFile.good(); ++j)
	{
		betaFile<<coordinates[j]<<"\t"<<beta[j];
		for (unsigned k = 0; k < N[j].size(); ++k)
			betaFile<<"\t"<<coordinates[N[j][k]];
		betaFile<<endl;
	}
	betaFile.close();
	#endif

}
