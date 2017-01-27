#include "geopolygon.h"
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

#define round2digit(dist) (pow(10, floor(log10(dist)) - 1)*floor(dist / pow(10, floor(log10(dist)) - 1)))

GeoPolygon::GeoPolygon(char* GDS_Name, double Height, double Thickness, struct ProcessLayer *Layer, bool isHole) : GDSPolygon(Height, Thickness, Layer) {
	GDSName = GDS_Name;
	writen = false;
	hole = isHole;
	extruded_Points = false;
	extruded = false;
	Bottom_Surfaces_Contact = "";
	EndGeo = "";
	MeshElem.Size = 0;
}

GeoPolygon::GeoPolygon(char* GDS_Name, GDSPolygon *poly, bool isHole) : GDSPolygon(poly->GetHeight(), poly->GetThickness(), poly->GetLayer())
{
	GDSName = GDS_Name;
	writen = false;
	hole = isHole;
	extruded_Points = false;
	extruded = false;
	Bottom_Surfaces_Contact = "";
	EndGeo = "";
	MeshElem.Size = 0;

	for (size_t j = 0; j < poly->GetPoints(); j++) {
		AddPoint(poly->GetCoords(j));
	}
	Orientate();
	Tesselate();
	SetMeshElemSize();

}

GeoPolygon::~GeoPolygon()
{
	for (size_t i = 0; i < Holes.size(); i++) {
		delete Holes[i];
	}
}

void GeoPolygon::AddPoint(Point2D P) {
	AddPoint(P.X, P.Y);
}

void GeoPolygon::AddPoint(double x, double y) {
	GDSPolygon::AddPoint(x, y);
	CoordMeshElemSize CoordMesh;
	CoordMesh.Coord = Point2D(x, y);
	CoordMesh.TopMeshElemSize = MeshElem.Size;
	CoordMesh.BottomMeshElemSize = MeshElem.Size;
	_CoordMeshElemSize.push_back(CoordMesh);
}

bool GeoPolygon::RemovePoint(Point2D P) {
	if (GDSPolygon::RemovePoint(P)) {
		_CoordMeshElemSize.clear();
		for (size_t i = 0; i < GDSPolygon::GetPoints(); i++) {
			CoordMeshElemSize CoordMesh;
			CoordMesh.Coord = GDSPolygon::GetCoords(i);
			CoordMesh.TopMeshElemSize = MeshElem.Size;
			CoordMesh.BottomMeshElemSize = MeshElem.Size;
			_CoordMeshElemSize.push_back(CoordMesh);

		}
		return true;
	}
	return false;
}
size_t GeoPolygon::GetPoints() {
	return _CoordMeshElemSize.size();

}

CoordMeshElemSize GeoPolygon::GetMeshCoords(size_t Index)
{
	return _CoordMeshElemSize[Index];
}

Point2D GeoPolygon::GetCoords(size_t Index)
{
	return Point2D(GetXCoords(Index), GetYCoords(Index));
}

double GeoPolygon::GetXCoords(size_t index) {
	return int(rounded(_CoordMeshElemSize[index].Coord.X / GetLayer()->Units->Unitu))*GetLayer()->Units->Unitu;
}
double GeoPolygon::GetYCoords(size_t index) {
	return int(rounded(_CoordMeshElemSize[index].Coord.Y / GetLayer()->Units->Unitu))*GetLayer()->Units->Unitu;
}

void GeoPolygon::CopyInto(GeoPolygon *p) {
	GDSPolygon::CopyInto(p);
	p->MeshElem.Size = 0;
	p->_CoordMeshElemSize.clear();
	for (size_t i = 0; i < GDSPolygon::GetPoints(); i++) {
		CoordMeshElemSize CoordMesh;
		CoordMesh.Coord = GDSPolygon::GetCoords(i);
		CoordMesh.TopMeshElemSize = MeshElem.Size;
		CoordMesh.BottomMeshElemSize = MeshElem.Size;
		p->_CoordMeshElemSize.push_back(CoordMesh);

	}	
	p->SetMeshElemSize();

}

void GeoPolygon::Flip()
{
	// Flip points for boundary
	GDSPolygon::Flip();
	vector<CoordMeshElemSize> TCoords = _CoordMeshElemSize;
	for (unsigned int i = 0; i < TCoords.size(); i++)
		_CoordMeshElemSize[i] = TCoords[TCoords.size() - 1 - i];
}

void GeoPolygon::Orientate() {
	if (!OrientateAnticlockwise())
		Flip();
}

bool GeoPolygon::intersect(GeoPolygon *P1, GeoPolygon *P2)
{
	GDSTriangle		T1, T2;

	// Bounding box intersection
	if (!GDSBB::intersect(*P1->GetBBox(), *P2->GetBBox()))
		return false;

	// Holes
	if (P1->Holes.size() >0)
		for (size_t i = 0; i < P1->Holes.size(); i++) {
			if (P1->Holes[i]->GDSPolygon::isPolygonInside(*P2))
				return false;
		}
	if (P2->Holes.size() >0)
		for (size_t i = 0; i < P2->Holes.size(); i++) {
			if (P2->Holes[i]->GDSPolygon::isPolygonInside(*P1))
				return false;
		}

	for (size_t i = 0; i < P1->GetPoints(); i++) {
		if (P2->IsPointInside(P1->GetCoords(i)))
			return true;
	}

	for (size_t i = 0; i < P2->GetPoints(); i++) {
		if (P1->IsPointInside(P2->GetCoords(i)))
			return true;
	}

	//TODO Check edge intersetion

	return false;
}

bool GeoPolygon::IsPointInside(Point2D P) {
	size_t i, j;
	bool c = false;

	for (i = 0, j = GetPoints() - 1; i < GetPoints(); j = i++) {
		if (((GetCoords(i).Y > P.Y) != (GetCoords(j).Y > P.Y)) &&
			(P.X < (GetCoords(j).X - GetCoords(i).X) * (P.Y - GetCoords(i).Y) / (GetCoords(j).Y - GetCoords(i).Y) + GetCoords(i).X))
			c = !c;
	}

	return c;
}

bool GeoPolygon::intersect(Point2D *IntersectPoint) {

	for (size_t i = 0; i < GetPoints() - 1; i++) {
		Point2D cur_Point(GetCoords(i));
		for (size_t j = i + 1; j < GetPoints(); j++) {
			Point2D Point(GetCoords(j));
			if (cur_Point == Point) {
				if (IntersectPoint != NULL) {
					*IntersectPoint = Point;
				}
				return true;
			}
		}
		for (size_t j = 0; j < GetPoints(); j++) {
			Point2D Point(GetCoords(j));
			Point2D PointB;
			if (j + 1 < GetPoints()) {
				PointB = GetCoords(j + 1);
			}
			else
				PointB = GetCoords(0);
			if (cur_Point != Point &&  cur_Point != PointB && cur_Point.IsonLine(Point, PointB)) {
				if (IntersectPoint != NULL) {
					*IntersectPoint = cur_Point;
				}
				return true;
			}
		}

	}
	return false;
}

double GeoPolygon::Area() {
	double area;
	area = GDSPolygon::Area();
	for (size_t i = 0; i < Holes.size(); i++) {
		area -= Holes[i]->GDSPolygon::Area();
	}
	assert(area > 0);
	return area;
}

