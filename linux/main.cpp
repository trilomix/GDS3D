//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2021 Bertrand Pigeard
//  Copyright (C) 2012 IC-Design Group, University of Twente.
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


#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <GL/glx.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "main.h"

EventKey Wm_X11::translateKey(int key)
{
	switch(key)
	{
	case 0x61:
		return KEY_A;
	case 0x62:
		return KEY_B;
	case 0x63:
		return KEY_C;
	case 0x64:
		return KEY_D;
	case 0x65:
		return KEY_E;
	case 0x66:
		return KEY_F;
	case 0x67:
		return KEY_G;
	case 0x68:
		return KEY_H;
	case 0x69:
		return KEY_I;
	case 0x6A:
		return KEY_J;
	case 0x6B:
		return KEY_K;
	case 0x6C:
		return KEY_L;
	case 0x6D:
		return KEY_M;
	case 0x6E:
		return KEY_N;
	case 0x6F:
		return KEY_O;
	case 0x70:
		return KEY_P;
	case 0x71:
		return KEY_Q;
	case 0x72:
		return KEY_R;
	case 0x73:
		return KEY_S;
	case 0x74:
		return KEY_T;
	case 0x75:
		return KEY_U;
	case 0x76:
		return KEY_V;
	case 0x77:
		return KEY_W;
	case 0x78:
		return KEY_X;
	case 0x79:
		return KEY_Y;
	case 0x7A:
		return KEY_Z;
	case 0x30:
		return KEY_0;
	case 0x31:
		return KEY_1;
	case 0x32:
		return KEY_2;
	case 0x33:
		return KEY_3;
	case 0x34:
		return KEY_4;
	case 0x35:
		return KEY_5;
	case 0x36:
		return KEY_6;
	case 0x37:
		return KEY_7;
	case 0x38:
		return KEY_8;
	case 0x39:
		return KEY_9;
	case XK_minus:
		return KEY_MINUS;
	caseXK_equal:
		return KEY_PLUS;
	case XK_comma:
		return KEY_COMMA;
	case XK_period:
		return KEY_PERIOD;
	case XK_semicolon:
		return KEY_COLON;
	case XK_slash:
		return KEY_SLASH;
	case XK_dead_grave:
		return KEY_TILDE;
	case XK_bracketleft:
		return KEY_BRACKET_O;
	case XK_backslash:
		return KEY_BACKSLASH;
	case XK_bracketright:
		return KEY_BRACKET_C;
	case XK_dead_acute:
		return KEY_QUOTE;
	case XK_Shift_L:
		return KEY_LSHIFT;
	case XK_Shift_R:
		return KEY_RSHIFT;
	case XK_Control_L:
		return KEY_LCTRL;
	case XK_Control_R:
		return KEY_RCTRL;
	case XK_Alt_L:
		return KEY_LALT;
	case XK_Alt_R:
		return KEY_RALT;
	case XK_Left:
		return KEY_LEFT;
	case XK_Right:
		return KEY_RIGHT;
	case XK_Up:
		return KEY_UP;
	case XK_Down:
		return KEY_DOWN;
	case XK_F1:
		return KEY_F1;
	case XK_F2:
		return KEY_F2;
	case XK_F3:
		return KEY_F3;
	case XK_F4:
		return KEY_F4;
	case XK_F5:
		return KEY_F5;
	case XK_F6:
		return KEY_F6;
	case XK_F7:
		return KEY_F7;
	case XK_F8:
		return KEY_F8;
	case XK_F9:
		return KEY_F9;
	case XK_F10:
		return KEY_F10;
	case XK_F11:
		return KEY_F11;
	case XK_F12:
		return KEY_F12;
	case XK_KP_Insert:
		return KEY_N0;
	case XK_KP_End:
		return KEY_N1;
	case XK_KP_Down:
		return KEY_N2;
	case XK_KP_Page_Down:
		return KEY_N3;
	case XK_KP_Left:
		return KEY_N4;
	case XK_KP_Begin:
		return KEY_N5;
	case XK_KP_Right:
		return KEY_N6;
	case XK_KP_Home:
		return KEY_N7;
	case XK_KP_Up:
		return KEY_N8;
	case XK_KP_Prior:
		return KEY_N9;
	case XK_KP_Add:
		return KEY_N_ADD;
	case XK_KP_Subtract:
		return KEY_N_SUBTRACT;
	case XK_KP_Multiply:
		return KEY_N_MULTIPLY;
	case XK_KP_Divide:
		return KEY_N_DIVIDE;
	case XK_KP_Delete:
		return KEY_N_DECIMAL;
	case XK_KP_Enter:
	case XK_Return:
		return KEY_ENTER;
	case XK_space:
		return KEY_SPACE;
	case XK_Tab:
		return KEY_TAB;
	case XK_BackSpace:
		return KEY_BACKSPACE;
	case XK_Escape:
		return KEY_ESC;
	case XK_Delete:
		return KEY_DEL;
	case XK_Insert:
		return KEY_INS;
	case XK_End:
		return KEY_END;
	case XK_Home:
		return KEY_HOME;
	case XK_Prior:
		return KEY_PGUP;
	case XK_Next:
		return KEY_PGDOWN;
	case XK_Pause:
		return KEY_PAUSE;
	case XK_Caps_Lock:
		return KEY_CAPSLOCK;
	case XK_Num_Lock:
		return KEY_NUMLOCK;
	case XK_Scroll_Lock:
		return KEY_SCROLLOCK; 
	}

	return KEY_NONE; // This should never happen..
}

