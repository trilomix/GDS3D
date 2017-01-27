#include "outputStream.h"
#include "gdsobject_ogl.h"
#include <algorithm>

#ifdef linux
#include<sstream>
template <typename T>
std::string to_string(T value)
{
	//create an output string stream
	std::ostringstream os;

	//throw the value into the string stream
	os << std::fixed << value;

	//convert the string stream into a string and return
	return os.str();
}
#endif // linux

int FindOccurOf(string substr, string s) {
        int count = 0;
	size_t nPos = s.find(substr, 0); // fist occurrence
	while (nPos != string::npos)
	{
		count++;
		nPos = s.find(substr, nPos + 1);
	}
	return count;
}

// GDSGroup Class
GDSGroup::GDSGroup(GDSObject_ogl *render_object)
{
	Name = render_object->GetGDSName();
	bbox = render_object->GetTotalBoundary();
	Unit = 0;
	PolygonWrited = 0;
	MeshMaxElemSize = bbox.min.double_distance(bbox.max);
}

GDSGroup::GDSGroup(GDSObject * object, GDSMat object_mat)
{
	Name = object->GetGDSName();
	bbox = object->GetTotalBoundary() * object_mat;
	Unit = 0;
	PolygonWrited = 0;
	
	if (bbox.max.Y < bbox.min.Y) {
		double Y = bbox.max.Y;
		bbox.max.Y = bbox.min.Y;
		bbox.min.Y = Y;
	}
	if (bbox.max.X < bbox.min.X) {
		double X = bbox.max.X;
		bbox.max.X = bbox.min.X;
		bbox.min.X = X;
	}
	MeshMaxElemSize = bbox.min.double_distance(bbox.max);
}

GDSGroup::~GDSGroup()
{
	//for (size_t i = 0; i < FullPolygonItems.size(); i++)
	//	delete FullPolygonItems[i];
	for (size_t i = 0; i < OutLinesItems.size(); i++)
		delete OutLinesItems[i];
	
	//delete[] Name;
}


// output Class
Output::Output()
{
	file = NULL;
	cur_GDS = NULL;
	// TO DO Create directory
	do_mkdir("Gmsh_OutPut");
	filename = "Gmsh_OutPut/" + string(remove_extension(getfilename(wm->filename)));
	cur_Point_Index = 1;
	cur_Line_Index = 1;
	cur_Line_Loop_Index = 1;
	cur_Surface_Index = 2;
	cur_Physical_Index = 1;
	cur_Field_Index = 1;
	cur_HEAT_Layer = 0;
	Surface_Geo = "";
	Volume_Geo = "";
	EndGeo = "";

	Waittime = 1.0f;
	time = wm->new_timer();
}

Output::~Output() {
	
	for (size_t i = 0; i < FullGDSItems.size(); i++)
		delete FullGDSItems[i];
	
	//delete[] filename;
	//delete file;
}

void Output::SaveToGEO(GDSObject_ogl *render_object) {
	//render_layer_t render_layer;
	//struct ProcessLayer *layer;
	//bool found;

	vector<GeoPolygon*> PolygonItems;


	if (render_object->PolygonItems.empty())
		return;
	
	cur_GDS = new GDSGroup(render_object);
	GDS* gds = wm->Assembly->GetGDS(render_object->GetName());
	if (gds == NULL) {
		gds = wm->Assembly->GetGDS();
		cur_GDS->Name = gds->Name;
	}
	if (gds->MeshMaxElemSize && gds->MeshMaxElemSize < cur_GDS->MeshMaxElemSize)
		cur_GDS->MeshMaxElemSize = gds->MeshMaxElemSize;
	FullGDSItems.push_back(cur_GDS);
	for (size_t i = 0; i < render_object->PolygonItems.size(); i++) {
		GDSPolygon *GDSpoly;
		GDSPolygon* poly = render_object->PolygonItems[i];
		if (wm->assembly) {
			GDSProcess *process = wm->getProcess();
			GDS *gds;
			// check if Poly layer is in cur_GDS layers list
			if (cur_GDS->Name) {
				gds = wm->Assembly->GetGDS(cur_GDS->Name);
				ProcessLayer* Layer = process->GetLayer(poly->GetLayer()->Layer, poly->GetLayer()->Datatype, gds->ProcessName);
				if (Layer == NULL || Layer->Index != poly->GetLayer()->Index) {
					// Layer not found in current GDS
					gds = wm->Assembly->GetGDS();
					while (gds) {
						ProcessLayer* Layer = process->GetLayer(poly->GetLayer()->Layer, poly->GetLayer()->Datatype, gds->ProcessName);
						if (Layer != NULL && Layer->Index == poly->GetLayer()->Index) {
							// GDS found

							break;
						}
						gds = gds->Next;
					}
					bool exist = false;
					for (size_t i = 0; i < FullGDSItems.size(); i++) {
						if (strcmp(FullGDSItems[i]->Name, gds->Name) == 0) {
							exist = true;
							cur_GDS = FullGDSItems[i];
							break;
						}
					}
					if (!exist) {
						cur_GDS = new GDSGroup(render_object);
						cur_GDS->Name = gds->Name;
						if (gds->MeshMaxElemSize>1 && gds->MeshMaxElemSize < cur_GDS->MeshMaxElemSize)
							cur_GDS->MeshMaxElemSize = gds->MeshMaxElemSize;
						FullGDSItems.push_back(cur_GDS);
					}
				}
			}
			else {
				gds = wm->Assembly->GetGDS();
				while (gds) {
					ProcessLayer* Layer = process->GetLayer(poly->GetLayer()->Layer, poly->GetLayer()->Datatype, gds->ProcessName);
					if (Layer != NULL && Layer->Index == poly->GetLayer()->Index) {
						// GDS found
						
						break;
					}
					gds = gds->Next;
				}
				cur_GDS->Name = gds->Name;

			}
		}
		GDSpoly = new GDSPolygon(*poly);
		cur_GDS->GDSPolygonItems.push_back(GDSpoly);
	}

	// Build unique list of layers
	LayerList();



	Convert_Polygon();

	Geometry();
	Simu_GetDP();
}

void Output::SaveToGEO(GDSObject_ogl *object, bool flat) {
	
	GDSMat identity; // For the world root


	if (object->PolygonItems.empty() && object->refs.empty())
		return;


	cur_GDS = new GDSGroup(object);
	if (object->GetGDSName()) {
		GDS* gds = wm->Assembly->GetGDS(object->GetName());
		if (gds->MeshMaxElemSize>1 && gds->MeshMaxElemSize < cur_GDS->MeshMaxElemSize)
			cur_GDS->MeshMaxElemSize = gds->MeshMaxElemSize;
	}

	FullGDSItems.push_back(cur_GDS);

	Traverse(object, identity);

	// Build unique list of layers
	LayerList();

	Convert_Polygon();
	Geometry();
	Simu_GetDP();
}

void Output::Convert_Polygon() {
	
	vector<GeoPolygon*> PolygonItems;

	v_printf(0, "Convert Polygon\n");
	for (size_t i = 0; i < FullGDSItems.size(); i++)
	{
		cur_GDS = FullGDSItems[i];
		for (size_t j = 0; j < cur_GDS->layer_list.size(); j++)
		{
			ProcessLayer *do_layer = cur_GDS->layer_list[j];

			PolygonItems = SimplifyPolyItems(cur_GDS->GDSPolygonItems, do_layer);
			for (size_t k = 0; k < PolygonItems.size(); k++) {
				cur_GDS->FullPolygonItems.push_back(PolygonItems[k]);
			}
		}

		// Object to Sort polygon
		cur_GDS->FullSortPolygonItems.SetLayerList(cur_GDS->layer_list);
		cur_GDS->FullSortPolygonItems.Add(cur_GDS->FullPolygonItems);

		// HETALAYER
		for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
			GeoPolygon *poly = cur_GDS->FullPolygonItems[j];
			if (strcmp(poly->GetLayer()->Name, "HEATLAYER") == 0) {
				HeatLayer(poly, cur_GDS);
			}
		}
	}
	v_printf(0, "\r                                               \r");
	fflush(stdout);

}

void Output::LayerList() {
	bool found;
	struct ProcessLayer *layer;
	// Build unique list of layers
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];
		if (!cur_GDS->GDSPolygonItems.empty())
		{
			for (size_t i = 0; i < cur_GDS->GDSPolygonItems.size(); i++)
			{
				// Get layer
				layer = cur_GDS->GDSPolygonItems[i]->GetLayer();
				if (!layer || !layer->Show)
					continue;

				// Try to find layer
				found = false;
				for (unsigned long j = 0; j < cur_GDS->layer_list.size(); j++)
				{
					if (cur_GDS->layer_list[j] == layer)
					{
						found = true;
						break;
					}
				}

				// New layer?
				if (!found)
				{
					cur_GDS->layer_list.push_back(layer);
					if (cur_GDS->Unit == 0)
						cur_GDS->Unit = layer->Units->Unitu;
				}
			}
		}
	}

}

bool Output::CheckMeshSize(GeoPolygon *poly, vector<GeoPolygon*> PolyItemsNear, double TopDownMaxRatio, bool Top) {
	bool Modif = false;
	vector<GeoPolygon*> PolyItems;
	
	double TopDownMaxRatio_local= TopDownMaxRatio;

	PolyItems = GetContactPolyItems(poly, PolyItemsNear, Top);
	for (size_t k = 0; k < PolyItems.size(); k++) {
		GeoPolygon *poly_contact = PolyItems[k];
		if (poly->GDSPolygon::isPolygonInside_wborders(*poly_contact)  && poly->GetLayer()->Metal 
			|| poly_contact->GDSPolygon::isPolygonInside_wborders(*poly) && !poly->GetLayer()->Metal
			) {
			if (poly_contact->GetMeshElemSize() < poly->GetMeshElemSize() / TopDownMaxRatio_local
				|| poly_contact->GetMeshElemSizeMin() < poly->GetMeshElemSizeMin()
				|| poly->DistBB2BBLessThan(poly_contact, TopDownMaxRatio)) {
				if (poly->SetMeshElemSize(poly_contact, Top, TopDownMaxRatio) && !Modif)
					Modif = true;
			}
			if (poly->GetMeshElemSize() < poly_contact->GetMeshElemSize() / TopDownMaxRatio_local) {
				if (poly_contact->GetBBox()->max == Point2D(1160.375, 1433.805)) {
					assert(k == k);
				}

				if (poly_contact->SetMeshElemSize(poly, !Top, TopDownMaxRatio) && !Modif)
					Modif = true;
			}
		}
		else {
			for (size_t i = 0; i < poly_contact->GetHoles().size(); i++) {
				GeoPolygon *hole = poly_contact->GetHoles()[i];
				if (poly->GDSPolygon::isPolygonInside_wborders(*hole)) {
					if (hole->GetMeshElemSize() < poly->GetMeshElemSize() / TopDownMaxRatio_local) {
						if (poly->SetMeshElemSize(hole, Top, TopDownMaxRatio) && !Modif)
							Modif = true;
					}
					if (poly->GetMeshElemSize() < hole->GetMeshElemSize() / TopDownMaxRatio_local) {
						if (hole->SetMeshElemSize(poly, !Top, TopDownMaxRatio) && !Modif)
							Modif = true;
					}

				}
			}
		}
	}
	return Modif;

}

void Output::SetMeshElemSize(GeoPolygon *poly, bool *Modif) {
	
	
	double currentMesh = poly->GetMeshElemSize(Modif);
	double TopDownMaxRatio = 3;
	if (poly->GetThickness() > 1) {
		TopDownMaxRatio = TopDownMaxRatio * pow(10, floor(log10(poly->GetThickness())))*floor(poly->GetThickness() / pow(10, floor(log10(poly->GetThickness()))));
	}

	vector<GeoPolygon*> PolyItemsNear = cur_GDS->FullSortPolygonItems.GetPolyNear(*poly->Get3DBBox());
	
	for (size_t i = 0; i < PolyItemsNear.size(); i++) {
		GeoPolygon* cur_poly = PolyItemsNear[i];
		if (cur_poly == poly)
			continue;
		if(cur_poly->GetPoints() == 4 && cur_poly->GetBBox()->max == Point2D(1156.775,1430.35)	
			){
			assert(i == i);
		}
		if (cur_poly->GetLayer() == poly->GetLayer() && cur_poly->GetMeshElemSize()/TopDownMaxRatio < poly->GetMeshElemSize()) {
			if(poly->MinDistFromPoly(cur_poly, TopDownMaxRatio) && !Modif)
				*Modif = true;
		}
	}

	if(CheckMeshSize(poly, PolyItemsNear, TopDownMaxRatio, false /*Bottom*/) && !Modif)
		*Modif = true;
	if (CheckMeshSize(poly, PolyItemsNear, TopDownMaxRatio, true /*Top*/) && !Modif)
		*Modif = true;

	if (currentMesh != poly->GetMeshElemSize() && !*Modif)
		*Modif = true;
}

