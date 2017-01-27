//////////////////////////////////////////////////////////////////////////////////////////
//	FRUSTUM.cpp
//	Functions for frustum
//	Downloaded from: www.paulsprojects.net
//	Created:	13th August 2002
//	Updated:	26th September 2002	-	Removed need to call GetFloatv
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#include "Maths.h"


void FRUSTUM::SetFromMatrices(const MATRIX4X4 & view, const MATRIX4X4 & projection)
{
	MATRIX4X4 clip;

	//Multiply the matrices
	clip=projection*view;

	//calculate planes
	planes[FRUSTUM_RIGHT_PLANE].normal.x=clip.entries[3]-clip.entries[0];
	planes[FRUSTUM_RIGHT_PLANE].normal.y=clip.entries[7]-clip.entries[4];
	planes[FRUSTUM_RIGHT_PLANE].normal.z=clip.entries[11]-clip.entries[8];
	planes[FRUSTUM_RIGHT_PLANE].intercept=clip.entries[15]-clip.entries[12];

	planes[FRUSTUM_LEFT_PLANE].normal.x=clip.entries[3]+clip.entries[0];
	planes[FRUSTUM_LEFT_PLANE].normal.y=clip.entries[7]+clip.entries[4];
	planes[FRUSTUM_LEFT_PLANE].normal.z=clip.entries[11]+clip.entries[8];
	planes[FRUSTUM_LEFT_PLANE].intercept=clip.entries[15]+clip.entries[12];

	planes[FRUSTUM_BOTTOM_PLANE].normal.x=clip.entries[3]+clip.entries[1];
	planes[FRUSTUM_BOTTOM_PLANE].normal.y=clip.entries[7]+clip.entries[5];
	planes[FRUSTUM_BOTTOM_PLANE].normal.z=clip.entries[11]+clip.entries[9];
	planes[FRUSTUM_BOTTOM_PLANE].intercept=clip.entries[15]+clip.entries[13];

	planes[FRUSTUM_TOP_PLANE].normal.x=clip.entries[3]-clip.entries[1];
	planes[FRUSTUM_TOP_PLANE].normal.y=clip.entries[7]-clip.entries[5];
	planes[FRUSTUM_TOP_PLANE].normal.z=clip.entries[11]-clip.entries[9];
	planes[FRUSTUM_TOP_PLANE].intercept=clip.entries[15]-clip.entries[13];

	planes[FRUSTUM_FAR_PLANE].normal.x=clip.entries[3]-clip.entries[2];
	planes[FRUSTUM_FAR_PLANE].normal.y=clip.entries[7]-clip.entries[6];
	planes[FRUSTUM_FAR_PLANE].normal.z=clip.entries[11]-clip.entries[10];
	planes[FRUSTUM_FAR_PLANE].intercept=clip.entries[15]-clip.entries[14];

	planes[FRUSTUM_NEAR_PLANE].normal.x=clip.entries[3]+clip.entries[2];
	planes[FRUSTUM_NEAR_PLANE].normal.y=clip.entries[7]+clip.entries[6];
	planes[FRUSTUM_NEAR_PLANE].normal.z=clip.entries[11]+clip.entries[10];
	planes[FRUSTUM_NEAR_PLANE].intercept=clip.entries[15]+clip.entries[14];

	//normalize planes
	for(int i=0; i<6; ++i)
		planes[i].Normalize(); // Very costly
}

void FRUSTUM::SetFromMatrix(const MATRIX4X4 & projection)
{
	//calculate planes
	planes[FRUSTUM_RIGHT_PLANE].normal.x=projection.entries[3]-projection.entries[0];
	planes[FRUSTUM_RIGHT_PLANE].normal.y=projection.entries[7]-projection.entries[4];
	planes[FRUSTUM_RIGHT_PLANE].normal.z=projection.entries[11]-projection.entries[8];
	planes[FRUSTUM_RIGHT_PLANE].intercept=projection.entries[15]-projection.entries[12];

	planes[FRUSTUM_LEFT_PLANE].normal.x=projection.entries[3]+projection.entries[0];
	planes[FRUSTUM_LEFT_PLANE].normal.y=projection.entries[7]+projection.entries[4];
	planes[FRUSTUM_LEFT_PLANE].normal.z=projection.entries[11]+projection.entries[8];
	planes[FRUSTUM_LEFT_PLANE].intercept=projection.entries[15]+projection.entries[12];

	planes[FRUSTUM_BOTTOM_PLANE].normal.x=projection.entries[3]+projection.entries[1];
	planes[FRUSTUM_BOTTOM_PLANE].normal.y=projection.entries[7]+projection.entries[5];
	planes[FRUSTUM_BOTTOM_PLANE].normal.z=projection.entries[11]+projection.entries[9];
	planes[FRUSTUM_BOTTOM_PLANE].intercept=projection.entries[15]+projection.entries[13];

	planes[FRUSTUM_TOP_PLANE].normal.x=projection.entries[3]-projection.entries[1];
	planes[FRUSTUM_TOP_PLANE].normal.y=projection.entries[7]-projection.entries[5];
	planes[FRUSTUM_TOP_PLANE].normal.z=projection.entries[11]-projection.entries[9];
	planes[FRUSTUM_TOP_PLANE].intercept=projection.entries[15]-projection.entries[13];

	planes[FRUSTUM_FAR_PLANE].normal.x=projection.entries[3]-projection.entries[2];
	planes[FRUSTUM_FAR_PLANE].normal.y=projection.entries[7]-projection.entries[6];
	planes[FRUSTUM_FAR_PLANE].normal.z=projection.entries[11]-projection.entries[10];
	planes[FRUSTUM_FAR_PLANE].intercept=projection.entries[15]-projection.entries[14];

	planes[FRUSTUM_NEAR_PLANE].normal.x=projection.entries[3]+projection.entries[2];
	planes[FRUSTUM_NEAR_PLANE].normal.y=projection.entries[7]+projection.entries[6];
	planes[FRUSTUM_NEAR_PLANE].normal.z=projection.entries[11]+projection.entries[10];
	planes[FRUSTUM_NEAR_PLANE].intercept=projection.entries[15]+projection.entries[14];

	//normalize planes
	for(int i=0; i<6; ++i)
		planes[i].Normalize(); // Very costly
}

