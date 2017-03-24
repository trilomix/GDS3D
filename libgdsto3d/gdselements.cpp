#include "gdselements.h"
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef NULL
#define NULL            0
#endif

double Point2D::double_distance(const Point2D &p) const
{
	double ddx(p.X - X);
	double ddy(p.Y - Y);
	return sqrt(ddx * ddx + ddy * ddy);
}

bool
Point2D::IsonLine(const Point2D& A, const Point2D& B)
{
	// Check if a point lies on a line segment


	// Inside bounding box?
	if (X < A.X && X < B.X)
		return false;
	if (X > A.X && X > B.X)
		return false;
	if (Y < A.Y && Y < B.Y)
		return false;
	if (Y > A.Y && Y > B.Y)
		return false;

	// On line?
	double a, b;

	if (B.X - A.X != 0.0f)
	{
		a = (B.Y - A.Y) / (B.X - A.X);
		b = A.Y - a * A.X;
		if (fabs(Y - (a*X + b)) < 1/EPSILONPOINT)
			return true;
	}
	else
	{
		a = (B.X - A.X) / (B.Y - A.Y);
		b = A.X - a * A.Y;
		if (fabs(X - (a*Y + b)) < 1 / EPSILONPOINT)
			return true;
	}

	return false;
}

double Edge::distance(Point2D P)
{

		if (A.X == B.X && A.Y == B.Y) return A.double_distance(P);
		if ((A.X == B.X)&& (P.Y<max(A.Y,B.Y)&& P.Y>min(A.Y, B.Y))) {
			return fabs(A.X - P.X);
		}

		double sx = B.X - A.X;
		double sy = B.Y - A.Y;

		double ux = P.X - A.X;
		double uy = P.Y - A.Y;

		double dp = sx*ux + sy*uy;
		if (dp<0) return A.double_distance(P);

		int sn2 = sx*sx + sy*sy;
		if (dp>sn2) return B.double_distance(P);

		double ah2 = dp*dp / sn2;
		double un2 = ux*ux + uy*uy;
		return sqrt(un2 - ah2); 
}

double Edge::length() const
{
	return A.double_distance(B);
}

double Edge::direction() const
{
	double angle = atan2((B.Y - A.Y) / length(), (B.X - A.X) / length());
	return angle;
		 
}

Point2D Edge::GetA() const
{
	return A;
}

Point2D Edge::GetB() const
{
	return B;
}

bool Edge::intersection_woborder(const Edge& E, Point2D *I)
{
	Point2D P;
	bool res = intersection(E, &P);
	if (res && (P == E.A || P == E.B || P == A || P == B)) {
		return false;
	}
	else {
		if (res && I != NULL) {
			I->X = P.X;
			I->Y = P.Y;
		}
		return res;
	}
}

bool Edge::intersection(const Edge& E, Point2D *I)
{
	Point2D P0, P1, P2, P3;
	P0 = A;
	P1 = B;
	P2 = E.GetA();
	P3 = E.GetB();

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
