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

#ifndef __GDSObjectList_H__
#define __GDSObjectList_H__

#include "gdsobject.h"

class ObjectTree
{
private:
	GDSBB						bbox;
	GDSMat						mat;
	GDSObject					*object;

	list<int>					polygons;
	vector<class ObjectTree*>	leaves;

public:
	ObjectTree(GDSObject *object, const GDSMat& mat);
	~ObjectTree();

};

class GDSObjectList
{
private:
	// List of all the objects
	vector<GDSObject*> objects;

	// Tree of all object instances for net highlighting
	ObjectTree	*tree;

public:
	GDSObjectList();
	~GDSObjectList();

	GDSObject *AddObject(class GDSObject *newobject);
	GDSObject * SearchObject(const char * Name, const char * gdsName);
	//GDSObject *SearchObject(const char *Name);
	GDSObject *GetTopObject();
	size_t	getNumObjects();
	GDSObject* getObject(unsigned int index);

	void ConnectReferences();

	bool FindObject(GDSObject * object);

	// For net highlighting
	void	buildObjectTree();
	void ClearNetList();
};

#endif // __GDSObjectList_H__
