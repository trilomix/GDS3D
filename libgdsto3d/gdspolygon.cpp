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

#ifdef WIN32
#include <algorithm>
#endif

#include "gdsobject.h"
#include "gdspolygon.h"
#include "../math/Maths.h"
//#include "clipper/clipper.hpp"
using namespace ClipperLib;

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

// GDSMat Class
GDSMat::GDSMat()
{
	loadIdentity();
}

GDSMat::GDSMat(double e0, double e1, double e2, double e3, double e4, double e5)
{
	entries[0] = e0;
	entries[1] = e1;
	entries[2] = e2;
	entries[3] = e3;
	entries[4] = e4;
	entries[5] = e5;
}

void GDSMat::loadIdentity()
{
	memset(entries, 0, 6*sizeof(double));
	entries[0]=1.0f;
	entries[3]=1.0f;
}

void GDSMat::setScaling(const double& X, const double& Y)
{
	memset(entries, 0, 6*sizeof(double));
	entries[0] = X;
	entries[3] = Y;
}

void GDSMat::setTranslation(const double& X, const double& Y)
{
	memset(entries, 0, 6*sizeof(double));
	entries[0]=1.0f;
	entries[3]=1.0f;
	entries[4] = X;
	entries[5] = Y;
}

void GDSMat::setRotation(const double& angle)
{
	memset(entries, 0, 6*sizeof(double));

	entries[0]=(double)cos(M_PI*angle/180);
	entries[1]=(double)sin(M_PI*angle/180);

	entries[2]=-entries[1];
	entries[3]=entries[0];
}

// This code is untested...
// http://www.wikihow.com/Inverse-a-3X3-Matrix
//
GDSMat GDSMat::Inverse()const
{
	GDSMat result;
	double det;
	double tmp[9];

	det = entries[0]*entries[3]-entries[2]*entries[1];

	tmp[0] = entries[3];
	tmp[1] = -entries[2];
	tmp[2] = (entries[2]*entries[5] - entries[4]*entries[3]);

	tmp[3] = -entries[1];
	tmp[4] = entries[0];
	tmp[5] = -(entries[0]*entries[5] - entries[4]*entries[1]);

	tmp[6] = 0.0;
	tmp[7] = 0.0;
	tmp[8] = (entries[0]*entries[3] - entries[2]*entries[1]);

	return GDSMat(tmp[0]/det, tmp[3]/det, tmp[1]/det, tmp[4]/det, tmp[2]/det, tmp[5]/det);
}

// GDSBB Class
GDSBB::GDSBB()
{
	clear();
}

GDSBB::GDSBB(Point2D A, Point2D B)
{
	clear();
	if (A < B) {
		min = A;
		max = B;
	} else {
		min = B;
		max = A;
	}
}



void GDSBB::clear()
{
	min.X = min.Y = 10000.0f;
	max.X = max.Y = -10000.0f;
}

bool GDSBB::isEmpty() const
{
	if(min.X > max.X || min.Y > max.Y)
		return true;

	return false;
}

void GDSBB::addPoint(const Point2D& P)
{
	min.X = std::min(min.X, P.X);
	min.Y = std::min(min.Y, P.Y);
	max.X = std::max(max.X, P.X);
	max.Y = std::max(max.Y, P.Y);
}

void GDSBB::merge(const GDSBB& BB)
{
	if (BB.isEmpty())
		return;
	min.X = std::min(min.X, BB.min.X);
	min.Y = std::min(min.Y, BB.min.Y);
	max.X = std::max(max.X, BB.max.X);
	max.Y = std::max(max.Y, BB.max.Y);
}

void GDSBB::transform(const GDSMat& M)
{
	if (isEmpty())
		return;

	Point2D P0(min.X, min.Y);
	Point2D P1(min.X, max.Y);
	Point2D P2(max.X, min.Y);
	Point2D P3(max.X, max.Y);

	clear();
	addPoint(M * P0);
	addPoint(M * P1);
	addPoint(M * P2);
	addPoint(M * P3);
}

double GDSBB::area() {
	return (max.X - min.X)*(max.Y - min.Y);
}

bool GDSBB::isPointInside(const Point2D& P)
{
	if( (P.X - max.X) > 0.001f || (min.X-P.X) > 0.001f )
			return false;
	if( (P.Y - max.Y) > 0.001f || (min.Y-P.Y) > 0.001f )
			return false;

	return true;
}

bool GDSBB::isBBInside(const GDSBB& BB)
{
	if ((BB.max.X - max.X) >= 0.001f || (min.X - BB.min.X) >= 0.001f)
		return false;
	if ((BB.max.Y - max.Y) >= 0.001f || (min.Y - BB.min.Y) >= 0.001f)
		return false;

	return true;
}

bool GDSBB::isBBInside_wborders(const GDSBB& BB)
{
	if ((BB.max.X - max.X) > 0.000f || (min.X - BB.min.X) > 0.000f)
		return false;
	if ((BB.max.Y - max.Y) > 0.000f || (min.Y - BB.min.Y) > 0.000f)
		return false;

	return true;
}

bool GDSBB::intersect(const GDSBB& BB1, const GDSBB& BB2)
{
	// How much of a margin??
	if( (BB1.min.X - BB2.max.X) > 0.001f || (BB2.min.X-BB1.max.X) > 0.001f )
			return false;
	if( (BB1.min.Y - BB2.max.Y) > 0.001f || (BB2.min.Y-BB1.max.Y) > 0.001f )
			return false;

	return true;
}

bool GDSBB::intersect_wborders(const GDSBB& BB1, const GDSBB& BB2)
{
	return intersect_wborders( BB1, BB2, 0.000f);
}

bool GDSBB::intersect_wborders(const GDSBB& BB1, const GDSBB& BB2, float margin)
{
	// How much of a margin??
	if ((BB1.min.X - BB2.max.X) > margin || (BB2.min.X - BB1.max.X) > margin)
		return false;
	if ((BB1.min.Y - BB2.max.Y) > margin || (BB2.min.Y - BB1.max.Y) > margin)
		return false;

	return true;
}

Point3D _2Dto3D(const Point2D& P, double Z) {
	Point3D P3D;
	P3D.X = P.X;
	P3D.Y = P.Y;
	P3D.Z = Z;

	return P3D;
}

// GDS3dBB Class
GDS3DBB::GDS3DBB()
{
	clear();
}

void GDS3DBB::clear()
{
	min.X = min.Y = min.Z = 10000.0f;
	max.X = max.Y = max.Z = -10000.0f;
}

bool GDS3DBB::isEmpty()
{
	if (min.X > max.X || min.Y > max.Y || min.Z > max.Z )
		return true;

	return false;
}

void GDS3DBB::addPoint(const Point3D& P)
{
	min.X = std::min(min.X, P.X);
	min.Y = std::min(min.Y, P.Y);
	min.Z = std::min(min.Z, P.Z);
	max.X = std::max(max.X, P.X);
	max.Y = std::max(max.Y, P.Y);
	max.Z = std::max(max.Z, P.Z);
}

void GDS3DBB::merge(const GDS3DBB& BB)
{
	min.X = std::min(min.X, BB.min.X);
	min.Y = std::min(min.Y, BB.min.Y);
	min.Z = std::min(min.Z, BB.min.Z);
	max.X = std::max(max.X, BB.max.X);
	max.Y = std::max(max.Y, BB.max.Y);
	max.Z = std::max(max.Z, BB.max.Z);
}

void GDS3DBB::transform(const GDSMat& M)
{
	Point2D P0(min.X, min.Y);
	Point2D P1(min.X, max.Y);
	Point2D P2(max.X, min.Y);
	Point2D P3(max.X, max.Y);
	double Zmin = min.Z;
	double Zmax = max.Z;
	clear();
	addPoint(_2Dto3D(M * P0, Zmin));
	addPoint(_2Dto3D(M * P1, Zmin));
	addPoint(_2Dto3D(M * P2, Zmin));
	addPoint(_2Dto3D(M * P3, Zmin));
	addPoint(_2Dto3D(M * P0, Zmax));
	addPoint(_2Dto3D(M * P1, Zmax));
	addPoint(_2Dto3D(M * P2, Zmax));
	addPoint(_2Dto3D(M * P3, Zmax));
}

