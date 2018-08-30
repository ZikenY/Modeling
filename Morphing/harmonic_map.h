#include "../Mesh/HalfEdge.h"
#include "../Mesh/Vertex.h"
#include "../Mesh/Boundary.h"
#include "../subdivision/subdivision.h"
#include "../../../../commoninclude/commoninclude.h"
#include "../../../../commoninclude/stringfuncs.h"

using namespace MeshLib;
using namespace ziken;
using namespace std;

double hamonic_energy(CMesh &mesh);
void gradient_descent(CMesh &mesh, map<int, CPoint> &fixedvertics);
bool harmonic_map(CMesh &mesh, vector<CVertex*> &boundary_vertics,
    map<int, CPoint> &fixedvertex);

// fixed_pair
void morph_map(CMesh &mesh1, CMesh &mesh2, map<int, int> &fixed_pair);

void merge(string remapped, string ori);