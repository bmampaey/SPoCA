; +
; Description:
;	IDL code to run spoca, the tracking and get_regions_stats and save the results to text files
; Author:
; 	Benjamin Mampaey
; Date:
; 	26 October 2010
; Params:
; 	imagesDirectory: in, optional, type string, directory containing the image fits files (! You need to specify either imagesDirectory or files171 and files 195)
;	files171: in, optional, type string array, list of fits files of wavelength 171
;	files195: in, optional, type string array, list of fits files of wavelength 195
;	outputDirectory: in, optional, type string, directory to save the results to
;	cCodeLocation: in, optional, type string, directory with the c executables


PRO runspoca, imagesDirectory=imagesDirectory, files171=files171, files195=files195, outputDirectory=outputDirectory, cCodeLocation=cCodeLocation

; Type of images you want
w171='171'
w195='195'
instrument = 'UNKNOWN' ; If you dont know, set to "UNKNOWN" or leave blank

; Parameters for the classification
spocaArgsClassifierType = 'HFCM'
spocaArgsPreprocessing = 'ALC,DivMedian'
spocaArgsNumberclasses ='3'
spocaArgsPrecision = '0.000000001' ; Set accordingly to the preprocessing steps
spocaArgsBinsize = '0.01,0.01' ; Set accordingly to the preprocessing steps

; Parameters for the tracking
trackingArgsDeltat = '90000'; Should never be smaller than the maximal time between 2 consecutives images

; Parameters for the get_regions
getregionArgsPreprocessing = 'NAR'
getregionArgsRadiusRatio = '0.95'
	
; set debugging
debug = 1


; newline shortcut for the c++ programmer
endl=STRING(10B)


; --------- We verify the parameters -----------------

