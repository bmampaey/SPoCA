#include "SPoCAClassifier.h"

using namespace std;

SPoCAClassifier::SPoCAClassifier(unsigned neighborhoodRadius, Real fuzzifier)
:PCMClassifier(fuzzifier), Nradius(neighborhoodRadius)
{}

void SPoCAClassifier::addImages(vector<EUVImage*> images)
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
	Xaxes = images[0]->Xaxes();
	Yaxes = images[0]->Yaxes();
	for (unsigned p = 1; p <  NUMBERCHANNELS; ++p)
	{
		Xaxes = images[p]->Xaxes() < Xaxes ? images[p]->Xaxes() : Xaxes;
		Yaxes = images[p]->Yaxes() < Yaxes ? images[p]->Yaxes() : Yaxes;
	}

	X.reserve(images[0]->NumberPixels());
	coordinates.reserve(images[0]->NumberPixels());
	smoothedX.reserve(images[0]->NumberPixels());

	//Temporary vectors to build the neihboorhood
	vector<unsigned> caardNeighbors(Xaxes * Yaxes, 0);
	NeighborhoodVector neighbors(Xaxes * Yaxes);

	bool validPixel;
	RealFeature f;
	//We initialise the valid pixels vector X, and the neighbors N
	for (int y = 0; y < int(Yaxes); ++y)
	{
		for (int x = 0; x < int(Xaxes); ++x)
		{
			validPixel = true;
			for (unsigned p = 0; p <  NUMBERCHANNELS && validPixel; ++p)
			{
				f.v[p] = images[p]->pixel(x,y);
				if(f.v[p] == images[p]->null())
					validPixel=false;
			}
			
			if(! validPixel)
				continue;

			//Since I am the neigboor of my neigboors, I can add myself to their neigboorhood (my index == numberFeatureVectors)
			for (int ny = y - int(Nradius); ny <= y + int(Nradius); ++ny)
			{
				NeighborIndex neighbor = (ny * Xaxes + x) - Nradius;
				for (int nx = x - int(Nradius); nx <= x + int(Nradius); ++nx)
				{
					if(!(nx == x && ny == y) && neighbor > 0 && neighbor < images[0]->NumberPixels())
					{
						neighbors[neighbor].push_back(numberFeatureVectors);
						++caardNeighbors[neighbor];
					}
					++neighbor;
				}
			}
			
			X.push_back(f);
			smoothedX.push_back(f);
			coordinates.push_back(PixLoc(x,y));
			++numberFeatureVectors;
			
		}
	}


	//Calculation of beta, smoothedX (the picture of the mean intensities) and of N the neighbors
	beta.reserve(numberFeatureVectors);

	for (unsigned j = 0; j < numberFeatureVectors; ++j)
	{
		unsigned coord = coordinates[j].x + coordinates[j].y * Xaxes;
		beta.push_back(caardNeighbors[coord] != 0 ? (1. / caardNeighbors[coord]) : 0);
		N.push_back(neighbors[coord]);
		
		if( N[j].size() == 0)
			continue;

		RealFeature sum = 0;
		for (Neighborhood::iterator k = N[j].begin(); k!=N[j].end(); ++k)
			sum +=  X[*k];

		smoothedX[j] += sum * beta[j];

	}
	
	// We write the fits file of smoothedX for verification
	#if DEBUG >= 2

	Image<EUVPixelType> image(Xaxes,Yaxes);
	for (unsigned p = 0; p <  NUMBERCHANNELS; ++p)
	{
		image.zero();
		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
			image.pixel(coordinates[j]) = smoothedX[j].v[p];

		image.writeFits(filenamePrefix + "smoothed." + images[p]->Channel() + ".fits");

	}
	#endif
	#if DEBUG >= 2
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

void SPoCAClassifier::computeB()
{
	B.assign(numberClasses, 0.);
	vector<Real> sum(numberClasses, 0.);
	
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator sxj = smoothedX.begin(); sxj != smoothedX.end(); ++sxj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = *uij * *uij;
				B[i] += *sxj * uij_m;
				sum[i] += uij_m;
			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator sxj = smoothedX.begin(); sxj != smoothedX.end(); ++sxj)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				Real uij_m = pow(*uij,fuzzifier);
				B[i] += *sxj * uij_m;
				sum[i] += uij_m;
			}
		}
	}
	
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		B[i] /= 2 * sum[i];
}



