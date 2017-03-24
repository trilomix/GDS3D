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

#include "windowmanager.h"
#include "gdsparse_ogl.h"
#include "process_cfg.h"
#include "gds_globals.h"
#include "listview.h"
#include "ui_ruler.h"
#include "ui_highlight.h"
#include "outputStream.h"

extern int verbose_output;

MATRIX4X4 worldview;
MATRIX4X4 projection;
float capture_timer = 0.0;
int capture_width = 1920*2;
int capture_height = 1080*2;

GDSParse_ogl::GDSParse_ogl(class GDSProcess *process, bool generate_process) : GDSParse(process, generate_process)
{
	assembly = false;

	_perfmon = false;
	_mouse_control = false;
	_mouse_control2 = false;
	_first_move = false;
	_temp_mouse = false;
	_turbo = false;
	_slow = false;
    
	sub_layer = NULL;
	substrate = NULL;
	firstrun = true;
    
	_x = _rx = _vrx = 0.0f;
	_y = _ry = _vry = 0.0f;
	_z = _vx = _vy = _vz = 0.0f;
	_vx2 = _vy2 = _vz2 = 0.0f;
    
	tt = 0.0; drawfps=0.0;
	
	_speed_factor = 1;
	_xmin=_ymin=0;
	_xmax=_ymax=1;
	
	_topcell = NULL;
    
    // Create UI elements
    ui_ruler = new UIRuler; // Creation and viewing of rulers
    ui_elements.push_back(ui_ruler); 
	ui_highlight = new UIHighlight; // Creation and viewing of rulers
    ui_elements.push_back(ui_highlight); 
}

GDSParse_ogl::~GDSParse_ogl()
{
    // Delete UI elements
    for(list<UIElement*>::iterator l = ui_elements.begin(); l!= ui_elements.end(); l++)
		delete *l;
}

class GDSObject *GDSParse_ogl::NewObject(char *Name, char *gdsName)
{
	return new class GDSObject_ogl(Name, gdsName);
}

FILE * GDSParse_ogl::GetFilePtr() {
	return _iptr;
}

bool GDSParse_ogl::Parse(FILE *iptr, char *topcell)
{
	return Parse(iptr, topcell, 0, 0, 0, 0);
}

bool GDSParse_ogl::Parse(FILE *iptr, char *topcell, float X=0, float Y = 0, float angle = 0,int Flipped = 0)
{
	char *currentTopCell;
	assembly = wm->assembly;
	_iptr = iptr;
	if (assembly) {
		bool result = true;
		if (iptr) {
			ivecptr.push_back(iptr);
			if (!_Objects) {
				_currentGDSName = new char[strlen(topcell) + 1];
				strcpy(_currentGDSName, topcell);

				_Objects = new GDSObjectList;
				//_CurrentObject = _Objects->AddObject(NewObject("Assembly", NULL));
				_CurrentObject = _Objects->AddObject(NewObject(remove_extension(getfilename(wm->filename)), NULL));
				
				currentTopCell = new char[strlen(topcell) + 1];
				strcpy(currentTopCell, topcell);
				_CurrentObject->AddSRef(currentTopCell, 0, 0, 0, 1);
			} else {
				_currentGDSName = new char[strlen(topcell) + 1];
				strcpy(_currentGDSName, topcell);

				currentTopCell = new char[strlen(topcell) + 1];
				strcpy(currentTopCell, topcell);
				_libname = NULL;
				_sname = NULL;
				_textstring = NULL;

				_currentelement = elNone;

				_currentangle = 0.0;
				_currentwidth = 0.0;
				_currentstrans = 0;
				_currentpathtype = 0;
				_currentlayer = -1;
				_currentdatatype = -1;
				_currentmag = 1.0;
				_currentbgnextn = 0.0;
				_currentendextn = 0.0;
				_currenttexttype = 0;
				_currentpresentation = 0;
				_arrayrows = 0;
				_arraycols = 0;
				_angle = 0.0;
				_recordlen = 0;

				_Objects->SearchObject(remove_extension(getfilename(wm->filename)), NULL)->AddSRef(currentTopCell, X, Y, Flipped, 1);
				_Objects->SearchObject(remove_extension(getfilename(wm->filename)), NULL)->SetSRefRotation(0, angle, 0);
			}

			result = GDSParse::ParseFile(NULL);
			
			v_printf(1, "\nSummary:\n\tPaths:\t\t%ld\n\tBoundaries:\t%ld\n\tBoxes:\t\t%ld\n\tStrings:\t%ld\n\tStuctures:\t%ld\n\tArrays:\t\t%ld\n\n",
				_PathElements, _BoundaryElements, _BoxElements, _TextElements, _SRefElements, _ARefElements);

			return result;
		}
		else {
			return result;
		}

	} else {
		_currentGDSName = new char[strlen(topcell) + 1];
		strcpy(_currentGDSName, topcell);
		return GDSParse::Parse(iptr, topcell);
	}
}

