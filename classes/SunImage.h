#pragma once
#ifndef SunImage_H
#define SunImage_H


#include <vector>
#include <string>
#include <ctime>


#include "Image.h"
#include "WCS.h"
#include "Header.h"
#include "Coordinate.h"
#include "FitsFile.h"



//! General class for storing a Helioprojective cartesian image of the sun.
/* ! 

 Simple mono channel 2 dimensions image, with additional WCS parameters about the Helioprojective cartesian projection of the sun represented in the image.
 
 It implements a series of routines to retrieve and convert sun coordinates.
 
 It implements a series of routines to convert to other Heliographic projections.
 
 It implements some common routines related to the sun, such as the computation of the differential rotation.

*/

//! @tparam T Type of a single pixel
template<class T>
class SunImage : public Image<T>
{
	protected :
		//! A header containing all keywords when the image is read from a fits file
		Header header;
	
		//! Parameters about the coordinates of the sun's image.
		WCS wcs;
	
	public :
		//! Constructor for an SunImage of size xAxes x yAxes
		SunImage(const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Constructor for an SunImage of size xAxes x yAxes, with sun center and radius
		SunImage(const unsigned& xAxes, const unsigned& yAxes, const RealPixLoc& suncenter, const Real& sunradius);
		
		//! Constructor for an SunImage from an header
		SunImage(const Header& header, const unsigned& xAxes = 0, const unsigned& yAxes = 0);

		//! Constructor for an SunImage from a WCS
		SunImage(const WCS& wcs, const unsigned& xAxes = 0, const unsigned& yAxes = 0);
		
		//! Copy Constructor
		SunImage(const SunImage& i);
		
		//! Copy Constructor
		SunImage(const SunImage* i);
		
		//! Destructors
		~SunImage();
		
		//! Accessor to retrieve the SunCenter
		RealPixLoc SunCenter() const;
		
		//! Accessor to retrieve the SunRadius
		Real SunRadius() const;
		
		//! Accessor to retrieve the ObservationTime
		time_t ObservationTime() const;
		
		//! Accessor to retrieve the ObservationDate
		std::string ObservationDate() const;
		
		//! Accessor to retrieve the Instrument
		virtual std::string Instrument() const;
		
		//! Return a label for the image
		virtual std::string Label() const;
		
		//! Accessor to retrieve the PixelLength in arcsec (i.e. cdelt1)
		Real PixelLength() const;
		
		//! Accessor to retrieve the PixelWidth in arcsec (i.e. cdelt2)
		Real PixelWidth() const;
		
		//! Accessor to retrieve the header
		Header& getHeader();
		
		
		//! Accessor to retrieve the wcs
		WCS& getWCS();
		
		//! Routine to read the sun parameters from the header
		virtual void parseHeader();
		
		//! Routine to write the sun parameters to the header
		virtual void fillHeader();
		
		//! Routine to set the pixels above a certain radius ratio to null
		void nullifyAboveRadius(const Real radiusRatio = 1.0);
		
		//! Routine to set the pixels above a certain longitude and/or latitude to null
		/*! Longitude and latitudes are absolute values and in Heliographic Stonyhurst coordinates*/
		void nullifyAboveLongLat(Real longitude = 360, Real latitude = 360);

		//! Routine to align the SunImage on the newCenter
		void recenter(const RealPixLoc& newCenter);
		
		//! Routine that rotate the sun in the image by delta_t seconds
		void rotate(const int delta_t);
		
		//! Routine that returns the transformation of the SunImage so that it is similar to img
		/*! Allocate memory for the returned SunImage*/
		SunImage* shifted_like(const SunImage* img) const;
		
		//! Routine that applies a transformation to the SunImage to be similar to img
		void shift_like(const SunImage* img);
		
		//! Routine that rotate a coordinate by delta_t seconds
		RealPixLoc rotate(const RealPixLoc c, const int delta_t) const;
		
		//! Routine that transform a coordinate to the referenciel of img 
		RealPixLoc shift_like(const RealPixLoc c, const SunImage* img) const;
		
		//! Routine that converts the pixel location to Helioprojective cartesian coordinate
		HPC toHPC(const RealPixLoc& c) const;
		
		//! Routine that converts the pixel location to Heliocentric cartesian coordinate
		HCC toHCC(const RealPixLoc& c) const;
		
		//! Routine that converts the pixel location to Heliographic Stonyhurst coordinate
		HGS toHGS(const RealPixLoc& c) const;
		
		//! Routine that converts a Helioprojective cartesian coordinate to the pixel location
		RealPixLoc toRealPixLoc(const HPC& c) const;
		
		//! Routine that converts a Heliocentric cartesian coordinate to the pixel location
		RealPixLoc toRealPixLoc(const HCC& c) const;
		
		//! Routine that converts a Heliographic Stonyhurst coordinate to the pixel location
		RealPixLoc toRealPixLoc(const HGS& c) const;
		
		//! Routine that convert a Heliocentric cartesian coordinate to Heliographic Stonyhurst
		HGS toHGS(const HCC& c) const;
		
		//! Routine that convert a Helioprojective cartesian coordinate to Heliographic Stonyhurst
		HGS toHGS(const HPC& c) const;
		
		//! Routine converts a Heliographic Stonyhurst coordinate to Heliocentric cartesian
		HCC toHCC(const HGS& c) const;
		
		//! Routine converts a Heliographic Stonyhurst coordinate to Helioprojective cartesian
		HPC toHPC(const HGS& c) const;
		
		//! Routine converts a Heliocentric cartesian coordinate to Helioprojective cartesian
		HPC toHPC(const HCC& c) const;
		
		//! Routine converts a Helioprojective cartesian coordinate to Heliocentric cartesian
		HCC toHCC(const HPC& c) const;
		
		//! Routine that returns the map of HGS coordinates of the image
		std::vector<HGS> HGSmap() const;
		
		//! Routine that returns the map of HPC coordinates of the image
		std::vector<HPC> HPCmap() const;
		
		//! Routine that returns the map of HCC coordinates of the image
		std::vector<HCC> HCCmap() const;
		
		//! Accessor to retrieve the interpolated value of the image in c
		T interpolate(const HGS& c) const;
		
		//! Accessor to retrieve the interpolated value of the image in c
		T interpolate(const HPC& c) const;
		
		//! Accessor to retrieve the interpolated value of the image in c
		T interpolate(const HCC& c) const;
		
		using Image<T>::interpolate;
		
		//! Compute the area of a pixel in arcsec²
		Real PixelArea() const;
		
		//! Compute the approximate raw area of a pixel in Mm²
		Real RawPixelArea() const;
		
		//! Compute the area of a pixel at disk center in Mm²
		Real RealPixelArea(const RealPixLoc& c) const;
		
		//! Routine to write to a fits file
		FitsFile& writeFits(FitsFile& file, int mode = 0, const std::string imagename = "");
		
		//! Routine to read from a fits file
		FitsFile& readFits(FitsFile& file);
		
		//! Routine to write the image to a fits file
		/*! The routine will overwrite any fits file with the same name */
		bool writeFits(const std::string& filename, int mode = 0, const std::string imagename = "");
		
		//! Routine to read an image from a fits file
		bool readFits(const std::string& filename);
		
		//! Routine that does the equirectangular projection of the sunimage
		void equirectangular_projection(const SunImage<T>* image, bool exact = false);
		
		//! Routine that does the inverse of the equirectangular projection of the sunimage
		void equirectangular_deprojection(const SunImage<T>* image, bool exact = false);
		
		//! Routine that does the Lambert cylindrical equalarea of the sunimage
		void Lambert_cylindrical_projection(const SunImage<T>* image, bool exact = false);
		
		//! Routine that does the inverse of the Lambert cylindrical equalarea of the sunimage
		void Lambert_cylindrical_deprojection(const SunImage<T>* image, bool exact = false);
		
		//! Routine that does the sinuosidal projection of the sunimage
		void sinuosidal_projection(const SunImage<T>* image, bool exact = false);
		
		//! Routine that does the inverse of the sinuosidal projection of the sunimage
		void sinuosidal_deprojection(const SunImage<T>* image, bool exact = false);
		
		
	public:
		//! Constant for no projection
		static const int no_projection = NO_PROJECTION;
		//! Constant for projection equirectangular
		static const int equirectangular = EQUIRECTANGULAR_PROJECTION;
		//! Constant for projection Lambert_cylindrical
		static const int Lambert_cylindrical = LAMBERT_CYLINDRICAL_PROJECTION;
		//! Constant for projection sinuosidal
		static const int sinuosidal = SINUOSIDAL_PROJECTION;
		//! Constant for distance transform
		static const int distance_transform = DISTANCE_TRANSFORM;
};

//! Routine to check if the SunImage have similar parameters
template<class T1, class T2>
std::string checkSimilar(const SunImage<T1>* image1, const SunImage<T2>* image2)
{
	if (dynamic_cast<const void*>(image1) == dynamic_cast<const void*>(image2))
		return "";
	
	/*! Check if the sun center of the 2 images are more than 1 pixel appart */
	if(distance_squared(image1->SunCenter(), image2->SunCenter()) > 1)
	{
		return "Sun center mismatch";
	}
	
	/*! Check if the sun radius of the 2 images differ from more than 1% */
	if( fabs(1. - (image1->SunRadius() / image2->SunRadius())) > 0.01 )
	{
		return "Sun radius mismatch";
	}
	return "";
}

//! Routine that computes the average differential rotation speed of the sun for a particular latitude 
Real SunDifferentialAngularSpeed(const Real& latitude);
//! Routine that computes the distance between the sun and the earth at a given time
Real distance_sun_earth(const time_t& time_obs);
//! Routine that computes the sun latitude of the earth at a given time
Real earth_latitude(const time_t& time_obs);
#endif
