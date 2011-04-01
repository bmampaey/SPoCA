#pragma once
#ifndef HMIImage_H
#define HMIImage_H

#include "EUVImage.h"
#include "Header.h"

class HMIImage : public EUVImage
{
	public :
		
		//Constructors and destructors
		HMIImage();
		HMIImage(const EUVImage& i);
		HMIImage(const EUVImage* i);
		~HMIImage();
		
		//Routines to read and write the keywords from/to the header
		void postRead();
		void preWrite();

		

};

bool isHMI(const Header& header);
#endif

