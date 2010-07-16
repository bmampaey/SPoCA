; +
; Description:
;	IDL code to call spoca_V3D and the tracking
; Author:
; 	Benjamin Mampaey
; Date:
; 	16 February 2010
; Params:
; 	image171, image195: in, required, type string, images filename of wavelength 171 and 195
;	events: out, required, type string array, see document SDO EDS API
;	write_file: in, optional, type boolean, see document SDO EDS API
;	restart: in, optional, type boolean, see document SDO EDS API
;	error: out, required, type string array, see document SDO EDS API
;	imageRejected: out, required, type boolean, see document SDO EDS API
;	status: in/out, required, type struct, see document SDO EDS API
;	runMode: in, required, type string, see document SDO EDS API
;	inputStatusFilename: in, optional, type string, see document SDO EDS API
;	outputStatusFilename: in, required, type string, see document SDO EDS API
;	numActiveEvents: out, required, type integer, see document SDO EDS API
;	outputDirectory: in, required, type string, folder where spoca can store temporary files (The modules manage the cleanup of old files) 
;	writeEventsFrequency: in, required, type integer, number of seconds between events write to the HEK
;	cCodeLocation: in, optional, type string, directory of the c executables
;	spocaArgsPreprocessing: in, optional, type string, type of image preprocessing for spoca
;	spocaArgsNumberclasses: in, optional, type string, number of classes for spoca
;	spocaArgsPrecision: in, optional, type string, precision for spoca
;	spocaArgsBinsize: in, optional, type string, bin size for spoca
;	trackingArgsDeltat: in, optional, type string, maximal time difference between 2 images for tracking
;	trackingNumberImages: in, optional, type integer, number of images to track at the same time
;	trackingOverlap: in, optional, type integer, proportion of the number of images to overlap between tracking succesive run
; -
 
; TODO :
; - Handle the imageRejected and quality when we know what is that quality keyword
; - Take care of the Clear Events case
; 


PRO SPoCA_V3D, image171=image171, image195=image195, $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = runMode, $
	inputStatusFilename = inputStatusFilename, $
	outputStatusFilename = outputStatusFilename, $
	outputDirectory = outputDirectory, $
	cCodeLocation = cCodeLocation, $
	spocaArgsPreprocessing = spocaArgsPreprocessing, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsBinsize = spocaArgsBinsize, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingNumberImages = trackingNumberImages, $
	trackingOverlap = trackingOverlap

	
	
; set debugging
debug = 1

; newline shortcut for the c++ programmer
endl=STRING(10B)

; Because we cannot declare variable in IDL
IF N_ELEMENTS(error) EQ 0 THEN BEGIN
	error = ''
ENDIF


; --------- We take care of the arguments -----------------

; We look at what is the runMode and take care of the status

