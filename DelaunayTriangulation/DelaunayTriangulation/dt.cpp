#include <commoninclude.h>
#include <mathlib2d.h>
#include <stringfuncs.h>
using namespace ziken;

// make sure the direction of v1 v2 v3 follows CCW
struct triangle
{
    // verties indexes in the mesh for this triangle
    int v1;
    int v2;
    int v3;

    list<triangle*> descendents;

    // leaf triangles adjacent to edges
    triangle *adj12;
    triangle *adj23;
    triangle *adj31;

    triangle() {adj12 = adj23 = adj31 = NULL;}
    triangle(int _v1, int _v2, int _v3)
        :v1(_v1),v2(_v2),v3(_v3)
        {adj12 = adj23 = adj31 = NULL;}

    // single way
    bool set_adjacency(triangle *adj);

    // a b can be any order
    bool find_shared_edge(triangle &adj, int &a, int &b);

    // a b can be any order
    triangle* find_adjacency(int a, int b);

    //transfer tri's adjacency to this
    bool give_me_ur_adjacency(triangle &tri);

    // a b can be any order
    int find_another_vertex(int a, int b);

    void output() const;

private://disabled
    triangle(const triangle& tri);
    triangle& operator =(const triangle &tri);
};

struct mesh
{
    // notice that the first three vertices are auxilary
    // vertices that contains all the genuine vertices
    vector<point_d> vertices;   // all vertices

    // root is a auxilary bound
    // all valid triangles are divided from root recurrsively
    triangle *root;

    // deal with multiple parents when exporting
    // and releasing
    set<triangle*> exporting;

    // not much useful
    int tag;

    void reset();
    mesh() {root = NULL; this->reset();}
    ~mesh() {this->reset();}

    // find the minimal bounded rectangle of all the vertices
    // except for the first three auxilary vertices
    bool get_mbr(rect_d &mbr) const;

    // gotta be following CCW dirction
    double make_triangle_valid(triangle &tri);

    // see if the vertex is inside or outside the triangle,
    // or lies on a edge of the triangle
    // be sure that v1 v2 v3 in t are in CCW direction
    //
    // return:
    //  1   -   v is inside of t
    //  0   -   v lies on one of the edges
    //  -1  -   v is one of the vertices of t
    //  -2  -   v is outside of t
    int vertex_in_triangle(int v, triangle &t);

    // make sure CCW order for (adj_v1, a, adj_v2) & (adj_v2, b, adj_v1)
    bool find_shared_edge(triangle &t1, triangle &t2,
        int &adj_v1, int &adj_v2, int &a, int &b);

    // pinpoint the target  triangle(s)
    // input:
    //  a new vertex that will be added to form triangles;
    //  root triangle
    //  make sure *t1, *T2 are NULL when first call
    //
    // return:
    //  1   -   v is inside of t1
    //  0   -   v is on the edge shared by t1 and t2
    int target_trangles(int v, triangle &t,
        triangle **t1, triangle **t2);

    void split_one_triangle_to_three(int v, triangle &t,
        triangle **t1, triangle **t2, triangle **t3);

    void split_two_adjacencies_to_four(int v,
        triangle &adj1, triangle &adj2,
        triangle **t1, triangle **t2,
        triangle **t3, triangle **t4);

    // edge(a b) is the edge that tri shared with its adjacency
    // on the other side against the edge.
    // make sure a & b follow CCW direction in tri
    void flip(triangle &tri, int a, int b);

    // compute the three auxilary verties that contains all
    // the genuine vertices to form the initial trianglation
    void prepare_delaunay_root();

    void do_delaunay();

    // export triangles to stream recursively
    void export_triangle(ofstream &obj,
        triangle &tri, int &triangle_num);

    // export all stuff to a .obj file
    void export_obj(string filename);

    void output(const triangle const *root) const;
};

