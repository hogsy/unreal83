// InpActDi.cpp : implementation file
//
/*
==============================================================================

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Description:
    This is the CDialog-based class for handling the Unreal input
    actions dialog.

Revision history:
    * 06/26/96, Created by Mark
==============================================================================
*/

#include "stdafx.h"
#include "unwn.h"
#include "UnWnActD.h"
#include "UnWnAcCD.h"
#include "UnWnKeyD.h"

#include "Unreal.h"
#include "UnAction.h"
#include "UnConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogInputActions dialog


CDialogInputActions::CDialogInputActions(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogInputActions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogInputActions)
	ActionDescription = _T("");
	//}}AFX_DATA_INIT
}

//----------------------------------------------------------------------------
//               Dialog Initialization
//----------------------------------------------------------------------------
BOOL CDialogInputActions::OnInitDialog()
{
    CDialog::OnInitDialog();
    ChangesWereSaved = FALSE;
    // Get the combo info for each action.
    {
        for( int Action_ = 1; Action_ < FAction::ActionCount; Action_++ )
        {
            const FAction::TAction Action = FAction::TAction(Action_);
            TActionState & State = ActionStates[Action];
            State.IsShown = FALSE;
            State.WasChanged = FALSE;
            GAction.GetCombos( Action, State.ComboCount, &State.Combos );
        }
    }
    UserOperation = NoOperation;
    static int TabStops[] = { 65 } ;
    AllCombos.SetTabStops(1, &TabStops[0] );
    SelectAllClasses();
    ArrangeSelectedActions();
    return FALSE; // FALSE tells Windows not to set the input focus. 
                  // We do this because the dialog is expected to be part
                  // of a tab group and we don't want to change the focus.
}


void CDialogInputActions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogInputActions)
	DDX_Control(pDX, IDC_COMBOS, AllCombos);
	DDX_Control(pDX, IDC_ACTION_HEADER_BUTTON, ActionHeader);
	DDX_Text(pDX, IDC_ACTION_DESCRIPTION, ActionDescription);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDialogInputActions, CDialog)
	//{{AFX_MSG_MAP(CDialogInputActions)
	ON_BN_CLICKED(IDC_ACTION_HEADER_BUTTON, OnActionHeaderButton)
	ON_LBN_DBLCLK(IDC_COMBOS, OnDblClkCombos)
	ON_LBN_SELCHANGE(IDC_COMBOS, OnSelChangeCombos)
	ON_WM_VKEYTOITEM()
	ON_BN_CLICKED(IDC_USE_DEFAULT, OnUseDefault)
	ON_BN_CLICKED(IDC_USE_DEFAULT_FOR_ALL, OnUseDefaultForAll)
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogInputActions message handlers

//----------------------------------------------------------------------------
//               Arrange the selected actions for display
//----------------------------------------------------------------------------
void CDialogInputActions::ArrangeSelectedActions(int TopPosition, int Selection)
{
    AllCombos.ResetContent();
    if( SelectedClassCount() == 0 )
    {
        SelectAllClasses();
    }
    // Fill the actions list with the actions:
    {
        for( int Action_ = 1; Action_ < FAction::ActionCount; Action_++ )
        {
            const FAction::TAction Action = FAction::TAction(Action_);
            TActionState & State = ActionStates[Action];
            State.IsShown = ShowingClass[FAction::Class(Action)];
            if( State.IsShown )
            {
                State.Index = AllCombos.GetCount();
                if( State.ComboCount == 0 )
                {
                    const int Index = AllCombos.AddString(""); // Add a placeholder string.
                    RefreshCombo(Action,0); // Fill in the placeholder.
                }
                for( int WhichCombo = 0; WhichCombo < State.ComboCount; ++WhichCombo )
                {
                    const int Index = AllCombos.AddString(""); // Add a placeholder string.
                    RefreshCombo(Action,WhichCombo); // Fill in the placeholder.
                }
            }
        }
    }
    AllCombos.SetTopIndex( TopPosition < 0 ? 0 : TopPosition );
    Selection = OurMin( Selection, AllCombos.GetCount()-1 );
    AllCombos.SetCurSel(Selection < 0 ? 0 : Selection);
    ArrangeCurrentCombo();
}

//----------------------------------------------------------------------------
//       Refresh the specified combo, using the info stored locally 
//----------------------------------------------------------------------------
void CDialogInputActions::RefreshCombo( FAction::TAction Action, int WhichCombo )
{
    TActionState & State = ActionStates[Action];
    if( State.IsShown )
    {
        char Description[200];
        const int Index = State.Index + WhichCombo;
        AllCombos.DeleteString( Index ); // Remove the old string
        char * Text = Description;
        if( State.ComboCount ==0 || WhichCombo == 0 ) // First combo (or no combo):
        {
            Text += sprintf( Text, "%s", FAction::ActionName(Action) ); // Add action name
        }
        if( WhichCombo < State.ComboCount )
        {
            const FInput::TCombo & Combo = State.Combos[WhichCombo];
            Text += sprintf( Text, "\t%s", Combo.MainInputText(TRUE) );
            if( Combo.Modifiers.Has1() )
            {
                Text += sprintf( Text, "     +    [%s]", Combo.Modifiers.MetaSwitch1.Text(TRUE) );
            }
        }
        const int NewIndex = AllCombos.InsertString(Index,Description);
        const DWORD Datum = MakeDatum(Action,WhichCombo);
        AllCombos.SetItemData( Index, Datum );
    }
}

//----------------------------------------------------------------------------
//       Refresh the current combo with local information
//----------------------------------------------------------------------------
void CDialogInputActions::RefreshCurrentCombo()
{
    FAction::TAction Action;
    int WhichCombo = 0;
    GetCurrentCombo(Action,WhichCombo);
    if( Action !=  0 )
    {
        RefreshCombo(Action,WhichCombo);
    }
}

//----------------------------------------------------------------------------
//               Arrange for the currently selected combo
//----------------------------------------------------------------------------
void CDialogInputActions::ArrangeCurrentCombo()
{
    // Change the action description to reflect the combo:
    const FAction::TAction CurrentAction = this->CurrentAction();
    ActionDescription = FAction::ActionName( CurrentAction );
    ActionDescription += ": ";
    ActionDescription += FAction::ActionHelp( CurrentAction );
    UpdateData(FALSE);
}

//----------------------------------------------------------------------------
//               Select a class of actions to be shown or hidden
//----------------------------------------------------------------------------
void CDialogInputActions::SelectClass(FAction::TClass Class, BOOL Select)
{
    ShowingClass[Class] = Select;
}

//----------------------------------------------------------------------------
//                       Show all classes of actions
//----------------------------------------------------------------------------
// Actually, we always hide the unclassified actions.
void CDialogInputActions::SelectAllClasses()
{
    for( int Class_ = 1; Class_ < FAction::ClassificationCount; Class_++ )
    {
        const FAction::TClass Class = FAction::TClass(Class_);
        SelectClass(Class);
    }    
    ShowingClass[FAction::NoClassification] = FALSE;
}

