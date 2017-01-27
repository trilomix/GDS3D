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

#include "gds_globals.h"
#include "gdsobject.h"
#include "gdsobjectlist.h"
#include <cmath> 
#include <algorithm>



#define HIERARCHY_LIMIT  30000
#define POLYAREAGROUP 500

KeyInstance::KeyInstance(class GDSObject *object, GDSMat Mat) {
	this->object = object;
	this->Mat = Mat;
	this->ref.object = object;
	this->ref.mat = Mat;
}

KeyInstance::KeyInstance(struct GDSRef ref)
{
	this->Mat = ref.mat;
	this->object = ref.object;
	this->ref = ref;
}



GDSObject::GDSObject(char *NewName, char *gdsName)
{
    PointCount = 0;
    noHierarchy = false;
	collapsed = false;

	hasBoundary = false;
	has3DBoundary = false;
    
	Name = new char[strlen(NewName)+1];
	strcpy(Name, NewName); 
	
	GDSName = gdsName;

    PCell = false;
}

GDSObject::~GDSObject()
{
	for(unsigned int i=0;i<PolygonItems.size();i++)
		delete PolygonItems[i];

	for(unsigned int i=0;i<PathItems.size();i++)
		delete PathItems[i];

	for(unsigned int i=0;i<TextItems.size();i++)
		delete TextItems[i];

	for(unsigned int i=0;i<SRefItems.size();i++)
	{
		if(SRefItems[i]->Name)
			delete [] SRefItems[i]->Name;
		delete SRefItems[i];
	}

	for(unsigned int i=0;i<ARefItems.size();i++)
	{
		if(ARefItems[i]->Name)
			delete [] ARefItems[i]->Name;
		delete ARefItems[i];
	}

	for(unsigned int i=0;i<refs.size();i++)		
		delete refs[i];	
    
	if (Object_Nets.size() > 0) {
		Object_Nets.CleanNetName();
	}

	delete [] Name;
}

void GDSObject::AddText(float newX, float newY, float newZ, bool newFlipped, float newMag, int newVJust, int newHJust, struct ProcessLayer *newlayer)
{
	TextItems.push_back(new class GDSText(newX, newY, newZ, newFlipped, newMag, newVJust, newHJust, newlayer));
}

class GDSText *GDSObject::GetCurrentText()
{
	if(TextItems.size()){
		return TextItems[TextItems.size()-1];
	}else{
		return NULL;
	}
}

char *GDSObject::GetName()
{
	return Name;
}

char *GDSObject::GetGDSName()
{
	return GDSName;
}

void GDSObject::AddPolygon(double Height, double Thickness, size_t Points, struct ProcessLayer *layer)
{
	if (GetCurrentPolygon() != NULL) {
		if (GetCurrentPolygon()->GetPoints() == 0) {
			// Empty polygon Remove it
			PolygonItems.pop_back();
		}
	}

	PolygonItems.push_back(new class GDSPolygon(Height, Thickness, layer));

    PointCount += Points*2;
}

class GDSPolygon *GDSObject::GetCurrentPolygon()
{
    if(PolygonItems.size()>0)
        return PolygonItems[PolygonItems.size()-1];
    else
        return NULL;
}

void GDSObject::AddSRef(char *Name, float X, float Y, int Flipped, float Mag)
{
	SRefElement *NewSRef = new SRefElement;
      
	NewSRef->Name = new char[strlen(Name)+1];
	strcpy(NewSRef->Name, Name);
	NewSRef->X = X;
	NewSRef->Y = Y;
	NewSRef->Rotate.X = 0.0;
	NewSRef->Rotate.Y = 0.0;
	NewSRef->Rotate.Z = 0.0;
	NewSRef->Flipped = Flipped;
	NewSRef->Mag = Mag;
	NewSRef->object = NULL;
    NewSRef->collapsed = false;
    
	SRefItems.push_back(NewSRef);
}

void GDSObject::SetSRefRotation(float X, float Y, float Z)
{
	if(SRefItems.size() > 0)
	{
		SRefItems[SRefItems.size()-1]->Rotate.X = X;
		SRefItems[SRefItems.size()-1]->Rotate.Y = Y;
		SRefItems[SRefItems.size()-1]->Rotate.Z = Z;
	}
}

void GDSObject::AddARef(char *Name, float X1, float Y1, float X2, float Y2, float X3, float Y3, int Columns, int Rows, int Flipped, float Mag)
{
	ARefElement *NewARef = new ARefElement;
    
	NewARef->Name = new char[strlen(Name)+1];
	strcpy(NewARef->Name, Name);
	NewARef->X1 = X1;
	NewARef->Y1 = Y1;
	NewARef->X2 = X2;
	NewARef->Y2 = Y2;
	NewARef->X3 = X3;
	NewARef->Y3 = Y3;
	NewARef->Columns = Columns;
	NewARef->Rows = Rows;
	NewARef->Rotate.X = 0.0;
	NewARef->Rotate.Y = 0.0;
	NewARef->Rotate.Z = 0.0;
	NewARef->Flipped = Flipped;
	NewARef->Mag = Mag;
	NewARef->object = NULL;
    NewARef->collapsed = false;

	ARefItems.push_back(NewARef);
}

void GDSObject::SetARefRotation(float X, float Y, float Z)
{
	if(ARefItems.size() > 0)
	{
		ARefItems[ARefItems.size()-1]->Rotate.X = X;
		ARefItems[ARefItems.size()-1]->Rotate.Y = Y;
		ARefItems[ARefItems.size()-1]->Rotate.Z = Z;
	}
}

