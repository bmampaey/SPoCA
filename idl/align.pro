; Procedure that align hmi and aia images
PRO align, hmidir = hmidir, aiadir = aiadir, outdir = outdir
	hmifiles = file_search(hmidir, '*fits')
	aiafiles = file_search(aiadir, '*fits')
	IF N_ELEMENTS(hmifiles) EQ 0 THEN BEGIN
		print, "No hmi files found"
		RETURN
	ENDIF
	
	IF N_ELEMENTS(aiafiles) EQ 0 THEN BEGIN
		print, "No aia files found"
		RETURN
	ENDIF
	
	IF NOT FILE_TEST(outdir, /DIRECTORY, /WRITE) THEN BEGIN
		print, outdir, " not a writable directory"
		RETURN
	ENDIF
	
	FOR i = 0,N_ELEMENTS(hmifiles)-1 DO BEGIN
		
		print, "Aligning ", hmifiles[i], " with ", aiafiles[i] 
		read_sdo, hmifiles[i], hmiheader, hmidata
		read_sdo, aiafiles[i], aiaheader
		hmi_radius = hmiheader.RSUN_OBS/hmiheader.CDELT1
		scale = aiaheader.R_SUN / hmi_radius
		print, "Center of the sun:", hmiheader.CRPIX1, hmiheader.CRPIX2, hmidata[hmiheader.CRPIX1, hmiheader.CRPIX2]
		hmidata [where (hmidata eq hmidata[0,0]) ] = 0 
		hmidata = rot(hmidata, hmiheader.CROTA2, scale, hmiheader.CRPIX1, hmiheader.CRPIX2, interp = 1, cubic = 0, missing = hmidata[0,0])
		hmiheader.CRPIX1 = hmiheader.NAXIS1/2
		hmiheader.CRPIX2 = hmiheader.NAXIS2/2
		hmiheader.CDELT1 = aiaheader.CDELT1
		hmiheader.CDELT2 = aiaheader.CDELT2
		hmiheader.CROTA2 = 0.0
		hmiheader = add_tag(hmiheader,aiaheader.R_SUN,'R_SUN')
		mwritefits, hmiheader, hmidata, outfile=concat_dir(outdir, FILE_BASENAME(hmifiles[i]))
		
	ENDFOR
END
