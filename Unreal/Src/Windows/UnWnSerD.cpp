// UnWnSerD.cpp : implementation file
//

#include "stdafx.h"
#include "unwn.h"
#include "UnWnSerD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogServerProperties dialog


CDialogServerProperties::CDialogServerProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogServerProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogServerProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogServerProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogServerProperties)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogServerProperties, CDialog)
	//{{AFX_MSG_MAP(CDialogServerProperties)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogServerProperties message handlers

BOOL CDialogServerProperties::OnInitDialog()
{
    CDialog::OnInitDialog();
    return FALSE; // FALSE tells Windows not to set the input focus. 
                  // We do this because the dialog is expected to be part
                  // of a tab group and we don't want to change the focus.
}
