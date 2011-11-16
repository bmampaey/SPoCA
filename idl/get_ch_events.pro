PRO write_events, events
	FILE_MKDIR, 'voevents'
	print, "Found ", N_ELEMENTS(events), " events"
	FOR e = 0, N_ELEMENTS(events) - 1 DO BEGIN
		; find the ivorn
		startpos = STRPOS(events[e], 'CH_SPoCA')
		IF startpos GE 0 THEN BEGIN
			length = STRPOS(events[e], '"', startpos) - startpos
			ivorn = STRMID(events[e], startpos , length) 
			print, "Event ", e, " ivorn: " , ivorn
			openw, lun, 'voevents/'+ivorn +'.xml', /get_lun, width=100000000
			printf, lun, events[e]
			free_lun, lun
		ENDIF
	ENDFOR
END

; +
; Description:
;	IDL code to extract CH events from maps
; Author:
; 	Benjamin Mampaey
; Date:
; 	16 February 2010
; Params:
; 	mapfile: in, required, type string, CHMap file
;	events: out, required, type string array, see document SDO EDS API
;	write_file: in, optional, type boolean, see document SDO EDS API
;	status: in/out, required, type struct, see document SDO EDS API
;	verbose: in, optional, type integer, verbose level of the module (0 is off)
;	minLifeTime: in, optional, type integer, Minimal time for a CH to be alife to be exported
;	minDeathTime: in, optional, type integer, Minimal time for a CH to be dead to be devinitively supressed
; - 


PRO extract_ch_events, mapfile=mapfile, $
	events = events, $
	write_file = write_file, $
	status = status, $
	verbose = verbose, $
	minLifeTime = minLifeTime, $
	minDeathTime = minDeathTime

	
; Version number
ModuleVersionNumber = 0.7

; When fits files are compressed we read the HDU 1, otherwise the 0
compressed = 0

; Global constant
regionStatsTableHdu = 'CoronalHoleStats'

; Newline shortcut for the c++ programmer
endl=STRING(10B)

; We set the vebosity of the module
IF N_ELEMENTS(verbose) EQ 0 THEN verbose = 0

; --------- We take care of the arguments -----------------

IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF PARAMETERS CHECK', 100, fill='_')
ENDIF


IF size(status, /tn) NE "STRUCT" THEN BEGIN ; The status is empty
	; We will set the start of the first event later
	last_event_written_date = 0
	; We need a dummy value because IDL does not have empty arrays 
	meta_events = REPLICATE({meta_event, color:0, first_seen:!VALUES.D_INFINITY, last_seen:!VALUES.D_INFINITY, last_ivorn:'DUMMY VALUE SHOULD NEVER APPEAR IN EVENTS RELATIONS'}, 1)
	saved_events =  REPLICATE({event_info, color:0, ivorn:'DUMMY VALUE SHOULD NEVER APPEAR', info:'DUMMY VALUE SHOULD NEVER APPEAR IN EXPORTED EVENTS'}, 1)
	status = {last_event_written_date : last_event_written_date, meta_events : meta_events, saved_events : saved_events}
ENDIF ELSE BEGIN; We read the status
	last_event_written_date = status.last_event_written_date
	meta_events = status.meta_events
	saved_events = status.saved_events
ENDELSE


IF (verbose GT 0) THEN BEGIN
	PRINT, 'Status :'
	PRINT, 'last_event_written_date : ', anytim(last_event_written_date, /ccsds)
	
	PRINT, 'meta_events : '
	FOR e = 1, N_ELEMENTS(meta_events) - 1 DO BEGIN
		PRINT, meta_events[e].color, " ", anytim(meta_events[e].first_seen, /ccsds), " ", anytim(meta_events[e].last_seen, /ccsds), " ", meta_events[e].last_ivorn
	ENDFOR
	PRINT, 'saved_events : '
	FOR e = 1, N_ELEMENTS(saved_events) - 1 DO BEGIN
		PRINT, saved_events[e].color, " ", saved_events[e].ivorn
		IF (verbose GT 1) THEN PRINT, saved_events[e].info
	ENDFOR
ENDIF

; We verify our module arguments

