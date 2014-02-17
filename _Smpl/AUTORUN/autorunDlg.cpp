// autorunDlg.cpp : implementation file
//


#include "stdafx.h"
#include "autorun.h"
#include "autorunDlg.h"

#include <mmsystem.h>
#include <direct.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutorunDlg dialog

CAutorunDlg::CAutorunDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAutorunDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAutorunDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutorunDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutorunDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAutorunDlg, CDialog)
	//{{AFX_MSG_MAP(CAutorunDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutorunDlg message handlers

BOOL CAutorunDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	regupdate();

	if(path[0]==0) instaled=false;
	else instaled=true;

	{
		CWinApp *wapp=AfxGetApp( );
		int x=0,y=0;
		while(wapp->m_pszHelpFilePath[x++]!=0)
		{
			if(wapp->m_pszHelpFilePath[x-1]=='\\') y=x;
		}
		strcpy(cd,wapp->m_pszHelpFilePath);
		cd[y]=0;
	}


	back.LoadBitmap(IDB_BACK);

	bmp[0].x=15;bmp[0].y=79;
	bm[0][0].LoadBitmap(IDB_PLAY1);
	bm[0][1].LoadBitmap(IDB_PLAY2);
	bm[0][2].LoadBitmap(IDB_PLAY0);

	bmp[1].x=148;bmp[1].y=274;
	bm[1][0].LoadBitmap(IDB_INST1);
	bm[1][1].LoadBitmap(IDB_INST2);
	bm[1][2].LoadBitmap(IDB_INST0);

	bmp[2].x=159;bmp[2].y=265;
	bm[2][0].LoadBitmap(IDB_UNINST1);
	bm[2][1].LoadBitmap(IDB_UNINST2);
	bm[2][2].LoadBitmap(IDB_UNINST0);

	bmp[3].x=259;bmp[3].y=289;
	bm[3][0].LoadBitmap(IDB_EXIT1);
	bm[3][1].LoadBitmap(IDB_EXIT2);
	bm[3][2].LoadBitmap(IDB_EXIT0);

	curs=-1;
	down=false;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAutorunDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{

		CPaintDC dc(this); // device context for painting
		CDC dcb;
		dcb.CreateCompatibleDC(&dc);

		dcb.SelectObject(back);
		dc.BitBlt(0,0,640,480,&dcb,0,0,SRCCOPY);

		if (instaled)
		{
			if(curs!=0) dcb.SelectObject(bm[0][0]);
			else if(down) dcb.SelectObject(bm[0][2]);
			else dcb.SelectObject(bm[0][1]);
			dc.BitBlt(bmp[0].x,bmp[0].y,100,100,&dcb,0,0,SRCCOPY);

			if(curs!=2) dcb.SelectObject(bm[2][0]);
			else if(down) dcb.SelectObject(bm[2][2]);
			else dcb.SelectObject(bm[2][1]);
			dc.BitBlt(bmp[2].x,bmp[2].y,102,100,&dcb,0,0,SRCCOPY);
		}
		else
		{
			if(curs!=1) dcb.SelectObject(bm[1][0]);
			else if(down) dcb.SelectObject(bm[1][2]);
			else dcb.SelectObject(bm[1][1]);
			dc.BitBlt(bmp[1].x,bmp[1].y,102,100,&dcb,0,0,SRCCOPY);
		}

		if(curs!=3) dcb.SelectObject(bm[3][0]);
		else if(down==false) dcb.SelectObject(bm[3][1]);
		else dcb.SelectObject(bm[3][2]);
		dc.BitBlt(bmp[3].x,bmp[3].y,100,100,&dcb,0,0,SRCCOPY);

		
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAutorunDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CAutorunDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default


	int newcurs=-1;
	for(int x=0;x<4;x++)
	{
		if((x==1) && (instaled==true)) continue;
		if((x==2) && (instaled==false)) continue;

		CRect re(bmp[x],CSize(90,40));
		if((x==1) || (x==2)) re.bottom+=25;

		if(re.PtInRect(point)) newcurs=x;
	}
	if (curs!=newcurs)
	{
		curs=newcurs;
		Invalidate(false);

//		if (curs!=-1) mciSendString("play button.wav",0,0,0);
		if (curs!=-1) PlaySound("button.wav",0,SND_FILENAME|SND_ASYNC);
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CAutorunDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	down=true;
		Invalidate(false);
	CDialog::OnLButtonDown(nFlags, point);

	char str[256];
	switch(curs)
	{
	case 0:
		sprintf(str,"%s\\WDWRacing.exe",path);
		_chdir(path);
		WinExec(str,SW_SHOW);
		PostQuitMessage(0);
		break;
	case 1:
	case 2:
		sprintf(str,"%s\\setup\\setup.exe",cd);
		_chdir(cd);
		WinExec("setup\\setup.exe",SW_SHOW);
		PostQuitMessage(0);
		break;
	case 3:
		PostQuitMessage(0);
		break;
	}
}

void CAutorunDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	down=false;
	Invalidate(false);
	CDialog::OnLButtonUp(nFlags, point);
}

int CAutorunDlg::regupdate()
{
	unsigned long t,l;
	memset(path,0,sizeof(path));
	HKEY ke;
	
	l=256;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Disney Interactive\\Walt Disney World Quest Magical Racing Tour\\",0,KEY_QUERY_VALUE|KEY_READ,&ke);
	RegQueryValueEx(ke,"InstallDir",0,&t,(unsigned char*)path,&l);
	RegCloseKey(ke);
	
	return 0;
}