void Output::Traverse(GDSObject *object, GDSMat object_mat) {
	GDSPolygon *GDSpoly;

	if (cur_GDS->Name != object->GetGDSName() || cur_GDS->Name != NULL && object->GetGDSName() != NULL 
		&& strlen(cur_GDS->Name) == strlen(object->GetGDSName())
		&& strcmp(cur_GDS->Name, object->GetGDSName()) != 0 ) {
		cur_GDS = new GDSGroup(object, object_mat);
		GDS* gds = wm->Assembly->GetGDS(object->GetName());
		if (gds->MeshMaxElemSize>1 && gds->MeshMaxElemSize < cur_GDS->MeshMaxElemSize)
			cur_GDS->MeshMaxElemSize = gds->MeshMaxElemSize;
		FullGDSItems.push_back(cur_GDS);
	}


	// Add Polygon of the current CellView
	for (size_t i = 0; i < object->PolygonItems.size(); i++) {
		GDSpoly = new GDSPolygon(*object->PolygonItems[i]);
		GDSpoly->transformPoints(object_mat);
		cur_GDS->GDSPolygonItems.push_back(GDSpoly);
	}
	// Add Polgon of the refs CellView
	for (size_t i = 0; i < object->refs.size(); i++) {
		Traverse(object->refs[i]->object, object_mat * object->refs[i]->mat);
	}
}

void Output::Geometry() {
	size_t TotalNbPoly = 0;
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];

		 // Create GeoPolygon for each dielectric
		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++) {
			ProcessLayer *do_layer = cur_GDS->layer_list[i];
			if (strcmp(do_layer->Name, "HEATLAYER") == 0)
				continue;
			if (do_layer->Top && !do_layer->Metal)
				continue; // Last layer must be the contact layer with other part so no dielectric space

			GeoPolygon *Outline = new GeoPolygon(cur_GDS->Name, do_layer->Height * cur_GDS->Unit, do_layer->Thickness* cur_GDS->Unit, do_layer, true /*isHole*/);
			double EdgeOffset = 1.0;
			double sign = 1.0;
			for (size_t i = 0; i < 4; i++) {
				if (i == 0)
					Outline->AddPoint(cur_GDS->bbox.min - Point2D(EdgeOffset, sign*EdgeOffset));
				if (i == 1)
					Outline->AddPoint(cur_GDS->bbox.min.X - EdgeOffset, cur_GDS->bbox.max.Y + sign*EdgeOffset);
				if (i == 2)
					Outline->AddPoint(cur_GDS->bbox.max + Point2D(EdgeOffset, sign*EdgeOffset));
				if (i == 3)
					Outline->AddPoint(cur_GDS->bbox.max.X + EdgeOffset, cur_GDS->bbox.min.Y - sign*EdgeOffset);

			}
			Outline->Orientate();
			Outline->Tesselate();
			Outline->SetMeshElemSize();
			cur_GDS->OutLinesItems.push_back(Outline);
			vector<GeoPolygon*> PolyList = Outline->FindPolyInside(cur_GDS->FullSortPolygonItems.GetPolyNear(*Outline->Get3DBBox()), false /*all*/);
			for (size_t i = 0; i < PolyList.size(); i++)
			{
				Outline->AddHole(PolyList[i]);
			} // PolyLoop

		}// Layers Loop
		TotalNbPoly += cur_GDS->FullPolygonItems.size();
	}// GDS Loop


	// Set Mesh Elem Size
	size_t PassCnt = 0;
	bool Modif = true;
	wm->timer(time, 1);

	while (Modif) {
		PassCnt += 1;
		size_t TotDone = 0;
		v_printf(1, "Set Mesh Elem Size Pass %zd\n", PassCnt);
		Modif = false;
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			// First Pass with no hole poly
			for (size_t i = 0; i < cur_GDS->FullPolygonItems.size(); i++)
			{
				GeoPolygon *poly = cur_GDS->FullPolygonItems[i];
				if (poly->GetHoles().size() == 0) {
					if (poly->GetBBox()->max==Point2D(851.355, 1541.57)) {
						assert(i == i);
					}
					SetMeshElemSize(poly, &Modif);
					TotDone += 1;
				}
				// Check timer?
				if (wm->timer(time, 0) > Waittime) {
					v_printf(0,"\rTotal Done %5.2f%% (%zd/%zd)", 1.0*(TotDone)/TotalNbPoly*100.0, TotDone, TotalNbPoly);
					fflush(stdout);
					wm->timer(time, 1);
				}
			}
			// Second Pass with hole poly
			for (size_t i = 0; i < cur_GDS->FullPolygonItems.size(); i++)
			{
				GeoPolygon *poly = cur_GDS->FullPolygonItems[i];
				
				if (poly->GetHoles().size() != 0) {

					if (Point2D(900, 1400) < poly->GetBBox()->max && poly->GetBBox()->max < Point2D(1000, 1450)) {
						assert(i == i);
					}
					if (poly->GetBBox()->max == Point2D(1157.49, 1430.92)) {
						assert(i == i);
					}
					for (size_t j = 0; j < poly->GetHoles().size(); j++) {
						GeoPolygon *hole = poly->GetHoles()[j];
						if (hole->GetBBox()->max == Point2D(1157.49, 1430.92)) {
							assert(i == i);
						}

						SetMeshElemSize(hole, &Modif);
						// Check timer?
						if (wm->timer(time, 0) > Waittime) {
							v_printf(0, "\rTotal Done %5.2f%% (%zd/%zd) inside (%zd/%zd)", 1.0*(TotDone) / TotalNbPoly*100.0, TotDone, TotalNbPoly, j, poly->GetHoles().size()-1);
							fflush(stdout);
							wm->timer(time, 1);
						}
					}
					SetMeshElemSize(poly, &Modif);
					TotDone += 1;

				}
				// Check timer?
				if (wm->timer(time, 0) > Waittime) {
					v_printf(0, "\r                                               \r");
					v_printf(0, "\rTotal Done %5.2f%% (%zd/%zd)", 1.0*(TotDone) / TotalNbPoly*100.0, TotDone, TotalNbPoly);
					fflush(stdout);
					wm->timer(time, 1);
				}

			}
			for (size_t i = 0; i < cur_GDS->OutLinesItems.size(); i++)
			{
				GeoPolygon *poly = cur_GDS->OutLinesItems[i];
				SetMeshElemSize(poly, &Modif);
				// Find Upper Layer
				for (size_t j = 0; j < cur_GDS->OutLinesItems.size(); j++) {
					if (i == j)
						continue;
					ProcessLayer *layer = cur_GDS->OutLinesItems[j]->GetLayer();
					GeoPolygon* cur_OutlinePoly = cur_GDS->OutLinesItems[j];
					if (layer->Top && !layer->Metal)
						continue; // Last layer must be the contact layer with other part so no dielectric space

					if (poly->GetLayer()->Height + poly->GetLayer()->Thickness == layer->Height) {
						if (poly->GetMeshElemSize() > cur_OutlinePoly->GetMeshElemSize()) {
							double TopDownMaxRatio = 3;
							if (poly->GetThickness() > 1) {
								TopDownMaxRatio = TopDownMaxRatio * pow(10, floor(log10(poly->GetThickness())))*floor(poly->GetThickness() / pow(10, floor(log10(poly->GetThickness()))));
							}
							poly->SetMeshElemSize(cur_OutlinePoly,true, TopDownMaxRatio);
						}
					}
				}
			}
		}
		v_printf(0, "\r                                               \r");
		fflush(stdout);
	}
	v_printf(0, "Start Writing Geometry\n");
	
	string OutFileName = filename + ".geo";
	// open file and check for errors
	file = fopen(OutFileName.c_str(), "wb");
	if (file == NULL) {
		return;
	}
	
	fprintf(file, "//Geo File Generate by GDS3D %s\n", GDS3D_VERSION);
	double Unit =0;
	string GDS_cl;
	for (size_t i = 0; i < FullGDSItems.size(); i++) {
		cur_GDS = FullGDSItems[i];
		if (cur_GDS->Unit != 0)
			if(Unit == 0)
				Unit = cur_GDS->Unit;
			else if (cur_GDS->Unit < Unit)
				Unit = cur_GDS->Unit;
		if (cur_GDS->Name) {
			string Name = cur_GDS->Name;
			std::replace(Name.begin(), Name.end(), '-', '_');
			GDS_cl = GDS_cl + "GDS_" + Name + "_cl = " + to_string(cur_GDS->MeshMaxElemSize) + "; \n";
		}
	}
	//fprintf(file, "Geometry.Tolerance = %s;	                        // Default 1e-8\n", to_string(cur_GDS->Unit*1e-3).c_str());
	fprintf(file, "Geometry.Tolerance = 1e-8;	                        // Default 1e-8\n");
	fprintf(file, "Mesh.RandomFactor = 1e-9;                        // Default 1e-9\n");
	fprintf(file, "Mesh.CharacteristicLengthMin = 0;             // Default 0\n");
	fprintf(file, "Mesh.CharacteristicLengthMax = 1e+22;              // Default 1e+22\n");
	fprintf(file, "Mesh.CharacteristicLengthExtendFromBoundary = 1; //Default 1\n");
	fprintf(file, "//Mesh.Algorithm = 8; Default 2 \n");
	fprintf(file, "Mesh.Algorithm = 8; // mesh algorithm 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=BAMG, 8=DelQuad\n");
	fprintf(file, "Mesh.RecombineAll = 0;                           // Default 0\n");
	fprintf(file, "Mesh.RecombinationAlgorithm = 0;                 // Default 1 0=standard, 1=blossom\n");
	fprintf(file, "Mesh.Algorithm3D = 1; //Default 1 mesh algorithm 1=Delaunay, 2=New Delaunay, 4=Frontal, 5=Frontal Delaunay,6 = Frontal Hex, 7 = MMG3D, 9 = R - tree\n");
	fprintf(file, "\n");
	fprintf(file, "cl = 1;\n");
	//fprintf(file, "Geometry.AutoCoherence = 0;\n");

	//Define Elem size
	fprintf(file, "%s", GDS_cl.c_str());
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];

		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++) {
			ProcessLayer *do_layer = cur_GDS->layer_list[i];
			fprintf(file, "cl%d = cl*1; //Elem Size for Layer %s\n", do_layer->Index, do_layer->Name);

		}
	}

	// Add Macro
	fprintf(file, "// *** Macro ***\n");
	fprintf(file, "Macro PointElemSize\n");
	fprintf(file, "\tIf (Pt_cl > GDS_cl)\n");
	fprintf(file, "\t\tPt_cl = GDS_cl;\n");
	fprintf(file, "\tEndIf\n");
	fprintf(file, "\tPoint(Pt_i) = {Pt_x, Pt_y, Pt_z, Pt_cl};\n");
	fprintf(file, "Return\n");

	//Define Physical Volume
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];

		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++) {
			ProcessLayer *do_layer = cur_GDS->layer_list[i];
			if (strcmp(do_layer->Name, "HEATLAYER") == 0) {
				//fprintf(file, "Physical Volume ( \"%s\") = {};\n", do_layer->Name);
				continue;
			}
			else {
				fprintf(file, "Physical Volume ( \"%s\",%zd) = {};\n", do_layer->Name, cur_Physical_Index);
				cur_Physical_Index += 1;
				fprintf(file, "Physical Volume ( \"Dielec-%s\",%zd) = {};\n", do_layer->Name, cur_Physical_Index);
				cur_Physical_Index += 1;
			}

		}
	}
	v_printf(0, "Via Writing Geometry\n");
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];

		// Output geometry for via layer only
		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++)
		{
			ProcessLayer *do_layer = cur_GDS->layer_list[i];

			if (strcmp(do_layer->Name, "HEATLAYER") == 0)
				continue;
			if (do_layer->Metal != 1) {

				// Reset Timer
				wm->timer(time, 1);
				size_t TotalNbPoly = cur_GDS->FullPolygonItems.size();

				for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
					// Check timer?
					if (wm->timer(time, 0) > Waittime) {
						v_printf(0, "\rLayer %s Done %5.2f%% (%zd/%zd)", do_layer->Name, 1.0*(j) / (TotalNbPoly)*100.0, j, TotalNbPoly);
						fflush(stdout);
						wm->timer(time, 1);
					}

					Polygon(cur_GDS->FullPolygonItems[j], do_layer);
				}
			}
			v_printf(0, "\r                                               \r");
			fflush(stdout);
		}
	}// GDS Loop
	v_printf(0, "Via Writing Geometry Done\n");
	
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];

		// Output geometry for each Metal layer
		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++)
		{
			ProcessLayer *do_layer = cur_GDS->layer_list[i];

			if (do_layer->Metal == 1) {

				// Reset Timer
				wm->timer(time, 1);
				size_t TotalNbPoly = cur_GDS->FullPolygonItems.size();

				v_printf(0, "\r                                               \r");
				fflush(stdout);
				v_printf(0, "Metal (%s) Writing Geometry\n", do_layer->Name);
				for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++)
				{
					// Check timer?
					if (wm->timer(time, 0) > Waittime) {
						v_printf(0, "\rLayer %s Done %5.2f%% (%zd/%zd)", do_layer->Name, 1.0*(j) / (TotalNbPoly)*100.0, j, TotalNbPoly);
						fflush(stdout);
						wm->timer(time, 1);
					}

					if (false && strcmp(do_layer->Name, "HEATLAYER") == 0 && cur_GDS->FullPolygonItems[j]->GetLayer() == do_layer) {
						HeatLayer(cur_GDS->FullPolygonItems[j], cur_GDS);
					}
					else
						Polygon(cur_GDS->FullPolygonItems[j], do_layer);
				} // PolyLoop
			}
			v_printf(0, "\r                                               \r");
			fflush(stdout);
		}// Layers Loop

		// Output geometry for each dielectric
		for (size_t j = 0; j < cur_GDS->OutLinesItems.size(); j++) {
			GeoPolygon *Outline = cur_GDS->OutLinesItems[j];
			Polygon(Outline, Outline->GetLayer(), true /*dielectrique*/);
		}
		v_printf(0, "\r                                               \r");
		fflush(stdout);

	}// GDS Loop
	ProcessLayer *HeatLayerHigh = NULL;
	
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];

		// Change Top Surface for each outline dielectric
		for (size_t i = 0; i < cur_GDS->OutLinesItems.size(); i++) {
			ProcessLayer *do_layer = cur_GDS->OutLinesItems[i]->GetLayer();
			if (do_layer->Top && !do_layer->Metal)
				continue; // Last layer must be the contact layer with other part so no dielectric space
			GeoPolygon *Outline = cur_GDS->OutLinesItems[i];

			// Find Upper Layer
			for (size_t j = 0; j < cur_GDS->OutLinesItems.size(); j++) {
				ProcessLayer *layer = cur_GDS->OutLinesItems[j]->GetLayer();
				GeoPolygon* cur_OutlinePoly = cur_GDS->OutLinesItems[j];
				if (layer->Top && !layer->Metal)
					continue; // Last layer must be the contact layer with other part so no dielectric space

				if (do_layer->Height + do_layer->Thickness == layer->Height) {
					EndGeo = EndGeo + "\n// Combine Volume Dielec-" + string(do_layer->Name) + " with Dielec-" + string(layer->Name) + "\n";
					EndGeo = EndGeo + "SurfacesLoop[] = Boundary { Volume{" + Outline->GetVolumeID() + "}; } ; \n";
					EndGeo = EndGeo + "Delete {Volume { " + Outline->GetVolumeID() + " }; }\n";
					EndGeo = EndGeo + "SurfaceList[]={};\n";
					EndGeo = EndGeo + "For i In {0 : #SurfacesLoop[]-1}\n";
					EndGeo = EndGeo + "  If (SurfacesLoop[i] != " + Outline->GetTopSurfaceID() + ")\n";
					EndGeo = EndGeo + "    SurfaceList[] += {SurfacesLoop[i]};\n";
					EndGeo = EndGeo + "  Else\n";
					EndGeo = EndGeo + "    SurfaceList[] += {" + cur_OutlinePoly->GetBottomSurfaceIDList(cur_GDS->FullSortPolygonItems.GetPolyNear(*cur_OutlinePoly->Get3DBBox()), false /*No Hole*/) + "};\n";
					EndGeo = EndGeo + "    Delete {Surface {" + Outline->GetTopSurfaceID() + "};}\n";
					EndGeo = EndGeo + "  EndIf\n";
					EndGeo = EndGeo + "EndFor\n";
					EndGeo = EndGeo + "NewSurfacLoop = news;\n";
					EndGeo = EndGeo + "Surface Loop (NewSurfacLoop) = { SurfaceList[] };\n";
					EndGeo = EndGeo + "Volume (" + Outline->GetVolumeID() + ") = { NewSurfacLoop };\n";

					Recombine(Outline, layer);
				}
			}
		}// Layers Loop

		 // HEATLAYER
		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++)
		{
			ProcessLayer *do_layer = cur_GDS->layer_list[i];

			if (strcmp(do_layer->Name, "HEATLAYER") == 0) {
				for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
					GeoPolygon *poly = cur_GDS->FullPolygonItems[j];
					if (strcmp(poly->GetLayer()->Name, "HEATLAYER") == 0) {
						HeatLayer_PostProcess(poly, cur_GDS);
					}
				}
				HeatLayerHigh = do_layer;
			}
		}

	}// GDS Loop
	EndGeo = EndGeo + "Coherence;\n";
	
	//If no HEATLAYER
	if(HeatLayerHigh == NULL){
		// Find the Highest Layer
		ProcessLayer *Top_Value = NULL;
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			for (size_t i = 0; i < cur_GDS->layer_list.size(); i++)
			{
				ProcessLayer *do_layer = cur_GDS->layer_list[i];
				if (do_layer->Show) {
					if (Top_Value == NULL || Top_Value->Height + Top_Value->Thickness < do_layer->Height + do_layer->Thickness)
						Top_Value = do_layer;
				}
			}
		}
		HeatLayerHigh = Top_Value;
		// Find Surface
		GeoPolygon *Temppoly = NULL;
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
				GeoPolygon *poly = cur_GDS->FullPolygonItems[j];
				if (poly->GetLayer() == Top_Value) {
					if (Temppoly == NULL) {
						Temppoly = poly;
					}
					else {
						if (Temppoly->Area() < poly->Area())
							Temppoly = poly;
					}
				}
			}
		}
		EndGeo += "\nPhysical Surface(\"HEATLAYER\", " + to_string(cur_Physical_Index) + ") = { " + Temppoly->GetTopSurfaceID() + " };\n";
		cur_Physical_Index += 1;
	}


	vector<string> FixTempSurface;
	if (HeatLayerHigh) {
		// Trouver la surface la plus eloigné 
		ProcessLayer *Top_Value = HeatLayerHigh;
		ProcessLayer *Bottom_Value = HeatLayerHigh;
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			for (size_t i = 0; i < cur_GDS->layer_list.size(); i++)
			{
				ProcessLayer *do_layer = cur_GDS->layer_list[i];
				if (do_layer->Show) {
					if (Top_Value->Height + Top_Value->Thickness < do_layer->Height + do_layer->Thickness)
						Top_Value = do_layer;
					if (Bottom_Value->Height > do_layer->Height)
						Bottom_Value = do_layer;
				}
			}
		}
		ProcessLayer *Farest_Value;
		ProcessLayer *Other_Value;
		if (fabs((Top_Value->Height + Top_Value->Thickness) - HeatLayerHigh->Height) < fabs(Bottom_Value->Height - HeatLayerHigh->Height)) {
			Farest_Value = Bottom_Value;
			Other_Value = Top_Value;
		} else {
			Farest_Value = Top_Value;
			Other_Value = Bottom_Value;
		}
		GeoPolygon *Temppoly = NULL;
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
				GeoPolygon *poly = cur_GDS->FullPolygonItems[j];
				if (poly->GetLayer() == Farest_Value) {
					if (Temppoly == NULL) {
						Temppoly = poly;
					} else {
						if(Temppoly->Area() < poly->Area())
							Temppoly = poly;
					}
					if (Top_Value == Farest_Value)
						FixTempSurface.push_back(poly->GetTopSurfaceID());
					else
						FixTempSurface.push_back(poly->GetSurfaceID());
				}
			}
		}
		// Add Other side also
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
				GeoPolygon *poly = cur_GDS->FullPolygonItems[j];
				if (poly->GetLayer() == Other_Value) {
					if (Top_Value == Other_Value)
						FixTempSurface.push_back(poly->GetTopSurfaceID());
					else
						FixTempSurface.push_back(poly->GetSurfaceID());
				}
			}
		}
		// Put the biggest first
		if (Top_Value == Farest_Value)
			FixTempSurface.insert(FixTempSurface.begin(), Temppoly->GetTopSurfaceID());
		else
			FixTempSurface.insert(FixTempSurface.begin(), Temppoly->GetSurfaceID());
	}
	fprintf(file, Surface_Geo.c_str());
	fprintf(file, Volume_Geo.c_str());
	fprintf(file, EndGeo.c_str());
	
	if (HeatLayerHigh) {
		for (size_t i = 0; i < FixTempSurface.size(); i++) {
			string FixTempSurface_StrVal = FixTempSurface[i];
			if (FixTempSurface_StrVal == FixTempSurface[0] && i != 0)
				continue;
			fprintf(file, "\nPhysical Surface(\"FIXTEMP%zd\", %zd) = { %s };\n", i, cur_Physical_Index, FixTempSurface_StrVal.c_str());
			cur_Physical_Index += 1;
		}
	}
	fclose(file);

}

