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

#ifndef _ASSEMBLY_CFG2_H
#define _ASSEMBLY_CFG2_H

#include "gds_globals.h"

struct GDS {
	struct GDS *Next;
	char *Name;
	char * gdsfile;
	FILE * iptr;
	char *processfile;
	float X, Y, Z;
	float angle;
	int Flipped;
	bool FlipChip;
	int Index;
	int LegendIndex;
	bool Show;
	char *ProcessName;
	double MeshMaxElemSize;
};

class GDSAssembly
{
private:
	struct GDS	*_FirstGDS;
	bool _Valid;
	int _Count;
	char * Name;
protected:
	FILE *pptr;

public:
	GDSAssembly();
	~GDSAssembly();

	void PareseFileError(GDS NewGDS);

	void ParseFile(char * assemblyfile);

	GDS * GetGDS(int Index);

	GDS * GetGDS(const char * Name);

	GDS * GetGDS();

	int GDSCount();

	char * GetName();

	void ChangeVisibility(GDS * gds, bool Show);

	void ChangeLegendIndex(GDS * gds, int LegendIndex);

	void AddGDS(GDS * NewGDS);

	bool IsValid();

	bool Save(const char * filename);
	
};

#endif // _ASSEMBLY_CFG_H

