#include "../gdsoglviewer/geopolygon.h"
#include "../gdsoglviewer/outputStream.h"

class VoronoiElement {

};


class clip{
private:
	vector<Point3D> vertices;
	vector<GoePolyPoint> MeshPoints;
	size_t z_div;
	htime* time;
	double Waittime;

public:
  clip();
  ~clip();
  void AddPoly(GeoPolygon * poly, bool recurse);
  void execute(vector<GDSGroup*> FullGDSItems);
  void execute(vector<GeoPolygon*> poly);
  void execute();
  //void execute_in(vector<Point3D> vertices);
  int category(int,int,int,int);
};
