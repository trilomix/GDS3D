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


// Headers of OpenGL extension mechanism
#ifdef __APPLE__
	#define GL_GLEXT_PROTOTYPES // Already includes all extension pointers
#endif

#include "gds_globals.h"
#include "renderer.h"

#if defined(WIN32)
	#include "glext.h"
#elif !defined(__APPLE__)
	#include <GL/glx.h>
#endif

// Define extensions
#ifndef __APPLE__
// Warn if compiling without OpenGL extensions
#ifndef GL_ARB_vertex_buffer_object
	#pragma message "  GL_ARB_vertex_buffer_object not available during compiling."
#endif
#ifndef GL_ARB_shader_objects
	#pragma message "  GL_ARB_shader_objects not available during compiling."
#endif
#ifndef GL_EXT_framebuffer_object
	#pragma message "  GL_EXT_framebuffer_object not available during compiling."
#endif
#ifndef GL_EXT_framebuffer_multisample
	#pragma message "  GL_EXT_framebuffer_multisample not available during compiling."
#endif
#ifndef GL_EXT_framebuffer_blit
	#pragma message "  GL_EXT_framebuffer_multisample not available during compiling."
#endif
// Extension Function Pointers
#ifdef GL_ARB_vertex_buffer_object
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;					// VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;					// VBO Bind Procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;					// VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;			// VBO Deletion Procedure
#endif
#ifdef GL_ARB_shader_objects
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB = NULL;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = NULL;
#endif
#ifdef GL_EXT_framebuffer_object
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
#endif
#ifdef GL_EXT_framebuffer_multisample
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = NULL;
#endif
#ifdef GL_EXT_framebuffer_blit
PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = NULL;
#endif
#endif // __APPLE__

Renderer renderer;
unsigned long	total_tris;

const char vertexProgramSource[512] = "void main(){	gl_FrontColor = gl_Color*(vec4(0.7,0.7,0.7,1.0) + vec4(0.5,0.5,0.5,0.0)*max(dot(gl_NormalMatrix *gl_Normal, vec3(0.0,-0.89,-0.45)),0.0)); gl_Position = ftransform(); }";
//const char vertexProgramSource[512] = "void main(){	gl_FrontColor = vec4(0.5,0.5,0.5,1.0); gl_Position = ftransform(); }";
const char fragmentProgramSource[512] = "void main(){ float z = gl_FragCoord.z / gl_FragCoord.w; float fogFactor = exp( -gl_Fog.density  * z ); fogFactor = clamp(fogFactor, 0.0, 1.0); gl_FragColor.rgb = gl_Color.rgb*fogFactor; gl_FragColor.a = gl_Color.a; }";
//const char fragmentProgramSource[512] = "void main(){gl_FragColor = gl_Color; }";

renderQueue_t *renderQueue = NULL;
int queueLength = 0;
int queueMax = 0;

void
Renderer::loadGLExtensions()
{
#ifndef __APPLE__
#ifdef WIN32
    // Get Pointers To The GL Functions
#ifdef GL_ARB_vertex_buffer_object
    glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
    glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
    glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
    glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
#endif
#ifdef GL_ARB_shader_objects
    glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)  wglGetProcAddress("glCreateShaderObjectARB");
    glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress("glGetObjectParameterivARB");
    glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC) wglGetProcAddress("glGetInfoLogARB");
    glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC) wglGetProcAddress("glShaderSourceARB");
    glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)  wglGetProcAddress("glCompileShaderARB");
    glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)  wglGetProcAddress("glCreateProgramObjectARB");
    glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC) wglGetProcAddress("glAttachObjectARB");
    glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC) wglGetProcAddress("glLinkProgramARB");
    glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)  wglGetProcAddress("glUseProgramObjectARB");
#endif
#ifdef GL_EXT_framebuffer_object
    glGenFramebuffersEXT                     = (PFNGLGENFRAMEBUFFERSPROC)                      wglGetProcAddress("glGenFramebuffersEXT");
    glBindFramebufferEXT                     = (PFNGLBINDFRAMEBUFFERPROC)                      wglGetProcAddress("glBindFramebufferEXT");
    glGenRenderbuffersEXT                    = (PFNGLGENRENDERBUFFERSPROC)                     wglGetProcAddress("glGenRenderbuffersEXT");
    glBindRenderbufferEXT                    = (PFNGLBINDRENDERBUFFERPROC)                     wglGetProcAddress("glBindRenderbufferEXT");
    glRenderbufferStorageEXT                = (PFNGLRENDERBUFFERSTORAGEPROC)                  wglGetProcAddress("glRenderbufferStorageEXT");
    glFramebufferRenderbufferEXT             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)              wglGetProcAddress("glFramebufferRenderbufferEXT");
    glDeleteRenderbuffersEXT                = (PFNGLDELETERENDERBUFFERSPROC)                  wglGetProcAddress("glDeleteRenderbuffersEXT");
    glDeleteFramebuffersEXT                  = (PFNGLDELETEFRAMEBUFFERSPROC)                   wglGetProcAddress("glDeleteFramebuffersEXT");
    glCheckFramebufferStatusEXT			= (PFNGLCHECKFRAMEBUFFERSTATUSPROC)		wglGetProcAddress("glCheckFramebufferStatusEXT");
