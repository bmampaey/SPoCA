PRO test_spoca_ar, dir=dir, resume=resume, files171=files171, files195=files195

;files171 = FILE_SEARCH('/pool/bem/data/AIA_June2010_Sept2011/', '*171.fits', /TEST_READ, /TEST_REGULAR)
;files195 = FILE_SEARCH('/pool/bem/data/AIA_June2010_Sept2011/', '*193.fits', /TEST_READ, /TEST_REGULAR)

verbose = 1
saveFiles = 2
outputDirectory = "results/"
saveDirectory = "save/"
cCodeLocation = "bin/"

writeEventsFrequency = 14400 ; For my test I write every 4 hours

instrument = 'AIA'

spocaArgs = ['--preprocessingSteps', 'DivExpTime,ALC,DivMedian', $
			'--classifierType', 'HPCM2', $
			'--numberClasses', '4', $
			'--precision', '0.0015', $
			'--radiusratio', '1.2', $
			'--binSize', '0.01,0.01', $
			'--segmentation', 'threshold', '-t', '2,0,0.0001', $
			'--numberPreviousCenters', '10', $
			'--intensitiesStatsPreprocessing', 'NAR,DivExpTime', $
			'--intensitiesStatsRadiusRatio', '0.95', $
			'--maps', 'A']

chaincodeArgs = ['--chaincodeMaxPoints', '100', $
			'--chaincodeMaxDeviation','10' ]

trackingArgs = ['--max_delta_t', '14400', $ ; == 4h
			'--derotate' ]


trackingOverlap = 2


w171='171'
w195='193'

inputStatusFilename = "spoca.sav"
outputStatusFilename = "spoca.sav"
write_file = 1


IF N_ELEMENTS(resume) > 0 THEN BEGIN
	
	IF SIZE(resume, /tn) NE "STRING" THEN resume = "test_spoca.sav"
	IF ~ FILE_TEST(resume, /READ, /REGULAR) THEN BEGIN
		print, 'Cannot resume, no sav file found. Please set valid sav file as resume parameter'
		RETURN
	ENDIF
	RESTORE ,resume , /VERBOSE
	
ENDIF ELSE BEGIN

	IF KEYWORD_SET(dir) THEN BEGIN
		files171=['']
		files195=['']
		files = FILE_SEARCH(dir, '*.fits', /TEST_READ, /TEST_REGULAR)
		PRINT, files
		FOR i=0, N_ELEMENTS(files) - 1 DO BEGIN
			IF instrument EQ 'AIA' THEN read_sdo, files[i], header, /nodata ELSE header = fitshead2struct(headfits(files[i]))
			IF header.WAVELNTH EQ w171 THEN files171 = [files171, files[i]]
			IF header.WAVELNTH EQ w195 THEN files195 = [files195, files[i]]
		ENDFOR
		PRINT, N_ELEMENTS(files171)
		files171 = files171[1:*]
		files195 = files195[1:*]
	ENDIF
	
ENDELSE

IF N_ELEMENTS(files171) EQ 0 THEN BEGIN

	PRINT, 'No files with wavelength ', w171, ' found!"
	RETURN
	
ENDIF

IF N_ELEMENTS(files195) EQ 0 THEN BEGIN

	PRINT, 'No files with wavelength ', w195, ' found!"
	RETURN
	
ENDIF

IF ~ KEYWORD_SET(resume) THEN BEGIN

	SPoCA_AR, image171 = files171[0], image195 = files195[0], $
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
	trackingOverlap = trackingOverlap
	
	IF N_ELEMENTS(error) GT 1 THEN PRINT, "Errors: ", error
	
	index = 1
	SAVE, status, index, files171, files195, DESCRIPTION='test_spoca status after running spoca module on images ' + STRING(index - 1), FILENAME='test_spoca.sav', /VERBOSE
	
ENDIF

FOR i=index, MIN([(N_ELEMENTS(files171) - 1), (N_ELEMENTS(files195) - 1)]) DO BEGIN

SPoCA_AR, image171 = files171[i], image195 = files195[i], $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Normal', $
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
	trackingOverlap = trackingOverlap

	IF N_ELEMENTS(error) GT 1 THEN PRINT, "Errors: ", error
	
	index = index + 1
	SAVE, status, index, files171, files195, DESCRIPTION='test_spoca status after running spoca module on images ' + STRING(index - 1), FILENAME='test_spoca.sav', /VERBOSE
	
ENDFOR

END