void SPoCAClassifier::computeU()
{
	U.assign(numberFeatureVectors * numberClasses, 0.);

	// For each feature vector, we compute the distance to each Bi
	// And we had the distance to all the neighbors of j (including myself) multiplied by beta[N[j]]
	MembershipSet::iterator uij = U.begin();
	NeighborhoodVector::iterator Nj = N.begin();
	for (FeatureVectorSet::iterator xj = X.begin(); xj != X.end(); ++xj, ++Nj)
	{
		for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
		{
			Real d2BiXj = distance_squared(*xj,B[i]);
			*uij += d2BiXj;
			for(Neighborhood::iterator k = Nj->begin(); k!=Nj->end(); ++k)
			{
				U[(*k)*numberClasses+i] += beta[*k] * d2BiXj;
			}
		}
	}
	// Now I fuzzify and inverse uij
	if(fuzzifier == 2)
	{
		for (MembershipSet::iterator uij = U.begin(); uij != U.end();)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij /= eta[i];
				*uij = 1. / (1. + *uij);
			}
		}
	}
	else if(fuzzifier == 1.5)
	{
		for (MembershipSet::iterator uij = U.begin(); uij != U.end();)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij /= eta[i];
				*uij *= *uij;
				*uij = 1. / (1. + *uij);
			}
		}
	}
	else
	{
		for (MembershipSet::iterator uij = U.begin(); uij != U.end();)
		{
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				*uij /= eta[i];
				*uij = pow( *uij, Real(1./(fuzzifier-1.)));
				*uij = 1. / (1. + *uij);
			}
		}
	}

}


Real SPoCAClassifier::computeJ() const
{
	Real result = 0, sumNeighbors, sum1, sum2;
	vector<Real> d2BiX(numberFeatureVectors);

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		sum1 = 0;
		sum2 = 0;

		//We precalculate all the distances from each pixel Xj to the center Bi
		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
		{
			d2BiX[j] = distance_squared(X[j],B[i]);
		}

		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
		{
			sumNeighbors = 0;
			
			for (Neighborhood::const_iterator k = N[j].begin(); k!=N[j].end(); ++k)
				sumNeighbors +=  d2BiX[(*k)];

			sumNeighbors = (sumNeighbors * beta[j]) + d2BiX[j];

			if(fuzzifier == 2)
				sum1 +=  U[j*numberClasses+i] * U[j*numberClasses+i] * sumNeighbors;
			else
				sum1 +=  pow(U[j*numberClasses+i], fuzzifier) * sumNeighbors;

			if(fuzzifier == 2)
				sum2 += (1 - U[j*numberClasses+i]) * (1 - U[j*numberClasses+i]);
			else
				sum2 +=  pow(Real(1. - U[j*numberClasses+i]), fuzzifier);

		}
		result += sum1 + (eta[i] * sum2);
	}
	return result;

}


Real SPoCAClassifier::assess(vector<Real>& V)
{
	V.assign(numberClasses, 0.);
	Real score = 0;

	//This is the vector of the min distances between the centers Bi and all the others centers Bii with ii!=i
	vector<Real> minDist(numberClasses, numeric_limits<Real>::max());
	//The min distance between all centers
	Real minDistBiBii = numeric_limits<Real>::max() ;

	Real distBiBii;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
		for (unsigned ii = i + 1 ; ii < numberClasses ; ++ii)
		{
			distBiBii = distance_squared(B[i],B[ii]);
			if(distBiBii < minDist[i])
				minDist[i] = distBiBii;
			if(distBiBii < minDist[ii])
				minDist[ii] = distBiBii;
		}

	vector<Real> d2BiX(numberFeatureVectors);
	Real sumNeighbors, sum2;
	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{

		//We precalculate all the distances from each pixel Xj to the center Bi
		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
		{
			d2BiX[j] = distance_squared(X[j],B[i]);
		}

		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
		{
			sumNeighbors = 0;
			for (Neighborhood::iterator k = N[j].begin(); k!=N[j].end(); ++k)
				sumNeighbors +=  d2BiX[(*k)];

			sumNeighbors = (sumNeighbors * beta[j]) + d2BiX[j];

			if(fuzzifier == 2)
				V[i] += U[j*numberClasses+i] * U[j*numberClasses+i] * sumNeighbors;
			else
				V[i] += pow(U[j*numberClasses+i],fuzzifier) * sumNeighbors;

		}
		sum2 = 0;
		for (unsigned j = 0 ; j < numberFeatureVectors ; ++j)
		{
			if(fuzzifier == 2)
				sum2 += (1. - U[j*numberClasses+i]) * (1. - U[j*numberClasses+i]);
			else
				sum2 += pow(Real(1. - U[j*numberClasses+i]),fuzzifier);

		}
		V[i] += eta[i] * sum2;
		score += V[i];
		if(minDist[i] < minDistBiBii)
			minDistBiBii = minDist[i];

		V[i] /= (minDist[i] * numberFeatureVectors);

	}

	score /= (minDistBiBii * numberFeatureVectors);
	return score;

}