#endif
#ifdef GL_EXT_framebuffer_multisample
    glRenderbufferStorageMultisampleEXT      = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)       wglGetProcAddress("glRenderbufferStorageMultisampleEXT");
#endif
#ifdef GL_EXT_framebuffer_blit
    glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFERPROC) wglGetProcAddress("glBlitFramebuffer");
#endif
#else
    // Get Pointers To The GL Functions
#ifdef GL_ARB_vertex_buffer_object
    glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) glXGetProcAddress((const GLubyte *) "glGenBuffersARB");
    glBindBufferARB = (PFNGLBINDBUFFERARBPROC) glXGetProcAddress((const GLubyte *) "glBindBufferARB");
    glBufferDataARB = (PFNGLBUFFERDATAARBPROC) glXGetProcAddress((const GLubyte *) "glBufferDataARB");
    glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) glXGetProcAddress((const GLubyte *) "glDeleteBuffersARB");
#endif
#ifdef GL_ARB_shader_objects
    glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)  glXGetProcAddress((const GLubyte *) "glCreateShaderObjectARB");
    glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) glXGetProcAddress((const GLubyte *) "glGetObjectParameterivARB");
    glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC) glXGetProcAddress((const GLubyte *) "glGetInfoLogARB");
    glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC) glXGetProcAddress((const GLubyte *) "glShaderSourceARB");
    glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)  glXGetProcAddress((const GLubyte *) "glCompileShaderARB");
    glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)  glXGetProcAddress((const GLubyte *) "glCreateProgramObjectARB");
    glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC) glXGetProcAddress((const GLubyte *) "glAttachObjectARB");
    glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC) glXGetProcAddress((const GLubyte *) "glLinkProgramARB");
    glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)  glXGetProcAddress((const GLubyte *) "glUseProgramObjectARB");
#endif
#ifdef GL_EXT_framebuffer_object
    glGenFramebuffersEXT                     = (PFNGLGENFRAMEBUFFERSEXTPROC)                      glXGetProcAddress((const GLubyte *) "glGenFramebuffersEXT");
    glBindFramebufferEXT                     = (PFNGLBINDFRAMEBUFFEREXTPROC)                      glXGetProcAddress((const GLubyte *) "glBindFramebufferEXT");
    glGenRenderbuffersEXT                    = (PFNGLGENRENDERBUFFERSEXTPROC)                     glXGetProcAddress((const GLubyte *) "glGenRenderbuffersEXT");
    glBindRenderbufferEXT                    = (PFNGLBINDRENDERBUFFEREXTPROC)                     glXGetProcAddress((const GLubyte *) "glBindRenderbufferEXT");
    glRenderbufferStorageEXT                 = (PFNGLRENDERBUFFERSTORAGEEXTPROC)                  glXGetProcAddress((const GLubyte *) "glRenderbufferStorageEXT");
    
    glFramebufferRenderbufferEXT             = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)              glXGetProcAddress((const GLubyte *) "glFramebufferRenderbufferEXT");
    glDeleteRenderbuffersEXT                 = (PFNGLDELETERENDERBUFFERSEXTPROC)                  glXGetProcAddress((const GLubyte *) "glDeleteRenderbuffersEXT");
    glDeleteFramebuffersEXT                  = (PFNGLDELETEFRAMEBUFFERSEXTPROC)                   glXGetProcAddress((const GLubyte *) "glDeleteFramebuffersEXT");
    glCheckFramebufferStatusEXT			= (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)		glXGetProcAddress((const GLubyte *) "glCheckFramebufferStatusEXT");
#endif
#ifdef GL_EXT_framebuffer_multisample
    glRenderbufferStorageMultisampleEXT      = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)       glXGetProcAddress((const GLubyte *) "glRenderbufferStorageMultisampleEXT");
#endif
#ifdef GL_EXT_framebuffer_blit
    glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC) glXGetProcAddress((const GLubyte *) "glBlitFramebufferEXT");
