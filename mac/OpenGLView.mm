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

#import <Cocoa/Cocoa.h>
#import <GLUT/GLUT.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <sys/time.h> // For the timers

#include "main.h"

Wm_Cocoa *root; // Use this to access the C++ window manager

@interface AppDelegate : NSObject
{
}
-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication;
@end

@implementation AppDelegate

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication
{
    // Again, a pain in the ass to get this to work.. Cocoa sucks
    return YES;
}

@end

//---------------------------------------------------------------------------------
@interface OpenGLView : NSOpenGLView
{
@public
    int                 viewHeight;
    int                 viewWidth;
    NSPoint             lastMousePoint;				// last event mouse point
    NSPoint             lastMovePoint;				// last move mouse point
    NSPoint             lastScreenPoint;				// last screen mouse point

@private
    BOOL                leftMouseIsDown;			// was the left mouse button pressed?
    BOOL                rightMouseIsDown;			// was the right mouse button pressed?
    
    NSTimeInterval      lastFrameReferenceTime;		// used to compute change in time
    NSTimer            *timer;						// timer to update the view content
} // OpenGLView

@end

OpenGLView *os_pointer; // Use this pointer to access Cocoa functions


OpenGLView *view_gl; // Us this to access the Cocoa stuff

@implementation OpenGLView


- (void) heartbeat
{
	[self drawRect:[self bounds]];
} // heartbeat

//---------------------------------------------------------------------------------

- (void) initUpdateTimer
{
    NSTimeInterval  kScheduledTimerInSeconds      = 1.0f/60.0f;
	timer = [NSTimer timerWithTimeInterval:kScheduledTimerInSeconds
									target:self
								  selector:@selector(heartbeat)
								  userInfo:nil
								   repeats:YES];
	
	[[NSRunLoop currentRunLoop] addTimer:timer
								 forMode:NSDefaultRunLoopMode];
	
	[[NSRunLoop currentRunLoop] addTimer:timer
								 forMode:NSEventTrackingRunLoopMode];
} // initUpdateTimer


- (void) initOpenGLView:(const NSRect *)theFrame
{
	// Setting the view's frame size
	
	[self setFrameSize:theFrame->size];
	
	// View attributes initilizations
	
	lastFrameReferenceTime = [NSDate timeIntervalSinceReferenceDate];
	
	leftMouseIsDown     = NO;
	rightMouseIsDown    = NO;
    
	// New timer for updating OpenGL view
	
	[self initUpdateTimer];
	
	// Sync to VBL to avoid tearing
	
	GLint  swapInterval = 1;
	
	[[self openGLContext] setValues:&swapInterval
					   forParameter:NSOpenGLCPSwapInterval];
	
	// OpenGL initializations
	
    [[self openGLContext] makeCurrentContext];
	
	// Did the frame change?
	
	[self setPostsFrameChangedNotifications:YES];
    
    view_gl = self;
} // initOpenGLView


- (NSOpenGLPixelFormat *) initPixelFormat
{
	NSOpenGLPixelFormatAttribute pixelAttributes[]
	=	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		//NSOpenGLPFAStencilSize, 8, // No need for stencil buffer
		0
	};
	
	NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:pixelAttributes]
										autorelease];
	
	return pixelFormat;
} // initPixelFormat

//---------------------------------------------------------------------------------

- (id) initWithFrame:(NSRect)theFrame
		 pixelFormat:(NSOpenGLPixelFormat *)thePixelFormat
{
	// Create a GL Context to use - i.e. init the superclass
	
	if( thePixelFormat == nil )
	{
		thePixelFormat = [self initPixelFormat];
	} //  if
	
	self = [super initWithFrame:theFrame
					pixelFormat:thePixelFormat];
	
	if( self )
	{
		[self initOpenGLView:&theFrame];
	} // if
	
	return self;
} // initWithFrame

//---------------------------------------------------------------------------------

- (id) initWithFrame:(NSRect)theFrame
{
    
	id some_id =  [self initWithFrame:theFrame
                          pixelFormat:nil];
    
    // Initialize the window manager
    os_pointer = self;
    root->resize(1024, 768); // Query correct screensize here!
    root->init();
    
    return some_id;
} // initWithFrame

- (void) dealloc
{
	// Release the update timer
	if( timer )
	{
		[timer invalidate];
		[timer release];
		
		timer = nil;
	}
	
    
	//Dealloc the superclass
	[super dealloc];
} // dealloc


- (NSTimeInterval) updateTimeDelta
{
	NSTimeInterval  timeNow   = [NSDate timeIntervalSinceReferenceDate];
	NSTimeInterval  timeDelta = timeNow - lastFrameReferenceTime;
	
	return  timeDelta;
} // updateTimeDelta


