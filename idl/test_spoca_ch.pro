PRO write_events, events
	FILE_MKDIR, 'voevents'
	print, "Found ", N_ELEMENTS(events), " events"
	FOR e = 0, N_ELEMENTS(events) - 1 DO BEGIN
		; find the ivorn
		startpos = STRPOS(events[e], 'CH_SPoCA')
		IF startpos GE 0 THEN BEGIN
			length = STRPOS(events[e], '"', startpos) - startpos
			ivorn = STRMID(events[e], startpos , length) 
			print, "Writting event ", e, " with ivorn " , ivorn
			openw, lun, 'voevents/'+ivorn +'.xml', /get_lun, width=100000000
			printf, lun, events[e]
			free_lun, lun
		ENDIF
	ENDFOR
END

PRO test_spoca_ch, dir=dir, resume=resume, files195=files195

;files195 = FILE_SEARCH('/pool/bem/data/AIA_June2010_Sept2011/', '*193.fits', /TEST_READ, /TEST_REGULAR)

verbose = 1
saveFiles = 2

outputDirectory = "results/"
saveDirectory = "save/"
cCodeLocation = "bin/"

writeEventsFrequency = 14400 ; For my test I write every 4 hours

instrument = 'AIA'

spocaArgs = ['--preprocessingSteps', 'DivExpTime,ALC,ThrMax80,TakeSqrt', $
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


chaincodeArgs = ['--chaincodeMaxPoints', '100', $
			'--chaincodeMaxDeviation','10' ]

trackingArgs = ['--max_delta_t', '14400', $ ; == 4h
			'--derotate' ]

trackingOverlap = 2

w195='193'

inputStatusFilename = "spoca.sav"
outputStatusFilename = "spoca.sav"
write_file = 0

minLifeTime = 3 * 24 * 3600.0D
minDeathTime = 28800.0D


events = ['']

IF N_ELEMENTS(resume) > 0 THEN BEGIN
	
	IF SIZE(resume, /tn) NE "STRING" THEN resume = "test_spoca.sav"
	IF ~ FILE_TEST(resume, /READ, /REGULAR) THEN BEGIN
		print, 'Cannot resume, no sav file found. Please set valid sav file as resume parameter'
		RETURN
	ENDIF
	RESTORE ,resume , /VERBOSE
	
ENDIF ELSE BEGIN

	IF KEYWORD_SET(dir) THEN BEGIN
		BEGIN
			files195=['']
			files = FILE_SEARCH(dir, '*.fits', /TEST_READ, /TEST_REGULAR)
			print, files
			FOR i=0, N_ELEMENTS(files) - 1 DO BEGIN
				IF instrument EQ 'AIA' THEN read_sdo, files[i], header, /nodata ELSE header = fitshead2struct(headfits(files[i]))
				IF header.WAVELNTH EQ w195 THEN files195 = [files195, files[i]]
			ENDFOR
			files195 = files195[1:*]
		ENDIF
	ENDIF
ENDELSE

IF N_ELEMENTS(files195) EQ 0 THEN BEGIN
	
	print, 'No files with wavelength ', w195, ' found!"
	RETURN
	
ENDIF


IF ~ KEYWORD_SET(resume) THEN BEGIN
	
	events = ['']

	SPoCA_CH, image195 = files195[0], $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Construct', $
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
	
	IF N_ELEMENTS(error) GT 1 THEN PRINT, "Errors: ", error
	write_events, events
	
	index = 1
	SAVE, status, index, files195, DESCRIPTION='test_spoca status after running spoca module on images ' + STRING(index - 1), FILENAME='test_spoca.sav', /VERBOSE
	
ENDIF


FOR i=index, N_ELEMENTS(files195) - 1 DO BEGIN

	events = ['']

SPoCA_CH, image195 = files195[i], $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Normal', $
	inputStatusFilename = inputStatusFilename, $
	outputStatusFilename = inputStatusFilename, $
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
	
	IF N_ELEMENTS(error) GT 1 THEN PRINT, "Errors: ", error
	write_events, events
	
	index = index + 1
	SAVE, status, index, files195, DESCRIPTION='test_spoca status after running spoca module on images ' + STRING(index - 1), FILENAME='test_spoca.sav', /VERBOSE

ENDFOR

END


