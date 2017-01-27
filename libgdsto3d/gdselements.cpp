#include "gdselements.h"

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
