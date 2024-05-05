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
#include "win_legend.h"

WinLegend::WinLegend()
{
	int i;
	struct ListItem *item = new struct ListItem();

	current_process = wm->getProcess();
	current_layer = NULL;

	SetTitle("Legend");
	SetScreenSize(wm->screenWidth, wm->screenHeight);
	SetPos(20, wm->screenHeight - 400 - 50);
	SetSize(175, 400);
	SetColorBox(true);
	SetVisibility(false);
	SetSingleSelect(false);
	SetSorted(false);

	for (i = 0; i < current_process->LayerCount(); i++) {
		current_layer = current_process->GetLayer(i);

		if (current_layer) {
			if (current_layer->Red > 0.1 || current_layer->Green > 0.1 || current_layer->Blue > 0.1) {
				item->Red = current_layer->Red;
				item->Green = current_layer->Green;
				item->Blue = current_layer->Blue;
				item->Text = current_layer->Name;
				item->Selected = (current_layer->Show == 1);
				AddItem(item);
			}
		}
	}

	if (item)
		delete item;
}

bool WinLegend::Event(int event, int data, int x, int y, bool shift, bool control, bool alt)
{
	class GDSProcess *current_process = wm->getProcess();
	struct ProcessLayer * current_layer = NULL;
	struct EventInfo eventinfo;
	bool block = true;
    
    if( event == 4 ) // Key up
	{
		int numkey = 0;
        
		switch (data) {
            case KEY_9:
                numkey++;
            case KEY_8:
                numkey++;
            case KEY_7:
                numkey++;
            case KEY_6:
                numkey++;
            case KEY_5:
                numkey++;
            case KEY_4:
                numkey++;
            case KEY_3:
                numkey++;
            case KEY_2:
                numkey++;
            case KEY_1:
                numkey++;
            case KEY_0:
                for (int i = 0; i < current_process->LayerCount(); i++) {
                    current_layer = current_process->GetLayer(i);
                    
                    if (current_layer) {
                        if (current_layer->Alt == alt && current_layer->Ctrl == control && current_layer->Shift == shift && current_layer->ShortKey == numkey) {
                            current_layer->Show = !current_layer->Show;
                            GetItem(i)->Selected = current_layer->Show == 1;
						}
					}
				}
			break;
        }
        
    }

    // Implement listview window behavior
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
			//current_layer = current_process->GetLayer(eventinfo.Item->Index);
			current_layer = current_process->GetLayer((char*) eventinfo.Item->Text.c_str());

			// Set layer visibility
			if (current_layer) {
				current_process->ChangeVisibility(current_layer, eventinfo.Item->Selected);
				wm->getWorld()->_Objects->ClearNetList();
			}

		}
	case LV_EVENT_STATE_CHANGED:
	case LV_EVENT_CLICK:
	case LV_EVENT_SCROLL:
	default:
		break;
	}

	return block;
}

void WinLegend::Draw()
{
	// Update the visibility status of the layers
	struct ListItem *legenditem = NULL;
	struct ProcessLayer * current_layer = NULL;

	legenditem = GetFirst();
	while (legenditem) {
		//current_layer = wm->getProcess()->GetLayer(legenditem->Index);
		current_layer = wm->getProcess()->GetLayer((char*) legenditem->Text.c_str());
		if(current_layer)
			legenditem->Selected = (current_layer->Show == 1);
                    
		legenditem = legenditem->NextItem;
	}

	// Draw the listview
	ListView::Draw();
}