bool GDS3DBB::isPointInside(const Point3D& P)
{
	if ((P.X - max.X) > 0.001f || (min.X - P.X) > 0.001f)
		return false;
	if ((P.Y - max.Y) > 0.001f || (min.Y - P.Y) > 0.001f)
		return false;
	if ((P.Z - max.Z) > 0.001f || (min.Z - P.Z) > 0.001f)
		return false;

	return true;
}

bool GDS3DBB::isPointInside(const Point2D& P)
{
	if ((P.X - max.X) > 0.001f || (min.X - P.X) > 0.001f)
		return false;
	if ((P.Y - max.Y) > 0.001f || (min.Y - P.Y) > 0.001f)
		return false;

	return true;
}

bool GDS3DBB::isBBInside(const GDS3DBB& BB)
{
	if ((BB.max.X - max.X) > 0.001f || (min.X - BB.min.X) > 0.001f)
		return false;
	if ((BB.max.Y - max.Y) > 0.001f || (min.Y - BB.min.Y) > 0.001f)
		return false;
	if ((BB.max.Z - max.Z) > 0.001f || (min.Z - BB.min.Z) > 0.001f)
		return false;

	return true;
}

bool GDS3DBB::isBBInside_wborders(const GDS3DBB& BB)
{
	return isBBInside_wborders(BB, 0.001f);
}

bool GDS3DBB::isBBInside_wborders(const GDS3DBB& BB, float margin)
{
	if ((BB.max.X - max.X) >= margin || (min.X - BB.min.X) >= margin)
		return false;
	if ((BB.max.Y - max.Y) >= margin || (min.Y - BB.min.Y) >= margin)
		return false;
	if ((BB.max.Z - max.Z) >= margin || (min.Z - BB.min.Z) >= margin)
		return false;

	return true;
}


bool GDS3DBB::intersect(const GDS3DBB& BB1, const GDS3DBB& BB2)
{
	// How much of a margin??
	if ((BB1.min.X - BB2.max.X) > 0.001f || (BB2.min.X - BB1.max.X) > 0.001f)
		return false;
	if ((BB1.min.Y - BB2.max.Y) > 0.001f || (BB2.min.Y - BB1.max.Y) > 0.001f)
		return false;
	if ((BB1.min.Z - BB2.max.Z) > 0.001f || (BB2.min.Z - BB1.max.Z) > 0.001f)
		return false;
	return true;
}

bool GDS3DBB::intersect(const GDSBB& BB1, const GDS3DBB& BB2)
{
	// How much of a margin??
	if ((BB1.min.X - BB2.max.X) > 0.001f || (BB2.min.X - BB1.max.X) > 0.001f)
		return false;
	if ((BB1.min.Y - BB2.max.Y) > 0.001f || (BB2.min.Y - BB1.max.Y) > 0.001f)
		return false;

	return true;
}

// GDSTriangle Class
void GDSTriangle::set(const Point2D& P1, const Point2D& P2, const Point2D& P3)
{
	coords[0] = P1;
	coords[1] = P2;
	coords[2] = P3;

	bbox.addPoint(P1);
	bbox.addPoint(P2);
	bbox.addPoint(P3);
}

void GDSTriangle::project(const Point2D& axis, double& min, double& max) const
{
	double d;

	min = axis.X*coords[0].X+axis.Y*coords[0].Y;
	max = min;

	d = axis.X*coords[1].X+axis.Y*coords[1].Y;
	min = std::min(min, d);
	max = std::max(max, d);

	d = axis.X*coords[2].X+axis.Y*coords[2].Y;
	min = std::min(min, d);
	max = std::max(max, d);
}

void GDSTriangle::normal(const Point2D& P1, const Point2D& P2, Point2D& N)
{
	N.X = P1.Y - P2.Y;
	N.Y = P2.X - P1.X;
}

bool GDSTriangle::intersect(const GDSTriangle& T1, const GDSTriangle& T2)
{
	Point2D axis;
	double min1 = 0, max1 = 0, min2 = 0, max2 = 0;

	// Bounding box Test
	if(!GDSBB::intersect(T1.bbox, T2.bbox))
		return false;

	// Test T1 edges
	for(unsigned int i=0;i<3;i++)
	{
		GDSTriangle::normal(T1.coords[i], T1.coords[(i+1)%3], axis);
		T1.project(axis, min1, max1);
		T2.project(axis, min2, max2);
		if( (min1 - max2) > 0.001f || (min2-max1) > 0.001f )
			return false;
	}

	// Test T2 edges
	for(unsigned int i=0;i<3;i++)
	{
		GDSTriangle::normal(T2.coords[i], T2.coords[(i+1)%3], axis);
		T1.project(axis, min1, max1);
		T2.project(axis, min2, max2);
		if( (min1 - max2) > 0.001f || (min2-max1) > 0.001f )
			return false;
	}

	// No seperating axis, so intersection must have taken place
	return true;
}

bool GDSTriangle::isTriangleInside(const GDSTriangle& T) {
	if (!this->bbox.isBBInside(T.bbox))
		return false;

	for (unsigned int i = 0; i < 3; i++) {
		if (!this->isPointInside(T.coords[i]))
			return false;
	}
	return true;
}

bool GDSTriangle::isPointInside(const Point2D& P)
{
	/*Point2D A = this->coords[0];
	Point2D B = this->coords[1];
	Point2D C = this->coords[2];
	double ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	double cCROSSap, bCROSScp, aCROSSbp;

	// Include points on vertices
	if (A == P)
		return true;
	if (B == P)
		return true;
	if (C == P)
		return true;
		*/
	int i, j;
	bool c = false;
		
	for (i = 0, j = 3 - 1; i < 3; j = i++) {
		if (((coords[i].Y > P.Y) != (coords[j].Y > P.Y)) &&
			(P.X < (coords[j].X - coords[i].X) * (P.Y - coords[i].Y) / (coords[j].Y - coords[i].Y) + coords[i].X))
			c = !c;
	}
		
	return c;
	/*
	ax = C.X - B.X;  ay = C.Y - B.Y;
	bx = A.X - C.X;  by = A.Y - C.Y;
	cx = B.X - A.X;  cy = B.Y - A.Y;
	apx = P.X - A.X;  apy = P.Y - A.Y;
	bpx = P.X - B.X;  bpy = P.Y - B.Y;
	cpx = P.X - C.X;  cpy = P.Y - C.Y;

	aCROSSbp = ax * bpy - ay * bpx;
	cCROSSap = cx * apy - cy * apx;
	bCROSScp = bx * cpy - by * cpx;
	
	float alpha = ((B.Y - C.Y)*(P.X - C.X) + (C.X - B.X)*(P.Y - C.Y)) /
		((B.Y - C.Y)*(A.X - C.X) + (C.X - B.X)*(A.Y - C.Y));
	float beta = ((C.Y - A.Y)*(P.X - C.X) + (A.X - C.X)*(P.Y - C.Y)) /
		((B.Y - C.Y)*(A.X - C.X) + (C.X - B.X)*(A.Y - C.Y));
	float gamma = 1.0f - alpha - beta;

	if(alpha> 0 && beta > 0 && gamma>0) 
		if (!((aCROSSbp >= -1.0f) && (bCROSScp >= -1.0f) && (cCROSSap >= -1.0f)))
		{
			ax = ax;
		}
    
	return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
	*/
	//return ((aCROSSbp >= epsilon) && (bCROSScp >= epsilon) && (cCROSSap >= epsilon));
}

double GDSTriangle::Area()
{
	Point2D A = coords[0];
	Point2D B = coords[1];
	Point2D C = coords[2];

	double area = (((B.X - A.X) * (C.Y - A.Y)) - ((B.Y - A.Y) * (C.X - A.X))) / 2;

	return area < 0 ? -area : area;
}

// GDSPolygon Class
GDSPolygon::GDSPolygon(double Height, double Thickness, struct ProcessLayer *Layer)
{
	_Height = Height;
	_Thickness = Thickness;
	_Layer = Layer;
	
	epsilon = Layer->Units->Unitu; // Default precision of 1nm
	_NetName = NULL;
	SetNetName((char*)"None");
}

