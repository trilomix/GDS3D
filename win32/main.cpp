//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, http://icd.el.utwente.nl
//  Based on code by Roger Light, http://atchoo.org/gds2pov/
//  
//  Copyright (C) 2012 IC-Design Group, University of Twente.
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

#include <windows.h>
#include <WindowsX.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "main.h"

EventKey Wm_Win32::translateKey(WPARAM key)
{
	switch(key)
	{
	case 0x41:
		return KEY_A;
	case 0x42:
		return KEY_B;
	case 0x43:
		return KEY_C;
	case 0x44:
		return KEY_D;
	case 0x45:
		return KEY_E;
	case 0x46:
		return KEY_F;
	case 0x47:
		return KEY_G;
	case 0x48:
		return KEY_H;
	case 0x49:
		return KEY_I;
	case 0x4A:
		return KEY_J;
	case 0x4B:
		return KEY_K;
	case 0x4C:
		return KEY_L;
	case 0x4D:
		return KEY_M;
	case 0x4E:
		return KEY_N;
	case 0x4F:
		return KEY_O;
	case 0x50:
		return KEY_P;
	case 0x51:
		return KEY_Q;
	case 0x52:
		return KEY_R;
	case 0x53:
		return KEY_S;
	case 0x54:
		return KEY_T;
	case 0x55:
		return KEY_U;
	case 0x56:
		return KEY_V;
	case 0x57:
		return KEY_W;
	case 0x58:
		return KEY_X;
	case 0x59:
		return KEY_Y;
	case 0x5A:
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
	case VK_OEM_MINUS:
		return KEY_MINUS;
	case VK_OEM_PLUS:
		return KEY_PLUS;
	case VK_OEM_COMMA:
		return KEY_COMMA;
	case VK_OEM_PERIOD:
		return KEY_PERIOD;
	case 0xBA:
		return KEY_COLON;
	case 0xBF:
		return KEY_SLASH;
	case 0xC0:
		return KEY_TILDE;
	case 0xDB:
		return KEY_BRACKET_O;
	case 0xDC:
		return KEY_BACKSLASH;
	case 0xDD:
		return KEY_BRACKET_C;
	case 0xDE:
		return KEY_QUOTE;
	case VK_SHIFT:
		return KEY_LSHIFT;
	case VK_RSHIFT:
		return KEY_RSHIFT;
	case VK_CONTROL:
		return KEY_LCTRL;
	case VK_RCONTROL:
		return KEY_RCTRL;
	case VK_MENU:
		return KEY_LALT;
	case VK_RMENU:
		return KEY_RALT;
	case VK_LEFT:
		return KEY_LEFT;
	case VK_RIGHT:
		return KEY_RIGHT;
	case VK_UP:
		return KEY_UP;
	case VK_DOWN:
		return KEY_DOWN;
	case VK_F1:
		return KEY_F1;
	case VK_F2:
		return KEY_F2;
	case VK_F3:
		return KEY_F3;
	case VK_F4:
		return KEY_F4;
	case VK_F5:
		return KEY_F5;
	case VK_F6:
		return KEY_F6;
	case VK_F7:
		return KEY_F7;
	case VK_F8:
		return KEY_F8;
	case VK_F9:
		return KEY_F9;
	case VK_F10:
		return KEY_F10;
	case VK_F11:
		return KEY_F11;
	case VK_F12:
		return KEY_F12;
	case VK_NUMPAD0:
		return KEY_N0;
	case VK_NUMPAD1:
		return KEY_N1;
	case VK_NUMPAD2:
		return KEY_N2;
	case VK_NUMPAD3:
		return KEY_N3;
	case VK_NUMPAD4:
		return KEY_N4;
	case VK_NUMPAD5:
		return KEY_N5;
	case VK_NUMPAD6:
		return KEY_N6;
	case VK_NUMPAD7:
		return KEY_N7;
	case VK_NUMPAD8:
		return KEY_N8;
	case VK_NUMPAD9:
		return KEY_N9;
	case VK_ADD:
		return KEY_N_ADD;
	case VK_SUBTRACT:
		return KEY_N_SUBTRACT;
	case VK_MULTIPLY:
		return KEY_N_MULTIPLY;
	case VK_DIVIDE:
		return KEY_N_DIVIDE;
	case VK_DECIMAL:
		return KEY_N_DECIMAL;
	case VK_RETURN:
		return KEY_ENTER;
	case VK_SPACE:
		return KEY_SPACE;
	case VK_TAB:
		return KEY_TAB;
	case VK_BACK:
		return KEY_BACKSPACE;
	case VK_ESCAPE:
		return KEY_ESC;
	case VK_DELETE:
		return KEY_DEL;
	case VK_INSERT:
		return KEY_INS;
	case VK_END:
		return KEY_END;
	case VK_HOME:
		return KEY_HOME;
	case VK_PRIOR:
		return KEY_PGUP;
	case VK_NEXT:
		return KEY_PGDOWN;
	case VK_PAUSE:
		return KEY_PAUSE;
	case VK_CAPITAL:
		return KEY_CAPSLOCK;
	case VK_NUMLOCK:
		return KEY_NUMLOCK;
	case VK_SCROLL:
		return KEY_SCROLLOCK; 
	}

	return KEY_NONE; // This should never happen..
}