void release_trangles(triangle* tri, set<triangle*> &check)
{
    if (!tri)
        return;

    list<triangle*>::iterator it = tri->descendents.begin();
    while (it != tri->descendents.end())
    {
        release_trangles(*it, check);
        it++;
    }

    set<triangle*>::iterator it_set = check.find(tri);
    if (it_set != check.end())
    {
        check.insert(tri);
        delete tri;
    }
}

inline bool vertex_from_triangle(int v, triangle *tri)
{
    if (tri && v == tri->v1 || v == tri->v2 || v == tri->v3)
        return true;

    return false;
}

bool triangle::set_adjacency(triangle *adj)
{
    if (!adj)
        return false;

    // check if v1 & v2 are shared with adj
    if (vertex_from_triangle(v1, adj)
        && vertex_from_triangle(v2, adj))
    {
        this->adj12 = adj;
        return true;
    }

    // check if v2 & v3 are shared with adj
    if (vertex_from_triangle(v2, adj)
        && vertex_from_triangle(v3, adj))
    {
        this->adj23 = adj;
        return true;
    }

    // check if v3 & v1 are shared with adj
    if (vertex_from_triangle(v3, adj)
        && vertex_from_triangle(v1, adj))
    {
        this->adj31 = adj;
        return true;
    }

    return false;
}

bool triangle::find_shared_edge(triangle &adj, int &a, int &b)
{
    list<int> aa;
    if (v1 == adj.v1
        || v1 == adj.v2
        || v1 == adj.v3)
        aa.push_back(v1);

    if (v2 == adj.v1
        || v2 == adj.v2
        || v2 == adj.v3)
        aa.push_back(v2);

    if (v3 == adj.v1
        || v3 == adj.v2
        || v3 == adj.v3)
        aa.push_back(v3);

    if (aa.size() != 2)
        return false;

    list<int>::const_iterator it = aa.begin();
    a = *it; it++;
    b = *it;
    return true;
}

triangle* triangle::find_adjacency(int a, int b)
{
    if ((v1!=a && v1!=b)    //exclude v1 & check the other two
        && ((v2==a || v2==b) && (v3==a || v3==b)))
    {
        return adj23;
    }
    else if ((v2!=a && v2!=b)   //exclude v2
        && ((v1==a || v1==b) && (v3==a || v3==b)))
    {
        return adj31;
    }
    else if ((v3!=a && v3!=b)   //exclude v3
        && ((v1==a || v1==b) && (v2==a || v2==b)))
    {
        return adj12;
    }

    return NULL;
}

bool triangle::give_me_ur_adjacency(triangle &tri)
{
    int a, b;
    if (!this->find_shared_edge(tri, a, b))
        return false;

    // transfer tri's adjacency againt edge(a, b) to this
    triangle *adj = tri.find_adjacency(a, b);
    if (!adj)
        return false;

    if (descendents.size() > 0 || adj->descendents.size() > 0)
    {
        cout << "\nno good for triangle::give_me_ur_adjacency()..." << endl;
    }

    this->set_adjacency(adj);
    adj->set_adjacency(this);
    return true;
}

int triangle::find_another_vertex(int a, int b)
{
    if (v1 != a && v1 != b)
        return v1;

    if (v2 != a && v2 != b)
        return v2;

//    if (v3 != a && v3 != b)
        return v3;
}

void triangle::output() const
{
    cout << "   - triangle -" << endl;
    cout << "   descendent num: " << descendents.size() << endl;
    cout << "   vertices: " << v1 << ", " << v2 << ", " << v3 << endl;
}

void mesh::reset()
{
    vertices.clear();
    point_d dummy;
    vertices.push_back(dummy);
    vertices.push_back(dummy);
    vertices.push_back(dummy);

    exporting.clear();
    release_trangles(root, exporting);
    root = new triangle;
}