GDSPolygon::GDSPolygon(struct ProcessLayer *Layer)
{
	_Height = Layer->Height*Layer->Units->Unitu;
	_Thickness = Layer->Thickness*Layer->Units->Unitu;
	_Layer = Layer;

	epsilon = Layer->Units->Unitu; // Default precision of 1nm
	_NetName = NULL;
	SetNetName((char*)"None");
}

GDSPolygon::~GDSPolygon()
{
}

void
GDSPolygon::Clear()
{
    _Coords.clear();
    indices.clear();
	bbox.clear();
    bbox3D.clear();

	SetNetName((char*)"None");
}

void
GDSPolygon::SetNetName(char *NewName) {
	/*if (_NetName) {
		delete[] _NetName;
		_NetName = NULL;
	}*
	_NetName = new char[strlen(NewName) + 1];
	strcpy(_NetName, NewName);
	*/
	_NetName = NewName;
}

char *GDSPolygon::GetNetName() {
	return _NetName;
}

void
GDSPolygon::SetLayer(struct ProcessLayer *Layer, double units) {
	_Layer = Layer;
	_Height = Layer->Height*units;
	_Thickness = Layer->Thickness*units;
	vector<Point2D> Coords;
	for (size_t i = 0; i < _Coords.size(); i++) {
		Coords.push_back(_Coords[i]);
	}
	Clear();
	for (size_t i = 0; i < Coords.size(); i++) {
		AddPoint(Coords[i]);
	}
}

void 
GDSPolygon::CopyInto(GDSPolygon *p)
{
	// Nothing special happens here
	p->_Height = _Height;
	p->_Thickness = _Thickness;
	p->_Layer = _Layer;
	
	// Remove align point
	vector<Point2D> TCoords = _Coords;
	vector<size_t> AlignPointsIndex;
	Point2D A;
	Point2D B;
	Point2D C;
	size_t j, k;
	do {
		AlignPointsIndex.clear();
		j = _Coords.size() - 1;
		k = _Coords.size() - 2;
		for (size_t i = 0; i < _Coords.size(); i++) {
			A = GetCoords(i);
			B = GetCoords(j);
			C = GetCoords(k);
			if (A == B) {
				AlignPointsIndex.push_back(j);
				break;
			}
			else if (A.X == B.X && A.X == C.X && (fabs(A.Y - C.Y) >= fabs(A.Y - B.Y) && fabs(A.Y - C.Y) >= fabs(C.Y - B.Y))) {
				AlignPointsIndex.push_back(j);
				break;
			}
			else if (A.Y == B.Y && A.Y == C.Y && (fabs(A.X - C.X) >= fabs(A.X - B.X) && fabs(A.X - C.X) >= fabs(C.X - B.X))) {
				AlignPointsIndex.push_back(j);
				break;
			}
			else if ((A.Y == B.Y && A.Y == C.Y) || (A.X == B.X && A.X == C.X)) {
				// A B C align but B not inside A C
				AlignPointsIndex.push_back(j);
				break;
			}
			k = j;
			j = i;
		}
		if (AlignPointsIndex.size() > 0) {
			Clear();
			for (size_t i = 0; i < TCoords.size(); i++) {
				bool Add = true;
				for (j = 0; j < AlignPointsIndex.size(); j++) {
					if (AlignPointsIndex[j] == i) {
						Add = false;
						break;
					}
				}
				if (Add)
					AddPoint(TCoords[i]);
			}
			TCoords = _Coords;
			Tesselate(true /* force */);
		} 
	} while (AlignPointsIndex.size() > 0);
	p->indices = indices;
	p->_Coords = _Coords;
	p->bbox = bbox;
	p->bbox3D = bbox3D;

}

bool GDSPolygon::FindCoord(Point2D P) {
	for (unsigned int i = 0; i < _Coords.size(); i++) {
		if (_Coords[i] == P) {
			return true;
			break;
		}
	}
	return false;
}
void
GDSPolygon::AddPoint(Point2D P)
{
	/*if (_Coords.size() > 0) { 
		assert(_Coords[_Coords.size() - 1].X == P.X || _Coords[_Coords.size() - 1].Y == P.Y); 
	}*/
	GDSPolygon::AddPoint(P.X, P.Y);
}
void 
GDSPolygon::AddPoint(double X, double Y)
{

	_Coords.push_back(Point2D(X,Y));
	bbox.addPoint(Point2D(X,Y));
	// For 3D bbox
	double Zmin, Zmax;
	Zmin = _Height;
	Zmax = _Height + _Thickness;
	bbox3D.addPoint(_2Dto3D(Point2D(X, Y), Zmin));
	bbox3D.addPoint(_2Dto3D(Point2D(X, Y), Zmax));
}

bool GDSPolygon::RemovePoint(Point2D P)
{
	bool found = false;
	vector<Point2D> Coords;
	for (size_t i = 0; i < _Coords.size(); i++) {
		if (P == _Coords[i] && !found)
			found = true;
		else
			Coords.push_back(_Coords[i]);
	}
	if (found) {
		Clear();
		// Find remove align points
		vector<size_t> AlignPointsIndex;
		Point2D A;
		Point2D B;
		Point2D C;
		size_t j, k;
		j = Coords.size() - 1;
		k = Coords.size() - 2;
		for (size_t i = 0; i < Coords.size(); i++) {
			A = Coords[i];
			B = Coords[j];
			C = Coords[k];
			if (A.X == B.X && A.X == C.X && (fabs(A.Y - C.Y) >= fabs(A.Y - B.Y) && fabs(A.Y - C.Y) >= fabs(C.Y - B.Y)))
				AlignPointsIndex.push_back(j);
			if (A.Y == B.Y && A.Y == C.Y && (fabs(A.X - C.X) >= fabs(A.X - B.X) && fabs(A.X - C.X) >= fabs(C.X - B.X)))
				AlignPointsIndex.push_back(j);
			k = j;
			j = i;
		}

		for (size_t i = 0; i < Coords.size(); i++) {
			bool Add = true;
			for (j = 0; j < AlignPointsIndex.size(); j++) {
				if (AlignPointsIndex[j] == i) {
					Add = false;
					break;
				}
			}
			if(Add)
				AddPoint(Coords[i]);
		}
		Tesselate(true /* force */);
	}
	return found;
}

void
GDSPolygon::Tesselate()
{
	GDSPolygon::Tesselate(false);
}

void
GDSPolygon::Tesselate( bool force)
{
	if (force)
		indices.clear();

	if(indices.size() > 0 || _Coords.size() < 3)
		return;

	// Fast path for simple polygons
	if(isSimple())
	{
		for(unsigned int j=0;j<_Coords.size()-2;j++)
        {
                indices.push_back(0);
                indices.push_back(j+1);
                indices.push_back(j+2);
        }
		return;
	}
    
    // Build double linked list, with N indices
    vector<size_t> V;
    for (unsigned int i = 0; i <_Coords.size(); i++)
        V.push_back(i);
    
    // Prepare to build N-2 triangles
    int a,b,c;
    for (unsigned int i = 0; i < _Coords.size()-2; i++)
    {
        // Go through the double linked list
        for(unsigned int j=0;j<V.size();j++)
        {
            a = j; b = (j+1)%V.size(); c = (j+2)%V.size();
            bool flagged = false;     
            
            // Orientation or degenerate?
            if(area(_Coords[V[a]], _Coords[V[b]], _Coords[V[c]]) < (epsilon/10))
                continue;
            
            // Go through all of the points
            for(unsigned int k=0;k<V.size();k++) 
            {
				// Same point as triangle
                if(k==a || k==b || k==c) 
                    continue;

				// Check if on polygon edge				
                if( fabs(V[a]-V[b])==1 ||  fabs(V[a]-V[b])==_Coords.size()-1)
				{
					if(onLine(_Coords[V[a]], _Coords[V[b]], _Coords[V[k]]))
						continue;
				}
				if( fabs(V[b]-V[c])==1 ||  fabs(V[b]-V[c])==_Coords.size()-1)
				{
					if(onLine(_Coords[V[b]], _Coords[V[c]], _Coords[V[k]]))
						continue;
				}
				if( fabs(V[c]-V[a])==1 ||  fabs(V[c]-V[a])==_Coords.size()-1)
				{
					if(onLine(_Coords[V[c]], _Coords[V[a]], _Coords[V[k]]))
						continue;
				}

				// Check if in triangle
				if (insideTriangle_woborder(_Coords[V[a]], _Coords[V[b]], _Coords[V[c]], _Coords[V[k]])) {
					flagged = true;
					break;
				}
				else {
					if (V[k] > V[c] && area(_Coords[V[a]], _Coords[V[c]], _Coords[V[k]]) < epsilon / 10) {
						// Check if the next V[k] point is align with the current triangle so so we should invalid the current 
						if (insideTriangle(_Coords[V[a]], _Coords[V[b]], _Coords[V[c]], _Coords[V[k]])) {
							flagged = true;
							break;
						}
					}
                }
            }
            
            // We have an ear
			if (!flagged)
			{
				if (V[a] >= _Coords.size() || V[b] >= _Coords.size() || V[c] >= _Coords.size()) {
					v_printf(0, "Error Tesslate \n");
				}

				indices.push_back(V[a]);
                indices.push_back(V[b]);
                indices.push_back(V[c]);
                V.erase(V.begin()+b);
                break;
            }
        }
    }
    
    
    //Clean up
    V.clear();
}