IF (debug GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF PARAMETERS CHECK', 100, fill='_')
ENDIF





IF KEYWORD_SET(imagesDirectory) THEN BEGIN

	files171=['']
	files195=['']
	files = FILE_SEARCH(imagesDirectory, '*.fits', /TEST_READ, /TEST_REGULAR)
	FOR i=0, N_ELEMENTS(files) - 1 DO BEGIN
		IF instrument EQ 'AIA' THEN read_sdo, files[i], header, /nodata ELSE header = fitshead2struct(headfits(files[i]))
		IF header.WAVELNTH EQ w171 THEN files171 = [files171, files[i]]
		IF header.WAVELNTH EQ w195 THEN files195 = [files195, files[i]]
	ENDFOR
	files171 = files171[1:*]
	files195 = files195[1:*]
	
	

	IF N_ELEMENTS(files171) EQ 0 THEN BEGIN

		print, 'No files with wavelength ', w171, ' found!"
		RETURN
	
	ENDIF

	IF N_ELEMENTS(files195) EQ 0 THEN BEGIN

		print, 'No files with wavelength ', w195, ' found!"
		RETURN
	
	ENDIF
	
ENDIF ELSE BEGIN

; We test the provided filenames
	FOR i=0, MIN([(N_ELEMENTS(files171) - 1), (N_ELEMENTS(files195) - 1)]) DO BEGIN
	
		IF (~ FILE_TEST( files171[i], /READ, /REGULAR)) THEN BEGIN
			PRINT, 'Cannot find image ' , files171[i] 
			RETURN
		ENDIF
		
		IF (~ FILE_TEST( files195[i], /READ, /REGULAR)) THEN BEGIN
			PRINT, 'Cannot find image ' , files195[i] 
			RETURN
		ENDIF
		
	ENDFOR
	
ENDELSE

print, files171
print, files195

IF NOT KEYWORD_SET(outputDirectory) THEN outputDirectory = "./" 
IF NOT KEYWORD_SET(cCodeLocation) THEN cCodeLocation = "../bin/" 
IF N_ELEMENTS(instrument) EQ 0 THEN instrument = 'UNKNOWN'


; SPoCA parameters

spoca_bin = cCodeLocation + 'classification.x'

IF ~ FILE_TEST( spoca_bin, /EXECUTABLE)  THEN BEGIN
	error = [ error, 'Cannot find executable ' + spoca_bin ]
	IF (debug GT 0) THEN BEGIN
		PRINT , 'Cannot find executable ' + spoca_bin
	ENDIF
	RETURN
ENDIF
IF N_ELEMENTS(spocaArgsClassifierType) EQ 0 THEN spocaArgsPreprocessing = 'HFCM'
IF N_ELEMENTS(spocaArgsPreprocessing) EQ 0 THEN spocaArgsPreprocessing = 'ALC,DivMedian'  
IF N_ELEMENTS(spocaArgsNumberclasses) EQ 0 THEN spocaArgsNumberclasses = '3'
IF N_ELEMENTS(spocaArgsPrecision) EQ 0 THEN spocaArgsPrecision = '0.000001'
IF N_ELEMENTS(spocaArgsBinsize) EQ 0 THEN spocaArgsBinsize = '0.01,0.01'
spoca_args_centersfile = outputDirectory + 'centers.txt'


; Tracking parameters

tracking_bin = cCodeLocation + 'tracking.x'

IF ~ FILE_TEST( tracking_bin, /EXECUTABLE)  THEN BEGIN
	error = [ error, 'Cannot find executable ' + tracking_bin ]
	IF (debug GT 0) THEN BEGIN
		PRINT , 'Cannot find executable ' + tracking_bin
	ENDIF
	RETURN
ENDIF
IF N_ELEMENTS(trackingArgsDeltat) EQ 0 THEN trackingArgsDeltat = '21600' ; It is in seconds


; GetRegionStats parameters

getregionstats_bin = cCodeLocation + 'get_regions_HEK.x'
IF ~ FILE_TEST( getregionstats_bin, /EXECUTABLE)  THEN BEGIN
	error = [ error, 'Cannot find executable ' + getregionstats_bin ]
	IF (debug GT 0) THEN BEGIN
		PRINT , 'Cannot find executable ' + getregionstats_bin
	ENDIF
	RETURN
ENDIF
IF N_ELEMENTS(getregionArgsPreprocessing) EQ 0 THEN getregionArgsPreprocessing = 'NAR'
IF N_ELEMENTS(getregionArgsRadiusRatio) EQ 0 THEN getregionArgsRadiusRatio = '0.95'


IF (debug GT 0) THEN BEGIN

	PRINT, endl, STRPAD('END OF PARAMETERS CHECK', 100, fill='_')
ENDIF


; --------- We take care of running spoca -----------------

IF (debug GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF SPOCA', 100, fill='_')
ENDIF


FOR i=0, MIN([(N_ELEMENTS(files171) - 1), (N_ELEMENTS(files195) - 1)]) DO BEGIN

	; We initialise correctly the arguments for SPoCA
	spoca_args = [	'-P', spocaArgsPreprocessing, $
				'-T', spocaArgsClassifierType, $
				'-C', spocaArgsNumberclasses, $
				'-p', spocaArgsPrecision, $
				'-z', spocaArgsBinsize, $
				'-B', spoca_args_centersfile, $
				'-O', outputDirectory + STRING(i, FORMAT='(I010)'), $
				'-I', instrument, $
				files171[i], files195[i] ]

	IF (debug GT 0) THEN BEGIN

		PRINT, 'About to run : ' , STRJOIN( [spoca_bin , spoca_args] , ' ', /SINGLE ) 
		time_before_run = SYSTIME(/SECONDS) 
	
	ENDIF

	; We call SPoCA with the correct arguments

	SPAWN, [spoca_bin , spoca_args], spoca_output, spoca_errors, /NOSHELL, EXIT_STATUS=spoca_exit 

	IF (debug GT 0) THEN BEGIN

		PRINT, 'run time (seconds): ' , SYSTIME(/SECONDS) - time_before_run

	ENDIF

	; In case of error
	IF (spoca_exit NE 0) THEN BEGIN

		IF (debug GT 0) THEN BEGIN
			PRINT , "SPoCA exited with error : ", spoca_exit, endl, spoca_errors
		ENDIF
	
	ENDIF
	
ENDFOR


IF (debug GT 0) THEN BEGIN

	PRINT, endl, STRPAD('END OF SPOCA', 100, fill='_')
ENDIF


; --------- We take care of the tracking -----------------

IF (debug GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF TRACKING', 100, fill='_')
ENDIF


ARmaps = FILE_SEARCH(outputDirectory, '*ARmap.fits', /TEST_READ, /TEST_REGULAR , /TEST_WRITE  ) ; FILE_SEARCH sort the filenames

IF (debug GT 0) THEN BEGIN
	PRINT , "Found files : ", endl + ARmaps
ENDIF

		
; We initialise correctly the arguments for Tracking_HEK

tracking_args =	[	'-n', '0', $
					'-d', trackingArgsDeltat, $
					'-o', '1', $
					'-D', '-A', $
					ARmaps ]


IF (debug GT 0) THEN BEGIN
	PRINT, 'About to run : ', STRJOIN( [tracking_bin , tracking_args] , ' ', /SINGLE )
	time_before_run = SYSTIME(/SECONDS) 
ENDIF

SPAWN, [tracking_bin , tracking_args] , tracking_output, tracking_errors, /NOSHELL, EXIT_STATUS=tracking_exit 

IF (debug GT 0) THEN BEGIN

	PRINT, 'run time (seconds): ' , SYSTIME(/SECONDS) - time_before_run

ENDIF

IF (tracking_exit NE 0) THEN BEGIN
	
	IF (debug GT 0) THEN BEGIN
		PRINT , "Tracking exited with error : ", tracking_exit, endl, tracking_errors
	ENDIF
	; We will not run GetRegionStats
	GOTO, Finish
	
ENDIF


IF (debug GT 0) THEN BEGIN
	PRINT, 'Tracking Output is :', endl + tracking_output
ENDIF


IF (debug GT 0) THEN BEGIN

	PRINT, endl, STRPAD('END OF TRACKING', 100, fill='_')
ENDIF

; --------- We take care of the computing of the Regions Stats -----------------

IF (debug GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF GETREGIONSTATS', 100, fill='_')
ENDIF

FOR i=0, MIN([(N_ELEMENTS(files171) - 1), (N_ELEMENTS(files195) - 1)]) DO BEGIN

	ARmap = outputDirectory + STRING(i, FORMAT='(I010)') + '.ARmap.fits'

	IF (~ FILE_TEST( ARmap, /READ, /REGULAR))  THEN BEGIN
		PRINT, 'Cannot find AR map ', ARmap, ', trying next one!' 
		CONTINUE
	ENDIF


	getregionstats_args = [	'-P', getregionArgsPreprocessing, $
						'-r', getregionArgsRadiusRatio, $
						'-M', ARmap, $
						'-I', instrument, $
						files171[i] ]

	IF (debug GT 0) THEN BEGIN

		PRINT, 'About to run : ' , STRJOIN( [getregionstats_bin , getregionstats_args] , ' ', /SINGLE )
		time_before_run = SYSTIME(/SECONDS) 
	
	ENDIF

	; We call RegionsStats with the correct arguments

	SPAWN, [getregionstats_bin , getregionstats_args], getregionstats_output, getregionstats_errors, /NOSHELL, EXIT_STATUS=getregionstats_exit 

	IF (debug GT 0) THEN BEGIN

		PRINT, 'run time (seconds): ' , SYSTIME(/SECONDS) - time_before_run

	ENDIF

	; In case of error
	IF (getregionstats_exit NE 0) THEN BEGIN
	
		IF (debug GT 0) THEN BEGIN
			PRINT , "RegionsStats exited with error : ", getregionstats_exit, endl, getregionstats_errors
		ENDIF
		CONTINUE
	ENDIF

	; As output of GetRegionStats we receive the stats on the AR intra limb 
	IF (debug GT 0) THEN BEGIN
		PRINT, 'GetRegionStats Output is :', endl + getregionstats_output
	ENDIF


	; We check that output is not null
	IF (N_ELEMENTS(getregionstats_output) LT 1 || STRLEN(getregionstats_output[0]) LE 1 ) THEN BEGIN
		IF (debug GT 0) THEN BEGIN
			PRINT, 'ERROR GetRegionStats Output is void, going to Finish'
		ENDIF
		CONTINUE
	ENDIF

	; We open the file for saving the results
	OPENW, regions_file, outputDirectory + STRING(i, FORMAT='(I010)') + '.regions.txt', /GET_LUN, WIDTH = 1000

	; We need the wcs info in the header of one of the image to transform the coordinates
	IF instrument EQ 'AIA' THEN read_sdo, files171[i] , header, /nodata ELSE header = fitshead2struct(headfits(files171[i] ))
	wcs = fitshead2wcs(header)


	FOR k = 0, N_ELEMENTS(getregionstats_output) - 1 DO BEGIN 

		; The output of GetRegionStats is: label id color observationdate (center.x, center.y) (boxLL.x, boxLL.y) (boxUR.x, boxUR.y) numberpixels minintensity maxintensity mean variance skewness kurtosis totalintensity (centererror.x, centererror.y) area_raw area_rawuncert area_atdiskcenter area_atdiskcenteruncert
		output = strsplit( getregionstats_output[k] , ' 	(),', /EXTRACT) 
		; We parse the output
		label = output[0]
		id = output[1]
		color = LONG(output[2])
		observationdate = output[3]
		cartesian_x = FLOAT(output[4:8:2])
		cartesian_y = FLOAT(output[5:9:2])
		cartesian = FLTARR(2,N_ELEMENTS(cartesian_x))
		cartesian[0,*]=cartesian_x
		cartesian[1,*]=cartesian_y
		numberpixels = LONG(output[7])

		minintensity = FLOAT(output[11])
		maxintensity = FLOAT(output[12])
		mean = FLOAT(output[13])
		variance = FLOAT(output[14])
		skewness = FLOAT(output[15])
		kurtosis = FLOAT(output[16])
		totalintensity = FLOAT(output[17])
		centerx_error = FLOAT(output[18])
		centery_error = FLOAT(output[19])
		area_raw = FLOAT(output[20])
		area_rawuncert = FLOAT(output[21])
		area_atdiskcenter = FLOAT(output[22])
		area_atdiskcenteruncert = FLOAT(output[23])
	
		IF (debug GT 0) THEN BEGIN
			PRINT , "cartesians coordinates for the region ", k
			PRINT, cartesian
		ENDIF
		
		; We convert the cartesian pixel coodinates into WCS
		   wcs_coord = WCS_GET_COORD(wcs, cartesian)
		   
		; We convert the WCS coodinates into helioprojective cartesian
		WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HPC', /ARCSECONDS, hpc_x, hpc_y
	
		IF (debug GT 0) THEN BEGIN
			PRINT , "x, y, z HPC coordinates for the region ", k
			PRINT, hpc_x
			PRINT, hpc_y
		ENDIF
	
		; We write the desire info to the results file
	
		PRINTF, regions_file, label, $
		' ' , id, $
		' ' , color, $
		' ' , observationdate, $
		
		' ' , hpc_x[0], $ ; Center
		' ' , hpc_y[0], $
		' ' , hpc_x[1], $ ; Box inf corner
		' ' , hpc_y[1], $
		' ' , hpc_x[2], $ ; Box sup corner
		' ' , hpc_y[2], $
		
		' ' , numberpixels, $
		' ' , minintensity, $
		' ' , maxintensity, $
		' ' , mean, $
		' ' , variance, $
		' ' , skewness, $
		' ' , kurtosis, $
		' ' , totalintensity, $
		' ' , centerx_error, $
		' ' , centery_error, $
		' ' , area_raw, $
		' ' , area_rawuncert, $
		' ' , area_atdiskcenter, $
		' ' , area_atdiskcenteruncert

	ENDFOR 
	FREE_LUN, regions_file
ENDFOR

IF (debug GT 0) THEN BEGIN
	PRINT, endl, STRPAD('END OF GETREGIONSTATS', 100, fill='_')
ENDIF

Finish :
END 