bool GeoPolygon::isPolygonInside(GeoPolygon& poly) {
	// Check if the "poly" is inside "this"
	bool inside = true;
	if (GDSPolygon::isPolygonInside(poly)) {
		if (hole != poly.IsHole()) {
			// Check if part of current holes list
			for (size_t i = 0; i < GetHoles().size(); i++) {
				if (Holes[i] == &poly) {
					return true;
				}
			}
		}
		for (size_t i = 0; i < GetHoles().size(); i++) {
			// Check Holes
			/*if (poly.GetHoles().size() == 0) {
			inside = false;
			break;
			}*/

			if (Holes[i]->GDSPolygon::isPolygonInside(poly)) {
				// poly must be outside Holes
				inside = false;
				break;
			}
			else {
				for (size_t j = 0; j < poly.GetHoles().size(); j++) {
					if (Holes[i]->GDSPolygon::isPolygonInside(*poly.GetHoles()[j])) {
						// poly Holes must be outside Holes
						inside = false;
						break;
					}
				}
			}
		}

	}
	else { inside = false; }
	return inside;
}
vector<GeoPolygon*> GeoPolygon::FindPolyInside(vector<GeoPolygon*> PolygonItems, bool all) {
	return FindPolyInside(PolygonItems, all, GetLayer());
}
vector<GeoPolygon*> GeoPolygon::FindPolyInside(vector<GeoPolygon*> PolygonItems, bool all, ProcessLayer *Layer) {
	// Find Polygon inside one Hole
	vector<GeoPolygon*> PolyList;
	vector<GeoPolygon*> PolyResList;
	vector<GeoPolygon*> PolyInholeList;
	set<GeoPolygon*> RemovePolyList;

	for (size_t i = 0; i < PolygonItems.size(); i++) {
		GeoPolygon *cur_Polygon = PolygonItems[i];
		if (cur_Polygon->GetLayer() != Layer)
			continue;
		if (cur_Polygon == this)
			continue;
		if (all) {
			if (GDSPolygon::isPolygonInside(*cur_Polygon)) {
				PolyList.push_back(cur_Polygon);
			}
		}
		else if (isPolygonInside(*cur_Polygon)) {
			PolyList.push_back(cur_Polygon);
		}
		else {
			for (size_t j = 0; j < cur_Polygon->Holes.size(); j++) {
				GeoPolygon *cur_Hole = cur_Polygon->Holes[j];
				if (isPolygonInside(*cur_Hole)) {
					PolyList.push_back(cur_Hole);
				}
			}
		}

	}
	if (all)
		return PolyList;
	// Remove poly in poly hole
	for (size_t i = 0; i < PolyList.size(); i++) {
		GeoPolygon *cur_Polygon = PolyList[i];
		PolyInholeList = cur_Polygon->FindPolyInsideHoles(PolyList);
		if (PolyInholeList.size()>0) {
			for (size_t j = 0; j < PolyInholeList.size(); j++) {
				RemovePolyList.insert(PolyInholeList[j]);
			}
		}
	}
	for (size_t i = 0; i < PolyList.size(); i++) {
		GeoPolygon *cur_Polygon = PolyList[i];
		if (RemovePolyList.find(cur_Polygon) == RemovePolyList.end())
			PolyResList.push_back(cur_Polygon);
	}

	return PolyResList;
}


vector<GeoPolygon*> GeoPolygon::FindPolyInsideHoles(vector<GeoPolygon*> PolygonItems) {
	// Find Polygon inside Holes
	vector<GeoPolygon*> PolyResList;
	vector<GeoPolygon*> PolyList;
	for (size_t i = 0; i < GetHoles().size(); i++) {
		GeoPolygon *cur_HolePolygon = GetHoles()[i];
		PolyList = cur_HolePolygon->FindPolyInside(PolygonItems, true);
		if (PolyList.size() > 0) {
			for (size_t j = 0; j < PolyList.size(); j++) {
				PolyResList.push_back(PolyList[j]);
			}
		}

	}
	return PolyResList;
}

void GeoPolygon::AddHole(GeoPolygon* Hole) {
	AddHole(Hole, false /*No recurse*/);
}
void GeoPolygon::AddHole(GeoPolygon* Hole, bool recurse) {
	Point2D Intersect;
	if (recurse) {
		while (Hole->intersect(&Intersect)) {
			Hole->FindHole(Intersect);
		}
		// Add extra holes
		for (size_t i = 0; i < Hole->Holes.size(); i++) {
			if (Hole->Holes[i]->GetMeshElemSize() > GetMeshElemSize() * 10)
				Hole->Holes[i]->SetMeshElemSize(this);
			Holes.push_back(Hole->Holes[i]);
		}
		Hole->Holes.clear();
	}
	if (Hole->GetMeshElemSize() > GetMeshElemSize() * 10)
		Hole->SetMeshElemSize(this);
	else if (Hole->GetMeshElemSize() * 10 < GetMeshElemSize())
		SetMeshElemSize(Hole);

	Holes.push_back(Hole);
}
vector<GeoPolygon*> GeoPolygon::GetHoles() {
	return Holes;
}

void GeoPolygon::FindHole(Point2D I) {
	// When convert GDSPolygon To GeoPolygon we need to recreate hole 
	GeoPolygon* Poly0 = new GeoPolygon(GDSName, GetHeight(), GetThickness(), GetLayer(), false);
	GeoPolygon* Poly1 = new GeoPolygon(GDSName, GetHeight(), GetThickness(), GetLayer(), false);
	bool cur_Poly0 = true;
	bool HoleDone = false;
	Point2D prev_Point(GetCoords(GetPoints() - 1));
	size_t i;
	size_t j;
	size_t k;
	size_t l;
	size_t m;
	size_t n;
	for (i = 0, j = GetPoints() - 1; i < GetPoints() - 1; j = i++) {
		Point2D PointA(GetCoords(i));
		Point2D PointB(GetCoords(j));
		for (k = i + 1, l = i; k < GetPoints(); l = k++) {
			Point2D PointC(GetCoords(k));
			Point2D PointD(GetCoords(l));
			// Same Line
			if ((PointA.X == PointB.X && PointC.X == PointD.X && PointC.X == PointB.X)
				|| (PointA.Y == PointB.Y && PointC.Y == PointD.Y && PointC.Y == PointB.Y)) {
				// Check oposite
				if (PointA.X == PointB.X && PointC.X == PointD.X && PointC.X == PointB.X) {
					if (!((PointA.Y - PointB.Y) / fabs(PointA.Y - PointB.Y) == -(PointC.Y - PointD.Y) / fabs(PointC.Y - PointD.Y)))
						continue;
				}
				else {
					// Check oposite
					if (!((PointA.X - PointB.X) / fabs(PointA.X - PointB.X) == -(PointC.X - PointD.X) / fabs(PointC.X - PointD.X)))
						continue;
				}
				// Check Common Point
				if (!PointA.IsonLine(PointC, PointD) && !PointB.IsonLine(PointC, PointD)
					&& !PointC.IsonLine(PointA, PointB) && !PointD.IsonLine(PointA, PointB))
					continue;
				// Segment found
				for (m = 0, n = GetPoints() - 1; m < GetPoints(); n = m++) {
					Point2D prev_Point(GetCoords(n));
					Point2D cur_Point(GetCoords(m));
					if (cur_Point == PointA && prev_Point == PointB)
						cur_Poly0 = !cur_Poly0;
					if (cur_Point == PointC && prev_Point == PointD)
						cur_Poly0 = !cur_Poly0;
					if (cur_Poly0) {
						Poly0->AddPoint(cur_Point);
					}
					else {
						Poly1->AddPoint(cur_Point);
					}
				}
				i = GetPoints() - 1;
				break;

			}

		}
	}
	// 3 poinst only 
	if (Poly0->GetPoints() <= 3) {
		this->RemovePoint(I);
		return;
	}
	if (Poly1->GetPoints() <= 3) {
		this->RemovePoint(I);
		return;
	}
	Poly0->Orientate();
	Poly1->Orientate();
	Poly0->Tesselate();
	Poly1->Tesselate();
	Poly0->SetMeshElemSize();
	Poly1->SetMeshElemSize();

	if (Poly0->isPolygonInside(*Poly1)) {
		if (Holes.size() > 0) {
			for (size_t i = 0; i < Holes.size(); i++) {
				Poly0->AddHole(Holes[i], false /*no recurse*/);
			}
		}
		Poly0->CopyInto(this);
		Poly1->SetIsHole(true);
		AddHole(Poly1, true /*recurse*/);
	}
	else {
		if (Holes.size() > 0) {
			for (size_t i = 0; i < Holes.size(); i++) {
				Poly1->AddHole(Holes[i], false /*no recurse*/);
			}
		}
		Poly1->CopyInto(this);
		Poly0->SetIsHole(true);
		AddHole(Poly0, true /*recurse*/);
	}

}

void GeoPolygon::AddLine(size_t ID)
{
	Lines.push_back(ID);
}

void GeoPolygon::AddTopLine(size_t ID)
{
	TopLines.push_back(ID);
}