bool mesh::get_mbr(rect_d &mbr) const
{
    if (this->vertices.size() <= 3)
        return false;

    vector<point_d>::const_iterator it = this->vertices.begin();
    it++;it++;it++;
    mbr.min_x = mbr.max_x = (*it).x;
    mbr.min_y = mbr.max_y = (*it).y;
    it++;
    while (it != this->vertices.end())
    {
        if (mbr.min_x > (*it).x)
            mbr.min_x = (*it).x;
        else if (mbr.max_x < (*it).x)
            mbr.max_x = (*it).x;

        if (mbr.min_y > (*it).y)
            mbr.min_y = (*it).y;
        else if (mbr.max_y < (*it).y)
            mbr.max_y = (*it).y;

        it++;
    }

    return true;
}

double mesh::make_triangle_valid(triangle &tri)
{
    double r = integral_area(vertices[tri.v1],
        vertices[tri.v2], vertices[tri.v3]);
    if (r >= 0)
        return r;

    int t = tri.v1;
    tri.v1 = tri.v2;
    tri.v2 = t;
    return -r;
}

int mesh::vertex_in_triangle(int v, triangle &t)
{
    point_d &v1 = this->vertices[t.v1];
    point_d &v2 = this->vertices[t.v2];
    point_d &v3 = this->vertices[t.v3];

    return within_triangle(this->vertices[v], v1, v2, v3);
}

int find_another_v(triangle &t, int v1, int v2)
{
    if (t.v1 != v1 && t.v1 != v2)
        return t.v1;

    if (t.v2 != v1 && t.v2 != v2)
        return t.v2;

    if (t.v3 != v1 && t.v3 != v2)
        return t.v3;

    return -1;
}

// make sure CCW order for (adj_v1, a, adj_v2) & (adj_v2, b, adj_v1)
bool mesh::find_shared_edge(triangle &t1, triangle &t2,
    int &adj_v1, int &adj_v2, int &a, int &b)
{
    vector<int> aa;
    if (t1.v1 == t2.v1 || t1.v1 == t2.v2 || t1.v1 == t2.v3)
        aa.push_back(t1.v1);

    if (t1.v2 == t2.v1 || t1.v2 == t2.v2 || t1.v2 == t2.v3)
        aa.push_back(t1.v2);

    if (t1.v3 == t2.v1 || t1.v3 == t2.v2 || t1.v3 == t2.v3)
        aa.push_back(t1.v3);

    if (aa.size() != 2)
        return false;

    adj_v1 = aa[0];
    adj_v2 = aa[1];

    int tv1 = find_another_v(t1, adj_v1, adj_v2);
    int tv2 = find_another_v(t2, adj_v1, adj_v2);
    if (integral_area(this->vertices[adj_v1], this->vertices[tv1],
        this->vertices[adj_v2]) > 0)
    {
        a = tv1;
        b = tv2;
    }
    else
    {
        a = tv2;
        b = tv1;
    }

    return true;
}

int mesh::target_trangles(int v, triangle &t, triangle **t1, triangle **t2)
{
    int r = vertex_in_triangle(v, t);
    if (r < 0)
        return r;

    if (t.descendents.size() > 0)
    {
        //non-leaf
        list<triangle*>::iterator it = t.descendents.begin();
        while (it != t.descendents.end())
        {
            // traverse the children
            r = this->target_trangles(v, **it, t1, t2);
            if (r == 1)
            {
                // within t
                break;
            }
            else if (r == 0 && *t1 && *t2)
            {
                // on the shared edge and found two
                break;
            }

            it++;
        }
    }
    else
    {
        // leaf
        if (*t1 == NULL)
        {
            *t1 = &t;
        }
        else if (r == 0)
        {
            *t2 = &t;
        }
    }

    return r;
}

inline void adj_each_other(triangle *tri1, triangle *tri2)
{
    if (!tri1 || !tri2)
        return;

    tri1->set_adjacency(tri2);
    tri2->set_adjacency(tri1);
}