//----------------------------------------------------------------------------
//                       Number of classes selected
//----------------------------------------------------------------------------
int CDialogInputActions::SelectedClassCount() const
{
    int Count = 0;
    for( int Class_ = 1; Class_ < FAction::ClassificationCount; Class_++ )
    {
        const FAction::TClass Class = FAction::TClass(Class_);
        if( ShowingClass[Class] )
        {
            Count++;
        }
    }    
    return Count;
}    
//----------------------------------------------------------------------------
//                       Show no classes of actions
//----------------------------------------------------------------------------
void CDialogInputActions::SelectNoClasses()
{
    for( int Class_ = 1; Class_ < FAction::ClassificationCount; Class_++ )
    {
        const FAction::TClass Class = FAction::TClass(Class_);
        SelectClass(Class,FALSE);
    }    
}

//----------------------------------------------------------------------------
//                      Check input and accept changes
//----------------------------------------------------------------------------
BOOL CDialogInputActions::Accept() 
{
    BOOL Accepted = TRUE; // We always accept the input (it cannot be wrong).
    for( int Action_ = 1; Action_ < FAction::ActionCount; Action_++ )
    {
        const FAction::TAction Action = FAction::TAction(Action_);
        TActionState & State = ActionStates[Action];
        if( State.WasChanged )
        {
            GAction.Empty(Action); // Remove current definitions for action.
            GAction.Add( Action, State.ComboCount, State.Combos );
            ChangesWereSaved = TRUE;                
        }
    }
    return Accepted;
}


void CDialogInputActions::PostNcDestroy() 
{
    Free();
    CDialog::PostNcDestroy();
    delete this;
}

//----------------------------------------------------------------------------
//               Free storage allocated by TActionState
//----------------------------------------------------------------------------
void CDialogInputActions::TActionState::Free() 
{
    if( Combos != 0 )
    {
        appFree( Combos );
        Combos = 0;
    }
    ComboCount = 0;
}

//----------------------------------------------------------------------------
//               Free storage allocated by the dialog
//----------------------------------------------------------------------------
void CDialogInputActions::Free() 
{
    for( int Action_ = 1; Action_ < FAction::ActionCount; Action_++ )
    {
        const FAction::TAction Action = FAction::TAction(Action_);
        TActionState & State = ActionStates[Action];
        State.Free();
    }
}

//----------------------------------------------------------------------------
//             The action of the currently selected combo.
//----------------------------------------------------------------------------
void CDialogInputActions::GetCurrentCombo( FAction::TAction & Action, int & WhichCombo) const

{
    Action = FAction::NoAction;
    WhichCombo = 0;
    const int Selection = AllCombos.GetCurSel();
    if( Selection != LB_ERR )
    {
        const DWORD Datum = AllCombos.GetItemData( AllCombos.GetCurSel() );
        InterpretDatum( Datum, Action, WhichCombo );
    }
}

//----------------------------------------------------------------------------
//             The action of the currently selected combo.
//----------------------------------------------------------------------------
FAction::TAction CDialogInputActions::CurrentAction() const
{
    FAction::TAction Action = FAction::NoAction;
    int Index = 0;
    GetCurrentCombo(Action,Index);
    return Action;
}

//----------------------------------------------------------------------------
//             The combo number for the currently selected combo
//----------------------------------------------------------------------------
int CDialogInputActions::CurrentComboIndex() const
{
    FAction::TAction Action = FAction::NoAction;
    int Index = 0;
    GetCurrentCombo(Action,Index);
    return Index;
}

//----------------------------------------------------------------------------
//             Make a list datum for an item in the action list:
//----------------------------------------------------------------------------
DWORD CDialogInputActions::MakeDatum(FAction::TAction Action, int WhichCombo) const
{
    WORD Part1 = WORD(Action);
    WORD Part2 = WORD(WhichCombo);
    const DWORD Datum = ( DWORD(Part1)<<16 ) | DWORD(Part2);
    return Datum;
}

//----------------------------------------------------------------------------
//             Save current position and selection
//----------------------------------------------------------------------------
void CDialogInputActions::SavePositions()
{
    SavedTopPosition = AllCombos.GetTopIndex();
    SavedSelection = AllCombos.GetCurSel();
}

//----------------------------------------------------------------------------
//             Restore slection and top position.
//----------------------------------------------------------------------------
void CDialogInputActions::RestorePositions()
{
    AllCombos.SetTopIndex( SavedTopPosition < 0 ? 0 : SavedTopPosition );
    int Selection = OurMin( SavedSelection, AllCombos.GetCount()-1 );
    AllCombos.SetCurSel(Selection < 0 ? 0 : Selection);
}

//----------------------------------------------------------------------------
//             Decompose a list datum for an item in the action list:
//----------------------------------------------------------------------------
void CDialogInputActions::InterpretDatum(DWORD Datum, FAction::TAction & Action, int & WhichCombo) const
{
    WORD Part1 = WORD(Datum>>16);
    WORD Part2 = WORD(Datum & 0xffff);
    Action = FAction::TAction(Part1);
    WhichCombo = int(Part2);
}

//----------------------------------------------------------------------------
//                  Delete the currently selected combo
//----------------------------------------------------------------------------
void CDialogInputActions::DeleteCurrentCombo()
{
    FAction::TAction Action;
    int WhichCombo;
    GetCurrentCombo(Action,WhichCombo);
    if( Action != 0 )
    {
        TActionState & State = ActionStates[Action];
        if( State.ComboCount > 0 )
        {
            SavePositions();
            // Delete the combo by shifting later entries into its position,
            // unless the combo being deleted is the last one.
            const int ShiftCount = State.ComboCount - WhichCombo - 1;
            if( ShiftCount > 0 )
            {
                memmove( &State.Combos[WhichCombo], &State.Combos[WhichCombo+1], sizeof(State.Combos[0])*ShiftCount );
            }
            State.ComboCount--;
            SavePositions();
            ArrangeSelectedActions();
            RestorePositions();
            State.WasChanged = TRUE;
        }
    }
}

static UINT CheckFlag(BOOL Value) // Return MF_... flag appropriate for a boolean value (TRUE==>Checked)
{
    return Value ? MF_CHECKED : MF_UNCHECKED;
}