bool GDSParse_ogl::Parse(char *topcell, float X = 0, float Y = 0, float angle = 0, int Flipped = 0)
{
	char *currentTopCell;
	if (assembly) {
		bool result = true;
		if (!_Objects) {
			_currentGDSName = new char[strlen(topcell) + 1];
			strcpy(_currentGDSName, topcell);

			_Objects = new GDSObjectList;
			_CurrentObject = _Objects->AddObject(NewObject(remove_extension(getfilename(wm->filename)), NULL));

			currentTopCell = new char[strlen(topcell) + 1];
			strcpy(currentTopCell, topcell);
			_CurrentObject->AddSRef(currentTopCell, 0, 0, 0, 1);
		}
		else {
			_currentGDSName = new char[strlen(topcell) + 1];
			strcpy(_currentGDSName, topcell);

			currentTopCell = new char[strlen(topcell) + 1];
			strcpy(currentTopCell, topcell);
			_libname = NULL;
			_sname = NULL;
			_textstring = NULL;

			_currentelement = elNone;

			_currentangle = 0.0;
			_currentwidth = 0.0;
			_currentstrans = 0;
			_currentpathtype = 0;
			_currentlayer = -1;
			_currentdatatype = -1;
			_currentmag = 1.0;
			_currentbgnextn = 0.0;
			_currentendextn = 0.0;
			_currenttexttype = 0;
			_currentpresentation = 0;
			_arrayrows = 0;
			_arraycols = 0;
			_angle = 0.0;
			_recordlen = 0;

			_Objects->SearchObject(remove_extension(getfilename(wm->filename)), NULL)->AddSRef(currentTopCell, X, Y, Flipped, 1);
			_Objects->SearchObject(remove_extension(getfilename(wm->filename)), NULL)->SetSRefRotation(0, angle, 0);
		}

		result = GDSParse::ParseFile(NULL);

		return result;
	}
	else {
		return GDSParse::ParseFile(topcell);
	}
}
int GDSParse_ogl::SetTopcell(const char *topcell)
{
	// Cleanup? -> this can be moved somewhere else..
	for(unsigned int i=0;i<_Objects->getNumObjects();i++)
		((GDSObject_ogl*)_Objects->getObject(i))->DeleteBuffers();
	if(substrate)
	{
		renderer.deleteRecipe(substrate); // Throw away substrate
		substrate = NULL;
	}
	for(list<UIElement*>::iterator l = ui_elements.begin(); l!= ui_elements.end(); l++)
		(*l)->Reset();
	firstrun = true; // Loading screen??

	// Find new topcell
	if(topcell)
		_topcell = (GDSObject_ogl*) _Objects->SearchObject(topcell,NULL);
	else if (_Objects->getNumObjects() > 0) {
		_topcell = (GDSObject_ogl*)_Objects->SearchObject(_Objects->getObject(0)->GetGDSName(), NULL);
	}
	if(!_topcell)
	{
		if(topcell)
			v_printf(1, "Topcell \"%s\" is not in the gds.\n", topcell);
        _topcell = (GDSObject_ogl*) _Objects->GetTopObject();  //Try to find a new good topcell
	}
    
    if(_topcell)
    {
		v_printf(1, "Picking \"%s\" as topcell.\n\n", _topcell->GetName());
        return 1; // Valid topcell
    }

	return 0; // Invalid topcell
}

void GDSParse_ogl::initWorld()
{
    v_printf(1, "Building hierarchy.. ");
    
    // Absorb small objects into larger objects
    _topcell->countTotalPoints();
    _topcell->collapseHierachy();

	 v_printf(1, "done\n\n");

	 ProcessLayer *layer = _process->GetLayer(255, 0); //Try to find the substrate layer
	 if(!layer)
	 {
        v_printf(1, "\nNo substrate layer found in process file.\n");
        v_printf(1, "Add layer 255 to the process file to define a substrate layer.\n");
	 }
}

