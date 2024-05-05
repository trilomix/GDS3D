//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2017 Bertrand Pigeard
//
//  Based on gds2pov by Roger Light, http://atchoo.org/gds2pov/ / https://github.com/ralight/gds2pov
//  Copyright (C) 2004-2008 by Roger Light
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


	/*
	** AssemblyFile structure
	**
	** # at the start of a line is a comment
	**
	** StartGDS: Name
	** GDS_File: <Path to gds file>
	** Process_File: <Path to process file>
	** X: Value
	** Y: Value
	** Z: Value
	** Angle: <-180,180>
	** Flipped: 1/0
	** FlipChip: 1/0
	** EndGDS
	*/

#include "assembly_cfg.h"
#include "gds_globals.h"
#include "../math/Maths.h"

GDSAssembly::GDSAssembly()
{
	_Count = 0;
	_Valid = true;
	_FirstGDS = NULL;
}


GDSAssembly::~GDSAssembly ()
{
	if (_FirstGDS) {
		struct GDS *gds1;
		struct GDS *gds2;

		gds1 = _FirstGDS;

		while (gds1->Next) {
			gds2 = gds1->Next;
			if (gds1->Name) {
				delete[] gds1->Name;
			}
			if (gds1->gdsfile) {
				delete[] gds1->gdsfile;
			}
			if (gds1->processfile) {
				delete[] gds1->processfile;
			}
			if (gds1) {
				delete gds1;
			}
			gds1 = gds2;
		}
		if (gds1->Name) {
			delete[] gds1->Name;
		}
		if (gds1->gdsfile) {
			delete[] gds1->gdsfile;
		}
		if (gds1->processfile) {
			delete[] gds1->processfile;
		}
		if (gds1) {
			delete gds1;
		}
	}

}
void GDSAssembly::PareseFileError(struct GDS NewGDS) {
	_Valid = false;
	if (NewGDS.Name) {
		delete[] NewGDS.Name;
		NewGDS.Name = NULL;
	}
	fclose(pptr);
}