- (void) resizeView
{
    int newHeight = self.window.initialFirstResponder.bounds.size.height;
    int newWidth  = self.window.initialFirstResponder.bounds.size.width;
    
    if(viewWidth==newWidth && viewHeight==newHeight) // This function is called every frame...
        return;
    
    viewWidth  = newWidth;
    viewHeight = newHeight;
    
    //v_printf(1, "Resize to %d x %d\n", viewWidth, viewHeight);
    root->resize(viewWidth, viewHeight);
    
    [[self window] setAcceptsMouseMovedEvents:YES]; // Capture mouse move events
    
} // resizeView

- (void) update
{
	[super update];
} // update

- (void) drawRect:(NSRect)theRect
{
    [[self openGLContext] makeCurrentContext];
	[self resizeView];
	
    // Tell the window manager to draw all stuff
    root->draw();
    
    // Debug information
    /*
    char mes[256];
    sprintf(mes, "Event pos: %f, %f", lastMousePoint.x, lastMousePoint.y);
    root->render_text(100,100, mes, VECTOR4D(1.0,1.0,1.0,1.0));
    sprintf(mes, "Move pos: %f, %f", lastMovePoint.x, lastMovePoint.y);
    root->render_text(100,80, mes, VECTOR4D(1.0,1.0,1.0,1.0));
    sprintf(mes, "Screen pos: %f, %f", lastScreenPoint.x, lastScreenPoint.y);
    root->render_text(100,60, mes, VECTOR4D(1.0,1.0,1.0,1.0));*/
    
    // Change the window title (move elsewhere?)
    NSString *AppTitle = [NSString stringWithFormat:@"%s  -  %s  -  %s  -  IC Design Group, University of Twente", GDS3D_VERSION, root->filename, root->techname];
    [[self window] setTitle:AppTitle];
    //[AppTitle dealloc]; // No need for dealloc?

    
    // Finish the frame
	root->gl_finish();
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (EventKey)translateKey:(unichar)key
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
            return KEY_9;/*
        case XK_minus:
            return KEY_MINUS;
        case XK_equal:
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
            return KEY_QUOTE;*//*
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
            return KEY_RALT;*/
        case NSLeftArrowFunctionKey:
            return KEY_LEFT;
        case NSRightArrowFunctionKey:
            return KEY_RIGHT;
        case NSUpArrowFunctionKey:
            return KEY_UP;
        case NSDownArrowFunctionKey:
            return KEY_DOWN;
        case NSF1FunctionKey:
            return KEY_F1;
        case NSF2FunctionKey:
            return KEY_F2;
        case NSF3FunctionKey:
            return KEY_F3;
        case NSF4FunctionKey:
            return KEY_F4;
        case NSF5FunctionKey:
            return KEY_F5;
        case NSF6FunctionKey:
            return KEY_F6;
        case NSF7FunctionKey:
            return KEY_F7;
        case NSF8FunctionKey:
            return KEY_F8;
        case NSF9FunctionKey:
            return KEY_F9;
        case NSF10FunctionKey:
            return KEY_F10;
        case NSF11FunctionKey:
            return KEY_F11;
        case NSF12FunctionKey:
            return KEY_F12;/*
        case NSInsertFunctionKey:
            return KEY_N0;
        case NSEndFunctionKey:
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
        case NSHomeFunctionKey:
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
            return KEY_BACKSPACE;*/
        case 0x1B:
            return KEY_ESC;
        case NSDeleteFunctionKey:
            return KEY_DEL;
        case NSInsertFunctionKey:
            return KEY_INS;
        case NSEndFunctionKey:
            return KEY_END;
        case NSHomeFunctionKey:
            return KEY_HOME;
        /*case XK_Prior:
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
            return KEY_SCROLLOCK;*/
	}
    
	return KEY_NONE; // This should never happen..
}

