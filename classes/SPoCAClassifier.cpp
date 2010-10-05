#include "SPoCAClassifier.h"

using namespace std;

SPoCAClassifier::SPoCAClassifier(unsigned neighboorhoodRadius, Real fuzzifier)
:PCMClassifier(fuzzifier), Nradius(neighboorhoodRadius)
{}

void SPoCAClassifier::addImages(vector<SunImage*>& images)
{
	checkImages(images);
	ordonateImages(images);
	unsigned numberValidPixelsEstimate = images[0]->numberValidPixelsEstimate();
	Xaxes = images[0]->Xaxes();
	Yaxes = images[0]->Yaxes();
	for (unsigned p = 1; p <  NUMBERWAVELENGTH; ++p)
	{
		Xaxes = images[p]->Xaxes() < Xaxes ? images[p]->Xaxes() : Xaxes;
		Yaxes = images[p]->Yaxes() < Yaxes ? images[p]->Yaxes() : Yaxes;
		numberValidPixelsEstimate = images[p]->numberValidPixelsEstimate() > numberValidPixelsEstimate ? : numberValidPixelsEstimate;
	}

	X.reserve(numberValidPixelsEstimate);
	coordinates.reserve(numberValidPixelsEstimate);
	smoothedX.reserve(numberValidPixelsEstimate);

	//Temporary vectors to build the neihboorhood
	vector<unsigned short> caardNeighboors(Xaxes * Yaxes, 0);
	vector<vector<unsigned> > neighboors(Xaxes * Yaxes);

	bool validPixel;
	PixelFeature xj;
	//We initialise the valid pixels vector X, and the neighboors N
	for (unsigned y = 0; y < Yaxes; ++y)
	{
		for (unsigned x = 0; x < Xaxes; ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERWAVELENGTH && validPixel; ++p)
			{
				xj.v[p] = images[p]->pixel(x,y);
				if(xj.v[p] == images[p]->nullvalue())
					validPixel=false;
			}
			
			if(! validPixel)
				continue;
			
			//In a normal situation, we can suppose that the sun disc isn't going to touch the borders of the picture. Otherwise just take a radiusRatio smaller.
			#if DEBUG >= 1
			#define ASSERTBOUNDARIES neighboor > 0 && neighboor < images[0]->NumberPixels()
			#else
			#define ASSERTBOUNDARIES true
			#endif

			//Since I am the neigboor of my neigboors, I can add myself to their neigboorhood
			for (unsigned ny = y - Nradius; ny <= y + Nradius; ++ny)
			{
				unsigned neighboor = ny * Xaxes + x - Nradius;
				for (unsigned nx = x - Nradius; nx <= x + Nradius; ++nx)
				{
					if(nx != x && ny != y && ASSERTBOUNDARIES)
					{
						neighboors[neighboor].push_back(numberValidPixels);
						++caardNeighboors[neighboor];
					}
					++neighboor;
				}
			}
			X.push_back(xj);
			smoothedX.push_back(xj);
			coordinates.push_back(Coordinate(x,y));
			++numberValidPixels;
			
		}
	}


	//Calculation of beta, Initialisation of smoothedX (the picture of the mean intensities) and of N the neighboors
	beta.reserve(numberValidPixels);
	RealFeature sum;
	for (unsigned j = 0; j < numberValidPixels; ++j)
	{
		unsigned coord = coordinates[j].x + coordinates[j].y * Xaxes;
		beta.push_back(caardNeighboors[coord] != 0 ? (1. / caardNeighboors[coord]) : 0);
		N.push_back(neighboors[coord]);
		
		if( N[j].size() == 0)
			continue;

		sum = 0;
		for (vector<unsigned>::iterator k = N[j].begin(); k!=N[j].end(); ++k)
			sum +=  X[*k];

		smoothedX[j] += sum * beta[j];
		

	}
	
	// We write the fits file of smoothedX for verification
	#if DEBUG >= 2

	Image<PixelType> image(Xaxes,Yaxes);
	for (unsigned p = 0; p <  NUMBERWAVELENGTH; ++p)
	{
		image.zero();

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
			image.pixel(coordinates[j]) = smoothedX[j].v[p];

		image.writeFitsImage(outputFileName + "smoothed." + itos(int(images[p]->Wavelength())) + ".fits");

	}
	#endif
	#if DEBUG >= 3
	#include <fstream>
	ofstream betaFile((outputFileName + "betaN.txt").c_str());
	for (unsigned j = 0; j < numberValidPixels && betaFile.good(); ++j)
	{
		betaFile<<coordinates[j]<<"\t"<<beta[j];
		for (unsigned k = 0; k < N[j].size(); ++k)
			betaFile<<"\t"<<coordinates[N[j][k]];
		betaFile<<endl;
	}
	betaFile.close();
	#endif
}


void SPoCAClassifier::computeB()
{
	Real sum, uij_m;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		B[i] = 0;
		sum = 0;
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			if(fuzzifier == 2)
				uij_m = U[i*numberValidPixels+j] * U[i*numberValidPixels+j];
			else
				uij_m = pow(U[i*numberValidPixels+j],fuzzifier);

			B[i] += smoothedX[j] * uij_m;
			sum += uij_m;

		}

		sum *= 2;
		B[i] /= sum;

	}

}


