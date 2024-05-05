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

#include "gds_globals.h"
#include "listview.h"
#include "renderer.h"
#include "windowmanager.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

struct compare_strings {
	bool operator() (struct ListItem *A, struct ListItem *B) {
		return (A->Text.compare(B->Text) < 0);
	}
};

ListView::ListView() {
	_RootNode = new struct ListItem;
	_RootNode->Text = "root";
	_RootNode->Parent = NULL;
	_RootNode->PrevItem = NULL;
	_RootNode->NextItem = NULL;
	_RootNode->State = 1;
	_RootNode->Level = -1;

	_Title = "";
	_Count = 0;
	_Depth = -1;
	_FirstVisible = NULL;
	_Visible = false;
	_Sorted = false;
	_X = 0;
	_Y = 0;
	_Height = 0;
	_Width = 0;
	_ScreenHeight = 0;
	_ScreenWidth = 0;
	_SingleSelect = false;
	_ColorBox = false;
	_SelectionBox = true;
	_DragX = -1;
	_DragY = -1;
	_MouseDragOperation = LV_OP_NONE;
}

ListView::~ListView() {
	RemoveItem(_RootNode);
}

void ListView::Draw() {
	if (_Visible) {
		ListItem *item = _FirstVisible;
		int visibleitems;
		int sliderheight;
		int slidertop;
		int capacity;
		int xpos, ypos;
		int i;
		
		// Calculate how many items fit in the list, using 20 pixels per item and subtracting 35 for the title and margins
		capacity = (_Height - 35) / 20;

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);	
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FOG);
        
		// Outline
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
		gl_square(_X, _Y, _X + _Width, _Y + _Height, true);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		gl_square(_X, _Y, _X + _Width, _Y + _Height, false);

		glBegin(GL_LINES);
		glVertex2i(_X, _Y + _Height - 28);
		glVertex2i(_X + _Width, _Y + _Height - 28);
		glEnd();
		
		// Scrollbar
		glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
		gl_square(_X + _Width - 16, _Y + _Height - 30, _X + _Width - 2, _Y + _Height - 44, true);
		gl_square(_X + _Width - 16, _Y + 16, _X + _Width - 2, _Y + 2, true);

		glColor4f(0.35f, 0.35f, 0.35f, 1.0f);
		gl_square(_X + _Width - 16, _Y + _Height - 45, _X + _Width - 2, _Y + 17, true);

		// Arrows
		glColor4f(0.05f, 0.05f, 0.05f, 1.0f);
		glBegin(GL_TRIANGLES);
		glVertex2i(_X + _Width - 13, _Y + _Height - 41);
		glVertex2i(_X + _Width - 5, _Y + _Height - 41);
		glVertex2i(_X + _Width - 9, _Y + _Height - 33);
		glEnd();  

		glBegin(GL_TRIANGLES);
		glVertex2i(_X + _Width - 13, _Y + 13);
		glVertex2i(_X + _Width - 5, _Y + 13);
		glVertex2i(_X + _Width - 9, _Y + 5);
		glEnd();  

		// Scrollbar slider
		if (!_RootNode->Children.empty() && _FirstVisible) {
			visibleitems = count_visible(_RootNode->Children[0]);
			slidertop = int(_Y + _Height - 45 - (float(visibleitems - count_visible(_FirstVisible)) / float(visibleitems)) * (_Height - 62));
			sliderheight = int((float(capacity) / float(visibleitems)) * (_Height - 62));
		}
		else {
			slidertop = _Y + _Height - 45;
			sliderheight = _Height - 62;
		}

		if (sliderheight > _Height - 62)
			sliderheight = _Height - 62;

		if (sliderheight < 4)
			sliderheight = 4;

		if (slidertop - sliderheight < _Y + 17) 
			slidertop = _Y + 17 + sliderheight;

		glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
		gl_square(_X + _Width - 16, slidertop, _X + _Width - 2, slidertop - sliderheight, true);

		// Title
		gl_text(_X + 10, _Y + _Height - 20, 1.0f, 1.0f, 1.0f, _Title, _Width - 20);
		
		// Populate list
		if (item) {
			ypos = 	_Y + _Height - 35;

			for (i = 0; i < capacity; i++) {
				xpos = _X + 10 + LV_INDENT * item->Level;

				// Draw color boxes
				if (_ColorBox) {
					glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					gl_square(xpos, ypos, xpos + 12, ypos - 12, false);
					glColor4f(GLfloat(item->Red), GLfloat(item->Green), GLfloat(item->Blue), 1.0f);
					gl_square(xpos + 1, ypos - 1, xpos + 11, ypos - 11, true);
					xpos += 20;
				}

				// Draw selection boxes
				if (_SelectionBox) {
					glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					gl_square(xpos, ypos, xpos + 12, ypos - 12, false);

					// Cross out selection box if item state is true
					if (item->Selected) {
						glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

						glBegin(GL_LINES);
						glVertex2i(xpos + 1, ypos - 1);
						glVertex2i(xpos + 11, ypos - 11);

						glVertex2i(xpos + 1, ypos - 11);
						glVertex2i(xpos + 11, ypos - 1);
						glEnd();
					}

					xpos += 20;
				}

				// Display item text
				switch (item->State) {
					case 0:
						gl_triangle(xpos, ypos - 2, xpos + 8, ypos - 6, xpos, ypos - 10);
						xpos += 12;
						break;
					case 1:
						gl_triangle(xpos, ypos - 2, xpos + 8, ypos - 2, xpos + 4, ypos - 10);
						xpos += 12;
						break;
					default:
						break;
				}

				if (item->Text.length() > 0)
					gl_text(xpos, ypos - 12, 1.0f, 1.0f, 1.0f,  item->Text, _X + _Width - xpos - 25);

				// Find next visible item
				item = find_visible(item, 1);
				ypos -= 20;

				if (!item)
					break;
			}
		}
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		glEnable(GL_FOG);
	}
}