IF N_ELEMENTS(minLifeTime) EQ 0 THEN minLifeTime = 3 * 24 * 3600.0D
IF N_ELEMENTS(minDeathTime) EQ 0 THEN minDeathTime = 6 * 3600.0D



; --------- We write the events -----------------

IF (verbose GT 0) THEN BEGIN

	PRINT, endl, STRPAD('BEGINNING OF WRITING EVENTS', 100, fill='_')
ENDIF

mapfile_header = fitshead2struct(HEADFITS(mapfile, EXTEN=1))
current_observation_date = anytim(mapfile_header.DATE_OBS, /sec)

; We read the table of Regions
region_table = MRDFITS(mapfile , "Regions", region_table_header, extnum=extnum, status=status) 

; If the table of region is empty, MRDFITS return 0 
IF size(region_table, /tn) NE "STRUCT" THEN BEGIN
	; Even IF there is no event to write, it was time to write them
	last_event_written_date = current_observation_date
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'No event, exiting'
	ENDIF
	GOTO, Finish
ENDIF 

region_table_header = fitshead2struct(region_table_header)
number_events = N_ELEMENTS(region_table)

; We read the table of Coronal Hole stats
region_stats_table = MRDFITS(mapfile, regionStatsTableHdu, region_stats_table_header, extnum=extnum, status=status) 
region_stats_table_header = fitshead2struct(region_stats_table_header)

; We read the table of Tracking Relations
relation_table = MRDFITS(mapfile , 'TrackingRelations', relation_table_header, extnum=extnum, status=status)
 
IF size(relation_table, /tn) NE "STRUCT" THEN BEGIN

	number_relations = 0
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'No event relations found'
	ENDIF

ENDIF ELSE BEGIN

	number_relations = N_ELEMENTS(relation_table)

ENDELSE

; We read the table of Chaincode
chaincode_table = MRDFITS(mapfile , 'Chaincode', chaincode_table_header, extnum=extnum, status=status) 

IF size(chaincode_table, /tn) NE "STRUCT" THEN BEGIN

	write_chaincode = 0
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'No chaincodes found'
	ENDIF

ENDIF ELSE BEGIN

	write_chaincode = 1
	chaincode_columns = TAG_NAMES(chaincode_table)

ENDELSE



; We need the wcs info in the header of the image to transform the coordinates
wcs = fitshead2wcs(mapfile_header)

; We predefine some common values for the event
FRM_ParamSet = 'image195 : calibrated image 193/195 A' 
FRM_ParamSet += '; spocaPreprocessing='       + STRTRIM(STRING(mapfile_header.CPREPROC), 2) 
FRM_ParamSet += '; spocaClassifierType='      + STRTRIM(STRING(mapfile_header.CLASTYPE), 2) 
FRM_ParamSet += '; spocaNumberclasses='       + STRING(mapfile_header.CNBRCLAS,FORMAT='(I0)') 
FRM_ParamSet += '; spocaChannels='            + STRTRIM(STRING(mapfile_header.CHANNELS), 2) 
FRM_ParamSet += '; spocaPrecision='           + STRTRIM(STRING(mapfile_header.CPRECIS), 2) 
FRM_ParamSet += '; spocaRadiusRatio='         + STRING(mapfile_header.CRADRATI , FORMAT='(F0.2)') 
FRM_ParamSet += '; spocaBinsize='             + STRTRIM(STRING(mapfile_header.CBINSIZE), 2) 
FRM_ParamSet += '; spocaSegmentationType='    + STRTRIM(STRING(mapfile_header.SEGMTYPE), 2) 
FRM_ParamSet += '; spocaVersion='             + STRING(mapfile_header.CVERSION, FORMAT='(F0.2)') 
FRM_ParamSet += '; regionStatsPreprocessing=' + STRTRIM(STRING(mapfile_header.RPREPROC), 2) 
FRM_ParamSet += '; regionStatsRadiusRatio='   + STRING(mapfile_header.RRADRATI, FORMAT='(F0.2)') 
FRM_ParamSet += '; trackingDeltat='           + STRING(region_table_header.TMAXDELT, FORMAT='(I0)') 
FRM_ParamSet += '; trackingOverlap='          + STRING(region_table_header.TOVERLAP, FORMAT='(I0)') 
FRM_ParamSet += '; trackingNumberImages='     + STRING(region_table_header.TNBRIMG, FORMAT='(I0)') 
FRM_ParamSet += '; minLifeTime='              + STRING(minLifeTime, FORMAT='(I0)') 
FRM_ParamSet += '; minDeathTime='             + STRING(minDeathTime, FORMAT='(I0)') 