/*
struct _Boundary *GDSObject::GetTotalBoundary(class MATRIX4X4 *modelview)
{
	MATRIX4X4 trans;
	MATRIX4X4 temp;
	AA_BOUNDING_BOX bbox;
	VECTOR3D point;
	float dx1, dx2, dy1, dy2;
	int i,j;
    
	if(GotBoundary){
		return &Boundary;
	}
    
	class GDSPolygon *polygon;
	for(unsigned long i=0; i<PolygonItems.size(); i++){
		polygon = PolygonItems[i];
		if(!polygon->GetLayer()->Show)
			continue;
		for(unsigned int j=0; j<polygon->GetPoints(); j++)
		{
			point.Set(polygon->GetXCoords(j), polygon->GetYCoords(j), 0.0);

			if(point.GetX() > Boundary.XMax){
				Boundary.XMax = point.GetX();
			}
			if(point.GetX() < Boundary.XMin){
				Boundary.XMin = point.GetX();
			}
			if(point.GetY() > Boundary.YMax){
				Boundary.YMax = point.GetY();
			}
			if(point.GetY() < Boundary.YMin){
				Boundary.YMin = point.GetY();
			}
		}
	}
    
	// FIXME!!!!!!
	if(FirstSRef){
		SRefElement dummysref;
		dummysref.Next=FirstSRef;
		SRefElement *sref = &dummysref;
        
		struct ObjectList *object;
		struct _Boundary *NewBound;
        
		while(sref->Next){
			sref = sref->Next;
			if(strcmp(sref->Name, this->Name)!=0){
				object = &dummyobject;
                
				// Build transformation matrix
				trans.LoadIdentity();
				if(sref->Mag!=1.0)
				{
					temp.SetScale(VECTOR3D(sref->Mag, sref->Mag, 1.0f));
					trans = trans * temp;
				}
				temp.SetTranslation(VECTOR3D(sref->X, sref->Y, 0.0f));
				trans = trans * temp;
				if(sref->Rotate.Y)
				{
					temp.SetRotationAxis(-sref->Rotate.Y, VECTOR3D(0.0f, 0.0f, 1.0f));
					trans = trans * temp;
				}
				if(sref->Flipped)
				{
					temp.SetScale(VECTOR3D(1.0f, -1.0f, 1.0f));
					trans = trans * temp;
				}
                
				while(object->Next){
					object = object->Next;
					if(strcmp(object->Object->GetName(), sref->Name)==0){
						NewBound = object->Object->GetBoundary(objectlist, NULL);
						bbox.SetFromMinsMaxes(VECTOR3D(NewBound->XMin,NewBound->YMin,0.0), VECTOR3D(NewBound->XMax,NewBound->YMax,0.0));
						bbox.Mult(trans); // Transform bounding box
                        
						if(bbox.maxes.GetX() > Boundary.XMax){
							Boundary.XMax = bbox.maxes.GetX();
						}
						if(bbox.mins.GetX() < Boundary.XMin){
							Boundary.XMin = bbox.mins.GetX();
						}
						if(bbox.maxes.GetY() > Boundary.YMax){
							Boundary.YMax = bbox.maxes.GetY();
						}
						if(bbox.mins.GetY() < Boundary.YMin){
							Boundary.YMin = bbox.mins.GetY();
						}
						break;
					}
				}
			}
		}
	}
    
	if(FirstARef){
		ARefElement dummyaref;
		dummyaref.Next = FirstARef;
        
		ARefElement *aref = &dummyaref;
        
		struct ObjectList *object;
		dummyobject.Next = objectlist;
		
		struct _Boundary *NewBound;
		while(aref->Next){
			aref = aref->Next;
			if(strcmp(aref->Name, this->Name)!=0){
				object = &dummyobject;
				object = &dummyobject;
                
				while(object->Next){
					object = object->Next;
					if(strcmp(object->Object->GetName(), aref->Name)==0){
                        
						dx1 = (float)(aref->X2 - aref->X1) / (float)aref->Columns;
						dy1 = (float)(aref->Y2 - aref->Y1) / (float)aref->Columns;
						dx2 = (float)(aref->X3 - aref->X1) / (float)aref->Rows;
						dy2 = (float)(aref->Y3 - aref->Y1) / (float)aref->Rows;
                        
						for(i=0; i<aref->Rows; i++){
							for(j=0; j<aref->Columns; j++){
                                
                                // Build transformation matrix
                                trans.LoadIdentity();
                                if(aref->Mag!=1.0)
                                {
                                    temp.SetScale(VECTOR3D(aref->Mag, aref->Mag, 1.0f));
                                    trans = trans * temp;
                                }
                                temp.SetTranslation(VECTOR3D(aref->X1+dx1*(float)j+dx2*(float)i, aref->Y1+dy2*(float)i+dy1*(float)j, 0.0f));
                                trans = trans * temp;
                                if(aref->Rotate.Y)
                                {
                                    temp.SetRotationAxis(-aref->Rotate.Y, VECTOR3D(0.0f, 0.0f, 1.0f));
                                    trans = trans * temp;
                                }
                                if(aref->Flipped)
                                {
                                    temp.SetScale(VECTOR3D(1.0f, -1.0f, 1.0f));
                                    trans = trans * temp;
                                }
                                
                                NewBound = object->Object->GetBoundary(objectlist, NULL);
                                bbox.SetFromMinsMaxes(VECTOR3D(NewBound->XMin,NewBound->YMin,0.0), VECTOR3D(NewBound->XMax,NewBound->YMax,0.0));
                                bbox.Mult(trans); // Transform bounding box
                                
                                if(bbox.maxes.GetX() > Boundary.XMax){
                                    Boundary.XMax = bbox.maxes.GetX();
                                }
                                if(bbox.mins.GetX() < Boundary.XMin){
                                    Boundary.XMin = bbox.mins.GetX();
                                }
                                if(bbox.maxes.GetY() > Boundary.YMax){
                                    Boundary.YMax = bbox.maxes.GetY();
                                }
                                if(bbox.mins.GetY() < Boundary.YMin){
                                    Boundary.YMin = bbox.mins.GetY();
                                }
                                
							}
						}
					}
				}
			}
		}
	}
    
	if(PathItems.empty() && PolygonItems.empty() && !FirstSRef && !FirstARef){
		Boundary.XMax = Boundary.XMin = Boundary.YMax = Boundary.YMin = 0;
	}
    
	v_printf(2, "%-30s XMax=%8.2f \tXMin=%8.2f \tYMax= %8.2f \tYMin=%8.2f\n", Name, Boundary.XMax, Boundary.XMin, Boundary.YMax, Boundary.YMin);
	GotBoundary = true;
    
	return &Boundary;
}*/

