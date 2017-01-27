;  GDS3D, a program for viewing GDSII files in 3D.
;  Created by Jasper Velner and Michiel Soer, http://icd.el.utwente.nl
;  Based on code by Roger Light, http://atchoo.org/gds2pov/
;  
;  Copyright (C) 2013 IC-Design Group, University of Twente.
;
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

;; Path Configuration 

; Version of the program
GDS3D_VERSION = "v1.8"
	
; Path to the server where GDS3D is located (and where in the skill subfolder this file is located)
GDS3D_SRC = "/cadappl/ictools/3D_viewer/GDS3D_1.8/"
	
; Path where temporary files are put (usually in the user's workdirectory)
GDS3D_DST = "~/GDS3D/"

; Set to t to copy executables to workdirectory or to nil to leave the executables on the server	
GDS3D_LOCAL_COPY = t  

;; Procedures which are loaded by typing load("<path_to_GDS3D_SRC>/skill/icdGDS3D.il") in your ICFB window in Cadence.
procedure( icdRunGDS3D()

	error_handler = (lambda (cid data) printf("%s" data))
	
	if( isFile(strcat(GDS3D_DST "RunLinux.sh")) == nil then
		printf(strcat("Error: can't run startup script " GDS3D_DST "RunLinux.sh, did you export a layout to GDS3D?\n"))
	else
		printf("Starting GDS3D..\n")
		ipcBeginProcess( strcat("export LD_LIBRARY_PATH=/lib; cd " GDS3D_DST "; ./RunLinux.sh &\n") "" nil error_handler nil strcat(GDS3D_DST "GDS3D.log"))	
	)
)

procedure(icdWriteGDS3D()
     prog( ()

	error_handler = (lambda (cid data) printf("%s" data))

	; Get parameters of opened cellview
	libName = geGetEditCellView()~>lib~>name
	if( libName==nil then
		printf("Please open a layout cellview!\n")
	else
	
	cellName = geGetEditCellView()~>cell~>name
	viewName = geGetEditCellView()~>view~>name
	
	; Check type
	objId=ddGetObj(libName cellName viewName "*")
	viewType=objId && ddMapGetFileViewType(objId)
	if(viewType == "maskLayout" then
	
	; Get technology file name
	techname = techGetTechLibName(ddGetObj(libName))
	
	
	; Check if source and destination directories end with /
	if( get_pname(getchar(GDS3D_SRC strlen(GDS3D_SRC))) != "/" then
		GDS3D_SRC = strcat( GDS3D_SRC "/")
	)
	if( get_pname(getchar(GDS3D_DST strlen(GDS3D_DST))) != "/" then
		GDS3D_DST = strcat( GDS3D_DST "/")
	)
	
	; Check source directory
	if( isDir(GDS3D_SRC) == nil then
		printf(strcat("Error: GDS3D source directory " GDS3D_SRC " does not exist.\n"));
		return(t);
	)
	
	; Create destination directory
	createDir(GDS3D_DST)
	if( isDir(GDS3D_DST) == nil then
		printf(strcat("Error: directory " GDS3D_DST " could not be created.\n"));
		return(t);
	)
	createDir(strcat(GDS3D_DST "gds"))
	if( isDir(strcat(GDS3D_DST "gds")) == nil then
		printf(strcat("Error: directory " GDS3D_DST "gds could not be created.\n"));
		return(t);
	)	
	
	; Copy latest GDS3D executables and techfiles
	if( GDS3D_LOCAL_COPY == t then
		if( sh(strcat("cp -R " GDS3D_SRC "win32 " GDS3D_DST )) == nil then 
			if(isDir(strcat(GDS3D_SRC "win32")) == nil then
				printf(strcat("Error: could not copy GDS3D executables to " GDS3D_DST "win32\n"))
				return(t)
			)
		)
		if( sh(strcat("cp -R " GDS3D_SRC "linux " GDS3D_DST )) == nil then
			if(isDir(strcat(GDS3D_SRC "linux")) == nil then 
				printf(strcat("Error: could not copy GDS3D executables to " GDS3D_DST "linux\n"))
				return(t)
			)
		)
		if( sh(strcat("cp -R " GDS3D_SRC "mac " GDS3D_DST )) == nil then 
			if(isDir(strcat(GDS3D_SRC "mac")) == nil then
				printf(strcat("Error: could not copy GDS3D executables to " GDS3D_DST "mac\n"))
				return(t)
			)
		)
		if( sh(strcat("cp -R -L " GDS3D_SRC "techfiles " GDS3D_DST )) == nil then
			if(isDir(strcat(GDS3D_SRC "techfiles")) == nil then 
				printf(strcat("Error: could not copy techfiles to " GDS3D_DST "techfiles\n"))
				return(t)
			)
		)
		if( sh(strcat("cp " GDS3D_SRC "README.txt " GDS3D_DST )) == nil then 
			if(isFile(strcat(GDS3D_SRC "README.txt")) == nil then
				printf(strcat("Error: could not copy GDS3D readme to " GDS3D_DST "\n"))
				return(t)
			)
		)
		GDS3D_TARGET = ""
	else
		GDS3D_TARGET = GDS3D_SRC
	) ; GDS3D_LOCAL_COPY
	
	; Check availability of technology file
	techfile = strcat(GDS3D_SRC "techfiles/" techname ".txt")
	if( isFile(techfile) == nil then
		printf(strcat("Error: process definition file not available for " techname ", please create " techfile "\n"))
		return(t);
	)

	; Output Win32 batch file
	filename = strcat(GDS3D_DST "RunWin32.cmd")
	runfile = outfile(filename)
	if( runfile == nil then
		printf(strcat("Error: could not write to " fileName "\n"))
		return(t)
	)
	fprintf(runfile strcat(GDS3D_TARGET "win32\\GDS3D.exe -p " GDS3D_TARGET "techfiles/" techname ".txt -i gds/" cellName ".gds"))
	close(runfile)

	; Output Linux shell script
	filename = strcat(GDS3D_DST "RunLinux.sh")
	runfile = outfile(filename)
	if( runfile == nil then
		printf(strcat("Error: could not write to " fileName "\n"))
		return(t)
	)
	fprintf(runfile strcat(GDS3D_TARGET "linux/GDS3D -p " GDS3D_TARGET "techfiles/" techname ".txt -i gds/" cellName ".gds"))
	close(runfile)
	ipcBeginProcess(strcat("chmod +x " GDS3D_DST "RunLinux.sh\n")) ; Change to executable

	; Output Mac shell script
	filename = strcat(GDS3D_DST "RunMac.command")
	runfile = outfile(filename)
	if( runfile == nil then
		printf(strcat("Error: could not write to " fileName "\n"))
		return(t)
	)
	fprintf(runfile strcat("cd `dirname $0`\n" GDS3D_TARGET "mac/GDS3D.app/Contents/MacOS/GDS3D -p " GDS3D_TARGET "techfiles/" techname ".txt -i gds/" cellName ".gds"))
	close(runfile)
	ipcBeginProcess(strcat("chmod +x " GDS3D_DST "RunMac.command\n")) ; Change to executable
	
	; Write GDS
	gdsFile =  strcat(GDS3D_DST "gds/" cellName ".gds")
	printf("Writing GDS for %s/%s/%s to %s, please wait..\n" libName cellName viewName gdsFile)
	; dbSave(geGetEditCellView()) ; How about read-only cells?
	if( dbGetDatabaseType() == "CDBA" then
		if( icdWriteCellGDS(techname libName cellName viewName ".gds" strcat(GDS3D_DST "gds/")) == t then
			return()
		)
	else
		
		ipcBeginProcess(strcat("strmout -library " libName " -flattenPcells -flattenVias -topCell " cellName " -view " viewName
		" -strmFile " gdsFile) "" nil error_handler 'icdNotifyDone strcat(GDS3D_DST "GDS3D.log"))
	)
	
	else
	  printf("Please open a layout cellview!\n")
	);else
	);else
	
   ) ; prog

) ; procedure

procedure( icdAddGDS3DMenu()

	; Do in CIW
	window = hiGetCIWindow();

	; Create ICD Tools menu
	if( member('icdTools hiGetBannerMenus(window) ) then
		icdToolsMenu = symeval(car(member('icdTools hiGetBannerMenus(window))))
	
		unless( member('GDS hiGetMenuItems(icdToolsMenu))
			newMenuItem = hiCreateMenuItem(
	    			?name     'GDS
	    			?itemText "Write GDS"
	    			?callback "icdWriteGDS3D()"
			)
			hiAddMenuItem(icdToolsMenu newMenuItem)
		)

		unless( member('GDS hiGetMenuItems(icdToolsMenu))
			newMenuItem2 = hiCreateMenuItem(
	   			?name     'GDS3D
	   			?itemText "Run 3D Viewer"
	    			?callback "icdRunGDS3D()"
			)
			hiAddMenuItem(icdToolsMenu newMenuItem2)
		)
	else
		
		newMenuItem = hiCreateMenuItem(
	    		?name     'GDS
	    		?itemText "Layout -> GDS3D"
	    		?callback "icdWriteGDS3D()"
		)

		newMenuItem2 = hiCreateMenuItem(
	   		?name     'GDS3D
	   		?itemText "Run GDS3D"
	    		?callback "icdRunGDS3D()"
		)
		
		icdToolsMenu = hiCreatePulldownMenu('icdTools "ICD-Tools" list(newMenuItem newMenuItem2))
		hiInsertBannerMenu(window icdToolsMenu 1024)
	
	)
	
	; Check if source and destination directories end with /
	if( get_pname(getchar(GDS3D_SRC strlen(GDS3D_SRC))) != "/" then
		GDS3D_SRC = strcat( GDS3D_SRC "/")
	)
	if( get_pname(getchar(GDS3D_DST strlen(GDS3D_DST))) != "/" then
		GDS3D_DST = strcat( GDS3D_DST "/")
	)
	
	; Check source directory
	if(isDir(GDS3D_SRC) == nil then
		printf(strcat("Error: GDS3D source directory " GDS3D_SRC " could not be found.\n"))
	else
		printf(strcat("GDS3D " GDS3D_VERSION " has been initialized.\n"))
	) ; if
		
)

procedure( icdAddGDS3DToolsMenu( _args)

	window = _args->window


	tools = symeval( car( banner = hiGetBannerMenus( window )))

	unless( member('GDS3D hiGetMenuItems(tools) )

		newMenuItem = hiCreateMenuItem(
			      ?name     'GDS3D
			      ?itemText "Prepare GDS3D"
			      ?callback "icdGDS3D()"
			  )

		hiAddMenuItem(tools newMenuItem)
	)	

) ; procedure

procedure(icdNotifyDone(o_childId x_exitStatus)
	if(x_exitStatus == 0 then
		printf("GDS written successfully.\n")  
	else
		printf("GDS writing failed!\n")	
	)

	; Remove template file
	deleteFile(strcat(GDS3D_DST "gds/streamOut.template"))
		
) ; procedure

 

procedure(icdWriteCellGDS(techname libName cellName viewName suffix outdir)
	
	templateName = strcat(GDS3D_SRC "/techfiles/" techname ".template")
	if( isFile(templateName) then
		loadi(templateName)
	else	
	
; Default streamOut list
streamOutKeys = list(nil
	'runDir			"."
	'libName		""
	'primaryCell		""
	'viewName		""
	'outFile		""
	'scale			0.001000
	'units			"micron"
	'compression		"none"
	'hierDepth		32
	'convertToGeo		t
	'noPreservePcellPins	nil
	'maxVertices		200
	'refLib			nil
	'libVersion		"5.0"
	'checkPolygon		t
	'snapToGrid		nil
	'simMosaicToArray	t
	'caseSensitivity	"preserve"
	'textCaseSensitivity	"preserve"
	'lineToZeroPath		"path"
	'convertDot	"ignore"
	'rectToBox		nil
	'convertPathToPoly	nil
	'keepPcell	nil
	'replaceBusBitChar	nil
	'useParentXYforText	nil
	'reportPrecision	t
	'runQuiet		nil
	'comprehensiveLog		nil
	'ignorePcellEvalFail		nil
	'translateTopDown		nil
	'errFile		"PIPO.LOG"
	'NOUnmappingLayerWarning		nil
	'techFileChoice		nil
	'pcellSuffix		"DbId"
	'respectGDSIILimits		nil
	'dumpPcellInfo		nil
	'genListHier		nil
	'cellMapTable		""
	'layerTable		nil
	'textFontTable		""
	'convertPin		"geometry"
	'pinInfo		0
	'pinTextMapTable	""
	'propMapTable		""
	'propSeparator		","
	'userSkillFile		""
	'rodDir			""
	'refLibList		""
)

	) ; if

    prog((template fileName ipcId)

        streamOutKeys->libName=libName

        streamOutKeys->primaryCell=cellName

        streamOutKeys->viewName=viewName

        streamOutKeys->outFile=strcat(outdir cellName suffix)

        streamOutKeys->errFile=strcat("PIPO"  ".LOG")
	
	; streamOutKeys->layerTable?

        

	fileName=strcat(GDS3D_DST "gds/streamOut.template")
	
        template=outfile(fileName)
	if( template == nil then
		printf(strcat("Error: could not write to " fileName "\n"))
		return(t)
	)
        fprintf(template "streamOutKeys='%L\n" streamOutKeys)

        close(template)


	error_handler = (lambda (cid data) printf("%s" data))
        ipcId=ipcBeginProcess(strcat("pipo strmout " fileName) "" nil error_handler
	'icdNotifyDone strcat(GDS3D_DST "GDS3D.log"))
	  
        

    ) ; prog, returns nil by default

) ; procedure

;; Startup code
icdAddGDS3DMenu()

