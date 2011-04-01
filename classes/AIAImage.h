#pragma once
#ifndef AIAImage_H
#define AIAImage_H

#include "EUVImage.h"
#include "Header.h"

class AIAImage : public EUVImage
{
	public :
		//Constructors and destructors
		AIAImage();
		AIAImage(const EUVImage& i);
		AIAImage(const EUVImage* i);
		~AIAImage();
		
		//Routines to read and write the keywords from/to the header
		void postRead();
		void preWrite();
};

bool isAIA(const Header& header);
#endif
