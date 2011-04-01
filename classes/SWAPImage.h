#pragma once
#ifndef SWAPImage_H
#define SWAPImage_H

#include "EUVImage.h"
#include "Header.h"

class SWAPImage : public EUVImage
{
	public :
		//Constructors and destructors
		SWAPImage();
		SWAPImage(const EUVImage& i);
		SWAPImage(const EUVImage* i);
		~SWAPImage();
		
		//Routines to read and write the keywords from/to the header
		void postRead();
		void preWrite();
};

bool isSWAP(const Header& header);
#endif
