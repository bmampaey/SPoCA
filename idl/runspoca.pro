; procedure "runspoca_script" = specific call to runspoca

PRO runspoca_script

imagesDirectory = '/home/francisv/data/ISSIdataset/2003/CompletePairs_noMissBlock/'
;files171       = files171
;files195       = files195
outputDirectory = '/home/francisv/data/ISSIdataset/2003/CompletePairs_noMissBlock/results/rev15_attribution_center_101_end/'
cCodeLocation   = '/home/francisv/SPoCA/svn_2010_10_rev15_ISSI_MayJune2003/spoca/bin/'
outputFilename  = 'AR_EIT_MayJune2003.txt'

runspoca, imagesDirectory=imagesDirectory, outputDirectory=outputDirectory, cCodeLocation=cCodeLocation, outputFilename = outputFilename
;runspoca, files171=files171, files195=files195, outputDirectory=outputDirectory, cCodeLocation=cCodeLocation, outputFilename = outputFilename

END


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; +
; Description:
;	IDL code to run spoca, the tracking and get_regions_stats and save the results to text files
; IMPORTANT REMARK: YOU NEED TO ADD "DFLAGS= -DAGGREGATE_DILATE" IN CLASSIFICATION.MK OR ATTRIBUTION.MK IF YOU WANT THE DILATED AR MAPS.
;		    BENJAMIN SAYS IT IS NOT NECESSARY TO ADD "DFLAGS= -DHEK" IN TRACKING.MK!
;		    
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
;	outputFilename: out, required, relative path name of the output file


PRO runspoca, imagesDirectory=imagesDirectory, files171=files171, files195=files195, outputDirectory=outputDirectory, cCodeLocation=cCodeLocation, outputFilename = outputFilename


; Type of images you want
w171                        ='171'
w195                        ='195'
instrument                  = 'EIT'		; If you dont know, set to "UNKNOWN" or leave blank


; Parameters for the classification
spocaArgsRadiusratio        = '1.30'
spocaArgsPreprocessing      = 'ALC,TakeLog'
spocaArgsClassifierType     = 'SPoCA2'
spocaArgsNumberclasses      = '6'
spocaArgsPrecision          = '0.000001'	; Set accordingly to the preprocessing steps
spocaArgsMaxNumberIteration = '1000'
spocaArgsBinsize            = '0.01,0.01'	; Set accordingly to the preprocessing steps
spocaArgsSegmentation       = 'fix'		; regroup classes into AR, QS, CH using fixed class mergers
spocaArgsARClasses          = '6'		; AR consist of class 6
spocaArgsQSClasses          = '3,4,5'		; QS consist of classes 3, 4, 5
spocaArgsCHClasses          = '1,2'		; CH consist of classes 1, 2


; Parameters for the tracking
trackingArgsDeltat          = '90000'		; Should never be smaller than the maximal time between 2 consecutives images (90000 = 25 h)
						; Two images more than trackingArgsDeltat apart in time will not be compared for tracking.

; Parameters for the get_regions
getregionArgsPreprocessing = 'NAR'
getregionArgsRadiusRatio = '1.00'

	
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
	files = FILE_SEARCH(imagesDirectory + '*.fits', /TEST_READ, /TEST_REGULAR)
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