Wm_X11::Wm_X11()
{
	active = false;
	run = false;
}

Wm_X11::~Wm_X11()
{
	for(unsigned int i=0;i<timers.size();i++)
		delete timers[i];
}

void Wm_X11::gl_finish()
{
	// Swap buffers
	#ifndef __APPLE__
		glFinish(); // Necessary for client-server synchronization over the network
	#endif
	glXSwapBuffers( dpy, win );
}

bool Wm_X11::hide_mouse( void )
{
	if(WindowManager::hide_mouse()) // Base class behavior
		XDefineCursor( dpy, win, null_cursor );
	else
		return false; // Wrong!

	return true;
}

bool Wm_X11::show_mouse( void )
{
	if(WindowManager::show_mouse()) // Base class behavior
		XDefineCursor( dpy, win, XC_X_cursor );
	else
		return false; // Wrong!

	return true;
}

void Wm_X11::change_cursor( int shape )
{
	//if (!_mouse_control && !_mouse_control2) {
	if(!hidden_mouse)
	{
		Cursor cursor;
		
		switch (shape) {
			case 1: // Drag
				cursor = XCreateFontCursor(dpy, XC_fleur);
				break;
			case 2: // Horizontal resize
				cursor = XCreateFontCursor(dpy, XC_sb_h_double_arrow);
				break;
			case 3: // Vertical resize
				cursor = XCreateFontCursor(dpy, XC_sb_v_double_arrow);
				break;
			case 4: // Diagonal resize 1 (left top)
				cursor = XCreateFontCursor(dpy, XC_top_left_corner);
				break;
			case 5: // Diagonal resize 2 (right bottom)
				cursor = XCreateFontCursor(dpy, XC_bottom_right_corner);
				break;
			case 6: // Diagonal resize 3 (top right)
				cursor = XCreateFontCursor(dpy, XC_top_right_corner);
				break;
			case 7: // Diagonal resize 4 (bottom left)
				cursor = XCreateFontCursor(dpy, XC_bottom_left_corner);
				break;
			default: // Default
				cursor = XC_X_cursor;
				break;
		}

		XDefineCursor( dpy, win, cursor );
	}
}

void Wm_X11::move_mouse( int x, int y )
{
	XWarpPointer( dpy, None, win, 0, 0, 0, 0, x, y );
}

GLuint Wm_X11::get_font()
{
	return font;
}

float Wm_X11::timer( struct htime *t, int reset )
{
	float delta;
	struct timeval offset;
	gettimeofday( &offset, NULL );

	htime_X11 *t2 = (htime_X11*) t; // Static cast

	delta = (float) ( offset.tv_sec - t2->start.tv_sec ) +
		(float) ( offset.tv_usec - t2->start.tv_usec ) / 1e6;

	if( reset )
	{
		t2->start.tv_sec = offset.tv_sec;
		t2->start.tv_usec = offset.tv_usec;
	}

	return delta;
}

htime* Wm_X11::new_timer()
{
	htime_X11 *t = new htime_X11;

	timers.push_back(t);

	return t;
}

