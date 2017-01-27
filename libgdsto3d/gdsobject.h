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

#ifndef __GDSOBJECT_H__
#define __GDSOBJECT_H__

#include "gds_globals.h"
#include "process_cfg.h"
#include "gdselements.h"
#include "gdspath.h"
#include "gdstext.h"
#include "gdspolygon.h"

class PolySpace {
private:
	bool _hasBBox;
	
	GDSBB BBox;
	size_t cur_size;
protected:
public:
	PolySpace() { _hasBBox = false; cur_size = 0; BBox.clear();
	}
	vector<GDSPolygon*> polys;
	~PolySpace();
	void Clear();
	void Add(vector<GDSPolygon*> polyList);
	void Add(GDSPolygon* poly);
	void Remove(GDSPolygon* poly);
	vector<GDSPolygon*> Get();
	GDSBB GetBB();
	size_t size();
};

class PolygonSort
{
private:
	// Temp list with full size
	PolySpace polys;

	// List of poly devided by space area
	map < GDSBB, PolySpace> PolyBySpace;
public:
	void Add(vector<GDSPolygon*> polyList);
	void Add(GDSPolygon* poly);
	void Remove(GDSPolygon* poly);
	vector<GDSPolygon*> Get();
	vector<GDSPolygon*> GetPolyInside(GDSBB BBox);
	vector<GDSPolygon*> GetPolyNear(GDSBB BBox);
	void SpaceDiv(PolySpace poly_list);
	size_t GetPolyBySpaceSize();
	bool Find(GDSBB BB);
	void Check();
	void Clear();
	void ClearPolys();
};

class Net {
private:
	bool _hasBBox;
	GDS3DBB BBox;
	PolygonSort polys;
public:
	Net();
	~Net();
	GDS3DBB GetBB();
	void AddPoly(GDSPolygon * poly);
	vector<GDSPolygon*> GetPolys();
	vector<GDSPolygon*> GetPolysNear(GDSBB BB);
};
class Nets {
protected:
	set<char*> NetList;
	map<char*, Net*> PolyByNet;
public:
	~Nets();
	void CleanNetName();
	void AddPolyToNet(GDSPolygon * poly, char *NetName);
	void SetNetName(GDSPolygon * poly);
	vector<GDSPolygon*> GetPolyOnNet(char * NetName, Point2D P);
	vector<GDSPolygon*> GetPolyOnNet(char * NetName, GDSBB BB);
	vector<GDSPolygon*> GetPolyOnNet(char *NetName);
	GDS3DBB GetNetBB(char *NetName);
	vector<char*> GetNetsNames();
	bool find(GDSPolygon * poly);
	size_t size();
};

typedef struct GDSRef
{
	GDSObject	*object;
	GDSMat		mat;
}GDSRef;


class KeyInstance {
private:
	GDSRef		ref;
public:
	GDSObject	*object;
	GDSMat		Mat;
	
	KeyInstance() { object = NULL; }

	KeyInstance(class GDSObject *object, GDSMat Mat);
	KeyInstance(struct GDSRef ref);

	bool operator<(const KeyInstance & A) const;
	bool operator()(const KeyInstance & A) const;
};

class GDSObject
{
protected:
	// Temporary data for parsing	
	vector<GDSPath*> PathItems;
	vector<GDSText*> TextItems;
	vector<SRefElement*> SRefItems;
	vector<ARefElement*> ARefItems;	
	
	size_t PointCount;
    size_t AccumPointCount;
    bool noHierarchy;

	bool hasBoundary;
	GDSBB boundary;
	bool has3DBoundary;
	GDS3DBB _3Dboundary;

	char *Name;
	char *GDSName;
	bool PCell; // After PCell detection
	bool collapsed;
	Nets Object_Nets;

	// For Net checking
	set<GDSPolygon*> checked_poly;
	set<GDSPolygon*> unchecked_poly;
	//vector<GDSPolygon*> pool_poly;
	PolygonSort pool_poly;

public:
	// Please move to private...
	vector<GDSPolygon*> PolygonItems; 	
	vector<GDSRef*> refs; // Use these references for rendering

	GDSObject(char *Name, char *gdsName);
	virtual ~GDSObject();

	// Adding of new elements
	void AddText(float newX, float newY, float newZ, bool newFlipped, float newMag, int newVJust, int newHJust, struct ProcessLayer *newlayer);
	class GDSText *GetCurrentText();
	void AddPolygon(double Height, double Thickness, size_t Points, struct ProcessLayer *layer);
	class GDSPolygon *GetCurrentPolygon();
	void AddSRef(char *Name, float X, float Y, int Flipped, float Mag);
	void SetSRefRotation(float X, float Y, float Z);
	void AddARef(char *Name, float X1, float Y1, float X2, float Y2, float X3, float Y3, int Columns, int Rows, int Flipped, float Mag);
	void SetARefRotation(float X, float Y, float Z);
	void AddPath(int PathType, float Height, float Thickness, int Points, float Width, float BgnExtn, float EndExtn, struct ProcessLayer *layer);
	class GDSPath *GetCurrentPath();

	void ConnectReferences(class GDSObjectList *Objects);
	void TransformAddObject(GDSObject *obj, GDSMat mat);

	// Get stuff
	char *GetName();
	char * GetGDSName();
	bool referencesToObject(char *name);
	char *GetProcessName();
	bool Has3DBBox();
	void ResetBBox();
	GDS3DBB GetTotal3DBoundary();
	GDSBB GetTotalBoundary();
	bool isPCell();
	size_t GetNumSRefs();
	SRefElement* GetSRef(unsigned int index);
	size_t GetNumARefs();
	ARefElement* GetARef(unsigned int index);
    
    // Flatten lower part of hierarchy
    void printHierarchy(int);
    size_t countTotalPoints();
    void collapseHierachy();   

	// Netlist
	void SetNetName(GDSPolygon * poly);
	size_t GetNetsSize();
	vector<char*> GetNetsNames();
	vector<GDSPolygon*> GetPolyOnNet(char * NetName);
	void FindAllConnectPoly(GDSPolygon * poly);
	void SetPool_Poly();
	vector<GDSPolygon*> GetPolyNear(Point2D P);
	vector<GDSPolygon*> GetPolyNear(GDSBB BB);
	void intersectPoly(GDSPolygon * poly);
	Nets GetNetlist();

	void CleanNetList();

};


inline bool KeyInstance::operator<(const KeyInstance& A) const
{
	if (this->Mat < A.Mat)
		return true;
	else if (A.Mat < this->Mat)
		return false;
	else if (this->object->countTotalPoints() < A.object->countTotalPoints())
		return true;
	else if (this->object->countTotalPoints() > A.object->countTotalPoints())
		return false;
	//We should never reach this point, means matrices are equal and countTotalPoints are equal
	return false;
}
inline bool KeyInstance::operator()(const KeyInstance& A) const
{
	if (this->Mat < A.Mat)
		return true;
	else if (A.Mat < this->Mat)
		return false;
	else if (this->object->countTotalPoints() < A.object->countTotalPoints())
		return true;
	else if (this->object->countTotalPoints() > A.object->countTotalPoints())
		return false;
	//We should never reach this point, means matrices are equal and countTotalPoints are equal
	return false;
}

#endif // __GDSOBJECT_H__