//----------------------------------------------------------------------------
//                  Let the user change the currently selected combo
//----------------------------------------------------------------------------
void CDialogInputActions::ChangeCurrentCombo(int X, int Y)
{
    FAction::TAction Action;
    int WhichCombo;
    GetCurrentCombo(Action,WhichCombo);
    if( Action != 0 )
    {
        TActionState & State = ActionStates[Action];
        if( State.ComboCount == 0 )
        {
            // There are no combos, so add a new one instead...
            AddNewCombo();
        }
        else
        {
            // *** WARNING ***
            // This code assumes the following structure in IDR_COMBO_POPUP:
            //    Top
            //    [0]  Change             (identified by position)  
            //           [?] Always on    (identified by command)
            //    [1]  Advanced           (identified by position)  
            //           [0]Double        (identified by command)
            //           [1]Toggle        (identified by command)
            //           [2]MovementKind  (identified by position)
            //           [3]Modifier      (identified by position) 
            //                 ...
            //                 Double     (identified by command) 
            //                 Toggle     (identified by command) 
            //                 Delete     (identified by position) 
            //                 ...
            const FInput::TCombo & Combo = State.Combos[WhichCombo];
            const FInput::TMetaSwitch & Modifier1 = Combo.Modifiers.MetaSwitch1;
            CMenu Menu;
            Menu.LoadMenu(IDR_COMBO_POPUP);
            CMenu * PopUp           = Menu.GetSubMenu(0);
            CMenu * ChangeMenu      = PopUp->GetSubMenu(0);
            CMenu * AdvancedMenu    = PopUp->GetSubMenu(1); 
            CMenu * Modifier1Menu   = AdvancedMenu->GetSubMenu(3); 
            if( Combo.IsSwitch() )
            {
                AdvancedMenu->CheckMenuItem( ID_ACI_DOUBLE, MF_BYCOMMAND | CheckFlag(Combo.MetaSwitch.IsDouble) );
                AdvancedMenu->CheckMenuItem( ID_ACI_TOGGLE, MF_BYCOMMAND | CheckFlag(Combo.MetaSwitch.IsToggle) );
                AdvancedMenu->EnableMenuItem( 2, MF_BYPOSITION | MF_GRAYED );
            }
            else
            {
                AdvancedMenu->EnableMenuItem( ID_ACI_DOUBLE, MF_BYCOMMAND | MF_GRAYED );
                AdvancedMenu->EnableMenuItem( ID_ACI_TOGGLE, MF_BYCOMMAND | MF_GRAYED );
            }
            if( Combo.Modifiers.Has1() )
            {
                Modifier1Menu->CheckMenuItem( ID_ACM1_DOUBLE, MF_BYCOMMAND | CheckFlag(Modifier1.IsDouble) );
                Modifier1Menu->CheckMenuItem( ID_ACM1_TOGGLE, MF_BYCOMMAND | CheckFlag(Modifier1.IsToggle) );
            }
            else
            {
                // There are no modifiers - disable some modifer 1 items:
                Modifier1Menu->EnableMenuItem( ID_ACM1_DOUBLE, MF_BYCOMMAND | MF_GRAYED );
                Modifier1Menu->EnableMenuItem( ID_ACM1_TOGGLE, MF_BYCOMMAND | MF_GRAYED );
                Modifier1Menu->EnableMenuItem( ID_ACM1_DELETE, MF_BYCOMMAND | MF_GRAYED );
                AdvancedMenu->EnableMenuItem( 4, MF_BYPOSITION | MF_GRAYED );
            }
            UserOperation = ChangingComboOperation;
            if( PopUp != 0 )
            {
                PopUp->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, X, Y, this);
            }
        }
    }
}

//----------------------------------------------------------------------------
// Make sure the current selection is shown in the list, scrolling if needed.
//----------------------------------------------------------------------------
void CDialogInputActions::EnsureCurrentIsShown(RECT & ItemRegion)
{
    RECT ClientRegion;
    AllCombos.GetClientRect(&ClientRegion);
    const int Selection = AllCombos.GetCurSel();
    AllCombos.GetItemRect( Selection, &ItemRegion );
    // Make sure the selected item is visible in the client area:
    if( ItemRegion.top < 0 || ItemRegion.bottom > ClientRegion.bottom )
    {
        AllCombos.SetTopIndex(Selection);
        AllCombos.GetItemRect( Selection, &ItemRegion );
    }
    AllCombos.ClientToScreen( &ItemRegion );
}

//----------------------------------------------------------------------------
//                  Let the user change the currently selected combo
//----------------------------------------------------------------------------
// Uses the position of the item to place the pop-up menu.
// Scrolls the list if necessary to bring the selection into view.
void CDialogInputActions::ChangeCurrentCombo()
{
    RECT ItemRegion;
    EnsureCurrentIsShown(ItemRegion);
    ChangeCurrentCombo(ItemRegion.left, ItemRegion.top+10); // + a little so we don't cover the selection
}

//----------------------------------------------------------------------------
//                  Let the user add a new combo to the current action
//----------------------------------------------------------------------------
// Uses the position of the item to place the pop-up menu.
// Scrolls the list if necessary to bring the selection into view.
void CDialogInputActions::AddNewCombo()
{
    RECT ItemRegion;
    EnsureCurrentIsShown(ItemRegion);

    FAction::TAction Action;
    int WhichCombo;
    GetCurrentCombo(Action,WhichCombo);
    if( Action != 0 )
    {
        TActionState & State = ActionStates[Action];
        CMenu Menu;
        Menu.LoadMenu(IDR_COMBO_POPUP);
        // Get the change pop-up, which is the first submenu of the menu:
        // *** WARNING ***
        // This code assumes the following structure in IDR_COMBO_POPUP:
        //    Top
        //      Change  ->   
        //                 Intro
        //                 ...
        //                 Always on     (identified by command)
        //      ...
        CMenu * PopUp = Menu.GetSubMenu(0);
        CMenu * ChangePopUp = PopUp == 0 ? 0 : PopUp->GetSubMenu(0);
        if( ChangePopUp != 0 )
        {
            UserOperation = AddingComboOperation;
            char Intro[30];
            sprintf( Intro, "New input for %s", FAction::ActionName(Action) );
            ChangePopUp->ModifyMenu(0,MF_BYPOSITION|MF_STRING, 0, Intro );
            ChangePopUp->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ItemRegion.left, ItemRegion.top, this);
        }
    }
}

//----------------------------------------------------------------------------
//               The action header button was clicked
//----------------------------------------------------------------------------
void CDialogInputActions::OnActionHeaderButton() 
{
    CDialogSelectActionClasses * Dialog = new CDialogSelectActionClasses;
    Dialog->SelectAdministrativeActions = ShowingClass[FAction::AdministrativeAction];
    Dialog->SelectAdvancedActions       = ShowingClass[FAction::AdvancedAction];
    Dialog->SelectMiscellaneousActions  = ShowingClass[FAction::MiscellaneousAction];
    Dialog->SelectMoveActions           = ShowingClass[FAction::MovingAction];
    Dialog->SelectTurnActions           = ShowingClass[FAction::TurningAction];
    Dialog->SelectWeaponActions         = ShowingClass[FAction::WeaponAction];
    if( Dialog->DoModal() == IDOK )
    {
        if( Dialog->ShowAllActions )
        {
            SelectAllClasses();
        }
        else
        {
            SelectClass( FAction::AdministrativeAction  , Dialog->SelectAdministrativeActions );
            SelectClass( FAction::AdvancedAction        , Dialog->SelectAdvancedActions       );
            SelectClass( FAction::MiscellaneousAction   , Dialog->SelectMiscellaneousActions  );
            SelectClass( FAction::MovingAction          , Dialog->SelectMoveActions           );
            SelectClass( FAction::TurningAction         , Dialog->SelectTurnActions           );
            SelectClass( FAction::WeaponAction          , Dialog->SelectWeaponActions         );
        }
        ArrangeSelectedActions();
    }
    delete Dialog;
}


//----------------------------------------------------------------------------
//                  A combo was double-clicked
//----------------------------------------------------------------------------
void CDialogInputActions::OnDblClkCombos() 
{
    POINT Point;
    GetCursorPos(&Point);
    ChangeCurrentCombo(Point.x,Point.y+10); // .y+ a little so we can still see the selected item
}

//----------------------------------------------------------------------------
//                  The currently selected combo has changed
//----------------------------------------------------------------------------
void CDialogInputActions::OnSelChangeCombos() 
{
    ArrangeCurrentCombo();
}

