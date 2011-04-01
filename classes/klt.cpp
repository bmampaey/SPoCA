#include "klt.h"

using namespace std;
void computeKernels(float sigma, vector<float> &gauss, vector<float> &gaussderiv, int max_kernel_width = 71)
{
	const float factor = 0.01f;					  /* for truncating tail */

	if (max_kernel_width % 2 != 1)
	{
		cerr<<"Error: computeKernels : max_kernel_width must be odd"<<endl;
		exit(EXIT_FAILURE);
	}
	if (sigma < 0.0)
	{
		cerr<<"Error: computeKernels : sigma must be positive"<<endl;
		exit(EXIT_FAILURE);
	}

	/* Compute kernels, and automatically determine widths */
	const int hw = max_kernel_width / 2;
	float max_gauss = 1.0f;
	float max_gaussderiv = sigma*exp(-0.5f);

	gauss.resize(max_kernel_width, 0);
	gaussderiv.resize(max_kernel_width, 0);

	/* Compute gauss and deriv */
	for (int i = -hw ; i <= hw ; i++)
	{
		gauss[i+hw]      = exp(float(-i*i) / (2*sigma*sigma));
		gaussderiv[i+hw] = -i * gauss[i+hw];
	}

	/* Compute widths */
	int gauss_size = max_kernel_width;
	for (int i = -hw ; fabs(gauss[i+hw] / max_gauss) < factor ; i++)
		gauss_size -= 2;

	int gaussderiv_size = max_kernel_width;

	for (int i = -hw ; fabs(gaussderiv[i+hw] / max_gaussderiv) < factor ; i++)
		gaussderiv_size -= 2;

	if (gauss_size == max_kernel_width || gaussderiv_size == max_kernel_width)
	{
		cerr<<"computeKernels: max_kernel_width "<< max_kernel_width<< " is too small for a sigma of "<<sigma<<endl;
		exit(EXIT_FAILURE);
	}

	/* Shift if width less than max_kernel_width */
	for (unsigned i = 0 ; i < gauss.size() ; i++)
		gauss[i] = gauss[i+(max_kernel_width-gauss_size)/2];

	gauss.resize(gauss_size);

	for (unsigned i = 0 ; i < gaussderiv.size() ; i++)
		gaussderiv[i] = gaussderiv[i+(max_kernel_width-gaussderiv_size)/2];

	gaussderiv.resize(gaussderiv_size);

	/* Normalize gauss and deriv */

	float den = 0.0;
	for (unsigned i = 0 ; i < gauss.size() ; i++)
		den += gauss[i];
	for (unsigned i = 0 ; i < gauss.size() ; i++)
		gauss[i] /= den;

	const int ww = gaussderiv.size() / 2;

	den = 0.0;
	for (int i = -ww ; i <= ww ; i++)
		den -= i*gaussderiv[i+ww];
	for (int i = -ww ; i <= ww ; i++)
		gaussderiv[i+ww] /= den;

}


Pyramid computePyramid(const SunImage* img, float smoothSigma, float sigma_fact, const int subsampling, const unsigned pyramidLevels)
{
	Pyramid pyramid;
	int subhalf = subsampling / 2;

	if (subsampling != 2 && subsampling != 4 && subsampling != 8 && subsampling != 16 && subsampling != 32)
	{
		cerr<<"Error: computePyramid : Pyramid's subsampling must be either 2, 4, 8, 16, or 32"<<endl;
		exit(EXIT_FAILURE);
	}
	
	// We compute the kernels for the smoothing
	vector<float> gauss_kernel, gaussderiv_kernel;
	computeKernels(smoothSigma, gauss_kernel, gaussderiv_kernel);
	
	// Smooth the first image, and copy to level 0 of pyramid
	SunImage* currimg = new SunImage();
	currimg->convolveSeparate(img, gauss_kernel, gauss_kernel);
	pyramid.push_back(currimg);
	
	if(smoothSigma != sigma_fact)
		computeKernels(sigma_fact, gauss_kernel, gaussderiv_kernel);


	// TODO We could try by taking an average of the pixels
	for (unsigned L = 1 ; L < pyramidLevels ; ++L)
	{
		SunImage* tmpimg = new SunImage();
		tmpimg->convolveSeparate(currimg, gauss_kernel, gauss_kernel);
		pyramid.push_back(new SunImage(currimg->Xaxes()/subsampling, currimg->Yaxes()/subsampling));
		for (unsigned y = 0 ; y <  pyramid[L]->Yaxes() ; y++)
			for (unsigned x = 0 ; x < pyramid[L]->Xaxes() ; x++)
				pyramid[L]->pixel(x,y) = tmpimg->pixel(subsampling*x+subhalf, subsampling*y+subhalf);
				
		delete tmpimg;
		/* Reassign current image */
		currimg = pyramid[L];
	}
	return pyramid;

}