#endif
#endif
#endif
}

// Private members
bool 
Renderer::IsExtensionSupported2( char* szTargetExtension )
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;
    
	// Extension names should not have spaces
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if( pszWhere || *szTargetExtension == '\0' )
		return false;
    
	// Get Extensions String
	pszExtensions = glGetString( GL_EXTENSIONS );
    
	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for(;;)
	{
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
			if( *pszTerminator == ' ' || *pszTerminator == '\0' )
				return true;
		pszStart = pszTerminator;
	}
	return false;
}

void
Renderer::emitTriangles()
{
	// Do the normals
	float nx, ny, nz;
	float dx1, dy1, dz1;
	float dx2, dy2, dz2;
	float l;
	for(unsigned int j=0;j<numIndices/3;j++)
	{
		dx1 = drawverts[indices[j*3+2]].vertex[0]-drawverts[indices[j*3+1]].vertex[0];
		dy1 = drawverts[indices[j*3+2]].vertex[1]-drawverts[indices[j*3+1]].vertex[1];
		dz1 = drawverts[indices[j*3+2]].vertex[2]-drawverts[indices[j*3+1]].vertex[2];
		dx2 = drawverts[indices[j*3+2]].vertex[0]-drawverts[indices[j*3]].vertex[0];
		dy2 = drawverts[indices[j*3+2]].vertex[1]-drawverts[indices[j*3]].vertex[1];
		dz2 = drawverts[indices[j*3+2]].vertex[2]-drawverts[indices[j*3]].vertex[2];		
		nx = dy1*dz2 - dz1*dy2;
		ny = dz1*dx2 - dx1*dz2;
		nz = dx1*dy2 - dy1*dx2;
		l = sqrt(nx*nx + ny*ny + nz*nz);
		nx = nx / l; ny = ny / l; nz = nz / l;

		//drawverts[indices[j*3+0]].normal[0] = nx;
		//drawverts[indices[j*3+0]].normal[1] = ny;
		//drawverts[indices[j*3+0]].normal[2] = nz;

		//drawverts[indices[j*3+1]].normal[0] = nx;
		//drawverts[indices[j*3+1]].normal[1] = ny;
		//drawverts[indices[j*3+1]].normal[2] = nz;

		drawverts[indices[j*3+2]].normal[0] = nx;
		drawverts[indices[j*3+2]].normal[1] = ny;
		drawverts[indices[j*3+2]].normal[2] = nz;
	}
    
	if(enableVBO)
	{
#ifdef GL_ARB_vertex_buffer_object
		// Put in buffer
		glGenBuffersARB( 1, &curVBO->vertbuffer );
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, curVBO->vertbuffer );
		//glBufferDataARB( GL_ARRAY_BUFFER_ARB, numDrawverts*sizeof(drawvert2_t), drawverts, GL_STATIC_DRAW_ARB );
		glBufferDataARB( GL_ARRAY_BUFFER_ARB, Renderer_SIZE*sizeof(drawvert2_t), drawverts, GL_STATIC_DRAW_ARB ); // Upload whole block
        
		glGenBuffersARB( 1, &curVBO->indexbuffer );
		glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, curVBO->indexbuffer );
		//glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, numIndices*sizeof(GLushort), indices, GL_STATIC_DRAW_ARB );
		glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, Renderer_SIZE*VERTEX_INDEX_RATIO*sizeof(GLushort), indices, GL_STATIC_DRAW_ARB ); // Upload whole block
        
		// Unbind
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
		glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
		boundVBO = NULL;
#endif
	}
	else
		glDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
    
    // Count number of VBOs and report statistics
    int count = 0;
    VBO2_t *point = firstVBO;
    while(point)
    {
        count++;
        point=point->next;
    }
    
	if(enableVBO)
		v_printf(2, "  VBO %d and %d uploaded with %d vertices and %d indices (%4.1fMB VRAM total).\n", curVBO->vertbuffer, curVBO->indexbuffer, numDrawverts, numIndices, Renderer_SIZE*(24.0f+VERTEX_INDEX_RATIO*2.0f)*count/1024.0f/1024.0f);
        
	numDrawverts = 0;
	numIndices = 0;
    
    // Generate new VBO_t
    curVBO->next = new VBO2_t;
    curVBO->next->prev = curVBO;
    curVBO = curVBO->next;
    curVBO->numObjects = 0;
    curVBO->vertbuffer = 0;
    curVBO->indexbuffer = 0;
    curVBO->next = NULL;
}

