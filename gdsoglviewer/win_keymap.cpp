//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2017 Bertrand Pigeard
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
#include "win_keymap.h"

WinKeymap::WinKeymap()
{
	struct ListItem *item = new struct ListItem();

	SetTitle("Keymap");
	SetScreenSize(wm->screenWidth, wm->screenHeight);
	SetPos(wm->screenWidth / 2 - 255, wm->screenHeight / 2 - 230);
	SetSize(510, 460);
	SetSelectionBox(false);
	SetVisibility(false);
	SetSorted(false);

	item->Text = "INTERFACE"; AddItem(item);
	//item->Text = (char*)"<ALT> F4:           Exit Program"; AddItem(item); item->Index++;  // Does not always work..
	item->Text = "F1:                 Toggle Keymap"; AddItem(item);
	item->Text = "F8:                 Capture Screenshot"; AddItem(item);
	item->Text = "P:                  Toggle Performance Counter"; AddItem(item);
	item->Text = "L:                  Toggle Legend"; AddItem(item);
	item->Text = "T:                  Topcell selection"; AddItem(item);
	item->Text = "R:                  Reset View"; AddItem(item);
	item->Text = "E:                  Toggle Exploded View"; AddItem(item);
	item->Text = "F:                  Export to GMSH"; AddItem(item);
	item->Text = "K:                  Enable Ruler"; AddItem(item);
	item->Text = "H:                  Enable Net Highlighting"; AddItem(item);
	item->Text = "ESC:                Cancel"; AddItem(item);
	item->Text = " "; AddItem(item);
	item->Text = "MOTION"; AddItem(item);
	item->Text = "M:                  Lock Mouse"; AddItem(item);
	item->Text = "W, Up:              Forward"; AddItem(item);
	item->Text = "S, Down:            Backward"; AddItem(item);
	item->Text = "A, Left:            Left"; AddItem(item);
	item->Text = "D, Right:           Right"; AddItem(item);
	item->Text = "Q:                  Up"; AddItem(item);
	item->Text = "Z:                  Down"; AddItem(item);
	item->Text = "Hold Shift:         Increase Movement Speed"; AddItem(item);
	item->Text = "  "; AddItem(item); // Must be unique from the first space item
	item->Text = "MOUSE"; AddItem(item);
	item->Text = "Hold Left Button:   Rotate"; AddItem(item);
	item->Text = "Hold Right Button:  Walk and Strafe"; AddItem(item);
	item->Text = "Wheel Up/Down:      Move Up/Down"; AddItem(item);
	item->Text = "<Alt> Wheel Up:     Show First Invisible Layer"; AddItem(item);
	item->Text = "<Alt> Wheel Down:   Hide First Visible Layer"; AddItem(item);
	item->Text = "<Ctrl> Wheel Up:    Hide Last Visible Layer / Change Ruler Layer"; AddItem(item);
	item->Text = "<Ctrl> Wheel Down:  Show Last Invisible Layer / Change Ruler Layer"; AddItem(item);

	if (item)
		delete item;
}

bool WinKeymap::Event(int event, int data, int x, int y, bool shift, bool control, bool alt)
{
	struct EventInfo eventinfo;
	bool block = true;
	
	ListView::Event(event, data, x, y, shift, control, alt);
	eventinfo = LastEventInfo();
	wm->change_cursor(eventinfo.EventCursor);
	
	switch (eventinfo.EventID) {
	case LV_EVENT_NONE:
		//block = (eventinfo.EventCursor != 0);
		block = false;
		break;
	case LV_EVENT_SELECTED_CHANGED:
	case LV_EVENT_STATE_CHANGED:
	case LV_EVENT_CLICK:
	case LV_EVENT_SCROLL:
	default:
		break;
	}

	return block;
}

void WinKeymap::Draw()
{
	ListView::Draw();
}