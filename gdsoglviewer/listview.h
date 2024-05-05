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

#ifndef _listview_H
#define _listview_H

#include <string>
#include <vector>

typedef int GLint;
typedef unsigned int GLuint;
typedef float GLfloat;

#define LV_EVENT_NONE				0x00
#define LV_EVENT_CLICK				0x01
#define LV_EVENT_MOVE				0x02
#define LV_EVENT_RESIZE				0x03
#define LV_EVENT_SCROLL				0x04
#define LV_EVENT_SELECTED_CHANGED	0x06
#define LV_EVENT_STATE_CHANGED		0x07

#define LV_MOUSE_DEFAULT			0x00
#define LV_MOUSE_MOVE				0x01
#define LV_MOUSE_RESIZE_H			0x02
#define LV_MOUSE_RESIZE_V			0x03
#define LV_MOUSE_RESIZE_D1			0x04
#define LV_MOUSE_RESIZE_D2			0x05
#define LV_MOUSE_RESIZE_D3			0x06
#define LV_MOUSE_RESIZE_D4			0x07

#define LV_OP_NONE					0x00
#define LV_OP_MOVE					0x01
#define LV_OP_SCROLLBAR				0x02
#define LV_OP_RESIZE_LEFT			0x03
#define LV_OP_RESIZE_RIGHT			0x04
#define LV_OP_RESIZE_TOP			0x05
#define LV_OP_RESIZE_BOTTOM			0x06
#define LV_OP_RESIZE_TOPLEFT		0x07
#define LV_OP_RESIZE_TOPRIGHT		0x08
#define LV_OP_RESIZE_BOTTOMLEFT		0x09
#define LV_OP_RESIZE_BOTTOMRIGHT	0x10

#define LV_POS_NONE					0x00
#define LV_POS_UP					0x01
#define LV_POS_DOWN					0x02
#define LV_POS_SCROLLBAR			0x03
#define LV_POS_RESIZE_LEFT			0x04
#define LV_POS_RESIZE_RIGHT			0x05
#define LV_POS_RESIZE_TOP			0x06
#define LV_POS_RESIZE_BOTTOM		0x07
#define LV_POS_RESIZE_TOPLEFT		0x08
#define LV_POS_RESIZE_TOPRIGHT		0x09
#define LV_POS_RESIZE_BOTTOMLEFT	0x10
#define LV_POS_RESIZE_BOTTOMRIGHT	0x11
#define LV_POS_MOVE					0x12

#define LV_INDENT					20

struct ListItem {
	struct ListItem *Parent;		// Parent item
	vector <ListItem*> Children;	// Pointers to children
	struct ListItem *PrevItem;		// Previous item in list at same level
	struct ListItem *NextItem;		// Next item in list at same level
	string Text;					// Item text
	int Index;						// Index of the item within its parent node's children list
	int Level;						// Depth of item in hierarchy
	int State;						// State (-1 = no children, 0 = collapsed, 1 = expanded)
	bool Selected;					// Item is selected
	float Red;						// Red value (for legend)
	float Green;					// Green value (for legend)
	float Blue;						// Blue value (for legend)
};

struct EventInfo {
	int EventID;
	int EventCursor;
	struct ListItem *Item;
};

class ListView
{
private:
	string _Title;					// Title text
	struct ListItem *_RootNode;		// Root item, invisible
	struct ListItem *_FirstVisible;	// First visible item in list
	int _Count;						// Number of items
	int _Depth;						// Number of hierarchy levels
	bool _Visible;					// List visibility
    bool _Sorted;					// Indicates whether the listitems are sorted
	int _X;							// X coordinate (bottom-left)
	int _Y;							// Y coordinate (bottom-left)
	int _Height;					// List height
	int _Width;						// List width
	int _ScreenWidth;				// Screen width
	int _ScreenHeight;				// Screen height
	bool _SingleSelect;				// Multi or single selection mode
	bool _ColorBox;					// Display of color boxes for legend
	bool _SelectionBox;				// Display of selection boxes
	int _DragX;						// X coordinate within ListView for mouse drag
	int _DragY;						// Y coordinate within ListView for mouse drag
	int _MouseDragOperation;		// Operation to be performed upon mouse drag
	EventInfo  ei;					// Lekker, hmm...

	bool delete_children(struct ListItem *item);
	void clear_selected();
	void clear_selected(struct ListItem *item);
	void recalc_depth();
	void recalc_depth(struct ListItem *parent_item);
	void sort_children(struct ListItem *parent_item);

	int count_visible(struct ListItem *start_item);
	struct ListItem *find_visible(struct ListItem *start_item, int n);
	bool is_visible(struct ListItem *item);
	
	void inc_firstvisible();
	void dec_firstvisible();
	void recalc_firstvisible();

	void gl_triangle(GLint x1, GLint y1, GLint x2, GLint y2, GLint x3, GLint y3);
	void gl_square(GLint x1, GLint y1, GLint x2, GLint y2, bool filled);
	void gl_text(GLint x, GLint y, GLfloat red, GLfloat green, GLfloat blue, string text, int maxwidth);
	void resize_top(int Y);
	void resize_bottom(int Y);
	void resize_left(int X);
	void resize_right(int X);
	int mouse_pos(int X, int Y);

protected:
	struct EventInfo LastEventInfo();
	
public:
	ListView();
	~ListView();

	virtual void Draw();
	virtual bool Event(int event, int data, int x, int y, bool shift, bool control, bool alt); // Keep arguments lower case!!

	void SetPos(int X, int Y);
	void SetSize(int Width, int Height);
	void SetScreenSize(int Width, int Height);
	void SetTitle(string Title);
	void SetVisibility(bool Visible);
	void SetSorted(bool Sorted);
	void SetSingleSelect(bool SingleSelect);
	void SetColorBox(bool Visible);
	void SetSelectionBox(bool Visible);
	void SetRulerBox(bool Visible);

	int GetCount();
	int GetY();
	int GetX();
	int GetWidth();
	int GetHeight();
	bool GetVisibility();
	bool GetSorted();
	bool GetSingleSelect();
	bool GetColorBox();
	bool GetSelectionBox();
	bool GetRulerBox();

	void Clear();
	struct ListItem *AddItem(struct ListItem *NewListItem);
	struct ListItem *AddItem(struct ListItem *Parent, struct ListItem *NewListItem);
	bool RemoveItem(struct ListItem *Item);
	struct ListItem *GetItem(unsigned int Index);
	struct ListItem *GetItem(struct ListItem *Parent, unsigned int Index);
	struct ListItem *GetFirst();
	string GetTitle();
	
	bool PointInWindow(int X, int Y);
};

#endif // _listview_H