bool GDSObject::Has3DBBox() {
	return has3DBoundary;
}
void GDSObject::ResetBBox() {
	if (has3DBoundary) {
		has3DBoundary = false;
		_3Dboundary.clear();
	}
	if (hasBoundary) {
		hasBoundary = false;
		boundary.clear();
	}
}

GDS3DBB GDSObject::GetTotal3DBoundary() {
	GDS3DBB _3DBB, _3Dt;
	GDSPolygon *polygon;
	if (has3DBoundary)
		return _3Dboundary;
	
	for (unsigned int i = 0; i<PolygonItems.size(); i++)
	{
		polygon = PolygonItems[i];
		if (!polygon->GetLayer()->Show)
			continue;

		_3DBB.merge(*polygon->Get3DBBox());
	}
	for (unsigned int i = 0; i < refs.size(); i++)
	{
		_3Dt = refs[i]->object->GetTotal3DBoundary();
		if (_3Dt.min.X <= _3Dt.max.X && _3Dt.min.Y <= _3Dt.max.Y && _3Dt.min.Z <= _3Dt.max.Z) {
			// If no thing in the object BB is still default value
			_3Dt.transform(refs[i]->mat);
		}
		_3DBB.merge(_3Dt);
	}
	
	_3Dboundary = _3DBB;
	has3DBoundary = true;
	return _3DBB;
}

GDSBB GDSObject::GetTotalBoundary()
{
	GDSBB BB, t;
	GDSPolygon *polygon;

	if(hasBoundary)
		return boundary;

	for(unsigned int i=0; i<PolygonItems.size(); i++)
	{
		polygon = PolygonItems[i];
		if(!polygon->GetLayer()->Show)
			continue;

		BB.merge(*polygon->GetBBox());
	}

	for(unsigned int i=0;i<refs.size();i++)
	{
		t = refs[i]->object->GetTotalBoundary();
		if (!t.isEmpty()) {
			// If no thing in the object, BB is still default value
			t.transform(refs[i]->mat);
		}
		BB.merge(t);
	}

	boundary = BB;
	hasBoundary = true;
	return BB;
}

void GDSObject::AddPath(int PathType, float Height, float Thickness, int Points, float Width, float BgnExtn, float EndExtn, struct ProcessLayer *layer)
{
	PathItems.push_back(new class GDSPath(PathType, Height, Thickness, Points, Width, BgnExtn, EndExtn, layer));

    PointCount += Points*4;
}

class GDSPath *GDSObject::GetCurrentPath()
{
	if(PathItems.size() < 1) // No pathitems
		return NULL;

	return PathItems[PathItems.size()-1];
}



void GDSObject::ConnectReferences(class GDSObjectList *Objects)
{
	GDSMat M;
	double dx1, dx2, dy1, dy2;
	int i,j;

	 //Find SRef objects
	for(unsigned int k=0;k<SRefItems.size();k++)
	{
		SRefElement *sref = SRefItems[k];
		sref->object = Objects->SearchObject(sref->Name, GDSName);

		if (std::string(sref->Name).find("metal_fill") != std::string::npos || std::string(sref->Name).find("filling") != std::string::npos
			|| std::string(sref->Name).find("_TILES") != std::string::npos
			) {
			v_printf(1, "Remove filling Object Name : %s\n", sref->Name);
			sref->object = NULL;
		}


		// If not found, remove from SRef list
		if(!sref->object)
		{
			SRefItems.erase(SRefItems.begin()+k);
			k--; // Check the new SRef on this index again
			continue;
		}

		// Decode 2D transformation matrix
		GDSRef *newRef = new GDSRef;
		newRef->object = sref->object;

		newRef->mat.loadIdentity();
		if(sref->Mag!=1.0)
		{
			M.setScaling(sref->Mag, sref->Mag);
			newRef->mat = newRef->mat * M;
		}
		M.setTranslation(sref->X, sref->Y);
		newRef->mat = newRef->mat * M;
		if(sref->Rotate.Y)
		{
			M.setRotation(-sref->Rotate.Y);
			newRef->mat = newRef->mat * M;
		}
		if(sref->Flipped)
		{
			M.setScaling(1.0f, -1.0f);
			newRef->mat = newRef->mat * M;
		}
		
		// Round matrix to avoid small errors
		newRef->mat.Round();

		// Add
		refs.push_back(newRef);
	}

	//Find ARefs objects
	for(unsigned int k=0;k<ARefItems.size();k++)
	{
		ARefElement *aref = ARefItems[k];
		aref->object = Objects->SearchObject(aref->Name,GDSName);

		// If not found, remove from ARef list
		if(!aref->object)
		{
			ARefItems.erase(ARefItems.begin()+k);
			k--; // Check the new ARef on this index again
			continue;
		}

		dx1 = (double)(aref->X2 - aref->X1) / (double)aref->Columns;
		dy1 = (double)(aref->Y2 - aref->Y1) / (double)aref->Columns;
		dx2 = (double)(aref->X3 - aref->X1) / (double)aref->Rows;
		dy2 = (double)(aref->Y3 - aref->Y1) / (double)aref->Rows;

		for(i=0; i<aref->Rows; i++)
		{
			for(j=0; j<aref->Columns; j++)
			{
				// Decode 2D transformation matrix
				GDSRef *newRef = new GDSRef;
				newRef->object = aref->object;

				// Build transformation matrix      
				newRef->mat.loadIdentity();
				if(aref->Mag!=1.0)
				{
					M.setScaling(aref->Mag, aref->Mag);
					newRef->mat = newRef->mat * M;
				}
				M.setTranslation(aref->X1+dx1*(double)j+dx2*(double)i, aref->Y1+dy2*(double)i+dy1*(double)j);
				newRef->mat = newRef->mat * M;
				if(aref->Rotate.Y)
				{
					M.setRotation(-aref->Rotate.Y);
					newRef->mat = newRef->mat * M;
				}
				if(aref->Flipped)
				{
					M.setScaling(1.0f, -1.0f);
					newRef->mat = newRef->mat * M;
				}
				
				// Round matrix to avoid small errors
				newRef->mat.Round();

				// Add
				refs.push_back(newRef);
			}
		}
	}
}

