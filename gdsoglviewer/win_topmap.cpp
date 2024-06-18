//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
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

void WinTopmap::build_topcell_list(class GDSObject *object, struct ListItem *parent) {
	unsigned int i;
	struct ListItem *item;
	struct ListItem *newitem;
	GDSObject *child;
	
	if (!object)
		return;
	
	newitem = new struct ListItem();
	newitem->Text = object->GetName();

	// Select item if it corresponds to the topcell (listview will prevent double selections automatically, only first occurence will be selected)
	if (strcmp(newitem->Text.c_str(), wm->getWorld()->_topcell->GetName()) == 0)
		newitem->Selected = true;
	else
		newitem->Selected = false;

	// Add new item, under parent if applicable, under root otherwise (in case item == topcell)
	if (parent)
		item = AddItem(parent, newitem);
	else
		item = AddItem(newitem);
		
	// Break if item was not added (double items)
	if (!item) {
		delete newitem;
		return;
	}

	// Add SRef children
	for(i = 0; i < object->GetNumSRefs(); i++)	{
		child = object->GetSRef(i)->object;
		
		if(child && (child != object) && !child->isPCell())
			build_topcell_list(child, item);
	}
	
	// Add ARef children
	for(i = 0; i < object->GetNumARefs(); i++) {
		child = object->GetARef(i)->object;
			
		if(child && (child != object) && !child->isPCell())
			build_topcell_list(child, item);
	}
		
	delete newitem;
}

WinTopmap::WinTopmap()
{
	// Initialize ListView
	SetTitle("Select Top Cell");
	SetScreenSize(wm->screenWidth, wm->screenHeight);
	SetPos(wm->screenWidth / 2 - 125, wm->screenHeight / 2 - 300);
	SetSize(250, 600);
	SetVisibility(false);
	SetSingleSelect(true);
	SetSorted(true);

	build_topcell_list(wm->getWorld()->_Objects->GetTopObject(), NULL);

	// Always expand first item in the list
	if (GetFirst() && GetFirst()->Children.size() > 0)
		GetFirst()->State = 1;
}

bool WinTopmap::Event(int event, int data, int x, int y, bool shift, bool control, bool alt)
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
		if (eventinfo.Item) {			
			wm->getWorld()->SetTopcell(eventinfo.Item->Text.c_str());
			wm->getWorld()->initWorld();			
			wm->getWorld()->init_viewposition();
			
			/*
			if(_mouse_control) {
				wm->show_mouse();
				_mouse_control = 0;
			}*/

			break;
		}
	// case LV_EVENT_RULER_CHANGED:
	case LV_EVENT_STATE_CHANGED:
	case LV_EVENT_CLICK:
	case LV_EVENT_SCROLL:
	default:
		break;
	}
	
	return block;
}

void WinTopmap::Draw()
{
	ListView::Draw();
}