void Wm_X11::render_text(int x, int y, const char * text, VECTOR4D color)
{
	// Assumes projection matrix is glOrtho with correct screen size
	glColor4f( (GLfloat) color.x, (GLfloat) color.y, (GLfloat) color.z, (GLfloat) color.w);	
	glRasterPos2i( x, y );
	glListBase( font );
	glCallLists( strlen( text ), GL_UNSIGNED_BYTE, text );
}

bool Wm_X11::query_update(FILE *f)
{
	struct stat attrib;

	if(!fstat(fileno(f), &attrib))
	{
		//v_printf(1, "Statting file..\n");
		if(low_date_time[f] != attrib.st_mtime && attrib.st_size > 0) // Is the file updated?
		{
			low_date_time[f] = attrib.st_mtime;
			return true;
		}
	}

	return false;
}

int Wm_X11::main(int argc, char *argv[])
{
	// Parse commandlines
	if(!commandLineParameters(argc, argv))
		return 0;

	// Create window
	int vi_attr[] =
	{
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
//        GLX_SAMPLE_BUFFERS  , 1,
//        GLX_SAMPLES         , 2,
		None
	};

	XVisualInfo *vi;
    Window root_win;
	XWindowAttributes win_attr;
	XSetWindowAttributes set_attr;
	XFontStruct *fixed;
	XColor black =
	{
		0, 0, 0, 0, 0, 0
	};

	if( ( dpy = XOpenDisplay( NULL ) ) == NULL )
	{
		fprintf( stderr, "XOpenDisplay failed\n" );
		return( 1 );
	}

	if( ( vi = glXChooseVisual( dpy, DefaultScreen( dpy ),
		vi_attr ) ) == NULL )
	{
		fprintf( stderr, "glXChooseVisual failed\n" );
		XCloseDisplay( dpy );
		return( 1 );
	}

	root_win = RootWindow( dpy, vi->screen );

	XGetWindowAttributes( dpy, root_win, &win_attr );

    //resize(( fullscreen ) ? win_attr.width : 1024, ( fullscreen ) ? win_attr.height : 768);
	screenWidth = 1024;
	screenHeight = 768;

	set_attr.border_pixel = 0;

	set_attr.colormap =
		XCreateColormap( dpy, root_win, vi->visual, AllocNone );

	set_attr.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask |
		ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

	set_attr.override_redirect = ( ( fullscreen ) ? True : False );

	win =
		XCreateWindow(
		dpy, root_win, 0, 0, screenWidth, screenHeight, 0, vi->depth,
		InputOutput, vi->visual, CWBorderPixel | CWColormap |
		CWEventMask | CWOverrideRedirect, &set_attr );

	// Window Title
	char AppTitle[1024];
	if(assembly)
		sprintf(AppTitle, "%s  -  %s  -  IC Design Group, University of Twente", GDS3D_VERSION, filename);
	else
		sprintf(AppTitle, "%s - %s - %s - IC Design Group, University of Twente", GDS3D_VERSION, filename, techname);
	XStoreName( dpy, win, AppTitle );
	XMapWindow( dpy, win );

    Status stat;
    
	if( fullscreen )
	{
		XGrabKeyboard( dpy, win, True, GrabModeAsync,
			GrabModeAsync, CurrentTime );
	}
	else
	{
		wmDelete = XInternAtom( dpy, "WM_DELETE_WINDOW", False );
		stat = XSetWMProtocols( dpy, win, &wmDelete, 1 );
	}

	if( ( ctx = glXCreateContext( dpy, vi, NULL, True ) ) == NULL )
	{
		fprintf( stderr, "glXCreateContext failed\n" );
		XDestroyWindow( dpy, win );
		XCloseDisplay( dpy );
        XFree(vi);
		return( 1 );
	}

	if( glXMakeCurrent( dpy, win, ctx ) == False )
	{
		fprintf( stderr, "glXMakeCurrent failed\n" );
		glXDestroyContext( dpy, ctx );
		XDestroyWindow( dpy, win );
		XCloseDisplay( dpy );
        XFree(vi);
		return( 1 );
	}

	font = glGenLists( 256 );

	fixed = XLoadQueryFont(
		dpy, "-misc-fixed-medium-r-*-*-20-*-*-*-*-*-*-*" );

	null_cursor = XCreateGlyphCursor(
		dpy, fixed->fid, fixed->fid, ' ', ' ', &black, &black );

	glXUseXFont( fixed->fid, 0, 256, font );

	XFreeFont( dpy, fixed );
	XFree(vi);

	// Main loop
	XEvent event2;
	bool control;
	bool shift;
    bool alt;

	resize(screenWidth, screenHeight);
	init();

	run = true;

	 // register interest in the delete window message
        Atom wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);

		while( run){
			if( active ){
				draw();
				gl_finish();
			}else{
				XPeekEvent( dpy, &event2 );
			}

			while( XPending( dpy ) ){
				XNextEvent( dpy, &event2 );

				switch( event2.type ){
				case ButtonPress:
					{
						int x = event2.xmotion.x,
							y = event2.xmotion.y;
						
						control = event2.xbutton.state & ControlMask;
						shift = event2.xbutton.state & ShiftMask;
						alt = event2.xkey.state & Mod1Mask;
						
						switch( event2.xbutton.button )
						{
							case Button1: event( 0, 0, x, y, shift, control, alt ); break; // Left button
							case Button3: event( 0, 1, x, y, shift, control, alt ); break; // Right button
							case Button4: event( 0, 2, x, y, shift, control, alt ); break; // Mousewheel up
							case Button5: event( 0, 3, x, y, shift, control, alt ); break; // Mousewheel down
						}

						break;
					}

				case ButtonRelease:
					{
						int x = event2.xmotion.x,
							y = event2.xmotion.y;

						control = event2.xbutton.state & ControlMask;
						shift = event2.xbutton.state & ShiftMask;
						alt = event2.xkey.state & Mod1Mask;

						switch( event2.xbutton.button )
						{
						case Button1: event( 1, 0, x, y , shift, control, alt ); break;
						case Button3: event( 1, 1, x, y, shift, control, alt  ); break;
						}

						break;
					}

				case MotionNotify:
					{
						int x = event2.xmotion.x, y = event2.xmotion.y;

						control = event2.xbutton.state & ControlMask;
						shift = event2.xbutton.state & ShiftMask;
						alt = event2.xkey.state & Mod1Mask;

						switch( event2.xbutton.button )
						{
							case Button1: event( 2, 0, x, y, shift, control, alt  ); break;
							case Button3: event( 2, 1, x, y, shift, control, alt  ); break;
							default: event( 2, -1, x, y, shift, control, alt  ); break;
						}
						break;
					}

				case KeyPress:
					{
						int key = XLookupKeysym( &event2.xkey, 0 );

						control = event2.xbutton.state & ControlMask;
						shift = event2.xbutton.state & ShiftMask;
						alt = event2.xkey.state & Mod1Mask;

						control = event2.xbutton.state & ControlMask;
						shift = event2.xbutton.state & ShiftMask;
						alt = (event2.xkey.state & Mod1Mask) | (event2.xkey.state & Mod3Mask);
                        
						event( 3, translateKey(key), -1, -1, shift, control, alt );
						break;
					}

				case KeyRelease:
					{
						int key = XLookupKeysym( &event2.xkey, 0 );

						control = event2.xbutton.state & ControlMask;
						shift = event2.xbutton.state & ShiftMask;
						alt = event2.xkey.state & Mod1Mask;
                        
                        control = event2.xbutton.state & ControlMask;
						shift = event2.xbutton.state & ShiftMask;
						alt = (event2.xkey.state & Mod1Mask) | (event2.xkey.state & Mod3Mask);

                        event( 4, translateKey(key), -1, -1, shift, control, alt  );
						break;
					}

				case UnmapNotify: active = 0; break;
				case MapNotify: active = 1; break;

				case ConfigureNotify:
					{
						resize(event2.xconfigure.width, event2.xconfigure.height);
						break;
					}

                case DestroyNotify:
				case ClientMessage:
					{
						//if( event.xclient.data.l[0] == (int) wmDelete || (Atom)event.xclient.data.l[0] == wm_delete_window)
						//{
							active = run = 0;
						//}
						break;
					}

				case ReparentNotify: break;

				default:
					{
						printf( "caught unknown event, type %d\n",
							event2.type );
						break;
					}
				}
			}
		}

		glXMakeCurrent( dpy, None, NULL );
		glXDestroyContext( dpy, ctx );
		XDestroyWindow( dpy, win );
		XCloseDisplay( dpy );
}

int main(int argc, char *argv[])
{
	// Create windowmanager
	 Wm_X11 *root = new Wm_X11();
	 wm = root;

	 // Run main loop
	 if(root->main(argc, argv))
		 v_printf(1, "\nNormal exit.\n\n");


	 // Delete windowmanager
	 delete wm;
}