void Output::Simu_GetDP() {

	string OutFileName = filename + ".pro";
	file = fopen(OutFileName.c_str(), "wb");
	if (file == NULL) {
		return;
	}

	string PhysicalVol;
	string Output_Pro;
	string Pro_Function_Const = "";
	string Pro_Function;
	bool Bulkdone = false;
	bool Viadone = false;
	bool Metaldone = false;
	double HeatSurfaceArea = 0;
	size_t Physical_Index = 1;
	fprintf(file, "//Process File\n");

	fprintf(file, "Group{\n");
	//Define Physical Volume
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];
		string UnitStr = to_string(cur_GDS->Unit*1e-3);
		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++) {
			ProcessLayer *do_layer = cur_GDS->layer_list[i];
			string LayerName = do_layer->Name;
			size_t nPos = LayerName.find(" ", 0); // fist occurrence
			while (nPos != string::npos)
			{
				LayerName.replace(nPos, 1, "_");
				nPos = LayerName.find(" ", 0);
			}
			//LayerName.replace(LayerName.begin(), LayerName.end(),' ', '_');

			if (strcmp(do_layer->Name, "HEATLAYER") != 0) {
				Output_Pro = Output_Pro + "\t" + LayerName + " = Region[" + to_string(Physical_Index) +"];\n";
				Physical_Index += 1;
				if (strcmp(do_layer->Name, "BULK") != 0) {
					Output_Pro = Output_Pro + "\tDielec_" + LayerName + " = Region[" + to_string(Physical_Index) + "];\n";
				}
				Physical_Index += 1;
				if (PhysicalVol != "")
					PhysicalVol += ", ";
				PhysicalVol = PhysicalVol + LayerName;
				if (strcmp(do_layer->Name, "BULK") != 0)
					PhysicalVol = PhysicalVol + ", Dielec_" + LayerName;
				if (strcmp(do_layer->Name, "BULK") == 0) {
					if (do_layer->Material != NULL) {
						Pro_Function = Pro_Function + "\tk[BULK] = k" + do_layer->Material + " / " + UnitStr + ";\n";
					} else {
						Pro_Function += "\tk[BULK] = kBULK / " + UnitStr + ";\n";
					}
					Pro_Function += "\trhoc[BULK] = rhocBULK; \n";
					if (!Bulkdone) {
						Bulkdone = true;
						Pro_Function_Const += ",\n\t\t\t";
						Pro_Function_Const += "kBULK = { 87.9, Min 1, Max 1000, Step 1,\n";
						Pro_Function_Const += "\t\t\t\tName \"Parameters / 2k(BULK) [W*(m*K)^-1]\" },\n";
						Pro_Function_Const += "\t\t\trhocBULK = { 0.852 * 2.32e6, Min 1e3, Max 1e8, Step 1e3,\n";
						Pro_Function_Const += "\t\t\t\tName \"Parameters / 2rho cp(BULK)\" }";
					}
				}
				else {
					
					
					if (do_layer->Metal == 0) {
						if (do_layer->Material == NULL) {
							Pro_Function = Pro_Function + "\tk[" + LayerName + "] = kVia / " + UnitStr + ";\n";
							Pro_Function = Pro_Function + "\tk[Dielec_" + LayerName + "] = kDielecVia / " + UnitStr + ";\n";
						} else {
							Pro_Function = Pro_Function + "\tk[" + LayerName + "] = k" + do_layer->Material + " / " + UnitStr + ";\n";
							Pro_Function = Pro_Function + "\tk[Dielec_" + LayerName + "] = k" + do_layer->OutMaterial + " / " + UnitStr + ";\n";

						}
						Pro_Function = Pro_Function + "\trhoc[" + LayerName + "] = rhocVia; \n";
						Pro_Function = Pro_Function + "\trhoc[Dielec_" + LayerName + "] = rhocDielecVia; \n";
						if (!Viadone) {
							Viadone = true;
							Pro_Function_Const += ",\n\t\t\t\t";
							Pro_Function_Const += "kVia = { 394, Min 1, Max 1000, Step 1,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2k(Via) [W*(m*K)^-1]\" },\n";
							Pro_Function_Const += "\t\t\t\trhocVia = { 385, Min 1e3, Max 1e8, Step 1e3,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2rho cp(Via)\" },\n";
							Pro_Function_Const += "\t\t\t\tkDielecVia = { 0.1, Min 1, Max 10000, Step 1,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2k(DielecVia) [W*(m*K)^-1]\" },\n";
							Pro_Function_Const += "\t\t\t\trhocDielecVia = { 8920 * 385, Min 1e3, Max 1e8, Step 1e3,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2rho cp(DielecVia)\" }";
						}
					}
					if (do_layer->Metal == 1) {
						if (do_layer->Material == NULL) {
							Pro_Function = Pro_Function + "\tk[" + LayerName + "] = kMetal / " + UnitStr + ";\n";
							Pro_Function = Pro_Function + "\tk[Dielec_" + LayerName + "] = kDielecMetal / " + UnitStr + ";\n";
						} else {
							Pro_Function = Pro_Function + "\tk[" + LayerName + "] = k" + do_layer->Material + " / " + UnitStr + ";\n";
							Pro_Function = Pro_Function + "\tk[Dielec_" + LayerName + "] = k" + do_layer->OutMaterial + " / " + UnitStr + ";\n";

						}
						Pro_Function = Pro_Function + "\trhoc[" + LayerName + "] = rhocMetal; \n";
						Pro_Function = Pro_Function + "\trhoc[Dielec_" + LayerName + "] = rhocDielecMetal; \n";
						if (!Metaldone) {
							Metaldone = true;
							Pro_Function_Const += ",\n\t\t\t\t";
							Pro_Function_Const += "kMetal = { 394, Min 1, Max 1000, Step 1,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2k(Metal) [W*(m*K)^-1]\" },\n";
							Pro_Function_Const += "\t\t\t\trhocMetal = { 385, Min 1e3, Max 1e8, Step 1e3,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2rho cp(Metal)\" },\n";
							Pro_Function_Const += "\t\t\t\tkDielecMetal = { 0.1, Min 1, Max 10000, Step 1,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2k(DielecMetal) [W*(m*K)^-1]\" },\n";
							Pro_Function_Const += "\t\t\t\trhocDielecMetal = { 8920 * 385, Min 1e3, Max 1e8, Step 1e3,\n";
							Pro_Function_Const += "\t\t\t\tName \"Parameters / 2rho cp(DielecMetal)\" }";
						}
					}
					
				}

			} else {
				//Physical_Index += 1;
			}

		}
	}
	
	fprintf(file, Output_Pro.c_str());
	
	// HEATLAYER
	ProcessLayer *HeatLayerHigh = NULL;
	Output_Pro = "";
	for (size_t k = 0; k < FullGDSItems.size(); k++) {
		cur_GDS = FullGDSItems[k];
		for (size_t i = 0; i < cur_GDS->layer_list.size(); i++)
		{
			ProcessLayer *do_layer = cur_GDS->layer_list[i];

			if (strcmp(do_layer->Name, "HEATLAYER") == 0) {
				for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
					GeoPolygon *poly = cur_GDS->FullPolygonItems[j];
					if (strcmp(poly->GetLayer()->Name, "HEATLAYER") == 0) {
						fprintf(file, "\tinflux%zd = Region[%zd];\n", Physical_Index, Physical_Index);
						if (Output_Pro != "")
							Output_Pro += ", ";
						Output_Pro += "influx" + to_string(Physical_Index);
						Physical_Index += 1;
						HeatSurfaceArea += poly->Area()*pow((cur_GDS->Unit*1e-3), 2);
					}
				}
				HeatLayerHigh = do_layer;

			}
		}
	}
	if (HeatLayerHigh == NULL) {
		fprintf(file, "\tinflux = Region[%zd];\n", Physical_Index);
		if (Output_Pro != "")
			Output_Pro += ", ";
		Output_Pro += "influx";
		Physical_Index += 1;

		// Find the Highest Layer
		ProcessLayer *Top_Value = NULL;
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			for (size_t i = 0; i < cur_GDS->layer_list.size(); i++)
			{
				ProcessLayer *do_layer = cur_GDS->layer_list[i];
				if (do_layer->Show) {
					if (Top_Value == NULL || Top_Value->Height + Top_Value->Thickness < do_layer->Height + do_layer->Thickness)
						Top_Value = do_layer;
				}
			}
		}
		HeatLayerHigh = Top_Value;
		// Find Surface
		GeoPolygon *Temppoly = NULL;
		for (size_t k = 0; k < FullGDSItems.size(); k++) {
			cur_GDS = FullGDSItems[k];
			for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
				GeoPolygon *poly = cur_GDS->FullPolygonItems[j];
				if (poly->GetLayer() == Top_Value) {
					if (Temppoly == NULL) {
						Temppoly = poly;
					}
					else {
						if (Temppoly->Area() < poly->Area())
							Temppoly = poly;
					}
				}
			}
		}
		HeatSurfaceArea += Temppoly->Area()*pow((cur_GDS->Unit*1e-3), 2);

	}

	fprintf(file, "\tfixtemp = Region[%zd];\n",Physical_Index);

	fprintf(file, "\tVol_The = Region[{%s}];\n", PhysicalVol.c_str());
	fprintf(file, "\tSur_The = Region[{%s}]; \n", Output_Pro.c_str());
	fprintf(file, "\tTot_The = Region[{Vol_The,Sur_The}]; \n");
	fprintf(file, "}\n");

	fprintf(file, "	Function{\n");
	fprintf(file, "			DefineConstant[\n");
	fprintf(file, "			Pmax = { 1, Min 0.1, Max 10, Step 0.2,\n");
	fprintf(file, "				Name \"Parameters / 1Peak Power [W]\" }\n");
	fprintf(file, "			pulse = { 1, Min 0.1, Max 5, Step 0.1,\n");
	fprintf(file, "				Name \"Parameters / 1Pulse length\" },\n");
	fprintf(file, "			timemax = { 2, Min 0.01, Max 20, Step 0.1,\n");
	fprintf(file, "				Name \"Parameters / 1Simulation time\" }\n");
	fprintf(file, "			dtime = { 0.05, Min 0.001, Max 1, Step 0.001,\n");
	fprintf(file, "				Name \"Parameters / 1Time step\" }");
	fprintf(file, Pro_Function_Const.c_str());
	fprintf(file, "\n\t\t\t];\n");
	fprintf(file, "\t// Material\n");
	fprintf(file, "\tkCu			= 394;		// [W*/(m*K)]\n");
	fprintf(file, "\tkCuPCBvias	= 144;		// [W*/(m*K)]\n");
	fprintf(file, "\tkAl			= 237;		// [W*/(m*K)]\n");
	fprintf(file, "\tkMold		= 1.7;		// [W*/(m*K)]\n");
	fprintf(file, "\tkW			= 174;		// [W*/(m*K)]\n");
	fprintf(file, "\tkSi			= 120;		// [W*/(m*K)]\n");
	fprintf(file, "\tkPolySi		= 100;		// [W*/(m*K)]\n");
	fprintf(file, "\tkSiO2		= 1.4;		// [W*/(m*K)]\n");
	fprintf(file, "\tkSi3N4		= 20;		// [W*/(m*K)]\n");
	fprintf(file, "\tkAir		= 0.024;	// [W*/(m*K)]\n");
	fprintf(file, "\tkBTOx		= 0.55;		// [W*/(m*K)]\n");
	fprintf(file, "\tkH_679FG	= 0.8;		// [W*/(m*K)]\n");
	fprintf(file, "\tkSolderMask	= 0.26;		// [W*/(m*K)]\n");
	fprintf(file, "\tkSnAgCu		= 60;		// [W*/(m*K)]\n");
	fprintf(file, "\tkSnPb		= 50;		// [W*/(m*K)]\n");
	fprintf(file, "\t\n");
	fprintf(file, "\t//Masse Volumique\n");
	fprintf(file, "\tpCu		= 8.920;		// [g/(m^3)]\n");
	fprintf(file, "\tpAl		= 2.7;		// [g/(m^3)]\n");
	fprintf(file, "\tpSi		= 2.33;		// [g/(m^3)]\n");
	fprintf(file, "\tpAir		= 1.293e-3;		// [g/m^3)]\n");
	fprintf(file, "\t\n");
	fprintf(file, "\t//Capacite thermique Massique (1[Joule] = 1[Watt] * 1[seconde])\n");
	fprintf(file, "\tcCu		= 0.385;		// [J/g*K]\n");
	fprintf(file, "\tcAl		= 0.897;		// [J/g*K]\n");
	fprintf(file, "\tcSi		= 0.705;		// [J/g*K]\n");
	fprintf(file, "\tcAir		= 1.006;		// [J/g*K]\n");
	fprintf(file, "\t\n");
	fprintf(file, Pro_Function.c_str());
	fprintf(file, "\n");
	fprintf(file, "	TimeFct[] = ($Time < pulse) ? 1 : 0;\n");
	fprintf(file, "\n");
	fprintf(file, "\tFlux[] = Pmax / %g * TimeFct[];       // Power density [W/m^2]\n", HeatSurfaceArea);

	//fprintf(file, "	Flux[] = flux * TimeFct[];\n");
	fprintf(file, "	qVol[] = 0;\n");

	fprintf(file, "	t0 = 0;\n");
	fprintf(file, "	t1 = timemax;\n");
	fprintf(file, "	dt = dtime;\n");
	fprintf(file, "\n");
	fprintf(file, "	SaveFct[] = 0; //!($TimeStep %% 20) ;\n");
	fprintf(file, "	}\n");
	fclose(file);
}