void
Renderer::deleteVBO(VBO2_t *vbo)
{
	if(enableVBO)
	{
#ifdef GL_ARB_vertex_buffer_object
		glDeleteBuffersARB(1, &vbo->vertbuffer);
		glDeleteBuffersARB(1, &vbo->indexbuffer);
#endif
	}
    
    if(vbo->next)
        vbo->next->prev = vbo->prev;
    if(vbo->prev)
        vbo->prev->next = vbo->next;
    
    if(firstVBO==vbo)
        firstVBO = vbo->next;
    
    delete vbo;
}

void 
Renderer::printShaderInfoLog(GLhandleARB obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
#ifdef GL_ARB_shader_objects
    char *infoLog;  
    
	glGetObjectParameterivARB((GLhandleARB) obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,&infologLength);
    
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetInfoLogARB((GLhandleARB) obj, infologLength, &charsWritten, infoLog);
		v_printf(1, "%s\n",infoLog);
        free(infoLog);
    }
#endif
}

void 
Renderer::printProgramInfoLog(GLhandleARB obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
#ifdef GL_ARB_shader_objects
    char *infoLog;
    
	glGetObjectParameterivARB((GLhandleARB) obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,&infologLength);
    
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetInfoLogARB((GLhandleARB) obj, infologLength, &charsWritten, infoLog);
		v_printf(1, "%s\n",infoLog);
        free(infoLog);
    }
#endif
}

void
Renderer::loadShaderProgram()
{
#ifdef GL_ARB_shader_objects
    vertexProgram = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragmentProgram = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    
    const char *vs = vertexProgramSource;
    const char *fs = fragmentProgramSource;
    
    glShaderSourceARB((GLhandleARB) vertexProgram, 1, &vs,NULL);
	glShaderSourceARB((GLhandleARB) fragmentProgram, 1, &fs,NULL);
    
    glCompileShaderARB(vertexProgram);
	glCompileShaderARB(fragmentProgram);
    
    printShaderInfoLog(vertexProgram);
	printShaderInfoLog(fragmentProgram);
    
    shaderProgram = glCreateProgramObjectARB();
    
    glAttachObjectARB(shaderProgram,vertexProgram);
	glAttachObjectARB(shaderProgram,fragmentProgram);
    
	glLinkProgramARB(shaderProgram);
    printProgramInfoLog(shaderProgram);
#endif
}


// Public members

Renderer::Renderer()
{
    enableVBO = false;
	enableShaders = false;
	enableFBO = false;
	enableMultiSample = false;
    numDrawverts = 0;
    numIndices = 0;
    wireframe = false;
    savedImages = 1;
}

Renderer::~Renderer()
{
    if(renderQueue)
        delete(renderQueue);
    renderQueue = NULL;
}

void
Renderer::init()
{
	// Try to get function pointers
	loadGLExtensions();
    
    // Detect VBO extension
#ifdef GL_ARB_vertex_buffer_object
    enableVBO = IsExtensionSupported2( (char*) "GL_ARB_vertex_buffer_object" );
	if( enableVBO )
	{
		v_printf(1, "GL_ARB_vertex_buffer_object found.\n");
    }
	else
		v_printf(1, "GL_ARB_vertex_buffer_object not found.\n");
#else
	v_printf(1, "Compiled without GL_ARB_vertex_buffer_object headers!\n");
#endif
    
    // Detect shader program extension, disabled for now for compatibility with Intel bloatware
#ifdef GL_ARB_shader_objects
    enableShaders = IsExtensionSupported2((char*) "GL_ARB_shader_objects" );
	if( enableShaders )
	{
		v_printf(1, "GL_ARB_shader_objects found.\n");
        //loadShaderProgram(); // Damn you, Intel
    }
	else
		v_printf(1, "GL_ARB_shader_objects not found.\n");
	enableShaders = false; // Damn you, Intel
#else
	v_printf(1, "Compiled without GL_ARB_shader_objects headers!\n");
#endif
    
	// Detect framebuffer extension
	GLint max_samples = 0;
	GLint max_size = 0;
#ifdef GL_EXT_framebuffer_object
    enableFBO = IsExtensionSupported2((char*) "GL_EXT_framebuffer_object" );
	if( enableVBO )
	{
		v_printf(1, "GL_EXT_framebuffer_object found.\n");
		//glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_samples);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &max_size);
		v_printf(2, "  GL_MAX_SAMPLES=%d\n", max_samples);
		v_printf(2, "  GL_MAX_RENDERBUFFER_SIZE=%d\n", max_size);
    }
	else
		v_printf(1, "GL_EXT_framebuffer_object not found.\n");
#else
	v_printf(1, "Compiled without GL_EXT_framebuffer_object headers!\n");
#endif
    
	// Detect multisample and blit framebuffer
#ifdef GL_EXT_framebuffer_multisample
#ifdef GL_EXT_framebuffer_blit
	if(max_samples >= 4 &&  IsExtensionSupported2((char*) "GL_EXT_framebuffer_multisample" ) && IsExtensionSupported2((char*) "GL_EXT_framebuffer_blit" )  )
	{
        enableMultiSample = true;
        v_printf(2, "  Multisampling capable\n", max_samples);
	}
#endif
#endif
    
    //Build first VBO
    firstVBO = new VBO2_t;
    firstVBO->numObjects = 0;
    firstVBO->vertbuffer = 0;
    firstVBO->indexbuffer = 0;
    firstVBO->next = NULL;
    firstVBO->prev = NULL;
    curVBO = firstVBO;
    
    //Build small renderqueue
    renderQueue = (renderQueue_t*)malloc(sizeof(renderQueue_t)*1024);
    queueLength = 0;
    queueMax = 1024;
}

