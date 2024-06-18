//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2017 Bertrand Pigeard
//  Copyright (C) 2013 IC-Design Group, University of Twente.
//
//  Based on gds2pov by Roger Light, http://atchoo.org/gds2pov/ / https://github.com/ralight/gds2pov
//  Copyright (C) 2004-2008 by Roger Light
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//  
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//  
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