//----------------------------------------------------------------------------
//                  A key was pressed in the current combo
//----------------------------------------------------------------------------
int CDialogInputActions::OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex) 
{
    BOOL Processed = FALSE;
    if( pListBox == &AllCombos )
    {
        switch( nKey )
        {
            case VK_INSERT:
            {
                AddNewCombo();
                Processed = TRUE;
                break;
            }
            case VK_DELETE:
            {
                DeleteCurrentCombo();
                Processed = TRUE;
                break;
            }
            case VK_SPACE:
            case VK_RETURN:
            {
                ChangeCurrentCombo();
                Processed = TRUE;
                break;
            }
        }
    }
    return Processed ? -2 : CDialog::OnVKeyToItem(nKey, pListBox, nIndex);
}

//----------------------------------------------------------------------------
//             
//----------------------------------------------------------------------------
void CDialogInputActions::AddNewCombo(FAction::TAction Action, const FInput::TCombo & Combo )
{
    if( Action != 0 )
    {
        TActionState & State = ActionStates[Action];
        // Add the new combo to the end of the list of current combos.
        // Make a new larger list and free the old one.
        const int NewCount = State.ComboCount + 1;
        FInput::TCombo * NewCombos = (FInput::TCombo *)appMallocArray(NewCount, FInput::TCombo, "Actions dialog combo list" );
        memmove( NewCombos, State.Combos, State.ComboCount*sizeof(NewCombos[0]) );
        if( State.Combos != 0 )
        {
            appFree(State.Combos);
        }
        State.Combos = NewCombos;
        State.ComboCount = NewCount;
        State.Combos[State.ComboCount-1] = Combo;
        SavePositions();
        ArrangeSelectedActions();
        RestorePositions();
        State.WasChanged = TRUE;
    }
}

//----------------------------------------------------------------------------
//                  Prompt for a switch
//----------------------------------------------------------------------------
// Present a dialog to get a key, validate the key (some keys are not allowed).
// Returns the switch identifying the key, or 0 if no valid key was entered.
// We only let the user select keyboard keys with this dialog, but we
// could extend this to include mouse buttons and joystick buttons.
FInput::TSwitch CDialogInputActions::SelectSwitch()
{
    FInput::TSwitch Switch = FInput::S_None;
    CDialogKeySelect * Dialog = new CDialogKeySelect;
    BOOL IsAllowed[256];
    for( int VirtualKey = 0; VirtualKey <= 255; VirtualKey++ )
    {
        IsAllowed[VirtualKey] = FInput::WindowsKeySwitches[VirtualKey] != 0;
    }
    Dialog->IsAllowed = IsAllowed;
    if( Dialog->DoModal() == IDOK && Dialog->SelectedKey != 0 )
    {
        //debugf( LOG_Info, "Selected vkey=%i", Dialog->SelectedKey );
        Switch = FInput::WindowsKeySwitches[Dialog->SelectedKey];
    }
    return Switch;
}