void Output::HeatLayer(GeoPolygon* polygon, GDSGroup* cur_GDS) {
	// Make hole on BULK Surface
	//ProcessLayer BulkLayer;
	// Find Layer and Top or Bottom
	for (unsigned long i = 0; i < cur_GDS->layer_list.size(); i++) {
		ProcessLayer *layer = cur_GDS->layer_list[i];
		
		if (polygon->GetLayer() != layer)
		{
			if (polygon->GetLayer()->Height > layer->Height + layer->Thickness + 1.0e-3f)
				continue; // Too high
			if (polygon->GetLayer()->Height + polygon->GetLayer()->Thickness + 1.0e-3f < layer->Height)
				continue; // Too low
			if (!layer->Metal )
				continue; // Only METAL
		}
		else {
			continue;
		}
		
		// find Bulk Polygon
		GeoPolygon *poly_bulk;
		for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
			poly_bulk = cur_GDS->FullPolygonItems[j];
			if (poly_bulk->GetLayer() == layer) {
				// Bulk Poly found
				poly_bulk->AddHole(polygon);
				break;
			}
		}

	}

}

void Output::HeatLayer_PostProcess(GeoPolygon* polygon, GDSGroup* cur_GDS) {
	
	// Make hole on BULK Surface
	//ProcessLayer BulkLayer;
	// Find Layer and Top or Bottom
	for (unsigned long i = 0; i < cur_GDS->layer_list.size(); i++) {
		ProcessLayer *layer = cur_GDS->layer_list[i];

		if (polygon->GetLayer() != layer)
		{
			if (polygon->GetLayer()->Height > layer->Height + layer->Thickness + 1.0e-3f)
				continue; // Too high
			if (polygon->GetLayer()->Height + polygon->GetLayer()->Thickness + 1.0e-3f < layer->Height)
				continue; // Too low
			if (!layer->Metal)
				continue; // Only METAL
		}
		else {
			continue;
		}

		// find Bulk Polygon
		GeoPolygon *poly_bulk;
		for (size_t j = 0; j < cur_GDS->FullPolygonItems.size(); j++) {
			poly_bulk = cur_GDS->FullPolygonItems[j];
			if (poly_bulk->GetLayer() == layer) {
				// Bulk Poly found
				break;
			}
		}
		
		if (polygon->GetLayer()->Height == layer->Height) {
		// Bottom Connect

		// Create Polygon
		PolygonPoints(polygon, true, poly_bulk->GetMeshElemSize());

		EndGeo += "\n// HEATING Bottom Surface Modifiy " + poly_bulk->GetExtrudeVarName() + "\n";

		if(polygon->Bottom_Surfaces_Contact != "")
			EndGeo = EndGeo + "Physical Surface(\"" + polygon->GetLayer()->Name + to_string(cur_HEAT_Layer) + "\", " 
			    + to_string(cur_Physical_Index) + ") = { "
				+ polygon->GetSurfaceID() + ", "
			    + polygon->Bottom_Surfaces_Contact +" };\n";
		else
			EndGeo = EndGeo + "Physical Surface(\"" + polygon->GetLayer()->Name + to_string(cur_HEAT_Layer) + "\", "
			    + to_string(cur_Physical_Index) +") = { " + polygon->GetSurfaceID() + " };\n";  
		cur_Physical_Index += 1;

		EndGeo += "SurfacesLoop[] = Boundary { Volume{" + poly_bulk->GetVolumeID() + "}; } ; \n";
		EndGeo += "Delete {Volume { " + poly_bulk->GetVolumeID() + " }; }\n";
		// Delete All excepte bottom surface
		EndGeo += "Delete { Volume { " + polygon->GetVolumeID() + " }; }\n";
		EndGeo += "Delete { Surface { " + polygon->GetTopSurfaceID() + "}; }\n";
		EndGeo += "For i In {2 : 5}\n";
		EndGeo += "		Delete { Surface { " + polygon->GetExtrudeVarName() + "[i]};}\n";
		EndGeo += "EndFor\n";
		
		EndGeo += "Delete { Surface { " + poly_bulk->GetTopSurfaceID() + " }; }\n";
		EndGeo += "Plane Surface(" + poly_bulk->GetTopSurfaceID() + ") = { " + poly_bulk->GetTopLoopLinesID() + " };\n";
		EndGeo += "SurfacesLoop[] += {" + polygon->GetSurfaceID() + "} ;\n";
		EndGeo += "SurfacesLoop[] += {" + polygon->Bottom_Surfaces_Contact + "} ;\n";
		EndGeo += "NewSurfacLoop = news;\n";
		EndGeo += "Surface Loop (NewSurfacLoop) = { SurfacesLoop[] };\n";
		EndGeo += "Volume (" + poly_bulk->GetVolumeID() + ") = { NewSurfacLoop };\n";
		break;
		} else {
		// Top Connect


		EndGeo += "\n// HEATING Top Surface Modifiy " + poly_bulk->GetExtrudeVarName() + "\n";

		if (polygon->Top_Surfaces_Contact != "")
			EndGeo = EndGeo + "Physical Surface(\"" + polygon->GetLayer()->Name + to_string(cur_HEAT_Layer) + "\", " 
			+ to_string(cur_Physical_Index) + ") = { " + polygon->GetTopSurfaceID() + ", "
			+ polygon->Top_Surfaces_Contact + " };\n";
		else
			EndGeo = EndGeo + "Physical Surface(\"" + polygon->GetLayer()->Name + to_string(cur_HEAT_Layer) + "\", " 
			+ to_string(cur_Physical_Index) + ") = { " + polygon->GetTopSurfaceID() + " };\n";
		cur_Physical_Index += 1;
		EndGeo += "SurfacesLoop[] = Boundary { Volume{" + poly_bulk->GetVolumeID() + "}; } ; \n";
		EndGeo += "Delete {Volume { " + poly_bulk->GetVolumeID() + " }; }\n";
		// Delete All excepte top surface
		EndGeo += "Delete { Volume { " + polygon->GetVolumeID() + " }; }\n";
		EndGeo += "Delete { Surface { " + polygon->GetSurfaceID() + "}; }\n";
		EndGeo += "For i In {2 : 5}\n";
		EndGeo += "		Delete { Surface { " + polygon->GetExtrudeVarName() + "[i]};}\n";
		EndGeo += "EndFor\n";

		EndGeo += "Delete { Surface { " + poly_bulk->GetSurfaceID() + " }; }\n";
		EndGeo += "Plane Surface(" + poly_bulk->GetSurfaceID() + ") = { " + poly_bulk->GetLoopLinesID() + " };\n";
		EndGeo += "SurfacesLoop[] += {" + polygon->GetTopSurfaceID() + "} ;\n";
		EndGeo += "SurfacesLoop[] += {" + polygon->Top_Surfaces_Contact + "} ;\n";
		EndGeo += "NewSurfaceLoop = news;\n";
		EndGeo += "Surface Loop (NewSurfaceLoop) = { SurfacesLoop[] };\n";
		EndGeo += "Volume (" + poly_bulk->GetVolumeID() + ") = { NewSurfaceLoop };\n";
		break;
		}
		

	}
	// Add HEAT Surface to BULK Volume
	cur_HEAT_Layer += 1;


}