void
Renderer::beginRender(FRUSTUM frustum)
{
    glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_NORMAL_ARRAY);
//    glEnable( GL_MULTISAMPLE );
    
    boundVBO = NULL;
	if(enableVBO)
	{
#ifdef GL_ARB_vertex_buffer_object
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
		glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
#endif
	}
	else
	{
		glVertexPointer (3, GL_FLOAT, sizeof(drawvert2_t), drawverts[0].vertex);
		glNormalPointer (GL_FLOAT, sizeof(drawvert2_t), drawverts[0].normal);
	}
    
#ifdef GL_ARB_shader_objects
    if(enableShaders)
        glUseProgramObjectARB(shaderProgram);
#endif
    
    queueLength = 0;
    
    // State
    glDisable(GL_BLEND);
    blending = false;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    cull_type = GL_BACK;
    cur_color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	modelview.LoadIdentity();
    this->frustum = frustum;
    
    if(wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
    //glDisable(GL_CULL_FACE);

	// Is this the place to set the states?
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FOG);
}

void
Renderer::endRender()
{
    // Empty Queue
    for(int i=0;i<queueLength;i++)
        renderObject(renderQueue[i].recipe, &renderQueue[i].mat , &renderQueue[i].color , false);
    queueLength = 0;
    
    // Disable states
//    glDisable( GL_MULTISAMPLE );
    boundVBO = NULL;
	if(enableVBO)
	{
#ifdef GL_ARB_vertex_buffer_object
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
		glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
#endif
	}
    
#ifdef GL_ARB_shader_objects
	if(enableShaders)
		glUseProgramObjectARB(0);
#endif
}

void
Renderer::setWireframe(bool enable)
{
    wireframe = enable;
}

bool				
Renderer::offlineFramebuffer(int width, int height)
{
	if(enableFBO)
	{
#ifdef GL_EXT_framebuffer_object
		if(enableMultiSample)
		{
			//Blit buffer
			glGenFramebuffersEXT(1, &FBO2);
			glGenRenderbuffersEXT(1, &RBO2_color);
            
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBO2_color);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB, width, height);
            
			if(glGetError()==GL_OUT_OF_MEMORY)
			{
				glDeleteFramebuffersEXT(1, &FBO2);
				glDeleteRenderbuffersEXT(1, &RBO2_color);
				v_printf(2, "Not enough memory for high quality screenshot.\n");
				enableFBO = false;
				return false;
			}
            
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO2);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, RBO2_color);
		}
        
		// Multisample buffer
		glGenFramebuffersEXT(1, &FBO);
		glGenRenderbuffersEXT(1, &RBO_depth);
		glGenRenderbuffersEXT(1, &RBO_color);
		
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBO_depth);
#ifdef GL_EXT_framebuffer_multisample
#ifdef GL_EXT_framebuffer_blit
		if(enableMultiSample)
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, 4, GL_DEPTH_COMPONENT, width, height);
		else
#endif
#endif
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
		if(glGetError()==GL_OUT_OF_MEMORY)
		{
			glDeleteFramebuffersEXT(1, &FBO2);
			glDeleteRenderbuffersEXT(1, &RBO2_color);
			glDeleteFramebuffersEXT(1, &FBO);
			glDeleteRenderbuffersEXT(1, &RBO_color);
			glDeleteRenderbuffersEXT(1, &RBO_depth);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			v_printf(2, "Not enough memory for high quality screenshot.\n");
			enableFBO = false;
			return false;
		}
        
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, RBO_color);
#ifdef GL_EXT_framebuffer_multisample
#ifdef GL_EXT_framebuffer_blit
		if(enableMultiSample)
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, 4, GL_RGBA, width, height);
		else