void ListView::SetPos(int X, int Y) {
	_X = X;
	_Y = Y;

	if (_X < 1)
		_X = 1;
	
	if (_Y + _Height > _ScreenHeight + 1)
		_Y = _ScreenHeight - _Height + 1;
}

void ListView::SetSize(int Width, int Height) {
	_Width = Width;
	_Height = Height;

	// Set minimum dimensions
	if (_Height < 75)
		_Height = 75;

	if (_Width < 70 + 20 * (_ColorBox + _SelectionBox))
		_Width = 70 + 20 * (_ColorBox + _SelectionBox);
}

void ListView::SetScreenSize(int Width, int Height) {
	_ScreenWidth = Width;
	_ScreenHeight = Height;
}


void ListView::SetTitle(string Title) {
	_Title = Title;
}

void ListView::SetVisibility(bool Visible) {
	_Visible = Visible;
}

void ListView::SetSorted(bool Sorted) {
	// Sort list if it is not sorted already
	if (Sorted && !_Sorted)
		sort_children(_RootNode);

	_Sorted = Sorted;
}

void ListView::SetSingleSelect(bool SingleSelect) {
	_SingleSelect = SingleSelect;

	// Set ony the first item to selected if _SingleSelect = true
	if (!_RootNode->Children.empty() && _SingleSelect) {
		clear_selected();
		_RootNode->Children[0]->Selected = true;
	}
}

void ListView::SetColorBox(bool Visible) {
	_ColorBox = Visible;
}

void ListView::SetSelectionBox(bool Visible) {
	_SelectionBox = Visible;
}

void ListView::Clear() {
	// Remove all ListItems and delete memory allocations
	RemoveItem(_RootNode);

	// Reset _Count and _FirstVisible and recreate default _RootNode
	_RootNode = new struct ListItem;
	_RootNode->Text = "root";
	_RootNode->Parent = NULL;
	_RootNode->PrevItem = NULL;
	_RootNode->NextItem = NULL;
	_RootNode->State = 1;
	_RootNode->Level = -1;
	_Count = 0;
	_Depth = -1;
	_FirstVisible = NULL;
}

