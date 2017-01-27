//////////////////////////////////////////////////////////////////////////////////////////
//	AA_BOUNDING_BOX.cpp
//	Functions for axis aligned bounding box. Derives from BOUNDING_VOLUME
//	Downloaded from: www.paulsprojects.net
//	Created:	13th August 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	
#include "Maths.h"

void AA_BOUNDING_BOX::SetFromMinsMaxes(const VECTOR3D & newMins, const VECTOR3D & newMaxes)
{
	//Save mins & maxes
	mins=newMins;
	maxes=newMaxes;

	//set the positions of the vertices
	vertices[0].Set(mins.x, mins.y, mins.z);
	vertices[1].Set(mins.x, mins.y, maxes.z);
	vertices[2].Set(mins.x, maxes.y, mins.z);
	vertices[3].Set(mins.x, maxes.y, maxes.z);
	vertices[4].Set(maxes.x, mins.y, mins.z);
	vertices[5].Set(maxes.x, mins.y, maxes.z);
	vertices[6].Set(maxes.x, maxes.y, mins.z);
	vertices[7].Set(maxes.x, maxes.y, maxes.z);
}

void AA_BOUNDING_BOX::SetFromPoints(int numpoints, VECTOR3D *points)
{
	VECTOR3D min, max;

	// Setup
	min = max = points[0];

	for(long i=1;i<numpoints;i++)
	{
		if(points[i].x > max.x) max.x = points[i].x;
		if(points[i].y > max.y) max.y = points[i].y;
		if(points[i].z > max.z) max.z = points[i].z;

		if(points[i].x < min.x) min.x = points[i].x;
		if(points[i].y < min.y) min.y = points[i].y;
		if(points[i].z < min.z) min.z = points[i].z;
	}

	// Make bbox
	SetFromMinsMaxes(min, max);
}



//is a point in the box
bool AA_BOUNDING_BOX::IsPointInside(const VECTOR3D & point) const
{
	if(point.x < mins.x)
		return false;
	if(point.y < mins.y)
		return false;
	if(point.z < mins.z)
		return false;

	if(point.x > maxes.x)
		return false;
	if(point.y > maxes.y)
		return false;
	if(point.z > maxes.z)
		return false;

	return true;
}


float AA_BOUNDING_BOX::DistFromPoint( const VECTOR3D & point){
	
	float X = (point.x < mins.x) ? mins.x : (point.x > maxes.x) ? maxes.x : point.x;
	float Y = (point.y < mins.y) ? mins.y : (point.y > maxes.y) ? maxes.y : point.y;
	float Z = (point.z < mins.z) ? mins.z : (point.z > maxes.z) ? maxes.z : point.z;
	
	VECTOR3D delta = point - VECTOR3D(X,Y,Z);
	
	return sqrt(delta.DotProduct(delta)); // Very costly!!
}

void AA_BOUNDING_BOX::AddBounds(const AA_BOUNDING_BOX & bounds)
{
	// Go through bounds
	for(long i=0;i<8;i++)
	{
		if(bounds.vertices[i].x > maxes.x) maxes.x = bounds.vertices[i].x;
		if(bounds.vertices[i].y > maxes.y) maxes.y = bounds.vertices[i].y;
		if(bounds.vertices[i].z > maxes.z) maxes.z = bounds.vertices[i].z;

		if(bounds.vertices[i].x < mins.x) mins.x = bounds.vertices[i].x;
		if(bounds.vertices[i].y < mins.y) mins.y = bounds.vertices[i].y;
		if(bounds.vertices[i].z < mins.z) mins.z = bounds.vertices[i].z;
	}

	// Make bbox
	SetFromMinsMaxes(mins, maxes);
}