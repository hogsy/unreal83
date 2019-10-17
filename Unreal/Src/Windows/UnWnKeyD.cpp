// UnWnKeyD.cpp : implementation file
//

#include "stdafx.h"
#include "unwn.h"
#include "UnWnKeyD.h"
#include "Unreal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogKeySelect dialog


CDialogKeySelect::CDialogKeySelect(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogKeySelect::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogKeySelect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    SelectedKey = 0;
    IsAllowed = 0;
}

//----------------------------------------------------------------------------
//               Dialog Initialization
//----------------------------------------------------------------------------
BOOL CDialogKeySelect::OnInitDialog()
{
    CDialog::OnInitDialog();
    return TRUE;
}

void CDialogKeySelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogKeySelect)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogKeySelect, CDialog)
	//{{AFX_MSG_MAP(CDialogKeySelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogKeySelect message handlers

LRESULT CDialogKeySelect::DefWindowProc(UINT Message, WPARAM wParam, LPARAM lParam) 
{
    //tbd:    if( Message != 0x36a )
    //tbd:    {
    //tbd:        debugf( LOG_Info, "DefWP(%x,%x,%x)", Message, wParam, lParam );
    //tbd:    }
    if( Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN )
    {
        // We override all key handling ...
        const int VirtualKey = (int)wParam;
        const int KeyData    = (int)lParam;
        if( VirtualKey == VK_ESCAPE )
        {
            OnCancel();
        }
        else if( VirtualKey >= 256 )
        {
            // Ignore it
        }
        else if( IsAllowed != 0 && !IsAllowed[VirtualKey] )
        {
            AfxMessageBox( "Sorry, but you are not allowed to use that key." );
        }
        else
        {
            SelectedKey = VirtualKey;
            OnOK();
        }
        return 0;
    }
    else if( Message == WM_HELP )
    {
        return 0;
    }
    else if( Message == WM_GETDLGCODE )
    {
        //todo: Put this and WM_KEYDOWN handler into the message map so we can get rid of DefWindowProc.
        return DLGC_WANTALLKEYS;
    }
    else
    {
    	return CDialog::DefWindowProc(Message, wParam, lParam);
    }
}


