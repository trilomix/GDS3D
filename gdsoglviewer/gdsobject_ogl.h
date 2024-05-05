//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2019 Bertrand Pigeard
//  Copyright (C) 2013 IC-Design Group, University of Twente.
//
//  Based on gds2pov by Roger Light, http://atchoo.org/gds2pov/ / https://github.com/ralight/gds2pov
//  Copyright (C) 2004-2008 by Roger Light
//
/*
 * File: gdsobject_ogl.h
 * Author: Roger Light
 * Project: gdsto3d
 *
  
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GDSOBJECT_OGL_H__
#define __GDSOBJECT_OGL_H__

#include "gdsobject.h"
#include "../math/Maths.h"
#include "renderer.h"

extern void init_render();

typedef struct render_layer_t
{
	struct ProcessLayer *layer;
	GLuint display_list;
	unsigned long numtris;
	float largest_dimension;
	AA_BOUNDING_BOX bbox;
    renderRecipe_t *renderRecipe;
}render_layer_t;

typedef struct drawvert_t{
	GLfloat vertex[3];
	GLfloat normal[3];
}drawvert_t;

class GDSObject_ogl : public GDSObject
{
private:
	AA_BOUNDING_BOX bbox; // 3D Bounding box
	unsigned long	numtris;
	size_t    PolyItemsCurIndex;
	float _Quality;

public:
	vector<render_layer_t> layer_list;	

	GDSObject_ogl(char *Name, char *gdsName);
	~GDSObject_ogl();

	void EndRender(bool Update);

	void UploadToVRAM();
	void UploadToVRAM(bool Update);
	void OutputOGLVertices2(struct ProcessLayer *do_layer, render_layer_t *data);

	void OutputOGLVertices2(ProcessLayer * do_layer, render_layer_t * data, bool Update);

	void PrepareRender(MATRIX4X4 projection_view, MATRIX4X4 object_view);
	void EndRender();
	void RenderList(MATRIX4X4 object_view, bool HQ, float fps);
	void RenderList(MATRIX4X4 object_view, float Quality, bool Update);
	void RenderOGLSRefs(MATRIX4X4 object_view, bool HQ);
	void RenderOGLARefs(MATRIX4X4 object_view, bool HQ);

	void BuildLists();

	void BuildLists(bool Update);

	void DeleteBuffers();
	
	AA_BOUNDING_BOX GetBBox();
};

extern unsigned long total_listtris;
extern unsigned long mem_tris;
extern float exploded_fraction;
extern float exploded_accel;
extern bool exploded_view;
extern float color_scale;
extern bool transparent_object;

#endif // __GDSOBJECT_OGL_H__

