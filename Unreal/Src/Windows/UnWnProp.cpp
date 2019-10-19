// UnWnProp.cpp : implementation file
//

#include "stdafx.h"
#include "unwn.h"
#include "UnWnProp.h"
#include "UnWnPref.h"
#include "UnWnActD.h"
#include "UnWnSenD.h"
#include "UnWnDevD.h"
#include "UnWnAudD.h"
#include "UnWnSerD.h"

#include "UnConfig.h" // So we can save configurations.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDialog * CDialogProperties::LatestInstance = 0;

/////////////////////////////////////////////////////////////////////////////
// CDialogProperties dialog


CDialogProperties::CDialogProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogProperties)
	DDX_Control(pDX, IDC_TABS, TabControl);
	//}}AFX_DATA_MAP
}

BOOL CDialogProperties::OnInitDialog()
{
    CDialog::OnInitDialog();
    CurrentPage = NoPage;
    PageDialog = 0;
    LatestInstance = this;
    {
        TC_ITEM TabInfo;
        TabInfo.mask = TCIF_TEXT;
        BOOL CreatedOkay = TRUE;

        TabInfo.pszText = (char*)"Preferences";
        CreatedOkay = CreatedOkay && TabControl.InsertItem( PreferencesPage, &TabInfo ) != -1;

        TabInfo.pszText = (char*)"Audio";
        CreatedOkay = CreatedOkay && TabControl.InsertItem( AudioPage, &TabInfo ) != -1;

        TabInfo.pszText = (char*)"Video";
        CreatedOkay = CreatedOkay && TabControl.InsertItem( VideoPage, &TabInfo ) != -1;

        TabInfo.pszText = (char*)"Devices";
        CreatedOkay = CreatedOkay && TabControl.InsertItem( DevicesPage, &TabInfo ) != -1;

        TabInfo.pszText = (char*)"Actions";
        CreatedOkay = CreatedOkay && TabControl.InsertItem( ActionsPage, &TabInfo ) != -1;

        TabInfo.pszText = (char*)"Sensitivity";
        CreatedOkay = CreatedOkay && TabControl.InsertItem( SensitivityPage, &TabInfo ) != -1;

//todo:add        TabInfo.pszText = "Server";
//todo:add        CreatedOkay = CreatedOkay && TabControl.InsertItem( ServerPage, &TabInfo ) != -1;

        if( !CreatedOkay )
        {
            appError( "Error creating properties page" );
        }

        StartNewPage();
        TabControl.SetFocus();
    }
    return TRUE;
}

