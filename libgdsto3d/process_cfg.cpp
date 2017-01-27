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


	/*
	** ProcessFile structure
	**
	** # at the start of a line is a comment
	**
	** StartLayer: Name
	** Layer: Number
	** Datatype: Number
	** Height: Number
	** Thickness: Number
	** Colour: <0, 0.5, 1>
	** Metal: 1/0
	** Transparent: 1/0
	** Show: 1/0
	** EndLayer
	*/

#include "process_cfg.h"
#include "gds_globals.h"

GDSProcess::GDSProcess()
{
	_Count = 0;
	_Valid = true;

	_FirstLayer = NULL;
	_CurrentProcess = NULL;
}

GDSProcess::~GDSProcess ()
{
	if(_FirstLayer){
		struct ProcessLayer *layer1;
		struct ProcessLayer *layer2;

		layer1 = _FirstLayer;

		while(layer1->Next){
			layer2 = layer1->Next;
			if(layer1->Name){
				delete [] layer1->Name;
			}
			if (layer1->ProcessName) {
				delete[] layer1->ProcessName;
			}
			if(layer1){
				delete layer1;
			}
			layer1 = layer2;
		}
		if(layer1->Name){
			delete [] layer1->Name;
		}
		if (layer1->ProcessName) {
			delete[] layer1->ProcessName;
		}if(layer1){
			delete layer1;
		}
	}
	if (_CurrentProcess) {
		delete[] _CurrentProcess;
	}
}

//bool GDSProcess::Parse(char *processfile)
void GDSProcess::Parse(char *processfile)
{
	ParseFile(processfile, 0,0,false);
}

