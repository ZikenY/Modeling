#include "harmonic_map.h"

inline double cot(double x)
{
    return cos(x) / sin(x);
}


CPoint mapxy2point2D(CVertex* v)
{
    CPoint pnt;
    pnt.v[0] = v->m_mapx;
    pnt.v[1] = v->m_mapy;
    return pnt;
}

double distance2d(const CPoint& point1, const CPoint& point2)
{
    double tmp = sqr(point1.v[0] - point2.v[0]) + sqr(point1.v[1] - point2.v[1]);
    if (fabs(tmp) < 0.00000001)
    {
        return 0;
    }
    else
    {
        return sqrt(tmp);
    }
}

// compute the elastic coeffcient K
void update_k(CMesh &mesh)
{
    list<CEdge*>::const_iterator it_edge = mesh.edges().begin();
    while (it_edge != mesh.edges().end())
    {
        CEdge *edge = *it_edge;
        if (!edge->boundary())
        {
            // find the two vertex connected to the ends of this edge
            CHalfEdge* he0 = edge->get_halfedge(0);
            CHalfEdge* he1 = edge->get_halfedge(1);
            CVertex* v0 = he0->he_next()->target();
            CVertex* v1 = he1->he_next()->target();

            double alpha = computer_angle_abc(v0->point(), he0->source()->point(), he0->target()->point());
            double beta = computer_angle_abc(v1->point(), he1->source()->point(), he1->target()->point());
            edge->m_k = (cot(alpha) + cot(beta)) / 2;
        }
        else
        {
            edge->m_k = 0;
        }

        it_edge++;
    }
}

double hamonic_energy(CMesh &mesh)
{
    double e = 0.0f;
    list<CVertex*>::const_iterator it_vertexlist = mesh.vertices().begin();
    while (it_vertexlist != mesh.vertices().end())
    {
        CVertex *vertex = *it_vertexlist;
        if (!vertex->boundary())
        {
            double e1 = 0;
            CHalfEdge* he = vertex->halfedge();
            // he is pointing to the vertex
            CHalfEdge* work = he;
            work = work->ccw_rotate_about_target();
            while (work != he)
            {
                CVertex *v1 = work->source();
                e1 += work->get_edge()->m_k * (sqr(vertex->m_mapx - v1->m_mapx) + sqr(vertex->m_mapy - v1->m_mapy));
                work = work->ccw_rotate_about_target();
            }
            CVertex *v1 = work->source();
            e1 += work->get_edge()->m_k * (sqr(vertex->m_mapx - v1->m_mapx) + sqr(vertex->m_mapy - v1->m_mapy));

            e += e1;
        }

        it_vertexlist++;
    }

    //    cout << "e = " << e << endl;
    return e;
}

void gradient_descent(CMesh &mesh, map<int, CPoint> &fixedvertics)
{
    // 注意每轮gradient descent必须针对原始的数据，执行完一轮后整体更新
    map<long, CPoint> modified_pnts;

    // compute all updates for vertics, store them using a map
    list<CVertex*>::const_iterator it_vertexlist = mesh.vertices().begin();
    while (it_vertexlist != mesh.vertices().end())
    {
        CVertex *vertex = *it_vertexlist;
        map<int, CPoint>::iterator it_set = fixedvertics.find(vertex->id());
        if (it_set == fixedvertics.end() && (!vertex->boundary()))
//        if (!vertex->boundary())
        {
            double e1_x = 0;
            double e1_y = 0;
            CHalfEdge* he = vertex->halfedge();
            // he is pointing to the vertex
            CHalfEdge* work = he;
            work = work->ccw_rotate_about_target();
            while (work != he)
            {
                CVertex *v1 = work->source();
                e1_x += work->get_edge()->m_k * (vertex->m_mapx - v1->m_mapx);
                e1_y += work->get_edge()->m_k * (vertex->m_mapy - v1->m_mapy);

                work = work->ccw_rotate_about_target();
            }
            CVertex *v1 = work->source();
            e1_x += work->get_edge()->m_k * (vertex->m_mapx - v1->m_mapx);
            e1_y += work->get_edge()->m_k * (vertex->m_mapy - v1->m_mapy);
            e1_x = e1_x * 0.01;
            e1_y = e1_y * 0.01;


            CPoint pnt;
            pnt.v[0] = vertex->m_mapx - e1_x;
            pnt.v[1] = vertex->m_mapy - e1_y;
            modified_pnts[vertex->id()] = pnt;
        }

        it_vertexlist++;
    }

    // apply updates to vertics
    it_vertexlist = mesh.vertices().begin();
    while (it_vertexlist != mesh.vertices().end())
    {
        CVertex *vertex = *it_vertexlist;
        map<int, CPoint>::iterator it_set = fixedvertics.find(vertex->id());
        if (it_set == fixedvertics.end() && (!vertex->boundary()))
//        if (!vertex->boundary())
        {
            map<long, CPoint>::iterator it_map = modified_pnts.find(vertex->id());

            vertex->m_mapx = it_map->second.v[0];
            vertex->m_mapy = it_map->second.v[1];
        }

        it_vertexlist++;
    }
}


