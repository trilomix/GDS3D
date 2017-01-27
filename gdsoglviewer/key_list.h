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

#ifndef __KEYLIST_H__
#define __KEYLIST_H__

enum EventKey{
KEY_NONE,

// Alphanumerical
KEY_A,	
KEY_B,
KEY_C,
KEY_D,
KEY_E,		
KEY_F,		
KEY_G,			
KEY_H,	
KEY_I,		
KEY_J,		
KEY_K,	
KEY_L,		
KEY_M,	
KEY_N,		
KEY_O,		
KEY_P,		
KEY_Q,	
KEY_R,
KEY_S,
KEY_T,
KEY_U,	
KEY_V,
KEY_W,		
KEY_X,			
KEY_Y,			
KEY_Z,		
KEY_0,			
KEY_1,			
KEY_2,			
KEY_3,			
KEY_4,			
KEY_5,		
KEY_6,			
KEY_7,			
KEY_8,			
KEY_9,		

// Printable Character
KEY_MINUS,		// -_ Key
KEY_PLUS,		// =+ Key
KEY_COMMA,		// ,< Key
KEY_PERIOD,		// .> Key
KEY_COLON,		// ;: Key
KEY_SLASH,		// /? Key
KEY_TILDE,		// `~ Key
KEY_BRACKET_O,	// [{ Key
KEY_BACKSLASH,	// \| Key
KEY_BRACKET_C,	// ]} Key
KEY_QUOTE,		// '" Key

// Modifier
KEY_LSHIFT,
KEY_RSHIFT,	
KEY_LCTRL,	
KEY_RCTRL,	
KEY_LALT,		
KEY_RALT,		

// Arrow
KEY_LEFT,		
KEY_RIGHT,		
KEY_UP,			
KEY_DOWN,		

// Function
KEY_F1,	
KEY_F2,
KEY_F3,
KEY_F4,
KEY_F5,	
KEY_F6,		
KEY_F7,		
KEY_F8,		
KEY_F9,		
KEY_F10,			
KEY_F11,		
KEY_F12,			

// Numpad
KEY_N0,			
KEY_N1,			
KEY_N2,			
KEY_N3,			
KEY_N4,			
KEY_N5,			
KEY_N6,			
KEY_N7,			
KEY_N8,			
KEY_N9,			
KEY_N_ADD,	
KEY_N_SUBTRACT,
KEY_N_MULTIPLY,	
KEY_N_DIVIDE,	
KEY_N_DECIMAL,	
KEY_N_ENTER,		

// Other
KEY_ENTER,		
KEY_SPACE,		
KEY_TAB,			
KEY_BACKSPACE,	
KEY_ESC,			
KEY_DEL,			
KEY_INS,			
KEY_END,		
KEY_HOME,		
KEY_PGUP,	
KEY_PGDOWN,		
KEY_PAUSE,		
KEY_CAPSLOCK,	
KEY_NUMLOCK,		
KEY_SCROLLOCK	
};

#endif