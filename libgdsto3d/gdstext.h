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

#ifndef __GDSTEXT_H__
#define __GDSTEXT_H__

#include "gdselements.h"
#include "process_cfg.h"

class GDSText
{
private:
	double			_X;
	double			_Y;
	double			_Z;
	double			_Mag;
	int			_VJust;
	int			_HJust;
	struct ProcessLayer	*_Layer;
	char			*_String;
	Transform		_Rotate;
	bool			_Flipped;

public:
	GDSText(double X, double Y, double Z, bool Flipped, double Mag, int VJust, int HJust, struct ProcessLayer *Layer);
	~GDSText();

	void SetString(const char *String);
	void SetRotation(double X, double Y, double Z);

	char *GetString();
	double GetX();
	double GetY();
	double GetZ();
	double GetRY();
	double GetMag();

	int GetVJust();
	int GetHJust();

	bool GetFlipped();

	struct ProcessLayer *GetLayer();
};

#endif // __GDSTEXT_H__
