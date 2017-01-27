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


#ifdef WIN32
/*
float fmin(float x, float y)
{
	if(x > y)
		return y;
	return x;
}

float fmax(float x, float y)
{
	if(x <= y)
		return y;
	return x;
}
*/
#endif

#include "gds_globals.h"
#include "gdsobject_ogl.h"
#include "gdsobjectlist.h"
#include "process_cfg.h"
#include "renderer.h"
#include "windowmanager.h"


unsigned long   mem_tris = 0;
unsigned long  mem_total = 0;
unsigned long total_listtris;
VECTOR3D cam; // Camera position
FRUSTUM frustum; // For bounding box culling
MATRIX4X4 view; // Modelview matrix
GLint cull_type;
float exploded_fraction = 0.0f;
float exploded_accel = 0.0f;
bool exploded_view = false;
float color_scale = 1.0f;
bool transparent_object = false;

// Render frontend
void init_render()
{
    // Backend
    renderer.init();
}


// GDSObject Class

GDSObject_ogl::GDSObject_ogl(char *Name, char *gdsName) : GDSObject(Name,gdsName){
	PolyItemsCurIndex = 0;
	numtris = 0;
}

GDSObject_ogl::~GDSObject_ogl()
{
	PolyItemsCurIndex = 0;
	numtris = 0;
	DeleteBuffers();
}
void GDSObject_ogl::OutputOGLVertices2(struct ProcessLayer *do_layer, render_layer_t *data)
{
	OutputOGLVertices2(do_layer, data, false);
}
// New, vertex list based rendering
void GDSObject_ogl::OutputOGLVertices2(struct ProcessLayer *do_layer, render_layer_t *data, bool Update)
{
	struct ProcessLayer *layer;
	float largest_dimension = 0.0; // Largest dimension of an object
	float xmin, ymin, zmin, xmax, ymax, zmax;
	class GDSPolygon *polygon;
    float dx, dy;
    vector<size_t> *indices; // Pointer to index array of the triangles
	size_t tp=0, bp=0, tp2=0, bp2=0; // Top and bottom pointer into the vertex array
	size_t v[3]; // Indices of a triangle
    
	if (Update) {
		GDS3DBB BBox3D = GetTotal3DBoundary();
		zmin = BBox3D.min.Z;
		xmin = BBox3D.min.X;
		ymin = BBox3D.min.Y;
		zmax = BBox3D.max.Z;
		xmax = BBox3D.max.Z;
		ymax = BBox3D.max.Z;
		
		if(PolyItemsCurIndex!=0)
			largest_dimension = data->largest_dimension;
	}
	else {
		zmin = xmin = ymin = 100000; zmax = xmax = ymax = -10000;
	}
    for(size_t i=PolyItemsCurIndex; i<PolygonItems.size(); i++)
    {
        polygon = PolygonItems[i];
        layer = polygon->GetLayer();
        if(do_layer && layer!= do_layer)
            continue;


        float z1 = polygon->GetHeight();
        float z2 = polygon->GetHeight() + polygon->GetThickness();
        float x0, y0;
        
        //Bbox
        for(size_t j=0; j<polygon->GetPoints(); j++){
            x0 = polygon->GetXCoords(j);
            y0 = polygon->GetYCoords(j);
            
            xmin = fmin(xmin, x0);
            ymin = fmin(ymin, y0);
            xmax = fmax(xmax, x0);
            ymax = fmax(ymax, y0);
        }
        zmin = z1;
        zmax = z2;
        
        // Send vertices to vertex buffer
        tp = tp2 = renderer.getCurIndex(); // Top pointer
        for(size_t j=0; j<polygon->GetPoints(); j++)
            renderer.addVertex((GLfloat) polygon->GetXCoords(j), (GLfloat) polygon->GetYCoords(j),z2);
        bp = bp2 = renderer.getCurIndex(); // Bottom pointer
        for(size_t j=0; j<polygon->GetPoints(); j++)
            renderer.addVertex((GLfloat) polygon->GetXCoords(j), (GLfloat) polygon->GetYCoords(j),z1);
        
        // Assemble triangles
		indices = polygon->GetIndices();
		if(!indices)
			continue;
        
        // Largest dimension
        for(size_t j=0;j<indices->size()/3;j++)
        {
            dx = fmax(fmax(fabs(polygon->GetXCoords((*indices)[j*3+0]) - polygon->GetXCoords((*indices)[j*3+1])), fabs(polygon->GetXCoords((*indices)[j*3+0]) - polygon->GetXCoords((*indices)[j*3+2]))), fabs(polygon->GetXCoords((*indices)[j*3+1]) - polygon->GetXCoords((*indices)[j*3+2])));
            dy = fmax(fmax(fabs(polygon->GetYCoords((*indices)[j*3+0]) - polygon->GetYCoords((*indices)[j*3+1])), fabs(polygon->GetYCoords((*indices)[j*3+0]) - polygon->GetYCoords((*indices)[j*3+2]))), fabs(polygon->GetYCoords((*indices)[j*3+1]) - polygon->GetYCoords((*indices)[j*3+2])));
            if(fmax(dx,dy)/fmin(dx,dy) > 2.0f)
                largest_dimension = fmax(fmin(dx, dy)/0.5f, largest_dimension);
            else
                largest_dimension = fmax(fmin(dx, dy)/3.0f, largest_dimension);
            
        }        
		
        // Even-odd ordering?
        int e=1;
        int o=1;
        for(size_t j=0;j<indices->size()/3;j++)
        {
            v[0] = (*indices)[j*3+0];
            v[1] = (*indices)[j*3+1];
            v[2] = (*indices)[j*3+2];
            
            if((v[0]%2)==0 && (v[1]%2)==0 && (v[2]%2)==0)
                o=0;
            if((v[0]%2)==1 && (v[1]%2)==1 && (v[2]%2)==1)
                e=0;
        }
        if( (e==0 && o==0) || (e==1 && o==0 && (indices->size()/3)%2==1)) // Oh oh, we need to duplicate vertices for the boundary
        {
            // Duplicate vertices
            tp2 = renderer.getCurIndex(); // Top pointer
            for(size_t j=0; j<polygon->GetPoints(); j++)
                renderer.addVertex((GLfloat) polygon->GetXCoords(j), (GLfloat) polygon->GetYCoords(j),z2);
            bp2 = renderer.getCurIndex(); // Bottom pointer
            for(size_t j=0; j<polygon->GetPoints(); j++)
                renderer.addVertex((GLfloat) polygon->GetXCoords(j), (GLfloat) polygon->GetYCoords(j),z1);	

			e = 0;
			o = 1;
        }
		if(e==1 && o==1)
		{
			e = 0;
			o = 1;
		}
        
        // Stream top
        for(unsigned int j=0;j<indices->size()/3;j++)
        {
            v[0] = (*indices)[j*3+0];
            v[1] = (*indices)[j*3+1];
            v[2] = (*indices)[j*3+2];
            
			if( (e && v[1]%2==0) || (o && v[1]%2==1) )
                renderer.addTriangle(tp+v[2], tp+v[0], tp+v[1]);
            else if( (e && v[2]%2==0) || (o && v[2]%2==1))
                renderer.addTriangle(tp+v[0], tp+v[1], tp+v[2]);            
            else
                renderer.addTriangle(tp+v[1], tp+v[2], tp+v[0]);
            numtris++;
        }
        
        // Stream bottom
        for(size_t j=0;j<indices->size()/3;j++)
        {
            v[0] = (*indices)[j*3+0];
            v[1] = (*indices)[j*3+1];
            v[2] = (*indices)[j*3+2];
            
			if( (e && v[1]%2==0) || (o && v[1]%2==1) )
				renderer.addTriangle(bp+v[0], bp+v[2], bp+v[1]);                
            else if( (e && v[2]%2==0) || (o && v[2]%2==1))
                renderer.addTriangle(bp+v[1], bp+v[0], bp+v[2]);
            else
                renderer.addTriangle(bp+v[2], bp+v[1], bp+v[0]);
            numtris++;
        }
        
        // Stream boundary
        for(unsigned int j=0;j<polygon->GetPoints();j++)
        {
            v[0] = j+0;
            v[1] = (j+1)%(polygon->GetPoints());
            
            if(v[1]%2!=o)
            {
                renderer.addTriangle(bp2+v[0], bp2+v[1], tp2+v[1]);
                renderer.addTriangle(tp2+v[0], bp2+v[0], tp2+v[1]);
            }
            else
            {
                renderer.addTriangle(bp2+v[1], tp2+v[1], bp2+v[0]);
                renderer.addTriangle(tp2+v[1], tp2+v[0], bp2+v[0]);
            }
            numtris+=2;
        }
        
        // Give renderer the chance to flush its buffers
        renderer.allowFlush(); 
    }	
	
	// Visibility data
	data->bbox.SetFromMinsMaxes(VECTOR3D(xmin, ymin, zmin), VECTOR3D(xmax, ymax, zmax) );
	data->largest_dimension = largest_dimension;
}

