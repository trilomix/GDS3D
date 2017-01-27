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

#ifndef __UI_ELEMENT__H
#define __UI_ELEMENT__H

class UIElement
{
public:
    virtual ~UIElement() {}; // Not sure about this...
    virtual void Disable() = 0;
	virtual bool GetState() = 0;
	virtual void Draw() = 0;
    virtual void Reset() = 0;
    virtual bool Event(int event, int data, int xpos, int ypos , bool shift, bool control, bool alt) = 0;
};



#endif