#if MERGE_TYPE==MERGEMAX
/*!
Compute the new center by computing the mean value of the featurevector belonging to one of the 2 centers to be merged, weighted by the max value of the membership of the 2 centers.
A featurevector belong to a class if it's memebership is maximal for that class. 
The values of the membership are computed using the regular method for computing memebership with the new centers.
*/

void SPoCAClassifier::merge(unsigned i1, unsigned i2)
{

	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator sxj = smoothedX.begin(); sxj != smoothedX.end(); ++sxj)
		{
			// We search to which class belongs the featureVector 
			Real max_uij = 0;
			unsigned max_i = 0;
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				if (*uij > max_uij)
				{
					max_uij = *uij;
					max_i = i;
				}
			}
			// If it belongs to one of the 2 class I am merging, I update it's B
			if(max_i == i1 || max_i == i2)
			{
				Real uij_m = max_uij * max_uij;
				newB += *sxj * uij_m;
				sum += uij_m;

			}
		}
	}
	else
	{
		for (FeatureVectorSet::iterator sxj = smoothedX.begin(); sxj != smoothedX.end(); ++sxj)
		{
			// We search to which class belongs the featureVector 
			Real max_uij = 0;
			unsigned max_i = 0;
			for (unsigned i = 0 ; i < numberClasses ; ++i, ++uij)
			{
				if (*uij > max_uij)
				{
					max_uij = *uij;
					max_i = i;
				}
			}
			// If it belongs to one of the 2 class I am merging, I update it's B
			if(max_i == i1 || max_i == i2)
			{
				Real uij_m = pow(max_uij,fuzzifier);
				newB += *sxj * uij_m;
				sum += uij_m;

			}
		}
	}

	

	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;
	B[i1] = newB;
	computeU();


}

#elif MERGE_TYPE==MERGECIS
/*!
The values of the membership are computed by taking the maximal membership value for the 2 classes to be merged.
The new center is computed using the regular method for computing centers with the new membership.
*/
void SPoCAClassifier::merge(unsigned i1, unsigned i2)
{
	Real sum = 0;
	RealFeature newB = 0;
	MembershipSet::iterator uij = U.begin();
	// If the fuzzifier is 2 we can optimise by avoiding the call to the pow function
	if (fuzzifier == 2)
	{
		for (FeatureVectorSet::iterator sxj = smoothedX.begin(); sxj != smoothedX.end(); ++sxj)
		{
			// We search to which class belongs the featureVector 
			if(*(uij+i1) < *(uij+i2))
				*(uij+i1) = *(uij+i2);
			
			Real uij_m = *(uij+i1) **(uij+i1);
			newB += *sxj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	else
	{
		for (FeatureVectorSet::iterator sxj = smoothedX.begin(); sxj != smoothedX.end(); ++sxj)
		{
			// We search to which class belongs the featureVector 
			if(*(uij+i1) < *(uij+i2))
				*(uij+i1) = *(uij+i2);
			
			Real uij_m = pow(*(uij+i1),fuzzifier);
			newB += *sxj * uij_m;
			sum += uij_m;
			
			uij = U.erase(uij + i2) + numberClasses - (i2 + 1);
		}
	}
	
	newB /= sum;

	#if DEBUG >= 3
	cout<<"Merging centers :"<<B[i1]<<"\t"<<B[i2]<<" into new center :"<<newB<<endl;
	#endif
	
	B[i1] = newB;
	B.erase(B.begin()+i2);
	--numberClasses;
	

}
#endif