bool ListView::Event(int event, int data, int x, int y, bool shift, bool control, bool alt) {
	EventInfo info;
	info.EventID = LV_EVENT_NONE;
	info.Item = NULL;
	info.EventCursor = LV_MOUSE_DEFAULT;
	ListItem *eventitem;
	int visibleitems;
	int firstvisibleindex;
	int selectionleft;
	int expansionleft;
	int listindex;
	int capacity;
	
	if (_Visible) {
		// Calculate how many items fit in the list, using 20 pixels per item and subtracting 35 for the title and margins
		capacity = (_Height - 35) / 20;
		
		if (!_RootNode->Children.empty())
			visibleitems = count_visible(_RootNode->Children[0]);
		else
			visibleitems = 0;
		
		if (event == 1 && data == 0 ) {
			// Mouse released
			_MouseDragOperation = LV_OP_NONE;
		}
		else if (event == 2 && _MouseDragOperation) { // <- why _MouseDragOperation??
			// Mouse moved and drag mode currently active
			switch (_MouseDragOperation) {
			case LV_OP_SCROLLBAR:
				// Set first visible item such that the slider will end up centered around the cursor location
				firstvisibleindex = int(float(visibleitems * (_Y + _Height - 45 - y)) / float(_Height - 62) - (float(capacity) / 2));
				if (firstvisibleindex > visibleitems - capacity) firstvisibleindex = visibleitems - capacity;
				if (firstvisibleindex < 0) firstvisibleindex = 0;
				_FirstVisible = find_visible(_RootNode->Children[0], firstvisibleindex);

				info.EventID = LV_EVENT_SCROLL;
				break;
			case LV_OP_RESIZE_TOPLEFT:
				resize_top(y);
				resize_left(x);
				info.EventCursor = LV_MOUSE_RESIZE_D1;
				info.EventID = LV_EVENT_RESIZE;
				break;
			case LV_OP_RESIZE_BOTTOMRIGHT:
				resize_bottom(y);
				resize_right(x);
				info.EventCursor = LV_MOUSE_RESIZE_D2;
				info.EventID = LV_EVENT_RESIZE;
				break;
			case LV_OP_RESIZE_TOPRIGHT:
				resize_top(y);
				resize_right(x);
				info.EventCursor = LV_MOUSE_RESIZE_D3;
				info.EventID = LV_EVENT_RESIZE;
				break;
			case LV_OP_RESIZE_BOTTOMLEFT:
				resize_bottom(y);
				resize_left(x);
				info.EventCursor = LV_MOUSE_RESIZE_D4;
				info.EventID = LV_EVENT_RESIZE;
				break;
			case LV_OP_RESIZE_LEFT:
				resize_left(x);
				info.EventCursor = LV_MOUSE_RESIZE_H;
				info.EventID = LV_EVENT_RESIZE;
				break;	
			case LV_OP_RESIZE_RIGHT:
				resize_right(x);
				info.EventCursor = LV_MOUSE_RESIZE_H;
				info.EventID = LV_EVENT_RESIZE;
				break;	
			case LV_OP_RESIZE_TOP:
				resize_top(y);
				info.EventCursor = LV_MOUSE_RESIZE_V;
				info.EventID = LV_EVENT_RESIZE;
				break;
			case LV_OP_RESIZE_BOTTOM:
				resize_bottom(y);
				info.EventCursor = LV_MOUSE_RESIZE_V;
				info.EventID = LV_EVENT_RESIZE;
				break;
			case LV_OP_MOVE:
				// Change top and left coordinates
				_Y = y - _DragY;
				_X = x - _DragX;
				
				// Clip at window edges
				if (_Y > _ScreenHeight - _Height - 1)
					_Y = _ScreenHeight - _Height - 1;
				if (_Y < 1)
					_Y = 1;
				if (_X > _ScreenWidth - _Width - 1)
					_X = _ScreenWidth - _Width - 1;
				if (_X < 1)
					_X = 1;
				
				info.EventCursor = LV_MOUSE_MOVE;
				info.EventID = LV_EVENT_MOVE;
			default:
				break;
			}
		}
		else if (x >= _X - 5 && x <= _X + _Width + 5 && y <= _Y + _Height + 5 && y >= _Y - 5) {
			switch (data) {
			case -1: // No button
				// Resize cursor
				info.EventID = LV_EVENT_MOVE;
				switch (mouse_pos(x, y)) {
				case LV_POS_RESIZE_TOPLEFT:
					info.EventCursor = LV_MOUSE_RESIZE_D1;
					break;
				case LV_POS_RESIZE_BOTTOMRIGHT:
					info.EventCursor = LV_MOUSE_RESIZE_D2;
					break;
				case LV_POS_RESIZE_TOPRIGHT:
					info.EventCursor = LV_MOUSE_RESIZE_D3;
					break;
				case LV_POS_RESIZE_BOTTOMLEFT:
					info.EventCursor = LV_MOUSE_RESIZE_D4;
					break;
				case LV_POS_RESIZE_TOP:
				case LV_POS_RESIZE_BOTTOM:
					info.EventCursor = LV_MOUSE_RESIZE_V;
					break;
				case LV_POS_RESIZE_LEFT:
				case LV_POS_RESIZE_RIGHT:
					info.EventCursor = LV_MOUSE_RESIZE_H;
					break;
				case LV_POS_MOVE:
					info.EventCursor = LV_MOUSE_MOVE;
				default:
					break;
				}

				break;
			case 0: // Left button
				info.EventID = LV_EVENT_CLICK;
				switch (event) {
				case 0: // Button down

					// Find index of clicked item
					if ((_Y + _Height - 35 - y) % 20 >= 0 && (_Y + _Height - 35 - y) % 20 <= 12) {
						listindex = (_Y + _Height - 35 - y) / 20;
						eventitem = find_visible(_FirstVisible, listindex);

						if (eventitem) {
							// Find left coordinates of selection box and expansion arrow
							if (_SelectionBox)
								selectionleft = _X + 10 + LV_INDENT * eventitem->Level + (int(_ColorBox)) * 20;
							else
								selectionleft = -1;

							if (eventitem->State == -1)
								expansionleft = -1;
							else
								expansionleft = _X + 10 + LV_INDENT * eventitem->Level + (int(_ColorBox + _SelectionBox)) * 20;

							// Selection box clicked
							if (selectionleft > -1 && x >= selectionleft && x <= selectionleft + 12) {
								
								// Clear any other selected items if _SingleSelect is true
								if (_SingleSelect)
									clear_selected();
								
								// Set state (inverse of previous state in multi-select mode or always on in single-select mode)
								eventitem->Selected = !eventitem->Selected | _SingleSelect;								
								
								info.EventID = LV_EVENT_SELECTED_CHANGED;
								info.Item = eventitem;
							}
							// Expansion triangle clicked
							else if (expansionleft > -1 && x >= expansionleft && x <= expansionleft + 8) {
								if (eventitem->State == 0)
									eventitem->State = 1;
								else
									eventitem->State = 0;
								
								recalc_firstvisible();
								info.EventID = LV_EVENT_STATE_CHANGED;
								info.Item = eventitem;
							}
						}
					}

					// Determine operation depending on mouse position
					switch (mouse_pos(x, y)) {
					case LV_POS_UP:
						dec_firstvisible();
						info.EventID = LV_EVENT_SCROLL;
						break;
					case LV_POS_DOWN:
						inc_firstvisible();
						info.EventID = LV_EVENT_SCROLL;
						break;
					case LV_POS_SCROLLBAR:
						// Set first visible item such that the slider will end up centered around the click location
						firstvisibleindex = int(float(visibleitems * (_Y + _Height - 45 - y)) / float(_Height - 62) - (float(capacity) / 2));
						if (firstvisibleindex > visibleitems - capacity) firstvisibleindex = visibleitems - capacity;
						if (firstvisibleindex < 0) firstvisibleindex = 0;
						_FirstVisible = find_visible(_RootNode->Children[0], firstvisibleindex);

						_MouseDragOperation = LV_OP_SCROLLBAR;
						info.EventID = LV_EVENT_SCROLL;
						break;
					case LV_POS_RESIZE_TOPLEFT:
						_DragX = _X;
						_DragY = _Y + _Height;
						_MouseDragOperation = LV_OP_RESIZE_TOPLEFT;
						info.EventCursor = LV_MOUSE_RESIZE_D1;
						break;
					case LV_POS_RESIZE_BOTTOMRIGHT:
						_DragX = _X + _Width;
						_DragY = _Y;
						_MouseDragOperation = LV_OP_RESIZE_BOTTOMRIGHT;
						info.EventCursor = LV_MOUSE_RESIZE_D2;
						break;
					case LV_POS_RESIZE_TOPRIGHT:
						_DragX = _X + _Width;
						_DragY = _Y + _Height;
						_MouseDragOperation = LV_OP_RESIZE_TOPRIGHT;
						info.EventCursor = LV_MOUSE_RESIZE_D3;
						break;
					case LV_POS_RESIZE_BOTTOMLEFT:
						_DragX = _X;
						_DragY = _Y;
						_MouseDragOperation = LV_OP_RESIZE_BOTTOMLEFT;
						info.EventCursor = LV_MOUSE_RESIZE_D4;
						break;
					case LV_POS_RESIZE_TOP:
						_DragY = _Y + _Height;
						_MouseDragOperation = LV_OP_RESIZE_TOP;
						info.EventCursor = LV_MOUSE_RESIZE_V;
						break;
					case LV_POS_RESIZE_BOTTOM:
						_DragY = _Y;
						_MouseDragOperation = LV_OP_RESIZE_BOTTOM;
						info.EventCursor = LV_MOUSE_RESIZE_V;
						break;
					case LV_POS_RESIZE_LEFT:
						_DragX = _X;
						_MouseDragOperation = LV_OP_RESIZE_LEFT;
						info.EventCursor = LV_MOUSE_RESIZE_H;
						break;
					case LV_POS_RESIZE_RIGHT:
						_DragX = _X + _Width;
						_MouseDragOperation = LV_OP_RESIZE_RIGHT;
						info.EventCursor = LV_MOUSE_RESIZE_H;
						break;
					case LV_POS_MOVE:
						_DragX = x - _X;
						_DragY = y - _Y;
						_MouseDragOperation = LV_OP_MOVE;
						info.EventCursor = LV_MOUSE_MOVE;
					default:
						break;
					}
				default:
					break;
				}
				break;
			case 1: // Right button
				info.EventID = LV_EVENT_CLICK;
				break;
			case 2: // Scroll wheel up
				if (control)
					break;

				dec_firstvisible();
				info.EventID = LV_EVENT_SCROLL;
				break;
			case 3: // Scroll wheel down
				if (control)
					break;

				inc_firstvisible();
				info.EventID = LV_EVENT_SCROLL;
				break;
			default:
				break;
			}
		}
	}
	else
		_MouseDragOperation = LV_OP_NONE;
	
	ei = info;
	return (info.EventID != LV_EVENT_NONE);
}