center_keywords = WHERE(STRPOS(tag_names(mapfile_header), 'CLSCTR') EQ 0)

FRM_ParamSet += '; spocaCenter='              + STRTRIM(STRING(mapfile_header.(center_keywords[0])), 2)

FOR c = 1, N_ELEMENTS(center_keywords) - 1 DO BEGIN
	FRM_ParamSet += ','                       + STRTRIM(STRING(mapfile_header.(center_keywords[c])), 2)
ENDFOR

FRM_DateRun = anytim(sys2ut(), /ccsds)
FRM_SpecificID_Prefix =  'SPoCA_v' + STRING(ModuleVersionNumber, FORMAT='(F0.1)') + '_CH_' 

FOR k = 0, number_events - 1 DO BEGIN 
	
	; We get the indice of the region stats with the same id than region
	idx = WHERE(region_stats_table.ID EQ region_table[k].ID, exists)
	IF exists EQ 0 THEN CONTINUE ELSE idx = idx[0]
	
	IF (verbose GT 0) THEN BEGIN
		PRINT, 'Index of region stats of region id ', region_table[k].ID, ' is ', idx, ' with region stats id ', region_stats_table[idx].ID
	ENDIF
	
	; We get the color of the region
	color = region_table[k].TRACKED_COLOR
	
	; We convert the cartesian pixel coodinates into WCS
	cartesian_x = FLOAT([region_stats_table[idx].XCENTER,region_table[k].XBOXMIN, region_table[k].XBOXMAX])
	cartesian_y = FLOAT([region_stats_table[idx].YCENTER,region_table[k].YBOXMIN, region_table[k].YBOXMAX])
	cartesian = FLTARR(2,N_ELEMENTS(cartesian_x))
	cartesian[0,*]=cartesian_x
	cartesian[1,*]=cartesian_y
	IF (verbose GT 1) THEN BEGIN
		PRINT , "cartesians coordinates for the region ", k
		PRINT, cartesian
	ENDIF
	wcs_coord = WCS_GET_COORD(wcs, cartesian)
	
	; We convert the WCS coodinates into helioprojective cartesian
	WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HPC', /ARCSECONDS, hpc_x, hpc_y
	
	IF (verbose GT 1) THEN BEGIN
		PRINT , "x, y, z HPC coordinates for the region ", k
		PRINT, hpc_x
		PRINT, hpc_y
	ENDIF
	
	
	; Create an Hek event and fill it
		
	event = struct4event('CH')

	event.required.OBS_Observatory = 'SDO'
	event.required.OBS_Instrument = 'AIA'
	event.required.OBS_ChannelID = 'AIA 193'
	event.required.OBS_MeanWavel =  FLOAT(region_stats_table_header.WAVELNTH); It is the value of the wavelength for the statistics
	event.required.OBS_WavelUnit = 'Angstroms'

	event.required.FRM_Name = 'SPoCA'
	event.optional.FRM_VersionNumber = FLOAT(ModuleVersionNumber)
	event.required.FRM_Identifier = 'vdelouille'
	event.required.FRM_Institute ='ROB'
	event.required.FRM_HumanFlag = 'F'
	event.required.FRM_ParamSet = FRM_ParamSet



	event.required.FRM_DateRun = FRM_DateRun
	event.required.FRM_Contact = 'veronique.delouille@sidc.be'
	event.required.FRM_URL = 'http://sdoatsidc.oma.be/web/sdoatsidc/SoftwareSPoCA'


	event.required.Event_StartTime = anytim(last_event_written_date, /ccsds) ; The start time is the previous time we wrote events
	event.required.Event_EndTime = anytim(region_table[k].DATE_OBS, /ccsds)
	  
	event.required.Event_CoordSys = 'UTC-HPC-TOPO'
	event.required.Event_CoordUnit = 'arcsec,arcsec'
	event.required.Event_Coord1 = hpc_x[0]
	event.required.Event_Coord2 = hpc_y[0]
	; Center error are in pixels in the table, so we convert them to arcsec
	event.required.Event_C1Error = region_stats_table[idx].XCENTER_ERROR * mapfile_header.CDELT1
	event.required.Event_C2Error = region_stats_table[idx].YCENTER_ERROR * mapfile_header.CDELT2
	event.required.BoundBox_C1LL = hpc_x[1]
	event.required.BoundBox_C2LL = hpc_y[1]
	event.required.BoundBox_C1UR = hpc_x[2]
	event.required.BoundBox_C2UR = hpc_y[2]
	
	; We only specify optional keywords if they are finite 
	IF FINITE(region_stats_table[idx].NUMBER_PIXELS) THEN event.optional.Event_Npixels = region_stats_table[idx].NUMBER_PIXELS
	event.optional.Event_PixelUnit = 'DN/s'
	event.optional.OBS_DataPrepURL = 'http://sdoatsidc.oma.be/web/sdoatsidc/SoftwareSPoCA' 
	event.optional.FRM_SpecificID =  FRM_SpecificID_Prefix +STRING(color, FORMAT='(I010)')
	IF FINITE(region_stats_table[idx].AREA_ATDISKCENTER) THEN event.optional.Area_AtDiskCenter = region_stats_table[idx].AREA_ATDISKCENTER
	IF FINITE(region_stats_table[idx].AREA_ATDISKCENTER_UNCERTAINITY) THEN event.optional.Area_AtDiskCenterUncert = region_stats_table[idx].AREA_ATDISKCENTER_UNCERTAINITY
	IF FINITE(region_stats_table[idx].RAW_AREA) THEN event.optional.Area_Raw = region_stats_table[idx].RAW_AREA
	IF FINITE(region_stats_table[idx].RAW_AREA_UNCERTAINITY) THEN event.optional.Area_Uncert = region_stats_table[idx].RAW_AREA_UNCERTAINITY
	event.optional.Area_Unit = 'Mm2'
	
	; New intensities statistics
	IF FINITE(region_stats_table[idx].MIN_INTENSITY) THEN event.optional.IntensMin = region_stats_table[idx].MIN_INTENSITY
	IF FINITE(region_stats_table[idx].MAX_INTENSITY) THEN event.optional.IntensMax = region_stats_table[idx].MAX_INTENSITY
	IF FINITE(region_stats_table[idx].MEAN_INTENSITY) THEN event.optional.IntensMean = region_stats_table[idx].MEAN_INTENSITY
	IF FINITE(region_stats_table[idx].MEDIAN_INTENSITY) THEN BEGIN 
		event.optional.IntensMedian = region_stats_table[idx].MEDIAN_INTENSITY
		; We add the event probability using the formula 1 - ((event median - min CH intensity) / (max CH intensity - min CH intensity))
		event.optional.Event_Probability = (1. - ((region_stats_table[idx].MEDIAN_INTENSITY - 7.0)/ 47.0)) > 0.0 < 1.0
	ENDIF
	IF FINITE(region_stats_table[idx].VARIANCE) THEN event.optional.IntensVar = region_stats_table[idx].VARIANCE
	IF FINITE(region_stats_table[idx].SKEWNESS) THEN event.optional.IntensSkew = region_stats_table[idx].SKEWNESS
	IF FINITE(region_stats_table[idx].KURTOSIS) THEN event.optional.IntensKurt = region_stats_table[idx].KURTOSIS
	IF FINITE(region_stats_table[idx].TOTAL_INTENSITY) THEN event.optional.IntensTotal = region_stats_table[idx].TOTAL_INTENSITY
	event.optional.IntensUnit = 'DN/s'
	
	event.optional.Event_ClippedSpatial = region_stats_table[idx].CLIPPED_SPATIAL
	

	; Required keywords from document SDO EDS API
	IF tag_exist(region_stats_table_header, "LVL_NUM") THEN event.optional.OBS_LevelNum = region_stats_table_header.LVL_NUM
	IF tag_exist(region_stats_table_header, "DATE") THEN event.optional.OBS_LastProcessingDate = region_stats_table_header.DATE
	IF tag_exist(region_stats_table_header, "QUALITY") THEN BEGIN
		NRT_bit = 2UL^30
		IF (region_stats_table_header.QUALITY AND NRT_bit) EQ NRT_bit THEN event.optional.OBS_IncludesNRT = 'T' ELSE event.optional.OBS_IncludesNRT = 'F'
	ENDIF
	
	; We chain the new event with past ones
	; i.e. we search in the relation_table for the relations such as
	; the color of my event == the present color in the relation table
	; the past color of that relation is among the color of the existing meta_events
	
	maxEdges = N_ELEMENTS(event.reference_names)
	edge = 0
	IF number_relations NE 0 THEN BEGIN
		good_relations = WHERE(relation_table.present_color EQ color, exists)
		IF exists GT 0 THEN BEGIN
			FOR r = 0, N_ELEMENTS(good_relations) - 1 DO BEGIN
				IF STRLOWCASE(STRTRIM(relation_table[good_relations[r]].relation_type, 2)) NE "new" THEN BEGIN
					past_meta_event = WHERE(meta_events.color EQ relation_table[good_relations[r]].past_color, exists)
					IF exists GT 0 THEN BEGIN
						event.reference_names[edge] = "Edge"
						event.reference_links[edge] = meta_events[past_meta_event[0]].last_ivorn
						event.reference_types[edge] = STRLOWCASE(STRTRIM(relation_table[good_relations[r]].relation_type, 2))
						edge = edge + 1
						; We cannot put more than 20 relations
						IF edge GE maxEdges THEN GOTO, MaxEdgesReached
					ENDIF
				ENDIF
			ENDFOR
		ENDIF
	ENDIF
	MaxEdgesReached : ; Label for when we have more than 20 edges
	
	; The chain codes of the coronal holes are stored in the table in column 
	IF write_chaincode THEN BEGIN
		x_column = where(chaincode_columns EQ STRING(region_table[k].ID, FORMAT='("X",I07)'))
		y_column = where(chaincode_columns EQ STRING(region_table[k].ID, FORMAT='("Y",I07)'))

		IF (x_column GE 0 AND y_column GE 0) THEN BEGIN
			; We convert the cartesian pixel coodinates into WCS
			cartesian_x = FLOAT(chaincode_table.(x_column))
			cartesian_y = FLOAT(chaincode_table.(y_column))
			cartesian = FLTARR(2,N_ELEMENTS(cartesian_x))
			cartesian[0,*]=cartesian_x
			cartesian[1,*]=cartesian_y
			
			; Keep only non null chaincode elements
			good_points = where(cartesian[0,*] NE 0 and cartesian[1,*] NE 0, number_chaincode_points)
			
			IF number_chaincode_points GT 0 THEN BEGIN
			
				cartesian = cartesian[*, good_points]
			
				IF (verbose GT 1) THEN BEGIN
					PRINT , "cartesians coordinates for the chaincode of the region ", k
					PRINT, cartesian
				ENDIF
				wcs_coord = WCS_GET_COORD(wcs, cartesian)
	
				; We convert the WCS coodinates into helioprojective cartesian
				WCS_CONVERT_FROM_COORD, wcs, wcs_coord, 'HPC', /ARCSECONDS, hpc_x, hpc_y
	
				IF (verbose GT 1) THEN BEGIN
					PRINT , "x, y HPC coordinates for the the chaincode of the region ", k
					PRINT, hpc_x
					PRINT, hpc_y
				ENDIF
				chaincode = FLTARR(number_chaincode_points*2)
				chaincode[indgen(number_chaincode_points)*2] = hpc_x
				chaincode[indgen(number_chaincode_points)*2+1] = hpc_y
				; We add the chain code keywords
				event.OPTIONAL.CHAINCODETYPE = "ordered list of points in HPC"
				event.OPTIONAL.BOUND_CCNSTEPS = number_chaincode_points
				event.OPTIONAL.BOUND_CCSTARTC1 = hpc_x[0]
				event.OPTIONAL.BOUND_CCSTARTC2 = hpc_y[0]
				event.OPTIONAL.BOUND_CHAINCODE = STRING(chaincode, FORMAT='(F0.3,'+STRING(2*number_chaincode_points-1)+'(",",F0.3))')
			
			ENDIF ELSE IF (verbose GT 0) THEN BEGIN
				PRINT, "No good chaincode points for region ", region_table[k].ID
			ENDIF
		
		ENDIF ELSE BEGIN
			IF (verbose GT 0) THEN BEGIN
				PRINT , "No chaincode for Coronalhole", k
			ENDIF
		ENDELSE
	ENDIF
	
	; We write the VOevent into a buffer
	IF KEYWORD_SET(write_file) THEN BEGIN
		export_event, event, /write, suff=region_table[k].HEKID, buff=buff
	ENDIF ELSE BEGIN
		export_event, event, suffix=region_table[k].HEKID, buff=buff
	ENDELSE
	
	; We add the event to saved_events
	saved_events = [saved_events, {event_info, color:color, ivorn:event.required.kb_archivid, info:STRJOIN(buff, /SINGLE)}]
	
	; We update the meta_event
	ind = WHERE(meta_events.color EQ color, exists)
	
	IF exists EQ 0 THEN BEGIN
		IF (verbose GT 0) THEN BEGIN
			PRINT , "Adding a new meta_event for event of color ", color
		ENDIF
		first_observation_date = anytim(region_table[k].FIRST_DATE_OBS, /sec)
		meta_events = [meta_events, {meta_event, color:color, first_seen:first_observation_date, last_seen:current_observation_date, last_ivorn:event.required.kb_archivid}]
	ENDIF ELSE BEGIN
		IF (verbose GT 0) THEN BEGIN
			PRINT , "Updating existing meta_event of color ", color
		ENDIF
		meta_events[ind[0]].last_seen = current_observation_date
		meta_events[ind[0]].last_ivorn = event.required.kb_archivid
	ENDELSE 
	