bool harmonic_map(CMesh &mesh, vector<CVertex*> &boundary_vertics,
    map<int, CPoint> &fixedvertex)
{
    CBoundary boundary(&mesh);
    list<CLoop*> &looplist = boundary.loops();
    if (looplist.size() != 1)
    {
        cout << "loop.size() != 1" << endl;
        return false;
    }

    // compute the elastic coeffcient K
    update_k(mesh);

    // find vertics on boundary
    list<CLoop*>::const_iterator it_loop = looplist.begin();
    CLoop *loop = *it_loop;
    list<CHalfEdge*> &helist = loop->halfedges();
    list<CHalfEdge*>::const_iterator it_he = helist.begin();
    while (it_he != helist.end())
    {
        CHalfEdge *he = *it_he;
        CVertex *vertex = he->source();
        boundary_vertics.push_back(vertex);

        it_he++;
    }

    // map the vertics on boundary to the unit disk with radius of r;
    double r = 1;
    double theta = 0.0f;
    int n = boundary_vertics.size() - 1;
    for (int i = 0; i <= n; i++)
    {
        boundary_vertics[i]->m_mapx = r*cos(theta);
        boundary_vertics[i]->m_mapy = r*sin(theta);

        theta += 2 * PI / n;
    }

    // init all vertics not on the boundary to (0,0) except those are fixed
    cout << "init all vertics not on the boundary to (0,0) except those are fixed" << endl;
    list<CVertex*>::iterator it_vertex = mesh.vertices().begin();
    while (it_vertex != mesh.vertices().end())
    {
        CVertex *vertex = *it_vertex;

        // check if it is fixed
        map<int, CPoint>::iterator it_mapfixed = fixedvertex.find(vertex->id());
        if (it_mapfixed != fixedvertex.end())
        {
            vertex->m_mapx = it_mapfixed->second.v[0];
            vertex->m_mapy = it_mapfixed->second.v[1];
            cout << "harmonic fixed id:" << vertex->id() << "; " << vertex->m_mapx << ", " << vertex->m_mapy << endl;
        }
        else if (!vertex->boundary())
        {
            vertex->m_mapx = 0;
            vertex->m_mapy = 0;
        }

        it_vertex++;
    }

    cout << "compute the hamonic energy..." <<endl;
    double e0 = 0;
    double e1 = hamonic_energy(mesh);

    while (fabs(e0 - e1) > 0.001)
    {
        // apply gradient_descent method
        gradient_descent(mesh, fixedvertex);

        e0 = e1;
        e1 = hamonic_energy(mesh);
    }

    return true;
}


double GetAbs2DTriangleSquar(const CPoint& pt0, const CPoint& pt1, const CPoint& pt2)
{
    CPoint AB, BC;
    AB.v[0] = pt1.v[0] - pt0.v[0];
    AB.v[1] = pt1.v[1] - pt0.v[1];
    BC.v[0] = pt2.v[0] - pt1.v[0];
    BC.v[1] = pt2.v[1] - pt1.v[1];
    return fabs((AB.v[0] * BC.v[1] - AB.v[1] * BC.v[0])) / 2.0f;
}