//----------------------------------------------------------------------------
//                  Process a command from the combo pop-up menus
//----------------------------------------------------------------------------
BOOL CDialogInputActions::ProcessMenuPopUpCommand(int CommandId)
{
    BOOL Processed = TRUE; // Set FALSE if we don't process the command.
    FInput::TCombo Combo; // We store most of the changes here, temporarily.
    Combo.Empty(); 
    FInput::TSwitch NewMainSwitch = FInput::S_None; // We store the new main input switch here, if appropriate.
    FInput::TMovement NewMainMovement = FInput::M_None; // We store the new main movement here, if appropriate.
    FAction::TAction Action;
    int WhichCombo;
    GetCurrentCombo(Action,WhichCombo);
    TActionState & State = ActionStates[Action];
    const BOOL Adding = UserOperation == AddingComboOperation;
    const BOOL Changing = UserOperation == ChangingComboOperation;
    BOOL Deleting = FALSE; // TRUE if the combo is being deleted.
    BOOL Canceled = FALSE; // TRUE if the change, addition, or deletion is canceled.
    if( Action == 0 || (!Adding && !Changing) )
    {
        return FALSE; // No operation in progress. <=== Unstructured exit
    }
    FInput::TCombo * OldCombo = Changing ? &State.Combos[WhichCombo] : 0;
    if( OldCombo != 0 )
    {
        // We are changing an existing combo, fill in the old information.
        Combo = *OldCombo;
    }
    FInput::TMetaSwitch & Modifier1 = Combo.Modifiers.MetaSwitch1;

    switch(CommandId)
    {
        // Abbreviations:
        //   _ACI_  Action combo input (for the main input)
        //   _ACM1_ Action combo modifier 1
        case ID_ACI_KEY             : 
        { 
            NewMainSwitch = SelectSwitch(); 
            Canceled = NewMainSwitch==0;
            break; 
        }
        case ID_ACM1_KEY            : 
        { 
            Modifier1.Switch = SelectSwitch(); 
            Canceled = Modifier1.Switch==0;
            break; 
        }
        case ID_ACI_MBLEFT          : { NewMainSwitch       = FInput::S_LeftMouse           ; break; }
        case ID_ACI_MBMIDDLE        : { NewMainSwitch       = FInput::S_MiddleMouse         ; break; }
        case ID_ACI_MBRIGHT         : { NewMainSwitch       = FInput::S_RightMouse          ; break; }

        case ID_ACI_MLEFT           : { NewMainMovement     = FInput::M_MouseL              ; break; }
        case ID_ACI_MRIGHT          : { NewMainMovement     = FInput::M_MouseR              ; break; }
        case ID_ACI_MFORWARD        : { NewMainMovement     = FInput::M_MouseF              ; break; }
        case ID_ACI_MBACKWARD       : { NewMainMovement     = FInput::M_MouseB              ; break; }

        case ID_ACI_J1B1            : { NewMainSwitch       = FInput::S_J1B1    ; break; }
        case ID_ACI_J1B2            : { NewMainSwitch       = FInput::S_J1B2    ; break; }
        case ID_ACI_J1B3            : { NewMainSwitch       = FInput::S_J1B3    ; break; }
        case ID_ACI_J1B4            : { NewMainSwitch       = FInput::S_J1B4    ; break; }
        case ID_ACI_J1B5            : { NewMainSwitch       = FInput::S_J1B5    ; break; }
        case ID_ACI_J1B6            : { NewMainSwitch       = FInput::S_J1B6    ; break; }
        case ID_ACI_J1B7            : { NewMainSwitch       = FInput::S_J1B7    ; break; }
        case ID_ACI_J1B8            : { NewMainSwitch       = FInput::S_J1B8    ; break; }
        case ID_ACI_J1B9            : { NewMainSwitch       = FInput::S_J1B9    ; break; }
        case ID_ACI_J1B10           : { NewMainSwitch       = FInput::S_J1B10   ; break; }
        case ID_ACI_J1HATON         : { NewMainSwitch       = FInput::S_J1HatOn      ; break; }
        case ID_ACI_J1HATN          : { NewMainSwitch       = FInput::S_J1HatN       ; break; }
        case ID_ACI_J1HATS          : { NewMainSwitch       = FInput::S_J1HatS       ; break; }
        case ID_ACI_J1HATW          : { NewMainSwitch       = FInput::S_J1HatW       ; break; }
        case ID_ACI_J1HATE          : { NewMainSwitch       = FInput::S_J1HatE       ; break; }
        case ID_ACI_J1HATNW         : { NewMainSwitch       = FInput::S_J1HatNW      ; break; }
        case ID_ACI_J1HATNE         : { NewMainSwitch       = FInput::S_J1HatNE      ; break; }
        case ID_ACI_J1HATSW         : { NewMainSwitch       = FInput::S_J1HatSW      ; break; }
        case ID_ACI_J1HATSE         : { NewMainSwitch       = FInput::S_J1HatSE      ; break; }

        case ID_ACI_J1XP            : { NewMainMovement     = FInput::M_J1XP     ; break; }
        case ID_ACI_J1XN            : { NewMainMovement     = FInput::M_J1XN     ; break; }
        case ID_ACI_J1YP            : { NewMainMovement     = FInput::M_J1YP     ; break; }
        case ID_ACI_J1YN            : { NewMainMovement     = FInput::M_J1YN     ; break; }
        case ID_ACI_J1ZP            : { NewMainMovement     = FInput::M_J1ZP     ; break; }
        case ID_ACI_J1ZN            : { NewMainMovement     = FInput::M_J1ZN     ; break; }
        case ID_ACI_J1RP            : { NewMainMovement     = FInput::M_J1RP     ; break; }
        case ID_ACI_J1RN            : { NewMainMovement     = FInput::M_J1RN     ; break; }

        case ID_ACI_J2B1            : { NewMainSwitch       = FInput::S_J2B1    ; break; }
        case ID_ACI_J2B2            : { NewMainSwitch       = FInput::S_J2B2    ; break; }
        case ID_ACI_J2B3            : { NewMainSwitch       = FInput::S_J2B3    ; break; }
        case ID_ACI_J2B4            : { NewMainSwitch       = FInput::S_J2B4    ; break; }
        case ID_ACI_J2B5            : { NewMainSwitch       = FInput::S_J2B5    ; break; }
        case ID_ACI_J2B6            : { NewMainSwitch       = FInput::S_J2B6    ; break; }
        case ID_ACI_J2B7            : { NewMainSwitch       = FInput::S_J2B7    ; break; }
        case ID_ACI_J2B8            : { NewMainSwitch       = FInput::S_J2B8    ; break; }
        case ID_ACI_J2B9            : { NewMainSwitch       = FInput::S_J2B9    ; break; }
        case ID_ACI_J2B10           : { NewMainSwitch       = FInput::S_J2B10   ; break; }

        case ID_ACI_J2HATON         : { NewMainSwitch       = FInput::S_J2HatOn      ; break; }
        case ID_ACI_J2HATN          : { NewMainSwitch       = FInput::S_J2HatN       ; break; }
        case ID_ACI_J2HATS          : { NewMainSwitch       = FInput::S_J2HatS       ; break; }
        case ID_ACI_J2HATW          : { NewMainSwitch       = FInput::S_J2HatW       ; break; }
        case ID_ACI_J2HATE          : { NewMainSwitch       = FInput::S_J2HatE       ; break; }
        case ID_ACI_J2HATNW         : { NewMainSwitch       = FInput::S_J2HatNW      ; break; }
        case ID_ACI_J2HATNE         : { NewMainSwitch       = FInput::S_J2HatNE      ; break; }
        case ID_ACI_J2HATSW         : { NewMainSwitch       = FInput::S_J2HatSW      ; break; }
        case ID_ACI_J2HATSE         : { NewMainSwitch       = FInput::S_J2HatSE      ; break; }

        case ID_ACI_J2XP            : { NewMainMovement     = FInput::M_J2XP     ; break; }
        case ID_ACI_J2XN            : { NewMainMovement     = FInput::M_J2XN     ; break; }
        case ID_ACI_J2YP            : { NewMainMovement     = FInput::M_J2YP     ; break; }
        case ID_ACI_J2YN            : { NewMainMovement     = FInput::M_J2YN     ; break; }
        case ID_ACI_J2ZP            : { NewMainMovement     = FInput::M_J2ZP     ; break; }
        case ID_ACI_J2ZN            : { NewMainMovement     = FInput::M_J2ZN     ; break; }
        case ID_ACI_J2RP            : { NewMainMovement     = FInput::M_J2RP     ; break; }
        case ID_ACI_J2RN            : { NewMainMovement     = FInput::M_J2RN     ; break; }

        case ID_ACI_ANALOG          : { Combo.MetaMovement.Kind = FInput::AnalogMovementKind                ; break; }
        case ID_ACI_DIFFERENTIAL    : { Combo.MetaMovement.Kind = FInput::DifferentialMovementKind          ; break; }
        case ID_ACI_DIGITAL         : { Combo.MetaMovement.Kind = FInput::DigitalMovementKind               ; break; }

        case ID_ACI_DELETE          : { Deleting = TRUE;                                    ; break; }
        case ID_ACI_DOUBLE          : { Combo.MetaSwitch.IsDouble = !Combo.MetaSwitch.IsDouble; break; }
        case ID_ACI_TOGGLE          : { Combo.MetaSwitch.IsToggle = !Combo.MetaSwitch.IsToggle; break; }

        case ID_ACM1_MBLEFT         : { Modifier1.Switch    = FInput::S_LeftMouse           ; break; }
        case ID_ACM1_MBMIDDLE       : { Modifier1.Switch    = FInput::S_MiddleMouse         ; break; }
        case ID_ACM1_MBRIGHT        : { Modifier1.Switch    = FInput::S_RightMouse          ; break; }

        case ID_ACM1_J1B1           : { Modifier1.Switch    = FInput::S_J1B1    ; break; }
        case ID_ACM1_J1B2           : { Modifier1.Switch    = FInput::S_J1B2    ; break; }
        case ID_ACM1_J1B3           : { Modifier1.Switch    = FInput::S_J1B3    ; break; }
        case ID_ACM1_J1B4           : { Modifier1.Switch    = FInput::S_J1B4    ; break; }
        case ID_ACM1_J1B5           : { Modifier1.Switch    = FInput::S_J1B5    ; break; }
        case ID_ACM1_J1B6           : { Modifier1.Switch    = FInput::S_J1B6    ; break; }
        case ID_ACM1_J1B7           : { Modifier1.Switch    = FInput::S_J1B7    ; break; }
        case ID_ACM1_J1B8           : { Modifier1.Switch    = FInput::S_J1B8    ; break; }
        case ID_ACM1_J1B9           : { Modifier1.Switch    = FInput::S_J1B9    ; break; }
        case ID_ACM1_J1B10          : { Modifier1.Switch    = FInput::S_J1B10   ; break; }
        case ID_ACM1_J1HATON        : { Modifier1.Switch    = FInput::S_J1HatOn      ; break; }
        case ID_ACM1_J1HATN         : { Modifier1.Switch    = FInput::S_J1HatN       ; break; }
        case ID_ACM1_J1HATS         : { Modifier1.Switch    = FInput::S_J1HatS       ; break; }
        case ID_ACM1_J1HATW         : { Modifier1.Switch    = FInput::S_J1HatW       ; break; }
        case ID_ACM1_J1HATE         : { Modifier1.Switch    = FInput::S_J1HatE       ; break; }
        case ID_ACM1_J1HATNW        : { Modifier1.Switch    = FInput::S_J1HatNW      ; break; }
        case ID_ACM1_J1HATNE        : { Modifier1.Switch    = FInput::S_J1HatNE      ; break; }
        case ID_ACM1_J1HATSW        : { Modifier1.Switch    = FInput::S_J1HatSW      ; break; }
        case ID_ACM1_J1HATSE        : { Modifier1.Switch    = FInput::S_J1HatSE      ; break; }

        case ID_ACM1_J2B1           : { Modifier1.Switch    = FInput::S_J2B1    ; break; }
        case ID_ACM1_J2B2           : { Modifier1.Switch    = FInput::S_J2B2    ; break; }
        case ID_ACM1_J2B3           : { Modifier1.Switch    = FInput::S_J2B3    ; break; }
        case ID_ACM1_J2B4           : { Modifier1.Switch    = FInput::S_J2B4    ; break; }
        case ID_ACM1_J2B5           : { Modifier1.Switch    = FInput::S_J2B5    ; break; }
        case ID_ACM1_J2B6           : { Modifier1.Switch    = FInput::S_J2B6    ; break; }
        case ID_ACM1_J2B7           : { Modifier1.Switch    = FInput::S_J2B7    ; break; }
        case ID_ACM1_J2B8           : { Modifier1.Switch    = FInput::S_J2B8    ; break; }
        case ID_ACM1_J2B9           : { Modifier1.Switch    = FInput::S_J2B9    ; break; }
        case ID_ACM1_J2B10          : { Modifier1.Switch    = FInput::S_J2B10   ; break; }
        case ID_ACM1_J2HATON        : { Modifier1.Switch    = FInput::S_J2HatOn      ; break; }
        case ID_ACM1_J2HATN         : { Modifier1.Switch    = FInput::S_J2HatN       ; break; }
        case ID_ACM1_J2HATS         : { Modifier1.Switch    = FInput::S_J2HatS       ; break; }
        case ID_ACM1_J2HATW         : { Modifier1.Switch    = FInput::S_J2HatW       ; break; }
        case ID_ACM1_J2HATE         : { Modifier1.Switch    = FInput::S_J2HatE       ; break; }
        case ID_ACM1_J2HATNW        : { Modifier1.Switch    = FInput::S_J2HatNW      ; break; }
        case ID_ACM1_J2HATNE        : { Modifier1.Switch    = FInput::S_J2HatNE      ; break; }
        case ID_ACM1_J2HATSW        : { Modifier1.Switch    = FInput::S_J2HatSW      ; break; }
        case ID_ACM1_J2HATSE        : { Modifier1.Switch    = FInput::S_J2HatSE      ; break; }

        case ID_ACM1_DELETE         : { Modifier1.Empty();                                  ; break; }
        case ID_ACM1_DOUBLE         : { Modifier1.IsDouble  = !Modifier1.IsDouble           ; break; }
        case ID_ACM1_TOGGLE         : { Modifier1.IsToggle  = !Modifier1.IsToggle           ; break; }

        default:
        {
            Processed = FALSE;
            break;
        }
    }
    if( Processed && !Canceled )
    {
        if( Deleting )
        {
            // Delete the combo:
            DeleteCurrentCombo();
            State.WasChanged = TRUE;
        }
        else if( Changing )
        {
            // Special handling when the main input was changed:
            if( NewMainSwitch != 0 )
            {
                if( OldCombo->IsMovement() || OldCombo->Switch() != NewMainSwitch )
                {
                    // Changed from a movement to a switch or a switch to a different switch.
                    Combo.MetaSwitch.IsDouble  = FALSE;
                    Combo.MetaSwitch.IsToggle  = FALSE;
                    Combo.MetaSwitch.Switch    = NewMainSwitch;
                    Combo.IsASwitch            = TRUE;
                }
            }
            if( NewMainMovement != 0 )
            {
                if( OldCombo->IsSwitch() || OldCombo->Movement() != NewMainMovement )
                {
                    // Changed from a switch to a movement or a movement to a different movement.
                    Combo.MetaMovement.Movement = NewMainMovement;
                    Combo.MetaMovement.Kind     = FInput::DefaultMovementKind(NewMainMovement);
                    Combo.IsASwitch             = FALSE;
                }
            }
            // After a change, normalize the combo.
            Combo.Normalize();

            // All changes are done ... now replace the old combo with the new one:
            *OldCombo = Combo;
            SavePositions();
            RefreshCurrentCombo();
            RestorePositions();
            State.WasChanged = TRUE;
        }
        else if( Adding && NewMainSwitch != 0 )
        {
            Combo.MetaSwitch.IsDouble  = FALSE;
            Combo.MetaSwitch.IsToggle  = FALSE;
            Combo.MetaSwitch.Switch    = NewMainSwitch;
            Combo.IsASwitch            = TRUE;
            Combo.Normalize();
            AddNewCombo( Action, Combo );
        }
        else if( Adding && NewMainMovement != 0 )
        {
            Combo.MetaMovement.Movement = NewMainMovement;
            Combo.MetaMovement.Kind     = FInput::DefaultMovementKind(NewMainMovement);
            Combo.IsASwitch             = FALSE;
            Combo.Normalize();
            AddNewCombo( Action, Combo );
        }
        if( (Changing||Adding) && !Deleting )
        {
            BOOL WasChanged = FALSE;
            if( NewMainMovement != 0 )
            {
                if( CheckMainInputUsers(NewMainMovement,Action) )
                {
                    WasChanged = TRUE;
                }
                if(  CheckOppositeOf(NewMainMovement,Action) )
                {
                    WasChanged = TRUE;
                }
            }
            else if( NewMainSwitch != 0 )
            {
                if( CheckMainInputUsers(NewMainSwitch,Action) )
                {
                    WasChanged = TRUE;
                }
            }
            if( WasChanged )
            {
                SavePositions();
                ArrangeSelectedActions();
                RestorePositions();
            }

        }
    }
    UserOperation = NoOperation; // Operation is no longer in progress.
    return Processed;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
BOOL CDialogInputActions::OnCommand(WPARAM wParam, LPARAM lParam) 
{
    BOOL Processed = FALSE; // TRUE when command is processed.
    const SHORT NotifyCode = HIWORD(wParam);
    const int CommandId  = LOWORD(wParam);
    const int ControlId  = lParam;
    if( NotifyCode == 0 ) // Menu command?
    {
        if( CommandId == ID_ACI_NEW )
        {
            AddNewCombo();
        }
        else
        {
            Processed = ProcessMenuPopUpCommand(CommandId);
        }
    }
    if( !Processed )
    {
        Processed = CDialog::OnCommand(wParam, lParam);
    }
    return Processed;
}

//----------------------------------------------------------------------------
//           Restore the current action to its default settings
//----------------------------------------------------------------------------
void CDialogInputActions::OnUseDefault() 
{
    const FAction::TAction Action = CurrentAction();
    if( Action != 0 )
    {
        // Get the default value for the action:
        const char * Key = FAction::ActionName(Action);
        char * Default = GConfiguration.Get
        (
            FConfiguration::ActionSection
        ,   Key
        ,   GApp->FactoryProfileFileName()
        );
        TActionState & State = ActionStates[Action];
        State.Free(); // Free any existing combos
        State.WasChanged = TRUE; 
        if( Default != 0 ) // If there is a default...
        {
            FAction * Actions = new FAction; // Place to hold new value.
            Actions->Initialize();
            Actions->SetValue(Key,Default);
            Actions->GetCombos( Action, State.ComboCount, &State.Combos );
            Actions->Finalize();
            delete Actions;
            FParse::FreeString(Default);
        }
        SavePositions();
        ArrangeSelectedActions();
        RestorePositions();
    }
}

//----------------------------------------------------------------------------
//           Restore all actions to their default settings
//----------------------------------------------------------------------------
void CDialogInputActions::OnUseDefaultForAll() 
{
    // Get the default values:
    FKeyValues::TStringList DefaultValues = FConfiguration::GetSection
    (
        FConfiguration::ActionSection
    ,   GApp->FactoryProfileFileName()
    );

    FAction * Actions = new FAction; // Place to hold new value.
    Actions->Initialize();
    Actions->SetValues(DefaultValues);
    DefaultValues.Free();

    for( int Action_ = 1; Action_ < FAction::ActionCount; Action_++ )
    {
        const FAction::TAction Action = FAction::TAction(Action_);
        TActionState & State = ActionStates[Action];
        State.Free(); // Free any existing combos
        State.WasChanged = TRUE; 
        Actions->GetCombos( Action, State.ComboCount, &State.Combos ); // Copy over new combos.
    }
    Actions->Finalize();
    delete Actions;

    SavePositions();
    ArrangeSelectedActions();
    RestorePositions();
}

#if 0 //tbd:
//todo: This is temporary - it gives a little bit of help for the action/input mappings.
void CDialogInputActions::OnTutorial() 
{
    static const char * Tutorial[] = 
    {
        "Overview:"             
    ,   ""
    ,   "You have various actions, such as jump, move forward, turn left, "
    ,   "and so on. The actions can be assigned input combos, such as "
    ,   "Shift+CursorUp to run forward. When an input combo is active, its"
    ,   "associated action is done. Input combos consist of keyboard keys,"
    ,   "mouse buttons, joystick buttons, mouse movements, and joystick movements."
    ,   ""
    ,   "You define input combos in the Actions property page of Unreal Properties."
    ,   "An input combo has 3 parts:"
    ,   "   - a main input (key, mouse button or movement, joystick button or movement)"
    ,   "   - up to two modifiers (key, mouse button, or joystick button)"
    ,   "An input combo is active if its modifiers are active (if there are any)"
    ,   "and if the main input is active."
    ,   ""
    ,   "Combo prioritization:"
    ,   "  Input combos with more active modifiers are used instead of input combos with"
    ,   "  fewer active modifiers, for the same main input. This means, for example,"
    ,   "  that the combo Shift+CursorUp will take precedence over the combo CursorUp."
    ,   "  Shift+CursorUp, however, does not affect Shift+CursorDown since they do not"
    ,   "  share the same main input."
    ,   ""
    ,   "Advanced settings:"
    ,   ""
    ,   "A key, mouse button, or joystick button can be Double, Toggle, or both."
    ,   "If Double, the key or button must be pressed twice quickly to be activated."
    ,   "If Toggle, the key or button is alternately activated and deactivated"
    ,   "as it is pressed. Note that you don't have to hold down a Toggle, it remains"
    ,   "active every other time you press it (like the CapsLock key)."
    ,   ""
    ,   "A key or button which is both Double and Toggle is activated and deactivated"
    ,   "every other time it is double-pressed."
    ,   ""
    ,   "The movement kind defines how mouse or joystick movements are to be treated."
    ,   " (Digital)"
    ,   "     The movement is either on or off, depending on how far away the"
    ,   "     movement is from the neutral or center position. For example,"
    ,   "     moving the joystick forward might turn on the MoveForward action."
    ,   "     Once you start moving, pushing the joystick further forward doesn't"
    ,   "     make you move any faster (this is the digital nature of the movement)."
    ,   " "
    ,   " (Analog)"
    ,   "     The position of the movement translates to the size or position of"
    ,   "     the player action. For example, if moving a joystick forward causes"
    ,   "     the player to move forward, then moving the joystick forward some"
    ,   "     more causes the player to move forward faster. "
    ,   "     "
    ,   " (FastAnalog)"
    ,   "     This is the same as (Analog) except that the response of the movement"
    ,   "     accelerates." 
    ,   ""
    ,   " (Change)"
    ,   "     The *change* in the movement translates to a change in the player's"
    ,   "     action. For example, moving the mouse forward a little moves the"
    ,   "     player forward a little. Note that it doesn't matter where the mouse"
    ,   "     is - it is the change in movement that results in the action."
    ,   "     Change (and FastChange) movements are typically used for mouse movements."
    ,   " "
    ,   " (FastChange)"
    ,   "     This is the same as (Change) except that the response of the"
    ,   "     movements is accelerated."
    ,   ""
    ,   0 // Ends the text.
    };
    CDialog * Dialog = new CDialog(IDD_ACTION_TUTORIAL,this);
    Dialog->Create(IDD_ACTION_TUTORIAL,0);
    CListBox * ListBox = (CListBox*)Dialog->GetDlgItem(IDC_ACTION_TUTORIAL);
    if( ListBox != 0 )
    {
        const char * * Strings = Tutorial;
        while( *Strings != 0 )
        {
            ListBox->SetTabStops(70);
            ListBox->AddString( *Strings );
            Strings++;
        }
        Dialog->ShowWindow(SW_SHOW);
    }
    else
    {
        delete Dialog;
    }
}
#endif

//----------------------------------------------------------------------------
//   Check other actions for use of Switch - see if they should be deleted
//----------------------------------------------------------------------------
BOOL CDialogInputActions::CheckMainInputUsers(FInput::TSwitch Switch, FAction::TAction ExceptFor)
{
    BOOL WasChanged = FALSE;
    for( int Action_ = 1; Action_ < FAction::ActionCount; Action_++ )
    {
        const FAction::TAction Action = FAction::TAction(Action_);
        TActionState & State = ActionStates[Action];
        if( Action != ExceptFor && State.UsesMainInput(Switch) )
        {
            char Question[200];
            //todo: Move this into the string table
            char * Text = Question;
            Text += sprintf
            ( 
                Text
            ,   "%s is also used for action %s."
            ,   FInput::Description(Switch)
            ,   FAction::ActionName(Action)
            );
            Text += sprintf
            ( 
                Text
            ,   "\nWould you like to remove all uses of %s from action %s?"
            ,   FInput::Description(Switch)
            ,   FAction::ActionName(Action)
            );
            if( IDYES == AfxMessageBox( Question, MB_YESNO | MB_DEFBUTTON2 ) )
            {
                WasChanged = TRUE;
                State.DeleteMainInput(Switch);
            }
        }
    }
    return WasChanged;
}

//----------------------------------------------------------------------------
//   Check other actions for use of Movement - see if they should be deleted
//----------------------------------------------------------------------------
BOOL CDialogInputActions::CheckMainInputUsers(FInput::TMovement Movement, FAction::TAction ExceptFor)
{
    BOOL WasChanged = FALSE;
    for( int Action_ = 1; Action_ < FAction::ActionCount; Action_++ )
    {
        const FAction::TAction Action = FAction::TAction(Action_);
        TActionState & State = ActionStates[Action];
        if( Action != ExceptFor && State.UsesMainInput(Movement) )
        {
            char Question[200];
            //todo: Move this into the string table
            char * Text = Question;
            Text += sprintf
            ( 
                Text
            ,   "%s is also used for action %s."
            ,   FInput::Description(Movement)
            ,   FAction::ActionName(Action)
            );
            Text += sprintf
            ( 
                Text
            ,   "\nWould you like to remove all uses of %s from action %s?"
            ,   FInput::Description(Movement)
            ,   FAction::ActionName(Action)
            );
            if( IDYES == AfxMessageBox( Question, MB_YESNO | MB_DEFBUTTON2 ) )
            {
                WasChanged = TRUE;
                State.DeleteMainInput(Movement);
            }
        }
    }
    return WasChanged;
}

//----------------------------------------------------------------------------
// Check to see if the user wants the opposite action assigned to the opposite movement
//----------------------------------------------------------------------------
BOOL CDialogInputActions::CheckOppositeOf(FInput::TMovement Movement, FAction::TAction Action)
{
    BOOL WasChanged = FALSE;
    const FAction::TAction OppositeAction = FAction::Opposite(Action);
    const FInput::TMovement OppositeMovement = FInput::OppositeMovement(Movement);
    if( OppositeAction != 0 )
    {
        TActionState & OppositeState = ActionStates[OppositeAction];
        if( !OppositeState.UsesMainInput(OppositeMovement) )
        {
            char Question[200];
            //todo: Move this into the string table
            char * Text = Question;
            Text += sprintf
            ( 
                Text
            ,   "You have chosen [%s] for action %s.\n"
            ,   FInput::Description(Movement)
            ,   FAction::ActionName(Action)
            );
            Text += sprintf
            ( 
                Text
            ,   "Would you like to use [%s] for action %s?"
            ,   FInput::Description(OppositeMovement)
            ,   FAction::ActionName(OppositeAction)
            );
            if( IDYES == AfxMessageBox( Question, MB_YESNO | MB_DEFBUTTON2 ) )
            {
                FInput::TCombo Combo;
                Combo.Empty();
                Combo.IsASwitch = FALSE;
                Combo.MetaMovement.Movement = OppositeMovement;
                Combo.MetaMovement.Kind     = FInput::DefaultMovementKind(OppositeMovement);
                AddNewCombo( OppositeAction, Combo );
                CheckMainInputUsers(OppositeMovement, OppositeAction );
                WasChanged = TRUE;
            }
        }
    }
    return WasChanged;
}

//----------------------------------------------------------------------------
//     Does an action have a combo using Switch as a main input?
//----------------------------------------------------------------------------
BOOL CDialogInputActions::TActionState::UsesMainInput(FInput::TSwitch Switch) const 
{
    BOOL Found = FALSE; // TRUE when a use of Movement is found.
    for( int WhichCombo = 0; !Found && WhichCombo < ComboCount; ++WhichCombo )
    {
        const FInput::TCombo & Combo = Combos[WhichCombo];
        if( Combo.IsSwitch() && Combo.Switch() == Switch )
        {
            Found = TRUE;
        }
    }
    return Found;
}

//----------------------------------------------------------------------------
//     Does an action have a combo using Movement as a main input?
//----------------------------------------------------------------------------
BOOL CDialogInputActions::TActionState::UsesMainInput(FInput::TMovement Movement) const
{
    BOOL Found = FALSE; // TRUE when a use of Movement is found.
    for( int WhichCombo = 0; !Found && WhichCombo < ComboCount; ++WhichCombo )
    {
        const FInput::TCombo & Combo = Combos[WhichCombo];
        if( Combo.IsMovement() && Combo.Movement() == Movement )
        {
            Found = TRUE;
        }
    }
    return Found;
}

//----------------------------------------------------------------------------
//     Does an action have a combo using Switch as a main input or modifier?
//----------------------------------------------------------------------------
BOOL CDialogInputActions::TActionState::Uses(FInput::TSwitch Switch) const
{
    BOOL Found = FALSE; // TRUE when a use of Movement is found.
    for( int WhichCombo = 0; !Found && WhichCombo < ComboCount; ++WhichCombo )
    {
        const FInput::TCombo & Combo = Combos[WhichCombo];
        if
        ( 
                Combo.IsSwitch() && Combo.Switch() == Switch 
            ||  Combo.Modifiers.Has1() && Combo.Modifiers.Switch1() == Switch
        )
        {
            Found = TRUE;
        }
    }
    return Found;
}

//----------------------------------------------------------------------------
//     Delete all of an action's combos which use Switch as a main input
//----------------------------------------------------------------------------
void CDialogInputActions::TActionState::DeleteMainInput(FInput::TSwitch Switch)
{
    for( int WhichCombo = 0; WhichCombo < ComboCount; ++WhichCombo )
    {
        const FInput::TCombo & Combo = Combos[WhichCombo];
        if( Combo.IsSwitch() && Combo.Switch() == Switch )
        {
            Delete(WhichCombo);
            WhichCombo--; // Adjust for deleted entry.
        }
    }
}

//----------------------------------------------------------------------------
//     Delete all of an action's combos which use Movement as a main input
//----------------------------------------------------------------------------
void CDialogInputActions::TActionState::DeleteMainInput(FInput::TMovement Movement)
{
    for( int WhichCombo = 0; WhichCombo < ComboCount; ++WhichCombo )
    {
        const FInput::TCombo & Combo = Combos[WhichCombo];
        if( Combo.IsMovement() && Combo.Movement() == Movement )
        {
            Delete(WhichCombo);
            WhichCombo--; // Adjust for deleted entry.
        }
    }
}

//----------------------------------------------------------------------------
//     Delete all of an action's combos which use Switch as a main input or modifier
//----------------------------------------------------------------------------
void CDialogInputActions::TActionState::Delete(FInput::TSwitch Switch)
{
    for( int WhichCombo = 0; WhichCombo < ComboCount; ++WhichCombo )
    {
        const FInput::TCombo & Combo = Combos[WhichCombo];
        if
        ( 
                Combo.IsSwitch() && Combo.Switch() == Switch 
            ||  Combo.Modifiers.Has1() && Combo.Modifiers.Switch1() == Switch
        )
        {
            Delete(WhichCombo);
            WhichCombo--; // Adjust for deleted entry.
        }
    }
}

//----------------------------------------------------------------------------
//                  Delete the N'th combo
//----------------------------------------------------------------------------
void CDialogInputActions::TActionState::Delete(int N)
{
    if( N < ComboCount )
    {
        // Delete the combo by shifting later entries into its position,
        // unless the combo being deleted is the last one.
        const int ShiftCount = ComboCount - N - 1;
        if( ShiftCount > 0 )
        {
            memmove( &Combos[N], &Combos[N+1], sizeof(Combos[0])*ShiftCount );
        }
        ComboCount--;
        WasChanged = TRUE;
    }
}

//----------------------------------------------------------------------------
//                  
//----------------------------------------------------------------------------
void CDialogInputActions::OnContextMenu(CWnd* pWnd, CPoint Point) 
{
    AllCombos.SetFocus();    
    BOOL IsOutside;
    AllCombos.ScreenToClient( &Point );
    const int Selection = AllCombos.ItemFromPoint( Point, IsOutside );
    if( !IsOutside && Selection >= 0 && Selection < AllCombos.GetCount() )
    {
        AllCombos.SetCurSel( Selection );
        ArrangeCurrentCombo();
        ChangeCurrentCombo();
    }
}
