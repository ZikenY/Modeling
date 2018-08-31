#include "subdivision.h"
#include "../../commoninclude/stringfuncs.h"

#include "../Mesh/mesh.h"
#include "../Mesh/iterators.h"
#include "../Geometry/Point.h"
#include "../m2obj/m2obj.h"

using namespace MeshLib;
using namespace ziken;

// circle around the target，compute the new position of the target
CPoint calcnewposition(CVertex* target, CHalfEdge *halfedge, CMesh *mesh)
{
    CHalfEdge *work = halfedge;
    if (work->target() != target)
    {
        work = work->he_prev();
    }

    CVertex *v_other = NULL;

    // use ccw_rotate_about_target to check if this vertex is on the edge
    bool at_edge = false;
    vector<CVertex*> points;
    points.push_back(work->source());
    CHalfEdge* next_he = work->ccw_rotate_about_target();
    if (!next_he)
    {
        // work->source() is on the boundary
        at_edge = true;
    }
    else
    {
        while (work != next_he)
        {
            points.push_back(next_he->source());
            next_he = next_he->ccw_rotate_about_target();
            if (!next_he)
            {
                // work->source() is on the boundary
                at_edge = true;
                break;
            }
        }
    }

    CPoint pos;

    if (at_edge)
    {
        CHalfEdge* last_he = work;
        v_other = work->source();
        CHalfEdge* next_he = work->clw_rotate_about_target();
        while (next_he)
        {
            last_he = next_he;
            v_other = work->source();
            next_he = next_he->clw_rotate_about_target();
        }

        // remember that clw_rotate_about_target()==NULL indicates that the next source is on the boundary
        // our purpose is to find the other vertex on the boundary
        next_he = last_he->edge()->other(last_he);
        if (!next_he)
        {
            // indicates that all on the boundary
            next_he = last_he->he_next();
        }
        else
        {
            next_he = next_he->clw_rotate_about_source();
        }
        v_other = next_he->target();

        for (int i = 0; i < 3; i++)
        {
            // 1/8, 3/4, 1/8
            pos.v[i] = target->point().v[i] * (3.0f / 4.0f);
            pos.v[i] += v_other->point().v[i] * (1.0f / 8.0f);      // one of the adjacent vertex on boundary
            pos.v[i] += points[points.size()-1]->point().v[i] * (1.0f / 8.0f);  // the other adjacent vertex on boundary
        }
    }
    else
    {
        // beta = 3/(8n)
        double beta = 3.0f / 16.0f;
        if (points.size() > 3)
        {
            beta = 3.0f / (8.0f * points.size());
        }

        vector<CVertex*>::iterator it_points = points.begin();
        while (it_points != points.end())
        {
            for (int i = 0; i < 3; i++)
            {
                pos.v[i] += (*it_points)->point().v[i] * beta;
            }
            it_points++;
        }

        for (int i = 0; i < 3; i++)
        {
            pos.v[i] += target->point().v[i] * (1 - points.size() * beta);
        }
    }

    return pos;
}

// retrieve the intersection of the two edge
CVertex* get_intersection_of_edges(CEdge *edge0, CEdge *edge1)
{
    CHalfEdge *hf0 = edge0->get_halfedge(0);
    if (!hf0)
    {
        hf0 = edge0->get_halfedge(1);
    }

    CHalfEdge *hf1 = edge1->get_halfedge(0);
    if (!hf1)
    {
        hf1 = edge1->get_halfedge(1);
    }

    if (hf0->target()->id() == hf1->target()->id() || hf0->target()->id() == hf1->source()->id())
        return hf0->target();
    else if (hf0->source()->id() == hf1->target()->id() || hf0->source()->id() == hf1->source()->id())
        return hf0->source();

    return NULL;
}