size_t GDSObject::GetNumSRefs()
{
	return SRefItems.size();
}

SRefElement* GDSObject::GetSRef(unsigned int index)
{
	assert(index < SRefItems.size());
	return SRefItems[index];
}

size_t GDSObject::GetNumARefs()
{
	return ARefItems.size();
}

ARefElement* GDSObject::GetARef(unsigned int index)
{
	assert(index < ARefItems.size());
	return ARefItems[index];
}

bool GDSObject::isPCell()
{
	return PCell;
}

void GDSObject::printHierarchy(int depth)
{
    for(int i=0;i<depth;i++)
        v_printf(2, "  ");
    v_printf(2, "%s, %d total points\n", Name, AccumPointCount);
    
    if(noHierarchy)
        return;
    
    for(unsigned int i=0;i<SRefItems.size();i++)
	{
            GDSObject* obj = SRefItems[i]->object;
            if(obj && ! SRefItems[i]->collapsed)
                obj->printHierarchy(depth+1);
    }

	for(unsigned int i=0;i<ARefItems.size();i++)
	{
            GDSObject* obj = ARefItems[i]->object;
            if(obj && ! ARefItems[i]->collapsed)
                obj->printHierarchy(depth+1);
    }    
}

size_t GDSObject::countTotalPoints()
{
    AccumPointCount = PointCount;
    char * dummy2 = NULL;
    char *dummy3 = NULL;
    
    // PCell detection, try to find something like __1018272
    dummy2 = strstr(Name, "__");
    if(dummy2)
    {
        dummy2+=2;
        dummy3 = dummy2;
        while(dummy2)
        {   
            dummy2 = strstr(dummy2, "__");
            if(dummy2)
            {
                dummy2 += 2;
                dummy3 = dummy2;
            }
        }
    }
    if(dummy3)
    {
        if(atoi(dummy3)) 
            PCell = true;
    }
    // PCell detection, try to find something like ___1018272
    dummy2 = strstr(Name, "___");
    if(dummy2)
    {
        dummy2+=3;
        dummy3 = dummy2;
        while(dummy2)
        {   
            dummy2 = strstr(dummy2, "___");
            if(dummy2)
            {
                dummy2 += 3;
                dummy3 = dummy2;
            }
        }
    }
    if(dummy3)
    {
        if(atoi(dummy3)) 
            PCell = true;
    }
    
	/*
    // Count SRefs
   for(unsigned int i=0;i<SRefItems.size();i++)
	{
            GDSObject* obj = SRefItems[i]->object;
            if(obj  && !SRefItems[i]->collapsed && !noHierarchy)
                AccumPointCount += obj->countTotalPoints();            
    }
    
    // Count ARefs
   for(unsigned int i=0;i<ARefItems.size();i++)
	{
            GDSObject* obj = ARefItems[i]->object;
            if(obj && !ARefItems[i]->collapsed && !noHierarchy)
                AccumPointCount += obj->countTotalPoints();          
    }*/
	for(unsigned int i=0;i<refs.size();i++)
		AccumPointCount += refs[i]->object->countTotalPoints();
    
    return AccumPointCount;
}

void GDSObject::collapseHierachy()
{
    // Traverse hierarchy
	//v_printf(1, ".");
	bool SameGDS = true;
    for(unsigned int i=0;i<SRefItems.size();i++)
	{
		GDSObject *cur_object = SRefItems[i]->object;
		if (SameGDS && ((GetGDSName() == NULL && cur_object->GetGDSName() != NULL) || cur_object->GetGDSName() != GetGDSName()))
			SameGDS = false;
		if(!cur_object->collapsed)
			cur_object->collapseHierachy();
	}
	for(unsigned int i=0;i<ARefItems.size();i++)
	{
		GDSObject *cur_object = ARefItems[i]->object;
		if (SameGDS && ((GetGDSName() == NULL && cur_object->GetGDSName() != NULL) || cur_object->GetGDSName() != GetGDSName()))
			SameGDS = false;
		if(!cur_object->collapsed)
			cur_object->collapseHierachy();
	}
  
    // Collapse total cell?
    if(SameGDS && AccumPointCount < HIERARCHY_LIMIT)
        noHierarchy = true;
    
	GDSObject *obj;
	vector<int> remove;
	for(unsigned int i=0;i<refs.size();i++)
	{
		obj = refs[i]->object;
		if(noHierarchy || ((obj->AccumPointCount < HIERARCHY_LIMIT/10) && SameGDS) )
		{
			remove.push_back(i);
			TransformAddObject(obj, refs[i]->mat);
		}
	}
	// This really hurts performance
	if(remove.size() > 0)
	{
		for(size_t i=1;i<remove.size()+1;i++)
        {
            delete refs[remove[remove.size()-i]]; // Remember refs are new
			refs.erase(refs.begin()+remove[remove.size()-i]);
        }
	}

	collapsed = true;
}

