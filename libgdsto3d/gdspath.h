//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2017 Bertrand Pigeard
//  Copyright (C) 2013 IC-Design Group, University of Twente.
//
//  Based on gds2pov by Roger Light, http://atchoo.org/gds2pov/ / https://github.com/ralight/gds2pov
//  Copyright (C) 2004-2008 by Roger Light
//
/*
 * File: gdspath.h
 * Author: Roger Light
 * Project: gdsto3d
 *
  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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

