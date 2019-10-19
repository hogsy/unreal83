// InpSenDi.cpp : implementation file
/*
==============================================================================

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0, Calling method=__fastcall

Description:
    This is the CDialog class for handling the Unreal input sensitivity dialog.

Revision history:
    * 06/26/96, Created by Mark
==============================================================================
*/
//

#include "stdafx.h"
#include "unwn.h"
#include "UnWnSenD.h"

#include "Unreal.h"
#include "UnInput.h" 
#include "UnConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogInputSensitivity dialog


CDialogInputSensitivity::CDialogInputSensitivity(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogInputSensitivity::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogInputSensitivity)
	AnalogThreshold = 0;
	DigitalThreshold = 0;
	//}}AFX_DATA_INIT
}

BOOL CDialogInputSensitivity::OnInitDialog()
{
    CDialog::OnInitDialog();
    ChangesWereSaved = FALSE;
    CurrentMovement = FInput::M_None;

    LoadValuesFrom( GInput.MovementsInfo );

    // Initialize sliders and spinners:
    {
        AnalogThresholdSpinner        .SetRange(  0,  90 );
        DigitalThresholdSpinner       .SetRange(  0,  90 );
        SpeedSlider                   .SetRange( 10, 300 );
    }

    ArrangeMovements( AxesAreSymmetrical() );

    return FALSE; // FALSE tells Windows not to set the input focus. 
                  // We do this because the dialog is expected to be part
                  // of a tab group and we don't want to change the focus.
}

//----------------------------------------------------------------------------
//                Arrange the movements
//----------------------------------------------------------------------------
void CDialogInputSensitivity::ArrangeMovements(BOOL Merged, FInput::TMovement Selection) 
{
    AxesAreMerged = Merged;
    PrepareSymmetryButtons();
    // Initialize movement list
    MovementList.ResetContent();
    for( int Movement_ = 1; Movement_ < FInput::MovementCount; ++Movement_ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        const char * Description = 0; // Leave as 0 if movement is not to be added.
        if( AxesAreMerged )
        {
            // Show only positive axes, but with a neutral description:
            if( FInput::IsPositive(Movement) )
            {
                Description = FInput::NeutralDescription(Movement);
            }
        }
        else
        {
            Description = FInput::Description(Movement);
        }
        if( Description != 0 )
        {
            const int Index = MovementList.AddString( Description );
            // Set the item's data to be the movement...
            MovementList.SetItemData( Index, DWORD(Movement) );
        }
    }
    MovementList.SetTopIndex(0); 
    const int Index = Selection != 0 ? Selection-1 : 0;
    MovementList.SetCurSel( Index < MovementList.GetCount() ? Index : 0 ); 
    PrepareDialogSettings();
}

//----------------------------------------------------------------------------
//                What is the displayed analog threshold?
//----------------------------------------------------------------------------
FLOAT CDialogInputSensitivity::GetAnalogThreshold() const
{
    return FLOAT( AnalogThreshold ) / 100.0;
}

//----------------------------------------------------------------------------
//              What is the displayed digital threshold?
//----------------------------------------------------------------------------
FLOAT CDialogInputSensitivity::GetDigitalThreshold() const 
{
    return FLOAT( DigitalThreshold ) / 100.0;
}

//----------------------------------------------------------------------------
//              What is the displayed speed?
//----------------------------------------------------------------------------
FLOAT CDialogInputSensitivity::GetSpeed() const
{
    return FLOAT( SpeedSlider.GetPos() ) / 100.0;
}

//----------------------------------------------------------------------------
//                   Set the displayed analog threshold.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::SetAnalogThreshold(FLOAT Value)
{
    const int IntValue = int(Value*100+.5);
    AnalogThreshold = IntValue;
    UpdateData(FALSE);
}

//----------------------------------------------------------------------------
//                  Set the displayed digital threshold.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::SetDigitalThreshold(FLOAT Value)
{
    const int IntValue = int(Value*100+.5);
    DigitalThreshold = IntValue;
    UpdateData(FALSE);
}

