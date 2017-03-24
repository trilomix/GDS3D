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

#include "windowmanager.h"
#include "gdsparse_ogl.h"
#include "ui_ruler.h"

UIRuler::UIRuler()
{
    Disable();
	rulerlayer = 0;
	rulerlayer = -1; // Reset to top layer next time ruler is enabled
	rulerdatatype = -1; 
}

bool
UIRuler::GetState()
{
	return rulerstate>0;
}

void
UIRuler::Disable()
{
    rulerstate = 0;
	ruler1x = ruler1y = ruler2x = ruler2y = 0.0;
}

void 
UIRuler::Reset()
{
	Disable();
	rulerlayer = -1; // Reset to top layer next time ruler is enabled
	rulerdatatype = -1;
}

void
UIRuler::Draw()
{
	ProcessLayer *layer = wm->getProcess()->GetLayer(rulerlayer, rulerdatatype, wm->getProcess()->GetCurrentProcess());

    glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_FOG);
    
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	float length = 0.0;
	float extension = 50.0;
	float rulerz=0;
	if (layer) {
		//rulerz = (layer->Height + layer->Height*1.5f*exploded_fraction + layer->Thickness) / 1000.0f;
		rulerz = (layer->Height + layer->Height*1.5f*exploded_fraction + layer->Thickness) * layer->Units->Unitu;
	}

	glLineWidth(3.0);
    
	if(rulerstate == 2 || rulerstate == 3)
	{
		glEnable(GL_DEPTH_TEST);
        
		if(fabs(ruler1x - ruler2x) > fabs(ruler1y - ruler2y))
		{
			length = fabs(ruler1x - ruler2x);
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_LINES);
			glVertex3f(ruler1x, ruler1y, rulerz+0.01f); // Add a little offset to keep the line on top
			glVertex3f(ruler2x, ruler1y, rulerz+0.01f);
            
			glVertex3f(ruler1x, ruler1y-wm->getWorld()->_z/extension, rulerz+0.01f);
			glVertex3f(ruler1x, ruler1y+wm->getWorld()->_z/extension, rulerz+0.01f);
			glVertex3f(ruler2x, ruler1y-wm->getWorld()->_z/extension, rulerz+0.01f);
			glVertex3f(ruler2x, ruler1y+wm->getWorld()->_z/extension, rulerz+0.01f);
			glEnd();
		}
		else if(fabs(ruler1x - ruler2x) < fabs(ruler1y - ruler2y))
		{
			length = fabs(ruler1y - ruler2y);
            
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_LINES);
			glVertex3f(ruler1x, ruler1y, rulerz);
			glVertex3f(ruler1x, ruler2y, rulerz);
            
			glVertex3f(ruler1x-wm->getWorld()->_z/extension, ruler1y, rulerz);
			glVertex3f(ruler1x+wm->getWorld()->_z/extension, ruler1y, rulerz);
			glVertex3f(ruler1x-wm->getWorld()->_z/extension, ruler2y, rulerz);
			glVertex3f(ruler1x+wm->getWorld()->_z/extension, ruler2y, rulerz);
			glEnd();
		}
        
	}
    
	glLineWidth(2.0);
    
	//if(_rulerstate == 1)
	if(rulerlayer>=0 && rulerstate>0)
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1,-1); // Bring closer
		glColor4f(1.0f, 0.0f, 0.0f, 0.33f);
		float extra = 0.05f;
        glEnable(GL_BLEND);
        
		glBegin(GL_TRIANGLES);

		float _xmin = wm->getWorld()->_xmin;
		float _ymin = wm->getWorld()->_ymin;
		float _xmax = wm->getWorld()->_xmax;
		float _ymax = wm->getWorld()->_ymax;
        
		glVertex3f(_xmin-(_xmax-_xmin)*extra, _ymin-(_ymax-_ymin)*extra, rulerz);
		glVertex3f(_xmin-(_xmax-_xmin)*extra, _ymax+(_ymax-_ymin)*extra, rulerz);
		glVertex3f(_xmax+(_xmax-_xmin)*extra, _ymax+(_ymax-_ymin)*extra, rulerz);
        
		glVertex3f(_xmax+(_xmax-_xmin)*extra, _ymax+(_ymax-_ymin)*extra, rulerz);
		glVertex3f(_xmax+(_xmax-_xmin)*extra, _ymin-(_ymax-_ymin)*extra, rulerz);
		glVertex3f(_xmin-(_xmax-_xmin)*extra, _ymin-(_ymax-_ymin)*extra, rulerz);
        
		glEnd();
        
        glDisable(GL_BLEND);
        
		glDisable(GL_POLYGON_OFFSET_FILL);
        
	}
    
	//if(length>0.0)
	if(rulerlayer >= 0  && rulerstate>0)
	{
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		//glPushMatrix();
        
		glLoadIdentity();
		glOrtho(0.0, (GLdouble) wm->screenWidth, 0.0, (GLdouble) wm->screenHeight, -1.0f, 1.0f);
        
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
        
		// Draw border
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
		wm->getWorld()->gl_square(wm->screenWidth - 300.0f,wm->screenHeight - 90.0f, wm->screenWidth - 20.0f, wm->screenHeight - 160.0f, 1);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		wm->getWorld()->gl_square(wm->screenWidth - 300.0f, wm->screenHeight - 90.0f, wm->screenWidth - 20.0f, wm->screenHeight - 160.0f, 0);
        
		// Text
        if(layer){
		if(length > 0.0)
		{
			if(length<1.0)
				wm->getWorld()->gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 280, wm->screenHeight - 110, "Ruler(%s): %4.0fnm",layer->Name, length*1000);
			else
				wm->getWorld()->gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 280, wm->screenHeight - 110, "Ruler(%s): %5.1fum",layer->Name, length);
		}
		else
		{
			wm->getWorld()->gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 280, wm->screenHeight - 110, "Ruler(%s): ",layer->Name);
		}
        }
		wm->getWorld()->gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 280, wm->screenHeight - 130, "Left-Click on Plane");
		wm->getWorld()->gl_printf(1.0f, 1.0f, 1.0f, 0.4f, wm->screenWidth - 280, wm->screenHeight - 150, "CTRL-Scroll: Change Layer");

	}
    
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FOG);
}