void GeoPolygon::AddLineLoop(size_t ID)
{
	LineLoop.push_back(ID);
}

void GeoPolygon::SetSurface(size_t ID)
{
	writen = true;
	Surface = ID;
}

void GeoPolygon::SetPoint_Index(size_t ID)
{
	Point_Index = ID;
}

size_t GeoPolygon::GetPoint_Index()
{
	return Point_Index;
}

bool GeoPolygon::IsWriten()
{
	return writen;
}

void GeoPolygon::SetExtruded_Points(bool Done)
{
	extruded_Points = Done;
}

bool GeoPolygon::IsExtruded_Points()
{
	return extruded_Points;
}

void GeoPolygon::SetExtruded(bool Done)
{
	extruded = Done;
}

bool GeoPolygon::IsExtruded()
{
	return extruded;
}

void GeoPolygon::SetIsHole(bool IsHole) {
	hole = IsHole;
}

bool GeoPolygon::IsHole()
{
	return hole;
}
void GeoPolygon::SetMeshElemSize() {
	double  ratio = GDSPolygon::Area() / GetBBox()->area();
	double  elem = sqrt(GDSPolygon::Area());
	if (GetPoints() <= 100)
		if (isSimple()) {
			//elem = min(GetBBox()->max.X - GetBBox()->min.X, GetBBox()->max.Y - GetBBox()->min.Y);
			SetMeshElemSize(elem);
		}
		else {
			SetMeshElemSize(elem / 2);
		}
	else {
		SetMeshElemSize(elem / ((GetPoints()) / 100.0));
		//SetMeshElemSize(elem );
	}
}

bool GeoPolygon::SetMeshElemSize(double Size)
{
	bool Modif = false;
	if (MeshElem.Size == 0) {
		MeshElem.Size = round2digit(Size);
		MeshElem.Max = MeshElem.Size;
		MeshElem.Min = MeshElem.Size;
		MeshElem.MaxDiagonal = GetBBox()->max.double_distance(GetBBox()->min);
		Modif = true;
	}
	else
		if (Size < MeshElem.Size) {
			double MeshElemSizeCalc = round2digit(Size);
			if (MeshElem.Size != MeshElemSizeCalc) {
				MeshElem.Size = MeshElemSizeCalc;
				MeshElem.Max = MeshElem.Size;
				MeshElem.Min = MeshElem.Size;
				for (size_t i = 0; i < Holes.size(); i++) {
					GeoPolygon* hole = Holes[i];
					if (hole->GetMeshElemSize() > 10 * MeshElem.Size) {
						hole->SetMeshElemSize(this);
					}
				}
				Modif = true;
			}
		}
	if (Modif) {
		// Update Points Mesh
		for (size_t i = 0; i < GetPoints(); i++) {
			CoordMeshElemSize *PointMesh = &_CoordMeshElemSize[i];
			if (PointMesh->TopMeshElemSize == 0)		//Init
				PointMesh->TopMeshElemSize = MeshElem.Size;
			if (PointMesh->BottomMeshElemSize == 0)	//Init
				PointMesh->BottomMeshElemSize = MeshElem.Size;
			if (PointMesh->TopMeshElemSize>MeshElem.Size)
				PointMesh->TopMeshElemSize = MeshElem.Size;
			if (PointMesh->BottomMeshElemSize>MeshElem.Size)
				PointMesh->BottomMeshElemSize = MeshElem.Size;
			if (PointMesh->TopMeshElemSize<MeshElem.Min)
				MeshElem.Min = PointMesh->TopMeshElemSize;
			if (PointMesh->BottomMeshElemSize<MeshElem.Min)
				MeshElem.Min = PointMesh->BottomMeshElemSize;
		}
	}
	assert(MeshElem.Size != 0);
	return Modif;
}

