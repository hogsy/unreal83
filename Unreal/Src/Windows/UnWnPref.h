// UnWnPref.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogPreferences dialog

#include "UnPrefer.h"
class CDialogPreferences : public CDialog
{
// Construction
public:
	CDialogPreferences(CWnd* pParent = NULL);   // standard constructor
    BOOL OnInitDialog();

    BOOL Accept(); 
        // If input is valid, save the changes and return TRUE. Otherwise,
        // notify the user of errors and return FALSE.

// Dialog Data
	//{{AFX_DATA(CDialogPreferences)
	enum { IDD = IDD_PROPERTIES_PREFERENCES };
	BOOL	SwitchFromEmptyWeapon;
	BOOL	SwitchToNewWeapon;
	BOOL	ViewFollowsIncline;
	BOOL	MovingViewBobs;
	BOOL	StillViewBobs;
	BOOL	WeaponsSway;
	BOOL	ReverseUpAndDown;
	BOOL	MouseLookAlwaysOn;
	BOOL	RunAlwaysOn;
	BOOL	ViewRolls;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogPreferences)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogPreferences)
	afx_msg void OnUseDefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void LoadValuesFrom( const FPreferences & Preferences ); // Load local values.
    void SaveValuesInto( FPreferences & Preferences ); // Save local values.
};
