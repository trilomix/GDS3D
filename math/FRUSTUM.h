//////////////////////////////////////////////////////////////////////////////////////////
//	FRUSTUM.h
//	class declaration for frustum for frustum culling, derives from BOUNDING_VOLUME
//	Downloaded from: www.paulsprojects.net
//	Created:	13th August 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#ifndef FRUSTUM_H
#define FRUSTUM_H

//planes of frustum. Normals point inwards
enum FRUSTUM_PLANES
{
	FRUSTUM_LEFT_PLANE=0,
	FRUSTUM_RIGHT_PLANE,
	FRUSTUM_TOP_PLANE,
	FRUSTUM_BOTTOM_PLANE,
	FRUSTUM_NEAR_PLANE,
	FRUSTUM_FAR_PLANE
};

class FRUSTUM
{
public:
	void SetFromMatrices(const MATRIX4X4 & view, const MATRIX4X4 & projection);
	void SetFromMatrix(const MATRIX4X4 & projection);
	
	virtual bool IsPointInside(const VECTOR3D & point) const;
	virtual bool IsAABoundingBoxInside(const AA_BOUNDING_BOX & box) const;
	virtual int ClassifyBoundingBoxInside(const AA_BOUNDING_BOX & box) const;
	

	PLANE planes[6];
};

#endif	//FRUSTUM_H