bool GeoPolygon::SetMeshElemSize(GeoPolygon *poly, bool Top, double TopDownRatio) {
	bool Modif = false;
	//double TopDownRatio = 6;
	//if (GetThickness() > 1) {
	//	TopDownRatio = TopDownRatio * pow(10, floor(log10(GetThickness())))*floor(GetThickness() / pow(10, floor(log10(GetThickness()))));
	//}
	
	if (poly->GetMeshElemSizeMin() >= MeshElem.Max)
		return Modif;
	if (GetLayer() == poly->GetLayer())
		assert(Modif == Modif);

	if (!IsHole()) {
		for (size_t i = 0; i < Holes.size(); i++) {
			if (Holes[i]->SetMeshElemSize(poly, Top, TopDownRatio) && !Modif)
				Modif = true;
		}
	}

	for(size_t i = 0; i < poly->GetHoles().size(); i++){
		GeoPolygon *poly_hole = poly->GetHoles()[i];
		if (SetMeshElemSize(poly_hole, Top, TopDownRatio) && !Modif)
			Modif = true;
	}
	size_t i, j;
	MeshElemSize curMesh = MeshElem;
	double dist;
	for (i = 0, j = GetPoints() - 1; i < GetPoints(); j = i++) {
		// All points
		CoordMeshElemSize A = GetMeshCoords(i);
		CoordMeshElemSize B = GetMeshCoords(j);
		
		if (A.Coord == Point2D(954.21, 1437.22)) {
			assert(i == i);
		}

		if (B.Coord == Point2D(1163.105, 1519.6)) {
			assert(i == i);
		}

		if (Top) {
			dist = max(A.TopMeshElemSize, B.TopMeshElemSize);
		}
		else {
			dist = max(A.BottomMeshElemSize, B.BottomMeshElemSize);
		}
		double PolyMesh;
		size_t k, l;
		bool segment = false;
		CoordMeshElemSize polyPoint = poly->GetMeshCoords(0);
		for (k = 0, l = poly->GetPoints() - 1; k < poly->GetPoints(); l = k++) {
			// All poly Points
			CoordMeshElemSize C = poly->GetMeshCoords(k);
			CoordMeshElemSize D = poly->GetMeshCoords(l);
			
			double edgedist = dist;
			if (C.Coord.X == D.Coord.X)
				if (C.Coord.Y < A.Coord.Y && A.Coord.Y < D.Coord.Y || D.Coord.Y < A.Coord.Y && A.Coord.Y < C.Coord.Y
					|| A.Coord.Y < C.Coord.Y && C.Coord.Y < B.Coord.Y || B.Coord.Y < C.Coord.Y && C.Coord.Y < A.Coord.Y) {
					edgedist = fabs(C.Coord.X - A.Coord.X);
				}
			if (C.Coord.Y == D.Coord.Y)
				if (C.Coord.X < A.Coord.X && A.Coord.X < D.Coord.X || D.Coord.X < A.Coord.X && A.Coord.X < C.Coord.X
					|| A.Coord.X < C.Coord.X && C.Coord.X < B.Coord.X || B.Coord.X < C.Coord.X && C.Coord.X < A.Coord.X) {
					edgedist = fabs(C.Coord.Y - A.Coord.Y);
				}
			if (edgedist < dist) {
				segment = true;
				dist = edgedist;
				if (A.Coord.double_distance(C.Coord) <= A.Coord.double_distance(D.Coord)) {
					polyPoint = C;
				}
				else {
					polyPoint = D;
				}
			} else {
				double pointdist = A.Coord.double_distance(C.Coord);
				if (pointdist < dist) {
					dist = pointdist;
					polyPoint = C;
					segment = false;
				}
			}
		}
		CoordMeshElemSize* Point;
		CoordMeshElemSize* Point1;
		
		PolyMesh = Top ? polyPoint.BottomMeshElemSize : polyPoint.TopMeshElemSize;
		Point = &_CoordMeshElemSize[i];
		Point1 = &_CoordMeshElemSize[j];

		if (Top && dist < Point->TopMeshElemSize) {
			//if (Point->Coord.double_distance(polyPoint.Coord) < MeshElem.Size) {
			if (Point->Coord.double_distance(polyPoint.Coord) < (PolyMesh > dist ? PolyMesh : Point->TopMeshElemSize)) {
				// segment point is close to point
				if (PolyMesh > dist) {
					if (Point->TopMeshElemSize > PolyMesh + GetLayer()->Units->Unitu) {
						Point->TopMeshElemSize = PolyMesh;
						Modif = true;
					}
				}
				else {
					if (Point->TopMeshElemSize > dist) {
						Point->TopMeshElemSize = round2digit(dist);
						Modif = true;
					}
				}
			}
			else if (segment && PolyMesh > dist) {
				if (Point->TopMeshElemSize > (PolyMesh * 2)) {
					Point->TopMeshElemSize = round2digit(PolyMesh * 2);
					Modif = true;
				}
			}				
			if (Point->BottomMeshElemSize > Point->TopMeshElemSize * TopDownRatio)
				Point->BottomMeshElemSize = round2digit(Point->TopMeshElemSize * TopDownRatio);
			if (min(Point->TopMeshElemSize, Point->BottomMeshElemSize) < MeshElem.Min)
				MeshElem.Min = min(Point->TopMeshElemSize, Point->BottomMeshElemSize);
			if (curMesh.Size > Point->TopMeshElemSize * 10)
				curMesh.Size = Point->TopMeshElemSize * 10;
		}
		if (!Top && dist < Point->BottomMeshElemSize) {
			//if (Point->Coord.double_distance(polyPoint.Coord) < MeshElem.Size) {
			if (Point->Coord.double_distance(polyPoint.Coord) < (PolyMesh > dist ? PolyMesh : Point->BottomMeshElemSize)) {
				// segment point is close to poly
				if (PolyMesh > dist) {
					if (Point->BottomMeshElemSize > PolyMesh + GetLayer()->Units->Unitu) {
						Point->BottomMeshElemSize = PolyMesh;
						Modif = true;
					}
				}
				else {
					if (Point->BottomMeshElemSize > dist) {
						Point->BottomMeshElemSize = round2digit(dist);
						Modif = true;
					}
				}
			}
			else if (segment && PolyMesh > dist) {
				if (Point->BottomMeshElemSize > (PolyMesh * 2)) {
					Point->BottomMeshElemSize = round2digit(PolyMesh * 2);
					Modif = true;
				}
			}
			if (Point->TopMeshElemSize > Point->BottomMeshElemSize * TopDownRatio)
				Point->TopMeshElemSize = round2digit(Point->BottomMeshElemSize * TopDownRatio);
			if (min(Point->TopMeshElemSize, Point->BottomMeshElemSize) < MeshElem.Min)
				MeshElem.Min = min(Point->TopMeshElemSize, Point->BottomMeshElemSize);
			if (curMesh.Size > Point->BottomMeshElemSize * 10)
				curMesh.Size = Point->BottomMeshElemSize * 10;

		}
		if (Top && segment && dist < Point1->TopMeshElemSize) {
			//if (Point1->Coord.double_distance(polyPoint.Coord) < MeshElem.Size) {
			if (Point1->Coord.double_distance(polyPoint.Coord) < (PolyMesh > dist ? PolyMesh : Point1->TopMeshElemSize)) {
				// segment point is close to poly
				if (PolyMesh > dist) {
					if (Point1->TopMeshElemSize > PolyMesh + GetLayer()->Units->Unitu) {
						Point1->TopMeshElemSize = PolyMesh;
						Modif = true;
					}
				}
				else {
					if (Point1->TopMeshElemSize > dist) {
						Point1->TopMeshElemSize = round2digit(dist);
						Modif = true;
					}
				}
			}
			else if (segment && PolyMesh > dist) {
				if (Point1->TopMeshElemSize > (PolyMesh * 2)) {
					Point1->TopMeshElemSize = round2digit(PolyMesh * 2);
					Modif = true;
				}
			}
			if (Point1->BottomMeshElemSize > Point1->TopMeshElemSize * TopDownRatio)
				Point1->BottomMeshElemSize = round2digit(Point1->TopMeshElemSize * TopDownRatio);
			if (min(Point1->TopMeshElemSize, Point1->BottomMeshElemSize) < MeshElem.Min)
				MeshElem.Min = min(Point->TopMeshElemSize, Point->BottomMeshElemSize);
			if (curMesh.Size > Point1->TopMeshElemSize * 10)
				curMesh.Size = Point1->TopMeshElemSize * 10;

		}
		if (!Top && segment && dist < Point1->BottomMeshElemSize) {
			//if (Point1->Coord.double_distance(polyPoint.Coord) < MeshElem.Size) {
			if (Point1->Coord.double_distance(polyPoint.Coord) < (PolyMesh > dist ? PolyMesh : Point1->BottomMeshElemSize)) {
				// segment point is close to poly
				if (PolyMesh > dist) {
					if (Point1->BottomMeshElemSize > PolyMesh + GetLayer()->Units->Unitu) {
						Point1->BottomMeshElemSize = PolyMesh;
						Modif = true;
					}
				}
				else {
					if (Point1->BottomMeshElemSize > dist) {
						Point1->BottomMeshElemSize = round2digit(dist);
						Modif = true;
					}
				}
			}
			else if (segment && PolyMesh > dist) {
				if (Point1->BottomMeshElemSize > (PolyMesh * 2)) {
					Point1->BottomMeshElemSize = round2digit(PolyMesh * 2);
					Modif = true;
				}
			}
			if (Point1->TopMeshElemSize > Point1->BottomMeshElemSize * TopDownRatio)
				Point1->TopMeshElemSize = round2digit(Point1->BottomMeshElemSize * TopDownRatio);
			if (min(Point1->TopMeshElemSize, Point1->BottomMeshElemSize) < MeshElem.Min)
				MeshElem.Min = min(Point->TopMeshElemSize, Point->BottomMeshElemSize);
			if (curMesh.Size > Point1->BottomMeshElemSize * 10)
				curMesh.Size = Point1->BottomMeshElemSize * 10;

		}
	}
	if (curMesh.Size < MeshElem.Size)
		if (SetMeshElemSize(curMesh.Size) && !Modif)
			Modif = true;
	return Modif;
}

bool GeoPolygon::DistBB2BBLessThan(GeoPolygon* poly, double dist) {
	bool Needtodig = false;
	GDSPolygon PolygonA, PolygonB;
	size_t i, j;
	PolygonA.SetLayer(GetLayer(), GetLayer()->Units->Unitu);
	PolygonA.AddPoint(GetBBox()->min);
	PolygonA.AddPoint(GetBBox()->min.X, GetBBox()->max.Y);
	PolygonA.AddPoint(GetBBox()->max);
	PolygonA.AddPoint(GetBBox()->max.X, GetBBox()->min.Y);
	PolygonB.SetLayer(poly->GetLayer(), poly->GetLayer()->Units->Unitu);
	PolygonB.AddPoint(poly->GetBBox()->min);
	PolygonB.AddPoint(poly->GetBBox()->min.X, poly->GetBBox()->max.Y);
	PolygonB.AddPoint(poly->GetBBox()->max);
	PolygonB.AddPoint(poly->GetBBox()->max.X, poly->GetBBox()->min.Y);
	for (i = 0, j = PolygonA.GetPoints() - 1; i < PolygonA.GetPoints(); j = i++) {
		Point2D A = PolygonA.GetCoords(i);
		Point2D B = PolygonA.GetCoords(j);
		size_t k, l;
		for (k = 0, l = PolygonB.GetPoints() - 1; k < PolygonB.GetPoints(); l = k++) {
			Point2D C = PolygonB.GetCoords(k);
			Point2D D = PolygonB.GetCoords(l);

			double edgedist = dist;
			bool edge = false;
			if (C.X == D.X)
				if (C.Y < A.Y && A.Y < D.Y || D.Y < A.Y && A.Y < C.Y
					|| A.Y < C.Y && C.Y < B.Y || B.Y < C.Y && C.Y < A.Y) {
					// only if A.Y is inside C.Y D.Y or C.Y is inside A.Y B.Y
					edgedist = fabs(C.X - A.X);
					edge = true;
				}
			if (C.Y == D.Y)
				if (C.X < A.X && A.X < D.X || D.X < A.X && A.X < C.X
					|| A.X < C.X && C.X < B.X || B.X < C.X && C.X < A.X) {
					// only if A.X is inside C.X D.X or C.X is inside A.X B.X
					edgedist = fabs(C.Y - A.Y);
					edge = true;
				}
			if (edge) {
				if (edgedist < dist) {
					Needtodig = true;
					break;
				}
			}
			else {
				// vector not ortho
				double pointdist = A.double_distance(C);
				if (pointdist < dist) {
					Needtodig = true;
					break;
				}
			}
		}
		if (Needtodig) {
			break;
		}

	}
	return Needtodig;

}