Wm_Win32::Wm_Win32()
{
	active = false;
	run = false;
}

Wm_Win32::~Wm_Win32()
{
	for(unsigned int i=0;i<timers.size();i++)
		delete timers[i];
}

void  Wm_Win32::gl_finish()
{
	// Swap buffers
	SwapBuffers( hDC );
}

bool Wm_Win32::hide_mouse( void )
{
	if(WindowManager::hide_mouse()) // Base class behavior
		ShowCursor(FALSE);
	else
		return false; // Wrong!

	return true;
}

bool Wm_Win32::show_mouse( void )
{
	if(WindowManager::show_mouse()) // Base class behavior
		ShowCursor(TRUE);
	else
		return false; // Wrong!

	return true;
}

void Wm_Win32::change_cursor( int shape )
{
	HCURSOR hCursor;

	switch (shape) {
		case 1: // Drag
			hCursor = LoadCursor(NULL, IDC_SIZEALL);
			break;
		case 2: // Horizontal resize
			hCursor = LoadCursor(NULL, IDC_SIZEWE);
			break;
		case 3: // Vertical resize
			hCursor = LoadCursor(NULL, IDC_SIZENS);
			break;
		case 4: // Diagonal resize 1
		case 5:
			hCursor = LoadCursor(NULL, IDC_SIZENWSE);
			break;
		case 6: // Diagonal resize 2
		case 7:
			hCursor = LoadCursor(NULL, IDC_SIZENESW);
			break;
		default: // Default
			hCursor = LoadCursor(NULL, IDC_ARROW);
			break;
	}
	
	SetCursor(hCursor);
}

void Wm_Win32::move_mouse( int x, int y )
{
	POINT p;
	p.x = x;
	p.y = y;
	ClientToScreen(hWnd, &p);
	SetCursorPos(p.x, p.y);
}

GLuint Wm_Win32::get_font()
{
	return font;
}

float Wm_Win32::timer( htime *t, int reset )
{
	float delta;

	htime_Win32* t2 = (htime_Win32*) t; // Static cast

	LARGE_INTEGER offset;

	QueryPerformanceCounter(&offset);

	if(t2->hfreq.QuadPart){
		delta = (float) (offset.QuadPart - t2->start.QuadPart) /
			(float) t2->hfreq.QuadPart;
	}

	if(reset){
		QueryPerformanceFrequency(&t2->hfreq);
		QueryPerformanceCounter(&t2->start);
	}

	return( delta );
}

