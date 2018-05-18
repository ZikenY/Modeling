// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "showoff.h"
#include "ChildView.h"
#include <commoninclude.h>
using namespace ziken;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

vector<circle_d> circles;
vector<point_d> results;

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_COMMAND(FILE_LOAD_CIRCLES, OnLoadCircles)
	ON_COMMAND(FILE_LOAD_POINTS, OnLoadPoints)
	ON_COMMAND(FILE_CLEARALL, OnClearall)
	ON_WM_DROPFILES()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this);
    RECT rect;
    this->GetClientRect(&rect);
    int height = rect.bottom - rect.top;

    vector<circle_d>::const_iterator it = circles.begin();
    while (it != circles.end())
    {
        circle_d c = *it;
        rect.left = 100 + c.center.x - c.r;
        rect.right = 100 + c.center.x + c.r;
        rect.top = height - 100 - c.center.y - c.r;
        rect.bottom = height - 100 - c.center.y + c.r;
        dc.SelectStockObject(NULL_BRUSH);
        CPen pen;
        pen.CreatePen(PS_SOLID, 1, RGB(0, 155, 20));
        dc.SelectObject(&pen);
        dc.Ellipse(&rect);
        it++;
    }

    vector<point_d>::const_iterator it_result = results.begin();
    while (it_result != results.end())
    {
        point_d p = *it_result;
        rect.left = 100 + p.x - 3;
        rect.right = 100 + p.x + 3;
        rect.top = height - 100 - p.y - 3;
        rect.bottom = height - 100 - p.y + 3;
        dc.SelectStockObject(GRAY_BRUSH);
        CPen pen;
        pen.CreatePen(PS_SOLID, 1, RGB(220, 15, 70));
        dc.SelectObject(&pen);
        dc.Rectangle(&rect);
        it_result++;
    }
}

//  load circle data from file.
//  create circle list
void load_circle(string filename)
{
    circles.clear();
    circle_d c;
    ifstream stream(filename.c_str());
    while(stream >> c.center.x >> c.center.y >> c.r)
    {
        circles.push_back(c);
    }
}

void load_results(string resultfile)
{
    results.clear();
    point_d p;
    ifstream stream(resultfile.c_str());
    while(stream >> p.x >> p.y)
    {
        results.push_back(p);
    }
}

void load_circle_withresults(string filename)
{
    load_circle(filename);
    string resultfile = RemoveExtNamePart(filename);
    resultfile = resultfile + "_result.txt";
    load_results(resultfile);
}

void CChildView::OnLoadCircles() 
{
    static CFileDialog dialog(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
        "midterm case file (*.txt)|*.txt|", ::AfxGetMainWnd());
    if (IDOK != dialog.DoModal()) return;
    POSITION pos = dialog.GetStartPosition();
    if (!pos)
    {
        return;
    }

    string datafile = dialog.GetNextPathName(pos);
    load_circle_withresults(datafile);
    this->Invalidate();
	this->UpdateWindow();
}

void CChildView::OnLoadPoints() 
{
    static CFileDialog dialog(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
        "result file (*.txt)|*.txt|", ::AfxGetMainWnd());
    if (IDOK != dialog.DoModal()) return;
    POSITION pos = dialog.GetStartPosition();
    if (!pos)
    {
        return;
    }

    string resultfile = dialog.GetNextPathName(pos);
    load_results(resultfile);
    this->Invalidate();
	this->UpdateWindow();	
}

void CChildView::OnClearall() 
{
    circles.clear();
    results.clear();
    this->Invalidate();
    this->UpdateWindow();
	
}

void CChildView::OnDropFiles(HDROP hDropInfo) 
{
    char filename[255];
    ::memset(filename, 0, sizeof(char)*255);

    UINT count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    if (count > 0)
    {        
        DragQueryFile(hDropInfo, 0, filename, sizeof(filename));
        this->OnClearall();
        load_circle_withresults(filename);
    }

    DragFinish(hDropInfo);
    this->Invalidate();
	this->UpdateWindow();
	CWnd ::OnDropFiles(hDropInfo);
}

void CChildView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd ::OnShowWindow(bShow, nStatus);
	
    this->DragAcceptFiles();	
}