bool GeoPolygon::MinDistFromPoly(GeoPolygon* poly, double TopDownRatio) {
	//Estimate the distance from edge to edge
	bool Modif = false;
	size_t i, j;
	double dist = max(poly->MeshElem.Size, MeshElem.Size);

	if(MeshElem.Max <= poly->MeshElem.Min)
		return Modif;

	// Reject far polygon
	bool Needtodig = DistBB2BBLessThan(poly, dist/ poly->MeshElem.Min);

	if (!Needtodig)
		return Modif;
	if (GetLayer() != poly->GetLayer())
		assert(dist == dist);

	double curMeshSize = MeshElem.Size;

	// For each edge we look for the closest point (or edge) and update mesh size
	// This is planar mesh size
	for (i = 0, j = GetPoints() - 1; i < GetPoints(); j = i++) {
		CoordMeshElemSize A = GetMeshCoords(i);
		CoordMeshElemSize B = GetMeshCoords(j);
		
		if (A.Coord == Point2D(1158.345, 1706.54)) {
			assert(i == i);
		}

		if (B.Coord == Point2D(1163.105, 1519.6)) {
			assert(i == i);
		}

		dist = max(poly->MeshElem.Size, curMeshSize);
		size_t k, l;
		bool segment = false;
		bool found = false;
		double pointdist = dist;
		double edgedist = dist;
		CoordMeshElemSize PointC;
		CoordMeshElemSize PointD;

		for (k = 0, l = poly->GetPoints() - 1; k < poly->GetPoints(); l = k++) {
			CoordMeshElemSize C = poly->GetMeshCoords(k);
			CoordMeshElemSize D = poly->GetMeshCoords(l);
			pointdist = dist;
			edgedist = dist;
			if (C.Coord.X == D.Coord.X)
				if (C.Coord.Y < A.Coord.Y && A.Coord.Y < D.Coord.Y || D.Coord.Y < A.Coord.Y && A.Coord.Y < C.Coord.Y
					|| A.Coord.Y < C.Coord.Y && C.Coord.Y < B.Coord.Y || B.Coord.Y < C.Coord.Y && C.Coord.Y < A.Coord.Y)
					edgedist = fabs(C.Coord.X - A.Coord.X);
			if (C.Coord.Y == D.Coord.Y)
				if (C.Coord.X < A.Coord.X && A.Coord.X < D.Coord.X || D.Coord.X < A.Coord.X && A.Coord.X < C.Coord.X
					|| A.Coord.X < C.Coord.X && C.Coord.X < B.Coord.X || B.Coord.X < C.Coord.X && C.Coord.X < A.Coord.X)
					edgedist = fabs(C.Coord.Y - A.Coord.Y);
			if (edgedist < dist) {
				dist = edgedist;
				segment = true;
				found = true;
				if (A.Coord.double_distance(C.Coord) < A.Coord.double_distance(D.Coord)) {
					PointC = C;
					PointD = D;
				}
				else {
					PointC = D;
					PointD = C;
				}
			}
			else {
				// vector not ortho
				pointdist = A.Coord.double_distance(C.Coord);
				if (pointdist < dist) {
					dist = pointdist;
					segment = false;
					PointC = C;
					found = true;
					//PointD = D;
				}
			}
		}
		if (A.Coord == Point2D(1158.345, 1706.54)) {
			assert(i == i);
		}
		if (!found) {
			continue;
		}
		if (segment) {
			CoordMeshElemSize* PointA;
			CoordMeshElemSize* PointB;

			PointA = &_CoordMeshElemSize[i];
			PointB = &_CoordMeshElemSize[j];
			if (GetLayer() != poly->GetLayer())
				assert(i == i);
			//if (max(max(PointA->TopMeshElemSize, PointA->BottomMeshElemSize), max(PointB->TopMeshElemSize, PointB->BottomMeshElemSize))
			//	<= min(min(PointC.TopMeshElemSize, PointC.BottomMeshElemSize), min(PointD.TopMeshElemSize, PointD.BottomMeshElemSize)))
			//	continue;
			//Modif = true;
			if (dist < 1) {
				dist = round2digit(dist);
				if (PointA->TopMeshElemSize > PointC.TopMeshElemSize)
					PointA->TopMeshElemSize = max(dist, PointC.TopMeshElemSize);
				if (PointA->BottomMeshElemSize > PointC.BottomMeshElemSize)
					PointA->BottomMeshElemSize = max(dist, PointC.BottomMeshElemSize);
				if (PointB->TopMeshElemSize > PointD.TopMeshElemSize)
					PointB->TopMeshElemSize = max(dist, PointD.TopMeshElemSize);
				if (PointB->BottomMeshElemSize > PointD.BottomMeshElemSize)
					PointB->BottomMeshElemSize = max(dist, PointD.BottomMeshElemSize);
			}
			else {
				if (PointA->TopMeshElemSize > PointC.TopMeshElemSize*dist && PointA->Coord.double_distance(PointC.Coord) < PointC.TopMeshElemSize*dist)
					PointA->TopMeshElemSize = round2digit(PointC.TopMeshElemSize*dist);
				if (PointA->BottomMeshElemSize > PointC.BottomMeshElemSize*dist && PointA->Coord.double_distance(PointC.Coord) < PointC.TopMeshElemSize*dist)
					PointA->BottomMeshElemSize = round2digit(PointC.BottomMeshElemSize*dist);
				if (PointB->TopMeshElemSize > PointD.TopMeshElemSize*dist && PointB->Coord.double_distance(PointD.Coord) < PointD.TopMeshElemSize*dist)
					PointB->TopMeshElemSize = round2digit(PointD.TopMeshElemSize*dist);
				if (PointB->BottomMeshElemSize > PointD.BottomMeshElemSize*dist && PointB->Coord.double_distance(PointD.Coord) < PointD.TopMeshElemSize*dist)
					PointB->BottomMeshElemSize = round2digit(PointD.BottomMeshElemSize*dist);
			}
			if (MeshElem.Min > min(PointA->TopMeshElemSize, PointA->BottomMeshElemSize))
				MeshElem.Min = min(PointA->BottomMeshElemSize, PointA->TopMeshElemSize);
			if (MeshElem.Min > min(PointB->TopMeshElemSize, PointB->BottomMeshElemSize))
				MeshElem.Min = min(PointB->TopMeshElemSize, PointB->BottomMeshElemSize);
			if (curMeshSize > max(PointA->BottomMeshElemSize, PointA->TopMeshElemSize) * MeshElem.MaxDiagonal)
				curMeshSize = max(PointA->BottomMeshElemSize, PointA->TopMeshElemSize) * MeshElem.MaxDiagonal;
			if (curMeshSize > max(PointB->BottomMeshElemSize, PointB->TopMeshElemSize) * MeshElem.MaxDiagonal)
				curMeshSize = max(PointB->BottomMeshElemSize, PointB->TopMeshElemSize) * MeshElem.MaxDiagonal;
			segment = false;
		} else if (!segment) {
			CoordMeshElemSize* PointA;
			//Modif = true;
			PointA = &_CoordMeshElemSize[i];
			if (dist <= 1) {
				dist = round2digit(dist);
				if (PointA->TopMeshElemSize > PointC.TopMeshElemSize) {
					PointA->TopMeshElemSize = max(PointC.TopMeshElemSize, dist);
					Modif = true;
				}
				if (PointA->BottomMeshElemSize > PointC.BottomMeshElemSize) {
					PointA->BottomMeshElemSize = max(PointC.BottomMeshElemSize, dist);
					Modif = true;
				}
			}
			else {
				
				if (PointA->TopMeshElemSize > PointC.TopMeshElemSize*dist) {
					double dist_calc = max(PointC.TopMeshElemSize*dist, dist);
					dist_calc = round2digit(dist_calc);
					PointA->TopMeshElemSize = dist_calc;
					Modif = true;
				}

				if (PointA->BottomMeshElemSize > PointC.BottomMeshElemSize*dist) {
					double dist_calc = max(PointC.BottomMeshElemSize*dist, dist);
					dist_calc = round2digit(dist_calc);
					PointA->BottomMeshElemSize = dist_calc;
					Modif = true;
				}
			}
			if (MeshElem.Min > min(PointA->TopMeshElemSize, PointA->BottomMeshElemSize))
				MeshElem.Min = min(PointA->TopMeshElemSize, PointA->BottomMeshElemSize);
			if (curMeshSize > max(PointA->BottomMeshElemSize, PointA->TopMeshElemSize) * MeshElem.MaxDiagonal)
				curMeshSize = max(PointA->BottomMeshElemSize, PointA->TopMeshElemSize) * MeshElem.MaxDiagonal;
		}
		
	}
	if (curMeshSize < MeshElem.Size) {
		if (SetMeshElemSize(curMeshSize) && !Modif)
			Modif = true;
	}
	return Modif;
}

