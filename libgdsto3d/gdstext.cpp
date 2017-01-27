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