void Output::Recombine(GeoPolygon* Polygon_Dielec, ProcessLayer *layer) { 
	// layer Upper Layer to recombine with
	for (size_t l = 0; l < Polygon_Dielec->GetHoles().size(); l++) {
		vector<GeoPolygon*> PolyList;
		//Polygon in the Dielec-Polygon
		PolyList = Polygon_Dielec->GetHoles()[l]->FindPolyInside(cur_GDS->FullSortPolygonItems.GetPolyNear(*Polygon_Dielec->Get3DBBox()), false);
		for (size_t m = 0; m < PolyList.size(); m++) {
			GeoPolygon* Polygon = PolyList[m];
			for (size_t h = 0; h < Polygon->GetHoles().size(); h++) {
				GeoPolygon* Hole = Polygon->GetHoles()[h];
				//string Modif = Hole->GetBottomSurfaceIDListOf(cur_GDS->FullPolygonItems, layer);
				string Modif = Hole->GetBottomSurfaceIDListOf(cur_GDS->FullSortPolygonItems.GetPolyNear(*Hole->Get3DBBox()), layer);
				Recombine(Hole, layer);
				if (Modif == "")
					continue;
				EndGeo += "// Combine Volume Hole " + Hole->GetExtrudeVarName() + "\n";
				EndGeo += "SurfacesLoop[] = Boundary { Volume{" + Hole->GetVolumeID() + "}; } ; \n";
				EndGeo += "Delete {Volume { " + Hole->GetVolumeID() + " }; }\n";
				EndGeo += "Delete { Surface { " + Hole->GetTopSurfaceID() + " }; }\n";
				EndGeo += "Plane Surface ( " + Hole->GetTopSurfaceID() + " ) =  { " + Hole->GetTopLoopLinesID() + ", " + Modif + "};\n";
				EndGeo += "SurfaceList[]={};\n";
				EndGeo += "For i In {0 : #SurfacesLoop[]-1}\n";
				EndGeo += "  If (SurfacesLoop[i] != " + Hole->GetTopSurfaceID() + ")\n";
				EndGeo += "    SurfaceList[] += {SurfacesLoop[i]};\n";
				EndGeo += "  Else\n";
				EndGeo += "    SurfaceList[] += {" + Hole->GetTopSurfaceID() + "};\n";
				EndGeo += "    SurfaceList[] += {" + Modif + "};\n";
				EndGeo += "  EndIf\n";
				EndGeo += "EndFor\n";
				EndGeo += "NewSurfacLoop = news;\n";
				EndGeo += "Surface Loop (NewSurfacLoop) = { SurfaceList[] };\n";
				EndGeo += "Volume (" + Hole->GetVolumeID() + ") = { NewSurfacLoop };\n";
			}
		}

	}
}

void Output::PolygonPoints(GeoPolygon * polygon, bool dielectrique, double MeshElemSize){
	ProcessLayer *layer = polygon->GetLayer();
	double z0 = floor(rounded(polygon->GetHeight()/ cur_GDS->Unit))* cur_GDS->Unit;
	GeoPolygon* cur_Hole;
	
	if (polygon->IsWriten())
		return;
	
	vector<GeoPolygon*> PolyItems;
	GeoPolygon* poly_BottomMesh = new GeoPolygon(cur_GDS->Name, polygon, polygon->IsHole());
	//poly_BottomMesh->SetMeshElemSize(polygon->GetMeshElemSize());
	//PolyItems = GetContactPolyItems(polygon, cur_GDS->FullPolygonItems, false /*Bottom*/);
	PolyItems = GetContactPolyItems(polygon, cur_GDS->FullSortPolygonItems.GetPolyNear(*polygon->Get3DBBox()), false /*Bottom*/);
	//for (size_t k = 0; k < PolyItems.size(); k++) {
	//	GeoPolygon *poly_contact = PolyItems[k];
		//double poly_contact_currentMesh = poly_contact->GetMeshElemSize();
		//if (poly_contact->GetMeshElemSize() < poly_BottomMesh->GetMeshElemSize() / 10)
		//	poly_BottomMesh->SetMeshElemSize(poly_contact);
	//}


	//polygon->SetSurface(cur_Line_Index + polygon->GetPoints() + 1);
	//polygon->SetPoint_Index(cur_Line_Index + polygon->GetPoints() + 1);
	polygon->SetSurface(cur_Surface_Index);
	if (polygon->IsHole())
		fprintf(file, "//Start Polygon %s (Dielec-%s)\n", polygon->GetExtrudeVarName().c_str(), layer->Name);
	else
		fprintf(file, "//Start Polygon %s (%s)\n", polygon->GetExtrudeVarName().c_str(), layer->Name);
	cur_Surface_Index += 2; // Two surface Top and Bottom
	

	//Points (n)
	polygon->SetPoint_Index(cur_Point_Index);
	fprintf(file, polygon->GetBottomGeo(cur_Line_Index).c_str());
	/*
	//Field
	fprintf(file, "AttField = %zd;\n", cur_Field_Index);
	fprintf(file, "Field[AttField] = Attractor;\n");
	fprintf(file, "Field[AttField].EdgesList = {");
	polygon->AddLineLoop(atoi(polygon->GetSurfaceID().c_str()));
	for (size_t j = 0; j < polygon->GetPoints(); j++) {

		if (j == polygon->GetPoints() - 1)
			fprintf(file, "%zd", cur_Line_Index + j);
		else
			fprintf(file, "%zd,", cur_Line_Index + j);
	}
	fprintf(file, " };\n");
	fprintf(file, "ThrField = %zd;\n", cur_Field_Index+1);
	fprintf(file, "Field[ThrField] = Threshold;\n");
	fprintf(file, "Field[ThrField].IField = AttField;\n");
	fprintf(file, "Field[ThrField].LcMin = %f*cl%d;\n",polygon->GetMeshElemSize(), layer->Index);
	fprintf(file, "Field[ThrField].LcMax = %f*cl%d;\n", MeshElemSize, layer->Index);
	fprintf(file, "Field[ThrField].DistMin = %f*cl%d;\n", polygon->GetMeshElemSize(), layer->Index);
	fprintf(file, "Field[ThrField].DistMax = 10*%f*cl%d;\n", polygon->GetMeshElemSize(), layer->Index);
	cur_Field_Index += 2;
	*/
	cur_Point_Index +=  polygon->GetPoints();
	cur_Line_Index += polygon->GetPoints();

	if (polygon->GetHoles().size() > 0) {

		if(polygon->IsHole())
			fprintf(file, "//Start Hole Definition for polygon :%s (Dielec-%s)\n", polygon->GetExtrudeVarName().c_str(), layer->Name);
		else
			fprintf(file, "//Start Hole Definition for polygon :%s (%s)\n", polygon->GetExtrudeVarName().c_str(), layer->Name);
		if (!dielectrique) {
			// Create Holes polygon
			for (size_t i = 0; i < polygon->GetHoles().size(); i++) {
				cur_Hole = polygon->GetHoles()[i];
				vector<GeoPolygon*>  PolyItems = cur_Hole->GetHoles();
				if (PolyItems.size() > 0 ) {
					for (size_t j = 0; j < PolyItems.size(); j++) {
						// Write Unknow Polygon First
						PolygonPoints(PolyItems[j], dielectrique, polygon->GetMeshElemSize());
					}
					PolygonPoints(cur_Hole, !dielectrique, polygon->GetMeshElemSize());
				} else {
					PolygonPoints(cur_Hole, !dielectrique, polygon->GetMeshElemSize());
				}
				
			}
		}
		for (size_t i = 0; i < polygon->GetHoles().size(); i++) {
			GeoPolygon *cur_hole = polygon->GetHoles()[i];
			if (!cur_hole->IsWriten())
				PolygonPoints(cur_hole, !dielectrique, polygon->GetMeshElemSize());
		}

		fprintf(file, "Plane Surface(%s) = { %s ", polygon->GetSurfaceID().c_str(), polygon->GetLoopLinesID().c_str());

		for (size_t i = 0; i < polygon->GetHoles().size(); i++) {
			GeoPolygon *cur_hole = polygon->GetHoles()[i];
			fprintf(file, ", %s", cur_hole->GetLoopLinesID().c_str());

		}
		
		fprintf(file, "};\n");
		if (dielectrique) {

			// Find Polygon inside Hole
			for (size_t k = 0; k < polygon->GetHoles().size(); k++) {
				GeoPolygon *cur_Polygon = polygon->GetHoles()[k];
				vector<GeoPolygon*> PolyList;
				for (size_t i = 0; i < cur_Polygon->GetHoles().size(); i++) {
					GeoPolygon *cur_HolePolygon = cur_Polygon->GetHoles()[i];
					Polygon(cur_HolePolygon, cur_HolePolygon->GetLayer(), cur_HolePolygon->IsHole(), cur_Polygon->GetMeshElemSize());

				}
			}
		}
	}
	else
		fprintf(file, "Plane Surface(%s) = { %s };\n", polygon->GetSurfaceID().c_str(), polygon->GetLoopLinesID().c_str());
	/*
	if (polygon->GetLayer()->Metal == 0) {
		fprintf(file, polygon->GetBottomGeoTransfinite().c_str());
	}*/
	fprintf(file, "//End  Definition for polygon :%s\n", polygon->GetExtrudeVarName().c_str());
	
	if ((layer->Metal && !dielectrique) || (layer->Metal == 0 && dielectrique)) {
		ConnectPoly(polygon, false /*Bottom*/);
	}
}

void Output::PolygonExtrude(GeoPolygon *polygon, ProcessLayer *do_layer, bool dielectrique, double MeshElemSize) {

	vector<GeoPolygon*> PolyTopItems;
	vector<GeoPolygon*> GDSPolyItems;

	//GeoPolygon* cur_Hole;

	ProcessLayer *layer = polygon->GetLayer();
	if (do_layer && layer != do_layer)
		return;

	assert(polygon->GetLayer() == do_layer);
	assert(polygon->IsHole() == dielectrique);

	PolygonPoints(polygon, dielectrique, MeshElemSize);
	if (polygon->IsExtruded_Points())
		return;
	polygon->SetExtruded_Points(true);

	if (polygon->GetHoles().size() > 0) {

		Surface_Geo += "//Start Extrude Hole Definition for polygon :" + polygon->GetExtrudeVarName() + " (" + polygon->GetLayer()->Name + ")\n";
		for (size_t i = 0; i < polygon->GetHoles().size(); i++) {
			GeoPolygon *cur_hole = polygon->GetHoles()[i];
			if (!cur_hole->IsExtruded())
				PolygonExtrude(cur_hole, cur_hole->GetLayer(), cur_hole->IsHole(), polygon->GetMeshElemSize());
		}
	}

	Surface_Geo += polygon->Extrude(cur_Point_Index, cur_Line_Index);

}

