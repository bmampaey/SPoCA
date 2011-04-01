#pragma once
#ifndef EITImage_H
#define EITImage_H

#include "EUVImage.h"
#include "Header.h"

class EITImage : public EUVImage
{
	public :
		//Constructors and destructors
		EITImage();
		EITImage(const EUVImage& i);
		EITImage(const EUVImage* i);
		~EITImage();
		
		//Routines to read and write the keywords from/to the header
		void postRead();
		void preWrite();
};

bool isEIT(const Header& header);
#endif
