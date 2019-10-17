// InpSenDi.h : header file
//
/*
==============================================================================

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Description:
    This is the MFC-generated class for handling the Unreal input
    sensitivity dialog.

Revision history:
    * 06/26/96, Created by Mark
==============================================================================
*/

/////////////////////////////////////////////////////////////////////////////
// CDialogInputSensitivity dialog

#include "UnInput.h"

class CDialogInputSensitivity : public CDialog
{
// Construction
public:
	CDialogInputSensitivity(CWnd* pParent = NULL);   // standard constructor
    BOOL OnInitDialog();

    BOOL Accept(); 
        // If input is valid, save the changes and return TRUE. Otherwise,
        // notify the user of errors and return FALSE.
    BOOL ChangesWereSaved; // TRUE if any settings were saved. Set after calling Accept().


// Dialog Data
	//{{AFX_DATA(CDialogInputSensitivity)
	enum { IDD = IDD_INPUT_SENSITIVITY };
	CEdit	DigitalThresholdControl;
	CEdit	AnalogThresholdControl;
	CListBox	MovementList;
	CButton	SeparateButton;
	CButton	MergeButton;
	CSliderCtrl	SpeedSlider;
	CSpinButtonCtrl	DigitalThresholdSpinner;
	CSpinButtonCtrl	AnalogThresholdSpinner;
	UINT	AnalogThreshold;
	UINT	DigitalThreshold;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogInputSensitivity)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogInputSensitivity)
	afx_msg void OnSelchangeInputMovementList();
	afx_msg void OnUseDefaults();
	afx_msg void OnUseDefaultsForAll();
	afx_msg void OnMergeAxes();
	afx_msg void OnSeparateAxes();
	afx_msg void OnChangeThreshold();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    FInput::TMovement CurrentMovement; // The currently selected movement or 0.
    FInput::TMovementInfo MovementInfo[FInput::MovementCount]; // A copy of the movement values being edited.
    BOOL AxesAreMerged; // TRUE if the positive/negative axes have been merged.

    void PrepareSymmetryButtons(); // Prepare Merge/Separate buttons based on AxesAreMerged.

    BOOL AxesAreSymmetrical(); // Return TRUE if axes pairs are the same.

    void ArrangeMovements(BOOL Merged, FInput::TMovement Selection = FInput::M_None);
        // Arrange the movements, setting the appropriate information into
        // the displayed list. Call this initially and after merging or 
        // separating the axes. If Merged, the axes are merged in the display.
        // Selection is where you want the current selection to be.
    void PrepareDialogSettings(); // Load the values for the selected movement into the dialog.
    BOOL RememberDialogSettings(); // Locally save the values currently shown in the dialog.
        // Return TRUE if the data is okay, FALSE otherwise.

    FLOAT GetAnalogThreshold       () const; // What is the displayed analog threshold?
    FLOAT GetDigitalThreshold      () const; // What is the displayed digital threshold?
    FLOAT GetSpeed                 () const; // What is the displayed speed?

    void SetAnalogThreshold       (FLOAT Value); // Set the displayed analog threshold.
    void SetDigitalThreshold      (FLOAT Value); // Set the displayed digital threshold.
    void SetSpeed                 (FLOAT Value); // Set the displayed speed.

    FInput::TMovementInfo & CurrentInfo() { return MovementInfo[CurrentMovement]; }
    FInput::TSensitivity & CurrentSensitivity() { return CurrentInfo().Sensitivity; }

    void LoadValuesFrom( const FInput::TMovementInfo * Info ); // Load values from an array of MovementCount values.
    void SaveValuesInto( FInput::TMovementInfo * Info ); // Save value into an array of MovementCount values.
};