void
GDSObject_ogl::BuildLists()
{
	BuildLists(false);
}

void
GDSObject_ogl::BuildLists(bool Update)
{
	render_layer_t render_layer;
	struct ProcessLayer *layer;
	bool found;

	if(PolygonItems.empty() && PathItems.empty())
		return;

	//
	if (!Update)
		numtris = 0;
	//v_printf(1, "Building display lists for object %s.\n", this->Name);

    // Build unique list of layers
	if(!PathItems.empty())
	{
		for(unsigned long i=0; i<PathItems.size(); i++)
		{
			// Get layer
			layer = PathItems[i]->GetLayer();
			if(!layer)
				continue;

			// Try to find layer
			found = false;
			for(unsigned long j=0;j<layer_list.size();j++)
			{
				if(layer_list[j].layer == layer)
				{
					found = true;
					break;
				}
			}

			// New layer?
			if(!found)
			{
				render_layer.layer = layer;
				render_layer.display_list = 0;
				layer_list.push_back(render_layer);
			}
		}
	}
	if(!PolygonItems.empty())
	{
		for(size_t i= PolyItemsCurIndex; i<PolygonItems.size(); i++)
		{
			// Get layer
			layer = PolygonItems[i]->GetLayer();
			if(!layer)
				continue;

			// Try to find layer
			found = false;
			for(unsigned long j=0;j<layer_list.size();j++)
			{
				if(layer_list[j].layer == layer)
				{
					found = true;
					break;
				}
			}

			// New layer?
			if(!found)
			{
				render_layer.layer = layer;
				render_layer.display_list = 0;
				render_layer.renderRecipe = NULL;
				layer_list.push_back(render_layer);
			}
		}
	}

	// Output geometry for each layer
	total_listtris = 0;
	for(unsigned long i=0;i<layer_list.size();i++)
	{
		numtris = 0;
        layer_list[i].renderRecipe = renderer.beginObject();
		OutputOGLVertices2(layer_list[i].layer, &layer_list[i], Update);
        renderer.endObject();
        
		layer_list[i].numtris = numtris;

		if(i==0)
			bbox = layer_list[i].bbox;
		else
			bbox.AddBounds(layer_list[i].bbox);

		
        mem_tris+=numtris;
		
		total_listtris+=numtris;	

	}
	
	v_printf(1, "Object %s created with %d triangles.\n", Name, total_listtris);
}

