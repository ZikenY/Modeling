#include <commoninclude.h>
#include <mathlib2d.h>
#include <stringfuncs.h>
using namespace ziken;

struct semicircle;
struct pointy : public point_d
{
    bool intersect;
    bool starting;  //starting point or ending point
    list<semicircle*> ref;  //references to semicircles

    inline bool operator==(const pointy& rhs) const
    {
        return (this->y == rhs.y) ? true : false;
    };
    inline bool operator>(const pointy& rhs) const
    {
        return (this->y > rhs.y) ? true : false;
    };
    inline bool operator<(const pointy& rhs) const
    {
        return (this->y < rhs.y) ? true : false;
    };

    pointy(double _x, double _y){x=_x; y=_y;};
    pointy(double _x, double _y,
        bool _intersect, bool _starting)
    {
        x=_x; y=_y;
        intersect =_intersect;
        starting = _starting;
    };

    pointy(){x=y=222222222;};

    void set(double _x, double _y)
    {
        x = _x; y = _y;
    };

    semicircle* getsemi(const index) const
    {
        int i = 0;
        list<semicircle*>::const_iterator it = ref.begin();
        while (it != ref.end())
        {
            if (index == i++)
                return *it;
        }

        return NULL;
    }
};

struct circle;
struct semicircle
{
    circle *pCircle;
    pointy start;
    pointy end;
    point_d middle;
    double tag;     //  x-axis order indicator for status T

    bool operator>(const semicircle& rhs) const
    {
        return (rhs.tag > this->tag) ? true : false;
    };

    bool operator<(const semicircle& rhs) const
    {
        return (rhs.tag < this->tag) ? true : false;
    };

private:
    semicircle(circle *c)
    {
        pCircle = c;
        start.intersect = false;
        start.starting = true;
        end.intersect = false;
        end.starting = false;
    };

    semicircle();
    semicircle& operator=(const semicircle& rhs);
    semicircle(const semicircle& rhs);

friend circle;
};

struct circle : public circle_d
{
    semicircle *pLeftSemi;
    semicircle *pRightSemi;
    string name;    //  no much useful
    set<circle*> history;

    void init()
    {
        pLeftSemi = new semicircle(this);
        pRightSemi = new semicircle(this);
    }

    void init(const circle& rhs)
    {
        this->init();
        c = rhs.c;
        r = rhs.r;
        name = rhs.name;
        this->constructor();
    }

    circle& operator=(const circle& rhs)
    {
        if (this == &rhs)
            return *this;

        this->init(rhs);
        return *this;
    };

    circle(const circle& rhs)
    {
        this->init(rhs);
    }

    circle()
    {
        this->init();
    };

    ~circle()
    {
        delete pLeftSemi;
        delete pRightSemi;
    };

    void constructor()
    {
        pLeftSemi->start.x = c.x;
        pLeftSemi->start.y = c.y + r;
        pLeftSemi->start.ref.clear();
        pLeftSemi->start.ref.push_back(pLeftSemi);
        pLeftSemi->end.x = c.x;
        pLeftSemi->end.y = c.y - r;
        pLeftSemi->middle.x = c.x - r;
        pLeftSemi->middle.y = c.y;
        pLeftSemi->end.ref.clear();
        pLeftSemi->end.ref.push_back(pLeftSemi);

        pRightSemi->start.x = c.x;
        pRightSemi->start.y = c.y + r;
        pRightSemi->start.ref.clear();
        pRightSemi->start.ref.push_back(pRightSemi);
        pRightSemi->end.x = c.x;
        pRightSemi->end.y = c.y - r;
        pRightSemi->middle.x = c.x + r;
        pRightSemi->middle.y = c.y;
        pRightSemi->end.ref.clear();
        pRightSemi->end.ref.push_back(pRightSemi);
    }

    pointy getleftstart()
    {
        this->constructor();
        return pLeftSemi->start;
    }

    pointy getleftend()
    {
        this->constructor();
        return pLeftSemi->end;
    }

    pointy getrightstart()
    {
        this->constructor();
        return pRightSemi->start;
    }

    pointy getrightend()
    {
        this->constructor();
        return pRightSemi->end;
    }
};


string getseminame(semicircle &sc)
{
    if (sc.middle.x < sc.start.x)
    {
        return sc.pCircle->name + "_l";
    }
    else
    {
        return sc.pCircle->name + "_r";
    }
}

vector<circle> circles; //  circle list
vector<pointy> events;  //  event queue Q
list<semicircle*> status;   //  status structure T. increasing order of x-axis
vector<point_d> intersections;  //results

//  load circle data from file.
//  create circle list
int load_circle(string filename)
{
    circles.clear();
    circle c;
    ifstream stream(filename.c_str());
    while(stream >> c.c.x >> c.c.y >> c.r)
    {
        c.name = "c" + IntToStr(circles.size());
        circles.push_back(c);
    }

    for (int i=0; i<circles.size(); i++)
    {
        cout << "x:" << circles[i].c.x
            << ", y:" << circles[i].c.y
            << ", r:" << circles[i].r << endl;

        circles[i].constructor();   //  construct the semi-circles
    }

    cout << "circle count: " << circles.size() << endl;
    return circles.size();
}

