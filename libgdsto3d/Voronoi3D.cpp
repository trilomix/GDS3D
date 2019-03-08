

#if defined(_OPENMP)
  #include <omp.h>
#endif

#include "Voronoi3D.h"
#include <fstream>
#include "voro++/src/voro++.hh"
#include "gdselements.h"

using namespace voro;

/*********class clip*********/

clip::clip(){
	Waittime = 1;
	time = wm->new_timer();

	// init timer
	wm->timer(time, 1);
}

clip::~clip(){}

void clip::AddPoly(GeoPolygon* poly, bool recurse, vector<Point3D> *vertices, vector<GoePolyPoint> *MeshPoints) {
	for (size_t i = 0; i<poly->GetPoints(); i++) {
		Point2D p = poly->GetCoords(i);
		vertices->push_back(Point3D(p.X, p.Y, poly->GetHeight()));
		vertices->push_back(Point3D(p.X, p.Y, poly->GetHeight() + poly->GetThickness()));
		GoePolyPoint Poly_P;
		Poly_P.poly = poly;
		Poly_P.index = i;
		MeshPoints->push_back(Poly_P);
	}
	if (recurse && poly->GetHoles().size()>0) {
		for (size_t j = 0; j < poly->GetHoles().size(); j++) {
			GeoPolygon* poly_hole = poly->GetHoles()[j];
			AddPoly(poly_hole, recurse, vertices, MeshPoints);
		}
	}
}

void clip::execute(vector<GDSGroup*> FullGDSItems){
	for (size_t i = 0; i < FullGDSItems.size(); i++) {
		GDSGroup *cur_GDS = FullGDSItems[i];
#pragma omp parallel for 
		for (long long j = 0; j < cur_GDS->layer_list.size(); j++)
		{
			vector<Point3D> vertices;
			vector<GoePolyPoint> MeshPoints;
			size_t z_div;
	size_t layer_cnt = 0;
			layers *Cur_Layer = cur_GDS->layer_list[j];
			bool LayerFound = false;
			GeoPolygon *poly;
			for (size_t k = 0; k < cur_GDS->OutLinesItems.size(); k++) {
				poly = cur_GDS->OutLinesItems[k];
				if (poly->GetLayer() == Cur_Layer) {
					LayerFound = true;
					break;
				}
			}
			if (!LayerFound) {
				for (size_t k = 0; k < cur_GDS->FullPolygonItems.size(); k++) {
					poly = cur_GDS->FullPolygonItems[k];
					if (poly->GetLayer() == Cur_Layer) {
						AddPoly(poly, false, &vertices, &MeshPoints);
						for (size_t l = 0; l < poly->GetHoles().size(); l++) {
							GeoPolygon* poly_hole = poly->GetHoles()[l];
							AddPoly(poly_hole, false, &vertices, &MeshPoints);
						}
					}
				}
				layer_cnt++;
			}
			else {
				AddPoly(poly, true, &vertices, &MeshPoints);
				layer_cnt++;
			}

			// Add Upper and lower Layers
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
				GDSGroup *add_cur_GDS = FullGDSItems[k];
				
				for (size_t l = 0; l < add_cur_GDS->layer_list.size(); l++) {
					layers *add_layer = add_cur_GDS->layer_list[l];
					
					if (add_layer == Cur_Layer)
						continue;
					if (Cur_Layer->Height*Cur_Layer->Units->Unitu > (add_layer->Height + add_layer->Thickness)*add_layer->Units->Unitu)
						continue;
					if ((Cur_Layer->Height + Cur_Layer->Thickness)*Cur_Layer->Units->Unitu  < add_layer->Height*add_layer->Units->Unitu)
						continue;
					GeoPolygon *add_poly;
					for (size_t m = 0; m < add_cur_GDS->OutLinesItems.size(); m++) {
						add_poly = add_cur_GDS->OutLinesItems[m];
						if (add_poly->GetLayer() == add_layer) {
							LayerFound = true;
							break;
						}
					}
					if (!LayerFound) {
						for (size_t n = 0; n < add_cur_GDS->FullPolygonItems.size(); n++) {
							add_poly = add_cur_GDS->FullPolygonItems[n];
							if (add_poly->GetLayer() == add_layer) {
								AddPoly(add_poly, false, &vertices, &MeshPoints);
								for (size_t o = 0; o < add_poly->GetHoles().size(); o++) {
									GeoPolygon* add_poly_hole = add_poly->GetHoles()[o];
									AddPoly(add_poly_hole, false, &vertices, &MeshPoints);
								}
							}
						}
			        layer_cnt++;
		            } else {
						AddPoly(add_poly, true, &vertices, &MeshPoints);
						layer_cnt++;
		            }
	            }

	}
	z_div = layer_cnt+1;

			execute(vertices, MeshPoints, z_div, Cur_Layer);
		}
		v_printf(0, "\r                                                               \r");
	}
	
	
}