void GDSParse_ogl::buildSubstrate()
{
	// Add a substrate    
    GDSBB bounds = _topcell->GetTotalBoundary();
    ProcessLayer *layer = _process->GetLayer(255, 0); //Try to find the substrate layer
    if(layer && !substrate)
    {
		renderRecipe_t *r = renderer.beginObject();
		
        int tp =
        renderer.addVertex((float)bounds.min.X-(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.min.Y-(bounds.max.Y-bounds.min.Y)*0.05f, (float)(layer->Height+layer->Thickness)*_units);
        renderer.addVertex((float)bounds.max.X+(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.min.Y-(bounds.max.Y-bounds.min.Y)*0.05f, (float)(layer->Height+layer->Thickness)*_units);
        renderer.addVertex((float)bounds.max.X+(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.max.Y+(bounds.max.Y-bounds.min.Y)*0.05f, (float)(layer->Height+layer->Thickness)*_units);
        renderer.addVertex((float)bounds.min.X-(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.max.Y+(bounds.max.Y-bounds.min.Y)*0.05f, (float)(layer->Height+layer->Thickness)*_units);
        
        int bp = 
        renderer.addVertex((float)bounds.min.X-(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.min.Y-(bounds.max.Y-bounds.min.Y)*0.05f, (float)layer->Height*_units);
        renderer.addVertex((float)bounds.max.X+(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.min.Y-(bounds.max.Y-bounds.min.Y)*0.05f, (float)layer->Height*_units);
        renderer.addVertex((float)bounds.max.X+(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.max.Y+(bounds.max.Y-bounds.min.Y)*0.05f, (float)layer->Height*_units);
        renderer.addVertex((float)bounds.min.X-(bounds.max.X-bounds.min.X)*0.05f, (float)bounds.max.Y+(bounds.max.Y-bounds.min.Y)*0.05f, (float)layer->Height*_units);
        
        renderer.addTriangle(tp+0, tp+1, tp+2);renderer.addTriangle(tp+3, tp+0, tp+2);
        renderer.addTriangle(bp+1, bp+0, bp+2);renderer.addTriangle(bp+0, bp+3, bp+2);
        renderer.addTriangle(tp+0, bp+0, bp+1);renderer.addTriangle(tp+0, bp+1, tp+1);
        renderer.addTriangle(bp+2, tp+1, bp+1);renderer.addTriangle(bp+2, tp+2, tp+1);
        renderer.addTriangle(tp+2, bp+2, bp+3);renderer.addTriangle(tp+2, bp+3, tp+3);
        renderer.addTriangle(tp+3, bp+3, bp+0);renderer.addTriangle(tp+3, bp+0, tp+0);
		
        renderer.endObject();

		v_printf(1, "Substrate created with 12 triangles.\n");

		substrate = r;
        sub_layer = layer;
        
		//v_printf(1, "Substrate with boundary x [%f, %f], y [%f, %f].\n", bounds.min.X, bounds.max.X, bounds.min.Y, bounds.max.Y);
    } 
}

/* gl initialization function */

int GDSParse_ogl::gl_init()
{
	GLfloat light_ambient[] = {0.5, 0.5, 0.5, 1.0}; /* Red diffuse light. */
	GLfloat light_diffuse[] = {0.5, 0.5, 0.5, 1.0}; /* Red diffuse light. */
	GLfloat light_position[] = {0.0, -2.0, -1.0, 0.0}; /* Infinite light location. */
	GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};

	// OpenGL information
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);

	v_printf(1, "GL Vendor: %s\n", vendor);
	v_printf(1, "GL Renderer: %s\n", renderer);
	v_printf(1, "GL Version: %s\n", version);

	// Setup opengl
	glLineWidth( 2.0 );

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/* Enable a single OpenGL light. */
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_FLAT);

	/* Some fog */
	glFogi(GL_FOG_MODE, GL_EXP); // Fog Mode
	glFogfv(GL_FOG_COLOR, fogColor); // Set Fog Color
	glFogf(GL_FOG_DENSITY, 0.0035f); // How Dense Will The Fog Be
	glEnable(GL_FOG);

	glEnable(GL_DEPTH_TEST);
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    //glDisable( GL_MULTISAMPLE );

	init_render(); //Backend init

	// The timers are owned and deleted by the windowmanager
	_tv = wm->new_timer();
	_mt = wm->new_timer();

	wm->timer( _mt, 1 );

	_speed_factor = 5.0f;

	//init_viewposition();

	// Get file info and close the file
	/*
	struct stat attrib;
	stat(_filename, &attrib);
	low_date_time = attrib.st_mtime;*/
	if (assembly) {
		for (size_t i = 0; i < ivecptr.size(); i++) {
			wm->query_update(ivecptr[i]);
		}
	}else
		wm->query_update(_iptr);


	v_printf(1, "Ready to render..\n\n");

	return( 1 );
}

void GDSParse_ogl::gl_drawloading()
{
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();

	glLoadIdentity();
	glOrtho(0.0, (GLdouble) wm->screenWidth, 0.0, (GLdouble) wm->screenHeight, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
	gl_square(wm->screenWidth/2-54.0f, wm->screenHeight/2-16.0f, wm->screenWidth/2+54.0f , wm->screenHeight/2+16.0f, 1);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	gl_square(wm->screenWidth/2-54.0f, wm->screenHeight/2-16.0f, wm->screenWidth/2+54.0f , wm->screenHeight/2+16.0f, 0);

	// Text
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth/2 - 40, wm->screenHeight/2-6, "Loading..");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FOG);

	wm->gl_finish();
}

void GDSParse_ogl::gl_drawcapturing()
{
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();

	glLoadIdentity();
	glOrtho(0.0, (GLdouble) wm->screenWidth, 0.0, (GLdouble) wm->screenHeight, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
	gl_square(wm->screenWidth/2-90.0f, wm->screenHeight/2-16.0f, wm->screenWidth/2+90.0f , wm->screenHeight/2+16.0f, 1);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	gl_square(wm->screenWidth/2-90.0f, wm->screenHeight/2-16.0f, wm->screenWidth/2+90.0f , wm->screenHeight/2+16.0f, 0);

	// Text
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth/2 - 75, wm->screenHeight/2-6, "Screen Captured");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FOG);
}

/* window drawing function */
void GDSParse_ogl::gl_draw()
{
	//int i;
	
	// Timing
	float l = wm->timer( _tv, 0 );
	if(l != 0.0000)
		_fps = 1.0f / l;
	wm->timer( _tv, 1 ); // Reset

	tt+=l;
	_frames++;

	if(firstrun)
	{
		gl_drawloading();
		firstrun = false;
	}

	if( _fps ){
		/*change angle according to velocity */
		//_ry += _vry / _fps;
		//_rx += _vrx / _fps;

		/* slow velocity down ??*/
		if( _fps > 10.0f ){
			_ry += _vry / _fps;
			_rx += _vrx / _fps;
			//_vry = 0.0f;
			//_vrx = 0.0f;
			_vry *= 1.0f - 6.0f / (6.0f+_fps);
			_vrx *= 1.0f - 6.0f / (6.0f+_fps);
		}else{
			_ry += _vry / 10.0f;
			_rx += _vrx / 10.0f;
			_vry = 0.0f;
			_vrx = 0.0f;
		}
        
        // Exploded view animation
        exploded_fraction += exploded_accel / _fps;
        exploded_accel *=  1.0f - 6.0f / (6.0f+_fps);
        if(!exploded_view && exploded_fraction<0.002f) // End of animation?
        {
            exploded_fraction = 0.0f;
            exploded_accel = 0.0f;
        }
	}


	if( _rx > 0.0f ) _rx = 0.0f;
	if( _rx < -180.0f ) _rx = -180.0f;

	MATRIX4X4 mod;
	MATRIX4X4 rot;
	mod.SetRotationAxis( _ry, VECTOR3D(0.0f, 0.0f, 1.0f ));
	rot.SetRotationAxis( _rx, VECTOR3D(1.0f, 0.0f, 0.0f ));
	mod = rot * mod;

	if( _fps ){
		//GLfloat h = fabs(_z)*8.0f;
		//if(h<25.0f)
		// h=25.0f;

		// Walk forward/backward
		_x -= ((GLfloat*)mod)[2] * (_vz+_vz2) * (1 + 3 * _turbo) / _fps * _zfar/200.0f;
		_y -= ((GLfloat*)mod)[6] * (_vz+_vz2) * (1 + 3 * _turbo) / _fps * _zfar/200.0f;
		_z -= ((GLfloat*)mod)[10] * (_vz+_vz2) * (1 + 3 * _turbo) / _fps * _zfar/200.0f;

		// Strafe
		_x -= ((GLfloat*)mod)[0] * (_vx+_vx2) * (1 + 3 * _turbo)/ _fps * _zfar/200.0f;
		_y -= ((GLfloat*)mod)[4] * (_vx+_vx2) * (1 + 3 * _turbo)/ _fps * _zfar/200.0f;
		_z -= ((GLfloat*)mod)[8] * (_vx+_vx2) * (1 + 3 * _turbo)/ _fps * _zfar/200.0f;

		// Up/Down
		_x -= ((GLfloat*)mod)[1] * (_vy+_vy2) * (1 + 3 * _turbo)/ _fps * _zfar/200.0f;
		_y -= ((GLfloat*)mod)[5] * (_vy+_vy2) * (1 + 3 * _turbo)/ _fps * _zfar/200.0f;
		_z -= ((GLfloat*)mod)[9] * (_vy+_vy2) * (1 + 3 * _turbo)/ _fps * _zfar/200.0f;

		// Reduce fluid movement
		_vx2 *= 1.0f - 6.0f / (6.0f+_fps);
		_vy2 *= 1.0f - 6.0f / (6.0f+_fps);
		_vz2 *= 1.0f - 6.0f / (6.0f+_fps);
	}

	if( _vz < -40.0f*_speed_factor ) _vz = -40.0f*_speed_factor;
	if( _vz > 40.0f*_speed_factor ) _vz = 40.0f*_speed_factor;

	/* add an estimate for zFar, so that the whole object is always in view */
	_zfar = fabs(_z)*8.0f;
	if(_zfar<30.0f) //50
		_zfar = 30.0f; //50

	gl_draw_world(wm->screenWidth, wm->screenHeight, false);
    
	// Overlays -> move to UI elements or window manager
	if (_perfmon)
		display_perfmon();

	// Popup screens
	if(capture_timer > 0.0f)
	{
		gl_drawcapturing();
		if(_fps)
			capture_timer -= 1.0f/_fps;
	}

	if( tt < 0.0f || tt >= 0.5f ) // Used to be 5 Hz
	{
		drawfps = (GLfloat) _frames / tt;
		_frames = 0;
		tt = 0.0;

		// Update?
		//if(update)
		{
			/*
			struct stat attrib;

#if defined(WIN32)
			if(!fstat(_fileno(_iptr), &attrib))
#else
			if(!fstat(fileno(_iptr), &attrib))
#endif
			{
				if(low_date_time != attrib.st_mtime && attrib.st_size > 0)
				{
					//_iptr = fopen(_filename, "rb");
					if(_iptr)
					{
						Reload();
						low_date_time = attrib.st_mtime;
						//fclose(_iptr);
					}
				}
			}*/

			query_update();
			/*if(wm->query_update(_iptr))
			{
				char tmp[256];
				strcpy(tmp, _topcell->GetName());
				v_printf(1, "GDS has been updated, reloading..\n");
				Reload();
				SetTopcell(tmp); // This is not elegant..
				initWorld();
			}*/
		}
	}

}
void GDSParse_ogl::query_update() {
	if (assembly) {
		bool update = false;
		for (size_t i = 0; i < ivecptr.size(); i++) {
			if (wm->query_update(ivecptr[i])) {
				update = true;
				break;
			}
		}
		if (update) {
			char tmp[256];
			strcpy(tmp, _topcell->GetName());
			v_printf(1, "GDS has been updated, reloading..\n");
			//void GDSParse::Reload()
			if (_Objects)
			{
				delete _Objects;
				//_Objects = new GDSObjectList;
				_Objects = NULL;
				GDS *gds;
				gds = wm->Assembly->GetGDS();
				while (gds) {
					_iptr = gds->iptr;
					char *topcell_Name = NULL;
					topcell_Name = remove_extension(getfilename(gds->gdsfile));
					_process->SetCurrentProcess(remove_extension(getfilename(gds->processfile)));
					Parse(topcell_Name,gds->X, gds->Y, gds->angle, gds->Flipped);
					//_Objects->ConnectReferences();
					//ComputeVirtualLayer();
					delete topcell_Name;
					gds = gds->Next;
				}
			_process->SetCurrentProcess(_process->GetLayer()->ProcessName);
			_Objects->ConnectReferences();
			ComputeVirtualLayer();
            }
			
			
			//_process->SetCurrentProcess(_process->GetLayer()->ProcessName);
			SetTopcell(tmp); // This is not elegant..
			initWorld();
		}
	}
	else {
		if (wm->query_update(_iptr))
		{
			char tmp[256];
			strcpy(tmp, _topcell->GetName());
			v_printf(1, "GDS has been updated, reloading..\n");
			Reload();
			_Objects->ConnectReferences();
			SetTopcell(tmp); // This is not elegant..
			ComputeVirtualLayer();
			initWorld();
		}
		

	}
}

void GDSParse_ogl::gl_draw_world(int width, int height, bool HQ)
{
	glDisable(GL_POLYGON_OFFSET_FILL);
	glViewport( 0, 0, width, height );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode(GL_PROJECTION);
	//MATRIX4X4 projection; // This variable is global now..

	GLdouble ratio = ((GLdouble) width) / ((GLdouble) height);
	if(HQ)
		projection.SetPerspective(50.0, GLfloat(ratio), _zfar/1024.0f, _zfar*4.0f);
	else
		projection.SetPerspective(50.0, GLfloat(ratio), _zfar/1024.0f, _zfar*1.5f);

	glLoadMatrixf((GLfloat*) &projection);
	glFogf(GL_FOG_DENSITY, 2.0f / _zfar); // Adjust fog

	glMatrixMode(GL_MODELVIEW);

	MATRIX4X4 view;
	MATRIX4X4 mod;

	mod.SetRotationAxis(_rx, VECTOR3D(1.0f, 0.0f, 0.0f)); view = view * mod;
	mod.SetRotationAxis(_ry, VECTOR3D(0.0f, 0.0f, 1.0f)); view = view * mod;
	mod.SetTranslation(VECTOR3D(-_x, -_y, -_z)); view = view * mod;
    
	glLoadMatrixf((GLfloat*) &view);
	worldview = view;

	// Find the top cell and draw
	total_tris = 0;

	_topcell->PrepareRender(projection, view);
	_topcell->RenderList(view, HQ);
	if(!substrate)
		buildSubstrate();
    if(sub_layer)
    {
        VECTOR4D color;
		if(sub_layer->Show) // Is substrate visible?
		{
			color.Set(sub_layer->Red*color_scale, sub_layer->Green*color_scale, sub_layer->Blue*color_scale, 1.0f-sub_layer->Filter);
			renderer.renderObject(substrate, &view, &color, true);
		}
    }
	_topcell->EndRender();
	glLoadMatrixf((GLfloat*) &view); // Reset modelview matrix
	
	// All the UI elements -> this should not be here!
    for(list<UIElement*>::iterator l = ui_elements.begin(); l!= ui_elements.end(); l++)
        (*l)->Draw();	

	// Reset view
	glLoadMatrixf((GLfloat*) &view);
}

void GDSParse_ogl::gl_printf( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha,
	GLint x, GLint y, const char *format, ... )
{
	va_list argp;
	char text[256];

	va_start( argp, format );
	vsprintf( text, format, argp );
	va_end( argp );

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	glDisable(GL_BLEND);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();

	glLoadIdentity();
	glOrtho(0.0, (GLdouble) wm->screenWidth, 0.0, (GLdouble) wm->screenHeight, -1.0f, 1.0f);

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	wm->render_text(x, y, text, VECTOR4D(red, green, blue, alpha));

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FOG);
	glEnable(GL_BLEND);
}

/* event handling function */

void GDSParse_ogl::gl_event( int event, int data, int xpos, int ypos , bool shift, bool control, bool alt)
{
	int numkey;
	list<GDSObject*> sorted_list;
	MATRIX4X4 identity;

    // Try to let UI element handle event, but only if the mouse is not captured
	//if((!_mouse_control) && (!_mouse_control2))
	{
		for(list<UIElement*>::iterator l = ui_elements.begin(); l!= ui_elements.end(); l++)
		{
			if((*l)->Event(event, data, xpos, ypos, shift, control, alt))
				return; // The UI element has handled the event
		}
	}
    
    // Try to handle event ourselves
	if( event == 0 ) /* mouse button down */
	{
		if( data == 0 ) /* left button */
		{
			/*
			if (_mouse_control && !_temp_mouse) {
				_vz += 5.0f*_speed_factor;
			}*/
			if(!_mouse_control && !_mouse_control2) 
			{
				_mouse_control = true;
				_temp_mouse = true; // This will end after mouse button up
				//hide_mouse();
				//mouse_prev_x = wm->screenWidth/2;
				//mouse_prev_y = wm->screenHeight/2;
				//move_mouse( wm->screenWidth / 2, wm->screenHeight / 2 );
				mouse_prev_x = xpos;
				mouse_prev_y = ypos;
				_first_move = true;
			}
		}

		if( data == 1 ) /* right button */
		{
			if(!_mouse_control && !_mouse_control2) 
			{
				_mouse_control2 = true;
				_temp_mouse = true; // This will end after mouse button up
				//hide_mouse();
				//mouse_prev_x = wm->screenWidth/2;
				//mouse_prev_y = wm->screenHeight/2;
				//move_mouse( wm->screenWidth / 2, wm->screenHeight / 2 );
				mouse_prev_x = xpos;
				mouse_prev_y = ypos;
				_first_move = true;

			}
			/*
			if (_mouse_control && !_temp_mouse)
				_vz += -3.0f*_speed_factor;*/
		}

		if ( data == 2 ) { // Mouse wheel down
			if(control)
			{
				struct ProcessLayer * current_layer = NULL;
				int i=-1;
				current_layer = wm->getProcess()->GetLayer(); // First layer
				while(current_layer)
				{
					if(!current_layer->Show)
					{
						i = current_layer->Index;
						break;
					}
					current_layer = current_layer->Next;
				}
				if(i > -1)
					wm->getProcess()->ChangeVisibility(wm->getProcess()->GetLayer(i), true);
			}
			else if(alt)
			{
				struct ProcessLayer * current_layer = NULL;
				int i=-1;
				current_layer = wm->getProcess()->GetLayer(); // First layer
				while(current_layer)
				{
					if(current_layer->Show)
					{
						i = current_layer->Index;
						break;
					}
					current_layer = current_layer->Next;
				}
				if(i > -1)
					wm->getProcess()->ChangeVisibility(wm->getProcess()->GetLayer(i), false);
			}
			else if(shift) {
				_vy2 -= 5.0f*_speed_factor/10.0;
			}
			else
				_vy2 -= 5.0f*_speed_factor;
		}
		
		if ( data == 3 ) { // Mouse wheel up
			if(control)
			{
				struct ProcessLayer * current_layer = NULL;
				int i=-1;
				current_layer = wm->getProcess()->GetLayer(); // First layer
				while(current_layer)
				{
					if(current_layer->Show)
						i = current_layer->Index;
					current_layer = current_layer->Next;
				}
				if(i > -1)
					wm->getProcess()->ChangeVisibility(wm->getProcess()->GetLayer(i), false);
			}
			else if(alt)
			{
				struct ProcessLayer * current_layer = NULL;
				int i=-1;
				current_layer = wm->getProcess()->GetLayer(); // First layer
				while(current_layer)
				{
					if(!current_layer->Show)
						i = current_layer->Index;
					current_layer = current_layer->Next;
				}
				if(i > -1)
					wm->getProcess()->ChangeVisibility(wm->getProcess()->GetLayer(i), true);
			}
			else if (shift) {
				_vy2 -= -5.0f*_speed_factor / 10.0;
			} 
			else
				_vy2 -= -5.0f*_speed_factor;
		}
	}

	if( event == 1 ) /* mouse button up */
	{
		/*
		if (_mouse_control && !_temp_mouse)
			_vz = 0.0f;*/
		if(data == 0 && _temp_mouse && _mouse_control) // Left button
		{
			if(!_first_move)
			{
				
				wm->show_mouse();
			}
			_mouse_control = false;
			
		}
		if(data == 1 && _temp_mouse && _mouse_control2) // Right button
		{
			if(!_first_move)
			{
				
				wm->show_mouse();
			}
			_mouse_control2 = false;
			
		}
		
	}
	if( event == 2) /* mouse move */
	{
		if(!_first_move || abs(xpos-mouse_prev_x)>10 || abs(ypos-mouse_prev_y)>10)
		{

			if(_mouse_control)
			{
				if(_first_move)
				{
					wm->hide_mouse();
					//xpos = mouse_prev_x = wm->screenWidth/2;
					//ypos = mouse_prev_y = wm->screenHeight/2;
					//move_mouse( wm->screenWidth / 2, wm->screenHeight / 2 );
					_first_move = false;
				
				}
				else
				{
					if (shift) {
						_vry += 128.0f * _speed_factor/10.0 * (GLfloat)(xpos - mouse_prev_x) / wm->screenWidth;
						_vrx += 128.0f * _speed_factor/10.0 * (GLfloat)(ypos - mouse_prev_y) / wm->screenHeight;
					}
					else {
						//#ifdef HAVE_GL_GLX_H
						_vry += 128.0f * _speed_factor * (GLfloat)(xpos - mouse_prev_x) / wm->screenWidth;
						_vrx += 128.0f * _speed_factor * (GLfloat)(ypos - mouse_prev_y) / wm->screenHeight;
						//#else
						//_vry += 128.0f * _speed_factor * (GLfloat) ( xpos - wm->screenWidth / 2 ) / wm->screenWidth;
						//_vrx += 128.0f * _speed_factor * (GLfloat) ( ypos - wm->screenHeight / 2 ) / wm->screenHeight;
						//#endif
					}
					if( wm->timer( _mt, 0 ) > 0.01 )
					{
						wm->timer( _mt, 1 );
						wm->move_mouse( mouse_prev_x, mouse_prev_y );
					}
				}
			}
			else if(_mouse_control2)
			{
				if(_first_move)
				{
					wm->hide_mouse();
					//xpos = mouse_prev_x = wm->screenWidth/2;
					//ypos = mouse_prev_y = wm->screenHeight/2;
					//move_mouse( wm->screenWidth / 2, wm->screenHeight / 2 );
					_first_move = false;
				}
				else
				{
			
					if (shift) {
						_vx2 -= 32.0f * _speed_factor / 10.0 * (GLfloat)(xpos - mouse_prev_x) / wm->screenWidth;
						_vz2 -= 32.0f * _speed_factor / 10.0 * (GLfloat)(ypos - mouse_prev_y) / wm->screenHeight;
					} else {
						_vx2 -= 32.0f * _speed_factor * (GLfloat)(xpos - mouse_prev_x) / wm->screenWidth;
						_vz2 -= 32.0f * _speed_factor * (GLfloat)(ypos - mouse_prev_y) / wm->screenHeight;
					}

					if( wm->timer( _mt, 0 ) > 0.01 )
					{
						wm->timer( _mt, 1 );
						wm->move_mouse( mouse_prev_x, mouse_prev_y );
					}
				}
			}
		}
	}
	if( event == 3 ) // Key down
	{

		switch (data) {
		case KEY_W:
		case KEY_UP:
			_vz = 5.0f*_speed_factor;
			if (shift) {
				_vz = _vz / 10.0;
			}
			break;
		case KEY_S:
		case KEY_DOWN:
			_vz = -5.0f*_speed_factor;
			if (shift) {
				_vz = _vz / 10.0;
			}
			break;
		case KEY_A:
		case KEY_LEFT:
			_vx = 5.0f*_speed_factor;
			if (shift) {
				_vx = _vx / 10.0;
			}
			break;
		case KEY_D:
		case KEY_RIGHT:
			_vx = -5.0f*_speed_factor;
			if (shift) {
				_vx = _vx / 10.0;
			}
			break;
		case KEY_Z:
			_vy = 5.0f*_speed_factor;
			if (shift) {
				_vz = _vz / 10.0;
			}
			break;
		case KEY_Q:
			_vy = -5.0f*_speed_factor;
			if (shift) {
				_vz = _vz / 10.0;
			}
			break;
		case KEY_F8:
			if(!renderer.offlineFramebuffer(capture_width, capture_height))
			{
				// Just capture framebuffer
				capture_width = wm->screenWidth;
				capture_height = wm->screenHeight;
			}
			gl_draw_world(capture_width, capture_height, true);
			renderer.blitFramebuffer(capture_width, capture_height);
			capture_timer = 1.0; // 1 second
			renderer.tgaGrabScreenSeries((char *)"gds/screenshot");
			renderer.onlineFramebuffer();
			break;
		case KEY_LSHIFT:
		case KEY_RSHIFT:
			_turbo = true;
			break;
		case KEY_LCTRL:
		case KEY_RCTRL:
			_slow = true;
			break;
		case KEY_E:
            exploded_view = !exploded_view;
			if(exploded_view)
				exploded_accel += 4.0f;
			else
				exploded_accel -= 4.0f;

            break;
		case KEY_F:
			if (!ui_highlight->GetState())
			{
				Output TopView2Geo;
				TopView2Geo.SaveToGEO(_topcell, true);
				v_printf(0, "Geo File Done\n");
			}
			break;
		default:
			break;
		}
	}

	if( event == 4 ) // Key up
	{
		numkey = 0;

		switch (data) {
		case KEY_W:
		case KEY_S:
		case KEY_UP:
		case KEY_DOWN:
			_vz = 0.0f;
			break;
		case KEY_A:
		case KEY_D:
		case KEY_LEFT:
		case KEY_RIGHT:
			_vx = 0.0f;
			break;
		case KEY_Z:
		case KEY_Q:
			_vy = 0.0f;
			break;
		case KEY_LSHIFT:
		case KEY_RSHIFT:
			_turbo = false;
			break;
		case KEY_LCTRL:
		case KEY_RCTRL:
			_slow = false;
			break;
        case KEY_ESC:
            // Disable all UI elements
            for(list<UIElement*>::iterator l = ui_elements.begin(); l!= ui_elements.end(); l++)
				(*l)->Disable();

            // Mouse control
            if(_mouse_control)
            {
                wm->show_mouse();
                _mouse_control = 0;
            }
            break;
		case KEY_P:
			_perfmon = !_perfmon;
			break;
		case KEY_M:
			_temp_mouse = false;
			_mouse_control = !_mouse_control;
			if (_mouse_control) {
				wm->hide_mouse();

				// Move mouse to center of window
				wm->move_mouse( wm->screenWidth / 2, wm->screenHeight / 2 ); 
				mouse_prev_x = wm->screenWidth/2;
				mouse_prev_y = wm->screenHeight/2;
			}
			else {
				wm->show_mouse();
			}
			break;
		case KEY_R:
			_rx = _vrx = 0.0f;
			_ry = _vry = 0.0f;

			init_viewposition();
			if(_mouse_control)
			{
				wm->show_mouse();
				_mouse_control = 0;
			}
			break;
        case KEY_J:
                if(control)
                    renderer.wireframe = !renderer.wireframe;
                break;
		default:
			break;
		}
	}
}

void GDSParse_ogl::display_perfmon()
{
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();

	glLoadIdentity();
	glOrtho(0.0, (GLdouble) wm->screenWidth, 0.0, (GLdouble) wm->screenHeight, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw border
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
	gl_square(wm->screenWidth - 270.0f, wm->screenHeight - 20.0f, wm->screenWidth - 20.0f, wm->screenHeight - 70.0f, 1);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	gl_square(wm->screenWidth - 270.0f, wm->screenHeight - 20.0f, wm->screenWidth - 20.0f, wm->screenHeight - 70.0f, 0);

	// Text
	gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 250, wm->screenHeight - 40, "FPS:            %5.1f", drawfps);
	if(total_tris < 1000)
		gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 250, wm->screenHeight - 60, "Triangles: %10d", total_tris);
	else if(total_tris < 1000000)
		gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 250, wm->screenHeight - 60, "Triangles: %9dK", total_tris/1000);
	else if(total_tris < 1000000000)
		gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 250, wm->screenHeight - 60, "Triangles: %9.1fM", total_tris/1000000.0f);
	else
		gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 250, wm->screenHeight - 60, "Triangles: %9dG", total_tris/1000000000);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FOG);

}