//----------------------------------------------------------------------------
//                        Set the displayed speed.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::SetSpeed(FLOAT Value)
{
    const int IntValue = int(Value*100+.5);
    SpeedSlider.SetPos( IntValue );
    UpdateData(FALSE);
}
//----------------------------------------------------------------------------
//        Load values from an array of MovementCount values.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::LoadValuesFrom( const FInput::TMovementInfo * Info )
{
    for( int Movement_ = 1; Movement_ < FInput::MovementCount; ++Movement_ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        MovementInfo[Movement] = Info[Movement];
    }
}

//----------------------------------------------------------------------------
//           Save value into an array of MovementCount values.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::SaveValuesInto( FInput::TMovementInfo * Info )
{
    for( int Movement_ = 1; Movement_ < FInput::MovementCount; ++Movement_ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        Info[Movement] = MovementInfo[Movement];
    }
}

//----------------------------------------------------------------------------
//                     Load settings into the dialog.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::PrepareDialogSettings()
{
    const int Selection = MovementList.GetCurSel();
    CurrentMovement = // The movement associated with an item is stored in the item's data.
        Selection == LB_ERR 
    ?   FInput::M_None 
    :   FInput::TMovement( MovementList.GetItemData(Selection) )
    ;
    if( CurrentMovement != 0 )
    {
        const FInput::TSensitivity & Sensitivity = CurrentSensitivity();
        SetAnalogThreshold      ( Sensitivity.AnalogThreshold       );
        SetDigitalThreshold     ( Sensitivity.DigitalThreshold      );
        SetSpeed                ( Sensitivity.Speed                 );
    }
}
//----------------------------------------------------------------------------
//               Locally save settings from the dialog.
//----------------------------------------------------------------------------
BOOL CDialogInputSensitivity::RememberDialogSettings()
{
    BOOL IsOkay = UpdateData(TRUE);
    if( IsOkay && CurrentMovement != 0 )
    {
        FInput::TSensitivity & Sensitivity = CurrentSensitivity();
        Sensitivity.AnalogThreshold         = GetAnalogThreshold      ();
        Sensitivity.DigitalThreshold        = GetDigitalThreshold     ();
        Sensitivity.Speed                   = GetSpeed                ();
        // If the axes are merged, duplicate the settings in the opposite movement.
        if( AxesAreMerged )
        {
            MovementInfo[ FInput::OppositeMovement(CurrentMovement) ].Sensitivity = Sensitivity;
        }
    }
    return IsOkay;
}

void CDialogInputSensitivity::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogInputSensitivity)
	DDX_Control(pDX, IDC_INPUT_DIGITAL_THRESHOLD, DigitalThresholdControl);
	DDX_Control(pDX, IDC_INPUT_ANALOG_THRESHOLD, AnalogThresholdControl);
	DDX_Control(pDX, IDC_INPUT_MOVEMENT_LIST, MovementList);
	DDX_Control(pDX, IDC_SEPARATE_AXES, SeparateButton);
	DDX_Control(pDX, IDC_MERGE_AXES, MergeButton);
	DDX_Control(pDX, IDC_SPEED, SpeedSlider);
	DDX_Control(pDX, IDC_INPUT_DIGITAL_THRESHOLD_SPINNER, DigitalThresholdSpinner);
	DDX_Control(pDX, IDC_INPUT_ANALOG_THRESHOLD_SPINNER, AnalogThresholdSpinner);
	DDX_Text(pDX, IDC_INPUT_ANALOG_THRESHOLD, AnalogThreshold);
	DDV_MinMaxUInt(pDX, AnalogThreshold, 0, 90);
	DDX_Text(pDX, IDC_INPUT_DIGITAL_THRESHOLD, DigitalThreshold);
	DDV_MinMaxUInt(pDX, DigitalThreshold, 0, 90);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogInputSensitivity, CDialog)
	//{{AFX_MSG_MAP(CDialogInputSensitivity)
	ON_CBN_SELCHANGE(IDC_INPUT_MOVEMENT_LIST, OnSelchangeInputMovementList)
	ON_BN_CLICKED(ID_USE_DEFAULTS, OnUseDefaults)
	ON_BN_CLICKED(ID_USE_DEFAULTS_FOR_ALL, OnUseDefaultsForAll)
	ON_BN_CLICKED(IDC_MERGE_AXES, OnMergeAxes)
	ON_BN_CLICKED(IDC_SEPARATE_AXES, OnSeparateAxes)
	ON_EN_CHANGE(IDC_INPUT_ANALOG_THRESHOLD, OnChangeThreshold)
	ON_EN_CHANGE(IDC_INPUT_DIGITAL_THRESHOLD, OnChangeThreshold)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogInputSensitivity message handlers


