/*=============================================================================
	UnPlatfm.cpp: All generic, platform-specific routines-specific routines.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"
#include "UnWn.h"
#include "UnPswd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*-----------------------------------------------------------------------------
	CPasswordDlg dialog
-----------------------------------------------------------------------------*/

CPasswordDlg::CPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPasswordDlg::IDD, pParent)
	{
	GUARD;
	//{{AFX_DATA_INIT(CPasswordDlg)
	//}}AFX_DATA_INIT
	UNGUARD("CPasswordDlg::CPasswordDlg");
	};

void CPasswordDlg::DoDataExchange(CDataExchange* pDX)
	{
	GUARD;
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPasswordDlg)
	DDX_Control(pDX, IDC_PROMPT, m_Prompt);
	DDX_Control(pDX, IDC_PASSWORD, m_Password);
	DDX_Control(pDX, IDC_NAME, m_Name);
	//}}AFX_DATA_MAP
	UNGUARD("CPasswordDlg::DoDataExchange");
	};

BEGIN_MESSAGE_MAP(CPasswordDlg, CDialog)
	//{{AFX_MSG_MAP(CPasswordDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*-----------------------------------------------------------------------------
	Custom
-----------------------------------------------------------------------------*/

BOOL CPasswordDlg::OnInitDialog() 
	{
	GUARD;
	CDialog::OnInitDialog();
	//
	// Custom initialization:
	//
	SetWindowText(Title);
	m_Name.ReplaceSel(Name);
	m_Password.ReplaceSel(Password);
	m_Prompt.SendMessage(WM_SETTEXT,0,(LPARAM)Prompt);
	//
	m_Name.SetFocus();
	//
	return FALSE;
	UNGUARD("CPasswordDlg::OnInitDialog");
	};

void CPasswordDlg::OnCancel() 
	{
	GUARD;
	CDialog::OnCancel();
	UNGUARD("CPasswordDlg::OnCancel");
	};

void CPasswordDlg::OnOK() 
	{
	GUARD;
	char T[256]; CString C;
	//
	m_Name.GetLine(0,T);
	C=T; C.TrimLeft(); C.TrimRight(); strcpy(Name,C);
	//
	m_Password.GetLine(0,T);
	C=T; C.TrimLeft(); C.TrimRight(); strcpy(Password,C);
	//
	strcpy(Encryption,"");
	//
	CDialog::OnOK();
	UNGUARD("CPasswordDlg::OnOK");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/

