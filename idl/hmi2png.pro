; Routine to batch convert hmi files to png
; filelist: string array, in, array with the paths of the fits files to aia_prep
; dir: string, in, directory containing fits files. If specified, all fits files in the directory will be aia_prep.
; filename: string, in, path to a text file containing the paths to fits files to aia_prep
; outdir: string, in, directory name where to write the aia_preped fits files

PRO hmi2png, filename=filename, filelist=filelist, dir=dir, outdir=outdir

	IF N_ELEMENTS(filename) > 0 THEN BEGIN
		filelist = RD_TFILE(filename, /compress)
	ENDIF ELSE IF N_ELEMENTS(dir) > 0 THEN BEGIN
		filelist = FILE_SEARCH(dir, '*.fits', /TEST_READ, /TEST_REGULAR) ; FILE_SEARCH sort the filenames
	ENDIF
	
	If N_ELEMENTS(filelist) EQ 0 THEN BEGIN
		print, "No fits files found"
		RETURN
	ENDIF
	
	print, "About to make nice hmi ", N_ELEMENTS(filelist), " fits files."
	
	 FOR i = 0, N_ELEMENTS(filelist) - 1 DO BEGIN
		read_sdo, filelist[i], header, data , /UNCOMP_DELETE
		data = SMOOTH(data, 4 , /EDGE_TRUNCATE, MISSING=0, /NAN)
		data = data >(-30) < 30
		data = ((data + 30) / 60) * 255
		pngfile = outdir + '/' + header.T_OBS + ".png" 
		WRITE_PNG, pngfile, data
	ENDFOR

END

