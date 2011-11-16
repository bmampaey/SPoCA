; +
; Description:
;	IDL code to convert pixel coordinates to helio coordinates using the wcs formulas
; Authors:
; 	Benjamin Mampaey
; Date:
; 	27 October 2011
; Params:
; 	wcs: in, required, a wcs structure 
;	x: in, required, the array with the x portion of the coordinates to convert
;	y: in, required, the array with the y portion of the coordinates to convert
;	coordinate: in, require, a string representing the type of helio coordinate (e.g. HPC, HGS, HGC, ...)
;	arcsec: in, optional, a flag indicating if the HPC coordinates should be in arcsec
;
;	return: an array [x, y] of the converted coordinates
; -

FUNCTION convert_coordinate, wcs, x, y, coordinate, arcsec=arcsec
	
	cartesian_coodinates = FLTARR(2, N_ELEMENTS(x))
	cartesian_coodinates[0,*] = x
	cartesian_coodinates[1,*] = y
	
	wcs_coodinates = WCS_GET_COORD(wcs, cartesian_coodinates)
	
	CASE coordinate OF 
		"HGS" : WCS_CONVERT_FROM_COORD, wcs, wcs_coodinates, 'HG', helio_x, helio_y
		"HGC" : WCS_CONVERT_FROM_COORD, wcs, wcs_coodinates, 'HG', /CARRINGTON, helio_x, helio_y
		"PIXLOC" : BEGIN 
				helio_x = x
				helio_y = y
			   END
		ELSE : WCS_CONVERT_FROM_COORD, wcs, wcs_coodinates, coordinate, helio_x, helio_y, arcsec=arcsec
	ENDCASE
	
	RETURN, [[helio_x], [helio_y]]

END


PRO get_stats, files=files, table_name=table_name, colors=colors, output=output, coordinate=coordinate, arcsec=arcsec

IF N_ELEMENTS(output) EQ 0 THEN output = 'get_stats.txt'
IF N_ELEMENTS(coordinate) EQ 0 THEN coordinate = 'PIXLOC'

IF N_ELEMENTS(colors) GT 0 THEN BEGIN
	IF size(colors, /tn) EQ "STRING" THEN BEGIN
		PRINT, "Reading colors from file ", colors
		colorstring = RD_TFILE(colors, /compress)
		IF execute("colors="+colorstring[0]) NE 1 THEN BEGIN
			PRINT, "Error reading colors from file"
			RETURN
		ENDIF
	ENDIF
	PRINT, "Writing stats for regions of colors ", colors
ENDIF

openw, lun, output, /get_lun, width=100000

printf, lun, "TRACKED_COLOR",",","DATE_OBS",",","XBOXMIN_"+coordinate,",","YBOXMIN_"+coordinate,",", "XBOXMAX_"+coordinate,",","YBOXMAX_"+coordinate,",","XCENTER_"+coordinate,",","YCENTER_"+coordinate,",","XCENTER_ERROR",",","YCENTER_ERROR",",", "NUMBER_PIXELS",",", "AREA_ATDISKCENTER",",", "AREA_ATDISKCENTER_UNCERTAINITY",",", "RAW_AREA",",", "RAW_AREA_UNCERTAINITY",",","MIN_INTENSITY",",", "MAX_INTENSITY",",", "MEAN_INTENSITY",",", "MEDIAN_INTENSITY",",", "VARIANCE",",", "SKEWNESS",",", "KURTOSIS",",", "TOTAL_INTENSITY",",","CLIPPED_SPATIAL"

for i = 0, N_ELEMENTS(files) -1 DO BEGIN

	fits_file = files[i]
	
	; We read the header
	header = fitshead2struct(HEADFITS(fits_file, EXTEN=1))
	wcs = fitshead2wcs(header)
	
	; We read the table of regions
	regions_table = MRDFITS(fits_file , "Regions", regions_table_header, extnum=extnum, status=status)
	
	; If the table of region is empty, MRDFITS return 0 
	IF size(regions_table, /tn) NE "STRUCT" THEN BEGIN
		print, 'No regions in file ', fits_file
		CONTINUE
	ENDIF 

	; We read the table of stats
	regions_stats_table = MRDFITS(fits_file , table_name, regions_stats_table_header, extnum=extnum, status=status) 

	; If the table of region is empty, MRDFITS return 0 
	IF size(regions_stats_table, /tn) NE "STRUCT" THEN BEGIN
		printf, lun, 'No stats in table'
		CONTINUE
	ENDIF 

	number_regions = N_ELEMENTS(regions_table)

	print, "Found ", number_regions, " regions in file ", fits_file

	FOR k = 0, number_regions - 1 DO BEGIN 
		
		; We check if the color is in the desired ones
		IF N_ELEMENTS(colors) GT 0 THEN BEGIN
			temp = WHERE(colors EQ regions_table[k].TRACKED_COLOR, count)
			IF count EQ 0 THEN CONTINUE
		ENDIF
		
		; We convert the box coodinates into helio
		x = FLOAT([regions_table[k].XBOXMIN, regions_table[k].XBOXMAX])
		y = FLOAT([regions_table[k].YBOXMIN, regions_table[k].YBOXMAX])
		
		helio_coordinates = convert_coordinate(wcs, x, y, coordinate, arcsec=arcsec)
		box_x = helio_coordinates[*,0]
		box_y = helio_coordinates[*,1]
		
		s_k = WHERE(regions_stats_table.ID EQ regions_table[k].ID, count)
		
		IF count EQ 0 THEN BEGIN
			print, "Writing region info for region ", regions_table[k].ID
			printf, lun, regions_table[k].TRACKED_COLOR,",", regions_table[k].DATE_OBS,",", box_x[0],",", box_y[0],",", box_x[1],",", box_y[1],",", "No stats"
		ENDIF ELSE BEGIN
			print, "Writing region info for region ", regions_table[k].ID, " and stats for region stats id ", regions_stats_table[s_k].ID
			; We convert the center coodinates into helio
			helio_coordinates = convert_coordinate(wcs, regions_stats_table[s_k].XCENTER, regions_stats_table[s_k].YCENTER, coordinate, arcsec=arcsec)
			center_x = helio_coordinates[*,0]
			center_y = helio_coordinates[*,1]
			printf, lun, regions_table[k].TRACKED_COLOR,",", regions_table[k].DATE_OBS,",", box_x[0],",", box_y[0],",", box_x[1],",", box_y[1],",", center_x,",", center_y,",", regions_stats_table[s_k].XCENTER_ERROR * header.CDELT1,",", regions_stats_table[s_k].YCENTER_ERROR * header.CDELT2,",", regions_stats_table[s_k].NUMBER_PIXELS,",", regions_stats_table[s_k].AREA_ATDISKCENTER,",", regions_stats_table[s_k].AREA_ATDISKCENTER_UNCERTAINITY,",", regions_stats_table[s_k].RAW_AREA,",", regions_stats_table[s_k].RAW_AREA_UNCERTAINITY,",",regions_stats_table[s_k].MIN_INTENSITY,",", regions_stats_table[s_k].MAX_INTENSITY,",", regions_stats_table[s_k].MEAN_INTENSITY,",", regions_stats_table[s_k].MEDIAN_INTENSITY,",", regions_stats_table[s_k].VARIANCE,",", regions_stats_table[s_k].SKEWNESS,",", regions_stats_table[s_k].KURTOSIS,",", regions_stats_table[s_k].TOTAL_INTENSITY,",", regions_stats_table[s_k].CLIPPED_SPATIAL
		ENDELSE
	ENDFOR
	FLUSH, lun
ENDFOR
Free_lun, lun
END


