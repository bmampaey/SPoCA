// This program will call optical flow on a pair of sun images
// Written by Benjamin Mampaey on 15 July 2010

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/mainutilities.h"
#include "../classes/ArgumentHelper.h"

#include "../classes/SunImage.h"
#include "../klt/klt.h"



using namespace std;
using namespace dsr;


string filenamePrefix;



int main(int argc, const char **argv)
{
	cout<<setiosflags(ios::fixed);
	
	// The list of names of the sun images to process
	vector<string> imagesFilenames;

	string programDescription = "This Program will do optical flow between 2 or more images.\n";
	programDescription+="Compiled with options :";
	programDescription+="\nDEBUG: "+ itos(DEBUG);
	programDescription+="\nEUVPixelType: " + string(typeid(EUVPixelType).name());
	programDescription+="\nReal: " + string(typeid(Real).name());

	ArgumentHelper arguments;
	arguments.set_string_vector("fitsFileName1 fitsFileName2 ...", "\n\tThe name of the fits files containing the maps of the regions to track.\n\t", imagesFilenames);
	arguments.set_description(programDescription.c_str());
	arguments.set_author("Benjamin Mampaey, benjamin.mampaey@sidc.be");
	arguments.set_build_date(__DATE__);
	arguments.set_version("1.0");
	arguments.process(argc, argv);

	// We read and preprocess the sun images
	vector<SunImage*> images = getImagesFromFiles("UNKNOWN", imagesFilenames, true);
	for (unsigned p = 0; p < images.size(); ++p)
	{
		images[p]->preprocessing("ALC", 3);
		#if DEBUG >= 2
		images[p]->writeFits(filenamePrefix + "preprocessed." +  stripPath(imagesFilenames[p]) );
		#endif
		for(unsigned i = 0; i < images[p]->NumberPixels(); ++i)
			if(images[p]->pixel(i) == images[p]->nullvalue())
				images[p]->pixel(i) = 0;
	}


  KLT_TrackingContext tc;
  KLT_FeatureList fl;
  int nFeatures = 10;
  int ncols = images[0]->Xaxes(), nrows = images[0]->Yaxes();
  int i;

  tc = KLTCreateTrackingContext();
  tc->max_residue = 1000;
  tc->window_width *= 4;
  tc->window_height *= 4;
      tc->nPyramidLevels *= 2;
    tc->subsampling /= 2;
    cout<<tc->window_width<<endl<<tc->window_height<<endl<<tc->nPyramidLevels<<endl<<tc->subsampling<<endl;
  KLTPrintTrackingContext(tc);
  fl = KLTCreateFeatureList(nFeatures);


  KLTSelectGoodFeatures(tc, &(images[0]->pixel(0)), ncols, nrows, fl);

	SunImage* temp = new SunImage(images[0]);

  printf("\nIn first image:\n");
  for (i = 0 ; i < fl->nFeatures ; i++)  {
  	fl->feature[i]->x = 200 + i*60;
  	fl->feature[i]->y = fl->feature[i]->x;
    printf("Feature #%d:  (%f,%f) with value of %d\n",
           i, fl->feature[i]->x, fl->feature[i]->y,
           fl->feature[i]->val);
    temp->drawCross(0, Coordinate(fl->feature[i]->x, fl->feature[i]->y), 5+2*i);
  }
  temp->writeFits(filenamePrefix + "features." +  stripPath(imagesFilenames[0]) );
	delete temp;
  //KLTWriteFeatureListToPPM(fl, img1, ncols, nrows, "feat1.ppm");
  KLTWriteFeatureList(fl, "feat1.txt", "%3d");

  KLTTrackFeatures(tc, &(images[0]->pixel(0)), &(images[1]->pixel(0)), ncols, nrows, fl);
  
	temp = new SunImage(images[1]);
  printf("\nIn second image:\n");
  for (i = 0 ; i < fl->nFeatures ; i++)  {
    printf("Feature #%d:  (%f,%f) with value of %d\n",
           i, fl->feature[i]->x, fl->feature[i]->y,
           fl->feature[i]->val);
    if(fl->feature[i]->val == 0)
    temp->drawCross(0, Coordinate(fl->feature[i]->x, fl->feature[i]->y), 5+2*i);
  }
 temp->writeFits(filenamePrefix + "features." +  stripPath(imagesFilenames[1]) );
	delete temp;
  //KLTWriteFeatureListToPPM(fl, img2, ncols, nrows, "feat2.ppm");
  KLTWriteFeatureList(fl, "feat2.fl", NULL);      /* binary file */
  KLTWriteFeatureList(fl, "feat2.txt", "%5.1f");  /* text file   */

	
	return EXIT_SUCCESS;
}