void Output::Polygon(GeoPolygon *polygon, ProcessLayer *do_layer) {
	Polygon(polygon, do_layer, false);
}

void Output::Polygon(GeoPolygon *polygon, ProcessLayer *do_layer, bool dielectrique) {
	Polygon(polygon, do_layer, dielectrique, polygon->GetMeshElemSize());
}
void Output::Polygon(GeoPolygon *polygon, ProcessLayer *do_layer, bool dielectrique, double MeshElemSize) {

	
	GeoPolygon* cur_Hole;
	
	ProcessLayer *layer = polygon->GetLayer();
	if (do_layer && layer != do_layer)
		return;
	
	assert(polygon->GetLayer() == do_layer);
	assert(polygon->IsHole() == dielectrique);

	PolygonPoints(polygon,dielectrique, MeshElemSize);
	
	PolygonExtrude(polygon, do_layer, dielectrique, MeshElemSize);
	//EndGeo = EndGeo + "Geometry.AutoCoherence = 1;\n";
	if (polygon->IsExtruded())
		return;
	polygon->SetExtruded(true);

	if(dielectrique) { 
		// Create Dielectrique Volume
		//vector<GeoPolygon*> PolyList = polygon->FindPolyInside(cur_GDS->FullSortPolygonItems.GetPolyNear(*polygon->Get3DBBox()),false);
		//vector<GeoPolygon*> PolyList = polygon->FindPolyInside(cur_GDS->FullPolygonItems, false);
		vector<GeoPolygon*> PolyList = polygon->GetHoles();
		for(size_t i=0; i<PolyList.size() ; i++) {
			for(size_t j=0; j<PolyList[i]->GetHoles().size();j++){
				cur_Hole = PolyList[i]->GetHoles()[j];
				Polygon(cur_Hole, do_layer, dielectrique);
			}
		}
	}

	//if (do_layer->Metal && !dielectrique || do_layer->Metal==0 && dielectrique) {
	if ((do_layer->Metal && !dielectrique) || (do_layer->Metal == 0 && dielectrique)) {
		//TopConnectPoly(polygon);
		ConnectPoly(polygon, true /*Top*/);
	}

	Volume_Geo += polygon->Extrude_Volume();

	size_t TotalNbPoly = cur_GDS->FullPolygonItems.size();
	if(!dielectrique)
		cur_GDS->PolygonWrited += 1;
	// Check timer?
	if (wm->timer(time, 0) > Waittime) {
		v_printf(0, "\rLayer %s Done %5.2f%% (%zd/%zd)", do_layer->Name, 1.0*(cur_GDS->PolygonWrited) / (TotalNbPoly)*100.0, cur_GDS->PolygonWrited, TotalNbPoly);
		fflush(stdout);
		wm->timer(time, 1);
	}

}
void Output::ConnectPoly(GeoPolygon *polygon, bool Top) {
	vector<GeoPolygon*> PolyItems;
	vector<GeoPolygon*> GDSPolyItems;

	ProcessLayer *do_layer = polygon->GetLayer();
	bool dielectrique = polygon->IsHole();
	string *Surfaces_Contact;
	if (Top) {
		Surfaces_Contact = &polygon->Top_Surfaces_Contact;
	}
	else {
		Surfaces_Contact = &polygon->Bottom_Surfaces_Contact;
	}

	string Surface_LineLoops = "";
	// Get All Contact Poly
	vector<GeoPolygon*> PolyItemsNear = cur_GDS->FullSortPolygonItems.GetPolyNear(*polygon->Get3DBBox());
	//PolyItems = GetContactPolyItems(polygon, cur_GDS->FullPolygonItems, Top /*Top*/);
	PolyItems = GetContactPolyItems(polygon, PolyItemsNear, Top /*Top*/);
	if (do_layer->Metal && do_layer->Top && !dielectrique)
		// Connect other GDS
		for (size_t GDSindex = 0; GDSindex < FullGDSItems.size(); GDSindex++) {
			if (FullGDSItems[GDSindex] == cur_GDS)
				continue;
			//GDSPolyItems = GetContactPolyItems(polygon, FullGDSItems[GDSindex]->FullPolygonItems, Top /*Top*/);
			GDSPolyItems = GetContactPolyItems(polygon, FullGDSItems[GDSindex]->FullSortPolygonItems.GetPolyNear(*polygon->Get3DBBox()), Top /*Top*/);
			if (GDSPolyItems.size() > 0)
				for (size_t polyindex = 0; polyindex < GDSPolyItems.size(); polyindex++) {
					PolyItems.push_back(GDSPolyItems[polyindex]);
				}
		}
	if (PolyItems.size() > 0) {
		Surface_LineLoops = "";

		// Modify Poly hole
		// refine PolyList
		vector<GeoPolygon*> PolyholesItems;
		vector<GeoPolygon*> PolyholesItemsInside;
		for (size_t k = 0; k < PolyItems.size(); k++) {
			GeoPolygon *poly_contact = PolyItems[k];
			// reject Bigger Polygon
			if (poly_contact->GDSPolygon::isPolygonInside(*polygon))
				continue;
			bool IsInside = false;
			for (size_t l = 0; l < PolyholesItems.size(); l++) {
				GeoPolygon *poly_hole = PolyholesItems[l];
				if (poly_hole->GDSPolygon::isPolygonInside(*poly_contact)) {
					IsInside = true;
					break;
				}
			}
			if (!IsInside) {
				vector<GeoPolygon*> PolytmpItems;
				PolytmpItems.push_back(poly_contact);
				for (size_t l = 0; l < PolyholesItems.size(); l++) {
					GeoPolygon *poly_hole = PolyholesItems[l];
					if (!poly_contact->GDSPolygon::isPolygonInside(*poly_hole)) {
						PolytmpItems.push_back(poly_hole);
					}
					else {
						PolyholesItemsInside.push_back(poly_hole);
					}
				}
				PolyholesItems = PolytmpItems;
			}
			else {
				PolyholesItemsInside.push_back(poly_contact);
			}
		}
		// Add hole(s)
		for (size_t k = 0; k < PolyholesItems.size(); k++) {
			GeoPolygon *poly_contact = PolyholesItems[k];
			string(GeoPolygon::*GetContactLineLoop)();
			if (Top) {
				GetContactLineLoop = &GeoPolygon::GetLineLoop;
			}
			else {
				GetContactLineLoop = &GeoPolygon::GetTopLoopLinesID;
			}
			if (do_layer->Metal == 0 && dielectrique) {
				if (poly_contact->isPolygonInside(*polygon))
					continue;
				else if (poly_contact->GDSPolygon::isPolygonInside(*polygon)
					&& poly_contact->GetHoles().size() > 0
					&& polygon->GDSPolygon::isPolygonInside(*poly_contact->GetHoles()[0])
					&& !polygon->GetHoles()[0]->GDSPolygon::isPolygonInside(*poly_contact->GetHoles()[0])
					)
					continue;
			}

			if (Top) {
				if (!poly_contact->IsWriten())
					PolygonPoints(poly_contact, poly_contact->IsHole(), polygon->GetMeshElemSize());
			}
			else {
				if (!poly_contact->IsExtruded())
					Polygon(poly_contact, poly_contact->GetLayer(), poly_contact->IsHole(), polygon->GetMeshElemSize());

			}
			if (Surface_LineLoops != "") {
				char lastChar = *Surface_LineLoops.rbegin();
				if (lastChar != ',' && lastChar != ' ')
					Surface_LineLoops = Surface_LineLoops + ", ";
			}
			Surface_LineLoops = Surface_LineLoops + (poly_contact->*GetContactLineLoop)();
		}
		if (polygon->GetHoles().size() > 0) {
			string tmpStr = polygon->GetHolesLoopLinesList(Top, PolyholesItems);
			if (tmpStr != "")
				if (Surface_LineLoops == "")
					Surface_LineLoops = tmpStr;
				else
					Surface_LineLoops = Surface_LineLoops + ", " + tmpStr;
		}

		// Add surface contact
		bool AddHole = false;
		for (size_t k = 0; k < PolyItems.size(); k++) {
			GeoPolygon *poly_contact = PolyItems[k];
			string(GeoPolygon::*GetContactSurfaceID) () = &GeoPolygon::GetBottomSurfaceID;
			string(GeoPolygon::*GetContactLoopLinesID) () = &GeoPolygon::GetLoopLinesID;
			if (Top) {
				GetContactSurfaceID = &GeoPolygon::GetBottomSurfaceID;
				GetContactLoopLinesID = &GeoPolygon::GetLoopLinesID;
			}
			else {
				GetContactSurfaceID = &GeoPolygon::GetTopSurfaceID;
				GetContactLoopLinesID = &GeoPolygon::GetTopLoopLinesID;
			}
			// reject Bigger Polygon
			if (poly_contact->isPolygonInside(*polygon))
				continue;

			bool AddPoly = true;
			if (do_layer->Metal == 0 && dielectrique) {
				if (poly_contact->GDSPolygon::isPolygonInside(*polygon)
					&& poly_contact->GetHoles().size() > 0
					&& polygon->GDSPolygon::isPolygonInside(*poly_contact->GetHoles()[0])
					&& !polygon->GetHoles()[0]->GDSPolygon::isPolygonInside(*poly_contact->GetHoles()[0])
					)
					AddPoly = false;
			}
			if (AddPoly) {
				if (Top) {
					if (!poly_contact->IsWriten())
						PolygonPoints(poly_contact, poly_contact->IsHole(), polygon->GetMeshElemSize());
				}
				else {
					if (!poly_contact->IsExtruded())
						Polygon(poly_contact, poly_contact->GetLayer(), poly_contact->IsHole(), polygon->GetMeshElemSize());
				}
				if (*Surfaces_Contact != "")
					*Surfaces_Contact += ", ";
				*Surfaces_Contact += (poly_contact->*GetContactSurfaceID)();
			}
			vector<GeoPolygon*> PolyList;
			GeoPolygon* cur_Hole;
			GeoPolygon* cur_Hole_poly;
			if (poly_contact->GetHoles().size() > 0) {

				for (size_t l = 0; l < poly_contact->GetHoles().size(); l++) {
					cur_Hole = poly_contact->GetHoles()[l];
					string(GeoPolygon::*HoleGetSurfaceID)();
					string(GeoPolygon::*HoleGetLoopLinesID)();
					if (Top) {
						HoleGetSurfaceID = &GeoPolygon::GetSurfaceID;
						HoleGetLoopLinesID = &GeoPolygon::GetLoopLinesID;
					}
					else {
						HoleGetSurfaceID = &GeoPolygon::GetTopSurfaceID;
						HoleGetLoopLinesID = &GeoPolygon::GetTopLoopLinesID;
					}
					// Not Inside
					if (polygon->GetHoles().size() >0 && polygon->GetHoles()[0]->GDSPolygon::isPolygonInside(*cur_Hole)) {
						// No change
						continue;
					}
					if (strcmp(cur_Hole->GetLayer()->Name, "HEATLAYER") == 0) {
						// Ignore Heat Layer
						continue;
					}
					// OverLap  polygon is inside poly_contact but poly_contact hole also contain some part of polygon
					if (poly_contact->GDSPolygon::isPolygonInside(*polygon)
						&& polygon->GDSPolygon::isPolygonInside(*cur_Hole)
						&& polygon->GetHoles().size() > 0
						&& !polygon->GetHoles()[0]->GDSPolygon::isPolygonInside(*cur_Hole)) {


						// Remove Surface
						if (*Surfaces_Contact != "") {
							*Surfaces_Contact += ", ";
						}
						if (Top) {
							if (!cur_Hole->IsWriten())
								PolygonPoints(cur_Hole, cur_Hole->IsHole(), poly_contact->GetMeshElemSize());
						}
						else {
							if (!cur_Hole->IsExtruded())
								Polygon(cur_Hole, cur_Hole->GetLayer(), cur_Hole->IsHole(), poly_contact->GetMeshElemSize());
						}
						*Surfaces_Contact += (cur_Hole->*HoleGetSurfaceID)();
						Surface_LineLoops = (cur_Hole->*HoleGetLoopLinesID)();
						continue;
					}
					// OverLap  polygon is inside poly_contact but poly_contact hole also contain some part of polygon
					if (poly_contact->GDSPolygon::isPolygonInside(*polygon)
						&& polygon->GDSPolygon::isPolygonInside(*cur_Hole)
						&& polygon->GetHoles().size() == 0
						) {
						// Remove Bottom Surface
						std::string::size_type s_i;
						if (*Surfaces_Contact != "") {
							if (Surfaces_Contact->length() == (poly_contact->*GetContactSurfaceID)().length())
								s_i = Surfaces_Contact->find((poly_contact->*GetContactSurfaceID)());
							else {
								s_i = Surfaces_Contact->find(", " + (poly_contact->*GetContactSurfaceID)());
								if (s_i != std::string::npos)
									s_i += 2;
							}
							if (s_i != std::string::npos)
								Surfaces_Contact->erase(s_i, (poly_contact->*GetContactSurfaceID)().length());
							else {
								if (*Surfaces_Contact != "")
									*Surfaces_Contact += ", ";
							}
						}
						if (Top) {
							if (!cur_Hole->IsWriten())
								PolygonPoints(cur_Hole, cur_Hole->IsHole(), poly_contact->GetMeshElemSize());
						}
						else {
							if (!cur_Hole->IsExtruded())
								Polygon(cur_Hole, cur_Hole->GetLayer(), cur_Hole->IsHole(), poly_contact->GetMeshElemSize());
						}
						*Surfaces_Contact += (poly_contact->*GetContactSurfaceID)();
						Surface_LineLoops = (poly_contact->*GetContactLoopLinesID)();
						continue;
					}
					// OverLap  poly_contact is inside polygon but polygon hole also contain some part of poly_contact
					if (polygon->GDSPolygon::isPolygonInside(*poly_contact)
						&& polygon->GetHoles().size() > 0
						&& poly_contact->GDSPolygon::isPolygonInside(*polygon->GetHoles()[0])
						&& !polygon->isPolygonInside(*cur_Hole)
						&& !cur_Hole->GDSPolygon::isPolygonInside(*polygon)) {
						if (false && !polygon->isPolygonInside(*poly_contact)) {
							string SurfaceVarName = "NewSurface";
							int NbOcc = FindOccurOf("NewSurface", *Surfaces_Contact);
							if (NbOcc > 0) {
								SurfaceVarName += to_string(NbOcc);
							}
							polygon->EndGeo += SurfaceVarName + " = news;\n";
							polygon->EndGeo += "Plane Surface(" + SurfaceVarName + ") = { " + (poly_contact->*GetContactLoopLinesID)() + ", " + (cur_Hole->*HoleGetLoopLinesID)() + " };\n";
							// Remove TopSurface
							std::string::size_type s_i;
							if (Surfaces_Contact->length() == (poly_contact->*GetContactSurfaceID)().length())
								s_i = Surfaces_Contact->find((poly_contact->*GetContactSurfaceID)());
							else {
								s_i = Surfaces_Contact->find(", " + (poly_contact->*GetContactSurfaceID)());
								if (s_i != std::string::npos)
									s_i += 2;
							}
							if (s_i != std::string::npos) {
								Surfaces_Contact->erase(s_i, (poly_contact->*GetContactSurfaceID)().length());
								*Surfaces_Contact += SurfaceVarName;
							}
							else {
								if (*Surfaces_Contact != "")
									*Surfaces_Contact += ", ";
								*Surfaces_Contact += SurfaceVarName;
							}
							l = poly_contact->GetHoles().size();
						}
						else {
							if (!cur_Hole->IsWriten())
								PolygonPoints(cur_Hole, cur_Hole->IsHole(), poly_contact->GetMeshElemSize());
							if (*Surfaces_Contact != "")
								*Surfaces_Contact += ", ";
							*Surfaces_Contact += (cur_Hole->*HoleGetSurfaceID)();
						}
						//l = poly_contact->GetHoles().size();
						continue;
					}
					bool AllIn = true;
					/*for (size_t m = 0; m < cur_Hole->GetHoles().size(); m++) {
						cur_Hole_poly = cur_Hole->GetHoles()[m];
						if (!polygon->isPolygonInside(*cur_Hole_poly)) {
							AllIn = false;
							break;
						}
					}*/
					if (AllIn) {
						PolyList.push_back(cur_Hole);
						AddHole = true;
					}

					if (AddHole && !AllIn) {
						vector<GeoPolygon*> poly_hole_List;

						for (size_t i = 0; i < polygon->GetHoles().size(); i++) {
							GeoPolygon *poly_hole = polygon->GetHoles()[i];
							if (cur_Hole->GDSPolygon::isPolygonInside(*poly_hole))
								poly_hole_List.push_back(poly_hole);
						}
						string SurfaceVarName = "NewSurface";
						int NbOcc = FindOccurOf("NewSurface", *Surfaces_Contact);
						if (NbOcc > 0) {
							SurfaceVarName += to_string(NbOcc);
						}

						polygon->EndGeo += SurfaceVarName + " = news;\n";
						polygon->EndGeo += "Plane Surface(" + SurfaceVarName + ") = { " + (cur_Hole->*HoleGetLoopLinesID)();
						for (size_t i = 0; i < PolyholesItemsInside.size(); i++) {
							GeoPolygon *poly_hole = PolyholesItemsInside[i];
							if (!cur_Hole->GDSPolygon::isPolygonInside(*poly_hole))
								continue;
							if (Top && poly_hole->IsWriten()) {
								// Create Polygon as it doesn't existe yet
								PolygonPoints(poly_hole, poly_hole->IsHole(), polygon->GetMeshElemSize());
							}
							else if (!Top && !polygon->GetHoles()[0]->IsExtruded()) {
								// Create Polygon as it doesn't existe yet
								Polygon(poly_hole, do_layer, poly_hole->IsHole(), polygon->GetMeshElemSize());
							}
							if (Top)
								polygon->EndGeo += ", " + poly_hole->GetLoopLinesID();
							else
								polygon->EndGeo += ", " + poly_hole->GetTopLoopLinesID();
						}
						polygon->EndGeo += " };\n";
						if (*Surfaces_Contact != "")
							*Surfaces_Contact += ", ";
						*Surfaces_Contact += SurfaceVarName;

					}
				}
			}

			if (PolyList.size() > 0) {
				for (size_t l = 0; l < PolyList.size(); l++) {
					GeoPolygon *cur_poly = PolyList[l];
					// Check if current poly is Via and Dielectrique then extrude metal contact polygon
					if (do_layer->Metal == 0 && dielectrique)
						Polygon(cur_poly, cur_poly->GetLayer(), cur_poly->IsHole());
					if (Top && cur_poly->GetLayer()->Metal == 0 && cur_poly->IsHole())
						PolygonPoints(cur_poly, cur_poly->IsHole(), poly_contact->GetMeshElemSize());
					if (!Top && !cur_poly->IsExtruded()) {
						// Create Polygon as it doesn't existe yet
						Polygon(cur_poly, cur_poly->GetLayer(), cur_poly->IsHole(), poly_contact->GetMeshElemSize());
					}
					if (*Surfaces_Contact != "")
						*Surfaces_Contact += ", ";
					if (Top)
						*Surfaces_Contact += cur_poly->GetSurfaceID();
					else
						*Surfaces_Contact += cur_poly->GetTopSurfaceID();
				}
			}
		}
		if (Surface_LineLoops != "") {
			if (polygon->GetHoles().size() > 0) {

				for (size_t i = 0; i < polygon->GetHoles().size(); i++) {
					GeoPolygon *cur_hole = polygon->GetHoles()[i];
					bool Inside = false;
					for (size_t k = 0; k < PolyholesItems.size(); k++) {
						GeoPolygon *poly_contact = PolyholesItems[k];
						if (poly_contact->GDSPolygon::isPolygonInside(*cur_hole)) {
							Inside = true;
							break;
						}
					}
					if (!Inside) {
						if (Surface_LineLoops != "")
							Surface_LineLoops += ", ";
						if (Top)
							Surface_LineLoops += cur_hole->GetTopLoopLinesID();
						else
							Surface_LineLoops += cur_hole->GetLoopLinesID();
					}
				}
			}

			if (Top) {
				Surface_Geo += "// Top Surface Modify\n";
				Surface_Geo += "Delete {Surface { " + polygon->GetTopSurfaceID() + " }; }\n";
				//if (polygon->EndGeo != "")
				//	Surface_Geo += polygon->EndGeo;
				Surface_Geo += "Plane Surface( " + polygon->GetTopSurfaceID() + " ) = { " + polygon->GetTopLoopLinesID() + ", " + Surface_LineLoops + " };\n";
			}
			else {
				Surface_Geo += "// Bottom Surface Modify\n";
				Surface_Geo += "Delete {Surface { " + polygon->GetSurfaceID() + " }; }\n";
				// Remove ??
				//if (polygon->EndGeo != "")
				//	Surface_Geo += polygon->EndGeo;
				Surface_Geo += "Plane Surface(" + polygon->GetSurfaceID() + ") = { " + polygon->GetLoopLinesID() + ", " + Surface_LineLoops + " };\n";
			}

		}
	}
}