Point2D GDSPolygon::GetCoords(size_t Index)
{
	return Point2D(GetXCoords(Index), GetYCoords(Index));
}

double GDSPolygon::GetXCoords(size_t Index)
{
	return int(rounded(_Coords[Index].X / _Layer->Units->Unitu))*_Layer->Units->Unitu;
}

double GDSPolygon::GetYCoords(size_t Index)
{
	return int(rounded(_Coords[Index].Y / _Layer->Units->Unitu))*_Layer->Units->Unitu;
}

size_t GDSPolygon::GetPoints()
{
	return _Coords.size();
}

vector<size_t>* GDSPolygon::GetIndices()
{
	// Tesselate if not done before
	if(indices.size() == 0)
		Tesselate();

	return &indices;
}

void GDSPolygon::Flip()
{
	// Flip points for boundary
    vector<Point2D> TCoords = _Coords;
    for(unsigned int i=0;i<TCoords.size();i++)
        _Coords[i] = TCoords[TCoords.size()-1-i];

	// Adjust indices?
    size_t a,b,c;

    for(unsigned int i=0;i<indices.size()/3;i++)
    {
        // New index numbers
        a = _Coords.size()-1-indices[i*3+0];
        b = _Coords.size()-1-indices[i*3+1];
        c = _Coords.size()-1-indices[i*3+2];

        // Swap Order
        indices[i*3+0] = a;
        indices[i*3+1] = c;
        indices[i*3+2] = b;
    }
}

void GDSPolygon::Orientate()
{
    
    // Do we have to flip?        
    if(!OrientateAnticlockwise())
        Flip();
}

bool GDSPolygon::OrientateAnticlockwise()
{
	double x0, y0, x1, y1, x2, y2, nz;
	double dx1, dy1, dx2, dy2;
	nz = 0.0f;

	// Do we have to flip?    
	for (unsigned int j = 1; j<_Coords.size(); j++) {
		if (j == 1)
		{
			x0 = _Coords[0].X;
			y0 = _Coords[0].Y;
		}
		x1 = _Coords[j].X;// + offx;
		y1 = _Coords[j].Y;// + offy;

		x2 = _Coords[(j + 1) % _Coords.size()].X;// + offx;
		y2 = _Coords[(j + 1) % _Coords.size()].Y;// + offy;

		dx1 = x2 - x0;
		dy1 = y2 - y0;

		dx2 = x2 - x1;
		dy2 = y2 - y1;
		nz += dx1*dy2 - dy1*dx2;
	}

	if (nz < 0.0f)
		return false;
	else
		return true;
}


bool
GDSPolygon::isSimple()
{
    int numPos, numNeg;
    numPos = numNeg = 0;
    
	double dx1, dy1, dx2, dy2, nz;
    
    for(unsigned int j=0; j<_Coords.size(); j++){ // Iterate over edges
        dx1 = _Coords[(j+1)%_Coords.size()].X - _Coords[j].X;
        dy1 = _Coords[(j+1)%_Coords.size()].Y - _Coords[j].Y;
        
        dx2 = _Coords[(j+2)%_Coords.size()].X - _Coords[(j+1)%_Coords.size()].X;
        dy2 = _Coords[(j + 2) % _Coords.size()].Y - _Coords[(j + 1) % _Coords.size()].Y;
        
        nz = dx1*dy2 - dy1*dx2;
        
        if (nz > 0.0000001f)
        numPos += 1;
        if (nz < -0.0000001f)
	        numNeg += 1;
	}

	if (numPos > 0 && numNeg > 0)
		return false;

	return true;
}

bool
GDSPolygon::isPointInside(const Point2D& P)
{
	/*
	//GDSTriangle T;

	//We are doing this brute force
	for(unsigned int i=0;i<indices.size()/3;i++)
	{
		if( insideTriangle(_Coords[indices[i*3+0]], _Coords[indices[i*3+1]], _Coords[indices[i*3+2]], P))
			return true;
	}
	return false;
	*/
	size_t i, j;
	bool c = false;

	for (i = 0, j = _Coords.size() - 1; i < _Coords.size(); j = i++) {
		if (onLine(_Coords[i], _Coords[j], P)) {
			c = false;
			break;
		}
		if (((_Coords[i].Y > P.Y) != (_Coords[j].Y > P.Y)) &&
			(P.X < (_Coords[j].X - _Coords[i].X) * (P.Y - _Coords[i].Y) / (_Coords[j].Y - _Coords[i].Y) + _Coords[i].X))
			c = !c;
	}

	return c;

}

bool
GDSPolygon::isPointInside_wborders(const Point2D& P)
{
	/*GDSTriangle T;

	//We are doing this brute force
	for (unsigned int i = 0; i<(indices.size() / 3); i++)
	{
		T.set(_Coords[indices[i * 3 + 0]], _Coords[indices[i * 3 + 1]], _Coords[indices[i * 3 + 2]]);
		if (T.isPointInside(P))
			return true;
	}
	return false;*/
	size_t i, j;
	bool c = false;

	for (i = 0, j = _Coords.size() - 1; i < _Coords.size(); j = i++) {
		if (((_Coords[i].Y > P.Y) != (_Coords[j].Y > P.Y)) &&
			(P.X < (_Coords[j].X - _Coords[i].X) * (P.Y - _Coords[i].Y) / (_Coords[j].Y - _Coords[i].Y) + _Coords[i].X))
			c = !c;
		if (onLine(_Coords[i], _Coords[j], P)) {
			c = true;
			break;
		}
	}

	return c;

}