bool GeoPolygon::SetMeshElemSize(GeoPolygon *poly) {
	bool Modif = false;
	bool Inside = true;
	GeoPolygon *Smallerpoly;
	GeoPolygon *Biggerpoly;
	double Magnify = 10;
	if (GDSPolygon::isPolygonInside(*poly)) {
		Smallerpoly = poly;
		Biggerpoly = this;
	}
	else if (poly->GDSPolygon::isPolygonInside(*this)) {
		Smallerpoly = this;
		Biggerpoly = poly;
	}
	else {
		if (this->GetBBox()->area() < poly->GetBBox()->area()) {
			Smallerpoly = this;
			Biggerpoly = poly;
		}
		else {
			Smallerpoly = poly;
			Biggerpoly = this;
		}
		Inside = false;
	}
	// Compare Area and if big difference estimate the distance from edge
	assert(Smallerpoly->GetBBox()->area() <= Biggerpoly->GetBBox()->area());
	if (false && Inside && Smallerpoly->GetBBox()->area() < Biggerpoly->GetBBox()->area() / 10) {

		// estimate the distance between poly and "this"edges  
		// Barycentre poly
		Point2D polyCenter;
		polyCenter = Point2D((Smallerpoly->GetBBox()->max.X - Smallerpoly->GetBBox()->min.X) / 2 + Smallerpoly->GetBBox()->min.X,
			(Smallerpoly->GetBBox()->max.Y - Smallerpoly->GetBBox()->min.Y) / 2 + Smallerpoly->GetBBox()->min.Y);
		size_t i, j;
		double dist = Biggerpoly->GetBBox()->max.double_distance(Biggerpoly->GetBBox()->min);

		for (i = 0, j = Biggerpoly->GetPoints() - 1; i < Biggerpoly->GetPoints(); j = i++) {
			Point2D A = Biggerpoly->GetCoords(i);
			Point2D B = Biggerpoly->GetCoords(j);
			if (A.Y == B.Y)
				if ( A.X<polyCenter.X && polyCenter.X<B.X || B.X<polyCenter.X && polyCenter.X<A.X)
					if (dist > fabs(A.Y - polyCenter.Y))
						dist = fabs(A.Y - polyCenter.Y);
			if (A.X == B.X)
				if (A.Y<polyCenter.Y && polyCenter.Y<B.Y || B.Y<polyCenter.Y && polyCenter.Y<A.Y)
					if (dist > fabs(A.X - polyCenter.X))
						dist = fabs(A.X - polyCenter.X);
			if (A.X != B.X && A.Y != B.Y)
				if (dist > A.double_distance(polyCenter))
					dist = A.double_distance(polyCenter);
			if (dist < 1)
				break;
		}
		if (dist <= 1) {
			if (SetMeshElemSize(poly->GetMeshElemSize() * 1) && !Modif)
				Modif = true;
		}
		else {
			if (SetMeshElemSize(poly->GetMeshElemSize() *Magnify* dist) && !Modif)
				Modif = true;
		}
	}
	else {
		if (MinDistFromPoly(poly, Magnify) && !Modif)
			Modif = true;
	}
	return Modif;
}

double GeoPolygon::GetMeshElemSizeMin() {
	return MeshElem.Min;
}

double GeoPolygon::GetMeshElemSize() {
	return GetMeshElemSize(NULL);
}
double GeoPolygon::GetMeshElemSize(bool *Modif)
{
	assert(MeshElem.Size != 0);
	if (Modif != NULL) {
		bool LocalModif = false;
		for (size_t i = 0; i < Holes.size(); i++) {
			GeoPolygon* hole = Holes[i];
			if (hole->GetMeshElemSize() < MeshElem.Size / MeshElem.MaxDiagonal) {
				LocalModif = SetMeshElemSize(hole);
			}

		}
		if (LocalModif) {
			for (size_t i = 0; i < Holes.size(); i++) {
				GeoPolygon* hole = Holes[i];
				if (hole->GetMeshElemSize(Modif) > MeshElem.Size * MeshElem.MaxDiagonal) {
					hole->SetMeshElemSize(this);
				}

			}

		}
		if (*Modif == false && LocalModif)
			*Modif = true;
	}

	return MeshElem.Size;
}

string GeoPolygon::GetPointsAtZ(size_t cur_Point_Index, size_t cur_Line_Index, double z, double MeshFactor) {
	string Geo;

	double z0 = z;
	bool Top = false;
	if (z > int(rounded(GetHeight() / GetLayer()->Units->Unitu))* GetLayer()->Units->Unitu)
		Top = true;
	
	string Name = GDSName;
	std::replace(Name.begin(), Name.end(), '-', '_');
	Geo = Geo + "GDS_cl = GDS_" + Name + "_cl; \n";
	
	for (size_t j = 0; j <GetPoints(); j++) {
		double x0 = GetXCoords(j);
		double y0 = GetYCoords(j);
		double MeshSize;
		if (Top)
			MeshSize = GetMeshCoords(j).TopMeshElemSize;
		else
			MeshSize = GetMeshCoords(j).BottomMeshElemSize;
		
		Geo += "Pt_i = " + to_string(j + cur_Point_Index) + "; ";

		Geo += "Pt_x = " + to_string(x0) + "; ";
		Geo += "Pt_y = " + to_string(y0) + "; ";
		Geo += "Pt_z = " + to_string(z0) + "; ";
		Geo += "Pt_cl = " + to_string(MeshSize*MeshFactor) + "*cl" + to_string(GetLayer()->Index) + "; \n";
		Geo += "Call PointElemSize; \n";
	}
	//Lines (n)
	for (size_t j = 0; j < GetPoints(); j++) {
		if (MeshFactor == 1) {
			if (Top)
				AddTopLine(cur_Line_Index + j);
			else
				AddLine(cur_Line_Index + j);
		}
		if (j == GetPoints() - 1)
			Geo += "Line(" + to_string(cur_Line_Index + j) + ") = {" + to_string(cur_Point_Index + j) + ", " + to_string(cur_Point_Index) + "};\n";
		else
			Geo += "Line(" + to_string(cur_Line_Index + j) + ") = {" + to_string(cur_Point_Index + j) + ", " + to_string(cur_Point_Index + j + 1) + "};\n";
	}
	//Line Loop
	if (MeshFactor == 1) {
		if (Top)
			Geo += "Line Loop(" + GetTopLoopLinesID() + ") = { ";
		else {
			Geo += "Line Loop(" + GetLoopLinesID() + ") = { ";
			AddLineLoop(atoi(GetSurfaceID().c_str()));
		}
	}
	else {
		Geo += "Line Loop(newl) = { ";
	}
	for (size_t j = 0; j < GetPoints(); j++) {
		Geo += to_string(cur_Line_Index + j);
		if (j == GetPoints() - 1)
			Geo += " ";
		else
			Geo += ", ";
	}
	Geo += " };\n";
	return Geo;
}

string GeoPolygon::GetBottomGeo(size_t cur_Line_Index) {
	string Geo;
	double scaleFactor = 1;

	double z0 = int(rounded(GetHeight() / GetLayer()->Units->Unitu))* GetLayer()->Units->Unitu;
	Geo = GetPointsAtZ(Point_Index, cur_Line_Index, z0, 1);
	return Geo;
}

