//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2017 Bertrand Pigeard
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

#ifndef __GDSELEMENTS_H__
#define __GDSELEMENTS_H__

#include <math.h> 
#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
#endif

#define EPSILONPOINT 1000.0f

typedef signed long long cInt;

class Point2D
{
public:
	double X;
	double Y;
	double Unit;

	Point2D(){};
	Point2D(double X, double Y){this->X = X; this->Y = Y;};
	bool operator==(Point2D P) const;
	bool operator!=(Point2D P) const;
	bool operator<(Point2D P) const;
	bool operator>(Point2D P) const;
	double double_distance() const;
	double double_distance(const Point2D & p) const;
	bool IsonLine(const Point2D & A, const Point2D & B);
	Point2D dpx(double d);
};

inline bool Point2D::operator==(Point2D P) const {
	return floor(X* EPSILONPOINT + 0.5f) / EPSILONPOINT == floor(P.X* EPSILONPOINT + 0.5f) / EPSILONPOINT && floor(Y* EPSILONPOINT + 0.5f) / EPSILONPOINT == floor(P.Y* EPSILONPOINT + 0.5f) / EPSILONPOINT;
}

inline bool Point2D::operator!=(Point2D P) const {
	return (floor(X* EPSILONPOINT + 0.5f) / EPSILONPOINT != floor(P.X* EPSILONPOINT + 0.5f) / EPSILONPOINT || floor(Y* EPSILONPOINT + 0.5f) / EPSILONPOINT != floor(P.Y* EPSILONPOINT + 0.5f) / EPSILONPOINT);
}

inline bool Point2D::operator<(Point2D P) const {
	return (X < P.X || ((X == P.X)&& Y<P.Y) );
}

inline bool Point2D::operator>(Point2D P) const {
	return (X > P.X || ((X == P.X) && Y>P.Y) );
}

inline double Point2D::double_distance() const
{
	if (X == 0 && Y == 0) {
		return 0;
	}
	return sqrt(X * X + Y * Y);
}

inline Point2D
operator* (const Point2D &p, double s)
{
	return Point2D(p.X * s, p.Y * s);
}

inline Point2D
operator/ (const Point2D &p, double s)
{
	return Point2D(p.X / s, p.Y / s);
}

inline Point2D
operator+ (const Point2D &p1, const Point2D &p2)
{
	return Point2D(p1.X + p2.X, p1.Y + p2.Y);
}

inline Point2D
operator- (const Point2D &p1, const Point2D &p2)
{
	return Point2D(p1.X - p2.X, p1.Y - p2.Y);
}

static double rounded(double v) { return (v > 0 ? v + 0.5 : v - 0.5); }

/**
*  @brief The sign of the scalar product of two vectors.
*
*  Computes the scalar product of two vectors.
*  The first vector is A - B, the second B - C,
*  where A, B and C are points.
*
*  @param A The first point
*  @param B The second point
*  @param C The third point
*
*  @return The scalar product: (A.X - C.X) * (B.X - C.X) + (A.Y - C.Y) * (B.Y - C.Y)
*/
static double sprod(Point2D A, Point2D B, Point2D C)
{
	return (A.X - C.X) * (B.X - C.X) + (A.Y - C.Y) * (B.Y - C.Y);
}

static double sprod(Point2D A, Point2D B)
{
	return sprod( A, B, Point2D(0,0));
}

/**
*  @brief the vector product of two vectors.
*
*  Computes the vector product of two vectors.
*  The first vector is A - C, the second B - C,
*  where A, B and C are points.
*
*  @param A The first point
*  @param B The second point
*  @param C The third point
*
*  @return The vector product: (A.X - C.X) * (B.Y - C.Y) - (A.Y - C.Y) * (B.X - C.X)
*/
static double vprod(Point2D A, Point2D B, Point2D C)
{
	return (A.X - C.X) * (B.Y - C.Y) - (A.Y - C.Y) * (B.X - C.X);
}

static double vprod(Point2D A, Point2D B) {
	return vprod(A, B, Point2D(0, 0));
}



inline Point2D Point2D::dpx( double d)
{
	if (fabs(X) < EPSILONPOINT/1.0e10 || fabs(Y) < EPSILONPOINT / 1.0e10) {
//		return Point2D(X,Y) * 1/d * rounded(d);
		return Point2D(X, Y) * d;
	}
	else if (fabs(fabs(X) - fabs(Y)) < EPSILONPOINT / 1.0e10) {
		//  45 degree case: try to round d such that if p is on the grid it will be later
//		return Point2D(X, Y) * M_SQRT2 * rounded(d * M_SQRT1_2);
		return Point2D(X, Y) * d;
	}
	else {
		return Point2D(X, Y) * d;
	}
}

// Edge Class
class Edge{
public:
	Edge() { A = Point2D(0, 0); B = Point2D(0, 0); };
	Edge(Point2D C, Point2D D) { A = C; B = D; };
	double distance(Point2D P);
	double length() const;
	double direction() const;
	Point2D GetA() const;
	Point2D GetB() const;
	bool intersection_woborder(const Edge & E, Point2D * I);
	bool intersection(const Edge & E, Point2D * I);
private:
	Point2D A;
	Point2D B;
};

// Point 3D Class
class Point3D
{
public:
	double X;
	double Y;
	double Z;

	Point3D() {};
	Point3D(double X, double Y, double Z) { this->X = X; this->Y = Y; this->Z = Z; };
	bool operator==(Point3D P) const;
	bool operator!=(Point3D P) const;
	bool operator<(Point3D P) const;
};

inline bool Point3D::operator==(Point3D P) const {
	return floor(X* EPSILONPOINT + 0.5f) / EPSILONPOINT == floor(P.X* EPSILONPOINT + 0.5f) / EPSILONPOINT
		&& floor(Y* EPSILONPOINT + 0.5f) / EPSILONPOINT == floor(P.Y* EPSILONPOINT + 0.5f) / EPSILONPOINT
		&& floor(Z* EPSILONPOINT + 0.5f) / EPSILONPOINT == floor(P.Z* EPSILONPOINT + 0.5f) / EPSILONPOINT;
}

inline bool Point3D::operator!=(Point3D P) const {
	return floor(X* EPSILONPOINT + 0.5f) / EPSILONPOINT != floor(P.X* EPSILONPOINT + 0.5f) / EPSILONPOINT
		|| floor(Y* EPSILONPOINT + 0.5f) / EPSILONPOINT != floor(P.Y* EPSILONPOINT + 0.5f) / EPSILONPOINT
		|| floor(Z* EPSILONPOINT + 0.5f) / EPSILONPOINT != floor(P.Z* EPSILONPOINT + 0.5f) / EPSILONPOINT;
}

inline bool Point3D::operator<(Point3D P) const {
	return (X < P.X || ((X == P.X) && Y<P.Y) || ((X == P.X) && (Y == P.Y) && Z<P.Z));
}

typedef struct Transform{
	double X;
	double Y;
	double Z;
} Transform;

typedef struct ElementColour{
	double R;
	double G;
	double B;
	double F;
	int Metal;
} ElementColour;

typedef struct SRefElement {
    bool collapsed;
	double X;
	double Y;
	double Mag;
	char *Name;
	Transform Rotate;
	int Flipped;

	class GDSObject *object;
} SRefElement;

typedef struct ARefElement {
    bool collapsed;
	double X1;
	double Y1;
	double X2;
	double Y2;
	double X3;
	double Y3;
	float Mag;
	int Columns;
	int Rows;
	char *Name;
	Transform Rotate;
	int Flipped;

	class GDSObject *object;
} ARefElement;

#endif // __GDSELEMENTS_H__