bool
GDSPolygon::isEdgeInside_wborders(const Edge& E)
{
	size_t i, j, k, l;
	bool c = false;

	Point2D P = E.GetA();
	Point2D P1 = E.GetB();
	set<size_t> IntesectEdgeList;
	for (i = 0, j = _Coords.size() - 1; i < _Coords.size() - 1; j = i++) {
		for (k = i + 1, l = i; k < _Coords.size(); l = k++) {
			if (_Coords[i] == _Coords[k]) {
				IntesectEdgeList.insert(i + 1);
				IntesectEdgeList.insert(k);
				break;
			}
			if (_Coords[l].IsonLine(_Coords[i], _Coords[j]) && _Coords[k].IsonLine(_Coords[i], _Coords[j])) {
				IntesectEdgeList.insert(k);
			}
			if (_Coords[i].IsonLine(_Coords[k], _Coords[l]) && _Coords[j].IsonLine(_Coords[k], _Coords[l])) {
				IntesectEdgeList.insert(i);
			}
		}
	}
	for (i = 0, j = _Coords.size() - 1; i < _Coords.size(); j = i++) {
		if (((_Coords[i].Y > P.Y) != (_Coords[j].Y > P.Y)) &&
			(P.X < (_Coords[j].X - _Coords[i].X) * (P.Y - _Coords[i].Y) / (_Coords[j].Y - _Coords[i].Y) + _Coords[i].X))
			c = !c;
		
		if (IntesectEdgeList.find(i) != IntesectEdgeList.end()){
			if (onLine(_Coords[i], _Coords[j], P)) {
				if (isPointInside_wborders(P1)) {
					c = true;
					break;
				}
			}
			continue;
		}
		if (onLine(_Coords[i], _Coords[j], P)) {
			if (isPointInside_wborders(P1)) {
				Edge curEdge;
				curEdge = Edge(_Coords[j], _Coords[i]);
				if (fabs(fabs((E.direction() - curEdge.direction())) - M_PI) < 0.01) {
					// opposit direction
					c = false;
					break;
				} else if(fabs((E.direction() - curEdge.direction()))  < 0.01 
					&& !isPointInside(P1)
					&& !onLine(_Coords[i], _Coords[j], P1)
					) {
					// Same direction but Edge longer than current
					// _Coords[i] is the first point on the line P P1 who will be out of poly
					Point2D Pepsilon(_Layer->Units->Unitu*cos(E.direction()), _Layer->Units->Unitu*sin(E.direction()));
					if (!isPointInside_wborders(_Coords[i] + (Pepsilon * 1.5))) {
						c = false;
							break;
					}
					else {
						c = true;
						break;
					}
				} else if (fabs(fabs((E.direction() - curEdge.direction()))- M_PI/2) < 0.01) {
					// Perpendicular So look for interserction
					if (intersect_woborder(E)) {
						c = false;
						break;
					} else {
						c = true;
						break;
					}
				} else {
					c = true;
					break;
				}
			} else {
				c = false;
				break;
			}
			
		}
	}

	return c;

}

bool
GDSPolygon::isPolygonInside(const GDSPolygon& poly) {
	// Check if the "poly" is inside "this"
	if (this->bbox.isBBInside(poly.bbox)) {
		if (_Coords.size() <= 4) {
			return true;
		}
		//Check all poly point
		for (unsigned int i = 0; i<poly._Coords.size(); i++)
		{
			if (!isPointInside(poly._Coords[i])) {
				return false;
			}
		}
		return true;

	}
	else
		return false;
}

bool
GDSPolygon::isPolygonInside_wborders(const GDSPolygon& poly) {

	//if (this->bbox.isBBInside_wborders(poly.bbox)) {
	if (this->bbox.isBBInside(poly.bbox)) {
			//Check all poly point
		size_t i, j, k, l;
		set<size_t> IntesectEdgeList;
		/*for (i = 0; i < poly._Coords.size() - 1; i++) {
			for (j = i + 1; j < poly._Coords.size(); j++) {
				if (poly._Coords[i] == poly._Coords[j]) {
					IntesectPtList.insert(poly._Coords[i]);
					i = j;
					j = poly._Coords.size();
				}
			}
		}*/
		for (i = 0, j = poly._Coords.size() - 1; i < poly._Coords.size() - 1; j = i++) {
			for (k = i + 1, l = i; k < poly._Coords.size(); l = k++) {
				if (poly._Coords[i] == poly._Coords[k]) {
					IntesectEdgeList.insert(i + 1);
					IntesectEdgeList.insert(k);
					break;
				}
				Point2D A = poly._Coords[i];
				Point2D B = poly._Coords[j];
				Point2D C = poly._Coords[l];
				Point2D D = poly._Coords[k];
				if (A.IsonLine(poly._Coords[k], poly._Coords[l]) && B.IsonLine(poly._Coords[k], poly._Coords[l])) {
					IntesectEdgeList.insert(i);
				}
				if (C.IsonLine(poly._Coords[i], poly._Coords[j]) && D.IsonLine(poly._Coords[i], poly._Coords[j])) {
					IntesectEdgeList.insert(k);
				}
			}
		}
		
		for ( i = 0, j = poly._Coords.size() - 1; i < poly._Coords.size(); j = i++)
		{
			if (IntesectEdgeList.find(i) != IntesectEdgeList.end()) {
				continue;
			}
			Edge E;
			E = Edge(poly._Coords[j], poly._Coords[i]);
			
			if (!isEdgeInside_wborders(E)) {
				return false;
			}
		}
		return true;

	}
	else
		return false;
}