// Adds a new item to the list, under _RootNode
ListItem *ListView::AddItem(struct ListItem *NewListItem) {
	return AddItem(_RootNode, NewListItem);
}

// Adds a new item to the list under the specified parent
ListItem *ListView::AddItem(struct ListItem *Parent, struct ListItem *NewListItem) {
	struct ListItem *new_item;
	unsigned int i;

	if (!NewListItem || !Parent)
		return NULL;

	// Break if item with same text already exists
	for (i = 0; i < Parent->Children.size(); i++) {
		if (Parent->Children[i]->Text.compare(NewListItem->Text) == 0)
			return NULL;
	}

	// Create new item
	new_item = new struct ListItem;
	new_item->Parent = Parent;
	new_item->Text = NewListItem->Text;
	new_item->Level = Parent->Level + 1;
	new_item->State = -1;
	new_item->Selected = NewListItem->Selected;
	new_item->Red = NewListItem->Red;
	new_item->Green = NewListItem->Green;
	new_item->Blue = NewListItem->Blue;

	// Add to parent's list and sort if required
	Parent->Children.push_back(new_item);
	
	if (_Sorted)
		sort(Parent->Children.begin(), Parent->Children.end(), compare_strings());
	
	// Iterate through parent's children and set Index, NextItem and PrevItem values
	for (i = 0; i < Parent->Children.size(); i++) {
		Parent->Children[i]->Index = i;

		if (i == 0) 
			Parent->Children[i]->PrevItem = NULL;
		else
			Parent->Children[i]->PrevItem = Parent->Children[i - 1];
			
		if (i == Parent->Children.size() - 1)
			Parent->Children[i]->NextItem = NULL;
		else
			Parent->Children[i]->NextItem = Parent->Children[i + 1];
	}

	// Allow parent item to be expanded
	if (Parent->State == -1)
		Parent->State = 0;

	// Update _Depth if required
	if (new_item->Level > _Depth)
		_Depth = new_item->Level;

	// Reset selected property of all other items if _SingleSelect is true and the new item is selected
	if (new_item->Selected && _SingleSelect) {
		clear_selected();
		new_item->Selected = true;
	}

	_Count++;
	recalc_firstvisible();
	
	return new_item;
}

