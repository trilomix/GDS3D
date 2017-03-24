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

#ifndef __GDSPOLYGON_H__
#define __GDSPOLYGON_H__

#include "gds_globals.h"
#include "process_cfg.h"
#include <math.h>
#include "clipper/clipper.hpp"

// All these types are 2D
class GDSMat
{
private:
	// 3x3 matrix, in XYW form, assumes bottom row is always [0,0,1]
	// | 0 2 4 |
	// | 1 3 5 |
	double entries[6];
public:
	GDSMat();
	GDSMat(double e0, double e1, double e2, double e3, double e4, double e5);

	// Operators
	Point2D operator*(const Point2D& P) const;
	GDSMat operator*(const GDSMat& M) const;
	double operator[](const int index) const {return entries[index];};
	bool operator<(const GDSMat& B) const; // Comparison for ordered lists

	// Manipulation
	void loadIdentity();
	void setScaling(const double& X, const double& Y);
	void setTranslation(const double& X, const double& Y);
	void setRotation(const double& angle);

	bool NegativeTrace() const;
	GDSMat Inverse() const;
	void Round();
};

inline
bool GDSMat::NegativeTrace() const
{
	return ((entries[0] < 0) != (entries[3] < 0));
}

// | 0 2 4 |   | X |
// | 1 3 5 | X | Y | 
inline Point2D GDSMat::operator*(const Point2D& P) const
{
	return Point2D(entries[0]*P.X+entries[2]*P.Y+entries[4], entries[1]*P.X+entries[3]*P.Y+entries[5]);
}

// | 0 2 4 |   | 0 2 4 |
// | 1 3 5 | X | 1 3 5 | 
inline GDSMat GDSMat::operator*(const GDSMat& M) const
{
	return GDSMat(
		entries[0]*M.entries[0]+entries[2]*M.entries[1],
		entries[1]*M.entries[0]+entries[3]*M.entries[1],
		entries[0]*M.entries[2]+entries[2]*M.entries[3],
		entries[1]*M.entries[2]+entries[3]*M.entries[3],
		entries[0]*M.entries[4]+entries[2]*M.entries[5]+entries[4],
		entries[1]*M.entries[4]+entries[3]*M.entries[5]+entries[5]
		);
}

inline bool GDSMat::operator<(const GDSMat& A) const
{
	for(unsigned int i=0;i<6;i++)
	{
		if(this->entries[i] < A.entries[i])
			return true;
		if(this->entries[i] > A.entries[i])
			return false;

		// Jump to next entry
	}
	 
	//We should never reach this point, means matrices are equal
	return false;
}

inline
void GDSMat::Round()
{
	// Rounds the matrix translation on 1nm grid and locks rotation to 90 degrees
	//for(unsigned int i=0;i<=3;i++)
	//	entries[i] = floor(entries[i] + 0.5f); // Make either 0 or 1
	for(unsigned int i=4;i<=5;i++)
		entries[i] = floor(entries[i] *1000.0f + 0.5f) / 1000.0f;
}

class GDSBB
{
public:

	Point2D min, max;

	GDSBB();

	GDSBB(Point2D A, Point2D B);

	void clear();
	bool isEmpty() const;
	void addPoint(const Point2D& P);
	void merge(const GDSBB& BB);
	void transform(const GDSMat& M);
	double area();
	bool isPointInside(const Point2D& P);

	bool isBBInside(const GDSBB & BB);

	bool isBBInside_wborders(const GDSBB & BB);

	static bool intersect(const GDSBB& BB1, const GDSBB& BB2);
	bool intersect_wborders(const GDSBB & BB1, const GDSBB & BB2);
	//static bool intersect(const GDSBB& BB1, const GDS3DBB& BB2);
	bool operator<(const GDSBB & BB) const;
	bool operator==(const GDSBB & BB)const;
	bool operator!=(const GDSBB & BB) const;
	GDSBB operator*(const GDSMat & M) const;
};

inline bool GDSBB::operator<(const GDSBB & BB) const
{
	if (this->max < BB.max && this->max != BB.max) {
		return true;
	}
	if(this->max == BB.max && this->min < BB.min)
		return true;
	return false;
}

inline bool GDSBB::operator==(const GDSBB & BB) const
{
	if (this->max == BB.max && this->min == BB.min) {
		return true;
	}
	return false;
}
inline bool GDSBB::operator!=(const GDSBB & BB) const
{
	if (this->max != BB.max || this->min != BB.min) {
		return true;
	}
	return false;
}
inline GDSBB GDSBB::operator*(const GDSMat& M) const
{
	return GDSBB(M * min, M * max);
}


class GDS3DBB
{
public:

	Point3D min, max;

	GDS3DBB();

	void clear();
	bool isEmpty();
	void addPoint(const Point3D& P);
	void merge(const GDS3DBB& BB);
	void transform(const GDSMat& M);
	bool isPointInside(const Point3D& P);

	bool isPointInside(const Point2D & P);