#endif
#endif
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, width, height);
		if(glGetError()==GL_OUT_OF_MEMORY)
		{
			glDeleteFramebuffersEXT(1, &FBO2);
			glDeleteRenderbuffersEXT(1, &RBO2_color);
			glDeleteFramebuffersEXT(1, &FBO);
			glDeleteRenderbuffersEXT(1, &RBO_color);
			glDeleteRenderbuffersEXT(1, &RBO_depth);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			v_printf(2, "Not enough memory for high quality screenshot.\n");
			enableFBO = false;
			return false;
		}
        
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, RBO_color);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, RBO_depth);
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
#endif
		return true;
	}
    
	return false;
}

void
Renderer::blitFramebuffer(int width, int height)
{
	if(enableFBO && enableMultiSample)
	{
#ifdef GL_EXT_framebuffer_object
#ifdef GL_EXT_framebuffer_multisample
#ifdef GL_EXT_framebuffer_blit
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, FBO);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, FBO2);
		glBlitFramebufferEXT(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO2);
#endif
#endif
#endif
	}
}

void				
Renderer::onlineFramebuffer()
{
	if(enableFBO)
	{
#ifdef GL_EXT_framebuffer_object
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDeleteFramebuffersEXT(1, &FBO);
		
		glDeleteRenderbuffersEXT(1, &RBO_depth);
		glDeleteRenderbuffersEXT(1, &RBO_color);
		if(enableMultiSample)
		{
			glDeleteFramebuffersEXT(1, &FBO2);
			glDeleteRenderbuffersEXT(1, &RBO2_color);
		}
#endif
	}
}

renderRecipe_t*
Renderer::beginObject()
{
	curRecipe = new renderRecipe_t;
	curRecipe->next = NULL;
	curRecipe->VBO = curVBO;
	curVBO->numObjects++;
	curRecipe->firstIndex = numIndices;
	curRecipe->displaylist = 0;
	curRecipe->numIndices = 0;
	curRecipe->bounds.SetFromMinsMaxes(VECTOR3D(10000.0f, 10000.0f, 10000.0f), VECTOR3D(-10000.0f, -10000.0f, -10000.0f));

	if(!enableVBO)
	{
		curRecipe->displaylist = glGenLists(1);
		glNewList(curRecipe->displaylist, GL_COMPILE); // Begin display list recording
	}
    
    return curRecipe;
}

unsigned int
Renderer::getCurIndex()
{
    return numDrawverts;
}

unsigned int
Renderer::addVertex(GLfloat x, GLfloat y, GLfloat z)
{
    if(numDrawverts >= Renderer_SIZE-1) {
		v_printf(1, "numDrawverts = %d \n", numDrawverts);
		forceFlush();
	}
	// Add to cache
    drawverts[numDrawverts].vertex[0] = x;
    drawverts[numDrawverts].vertex[1] = y;
    drawverts[numDrawverts].vertex[2] = z;	
    numDrawverts++;
    
    // Update bounding box
    if(x < curRecipe->bounds.mins.x)
        curRecipe->bounds.mins.x = x;
    if(y < curRecipe->bounds.mins.y)
        curRecipe->bounds.mins.y = y;
    if(z < curRecipe->bounds.mins.z)
        curRecipe->bounds.mins.z = z;
    if(x > curRecipe->bounds.maxes.x)
        curRecipe->bounds.maxes.x = x;
    if(y > curRecipe->bounds.maxes.y)
        curRecipe->bounds.maxes.y = y;
    if(z > curRecipe->bounds.maxes.z)
        curRecipe->bounds.maxes.z = z;
    curRecipe->bounds.SetFromMinsMaxes(curRecipe->bounds.mins, curRecipe->bounds.maxes);
    
    return numDrawverts-1;
}

void
Renderer::addTriangle(size_t v1, size_t v2, size_t v3)
{
	if ((numIndices + 2) > Renderer_SIZE*VERTEX_INDEX_RATIO)
		forceFlush();
	indices[numIndices+0] = v1;
    indices[numIndices+1] = v2;
    indices[numIndices+2] = v3;
    numIndices+=3;
    
    curRecipe->numIndices+=3;
}