ENDFOR 

; We update the time we wrote an event
last_event_written_date = current_observation_date

;  --------- We take care of ripe events -----------------

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('TAKE CARE OF RIPE EVENTS', 100, fill='_')
ENDIF

; We exports all events that are ripe, i.e. their corresponding meta_event is old enough 
ripe_meta_events = WHERE(meta_events.last_seen - meta_events.first_seen GE minLifeTime, exists)

IF exists GT 0 THEN BEGIN
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Found the following ripe meta_events: "
		FOR c = 0, N_ELEMENTS(ripe_meta_events) - 1 DO BEGIN
			PRINT, meta_events[ripe_meta_events[c]].color, " ", meta_events[ripe_meta_events[c]].last_ivorn, " age: ", (meta_events[ripe_meta_events[c]].last_seen - meta_events[ripe_meta_events[c]].first_seen) / 3600, " h"
		ENDFOR
	ENDIF
	ripe_meta_events_colors = meta_events[ripe_meta_events].color
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Colors of the ripe events: ", ripe_meta_events_colors
	ENDIF
	FOR c = 0, N_ELEMENTS(ripe_meta_events_colors) - 1 DO BEGIN
		ripe_events = WHERE(saved_events.color EQ ripe_meta_events_colors[c], exists, COMPLEMENT=unripe_events)
		IF exists GT 0 THEN BEGIN
			
			IF (verbose GT 0) THEN BEGIN
				PRINT , "Found the following ripe events: "
				FOR e = 0, N_ELEMENTS(ripe_events) - 1 DO BEGIN
					PRINT, saved_events[ripe_events[e]].color, " ", saved_events[ripe_events[e]].ivorn
					IF (verbose GT 1) THEN PRINT, saved_events[ripe_events[e]].info
				ENDFOR
			ENDIF
			
			IF N_ELEMENTS(events) GT 0 THEN BEGIN
				events = [events, saved_events[ripe_events].info]
			ENDIF ELSE BEGIN
				events = saved_events[ripe_events].info
			ENDELSE
			saved_events = saved_events[unripe_events]
		ENDIF
	ENDFOR
	