inline void adj_one_three(triangle *t, triangle *t1,
    triangle *t2, triangle *t3)
{
    adj_each_other(t, t1);
    adj_each_other(t, t2);
    adj_each_other(t, t3);
}

void mesh::split_one_triangle_to_three(int v, triangle &t,
    triangle **t1, triangle **t2, triangle **t3)
{
    *t1 = new triangle(t.v1, t.v2, v);
    *t2 = new triangle(t.v2, t.v3, v);
    *t3 = new triangle(t.v3, t.v1, v);

    // adjacencies for each new triangle
    // double way!!!
    adj_one_three(*t1, t.adj12, *t2, *t3);
    adj_one_three(*t2, t.adj23, *t3, *t1);
    adj_one_three(*t3, t.adj31, *t1, *t2);

    t.adj12 = t.adj23 = t.adj31 = NULL;

    t.descendents.push_back(*t1);
    t.descendents.push_back(*t2);
    t.descendents.push_back(*t3);

    cout << ".";

    this->flip(**t1, t.v1, t.v2);
    this->flip(**t2, t.v2, t.v3);
    this->flip(**t3, t.v3, t.v1);
}

void mesh::split_two_adjacencies_to_four(int v, triangle &adj1, triangle &adj2,
    triangle **t1, triangle **t2, triangle **t3, triangle **t4)
{
    // compute the shared edge
    // make sure CCW order for (adj_v1, a, adj_v2) & (adj_v2, b, adj_v1)
    int adj_v1, adj_v2, a, b;
    this->find_shared_edge(adj1, adj2, adj_v1, adj_v2, a, b);

    *t1 = new triangle(adj_v1, a, v);
    *t2 = new triangle(v, a, adj_v2);
    *t3 = new triangle(v, adj_v2, b);
    *t4 = new triangle(adj_v1, v, b);

    adj1.descendents.push_back(*t1);
    adj1.descendents.push_back(*t2);
    adj2.descendents.push_back(*t3);
    adj2.descendents.push_back(*t4);

    // set all adjacencies for each new triangles
    // double way!!!
    triangle *t = adj1.find_adjacency((*t1)->v1, (*t1)->v2);
    adj_one_three(*t1, t, *t2, *t4);

    t = adj1.find_adjacency((*t2)->v2, (*t2)->v3);
    adj_one_three(*t2, *t1, t, *t3);

    t = adj2.find_adjacency((*t3)->v2, (*t3)->v3);
    adj_one_three(*t3, *t2, t, *t4);

    t = adj2.find_adjacency((*t4)->v3, (*t4)->v1);
    adj_one_three(*t4, *t1, *t3, t);

    adj1.adj12 = adj1.adj23 = adj1.adj31 = NULL;
    adj2.adj12 = adj2.adj23 = adj2.adj31 = NULL;

    cout << "^";

    this->flip(**t1, adj_v1, a);
    this->flip(**t2, a, adj_v2);
    this->flip(**t3, adj_v2, b);
    this->flip(**t4, b, adj_v1);
}

