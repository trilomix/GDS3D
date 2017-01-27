#ifndef __GEOPOLYGON__H
#define __GEOPOLYGON__H

#include "gdsobject_ogl.h"

struct CoordMeshElemSize {
	Point2D Coord;
	double TopMeshElemSize;
	double BottomMeshElemSize;
};

struct MeshElemSize {
	double Size;
	double Max;
	double Min;
	double MaxDiagonal;
};


class GeoPolygon : public GDSPolygon {
private:

	char* GDSName;
	vector<GeoPolygon*> Holes;
	vector<size_t> Lines;
	vector<size_t> TopLines;
	vector<size_t> LineLoop;
	vector<size_t> TopLineLoop;
	size_t Surface;
	size_t Point_Index;
	bool writen;			/* Points, Lines, Line Loop and Surface define */
	bool hole;
	bool extruded_Points;	/*Polygon Extrude Points, Lines, Line Loop and Surface define*/
	bool extruded;			/*Polygon Extrude done*/
	MeshElemSize MeshElem;
	vector<CoordMeshElemSize> _CoordMeshElemSize;

public:

	string Bottom_Surfaces_Contact;
	string Top_Surfaces_Contact;
	string EndGeo;

	//GeoPolygon(double Height, double Thickness, ProcessLayer * Layer);
	//GeoPolygon(double Height, double Thickness, ProcessLayer * Layer, bool isHole);
	GeoPolygon(char * GDS_Name, double Height, double Thickness, ProcessLayer * Layer, bool isHole);
	GeoPolygon(char * GDS_Name, GDSPolygon * poly, bool isHole);
	//GeoPolygon(GDSPolygon * poly, bool isHole);
	~GeoPolygon();
	void AddPoint(Point2D P);
	void AddPoint(double x, double y);
	bool RemovePoint(Point2D P);
	//GeoPolygon(GDSPolygon * poly);
	size_t GetPoints();
	CoordMeshElemSize GetMeshCoords(size_t Index);
	Point2D GetCoords(size_t Index);
	double GetXCoords(size_t index);
	double GetYCoords(size_t index);
	void CopyInto(GeoPolygon * p);
	void Flip();
	void Orientate();
	static bool intersect(GeoPolygon * P1, GeoPolygon * P2);
	bool IsPointInside(Point2D P);
	bool intersect(Point2D * IntersectPoint);
	double Area();
	bool isPolygonInside(GeoPolygon & poly);
	vector<GeoPolygon*> FindPolyInside(vector<GeoPolygon*> PolygonItems, bool all);
	vector<GeoPolygon*> FindPolyInside(vector<GeoPolygon*> PolygonItems, bool all, ProcessLayer * Layer);
	vector<GeoPolygon*> FindPolyInsideHoles(vector<GeoPolygon*> PolygonItems);
	void AddHole(GeoPolygon * Hole);
	void AddHole(GeoPolygon * Hole, bool recurse);
	vector<GeoPolygon*> GetHoles();
	void FindHole(Point2D I);
	void AddLine(size_t ID);
	void AddTopLine(size_t ID);
	void AddLineLoop(size_t ID);
	void SetSurface(size_t ID);
	void SetPoint_Index(size_t ID);
	size_t GetPoint_Index();
	bool IsWriten();
	void SetExtruded(bool Done);
	bool IsExtruded();
	void SetExtruded_Points(bool Done);
	bool IsExtruded_Points();
	void SetIsHole(bool IsHole);
	bool IsHole();
	void SetMeshElemSize();
	bool SetMeshElemSize(double Size);
	bool SetMeshElemSize(GeoPolygon * poly, bool Top, double TopDownRatio);
	bool DistBB2BBLessThan(GeoPolygon * poly, double dist);
	//bool SetMeshElemSize(GeoPolygon * poly, bool Top);
	bool MinDistFromPoly(GeoPolygon * poly, double TopDownRatio);
	bool SetMeshElemSize(GeoPolygon * poly);
	double GetMeshElemSizeMin();
	double GetMeshElemSize();
	double GetMeshElemSize(bool * Modif);
	string GetPointsAtZ(size_t cur_Point_Index, size_t cur_Line_Index, double z, double MeshFactor);
	string GetBottomGeo(size_t cur_Line_Index);
	string GetBottomGeoTransfinite();
	string Extrude(size_t & cur_Point_Index, size_t & cur_Line_Index);
	string GetTopGeoTransfinite(size_t cur_Point_Index);
	string Extrude_Volume();
	string GetSurfaceID();
	size_t GetLineLoopSize();
	string GetLoopLinesID();
	string GetExtrudeVarName();
	string GetExtrudeVar();
	string GetExtrudeVar(size_t index);
	string GetTopSurfaceID();
	string GetTopLoopLinesID();
	string GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems);
	string GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems, bool wHole);
	string GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems, ProcessLayer * layer);
	string GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems, ProcessLayer * layer, bool wHole);
	string GetBottomSurfaceIDListOf(vector<GeoPolygon*> PolygonItems, ProcessLayer * layer);
	string GetBottomSurfaceIDListOf(vector<GeoPolygon*> PolygonItems, ProcessLayer * layer, bool wHole);
	string GetBottomSurfaceID();
	string GetVolumeID();
	string GetLineLoop();
	string GetHolesLoopLinesList(bool Top, vector<GeoPolygon*> PolyItems);
	string GetHolesSurfacesList(bool Top);
	string GetHolesSurfacesList(bool Top, vector<GeoPolygon*> PolyItems);
};
#endif