//  put all the starting/ending points of those semicircles to events queue Q
void init_events()
{
    cout << endl << endl << endl;
    cout << "initial event queue Q:" << endl;
    events.clear();
    vector<circle>::iterator it = circles.begin();
    while (it != circles.end())
    {
        events.push_back((*it).getleftstart());
        events.push_back((*it).getrightstart());
        events.push_back((*it).getleftend());
        events.push_back((*it).getrightend());
        it++;
    }

    //  create a max heap
    make_heap(events.begin(), events.end());
    vector<pointy>::iterator it_events = events.begin();
/*
    while (it_events != events.end())
    {
        cout << "is starting: " << (*it_events).starting << ", x:"
            << (*it_events).x << ", y: " << (*it_events).y
            << " -->" << (*it_events).getsemi(0)->name << endl;
        it_events++;
    }
    */
}

//  bound a intersect point with the corresponded semi-circles.
//  then insert it to the event queue Q
void insert_to_events(pointy &intersect, semicircle &sc0, semicircle &sc1)
{
    intersect.intersect = true;
    intersect.ref.push_back(&sc0);
    intersect.ref.push_back(&sc1);
    events.push_back(intersect);

    cout << endl << "event queue y before heapfy. intersect y:" << intersect.y << endl;
    for (int i=0; i<events.size(); i++)
    {
        cout << "y:" << events[i].y << " ";
    }
    cout << endl;
    
    //  heapfy
    push_heap(events.begin(), events.end());

    cout << "event queue y after insertion: " << endl;
    for (i=0; i<events.size(); i++)
    {
        cout << "y:" << events[i].y << " ";
    }
    cout << endl << endl;
}

//  compute the intersection(s) between two semi,
//  and put them into event queue
//  if the two semi belong to the same circle, do nothing.
void intersect_event(semicircle &sc0, semicircle &sc1)
{
    if (sc0.pCircle == sc1.pCircle)
        return;

    set<circle*>::iterator it_set = sc0.pCircle->history.find(sc1.pCircle);
    if (it_set != sc0.pCircle->history.end())
    {
        return;
    }

    it_set = sc1.pCircle->history.find(sc0.pCircle);
    if (it_set != sc1.pCircle->history.end())
    {
        return;
    }

    sc0.pCircle->history.insert(sc1.pCircle);
    sc1.pCircle->history.insert(sc0.pCircle);

    pointy intersect0, intersect1;
    int r = intersect(*sc0.pCircle, *sc1.pCircle, intersect0, intersect1);
    if (r > 0)
    {
        intersections.push_back(intersect0);
        // insert intersect0 into event queue
        insert_to_events(intersect0, sc0, sc1);
        cout << "insert intersect: " << getseminame(sc0) << " & " << getseminame(sc1)
            << " x:" << intersect0.x << ", y:" <<intersect0.y << endl;

        if (r == 2)
        {
            intersections.push_back(intersect1);
            // insert intersect1 into event queue
            insert_to_events(intersect1, sc0, sc1);
            cout << "insert intersect: " << getseminame(sc0) << " & " << getseminame(sc1)
                << " x:" << intersect1.x << ", y:" <<intersect1.y << endl;
        }
    }
}

//  introduce a new semicircle to the status structure
//  be called when a new starting point being swept
void insert_new_status(semicircle *sc)
{
    list<semicircle*>::iterator it = status.begin();
    while (it != status.end())
    {
        if ((*it)->tag > sc->tag)
        {
            //  put sc to the left of *it
            status.insert(it, sc);
            break;
        }

        it++;
    }

    if (it == status.end())
    {
        //  sc->tag is the largest one
        status.insert(status.end(), sc);
    }

    //  move it to the right place of c
    it = status.begin();
    while (it != status.end())
    {
        if (*it == sc)
        {
            break;
        }
        it++;
    }

    //  check intersections with the adjacent circles
    if (it != status.begin())
    {
        //  check the left neighbor
        semicircle &left = **(--it);
        //  restore the original it position
        it++;
        intersect_event(*sc, left);
    }

    if (++it != status.end())
    {
        //  check the right neighbor, notice that it has already been pointed to the right neighbor
        intersect_event(*sc, **(it));
    }

    cout << "insert " << getseminame(*sc) << " to status... updated:";
    it = status.begin();
    while (it != status.end())
    {
        cout << getseminame(*(*it)) << " ";
        it++;
    }

    cout << endl;
}

void remove_from_status(semicircle *sc)
{
    list<semicircle*>::iterator it = status.begin();
    while (it != status.end())
    {
        if ((*it) == sc)
        {
            status.erase(it);
            break;
        }

        it++;
    }

    cout << "half of " << sc->pCircle->name << " has been remove from status... " << endl;
    cout << "remaining semi-circles: ";
    it = status.begin();
    while (it != status.end())
    {
        cout << getseminame(*(*it)) << " ";
        it++;
    }
    cout << endl;
}