bool
UIRuler::Event(int event, int data, int xpos, int ypos , bool shift, bool control, bool alt)
{
    float ray_x=0;
	float ray_y;
	float ray_z;
	ProcessLayer *layer = wm->getProcess()->GetLayer(rulerlayer, rulerdatatype, wm->getProcess()->GetCurrentProcess());

	if( event == EVENT_KEY_UP ) // Key up
	{
		switch(data)
		{
		case KEY_K:
			if(rulerstate == 0)
			{
				if(rulerlayer < 0)
				{
					ProcessLayer *layer = wm->getProcess()->GetLayerProcess(wm->getProcess()->GetCurrentProcess());
					while(layer)
					{
						if (layer->Show) {
							rulerlayer = layer->Layer;
							wm->getProcess()->SetCurrentProcess(layer->ProcessName);
						}
						layer = layer->Next;
					}	
				}
				rulerstate = 1;

				wm->getWorld()->LockOnUIElement(this);
			}
			else
				rulerstate = 0;
			return true;		
		}
     }

	// Not enabled? -> exit
	if(rulerstate == 0)
		return false;
    
    if(rulerlayer >= 0)
	{
		VECTOR3D ray = VECTOR3D(-((float)wm->screenWidth)/((float)wm->screenHeight)*(((float)xpos)/((float)wm->screenWidth)-0.5f), 1.0f*(((float)ypos)/((float)wm->screenHeight)-0.5f), 1.07f); // HACK!!
		ray.Normalize(); // Is dit nodig?
		MATRIX4X4 view = worldview.GetInverse();
		VECTOR3D dir = view.GetRotatedVector3D(ray);
		//rulerz = (layer->Height+layer->Height*1.5f*exploded_fraction + layer->Thickness) / 1000.0f;
		rulerz = (layer->Height + layer->Height*1.5f*exploded_fraction + layer->Thickness) * layer->Units->Unitu;
        
		ray_x = wm->getWorld()->_x-(wm->getWorld()->_z-rulerz)/dir.z*dir.x;
		ray_y = wm->getWorld()->_y-(wm->getWorld()->_z-rulerz)/dir.z*dir.y;
		ray_z = wm->getWorld()->_z-rulerz;
	}
    
    if( event == EVENT_BUTTON_UP ) // mouse button up 
	{
        if(data == 0 && wm->getWorld()->_first_move)
		{
            
			//if(!_mouse_control)
			{
                
				{
					if(rulerlayer >= 0)
					{
						switch(rulerstate)
						{
                            case 1:
                            case 3:
                                if(ray_z > 0.0)
                                {
                                    rulerstate = 2;
                                    ruler1x = ray_x;
                                    ruler1y = ray_y;
                                    ruler2x = ray_x;
                                    ruler2y = ray_y;
                                }
                                break;
								//wm->getWorld()->_first_move = false;
								//return true;
                            case 2:
                                if(ray_z > 0.0)
                                {
                                    ruler2x = ray_x;
                                    ruler2y = ray_y;
                                    rulerstate = 3;
                                }
                                break; 
								//wm->getWorld()->_first_move = false;
								//return true;
						}
					}
                    
				}
			}
			wm->getWorld()->_first_move = false;
		}
    }
    
    if( event == EVENT_MOUSE_MOVE) // mouse move ruler?
	{
        
		if(rulerstate == 2)
		{
			ruler2x = ray_x;
			ruler2y = ray_y;
		}
	}

	if( event == EVENT_BUTTON_DOWN ) /* mouse button down */
	{
        if ( data == 2 ) {// Mouse wheel down
			if (control && !alt) 
			{ 
				ProcessLayer *layer = wm->getProcess()->GetLayer(rulerlayer, rulerdatatype, wm->getProcess()->GetCurrentProcess());
				if(layer)
					layer = layer->Next;
				while(layer)
				{
					if(layer->Show)
					{
						rulerlayer = layer->Layer;
						rulerdatatype = layer->Datatype;
						wm->getProcess()->SetCurrentProcess(layer->ProcessName);
						break;
					}	
					layer = layer->Next;
				}
				return true;
			}
		}
		if ( data == 3 ) {// Mouse wheel up
			if (control && !alt) 
			{ 
				
				ProcessLayer *curlayer = wm->getProcess()->GetLayer(rulerlayer, rulerdatatype, wm->getProcess()->GetCurrentProcess());
				ProcessLayer *layer = wm->getProcess()->GetLayer(); // First layer
				while(layer)
				{
					if(layer == curlayer)
						break;
					if (layer->Show) {
						rulerlayer = layer->Layer; 
						rulerdatatype = layer->Datatype;
						wm->getProcess()->SetCurrentProcess(layer->ProcessName);
					}
					layer = layer->Next;
				}
				return true;
			}
		}
	}
    
    return false;
}