	bool isBBInside(const GDS3DBB & BB);

	bool isBBInside_wborders(const GDS3DBB & BB);

	static bool intersect(const GDS3DBB& BB1, const GDS3DBB& BB2);
	static bool intersect(const GDSBB& BB1, const GDS3DBB& BB2);
	bool operator<(const GDS3DBB & BB) const;
	bool operator==(const GDS3DBB & BB) const;
};

inline bool GDS3DBB::operator<(const GDS3DBB & BB) const
{
	if (this->max < BB.max && this->max != BB.max) {
		return true;
	}
	if (this->max == BB.max && this->min < BB.min)
		return true;
	return false;
}

inline bool GDS3DBB::operator==(const GDS3DBB & BB) const
{
	if (this->max == BB.max && this->min == BB.min) {
		return true;
	}
	return false;
}


class GDSTriangle
{
private:
	Point2D	coords[3];
	GDSBB	bbox;

	void project(const Point2D& axis, double& min, double& max) const;
	static void normal(const Point2D& P1, const Point2D& P2, Point2D& N);
public:
	GDSTriangle() {};

	void set(const Point2D& P1, const Point2D& P2, const Point2D& P3);
	//void project(const Point2D & axis, double & min, double & max) const;
	static bool intersect(const GDSTriangle& T1, const GDSTriangle& T2);	
	bool isTriangleInside(const GDSTriangle & T);
	bool isPointInside(const Point2D & P);
	double Area();
};

class GDSPolygon
{
private:
	double			_Height;
	double			_Thickness;
	vector<Point2D>	_Coords;
	vector<size_t>		indices;
	struct ProcessLayer	*_Layer;
	GDSBB			bbox;
	GDS3DBB			bbox3D;
	char			*_NetName;

	bool			_FrameStartIndexSet;
	bool			_FrameCurrentIndexSet;
	size_t			_FrameStartIndex;
	size_t			_FrameCurrentIndex;
	size_t			LastpointInsideIndex;

	double epsilon;
	double area(const Point2D& A, const Point2D& B, const Point2D& C);
	bool intersect(GDSPolygon * Poly, const Point2D & A, const Point2D & B);
	bool intersect_woborder(const Edge & E);
	bool intersect(const Edge & E);
	bool onLine(const Point2D& A, const Point2D& B, const Point2D& P);
    bool insideTriangle(const Point2D& A, const Point2D& B, const Point2D& C,const Point2D& P);

public:
        GDSPolygon() {_Layer = NULL; _NetName = NULL;};
	GDSPolygon(double Height, double Thickness, struct ProcessLayer *Layer);
	GDSPolygon(ProcessLayer * Layer);
	~GDSPolygon();

    void Clear();
	void SetNetName(char * NewName);
	char * GetNetName();
	void SetLayer(ProcessLayer * Layer, double units);
    void CopyInto(GDSPolygon *p); // Remove? nothing really different from default copy..
	bool FindCoord(Point2D P);
	void AddPoint(Point2D P);
	void AddPoint(double X, double Y);
	bool RemovePoint(Point2D P);
	void Tesselate(); // Build a triangle index list

	void Tesselate(bool force);

	Point2D GetCoords(size_t Index);

	GDSBB* GetBBox();
	GDS3DBB* Get3DBBox();
	double GetHeight();
	double GetThickness();
	size_t GetPoints();
	vector<size_t>* GetIndices();
	double GetXCoords(size_t Index);
	double GetYCoords(size_t Index);
	float GetAngleCoords(unsigned int Index);
	void SetAngleCoords(unsigned int Index, float value);
    void Flip(); // Flip the winding order
    void Orientate(); // Make sure normal points upwards
	bool OrientateAnticlockwise();
	struct ProcessLayer *GetLayer();
    bool isSimple();
	bool isPointInside(const Point2D& P);

	bool isPointInside_wborders(const Point2D & P);

	bool isEdgeInside_wborders(const Edge & E);

	bool isPolygonInside(const GDSPolygon & poly);

	bool isPolygonInside_wborders(const GDSPolygon & poly);

	void FollowFrameAgain(GDSPolygon * poly, GDSPolygon * Mergepoly);

	void MergePoly_wClipper(GDSPolygon * poly, GDSPolygon * Mergepoly);

	void Merge(GDSPolygon * poly);

	bool SelfIntersect();

	double Area();

	void transformPoints(const GDSMat& M);
	static bool intersect(GDSPolygon *P1, GDSPolygon *P2);
};

inline GDSBB* GDSPolygon::GetBBox()
{
	return &bbox;
}

inline GDS3DBB* GDSPolygon::Get3DBBox()
{
	return &bbox3D;
}

inline struct ProcessLayer *GDSPolygon::GetLayer()
{
	return _Layer;
}

inline double GDSPolygon::GetHeight()
{
	return _Height;
}

inline double GDSPolygon::GetThickness()
{
	return _Thickness;
}

#endif // __GDSPOLYGON_H__
