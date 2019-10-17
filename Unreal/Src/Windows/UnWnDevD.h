// InpDevDi.h : header file
//
/*
==============================================================================

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Description:
    This is the MFC-generated class for handling the Unreal input
    devices dialog.

Revision history:
    * 06/26/96, Created by Mark
==============================================================================
*/

/////////////////////////////////////////////////////////////////////////////
// CDialogInputDevices dialog

class CDialogInputDevices : public CDialog
{
// Construction
public:
	CDialogInputDevices(CWnd* pParent = NULL);   // standard constructor
    BOOL OnInitDialog();
    BOOL Accept(); 
        // If input is valid, save the changes and return TRUE. Otherwise,
        // notify the user of errors and return FALSE.

// Dialog Data
	//{{AFX_DATA(CDialogInputDevices)
	enum { IDD = IDD_INPUT_DEVICES };
	BOOL	UseJoystick1;
	BOOL	UseJoystick2;
	BOOL	UseMouse;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogInputDevices)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogInputDevices)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
