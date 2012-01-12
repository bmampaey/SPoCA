; +
; Description:
;	IDL code to test the quality of a file
; Authors:
; 	Paolo Grigis, Ryan Timmons, Benjamin Mampaey
; Date:
; 	9 November 2010
; Params:
; 	header: in, required, struct containing the keywords of a fits file
;	imageRejected: out, required, flag to tell if the image is to be rejected
;	rejectionString: out, optional, type string, reason for the rejection


PRO checkQuality, header, imageRejected, rejectionString

;Actual code for checking the headers, standardized with code borrowed from Ryan/Paolo's work on the flares

imageRejected = 0

;reject open filters
IF tag_exist(header, 'WAVE_STR') && strmatch(header.wave_str, 'open', /FOLD_CASE) EQ 1 THEN BEGIN
	rejectionString = 'Open filter (WAVE_STR =~ open)'
	imageRejected = 1
	RETURN
ENDIF

;check for darks or non -light images
IF tag_exist(header, 'IMG_TYPE') && header.img_type NE 'LIGHT' THEN BEGIN 
	rejectionString = 'Dark image (IMG_TYPE != LIGHT)'
	imageRejected = 1
	RETURN
ENDIF

;New eclipse flag
IF tag_exist (header, 'aiagp6') && header.aiagp6 NE 0 THEN BEGIN
	rejectionString = 'Eclipse (AIAGP6 != 0)'
	imageRejected = 1
	RETURN
ENDIF


IF tag_exist(header, "exptime") && header.exptime LT 1.5 THEN BEGIN
	rejectionString = 'Exposure time too short (exptime <= 1.5)'
	imageRejected = 1
	RETURN
ENDIF


IF tag_exist(header, "aiftsid") && header.aiftsid GE 49152 THEN BEGIN
	rejectionString = 'Calibration image (aiftsid >= 49152)'
	imageRejected = 1
	RETURN
ENDIF


IF tag_exist (header, "percentd") && header.percentd LT 99.9999 THEN BEGIN
	rejectionString = 'Missing pixels (percentd < 99.9999)'
	imageRejected = 1
	RETURN
ENDIF

;IF tag_exist (header, "aectype") && header.aectype LT 2 then begin
;	rejectionString = 'Passing on non-AEC image'
;	imageRejected = 1
;	return
;endif

; Quality keyword in AIA - details TBD
; Need to understand in more details what "quality" means as a flag
; Now is e.g. set to 131072=2^17 just means ISS loops is open
; Seems to be OK for now
; Eventually we want to reject everything but 0 - but for now just reject based on a list of forbidden bits

IF tag_exist(header,'QUALITY') THEN BEGIN 

	;create an array of number such that the j-th elementh as bit j set to 1 and all others set to 0
	;i.e. 1,2,4,8,...,2^J,...
	BitArray=2UL^ulindgen(32)
	BitSet=(header.quality AND BitArray) NE 0
	
	; If any of these bits is set - reject the image
	ForbiddenBits=[0,1,2,3,4,12,13,14,15,16,17,18,20,21,31] 
	;RPT - added bits for ISS loop (17), ACS_MODE not SCIENCE (12)
	;RPT - 9/25/10 - bits 20, 21, below from Rock's new def file
	;	20	(AIFCPS <= -20 or AIFCPS >= 100)	;	AIA focus out of range 
	;	21	AIAGP6 != 0					;	AIA register flag


	IF total(BitSet[ForbiddenBits]) GT 0 THEN BEGIN 
		rejectionString = 'Bad quality1 ('+STRTRIM(STRING(header.quality), 2)+')'
		imageRejected = 1
		RETURN
	ENDIF 

ENDIF 

IF tag_exist(header,'QUALLEV0') THEN BEGIN 

	;create an array of number such that the j-th elementh as bit j set to 1 and all others set to 0
	;i.e. 1,2,4,8,...,2^J,...
	BitArray=2UL^ulindgen(32)
	BitSet=(header.quallev0 AND BitArray) NE 0
	
	; If any of these bits is set - reject the image
	ForbiddenBits=[0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23,24,25,26,27,28] 
	;RPT - added bits for ISS loop (17), ACS_MODE not SCIENCE (12)


	IF total(BitSet[ForbiddenBits]) GT 0 THEN BEGIN 
		rejectionString = 'Bad quality0 ('+STRTRIM(STRING(header.quallev0), 2)+')'
		imageRejected = 1
		RETURN
	ENDIF 

ENDIF 

END; of checkQuality




; +
; Description:
;	IDL code to call spoca and the tracking
; Author:
; 	Benjamin Mampaey
; Date:
; 	16 February 2010
; Params:
; 	image195: in, required, type string, image filename of wavelength 193/195
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
;	verbose: in, optional, type integer, verbose level of the module (0 is off)
;	saveFiles: in, optional, type integer, save files deleted from outputDirectory to saveDirectory below (0: none, 1: last one, 2: all)
;	saveDirectory: in, optional, type string, folder to use to save images corresponding to events for debugging
;	writeEventsFrequency: in, required, type integer, number of seconds between events write to the HEK
;	cCodeLocation: in, optional, type string, directory of the c executables
;	instrument: in, optional, type string, instrument that took the images (AIA,EIT,EUVI)
;	spocaArgs: in, required, type string array, options for running the classification of spoca
;	chaincodeArgs: in, required, type string array, options for specifying the numbers and precision of the chain codes
;	trackingArgs: in, required, type string array, options for running the tracking of spoca
;	trackingOverlap: in, optional, type integer, number of images to overlap between succesive tracking run
;	minLifeTime: in, optional, type integer, Minimal time for a CH to be alife to be exported
;	minDeathTime: in, optional, type integer, Minimal time for a CH to be dead to be devinitively supressed
; - 