void GDSProcess::ParseFile(char *processfile, int cur_layer, float offset = 0, bool flip = false)
{
	int layerstart_cnt = cur_layer; // Count Start occur
	int layerend_cnt = cur_layer;   // count End occur
	char line[1024];
	char *CurrentProcess;

	int current_line = 0;
	int current_element = cur_layer -1;

	int sign = flip ? -1 : 1;

	/* State variables */
	bool in_layer = false;
	bool got_layer = false;
	bool got_datatype = false;
	bool got_virtual = false;
	bool got_height = false;
	bool got_thickness = false;
	bool got_material = false;
	bool got_outmaterial = false;
	bool got_red = false;
	bool got_green = false;
	bool got_blue = false;
	bool got_filter = false;
	bool got_metal = false;
	bool got_show = false; 
	bool got_shortkey = false;
	/* End State variables */
	bool showing;

	struct ProcessLayer NewLayer;
	NewLayer.Name = NULL;
	NewLayer.ProcessName = NULL;
	NewLayer.Units = NULL;

	FILE *pptr = NULL;

	pptr = fopen(processfile, "rt");
	
	if(!pptr){
		v_printf(1, "Unable to open process file \"%s\".\n", processfile);
		_Valid = false;
		return;
	}
	else
		v_printf(1, "Opened process file \"%s\"\n", processfile);

	CurrentProcess = remove_extension(getfilename(processfile));
	_CurrentProcess  = new char[strlen(CurrentProcess) + 1];
	strcpy(_CurrentProcess, CurrentProcess);

	while(!feof(pptr) && fgets(line, 1024, pptr)){
		if(line[0]!='#'){
			if(strstr(line, "LayerStart")){			
				layerstart_cnt++;
			}else if(strstr(line, "LayerEnd")){
				layerend_cnt++;
			}
		}
	}
	if(layerstart_cnt!=layerend_cnt){
		v_printf(1, "Invalid process file. ");
		v_printf(1, "There should be equal numbers of LayerStart and LayerEnd elements! ");
		v_printf(1, "(%d and %d found respectively)\n", layerstart_cnt, layerend_cnt);
		_Valid = false;
		fclose(pptr);
		return;
	}

	_Count = layerstart_cnt;

	fseek(pptr, 0, SEEK_SET);
	while(!feof(pptr) && fgets(line, 1024, pptr)){
		current_line++;
		if(line[0]!='#'){
			if (strstr(line, "#")) {
				size_t nbChar = strstr(line, "#") - line;
				strncpy(line, line, nbChar);
				line[nbChar] = '\0';
			}
			if(strstr(line, "LayerStart:")){
				if(in_layer){
					v_printf(1, "Error: LayerStart without LayerEnd not allowed. LayerEnd should appear before line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				in_layer = true;
				got_layer = false;
				got_datatype = false;
				got_virtual = false;
				got_material = false;
				got_outmaterial = false;
				got_height = false;
				got_thickness = false;
				got_red = false;
				got_green = false;
				got_blue = false;
				got_filter = false;
				got_metal = false;
				got_show = false;
				got_shortkey = false;
				current_element++;

				if(NewLayer.Name){
					delete [] NewLayer.Name;
					NewLayer.Name = NULL;
				}
				NewLayer.Name = new char[strlen(line)-strlen("LayerStart: ")+1];
//				strncpy(NewLayer.Name, line+strlen("LayerStart: "), strlen(line)-strlen("LayerStart: "));
				strcpy(NewLayer.Name, line+strlen("LayerStart: "));
				strCleanUp(NewLayer.Name);

				if (NewLayer.ProcessName) {
					delete[] NewLayer.ProcessName;
					NewLayer.ProcessName = NULL;
				}
				NewLayer.ProcessName = new char[strlen(_CurrentProcess) + 1];
				strcpy(NewLayer.ProcessName, _CurrentProcess);
				NewLayer.Virtual = NULL;
				NewLayer.Material = NULL;
				NewLayer.OutMaterial = NULL;
				NewLayer.Datatype = -1;
				NewLayer.Height = 0.0;
				NewLayer.Thickness = 0.0;
				NewLayer.Top = false;
				NewLayer.Bottom = false;
				NewLayer.Red = 0.0;
				NewLayer.Green = 0.0;
				NewLayer.Blue = 0.0;
				NewLayer.Filter = 0.0;
				NewLayer.Metal = 0;
				NewLayer.Show = false;
				NewLayer.Alt = false;
				NewLayer.Ctrl = false;
				NewLayer.Shift = false;
				NewLayer.ShortKey = -1;
				NewLayer.Next = NULL;
				NewLayer.LegendIndex = -1;
				NewLayer.Index = current_element;
			}else if(strstr(line, "Layer:")){
				if(!in_layer){
					v_printf(1, "Error: Layer definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_layer){
					v_printf(1, "Warning: Duplicate Layer definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Layer: %d", &NewLayer.Layer);
					got_layer = true;
				}
			}else if(strstr(line, "Datatype:")){
				if(!in_layer){
					v_printf(1, "Error: Datatype definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_datatype){
					v_printf(1, "Warning: Duplicate Datatype definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Datatype: %d", &NewLayer.Datatype);
					got_datatype = true;
				}
			}
			else if (strstr(line, "Virtual:")) {
				if (!in_layer) {
					v_printf(1, "Error: Virtual definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if (NewLayer.Virtual) {
						delete[] NewLayer.Virtual;
						NewLayer.Virtual = NULL;
					}
					fclose(pptr);
					return;
				}
				if (got_virtual) {
					v_printf(1, "Warning: Duplicate Virtual definition on line %d of process file. Ignoring new definition.\n", current_line);
				}
				else {
					NewLayer.Virtual = new char[strlen(line) - strlen("Virtual: ") + 1];
					strcpy(NewLayer.Virtual, line + strlen("Virtual: "));
					strCleanUp(NewLayer.Virtual);
					got_virtual = true;
				}
			}
			else if (strstr(line, "Material:") && strncmp(line, "Material:", strlen("Material:")) == 0) {
				if (!in_layer) {
					v_printf(1, "Error: Material definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if (NewLayer.Material) {
						delete[] NewLayer.Material;
						NewLayer.Material = NULL;
					}
					fclose(pptr);
					return;
				}
				if (got_material) {
					v_printf(1, "Warning: Duplicate Material definition on line %d of process file. Ignoring new definition.\n", current_line);
				}
				else {
					NewLayer.Material = new char[strlen(line) - strlen("Material: ") + 1];
					strcpy(NewLayer.Material, line + strlen("Material: "));
					strCleanUp(NewLayer.Material);
					got_material = true;
				}
			}
			else if (strstr(line, "OutMaterial:")) {
				if (!in_layer) {
					v_printf(1, "Error: OutMaterial definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if (NewLayer.OutMaterial) {
						delete[] NewLayer.OutMaterial;
						NewLayer.OutMaterial = NULL;
					}
					fclose(pptr);
					return;
				}
				if (got_outmaterial) {
					v_printf(1, "Warning: Duplicate OutMaterial definition on line %d of process file. Ignoring new definition.\n", current_line);
				}
				else {
					NewLayer.OutMaterial = new char[strlen(line) - strlen("OutMaterial: ") + 1];
					strcpy(NewLayer.OutMaterial, line + strlen("OutMaterial: "));
					strCleanUp(NewLayer.OutMaterial);
					got_outmaterial = true;
				}
			}
			else if(strstr(line, "Height:")){
				if(!in_layer){
					v_printf(1, "Error: Height definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_height){
					v_printf(1, "Warning: Duplicate Height definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					NewLayer.Height = GetLineValue(line, (char*)"Height: ");
					NewLayer.Height = NewLayer.Height*sign + offset;
					got_height = true;
					if (got_thickness && flip) {
						NewLayer.Height = NewLayer.Height - NewLayer.Thickness;
					}
				}
			}else if(strstr(line, "Thickness:")){
				if(!in_layer){
					v_printf(1, "Error: Thickness definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_thickness){
					v_printf(1, "Warning: Duplicate Thickness definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					NewLayer.Thickness = GetLineValue(line, (char*)"Thickness: ");
					got_thickness = true;
					if (got_height) {
						if (flip) {
							NewLayer.Height = NewLayer.Height- NewLayer.Thickness;
						}
					}
				}
			}else if(strstr(line, "Red:")){
				if(!in_layer){
					v_printf(1, "Error: Red definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_red){
					v_printf(1, "Warning: Duplicate Red definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Red: %f", &NewLayer.Red);
					got_red = true;
				}
			}else if(strstr(line, "Green:")){
				if(!in_layer){
					v_printf(1, "Error: Green definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_green){
					v_printf(1, "Warning: Duplicate Green definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Green: %f", &NewLayer.Green);
					got_green = true;
				}
			}else if(strstr(line, "Blue:")){
				if(!in_layer){
					v_printf(1, "Error: Blue definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_blue){
					v_printf(1, "Warning: Duplicate Blue definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Blue: %f", &NewLayer.Blue);
					got_blue = true;
				}
			}else if(strstr(line, "Filter:")){
				if(!in_layer){
					v_printf(1, "Error: Filter definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_filter){
					v_printf(1, "Warning: Duplicate Filter definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Filter: %f", &NewLayer.Filter);
					got_filter = true;
				}
			}else if(strstr(line, "Metal:")){
				if(!in_layer){
					v_printf(1, "Error: Metal definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_metal){
					v_printf(1, "Warning: Duplicate Metal definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Metal: %d", &NewLayer.Metal);
					got_metal = true;
				}
			}else if(strstr(line, "Show:")){
				if(!in_layer){
					v_printf(1, "Error: Show definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_show){
					v_printf(1, "Warning: Duplicate Show definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Show: %d", &NewLayer.Show);
					got_show = true;
				}
			}else if(strstr(line, "Shortkey:")){
				if(!in_layer){
					v_printf(1, "Error: Shortkey definition outside of LayerStart and LayerEnd on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}
				if(got_shortkey){
					v_printf(1, "Warning: Duplicate Shortkey definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "%*[^0-9]%d", &NewLayer.ShortKey);
					
					if (NewLayer.ShortKey < 9) {
						NewLayer.Alt = strstr(line, "<Alt>") != NULL;
						NewLayer.Ctrl = strstr(line, "<Ctrl>") != NULL;
						NewLayer.Shift = strstr(line, "<Shift>") != NULL;
					}
					else {
						v_printf(1, "Warning: Shortkey is larger than 9, ignoring.\n", current_line);
						NewLayer.ShortKey = -1;
					}

					got_shortkey = true;
				}
			}else if(strstr(line, "LayerEnd")){
				showing = (NewLayer.Show == 1);
				if(!in_layer){
					v_printf(1, "Error: LayerEnd without LayerStart on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}else if(!got_layer){
					v_printf(1, "Error: LayerEnd without Layer on line %d of process file.\n", current_line);
					_Valid = false;
					if(NewLayer.Name){
						delete [] NewLayer.Name;
						NewLayer.Name = NULL;
					}
					fclose(pptr);
					return;
				}else if(!got_height){
					v_printf(1, "Error: LayerEnd without Height on line %d of process file.\n", current_line);
					showing = false;
				}else if(!got_thickness){
					v_printf(1, "Error: LayerEnd without Thickness on line %d of process file.\n", current_line);
					showing = false;
				} else if (!got_material && got_outmaterial) {
					v_printf(1, "Error: LayerEnd with OutMaterial but without Material on line %d of process file.\n", current_line);
					NewLayer.OutMaterial = NULL;
				} else if (got_material && !got_outmaterial) {
					v_printf(1, "Error: LayerEnd with Material but without OutMaterial on line %d of process file.\n", current_line);
					NewLayer.Material = NULL;
				}
				AddLayer(&NewLayer,flip);
				
					if(!showing){
						if(NewLayer.Datatype == -1){
							v_printf(2, "Notice: Not showing layer %d (all datatypes)\n", NewLayer.Layer);
						}else{
							v_printf(2, "Notice: Not showing layer %d datatype %d\n", NewLayer.Layer, NewLayer.Datatype);
						}
					}

				if(NewLayer.Name){
					delete [] NewLayer.Name;
					NewLayer.Name = NULL;
				}
				in_layer = false;
			}
		}
	}
	if(NewLayer.Name){
		delete [] NewLayer.Name;
		NewLayer.Name = NULL;
	}
	fclose(pptr);
	v_printf(1, "\n");
}

struct ProcessLayer *GDSProcess::GetLayer(int Number, int Datatype)
{
	return GetLayer(Number, Datatype, _CurrentProcess);
}
struct ProcessLayer *GDSProcess::GetLayer(int Number, int Datatype, char *ProcessName)
{
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	if(Number == -1) return NULL;

	while(layer){
		if(layer->Layer == Number && layer->Datatype == -1){
			if (!ProcessName) {
				return layer;
			} else {
				if (strcmp(layer->ProcessName,ProcessName) == 0) {
			return layer;
				}
			}
		}else if(layer->Layer == Number && layer->Datatype == Datatype){
			if (!ProcessName) {
			return layer;
		}
			else {
				if (strcmp(layer->ProcessName, ProcessName) == 0) {
					return layer;
				}
			}
		}
		layer = layer->Next;
	}
	return NULL;
}

struct ProcessLayer *GDSProcess::GetLayer(int Index)
{
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	while(layer){
		if(layer->Index == Index){
			return layer;
		}
		layer = layer->Next;
	}
	return NULL;
}

struct ProcessLayer *GDSProcess::GetLayer(const char *Name)
{
	
	if (atoi(Name) > 0)
		return GetLayer(atoi(Name),-1);
	
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	while(layer){
		if(strcmp(Name, layer->Name) == 0){
			return layer;
		}
		layer = layer->Next;
	}
	return NULL;
}

struct ProcessLayer *GDSProcess::GetLayer()
{
	return _FirstLayer;
}

struct ProcessLayer *GDSProcess::GetLayerProcess(const char *ProcessName)
{
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	while (layer) {
		if (strcmp(layer->ProcessName, ProcessName) == 0) {
			return layer;
		}
		layer = layer->Next;
	}
	return NULL;
}

int GDSProcess::LayerCount()
{
	return _Count;
}

char * GDSProcess::GetCurrentProcess()
{
	return _CurrentProcess;
}

void GDSProcess::SetCurrentProcess(char * CurrentProcess)
{
	if (_CurrentProcess) {
		delete[] _CurrentProcess;
		_CurrentProcess = NULL;
	}
	_CurrentProcess = new char[strlen(CurrentProcess) + 1];
	strcpy(_CurrentProcess, CurrentProcess);
}

void GDSProcess::SetUnits(double dbUnitu, double dbUnituu) {
	struct ProcessLayer *layer;
	layer = GetLayerProcess(_CurrentProcess);
	layer->Units->Unitu = dbUnitu; // db Unit in micron
	layer->Units->UserUnit = dbUnituu;
}

void GDSProcess::AddLayer(int Layer, int Datatype)
{
	struct ProcessLayer NewLayer;

	if(Datatype<-1)
		Datatype = -1; // Check for negative datatype (-1 is okay though)

	NewLayer.Name = NULL;
	NewLayer.ProcessName = NULL;
	NewLayer.Virtual = NULL;
	NewLayer.Material = NULL;
	NewLayer.OutMaterial = NULL;
	NewLayer.Layer = Layer;
	NewLayer.Datatype = Datatype;
	NewLayer.Height = 0.0;
	NewLayer.Thickness = 0.0;
	NewLayer.Red = 0.0;
	NewLayer.Green = 0.0;
	NewLayer.Blue = 0.0;
	NewLayer.Filter = 0.0;
	NewLayer.Metal = 0;
	NewLayer.Show = false;
	NewLayer.Alt = false;
	NewLayer.Ctrl = false;
	NewLayer.Shift = false;
	NewLayer.ShortKey = -1;
	NewLayer.Next = NULL;
	NewLayer.Index = -1;
	NewLayer.LegendIndex = -1;

	AddLayer(&NewLayer);
}

void GDSProcess::ChangeVisibility(struct ProcessLayer *Layer, bool Show)
{
	Layer->Show = Show;
}

void GDSProcess::ChangeLegendIndex(struct ProcessLayer *Layer, int LegendIndex)
{
	Layer->LegendIndex = LegendIndex;
}

bool GDSProcess::LayerExist(struct ProcessLayer *NewLayer) 
{
	bool ans = true;
	if (NewLayer->Name) {
		if (!GDSProcess::GetLayer(NewLayer->Name)) {
			ans = false;
		} else {
			v_printf(1, "Notice: Duplicate layer Name %s\n", NewLayer->Name);
		}
	}
	if (!GDSProcess::GetLayer(NewLayer->Layer, NewLayer->Datatype)) {
		ans = false;
	} else {
		v_printf(1, "Notice: Duplicate layer %d datatype %d\n", NewLayer->Layer, NewLayer->Datatype);
	}
	return ans;
}
void GDSProcess::ChangeTopLayer(struct ProcessLayer *layer, struct ProcessLayer *NewLayer, bool flip) {
	v_printf(3, "Process %s ... Current Layer : %s \n", layer->ProcessName, layer->Name);
	// Change the Top Layer
	if (layer->Top && strcmp(NewLayer->ProcessName, layer->ProcessName) == 0) {
		v_printf(3, "Process %s ... Current Top Layer : %s\n", layer->ProcessName, layer->Name);
		if (flip) {
			if (layer->Height > NewLayer->Height) {
				//Change Top layer for the process
				NewLayer->Top = true;
				layer->Top = false;
				v_printf(3, "Process %s ... New Top Layer : %s (flip)\n", NewLayer->ProcessName, NewLayer->Name);
			}
			else if (layer->Height == NewLayer->Height){
				NewLayer->Top = true;
			}
		}
		else {
			if (layer->Height + layer->Thickness < NewLayer->Height + NewLayer->Thickness) {
				//Change Top layer for the process
				NewLayer->Top = true;
				layer->Top = false;
				v_printf(3, "Process %s ... New Top Layer : %s\n", NewLayer->ProcessName, NewLayer->Name);
			} else if (layer->Height + layer->Thickness == NewLayer->Height + NewLayer->Thickness) {
				NewLayer->Top = true;
			}
		}
	}
}
void GDSProcess::ChangeBottomLayer(struct ProcessLayer *layer, struct ProcessLayer *NewLayer, bool flip) {
	// Change the Bottom Layer
	if (layer->Bottom && strcmp(NewLayer->ProcessName, layer->ProcessName) == 0) {
		v_printf(3, "Process %s ... Current Bottom Layer : %s\n", layer->ProcessName, layer->Name);
		if (flip) {
			if (layer->Height + layer->Thickness < NewLayer->Height + NewLayer->Thickness || layer->Layer == 255) {
				//Change Bottom layer for the process
				NewLayer->Bottom = true;
				layer->Bottom = false;
				v_printf(3, "Process %s ... New Bottom Layer : %s (flip)\n", NewLayer->ProcessName, NewLayer->Name);
			}
			else if (layer->Height + layer->Thickness == NewLayer->Height + NewLayer->Thickness) {
				NewLayer->Bottom = true;
			}
		}
		else {
			if (layer->Height > NewLayer->Height || layer->Layer == 255) {
				//Change Bottom layer for the process
				NewLayer->Bottom = true;
				layer->Bottom = false;
				v_printf(3, "Process %s ... New Bottom Layer : %s\n", NewLayer->ProcessName, NewLayer->Name);
			}
			else if (layer->Height == NewLayer->Height) {
				NewLayer->Bottom = true;
			}
		}
	}
}
void GDSProcess::AddLayer(struct ProcessLayer *NewLayer) {
	AddLayer(NewLayer, false);
}
void GDSProcess::AddLayer(struct ProcessLayer *NewLayer, bool flip)
{
	struct ProcessLayer *layer;

	layer = _FirstLayer;

	if(_FirstLayer){
		if (LayerExist(NewLayer)) {
			return;
		}
		ChangeTopLayer(layer, NewLayer, flip);
		ChangeBottomLayer(layer, NewLayer, flip);
		// Find the last Layer
		while(layer->Next){
			layer = layer->Next;
			ChangeTopLayer(layer, NewLayer, flip);
			ChangeBottomLayer(layer, NewLayer, flip);
		}
		layer->Next = new struct ProcessLayer;
		if (strcmp(NewLayer->ProcessName, layer->ProcessName) != 0) {
			// Start New Process
			NewLayer->Top = true;
			NewLayer->Bottom = true;
			NewLayer->Units = new struct GDSUnits;
			v_printf(3, "Process %s ... New Top Layer : %s\n", NewLayer->ProcessName, NewLayer->Name);
		}
		layer = layer->Next;
		layer->Next = NULL;
	}else{
		_FirstLayer = new struct ProcessLayer;
		layer = _FirstLayer;
		// Set as Top Layer;
		NewLayer->Top = true;
		NewLayer->Bottom = true;
		NewLayer->Units = new struct GDSUnits;
		layer->Next = NULL;
		v_printf(3, "Process %s ... New Top Layer : %s\n", NewLayer->ProcessName, NewLayer->Name);
	}

	// Copy NewLayer
	layer->Name = NULL;
	if(NewLayer->Name){
		layer->Name = new char[strlen(NewLayer->Name)+1];
		strcpy(layer->Name, NewLayer->Name);
	}
	layer->ProcessName = NULL;
	if (NewLayer->ProcessName) {
		layer->ProcessName = new char[strlen(NewLayer->ProcessName) + 1];
		strcpy(layer->ProcessName, NewLayer->ProcessName);
	}
	layer->Layer = NewLayer->Layer;
	layer->Datatype = NewLayer->Datatype;
	layer->Virtual = NULL;
	if (NewLayer->Virtual) {
		layer->Virtual = new char[strlen(NewLayer->Virtual) + 1];
		strcpy(layer->Virtual, NewLayer->Virtual);
	}
	layer->Material = NULL;
	if (NewLayer->Material) {
		layer->Material = new char[strlen(NewLayer->Material) + 1];
		strcpy(layer->Material, NewLayer->Material);
	}
	layer->OutMaterial = NULL;
	if (NewLayer->OutMaterial) {
		layer->OutMaterial = new char[strlen(NewLayer->OutMaterial) + 1];
		strcpy(layer->OutMaterial, NewLayer->OutMaterial);
	}
	layer->Height = NewLayer->Height;
	layer->Thickness = NewLayer->Thickness;
	layer->Units = NewLayer->Units;
	layer->Top = NewLayer->Top;
	layer->Bottom = NewLayer->Bottom;
	layer->Show = NewLayer->Show;
	layer->Red = NewLayer->Red;
	layer->Green = NewLayer->Green;
	layer->Blue = NewLayer->Blue;
	layer->Filter = NewLayer->Filter;
	layer->Metal = NewLayer->Metal;
	layer->Index = NewLayer->Index;
	layer->Alt = NewLayer->Alt;
	layer->Ctrl = NewLayer->Ctrl;
	layer->Shift = NewLayer->Shift;
	layer->ShortKey = NewLayer->ShortKey;
	layer->LegendIndex = NewLayer->LegendIndex;
}

bool GDSProcess::IsValid()
{
	return _Valid;
}

double GDSProcess::GetHighest()
{
	double Highest = -10000.0;
	struct ProcessLayer *layer;

	layer = _FirstLayer;
	while(layer){
		if(layer->Height + layer->Thickness > Highest && layer->Show){
			Highest = layer->Height + layer->Thickness;
		}
		layer = layer->Next;
	}
	return Highest;
}

double GDSProcess::GetLowest()
{
	double Lowest = 10000.0;
	struct ProcessLayer *layer;

	layer = _FirstLayer;
	while(layer){
		if(layer->Height < Lowest && layer->Show){
			Lowest = layer->Height;
		}
		layer = layer->Next;
	}
	return Lowest;
}


bool GDSProcess::Save(const char *filename)
{
	struct ProcessLayer *layer;
	FILE *fptr = NULL;

	if(!filename) return false;

	fptr = fopen(filename, "wt");
	if(!fptr) return false;

	layer = _FirstLayer;
	while(layer){
		if (layer->Name) {
			fprintf(fptr, "LayerStart: %s\n", layer->Name);
		} else {
		fprintf(fptr, "LayerStart: LAYER-%d-%d\n", layer->Layer, layer->Datatype);
		}
		fprintf(fptr, "Layer: %d\n", layer->Layer);
		fprintf(fptr, "Datatype: %d\n", layer->Datatype);
		fprintf(fptr, "Height: 0\n");
		fprintf(fptr, "Thickness: 0\n");
		fprintf(fptr, "Red: 0.0\n");
		fprintf(fptr, "Green: 0.0\n");
		fprintf(fptr, "Blue: 0.0\n");
		fprintf(fptr, "Filter: 0.0\n");
		fprintf(fptr, "Metal: 0\n");
		fprintf(fptr, "Show: 1\n");
		fprintf(fptr, "LayerEnd\n\n");

		layer = layer->Next;
	}
	fclose(fptr);

	return true;
}