// Removes an item and all its children from the list
bool ListView::RemoveItem(struct ListItem *Item) {
	struct ListItem *item = Item;
	unsigned int i;
	
	if (!item)
		return false;

	// Delete children
	if (!delete_children(item))
		return false;

	// Clear _RootNode or _FirstVisible if either is being deleted
	if (item == _RootNode)
		_RootNode = NULL;
	
	if (item == _FirstVisible)
		_FirstVisible = NULL;

	// Recreate item sequence
	if (item->NextItem)
		item->NextItem->PrevItem = item->PrevItem;

	if (item->PrevItem)
		item->PrevItem->NextItem = item->NextItem;

	// Remove item from parent's list
	if (item->Parent) {
		item->Parent->Children.erase(item->Parent->Children.begin() + item->Index);
		
		if (item->Parent->Children.empty() && (item->Parent != _RootNode)) 
			item->Parent->State = -1;

		// Update indices
		for (i = 0; i < item->Parent->Children.size(); i++)
			item->Parent->Children[i]->Index = i;
	}

	// Delete item
	delete item;
	item = NULL;
	_Count--;

	// Recaculate _Depth and _FirstVisible
	if (_RootNode) {
		recalc_depth();
		recalc_firstvisible();
	}

	return true;
}

// Retrieve a top-level item from the list
struct ListItem *ListView::GetItem(unsigned int Index) {
	return GetItem(_RootNode, Index);
}