string GeoPolygon::GetBottomGeoTransfinite() {
	string Geo = "";
	if (Lines.size() > 4)
		return Geo;
	if (Holes.size() > 0)
		return Geo;
	Geo = "Transfinite Line{" + to_string(Lines[0]) + ":" + to_string(Lines[Lines.size() - 1]) + " } = 3*cl" + to_string(GetLayer()->Index) + ";\n";
	Geo += "Transfinite Surface{ " + GetSurfaceID() + " } = {";
	for (size_t i = 0; i < GetPoints(); i++) {
		size_t PointID = Point_Index + i;
		if (i == GetPoints() - 1)
			Geo += to_string(PointID);
		else
			Geo += to_string(PointID) + ", ";
	}
	Geo += "};\n";
	Geo += "Recombine Surface{ " + GetSurfaceID() + " };\n";
	return Geo;
}
string GeoPolygon::Extrude(size_t &cur_Point_Index, size_t &cur_Line_Index) {
	string EndGeo;
	ProcessLayer *layer = GetLayer();
	double scaleFactor = 1;

	if (hole)
		EndGeo += "\n// Start Poly Extrude " + GetExtrudeVar() + " Dielec-" + layer->Name + "\n";
	else
		EndGeo += "\n// Start Poly Extrude " + GetExtrudeVar() + " " + layer->Name + "\n";

	double z = int(round((GetHeight() + GetThickness()) / layer->Units->Unitu))* layer->Units->Unitu;

	if (false && Holes.size() == 0 && layer->Metal == 0) {
		EndGeo += GetExtrudeVar() + " = Extrude{ 0.0, 0.0, " + to_string(GetThickness()) + " } { Surface{ " + GetSurfaceID() + " };\n";
		EndGeo += "Layers{ 2 };\n";
		//EndGeo += "QuadTriNoNewVerts RecombLaterals; Recombine; };\n";
		EndGeo += "QuadTriAddVerts RecombLaterals; Recombine; };\n";
		EndGeo += "TopLineLoop[] = Boundary{ Surface{ " + GetTopSurfaceID() + " }; };\n";
		EndGeo += "Line Loop(" + GetTopLoopLinesID() + ") = { TopLineLoop[] };\n";
	}

	EndGeo += GetExtrudeVar() + " = {};\n";

	EndGeo += GetPointsAtZ(cur_Point_Index, cur_Line_Index, z, 1);

	// Create Top Surfaces
	EndGeo += GetTopSurfaceID() + " = news;\n";
	if (GetHoles().size() > 0) {

		EndGeo += "Plane Surface(" + GetTopSurfaceID() + ") = { " + GetTopLoopLinesID();

		for (size_t i = 0; i < GetHoles().size(); i++) {
			GeoPolygon *cur_hole = GetHoles()[i];
			EndGeo += ", " + cur_hole->GetTopLoopLinesID();
		}
		EndGeo += "};\n";
	}
	else
		EndGeo += "Plane Surface(" + GetTopSurfaceID() + ") = { " + GetTopLoopLinesID() + " };\n";

	cur_Line_Index = cur_Line_Index + GetPoints();

	// Create Extrud Lines
	for (size_t j = 0; j < GetPoints(); j++) {
		EndGeo += "Line(" + to_string(cur_Line_Index + j) + ") = {" + to_string(cur_Point_Index + j) + "," + to_string(Point_Index + j) + "};\n";
		//EndGeo += "ExtLine[" + to_string(j + GetPoints()) + "] = newl;\n";
		//EndGeo += "Line(ExtLine[" + to_string(j + GetPoints()) + "]) = {ExtPoint[" + to_string(j) + "]," + to_string(Point_Index + j) + "};\n";
	}

	// Create Extrud Surface
	for (size_t j = 0; j < GetPoints(); j++) {
		EndGeo += GetExtrudeVar(j + 2) + " = news;\n";
		EndGeo += "Line Loop(" + GetExtrudeVar(j + 2) + ") = { ";
		EndGeo += to_string(Lines[j]) + ", "; // Bottom
		if (j == GetPoints() - 1) {
			EndGeo += "-" + to_string(cur_Line_Index) + ", "; // Right
		}
		else {
			EndGeo += "-" + to_string(cur_Line_Index + j + 1) + ", "; // Right
		}
		EndGeo += "-" + to_string(TopLines[j]) + ", "; // Top
		EndGeo += to_string(cur_Line_Index + j) + " "; // Left
													   /*
													   EndGeo += to_string(Lines[j]) + ", "; // Bottom
													   if (j == GetPoints() - 1) {
													   EndGeo += "-ExtLine[" + to_string(GetPoints()) + "], "; // Right
													   }
													   else {
													   EndGeo += "-ExtLine[" + to_string(GetPoints() + j + 1) + "], "; // Right
													   }
													   EndGeo += "-ExtLine[" + to_string(j) + "], "; // Top
													   EndGeo += "ExtLine[" + to_string(GetPoints() + j) + "] "; // Left
													   */
		EndGeo += "};\n";
		EndGeo += "Plane Surface(" + GetExtrudeVar(j + 2) + ") = {" + GetExtrudeVar(j + 2) + "};\n";
	}
	//EndGeo += GetTopGeoTransfinite(cur_Point_Index);
	cur_Point_Index = cur_Point_Index + GetPoints();
	cur_Line_Index = cur_Line_Index + GetPoints();

	return EndGeo;
}

string GeoPolygon::GetTopGeoTransfinite(size_t cur_Point_Index) {
	string Geo = "";
	if (GetPoints() > 4)
		return Geo;
	if (Holes.size() > 0)
		return Geo;
	//Geo = "Transfinite Line{" + to_string(TopLines[0]) + ":" + to_string(TopLines[TopLines.size() - 1]) + " } = 3*cl" + to_string(GetLayer()->Index) + ";\n";
	Geo = "Transfinite Line{ExtLine[0]:ExtLine[" + to_string(GetPoints() - 1) + "] } = 3*cl" + to_string(GetLayer()->Index) + ";\n";
	Geo += "Transfinite Surface{ " + GetTopSurfaceID() + " } = {";
	for (size_t i = 0; i < GetPoints(); i++) {
		/*
		size_t PointID = cur_Point_Index + i;
		if (i == GetPoints() - 1)
		Geo += to_string(PointID);
		else
		Geo += to_string(PointID) + ", ";
		*/
		if (i == GetPoints() - 1)
			Geo += "ExtPoint[" + to_string(i) + "]";
		else
			Geo += "ExtPoint[" + to_string(i) + "], ";
	}
	Geo += "};\n";
	Geo += "Recombine Surface{ " + GetTopSurfaceID() + " };\n";
	double ElemFactor;
	if (GetThickness() > MeshElem.Size)
		ElemFactor = floor(GetThickness() / MeshElem.Size);
	else
		ElemFactor = 1;
	//Geo += "Transfinite Line{" + to_string(TopLines[0]+ GetPoints()) + ":" + to_string(TopLines[TopLines.size() - 1] + GetPoints()) + 
	Geo += "Transfinite Line{ExtLine[" + to_string(GetPoints()) + "]:ExtLine[" + to_string(GetPoints() + GetPoints() - 1) + "]" +
		" } = " + to_string(ElemFactor) + "*cl" + to_string(GetLayer()->Index) + ";\n";
	for (size_t i = 0; i < GetPoints(); i++) {
		Geo += "Transfinite Surface{ " + GetExtrudeVar(i + 2) + " };\n";
		Geo += "Recombine Surface{ " + GetExtrudeVar(i + 2) + " };\n";
	}
	return Geo;
}