vector<GeoPolygon*>  Output::SimplifyPolyItems(vector<GDSPolygon*> PolygonItems, ProcessLayer *Layer) {
	GDSPolygon *poly;
	GDSPolygon *biggest_poly;
	GDSPolygon *polygon;
	PolygonSort PolygonItemsChecked;
	set<GDSPolygon*> PolygonRemoveItems;
	set<GDSPolygon*> PolygonItemsOutSet;
	vector<GDSPolygon*> PolygonOnLayer;
	
	GeoPolygon *GEO_poly;
	GeoPolygon *GEO_poly_it;
	vector<GeoPolygon*> PolygonItemsOutVec;

	biggest_poly = NULL;
	int WarningCnt = 0;

	for (size_t i = 0; i < PolygonItems.size(); i++)
	{
		poly = PolygonItems[i];
		// Exclude for different layer
		if (poly->GetLayer() != Layer)
			if (strcmp(poly->GetLayer()->ProcessName, Layer->ProcessName) != 0 || poly->GetLayer()->Height != Layer->Height || poly->GetLayer()->Thickness != Layer->Thickness)
				// if same process and same high and same thickness then consider  as same layer
				continue;
		PolygonOnLayer.push_back(poly);
	}

	PolygonItemsChecked.Add(PolygonOnLayer);
	
	// Reset Timer
	wm->timer(time, 1);
	size_t TotalNbPoly = PolygonOnLayer.size();

	for (size_t i = 0; i < PolygonOnLayer.size(); i++)
	{
		poly = PolygonOnLayer[i];

		// Eclude if list as remove
		if (PolygonRemoveItems.find(poly) != PolygonRemoveItems.end()) {
			continue;
		}
		if (biggest_poly == NULL || poly->GetBBox()->isBBInside(*biggest_poly->GetBBox()))
			biggest_poly = poly;
		PolygonItemsOutSet.insert(poly);
		//PolygonItemsChecked.push_back(poly);

		vector<GDSPolygon*> PolygonItemstoCheck = PolygonItemsChecked.GetPolyNear(*poly->GetBBox());
		

		//for (size_t j = 0; j < PolygonItemsChecked.size()-1; j++) {
		for (size_t j = 0; j < PolygonItemstoCheck.size(); j++) {
			
			// Check timer?
			if (wm->timer(time, 0) > Waittime) {
				v_printf(0, "\rLayer %s Done %5.2f%% (%zd/%zd)", Layer->Name, 1.0*(i+j/ TotalNbPoly) / (TotalNbPoly)*100.0, i, TotalNbPoly);
				fflush(stdout);
				wm->timer(time, 1);
			}

			//polygon = PolygonItemsChecked[j];
			polygon = PolygonItemstoCheck[j];
			
			if(polygon == poly)
				continue;

			if (PolygonRemoveItems.find(polygon) != PolygonRemoveItems.end())
				continue;
			
			// No BB intersect
			if(!GDSBB::intersect(*poly->GetBBox(), *polygon->GetBBox()))
				continue;
			
			if (polygon->isPolygonInside_wborders(*poly)) {
				// Remove poly
				PolygonRemoveItems.insert(poly);
				break;
			}			
			
			if (poly->isPolygonInside_wborders(*polygon)) {
				// Remove polygon
				PolygonRemoveItems.insert(polygon);
				continue;

			} else if (GDSPolygon::intersect(poly, polygon)) {
				// Merge poly and  polygon
				if (WarningCnt < 8) {
					v_printf(0, "\rWARNING, Merge polygon on layer %s\n", Layer->Name);
					WarningCnt += 1;
				}
				else if (WarningCnt == 8) {
					v_printf(0, "\rWARNING, Merge polygon on layer %s (Last repport ...)\n", Layer->Name);
					WarningCnt += 1;
				}
				if (poly->GetBBox()->isBBInside(*polygon->GetBBox())) {
					poly->Merge(polygon);
					PolygonRemoveItems.insert(polygon);
				}
				else {
					polygon->Merge(poly);
					PolygonRemoveItems.insert(poly);
					break;
				}
			}
		}
	}

	// Remove useless polygons
	if (PolygonRemoveItems.size() > 0) {
		for (set<GDSPolygon*>::iterator it = PolygonRemoveItems.begin(); it != PolygonRemoveItems.end(); ++it) {
			poly = *it;
			PolygonItemsOutSet.erase(poly);

		}
	}
	
	// Translate GDSPolygon to GeoPolygon
	if (PolygonItemsOutSet.find(biggest_poly) != PolygonItemsOutSet.end()) {
		GEO_poly = new GeoPolygon(cur_GDS->Name, biggest_poly, false /*isHole*/);
		PolygonItemsOutVec.push_back(GEO_poly);
	}
	else {
		biggest_poly = NULL;
	}

	for (set<GDSPolygon*>::iterator it = PolygonItemsOutSet.begin(); it != PolygonItemsOutSet.end(); ++it) {
		poly = *it;
		if (biggest_poly != poly) {
			GEO_poly = new GeoPolygon(cur_GDS->Name, poly, false /*isHole*/);
			PolygonItemsOutVec.push_back(GEO_poly);
		}
	}
	
	// Extract Hole from polygons
	Point2D Intersect;
	for (size_t i = 0; i < PolygonItemsOutVec.size(); i++) {
		GEO_poly_it = PolygonItemsOutVec[i];
		//if (GEO_poly_it->GetBBox()->max == Point2D(1160.375, 1433.805)) {
		//	assert(i == i);
		//}

		while (GEO_poly_it->intersect(&Intersect)) {
			GEO_poly_it->FindHole(Intersect);
		}
	}
	for (size_t i = 0; i < PolygonItemsOutVec.size(); i++) {
		GEO_poly_it = PolygonItemsOutVec[i];
		// Add Poly in holes
		for (size_t j = 0; j < GEO_poly_it->GetHoles().size(); j++) {
			GeoPolygon* cur_Hole = GEO_poly_it->GetHoles()[j];
			vector<GeoPolygon*>  PolyItems = cur_Hole->FindPolyInside(PolygonItemsOutVec, false);
			for (size_t j = 0; j < PolyItems.size(); j++) {
				cur_Hole->AddHole(PolyItems[j]);
			}
		}
	}
	return PolygonItemsOutVec;
}