PRO SPoCA_CH, image195=image195, $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = runMode, $
	inputStatusFilename = inputStatusFilename, $
	outputStatusFilename = outputStatusFilename, $
	numActiveEvents = numActiveEvents, $
	outputDirectory = outputDirectory, $
	verbose = verbose, $
	saveFiles = saveFiles, $
	saveDirectory = saveDirectory, $
	writeEventsFrequency = writeEventsFrequency, $
	cCodeLocation = cCodeLocation, $
	instrument = instrument, $
	spocaArgs = spocaArgs, $
	chaincodeArgs = chaincodeArgs, $
	trackingArgs = trackingArgs, $
	trackingOverlap = trackingOverlap, $
	minLifeTime = minLifeTime, $
	minDeathTime = minDeathTime

	
; Version number
ModuleVersionNumber = 1.0

; When fits files are compressed we read the HDU 1, otherwise the 0
compressed = 0

; Global constant
regionStatsTableHdu = 'AIA_193_CoronalHoleStats'

; Newline shortcut for the c++ programmer
endl=STRING(10B)

; We reset the error variable
error = ''

; We set the vebosity of the module
IF N_ELEMENTS(verbose) EQ 0 THEN verbose = 0

; --------- We take care of the arguments -----------------

IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF PARAMETERS CHECK', 100, fill='_')
ENDIF


; We look at what is the runMode and take care of the status

