// InpActDi.h : header file
//
/*
==============================================================================

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Description:
    This is the MFC-generated class for handling the Unreal input
    actions dialog.

Revision history:
    * 06/26/96, Created by Mark
==============================================================================
*/

/////////////////////////////////////////////////////////////////////////////
// CDialogInputActions dialog

#include "Unreal.h"
#include "UnAction.h"
class CDialogInputActions : public CDialog
{
// Construction
public:
	CDialogInputActions(CWnd* pParent = NULL);   // standard constructor
    BOOL OnInitDialog();


    BOOL Accept(); 
        // If input is valid, save the changes and return TRUE. Otherwise,
        // notify the user of errors and return FALSE.
        // If SaveCurrentCombos, take the CurrentCombos and use them to replace
        // all the combos for action CurrentAction.
    BOOL ChangesWereSaved;  // Set after calling Accept(). TRUE if any changes were made and saved.


// Dialog Data
	//{{AFX_DATA(CDialogInputActions)
	enum { IDD = IDD_INPUT_ACTIONS };
	CListBox	AllCombos;
	CButton	ActionHeader;
	CString	ActionDescription;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogInputActions)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

private:
    struct TActionState
    {
        BOOL             IsShown     ; // TRUE if this action is displayed.
        BOOL             WasChanged  ; // User made changes, saving is needed.
        int              Index       ; // If IsShown, the 0-based index in the list where this action starts.
        int              ComboCount  ; // Number of combos in Combos.
        FInput::TCombo * Combos      ; // List of ComboCount input combos.
        TActionState()
        {
            IsShown     = FALSE ;
            ComboCount  = 0     ;
            Combos      = 0     ;
        }
        void Free();
        //todo: These should all (or mostly) be in FAction::TCombos
        BOOL UsesMainInput(FInput::TSwitch Switch) const; // Does one of the combos use Switch for a main input?
        BOOL UsesMainInput(FInput::TMovement Movement) const; // Does one of the combos use Movement for a main input?
        BOOL Uses(FInput::TSwitch Switch) const; // Does one of the combos use Switch for a main input or modifier?
        void DeleteMainInput(FInput::TSwitch Switch); // Remove all combos with Switch as a main input.
        void DeleteMainInput(FInput::TMovement Movement); // Remove all combos with Movement as a main input.
        void Delete(FInput::TSwitch Switch); // Remove all combos which use Switch (as a main switch or modifier).
        void Delete(int N); // Delete the N'th combo, N=0,1,...
    };
    TActionState ActionStates[FAction::ActionCount];
    BOOL ShowingClass[FAction::ClassificationCount]; // TRUE for each category of action to be shown.

    void Free(); // Free all storage.

    BOOL CheckMainInputUsers(FInput::TSwitch Switch, FAction::TAction ExceptFor); 
        // Checks for all combos which use Switch as a main input, and asks the
        // user if he wants it deleted. ExceptFor is not checked.
        // Returns TRUE if a change was made.

    BOOL CheckMainInputUsers(FInput::TMovement Movement, FAction::TAction ExceptFor); 
        // Checks for all combos which use Movement as a main input, and asks the
        // user if he wants it deleted. ExceptFor is not checked.
        // Returns TRUE if a change was made.

    BOOL CheckOppositeOf(FInput::TMovement Movement, FAction::TAction Action);
        // Check to see if there is an opposite action of Action.
        // If there is, check to see if the opposite movement to Movement
        // is used as a main input to the opposite action of Action.
        // If not, ask the user if he wishes to make it so.
        // Returns TRUE if a change was made.



// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogInputActions)
	afx_msg void OnActionHeaderButton();
	afx_msg void OnDblClkCombos();
	afx_msg void OnSelChangeCombos();
	afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
	afx_msg void OnUseDefault();
	afx_msg void OnUseDefaultForAll();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void ArrangeSelectedActions(int TopPosition = 0, int Selection = 0);
        // Clears out any current display of actions and inputs and
        // constructs a display for the currently selection classes of
        // actions (chosen with SelectClass()).
        // This function sets up important structures and should be
        // called every time combos are deleted or added, or when the
        // set of actions to be shown changes.
        // The top item is left positioned at TopPosition (or at the end if
        // TopItem is too large), and the selected item is Selection
        // (or the last item if Selection is too large).

    void ArrangeCurrentCombo();
        // Set up anything needed to reflect the currently selected
        // combo.

    void RefreshCombo( FAction::TAction Action, int WhichCombo ); // Refresh the WhichCombo'th (0,1,...) combo of Action
        // This can be used for an action with no combos.
    void RefreshCurrentCombo();
        // Call this after making changes to the current combo.

    void SelectClass(FAction::TClass Class, BOOL Select = TRUE);
        // Show actions in the specified Class (if Select) or 
        // hide them (if !Select). This doesn't actually refresh the
        // display - you need to call ArrangeSelectedActions to do that.

    int SelectedClassCount() const; // Return the number of action classes selected for display.

    void SelectAllClasses(); // Does SelectClass(X,TRUE) for each class X.
    void SelectNoClasses(); // Does SelectClass(X,FALSE) for each class X.

    DWORD MakeDatum(FAction::TAction Action, int WhichCombo) const;
        // Make a datum to be associated with a list item.
        // The datum contains the action and the combo number, starting at 0.
    void InterpretDatum(DWORD Datum, FAction::TAction & Action, int & WhichCombo) const;
        // Reverses the effect of MakeDatum. Use this to convert a list item
        // in the actions list to its associated action and combo number.
    FAction::TAction CurrentAction() const; // Returns the action of the currently selected combo.
    int CurrentComboIndex() const; // Returns the index (from 0) of the currently selected combo.
    void GetCurrentCombo( FAction::TAction & Action, int & WhichCombo) const;
    void DeleteCurrentCombo(); 
    void ChangeCurrentCombo(int X, int Y);  // Use X and Y as position for any pop-up menus.
    void ChangeCurrentCombo();
    void AddNewCombo();
    void AddNewCombo(FAction::TAction Action, const FInput::TCombo & Combo );
    void EnsureCurrentIsShown(RECT & ItemRegion); // Make sure the current selection is shown in the list, scrolling if needed.
        // ItemRegion, on output, holds the screen area holding the current item.
    BOOL ProcessMenuPopUpCommand(int CommandId);

    typedef enum  // The state of popup-menu processing
    {
        NoOperation             = 0 // Always 0. Indicates no processing.
    ,   AddingComboOperation        // Adding a new input combination to the current action.
    ,   ChangingComboOperation      // Changing the input of the current combo.
    }
    TUserOperation;
    TUserOperation UserOperation;
    int SavedTopPosition;
    int SavedSelection;
    void SavePositions(); // Save positions of top and current selection (into SavedTopPosition and SavedSelection).
    void RestorePositions(); // Restore position from SavedTopPosition and SavedSelection.
    FInput::TSwitch SelectSwitch(); // Prompts the user to select a keyboard key.
};
