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

#ifndef __UI_HIGHLIGHT__H
#define __UI_HIGHLIGHT__H

#include "ui_element.h"

class ObjectInstance // Also sort on object*!!
{
public:
	set<GDSPolygon*> checked_poly;
	set<GDSPolygon*> unchecked_poly; 
	vector<char*> NetNames;
	//vector<GDSPolygon*> poly_pool; // Not yet implemented
	PolygonSort poly_pool;

	ObjectInstance(vector<GDSPolygon*> PolyList);
	ObjectInstance() {};

};// Use matrix as a sort key

class UIHighlight : public UIElement
{
private:    
    int state;
	int PolySize;
	float Waittime;
	bool hide_other_objects;
	ProcessLayer *cur_layer;
	GDSPolygon *cur_poly;
	GDSMat		cur_mat;
	GDSObject *cur_object;
	ObjectInstance *cur_instance;
	htime *time;

	map<KeyInstance, ObjectInstance> instances;
	vector<VECTOR3D> triangles;
	GDSObject_ogl *render_object;

	void tracePoint(float x, float y, GDSObject *object, GDSMat object_mat, ProcessLayer *layer);
	void processList();
	void intersectTraverse(GDSPolygon *poly, GDSMat poly_mat, GDSObject *object, GDSMat object_mat);
	void intersectPolyOnObject(GDSPolygon *poly, GDSMat poly_mat, GDSObject *object, GDSMat object_mat);

	map<KeyInstance, ObjectInstance>::iterator GetPolyOnNet(GDSMat object_mat, GDSObject * object, char * NetName, set<GDSPolygon*>& PolygonItemsNetRemove);

	void buildRenderObject();
	void drawTracing(bool finish);
    
public:
    UIHighlight();
    
	bool GetState();

	void Disable();
    void Reset();
    void Draw();
    bool Event(int event, int data, int xpos, int ypos , bool shift, bool control, bool alt); 
};

#endif