void GDSObject_ogl::PrepareRender(MATRIX4X4 projection_view, MATRIX4X4 object_view)
{
	MATRIX4X4 projection;

	projection = projection_view;
	frustum.SetFromMatrix(projection);
	cam = VECTOR3D( ((GLfloat*)projection)[3], ((GLfloat*)projection)[7], ((GLfloat*)projection)[11]);

	view = object_view;

	glCullFace(GL_BACK); cull_type = GL_BACK;

    renderer.beginRender(frustum);
}

void GDSObject_ogl::EndRender() {
	GDSObject_ogl::EndRender(false);
}

void GDSObject_ogl::EndRender(bool Update)
{
    renderer.endRender();
	if (Update)
		PolyItemsCurIndex = PolygonItems.size();
}
void GDSObject_ogl::UploadToVRAM()
{
	GDSObject_ogl::UploadToVRAM(false);
}
void GDSObject_ogl::UploadToVRAM(bool Update)
{    
    // Do we need to build the geometry?
	if((!layer_list.size() && (!PolygonItems.empty() || !PathItems.empty())) || Update)
		BuildLists(Update);
    
	for (unsigned int i = 0; i < refs.size(); i++) {
		((GDSObject_ogl*)refs[i]->object)->UploadToVRAM();
	}
}

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114
void GDSObject_ogl::RenderList(MATRIX4X4 object_view, bool HQ)
{
	RenderList(object_view, HQ, false);
}
void GDSObject_ogl::RenderList(MATRIX4X4 object_view, bool HQ, bool Update)
{
	struct ProcessLayer *layer;
    
    // Do we need to build the geometry?
	if((!layer_list.size() && (!PolygonItems.empty() || !PathItems.empty())) || (Update && (PolygonItems.size()>PolyItemsCurIndex)) )
    {
        // Recursively step through geometry, this is only called here from the topcell
        UploadToVRAM(Update);

		// Add substrate somewhere
		wm->getWorld()->buildSubstrate();

		// Flush the renderer
        renderer.forceFlush();
    }
    
    // Go to sub cells
	MATRIX4X4 M;
	for (unsigned int i = 0; i < refs.size(); i++) {
			((GDSObject_ogl*)refs[i]->object)->RenderList(object_view * MATRIX4X4(refs[i]->mat[0], refs[i]->mat[1], 0.0f, 0.0f, refs[i]->mat[2], refs[i]->mat[3], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, refs[i]->mat[4], refs[i]->mat[5], 0.0f, 1.0f), HQ);
	}

	// Frustum
	AA_BOUNDING_BOX bounds;
	float distance;
	MATRIX4X4 mod, total;
    VECTOR4D color;
    bool transparent;

	// Prepare bounding box
	bounds = bbox;
	bounds.maxes.z *= 1.0f+exploded_fraction;
	bounds.mins.z *= 1.0f+exploded_fraction;
	for(unsigned long i=0;i<8;i++)
		bounds.vertices[i].z *= 1.0f+exploded_fraction;
	bounds.Mult(object_view);
	
	// Frustum culling of bounding boxes
	if(!frustum.IsAABoundingBoxInside(bounds))
		return;

	distance = bounds.DistFromPoint(cam);

	// Output geometry for each layer
	for(unsigned long i=0;i<layer_list.size();i++)
	{
		layer = layer_list[i].layer;

		if(!layer->Show)
			continue;
	
		//// Frustum culling of bounding boxes
		//if(!frustum.IsAABoundingBoxInside(bbox))
			//continue;
        
        GLfloat alpha = 1.0f-layer->Filter;
        //alpha = 1.0f;
		
		// Visibility of small objects
		float zrel;

		if(HQ)
		{
			zrel = (0.00075f/4.0f) / fabs(layer_list[i].largest_dimension / distance);
			if(zrel > 1.0f)
				continue;
		}
		else
		{
			zrel = 0.00075f / fabs(layer_list[i].largest_dimension / distance);
			if(zrel > 1.0f)
				continue;
		}
        if(zrel>0.5f)
        {
            alpha = alpha - (zrel-0.5f)*2.0f;
            if(alpha < 0.0f)
                alpha = 0.0f;
            transparent = true;
        }
        else
            transparent = false;

		if (transparent_object) {
			transparent = true;
			continue;
		}

		// Matrix manipulation
		if(exploded_fraction != 0.0f)
		{
			//mod.SetTranslation(VECTOR3D(0.0f, 0.0f, (layer_list[i].layer->Height+layer_list[i].layer->Height/2.0f)/1000.0f*exploded_fraction));
			mod.SetTranslation(VECTOR3D(0.0f, 0.0f, (layer_list[i].layer->Height + layer_list[i].layer->Height / 2.0f) * layer_list[i].layer->Units->Unitu *exploded_fraction));
			total = object_view * mod;
		}
		else
		{
            total = object_view;
		}

		// Render with layer color -> cache color computation!!
        color.Set(layer->Red, layer->Green, layer->Blue, alpha);
		if(color_scale != 1.0f)
		{
			float P = (float) sqrt(color.x*color.x*Pr+color.y*color.y*Pg+color.z*color.z*Pb);
			color.x = 0.5f*(P + (color.x-P)*color_scale);
			color.y = 0.5f*(P + (color.y-P)*color_scale);
			color.z = 0.5f*(P + (color.z-P)*color_scale);
		}
        renderer.renderObject(layer_list[i].renderRecipe, &total, &color, transparent);
	}
   
}