void GDSObject::TransformAddObject(GDSObject *obj, GDSMat mat)
{
    GDSPolygon *polygon;
    
    // Polygons
    if(!obj->PolygonItems.empty())
    {
		for(unsigned long i=0; i<obj->PolygonItems.size(); i++)
        {
			// Copy the polygon into the object
            AddPolygon(obj->PolygonItems[i]->GetHeight(), obj->PolygonItems[i]->GetThickness(),obj->PolygonItems[i]->GetPoints(), obj->PolygonItems[i]->GetLayer());
			polygon = GetCurrentPolygon();
			obj->PolygonItems[i]->CopyInto(polygon);
            
			// Transform polygon
            polygon->transformPoints(mat);            
            
            // Flipped by transformation?
            if(mat.NegativeTrace())
                polygon->Flip();
        }
    }
}

bool 
GDSObject::referencesToObject(char *name)
{
	//SRefs
	for (unsigned int i = 0; i < SRefItems.size(); i++)
	{
		SRefElement *sref = SRefItems[i];

		if (!strcmp(sref->object->GetName(), name))
			return true;
	}

	//ARefs
	for (unsigned int i = 0; i < ARefItems.size(); i++)
	{
		ARefElement *aref = ARefItems[i];


		if (!strcmp(aref->object->GetName(), name))
			return true;
	}

	return false;
}

char * GDSObject::GetProcessName() {
	if (PolygonItems.size() > 0)
		return PolygonItems[0]->GetLayer()->ProcessName;
	else
	{
		if (refs.size() > 0)
		{
			size_t i = 0;
			while (refs[i]->object->GetProcessName() == NULL) {
				i++;
			}
			return refs[i]->object->GetProcessName();
		}
		else
			return NULL;
	}
}

void GDSObject::SetNetName(GDSPolygon *poly) {
	Object_Nets.SetNetName(poly);

	// Find all other poly connect to this poly
	FindAllConnectPoly(poly);
}

size_t GDSObject::GetNetsSize() {
	return Object_Nets.size();
}

vector<char*> GDSObject::GetNetsNames() {
	return Object_Nets.GetNetsNames();
}

vector<GDSPolygon*> GDSObject::GetPolyOnNet(char *NetName) {
	return Object_Nets.GetPolyOnNet(NetName);
}

void GDSObject::FindAllConnectPoly(GDSPolygon *poly) {
	unchecked_poly.insert(poly);
	if (PolygonItems.size() > POLYAREAGROUP*10) {
		v_printf(2, "Cell:%s Total Polygons:%d\n", this->Name, PolygonItems.size());
		v_printf(2, "NetName:%s \n Total pool_poly:%d, Total NetFound:%d\n", poly->GetNetName(), pool_poly.GetPolyBySpaceSize(), Object_Nets.size());
	}
	
	while (unchecked_poly.size()>0) {

		GDSPolygon *poly_ToCheck = *unchecked_poly.begin();

		// Move to checked list
		unchecked_poly.erase(poly_ToCheck);
		checked_poly.insert(poly_ToCheck);

		intersectPoly(poly_ToCheck);
	}
}

void GDSObject::SetPool_Poly() {
	if (pool_poly.GetPolyBySpaceSize() == 0)
	{
		pool_poly.Add(PolygonItems);
	}
}

vector<GDSPolygon*> GDSObject::GetPolyNear(Point2D P) {
	GDSBB BB;
	BB.addPoint(P);

	SetPool_Poly();
	return pool_poly.GetPolyNear(BB);
}

vector<GDSPolygon*> GDSObject::GetPolyNear(GDSBB BB) {
	SetPool_Poly();
	return pool_poly.GetPolyNear(BB);
}

