; To compile the procedure in a runtime program, start a clean solar soft session with at least aia and onthology in your SSW_INSTR variable
; e.g. SSW_INSTR="ontology aia"
; Then once in the idl prompt, type the following commands, then exit
; .compile runtime_aia_prep
; resolve_all,  /CONTINUE_ON_ERROR
; save, /routines, filename="runtime_aia_prep.sav", DESCRIPTION="Runtime IDL program to call aia_prep on fits file", /verbose, /embedded

; For this procedure to run, the 3 following environement variables must be set
; SSW=/path/to/your/solar/soft
; SSW_ONTOLOGY=$SSW/vobs/ontology
; AIA_CALIBRATION=$SSW/sdo/aia/calibration

; Then to run the procedure, type the following command in a regular shell
; idl -rt=runtime_aia_prep.sav -args fitsfile1 fitsfile2 fitsfile3 ...


PRO runtime_aia_prep

files = COMMAND_LINE_ARGS(count=nfiles)
IF nfiles EQ 0 THEN BEGIN
	print, "Error you must provide at least one fits file to prep"
	RETURN
ENDIF

print, "About to aia_prep the following ", nfiles, " files: ", files

aia_prep, files, indgen(nfiles), /verbose, outdir='.', /do_write_fits

END
