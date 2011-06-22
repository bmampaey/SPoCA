; Routine to batch aia_prep a lot of fits files
; filelist: string array, in, array with the paths of the fits files to aia_prep
; dir: string, in, directory containing fits files. If specified, all fits files in the directory will be aia_prep.
; filename: string, in, path to a text file containing the paths to fits files to aia_prep
; outdir: string, in, directory name where to write the aia_preped fits files

PRO aia_prep_many, filename=filename, filelist=filelist, dir=dir, outdir=outdir

	IF N_ELEMENTS(filename) > 0 THEN BEGIN
		filelist = RD_TFILE(filename, /compress)
	ENDIF ELSE IF N_ELEMENTS(dir) > 0 THEN BEGIN
		filelist = FILE_SEARCH(dir, '*.fits', /TEST_READ, /TEST_REGULAR) ; FILE_SEARCH sort the filenames
	ENDIF
	
	If N_ELEMENTS(filelist) EQ 0 THEN BEGIN
		print, "No fits files found"
		RETURN
	ENDIF
	
	print, "About to aia_prep ", N_ELEMENTS(filelist), " fits files."
	
	tempdir = get_temp_dir()
	
	i=0
	WHILE i LT N_ELEMENTS(filelist) DO BEGIN
		; We only aia_prep maximum 10 files at a time because aia_prep open them all at once
		ind_max = min([i+9, N_ELEMENTS(filelist)-1])
		files = filelist[i:ind_max]
		indices = INDGEN(n_elements(files))
		aia_prep, files, indices, /verbose, outdir=outdir, /do_write_fits
		
		; aia_prep does not clean after itself, so we do it for him
		tempfiles = tempdir + PATH_SEP() + FILE_BASENAME(files)
		print, "Deleting temp files : ", tempfiles
		file_delete, tempfiles, /ALLOW_NONEXISTENT, /VERBOSE
		
		i = i + 10
	ENDWHILE

END