BEGIN_MESSAGE_MAP(CDialogProperties, CDialog)
	//{{AFX_MSG_MAP(CDialogProperties)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABS, OnSelchangeTabs)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TABS, OnSelchangingTabs)
	ON_BN_CLICKED(ID_OPEN, OnOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------
//                     What is the selected page?
//----------------------------------------------------------------------------
CDialogProperties::PageType CDialogProperties::SelectedPage() const
{
    PageType Page = NoPage;
    int Selection = TabControl.GetCurSel();
    if( Selection != -1 )
    {
        Page = PageType(Selection + 1); // +1 to convert 0-based index into PageType.
    }
    return Page;
}

//----------------------------------------------------------------------------
//             Destroy any existing page dialog.
//----------------------------------------------------------------------------
// Don't call this if the page destroyed itself 
void CDialogProperties::DestroyCurrentPage() 
{
    if( PageDialog != 0 )
    {
        PageDialog->DestroyWindow();
        // There is no need to delete the dialog, since it deletes itself
        // during the window destruction process.
        PageDialog = 0;
    }
}

//----------------------------------------------------------------------------
//          Show a new page for the currently selected tab.
//----------------------------------------------------------------------------
void CDialogProperties::StartNewPage()
{
    CurrentPage = SelectedPage();
    if( CurrentPage == NoPage ) // No page selected?
    {
        TabControl.SetCurSel(0); // Select first page.
        CurrentPage = SelectedPage();
    }
    CWnd * TabParent = this;
    CDialog * NewDialog = 0;
    int NewDialogId = 0;
    switch(CurrentPage)
    {
        case NoPage:
        {
            // This probably never happens, but let's just quietly ignore it if it does.
            break;
        }
        case PreferencesPage:
        {
            NewDialog = new CDialogPreferences(TabParent);
            NewDialogId = CDialogPreferences::IDD;
            break;
        }
        case AudioPage:
        {
            NewDialog = new CDialogAudio(TabParent);
            NewDialogId = CDialogAudio::IDD;
            break;
        }
        case VideoPage:
        {
            //todo: 
            NewDialog = new CDialogAudio(TabParent);
            NewDialogId = CDialogAudio::IDD;
            break;
        }
        case DevicesPage:
        {
            NewDialog = new CDialogInputDevices(TabParent);
            NewDialogId = CDialogInputDevices::IDD;
            break;
        }
        case ActionsPage:
        {
            NewDialog = new CDialogInputActions(TabParent);
            NewDialogId = CDialogInputActions::IDD;
            break;
        }
        case SensitivityPage:
        {
            NewDialog = new CDialogInputSensitivity(TabParent);
            NewDialogId = CDialogInputSensitivity::IDD;
            break;
        }
//todo:add        case ServerPage:
//todo:add        {
//todo:add            NewDialog = new CDialogServerProperties(TabParent);
//todo:add            NewDialogId = CDialogServerProperties::IDD;
//todo:add            break;
//todo:add        }
        default:
        {
            appError( "Logic" );
            break;
        }
    }
    // Create the new dialog before we destroy the old one, so
    // they change smoothly.
    if( NewDialog != 0 )
    {
        NewDialog->Create(NewDialogId, TabParent );
        RECT Area;
        TabControl.GetWindowRect(&Area);
        TabControl.AdjustRect( FALSE, &Area );
        TabParent->ScreenToClient( &Area );
        NewDialog->SetWindowPos( &TabControl, Area.left, Area.top, 0, 0, SWP_NOSIZE/*|SWP_NOZORDER*/ );
        NewDialog->ShowWindow(SW_SHOW);
    }
    DestroyCurrentPage();
    PageDialog = NewDialog;
    PageDialogId = NewDialogId;
}

/////////////////////////////////////////////////////////////////////////////
// CDialogProperties message handlers

//----------------------------------------------------------------------------
//             Accept the input for the current page.
//----------------------------------------------------------------------------
BOOL CDialogProperties::Accept() 
{
    BOOL Accepted = FALSE;
    switch( CurrentPage )
    {
        case NoPage:
        {
            Accepted = TRUE;
            break;
        }
        case PreferencesPage:
        {
            CDialogPreferences & Dialog = *(CDialogPreferences*)PageDialog;
            Accepted = Dialog.Accept();
            if( Accepted )
            {
                GConfiguration.WriteValuesToFile( FConfiguration::PreferencesSection );
            }
            break;
        }
        case AudioPage:
        {
            CDialogAudio & Dialog = *(CDialogAudio*)PageDialog;
            Accepted = Dialog.Accept();
            if( Accepted )
            {
                GConfiguration.WriteValuesToFile( FConfiguration::AudioSection );
            }
            break;
        }
        case VideoPage:
        {
            CDialogAudio & Dialog = *(CDialogAudio*)PageDialog;
            Accepted = Dialog.Accept();
            if( Accepted )
            {
                GConfiguration.WriteValuesToFile( FConfiguration::AudioSection );
            }
            break;
        }
        case DevicesPage:
        {
            CDialogInputDevices & Dialog = *(CDialogInputDevices*)PageDialog;
            Accepted = Dialog.Accept();
            if( Accepted )
            {
                GConfiguration.WriteValuesToFile( FConfiguration::InputSection );
            }
            break;
        }
        case ActionsPage:
        {
            CDialogInputActions & Dialog = *(CDialogInputActions*)PageDialog;
            Accepted = Dialog.Accept();
            if( Accepted && Dialog.ChangesWereSaved )
            {
                GConfiguration.WriteValuesToFile( FConfiguration::ActionSection );
            }
            break;
        }
        case SensitivityPage:
        {
            CDialogInputSensitivity & Dialog = *(CDialogInputSensitivity*)PageDialog;
            Accepted = Dialog.Accept();
            if( Accepted && Dialog.ChangesWereSaved )
            {
                GConfiguration.WriteValuesToFile( FConfiguration::InputSection );
            }
            break;
        }
//todo:add        case ServerPage:
//todo:add        {
//todo:add            //todo: Server page input acceptance
//todo:add            Accepted = TRUE;
//todo:add            break;
//todo:add        }
        default:
        {
            appError( "Logic" );
            break;
        }
    }
	return Accepted;
}

//----------------------------------------------------------------------------
//             A new page has been selected.
//----------------------------------------------------------------------------
void CDialogProperties::OnSelchangeTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
    StartNewPage();
    *pResult = 0;
}