void computeGradientPyramids(const Pyramid& pyramid, float sigma, Pyramid& pyramid_gradx, Pyramid& pyramid_grady)
{
	pyramid_gradx.clear();
	pyramid_grady.clear();
	
	vector<float> gauss_kernel, gaussderiv_kernel;
	computeKernels(sigma, gauss_kernel, gaussderiv_kernel);
	for (unsigned L = 0 ; L < pyramid.size() ; ++L)
	{
		pyramid_gradx.push_back(new SunImage);
		pyramid_gradx[L]->convolveSeparate(pyramid[L], gaussderiv_kernel, gauss_kernel);
		pyramid_grady.push_back(new SunImage);
		pyramid_grady[L]->convolveSeparate(pyramid[L], gauss_kernel, gaussderiv_kernel);
	}
}




SunImage* imageDifference(const SunImage* img1, const SunImage* img2, float x1, float y1, float x2, float y2, int width, int height)
{
	register int hw = width/2, hh = height/2;
	register int i, j;
	SunImage* imgdiff = new SunImage(width, height);
	PixelType *ptr = &(imgdiff->pixel(0));
	/* Compute values */
	for (j = -hh ; j <= hh ; j++)
		for (i = -hw ; i <= hw ; i++)
		{
			*ptr++ = img1->interpolate(x1+i, y1+j) - img2->interpolate(x2+i, y2+j);
		}
	return imgdiff;
}

SunImage* imageSum(const SunImage* img1, const SunImage* img2, float x1, float y1, float x2, float y2, int width, int height)
{
	register int hw = width/2, hh = height/2;
	register int i, j;
	SunImage* imgsum = new SunImage(width, height);
	PixelType *ptr = &(imgsum->pixel(0));
	/* Compute values */
	for (j = -hh ; j <= hh ; j++)
		for (i = -hw ; i <= hw ; i++)
		{
			*ptr++ = img1->interpolate(x1+i, y1+j) + img2->interpolate(x2+i, y2+j);
		}
	return imgsum;
}

void compute2by2GradientMatrix(
const SunImage*  gradx,
const SunImage*  grady,
float& gxx,										  /* return values */
float& gxy,
float& gyy)

{

	/* Compute values */
	gxx = 0.0;  gxy = 0.0;  gyy = 0.0;
	for (unsigned i = 0 ; i < gradx->NumberPixels() ; ++i)
	{
		gxx += gradx->pixel(i)*gradx->pixel(i);
		gxy += gradx->pixel(i)*grady->pixel(i);
		gyy += grady->pixel(i)*grady->pixel(i);
	}
}


void compute2by1ErrorVector(
const SunImage*   imgdiff,
const SunImage*   gradx,
const SunImage*   grady,
float step_factor,								  /* 2.0 comes from equations, 1.0 seems to avoid overshooting */
float &ex,										  /* return values */
float &ey)
{

	/* Compute values */
	ex = 0;  ey = 0;
	for (unsigned i = 0 ; i < imgdiff->NumberPixels() ; i++)
	{
		ex += imgdiff->pixel(i) * gradx->pixel(i);
		ey += imgdiff->pixel(i) * (grady->pixel(i));
	}
	ex *= step_factor;
	ey *= step_factor;
}


static int solveEquation(
float gxx, float gxy, float gyy,
float ex, float ey,
float small,
float &dx, float &dy)
{
	float det = gxx*gyy - gxy*gxy;

	if (det < small)  return KLT_SMALL_DET;

	dx = (gyy*ex - gxy*ey)/det;
	dy = (gxx*ey - gxy*ex)/det;
	return KLT_TRACKED;
}


float sumAbsWindow(const SunImage* window, int width, int height)
{
	float sum = 0.0;
	int w;
	const PixelType *ptr = &(window->pixel(0));
	for ( ; height > 0 ; height--)
		for (w=0 ; w < width ; w++)
			sum += (float) fabs(*ptr++);

	return sum;
}