void SPoCAClassifier::computeU()
{

	Real sumNeighboors;
	vector<Real> d2BiX(numberValidPixels);
	vector<unsigned>::iterator lastneighboor;
	U.resize(numberValidPixels * numberClasses);
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		//We precalculate all the distances from each pixel Xj to the center Bi
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			d2BiX[j] = d2(X[j],B[i]);
		}

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			sumNeighboors = 0;
			lastneighboor = N[j].end();
			for (vector<unsigned>::iterator k = N[j].begin(); k!=lastneighboor; ++k)
				sumNeighboors +=  d2BiX[(*k)];

			sumNeighboors = (sumNeighboors * beta[j]) + d2BiX[j];

			if(fuzzifier == 2)
				U[i*numberValidPixels+j] = sumNeighboors / eta[i] ;
			else if(fuzzifier == 1.5)
			{
				U[i*numberValidPixels+j] = sumNeighboors / eta[i] ;
				U[i*numberValidPixels+j] *= U[i*numberValidPixels+j];
			}
			else
				U[i*numberValidPixels+j] = pow( sumNeighboors / eta[i], 1./(fuzzifier-1.) );

			U[i*numberValidPixels+j] = 1. / (1. + U[i*numberValidPixels+j]);

		}

	}

}


Real SPoCAClassifier::computeJ() const
{
	Real result = 0, sumNeighboors, sum1, sum2;
	vector<Real> d2BiX(numberValidPixels);

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		sum1 = 0;
		sum2 = 0;

		//We precalculate all the distances from each pixel Xj to the center Bi
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			d2BiX[j] = d2(X[j],B[i]);
		}

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			sumNeighboors = 0;
			
			for (vector<unsigned>::const_iterator k = N[j].begin(); k!=N[j].end(); ++k)
				sumNeighboors +=  d2BiX[(*k)];

			sumNeighboors = (sumNeighboors * beta[j]) + d2BiX[j];

			if(fuzzifier == 2)
				sum1 +=  U[i*numberValidPixels+j] * U[i*numberValidPixels+j] * sumNeighboors;
			else
				sum1 +=  pow(U[i*numberValidPixels+j], fuzzifier) * sumNeighboors;

			if(fuzzifier == 2)
				sum2 += (1 - U[i*numberValidPixels+j]) * (1 - U[i*numberValidPixels+j]);
			else
				sum2 +=  pow(1 - U[i*numberValidPixels+j], fuzzifier);

		}
		result += sum1 + (eta[i] * sum2);
	}
	return result;

}


Real SPoCAClassifier::assess(vector<Real>& V)
{
	V = vector<Real>(numberClasses, 0.);
	Real score = 0;

	//This is the vector of the min distances between the centers Bi and all the others centers Bii with ii!=i
	vector<Real> minDist(numberClasses, numeric_limits<Real>::max());
	//The min distance between all centers
	Real minDistBiBii = numeric_limits<Real>::max() ;

	Real distBiBii;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		for (unsigned ii = i + 1 ; ii < numberClasses ; ++ii)
	{
		distBiBii = d2(B[i],B[ii]);
		if(distBiBii < minDist[i])
			minDist[i] = distBiBii;
		if(distBiBii < minDist[ii])
			minDist[ii] = distBiBii;
	}

	vector<Real> d2BiX(numberValidPixels);
	Real sumNeighboors, sum2;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{

		//We precalculate all the distances from each pixel Xj to the center Bi
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			d2BiX[j] = d2(X[j],B[i]);
		}

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			sumNeighboors = 0;
			for (vector<unsigned>::iterator k = N[j].begin(); k!=N[j].end(); ++k)
				sumNeighboors +=  d2BiX[(*k)];

			sumNeighboors = (sumNeighboors * beta[j]) + d2BiX[j];

			if(fuzzifier == 2)
				V[i] += U[i*numberValidPixels+j] * U[i*numberValidPixels+j] * sumNeighboors;
			else
				V[i] += pow(U[i*numberValidPixels+j],fuzzifier) * sumNeighboors;

		}
		sum2 = 0;
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			if(fuzzifier == 2)
				sum2 += (1. - U[i*numberValidPixels+j]) * (1. - U[i*numberValidPixels+j]);
			else
				sum2 += pow(1. - U[i*numberValidPixels+j],fuzzifier);

		}
		V[i] += eta[i] * sum2;
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberValidPixels);

	}

	score /= (minDistBiBii * numberValidPixels);
	return score;

}


#if MERGE==MERGEMAX
//We merge according to Benjamin's method
void SPoCAClassifier::merge(unsigned i1, unsigned i2)
{

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2];
	#endif

	Real max_uij, uij_m, sum = 0;
	unsigned max_i;
	B[i1] = 0;
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		max_uij = 0;
		max_i = 0;
		for (unsigned i = 0 ; i < numberClasses ; ++i)
			if (U[i*numberValidPixels+j] > max_uij)
		{
			max_uij = U[i*numberValidPixels+j];
			max_i = i;
		}
		if(max_i == i1 || max_i == i2)
		{
			if(fuzzifier == 2)
				uij_m = max_uij * max_uij;
			else
				uij_m = pow(max_uij,fuzzifier);

			B[i1] += smoothedX[j] * uij_m;
			sum += uij_m;

		}

	}

	sum *= 2;
	B[i1] /= sum;

	#if DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;

	computeU();

}


#elif MERGE==MERGECIS
//We merge according to Cis's method
void SPoCAClassifier::merge(unsigned i1, unsigned i2)
{

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2];
	#endif

	Real uij_m, sum = 0;
	B[i1] = 0;
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		if(U[i1*numberValidPixels+j] < U[i2*numberValidPixels+j])
			U[i1*numberValidPixels+j] = U[i2*numberValidPixels+j];

		if(fuzzifier == 2)
			uij_m = U[i1*numberValidPixels+j] * U[i1*numberValidPixels+j];
		else
			uij_m = pow(U[i1*numberValidPixels+j],fuzzifier);

		B[i1] += smoothedX[j] * uij_m;
		sum += uij_m;

	}

	B[i1] /= 2 * sum;

	#if DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;
	U.erase(U.begin() + i2 * numberValidPixels, U.begin() + (i2 + 1)  * numberValidPixels);
}
#endif
