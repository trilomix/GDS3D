//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2021 Bertrand Pigeard
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

#include "gdsobjectlist.h"
#include <algorithm>

// ObjectTree Class
ObjectTree::ObjectTree(GDSObject *object, const GDSMat& mat)
{
	this->object = object;
	this->mat = mat;

	// Go through all ARefs and SRefs
}

ObjectTree::~ObjectTree()
{
	for(unsigned int i=0;i<leaves.size();i++)
		delete leaves[i];
	leaves.clear();
}

// GDSObjectList Class
GDSObjectList::GDSObjectList()
{
	tree = NULL;
}

GDSObjectList::~GDSObjectList()
{
	for(unsigned int i=0;i<objects.size();i++)
		delete objects[i];

	if(tree)
		delete tree;
}

GDSObject *GDSObjectList::AddObject(class GDSObject *newobject)
{
	objects.push_back(newobject);

	return newobject;
}

GDSObject *GDSObjectList::SearchObject(const char *Name, const char *gdsName)
{
	for(unsigned int i=0;i<objects.size();i++)
	{
		if (gdsName != NULL && gdsName != objects[i]->GetGDSName())
			continue;
		if(strcmp(Name, objects[i]->GetName())==0)
			return objects[i];
	}
	
	for (unsigned int i = 0; i<objects.size(); i++)
	{
		size_t namelen = strlen(objects[i]->GetName());
		char *GDSName = new char [namelen+1];
		strncpy(GDSName,Name, namelen);
		GDSName[namelen] = '\0';
		if(gdsName == NULL && objects[i]->GetGDSName()!= NULL 
			&& strcmp(Name, objects[i]->GetGDSName()) == 0 
           && strcmp(GDSName, objects[i]->GetName()) == 0) {
            //delete GDSName;
			return objects[i];
		}
        // delete GDSName;
	}

	return NULL;
}

void GDSObjectList::ConnectReferences()
{
	vector<GDSObject*> remove;
	vector<GDSObject*> rmrefObject;
	GDSObject * Object;
	vector<char*> ObjectNameList;
	GDSObject * RefObject;

	for (size_t i = 0; i < objects.size(); i++) {
		if (std::string(objects[i]->GetName()).find("metal_fill") != std::string::npos 
			|| std::string(objects[i]->GetName()).find("filling") != std::string::npos
			|| std::string(objects[i]->GetName()).find("_TILES") != std::string::npos
			) {
			remove.push_back(objects[i]);
		}
		objects[i]->ConnectReferences(this);
	}

	if (remove.size() > 0) {
		for (size_t i = 0; i < remove.size(); i++) {
			if (remove.size() > 10) {
				v_printf(0, "Remove complete %d%%\r", i*100/ remove.size());
			}
			vector<GDSObject*>::iterator it = std::find(objects.begin(), objects.end(), remove[i]);
			if (it != objects.end()) {
				Object = *it;
				// check ref Object
				for (size_t j = 0; j < Object->refs.size(); j++) {
					RefObject = Object->refs[j]->object;
					// unique 
					vector<GDSObject*>::iterator itRef = std::find(rmrefObject.begin(), rmrefObject.end(), RefObject);
					if (itRef != rmrefObject.end()) {
						continue;
					}
					bool found = false;
					for (size_t k = 0; k < objects.size(); k++) {
						GDSObject *obj = objects[k];
						if (RefObject->GetGDSName() != obj->GetGDSName())
							continue;
						if (RefObject->GetProcessName()!= NULL && obj->GetProcessName() != NULL
							&& strcmp(obj->GetProcessName(), RefObject->GetProcessName())!=0 )
							continue;
						if (strcmp(obj->GetName(), Object->GetName()) != 0) {
							if (obj->referencesToObject(RefObject->GetName())) {
								found = true;
								break;
							}
						}
					}
					if(!found) {
						rmrefObject.push_back(RefObject);
					}
				}
				// delete only reference object
				objects.erase(it);
				for (size_t j = 0; j < rmrefObject.size(); j++) {
					vector<GDSObject*>::iterator itRef = std::find(objects.begin(), objects.end(), rmrefObject[j]);
					if (itRef != objects.end())
						objects.erase(itRef);
				}
			}
		}
		if (remove.size() > 10) {
			v_printf(0, "Remove complete %d%%\r",  100 );
			v_printf(0, "\n");
		}

	}
}

bool GDSObjectList::FindObject(GDSObject * object) {
	for (unsigned int j = 0; j < objects.size(); j++)
	{
		GDSObject *obj = objects[j];

		if (obj->referencesToObject(object))
		{
			return true;
		}
	}
	return false;
}

GDSObject * GDSObjectList::GetTopObject()
{
    // Loop through objects
    for(unsigned int i=0;i<objects.size();i++)
	{
        bool found = FindObject(objects[i]);
        if(!found) // Object is not referenced by any other objects
            return objects[i];
    }
    return NULL;
}

size_t	GDSObjectList::getNumObjects()
{
	return objects.size();
}

GDSObject* GDSObjectList::getObject(unsigned int index)
{
	assert(index < objects.size());
	return objects[index];
}

void GDSObjectList::buildObjectTree()
{
	if(tree)
		delete tree;

	// This will recursively build itself
	tree = new ObjectTree(GetTopObject(), GDSMat());
}

void GDSObjectList::ClearNetList() {
	// Loop through objects
	for (unsigned int i = 0; i < objects.size(); i++)
	{
		objects[i]->CleanNetList();

	}
}