//----------------------------------------------------------------------------
//             The current page is about to be changed.
//----------------------------------------------------------------------------
void CDialogProperties::OnSelchangingTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
    *pResult = !Accept();
}

#if 0 //tbd: obsolete
//----------------------------------------------------------------------------
//             Command from child window (current page).
//----------------------------------------------------------------------------
BOOL CDialogProperties::OnCommand(WPARAM wParam, LPARAM lParam) 
{
    BOOL Processed = FALSE; // TRUE when command is processed.
    const SHORT NotifyCode = HIWORD(wParam);
    const SHORT Id         = LOWORD(wParam); // Item, control, or accelerator id.
    if( PageDialog != 0 && PageDialogId == Id )
    {
        // We have a command from the current page dialog.
        switch(NotifyCode)
        {
            case NM_RETURN:
            {
                // User pressed Enter in a page - treat this as OnOK.
                OnOK();
                Processed = TRUE;
                break;
            }
            case IDCANCEL:
            {
                //todo:[Mark] Is it okay to pass IDCANCEL using WM_COMMAND?
                // User pressed Esc in a page - treat this as OnCancel.
                OnCancel();
                Processed = TRUE;
                break;
            }
        }
    }
    if( !Processed )
    {
        Processed = CDialog::OnCommand(wParam, lParam);
    }
    return Processed;
}
#endif

//----------------------------------------------------------------------------
//                         OK
//----------------------------------------------------------------------------
void CDialogProperties::OnOK() 
{
    if( Accept() )
    {
        DestroyWindow();
    }
}


//----------------------------------------------------------------------------
//                  Read settings from a file.
//----------------------------------------------------------------------------
void CDialogProperties::OnOpen() 
{
    if( Accept() )
    {
        CFileDialog * Dialog = new CFileDialog
        (
            TRUE                             // TRUE for an "open" dialog.
        ,   ".ini"                           // Default extension.
        ,   0                                // Initial file name.
        ,   OFN_EXPLORER                     // Customization flags.
          | OFN_HIDEREADONLY
          | OFN_NOCHANGEDIR
        ,   "Unreal configurations (*.ini)|*.ini|All Files (*.*)|*.*||"
        );
  
        if( Dialog->DoModal() == IDOK )
        {
            CString FileName = Dialog->GetPathName();
            if( FileName.GetLength() != 0 )
            {
                GConfiguration.InterpretValuesFromFile( FConfiguration::NoSection, FileName );
                GConfiguration.WriteValuesToFile();
                StartNewPage(); // Refresh the current page.
            }
        }
        delete Dialog;
    }
}

void CDialogProperties::PostNcDestroy() 
{
    if( this == LatestInstance )
    {
        LatestInstance = 0;
    }
	CDialog::PostNcDestroy();
    delete this;
}

//----------------------------------------------------------------------------
//                         Cancel
//----------------------------------------------------------------------------
void CDialogProperties::OnCancel() 
{
    DestroyWindow();
}

