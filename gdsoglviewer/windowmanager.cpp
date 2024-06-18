//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2019 Bertrand Pigeard
//  Copyright (C) 2013 IC-Design Group, University of Twente.
//
//  Based on gds2pov by Roger Light, http://atchoo.org/gds2pov/ / https://github.com/ralight/gds2pov
//  Copyright (C) 2004-2008 by Roger Light
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//  
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "windowmanager.h"
#include "win_topmap.h"
#include "win_keymap.h"
#include "win_legend.h"
#include "renderer.h"

#if defined(_OPENMP)
  #include <omp.h>
#endif

WindowManager *wm;

WindowManager::WindowManager()
{
	visibility_checking= 1;
	fullscreen = 0;
	update = 1;
	assembly = false;
	mergeVia = true;

	world = NULL;
	process = NULL;
	Assembly = NULL;

	hidden_mouse = false;
}

WindowManager::~WindowManager()
{
	if(world)
		delete world;
	if(process)
		delete process;

	for(list<ListView*>::iterator l = windows.begin(); l!= windows.end(); l++)
		delete *l;
}

void WindowManager::printUsage()
{
	v_printf(1, "\n");
	v_printf(1, "GDS3D is a program for viewing a GDSII file in 3D.\n");
	v_printf(1, "Usage: GDS3D -p process.txt -i input.gds [-t topcell] [-f] [-u] [-h] [-v]\n\n");
	v_printf(1, "Usage: GDS3D -a assembly.txt [-t topcell] [-f] [-u] [-h] [-v]\n\n");
	v_printf(1, "Options\n");
	v_printf(1, " -a\t\tSpecify assembly file\n");
	v_printf(1, " -p\t\tSpecify process file\n");
	v_printf(1, " -i\t\tInput GDSII file\n");
	v_printf(1, " -t\t\tSpecify top cell name\n");
	v_printf(1, " -f\t\tFullscreen mode\n");
	v_printf(1, " -u\t\tDon't check GDS for update\n");
	v_printf(1, " -h\t\tDisplay this help\n");
	v_printf(1, " -m\t\tDon't merge Via\n\n");
	v_printf(1, " -v\t\tVerbose output\n\n");
}