void
Renderer::allowFlush()
{
    // Flush with 2K buffers for display list renderers (weird bug with icestm and Exceed 3D combination)
    if( (enableVBO && (numDrawverts > Renderer_SIZE-256 || numIndices > (Renderer_SIZE-256)*VERTEX_INDEX_RATIO)) || (!enableVBO && (numDrawverts > 2048-256 || numIndices > (2048-256)*VERTEX_INDEX_RATIO)))
    {
        // Upload VBO
        emitTriangles();
        
		if(enableVBO)
		{
			// String new recipe
			curRecipe->next = new renderRecipe_t;
			curRecipe = curRecipe->next;
			curRecipe->next = NULL;
			curRecipe->VBO = curVBO;
			curRecipe->displaylist = 0;
			curVBO->numObjects++;
			curRecipe->firstIndex= numIndices;
			curRecipe->numIndices = 0;
            curRecipe->bounds.SetFromMinsMaxes(VECTOR3D(10000.0f, 10000.0f, 10000.0f), VECTOR3D(-10000.0f, -10000.0f, -10000.0f));
		}
    }
}

void
Renderer::endObject()
{
    if(numDrawverts > Renderer_SIZE-256 || numIndices > (Renderer_SIZE-256)*VERTEX_INDEX_RATIO || (!enableVBO))
        emitTriangles();
    
	if(!enableVBO)
	{
		if(curRecipe)
			v_printf(2, "  Display list %d uploaded with %d triangles.\n", curRecipe->displaylist, curRecipe->numIndices/3);
		glEndList(); // End display list recording
	}
}

void                
Renderer::renderObject(renderRecipe_t *recipe, MATRIX4X4 *mat, VECTOR4D *color, bool transparent)
{
    AA_BOUNDING_BOX bounds;
    
    // Put to queue?
    if(transparent)
    {
        // Resize?
        if(queueLength+1 > queueMax)
        {
            queueMax+=1024;
            renderQueue = (renderQueue_t*)realloc(renderQueue, sizeof(renderQueue_t)*queueMax);
        }
        
        // Add to queue
        renderQueue[queueLength].recipe = recipe;
        renderQueue[queueLength].mat = *mat;
        renderQueue[queueLength].color = *color;
        queueLength++;
        return;
    }
    
    // Check bounding box
    bounds = recipe->bounds;
    bounds.Mult(*mat);
    if(frustum.IsAABoundingBoxInside(bounds))
    {
        //State
        if(modelview != *mat)
        {
            glLoadMatrixf((GLfloat*) mat);
            modelview = *mat;
            
            // Update cullface?
            bool t = mat->NegativeTrace();
            if(t && cull_type==GL_BACK)
            {
                glCullFace(GL_FRONT);
                cull_type = GL_FRONT;
            }
            else if (!t && cull_type==GL_FRONT)
            {
                glCullFace(GL_BACK);
                cull_type = GL_BACK;
            }
        }
        if(!blending && color->GetW() < 0.99f)
        {
            glEnable(GL_BLEND);
            blending = true;
        }
        else if(blending && color->GetW() >= 0.99f)
        {
            glDisable(GL_BLEND);
            blending = false;
        }
        
        // Bind geometry?
        if(recipe->VBO != boundVBO && enableVBO)
        {
#ifdef GL_ARB_vertex_buffer_object
            // Bind buffers
            glBindBufferARB( GL_ARRAY_BUFFER_ARB, recipe->VBO->vertbuffer );
            glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, recipe->VBO->indexbuffer );
            glVertexPointer (3, GL_FLOAT, sizeof(drawvert2_t), (char*) NULL);
            glNormalPointer (GL_FLOAT, sizeof(drawvert2_t), (char*) (3*sizeof(GLfloat)));
            boundVBO = recipe->VBO;
#endif
        }
        
        // Render geometry
        if(*color != cur_color)
        {
            glColor4f(color->GetX(), color->GetY(), color->GetZ(), color->GetW());
            cur_color = *color;
        }
        
        if(enableVBO)
        {
            // Draw the triangles
            glDrawElements(GL_TRIANGLES, recipe->numIndices, GL_UNSIGNED_SHORT, (char *) NULL+recipe->firstIndex*sizeof(GLushort));
        }
        else
            glCallList(recipe->displaylist);
        
        total_tris += recipe->numIndices / 3;
        
    }
    
    // Next batch
    if(recipe->next)
        renderObject(recipe->next, mat, color, transparent);
}

void
Renderer::forceFlush()
{
    emitTriangles();
}

void
Renderer::deleteRecipe(renderRecipe_t *recipe)
{
    if(recipe->next)
        deleteRecipe(recipe->next);
    
    if(recipe->VBO)
    {
        recipe->VBO->numObjects--;
        if(recipe->VBO->numObjects<1)
            deleteVBO(recipe->VBO);
    }
	if(recipe->displaylist)
		glDeleteLists(recipe->displaylist, 1);
    
    delete recipe;
}

