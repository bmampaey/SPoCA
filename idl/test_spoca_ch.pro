PRO write_events, events
	FILE_MKDIR, 'voevents'
	print, "Found ", N_ELEMENTS(events) - 1, " events"
	FOR e = 1, N_ELEMENTS(events) - 1 DO BEGIN
		; find the ivorn
		startpos = STRPOS(events[e], 'CH_SPoCA')
		length = STRPOS(events[e], '"', startpos) - startpos
		ivorn = STRMID(events[e], startpos , length) 
		print, "Event ", e, " ivorn: " , ivorn
		openw, lun, 'voevents/'+ivorn +'.xml', /get_lun, width=100000000
		printf, lun, events[e]
		free_lun, lun
	ENDFOR
	events = ['']
END

PRO test_spoca, dir=dir, resume=resume, files195=files195, start_index=start_index

;files195 = FILE_SEARCH('/pool/bem/data/oneperhour', '*193.fits', /TEST_READ, /TEST_REGULAR)

outputDirectory = "results/"
saveDirectory = "save/
writeEventsFrequency = 14400 ; For my test I write every 4 hours
cCodeLocation = "bin/"
instrument = 'AIA'

spocaArgsPreprocessing = 'DivExpTime,ALC,TakeSqrt'
spocaArgsClassifierType = 'HFCM'
spocaArgsNumberclasses ='4'
spocaArgsPrecision = '0.0015'
spocaArgsRadiusRatio = '1.2'
spocaArgsBinsize = '0.01'
spocaArgsSegmentation = 'max'
spocaArgsNumberCenters = '10'

trackingArgsDeltat = '14400'; == 1h
trackingOverlap = 3
regionStatsPreprocessing = 'NAR,DivExpTime'
regionStatsRadiusRatio = '0.95'

w195='193'

inputStatusFilename = "spoca.sav"
outputStatusFilename = "spoca.sav"
write_file = 1

minLifeTime = 5 * 24 * 3600.0D
minDeathTime = 21600.0D

events = ['']

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


IF N_ELEMENTS(files195) EQ 0 THEN BEGIN

	print, 'No files with wavelength ', w195, ' found!"
	RETURN
	
ENDIF



IF KEYWORD_SET(resume) THEN BEGIN

	RESTORE ,inputStatusFilename , VERBOSE = debug
	
	SPoCA_ch, image195 = files195[start_index], $
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
	spocaArgsClassifierType = spocaArgsClassifierType, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsRadiusRatio = spocaArgsRadiusRatio, $
	spocaArgsBinsize = spocaArgsBinsize, $
	spocaArgsSegmentation = spocaArgsSegmentation, $
	spocaArgsNumberCenters = spocaArgsNumberCenters, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio, $
	minLifeTime = minLifeTime, $
	minDeathTime = minDeathTime
	
	write_events, events
	start_index = start_index + 1
	
ENDIF ELSE BEGIN

	
	SPoCA_ch, image195 = files195[0], $
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
	spocaArgsClassifierType = spocaArgsClassifierType, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsRadiusRatio = spocaArgsRadiusRatio, $
	spocaArgsBinsize = spocaArgsBinsize, $
	spocaArgsSegmentation = spocaArgsSegmentation, $
	spocaArgsNumberCenters = spocaArgsNumberCenters, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio, $
	minLifeTime = minLifeTime, $
	minDeathTime = minDeathTime
	
	write_events, events
	start_index = 1
	
ENDELSE

PRINT, "error=", error

FOR i=start_index, N_ELEMENTS(files195) - 1 DO BEGIN

PRINT, "start_index=", start_index

SPoCA_ch, image195 = files195[i], $
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
	spocaArgsClassifierType = spocaArgsClassifierType, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsRadiusRatio = spocaArgsRadiusRatio, $
	spocaArgsBinsize = spocaArgsBinsize, $
	spocaArgsSegmentation = spocaArgsSegmentation, $
	spocaArgsNumberCenters = spocaArgsNumberCenters, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio, $
	minLifeTime = minLifeTime, $
	minDeathTime = minDeathTime
	
	write_events, events
	PRINT, "error=", error
	start_index = start_index + 1

ENDFOR

SPoCA_ch, image195 = '', $
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
	spocaArgsClassifierType = spocaArgsClassifierType, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsRadiusRatio = spocaArgsRadiusRatio, $
	spocaArgsBinsize = spocaArgsBinsize, $
	spocaArgsSegmentation = spocaArgsSegmentation, $
	spocaArgsNumberCenters = spocaArgsNumberCenters, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingOverlap = trackingOverlap, $
	regionStatsPreprocessing = regionStatsPreprocessing, $
	regionStatsRadiusRatio = regionStatsRadiusRatio, $
	minLifeTime = minLifeTime, $
	minDeathTime = minDeathTime

	write_events, events
	PRINT, "error=", error
END