// Retrieve an item from the list by parent node and index
struct ListItem *ListView::GetItem(struct ListItem *Parent, unsigned int Index) {
	if (Parent && Index < Parent->Children.size())
		return Parent->Children[Index];
	
	return NULL;
}

// Return the first item in the list
struct ListItem *ListView::GetFirst() {
	if(!_RootNode->Children.empty()) 
		return _RootNode->Children[0];

	return NULL;
}

int ListView::GetCount() {
	return _Count;
}

int ListView::GetY() {
	return _Y;
}

int ListView::GetX() {
	return _X;
}

int ListView::GetWidth() {
	return _Width;
}

int ListView::GetHeight() {
	return _Height;
}

bool ListView::GetVisibility() {
	return _Visible;
}

bool ListView::GetSorted() {
	return _Sorted;
}

bool ListView::GetSingleSelect() {
	return _SingleSelect;
}

string ListView::GetTitle() {
	return _Title;
}

bool ListView::GetColorBox() {
	return _ColorBox;
}

bool ListView::GetSelectionBox() {
	return _SelectionBox;
}

// Hierarchically delete all children of an item
bool ListView::delete_children(struct ListItem *item) {
	unsigned int i;
	
	for (i = 0; i < item->Children.size(); i++) {
		if (!delete_children(item->Children[i]))
			return false;

		delete item->Children[i];
		item->Children[i] = NULL;
		_Count--;
	}

	return true;
}

// Iterate through list and deselect all items
void ListView::clear_selected() {
	clear_selected(_RootNode);
}

void ListView::clear_selected(struct ListItem *item) {
	unsigned int i;

	if (!item)
		return;

	item->Selected = false;
	
	for (i = 0; i < item->Children.size(); i++)
		clear_selected(item->Children[i]);
}

// Recalculates the hierarchy depth
void ListView::recalc_depth() {
	recalc_depth(_RootNode);
}

void ListView::recalc_depth(struct ListItem *root_item) {
	struct ListItem *item;
	unsigned int i;

	// Break if root_item does not exist
	if (!root_item) return;

	// Hierarchically iterate through children
	for (i = 0; i < root_item->Children.size(); i++) {
		item = root_item->Children[i];
		
		if (item->Level > _Depth)
			_Depth = item->Level;
		
		recalc_depth(item);
	}
}

// Hierarchically go through the list and sort all items under parent_item
void ListView::sort_children(struct ListItem *parent_item) {
	unsigned int i;
	
	if (!parent_item)
		return;

	sort(parent_item->Children.begin(), parent_item->Children.end(), compare_strings());
	
	// Iterate through parent's children and set Index, NextItem and PrevItem values and descent into hierarchy
	for (i = 0; i < parent_item->Children.size(); i++) {
		parent_item->Children[i]->Index = i;

		if (i == 0) 
			parent_item->Children[i]->PrevItem = NULL;
		else
			parent_item->Children[i]->PrevItem = parent_item->Children[i - 1];
			
		if (i == parent_item->Children.size() - 1)
			parent_item->Children[i]->NextItem = NULL;
		else
			parent_item->Children[i]->NextItem = parent_item->Children[i + 1];

		sort_children(parent_item->Children[i]);
	}
}

// Counts the number of visible items starting from start_item (which does not have to be visible itself)
int ListView::count_visible(struct ListItem *start_item) {
	int visible_items = 0;
	struct ListItem *item = start_item;
	
	while (item) {
		if (is_visible(item))
			visible_items++;

		if ((item->Children.size() > 0) && (item->State == 1))
			item = item->Children[0];
		else {
			while (item && !item->NextItem)
				item = item->Parent;

			if (item) 
				item = item->NextItem;
		}
	}

	return visible_items;
}	