void				
Renderer::start2D(int width, int height)
{
	endRender(); // flush all 3D buffers and stuff

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, (GLdouble) width, 0.0, (GLdouble) height, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void
Renderer::drawSquare(float x1, float y1, float x2, float y2, int filled, VECTOR4D color)
{
	glColor4f(color.x, color.y, color.z, color.w);
	if (filled) {
		glBegin(GL_QUADS);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glEnd();
	}
	else {
		glBegin(GL_LINES);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(x1, y2);
		glVertex2f(x1, y1);
		glEnd();
	}
}

void
Renderer::drawLines(unsigned int numLines, float *x_coords, float *y_coords, VECTOR4D color)
{
	glColor4f(color.x, color.y, color.z, color.w);
	glBegin(GL_LINES);
	for(unsigned int i=0; i<numLines; i++)
		glVertex2f(x_coords[i], y_coords[i]);
	glEnd();
}

// saves an array of pixels as a TGA image
int
Renderer::tgaSave( char* filename,  short int width, short int height, unsigned char	pixelDepth,unsigned char* imageData)
{
    
	unsigned char cGarbage = 0, type,mode,aux;
	short int iGarbage = 0;
	int i;
	FILE *file;
    
    // open file and check for errors
	file = fopen(filename, "wb");
	if (file == NULL) {
		return(0);
	}
    
    // compute image type: 2 for RGB(A), 3 for greyscale
	mode = pixelDepth / 8;
	if ((pixelDepth == 24) || (pixelDepth == 32))
		type = 2;
	else
		type = 3;
    
    // write the header
	fwrite(&cGarbage, sizeof(unsigned char), 1, file);
	fwrite(&cGarbage, sizeof(unsigned char), 1, file);
    
	fwrite(&type, sizeof(unsigned char), 1, file);
    
	fwrite(&iGarbage, sizeof(short int), 1, file);
	fwrite(&iGarbage, sizeof(short int), 1, file);
	fwrite(&cGarbage, sizeof(unsigned char), 1, file);
	fwrite(&iGarbage, sizeof(short int), 1, file);
	fwrite(&iGarbage, sizeof(short int), 1, file);
    
	fwrite(&width, sizeof(short int), 1, file);
	fwrite(&height, sizeof(short int), 1, file);
	fwrite(&pixelDepth, sizeof(unsigned char), 1, file);
    
	fwrite(&cGarbage, sizeof(unsigned char), 1, file);
    
    // convert the image data from RGB(a) to BGR(A)
	if (mode >= 3)
        for (i=0; i < width * height * mode ; i+= mode) {
            aux = imageData[i];
            imageData[i] = imageData[i+2];
            imageData[i+2] = aux;
        }
    
    // save the image data
	fwrite(imageData, sizeof(unsigned char),
           width * height * mode, file);
	fclose(file);
    // release the memory
	free(imageData);
    
	return(1);
}

// saves a series of files with names "filenameX.tga"
int
Renderer::tgaSaveSeries(char* filename, short int width, short int height, unsigned char	pixelDepth,unsigned char* imageData)
{
	char *newFilename;
	int status;
	
    // compute the new filename by adding the
    // series number and the extension
	newFilename = (char *)malloc(sizeof(char) * strlen(filename)+8);
    
	sprintf(newFilename,"%s%d.tga",filename,savedImages);
	
    // save the image
	status = tgaSave(newFilename,width,height,pixelDepth,imageData);
	
    //increase the counter
	if (status == 1)
	{
		savedImages++;
		v_printf(1, "Screenshot %s saved at %dx%d\n", newFilename, width, height);
	}
	else
		v_printf(1, "Screenshot saving failed\n");
	free(newFilename);
	return(status);
}

// takes a screen shot and saves it to a TGA image
int
Renderer::tgaGrabScreenSeries(char *filename)
{
	
	int w, h;
	unsigned char *imageData;
	GLint params[4];
	int xmin, ymin, xmax, ymax;
    
	glGetIntegerv(GL_VIEWPORT, (GLint*)params);
	xmin = params[0]; ymin=params[1];
	xmax = params[0]+params[2]; ymax = params[1]+params[3];
    
    // compute width and heidth of the image
	w = xmax - xmin;
	h = ymax - ymin;
    
    // allocate memory for the pixels
	imageData = (unsigned char *)malloc(sizeof(unsigned char) * w * h * 3);
    
    // read the pixels from the frame buffer
	glReadPixels(xmin,ymin,xmax,ymax,GL_RGB,GL_UNSIGNED_BYTE, (GLvoid *)imageData);
    
    // save the image 
	return(tgaSaveSeries(filename,w,h,24,imageData));
}

