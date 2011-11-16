pro make_ct
	filename = 'colortables.h'
	format = '("{",I0, ", ", I0, ", ", I0,"}")'
	openw, lun, filename, /get_lun, width=10000
	
	; Create aia color tables
	wave=[1600,1700,4500,94,131,171,193,211,304,335]
	for w=0, N_ELEMENTS(wave)-1 do begin
		aia_lct, r, g, b, wavelnth=wave[w], load=0
		colortable = STRARR(N_ELEMENTS(r))
		for c = 0, N_ELEMENTS(r)-1 do begin
			colortable[c] = STRING(r[c], g[c], b[c], format=format)
		endfor
		printf, lun, "#define CT_AIA_", wave[w] , format='(A0, I0, $)'
		printf, lun, " {", STRJOIN(colortable , "," , /SINGLE), "}", format='(A0, A0, A0)'
	endfor
	
	; Create eit color tables
	wave=[171,195,284,304]
	for w=0, N_ELEMENTS(wave)-1 do begin
		eit_colors, wave[w], r, g, b
		colortable = STRARR(N_ELEMENTS(r))
		for c = 0, N_ELEMENTS(r)-1 do begin
			colortable[c] = STRING(r[c], g[c], b[c], format=format)
		endfor
		printf, lun, "#define CT_EIT_", wave[w] , format='(A0, I0, $)'
		printf, lun, " {", STRJOIN(colortable , "," , /SINGLE), "}", format='(A0, A0, A0)'
	endfor
	
	; Create Secchi EUVI color tables
	wave=[171,195,284,304]
	for w=0, N_ELEMENTS(wave)-1 do begin
		path = concat_dir(getenv('SSW'),'stereo')
		color_table = strtrim(string(wave[w]),2)+'_EUVI_color.dat'
		RESTORE, FILEPATH(color_table, ROOT_DIR=path, SUBDIRECTORY=['secchi','data','color'])
		colortable = STRARR(N_ELEMENTS(r))
		for c = 0, N_ELEMENTS(r)-1 do begin
			colortable[c] = STRING(r[c], g[c], b[c], format=format)
		endfor
		printf, lun, "#define CT_EUVI_", wave[w] , format='(A0, I0, $)'
		printf, lun, " {", STRJOIN(colortable , "," , /SINGLE), "}", format='(A0, A0, A0)'
	endfor
	
	; Extract some common and nice IDL color tables
	wave = ["GREEN-PINK", "BLUE-RED", "Rainbow"]
	defs = ["CT_GREEN_PINK", "CT_BLUE_RED", "CT_RAINBOW"]
	loadct, get_names=colortable_names
	for w=0, N_ELEMENTS(wave)-1 do begin
		LOADCT, WHERE(colortable_names EQ wave[w]), RGB_TABLE=colors
		length = N_ELEMENTS(colortable[*,0])
		colortable = STRARR(length)
		for c = 0, length-1 do begin
			colortable[c] = STRING(colors[c,*], format=format)
		endfor
		printf, lun, "#define ", defs[w] , format='(A0, A0, $)'
		printf, lun, " {", STRJOIN(colortable , "," , /SINGLE), "}", format='(A0, A0, A0)'
	endfor
	
	free_lun, lun
end
