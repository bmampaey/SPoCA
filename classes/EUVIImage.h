#pragma once
#ifndef EUVIImage_H
#define EUVIImage_H

#include "EUVImage.h"
#include "Header.h"

class EUVIImage : public EUVImage
{
	public :
		//Constructors and destructors
		EUVIImage();
		EUVIImage(const EUVImage& i);
		EUVIImage(const EUVImage* i);
		~EUVIImage();
		
		//Routines to read and write the keywords from/to the header
		void postRead();
		void preWrite();
};

bool isEUVI(const Header& header);
#endif
