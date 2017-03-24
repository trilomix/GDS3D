#ifndef __OUTSRTEAM__H
#define __OUTSRTEAM__H

#include "gdsobject_ogl.h"
#include "windowmanager.h"
#include "geopolygon.h"

class GeoPolySpace : public PolySpace {
private:
	bool _hasBBox;

	GDS3DBB BBox;
	size_t cur_size;
protected:
public:
	GeoPolySpace() {
		_hasBBox = false; cur_size = 0; BBox.clear();
	}
	vector<GeoPolygon*> polys;
	~GeoPolySpace();
	void Clear();
	GDS3DBB GetBB();
	void Add(vector<GeoPolygon*> polyList);
	void Add(GeoPolygon* poly);
	void Remove(GeoPolygon* poly);
	vector<GeoPolygon*> Get();
	size_t size();
};
#define POLYAREAGROUP 500

class GeoPolygonSort : public PolygonSort {
private:
	vector<struct ProcessLayer*> layer_list;
	
	// Temp list with full size
	GeoPolySpace polys;

	// List of poly devided by space area
	map < GDS3DBB, GeoPolySpace> PolyBySpace;

public:
	void SetLayerList(vector<struct ProcessLayer*> layer_list);
	void Add(GeoPolygon* poly);
	void Add(vector<GeoPolygon*> polyList);
	vector<GeoPolygon*> GetPolyNear(GDS3DBB BBox);
	void SpaceDiv(GeoPolySpace poly_list);
	void SpaceDivEnd(GeoPolySpace polys_0);
	size_t GetPolyBySpaceSize();
	bool Find(GDS3DBB BB);
	void Check();
};

class GDSGroup {
public:
	vector<struct ProcessLayer*> layer_list;
	vector<GeoPolygon*> FullPolygonItems;
	vector<GDSPolygon*> GDSPolygonItems;
	vector<GeoPolygon*> OutLinesItems;
	
	GeoPolygonSort FullSortPolygonItems;
	
	char * Name;
	GDSBB bbox;
	double Unit;
	double MeshMaxElemSize;
	size_t PolygonWrited;
	GDSGroup(GDSObject_ogl * render_object);
	GDSGroup(GDSObject * object, GDSMat object_mat);
	~GDSGroup();
};

class Output {
	string filename;
	FILE *file;
	
	GDSGroup *cur_GDS;

	//vector<struct ProcessLayer*> layer_list;
	size_t cur_Point_Index;
	size_t cur_Line_Index;
	size_t cur_Line_Loop_Index;
	size_t cur_Surface_Index;
	size_t cur_Physical_Index;
	size_t cur_Field_Index;
	size_t cur_HEAT_Layer;
	string Surface_Geo;
	string Volume_Geo;
	string EndGeo;
	vector<GDSGroup*> FullGDSItems;
	GDSBB bbox;
private:
	htime* time;
	double Waittime;

public:
	Output();
	~Output();
	void SaveToGEO(GDSObject_ogl *render_object);
	void SaveToGEO(GDSObject_ogl * object, bool flat);
	void Convert_Polygon();
	void LayerList();
	size_t CheckMeshSize(GeoPolygon * poly, vector<GeoPolygon*> PolyItemsNear, double TopDownMaxRatio, bool Top);
	void SetMeshElemSize(GeoPolygon * poly, size_t * Modif, size_t *TotDone, size_t TotalNbPoly);
	void SetMeshElemSize(GeoPolygon * poly, size_t * Modif, size_t *TotDone, size_t TotalNbPoly, bool Countpoly);
	void Traverse(GDSObject * object, GDSMat object_mat);
	vector<GeoPolygon*> TraverseGeoLayer(GeoPolygon * poly);
	void Geometry();
	void Simu_GetDP();
	void HeatLayer(GeoPolygon * Polygon, GDSGroup * cur_GDS);
	void HeatLayer_PostProcess(GeoPolygon * polygon, GDSGroup * cur_GDS);
	void Recombine(GeoPolygon * Polygon, ProcessLayer * layer);
	void PolygonPoints(GeoPolygon * polygon, bool dielectrique, double MeshElemSize);
	void PolygonExtrude(GeoPolygon * polygon, ProcessLayer * do_layer, bool dielectrique, double MeshElemSize);
	//void PolygonExtrude(GeoPolygon * polygon, ProcessLayer * do_layer, bool dielectrique);
	void Polygon(GeoPolygon * polygon, ProcessLayer * do_layer);
	void Polygon(GeoPolygon * polygon, ProcessLayer * do_layer, bool dielectrique);
	void Polygon(GeoPolygon * polygon, ProcessLayer * do_layer, bool dielectrique, double MeshElemSize);
	void ConnectPoly(GeoPolygon * polygon, bool Top);
	vector<GeoPolygon*> SimplifyPolyItems_wClipper(vector<GDSPolygon*> PolygonItems, ProcessLayer * Layer);
	vector<GeoPolygon*> SimplifyPolyItems_wClipper(vector<GeoPolygon*> PolygonItems, ProcessLayer * Layer);
	vector<GeoPolygon*> Convert_ClipperPolyTree(ClipperLib::PolyTree * pTree, ProcessLayer * Layer);
	GeoPolygon * Convert_ClipperPolyNode(const ClipperLib::PolyNode pNode, ProcessLayer * Layer);
	//GeoPolygon * Convert_ClipperPolyNode(ClipperLib::PolyNode * pNode, ProcessLayer * Layer);
	vector<GeoPolygon*> SimplifyPolyItems(vector<GDSPolygon*> PolygonItems, ProcessLayer * Layer);
	vector<GeoPolygon*> GetContactPolyItems(GeoPolygon * poly, vector<GeoPolygon*> PolygonItems, bool Top);
};

#endif