//----------------------------------------------------------------------------
//                   The selected movement was just changed.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::OnSelchangeInputMovementList() 
{
    if( RememberDialogSettings() )
    {
        PrepareDialogSettings();
    }
    else
    {
        // Don't allow the change...
        MovementList.SetCurSel(CurrentMovement);
    }
}

//----------------------------------------------------------------------------
//                      See if two floating values are close
//----------------------------------------------------------------------------
// Return TRUE if all the values are the same (or very close), FALSE otherwise.
static BOOL IsSame(FLOAT Value1, FLOAT Value2)
{
    const FLOAT MaxDelta = 0.001f;
    const FLOAT Delta = Value2-Value1;
    const BOOL IsSame = -MaxDelta <= Delta && Delta <= MaxDelta;
    return IsSame;
}

//----------------------------------------------------------------------------
//                     Compare old and new sensitivity settings
//----------------------------------------------------------------------------
// Return TRUE if all the settings are the same (or very close), FALSE otherwise.
static BOOL IsSame(const FInput::TMovementInfo & Value1, const FInput::TMovementInfo & Value2 )
{
    return 
        IsSame( Value1.Sensitivity.AnalogThreshold, Value2.Sensitivity.AnalogThreshold )    
    &&  IsSame( Value1.Sensitivity.DigitalThreshold, Value2.Sensitivity.DigitalThreshold )    
    &&  IsSame( Value1.Sensitivity.Speed, Value2.Sensitivity.Speed )    
    ;
}

//----------------------------------------------------------------------------
//                      Check input and accept changes
//----------------------------------------------------------------------------
BOOL CDialogInputSensitivity::Accept() 
{
    BOOL Accepted = RememberDialogSettings();
    if( Accepted )
    {
        // See if any movements have changed...
        BOOL ChangesWereMade = FALSE;
        for( int Movement_ = 1; !ChangesWereMade && Movement_ < FInput::MovementCount; ++Movement_ )
        {
            const FInput::TMovement Movement = FInput::TMovement(Movement_);
            if( !IsSame( MovementInfo[Movement], GInput.MovementsInfo[Movement] ) )
            {
                ChangesWereMade = TRUE;
            }
        }
        if( ChangesWereMade )
        {
            SaveValuesInto( GInput.MovementsInfo );
        }
        ChangesWereSaved = ChangesWereMade;
    }
    return Accepted;
}

//----------------------------------------------------------------------------
//           Restore the current movement to its default settings
//----------------------------------------------------------------------------
void CDialogInputSensitivity::OnUseDefaults() 
{
    if( CurrentMovement != 0 )
    {
        FInput::TMovementInfo Defaults[FInput::MovementCount];
        memmove( Defaults, MovementInfo, sizeof(Defaults) );
        // Get the default values:
        FKeyValues::TStringList DefaultValues = FConfiguration::GetSection
        (
            FConfiguration::InputSection
        ,   GApp->FactoryProfileFileName()
        );
        FInput::SetValues(DefaultValues,Defaults);
        DefaultValues.Free();
        MovementInfo[CurrentMovement] = Defaults[CurrentMovement];
        if( AxesAreMerged )
        {
            // We also restore the opposite movement:
            const FInput::TMovement Opposite = FInput::OppositeMovement(CurrentMovement);
            MovementInfo[Opposite] = Defaults[Opposite];
        }
        ArrangeMovements(AxesAreMerged && AxesAreSymmetrical(), CurrentMovement );
    }
}

