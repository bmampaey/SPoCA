#include <iostream>
#include <cmath>
#include <typeinfo>
#include <string>
#include "../classes/tools.h"
#include "../classes/constants.h"
#include "../classes/FeatureVector.h"

using namespace std;



int main()
{
	string hello = "[1,2,3,4]5,6 [7,8,9,10]";
	vector<int> a;
	RealFeature b;
	vector<RealFeature> c;
	vector<int>d;
	hello>>a>>b>>c;
	cout<<a<<endl<<b<<endl<<c<<endl<<d<<endl;

}