string GeoPolygon::Extrude_Volume() {
	string EndGeo;

	// Create Volume
	size_t index = GetPoints() + 2;
	for (size_t i = 0; i < GetHoles().size(); i++) {
		GeoPolygon *cur_hole = GetHoles()[i];
		if (strcmp(cur_hole->GetLayer()->Name, "HEATLAYER") == 0)
			continue;
		for (size_t j = 0; j < cur_hole->GetLineLoopSize(); j++) {
			EndGeo += GetExtrudeVar(index) + " = " + cur_hole->GetExtrudeVar(j + 2) + ";\n";
			index++;
		}
	}


	EndGeo += "// Top Bottom Surface Modify\n";
	if (this->EndGeo != "")
		EndGeo += this->EndGeo;

	EndGeo += "SurfaceList[] = {" + GetSurfaceID() + "," + GetTopSurfaceID() + "};\n";
	EndGeo += "For i In {2 : #" + GetExtrudeVar() + "-1}\n";
	EndGeo += "  SurfaceList[] += {" + GetExtrudeVarName() + "[i]};\n";
	EndGeo += "EndFor\n";
	if (Holes.size() != 0 || GetPoints() > 4 || GetLayer()->Metal == 1) {
		if (Top_Surfaces_Contact != "") {
			EndGeo += "  // Top Surface Modify\n";
			EndGeo += "  SurfaceList[] += {" + Top_Surfaces_Contact + "};\n";
		}
		if (Bottom_Surfaces_Contact != "") {
			EndGeo += "  // Bottom Surface Modify\n";
			EndGeo += "  SurfaceList[] += {" + Bottom_Surfaces_Contact + "};\n";
		}
	}

	EndGeo += GetVolumeID() + " = newv;\n";
	EndGeo += "Surface Loop ( " + GetVolumeID() + " ) = { SurfaceList[] };\n";
	EndGeo += "Volume (" + GetVolumeID() + ") = { " + GetVolumeID() + " };\n";

	if (strcmp(GetLayer()->Name, "HEATLAYER") == 0)
		return EndGeo;

	if (hole)
		EndGeo = EndGeo + "Physical Volume ( \"Dielec-" + GetLayer()->Name + "\" ) += { " + GetVolumeID() + " };\n";
	else
		EndGeo = EndGeo + "Physical Volume ( \"" + GetLayer()->Name + "\" ) += { " + GetVolumeID() + " };\n";

	/*
	if (Lines.size() <= 4 && Holes.size() == 0) {
	EndGeo = EndGeo + "Transfinite Volume{ " + GetVolumeID() + "};\n";
	EndGeo = EndGeo + "Recombine Volume{ " + GetVolumeID() + "};\n";
	}*/
	return EndGeo;
}
string GeoPolygon::GetSurfaceID()
{
	assert(IsWriten());
	return to_string(Surface);
}

size_t GeoPolygon::GetLineLoopSize()
{
	return Lines.size();
}
string GeoPolygon::GetLoopLinesID()
{
	assert(IsWriten());
	return to_string(Surface);
}
string GeoPolygon::GetExtrudeVarName()
{
	if (GetLayer()->Metal)
		return "Metal" + GetSurfaceID();
	else
		return "Via" + GetSurfaceID();

}
string GeoPolygon::GetExtrudeVar()
{
	return GetExtrudeVarName() + "[]";
}
string GeoPolygon::GetExtrudeVar(size_t Index)
{
	return GetExtrudeVarName() + "[" + to_string(Index) + "]";
}

string GeoPolygon::GetTopSurfaceID()
{
	return GetExtrudeVar(0);
}

string GeoPolygon::GetTopLoopLinesID()
{
	return to_string(Surface - 1);
}

string GeoPolygon::GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems) {
	return GetBottomSurfaceIDList(PolygonItems, GetLayer());
}
string GeoPolygon::GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems, bool wHole) {
	return GetBottomSurfaceIDList(PolygonItems, GetLayer(), wHole);
}
string GeoPolygon::GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems, ProcessLayer *layer) {
	return GetBottomSurfaceIDList(PolygonItems, layer, true);
}
string GeoPolygon::GetBottomSurfaceIDList(vector<GeoPolygon*> PolygonItems, ProcessLayer *layer, bool wHole)
{ // Find all surfaces include those in holes
	vector<GeoPolygon*> PolyList;
	string Ans;
	string test = "";

	if (Holes.size() == 0)
		return GetSurfaceID();
	else {
		//Ans = GetSurfaceID() + ", ";
		Ans = GetSurfaceID();
		if (layer->Metal == 0 && hole == 1)
			// All contact poly will be in hole for bottom layer must be metal hole
			return Ans;
		test = GetBottomSurfaceIDListOf(PolygonItems, layer, wHole);
		if (test != "")
			Ans += ", " + test;
		return Ans;
	}
}
string GeoPolygon::GetBottomSurfaceIDListOf(vector<GeoPolygon*> PolygonItems, ProcessLayer *layer) {
	return GetBottomSurfaceIDListOf(PolygonItems, layer, true);
}

string GeoPolygon::GetBottomSurfaceIDListOf(vector<GeoPolygon*> PolygonItems, ProcessLayer *layer, bool wHole)
{ // Find all surfaces include those in holes
	vector<GeoPolygon*> PolyList;
	string Ans = "";

	PolyList = FindPolyInside(PolygonItems, false /*all*/, layer);
	for (size_t j = 0; j < PolyList.size(); j++) {
		// 
		Ans += PolyList[j]->GetBottomSurfaceID();
		if ((j < PolyList.size() - 1) || (PolyList[j]->Holes.size() >0 && wHole))
			Ans += ", ";
		if (wHole) {
			for (size_t k = 0; k < PolyList[j]->Holes.size(); k++) {
				Ans += PolyList[j]->Holes[k]->GetBottomSurfaceIDList(PolygonItems);
				if (j < PolyList[j]->Holes.size() - 1)
					Ans += ", ";
			}
		}
	}

	return Ans;

}

string GeoPolygon::GetBottomSurfaceID()
{
	return GetSurfaceID();
}

string GeoPolygon::GetVolumeID()
{
	return GetExtrudeVar(1);
}

string GeoPolygon::GetLineLoop()
{
	string LineLoopList;
	for (size_t i = 0; i < LineLoop.size(); i++) {
		LineLoopList = LineLoopList + to_string(LineLoop[i]);
		if (i < LineLoop.size() - 1)
			LineLoopList = LineLoopList + ", ";
	}
	return LineLoopList;
}

string GeoPolygon::GetHolesLoopLinesList(bool Top, vector<GeoPolygon*> PolyItems)
{
	string LoopLinesList;
	for (size_t i = 0; i < Holes.size(); i++) {
		bool IsInside = false;
		for (size_t j = 0; j < PolyItems.size(); j++) {
			GeoPolygon *poly = PolyItems[j];
			if (poly->GDSPolygon::isPolygonInside(*Holes[i])) {
				IsInside = true;
				break;
			}
		}
		if (IsInside)
			continue;
		if (LoopLinesList != "")
			LoopLinesList += ", ";
		if (Top)
			LoopLinesList += Holes[i]->GetTopLoopLinesID();
		else
			LoopLinesList += Holes[i]->GetLoopLinesID();
	}
	return LoopLinesList;
}

string GeoPolygon::GetHolesSurfacesList(bool Top)
{
	string Surfacesist;
	for (size_t i = 0; i < Holes.size(); i++) {
		if (Top)
			Surfacesist = Surfacesist + Holes[i]->GetTopSurfaceID();
		else
			Surfacesist = Surfacesist + Holes[i]->GetBottomSurfaceID();
		if (i < Holes.size() - 1)
			Surfacesist = Surfacesist + ", ";
	}
	return Surfacesist;
}

string GeoPolygon::GetHolesSurfacesList(bool Top, vector<GeoPolygon*> PolyItems)
{
	string Surfacesist;
	for (size_t i = 0; i < Holes.size(); i++) {
		bool IsInside = false;
		for (size_t j = 0; j < PolyItems.size(); j++) {
			GeoPolygon *poly = PolyItems[j];
			if (poly->GDSPolygon::isPolygonInside(*Holes[i])) {
				IsInside = true;
				break;
			}
		}
		if (IsInside)
			continue;
		if (Top)
			Surfacesist = Surfacesist + Holes[i]->GetTopSurfaceID();
		else
			Surfacesist = Surfacesist + Holes[i]->GetBottomSurfaceID();
		if (i < Holes.size() - 1)
			Surfacesist = Surfacesist + ", ";
	}
	return Surfacesist;
}