SWITCH runMode OF 
	'Construct':	BEGIN
				
				; We create the color list
				numbercolors = 256
				colorlist=STRARR(numbercolors)
				seed=1978
				FOR c = 0l, numbercolors - 1 DO colorlist[c] = 'C ' + STRING(c, FORMAT='(I3)') + ' ' + STRING(RANDOMU(seed), FORMAT='(F10)') + ' ' + STRING(RANDOMU(seed), FORMAT='(F10)') + ' ' + STRING(RANDOMU(seed), FORMAT='(F10)')
				spoca_lastrun_number = 0
				last_color_assigned = 0
				status = {spoca_lastrun_number : spoca_lastrun_number, last_color_assigned : last_color_assigned, colorlist: colorlist, numbercolors : numbercolors  }
				BREAK

   			END
	'Recovery':	BEGIN
				IF FILE_TEST( inputStatusFilename , /REGULAR ) THEN BEGIN 

					RESTORE ,inputStatusFilename , VERBOSE = debug 
				
				ENDIF ELSE BEGIN 
				
					error = [ error,  "I am in recovery mode but i didn't receive my inputStatusFilename" ]
					RETURN
				
				ENDELSE
				; I don't break because now I am in normal mode
   			END 
   	
   	'Clear Events':	BEGIN
				; TODO close out events (altought I don't think we have that)
				trackingNumberImages = 0
				trackingOverlap = 0
				spoca_lastrun_number = status.spoca_lastrun_number
				last_color_assigned = status.last_color_assigned
				numbercolors = status.numbercolors
				colorlist = status.colorlist
				GOTO, Tracking
			END
			
   	'Normal':	BEGIN ; We read the status
				spoca_lastrun_number = status.spoca_lastrun_number
				last_color_assigned = status.last_color_assigned
				numbercolors = status.numbercolors
				colorlist = status.colorlist
				BREAK
   			END 

			
	ELSE:		BEGIN
				error = [ error, "I just don't know what to do with myself. runMode is " + runMode ]
				RETURN
			END   	
ENDSWITCH

IF (debug GT 0) THEN BEGIN
	PRINT, 'Status :'
	PRINT, 'spoca_lastrun_number : ' , spoca_lastrun_number
	PRINT, 'last_color_assigned : ', last_color_assigned
ENDIF

; We verify our module arguments

; We test the filenames

IF N_ELEMENTS(image171) EQ 0 THEN BEGIN 
	error = [ error, 'No image171 provided as argument']
	RETURN	
ENDIF 

IF N_ELEMENTS(image195) EQ 0 THEN BEGIN 
	error = [ error, 'No image195 provided as argument']
	RETURN	
ENDIF 

IF (~ FILE_TEST( image171, /READ, /REGULAR)) || (~ FILE_TEST( image195, /READ, /REGULAR) )  THEN BEGIN
	error = [ error, 'Cannot find images ' + image171 + ' or ' + image195 ]
	RETURN
ENDIF


IF N_ELEMENTS(outputDirectory) EQ 0 THEN outputDirectory = 'results/'
IF N_ELEMENTS(cCodeLocation) EQ 0 THEN cCodeLocation = 'bin/'

; SPoCA parameters

;spoca_bin = cCodeLocation + 'SPoCA_V3D.x'
spoca_bin = cCodeLocation + 'SPoCA2Sursegmentation.x'

IF N_ELEMENTS(spocaArgsPreprocessing) EQ 0 THEN spocaArgsPreprocessing = '3'  
IF N_ELEMENTS(spocaArgsNumberclasses) EQ 0 THEN spocaArgsNumberclasses = '4'
IF N_ELEMENTS(spocaArgsPrecision) EQ 0 THEN spocaArgsPrecision = '0.000000001'
IF N_ELEMENTS(spocaArgsBinsize) EQ 0 THEN spocaArgsBinsize = '0.01,0.01'
spoca_args_centersfile = outputDirectory + 'centers.txt'


IF (debug GT 0) THEN BEGIN
	PRINT, endl, "********END OF PARAMETERS CHECK BEGINNING OF SPOCA*******"
ENDIF


; --------- We take care of running spoca -----------------

; We initialise correctly the arguments for SPoCA_V3D

++spoca_lastrun_number

spoca_args =	' -P ' + spocaArgsPreprocessing + $
		' -C ' + spocaArgsNumberclasses + $
		' -p ' + spocaArgsPrecision + $
		;' -z ' + spocaArgsBinsize + $
		' -B ' + spoca_args_centersfile + $
		' -O ' + outputDirectory + STRING(spoca_lastrun_number, FORMAT='(I010)') + $
		' ' + image171 + ' ' + image195

IF (debug GT 0) THEN BEGIN

	PRINT, 'About to run : ' , spoca_bin + spoca_args
	
ENDIF

; We call SPoCA with the correct arguments

SPAWN, spoca_bin + spoca_args, spoca_output, spoca_errors, EXIT_STATUS=spoca_exit 

; In case of error
IF (spoca_exit NE 0) THEN BEGIN

	error = [ error, 'Error executing '+ spoca_bin + spoca_args ]
	error = [ error, spoca_errors ]
	; TODO Should we cleanup  ???
	
	IF (debug GT 0) THEN BEGIN
		PRINT , "SPoCA exited with error : ", spoca_exit, endl, spoca_errors
	ENDIF
	
ENDIF



IF (debug GT 0) THEN BEGIN
	PRINT, endl, "********END OF SPOCA BEGINNING OF TRACKING*******"
ENDIF


; --------- We take care of the tracking -----------------

Tracking :

; Tracking parameters

tracking_bin = cCodeLocation + 'Tracking_V3D.x'
IF N_ELEMENTS(trackingArgsDeltat) EQ 0 THEN trackingArgsDeltat = '21600' ; It is in seconds
IF N_ELEMENTS(trackingNumberImages) EQ 0 THEN trackingNumberImages = 9
IF N_ELEMENTS(trackingOverlap) EQ 0 THEN trackingOverlap = 3

ARmaps = FILE_SEARCH(outputDirectory, '*ARmap.tracking.fits', /TEST_READ, /TEST_REGULAR , /TEST_WRITE  ) 

IF (debug GT 0) THEN BEGIN
	PRINT , "Found files : ", endl + ARmaps
ENDIF

IF (N_ELEMENTS(ARmaps) LT trackingNumberImages) THEN BEGIN
	IF (debug GT 0) THEN BEGIN
		PRINT, 'Not enough files to do tracking, going to Finish'
	ENDIF
	GOTO, Finish
ENDIF
		
; We initialise correctly the arguments for Tracking_HEK

tracking_args =	' -n ' + STRING(last_color_assigned, FORMAT = '(I)') + $
		' -d ' + trackingArgsDeltat + $
		' -D ' + STRING(trackingOverlap, FORMAT = '(I)') + $
		' ' + STRJOIN( ARmaps , ' ', /SINGLE)
	
IF (debug GT 0) THEN BEGIN
	PRINT, 'About to run : ' , tracking_bin + tracking_args 
ENDIF

SPAWN, tracking_bin + tracking_args , tracking_output, tracking_errors, EXIT_STATUS=tracking_exit 

IF (tracking_exit NE 0) THEN BEGIN

	error = [ error, 'Error executing ', tracking_bin + tracking_args ]
	error = [ error, tracking_errors ]
	; What do we do in case of error ?
	
	IF (debug GT 0) THEN BEGIN
		PRINT , "Tracking exited with error : ", tracking_exit, endl, tracking_errors
	ENDIF
	
ENDIF

; As output of Tracking we receive the number of AR intra limb and their obs_date 
IF (debug GT 0) THEN BEGIN
	PRINT, 'Tracking Output is :', endl + tracking_output
ENDIF


; We check that output is not null
IF (N_ELEMENTS(tracking_output) LT 1 || STRLEN(tracking_output[0]) LE 1 ) THEN BEGIN

	error = [ error, 'Error No output from Tracking']	
	RETURN
	
ENDIF 

; The first output of Tracking is the last_color_assigned
output = strsplit( tracking_output[0] , ':', /EXTRACT) 
last_color_assigned = LONG(output[1])

; The files to write the results

monochromeresults = 50
rainbowresults = 51
OPENW , monochromeresults, 'monochromeresults.txt', /APPEND

OPENW, rainbowresults, 'rainbowresults.txt', /APPEND

; The rest of the tracking output are the regions

FOR k = 1, N_ELEMENTS(tracking_output) - 1 DO BEGIN 

	PRINT, "Caring for line ", tracking_output[k]

	IF STRMATCH( tracking_output[k], "*IMAGEEND*" , /FOLD_CASE ) THEN BEGIN
		PRINT, "Found IMAGEEND"
		PRINTF, monochromeresults, blockheader, endl, 'C 0 0.0 0.0 1.0', endl, STRJOIN(monochromeblock, endl, /SINGLE), endl, 'IMAGEEND'
		PRINTF,rainbowresults, blockheader, endl, STRJOIN(colorlist, endl, /SINGLE), endl,  STRJOIN(rainbowblock, endl, /SINGLE), endl, 'IMAGEEND'
		
		CONTINUE
	ENDIF

	IF STRMATCH( tracking_output[k], "*IMAGE*" , /FOLD_CASE ) THEN BEGIN
		PRINT, "Found IMAGE"
		; We need the header of the image to transform the coordinates
		output = strsplit( tracking_output[k] , ' 	(),', /EXTRACT)
		image = output[1]
		PRINT, "Image ", image
		header = headfits(image)
		wcs = fitshead2wcs(header)
		kdate_obs = FXPAR(header,'DATE-OBS')
		IF !err LT 0 THEN kdate_obs = FXPAR(header,'DATE_OBS')
		IF !err LT 0 THEN PRINT, "ERROR : could not find DATE_OBS nor DATE-OBS keyword in file ", image
		output = STRSPLIT( kdate_obs , '-Tt:.Zz', /EXTRACT) 
		date_obs = STRJOIN( output[0:4] , '', /SINGLE ) 
		
		kinstrument = FXPAR(header,'INSTRUME')
		IF !err LT 0 THEN PRINT, "ERROR : could not find INSTRUME keyword in file ", image
		IF STRMATCH( kinstrument, "*EIT*" , /FOLD_CASE ) THEN BEGIN
			observatory = '1'
			instrument = '2'
		ENDIF
		
		IF STRMATCH( kinstrument, "*EUVI*" , /FOLD_CASE ) THEN BEGIN
		
			instrument = '3'
			kobservatory = FXPAR(header,'OBSRVTRY')
			IF !err LT 0 THEN PRINT, "ERROR : could not find OBSRVTRY keyword in file ", image

			IF STRMATCH( kobservatory, "*STEREO_A*" , /FOLD_CASE ) THEN BEGIN
				observatory = '2'
			ENDIF
			
			IF STRMATCH( kobservatory, "*STEREO_B*" , /FOLD_CASE ) THEN BEGIN
				observatory = '3'
			ENDIF
		ENDIF
		
		imagetype = '6' ; i.e. 171 
		algo = 'SPOCA'
		monochromeblock=STRARR(1)
		rainbowblock=STRARR(1)
		blockheader="IMAGE " + observatory + instrument + imagetype + date_obs + ' ' + algo 
		print, blockheader
		  
		CONTINUE
	ENDIF 

	PRINT, "Found REGION"

	; The output of a region is (center.x, center.y) (boxLL.x, boxLL.y) (boxUR.x, boxUR.y) id numberpixels label date_obs color
	output = strsplit( tracking_output[k] , ' 	(),', /EXTRACT) 
	; We parse the output
	cartesian_x = FLOAT(output[0:4:2])
	cartesian_y = FLOAT(output[1:5:2])
	cartesian = FLTARR(2,N_ELEMENTS(cartesian_x))
	cartesian[0,*]=cartesian_x
	cartesian[1,*]=cartesian_y
	feature_tag = output[10]
	
	IF (debug GT 0) THEN BEGIN
		PRINT , "cartesians coordinates for the region ", k
		PRINT, cartesian
	ENDIF
		
	; We convert the cartesian pixel coodinates into WCS
        wcs_coord = WCS_GET_COORD(wcs, cartesian)
        
	; We convert the WCS coodinates into stonyhurst heliographics 
	WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HG', hgs_long, hgs_lat
	
	IF (debug GT 0) THEN BEGIN
		PRINT , "x, y, z HPC coordinates for the region ", k
		PRINT, hgs_long
		PRINT, hgs_lat
	ENDIF
	IF FINITE(hgs_lat[0]) && FINITE(hgs_long[0]) THEN BEGIN
		color = '0'
		region = 'AR' + ' ' + color + ' ' + STRING(hgs_lat[0]) + ' ' + STRING(hgs_long[0]) + ' ' +  feature_tag
		monochromeblock = [monochromeblock, region]
		PRINT, 'monochrome ', region
		
		color = STRING(LONG(feature_tag) MOD numbercolors, FORMAT='(I3)')
		region = 'AR' + ' ' + color + ' ' + STRING(hgs_lat[0]) + ' ' + STRING(hgs_long[0]) + ' ' +  feature_tag
		rainbowblock = [rainbowblock, region]
		PRINT, 'rainbow ', region
	ENDIF

ENDFOR 


CLOSE, monochromeresults, rainbowresults

Finish :	; Label for the case not enough images were present for the tracking, or if we do not write 

; --------- We finish up -----------------

; We cleanup old files

IF (N_ELEMENTS(ARmaps) GE trackingNumberImages) THEN BEGIN

	number_of_files_to_delete = N_ELEMENTS(ARmaps) - trackingOverlap
	IF (number_of_files_to_delete GT 0) THEN BEGIN

		files_to_delete = ARmaps[0:number_of_files_to_delete-1]
		
		IF (debug GT 0) THEN BEGIN
			PRINT , "Deleting files : ", endl + files_to_delete
		ENDIF
		
		FILE_DELETE, files_to_delete , /ALLOW_NONEXISTENT , /NOEXPAND_PATH , VERBOSE = debug

	ENDIF
ENDIF



; This may need to change
imageRejected = 0

; We save the variables for next run

status.spoca_lastrun_number = spoca_lastrun_number
status.numbercolors = numbercolors
status.colorlist = colorlist
status.last_color_assigned = last_color_assigned

SAVE, status , DESCRIPTION='Spoca last run status variable at ' + SYSTIME() , FILENAME=outputStatusFilename, VERBOSE = debug
 
 
END 