void mesh::flip(triangle &tri, int a, int b)
{
    // only do leaf triangles
    if (tri.descendents.size() > 0)
        return;

    // find the adjacent triangle against e(a, b).
    triangle *adj = tri.find_adjacency(a, b);
    if (!adj)
        return;

    if (adj->descendents.size() > 0)
    {
/*
        cout << endl << "tri -- v1v2:";
        cout << vertices[tri.v1].x << ", " << vertices[tri.v1].y << ", v2v3:";
        cout << vertices[tri.v2].x << ", " << vertices[tri.v2].y << ", v3v1:";
        cout << vertices[tri.v3].x << ", " << vertices[tri.v3].y << endl;
        cout << " a:" << vertices[a] << ", b:" << vertices[b];
        cout << endl << "!!!!!!!!!!!!!!!adj->descendents.size() > 0!!!!!!!!!!!" << endl;
        cout << vertices[adj->v1].x << ", " << vertices[adj->v1].y << endl;
        cout << vertices[adj->v2].x << ", " << vertices[adj->v2].y << endl;
        cout << vertices[adj->v3].x << ", " << vertices[adj->v3].y << endl;
        list<triangle*>::iterator it_desc = adj->descendents.begin();
        while (it_desc != adj->descendents.end())
        {
            cout << "desc - ";
            cout << " v1: " << vertices[(*it_desc)->v1].x << ", " << vertices[(*it_desc)->v1].y;
            cout << "; v2: " << vertices[(*it_desc)->v2].x << ", " << vertices[(*it_desc)->v2].y;
            cout << "; v3: " << vertices[(*it_desc)->v3].x << ", " << vertices[(*it_desc)->v3].y;
            cout << endl;
            it_desc++;
        }
*/
        return;
    }

    // v (pnt) is is the third vertex of
    // the adjacent triangle against a, b.
    // check if the flip is necessary.
    int v = adj->find_another_vertex(a, b);
    point_d pnt = vertices[v];
    bool r = within_circle(pnt, vertices[tri.v1], vertices[tri.v2], vertices[tri.v3]);
    if (!r)
    {
        // already delaunay. do not flip
        return;
    }

    // c is the third vertex of tri which is not on the shared edge(a, b)
    // new triangles t1 & t2 are composed with (acv) & (vcb)
    int c = tri.find_another_vertex(a, b);

    triangle *t1 = new triangle(a, c, v);
    triangle *t2 = new triangle(v, c, b);

    // make sure they are on CCW direction
    this->make_triangle_valid(*t1);
    this->make_triangle_valid(*t2);

    // t1 t2 are adjacencies for each other
    t1->set_adjacency(t2);
    t2->set_adjacency(t1);

    // transfer adjacency leaf triangles from olds to these two news
    t1->give_me_ur_adjacency(tri);
    t1->give_me_ur_adjacency(*adj);
    t2->give_me_ur_adjacency(tri);
    t2->give_me_ur_adjacency(*adj);

    // deal with the tree
    tri.descendents.push_back(t1);
    tri.descendents.push_back(t2);
    adj->descendents.push_back(t1);
    adj->descendents.push_back(t2);

    // these two become non-leaf, they don't need adjacencies
    tri.adj12 = tri.adj23 = tri.adj31 = NULL;
    adj->adj12 = adj->adj23 = adj->adj31 = NULL;

    cout << "*";

    // flip recursively
    this->flip(*t1, v, a);
    this->flip(*t1, v, b);
    this->flip(*t2, v, a);
    this->flip(*t2, v, b);
}

void mesh::prepare_delaunay_root()
{
    rect_d mbr;
    this->get_mbr(mbr);

    // get a very very large extent
    point_d v;
    v.x = mbr.min_x - mbr.extent_x()*5;
    v.y = mbr.min_y - mbr.extent_y()*5;
    this->vertices[0] = v;
    v.x = mbr.max_x + mbr.extent_x()*5;
    v.y = mbr.min_y - mbr.extent_y()*5;
    this->vertices[1] = v;
    v.x = (mbr.min_x + mbr.max_x)/2;
    v.y = mbr.max_y + mbr.extent_y()*10;
    this->vertices[2] = v;

    // push the first triangle
    // make sure the order of v1, v2 and v3 follow CCW direction
    root = new triangle(0, 1, 2);
}

void mesh::do_delaunay()
{
    cout << "doing delaunay...";
    this->prepare_delaunay_root();
    for (int i=3; i < vertices.size(); i++)
    {
        triangle *t1 = NULL;
        triangle *t2 = NULL;
        triangle *new_t1 = NULL;
        triangle *new_t2 = NULL;
        triangle *new_t3 = NULL;
        triangle *new_t4 = NULL;
        int r = this->target_trangles(i, *root, &t1, &t2);
        if (r == 1)
        {
            //vertices[i] is inside of t1
            this->split_one_triangle_to_three(i,
                *t1, &new_t1, &new_t2, &new_t3);
        }
        else if (r == 0)
        {
            //vertices[i] laies on the shared edge of t1 & t2
            this->split_two_adjacencies_to_four(i,
                *t1, *t2, &new_t1, &new_t2, &new_t3, &new_t4);
        }
    }
}