void GDSParse_ogl::gl_square(float x1, float y1, float x2, float y2, int filled)
{
	if (filled) {
		glBegin(GL_QUADS);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glEnd();
	}
	else {
		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(x1, y2);
		glVertex2f(x1, y1);
		glEnd();
	}
}

void GDSParse_ogl::init_viewposition()
{
	if( _Objects)
	{
		GDSBB Boundary = _topcell->GetTotalBoundary();
		
		float half_widthX = (Boundary.max.X - Boundary.min.X)/2;
		float half_widthY = (Boundary.max.Y - Boundary.min.Y)/2;
		float centreX = half_widthX + Boundary.min.X;
		float centreY = half_widthY + Boundary.min.Y;

		float distance;
		if(half_widthX > half_widthY){
			distance = half_widthX * 1.8f;
		}else{
			distance = half_widthY * 1.8f;
		}

		// This boundary stuff can be done somewhere else..
		_xmin = Boundary.min.X;
		_xmax = Boundary.max.X;
		_ymin = Boundary.min.Y;
		_ymax = Boundary.max.Y;

		_x = centreX;
		_y = centreY;
		_z = distance*1.5f;	

		_rx = _vrx = 0.0f;
		_ry = _vry = 0.0f;
	} /* end init view position */
}

void GDSParse_ogl::LockOnUIElement(UIElement* lock)
{
	//Disable all uielements except for the lock
	for(list<UIElement*>::iterator l = ui_elements.begin(); l!= ui_elements.end(); l++)
	{
		if(*l == lock)
			continue;
		(*l)->Disable();
	}
}
