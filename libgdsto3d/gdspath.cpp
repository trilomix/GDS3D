//  GDS3D, a program for viewing GDSII files in 3D.
//  Created by Jasper Velner and Michiel Soer, IC-Design Group, University of Twente: http://icd.el.utwente.nl
//
//  Copyright (C) 2017 Bertrand Pigeard
//  Copyright (C) 2013 IC-Design Group, University of Twente.
//
//  Based on gds2pov by Roger Light, http://atchoo.org/gds2pov/ / https://github.com/ralight/gds2pov
//  Copyright (C) 2004-2008 by Roger Light
//
/*
 * File: gdspath.cpp
 * Author: Roger Light
 * Project: gdsto3d
 *
 * This is the GDSPath class which is used to represent the GDS path type.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef WIN32
#include <algorithm>
#endif

#include "gdsobject.h"
#include "gdspath.h"

GDSPath::GDSPath(int Type, float Height, float Thickness, unsigned int Points, float Width, float BgnExtn, float EndExtn, struct ProcessLayer *Layer)
{
	_Type = Type;
	//_Coords = new char[Points];
	_Coords.clear();
	_Height = Height;
	_Thickness = Thickness;
	_Points = Points;
	_Width = Width;
	_BgnExtn = BgnExtn;
	_EndExtn = EndExtn;
	_Layer = Layer;
}

GDSPath::~GDSPath()
{
	//if(_Coords) delete [] _Coords;
}

void GDSPath::AddPoint(unsigned int Index, float X, float Y)
{
	if(_Points >= Index){
		_Coords.push_back(Point2D(X, Y));
	}
}


void GDSPath::SetRotation(float X, float Y, float Z)
{
	_Rotate.X = X;
	_Rotate.Y = Y;
	_Rotate.Z = Z;
}

float GDSPath::GetXCoords(unsigned int Index)
{
	return _Coords[Index].X;
}

float GDSPath::GetYCoords(unsigned int Index)
{
	return _Coords[Index].Y;
}

unsigned int GDSPath::GetPoints()
{
	return _Points;
}

float GDSPath::GetHeight()
{
	return _Height;
}

float GDSPath::GetThickness()
{
	return _Thickness;
}

float GDSPath::GetWidth()
{
	return _Width;
}

float GDSPath::GetBgnExtn()
{
	return _BgnExtn;
}

float GDSPath::GetEndExtn()
{
	return _EndExtn;
}

int GDSPath::GetType()
{
	return _Type;
}

struct ProcessLayer *GDSPath::GetLayer()
{
	return _Layer;
}

vector<Point2D> GDSPath::create_shifted_points(float start_ext, float end_ext, int ncircle) {
	vector<Point2D> Coord;
	vector<Point2D> Coord2;
	Coord = create_shifted_points(start_ext, end_ext, _Width*2.0, true, 0, GetPoints()-1, ncircle);
	Coord2 = create_shifted_points(start_ext, end_ext, _Width*2.0, false, GetPoints()-1, 0, ncircle);
	for (size_t i = 0; i < Coord2.size(); i++) {
		Coord.push_back(Coord2[i]);
	}
	return Coord;
}

vector<Point2D> GDSPath::create_shifted_points(float start, float end, float width, bool forward, size_t from, size_t to, int ncircle)
{
	vector<Point2D> Coord;
	Point2D pp;
	//  for safety reasons
	if (from == to) {
		return Coord;
	}

	double disp = double(width) * 0.5;
	size_t index= from;

	Point2D p = _Coords[index];
	if(from < to) {
		pp = _Coords[index + 1];
		index++;
	} else {
		pp = _Coords[index - 1];
		index--;
			}
	//++pp;

	if (pp == _Coords[to]) {

		//  Special case of degenerated path with one point: treat as infinitly small segment with direction (1,0)
		//Point2D ed(forward ? 1.0 : -1.0, 0.0);
		Point2D ed(pp - p);
		ed = ed / ed.double_distance();
		Point2D nd(-ed.Y, ed.X);
		nd = nd.dpx(disp);
		//Point2D nd(-ed.Y, ed.X);
		Point2D edd, ndd;

		//  The first point is taken as being simply shifted normally and pulled back by start_ext
		//  or, in round mode, approximated by a set of segments 
		if (ncircle > 2) {

			double a0 = M_PI / (2.0 * ncircle);

			double cd = cos(a0);
			double sd = sin(a0);
			double c2d = cd * cd - sd * sd;
			double s2d = 2 * cd * sd;
			double c, s;

			edd = ed * (double(-start) / cd);
			//ndd = nd * (disp / cd);
			ndd = nd * (1.0 / cd);

			c = cd;
			s = sd;

			for (int i = 0; i < ncircle / 2; ++i) {
				//*pts++ = *p + point<C>::from_double(edd * c + ndd * s);
				Coord.push_back(p + (edd * c + ndd * s));
				double cc = c * c2d - s * s2d;
				double ss = s * c2d + c * s2d;
				c = cc;
				s = ss;
			}

			edd = ed * (double(end) / cd);
			//ndd = nd * (disp / cd);
			ndd = nd * (1.0 / cd);

			c = cos(a0 * (ncircle - 1));
			s = sin(a0 * (ncircle - 1));

			for (int i = 0; i < ncircle / 2; ++i) {
				//*pts++ = *p + point<C>::from_double(edd * c + ndd * s);
				Coord.push_back(pp + (edd * c + ndd * s));
				double cc = c * c2d + s * s2d;
				double ss = s * c2d - c * s2d;
				c = cc;
				s = ss;
			}

		}
		else {
			//*pts++ = (*p + point<C>::from_double(ed * double(-start) + nd * disp));
			Coord.push_back(p + (ed * -start + nd * 1.0));
			//*pts++ = (*p + point<C>::from_double(ed * double(end) + nd * disp));
			Coord.push_back(pp + (ed * end + nd * 1.0));
		}

	}

	bool first = true;
	size_t indexbis = index;
	while (pp != _Coords[to]) {
		Point2D ppp;

		if (from < to) {
			indexbis++;
			ppp = _Coords[indexbis];
			
			while (ppp == pp) {
				indexbis++;
				ppp = _Coords[indexbis];
			}
		} else {
			indexbis--;
			ppp = _Coords[indexbis];
			
			while (ppp == pp) {
				indexbis--;
				ppp = _Coords[indexbis];
			}
		}
		//Point2D ppp = pp;
		//++ppp;

		//  Compute the unit vector of the line and it's normal (times width)

		Point2D ed(pp - p);
		ed = ed / ed.double_distance();
		Point2D nd(-ed.Y, ed.X);
		nd = nd.dpx(disp);

		if (first) {

			first = false;

			//  The first point is taken as being simply shifted normally and pulled back by start_ext
			//  or, in round mode, approximated by a set of segments 
			if (ncircle > 2) {

				double a0 = M_PI / (2.0 * ncircle);

				double cd = cos(a0);
				double sd = sin(a0);
				double c2d = cd * cd - sd * sd;
				double s2d = 2 * cd * sd;
				double c = cd, s = sd;

				Point2D edd = ed * (-start / cd);
				Point2D ndd = nd * (1.0 / cd);

				for (int i = 0; i < ncircle / 2; ++i) {
					//*pts++ = *p + point<C>::from_double(edd * c + ndd * s);
					Coord.push_back(p + (edd * c + ndd * s));
					double cc = c * c2d - s * s2d;
					double ss = s * c2d + c * s2d;
					c = cc;
					s = ss;
				}

			}
			else {
				//*pts++ = (*p + point<C>::from_double(dpx<C>(ed, -start) + nd));
				Coord.push_back(p + (ed.dpx(-start) + nd));
			}

		}

		

		//  Points in between are determined from taking two 
		//  edges being shifted perpendicular from the orginal
		//  and being slightly extended. The intersection point
		//  of both gives the new vertex. If there is no intersection,
		//  the edges are simply connected.

		Point2D eed(ppp - pp);
		eed = eed * 1.0 / eed.double_distance();
		Point2D nnd(-eed.Y, eed.X);
		nnd = nnd.dpx(disp);

		float l1max = nd.double_distance();
		float l2max = nnd.double_distance();

		float l1min = -p.double_distance(pp) - nd.double_distance();
		float l2min = -ppp.double_distance(pp) - nnd.double_distance();

		float dv = vprod(ed, eed);
		if (fabs(dv) > EPSILONPOINT / 1.0e10) {

			float l1 = vprod(nnd - nd, eed) / dv;
			float l2 = vprod(nd - nnd, ed) / dv;

			if ((l1 < -EPSILONPOINT / 1.0e10) != (l2 < -EPSILONPOINT / 1.0e10)) {

				//  No well-formed intersection (reflecting edge) ->
				//  create a direct connection
				//*pts++ = *pp + point<C>::from_double(nd);
				Coord.push_back(pp + nd);
				//*pts++ = *pp + point<C>::from_double(nnd);
				//Coord.push_back(pp + nnd);
			}
			else if (l1 < l1min - EPSILONPOINT / 1.0e10 || l2 < l2min - EPSILONPOINT / 1.0e10) {

				//  Segments are too short - the won't intersect: In this case we create a loop of three 
				//  points which define the area in self-overlapping way but confined to the path within
				//  the limits of it's width. 
				//  HINT: the execution of this code is a pretty strong evidence for the existance to loops 
				//  in the contour delivered. A proof however is missing ..
				//*pts++ = *pp + point<C>::from_double(nd);
				Coord.push_back(pp + nd);
				//*pts++ = *pp;
				Coord.push_back(pp);
				//*pts++ = *pp + point<C>::from_double(nnd);
				Coord.push_back(pp + nnd);
			}
			else if (l1 < l1max + EPSILONPOINT / 1.0e10 && l2 < l2max + EPSILONPOINT / 1.0e10) {

				//  well-formed corner
				//*pts++ = *pp + point<C>::from_double(nd + ed * l1);
				Coord.push_back(pp + nd + ed * l1);
			}
			else {

				//  cut-off corner: produce two points connecting the edges 
				//*pts++ = *pp + point<C>::from_double(nd + ed * std::min(l1max, l1));
				Coord.push_back(pp + nd + ed * std::min(l1max, l1));
				//*pts++ = *pp + point<C>::from_double(nnd - eed * std::min(l2max, l2));
				Coord.push_back(pp + nnd - eed * std::min(l2max, l2));
			}

			p = pp;

		}
		else if (sprod(ed, eed) < -EPSILONPOINT / 1.0e10) {

			//  reflecting segment
			//*pts++ = *pp + point<C>::from_double(nd + dpx<C>(ed, disp));
			Coord.push_back(pp + nd + ed.dpx(disp));
			//*pts++ = *pp + point<C>::from_double(nnd - dpx<C>(eed, disp));
			Coord.push_back(pp + nnd - eed.dpx(disp));

			p = pp;

		}

		

		if (ppp == _Coords[to]) {

			//  The last point is taken as being simply shifted normally and pulled forward by end_ext
			//  or, in round mode, approximated by a set of segments 
			if (ncircle > 2) {

				double a0 = M_PI / (2.0 * ncircle);

				double cd = cos(a0);
				double sd = sin(a0);
				double c2d = cd * cd - sd * sd;
				double s2d = 2 * cd * sd;
				double c = cos(a0 * (ncircle - 1));
				double s = sin(a0 * (ncircle - 1));

				Point2D edd = eed * (end / cd);
				Point2D ndd = nnd * (1.0 / cd);

				for (int i = 0; i < ncircle / 2; ++i) {
					//*pts++ = *pp + point<C>::from_double(edd * c + ndd * s);
					Coord.push_back(ppp + (edd * c + ndd * s));
					double cc = c * c2d + s * s2d;
					double ss = s * c2d - c * s2d;
					c = cc;
					s = ss;
				}

			}
			else {
				//*pts++ = (*pp + point<C>::from_double(dpx<C>(ed, end) + nd));
				Coord.push_back(ppp + (eed.dpx(end) + nnd));
			}

		} 

		pp = ppp;

	}
	return Coord;
}