void GDSObject::intersectPoly(GDSPolygon *poly)
{
	GDSPolygon *target_poly;
	vector<GDSPolygon*> PolygonToCheckItems;
	vector<GDSPolygon*> PolygonRemoveItems;

	SetPool_Poly();
	
	PolygonToCheckItems = pool_poly.GetPolyNear(*poly->GetBBox());

	for (unsigned int i = 0; i < PolygonToCheckItems.size(); i++)
	{
		target_poly = PolygonToCheckItems[i];

		if (!target_poly->GetLayer()->Show)
			continue;

		if (target_poly->GetLayer() != poly->GetLayer()) {
			if (target_poly->GetHeight() > poly->GetHeight() + poly->GetThickness() + 1.0e-3f)
				continue; // Too high
			if (target_poly->GetHeight() + target_poly->GetThickness() + 1.0e-3f < poly->GetHeight())
				continue; // Too low
			if (target_poly->GetLayer()->Metal == poly->GetLayer()->Metal)
				continue; // Only jump between VIA -> METAL or METAL -> VIA
		}
		else {
			if (!target_poly->GetLayer()->Metal)
				continue; // Do not intersect within VIA layers
		}

		// Do bounds overlap?
		if (!GDSBB::intersect(*poly->GetBBox(), *target_poly->GetBBox()))
			continue;

		// Do we already have this polygon?
		if (checked_poly.find(target_poly) != checked_poly.end())
			continue; // Found it
		if (unchecked_poly.find(target_poly) != unchecked_poly.end())
			continue; // Found it

		// Intersects with polygon?
		if (!GDSPolygon::intersect(poly, target_poly))
			continue;

		// remove from pool
		PolygonRemoveItems.push_back(target_poly);

		if (false && !target_poly->GetLayer()->Metal) {
			// Via detect
			bool Top_nBottom;
			bool Add = false;
			if (abs(poly->Get3DBBox()->min.Z - target_poly->Get3DBBox()->max.Z) < 0.001) {
 				// From Upper Layer
				Top_nBottom = true;
			}
			if (abs(target_poly->Get3DBBox()->min.Z - poly->Get3DBBox()->max.Z) < 0.001) {
				// From Lower Layer
				Top_nBottom = false;
			}
			for (size_t j = 0; j< Object_Nets.GetPolyOnNet(poly->GetNetName()).size(); j++) {
				GDSPolygon *poly_checked = Object_Nets.GetPolyOnNet(poly->GetNetName())[j];
				
				if (!poly_checked->GetLayer()->Metal)
					continue; // Only Metal
				if (poly_checked->GetLayer() != target_poly->GetLayer()) {
					if (poly_checked->Get3DBBox()->min.Z > target_poly->Get3DBBox()->max.Z)
						continue; // Too high
					if (poly_checked->Get3DBBox()->max.Z < target_poly->Get3DBBox()->min.Z)
						continue; // Too low
				}
				if (Top_nBottom) {
					if (poly_checked->Get3DBBox()->min.Z > target_poly->Get3DBBox()->min.Z) {
						continue; // Reject Upper Layer
					}
				}
				else {
					if (poly_checked->Get3DBBox()->max.Z < target_poly->Get3DBBox()->max.Z) {
						continue; // Reject Lower Layer
					}
				}
				
				// Do bounds overlap?
				if (!GDSBB::intersect(*poly_checked->GetBBox(), *target_poly->GetBBox()))
					continue;
				// Intersects with polygon?
				if (!GDSPolygon::intersect(poly_checked, target_poly))
					continue;
				Add = true;
				break;
			}
			if (Add) {
				// Add Poly To net
				Object_Nets.AddPolyToNet(target_poly, poly->GetNetName());
				checked_poly.insert(target_poly);
				continue;
			}
		}

		// Add Poly To net
		Object_Nets.AddPolyToNet(target_poly, poly->GetNetName());

		// Add to unchecked list
		unchecked_poly.insert(target_poly);
	}

	if (PolygonRemoveItems.size() > 0) {
		// Start from the end to remove the correct indexed object
		for (size_t i = PolygonRemoveItems.size(); i > 0; --i) {
			//pool_poly.erase(pool_poly.begin() + PolygonRemoveItems[i - 1]);
			pool_poly.Remove(PolygonRemoveItems[i-1]);
		}
	}
}

Nets GDSObject::GetNetlist()
{
	return Object_Nets;
}

void GDSObject::CleanNetList() {
	Object_Nets.CleanNetName();
	for(size_t i =0; i< PolygonItems.size(); i++) {
		
		PolygonItems[i]->SetNetName((char*)"None");
	}
	pool_poly.ClearPolys();
}

// Net Class
Net::Net()
{
	_hasBBox = false;
}

Net::~Net()
{
}


GDS3DBB Net::GetBB()
{
	return BBox;
}

void Net::AddPoly(GDSPolygon * poly)
{
	polys.Add(poly);
	BBox.merge(*poly->Get3DBBox());
}

vector<GDSPolygon*> Net::GetPolys()
{
	return polys.Get();
}

vector<GDSPolygon*> Net::GetPolysNear(GDSBB BB) {
	return polys.GetPolyNear(BB);
}

// Nets
Nets::~Nets() {
}
void Nets::CleanNetName() {
	for (set<char*>::iterator it = NetList.begin(); it != NetList.end(); it++) {
		delete *it;
	}
	NetList.clear();
	PolyByNet.clear();
}

void Nets::AddPolyToNet(GDSPolygon * poly, char *NetName)
{
	if (strcmp(poly->GetNetName(), "None") == 0) {
		poly->SetNetName(NetName);
	}
	else {
		// poly already assign to net
		return;
	}

	bool found = false;
	Net *net;
	for (set<char*>::iterator NetName_it = NetList.begin(); NetName_it != NetList.end(); ++NetName_it) {
		if (strcmp(*NetName_it, NetName) == 0)
		{
			found = true;
			break;
		}
	}
	if (!found) {
		// Unfound NetName Add it
		NetList.insert(NetName);
		net = new Net();
		PolyByNet[NetName] = net;
	}
	else {
		net = PolyByNet[NetName];
	}

	net->AddPoly(poly);

}
void Nets::SetNetName(GDSPolygon *poly) {
	char *NetName;
	size_t Size = NetList.size();
	NetName = new char[4 + int(log10(Size + 1)) + 2];
	sprintf(NetName, "Net_%zd", Size);
	AddPolyToNet(poly, NetName);
	//delete[] NetName;
}
vector<GDSPolygon*> Nets::GetPolyOnNet(char *NetName, Point2D P) {
	GDSBB BB;
	BB.addPoint(P);
	return GetPolyOnNet(NetName, BB);
}

