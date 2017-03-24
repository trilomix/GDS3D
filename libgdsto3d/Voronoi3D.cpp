
#include "Voronoi3D.h"
#include <fstream>
#include "voro++/src/voro++.hh"
#include "gdselements.h"

using namespace voro;

/*********class clip*********/

clip::clip(){
	Waittime = 1;
	time = wm->new_timer();
}

clip::~clip(){}

void clip::AddPoly(GeoPolygon* poly, bool recurse) {
	for (size_t i = 0; i<poly->GetPoints(); i++) {
		Point2D p = poly->GetCoords(i);
		vertices.push_back(Point3D(p.X, p.Y, poly->GetHeight()));
		vertices.push_back(Point3D(p.X, p.Y, poly->GetHeight() + poly->GetThickness()));
		GoePolyPoint Poly_P;
		Poly_P.poly = poly;
		Poly_P.index = i;
		MeshPoints.push_back(Poly_P);
	}
	if (recurse && poly->GetHoles().size()>0) {
		for (size_t j = 0; j < poly->GetHoles().size(); j++) {
			GeoPolygon* poly_hole = poly->GetHoles()[j];
			AddPoly(poly_hole, recurse);
		}
	}
}

void clip::execute(vector<GDSGroup*> FullGDSItems){
	size_t layer_cnt = 0;
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		GDSGroup *cur_GDS = FullGDSItems[k];
		for (size_t i = 0; i < cur_GDS->OutLinesItems.size(); i++)
		{
			GeoPolygon *poly = cur_GDS->OutLinesItems[i];
			AddPoly(poly, true);
			layer_cnt++;
		}
	}
	z_div = layer_cnt+1;
	execute();
}

void clip::execute(vector<GeoPolygon*> polyList) {
	//vector<Point3D> vertices;
	//vector<GoePolyPoint> MeshPoints;
	set<double> z_list;
	for (size_t i = 0; i<polyList.size(); i++) {
		GeoPolygon *poly = polyList[i];
		AddPoly(poly,false);
		double z = poly->GetHeight();
		z_list.insert(z);
	}
	z_div = z_list.size()+1;
	execute();
}

void clip::execute()
{
  size_t i;
  size_t j;
  size_t count;
  int pid;
  double x,y,z;
  double delta;
  double min_x,max_x;
  double min_y,max_y;
  double min_z,max_z;
  voronoicell_neighbor cell;
  std::vector<int> faces;
  std::vector<double> voronoi_vertices;
  std::vector<voronoicell_neighbor*> pointers;
  std::vector<Point3D> generators;
  std::vector<int> IDs;
  std::vector<int> IDs2;
  std::vector<int> neighbors;
  std::vector<std::vector<std::vector<int> > > bisectors;

  // init timer
  wm->timer(time, 1);
  min_x = 1000000000.0;
  max_x = -1000000000.0;
  min_y = 1000000000.0;
  max_y = -1000000000.0;
  min_z = 1000000000.0;
  max_z = -1000000000.0;
  for(i=0;i<vertices.size();i++){
    min_x = min(vertices[i].X,min_x);
	max_x = max(vertices[i].X,max_x);
	min_y = min(vertices[i].Y,min_y);
	max_y = max(vertices[i].Y,max_y);
	min_z = min(vertices[i].Z,min_z);
	max_z = max(vertices[i].Z,max_z);
  }

  delta = 0.2*(max_x - min_x);
  //container cont(min_x-delta,max_x+delta,min_y-delta,max_y+delta,min_z-delta,max_z+delta,6,6,6,false,false,false,vertices.size());
  size_t nb_points_per_z = vertices.size() / z_div;
  size_t nb__blocs_par_z = nb_points_per_z / 5;
  double nb__slice_xy = sqrt(nb__blocs_par_z);
  double ratio_xbyy = (max_x- min_x)/(max_y- min_y);
  size_t x_div = max(nb__slice_xy*ratio_xbyy,2.0);
  size_t y_div = max(nb__slice_xy/ratio_xbyy,2.0);
  container cont(min_x - delta, max_x + delta, min_y - delta, max_y + delta, min_z - delta, max_z + delta, x_div, y_div, z_div, false, false, false, 8);

  for(i=0;i<vertices.size();i++){
    cont.put(i,vertices[i].X,vertices[i].Y,vertices[i].Z);
  }

  count = 0;
  IDs.resize(vertices.size());
  // First Loop to fill table
  c_loop_all loop(cont);
  loop.start();
  do{
	loop.pos(x,y,z);
	generators.push_back(Point3D(x,y,z));
	pid = loop.pid();
	IDs[pid] = count;
	IDs2.push_back(pid);
	count++;
  }while(loop.inc());

  //second Loop to compute
  count = 0;
  loop.start();
  do {
	  cont.compute_cell(cell, loop);
	  cell.neighbors(neighbors);
	  pid = loop.pid();
	  assert(IDs[pid] == count);
	  
	  //v_printf(1, "%2d %2d", counti, IDs2[count]);
	  //v_printf(1, "(%2.2g, %2.2g, %2.2g)", generators[count].X, generators[count].Y, generators[count].Z);
	  //v_printf(1, "\n");
	  for (j = 0; j < neighbors.size(); j++) {
		  if (neighbors[j] >= 0) {
			  //v_printf(1, " %2d", neighbors[j]);
			  //v_printf(1, "(%2.2g, %2.2g, %2.2g)", generators[IDs[neighbors[j]]].X, generators[IDs[neighbors[j]]].Y, generators[IDs[neighbors[j]]].Z);

			  GoePolyPoint Poly_P = MeshPoints[IDs2[count] / 2];
			  GoePolyPoint Poly_P_Neighbor = MeshPoints[neighbors[j] / 2];
			  CoordMeshElemSize curPoint = Poly_P.poly->GetMeshCoords(Poly_P.index);
			  // check unicity
			  bool found = false;
			  for (size_t k = 0; k < curPoint.Neighbors.size(); k++) {
				  if (curPoint.Neighbors[k] == Poly_P_Neighbor) {
					  found = true;
					  break;
				  }
			  }
			  if (!found)
				  Poly_P.poly->AddPointNeighbor(Poly_P.index, Poly_P_Neighbor);
		  }

	  }
	  // Check timer?
	  if (wm->timer(time, 0) > Waittime) {
		  v_printf(0, "\r                                                               ");
		  v_printf(0, "\rCompute Voronoi Done %5.2f%% (%zd/%zd)", 1.0*(count) / IDs.size()*100.0, count, IDs.size());
		  fflush(stdout); 
		  wm->timer(time, 1);
	  }

	  count++;
  } while (loop.inc());
  v_printf(0, "\r                                                               \r");
}
/*
double clip::min(double a,double b){
  if(a<b) return a;
  else return b;
}

double clip::max(double a,double b){
  if(a>b) return a;
  else return b;
}
*/
int clip::category(int a,int b,int c,int d)
{
  int count;
  count = 0;
  if(a<0) count++;
  if(b<0) count++;
  if(c<0) count++;
  if(d<0) count++;
  if(count==0) return 1;
  else if(count==1) return 2;
  else if(count==2) return 3;
  else return 4;
}
/*
void clip::print_segment(Point3D p1,Point3D p2,std::ofstream& file){
  file << "SL ("
  << p1.X << ", " << p1.Y << ", " << p1.Z << ", "
  << p2.X << ", " << p2.Y << ", " << p2.Z
  << "){10, 20};\n";
}
*/
