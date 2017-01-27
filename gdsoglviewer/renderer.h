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

#ifndef GDS3D_Renderer_h
#define GDS3D_Renderer_h

#include "../math/Maths.h"

// This is the only place gl.h should be included!
#ifdef WIN32
	#include <windows.h>
#endif
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#ifndef __APPLE__
	#define GLhandleARB GLuint
#endif

#define Renderer_SIZE 1024*64 // 64K buffers max (GLushort limit)
#define VERTEX_INDEX_RATIO 5

typedef struct drawvert2_t{
	GLfloat vertex[3];
	GLfloat normal[3];
}drawvert2_t;

typedef struct VBO2_t{
	// VBO extension
    GLuint vertbuffer;
    GLuint indexbuffer;
    int numObjects;
    
    struct VBO2_t *next;
    struct VBO2_t *prev;
}VBO2_t;

typedef struct renderRecipe_t{
    VBO2_t   *VBO;
    int     firstIndex;
    int     numIndices;
    
    // Bounding box of geometry
    AA_BOUNDING_BOX bounds;

	// Fallback display lists
	GLuint displaylist;
    
    struct  renderRecipe_t* next;
}renderRecipe_t;

typedef struct renderQueue_t{
    renderRecipe_t *recipe;
    MATRIX4X4 mat;
    VECTOR4D color;
}renderQueue_t;

class Renderer
{
private:
    drawvert2_t  drawverts[Renderer_SIZE]; // Glfloat are 4 bytes each
    unsigned int         numDrawverts;
    GLushort    indices[Renderer_SIZE*VERTEX_INDEX_RATIO]; // Gluint (4 bytes) not native to ATI R300. GLushort is 2 bytes.
    unsigned int         numIndices;

	// Framebuffer
	GLuint	FBO;
	GLuint  RBO_depth;
	GLuint  RBO_color;

	GLuint	FBO2;
	GLuint  RBO2_color;
    
    // Vertex creation
    bool    enableVBO;
    bool    enableShaders;
	bool	enableFBO;
	bool	enableMultiSample;
    VBO2_t   *curVBO;
    VBO2_t   *firstVBO;
    renderRecipe_t *curRecipe;
    VBO2_t  *boundVBO;
    
    // Shaders
    GLhandleARB  vertexProgram;
    GLhandleARB  fragmentProgram;
    GLhandleARB  shaderProgram;
    void	loadGLExtensions();
    bool    IsExtensionSupported2( char* szTargetExtension );
    void    emitTriangles();
    void    deleteVBO(VBO2_t *vbo);
    void    printShaderInfoLog(GLhandleARB obj);
    void    printProgramInfoLog(GLhandleARB obj);
    void    loadShaderProgram();
    
    // State
    GLint       cull_type; // Backface culling
    MATRIX4X4   modelview; // Modelview matrix
    FRUSTUM     frustum; // For bounding box culling
    bool        blending;
    VECTOR4D    cur_color;
    
    // TGA saving
    int savedImages;
    
public:
    // Renderer
                        Renderer();
                        ~Renderer();
    void                init();
    void                beginRender(FRUSTUM frustum);
    void                endRender();
    void                setWireframe(bool enable);

	// Framebuffer
	bool				offlineFramebuffer(int width, int height);
	void				blitFramebuffer(int width, int height);
	void				onlineFramebuffer();
     
    // Vertex creation
    renderRecipe_t*     beginObject();
	renderRecipe_t * beginObject(bool Update);
    unsigned int        getCurIndex();
	unsigned int        addVertex(GLfloat x, GLfloat y, GLfloat z);
	void                addTriangle(size_t v1, size_t v2, size_t v3);
    void                allowFlush();
    void                endObject();
    void                renderObject(renderRecipe_t *recipe, MATRIX4X4 *mat, VECTOR4D *color, bool transparent);
    void                forceFlush();
    void                deleteRecipe(renderRecipe_t *recipe);

	// 2D Rendering
	void				start2D(int width, int height);
	void				drawSquare(float x1, float y1, float x2, float y2, int filled, VECTOR4D color);
	void				drawLines(unsigned int numLines, float *x_coords, float *y_coords, VECTOR4D color);
    
    // TGA image save
    int                 tgaSave(char* filename, short int width, short int height, unsigned char pixelDepth,unsigned char* imageData);
	int                 tgaSaveSeries(char* filename, short int width, short int height, unsigned char	pixelDepth,unsigned char* imageData);
	int                 tgaGrabScreenSeries(char *filename);
    
    bool        wireframe; // Wireframe rendering

};

extern Renderer renderer;
extern unsigned long total_tris;

#endif