vector<GDSPolygon*> Nets::GetPolyOnNet(char *NetName, GDSBB BB) {
	char *Found_NetName;
	bool found = false;
	for (set<char*>::iterator NetName_it = NetList.begin(); NetName_it != NetList.end(); ++NetName_it) {
		if (strcmp(*NetName_it, NetName) == 0)
		{
			found = true;
			Found_NetName = *NetName_it;
			break;
		}
	}
	if (found)
		return PolyByNet[Found_NetName]->GetPolysNear(BB);
	else
		// Net Name not found
		return vector<GDSPolygon*>();
}
vector<GDSPolygon*> Nets::GetPolyOnNet(char *NetName) {
	char *Found_NetName;
	bool found = false;
	for (set<char*>::iterator NetName_it = NetList.begin(); NetName_it != NetList.end(); ++NetName_it) {
		if (strcmp(*NetName_it, NetName) == 0)
		{
			found = true;
			Found_NetName = *NetName_it;
			break;
		}
	}
	if (found)
		return PolyByNet[Found_NetName]->GetPolys();
	else
		// Net Name not found
		return vector<GDSPolygon*>();
}

GDS3DBB Nets::GetNetBB(char * NetName)
{
	char *Found_NetName;
	bool found = false;
	for (set<char*>::iterator NetName_it = NetList.begin(); NetName_it != NetList.end(); ++NetName_it) {
		if (strcmp(*NetName_it, NetName) == 0)
		{
			found = true;
			Found_NetName = *NetName_it;
			break;
		}
	}
	if (found)
		return PolyByNet[Found_NetName]->GetBB();
	else
		// Net Name not found
		return GDS3DBB();
}

vector<char*> Nets::GetNetsNames()
{
	char *NetName;
	vector<char*> NetsNames;

	for (set<char*>::iterator NetName_it = NetList.begin(); NetName_it != NetList.end(); ++NetName_it) {
		NetName = *NetName_it;
		NetsNames.push_back(NetName);
	}
	return NetsNames;
}

bool Nets::find(GDSPolygon *  poly) {
	char *NetName;
	vector<GDSPolygon*> NetPolys;

	for (set<char*>::iterator NetName_it = NetList.begin(); NetName_it != NetList.end(); ++NetName_it) {
		NetName = *NetName_it;

		NetPolys = PolyByNet[NetName]->GetPolys();
		for (size_t i = 0; i < NetPolys.size(); i++) {
			if (NetPolys[i] == poly) {
				return true;
			}
		}
	}
	return false;
}

size_t Nets::size() {
	return NetList.size();
}

PolySpace::~PolySpace() {
}
void PolySpace::Clear()
{
	polys.clear();
	cur_size = 0;
	_hasBBox = false;
	BBox.clear();
}

void PolySpace::Add(vector<GDSPolygon*> polyList)
{
	for (size_t i = 0; i < polyList.size(); i++) {
		polys.push_back(polyList[i]);
		BBox.merge(*polyList[i]->GetBBox());
		cur_size += 1;
	}
	_hasBBox = true;

}

void PolySpace::Add(GDSPolygon * poly)
{
	polys.push_back(poly);
	BBox.merge(*poly->GetBBox());
	_hasBBox = true;
	cur_size += 1;
}

void PolySpace::Remove(GDSPolygon * poly)
{
	vector<GDSPolygon*>::iterator it = std::find(polys.begin(), polys.end(), poly);
	if (it != polys.end())
		polys.erase(it);
	//polys.erase(std::remove(polys.begin(), polys.end(), poly), polys.end());
	if (polys.size() < (cur_size - 100)) {
		BBox.clear();
		for (size_t i = 0; i < polys.size(); ++i) {
			GDSPolygon *cur_poly = polys[i];
			BBox.merge(*cur_poly->GetBBox());
		}
		cur_size = polys.size();
	}
}

vector<GDSPolygon*> PolySpace::Get()
{
	return polys;
}

GDSBB PolySpace::GetBB()
{
	if (_hasBBox) {
		return BBox;
	}
	BBox.clear();
	for (size_t i = 0; i < polys.size(); ++i) {
		GDSPolygon *cur_poly = polys[i];
		BBox.merge(*cur_poly->GetBBox());
	}
	_hasBBox = true;
	return BBox;
}

size_t PolySpace::size()
{
	return polys.size();
}

void PolygonSort::Add(vector<GDSPolygon *> polyList)
{
	polys.Add(polyList); 
}

void PolygonSort::Add(GDSPolygon * poly)
{
	// Add to Full List
	polys.Add(poly);
	// Try to add to one of the current BBox
	for (map<GDSBB, PolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); BB_it++) {
		GDSBB cur_BB = BB_it->first;
		if (cur_BB.isBBInside_wborders(*poly->GetBBox())) {
			BB_it->second.Add(poly);
			break;
		}
	}

}

void PolygonSort::Remove(GDSPolygon * poly)
{
	vector<GDSPolygon*> polyList;
	vector<GDSBB> BBSpaceremoveList;

	for (map<GDSBB, PolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); ++BB_it) {
		GDSBB cur_BB = BB_it->first;
		if (cur_BB.isBBInside_wborders(*poly->GetBBox())) {
			polyList = BB_it->second.polys;
			if (std::find(polyList.begin(), polyList.end(), poly) != polyList.end()) {
				BB_it->second.Remove(poly);

				break;
			}
		}
		if (BB_it->second.polys.size() == 0) {
			// Remove empty space
			BBSpaceremoveList.push_back(cur_BB);
		}
	}
	if (BBSpaceremoveList.size() > 0) {
		for (size_t i = 1; i < BBSpaceremoveList.size() + 1; i++) {
			PolyBySpace.erase(BBSpaceremoveList[BBSpaceremoveList.size() - i]);
		}
	}
	polys.Remove(poly);
}

vector<GDSPolygon*> PolygonSort::Get()
{
	return polys.Get();
}

