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

#ifndef _PROCESS_CFG2_H
#define _PROCESS_CFG2_H

struct GDSUnits {
	double Unitu; /* GDS Unit in micron*/
	double UserUnit;
};

struct ProcessLayer{
	struct ProcessLayer *Next;
	char *Name;
	char *ProcessName;
	char *Virtual;
	char *Material;
	char *OutMaterial;
	int Layer;
	int Datatype;
	double Height;
	double Thickness;
	struct GDSUnits *Units; // GDS Unit
	bool Top;
	bool Bottom;
	int Show;
	float Red;
	float Green;
	float Blue;
	float Filter;
	int Metal;
	double MinSpace;
	int Index;
	int LegendIndex;
	bool Alt;
	bool Ctrl;
	bool Shift;
	int ShortKey;
	bool operator<(const ProcessLayer & n) const {
		return this->Height < n.Height;
	}
	bool operator()(const ProcessLayer & n) const {
		return this->Height < n.Height;
	}

};

typedef struct ProcessLayer layers;

class GDSProcess
{
private:
	struct ProcessLayer	*_FirstLayer;
	int _Count;		/* Number of layers found */
	char * _CurrentProcess;
	bool _Valid;		/* Is the process file valid? */

public:
	GDSProcess ();
	~GDSProcess ();

	void Parse(char *processfile);
	void ParseFile(char * processfile, int cur_layer, float offset, bool flip);
	//void ParseFile(char * processfile, int cur_layer);
	//bool Parse(char *processfile);

	void AddLayer(struct ProcessLayer *NewLayer);
	void AddLayer(int Layer, int Datatype);
	void ChangeVisibility(struct ProcessLayer *Layer, bool Show);
	void ChangeLegendIndex(struct ProcessLayer *Layer, int LegendIndex);
	bool LayerExist(ProcessLayer * NewLayer);
	void ChangeTopLayer(ProcessLayer * layer, ProcessLayer * NewLayer, bool flip);
	void ChangeBottomLayer(ProcessLayer * layer, ProcessLayer * NewLayer, bool flip);
	void AddLayer(ProcessLayer * NewLayer, bool flip);
	struct ProcessLayer *GetLayer(int Number, int Datatype);
	struct ProcessLayer *GetLayer(int Number, int Datatype, char * ProcessName);
	struct ProcessLayer *GetLayer(int Index);
	ProcessLayer * GetLayer(const char * Name);
	struct ProcessLayer *GetLayer();
	ProcessLayer * GetLayerProcess(const char * ProcessName);
	int LayerCount();
	char *GetCurrentProcess();
	void SetCurrentProcess(char * CurrentProcess);
	void SetUnits(double dbUnitm, double dbUnituu);
	bool IsValid();
	double GetHighest();
	double GetLowest();
	bool Save(const char *filename);
};

#endif // _PROCESS_CFG_H

