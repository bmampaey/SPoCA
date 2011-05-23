#include <iostream>
#include <cmath>
#include <typeinfo>
#include <string>
#include "../classes/EITImage.h"
#include "../classes/Header.h"

using namespace std;



int main()
{
	EITImage image;
	image.readFits("/data/eit/200109/EFZ20010911.010014.fits");
	cout<<expand("Bonjour my name is {FILENAME} and my wavelength is {WAVELNTH}A ", image.header)<<endl;

}
