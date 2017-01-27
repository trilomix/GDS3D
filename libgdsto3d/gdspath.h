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

#ifndef __GDSPATH_H__
#define __GDSPATH_H__

#include "gds_globals.h"
#include "gdselements.h"
#include "process_cfg.h"
#include "../math/Maths.h"

class GDSPath
{
private:
	int 			_Type;
	float			_Height;
	float			_Thickness;
	unsigned int	_Points;
	float			_Width;
	float			_BgnExtn;
	float			_EndExtn;
	vector<Point2D> _Coords;
	Transform		_Rotate;
	struct ProcessLayer	*_Layer;

public:
	GDSPath(int PathType, float Height, float Thickness, unsigned int Points, float Width, float BgnExtn, float EndExtn, struct ProcessLayer *layer);
	~GDSPath();

	void AddPoint(unsigned int Index, float X, float Y);
	void SetRotation(float X, float Y, float Z);

	float GetXCoords(unsigned int Index);
	float GetYCoords(unsigned int Index);
	unsigned int GetPoints();

	float GetHeight();
	float GetThickness();
	float GetWidth();
	float GetBgnExtn();
	float GetEndExtn();

	int GetType();
	struct ProcessLayer *GetLayer();
	vector<Point2D> create_shifted_points(float start_ext, float end_ext, int ncircle);
	vector<Point2D> create_shifted_points(float start, float end, float width, bool forward, size_t start_index, size_t end_index, int ncircle);
};

#endif // __GDSPATH_H__