- (void)keyDown:(NSEvent *)theEvent
{
    NSString *theArrow = [theEvent charactersIgnoringModifiers];
    unichar keyChar = 0;
    if ( [theArrow length] == 0 )
        return;            // reject dead keys
    if ( [theArrow length] == 1 )
    {
        keyChar = [theArrow characterAtIndex:0];
        EventKey newkey = [self translateKey:keyChar];
        root->event( 3, newkey, -1, -1, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
    }
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    if(theEvent.keyCode == 56) // Left shift
    {
        if(theEvent.modifierFlags & NSShiftKeyMask) // Shift key has been pushed
            root->event( 3, KEY_LSHIFT, -1, -1, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
        else
            root->event( 4, KEY_LSHIFT, -1, -1, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
    }
    if(theEvent.keyCode == 60) // Left shift
    {
        if(theEvent.modifierFlags & NSShiftKeyMask) // Shift key has been pushed
            root->event( 3, KEY_RSHIFT, -1, -1, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
        else
            root->event( 4, KEY_RSHIFT, -1, -1, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
    }
}

- (void)keyUp:(NSEvent *)theEvent
{
    NSString *theArrow = [theEvent charactersIgnoringModifiers];
    unichar keyChar = 0;
    if ( [theArrow length] == 0 )
        return;            // reject dead keys
    if ( [theArrow length] == 1 )
    {
        keyChar = [theArrow characterAtIndex:0];
        EventKey newkey = [self translateKey:keyChar];
        root->event( 4, newkey, -1, -1, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
    }
}

- (void)mouseDown:(NSEvent *)theEvent
{
    lastMousePoint   = [self convertPoint:[theEvent locationInWindow] fromView:nil] ;
    
    root->event( 0, 0, lastMousePoint.x, viewHeight-lastMousePoint.y,  theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
}


- (void)rightMouseDown:(NSEvent *)theEvent
{
	lastMousePoint   = [self convertPoint:[theEvent locationInWindow] fromView:nil] ;
    
    root->event( 0, 1, lastMousePoint.x, viewHeight-lastMousePoint.y,  theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
}


- (void)mouseUp:(NSEvent *)theEvent
{
    
    lastMousePoint   = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    root->event( 1, 0, lastMousePoint.x, viewHeight-lastMousePoint.y,   theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
}


- (void)rightMouseUp:(NSEvent *)theEvent
{
    lastMousePoint   = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    root->event( 1, 1, lastMousePoint.x, viewHeight-lastMousePoint.y,   theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    lastMousePoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    root->event( 2, 0, lastMousePoint.x, viewHeight-lastMousePoint.y, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask );
    [self setNeedsDisplay:YES];
	
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	lastMousePoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	root->event( 2, 1, lastMousePoint.x, viewHeight-lastMousePoint.y, theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask );
	
	[self setNeedsDisplay:YES];
}


- (void)mouseMoved:(NSEvent *)theEvent
{
    lastMousePoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    root->event( 2, 0, lastMousePoint.x, viewHeight-lastMousePoint.y,  theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
    [self setNeedsDisplay:YES];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    lastMousePoint   = [self convertPoint:[theEvent locationInWindow] fromView:nil] ;
    if(theEvent.deltaY > 0.9f)
        root->event( 0, 2, lastMousePoint.x, viewHeight-lastMousePoint.y,   theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
    else if(theEvent.deltaY<-0.9f)
        root->event( 0, 3, lastMousePoint.x, viewHeight-lastMousePoint.y,  theEvent.modifierFlags & NSShiftKeyMask, theEvent.modifierFlags & NSControlKeyMask, theEvent.modifierFlags & NSAlternateKeyMask);
}


@end

//---------------------------------------------------------------------------------


Wm_Cocoa::Wm_Cocoa()
{
	active = false;
	run = false;
}

Wm_Cocoa::~Wm_Cocoa()
{
	for(unsigned int i=0;i<timers.size();i++)
		delete timers[i];
}

void Wm_Cocoa::gl_finish()
{
    // Finish
    [[view_gl.self openGLContext] flushBuffer];
}

bool Wm_Cocoa::hide_mouse( void )
{
    if(WindowManager::hide_mouse()) // Base class behavior
		CGDisplayHideCursor(kCGDirectMainDisplay);
	else
		return false; // Wrong!
    
	return true;
}

bool Wm_Cocoa::show_mouse( void )
{
    if(WindowManager::show_mouse()) // Base class behavior
        CGDisplayShowCursor(kCGDirectMainDisplay);
	else
		return false; // Wrong!
    
	return true;
}

void Wm_Cocoa::change_cursor( int shape )
{
    switch (shape) {
        case 1: // Drag
            [[NSCursor closedHandCursor] set];
            break;
        case 2: // Horizontal resize
            [[NSCursor resizeLeftRightCursor] set];
            break;
        case 3: // Vertical resize
            [[NSCursor resizeUpDownCursor] set];
            break;
        case 4: // Diagonal resize 1 (left top)
            [[NSCursor crosshairCursor] set];
            break;
        case 5: // Diagonal resize 2 (right bottom)
            [[NSCursor crosshairCursor] set];
            break;
        case 6: // Diagonal resize 3 (top right)
            [[NSCursor crosshairCursor] set];
            break;
        case 7: // Diagonal resize 4 (bottom left)
            [[NSCursor crosshairCursor] set];
            break;
        default: // Default
            [[NSCursor arrowCursor] set];
            break;
    }

    
}

void Wm_Cocoa::move_mouse( int x, int y )
{
    // I have been fucking around with this forever!!
    NSRect e = [[NSScreen mainScreen] frame];
    int H = (int)e.size.height;
    
    NSPoint viewPoint= NSMakePoint(x,screenHeight-y);
    view_gl->lastMovePoint   = [view_gl convertPoint:viewPoint toView:nil];

    
    NSPoint screenPoint = [[view_gl window] convertBaseToScreen:view_gl->lastMovePoint];
    
    screenPoint.y = H - screenPoint.y;
    view_gl->lastScreenPoint = screenPoint;
    
    CGEventSourceRef evsrc = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
    CGEventSourceSetLocalEventsSuppressionInterval(evsrc, 0.0);
    CGAssociateMouseAndMouseCursorPosition (0);
    
    CGWarpMouseCursorPosition(CGPointMake(screenPoint.x,screenPoint.y)); // Weird offset of title bar...
    
    CGAssociateMouseAndMouseCursorPosition (1);
}

GLuint Wm_Cocoa::get_font()
{
	//return font;
    return 0;
}

float Wm_Cocoa::timer( struct htime *t, int reset )
{
	float delta=0;
    
    struct timeval offset;
    gettimeofday( &offset, NULL );
    
    htime_Cocoa *t2 = (htime_Cocoa*) t; // Static cast
    
    delta = (float) ( offset.tv_sec - t2->start.tv_sec ) +
    (float) ( offset.tv_usec - t2->start.tv_usec ) / 1e6;
    
    if( reset )
    {
        t2->start.tv_sec = offset.tv_sec;
        t2->start.tv_usec = offset.tv_usec;
    }
    
	return delta;
}

htime* Wm_Cocoa::new_timer()
{
    htime_Cocoa *t = new htime_Cocoa;
    
    timers.push_back(t);
    
    return t;
    return NULL;
}

void Wm_Cocoa::render_text(int x, int y, const char * text, VECTOR4D color)
{
	// Assumes projection matrix is glOrtho with correct screen size
	glColor4f( (GLfloat) color.x, (GLfloat) color.y, (GLfloat) color.z, (GLfloat) color.w );
	glRasterPos2i( x, y );
    
    while (*text)
    {
        //glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *text);
        text++;
    }
}

bool Wm_Cocoa::query_update(FILE *f)
{
    return false; // Still need to implement this
}


//---------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    char GDSfile[1024];
    const char *temp1 = "-i";
    char Techfile[1024];
    const char *temp2 = "-p";
    
    // Maybe we offer a selection window if no arguments are given?
    if(argc < 2)
    {
        // Create the File Open Dialog class.
        NSOpenPanel* openDlg = [NSOpenPanel openPanel];
        
        // First open GDS
        [openDlg setMessage:@"Select a GDS file to open."];
        //[openDlg setDirectory:@"./gds"];
        [openDlg setDirectoryURL: [NSURL fileURLWithPath:@"file:./gds"]];
        if ( [openDlg runModal] == NSOKButton )
        {
            // Gets list of all files selected
            //for(NSString* filePath in [openDlg filenames])
            for(NSURL* filePath in [openDlg URLs])
            {
                //sprintf(GDSfile, "%s", [filePath UTF8String]);
                sprintf(GDSfile, "%s", (char*) [filePath path]);
                v_printf(1, "Opening %s..\n", GDSfile);
                //do something with the file at filePath
                argv[argc] = (char*)temp1; argc++;
                argv[argc] = GDSfile; argc++;
            }
        }
        
        // Then open technology file
        [openDlg setMessage:@"Select a technology txt file to open."];
        //[openDlg setDirectory:@"./techfiles"];
        [openDlg setDirectoryURL: [NSURL fileURLWithPath:@"file:./techfiles"]];
        if ( [openDlg runModal] == NSOKButton )
        {
            // Gets list of all files selected
            //for(NSString* filePath in [openDlg filenames])
            for(NSURL* filePath in [openDlg URLs])
            {
                sprintf(Techfile, "%s", (char*)[filePath path]);
                v_printf(1, "Opening %s..\n", Techfile);
                //do something with the file at filePath
                argv[argc] = (char*)temp2; argc++;
                argv[argc] = Techfile; argc++;
            }
        }

        // Done with the dialog
        [openDlg release];
    }
    
    // Create windowmanager
    root = new Wm_Cocoa();
    wm = root;
    
    // Process commandline
    if(!root->commandLineParameters(argc, argv))
    {
        v_printf(1, "\nPremature exit.\n\n");
        return 0;
    }
    
    // Start the application window
    NSApplicationMain(argc,  (const char **) argv);
    
    // Say goodbye
    delete root;
    v_printf(1, "\nNormal exit.\n\n");
}