int trackFeature(
float x1,										  /* location of window in first image */
float y1,
float &x2,										  /* starting location of search in second image */
float &y2,
const SunImage* img1,
const SunImage* gradx1,
const SunImage* grady1,
const SunImage* img2,
const SunImage* gradx2,
const SunImage* grady2,
const int width,								  /* size of window */
const int height,
float step_factor,								  /* 2.0 comes from equations, 1.0 seems to avoid overshooting */
int max_iterations,
float small_det,								  /* determinant threshold for declaring KLT_SMALL_DET */
float th,										  /* displacement threshold for stopping               */
float max_residue								  /* residue threshold for declaring KLT_LARGE_RESIDUE */
)
{

	float gxx, gxy, gyy, ex, ey, dx, dy;
	int iteration = 0;
	int status;
	int hw = width/2;
	int hh = height/2;
	int nc = img1->Xaxes();
	int nr = img1->Yaxes();
	float one_plus_eps = 1.001f;				  /* To prevent rounding errors */

	/* Iteratively update the window position */
	do
	{

		/* If out of bounds, exit loop */
		if (  x1-hw < 0.0f || nc-( x1+hw) < one_plus_eps ||
			x2-hw < 0.0f || nc-(x2+hw) < one_plus_eps ||
			 y1-hh < 0.0f || nr-( y1+hh) < one_plus_eps ||
			y2-hh < 0.0f || nr-(y2+hh) < one_plus_eps)
		{
			status = KLT_OOB;
			break;
		}

		/* Compute gradient and difference windows */

		SunImage* imgdiff = imageDifference(img1, img2, x1, y1, x2, y2, width, height);
		SunImage* gradx = imageSum(gradx1, gradx2, x1, y1, x2, y2, width, height);
		SunImage* grady = imageSum(grady1, grady2, x1, y1, x2, y2, width, height);


		/* Use these windows to construct matrices */
		compute2by2GradientMatrix(gradx, grady, gxx, gxy, gyy);
		compute2by1ErrorVector(imgdiff, gradx, grady, step_factor, ex, ey);

		delete imgdiff;
		delete gradx;
		delete grady;

		/* Using matrices, solve equation for new displacement */
		status = solveEquation(gxx, gxy, gyy, ex, ey, small_det, dx, dy);
		if (status == KLT_SMALL_DET)  break;

		x2 += dx;
		y2 += dy;
		iteration++;

	}  while ((fabs(dx)>=th || fabs(dy)>=th) && iteration < max_iterations);

	/* Check whether window is out of bounds */
	if (x2-hw < 0.0f || nc-(x2+hw) < one_plus_eps ||
		y2-hh < 0.0f || nr-(y2+hh) < one_plus_eps)
		status = KLT_OOB;

	/* Check whether residue is too large */
	if (status == KLT_TRACKED)
	{
		SunImage* imgdiff = imageDifference(img1, img2, x1, y1, x2, y2, width, height);
		if (sumAbsWindow(imgdiff, width, height)/(width*height) > max_residue)
			status = KLT_LARGE_RESIDUE;
		delete imgdiff;
	}


	/* Return appropriate value */
	if (status == KLT_SMALL_DET)  return KLT_SMALL_DET;
	else if (status == KLT_OOB)  return KLT_OOB;
	else if (status == KLT_LARGE_RESIDUE)  return KLT_LARGE_RESIDUE;
	else if (iteration >= max_iterations)  return KLT_MAX_ITERATIONS;
	else  return KLT_TRACKED;

}


//TODO Investigate the computation of kernels, and the sigma