void mesh::export_triangle(ofstream &obj, triangle &tri, int &triangle_num)
{
    list<triangle*>::const_iterator it_desc = tri.descendents.begin();
    if (it_desc == tri.descendents.end())
    {
        // deal with multiple parents
        set<triangle*>::iterator it_set = exporting.find(&tri);
        if (it_set == exporting.end())
        {
            // skip for all the auxilary triangles
            if (tri.v1 > 2 && tri.v2 > 2 && tri.v3 > 2)
            {
                obj << "f " << tri.v1-2 << " " << tri.v2-2 << " " << tri.v3-2 << endl;
                triangle_num++;
            }

            exporting.insert(&tri);
        }
    }

    while (it_desc != tri.descendents.end())
    {
        export_triangle(obj, **it_desc, triangle_num);
        it_desc++;
    }
}

void mesh::export_obj(string filename)
{
    ofstream obj(filename.c_str());
    obj.clear();
    obj << "# exported from TIN creator." << endl;
    obj << "# Copyright by Zhiqian You, 2015 November." << endl;
    obj << "mtllib ?.mtl" << endl << endl;

    int vertex_num = 0;
    int triangle_num = 0;
    cout << endl << "exporting vertices...";
    vector<point_d>::const_iterator it = vertices.begin();
    // skip for the first auxilary vextices
    it++;it++;it++;
    while (it != vertices.end())
    {
        obj << "v " << (*it).x << " " << (*it).y << " 0" << endl;
        it++;
        vertex_num++;
    }

    cout << endl << "exporting triangles..." << endl;
    exporting.clear();
    export_triangle(obj, *root, triangle_num);

    obj.close();
    cout << endl << vertex_num << " vertices exported." << endl;
    cout << triangle_num << " triangles exported." << endl;
    cout << "result file name: " << filename << endl;
}

void mesh::output(const triangle const *root) const
{
    if (!root)
        return;

    root->output();
    cout << "   ";
    vertices[root->v1].output(); cout << ";  ";
    vertices[root->v2].output(); cout << ";  ";
    vertices[root->v3].output(); cout << endl;
    cout << endl;

    list<triangle*>::const_iterator it = root->descendents.begin();
    while (it != root->descendents.end())
    {
        this->output(*it);
        it++;
    }

    ((mesh*)this)->tag++;
}

mesh g_mesh;

int load_vertices(string filename)
{
    g_mesh.reset();
    point_d v;    
    ifstream stream(filename.c_str());
    int count;
    stream >> count;
    while(stream >> v.x >> v.y)
    {
        g_mesh.vertices.push_back(v);
    }

    return g_mesh.vertices.size();
}

void dojob(string datafile)
{
    load_vertices(datafile);
    g_mesh.do_delaunay();
//    g_mesh.tag = 0;
//    g_mesh.output(g_mesh.root);
//    cout << endl << "triangle count: " << g_mesh.tag << endl;

    string resultfile = RemoveExtNamePart(datafile);
    resultfile = resultfile + "_result.obj";
    g_mesh.export_obj(resultfile);
}

int main(int args, char* argv[])
{
    if (args < 2)
    {
        cout << "command format: dt inputfile";
        getchar();
        return -65436956;
    }

    dojob(argv[1]);

    cout << endl << "-------------------------------------------------------" << endl;
    cout << "TIN creator for CSCE572 final project" << endl;
    cout << "Copyright by Zhiqian You, 2015 November." << endl;
    cout << endl << "press enter to exit...";
    getchar();
    return 0;
}