htime* Wm_Win32::new_timer()
{
	htime_Win32 *t = new htime_Win32;

	timers.push_back(t);

	return t;
}

void Wm_Win32::render_text(int x, int y, const char * text, VECTOR4D color)
{
	// Assumes projection matrix is glOrtho with correct screen size
	glColor4f( (GLfloat) color.x, (GLfloat) color.y, (GLfloat) color.z, (GLfloat) color.w );	
	glRasterPos2i( x, y );
	glListBase( font );
	glCallLists( (GLsizei) strlen( text ), GL_UNSIGNED_BYTE, text );
}

bool Wm_Win32::query_update(FILE *f)
{
	struct stat attrib;

	if(!fstat(_fileno(f), &attrib))
	{
		if(low_date_time[f] != attrib.st_mtime && attrib.st_size > 0) // Is the file updated?
		{
			low_date_time[f] = attrib.st_mtime;
			return true;
		}
	}

	return false;
}

LRESULT CALLBACK Wm_Win32::msgRouter(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        Wm_Win32 *wnd = 0;        // pointer to the window that should receive the message

        if (message == WM_NCCREATE || message == WM_CREATE) {  
                // if this message gets sent then a new window has just been created,
                // so we'll asociate its handle with its AbstractWindow instance pointer
            
# if !defined( _WIN64 )
                ::SetWindowLong (hwnd, GWL_USERDATA, long((LPCREATESTRUCT(lParam))->lpCreateParams));
# else
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, DWORD_PTR ((LPCREATESTRUCT(lParam))->lpCreateParams));
# endif
        }

        // --- messages different from WN_NCCREATE / or WM_NCCREATE was just processed ---
        // we retrieve the instance of AbstractWindow that corresponds to the destination window's HWND
# if !defined( _WIN64 )
        wnd = (Wm_Win32 *) (::GetWindowLong (hwnd, GWL_USERDATA));
# else
		wnd = (Wm_Win32 *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
# endif
        // we then route the message to the wndProc method defined in the derived AbstractWindow class
        if (wnd)
                return wnd->WindowProc (hwnd, message, wParam, lParam);
        else
                // for messages that arrive prior to WM_NCCREATE
                // and the HWND <-> AbstractWindow * association was not made
                return ::DefWindowProc (hwnd, message, wParam, lParam);
}