bool get_line_intersection(const Point2D& P0, const Point2D& P1,
	const Point2D& P2, const Point2D& P3, Point2D *I)
{
	double s1_x, s1_y, s2_x, s2_y;
	s1_x = P1.X - P0.X;     s1_y = P1.Y - P0.Y;
	s2_x = P3.X - P2.X;     s2_y = P3.Y - P2.Y;

	double s, t;
	s = (-s1_y * (P0.X - P2.X) + s1_x * (P0.Y - P2.Y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (P0.Y - P2.Y) - s2_y * (P0.X - P2.X)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		// Collision detected
		if (I != NULL) {
			I->X = P0.X + (t * s1_x);
			I->Y = P0.Y + (t * s1_y);
		}
		return true;
	}

	return false; // No collision
}

void GDSPolygon::FollowFrameAgain(GDSPolygon *poly, GDSPolygon *Mergepoly) {
	size_t Size = this->_Coords.size();
	size_t PolySize = poly->_Coords.size();
	int Polysign = poly->OrientateAnticlockwise() ? 1 : -1;
	size_t PolyFrameSartIndex;
	int sign = OrientateAnticlockwise() ? 1 : -1;
	Point2D *IntersectPoint;
	IntersectPoint = new Point2D;

	Point2D P0;
	Point2D P1;
	Point2D P2;
	Point2D P3;
	Point2D P4;
	Point2D P5;

	struct {
		unsigned int operator() (int val, int sign, unsigned int size) const {
			while (val < 0) {
				val += size;
			}
			return sign>0 ? val%size : size - 1 - val%size;
		}
	} index;

	if (_FrameCurrentIndexSet && poly->_FrameCurrentIndexSet && ( _FrameCurrentIndex == _Coords.size() + _FrameStartIndex)  && (poly->_FrameCurrentIndex == poly->_Coords.size() + poly->_FrameStartIndex))
		// both poly full loop end
		return;
	if (!_FrameStartIndexSet) {
		for (unsigned int i = 0; i < this->_Coords.size(); i++) {
			if (!poly->isPointInside_wborders(this->_Coords[index(i, sign, this->_Coords.size())])) {
				_FrameStartIndex = i;
				_FrameCurrentIndex = i;
				LastpointInsideIndex = -1;
				_FrameStartIndexSet = true;
				_FrameCurrentIndexSet = true;
				break;
			}
		}
	}

//	while (_FrameCurrentIndex < this->_Coords.size() + _FrameStartIndex+1) {
	while (_FrameCurrentIndex < this->_Coords.size() + _FrameStartIndex) {
		size_t i = _FrameCurrentIndex;
		P0 = this->_Coords[index(i - 1, sign, Size)];
		P1 = this->_Coords[index(i, sign, Size)];
		P2 = this->_Coords[index(i + 1, sign, Size)];

		if (poly->_FrameCurrentIndexSet) {
			P3 = poly->_Coords[index(poly->_FrameCurrentIndex - 1, Polysign, PolySize)];
			P4 = poly->_Coords[index(poly->_FrameCurrentIndex, Polysign, PolySize)];
			P5 = poly->_Coords[index(poly->_FrameCurrentIndex + 1, Polysign, PolySize)];
			get_line_intersection(P3, P4, P0, P1, IntersectPoint);
		}

		if (strcmp(this->_Layer->Name, "B_M3 ") == 0) {
			if (-1390 < P1.X && P1.X <-1380 && 1010<P1.Y && P1.Y <1030) {
				i = i;
			}
		}

		if ( poly->isPointInside_wborders(P1) 
			|| (!poly->isPointInside_wborders(P0) && !poly->isPointInside_wborders(P1) 
				&& intersect(poly, P0, P1) 
				&& ((Mergepoly->_Coords.size() > 0
				     && Mergepoly->_Coords[Mergepoly->_Coords.size() - 1] == P0) 
				   || Mergepoly->_Coords.size() == 0
				   )
				)
			|| (((Mergepoly->_Coords.size() == 0 && false)
				   || Mergepoly->_Coords.size() > 0 && Mergepoly->_Coords[Mergepoly->_Coords.size() - 1] != *IntersectPoint) 
				&& intersect(poly, P0, P1) && poly->isPointInside_wborders(P1))
			) {
			if (!poly->_FrameCurrentIndexSet) {
				// Parcours du polynome pour trouver le segment le plus proche 
				double dmin = -1;
				for (unsigned int j = 0; j < poly->_Coords.size(); j++) {
					P3 = poly->_Coords[index(j - 1, Polysign, PolySize)];
					P4 = poly->_Coords[index(j, Polysign, PolySize)];
					P5 = poly->_Coords[index(j + 1, Polysign, PolySize)];
					if (get_line_intersection(P3, P4,
						P0, P1, IntersectPoint)) {
						double dx = fabs(P0.X - IntersectPoint->X);
						double dy = fabs(P0.Y - IntersectPoint->Y);
						double d = sqrt(pow(dx, 2) + pow(dy, 2));
						if (d < dmin || dmin <0) {
							dmin = d;
							poly->_FrameCurrentIndex = j;
							poly->_FrameCurrentIndexSet = true;
							PolyFrameSartIndex = j;
						}
					}
				}
				if (!poly->_FrameCurrentIndexSet) {
					poly->_FrameCurrentIndex = 0;
					poly->_FrameCurrentIndexSet = true;
					PolyFrameSartIndex = 0;
					if (!poly->_FrameStartIndexSet) {
						poly->_FrameStartIndex = 1;
						poly->_FrameStartIndexSet = true;
					}
				}
				else {
					if (!poly->_FrameStartIndexSet) {
						poly->_FrameStartIndex = PolyFrameSartIndex;
						poly->_FrameStartIndexSet = true;
					}

				}
			}
			else {
				PolyFrameSartIndex = poly->_FrameStartIndex;
				//FirstCheck = false;
			}
			for (unsigned int j = poly->_FrameCurrentIndex; j < poly->_Coords.size() + PolyFrameSartIndex + 1; j++) {
				P3 = poly->_Coords[index(j - 1, Polysign, PolySize)];
				P4 = poly->_Coords[index(j, Polysign, PolySize)];
				P5 = poly->_Coords[index(j + 1, Polysign, PolySize)];
				
				if (this->isPointInside_wborders(P4)) {
					// Search for the fisrt point outside "this" poly
					continue;
				}
				if (onLine(P0, P1, P4) && onLine(P4, P5, P1)) {
					// The two poly follow the same line
					_FrameCurrentIndex++;
					poly->_FrameCurrentIndex = j + 1;
					poly->_FrameCurrentIndexSet = true;
					poly->FollowFrameAgain(this, Mergepoly);
					if (!poly->_FrameStartIndexSet) {
						poly->_FrameStartIndex = j;
						poly->_FrameStartIndexSet = true;
					}

					break;
				}
				else if (get_line_intersection(P3, P4,
					P0, P1, IntersectPoint)) {
					_FrameCurrentIndex = i;
					if (*IntersectPoint == P3 && onLine(P0, P1, P3) && (P1 > P0)==(P3> poly->_Coords[index(j - 2, Polysign, PolySize)])) {
						// Same point Same line Same direction
						poly->_FrameCurrentIndex = j;
						poly->LastpointInsideIndex = j - 1;
						// continue with "this" poly
						break;
					}
					//if ( ( (Size + _FrameStartIndex) > _FrameCurrentIndex ) 
					//	|| (Mergepoly->_Coords.size() > 0 && P0 == Mergepoly->_Coords[Mergepoly->_Coords.size() - 1]) ) {
						Mergepoly->AddPoint(IntersectPoint->X, IntersectPoint->Y);
					//}
					if (!poly->_FrameStartIndexSet) {
						poly->_FrameStartIndex = j;
						poly->_FrameStartIndexSet = true;
					}
					if(false && !this->isPointInside(P3) && !this->isPointInside(P4))
						// both point of other poly are out side the current poly
						poly->_FrameCurrentIndex = j+1;
					else
						poly->_FrameCurrentIndex = j;

					poly->_FrameCurrentIndexSet = true;
					poly->LastpointInsideIndex = j - 1;
					poly->FollowFrameAgain(this, Mergepoly);
					break;
				}
			}
		}

		//if( !poly->isPointInside_wborders(P1) && (!intersect(poly,P0,P1) || poly->isPointInside_wborders(P0))) {
		//if (!poly->isPointInside_wborders(P1) && (Size + _FrameStartIndex - 1) > _FrameCurrentIndex) {
		if (!poly->isPointInside_wborders(P1) && (Size + _FrameStartIndex ) > _FrameCurrentIndex) {
				Mergepoly->AddPoint(P1.X, P1.Y);
		}
		else if (poly->_FrameCurrentIndexSet && P1 == P4 
			&& (Size + _FrameStartIndex - 1) > _FrameCurrentIndex 
			&& (PolySize + poly->_FrameStartIndex-1) > poly->_FrameCurrentIndex) {
			// both poly follow the same route
			Mergepoly->AddPoint(P1.X, P1.Y);
			poly->_FrameCurrentIndex++;
		}

		if (((Size + _FrameStartIndex) == _FrameCurrentIndex) && ((PolySize + poly->_FrameStartIndex) == poly->_FrameCurrentIndex)) {
			// Close poly with intersect
			if (get_line_intersection(P3, P4, P0, P1, IntersectPoint)) {
				//Mergepoly->AddPoint(IntersectPoint->X, IntersectPoint->Y);
			}
		}

		_FrameCurrentIndex++;
	}
	// End Frame
	_FrameCurrentIndex = this->_Coords.size() + _FrameStartIndex;
    delete IntersectPoint;
}

void GDSPolygon::MergePoly_wClipper(GDSPolygon *poly, GDSPolygon *Mergepoly) {
	Clipper c;
	Paths p_this, p1, res;
	Path p, p2;
	p.resize(this->_Coords.size());
	for (size_t i = 0; i < this->_Coords.size(); i++) {
		p[i].X = rounded(this->GetXCoords(i) / _Layer->Units->Unitu);
		p[i].Y = rounded(this->GetYCoords(i) / _Layer->Units->Unitu);
	}
	p_this.resize(1);
	p_this[0] = p;

	p2.resize(poly->_Coords.size());
	for (size_t i = 0; i < poly->_Coords.size(); i++) {
		p2[i].X = rounded(poly->GetXCoords(i) / _Layer->Units->Unitu);
		p2[i].Y = rounded(poly->GetYCoords(i) / _Layer->Units->Unitu);
	}
	p1.resize(1);
	p1[0]=p2;

	c.AddPaths(p_this, ptSubject, true);
	c.AddPaths(p1, ptClip, true);

	c.Execute(ctUnion, res, pftNonZero, pftNonZero);
	//pftEvenOdd, pftNonZero, pftPositive, pftNegative
	Point2D LastPoint;
	bool CoutourFound = false;
	for (size_t i = 0; i < res.size(); i++) {
		p = res[i];
		if (Orientation(p)) {
			assert(!CoutourFound);
			
			// Contour found
			CoutourFound = true;
			Point2D AddPoint;
			Point2D NewPoint = Point2D(p[0].X, p[0].Y) * _Layer->Units->Unitu;
			size_t StartIndex = 0;
			int sign = 1;
			for (size_t j = 0; j < p.size(); j++) {
				AddPoint = Point2D(p[j].X, p[j].Y) * _Layer->Units->Unitu;
				if (AddPoint > NewPoint) {
					NewPoint = AddPoint;
					StartIndex = j;
				}
			}
			for (size_t j = 0; j < p.size(); j++) {
				if (j + (StartIndex*sign) > p.size() - 1) {
					StartIndex = j;
					sign = -1;
				}
				LastPoint = Point2D(p[j + (StartIndex*sign)].X, p[j + (StartIndex*sign)].Y) * _Layer->Units->Unitu;
				Mergepoly->AddPoint(LastPoint);
			}
		}
		else {
			assert((i < res.size() - 1) || Mergepoly->GetPoints() > 0);
		}
	}
	assert(Mergepoly->isPolygonInside_wborders(*poly) && Mergepoly->isPolygonInside_wborders(*this));
	
	Point2D FirstPoint= Mergepoly->GetCoords(0);
	for (size_t i = 0; i < res.size(); i++) {
		p = res[i];
		if (!Orientation(p)) {
			// Hole found
			Point2D AddPoint;
			Point2D StartPoint = Point2D(p[0].X, p[0].Y) * _Layer->Units->Unitu;
			size_t StartIndex = 0;
			int sign = 1;
			GDSPolygon poly= GDSPolygon(*Mergepoly);
			poly.Clear();
			
			// Found the start hole point ( max X or max Y)
			for (size_t j = 1; j < p.size(); j++) {
				AddPoint = Point2D(p[j].X, p[j].Y) * _Layer->Units->Unitu;
				if (AddPoint > StartPoint) {
					StartPoint = AddPoint;
					StartIndex = j;
				}
			}
			// Find the closest edge to point
			size_t j, k;
			size_t EdgeIndex=0;
			Point2D MidPoint ;
			if (StartIndex != 0 ) { 
				MidPoint = (StartPoint + Point2D(p[StartIndex - 1].X, p[StartIndex - 1].Y) * _Layer->Units->Unitu) / 2;
			} else {
				MidPoint = (StartPoint + Point2D(p[p.size()-1].X, p[p.size() - 1].Y) * _Layer->Units->Unitu) / 2;
			}
			assert(MidPoint.Y == StartPoint.Y);
			double dist = -1;
			
			for ( j = 0, k= Mergepoly->GetPoints()-1; j < Mergepoly->GetPoints(); k=j++) {
				Edge CurEdge = Edge(Mergepoly->GetCoords(k), Mergepoly->GetCoords(j));
				if ((dist >= CurEdge.distance(StartPoint) || dist <0)
					&& (   (StartPoint.X <= Mergepoly->GetCoords(j).X && StartPoint.X >= Mergepoly->GetCoords(k).X)
						|| (StartPoint.X >= Mergepoly->GetCoords(j).X && StartPoint.X <= Mergepoly->GetCoords(k).X)
						|| (StartPoint.Y <= Mergepoly->GetCoords(j).Y && StartPoint.Y >= Mergepoly->GetCoords(k).Y)
						|| (StartPoint.Y >= Mergepoly->GetCoords(j).Y && StartPoint.Y <= Mergepoly->GetCoords(k).Y)
						)
					) {
					dist = CurEdge.distance(StartPoint);
					EdgeIndex = k;
					//dist = CurEdge.distance(StartPoint);
				}
			}
			assert(dist >= 0);
			FirstPoint = Mergepoly->GetCoords(EdgeIndex);
			if (EdgeIndex < Mergepoly->GetPoints() - 1) {
				LastPoint = Mergepoly->GetCoords(EdgeIndex + 1); 
			} else {
				LastPoint = Mergepoly->GetCoords(0);
			}
			//assert(FirstPoint.Y == LastPoint.Y);
			if (FirstPoint.X == LastPoint.X) {
				AddPoint = Point2D(FirstPoint.X, StartPoint.Y);
			}
			else if (FirstPoint.Y == LastPoint.Y) {
				AddPoint = Point2D(StartPoint.X, FirstPoint.Y);
			}
			else {
				assert(AddPoint == AddPoint);
			}
			
			for (size_t j = 0; j <= EdgeIndex; j++) {
				poly.AddPoint(Mergepoly->GetCoords(j));
			}

			poly.AddPoint(AddPoint);
			poly.AddPoint(StartPoint);
			for (size_t j = 1; j < p.size(); j++) {
				if (j + (StartIndex*sign) > p.size() - 1) {
					StartIndex = j;
					sign = -1;
				}
				poly.AddPoint(p[j+(StartIndex*sign)].X * _Layer->Units->Unitu, p[j+ (StartIndex*sign)].Y * _Layer->Units->Unitu);
			}
			poly.AddPoint(StartPoint);
			poly.AddPoint(AddPoint);
			for (size_t j = EdgeIndex+1; j < Mergepoly->GetPoints(); j++) {
				poly.AddPoint(Mergepoly->GetCoords(j));
			}
			poly.CopyInto(Mergepoly);
		}
	}
	
}

void
GDSPolygon::Merge(GDSPolygon *poly) {
	GDSPolygon *Mergepoly;

	Mergepoly = new GDSPolygon;
	CopyInto(Mergepoly);
	Mergepoly->Clear();
	_FrameStartIndexSet = false;
	_FrameCurrentIndexSet = false;
	_FrameStartIndex = NULL;
	_FrameCurrentIndex = NULL;
	poly->_FrameStartIndexSet = false;
	poly->_FrameCurrentIndexSet = false;
	poly->_FrameStartIndex = NULL;
	poly->_FrameCurrentIndex = NULL;
	/*if (bbox.isBBInside(poly->bbox)) {
		FollowFrameAgain(poly, Mergepoly);
	} else {
		poly->FollowFrameAgain(this, Mergepoly);
	}*/
	MergePoly_wClipper(poly, Mergepoly);
	//Mergepoly->SelfIntersect();
	//Mergepoly->Tesselate();
	assert(Mergepoly->isPolygonInside_wborders(*poly) && Mergepoly->isPolygonInside_wborders(*this));
	assert(Mergepoly->isPolygonInside_wborders(*poly) && Mergepoly->isPolygonInside_wborders(*this));
	Mergepoly->CopyInto(this);
	this->Tesselate(true /*force*/);
}

bool GDSPolygon::SelfIntersect() {
	for (size_t i = 0; i < _Coords.size() - 1; i++) {
		for (size_t j = i + 1; j < _Coords.size(); j++) {
			if (GetCoords(i) == GetCoords(j)) {
				return true;
			}
		}
	}
	return false;
}

double
GDSPolygon::Area()
{
	GDSTriangle T;
	double area = 0;

	//We are doing this brute force
	for (unsigned int i = 0; i<(indices.size() / 3); i++)
	{
		T.set(_Coords[indices[i * 3 + 0]], _Coords[indices[i * 3 + 1]], _Coords[indices[i * 3 + 2]]);
		area += T.Area();
	}
	return area;

}

// Private functions

double 
GDSPolygon::area(const Point2D& A, const Point2D& B, const Point2D& C)
{
    return ((B.X - A.X) * (C.Y - A.Y)) - ((B.Y - A.Y) * (C.X - A.X));
}

bool 
GDSPolygon::intersect(GDSPolygon *Poly, const Point2D& A, const Point2D& B ) {
	Point2D P0, P1;

	for (unsigned int j = 0; j < Poly->_Coords.size(); j++) {
		P0 = Poly->_Coords[j%Poly->_Coords.size()];
		P1 = Poly->_Coords[(j + 1)%Poly->_Coords.size()];
		if (get_line_intersection(P0, P1, A, B, NULL)) {
			return true;
		}
	}
	return false;
}

bool
GDSPolygon::intersect(const Edge& E) {
	Edge CurEdge;

	for (unsigned int j = 0; j <_Coords.size(); j++) {
		CurEdge = Edge(_Coords[j%_Coords.size()],
			_Coords[(j + 1) % _Coords.size()]);
		if (CurEdge.intersection(E, NULL)) {
			return true;
		}
	}
	return false;
}

bool
GDSPolygon::intersect_woborder(const Edge& E) {
	Edge CurEdge;
	
	size_t i, j, k, l;
	set<size_t> IntesectEdgeList;
	
	for (i = 0, j = _Coords.size() - 1; i < _Coords.size() - 1; j = i++) {
		for (k = i + 1, l = i; k < _Coords.size(); l = k++) {
			if (_Coords[i] == _Coords[k]) {
				IntesectEdgeList.insert(i + 1);
				IntesectEdgeList.insert(k);
				break;
			}
			if (_Coords[l].IsonLine(_Coords[i], _Coords[j]) && _Coords[k].IsonLine(_Coords[i], _Coords[j])) {
				IntesectEdgeList.insert(k);
			}
			if (_Coords[i].IsonLine(_Coords[k], _Coords[l]) && _Coords[j].IsonLine(_Coords[k], _Coords[l])) {
				IntesectEdgeList.insert(i);
			}
		}
	}
	for (i = 0, j = _Coords.size() - 1; i < _Coords.size() - 1; j = i++) {
		
		if (IntesectEdgeList.find(i) != IntesectEdgeList.end()) {
			continue;
		}
		
		CurEdge = Edge(_Coords[j], _Coords[i]);
		if (CurEdge.intersection_woborder(E, NULL)) {
			return true;
		}
	}
	return false;
}

bool
GDSPolygon::onLine(const Point2D& A, const Point2D& B,const Point2D& P)
{
	// Check if a point lies on a line segment


	// Inside bounding box?
	if(P.X < A.X - _Layer->Units->Unitu && P.X < B.X - _Layer->Units->Unitu)
		return false;
	if(P.X > A.X + _Layer->Units->Unitu && P.X > B.X + _Layer->Units->Unitu)
		return false;
	if(P.Y < A.Y - _Layer->Units->Unitu && P.Y < B.Y - _Layer->Units->Unitu)
		return false;
	if(P.Y > A.Y + _Layer->Units->Unitu && P.Y > B.Y + _Layer->Units->Unitu)
		return false;
	
	if (A == P)
		return true;
	if (B == P)
		return true;
	if (fabs(A.Y - B.Y) < _Layer->Units->Unitu && fabs(P.Y - A.Y) < _Layer->Units->Unitu)
		return true;
	if (fabs(A.X - B.X) < _Layer->Units->Unitu && fabs(P.X - A.X) < _Layer->Units->Unitu)
		return true;
	
	// On line?
	double a,b;  
	
	if( B.X - A.X != 0.0f)
	{
		a = (B.Y - A.Y) / (B.X - A.X);
		b = A.Y - a * A.X;
		if ( fabs(P.Y - (a*P.X+b)) < epsilon)
			return true;
	}
	else
	{
		a = (B.X - A.X) / (B.Y - A.Y);
		b = A.X - a * A.Y;
		if ( fabs(P.X - (a*P.Y+b)) < epsilon)
			return true;
	}

	return false;
}

bool 
GDSPolygon::insideTriangle(const Point2D& A, const Point2D& B, const Point2D& C,const Point2D& P)
{
    double ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
    double cCROSSap, bCROSScp, aCROSSbp;

/*	// Exclude points on vertices
	if(A.X == P.X && A.Y == P.Y)
		return false;
	if(B.X == P.X && B.Y == P.Y)
		return false;
	if(C.X == P.X && C.Y == P.Y)
		return false;
*/   
/*    ax = C.X - B.X;  ay = C.Y - B.Y;
    bx = A.X - C.X;  by = A.Y - C.Y;
    cx = B.X - A.X;  cy = B.Y - A.Y;
    apx = P.X - A.X;  apy = P.Y - A.Y;
    bpx = P.X - B.X;  bpy = P.Y - B.Y;
    cpx = P.X - C.X;  cpy = P.Y - C.Y;
    
    aCROSSbp = ax * bpy - ay * bpx;
    cCROSSap = cx * apy - cy * apx;
    bCROSScp = bx * cpy - by * cpx;
    
    bool res ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
*/    
    //return ((aCROSSbp >= epsilon) && (bCROSScp >= epsilon) && (cCROSSap >= epsilon));
	vector<Point2D> Coords;
	Coords.push_back(A);
	Coords.push_back(B);
	Coords.push_back(C);
	size_t i, j;
	bool c = false;

	for (i = 0, j = Coords.size() - 1; i < Coords.size(); j = i++) {
		if (onLine(Coords[i], Coords[j], P)) {
			c = true;
			break;
		}
		if (((Coords[i].Y > P.Y) != (Coords[j].Y > P.Y)) &&
			(P.X < (Coords[j].X - Coords[i].X) * (P.Y - Coords[i].Y) / (Coords[j].Y - Coords[i].Y) + Coords[i].X))
			c = !c;
	}
//	assert(res == c);
	return c;

}

bool
GDSPolygon::insideTriangle_woborder(const Point2D& A, const Point2D& B, const Point2D& C, const Point2D& P)
{
	double ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	double cCROSSap, bCROSScp, aCROSSbp;

	// Exclude points on vertices
	if (A.X == P.X && A.Y == P.Y)
		return false;
	if (B.X == P.X && B.Y == P.Y)
		return false;
	if (C.X == P.X && C.Y == P.Y)
		return false;

/*	ax = C.X - B.X;  ay = C.Y - B.Y;
	bx = A.X - C.X;  by = A.Y - C.Y;
	cx = B.X - A.X;  cy = B.Y - A.Y;
	apx = P.X - A.X;  apy = P.Y - A.Y;
	bpx = P.X - B.X;  bpy = P.Y - B.Y;
	cpx = P.X - C.X;  cpy = P.Y - C.Y;

	aCROSSbp = ax * bpy - ay * bpx;
	cCROSSap = cx * apy - cy * apx;
	bCROSScp = bx * cpy - by * cpx;

	bool res((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
*/
	//return ((aCROSSbp >= epsilon) && (bCROSScp >= epsilon) && (cCROSSap >= epsilon));
	vector<Point2D> Coords;
	Coords.push_back(A);
	Coords.push_back(B);
	Coords.push_back(C);
	size_t i, j;
	bool c = false;

	for (i = 0, j = Coords.size() - 1; i < Coords.size(); j = i++) {
		if (onLine(Coords[i], Coords[j], P)) {
			c = false;
			break;
		}
		if (((Coords[i].Y > P.Y) != (Coords[j].Y > P.Y)) &&
			(P.X < (Coords[j].X - Coords[i].X) * (P.Y - Coords[i].Y) / (Coords[j].Y - Coords[i].Y) + Coords[i].X))
			c = !c;
	}
//	assert(res == c);
	return c;

}

void GDSPolygon::transformPoints(const GDSMat& M)
{
	// Clear bounding box
	bbox.clear();
	bbox3D.clear();

	for(size_t i=0;i<_Coords.size();i++)
	{
		_Coords[i] = M * _Coords[i];
		bbox.addPoint(_Coords[i]);
		double Zmin, Zmax;
		Zmin = _Height;
		Zmax = _Height + _Thickness;
		bbox3D.addPoint(_2Dto3D(_Coords[i], Zmin));
		bbox3D.addPoint(_2Dto3D(_Coords[i], Zmax));

	}
}

bool GDSPolygon::intersect(GDSPolygon *P1, GDSPolygon *P2)
{
	GDSTriangle		T1, T2;

	// Bounding box intersection
	if(!GDSBB::intersect(P1->bbox, P2->bbox))
		return false;
	
	// Bounding intersection
	for (size_t i = 0; i < P1->GetPoints(); i++)
	{
		for (size_t j = 0; j < P2->GetPoints(); j++)
		{
			if (P1->GetCoords(i) == P2->GetCoords(j)) { 
				return true;;
			}
		}
	}

	//We are doing this brute force
	for(size_t i=0;i<P1->indices.size()/3;i++)
	{
		if (P1->indices[i * 3 + 1] >= P1->_Coords.size()) {
			P1->Tesselate(true /*force*/);
			i = 0;
		}
		T1.set(P1->_Coords[P1->indices[i * 3 + 0]], P1->_Coords[P1->indices[i * 3 + 1]], P1->_Coords[P1->indices[i * 3 + 2]]);
		for(size_t j=0;j<P2->indices.size()/3;j++)
		{
			
			if (P2->indices[j * 3 + 1] >= P2->_Coords.size()) {
				P2->Tesselate(true /*force*/);
				j = 0;
			}
			if (P2->indices[j * 3 + 2] >= P2->_Coords.size()) {
				P2->Tesselate(true /*force*/);
				j = 0;
			}

			T2.set(P2->_Coords[P2->indices[j*3+0]], P2->_Coords[P2->indices[j*3+1]], P2->_Coords[P2->indices[j*3+2]]);

			if(GDSTriangle::intersect(T1, T2))
				return true;
		}
	}

	return false;
}

