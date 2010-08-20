#include "FCMClassifier.h"

using namespace std;

FCMClassifier::FCMClassifier(Real fuzzifier)
:Classifier(),fuzzifier(fuzzifier)
{
	#if DEBUG >= 1
	if (fuzzifier == 1)
	{
		cerr<<"Error : Fuzzifier must not equal 1.";
		exit(EXIT_FAILURE);
	}
	#endif
	

}


void FCMClassifier::computeB()
{

	Real sum, uij_m;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		B[i] = 0.;
		sum = 0;
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			if (fuzzifier == 2)
				uij_m = U[i*numberValidPixels+j] * U[i*numberValidPixels+j];
			else
				uij_m = pow(U[i*numberValidPixels+j],fuzzifier);

			B[i] += X[j] * uij_m;
			sum += uij_m;

		}

		B[i] /= sum;

	}
}


void FCMClassifier::computeU()
{

	Real sum;
	vector<Real> d2XjB(numberClasses);
	unsigned i;
	U.resize(numberValidPixels * numberClasses);
	
	for (unsigned j = 0 ; j < numberValidPixels ; ++j)
	{
		for (i = 0 ; i < numberClasses ; ++i)
		{
			d2XjB[i] = d2(X[j],B[i]);
			if (d2XjB[i] < precision)
				break;
		}
		if(i < numberClasses)					  // The pixel is very close to B[i]
		{
			for (unsigned ii = 0 ; ii < numberClasses ; ++ii)
			{
				U[ii*numberValidPixels+j] = 0.;
			}
			U[i*numberValidPixels+j] = 1.;
		}
		else
		{
			for (i = 0 ; i < numberClasses ; ++i)
			{
				sum = 0;
				for (unsigned ii = 0 ; ii < numberClasses ; ++ii)
				{
					if (fuzzifier == 2)
						sum += (d2XjB[i]/d2XjB[ii]);
					else
						sum += pow(d2XjB[i]/d2XjB[ii],1./(fuzzifier-1.));

				}
				U[i*numberValidPixels+j] = 1./sum;
			}

		}

	}

}


Real FCMClassifier::computeJ() const
{
	Real result = 0;

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{

		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{

			if (fuzzifier == 2)
				result +=  U[i*numberValidPixels+j] * U[i*numberValidPixels+j] * d2(X[j],B[i]);
			else
				result +=  pow(U[i*numberValidPixels+j], fuzzifier) * d2(X[j],B[i]);

		}
	}
	return result;

}


void FCMClassifier::classification(Real precision, unsigned maxNumberIteration)
{

	#if DEBUG >= 1
	if(X.size() == 0 || B.size() == 0)
	{
		cerr<<"Error : The Classifier must be initialized before doing classification."<<endl;
		exit(EXIT_FAILURE);

	}
	int excepts = feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
	cout<<setiosflags(ios::fixed);
	#endif

	#if DEBUG >= 3
	cout<<"--FCMClassifier::classification--START--"<<endl;
	#endif
	
	#if DEBUG >= 2
		stepinit(outputFileName+"iterations.txt");
		unsigned decimals = 1 - log10(precision);
	#endif
	
	//Initialisation of precision & U

	this->precision = precision;

	Real precisionReached = numeric_limits<Real>::max();
	vector<RealFeature> oldB = B;
	for (unsigned iteration = 0; iteration < maxNumberIteration && precisionReached > precision ; ++iteration)
	{
		FCMClassifier::computeU();
		FCMClassifier::computeB();

		for (unsigned i = 0 ; i < numberClasses ; ++i)
		{
			precisionReached = d2(oldB[i],B[i]);
			if (precisionReached > precision)
				break;

		}
		oldB = B;

		#if DEBUG >= 2
			stepout(iteration, precisionReached, decimals);
		#endif

	}

	#if DEBUG >= 3
	cout<<"--FCMClassifier::classification--END--"<<endl;
	#endif
	
	#if DEBUG >= 1
	feenableexcept(excepts);
	#endif

}


Real FCMClassifier::assess(vector<Real>& V)
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

	for (unsigned i = 0 ; i < numberClasses ; ++i)
	{
		for (unsigned j = 0 ; j < numberValidPixels ; ++j)
		{
			if (fuzzifier == 2)
				V[i] += d2(X[j],B[i]) * U[i*numberValidPixels+j] * U[i*numberValidPixels+j];
			else
				V[i] += d2(X[j],B[i]) * pow(U[i*numberValidPixels+j],fuzzifier);

		}

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

void FCMClassifier::merge(unsigned i1, unsigned i2)
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
			if (fuzzifier == 2)
				uij_m = max_uij * max_uij;
			else
				uij_m = pow(max_uij,fuzzifier);

			B[i1] += X[j] * uij_m;
			sum += uij_m;

		}

	}

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
void FCMClassifier::merge(unsigned i1, unsigned i2)
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

		if (fuzzifier == 2)
			uij_m = U[i1*numberValidPixels+j] * U[i1*numberValidPixels+j];
		else
			uij_m = pow(U[i1*numberValidPixels+j],fuzzifier);

		B[i1] += X[j] * uij_m;
		sum += uij_m;

	}

	B[i1] /= sum;

	#if DEBUG >= 3
	cout<<" into new center :"<<B[i1]<<endl;
	#endif

	B.erase(B.begin()+i2);
	--numberClasses;
	U.erase(U.begin() + i2 * numberValidPixels, U.begin() + (i2 + 1)  * numberValidPixels);
}
#endif