bool WindowManager::commandLineParameters(int argc, char *argv[])
{
	v_printf(1, "\n");
	v_printf(1, "==============================================================================\n");
	v_printf(1, GDS3D_VERSION); v_printf(1, ", Copyright (C) 2022 Bertrand Pigeard\n");
	v_printf(1, "Copyright (C) 2013 IC-Design Group, University of Twente\n");
	v_printf(1, "Created by Jasper Velner and Michiel Soer, http://icd.el.utwente.nl\n");
	v_printf(1, "Based on the gds2pov project by Roger Light, http://atchoo.org/gds2pov/\n");	
	v_printf(1, "Copyright (C) 2004-2008 by Roger Light\n");
	v_printf(1, "Gmsh software project copyright (C) 1997-2017 C. Geuzaine, J.-F. Remacle.\n");
	v_printf(1, "Voro++ copyright (c) 2008, The Regents of the University of California, through\nLawrence Berkeley National Laboratory (subject to receipt of any required\napprovals from the U.S. Dept. of Energy). All rights reserved.\n");
	v_printf(1, "Math library copyright (C) 2006 by Paul Baker\n");
	v_printf(1, "This program comes with ABSOLUTELY NO WARRANTY. You may distribute it freely\nas described in the readme.txt distributed with this file.\n");	
	v_printf(1, "==============================================================================\n\n");

	if(argc<2){
		v_printf(-1, "Error: No input arguments given.\n");
		printUsage();
		return false; // 1
	}

	char *gdsfile=NULL;
	char *processfile=NULL;
	char *assemblyfile = NULL;
	char *topcell=NULL;
	bool ans=true;

	for(int i=1; i<argc; i++){
		if(argv[i][0] == '-'){
			if(strncmp(argv[i], "-i", strlen("-i"))==0){
				if(i==argc-1){
					v_printf(-1, "Error: -i switch given but no input file specified.\n\n");
					printUsage();
					return false;
				}else{
					gdsfile = argv[i+1];
				}
			}else if(strncmp(argv[i], "-p", strlen("-p"))==0){
				if(i==argc-1){
					v_printf(-1, "Error: -p switch given but no process file specified.\n\n");
					printUsage();
					return false;
				}else{
					processfile = argv[i+1];
				}
			}
			else if (strncmp(argv[i], "-a", strlen("-a")) == 0) {
				if (i == argc - 1) {
					v_printf(-1, "Error: -a switch given but no assembly file specified.\n\n");
					printUsage();
					return false;
				}
				else {
					assemblyfile = argv[i + 1];
				}
			}else if(strncmp(argv[i], "-q", strlen("-q"))==0){
				verbose_output--;
			}else if(strncmp(argv[i], "-f", strlen("-q"))==0){
				fullscreen = 1;
			}else if(strncmp(argv[i], "-t", strlen("-t"))==0){
				if(i==argc-1){
					v_printf(-1, "Error: -t switch given but no top cell specified.\n\n");
					printUsage();
					return false;
				}else{
					topcell = argv[i+1];
				}
			}else if(strncmp(argv[i], "-v", strlen("-v"))==0){
				verbose_output++;
			} else if (strncmp(argv[i], "-m", strlen("-v")) == 0) {
				mergeVia = false;
			}else if(strncmp(argv[i], "-u", strlen("-u"))==0){
				update=0;
			}else{
				v_printf(1, "Unknown commandline option given: ");
				v_printf(1, argv[i]);
				v_printf(1, "\n");
				printUsage();
				//return false;
			}		
		}
	}	
	if (assemblyfile) {
		assembly = true;
		Assembly = new GDSAssembly();
		Assembly->ParseFile(assemblyfile);
		if (!Assembly) {
			v_printf(-1, "Error: Out of memory.\n");
			return false;
		}
		else if (!Assembly->IsValid()) {
			v_printf(-1, "Error: %s is not a valid assembly file\n", assemblyfile);
			//delete Assembly;
			return false;
		}
		filename = assemblyfile;
		GDS *gds;
		gds = Assembly->GetGDS();
		while (gds) {
			ans = GDSInit(gds,NULL);
			gds->iptr = world->GetFilePtr();
			gds->ProcessName = new char [strlen(process->GetCurrentProcess()) + 1] ;
			strcpy(gds->ProcessName, process->GetCurrentProcess());
			if (!ans)
				return ans;
			gds = gds->Next;
		}
		
		world->_Objects->ConnectReferences();
		world->ComputeVirtualLayer();
		
		process->SetCurrentProcess(process->GetLayer()->ProcessName);

		if (topcell == NULL)
			topcell = remove_extension(getfilename(assemblyfile));
		if (!world->SetTopcell(topcell))
			{
			//delete process;
			return false;
		}
		return ans;

	} else {
		if (gdsfile == NULL) {
			v_printf(-1, "Error: No gds input given.\n");
			printUsage();
			return false; // 1
		}
		if (processfile == NULL) {
			v_printf(-1, "Error: No process file specified.\n");
			printUsage();
			return false; // 1
		}
		return GDSInit(processfile, gdsfile, topcell);
	}
	
}

bool WindowManager::GDSInit(GDS *gds, char * topcell)
{
	return GDSInit(gds->processfile, gds->gdsfile, topcell, gds->X, gds->Y, gds->Z, gds->angle, gds->Flipped, gds->FlipChip);
}

bool WindowManager::GDSInit(char *processfile, char *gdsfile, char *topcell)
{
	return GDSInit(processfile, gdsfile, topcell, 0, 0, 0, 0, 0, false);
}

bool WindowManager::GDSInit(char *processfile, char *gdsfile, char *topcell, float X, float Y, float Z, float angle, int Flipped, bool FlipChip)
{

	if (!process)
	{
		process = new GDSProcess();

		process->Parse(processfile);
	} else {
		process->ParseFile(processfile, process->LayerCount(), Z, FlipChip);
	}
	if(!process)
	{
		v_printf(-1, "Error: Out of memory.\n");
		return false;
	}else if(!process->IsValid()){
		v_printf(-1, "Error: %s is not a valid process file\n", processfile);
		//delete process;
		return false;
	}else if(process->LayerCount()==0){
		v_printf(-1, "Error: No layers found in \"%s\".\n", processfile);
		//delete process;
		return false;
	}

	FILE *iptr=NULL;
	if(gdsfile){
		iptr = fopen(gdsfile, "rb");
	}else{
		v_printf(-1, "Error: No GDS file specified.\n");
		//delete process;
		return false;
	}
	if(iptr)
	{
        v_printf(1, "Opening GDS file \"%s\"..\n\n", gdsfile);
		if (!assembly)
		{
			world = new GDSParse_ogl(process, false);
		} else {
			if (!world) {
				world = new GDSParse_ogl(process, false);
			} else {
        
			}
		}
		if (!assembly) {
			filename = gdsfile;
		}
		techname = processfile;
		char *topcell_Name = NULL;
		if (topcell) { 
			topcell_Name = new char[strlen(topcell) + 1];
			strcpy(topcell_Name, topcell);
		}else {
			topcell_Name = getfilename(gdsfile);
			topcell_Name = remove_extension(topcell_Name);
		}

		if(!world->Parse(iptr, topcell_Name, X, Y, angle, Flipped) )
		{
			if(!update)
				fclose(iptr);
			if (!assembly) {
				world->_Objects->ConnectReferences();
				world->ComputeVirtualLayer();
				if (!world->SetTopcell(topcell))
				{
					//delete process;
					return false;
				}
			} else {
				//fclose(iptr);
			}
			
		}
		//delete [] topcell_Name;
	}
	else
	{
		v_printf(-1, "Error: GDS file %s could not be opened.\n", gdsfile);
		return false; // File could not be opened
	}

	// Succesful parsing
	return true;
}