void KLTTrackFeatures(
const SunImage* img1, const SunImage* img2,
vector<Coordinate>& featurelist,
int window_width, int window_height,
unsigned pyramidLevels, int subsampling,
float pyramid_sigma_fact,
float grad_sigma,
float smooth_sigma_fact,
int max_iterations,
float max_residue,
float step_factor,
float min_determinant,
float min_displacement)
{

	/* Check window size (and correct if necessary) */
	if (window_width < 3)
	{
		window_width = 3;
		cerr<<"Tracking context's window width must be at least three.  \n Changing to  "<< window_width<<endl;
	}
	if (window_height < 3)
	{
		window_height = 3;
		cerr<<"Tracking context's window height must be at least three.  \n Changing to  "<< window_height<<endl;
	}
	if (window_width % 2 != 1)
	{
		window_width = window_width+1;
		cerr<<"Tracking context's window width must be odd. Changing to  "<< window_width<<endl;
	}
	if (window_height % 2 != 1)
	{
		window_height = window_height+1;
		cerr<<"Tracking context's window height must be odd.   Changing to  "<< window_height<<endl;
	}

	/* Computing pyramid, and computing gradient pyramids */
	float smoothSigma = smooth_sigma_fact * (window_width > window_height ? window_width : window_height);

	Pyramid pyramid1 = computePyramid(img1, smoothSigma, pyramid_sigma_fact, subsampling, pyramidLevels);
	Pyramid pyramid1_gradx, pyramid1_grady;
	computeGradientPyramids(pyramid1, grad_sigma, pyramid1_gradx, pyramid1_grady);
	
	Pyramid pyramid2 = computePyramid(img2, smoothSigma, pyramid_sigma_fact, subsampling, pyramidLevels);
	Pyramid pyramid2_gradx, pyramid2_grady;
	computeGradientPyramids(pyramid2, grad_sigma, pyramid2_gradx, pyramid2_grady);

	#if DEBUG >= 2
	/* Write internal images */
	for (unsigned L = 0 ; L < pyramidLevels ; L++)
	{
		pyramid1[L]->writeFits(outputFileName + "img1.L" + itos(L) + ".fits");
		pyramid1_gradx[L]->writeFits(outputFileName + "img1.L" + itos(L) + "_dx.fits");
		pyramid1_grady[L]->writeFits(outputFileName + "img1.L" + itos(L) + "_dy.fits");
		pyramid2[L]->writeFits(outputFileName + "img2.L" + itos(L) + ".fits");
		pyramid2_gradx[L]->writeFits(outputFileName + "img2.L" + itos(L) + "_dx.fits");
		pyramid2_grady[L]->writeFits(outputFileName + "img2.L" + itos(L) + "_dy.fits");
	}
	#endif
	/* For each feature, do ... */
	for (unsigned indx = 0 ; indx < featurelist.size() ; ++indx)
	{

		float xloc = featurelist[indx].x;
		float yloc = featurelist[indx].y;

		/* Transform location to coarsest resolution */
		for (int L = pyramidLevels - 1 ; L >= 0 ; L--)
		{
			xloc /= subsampling;
			yloc /= subsampling;
		}
		float xlocout = xloc;
		float ylocout = yloc;

		int val = KLT_OOB;

		/* Beginning with coarsest resolution, do ... */
		for (int L = pyramidLevels - 1 ; L >= 0 ; L--)
		{
			/* Track feature at current resolution */
			xloc *= subsampling;  yloc *= subsampling;
			xlocout *= subsampling;  ylocout *= subsampling;

			val = trackFeature(xloc, yloc, xlocout, ylocout,
				pyramid1[L],
				pyramid1_gradx[L], pyramid1_grady[L],
				pyramid2[L],
				pyramid2_gradx[L], pyramid2_grady[L],
				window_width, window_height,
				step_factor,
				max_iterations,
				min_determinant,
				min_displacement,
				max_residue);

			if (val==KLT_SMALL_DET || val==KLT_OOB)
				break;
		}
		if (val==KLT_SMALL_DET || val==KLT_OOB)
		{
			#if DEBUG >= 3
				cerr<<featurelist[indx]<<" was lost. "<<(val==KLT_SMALL_DET ? "Small determinant":"Out of boundary")<<endl;
			#endif

			featurelist[indx] = Coordinate::Max;
		}
		else
		{
			#if DEBUG >= 3
			if(val==KLT_LARGE_RESIDUE)
				cerr<<featurelist[indx]<<" has a large residue"<<endl;
			if(val==KLT_MAX_ITERATIONS)
				cerr<<featurelist[indx]<<" has reached the maxiterations"<<endl;
			#endif

			featurelist[indx].x = xlocout;
			featurelist[indx].y = ylocout;
		}
		
	}
	// We clean up
	for (unsigned L = 0 ; L < pyramidLevels ; L++)
	{
		delete pyramid1[L];
		delete pyramid1_gradx[L];
		delete pyramid1_grady[L];
		delete pyramid2[L];
		delete pyramid2_gradx[L];
		delete pyramid2_grady[L];
	}
}