;spoca_bin = cCodeLocation + 'classification.x' ; for classification, ADD       OPTION -z; DECOMMENT eta file!!!
spoca_bin = cCodeLocation + 'attribution.x'     ; for attribution   , DECOMMENT OPTION -z; ADD       eta file!!!

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
spocaArgsCentersFile = outputDirectory + 'centers.txt'
spocaArgsEtaFile     = outputDirectory + 'eta.txt'
spocaArgsHistogram   = outputDirectory + 'histogram.txt'


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

	spoca_args = [	'-I', instrument, $
			'-r', spocaArgsRadiusratio, $
			'-P', spocaArgsPreprocessing, $
			'-T', spocaArgsClassifierType, $
			'-C', spocaArgsNumberclasses, $
			'-p', spocaArgsPrecision, $
			'-i', spocaArgsMaxNumberIteration, $
			'-B', spocaArgsCentersFile, $
			'-E', spocaArgsEtaFile, $
;			'-z', spocaArgsBinsize, $
;			'-H', spocaArgsHistogram, $
			'-S', spocaArgsSegmentation, $
			'-a', spocaArgsARClasses, $
			'-q', spocaArgsQSClasses, $
			'-c', spocaArgsCHClasses, $
			'-O', outputDirectory + STRING(i, FORMAT='(I010)'), $
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

ARmaps = FILE_SEARCH(outputDirectory + '*ARmap.fits', /TEST_READ, /TEST_REGULAR, /TEST_WRITE) ; FILE_SEARCH sort the filenames

IF (debug GT 0) THEN BEGIN
	PRINT , "Found files : ", endl + ARmaps
ENDIF

		
; We initialise correctly the arguments for Tracking_HEK

tracking_args =	['-n', '0', $
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

; We open the file for saving the results, and print the header
OPENW, 1, outputDirectory + outputFilename, /APPEND, WIDTH = 1000
	
;	PRINTF, 1, "FRM_SpecificID  Event_StartTime  Event_Coord1 (helioprojective Cartesian x, arcsec)  Event_Coord2  Event_C1Error  Event_C2Error  BoundBox_C1LL  BoundBox_C2LL  BoundBox_C1UR  BoundBox_C2UR  Stonyhurst longitude center (deg)  Stonyhurst latitude center (deg)  Carrington longitude center (deg)  Event_Npixels  Area_AtDiskCenter (Mm^2)  Area_AtDiskCenterUncert  Area_Raw  Area_Uncert  AR_IntensMin (DN/s)  AR_IntensMax  AR_IntensMean  AR_IntensVar  AR_IntensSkew  AR_IntensKurt  AR_IntensTotal"

PRINTF, 1, "FRM_SpecificID	Event_StartTime	Event_C1_HelioprojCart_arcsec	Event_C2_HelioprojCart_arcsec	Event_C1Error_HelioprojCart_arcsec	Event_C2Error_HelioprojCart_arcsec Event_Stonyhurst_longitude_deg	Event_Stonyhurst_latitude_deg	Event_Carrington_longitude_deg	BoundBox_C1LL_HelioprojCart_arcsec	BoundBox_C2LL_HelioprojCart_arcsec	BoundBox_C1UR_HelioprojCart_arcsec	BoundBox_C2UR_HelioprojCart_arcsec	BoundBox_C1LL_Stonyhurst_deg	BoundBox_C2LL_Stonyhurst_deg	BoundBox_C1UR_Stonyhurst_deg	BoundBox_C2UR_Stonyhurst_deg	Event_Npixels	Area_AtDiskCenter_Mm^2	Area_AtDiskCenterUncert_Mm^2	Area_Raw_Mm^2	Area_Uncert_Mm^2  AR_IntensMin	AR_IntensMax	AR_IntensMean	AR_IntensVar	AR_IntensSkew	AR_IntensKurt	AR_IntensTotal"


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


	; We need the wcs info in the header of one of the image to transform the coordinates
; THIS IS THE CODE PROVIDED BY BENJAMIN. IT DOES NOT YIELD A CORRECT DATE IN output[3]
;	IF instrument EQ 'AIA' THEN read_sdo, files171[i] , header, /nodata ELSE header = fitshead2struct(headfits(files171[i] ))
; I REPLACED THIS BY PREVIOUS CODE FROM SPOCA_ROB.PRO: IT WORKS FINE
	header = headfits(files171[i])	

print, header

	wcs = fitshead2wcs(header)

	totalNumPixels = 0

	FOR k = 0, N_ELEMENTS(getregionstats_output) - 1 DO BEGIN

; The output of GetRegionStats is: label id colo observationdate (center.x, center.y) (boxLL.x, boxLL.y) (boxUR.x, boxUR.y) numberpixels minintensity maxintensity mean variance skewness kurtosis totalintensity (centererror.x, centererror.y) area_raw area_rawuncert area_atdiskcenter area_atdiskcenteruncert
; label (=HEK_ID) is different for every HEK event (hence for every AR instance in time), and is 1-1 with ID
; color is the real FRM_ID corresponding to one and the same AR through time. In the tracking info, the relation between those colors is given. We want the color as "ID"

		output = strsplit( getregionstats_output[k] , ' 	(),', /EXTRACT)

PRINT, output

		; We parse the output
		label           = output[0]
		id              = output[1]
		color           = LONG(output[2])


; THIS IS THE CODE PROVIDED BY BENJAMIN. IT DOES NOT YIELD A CORRECT DATE IN output[3]
;	observationdate = output[3]
; I REPLACED THIS BY PREVIOUS CODE FROM SPOCA_ROB.PRO: IT WORKS FINE
	observationdate = FXPAR(header,'DATE-OBS')
	IF !err LT 0 THEN observationdate = FXPAR(header,'DATE_OBS')
	IF !err LT 0 THEN BEGIN
		error = [ error, "ERROR : could not find DATE_OBS nor DATE-OBS keyword in file " + image171 ]
		RETURN
	ENDIF
; NEW CODE FOR THIS:
; header = headfits(image171)
;IF tag_exist(header, 'DATE_OBS') THEN BEGIN
;	current_observation_date = header.DATE_OBS
;ENDIF ELSE BEGIN
;	error = [ error, 'ERROR : could not find DATE_OBS nor DATE-OBS keyword in file ' + image171 ]
;	RETURN	
;ENDELSE				

		cartesian_x = FLOAT(output[4:8:2])
		cartesian_y = FLOAT(output[5:9:2])
		cartesian = FLTARR(2,N_ELEMENTS(cartesian_x))
		cartesian[0,*]=cartesian_x
		cartesian[1,*]=cartesian_y
		numberpixels = LONG(output[10])	; LONG(output[7]) according to B.

		totalNumPixels = totalNumPixels + numberpixels

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
			PRINT , "Cartesian coordinates for the region ", k
			PRINT, cartesian
		ENDIF
		
		; We convert the cartesian pixel coordinates into WCS
		wcs_coord = WCS_GET_COORD(wcs, cartesian)
		
		; We convert the WCS coordinates
		WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HPC', /ARCSECONDS, hpc_x, hpc_y	; helioprojective Cartesian coordinates in arcseconds
		WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HG', stony_long, stony_lat		; Stonyhurst coordinates in degrees
		WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HG', /CARRINGTON, carr_long, carr_lat	; Carrington coordinates in degrees

		IF (debug GT 0) THEN BEGIN
			PRINT , "x, y HPC coordinates for the region ", k
			PRINT, hpc_x
			PRINT, hpc_y

			PRINT , "Stonyhurst longitude and latitude for the region ", k
			PRINT, stony_long
			PRINT, stony_lat

			PRINT , "Carrington longitude and latitude for the region ", k
			PRINT, carr_long
			PRINT, carr_lat
		ENDIF


		; Write output to ASCII file: in one line, free format	

		tab = STRING(9B)

		; FRM_SpecificID
		PRINTF, 1, color, tab, $
	; AR_NOAANum, LATER!		
	; ???,		
		; Event_StartTime: previous time we ran GetRegionStats
	;	last_event_written, tab,$
		; Event_EndTime
		anytim(observationdate, /ccsds), tab, $
		; Event_Coord1  (helioprojective Cartesian x, arcsec)
		hpc_x[0], tab, $
		; Event_Coord2  (helioprojective Cartesian y, arcsec)
		hpc_y[0], tab, $
		; Event_C1Error (helioprojective Cartesian x, arcsec)
		centerx_error, tab, $
		; Event_C2Error (helioprojective Cartesian y, arcsec)
		centery_error, tab, $
		; Stonyhurst longitude center (deg)
		stony_long[0], tab, $
		; Stonyhurst latitude  center (deg) = Carrington latitude center
		stony_lat [0], tab, $
	; TO BE DONE:	calculate errors of Stonyhurst coordinates
		; Carrington longitude center (deg)
		carr_long[0], tab, $	
	; TO BE DONE:	calculate errors of Carrington coordinates
		; BoundBox_C1LL	(helioprojective Cartesian x, arcsec)
		hpc_x[1], tab, $
		; BoundBox_C2LL	(helioprojective Cartesian y, arcsec)
		hpc_y[1], tab, $
		; BoundBox_C1UR	(helioprojective Cartesian x, arcsec)
		hpc_x[2], tab, $
		; BoundBox_C2UR	(helioprojective Cartesian y, arcsec)
		hpc_y[2], tab, $
		; BoundBox_C1LL	(Stonyhurst longitude, deg)
		stony_long[1], tab, $
		; BoundBox_C2LL	(Stonyhurst latitude, deg)
		stony_lat[1], tab, $
		; BoundBox_C1UR	(Stonyhurst longitude, arcsec)
		stony_long[2], tab, $
		; BoundBox_C2UR	(Stonyhurst latitude, deg)
		stony_lat[2], tab, $
		; Event_Npixels
		numberpixels, tab, $
		; Area_AtDiskCenter
		area_atdiskcenter, tab, $
		; Area_AtDiskCenterUncert
		area_atdiskcenteruncert, tab, $
		; Area_Raw
		area_raw, tab, $
		; Area_Uncert		
		area_rawuncert, tab, $		
		; AR_IntensMin
		minintensity, tab, $
		; AR_IntensMax
		maxintensity, tab, $
		; AR_IntensMean	
		mean, tab, $
		; AR_IntensVar	
		variance, tab, $
		; AR_IntensSkew	
		skewness, tab, $
		; AR_IntensKurt	
		kurtosis, tab, $
		; AR_IntensTotal
		totalintensity

	ENDFOR		; k = 0, N_ELEMENTS(getregionstats_output) - 1


; Write total number of pixels over all AR to an ASCII file
OPENW , 2, outputDirectory + 'totalNumPixels.txt', /APPEND, WIDTH = 1000
PRINTF, 2, anytim(observationdate, /ccsds), totalNumPixels
CLOSE, 2


ENDFOR			; i=0, MIN([(N_ELEMENTS(files171) - 1), (N_ELEMENTS(files195) - 1)])

CLOSE,1


IF (debug GT 0) THEN BEGIN
	PRINT, endl, STRPAD('END OF GETREGIONSTATS', 100, fill='_')
ENDIF

Finish :
END 

