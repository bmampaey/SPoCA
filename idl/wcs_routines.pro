; This module compact the 2 wcs routines necessary for converting coordonates
PRO get_wcs, header=header, wcs=wcs
	wcs = fitshead2wcs(header)
END

PRO convert, wcs=wcs, coord_type=coord_type, cartesian=cartesian, ARCSECONDS=arcseconds, x, y
	wcs_coord = WCS_GET_COORD(wcs, cartesian)
	IF N_ELEMENTS(arcsecond) EQ 0 THEN arcsecond = 0
	IF coord_type EQ "HGC" THEN BEGIN
		coord_type = "HG"
		carrington = 1
	ENDIF
	IF coord_type EQ "HGS" THEN BEGIN
		coord_type = "HG"
		carrington = 0
	ENDIF
	WCS_CONVERT_FROM_COORD,  wcs, wcs_coord, coord_type , ARCSECONDS = arcseconds, CARRINGTON = carrington, x, y
END

