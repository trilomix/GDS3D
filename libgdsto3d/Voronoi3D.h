// Updates for GDS3D
// Copyright (C) 2017 Bertrand Pigeard
//
// Gmsh - Copyright (C) 1997-2017 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@onelab.info>.
//
// Contributor(s):
//   Tristan Carrier

#include "../gdsoglviewer/geopolygon.h"
#include "../gdsoglviewer/outputStream.h"

class VoronoiElement {

};


class clip{
private:
	//vector<Point3D> vertices;
	//vector<GoePolyPoint> MeshPoints;
	//size_t z_div;
	htime* time;
	double Waittime;

public:
  clip();
  ~clip();
  void AddPoly(GeoPolygon * poly, bool recurse, vector<Point3D>* vertices, vector<GoePolyPoint>* MeshPoints);
  void execute(vector<GDSGroup*> FullGDSItems);
  void execute(vector<GeoPolygon*> poly);
  void execute(vector<Point3D> vertices, vector<GoePolyPoint> MeshPoints, size_t z_div, layers * CurLayer);
  int category(int,int,int,int);
};
