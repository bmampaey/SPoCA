PRO auto_prep

args = COMMAND_LINE_ARGS(count=nargs)
IF nargs EQ 0 THEN BEGIN
	print, "Error you must provide at least one fits file or directory, exiting"
	RETURN
ENDIF

files = [""]

FOR a=0,nargs-1 DO BEGIN
; If we have a diretory, we search fitsfiles in it
	IF FILE_TEST(args[a], /DIRECTORY, /READ) THEN BEGIN
		dir = FILE_SEARCH(args[a], '*.fits', /TEST_READ, /TEST_REGULAR, count=nfiles)
		; We warn if it didn't contain any fitsfiles
		IF nfiles EQ 0 THEN print, args[a], " does not contain any fits files" ELSE BEGIN
			files = [files, dir]
		ENDELSE
	ENDIF ELSE BEGIN
		files = [files, args[a]]
	ENDELSE
ENDFOR
print, "About to prep the following: ", files
; we pop the fake first file
files = files[1:*]

; if we have no fitsfile to prep we stop
nfiles = n_elements(files)
IF nfiles EQ 0 THEN BEGIN
	print, "No fitsfiles to prep, exiting"
ENDIF ELSE BEGIN
	aia_prep, [files], indgen(nfiles), /verbose, outdir='.', /do_write_fits
ENDELSE

END