SWITCH runMode OF 
	'Construct':	BEGIN
				IF (verbose GT 0) THEN BEGIN
					PRINT, "runMode Construct called"
				ENDIF
				; We will set the start of the first event later
				last_event_written_date = 0
				; We need a dummy value because IDL does not have empty arrays 
				meta_events = REPLICATE({meta_event, color:0, first_seen:!VALUES.D_INFINITY, last_seen:!VALUES.D_INFINITY, last_ivorn:'DUMMY VALUE SHOULD NEVER APPEAR IN EVENTS RELATIONS'}, 1)
				saved_events =  REPLICATE({event_info, color:0, ivorn:'DUMMY VALUE SHOULD NEVER APPEAR', info:'DUMMY VALUE SHOULD NEVER APPEAR IN EXPORTED EVENTS'}, 1)
				status = {last_event_written_date : last_event_written_date, meta_events : meta_events, saved_events : saved_events}
				
				; For safety we cleanup the outputDirectory first
				AllFiles = FILE_SEARCH(outputDirectory, '*', /TEST_READ, /TEST_REGULAR , /TEST_WRITE  )
				IF N_ELEMENTS(AllFiles) GT 0 AND STRLEN(AllFiles[0]) GT 0 THEN BEGIN
					IF (verbose GT 0) THEN BEGIN
						PRINT , "Deleting all files from outputDirectory : ", endl + AllFiles
					ENDIF
					FILE_DELETE, AllFiles , /ALLOW_NONEXISTENT , /NOEXPAND_PATH , VERBOSE = verbose 
				ENDIF
				BREAK

			END
	'Recovery':	BEGIN
				IF (verbose GT 0) THEN BEGIN
					PRINT, "runMode Recovery called"
				ENDIF
				IF FILE_TEST( inputStatusFilename , /REGULAR ) THEN BEGIN 

					RESTORE ,inputStatusFilename , VERBOSE = verbose 
				
				ENDIF ELSE BEGIN 
				
					error = [ error,  "I am in recovery mode but i didn't receive my inputStatusFilename" ]
					RETURN
				
				ENDELSE
				; I don't break because now I am in normal mode
			END 
	'Normal':	BEGIN ; We read the status

				last_event_written_date = status.last_event_written_date
				meta_events = status.meta_events
				saved_events = status.saved_events
				BREAK
			END 

	'Clear Events':	BEGIN
				; TODO close out events (altought I don't think we have that)
				IF (verbose GT 0) THEN BEGIN
					PRINT, "runMode Clear Events called"
				ENDIF

				AllFiles = FILE_SEARCH(outputDirectory, '*', /TEST_READ, /TEST_REGULAR , /TEST_WRITE  )
				IF N_ELEMENTS(AllFiles) GT 0 AND STRLEN(AllFiles[0]) GT 0 THEN BEGIN
					IF (verbose GT 0) THEN BEGIN
						PRINT , "Deleting all files from outputDirectory : ", endl + AllFiles
					ENDIF
					FILE_DELETE, AllFiles , /ALLOW_NONEXISTENT , /NOEXPAND_PATH , VERBOSE = verbose 
				ENDIF
				RETURN
			END
			
	ELSE:		BEGIN
				IF (verbose GT 0) THEN BEGIN
					PRINT, "runMode unknown called"
				ENDIF

				error = [ error, "I just don't know what to do with myself. runMode is " + runMode ]
				RETURN
			END   	
ENDSWITCH

IF (verbose GT 0) THEN BEGIN
	PRINT, 'Status :'
	PRINT, 'last_event_written_date : ', anytim(last_event_written_date, /ccsds)
	
	PRINT, 'meta_events : '
	FOR e = 1, N_ELEMENTS(meta_events) - 1 DO BEGIN
		PRINT, meta_events[e].color, " ", anytim(meta_events[e].first_seen, /ccsds), " ", anytim(meta_events[e].last_seen, /ccsds), " ", meta_events[e].last_ivorn
	ENDFOR
	PRINT, 'saved_events : '
	FOR e = 1, N_ELEMENTS(saved_events) - 1 DO BEGIN
		PRINT, saved_events[e].color, " ", saved_events[e].ivorn
		IF (verbose GT 1) THEN PRINT, saved_events[e].info
	ENDFOR
ENDIF

; We verify our module arguments

; We test the filenames

IF N_ELEMENTS(image195) EQ 0 THEN BEGIN 
	error = [ error, 'No image195 provided as argument']
	RETURN	
ENDIF 

IF ~ FILE_TEST( image195, /READ, /REGULAR) THEN BEGIN
	error = [ error, 'Cannot find image ' + image195 ]
	RETURN
ENDIF


IF N_ELEMENTS(writeEventsFrequency) EQ 0 THEN writeEventsFrequency = 4 * 3600
IF N_ELEMENTS(cCodeLocation) EQ 0 THEN cCodeLocation = 'bin/'
IF N_ELEMENTS(instrument) EQ 0 THEN instrument = 'AIA'
IF N_ELEMENTS(minLifeTime) EQ 0 THEN minLifeTime = 3 * 24 * 3600.0D
IF N_ELEMENTS(minDeathTime) EQ 0 THEN minDeathTime = 6 * 3600.0D

IF N_ELEMENTS(outputDirectory) EQ 0 THEN outputDirectory = 'results/'

IF ~ FILE_TEST(outputDirectory, /DIRECTORY, /WRITE) THEN BEGIN
	error = [ error, outputDirectory + ' is not a writable directory' ]
	IF (verbose GT 0) THEN BEGIN
		PRINT , outputDirectory + ' is not a writable directory'
	ENDIF
	RETURN
ENDIF

IF N_ELEMENTS(saveFiles) EQ 0 THEN saveFiles = 0
IF N_ELEMENTS(saveDirectory) EQ 0 THEN saveDirectory = 'save/'

IF saveFiles > 0 && ~ FILE_TEST(saveDirectory, /DIRECTORY, /WRITE) THEN BEGIN
	error = [ error, saveDirectory + ' is not a writable directory' ]
	IF (verbose GT 0) THEN BEGIN
		PRINT , saveDirectory + ' is not a writable directory'
	ENDIF
	RETURN
ENDIF

; SPoCA parameters

spoca_bin = cCodeLocation + 'classification.x'

IF ~ FILE_TEST( spoca_bin, /EXECUTABLE)  THEN BEGIN
	error = [ error, 'Cannot find executable ' + spoca_bin ]
	IF (verbose GT 0) THEN BEGIN
		PRINT , 'Cannot find executable ' + spoca_bin
	ENDIF
	RETURN
ENDIF

IF N_ELEMENTS(spocaArgs) EQ 0 THEN BEGIN
	spocaAgs = ['--preprocessingSteps', 'DivExpTime,ALC,TakeSqrt', $
			'--classifierType', 'HFCM', $
			'--numberClasses', '4', $
			'--precision', '0.0015', $
			'--radiusratio', '1.2', $
			'--binSize', '0.01', $
			'--segmentation', 'max', $
			'--numberPreviousCenters', '10', $
			'--intensitiesStatsPreprocessing', 'NAR,DivExpTime', $
			'--intensitiesStatsRadiusRatio', '0.95', $
			'--maps', 'C']
ENDIF

; ChainCodes parameters
IF N_ELEMENTS(chaincodeMaxPoints) EQ 0 THEN BEGIN
	chaincodeArgs = ['--chaincodeMaxPoints', '100', $
			'--chaincodeMaxDeviation','10' ]
ENDIF

spoca_centersfile = outputDirectory + 'centers.txt'


; Tracking parameters

tracking_bin = cCodeLocation + 'tracking.x'

IF ~ FILE_TEST( tracking_bin, /EXECUTABLE)  THEN BEGIN
	error = [ error, 'Cannot find executable ' + tracking_bin ]
	IF (verbose GT 0) THEN BEGIN
		PRINT , 'Cannot find executable ' + tracking_bin
	ENDIF
	RETURN
ENDIF

IF N_ELEMENTS(trackingArgs) EQ 0 THEN BEGIN
	trackingArgs = ['--max_delta_t', '14400'] ; == 4h
ENDIF

IF N_ELEMENTS(trackingOverlap) EQ 0 THEN trackingOverlap = 2


; We verify the quality of the images

imageRejected = 0

IF instrument EQ 'AIA' THEN BEGIN
	read_sdo, image195, header195, /nodata
ENDIF ELSE BEGIN
	header195 = fitshead2struct(headfits(image195, EXTEN=compressed))
ENDELSE

checkQuality, header195, imageRejected, rejectionString

IF imageRejected THEN BEGIN
	error = [ error, 'Image ' + image195 + 'rejected for :' + rejectionString ]
	IF (verbose GT 0) THEN BEGIN
		PRINT ,  'Image ' + image195 + 'rejected for :' + rejectionString
	ENDIF
	RETURN
ENDIF


IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('END OF PARAMETERS CHECK', 100, fill='_')
ENDIF


; --------- We take care of running spoca -----------------

IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF SPOCA', 100, fill='_')
ENDIF


; We initialise correctly the arguments for SPoCA

spoca_args = [spocaArgs, $
			chaincodeArgs, $
			'-I', instrument, $
			'-B', spoca_centersfile, $
			'-O', outputDirectory, $
			image195 ]

IF (verbose GT 0) THEN BEGIN

	PRINT, 'About to run : ' , STRJOIN([spoca_bin , spoca_args] , ' ', /SINGLE ) 
	time_before_run = SYSTIME(/SECONDS) 
	
ENDIF

; We call SPoCA with the correct arguments

SPAWN, [spoca_bin , spoca_args], spoca_output, spoca_errors, /NOSHELL, EXIT_STATUS=spoca_exit 

IF (verbose GT 0) THEN BEGIN
	PRINT, 'run time (seconds): ' , SYSTIME(/SECONDS) - time_before_run
	PRINT, 'Classification Output is :', endl + spoca_output
	PRINT, 'Classification Error is :', endl + spoca_errors
ENDIF

; In case of error
IF (spoca_exit NE 0) THEN BEGIN

	error = [ error, 'Error executing '+  STRJOIN( [spoca_bin , spoca_args] , ' ', /SINGLE )  ]
	error = [ error, spoca_errors ]
	
	imageRejected = 1
	RETURN
	
	IF (verbose GT 0) THEN BEGIN
		PRINT , "SPoCA exited with error : ", spoca_exit, endl, spoca_errors
	ENDIF
	
ENDIF

IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('END OF SPOCA', 100, fill='_')
ENDIF

; --------- We check IF it is time to write some events to the hek -----------------

; We get the observation date of the image195

IF instrument EQ 'AIA' && tag_exist(header195, 'T_OBS') THEN BEGIN
	current_observation_date = anytim(header195.T_OBS, /sec)
ENDIF ELSE IF tag_exist(header195, 'DATE_OBS') THEN BEGIN
	current_observation_date = anytim(header195.DATE_OBS, /sec)
ENDIF ELSE BEGIN
	error = [ error, 'ERROR : could not find T_OBS nor DATE_OBS keyword in file ' + image195 ]
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'ERROR : could not find T_OBS nor DATE_OBS keyword in file ' + image195
	ENDIF
	imageRejected = 1
	RETURN	
ENDELSE


IF (verbose GT 0) THEN BEGIN
	PRINT, image195, " observation date is ", anytim(current_observation_date, /ccsds)
ENDIF

; If it is the first time we run SPoCA (runMode == Construct) we set the start of the first event to the observation date of the first image
IF last_event_written_date EQ 0 THEN BEGIN
	last_event_written_date = current_observation_date
ENDIF

events_write_deltat = current_observation_date - last_event_written_date

IF (verbose GT 0) THEN BEGIN
	PRINT,  "last_event_written_date : ", anytim(last_event_written_date, /ccsds)
	PRINT,  "current_observation_date : ", anytim(current_observation_date, /ccsds)
	PRINT,  STRING(events_write_deltat, FORMAT='(I20)') + ' seconds elapsed between current_observation_date and last_event_written_date'
ENDIF


IF events_write_deltat LT writeEventsFrequency THEN BEGIN
	
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'Not running Tracking yet'
	ENDIF
	GOTO, Finish
	
ENDIF ELSE BEGIN

	IF (verbose GT 0) THEN BEGIN
		PRINT, 'Running Tracking'
	ENDIF

ENDELSE

; --------- We take care of the tracking -----------------

IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF TRACKING', 100, fill='_')
ENDIF


CHmaps = FILE_SEARCH(outputDirectory, '*CHMap.fits', /TEST_READ, /TEST_REGULAR , /TEST_WRITE  ) ; FILE_SEARCH sort the filenames

IF (verbose GT 0) THEN BEGIN
	PRINT , "Found files : ", endl + CHmaps
ENDIF

; This has been changed, we can run tracking anytime now
;IF (N_ELEMENTS(CHmaps) LT trackingNumberImages) THEN BEGIN
;	IF (verbose GT 0) THEN BEGIN
;		PRINT, 'Not enough files to do tracking, going to Finish'
;	ENDIF
;	GOTO, Finish
;ENDIF


IF (N_ELEMENTS(CHmaps) EQ 0) THEN BEGIN
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'No maps to do tracking, going to Finish'
	ENDIF
	GOTO, Finish
ENDIF
		
; We initialise correctly the arguments for Tracking_HEK

tracking_args =	[trackingArgs, $
				'-o', STRING(trackingOverlap, FORMAT = '(I)'), $
				CHmaps ]

IF (verbose GT 0) THEN BEGIN 
	tracking_args = [tracking_args, '-A']	; This tells that all maps must be recolored
ENDIF
	

IF (verbose GT 0) THEN BEGIN
	PRINT, 'About to run : ', STRJOIN( [tracking_bin , tracking_args] , ' ', /SINGLE )
	time_before_run = SYSTIME(/SECONDS) 
ENDIF

SPAWN, [tracking_bin , tracking_args] , tracking_output, tracking_errors, /NOSHELL, EXIT_STATUS=tracking_exit 

IF (verbose GT 0) THEN BEGIN

	PRINT, 'run time (seconds): ' , SYSTIME(/SECONDS) - time_before_run
	PRINT, 'Tracking Output is :', endl + tracking_output
	PRINT, 'Tracking Error is :', endl + tracking_errors
ENDIF

IF (tracking_exit NE 0) THEN BEGIN

	error = [ error, 'Error executing ' + STRJOIN( [tracking_bin , tracking_args] , ' ', /SINGLE ) ]
	error = [ error, tracking_errors ]
	; What do we do in case of error ?
	
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Tracking exited with error : ", tracking_exit, endl, tracking_errors
	ENDIF
	; We will not write events
	GOTO, Cleanup
	
ENDIF


IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('END OF TRACKING', 100, fill='_')
ENDIF

; --------- We write the events -----------------

IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF WRITING EVENTS', 100, fill='_')
ENDIF

; We write the events from the last CHmap
last_map = CHmaps[N_ELEMENTS(CHmaps) - 1]

; We read the table of Regions
region_table = MRDFITS(last_map , "Regions", region_table_header, extnum=extnum, status=status) 

; If the table of region is empty, MRDFITS return 0 
IF size(region_table, /tn) NE "STRUCT" THEN BEGIN
	; Even IF there is no event to write, it was time to write them
	last_event_written_date = current_observation_date
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'No event, going to Cleanup'
	ENDIF
	GOTO, Cleanup
ENDIF 

region_table_header = fitshead2struct(region_table_header)
number_events = N_ELEMENTS(region_table)

; We read the table of Coronal Hole stats
region_stats_table = MRDFITS(last_map, regionStatsTableHdu, region_stats_table_header, extnum=extnum, status=status) 
region_stats_table_header = fitshead2struct(region_stats_table_header)

; We read the table of Tracking Relations
relation_table = MRDFITS(last_map , 'TrackingRelations', relation_table_header, extnum=extnum, status=status)
 
IF size(relation_table, /tn) NE "STRUCT" THEN BEGIN

	number_relations = 0
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'No event relations found'
	ENDIF

ENDIF ELSE BEGIN

	number_relations = N_ELEMENTS(relation_table)

ENDELSE

; We read the table of ChainCodes
chaincode_table = MRDFITS(last_map , 'ChainCodes', chaincode_table_header, extnum=extnum, status=status) 

IF size(chaincode_table, /tn) NE "STRUCT" THEN BEGIN

	write_chaincode = 0
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'No chaincodes found'
	ENDIF

ENDIF ELSE BEGIN

	write_chaincode = 1
	chaincode_columns = TAG_NAMES(chaincode_table)

ENDELSE



; We need the wcs info in the header of the image to transform the coordinates
last_map_header = fitshead2struct(HEADFITS(last_map, EXTEN=1))
wcs = fitshead2wcs(last_map_header)

; We predefine some common values for the event
FRM_ParamSet = 'image195 : calibrated image 193/195 A' 
FRM_ParamSet += '; spocaPreprocessing='       + STRTRIM(STRING(last_map_header.CPREPROC), 2) 
FRM_ParamSet += '; spocaClassifierType='      + STRTRIM(STRING(last_map_header.CLASTYPE), 2) 
FRM_ParamSet += '; spocaNumberclasses='       + STRING(last_map_header.CNBRCLAS,FORMAT='(I0)') 
FRM_ParamSet += '; spocaChannels='            + STRTRIM(STRING(last_map_header.CHANNELS), 2) 
FRM_ParamSet += '; spocaPrecision='           + STRTRIM(STRING(last_map_header.CPRECIS), 2) 
FRM_ParamSet += '; spocaRadiusRatio='         + STRING(last_map_header.CRADRATI , FORMAT='(F0.2)') 
FRM_ParamSet += '; spocaBinsize='             + STRTRIM(STRING(last_map_header.CBINSIZE), 2) 
FRM_ParamSet += '; spocaSegmentationType='    + STRTRIM(STRING(last_map_header.SEGMTYPE), 2) 
FRM_ParamSet += '; spocaVersion='             + STRING(last_map_header.CVERSION, FORMAT='(F0.2)') 
FRM_ParamSet += '; intensitiesStatsPreprocessing=' + STRTRIM(STRING(last_map_header.RPREPROC), 2) 
FRM_ParamSet += '; intensitiesStatsRadiusRatio='   + STRING(last_map_header.RRADRATI, FORMAT='(F0.2)') 
FRM_ParamSet += '; trackingDeltat='           + STRING(region_table_header.TMAXDELT, FORMAT='(I0)') 
FRM_ParamSet += '; trackingOverlap='          + STRING(region_table_header.TOVERLAP, FORMAT='(I0)') 
FRM_ParamSet += '; trackingNumberImages='     + STRING(region_table_header.TNBRIMG, FORMAT='(I0)') 
FRM_ParamSet += '; minLifeTime='              + STRING(minLifeTime, FORMAT='(I0)') 
FRM_ParamSet += '; minDeathTime='             + STRING(minDeathTime, FORMAT='(I0)') 

center_keywords = WHERE(STRPOS(tag_names(last_map_header), 'CLSCTR') EQ 0)

FRM_ParamSet += '; spocaCenters='              + STRTRIM(STRING(last_map_header.(center_keywords[0])), 2)

FOR c = 1, N_ELEMENTS(center_keywords) - 1 DO BEGIN
	FRM_ParamSet += ','                       + STRTRIM(STRING(last_map_header.(center_keywords[c])), 2)
ENDFOR

FRM_DateRun = anytim(sys2ut(), /ccsds)
FRM_SpecificID_Prefix =  'SPoCA_v' + STRING(ModuleVersionNumber, FORMAT='(F0.1)') + '_CH_' 

; For the events refernces to always link to the past, we need to make a copy of met_events
past_meta_events = meta_events


FOR k = 0, number_events - 1 DO BEGIN 
	
	; We get the indice of the region stats with the same id than region
	idx = WHERE(region_stats_table.ID EQ region_table[k].ID, exists)
	IF exists EQ 0 THEN CONTINUE ELSE idx = idx[0]
	
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'Index of region stats of region id ', region_table[k].ID, ' is ', idx, ' with region stats id ', region_stats_table[idx].ID
	ENDIF
	
	; We get the color of the region
	color = region_table[k].TRACKED_COLOR
	
	; We convert the cartesian pixel coodinates into WCS
	cartesian_x = FLOAT([region_stats_table[idx].XCENTER,region_table[k].XBOXMIN, region_table[k].XBOXMAX])
	cartesian_y = FLOAT([region_stats_table[idx].YCENTER,region_table[k].YBOXMIN, region_table[k].YBOXMAX])
	cartesian = FLTARR(2,N_ELEMENTS(cartesian_x))
	cartesian[0,*]=cartesian_x
	cartesian[1,*]=cartesian_y
	IF (verbose GT 1) THEN BEGIN
		PRINT , "cartesians coordinates for the region ", k
		PRINT, cartesian
	ENDIF
	wcs_coord = WCS_GET_COORD(wcs, cartesian)
	
	; We convert the WCS coodinates into helioprojective cartesian
	WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HPC', /ARCSECONDS, hpc_x, hpc_y
	
	IF (verbose GT 1) THEN BEGIN
		PRINT , "x, y, z HPC coordinates for the region ", k
		PRINT, hpc_x
		PRINT, hpc_y
	ENDIF
	
	
	; Create an Hek event and fill it
		
	event = struct4event('CH')

	event.required.OBS_Observatory = 'SDO'
	event.required.OBS_Instrument = 'AIA'
	event.required.OBS_ChannelID = 'AIA 193'
	event.required.OBS_MeanWavel =  FLOAT(region_stats_table_header.WAVELNTH); It is the value of the wavelength for the statistics
	event.required.OBS_WavelUnit = 'Angstroms'

	event.required.FRM_Name = 'SPoCA'
	event.optional.FRM_VersionNumber = FLOAT(ModuleVersionNumber)
	event.required.FRM_Identifier = 'vdelouille'
	event.required.FRM_Institute ='ROB'
	event.required.FRM_HumanFlag = 'F'
	event.required.FRM_ParamSet = FRM_ParamSet



	event.required.FRM_DateRun = FRM_DateRun
	event.required.FRM_Contact = 'veronique.delouille@sidc.be'
	event.required.FRM_URL = 'http://sdoatsidc.oma.be/web/sdoatsidc/SoftwareSPoCA'


	event.required.Event_StartTime = anytim(last_event_written_date, /ccsds) ; The start time is the previous time we wrote events
	event.required.Event_EndTime = anytim(region_table[k].DATE_OBS, /ccsds)
	  
	event.required.Event_CoordSys = 'UTC-HPC-TOPO'
	event.required.Event_CoordUnit = 'arcsec,arcsec'
	event.required.Event_Coord1 = hpc_x[0]
	event.required.Event_Coord2 = hpc_y[0]
	; Center error are in pixels in the table, so we convert them to arcsec
	event.required.Event_C1Error = region_stats_table[idx].XCENTER_ERROR * last_map_header.CDELT1
	event.required.Event_C2Error = region_stats_table[idx].YCENTER_ERROR * last_map_header.CDELT2
	event.required.BoundBox_C1LL = hpc_x[1]
	event.required.BoundBox_C2LL = hpc_y[1]
	event.required.BoundBox_C1UR = hpc_x[2]
	event.required.BoundBox_C2UR = hpc_y[2]
	
	; We only specify optional keywords if they are finite 
	IF FINITE(region_stats_table[idx].NUMBER_PIXELS) THEN event.optional.Event_Npixels = region_stats_table[idx].NUMBER_PIXELS
	event.optional.Event_PixelUnit = 'DN/s'
	event.optional.OBS_DataPrepURL = 'http://sdoatsidc.oma.be/web/sdoatsidc/SoftwareSPoCA' 
	event.optional.FRM_SpecificID =  FRM_SpecificID_Prefix +STRING(color, FORMAT='(I010)')
	IF FINITE(region_stats_table[idx].AREA_ATDISKCENTER) THEN event.optional.Area_AtDiskCenter = region_stats_table[idx].AREA_ATDISKCENTER
	IF FINITE(region_stats_table[idx].AREA_ATDISKCENTER_UNCERTAINITY) THEN event.optional.Area_AtDiskCenterUncert = region_stats_table[idx].AREA_ATDISKCENTER_UNCERTAINITY
	IF FINITE(region_stats_table[idx].RAW_AREA) THEN event.optional.Area_Raw = region_stats_table[idx].RAW_AREA
	IF FINITE(region_stats_table[idx].RAW_AREA_UNCERTAINITY) THEN event.optional.Area_Uncert = region_stats_table[idx].RAW_AREA_UNCERTAINITY
	event.optional.Area_Unit = 'Mm2'
	
	; New intensities statistics
	IF FINITE(region_stats_table[idx].MIN_INTENSITY) THEN event.optional.IntensMin = region_stats_table[idx].MIN_INTENSITY
	IF FINITE(region_stats_table[idx].MAX_INTENSITY) THEN event.optional.IntensMax = region_stats_table[idx].MAX_INTENSITY
	IF FINITE(region_stats_table[idx].MEAN_INTENSITY) THEN event.optional.IntensMean = region_stats_table[idx].MEAN_INTENSITY
	IF FINITE(region_stats_table[idx].MEDIAN_INTENSITY) THEN BEGIN 
		event.optional.IntensMedian = region_stats_table[idx].MEDIAN_INTENSITY
		; We add the event probability using the formula 1 - ((event median - min CH intensity) / (max CH intensity - min CH intensity))
		event.optional.Event_Probability = (1. - ((region_stats_table[idx].MEDIAN_INTENSITY - 7.0)/ 47.0)) > 0.0 < 1.0
	ENDIF
	IF FINITE(region_stats_table[idx].VARIANCE) THEN event.optional.IntensVar = region_stats_table[idx].VARIANCE
	IF FINITE(region_stats_table[idx].SKEWNESS) THEN event.optional.IntensSkew = region_stats_table[idx].SKEWNESS
	IF FINITE(region_stats_table[idx].KURTOSIS) THEN event.optional.IntensKurt = region_stats_table[idx].KURTOSIS
	IF FINITE(region_stats_table[idx].TOTAL_INTENSITY) THEN event.optional.IntensTotal = region_stats_table[idx].TOTAL_INTENSITY
	event.optional.IntensUnit = 'DN/s'
	
	event.optional.Event_ClippedSpatial = region_stats_table[idx].CLIPPED_SPATIAL
	

	; Required keywords from document SDO EDS API
	IF tag_exist(region_stats_table_header, "LVL_NUM") THEN event.optional.OBS_LevelNum = region_stats_table_header.LVL_NUM
	IF tag_exist(region_stats_table_header, "DATE") THEN event.optional.OBS_LastProcessingDate = region_stats_table_header.DATE
	IF tag_exist(region_stats_table_header, "QUALITY") THEN BEGIN
		NRT_bit = 2UL^30
		IF (region_stats_table_header.QUALITY AND NRT_bit) EQ NRT_bit THEN event.optional.OBS_IncludesNRT = 'T' ELSE event.optional.OBS_IncludesNRT = 'F'
	ENDIF
	
	; We chain the new event with past ones
	; i.e. we search in the relation_table for the relations such as
	; the color of my event == the present color in the relation table
	; the past color of that relation is among the color of the existing past_meta_events
	
	maxEdges = N_ELEMENTS(event.reference_names)
	edge = 0
	IF number_relations NE 0 THEN BEGIN
		good_relations = WHERE(relation_table.present_color EQ color, exists)
		IF exists GT 0 THEN BEGIN
			FOR r = 0, N_ELEMENTS(good_relations) - 1 DO BEGIN
				IF STRLOWCASE(STRTRIM(relation_table[good_relations[r]].relation_type, 2)) NE "new" THEN BEGIN
					past_meta_event = WHERE(past_meta_events.color EQ relation_table[good_relations[r]].past_color, exists)
					IF exists GT 0 THEN BEGIN
						event.reference_names[edge] = "Edge"
						event.reference_links[edge] = past_meta_events[past_meta_event[0]].last_ivorn
						event.reference_types[edge] = STRLOWCASE(STRTRIM(relation_table[good_relations[r]].relation_type, 2))
						edge = edge + 1
						; We cannot put more than 20 relations
						IF edge GE maxEdges THEN GOTO, MaxEdgesReached
					ENDIF
				ENDIF
			ENDFOR
		ENDIF
	ENDIF
	MaxEdgesReached : ; Label for when we have more than 20 edges
	
	; The chain codes of the coronal holes are stored in the table in column 
	IF write_chaincode THEN BEGIN
		x_column = where(chaincode_columns EQ STRING(region_table[k].ID, FORMAT='("X",I07)'))
		y_column = where(chaincode_columns EQ STRING(region_table[k].ID, FORMAT='("Y",I07)'))

		IF (x_column GE 0 AND y_column GE 0) THEN BEGIN
			; We convert the cartesian pixel coodinates into WCS
			cartesian_x = FLOAT(chaincode_table.(x_column))
			cartesian_y = FLOAT(chaincode_table.(y_column))
			cartesian = FLTARR(2,N_ELEMENTS(cartesian_x))
			cartesian[0,*]=cartesian_x
			cartesian[1,*]=cartesian_y
			
			; Keep only non null chaincode elements
			good_points = where(cartesian[0,*] NE 0 and cartesian[1,*] NE 0, number_chaincode_points)
			
			IF number_chaincode_points GT 2 THEN BEGIN
			
				cartesian = cartesian[*, good_points]
			
				IF (verbose GT 1) THEN BEGIN
					PRINT , "cartesians coordinates for the chaincode of the region ", k
					PRINT, cartesian
				ENDIF
				wcs_coord = WCS_GET_COORD(wcs, cartesian)
	
				; We convert the WCS coodinates into helioprojective cartesian
				WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HPC', /ARCSECONDS, hpc_x, hpc_y
	
				IF (verbose GT 1) THEN BEGIN
					PRINT , "x, y HPC coordinates for the the chaincode of the region ", k
					PRINT, hpc_x
					PRINT, hpc_y
				ENDIF
				chaincode = FLTARR(number_chaincode_points*2)
				chaincode[indgen(number_chaincode_points)*2] = hpc_x
				chaincode[indgen(number_chaincode_points)*2+1] = hpc_y
				; We add the chain code keywords
				event.OPTIONAL.CHAINCODETYPE = "ordered list of points in HPC"
				event.OPTIONAL.BOUND_CCNSTEPS = number_chaincode_points
				event.OPTIONAL.BOUND_CCSTARTC1 = hpc_x[0]
				event.OPTIONAL.BOUND_CCSTARTC2 = hpc_y[0]
				event.OPTIONAL.BOUND_CHAINCODE = STRING(chaincode, FORMAT='(F0.3,'+STRING(2*number_chaincode_points-1)+'(",",F0.3))')
			
			ENDIF ELSE IF (verbose GT 0) THEN BEGIN
				PRINT, "Not enough chaincode points for region ", region_table[k].ID
			ENDIF
		
		ENDIF ELSE BEGIN
			IF (verbose GT 0) THEN BEGIN
				PRINT , "No chaincode for Coronalhole", k
			ENDIF
		ENDELSE
	ENDIF
	
	; We write the VOevent into a buffer
	IF KEYWORD_SET(write_file) THEN BEGIN
		export_event, event, /write, suff=region_table[k].HEKID, buff=buff
	ENDIF ELSE BEGIN
		export_event, event, suffix=region_table[k].HEKID, buff=buff
	ENDELSE
	
	; We add the event to saved_events
	saved_events = [saved_events, {event_info, color:color, ivorn:event.required.kb_archivid, info:STRJOIN(buff, /SINGLE)}]
	
	; We update the meta_event
	ind = WHERE(meta_events.color EQ color, exists)
	
	IF exists EQ 0 THEN BEGIN
		IF (verbose GT 0) THEN BEGIN
			PRINT , "Adding a new meta_event for event of color ", color
		ENDIF
		first_observation_date = anytim(region_table[k].FIRST_DATE_OBS, /sec)
		meta_events = [meta_events, {meta_event, color:color, first_seen:first_observation_date, last_seen:current_observation_date, last_ivorn:event.required.kb_archivid}]
	ENDIF ELSE BEGIN
		IF (verbose GT 0) THEN BEGIN
			PRINT , "Updating existing meta_event of color ", color
		ENDIF
		meta_events[ind[0]].last_seen = current_observation_date
		meta_events[ind[0]].last_ivorn = event.required.kb_archivid
	ENDELSE 
	
ENDFOR 

; We update the time we wrote an event
last_event_written_date = current_observation_date

;  --------- We take care of ripe events -----------------

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('TAKE CARE OF RIPE EVENTS', 100, fill='_')
ENDIF

; We exports all events that are ripe, i.e. their corresponding meta_event is old enough 
ripe_meta_events = WHERE(meta_events.last_seen - meta_events.first_seen GE minLifeTime, exists)

IF exists GT 0 THEN BEGIN
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Found the following ripe meta_events: "
		FOR c = 0, N_ELEMENTS(ripe_meta_events) - 1 DO BEGIN
			PRINT, meta_events[ripe_meta_events[c]].color, " ", meta_events[ripe_meta_events[c]].last_ivorn, " age: ", (meta_events[ripe_meta_events[c]].last_seen - meta_events[ripe_meta_events[c]].first_seen) / 3600, " h"
		ENDFOR
	ENDIF
	ripe_meta_events_colors = meta_events[ripe_meta_events].color
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Colors of the ripe events: ", ripe_meta_events_colors
	ENDIF
	FOR c = 0, N_ELEMENTS(ripe_meta_events_colors) - 1 DO BEGIN
		ripe_events = WHERE(saved_events.color EQ ripe_meta_events_colors[c], exists, COMPLEMENT=unripe_events)
		IF exists GT 0 THEN BEGIN
			
			IF (verbose GT 0) THEN BEGIN
				PRINT , "Found the following ripe events: "
				FOR e = 0, N_ELEMENTS(ripe_events) - 1 DO BEGIN
					PRINT, saved_events[ripe_events[e]].color, " ", saved_events[ripe_events[e]].ivorn
					IF (verbose GT 1) THEN PRINT, saved_events[ripe_events[e]].info
				ENDFOR
			ENDIF
			
			IF N_ELEMENTS(events) GT 0 THEN BEGIN
				events = [events, saved_events[ripe_events].info]
			ENDIF ELSE BEGIN
				events = saved_events[ripe_events].info
			ENDELSE
			saved_events = saved_events[unripe_events]
		ENDIF
	ENDFOR
	
ENDIF ELSE IF (verbose GT 0) THEN BEGIN
	PRINT , "No ripe elements found: "
	FOR c = 1, N_ELEMENTS(meta_events) - 1 DO BEGIN
		PRINT, meta_events[c].color, " ", meta_events[c].last_ivorn, " age: ", (meta_events[c].last_seen - meta_events[c].first_seen) / 3600, " h"
	ENDFOR
ENDIF


;  --------- We take care of old events -----------------

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('TAKE CARE OF OLD EVENTS', 100, fill='_')
ENDIF

; We remove all events that have not shown up for a long time, i.e. they haven't been seen in a long time
dead_meta_events = WHERE(current_observation_date - meta_events.last_seen GE  minDeathTime, exists, COMPLEMENT=undead_meta_events)

IF exists GT 0 THEN BEGIN
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Found following dead meta_events: "
		FOR c = 0, N_ELEMENTS(dead_meta_events) - 1 DO BEGIN
			PRINT, meta_events[dead_meta_events[c]].color, " ", meta_events[dead_meta_events[c]].last_ivorn, " dead since: ", (current_observation_date - meta_events[dead_meta_events[c]].last_seen) / 3600, " h"
		ENDFOR
	ENDIF
	dead_meta_events_colors = meta_events[dead_meta_events].color
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Colors of the dead events: ", dead_meta_events_colors
	ENDIF
	FOR c = 0, N_ELEMENTS(dead_meta_events_colors) - 1 DO BEGIN
		dead_events = WHERE(saved_events.color EQ dead_meta_events_colors[c], exists, COMPLEMENT=undead_events)
		IF exists GT 0 THEN BEGIN
			IF (verbose GT 0) THEN BEGIN
				PRINT , "Found the following dead events: "
				FOR e = 0, N_ELEMENTS(dead_events) - 1 DO BEGIN
					PRINT, saved_events[dead_events[e]].color, " ", saved_events[dead_events[e]].ivorn
					IF (verbose GT 1) THEN PRINT, saved_events[dead_events[e]].info
				ENDFOR
			ENDIF
			saved_events = saved_events[undead_events]
		ENDIF
	ENDFOR
	meta_events = meta_events[undead_meta_events]
	
ENDIF ELSE IF (verbose GT 0) THEN BEGIN
	PRINT , "No dead events found"
	FOR c = 1, N_ELEMENTS(meta_events) - 1 DO BEGIN
		PRINT, meta_events[c].color, " ", meta_events[c].last_ivorn, " dead since: ", (current_observation_date - meta_events[c].last_seen) / 3600, " h"
	ENDFOR
ENDIF

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('END OF WRITING EVENTS', 100, fill='_')
ENDIF

Cleanup :	; Label in case of a problem, or if there is no CH

;  --------- We cleanup old files -----------------

; We save the CHmaps
IF (saveFiles EQ 1) THEN BEGIN
	FILE_COPY, CHmaps[N_ELEMENTS(CHmaps) - 1], saveDirectory, /NOEXPAND_PATH, /OVERWRITE, /REQUIRE_DIRECTORY, VERBOSE = verbose 
ENDIF

IF (saveFiles GT 1) THEN BEGIN
	FILE_COPY, CHmaps, saveDirectory, /NOEXPAND_PATH, /OVERWRITE, /REQUIRE_DIRECTORY, VERBOSE = verbose 
ENDIF
	
number_of_files_to_delete = N_ELEMENTS(CHmaps) - trackingOverlap
IF (number_of_files_to_delete GT 0) THEN BEGIN

	files_to_delete = CHmaps[0:number_of_files_to_delete-1]
	
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Deleting files : ", endl + files_to_delete
	ENDIF
	
	FILE_DELETE, files_to_delete , /ALLOW_NONEXISTENT , /NOEXPAND_PATH , VERBOSE = verbose

ENDIF


Finish :	; Label for the case we didn't do the tracking

; --------- We finish up -----------------

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('FINISHING', 100, fill='_')
ENDIF

; We update the numActiveEvents
numActiveEvents = N_ELEMENTS(meta_events) - 1

; We save the centers file 
IF (saveFiles GT 0) THEN BEGIN
	FILE_COPY,  spoca_centersfile, saveDirectory + "/centers." + anytim(current_observation_date, /ccsds) + ".txt", /NOEXPAND_PATH, /OVERWRITE, VERBOSE = verbose
ENDIF

; We update the status for the next run
status = {	last_event_written_date: last_event_written_date, $
		meta_events: meta_events, $
		saved_events: saved_events $
	}

SAVE, status , DESCRIPTION='Spoca status at ' + SYSTIME() , FILENAME=outputStatusFilename, VERBOSE = verbose
 
IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('END OF FINISH', 100, fill='_')
ENDIF
 
END ; of spoca



