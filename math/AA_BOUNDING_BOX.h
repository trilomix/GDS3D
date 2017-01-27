//////////////////////////////////////////////////////////////////////////////////////////
//	AA_BOUNDING_BOX.h
//	class declaration for axis aligned bounding box, derives from BOUNDING_VOLUME
//	Downloaded from: www.paulsprojects.net
//	Created:	13th August 2002
//	Modified:	21st November 2002	-	CHanged mins and maxes from float* to VECTOR3D
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#ifndef AA_BOUNDING_BOX_H
#define AA_BOUNDING_BOX_H


//planes of box
enum BOUNDING_BOX_PLANES
{
	BOUNDING_BOX_NEG_X_PLANE=0,
	BOUNDING_BOX_POS_X_PLANE,
	BOUNDING_BOX_NEG_Y_PLANE,
	BOUNDING_BOX_POS_Y_PLANE,
	BOUNDING_BOX_POS_Z_PLANE,
	BOUNDING_BOX_NEG_Z_PLANE
};

class AA_BOUNDING_BOX
{
public:
	void SetFromMinsMaxes(const VECTOR3D & newMins, const VECTOR3D & newMaxes);
	void SetFromPoints(int numpoints, VECTOR3D *points);
	
	virtual bool IsPointInside(const VECTOR3D & point) const;
	
	float DistFromPoint( const VECTOR3D & point);
	void Mult(const MATRIX4X4 & mat);

	void AddBounds(const AA_BOUNDING_BOX & bounds);

	VECTOR3D vertices[8];
	VECTOR3D mins, maxes;
};

inline 
void AA_BOUNDING_BOX::Mult(const MATRIX4X4 & mat)
{
	// Mult by matrix
	vertices[0] = mat * vertices[0];
	mins = maxes = vertices[0];
	for(int i=1;i<8;i++)
	{
		vertices[i] = mat * vertices[i];
		if(vertices[i].x > maxes.x) maxes.x = vertices[i].x;
		if(vertices[i].y > maxes.y) maxes.y = vertices[i].y;
		if(vertices[i].z > maxes.z) maxes.z = vertices[i].z;

		if(vertices[i].x < mins.x) mins.x = vertices[i].x;
		if(vertices[i].y < mins.y) mins.y = vertices[i].y;
		if(vertices[i].z < mins.z) mins.z = vertices[i].z;
	}
}

#endif	//AA_BOUNDING_BOX_H