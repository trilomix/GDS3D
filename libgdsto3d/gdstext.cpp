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
 * File: gdstext.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the GDSText class which is used to represent the GDS text object.
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

#include "gdsobject.h"
#include "gdstext.h"

GDSText::GDSText(double X, double Y, double Z, bool Flipped, double Mag, int VJust, int HJust, struct ProcessLayer *Layer)
{
	_String = NULL;

	_X = X;
	_Y = Y;
	_Z = Z;
	_Rotate.X = 0.0;
	_Rotate.Y = 0.0;
	_Rotate.Z = 0.0;
	_Flipped = Flipped;
	_Mag = Mag;
	_HJust = HJust;
	_VJust = VJust;
	_Layer = Layer;
}

GDSText::~GDSText()
{
	if(_String) delete [] _String;
}

void GDSText::SetString(const char *String)
{
	if(_String) delete [] _String;

	_String = new char[strlen(String)+1];
	strcpy(_String, String);
}

void GDSText::SetRotation(double X, double Y, double Z)
{
	_Rotate.X = X;
	_Rotate.Y = Y;
	_Rotate.Z = Z;
}

char *GDSText::GetString()
{
	return _String;
}

double GDSText::GetX()
{
	return _X;
}

double GDSText::GetY()
{
	return _Y;
}

double GDSText::GetZ()
{
	return _Z;
}

double GDSText::GetRY()
{
	return _Rotate.Y;
}

double GDSText::GetMag()
{
	return _Mag;
}

int GDSText::GetVJust()
{
	return _VJust;
}

int GDSText::GetHJust()
{
	return _HJust;
}

bool GDSText::GetFlipped()
{
	return _Flipped;
}

struct ProcessLayer *GDSText::GetLayer()
{
	return _Layer;
}