bool PointIn2DTriangle(const CPoint& A, const CPoint& B, const CPoint& C, const CPoint& D)
{
    double SABC, SADB, SBDC, SADC;
    SABC = GetAbs2DTriangleSquar(A, B, C);
    SADB = GetAbs2DTriangleSquar(A, D, B);
    SBDC = GetAbs2DTriangleSquar(B, D, C);
    SADC = GetAbs2DTriangleSquar(A, D, C);

    double diff = (SADB + SBDC + SADC) - SABC;

    if (diff > 0.0000000001)
    {
        return false;
    }

    return true;
}

CPoint barycentric(const CPoint& v1_2d, const CPoint& v2_2d, const CPoint& v3_2d, const CPoint& v_2d,
    const CPoint& v1_3d, const CPoint& v2_3d, const CPoint& v3_3d)
{
    double s1 = GetAbs2DTriangleSquar(v_2d, v2_2d, v3_2d);
    double s2 = GetAbs2DTriangleSquar(v_2d, v1_2d, v3_2d);
    double s3 = GetAbs2DTriangleSquar(v_2d, v1_2d, v2_2d);
    double sss = s1 + s2 + s3;

    CPoint pnt;
    pnt.v[0] = v1_3d.v[0] * s1 / sss + v2_3d.v[0] * s2 / sss + v3_3d.v[0] * s3 / sss;
    pnt.v[1] = v1_3d.v[1] * s1 / sss + v2_3d.v[1] * s2 / sss + v3_3d.v[1] * s3 / sss;
    pnt.v[2] = v1_3d.v[2] * s1 / sss + v2_3d.v[2] * s2 / sss + v3_3d.v[2] * s3 / sss;
    return pnt;
}


double computer_boundary_z(vector<CVertex*> &boundary_vertics, CVertex* v)
{
    CVertex* v1 = boundary_vertics[0];
    CVertex* v2 = v1;
    double dist1 = distance2d(mapxy2point2D(v1), mapxy2point2D(v));
    double dist2 = dist1;

    // find nearest 2 vertices
    vector<CVertex*>::iterator it = boundary_vertics.begin();
    while (it != boundary_vertics.end())
    {
        CVertex* v_boundary = *it;
        double dist_boundary = distance2d(mapxy2point2D(v), mapxy2point2D(v_boundary));
        if (dist_boundary < dist1)
        {
            dist1 = dist_boundary;
            v1 = v_boundary;
        }
        else if (dist_boundary < dist2)
        {
            dist2 = dist_boundary;
            v2 = v_boundary;
        }

        it++;
    }

    return (v1->point().v[2] + v2->point().v[2]) / 2;
}