//is a point in the frustum?
bool FRUSTUM::IsPointInside(const VECTOR3D & point) const
{
	for(int i=0; i<6; ++i)
	{
		if(planes[i].ClassifyPoint(point)==POINT_BEHIND_PLANE)
			return false;
	}

	return true;
}

//is a bounding box in the frustum?
bool FRUSTUM::IsAABoundingBoxInside(const AA_BOUNDING_BOX & box) const
{
	//loop through planes
	for(int i=0; i<6; ++i)
	{
		//if a point is not behind this plane, try next plane
		if(planes[i].ClassifyPoint(box.vertices[0])!=POINT_BEHIND_PLANE)
			continue;
		if(planes[i].ClassifyPoint(box.vertices[1])!=POINT_BEHIND_PLANE)
			continue;
		if(planes[i].ClassifyPoint(box.vertices[2])!=POINT_BEHIND_PLANE)
			continue;
		if(planes[i].ClassifyPoint(box.vertices[3])!=POINT_BEHIND_PLANE)
			continue;
		if(planes[i].ClassifyPoint(box.vertices[4])!=POINT_BEHIND_PLANE)
			continue;
		if(planes[i].ClassifyPoint(box.vertices[5])!=POINT_BEHIND_PLANE)
			continue;
		if(planes[i].ClassifyPoint(box.vertices[6])!=POINT_BEHIND_PLANE)
			continue;
		if(planes[i].ClassifyPoint(box.vertices[7])!=POINT_BEHIND_PLANE)
			continue;

		//All vertices of the box are behind this plane
		return false;

		/*Find the vertex of the box which is least likely to be behind this plane
		//if it is, then all the vertices are behind, so return false
		//Otherwise, try the next plane
		if(planes[i].ClassifyPoint(VECTOR3D((planes[i].normal.x>0) ? box.mins.x : box.maxes.x,
											(planes[i].normal.y>0) ? box.mins.y : box.maxes.y,
											(planes[i].normal.z>0) ? box.mins.z : box.maxes.z))
																		==POINT_BEHIND_PLANE)
		{
			return false;
		}*/
	}

	return true;
}

//is a bounding box in the frustum?
int FRUSTUM::ClassifyBoundingBoxInside(const AA_BOUNDING_BOX & box) const
{
	int count = 0; // 48 if full in view, 0 if not in view

	//loop through planes
	for(int i=0; i<6; ++i)
	{
		//if a point is not behind this plane, try next plane
		if(planes[i].ClassifyPoint(box.vertices[0])!=POINT_BEHIND_PLANE)
			count++;
		if(planes[i].ClassifyPoint(box.vertices[1])!=POINT_BEHIND_PLANE)
			count++;
		if(planes[i].ClassifyPoint(box.vertices[2])!=POINT_BEHIND_PLANE)
			count++;
		if(planes[i].ClassifyPoint(box.vertices[3])!=POINT_BEHIND_PLANE)
			count++;
		if(planes[i].ClassifyPoint(box.vertices[4])!=POINT_BEHIND_PLANE)
			count++;
		if(planes[i].ClassifyPoint(box.vertices[5])!=POINT_BEHIND_PLANE)
			count++;
		if(planes[i].ClassifyPoint(box.vertices[6])!=POINT_BEHIND_PLANE)
			count++;
		if(planes[i].ClassifyPoint(box.vertices[7])!=POINT_BEHIND_PLANE)
			count++;

		//All vertices of the box are behind this plane
		//return false;

		/*Find the vertex of the box which is least likely to be behind this plane
		//if it is, then all the vertices are behind, so return false
		//Otherwise, try the next plane
		if(planes[i].ClassifyPoint(VECTOR3D((planes[i].normal.x>0) ? box.mins.x : box.maxes.x,
											(planes[i].normal.y>0) ? box.mins.y : box.maxes.y,
											(planes[i].normal.z>0) ? box.mins.z : box.maxes.z))
																		==POINT_BEHIND_PLANE)
		{
			return false;
		}*/
	}

	return count;
}