void clip::execute(vector<GeoPolygon*> polyList) {
	//vector<Point3D> vertices;
	//vector<GoePolyPoint> MeshPoints;
	set<double> z_list;
	for (size_t i = 0; i<polyList.size(); i++) {
		GeoPolygon *poly = polyList[i];
		//AddPoly(poly,false);
		double z = poly->GetHeight();
		z_list.insert(z);
	}
	//z_div = z_list.size()+1;
	//execute();
}

void clip::execute(vector<Point3D> vertices, vector<GoePolyPoint> MeshPoints, size_t z_div, layers *CurLayer)
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
  
  std::vector<std::vector<std::vector<int> > > bisectors;


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
	  //v_printf(0, "Hello I'm %d \n", omp_get_thread_num());
	  //v_printf(0, "We are %d\n", omp_get_num_threads());
	  std::vector<int> neighbors;
	  pid = loop.pid();
	  //GoePolyPoint Poly_P = MeshPoints[IDs2[count] / 2];
	  GoePolyPoint Poly_P = MeshPoints[IDs2[IDs[pid]] / 2];
	  if (Poly_P.poly->GetLayer() != CurLayer) {
		  count++;
		  continue;
	  }
	  cont.compute_cell(cell, loop);
	  cell.neighbors(neighbors);
	  assert(IDs[pid] == count);
	  
	  /*
	  bool Stop = false;
	  if (Poly_P.poly->GetCoords(Poly_P.index) == Point2D(1039.29, 1510.78)) {
		  Stop = true;
	  }
	  if (Stop) {
	  //v_printf(1, "%2d %2d", counti, IDs2[count]);
		  //v_printf(1, "(%8.7g, %8.7g, %8.7g)", generators[count].X, generators[count].Y, generators[count].Z);
		  v_printf(1, "(%8.7g, %8.7g, %8.7g)", generators[IDs[pid]].X, generators[IDs[pid]].Y, generators[IDs[pid]].Z);
		  v_printf(1, "\n");
	  }*/
	  for (j = 0; j < neighbors.size(); j++) {
		  if (neighbors[j] >= 0) {
			  /*
			  if (Stop) {
				  v_printf(1, " %2d", neighbors[j]);
				  v_printf(1, "(%8.7g, %8.7g, %8.7g)", generators[IDs[neighbors[j]]].X, generators[IDs[neighbors[j]]].Y, generators[IDs[neighbors[j]]].Z);
			  }*/
			  GoePolyPoint Poly_P_Neighbor = MeshPoints[neighbors[j] / 2];

			  // Add only next Upper and lower Layers
			  if (Poly_P.poly->GetHeight() > Poly_P_Neighbor.poly->GetHeight() + Poly_P_Neighbor.poly->GetThickness() + 0.001)
				  continue;
			  if (Poly_P.poly->GetHeight() + Poly_P.poly->GetThickness() + 0.001 < Poly_P_Neighbor.poly->GetHeight())
				  continue;
			  // remove same poly
			  if (Poly_P == Poly_P_Neighbor)
				  continue;
#pragma omp critical   
				  Poly_P.poly->AddPointNeighbor(Poly_P.index, Poly_P_Neighbor);
		  }

	  }
	  // Check timer?
	  if (wm->timer(time, 0) > Waittime) {
		  wm->timer(time, 1);
		  v_printf(0, "\r                                                               ");
		  v_printf(0, "\rCompute Voronoi %5.2f%% Done (%zd/%zd)", 1.0*(count) / IDs.size()*100.0, count, IDs.size());
		  v_printf(0, " Layer : %s", Poly_P.poly->GetLayer()->Name);
		  fflush(stdout); 
	  }

	  count++;
  } while (loop.inc());
	  
  
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
