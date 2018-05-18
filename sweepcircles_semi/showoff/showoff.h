// showoff.h : main header file for the SHOWOFF application
//

#if !defined(AFX_SHOWOFF_H__B6E14628_65A6_453B_906B_86B344616F2D__INCLUDED_)
#define AFX_SHOWOFF_H__B6E14628_65A6_453B_906B_86B344616F2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CShowoffApp:
// See showoff.cpp for the implementation of this class
//

class CShowoffApp : public CWinApp
{
public:
	CShowoffApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShowoffApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CShowoffApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWOFF_H__B6E14628_65A6_453B_906B_86B344616F2D__INCLUDED_)