void GDSObject_ogl::RenderOGLSRefs(MATRIX4X4 object_view, bool HQ)
{
	GDSObject_ogl *obj;
	MATRIX4X4 mod;
	MATRIX4X4 res;

	for(unsigned int k=0;k<SRefItems.size();k++)
	{
        SRefElement *sref = SRefItems[k];

            obj = (GDSObject_ogl*)sref->object;
			if(obj && !sref->collapsed){
				res = object_view;
				if(sref->Mag!=1.0){
					mod.SetScale(VECTOR3D(sref->Mag, sref->Mag, 1));
					res = res * mod;
				}
				mod.SetTranslation(VECTOR3D(sref->X, sref->Y, 0.0f));
				res = res * mod;
				if(sref->Rotate.Y){
					mod.SetRotationAxis(-sref->Rotate.Y, VECTOR3D(0.0f, 0.0f, 1.0f));
					res = res * mod;
				}
				if(sref->Flipped){
					mod.SetScale(VECTOR3D(1.0f, -1.0f, 1.0f));
					res = res * mod;
                }

				obj->RenderList(res, HQ);
			}
            
		
	}
}

void GDSObject_ogl::RenderOGLARefs(MATRIX4X4 object_view, bool HQ)
{
	GDSObject_ogl *obj;
	int i, j;
	MATRIX4X4 mod;
	MATRIX4X4 res;
    
	for(unsigned int k=0;k<ARefItems.size();k++)
	{
		ARefElement *aref = ARefItems[k];
        
		float dx1, dx2, dy1, dy2;
        
            obj = (GDSObject_ogl*)aref->object;
            
            dx1 = (float)(aref->X2 - aref->X1) / (float)aref->Columns;
            dy1 = (float)(aref->Y2 - aref->Y1) / (float)aref->Columns;
            dx2 = (float)(aref->X3 - aref->X1) / (float)aref->Rows;
            dy2 = (float)(aref->Y3 - aref->Y1) / (float)aref->Rows;
            
            if(obj && !aref->collapsed){
                for(i=0; i<aref->Rows; i++){
                    for(j=0; j<aref->Columns; j++){
                        res = object_view;
                        if(aref->Mag!=1.0){
                            mod.SetScale(VECTOR3D(aref->Mag, aref->Mag, 1));
                            res = res * mod;
                        }
                        mod.SetTranslation(VECTOR3D(aref->X1+dx1*(float)j+dx2*(float)i, aref->Y1+dy2*(float)i+dy1*(float)j, 0.0f));
                        res = res * mod;
                        if(aref->Rotate.Y){
                            mod.SetRotationAxis(-aref->Rotate.Y, VECTOR3D(0.0f, 0.0f, 1.0f));
                            res = res * mod;
                        }
                        if(aref->Flipped){
                            mod.SetScale(VECTOR3D(1.0f, -1.0f, 1.0f));
                            res = res * mod;
                        }
                        
                        obj->RenderList(res, HQ);
                        
                    }
                }
            }           
	}
}

void
GDSObject_ogl::DeleteBuffers()
{
	// Delete display lists of all layers
	for(unsigned long i=0;i<layer_list.size();i++)
	{
        if(layer_list[i].renderRecipe)
            renderer.deleteRecipe(layer_list[i].renderRecipe);
        layer_list[i].renderRecipe = NULL;
	}
	layer_list.clear();
	
	mem_tris = 0;
    mem_total = 0;
}

AA_BOUNDING_BOX GDSObject_ogl::GetBBox()
{
	return bbox;
}