vector<GeoPolygon *> Output::GetContactPolyItems(GeoPolygon *poly, vector<GeoPolygon*> PolygonItems, bool Top) {
	vector<GeoPolygon *> ContactPolyItems;
	GeoPolygon *cur_poly;
	for (size_t i = 0; i < PolygonItems.size(); i++) {
		cur_poly = PolygonItems[i];
		if (cur_poly->GetLayer()->Show ==0)
			continue;
		if (cur_poly->GetLayer() != poly->GetLayer())
		{
			if (cur_poly->GetHeight() > poly->GetHeight() + poly->GetThickness() + 1.0e-3f)
				continue; // Too high
			if (cur_poly->GetHeight() + cur_poly->GetThickness() + 1.0e-3f < poly->GetHeight())
				continue; // Too low
			if (cur_poly->GetLayer()->Metal == poly->GetLayer()->Metal )
				continue; // Only jump between VIA -> METAL or METAL -> VIA 
		}
		else {
			continue;
		}

		if (Top)
			if (cur_poly->GetHeight() < poly->GetHeight()) 
				continue; // Too Low contact
		if (!Top)
			if (cur_poly->GetHeight() > poly->GetHeight())
				continue; // Too High contact

		// Do bounds overlap?
		if (!GDSBB::intersect(*poly->GetBBox(), *cur_poly->GetBBox()))
			continue;

		// Intersects with polygon?
		if (!GeoPolygon::intersect(poly, cur_poly)) {
			continue;
		}

		bool SameFound = false;
		for (size_t j = 0; j < ContactPolyItems.size(); j++) {
			GDSPolygon *comp_poly = ContactPolyItems[j];
			if (comp_poly->GetBBox() == cur_poly->GetBBox()) {
				SameFound = true;
				break;
			}
		}

		if(!SameFound)
			ContactPolyItems.push_back(cur_poly);
	}
	return ContactPolyItems;
}
GeoPolySpace::~GeoPolySpace()
{
	//for (size_t i = 0; i < polys.size(); i++)
	//	delete polys[i];
}
void GeoPolySpace::Clear()
{
	polys.clear();
	cur_size = 0;
	_hasBBox = false;
	BBox.clear();
}
GDS3DBB GeoPolySpace::GetBB()
{
	if (_hasBBox) {
		return BBox;
	}
	BBox.clear();
	for (size_t i = 0; i < polys.size(); ++i) {
		GeoPolygon *cur_poly = polys[i];
		BBox.merge(*cur_poly->Get3DBBox());
	}
	_hasBBox = true;
	return BBox;
}

void GeoPolySpace::Add(vector<GeoPolygon*> polyList) {
	for (size_t i = 0; i < polyList.size(); i++) {
		polys.push_back(polyList[i]);
		BBox.merge(*polyList[i]->Get3DBBox());
		cur_size += 1;
	}
	_hasBBox = true;
}
void GeoPolySpace::Add(GeoPolygon * poly)
{
	polys.push_back(poly);
	BBox.merge(*poly->Get3DBBox());
	_hasBBox = true;
	cur_size += 1;

}
void GeoPolySpace::Remove(GeoPolygon * poly)
{
	PolySpace::Remove(poly);
}

size_t GeoPolySpace::size()
{
	return polys.size();
}
void GeoPolygonSort::SetLayerList(vector<struct ProcessLayer*> layer_list_arg)
{
	layer_list = layer_list_arg;
}
void GeoPolygonSort::Add(GeoPolygon* poly)
{
	PolygonSort::Add(poly);
}
void GeoPolygonSort::Add(vector<GeoPolygon *> polyList)
{
	//PolygonSort::Add(polyList);
	polys.Add(polyList);
}
vector<GeoPolygon*> GeoPolySpace::Get()
{
	return polys;
}

vector<GeoPolygon*> GeoPolygonSort::GetPolyNear(GDS3DBB BBox)
{
	//PolygonSort::GetPolyNear(BBox);
	Check();
	vector<GeoPolygon*> poly_list;
	vector<GeoPolygon*> polycheck_list;
	for (map<GDS3DBB, GeoPolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); ++BB_it) {
		GDS3DBB cur_BB = BB_it->first;
		if (cur_BB.intersect(BBox, cur_BB)) {
			polycheck_list = BB_it->second.polys;
			for (size_t i = 0; i < polycheck_list.size(); i++) {
				GeoPolygon *cur_poly = polycheck_list[i];
				poly_list.push_back(cur_poly);
			}
		}
	}
	return poly_list;
}
void GeoPolygonSort::SpaceDiv(GeoPolySpace poly_list) {
	double dx = poly_list.GetBB().max.X - poly_list.GetBB().min.X;
	double dy = poly_list.GetBB().max.Y - poly_list.GetBB().min.Y;
	double dz = poly_list.GetBB().max.Z - poly_list.GetBB().min.Z;
	double z_div;
	GeoPolySpace polys_0;
	GeoPolySpace polys_1;
	GeoPolySpace polys_2; // only if z split
	GeoPolySpace polys_3; // only if z split
	GDS3DBB BBox_0 = poly_list.GetBB();
	GDS3DBB BBox_2 = poly_list.GetBB();// only if z split
	bool split_z = true;

	if (dx > dy) {
		// divide X
		BBox_0.max.X -= dx / 2.0;
		BBox_2.max.X -= dx / 2.0;
	}
	else {
		// divide Y
		BBox_0.max.Y -= dy / 2.0;
		BBox_2.max.Y -= dy / 2.0;
	}
	//divide Z
	vector<ProcessLayer*> locallayerlist;
	for (size_t i = 0; i < layer_list.size(); i++) {
		ProcessLayer *cur_layer = layer_list[i];
		if ((cur_layer->Height + cur_layer->Thickness)*cur_layer->Units->Unitu <= poly_list.GetBB().max.Z
			&&cur_layer->Height*cur_layer->Units->Unitu >= poly_list.GetBB().min.Z)
			locallayerlist.push_back(cur_layer);
	}
	if (locallayerlist.size() > 1) {
		z_div = poly_list.GetBB().min.Z;
		double z_mid = poly_list.GetBB().min.Z + dz / 2.0;

		for (size_t i = 0; i < locallayerlist.size(); i++) {
			ProcessLayer *cur_layer = locallayerlist[i];
			if (fabs(cur_layer->Height*cur_layer->Units->Unitu - z_mid) < fabs(z_mid - z_div))
				z_div = cur_layer->Height*cur_layer->Units->Unitu;
		}
		BBox_0.min.Z = z_div;
		BBox_2.max.Z = z_div;

	}
	else {
		split_z = false;
	}

	vector<GeoPolygon*> Poly_Only_List = poly_list.Get();
	for (size_t i = 0; i < Poly_Only_List.size(); ++i) {
		GeoPolygon *cur_poly = Poly_Only_List[i];
		if (BBox_0.isBBInside_wborders(*cur_poly->Get3DBBox())) {
			polys_0.Add(cur_poly);
		} else if (split_z && BBox_2.isBBInside_wborders(*cur_poly->Get3DBBox())) {
			polys_2.Add(cur_poly);
		} else if (split_z && cur_poly->Get3DBBox()->max.Z < BBox_0.min.Z) {
			polys_3.Add(cur_poly);
		}
		else {
			polys_1.Add(cur_poly);
		}
	}

	if (polys_1.Get().size() == poly_list.Get().size()) {
		// All block put in the same part invert
		polys_1.Clear();
		polys_0.Clear();
		BBox_0 = poly_list.GetBB();
		BBox_2 = poly_list.GetBB();
		if (dx > dy) {
			// divide X
			BBox_0.min.X += dx / 2;
			BBox_2.min.X += dx / 2;
		}
		else {
			// divide Y
			BBox_0.min.Y += dy / 2;
			BBox_2.min.Y += dy / 2;
		}
		if (split_z) {
			BBox_0.min.Z = z_div;
			BBox_2.max.Z = z_div;
		}

		for (size_t i = 0; i < Poly_Only_List.size(); ++i) {
			GeoPolygon *cur_poly = Poly_Only_List[i];
			if (BBox_0.isBBInside_wborders(*cur_poly->Get3DBBox())) {
				polys_0.Add(cur_poly);
			} else if (split_z && BBox_2.isBBInside_wborders(*cur_poly->Get3DBBox())) {
				polys_2.Add(cur_poly);
			} else if (split_z && cur_poly->Get3DBBox()->max.Z < BBox_0.min.Z) {
				polys_3.Add(cur_poly);
			}
			else {
				polys_1.Add(cur_poly);
			}
		}
		if (polys_1.Get().size() == poly_list.Get().size()) {
			// All block put in the same part invert
			// Too Big blocs?
			PolyBySpace[polys_1.GetBB()] = polys_1;
			return;
		}
	}

	if (polys_0.Get().size() == poly_list.Get().size()) {
		// All block put in the same part invert
		polys_1.Clear();
		polys_0.Clear();
		BBox_0 = poly_list.GetBB();
		if (dx > dy) {
			// divide X
			BBox_0.min.X += dx / 2;
			BBox_2.min.X += dx / 2;
		}
		else {
			// divide Y
			BBox_0.min.Y += dy / 2;
			BBox_2.min.Y += dy / 2;
		}
		if (split_z) {
			BBox_0.min.Z = z_div;
			BBox_2.max.Z = z_div;
		}

		for (size_t i = 0; i < Poly_Only_List.size(); ++i) {
			GeoPolygon *cur_poly = Poly_Only_List[i];
			if (BBox_0.isBBInside_wborders(*cur_poly->Get3DBBox())) {
				polys_0.Add(cur_poly);
			} else if (split_z && BBox_2.isBBInside_wborders(*cur_poly->Get3DBBox())) {
				polys_2.Add(cur_poly);
			} else if (split_z && cur_poly->Get3DBBox()->max.Z < BBox_0.min.Z) {
				polys_3.Add(cur_poly);
			}
			else {
				polys_1.Add(cur_poly);
			}
		}
		if (polys_0.Get().size() == poly_list.Get().size()) {
			// All block put in the same part invert
			// Too Big blocs?
			PolyBySpace[polys_0.GetBB()] = polys_0;
			return;
		}
	}


	SpaceDivEnd(polys_0);
	SpaceDivEnd(polys_1);
	if (split_z) {
		SpaceDivEnd(polys_2);
		SpaceDivEnd(polys_3);
	}
}
void GeoPolygonSort::SpaceDivEnd(GeoPolySpace polyspace) {
	if (polyspace.Get().size() > POLYAREAGROUP) {
		SpaceDiv(polyspace);
	}
	else {
		if (Find(polyspace.GetBB())) {
			PolyBySpace[polyspace.GetBB()].Add(polyspace.polys);
		}
		else
			if(polyspace.size() >0)
				PolyBySpace[polyspace.GetBB()] = polyspace;
	}
}
size_t GeoPolygonSort::GetPolyBySpaceSize() {
	size_t count = 0;
	for (map<GDS3DBB, GeoPolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); BB_it++) {
		count += BB_it->second.size();
	}
	return count;
}

bool GeoPolygonSort::Find(GDS3DBB BB) {
	GDS3DBB BBox;
	for (map<GDS3DBB, GeoPolySpace>::iterator BB_it = PolyBySpace.begin(); BB_it != PolyBySpace.end(); BB_it++) {
		BBox = BB_it->first;
		if (BBox == BB) {
			return true;
		}

	}
	return false;
}


void GeoPolygonSort::Check()
{
	if (polys.Get().size() != GetPolyBySpaceSize()) {
		PolyBySpace.clear();

		// Update 
		if (polys.Get().size() > POLYAREAGROUP) {
			SpaceDiv(polys);
		}
		else {
			PolyBySpace[polys.GetBB()] = polys;
		}
	}
}