vector<GDSPolygon*> PolygonSort::GetPolyInside(GDSBB BBox)
{

	vector<GDSPolygon*> poly_list;
	vector<GDSPolygon*> polycheckedlist;
	poly_list = GetPolyNear(BBox);

	for (size_t i = 0; i < poly_list.size(); i++) {
		GDSPolygon *cur_poly = poly_list[i];
		if (BBox.isBBInside(*cur_poly->GetBBox())) {
			polycheckedlist.push_back(cur_poly);
		}
	}
	return polycheckedlist;
}

vector<GDSPolygon*> PolygonSort::GetPolyNear(GDSBB BBox)
{
	Check();
	vector<GDSPolygon*> poly_list;
	vector<GDSPolygon*> polycheck_list;
	for (map<GDSBB, PolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); ++BB_it) {
		GDSBB cur_BB = BB_it->first;
		if (cur_BB.intersect(BBox, cur_BB)) {
			polycheck_list = BB_it->second.polys;
			for (size_t i = 0; i < polycheck_list.size(); i++) {
				GDSPolygon *cur_poly = polycheck_list[i];
				poly_list.push_back(cur_poly);
			}
		}
	}
	return poly_list;
}

void PolygonSort::SpaceDiv(PolySpace poly_list) {
	double dx = poly_list.GetBB().max.X - poly_list.GetBB().min.X;
	double dy = poly_list.GetBB().max.Y - poly_list.GetBB().min.Y;
	PolySpace polys_0;
	PolySpace polys_1; 
	GDSBB BBox_half = poly_list.GetBB();
	if (dx > dy) {
		// divide X
		BBox_half.max.X -= dx / 2;
	}
	else {
		// divide Y
		BBox_half.max.Y -= dy / 2;
	}
	vector<GDSPolygon*> Poly_Only_List = poly_list.Get();
	for (size_t i = 0; i < Poly_Only_List.size(); ++i) {
		GDSPolygon *cur_poly = Poly_Only_List[i];
		if (BBox_half.isBBInside_wborders(*cur_poly->GetBBox())) {
			polys_0.Add(cur_poly);
		}
		else {
			polys_1.Add(cur_poly);
		}
	}

	if (polys_1.Get().size() == poly_list.Get().size()) {
		// All block put in the same part invert
		polys_1.Clear();
		polys_0.Clear();
		BBox_half = poly_list.GetBB();
		if (dx > dy) {
			// divide X
			BBox_half.min.X += dx / 2;
		}
		else {
			// divide Y
			BBox_half.min.Y += dy / 2;
		}

		for (size_t i = 0; i < Poly_Only_List.size(); ++i) {
			GDSPolygon *cur_poly = Poly_Only_List[i];
			if (BBox_half.isBBInside_wborders(*cur_poly->GetBBox())) {
				polys_0.Add(cur_poly);
			}
			else {
				polys_1.Add(cur_poly);
			}
		}
		if (polys_1.Get().size() == poly_list.Get().size()) {
			// All block put in the same part invert
			// Too Big blocs?
			PolyBySpace[polys_1.GetBB()] = polys_1;
			return;
		}
	}

	if (polys_0.Get().size() == poly_list.Get().size()) {
		// All block put in the same part invert
		polys_1.Clear();
		polys_0.Clear();
		GDSBB BBox_half = poly_list.GetBB();
		if (dx > dy) {
			// divide X
			BBox_half.min.X += dx / 2;
		}
		else {
			// divide Y
			BBox_half.min.Y += dy / 2;
		}

		for (size_t i = 0; i < Poly_Only_List.size(); ++i) {
			GDSPolygon *cur_poly = Poly_Only_List[i];
			if (BBox_half.isBBInside_wborders(*cur_poly->GetBBox())) {
				polys_0.Add(cur_poly);
			}
			else {
				polys_1.Add(cur_poly);
			}
		}
		if (polys_0.Get().size() == poly_list.Get().size()) {
			// All block put in the same part invert
			// Too Big blocs?
			PolyBySpace[polys_0.GetBB()] = polys_0;
			return;
		}
	}



	if (polys_0.Get().size() > POLYAREAGROUP) {
		SpaceDiv(polys_0);
	}
	else {
		if (Find(polys_0.GetBB()) ) {
			PolyBySpace[polys_0.GetBB()].Add(polys_0.polys);
		} else
		PolyBySpace[polys_0.GetBB()] = polys_0;
	}
	if (polys_1.Get().size() > POLYAREAGROUP) {
		SpaceDiv(polys_1);
	}
	else {
		if (Find(polys_1.GetBB())) {
			PolyBySpace[polys_1.GetBB()].Add(polys_1.polys);
		}
		else {
			PolyBySpace[polys_1.GetBB()] = polys_1;
		}
	}
}

size_t PolygonSort::GetPolyBySpaceSize() {
	size_t count = 0;
	for (map<GDSBB, PolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); BB_it++) {
		count += BB_it->second.size();
	}
	return count;
}

bool PolygonSort::Find(GDSBB BB) {
	GDSBB BBox;
	for (map<GDSBB, PolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); BB_it++) {
		BBox = BB_it->first;
		if (BBox == BB) {
			return true;
		}

	}
	return false;
}
void PolygonSort::Check()
{
	if (polys.Get().size() != GetPolyBySpaceSize()) {
		PolyBySpace.clear();

		// Update 
		if (polys.Get().size() > POLYAREAGROUP) {
			SpaceDiv(polys);
		}
		else {
			PolyBySpace[polys.GetBB()] = polys;
		}
	}
}

void PolygonSort::Clear()
{
	PolyBySpace.clear();
}

void PolygonSort::ClearPolys()
{
	polys.Clear();
	Clear();
}