void do_subdivision(CMesh &mesh)
{
    // old vertex <--> new position
    map<CVertex*, CPoint> vertics_newpos;

    // edge <--> new vertex
    map<CEdge*, CVertex*> edge_newvertex_map;

    // new position map to the corresponding edge(to break the edges)，因为不能在循环里面直接添加点，所以先存起来，拿到外面添加
    map<CEdge*, CPoint> edge_newposition_map;

    cout << "m_verts.size(): " << mesh.m_verts.size() << endl;
    cout << "m_map_vert.size(): " << mesh.m_map_vert.size() << endl;
    cout << "m_edges.size(): " << mesh.m_edges.size() << endl;
    cout << "m_map_edge.size(): " << mesh.m_map_edge.size() << endl;
    cout << "m_faces.size(): " << mesh.m_faces.size() << endl;
    cout << "m_map_face.size(): " << mesh.m_map_face.size() << endl;
    cout << "edge_newposition_map.size(): " << edge_newposition_map.size() << endl;

    cout << "computing positions of each new vertics" << endl;
    int dummylast = 0; int dummycount = 0;

    // compute positions of each new vertics
    for (MeshEdgeIterator eiter(&mesh); !eiter.end(); eiter++)
    {
        if (int((1.0f * dummycount++ / mesh.m_edges.size()) * 100) % 3 != dummylast)
        {
            dummylast = int((1.0f * dummycount++ / mesh.m_edges.size()) * 100) % 3;
            cout << ".";
        }

        CEdge *edge = *eiter;

        CHalfEdge *halfedge = edge->get_halfedge(0);
        if (!halfedge)
        {
            halfedge = edge->get_halfedge(1);
        }

        CVertex *vertex0 = halfedge->source();
        CVertex *vertex1 = halfedge->target();

        // compute new positions for vertex
        map<CVertex*, CPoint>::iterator it_newpos = vertics_newpos.find(vertex0);
        if (it_newpos == vertics_newpos.end())
        {
            // 没有才做
            vertics_newpos.insert(std::pair<CVertex*, CPoint>(vertex0, calcnewposition(vertex0, halfedge, &mesh)));
        }

        // calc the other one
        it_newpos = vertics_newpos.find(vertex1);
        if (it_newpos == vertics_newpos.end())
        {
            // 没有才做
            vertics_newpos.insert(std::pair<CVertex*, CPoint>(vertex1, calcnewposition(vertex1, halfedge, &mesh)));
        }

        // compute the positions for the new vertices
        CPoint newposition;
        CHalfEdge* opposite_he = halfedge->get_edge()->other(halfedge);
        if (opposite_he)
        {
            for (int i = 0; i < 3; i++)
            {
                newposition.v[i] += halfedge->target()->point().v[i] * (3.0f / 8.0f);
                newposition.v[i] += halfedge->source()->point().v[i] * (3.0f / 8.0f);
                newposition.v[i] += halfedge->he_next()->target()->point().v[i] * (1.0f / 8.0f);
                newposition.v[i] += opposite_he->he_next()->target()->point().v[i] * (1.0f / 8.0f);
            }                        
        }
        else
        {
            // on the boundary
            for (int i = 0; i < 3; i++)
            {
                newposition.v[i] += halfedge->target()->point().v[i] * (0.5f);
                newposition.v[i] += halfedge->source()->point().v[i] * (0.5f);
            }
        }

        edge_newposition_map.insert(std::pair<CEdge*, CPoint>(edge, newposition));
    }

    cout << endl;
    cout << "now create new vertices with edge_newposition_map" << endl;

    // now create new vertices with edge_newposition_map
    map<CEdge*, CPoint>::iterator it_newposition_map = edge_newposition_map.begin();
    while (it_newposition_map != edge_newposition_map.end())
    {
        CEdge *edge = it_newposition_map->first;
        CVertex* newvertex = mesh.createVertex(-1);
        newvertex->point() = it_newposition_map->second;
        edge_newvertex_map.insert(std::pair<CEdge*, CVertex*>(edge, newvertex));
        it_newposition_map++;
    }
    
    cout << "update each original vertex" << endl;

    // update each original vertex
    map<CVertex*, CPoint>::iterator it_newpos = vertics_newpos.begin();
    while (it_newpos != vertics_newpos.end())
    {
        CVertex* vertex = it_newpos->first;
        vertex->point() = it_newpos->second;
        it_newpos++;
    }

    // get all old faces
    vector<CFace*> old_faces;
    for (MeshFaceIterator fiter(&mesh); !fiter.end(); fiter++)
    {
        CFace *face = *fiter;
        old_faces.push_back(face); // for later deletion
    }

    cout << "building new faces" << endl;
    dummylast = 0; dummycount = 0;

    // build new faces, then remove all old faces
    std::vector<CFace*>::iterator it_oldfaces = old_faces.begin();
    while (it_oldfaces != old_faces.end())
    {
        if (int((1.0f * dummycount++ / old_faces.size()) * 100) % 3 != dummylast)
        {
            dummylast = int((1.0f * dummycount++ / old_faces.size()) * 100) % 3;
            cout << ".";
        }

        CFace *face = *it_oldfaces;
        // build the four new faces based on the three edges

        // three edges
        CEdge *old_edge0 = face->get_halfedge()->get_edge();
        CEdge *old_edge1 = face->get_halfedge()->he_next()->get_edge();
        CEdge *old_edge2 = face->get_halfedge()->he_prev()->get_edge();

        // three new vertics
        map<CEdge*, CVertex*>::iterator it_e_newv0 = edge_newvertex_map.find(old_edge0);
        map<CEdge*, CVertex*>::iterator it_e_newv1 = edge_newvertex_map.find(old_edge1);
        map<CEdge*, CVertex*>::iterator it_e_newv2 = edge_newvertex_map.find(old_edge2);
        if (it_e_newv0 == edge_newvertex_map.end()
            || it_e_newv1 == edge_newvertex_map.end()
            || it_e_newv2 == edge_newvertex_map.end())
        {
            cout << "what the hell?" << endl;
            it_oldfaces++;
            continue;
        }
        CVertex* newvertex0 = it_e_newv0->second;
        CVertex* newvertex1 = it_e_newv1->second;
        CVertex* newvertex2 = it_e_newv2->second;

        // make sure they are on right directions!!!

        // first new triangle, the central one: newvertex0, newvertex1, newvertex2
        mesh.createFace(newvertex0, newvertex1, newvertex2);

        // second new triangle: 
        mesh.createFace(newvertex0, get_intersection_of_edges(old_edge0, old_edge1), newvertex1);

        // third new triangle:
        mesh.createFace(newvertex1, get_intersection_of_edges(old_edge1, old_edge2), newvertex2);

        // forth new triangle: 
        mesh.createFace(newvertex0, newvertex2, get_intersection_of_edges(old_edge2, old_edge0));

        it_oldfaces++;
    }

    cout << endl;
    cout << "deleting all the old faces" << endl;
    dummylast = 0; dummycount = 0;

    // delete all the old faces
    it_oldfaces = old_faces.begin();
    while (it_oldfaces != old_faces.end())
    {
        if (int((1.0f * dummycount++ / old_faces.size()) * 100) % 3 != dummylast)
        {
            dummylast = int((1.0f * dummycount++ / old_faces.size()) * 100) % 3;
            cout << ".";
        }

        mesh.removeFace(*it_oldfaces);
        it_oldfaces++;
    }

    cout << "\nsubdivision completed" << endl;
}

void subdivision(const string src_m, int iteration_count, bool convert_to_obj)
{
    if (convert_to_obj)
    {
//        m2obj(src_m, RemoveExtNamePart(src_m) + ".obj", scale_factor);
    }

    CMesh mesh;
    mesh.read_m(src_m.c_str());
    for (int i = 0; i < iteration_count; i++)
    {
        cout << "iteration 1:" << endl;
        do_subdivision(mesh);
        string dest_m = RemoveExtNamePart(src_m) + "_subdivision_" + IntToStr(i + 1) + "." + GetExtNamePart(src_m);
        if (i == iteration_count-1)
        {
            mesh.write_m(dest_m.c_str());
            if (convert_to_obj)
            {
                string obj_file = RemoveExtNamePart(dest_m) + ".obj";
                cout << "converting to " + obj_file << endl;
                m2obj(dest_m, obj_file);
                cout << "converting completed!" << endl;
            }
        }
    }

    cout << "\ncleaning..." << endl;
}