// Returns the n'th visible item after start_item, or NULL if no such item exists
struct ListItem *ListView::find_visible(struct ListItem *start_item, int n) {
	int item_countdown = n + 1;
	struct ListItem *item = start_item;
	
	while (item) {
		if (is_visible(item))
			item_countdown--;

		if (item_countdown == 0)
			return item;

		if ((item->Children.size() > 0) && (item->State == 1))
			item = item->Children[0];
		else {
			while (item && !item->NextItem)
				item = item->Parent;

			if (item) 
				item = item->NextItem;
		}
	}

	return NULL;
}

// Returns whether or not an item is visible
bool ListView::is_visible(struct ListItem *item) {
	bool item_visible = true;
	struct ListItem *parent_item;
	
	if (!item || (item == _RootNode))
		return false;

	parent_item = item->Parent;

	while (parent_item) {
		if (parent_item->State != 1)
			item_visible = false;

		parent_item = parent_item->Parent;
	}

	return item_visible;
}

// Sets the first visible item to the next item that is actually displayed in the list (i.e. scrolls down)
void ListView::inc_firstvisible() {
	int capacity = (_Height - 35) / 20;

	// Break if _FirstVisible does not exist or list fits all items
	if (!_FirstVisible)
		return;
	
	if (capacity >= count_visible(_FirstVisible))
		return;
	
	_FirstVisible = find_visible(_FirstVisible, 1);
}

// Sets the first visible item to the previous item that is actually displayed in the list (i.e. scrolls up)
void ListView::dec_firstvisible() {
	// Break if _FirstVisible does not exist
	if (!_FirstVisible)
		return;
	
	// Previous visible item is either previous item at same level or parent, unless _FirstVisible is the first item under _RootNode
	if (_FirstVisible->PrevItem)
		_FirstVisible = _FirstVisible->PrevItem;
	else if (_FirstVisible->Parent != _RootNode)
		_FirstVisible = _FirstVisible->Parent;
}

// Re-calculates the first item to be displayed after e.g. a vertical resize
void ListView::recalc_firstvisible() {
	int capacity;
	int total_visible;
	int first_visible_index;

	capacity = (_Height - 35) / 20;

	// Break if list is empty
	if (_RootNode->Children.empty()) {
		_FirstVisible = NULL;
		return;
	}

	if (!_FirstVisible) {
		_FirstVisible = _RootNode->Children[0];
		return;
	}

	// Break if list fits fewer items than visible after _FirstVisible
	if (capacity < count_visible(_FirstVisible)) 
		return;

	// Find new first_visible
	total_visible = count_visible(_RootNode->Children[0]);
	first_visible_index = total_visible - capacity;
	
	if (first_visible_index >= 0)
		_FirstVisible = find_visible(_RootNode->Children[0], first_visible_index);
	else
		_FirstVisible = _RootNode->Children[0];
}

void ListView::gl_square(GLint x1, GLint y1, GLint x2, GLint y2, bool filled) {
	if (filled) {
		glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
		glEnd();
	}
	else {
		glBegin(GL_LINES);
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glEnd();

		glBegin(GL_LINES);
		glVertex2i(x2, y1);
		glVertex2i(x2, y2);
		glEnd();

		glBegin(GL_LINES);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
		glEnd();

		glBegin(GL_LINES);
		glVertex2i(x1, y2);
		glVertex2i(x1, y1);
		glEnd();
	}
}

