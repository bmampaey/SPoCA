PRO aia_movie
; Example of making an AIA movie in IDL/SolarSoft
; Documentation on how to get data is at http://sdoatsidc.oma.be/wiki/sdoatsidc/UsingData
; Search the data with vso
hlist=vso_search('1-mar-2011 12:00','1-mar-2011 12:20',sample=120,inst='aia', wave='171')
print, hlist.physobs, hlist.fileid


;
; Get the files locally
status=vso_get(hlist,pixels=4096,/rice,site='rob',filenames=files)
help, files

; The directory where you want to write the results
outdir = '.' 

; The subfield of the images that you want
subfield_center = [2048, 2048]
subfield_height = 400
subfield_width = 800


FOR i=0, N_ELEMENTS(files)-1 DO BEGIN
	
	; For each file, extract keywords and image 
	read_sdo, files[i], hdr, img, /UNCOMP_DELETE

	; To improve the contrast, you can for example use the Solar Soft aia_intscale procedure
	img = aia_intscale(img, exptime=hdr.EXPTIME, wavelnth=hdr.WAVELNTH, bytescale=1)

	; Reduce to a subfield
	img = img[subfield_center[0]-(subfield_width/2):subfield_center[0]+(subfield_width/2),subfield_center[1]-(subfield_height/2):subfield_center[1]+(subfield_height/2)]
	
	; Write the img array as a png file
	pngfile = outdir + '/' + hdr.T_OBS + ".png"
	WRITE_PNG, pngfile, img

ENDFOR

; Then to make a movie you can use an external program such as mencoder.
; For example
; mencoder "mf://outdir/*.png" -mf fps=24 -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell -o movie.avi
; There is also the IDL command XINTERANIMATE


END
