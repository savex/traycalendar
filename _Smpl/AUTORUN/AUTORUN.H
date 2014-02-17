// autorun.h : main header file for the AUTORUN application
//

#if !defined(AFX_AUTORUN_H__41136785_E22E_4EBB_ABA5_82A885218B43__INCLUDED_)
#define AFX_AUTORUN_H__41136785_E22E_4EBB_ABA5_82A885218B43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAutorunApp:
// See autorun.cpp for the implementation of this class
//

class CAutorunApp : public CWinApp
{
public:
	CAutorunApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutorunApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAutorunApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTORUN_H__41136785_E22E_4EBB_ABA5_82A885218B43__INCLUDED_)