bool check_semi_in_semis(semicircle* sc, list<semicircle*> &semicircles)
{
    list<semicircle*>::iterator it = semicircles.begin();
    while (it != semicircles.end())
    {
        if (*it == sc)
            return true;

        it++;
    }

    return false;
}

//  swap the circles' position in status
void swap_positions(list<semicircle*> &semicircles)
{
    vector<semicircle*> its;
    list<semicircle*>::iterator it = status.begin();
    while (it != status.end())
    {
        if (check_semi_in_semis(*it, semicircles))
        {
            cout << "   check: " << getseminame(*(*it)) << endl;
            its.push_back(*it);
        }

        it++;
    }

    int index = its.size()-1;
    it = status.begin();
    while (it != status.end())
    {
        if (check_semi_in_semis(*it, semicircles))
        {
            cout << endl << "swap: " << getseminame(*(its[index]))
                << " & " << getseminame(*(*it));
            *it = its[index--];
        }

        it++;
    }

    cout << endl << "after swap: ";
    it = status.begin();
    while (it != status.end())
    {
        cout << getseminame(*(*it)) << " ";
        it++;
    }
    cout << endl;
}

//  swap the circles' position in status
//  then check the new neighboors
void swap_positions(semicircle &sc1, semicircle &sc2)
{
    semicircle *left_neighbor = NULL;
    semicircle *right_neighbor = NULL;
    list<semicircle*>::iterator it_left, it_right;
    list<semicircle*>::iterator it = status.begin();
    while (it != status.end())
    {
        //  get the left/right position and neighbors of these two
        if ((*it) == &sc1 || (*it) == &sc2)
        {
            if (!left_neighbor)
            {
                it_left = it;
                if (it != status.begin())
                {
                    left_neighbor = *(--it);
                    it++;
                }
                else
                {
                    left_neighbor = *it;
                }
            }
            else
            {
                it_right = it;
                it++;
                if (it != status.end())
                {
                    it--;
                    right_neighbor = *(++it);
                    it--;
                }
                else
                {
                    it--;
                    left_neighbor = *it;
                }
            }
        }

        it++;
    }

    //  swap
    semicircle* sc = *it_left;
    *it_left = *it_right;
    *it_right = sc;

    //  check the two "new" neighboors
    if (left_neighbor)
    {
        intersect_event(**it_left, *left_neighbor);
    }

    if (right_neighbor)
    {
        intersect_event(**it_right, *right_neighbor);
    }
}

void sweep_me()
{
    intersections.clear();
    status.clear();
    while(events.size() > 0)
    {
        //  pop the head event of the max heap one by one
        pop_heap(events.begin(), events.end());
        pointy event = events[events.size()-1];
        events.pop_back();
        cout << "pop event: is intersect:" << event.intersect
            << ", is starting:"<< event.starting
            << " -->" << getseminame(*(event.getsemi(0)));
        cout << "; events remain " << events.size() << endl;

        if (event.intersect)
        {
            //  intersection point.
            //  modify every circle.tag
            semicircle *scs[2];
            int scindex = 0;
            list<semicircle*>::iterator it_ref = event.ref.begin();
            while (it_ref != event.ref.end())
            {
                (*it_ref)->tag = event.x;
                scs[scindex++] = *it_ref;
                it_ref++;
            }

            //  swap the positions of two corresponded circles in status
            swap_positions(*scs[0], *scs[1]);
        }
        else if (event.starting)
        {
            //  starting point
            //  introduce the corresponded circle to the status structure T
            semicircle *sc = event.getsemi(0);
            sc->tag = event.x;
            //  insert c to T with right order. then check intersections with the adjacent circles
            insert_new_status(sc);
        }
        else
        {
            //  ending point
            //  remove the corresponded circle from the status structure 
            semicircle *sc = event.getsemi(0);
            remove_from_status(sc);
        }
    }
}

//  store intersections to a text file
void save_result(string filename)
{
    cout << endl << "result count: " << intersections.size() << endl;
    for (int i=0; i<intersections.size(); i++)
    {
        cout << "x:" << intersections[i].x
            << " y:" << intersections[i].y << endl;
    }

    if (trim(filename) == "")
        return;

    ofstream resultfile(filename.c_str());
    resultfile.clear();

    for (i=0; i<intersections.size(); i++)
    {
        resultfile << intersections[i].x
            << " " << intersections[i].y << endl;
    }

    resultfile << "intersection number: " << intersections.size();
    resultfile.close();
}

//  what's up??
void dojob(string datafile)
{
    load_circle(datafile);
    init_events();
    sweep_me();

    string resultfile = RemoveExtNamePart(datafile);
    resultfile = resultfile + "_result.txt";
    save_result(resultfile);
}

int main(int args, char* argv[])
{
    if (args != 2)
    {
        cout << "r u kiddding me?";
        getchar();
        return -65436956;
    }

    dojob(argv[1]);

    getchar();
    return 0;
}