void GDSAssembly::ParseFile(char *assemblyfile)
{
	int GDSstart_cnt = 0;
	int GDSend_cnt = 0;
	char line[2048];

	int current_line = 0;
	int current_element = -1;

	char *inner_assemblyfile;
	GDSAssembly *Assembly;

	/* State variables */
	bool in_GDS = false;
	bool got_assemblyfile = false;
	bool got_gdsfile = false;
	bool got_processfile = false;
	bool got_X = false;
	bool got_Y = false;
	bool got_Z = false;
	bool got_angle = false;
	bool got_Flipped = false;
	bool got_FlipChip = false;
	bool got_MeshSize = false;
	/* End State variables */

	struct GDS NewGDS;
	NewGDS.Name = NULL;
	NewGDS.gdsfile = NULL;
	NewGDS.processfile = NULL;

	//FILE *pptr = NULL;

	pptr = fopen(assemblyfile, "rt");
	
	if(!pptr){
		v_printf(1, "Unable to open assembly file \"%s\".\n", assemblyfile);
		_Valid = false;
		return;
	}
	else
		v_printf(1, "Opened assembly file \"%s\"\n", assemblyfile);

	Name = remove_extension(getfilename(assemblyfile));

	while(!feof(pptr) && fgets(line, 1024, pptr)){
		if(line[0]!='#'){
			if(strstr(line, "GDS_Start")){			
				GDSstart_cnt++;
			}else if(strstr(line, "GDS_End")){
				GDSend_cnt++;
			}
		}
	}
	if(GDSstart_cnt != GDSend_cnt){
		v_printf(1, "Invalid assembly file. ");
		v_printf(1, "There should be equal numbers of GDS_Start and GDS_End elements! ");
		v_printf(1, "(%d and %d found respectively)\n", GDSstart_cnt, GDSend_cnt);
		_Valid = false;
		fclose(pptr);
		return;
	}

	_Count = GDSstart_cnt;
	if(_Count == 0)
	{
		v_printf(1, "Invalid assembly file. ");
		v_printf(1, "No GDS_Start element! ");
		_Valid = false;
	}


	fseek(pptr, 0, SEEK_SET);
	while(!feof(pptr) && fgets(line, 2048, pptr)){
		current_line++;
		if(line[0]!='#'){
			if (strstr(line, "#")) {
				size_t nbChar = strstr(line, "#") - line;
				strncpy(line, line, nbChar);
				line[nbChar] = '\0';
			}
			if(strstr(line, "GDS_Start:")){
				if(in_GDS){
					v_printf(1, "Error: GDS_Start without GDS_End not allowed. GDS_End should appear before line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				in_GDS = true;
				got_assemblyfile = false;
				got_gdsfile = false;
				got_processfile = false;
				got_X = false;
				got_Y = false;
				got_Z = false;
				got_angle = false;
				got_Flipped = false;
				got_FlipChip = false;
				got_MeshSize = false;
				current_element++;

				if(NewGDS.Name){
					delete [] NewGDS.Name;
					NewGDS.Name = NULL;
				}
				NewGDS.Name = new char[strlen(line)-strlen("GDS_Start: ")+1];
				strcpy(NewGDS.Name, line+strlen("GDS_Start: "));

				strCleanUp(NewGDS.Name);
				NewGDS.gdsfile = NULL;
				NewGDS.processfile = NULL;
				NewGDS.X = 0.0;
				NewGDS.Y = 0.0;
				NewGDS.Z = 0.0;
				NewGDS.angle = 0.0;
				NewGDS.Flipped = 0;
				NewGDS.FlipChip = false;
				NewGDS.MeshMaxElemSize = -1;
				NewGDS.Next = NULL;
				NewGDS.LegendIndex = -1;
				NewGDS.Index = current_element;
			}
			else if (strstr(line, "Assembly_File:")) {
				if (!in_GDS) {
					v_printf(1, "Error: Assembly_File definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if (got_assemblyfile) {
					v_printf(1, "Warning: Duplicate Assembly_File definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}
				else {
					inner_assemblyfile = new char[strlen(line) - strlen("Assembly_File: ") + 1];
					strcpy(inner_assemblyfile, line + strlen("Assembly_File: "));
					strCleanUp(inner_assemblyfile);
					Assembly = new GDSAssembly();
					Assembly->ParseFile(inner_assemblyfile);
					
					if (!Assembly) {
						v_printf(-1, "Error: Out of memory.\n");
						PareseFileError(NewGDS);
						delete[] inner_assemblyfile;
						return;
					}
					else if (!Assembly->IsValid()) {
						v_printf(-1, "Error: %s is not a valid assembly file\n", inner_assemblyfile);
						PareseFileError(NewGDS);
						delete[] inner_assemblyfile;
						return;
					}
                    delete[] inner_assemblyfile;
					got_assemblyfile = true;
				}
			}else if(strstr(line, "GDS_File:")){
				if(!in_GDS){
					v_printf(1, "Error: GDS_File definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if(got_gdsfile){
					v_printf(1, "Warning: Duplicate GDS_File definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}else{
					NewGDS.gdsfile = new char[strlen(line) - strlen("GDS_File: ") + 1];
					strcpy(NewGDS.gdsfile, line + strlen("GDS_File: "));
					strCleanUp(NewGDS.gdsfile);
					got_gdsfile = true;
				}
			}else if(strstr(line, "Process_File:")){
				if(!in_GDS){
					v_printf(1, "Error: Process_File definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					fclose(pptr);
					return;
				}
				if(got_processfile){
					v_printf(1, "Warning: Duplicate Datatype definition on line %d of process file. Ignoring new definition.\n", current_line);
				}else{
					NewGDS.processfile = new char[strlen(line) - strlen("Process_File: ") + 1];
					strcpy(NewGDS.processfile, line + strlen("Process_File: "));
					strCleanUp(NewGDS.processfile);
					got_processfile = true;
				}
			}
			else if (strstr(line, "X:")) {
				if (!in_GDS) {
					v_printf(1, "Error: X definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if (got_X) {
					v_printf(1, "Warning: Duplicate X definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}
				else {
					NewGDS.X = GetLineValue(line, "X: ");
					got_X = true;
				}
			}
			else if(strstr(line, "Y:")){
				if(!in_GDS){
					v_printf(1, "Error: Y definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if(got_Y){
					v_printf(1, "Warning: Duplicate Y definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}else{
					NewGDS.Y = GetLineValue(line, "Y: ");
					got_Y = true;
				}
			}else if(strstr(line, "Z:")){
				if(!in_GDS){
					v_printf(1, "Error: Z definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if(got_Z){
					v_printf(1, "Warning: Duplicate Z definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}else{
					NewGDS.Z = GetLineValue(line, "Z: ");
					got_Z = true;
				}
			}else if(strstr(line, "Angle:")){
				if(!in_GDS){
					v_printf(1, "Error: Angle definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if(got_angle){
					v_printf(1, "Warning: Duplicate Angle definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Angle: %f", &NewGDS.angle);
					got_angle = true;
				}
			}else if(strstr(line, "Flipped:")){
				if(!in_GDS){
					v_printf(1, "Error: Flipped definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if(got_Flipped){
					v_printf(1, "Warning: Duplicate Flipped definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}else{
					sscanf(line, "Flipped: %d", &NewGDS.Flipped);
					got_Flipped = true;
				}
			} else if (strstr(line, "FlipChip:")) {
				if (!in_GDS) {
					v_printf(1, "Error: FlipChip definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if (got_FlipChip) {
					v_printf(1, "Warning: Duplicate FlipChip definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}
				else {
					int FlipChip;
					sscanf(line, "FlipChip: %d", &FlipChip);
					FlipChip == 0 ? NewGDS.FlipChip = false : NewGDS.FlipChip = true;
					got_FlipChip = true;
				}
			} else if (strstr(line, "MeshMaxElemSize:")) {
				if (!in_GDS) {
					v_printf(1, "Error: MeshMaxElemSize definition outside of GDS_Start and GDS_End on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}
				if (got_MeshSize) {
					v_printf(1, "Warning: Duplicate MeshMaxElemSize definition on line %d of assembly file. Ignoring new definition.\n", current_line);
				}
				else {
					NewGDS.MeshMaxElemSize = GetLineValue(line, "MeshMaxElemSize: ");
					got_MeshSize = true;
				}
			} else if(strstr(line, "GDS_End")){
				if(!in_GDS){
					v_printf(1, "Error: GDS_End without GDS_Start on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				} else if (!got_assemblyfile && (!got_gdsfile && !got_processfile)) {
					v_printf(1, "Error: GDS_End without GDS File or Assembly File on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}else if(!got_gdsfile && !got_assemblyfile){
					v_printf(1, "Error: GDS_End without GDS File on line %d of assembly file.\n", current_line);
					PareseFileError(NewGDS);
					return;
				}else if(!got_processfile && !got_assemblyfile){
					v_printf(1, "Error: GDS_End without Process File on line %d of assembly file.\n", current_line);
					fclose(pptr);
					return;
				}else if(!got_X){
					v_printf(1, "Warning: GDS_End without X on line %d of assembly file.\n", current_line);
				} else if (!got_Y) {
					v_printf(1, "Warning: GDS_End without Y on line %d of assembly file.\n", current_line);
				} else if (!got_Z) {
					v_printf(1, "Warning: GDS_End without Z on line %d of assembly file.\n", current_line);
				}
				if (!got_assemblyfile) {
					AddGDS(&NewGDS);
				
				} else {
					GDS *gds;
					float x, y;
					gds = Assembly->GetGDS();
					while (gds) {
						x = gds->X * cos(M_PI / 180 * NewGDS.angle) + gds->Y * sin(M_PI / 180 * NewGDS.angle);
						y = gds->X * -sin(M_PI / 180 * NewGDS.angle) + gds->Y * cos(M_PI / 180 * NewGDS.angle);
						gds->X = x;
						gds->Y = y;
						gds->Flipped = (gds->Flipped ^ NewGDS.Flipped);
						if (NewGDS.Flipped) {
							gds->X = -gds->X;
						}
						gds->X += NewGDS.X;
						gds->Y += NewGDS.Y;
						gds->Z += NewGDS.Z;
						gds->angle += NewGDS.angle;
						gds->FlipChip = (gds->FlipChip ^ NewGDS.FlipChip) ;
						gds->MeshMaxElemSize = NewGDS.MeshMaxElemSize;
						AddGDS(gds);
						gds = gds->Next;
					}
					delete Assembly;

				}
				if					(NewGDS.Name) {
					delete [] NewGDS.Name;
					NewGDS.Name = NULL;
				}
				in_GDS = false;
			}
		}
	}
	if(NewGDS.Name){
		delete [] NewGDS.Name;
		NewGDS.Name = NULL;
	}
	if (NewGDS.gdsfile) {
		delete[] NewGDS.gdsfile;
		NewGDS.gdsfile = NULL;
	}
	if (NewGDS.processfile) {
		delete[] NewGDS.processfile;
		NewGDS.processfile = NULL;
	}
	fclose(pptr);
	v_printf(1, "\n");
}

struct GDS *GDSAssembly::GetGDS(int Index)
{
	struct GDS *gds;

	gds = _FirstGDS;

	while(gds){
		if(gds->Index == Index){
			return gds;
		}
		gds = gds->Next;
	}
	return NULL;
}

struct GDS *GDSAssembly::GetGDS(const char *Name)
{
	struct GDS *gds;

	gds = _FirstGDS;

	while(gds){
		if(strcmp(Name, gds->Name) == 0){
			return gds;
		}
		if (strcmp(Name, remove_extension(getfilename(gds->gdsfile))) == 0) {
			return gds;
		}
		gds = gds->Next;
	}
	return NULL;
}

struct GDS *GDSAssembly::GetGDS()
{
	return _FirstGDS;
}

int GDSAssembly::GDSCount()
{
	return _Count;
}

char * GDSAssembly::GetName()
{
	return Name;
}

void GDSAssembly::ChangeVisibility(struct GDS *gds, bool Show)
{
	gds->Show = Show;
}

void GDSAssembly::ChangeLegendIndex(struct GDS *gds, int LegendIndex)
{
	gds->LegendIndex = LegendIndex;
}

void GDSAssembly::AddGDS(struct GDS *NewGDS)
{
	struct GDS *gds;

	gds = _FirstGDS;

	if(_FirstGDS){
		while (gds->Next)
		{
			gds = gds->Next;
		}
		gds->Next = new struct GDS;
		gds = gds->Next;
		gds->Next = NULL;
	}else{
		_FirstGDS = new struct GDS;
		gds = _FirstGDS;
		gds->Next = NULL;
	}

	gds->Name = NULL;
	if(NewGDS->Name){
		gds->Name = new char[strlen(NewGDS->Name)+1];
		strcpy(gds->Name, NewGDS->Name);
	}
	gds->gdsfile = NULL;
	if (NewGDS->gdsfile) {
		gds->gdsfile = new char[strlen(NewGDS->gdsfile) + 1];
		strcpy(gds->gdsfile, NewGDS->gdsfile);
	}
	gds->processfile = NULL;
	if (NewGDS->processfile) {
		gds->processfile = new char[strlen(NewGDS->processfile) + 1];
		strcpy(gds->processfile, NewGDS->processfile);
	}
	gds->X = NewGDS->X;
	gds->Y = NewGDS->Y;
	gds->Z = NewGDS->Z;
	gds->angle = NewGDS->angle;
	gds->Flipped = NewGDS->Flipped;
	gds->FlipChip = NewGDS->FlipChip;
	gds->MeshMaxElemSize = NewGDS->MeshMaxElemSize;
	gds->Index = NewGDS->Index;
	gds->LegendIndex = NewGDS->LegendIndex;
}

bool GDSAssembly::IsValid()
{
	return _Valid;
}

bool GDSAssembly::Save(const char *filename)
{
	struct GDS *gds;
	FILE *fptr = NULL;

	if(!filename) return false;

	fptr = fopen(filename, "wt");
	if(!fptr) return false;

	gds = _FirstGDS;
	while(gds){
		if (gds->Name) {
			fprintf(fptr, "GDS_Start: %s\n", gds->Name);
		} else {
		fprintf(fptr, "GDS_Start: GDS-%s-%s\n", gds->gdsfile, gds->processfile);
		}
		fprintf(fptr, "GDS_File: %s\n", gds->gdsfile);
		fprintf(fptr, "GDS_process: %s\n", gds->processfile);
		fprintf(fptr, "X: 0\n");
		fprintf(fptr, "Y: 0\n");
		fprintf(fptr, "Z: 0.0\n");
		fprintf(fptr, "Angle: 0.0\n");
		fprintf(fptr, "Flipped: 0.0\n");
		fprintf(fptr, "FlipChip: 0.0\n");

		gds = gds->Next;
	}
	fclose(fptr);

	return true;
}