void ListView::gl_triangle(GLint x1, GLint y1, GLint x2, GLint y2, GLint x3, GLint y3) {
	glBegin(GL_TRIANGLES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glVertex2i(x3, y3);
	glEnd();
}

// Prints a text at the given position, if maxwidth > -1 it will try to concatenate the text if it is too long, appending '...' at the end
void ListView::gl_text(GLint x, GLint y, GLfloat red, GLfloat green, GLfloat blue, string text, int maxwidth) {
	string str = text;
	int length = str.length();
	
	// Shorten text if it is too wide
	if (maxwidth > 0 && length > (maxwidth / 10)) {
		if (maxwidth / 10 > 4)
			str = str.substr(0, maxwidth / 10 - 3).append(3, '.');
		else
			str = str.substr(0, 1).append(3, '.');
	}

	// Print the text
	wm->render_text(x, y, str.c_str(), VECTOR4D(red, green, blue, 1.0f));	
}

void ListView::resize_top(int Y) {
	// Set new Y
	if (Y - _Y > 95)
		_Height = Y - _Y;
	else
		_Height = 95;

	// Clip at top of window
	if (_Y + _Height > _ScreenHeight - 1) 
		_Height = _ScreenHeight - 1 - _Y;

	recalc_firstvisible();	
}

void ListView::resize_bottom(int Y) {
	// Set new Y and height
	if (_Y + _Height - Y > 95) {
		_Height = _Height + _Y - Y;
		_Y = Y;
	}
	else {
		_Y = _Y + _Height - 95;
		_Height = 95;
	}
	
	// Clip at bottom of window
	if (_Y < 1) {
		_Height = _Y + _Height - 1;
		_Y = 1;
	}

	recalc_firstvisible();
}

void ListView::resize_left(int X) {
	// Set new width and left coordinate
	if (_Width + _X - X > 70 + _Depth * LV_INDENT + 20 * (_ColorBox + _SelectionBox)) {
		_Width = _Width + _X - X;
		_X = X;
	}
	else {
		_X = _X + _Width - (70 + _Depth * LV_INDENT + 20 * (_ColorBox + _SelectionBox));
		_Width = 70 + _Depth * LV_INDENT + 20 * (_ColorBox + _SelectionBox);
	}

	// Clip at left side of window
	if (_X < 1) {
		_Width = _X + _Width - 1;
		_X = 1;
	}
}

void ListView::resize_right(int X) {
	// Set new width
	if (X - _X > 70 + _Depth * LV_INDENT + 20 * (_ColorBox + _SelectionBox))
		_Width = X - _X;
	else
		_Width = 70 + _Depth * LV_INDENT + 20 * (_ColorBox + _SelectionBox);

	// Clip at right side of window
	if (_Width > _ScreenWidth - 1 - _X)
		_Width = _ScreenWidth - 1 - _X;
}

int ListView::mouse_pos(int X, int Y) {
	if (X >= _X + _Width - 16 && X <= _X + _Width - 2 && Y >= _Y + _Height - 44 && Y <= _Y + _Height - 30)
		return LV_POS_UP;
	else if (X >= _X + _Width - 16 && X <= _X + _Width - 2 && Y >= _Y + 2 && Y <= _Y + 16)
		return LV_POS_DOWN;
	else if (X >= _X + _Width - 16 && X <= _X + _Width - 2 && Y >= _Y + 17 && Y <= _Y + _Height - 45)
		return LV_POS_SCROLLBAR;
	else if (Y >= _Y + _Height && Y <= _Y + _Height + 5 && X >= _X - 5 && X <= _X) 
		return LV_POS_RESIZE_TOPLEFT;
	else if (Y >= _Y - 5 && Y <= _Y && X >= _X + _Width && X <= _X + _Width + 5)
		return LV_POS_RESIZE_BOTTOMRIGHT;
	else if (Y >= _Y + _Height && Y <= _Y + _Height + 5 && X >= _X + _Width && X <= _X + _Width + 5)
		return LV_POS_RESIZE_TOPRIGHT;
	else if (Y >= _Y - 5 && Y <= _Y && X >= _X - 5 && X <= _X) 
		return LV_POS_RESIZE_BOTTOMLEFT;
	else if (Y >= _Y + _Height && Y <= _Y + _Height + 5)
		return LV_POS_RESIZE_TOP;
	else if (Y >= _Y - 5 && Y <= _Y)
		return LV_POS_RESIZE_BOTTOM;
	else if (X >= _X - 5 && X <= _X)
		return LV_POS_RESIZE_LEFT;
	else if (X >= _X + _Width && X <= _X + _Width + 5)
		return LV_POS_RESIZE_RIGHT;
	//else if (Y > _Top - 28)
	else if (Y > _Y + _Height - 28 && Y < _Y + _Height && X > _X && X < _X + _Width) // Please fix this in a good way..
		return LV_POS_MOVE;
	else
		return LV_POS_NONE;
}

struct EventInfo ListView::LastEventInfo()
{
	return ei;
}

bool ListView::PointInWindow(int X, int Y)
{
	if(!_Visible)
		return false;

	if(X < _X || X > (_X + _Width))
		return false;
	
	if(Y < _Y || Y > (_Y + _Height))
		return false;

	return true; // Point must be in window
}