//----------------------------------------------------------------------------
//           Restore all movements to their default settings
//----------------------------------------------------------------------------
void CDialogInputSensitivity::OnUseDefaultsForAll() 
{
    FInput::TMovementInfo Defaults[FInput::MovementCount];
    memmove( Defaults, MovementInfo, sizeof(Defaults) );
    // Get the default values:
    FKeyValues::TStringList DefaultValues = FConfiguration::GetSection
    (
        FConfiguration::InputSection
    ,   GApp->FactoryProfileFileName()
    );
    FInput::SetValues(DefaultValues,Defaults);
    DefaultValues.Free();
    LoadValuesFrom( Defaults );
    ArrangeMovements( AxesAreSymmetrical()  );
}

void CDialogInputSensitivity::PostNcDestroy() 
{
    CDialog::PostNcDestroy();
    delete this;
}

//----------------------------------------------------------------------------
// Prepare the Merge/Separate buttons based on AxesAreMerged.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::PrepareSymmetryButtons()
{
    MergeButton.EnableWindow( !AxesAreMerged );
    SeparateButton.EnableWindow( AxesAreMerged );
}

//----------------------------------------------------------------------------
//                   Check axes symmetry
//----------------------------------------------------------------------------
BOOL CDialogInputSensitivity::AxesAreSymmetrical()
{
    BOOL IsSymmetrical = TRUE;
    for( int Movement_ = 1; IsSymmetrical && Movement_ < FInput::MovementCount; ++Movement_ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        const FInput::TMovement OppositeMovement = FInput::OppositeMovement(Movement);
        if( !IsSame( MovementInfo[Movement], MovementInfo[OppositeMovement] ) )
        {
            IsSymmetrical = FALSE;
        }
    }
    return IsSymmetrical;
 }

//----------------------------------------------------------------------------
//         Merge each positive/negative axis pair into a single axis.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::OnMergeAxes() 
{
    if( RememberDialogSettings() )
    {
        for( int Movement_ = 1; Movement_ < FInput::MovementCount; ++Movement_ )
        {
            const FInput::TMovement Movement = FInput::TMovement(Movement_);
            const FInput::TMovement Opposite = FInput::OppositeMovement(Movement);
            // Average the sensitivity values for the positive/negative axes.
            // We actually do this twice for each pair.
            FInput::TSensitivity & Info1 = MovementInfo[Movement].Sensitivity;
            FInput::TSensitivity & Info2 = MovementInfo[Opposite].Sensitivity;
            Info1.AnalogThreshold = (Info1.AnalogThreshold + Info2.AnalogThreshold)/2;
            Info1.DigitalThreshold = (Info1.DigitalThreshold + Info2.DigitalThreshold)/2;
            Info1.Speed = (Info1.Speed + Info2.Speed)/2;
            Info2 = Info1; 
        }
        ArrangeMovements(TRUE);
    }
}

//----------------------------------------------------------------------------
//         Separate positive/negative axes.
//----------------------------------------------------------------------------
void CDialogInputSensitivity::OnSeparateAxes() 
{
    if( RememberDialogSettings() )
    {
        ArrangeMovements(FALSE);
    }
}

void CDialogInputSensitivity::OnChangeThreshold() 
{
    // I'm not sure why this is necessary, but changes made with the
    // spinner buttons don't cause the edit box to be updated.
    DigitalThresholdControl.RedrawWindow( 0, 0, RDW_UPDATENOW );
    AnalogThresholdControl.RedrawWindow( 0, 0, RDW_UPDATENOW );
}
