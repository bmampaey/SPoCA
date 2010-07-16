PRO run_spoca_V3D

files = FILE_SEARCH('/home/benjamin/data/eit/200305', '*.fits', /TEST_READ, /TEST_REGULAR)
inputStatusFilename = "spoca_V3D.sav"
outputStatusFilename = "spoca_V3D.sav"
write_file = 1

outputDirectory = "results/"
cCodeLocation = "bin/"
spocaArgsPreprocessing = '5'
spocaArgsNumberclasses ='3'
spocaArgsPrecision = '0.000000001'
spocaArgsBinsize = '0.1,0.1'
trackingArgsDeltat = '86400'
trackingNumberImages = 9
trackingOverlap = 3



spoca_V3D, image171 = files[0], image195 = files[1], $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Construct', $
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

FOR i=1, (N_ELEMENTS(files)/2 - 2) DO BEGIN

spoca_V3D, image171 = files[i*2], image195 = files[i*2+1], $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Normal', $
	inputStatusFilename = inputStatusFilename, $
	outputStatusFilename = inputStatusFilename, $
	outputDirectory = outputDirectory, $
	cCodeLocation = cCodeLocation, $
	spocaArgsPreprocessing = spocaArgsPreprocessing, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsBinsize = spocaArgsBinsize, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingNumberImages = trackingNumberImages, $
	trackingOverlap = trackingOverlap

ENDFOR

spoca_V3D, image171 = '', image195 = '', $
	events = events, $
	write_file = write_file, $
	error = error, $
	imageRejected = imageRejected, $
	status = status, $
	runMode = 'Clear Events', $
	inputStatusFilename = inputStatusFilename, $
	outputStatusFilename = inputStatusFilename, $
	outputDirectory = outputDirectory, $
	cCodeLocation = cCodeLocation, $
	spocaArgsPreprocessing = spocaArgsPreprocessing, $
	spocaArgsNumberclasses = spocaArgsNumberclasses, $
	spocaArgsPrecision = spocaArgsPrecision, $
	spocaArgsBinsize = spocaArgsBinsize, $
	trackingArgsDeltat = trackingArgsDeltat, $
	trackingNumberImages = trackingNumberImages, $
	trackingOverlap = trackingOverlap

end