LRESULT CALLBACK Wm_Win32::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    bool control;
	bool shift;
    bool alt;

	shift = (GetKeyState(VK_SHIFT) & 0x80) == 128;
	control = (GetKeyState(VK_CONTROL) & 0x80) == 128;
	alt = (GetKeyState(VK_MENU) & 0x80) == 128;

	switch( uMsg )
    {
        case WM_CREATE:
        {
            PIXELFORMATDESCRIPTOR pfd =
            {
                sizeof( PIXELFORMATDESCRIPTOR ),
                1,
                    PFD_DRAW_TO_WINDOW |
                    PFD_SUPPORT_OPENGL |
                    PFD_DOUBLEBUFFER,
                PFD_TYPE_RGBA,
                32,
                0, 0, 0, 0, 0, 0,
                0,
                0,
                0,
                0, 0, 0, 0,
                //16,
				24,
                0,
                0,
                PFD_MAIN_PLANE,
                0,
                0, 0, 0
            };
            int pixelformat;

            hRC = NULL;

            if( ! ( hDC = GetDC( hWnd ) ) )
            {
                PostQuitMessage( 0 );
                break;
            }

            if( ! ( pixelformat = ChoosePixelFormat( hDC, &pfd ) ) )
            {
                PostQuitMessage( 0 );
                break;
            }

            if( ! SetPixelFormat( hDC, pixelformat, &pfd ) )
            {
                PostQuitMessage( 0 );
                break;
            }

			PIXELFORMATDESCRIPTOR pfd_real;
			DescribePixelFormat(hDC, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_real);

            if( ! ( hRC = wglCreateContext( hDC ) ) )
            {
                PostQuitMessage( 0 );
                break;
            }
            
            if( ! wglMakeCurrent( hDC, hRC ) )
            {
                PostQuitMessage( 0 );
                break;
            }

            font = glGenLists( 256 );

            {
                HFONT courier = CreateFont( 20, 0, 0, 0, FW_MEDIUM, FALSE,
                                FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS,
                                CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                FF_DONTCARE | DEFAULT_PITCH, TEXT("Courier New") );

                SelectObject( hDC, courier );

                if( ! wglUseFontBitmaps( hDC, 0, 255, font ) )                
                    PostQuitMessage( 0 );
            }            
            break;
        }

		case WM_MOUSEWHEEL:
		{
			// For some reason the location of a scroll event is referenced to the screen's top left corner, rather than the window's
			POINT coords = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			ScreenToClient(hWnd, &coords);

			// Interpret mouse scrolls as a button press
			if ( GET_WHEEL_DELTA_WPARAM(wParam) >= 120 ) // Scroll up
				event( EVENT_BUTTON_DOWN, 2, coords.x, coords.y, shift, control, alt );
			else if ( GET_WHEEL_DELTA_WPARAM(wParam) <= -120 ) // Scroll down
				event( EVENT_BUTTON_DOWN, 3, coords.x, coords.y, shift, control, alt  );
			
			break;
		}
		case WM_LBUTTONDOWN:
        {
			SetCapture(hWnd);
			event( EVENT_BUTTON_DOWN, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , shift, control, alt  );
            break;
        }

        case WM_RBUTTONDOWN:
        {
			SetCapture(hWnd);
			event( EVENT_BUTTON_DOWN, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), shift, control, alt  );
            break;
        }

        case WM_LBUTTONUP:
        {
			ReleaseCapture();
            event( EVENT_BUTTON_UP, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), shift, control, alt   );
            break;
        }

        case WM_RBUTTONUP:
        {
			ReleaseCapture();
            event( EVENT_BUTTON_UP, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), shift, control, alt   );
            break;
        }

        case WM_MOUSEMOVE:
        {
            switch( wParam )
            {
                case MK_LBUTTON:

					event( EVENT_MOUSE_MOVE, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), shift, control, alt   );
                    break;

                case MK_RBUTTON:
            
					event( EVENT_MOUSE_MOVE, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), shift, control, alt   );
                    break;

                default:
					
					event( EVENT_MOUSE_MOVE, -1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), shift, control, alt   );
                    break;
            }

            break;
        }

        case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			event( EVENT_KEY_DOWN, translateKey(wParam), -1, -1, shift, control, alt  );
			break;
		}

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {            
            event( EVENT_KEY_UP,  translateKey(wParam), -1, -1, shift, control, alt  );
            break;
        }

        case WM_SIZE:
        {
            switch( wParam )
            {
                case SIZE_MINIMIZED:
					active = 0; 
					break;
                case SIZE_MAXIMIZED:
					active = 1; 
					break;
                case SIZE_RESTORED:  
					active = 1; 
					break;
            }
            resize(GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ));
            break;
        }

        case WM_CLOSE:
        {
            PostQuitMessage( 1 );
            break;
        }

        case WM_SYSCOMMAND:
        {
            switch( wParam )
            {
                case SC_MONITORPOWER:
                case SC_SCREENSAVE:

                    return 0;
            }
        }

        default:

            return( DefWindowProc( hWnd, uMsg, wParam, lParam ) );
    }

    return( 0 );
}