void morph_map(CMesh &mesh1, CMesh &mesh2, map<int, int> &fixed_pair)
{
    // map z-coordinates from meshfile2 to meshfile1
    vector<CVertex*> boundary_vertics1, boundary_vertics2;

    // compute the 2D coordinate for mesh1
    map<int, CPoint> dumb, fixedvertex;
    harmonic_map(mesh1, boundary_vertics1, dumb);

    cout << "find 2D coordinates of fixed vertics in mesh1. then they can be fixed when compute mesh2" << endl;
    map<int, int>::iterator it_fixed_pair = fixed_pair.begin();
    while (it_fixed_pair != fixed_pair.end())
    {
        int vertex_id_mesh1 = it_fixed_pair->second;
        int vertex_id_mesh2 = it_fixed_pair->first;
        CPoint fixed2d;
        fixed2d.v[0] = (mesh1.get_vertex(vertex_id_mesh1))->m_mapx;
        fixed2d.v[1] = (mesh1.get_vertex(vertex_id_mesh1))->m_mapy;
        fixedvertex[vertex_id_mesh2] = fixed2d;
        cout << "morph_map fixed id: " << vertex_id_mesh2 << "; " << fixed2d.v[0] << ", " << fixed2d.v[1] << endl;
        it_fixed_pair++;
    }

    harmonic_map(mesh2, boundary_vertics2, fixedvertex);

    // check which face is in mesh1 that a vertex in mesh2 is inside
    list<CVertex*>::iterator it_vertex_mesh2 = mesh2.vertices().begin();
    while (it_vertex_mesh2 != mesh2.vertices().end())
    {
        CVertex *vertex_mesh2 = *it_vertex_mesh2;

        CPoint pnt_mesh2 = mapxy2point2D(vertex_mesh2);

        if (vertex_mesh2->boundary())
        {
            double z = computer_boundary_z(boundary_vertics1, vertex_mesh2);
            vertex_mesh2->point().v[2] = z;
        }
        else
        {
            list<CFace*>::iterator it_face_mesh1 = mesh1.faces().begin();
            while (it_face_mesh1 != mesh1.faces().end())
            {
                // a, b, c, pnt_mesh2是2D
                CPoint a, b, c;
                // 3D coordinates after mapping
                CPoint a_3d, b_3d, c_3d;
                // normal after mapping
                CPoint a_normal, b_normal, c_normal;

                CHalfEdge* he = (*it_face_mesh1)->get_halfedge();
                a.v[0] = he->source()->m_mapx;
                a.v[1] = he->source()->m_mapy;
                a_3d = he->source()->point();
                a_normal = he->source()->normal();

                he = he->he_next();
                b.v[0] = he->source()->m_mapx;
                b.v[1] = he->source()->m_mapy;
                b_3d = he->source()->point();
                b_normal = he->source()->normal();

                he = he->he_next();
                c.v[0] = he->source()->m_mapx;
                c.v[1] = he->source()->m_mapy;
                c_3d = he->source()->point();
                c_normal = he->source()->normal();

                if (PointIn2DTriangle(a, b, c, pnt_mesh2))
                {
                    // compute barycentric and
                    // modify the coordinate for the vertex in mesh2
                    vertex_mesh2->point() = barycentric(a, b, c, pnt_mesh2, a_3d, b_3d, c_3d);
                    // compute the normal
                    vertex_mesh2->normal() = barycentric(a, b, c, pnt_mesh2, a_normal, b_normal, c_normal);
                    break;
                }

                it_face_mesh1++;
            }
        }

        it_vertex_mesh2++;
    }
}

void merge(string remapped, string ori)
{
    CMesh meshremapped;
    meshremapped.read_m(remapped.c_str());

    // read .m file line by line for "ori"
    string txt;
    File2String(ori, txt);
    Strings lines, output;
    lines.SetText(txt);
    for (dword i = 0; i<lines.GetLineCount(); i++)
    {
        string line;
        lines.GetLine(i, line);
        vector<string> stringvector = split(line, " ");

        string header = stringvector[0];
        if (stringvector.size() < 6 || !comparestr(header, "Vertex"))
        {
            output.AppendLine(line);
        }
        else
        {
            int vertex_id = StrToInt(stringvector[1]);
            CVertex *v_remapped = meshremapped.get_vertex(vertex_id);
            double x = v_remapped->point().v[0];
            double y = v_remapped->point().v[1];
            double z = v_remapped->point().v[2];
            string v_coordinate = "Opos=(" + FloatToStr(x) + " " + FloatToStr(y)
                + " " + FloatToStr(z) + ")";
            x = v_remapped->normal().v[0];
            y = v_remapped->normal().v[1];
            z = v_remapped->normal().v[2];
            string v_normal = "Onormal=(" + FloatToStr(x) + " " + FloatToStr(y)
                + " " + FloatToStr(z) + ")";

            string ext = stringvector[stringvector.size() - 1];
            ext = ext.substr(0, ext.size() - 1);  // remove the "}" on the tail
            ext = ext + " " + v_coordinate + " " + v_normal + "}";

            string ok = stringvector[0];
            for (dword j = 1; j<stringvector.size() - 1; j++)
            {
                ok = ok + " " + stringvector[j];
            }
            ok = ok + " " + ext;
            output.AppendLine(ok);
        }
    }

    string m_merged = RemoveExtNamePart(ori) + "_merged" + "." + GetExtNamePart(ori);
    String2File(output.GetText(), m_merged);
}