void WindowManager::bringToFront(ListView* window)
{
	windows.remove(window);
	windows.push_front(window);
}

void WindowManager::init()
{
	// Create the world
	world->gl_init();
	world->initWorld();
	world->init_viewposition();

	// Create the windows
	windows.push_back(new WinTopmap()); topmap = windows.back();
	windows.push_back(new WinKeymap()); keymap = windows.back();
	windows.push_back(new WinLegend()); legend = windows.back();		
}

void WindowManager::draw()
{
	// World in 3D
	world->gl_draw();

	// Switch to 2D rendering
	renderer.start2D(screenWidth, screenHeight);

	// Info bar
	draw_info();

	// Windows
	for(list<ListView*>::reverse_iterator l = windows.rbegin(); l!= windows.rend(); l++)
		(*l)->Draw();

	// Notifications

}

void WindowManager::draw_info()
{
	// Draw border
	renderer.drawSquare(0, 0, float(screenWidth), 22, 1, VECTOR4D(0.5f, 0.5f, 0.5f, 1.0f));

	// Lines
	float x[] = {0, float(screenWidth)};
	float y[] = {23, 23};
	renderer.drawLines(2, x, y, VECTOR4D(1.0f, 1.0f, 1.0f, 1.0f));
	
	// Text
	render_text(10, 5, "3D GDSII Viewer - University of Twente - IC Design Group", VECTOR4D(1.0f, 1.0f, 1.0f, 1.0f));
	render_text(screenWidth - 200, 5, "Press <F1> for help", VECTOR4D( 1.0f, 1.0f, 1.0f, 1.0f));
}

void WindowManager::resize(int width, int height)
{
	screenWidth = width;
	screenHeight = height;

	for(list<ListView*>::iterator l = windows.begin(); l!= windows.end(); l++)
		(*l)->SetScreenSize(width, height); // Please don't save local copies of width and height and font and such
}

void WindowManager::event( int event, int data, int xpos, int ypos , bool shift, bool control, bool alt)
{
	// Global keys
	if( event == 4 ) // Key up
	{
		switch (data) 
		{
		case KEY_F1:
			keymap->SetVisibility(!keymap->GetVisibility());
			bringToFront(keymap);
			return;
		case KEY_T:
			topmap->SetVisibility(!topmap->GetVisibility());
			bringToFront(topmap);
			return;
		case KEY_L:
			legend->SetVisibility(!legend->GetVisibility());
			bringToFront(legend);
			return;
		}
	}

	// Windows?
	for(list<ListView*>::iterator l = windows.begin(); l!= windows.end(); l++)
	{
		// Don't pass mouse events to the windows if mouse is hidden..
		if( hidden_mouse &&((event == EVENT_BUTTON_DOWN) || (event == EVENT_BUTTON_UP) || (event == EVENT_MOUSE_MOVE))) 
			continue;

		// Offer the event to the window
		if((*l)->Event(event, data, xpos, screenHeight - ypos, shift, control, alt)) // the flipped orientation of windows has to go, man
		{
			bringToFront(*l); // Window wants the focus
			return;
		}
	}

	// World?
	world->gl_event(event, data, xpos, ypos, shift, control, alt);
}

bool WindowManager::hide_mouse( void )
{
	if(!hidden_mouse)
		hidden_mouse = true;
	else
		return false; // Wrong!

	return true;
}

bool WindowManager::show_mouse( void )
{
	if(hidden_mouse)
		hidden_mouse = false;
	else
		return false; // Wrong!

	return true;
}