WPARAM WINAPI Wm_Win32::main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char ClassName[] = "opengl demo";
    WNDCLASSEX wcx;
    DWORD dwStyle;
    MSG msg;

	// Parse commandlines
	if(!commandLineParameters(__argc, __argv))
		return 0;

	// Create window
	 wcx.cbSize          = sizeof( WNDCLASSEX );
    wcx.style           = 0;
    wcx.lpfnWndProc     = Wm_Win32::msgRouter;
    wcx.cbClsExtra      = 0;
    wcx.cbWndExtra      = 0;
    wcx.hInstance       = hInstance;
    wcx.hIcon           = NULL;
    wcx.hCursor         = LoadCursor( NULL, IDC_ARROW );
	wcx.hbrBackground   = (HBRUSH) COLOR_BACKGROUND;
    wcx.lpszMenuName    = NULL;
    wcx.lpszClassName   = TEXT("GDS3D");
    wcx.hIconSm         = NULL;

    if( ! RegisterClassEx( &wcx ) )
        return( 0 );


        if( fullscreen )
        {
            if( ! ( hDC = GetDC( NULL ) ) )
                return( 0 );

            screenWidth  = GetDeviceCaps( hDC, HORZRES );
            screenHeight = GetDeviceCaps( hDC, VERTRES );

            ReleaseDC( NULL, hDC );

            dwStyle = WS_POPUP;
        }
        else
        {
            screenWidth  = 1024;
            screenHeight = 768;

            dwStyle = WS_OVERLAPPEDWINDOW;
        }

		char AppTitle[1024];
		if(assembly)
			sprintf(AppTitle, "%s  -  %s  -  IC Design Group, University of Twente", GDS3D_VERSION, filename);
		else
			sprintf(AppTitle, "%s  -  %s  -  %s  -  IC Design Group, University of Twente", GDS3D_VERSION, filename, techname);
		WCHAR widechar[256];
		MultiByteToWideChar( 0,0, AppTitle, 256, widechar, 256);
        if( ! ( hWnd = CreateWindow( TEXT("GDS3D"), widechar, dwStyle,
                        CW_USEDEFAULT, CW_USEDEFAULT, screenWidth, screenHeight,
                        NULL, NULL, hInstance, this ) ) )
        {
            return( 0 );
        }

		// Create the window, resize to the correct size and build all internal windows
        ShowWindow( hWnd, nCmdShow );
		resize(screenWidth, screenHeight);
		init();	

        run = 1;

        while( run )
        {
            if( active )
            {
                draw();
				gl_finish();
            }
            else
            {
				// Wait for message
                GetMessage( &msg, NULL, 0, 0 ); // BUG: Prevents window from restoring from minimized state
				TranslateMessage( &msg );
                DispatchMessage( &msg ); 
            }

            while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            {
                if( msg.message == WM_QUIT )
                {
                    run = 0;
                    break;
                }

                TranslateMessage( &msg );
                DispatchMessage( &msg ); 
            }
        }

        wglMakeCurrent( NULL, NULL );
        if( hDC ) ReleaseDC( hWnd, hDC );
        if( hRC ) wglDeleteContext( hRC );
        DestroyWindow( hWnd );

    return( msg.wParam );
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	  // redirect unbuffered STDOUT to the console
	 int hConHandle;
	 long lStdHandle;
	 FILE *fp;
	 

	 AttachConsole(ATTACH_PARENT_PROCESS);
	 lStdHandle = (unsigned long long)GetStdHandle(STD_OUTPUT_HANDLE);

	 if(lStdHandle)
	 {

		 hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

		 if(hConHandle != -1)
		 {
			 fp = _fdopen( hConHandle, "w" );

			 if(fp)
			 {
				 *stdout = *fp;

				 setvbuf( stdout, NULL, _IONBF, 0 );
			 }
		 }
	 }

	 // Create windowmanager
	 Wm_Win32 *root = new Wm_Win32();
	 wm = root;

	 // Run main loop
	 if(root->main(hInstance, hPrevInstance, lpCmdLine, nCmdShow ))
		 v_printf(1, "\nNormal exit.\n\n");
	 else
		 v_printf(1, "\nSomething went wrong, GDS3D will close now.\n\n");


	 // Delete windowmanager
	 delete wm;
	 return 0;
}
