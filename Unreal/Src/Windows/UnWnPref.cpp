// UnWnPref.cpp : implementation file
//

#include "stdafx.h"
#include "unwn.h"
#include "UnWnPref.h"
#include "UnPrefer.h"
#include "UnConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogPreferences dialog


CDialogPreferences::CDialogPreferences(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogPreferences::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogPreferences)
	SwitchFromEmptyWeapon = FALSE;
	SwitchToNewWeapon = FALSE;
	ViewFollowsIncline = FALSE;
	MovingViewBobs = FALSE;
	StillViewBobs = FALSE;
	WeaponsSway = FALSE;
	ReverseUpAndDown = FALSE;
	MouseLookAlwaysOn = FALSE;
	RunAlwaysOn = FALSE;
	ViewRolls = FALSE;
	//}}AFX_DATA_INIT
}

BOOL CDialogPreferences::OnInitDialog()
{
    CDialog::OnInitDialog();
    LoadValuesFrom( GPreferences );
    return FALSE; // FALSE tells Windows not to set the input focus. 
                  // We do this because the dialog is expected to be part
                  // of a tab group and we don't want to change the focus.
}

void CDialogPreferences::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogPreferences)
	DDX_Check(pDX, IDC_PREFERENCES_DROPEMPTYWEAPONS, SwitchFromEmptyWeapon);
	DDX_Check(pDX, IDC_PREFERENCES_USENEWWEAPONS, SwitchToNewWeapon);
	DDX_Check(pDX, IDC_PREFERENCES_VIEWALONGINCLINES, ViewFollowsIncline);
	DDX_Check(pDX, IDC_PREFERENCES_VIEWBOBSWHILEMOVING, MovingViewBobs);
	DDX_Check(pDX, IDC_PREFERENCES_VIEWBOBSWHILESTILL, StillViewBobs);
	DDX_Check(pDX, IDC_PREFERENCES_WEAPONSSWAY, WeaponsSway);
	DDX_Check(pDX, IDC_PREFERENCES_REVERSEUPANDDOWN, ReverseUpAndDown);
	DDX_Check(pDX, IDC_PREFERENCES_ALWAYSMOUSELOOK, MouseLookAlwaysOn);
	DDX_Check(pDX, IDC_PREFERENCES_ALWAYSRUN, RunAlwaysOn);
	DDX_Check(pDX, IDC_PREFERENCES_VIEWROLLS, ViewRolls);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogPreferences, CDialog)
	//{{AFX_MSG_MAP(CDialogPreferences)
	ON_BN_CLICKED(ID_USE_DEFAULTS, OnUseDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogPreferences message handlers


//----------------------------------------------------------------------------
//                      Check input and accept changes
//----------------------------------------------------------------------------
BOOL CDialogPreferences::Accept() 
{
    BOOL Accepted = TRUE; // We always accept the input (it cannot be wrong).
    UpdateData(TRUE);
    SaveValuesInto( GPreferences );
    return Accepted;
}

//----------------------------------------------------------------------------
//                Load values
//----------------------------------------------------------------------------
void CDialogPreferences::LoadValuesFrom( const FPreferences & Preferences )
{
    SwitchFromEmptyWeapon   = Preferences.SwitchFromEmptyWeapon    ;
    SwitchToNewWeapon       = Preferences.SwitchToNewWeapon        ;
    ViewFollowsIncline      = Preferences.ViewFollowsIncline       ;
    MovingViewBobs          = Preferences.MovingViewBobs           ;
    StillViewBobs           = Preferences.StillViewBobs            ;
    WeaponsSway             = Preferences.WeaponsSway              ;
    ReverseUpAndDown        = Preferences.ReverseUpAndDown         ;
	MouseLookAlwaysOn       = Preferences.MouseLookAlwaysOn        ;
	RunAlwaysOn             = Preferences.RunAlwaysOn              ;
	ViewRolls               = Preferences.ViewRolls                ;
    UpdateData(FALSE);
}    

//----------------------------------------------------------------------------
//                Save values
//----------------------------------------------------------------------------
void CDialogPreferences::SaveValuesInto( FPreferences & Preferences )
{
    Preferences.SwitchFromEmptyWeapon    = SwitchFromEmptyWeapon    ;
    Preferences.SwitchToNewWeapon        = SwitchToNewWeapon        ;
    Preferences.ViewFollowsIncline       = ViewFollowsIncline       ;
    Preferences.MovingViewBobs           = MovingViewBobs           ;
    Preferences.StillViewBobs            = StillViewBobs            ;
    Preferences.WeaponsSway              = WeaponsSway              ;
    Preferences.ReverseUpAndDown         = ReverseUpAndDown         ;
	Preferences.MouseLookAlwaysOn        = MouseLookAlwaysOn        ;
	Preferences.RunAlwaysOn              = RunAlwaysOn              ;
	Preferences.ViewRolls                = ViewRolls                ;
}

//----------------------------------------------------------------------------
//               Restore default settings
//----------------------------------------------------------------------------
void CDialogPreferences::OnUseDefaults() 
{
    // Get the default values:
    FKeyValues::TStringList DefaultValues = FConfiguration::GetSection
    (
        FConfiguration::PreferencesSection
    ,   GApp->FactoryProfileFileName()
    );
    FPreferences Preferences;
    Preferences.SetValues(DefaultValues);
    LoadValuesFrom( Preferences );
    DefaultValues.Free();
}