ENDIF ELSE IF (verbose GT 0) THEN BEGIN
	PRINT , "No ripe elements found: "
	FOR c = 1, N_ELEMENTS(meta_events) - 1 DO BEGIN
		PRINT, meta_events[c].color, " ", meta_events[c].last_ivorn, " age: ", (meta_events[c].last_seen - meta_events[c].first_seen) / 3600, " h"
	ENDFOR
ENDIF


;  --------- We take care of old events -----------------

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('TAKE CARE OF OLD EVENTS', 100, fill='_')
ENDIF

; We remove all events that have not shown up for a long time, i.e. they haven't been seen in a long time
dead_meta_events = WHERE(current_observation_date - meta_events.last_seen GE  minDeathTime, exists, COMPLEMENT=undead_meta_events)

IF exists GT 0 THEN BEGIN
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Found following dead meta_events: "
		FOR c = 0, N_ELEMENTS(dead_meta_events) - 1 DO BEGIN
			PRINT, meta_events[dead_meta_events[c]].color, " ", meta_events[dead_meta_events[c]].last_ivorn, " dead since: ", (current_observation_date - meta_events[dead_meta_events[c]].last_seen) / 3600, " h"
		ENDFOR
	ENDIF
	dead_meta_events_colors = meta_events[dead_meta_events].color
	IF (verbose GT 0) THEN BEGIN
		PRINT , "Colors of the dead events: ", dead_meta_events_colors
	ENDIF
	FOR c = 0, N_ELEMENTS(dead_meta_events_colors) - 1 DO BEGIN
		dead_events = WHERE(saved_events.color EQ dead_meta_events_colors[c], exists, COMPLEMENT=undead_events)
		IF exists GT 0 THEN BEGIN
			IF (verbose GT 0) THEN BEGIN
				PRINT , "Found the following dead events: "
				FOR e = 0, N_ELEMENTS(dead_events) - 1 DO BEGIN
					PRINT, saved_events[dead_events[e]].color, " ", saved_events[dead_events[e]].ivorn
					IF (verbose GT 1) THEN PRINT, saved_events[dead_events[e]].info
				ENDFOR
			ENDIF
			saved_events = saved_events[undead_events]
		ENDIF
	ENDFOR
	meta_events = meta_events[undead_meta_events]
	
