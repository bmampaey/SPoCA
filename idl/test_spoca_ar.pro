PRO test_spoca, dir=dir, resume=resume, files171=files171, files195=files195, start_index=start_index

;files171 = FILE_SEARCH('/home/benjamin/spoca/spoca_hek/data', '*171.fits', /TEST_READ, /TEST_REGULAR)
;files195 = FILE_SEARCH('/home/benjamin/spoca/spoca_hek/data', '*193.fits', /TEST_READ, /TEST_REGULAR)

outputDirectory = "results/"
saveDirectory = "save/"
writeEventsFrequency = 14400 ; For my test I write every 4 hours
cCodeLocation = "bin/"
instrument = 'AIA'

spocaArgsPreprocessing = 'DivExpTime,ALC,DivMedian'
spocaArgsClassifierType = 'HFCM'
spocaArgsNumberclasses ='4'
spocaArgsPrecision = '0.0015'
spocaArgsRadiusRatio = '1.2'
spocaArgsBinsize = '0.01,0.01'
spocaArgsSegmentation = 'max'

trackingArgsDeltat = '14400'; == 1h
trackingOverlap = 3
regionStatsPreprocessing = 'NAR,DivExpTime'
regionStatsRadiusRatio = '0.95'

w171='171'
w195='193'

inputStatusFilename = "spoca.sav"
outputStatusFilename = "spoca.sav"
write_file = 1



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

IF N_ELEMENTS(files171) EQ 0 THEN BEGIN

	PRINT, 'No files with wavelength ', w171, ' found!"
	RETURN
	
ENDIF

IF N_ELEMENTS(files195) EQ 0 THEN BEGIN

	PRINT, 'No files with wavelength ', w195, ' found!"
	RETURN
	
ENDIF



IF KEYWORD_SET(resume) THEN BEGIN

	RESTORE ,inputStatusFilename , VERBOSE = debug
	
	spoca, image171 = files171[start_index], image195 = files195[start_index], $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Recovery', $
	inputStatusFilename = inputStatusFilename, $
	outputStatusFilename = outputStatusFilename, $
	numActiveEvents = numActiveEvents, $
	outputDirectory = outputDirectory, $
	saveDirectory = saveDirectory, $
	writeEventsFrequency = writeEventsFrequency, $
	cCodeLocation = cCodeLocation, $
	instrument = instrument, $
	spocaArgsPreprocessing = spocaArgsPreprocessing, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsBinsize = spocaArgsBinsize, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio
	
	start_index = start_index + 1
	
ENDIF ELSE BEGIN

	spoca, image171 = files171[0], image195 = files195[0], $
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
	saveDirectory = saveDirectory, $
	writeEventsFrequency = writeEventsFrequency, $
	cCodeLocation = cCodeLocation, $
	instrument = instrument, $
	spocaArgsPreprocessing = spocaArgsPreprocessing, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsBinsize = spocaArgsBinsize, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio
	
	start_index = 1
	
ENDELSE

PRINT, "error=", error

FOR i=start_index, MIN([(N_ELEMENTS(files171) - 1), (N_ELEMENTS(files195) - 1)]) DO BEGIN

PRINT, "start_index=", start_index

spoca, image171 = files171[i], image195 = files195[i], $
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
	saveDirectory = saveDirectory, $
	writeEventsFrequency = writeEventsFrequency, $
	cCodeLocation = cCodeLocation, $
	instrument = instrument, $
	spocaArgsPreprocessing = spocaArgsPreprocessing, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsBinsize = spocaArgsBinsize, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio

	PRINT, "error=", error
	start_index = start_index + 1
ENDFOR

spoca, image171 = '', image195 = '', $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Clear Events', $
	inputStatusFilename = inputStatusFilename, $
	outputStatusFilename = inputStatusFilename, $
	numActiveEvents = numActiveEvents, $
	outputDirectory = outputDirectory, $
	saveDirectory = saveDirectory, $
	writeEventsFrequency = writeEventsFrequency, $
	cCodeLocation = cCodeLocation, $
	instrument = instrument, $
	spocaArgsPreprocessing = spocaArgsPreprocessing, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsBinsize = spocaArgsBinsize, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio

	PRINT, "error=", error
END



