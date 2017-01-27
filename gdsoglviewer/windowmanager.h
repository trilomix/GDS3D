//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, http://icd.el.utwente.nl
//  Based on code by Roger Light, http://atchoo.org/gds2pov/
//  
//  Copyright (C) 2013 IC-Design Group, University of Twente.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

class htime{
	// This is just an empty structure (better than a void pointer, right?)
	// The inherented windowmanager classes define their own htime
};

#include "gds_globals.h"
#include "gdsparse_ogl.h"
#include "process_cfg.h"
#include "assembly_cfg.h"
#include "key_list.h"

#define GDS3D_VERSION "GDS3D v1.9"

enum EventType
{
	EVENT_BUTTON_DOWN = 0,
	EVENT_BUTTON_UP = 1,
	EVENT_MOUSE_MOVE = 2,
	EVENT_KEY_DOWN = 3,
	EVENT_KEY_UP = 4
};

class WindowManager
{
private:
	// GDSII database
	GDSParse_ogl *world;
	GDSProcess *process;

	// Windows
	ListView* topmap;
	ListView* keymap;
	ListView* legend;
	list<ListView*> windows;

	void bringToFront(ListView* window);
	
protected:
	bool	hidden_mouse;	
	
public:
	GDSAssembly *Assembly;

	char *filename;
	char *techname;

	// Configuration properties
	int fullscreen;
	int visibility_checking;
	int update;
	bool assembly;

	// UI 
	int screenHeight;
	int screenWidth;

	WindowManager();
	~WindowManager();

	void printUsage();
	bool commandLineParameters(int argc, char *argv[]);
	bool GDSInit(GDS * gds, char * topcell);
	bool GDSInit(char * processfile, char * gdsfile, char * topcell);
	bool GDSInit(char * processfile, char * gdsfile, char * topcell, float X, float Y, float Z, float angle, int Flipped, bool FlipChip);
	// Return true if succesful
	//bool GDSInit(char *processfile, char *gdsfile, char *topcell, bool assembly);

	//bool GDSInit(char * processfile, char * gdsfile, char * topcell, bool assembly, float X, float Y, float Z, float angle, int Flipped, bool FlipChip);

	void init();
	void draw();
	void draw_info();
	void resize(int width, int height);
	void event( int event, int data, int xpos, int ypos , bool shift, bool control, bool alt); // Eventhandler

	GDSParse_ogl* getWorld() {return world;};
	GDSProcess* getProcess() {return process;};

	// Pure virtual functions
	virtual void gl_finish() = 0; // Do we still need this? -> screenshot capture..
	virtual bool hide_mouse(void);
	virtual bool show_mouse(void);
	virtual void change_cursor(int shape) = 0;
	virtual void move_mouse(int x, int y) = 0;
	virtual float timer( htime *t, int reset ) = 0;
	virtual htime* new_timer() = 0; // This will be cleaned up automatically
	virtual void render_text(int x, int y, const char * text, VECTOR4D color) = 0;
	virtual bool query_update(FILE *f) = 0;
};

extern WindowManager *wm; // Set with main()

#endif 