ENDIF ELSE IF (verbose GT 0) THEN BEGIN
	PRINT , "No dead events found"
	FOR c = 1, N_ELEMENTS(meta_events) - 1 DO BEGIN
		PRINT, meta_events[c].color, " ", meta_events[c].last_ivorn, " dead since: ", (current_observation_date - meta_events[c].last_seen) / 3600, " h"
	ENDFOR
ENDIF

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('END OF WRITING EVENTS', 100, fill='_')
ENDIF


Finish :	; Label for the case we didn't do the extraction

; --------- We finish up -----------------

IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('FINISHING', 100, fill='_')
ENDIF

; We update the status for the next run
status = {	last_event_written_date: last_event_written_date, $
		meta_events: meta_events, $
		saved_events: saved_events $
	}

 
IF (verbose GT 0) THEN BEGIN
	PRINT, endl, STRPAD('END OF FINISH', 100, fill='_')
ENDIF
 
END 


PRO get_ch_events, dir=dir, all=all, verbose=verbose

minLifeTime = 3 * 24 * 3600.0D
minDeathTime = 28800.0D

mapfiles = FILE_SEARCH(dir, '*.CHMap.fits', /TEST_READ, /TEST_REGULAR)

FOR i=0, N_ELEMENTS(mapfiles) - 1 DO BEGIN
	events = ['']
	IF KEYWORD_SET(all) THEN BEGIN
		extract_ch_events, mapfile = mapfiles[i], events = events, verbose = verbose, status = status, minLifeTime = minLifeTime, minDeathTime = minDeathTime
		write_events, events
	ENDIF ELSE BEGIN ; We test if there is a table of Tracking Relations
		relation_table = MRDFITS(mapfiles[i] , 'TrackingRelations')
		IF size(relation_table, /tn) EQ "STRUCT" THEN BEGIN
			extract_ch_events, mapfile = mapfiles[i], events = events, verbose = verbose, status = status, minLifeTime = minLifeTime, minDeathTime = minDeathTime
			write_events, events
		ENDIF ELSE BEGIN
			print, mapfiles[i], " not used to extract events"
		ENDELSE
	ENDELSE

ENDFOR

SAVE, status, DESCRIPTION='Last get_ch_events status', FILENAME='get_ch_events.sav', /VERBOSE

END

