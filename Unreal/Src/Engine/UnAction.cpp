/*==============================================================================
UnAction.cpp: Player input actions

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 05/30/96: Created by Mark
==============================================================================*/

#include "UnAction.h"
#include "UnChecks.h"
#include "UnParse.h"
#include "UnPrefer.h"

#define Debugging_      0 // 1 to turn on general debugging, 0 otherwise.
#define ShowMovements   0 // 1 to show movement, 0 otherwise.
#define ShowActions     0 // 1 to show actions, 0 otherwise.

#define arrayCount_(Array) ( sizeof(Array) / sizeof((Array)[0]) )

FAction GAction;

#if Debugging_
    static void __cdecl Debug(const char * Message, ...)
    {
        char Text[1000];
        va_list ArgumentList;
        va_start(ArgumentList,Message);
        vsprintf(Text,Message,ArgumentList);
        va_end(ArgumentList);
        debugf(LOG_Info,Text);
    }
#else
    static inline void __cdecl Debug(const char * Message, ...)
    {
    }
#endif

struct TActionInfo
{
    FAction::TAction    Action              ; // Unnecessary, but included for safety.
    FAction::TClass     Class               ; // The classification of the action.
    FLOAT               AnalogFactor        ; // Multiply analog magnitude by this value.
    FLOAT               DifferentialFactor  ; // Multiply differential magnitude by this value.
    FAction::TAction    Opposite            ; // The opposite action, or NoAction if there is none.
    EMovementAxis       Axis                ; // The axis affected by the action, or PlayerAxis_None.
    BOOL                IsPositiveAxis      ; // The action affects a movement axis in a "positive" way.
    const char *        Name                ; // Action name
    const char *        Help                ; // Action help text
    // Notes:
    //  1. Member Pass lets us process actions in a specific order.
    //     It lets us first process actions which might affect other actions
    //     during pass 1, then processing the remaining actions during pass 2.
};

static const TActionInfo ActionsInfo[FAction::ActionCount] = 
{
    // Cryptic abbreviations to simplify table:
    #define A FAction
    #define T TRUE
    #define F FALSE
    #define CNone A::NoClassification
    #define CTurn A::TurningAction
    #define CMove A::MovingAction
    #define CAdva A::AdvancedAction
    #define CMisc A::MiscellaneousAction
    #define CAdmi A::AdministrativeAction
    #define CWeap A::WeaponAction
    #define PANone PlayerAxis_None
    #define PAForw PlayerAxis_Forward
    #define PAYaw  PlayerAxis_Yaw
    #define PAPitc PlayerAxis_Pitch
    #define PARoll PlayerAxis_Roll
    #define PARigh PlayerAxis_Rightward
    #define PAUp   PlayerAxis_Upward
    // Use CNone for actions that are to be supressed in the action dialogs.
    // This might be for debugging actions or unimplemented actions.
    //Action                  Class    AF     DF   Opposite          Axis  IsP. Name               Help
    { A::NoAction           , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "None"           , "None"                                                  } 
,   { A::Accelerate         , CNone,  0.00,  0.00, A::Decelerate   , PANone, F, "Accelerate"     , "Increase forward speed"                                }
,   { A::BankLeft           , CNone, -1.00, -1.00, A::BankRight    , PARoll, F, "BankLeft"       , "[Flying] Bank to the left"                             }
,   { A::BankRight          , CNone, +1.00, +1.00, A::BankLeft     , PARoll, T, "BankRight"      , "[Flying] Bank to the right"                            }
,   { A::Chat               , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Chat"           , "Send a message to other players"                       }
,   { A::ConsoleFull        , CAdmi,  0.00,  0.00, A::NoAction     , PANone, F, "ConsoleFull"    , "Open a full-screen console"                            }
,   { A::ConsoleHalf        , CAdmi,  0.00,  0.00, A::NoAction     , PANone, F, "Console"        , "Open a half-screen console"                            }
,   { A::ConsoleType        , CAdmi,  0.00,  0.00, A::NoAction     , PANone, F, "ConsoleType"    , "Type a console command"                                }
,   { A::Crouch             , CMisc,  0.00,  0.00, A::NoAction     , PANone, F, "Crouch"         , "Crouch down to the ground"                             }
,   { A::Decelerate         , CNone,  0.00,  0.00, A::Accelerate   , PANone, F, "Decelerate"     , "Decrease forward speed"                                }
,   { A::FullScreen         , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "FullScreen"     , "Turn full-screen on or off"                            }
,   { A::Jump               , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Jump"           , "Jump upwards"                                          }
,   { A::KeyboardLookShift  , CTurn,  0.00,  0.00, A::NoAction     , PANone, F, "KeyboardLook"   , "Change keyboard movements into looks and turns"        }
,   { A::Kick               , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Kick"           , "Kick some alien butt!"                                 }
,   { A::LookDown           , CTurn, -1.00, -1.00, A::LookUp       , PAPitc, F, "LookDown"       , "Look downward"                                         } 
,   { A::LookFromBehind     , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "LookFromBehind" , "[Debug] Change view to behind player pawn"             }
,   { A::LookStraight       , CTurn,  0.00,  0.00, A::NoAction     , PANone, F, "LookStraight"   , "Look straight ahead (cancels LookUp or LookDown)"      }
,   { A::LookUp             , CTurn, +1.00, +1.00, A::LookDown     , PAPitc, T, "LookUp"         , "Look upward"                                           }
,   { A::LungeBackward      , CNone,  0.00,  0.00, A::LungeForward , PANone, F, "LungeBackward"  , "Move backward with a sudden short burst of speed"      }
,   { A::LungeDown          , CNone,  0.00,  0.00, A::LungeUp      , PANone, F, "LungeDown"      , "[Flying] Move down with a sudden short burst of speed" }
,   { A::LungeForward       , CNone,  0.00,  0.00, A::LungeBackward, PANone, F, "LungeForward"   , "Move forward with a sudden, short burst of speed"      }
,   { A::LungeLeft          , CNone,  0.00,  0.00, A::LungeRight   , PANone, F, "LungeLeft"      , "Move left with a sudden, short burst of speed"         }
,   { A::LungeRight         , CNone,  0.00,  0.00, A::LungeLeft    , PANone, F, "LungeRight"     , "Move right with a sudden, short burst of speed"        }
,   { A::LungeUp            , CNone,  0.00,  0.00, A::LungeDown    , PANone, F, "LungeUp"        , "[Flying] Move up with a sudden, short burst of speed"  }
,   { A::MotionLookShift    , CTurn,  0.00,  0.00, A::NoAction     , PANone, F, "MouseLook"      , "Change mouse/joystick movements into look up/down"     }
,   { A::MoveBackward       , CMove, -1.00, -1.00, A::MoveForward  , PAForw, F, "MoveBackward"   , "Move backward"                                         }
,   { A::MoveDown           , CNone, -1.00, -1.00, A::MoveUp       , PAUp  , F, "MoveDown"       , "[Flying] Move downward"                                }
,   { A::MoveForward        , CMove, +1.00, +1.00, A::MoveBackward , PAForw, T, "MoveForward"    , "Move forward"                                          } 
,   { A::MoveLeft           , CMove, -1.00, -1.00, A::MoveRight    , PARigh, F, "MoveLeft"       , "Move to the left"                                      }
,   { A::MoveRight          , CMove, +1.00, +1.00, A::MoveLeft     , PARigh, T, "MoveRight"      , "Move to the right"                                     }
,   { A::MoveUp             , CNone, +1.00, +1.00, A::MoveDown     , PAUp  , T, "MoveUp"         , "[Flying] Move upward"                                  }
,   { A::Pause              , CMisc,  0.00,  0.00, A::NoAction     , PANone, F, "Pause"          , "Pause or unpause the game"                             }
,   { A::RollLeft           , CNone, -1.00, -1.00, A::RollRight    , PARoll, F, "RollLeft"       , "[Flying] Bank to the left (faster than BankLeft)"      }
,   { A::RollRight          , CNone, +1.00, +1.00, A::RollLeft     , PARoll, T, "RollRight"      , "[Flying] Bank to the right (faster than BankRight)"    }
,   { A::RunShift           , CMove,  0.00,  0.00, A::NoAction     , PANone, F, "Run"            , "Move faster"                                           }
,   { A::RunBackward        , CMove, -2.00, -2.00, A::RunForward   , PAForw, F, "RunBackward"    , "Run backward (faster than MoveBackward)"               }
,   { A::RunDown            , CNone, -2.00, -2.00, A::RunUp        , PAUp  , F, "RunDown"        , "[Flying] Move downward (faster than MoveDown)"         }
,   { A::RunForward         , CMove, +2.00, +2.00, A::RunBackward  , PAForw, T, "RunForward"     , "Run forward (faster than MoveForward)"                 }
,   { A::RunLeft            , CMove, -2.00, -2.00, A::RunRight     , PARigh, F, "RunLeft"        , "Run left (faster than MoveLeft)"                       }
,   { A::RunRight           , CMove, +2.00, +2.00, A::RunLeft      , PARigh, T, "RunRight"       , "Run right (faster than MoveRight)"                     }
,   { A::RunUp              , CNone, +2.00, +2.00, A::RunDown      , PAUp  , T, "RunUp"          , "[Flying] Move up (faster than MoveUp)"                 }
,   { A::ScreenEnlarge      , CAdmi,  0.00,  0.00, A::NoAction     , PANone, F, "ScreenEnlarge"  , "Make the playing area larger"                          }
,   { A::ScreenShot         , CMisc,  0.00,  0.00, A::NoAction     , PANone, F, "ScreenShot"     , "Take a snapshot of the screen in .pcx file format"     }
,   { A::ScreenShrink       , CAdmi,  0.00,  0.00, A::NoAction     , PANone, F, "ScreenShrink"   , "Make the playing area smaller"                         }
,   { A::SlideShift         , CMove,  0.00,  0.00, A::NoAction     , PANone, F, "Slide"          , "Change left/right turns into left/right moves"         }
,   { A::SpinShift          , CTurn,  0.00,  0.00, A::NoAction     , PANone, F, "Spin"           , "Turn faster (changes Turn... into Spin...)"            }
,   { A::SpinDown           , CNone, -2.00, -2.00, A::SpinUp       , PAPitc, F, "SpinDown"       , "[Flying] Turn downward (faster than TurnDown)"         }
,   { A::SpinLeft           , CTurn, -2.00, -2.00, A::SpinRight    , PAYaw , F, "SpinLeft"       , "Turn left (faster than TurnLeft)"                      }
,   { A::SpinRight          , CTurn, +2.00, +2.00, A::SpinLeft     , PAYaw , T, "SpinRight"      , "Turn right (faster than TurnRight)"                    }
,   { A::SpinUp             , CNone, +2.00, +2.00, A::SpinDown     , PAPitc, T, "SpinUp"         , "[Flying] Turn upward (faster than TurnUp)"             }
,   { A::TurnDown           , CNone, -1.00, -1.00, A::TurnUp       , PAPitc, F, "TurnDown"       , "[Flying] Turn downward"                                }
,   { A::TurnLeft           , CTurn, -1.00, -1.00, A::TurnRight    , PAYaw , F, "TurnLeft"       , "Turn to the left"                                      }
,   { A::TurnRight          , CTurn, +1.00, +1.00, A::TurnLeft     , PAYaw , T, "TurnRight"      , "Turn to the right"                                     }
,   { A::TurnUp             , CNone, +1.00, +1.00, A::TurnDown     , PAPitc, T, "TurnUp"         , "[Flying] Turn upward"                                  }
,   { A::WeaponFire         , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponFire"     , "Fire current weapon"                                   }
,   { A::WeaponSpecial      , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponSpecial"  , "Use current weapon's special function"                 }
,   { A::WeaponCloseUp      , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponCloseUp"  , "Use current weapon's close-up function"                }
,   { A::WeaponNext         , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponNext"     , "Select next weapon"                                    }
,   { A::WeaponPrevious     , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponPrevious" , "Select previous weapon"                                }
,   { A::WeaponReady        , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponReady"    , "Lower or raise the current weapon"                     }
,   { A::WeaponSet1         , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponAutoMag"  , "Select AutoMag"                                        }
,   { A::WeaponSet2         , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponQuadShot" , "Select QuadShot"                                       }
,   { A::WeaponSet3         , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponStinger"  , "Select Stinger"                                        }
,   { A::WeaponSet4         , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Weapon4"        , "Select weapon set #4, or cycle within set"             }
,   { A::WeaponSet5         , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Weapon5"        , "Select weapon set #5, or cycle within set"             }
,   { A::WeaponSet6         , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Weapon6"        , "Select weapon set #6, or cycle within set"             }
,   { A::WeaponSet7         , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Weapon7"        , "Select weapon set #7, or cycle within set"             }
,   { A::WeaponSet8         , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Weapon8"        , "Select weapon set #8, or cycle within set"             }
,   { A::WeaponSet9         , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Weapon9"        , "Select weapon set #9, or cycle within set"             }
,   { A::WeaponSet10        , CNone,  0.00,  0.00, A::NoAction     , PANone, F, "Weapon10"       , "Select weapon set #10, or cycle within set"            }
,   { A::WeaponSwap         , CWeap,  0.00,  0.00, A::NoAction     , PANone, F, "WeaponSwap"     , "Switch between two most recently used weapons"         }
,   { A::Zoom               , CMisc,  0.00,  0.00, A::NoAction     , PANone, F, "Zoom"           , "Zooms (magnifies) view"                                }
    // Remove cryptic abbreviations
    #undef A
    #undef T
    #undef F
    #undef CNone 
    #undef CTurn 
    #undef CMove 
    #undef CAdva 
    #undef CMisc 
    #undef CAdmi 
    #undef CWeap 
};

//----------------------------------------------------------------------------
//                 The name for an action
//----------------------------------------------------------------------------
const char * FAction::ActionName(TAction Action)
{
    return ActionsInfo[Action].Name;
}

//----------------------------------------------------------------------------
//                 The "help text" for an action
//----------------------------------------------------------------------------
const char * FAction::ActionHelp(TAction Action)
{
    return ActionsInfo[Action].Help;
}


FAction::TClass FAction::Class(TAction Action)
{
    return ActionsInfo[Action].Class;
}

FAction::TAction FAction::Opposite(TAction Action)
{
    return ActionsInfo[Action].Opposite;
}

EMovementAxis FAction::Axis(TAction Action)
{
    return ActionsInfo[Action].Axis;
}

static inline FLOAT AnalogFactor(FAction::TAction Action)
{
    return ActionsInfo[Action].AnalogFactor;
}

static inline FLOAT DifferentialFactor(FAction::TAction Action)
{
    return ActionsInfo[Action].DifferentialFactor;
}

//----------------------------------------------------------------------------
//                 Parsing helper functions.
//----------------------------------------------------------------------------

// Find the action which matches the given text,
// returning the matching action or NoAction if not found.
static FAction::TAction FindAction(const char * Text)
{
    FAction::TAction Action = FAction::NoAction;
    if( Text != 0 && Text[0] != 0 )
    {
        for( int Check = 1; Action==0 && Check < FAction::ActionCount; Check++ )
        {
            if( stricmp( Text, FAction::ActionName( FAction::TAction(Check) ) ) == 0 )
            {
                Action = FAction::TAction(Check);
            }
        }
    }
    return Action;
}

//----------------------------------------------------------------------------
//                        Initialize
//----------------------------------------------------------------------------
void FAction::Initialize()
{
    PPlayerTick Check;
    Check.Actions[0] = 0; // Just to avoid compiler warning.
    checkVital( arrayCount_(Check.Actions) >= ActionCount, "Too few actions" );
    for( int Action_ = 0; Action_ < ActionCount; Action_++ )
    {
        const TAction Action = TAction(Action_);
        // Make sure the action table is correctly indexed:
        if( ActionsInfo[Action].Action != Action )
        {
            appError( "Action table error" );
        }
        else if( Opposite(Action) != 0 && Opposite(Opposite(Action)) != Action )
        {
            appError( "Action opposite error" );
        }
    }
    for( int Switch_ = 0; Switch_ < arrayCount_(SwitchCombos); Switch_++ )
    {
        const FInput::TSwitch Switch = FInput::TSwitch(Switch_);
        SwitchCombos[Switch].Initialize();
    }
    for( int Movement_ = 0; Movement_ < arrayCount_(MovementCombos); Movement_++ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        MovementCombos[Movement].Initialize();
    }
    Reset();
}

//----------------------------------------------------------------------------
//                        Finalize
//----------------------------------------------------------------------------
void FAction::Finalize()
{
    for( int Switch_ = 0; Switch_ < arrayCount_(SwitchCombos); Switch_++ )
    {
        const FInput::TSwitch Switch = FInput::TSwitch(Switch_);
        SwitchCombos[Switch].Finalize();
    }
    for( int Movement_ = 0; Movement_ < arrayCount_(MovementCombos); Movement_++ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        MovementCombos[Movement].Finalize();
    }
}
//----------------------------------------------------------------------------
// Clear out all definitions.
//----------------------------------------------------------------------------
void FAction::Empty()
{
    for( int Switch_ = 0; Switch_ < arrayCount_(SwitchCombos); Switch_++ )
    {
        const FInput::TSwitch Switch = FInput::TSwitch(Switch_);
        SwitchCombos[Switch].Initialize();
    }
    for( int Movement_ = 0; Movement_ < arrayCount_(MovementCombos); Movement_++ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        MovementCombos[Movement].Initialize();
    }
    Reset();
    UsePlainMouseMovements( FALSE );
}

//----------------------------------------------------------------------------
//       Reset the state of all actions
//----------------------------------------------------------------------------
void FAction::Reset()
{
    for( int Action = 1; Action < ActionCount; ++Action )
    {
        ClearStatus( Status[Action], IsActiveStatus | IsPendingStatus );
    }
}

//----------------------------------------------------------------------------
//         Make sure there is enough room for 1 more combo.
//----------------------------------------------------------------------------
void FAction::TActionCombos::MakeRoom()
{
    GUARD;
    if( Count >= MaxCount )
    {
        const int NewMaxCount = MaxCount + 5;
        TActionCombo * NewList = appMallocArray(NewMaxCount,TActionCombo, "ActionCombos" );
        if( Count > 0 ) // Are there any existing elements to copy over?
        {
            memmove( NewList, List, Count * sizeof( NewList[0] ) );
        }
        if( List != 0 ) // Should we delete the existing list?
        {
            appFree( List );
        }
        List = NewList;
        MaxCount = NewMaxCount;
    }
    UNGUARD( "FAction::TActionCombos::MakeRoom" );
}

//----------------------------------------------------------------------------
//         Free the storage associated with the action combos.
//----------------------------------------------------------------------------
void FAction::TActionCombos::Free()
{
    GUARD;
    if( List != 0 )
    {
        appFree(List);
        MaxCount = 0;
        Count = 0;
        List = 0;
    }
    UNGUARD( "FAction::TActionCombos::Free" );
}
//----------------------------------------------------------------------------
//          Is the specified combo somewhere in the list of combos?
//----------------------------------------------------------------------------
BOOL FAction::TActionCombos::Has(const TActionCombo & ActionCombo) const
{
    BOOL Found = FALSE;
    for( int Which = 0; !Found && Which < Count; ++Which )
    {
        const TActionCombo & Compare = List[Which];
        const FInput::TCombo & Combo = ActionCombo;
        const FInput::TCombo & CompareCombo = Compare;
        Found = Compare.Action==ActionCombo.Action && Combo==CompareCombo;
    }
    return Found;
}

//----------------------------------------------------------------------------
//                   Add an action combo.
//----------------------------------------------------------------------------
void FAction::TActionCombos::Add(const TActionCombo & ActionCombo)
{
    GUARD;
    if( ActionCombo.Action != 0 && !Has(ActionCombo) )
    {
        MakeRoom();
        TActionCombo & NewCombo = List[Count];
        NewCombo = ActionCombo;
        NewCombo.Normalize();
        Count++;
    }
    UNGUARD( "TActionCombos::Add" );
}

//----------------------------------------------------------------------------
//           Remove all definitions for Action.
//----------------------------------------------------------------------------
void FAction::TActionCombos::Empty(TAction Action)
{
    int Which = 0;
    while( Which < Count )
    {
        TActionCombo & ActionCombo = List[Which];
        if( ActionCombo.Action==Action )
        {
            const int LastIndex = Count-1;
            if( Which != LastIndex )
            {
                // Move the last one to this spot:
                TActionCombo & Last = List[LastIndex];
                ActionCombo = Last;
            }
            // Leave Which alone to possibly retest current position.
            Count--;
        }
        else
        {
            Which++;
        }
    }
}

//----------------------------------------------------------------------------
//           Remove all definitions for Action.
//----------------------------------------------------------------------------
void FAction::Empty(TAction Action)
{
    for( int Switch = 1; Switch < FInput::SwitchCount; ++Switch )
    {
        SwitchCombos[Switch].Empty(Action);
    }
    for( int Movement = 1; Movement < FInput::MovementCount; ++Movement )
    {
        MovementCombos[Movement].Empty(Action);
    }
}

//----------------------------------------------------------------------------
//              Empty an input action definition
//----------------------------------------------------------------------------
void FAction::TActionCombo::Empty()
{
    TCombo::Empty();
    Action = NoAction;
}

//----------------------------------------------------------------------------
//        Return a list of all combos for the given action.
//----------------------------------------------------------------------------
void FAction::GetCombos
(
    TAction              Action    // The action of interest.
,   int                & Count     // Output: A count of the number of combos returned.
,   FInput::TCombo **    Combos    // Output if !=0: A list of the combos.
)
const
{
    BOOL MakingList = Combos != 0; // TRUE if making a list, FALSE if just counting.
    if( MakingList )
    {
        *Combos = 0;
        GetCombos(Action, Count, 0); // Recursive call to get the count (no list is built here).
        if( Count != 0 )
        {
            *Combos = appMallocArray( Count, FInput::TCombo, "Action Combos" );
            Count = 0; // Reset the count - we'll count again below.
        }
    }
    const TActionCombos * ActionCombosList = SwitchCombos; // Note: 0'th entry is unused.
    int ActionCombosCount = FInput::SwitchCount; // Count of ActionCombos, including unused first entry.
    // Loop twice: Once for the switch combos, once for the movement combos:
    for( int WhichComboList = 1; WhichComboList <= 2; ++WhichComboList )
    {
        // Look for all combos which are mapped to the action:
        for( int WhichCombos = 0; WhichCombos < ActionCombosCount; ++WhichCombos )
        {
            const TActionCombos & ActionCombos = ActionCombosList[WhichCombos];
            for( int WhichCombo = 0; WhichCombo < ActionCombos.Count; WhichCombo++ )
            {
                const TActionCombo & ActionCombo = ActionCombos.List[WhichCombo];
                if( ActionCombo.Action == Action )
                {
                    Count++;
                    if( Combos != 0 ) // Does the caller want a list?
                    {
                        (*Combos)[Count-1] = ActionCombo;
                    }
                }
            }
        }
        // Second time through the loop we will use the movements:
        ActionCombosList = MovementCombos;
        ActionCombosCount = FInput::MovementCount; // Count of ActionCombos, including unused first entry.
    }
}

//----------------------------------------------------------------------------
//                Add an action combo.
//----------------------------------------------------------------------------
void FAction::Add( const TActionCombo & ActionCombo )
{
    // We had the combo to the switch or movement list, depending on the combo details.
    if( ActionCombo.IsSwitch() )
    {
        SwitchCombos[ ActionCombo.Switch() ].Add( ActionCombo );
    }
    else
    {
        MovementCombos[ ActionCombo.Movement() ].Add( ActionCombo );
    }
}

//----------------------------------------------------------------------------
// Add a list of combos for the given action.
//----------------------------------------------------------------------------
void FAction::Add
(
    TAction                   Action  // The action of interest.
,   int                       Count   // A count of the number of combos in Combos.
,   const FInput::TCombo *    Combos  // A list of the combos.
)
{
    TActionCombo ActionCombo;
    ActionCombo.Action = Action;
    for( int Which = 0; Which < Count; ++Which )
    {
        (FInput::TCombo&)ActionCombo = Combos[Which];
        Add( ActionCombo );
    }
}

//----------------------------------------------------------------------------
//              What are all the keys?
//----------------------------------------------------------------------------
FAction::TStringList FAction::Keys() const
{
    TStringList Keys;
    for( int Action_ = 1; Action_ < ActionCount; ++Action_ )
    {
        const TAction Action = TAction(Action_);
        Keys.Add( ActionName(Action) );
    }
    return Keys;
}

//----------------------------------------------------------------------------
//           What is the current value for the given key?
//----------------------------------------------------------------------------
char * FAction::Value(const char * Key) const
{
    char Value[FKeyValues::MaxValueLength+1]; // +1 for trailing null.
    Value[0] = 0;
    char * Text = Value; // Place in Value where we are adding text.
    const TAction Action = FindAction(Key);
    if( Action != 0 )
    {
        //----------------------------------------------
        //             Action = ...
        //----------------------------------------------
        BOOL FirstPart = TRUE;
        //tbi: Merge these two loops...

        // Check all switches which are mapped to the action.
        for( int Switch_ = 1; Switch_ < FInput::SwitchCount; ++Switch_ )
        {
            const FInput::TSwitch Switch = FInput::TSwitch(Switch_);
            const TActionCombos & ActionCombos = SwitchCombos[Switch];
            for( int Which = 0; Which < ActionCombos.Count; Which++ )
            {
                TActionCombo & ActionCombo = ActionCombos.List[Which];
                if( ActionCombo.Action == Action )
                {
                    if( !FirstPart )
                    {
                        Text += sprintf( Text, " | " );
                    }
                    const char * Definition = ActionCombo.Text(FALSE);
                    Text += sprintf( Text, "%s", Definition );
                    FirstPart = FALSE;
                }
            }
        }
        // Check all movements which are mapped to the action.
        for( int Movement_ = 1; Movement_ < FInput::MovementCount; ++Movement_ )
        {
            const FInput::TMovement Movement = FInput::TMovement(Movement_);
            const TActionCombos & ActionCombos = MovementCombos[Movement];
            for( int Which = 0; Which < ActionCombos.Count; Which++ )
            {
                TActionCombo & ActionCombo = ActionCombos.List[Which];
                if( ActionCombo.Action == Action )
                {
                    if( !FirstPart )
                    {
                        Text += sprintf( Text, " | " );
                    }
                    const char * Definition = ActionCombo.Text(FALSE);
                    Text += sprintf( Text, "%s", Definition );
                    FirstPart = FALSE;
                }
            }
        }
    }
    else
    {
        //----------------------------------------------
        //           Unrecognized key
        //----------------------------------------------
    }
    return Value[0] == 0 ? 0 : FParse::MakeString(Value);
}

//----------------------------------------------------------------------------
//            Set the value associated with a text key.
//----------------------------------------------------------------------------
BOOL FAction::SetValue(const char * Key, const char * Value)
{
    BOOL IsOkay = FALSE;
    const TAction Action = FindAction(Key);
    const char * Text = Value;
    if( Action != 0 )
    {
        //----------------------------------------------
        //             Action = ...
        //----------------------------------------------
        IsOkay = TRUE;
        Debug("Add action %s: %s", ActionName(Action), Value );
        const char * Text = Value;
        TActionCombo ActionCombo;
        Empty(Action); // Remove any existing definitions.
        BOOL Done = FALSE;
        while( IsOkay && !Done && ActionCombo.Parse(Text) )
        {
            TActionCombos & ActionCombos =
                ActionCombo.IsSwitch()
            ?   SwitchCombos[ActionCombo.Switch()]
            :   MovementCombos[ActionCombo.Movement()]
            ;
            ActionCombo.Action = Action;
            ActionCombos.Add(ActionCombo);
            if( !FParse::StartsWith(Text,'|') ) // '|' separates multiple action definitions.
            {
                Done = TRUE;
            }
        }
        // Make sure the definitions are properly terminated:
        if( IsOkay )
        {
            FParse::SkipWhiteSpace(Text);
            if( Text[0] != 0 && Text[0] != ';' )
            {
                IsOkay = FALSE;
            }
        }
    }
    return IsOkay;
}

static inline void TransformAxis(EMovementAxis Source, EMovementAxis Target, FAction     & Info)
{
    if( Source != Target )
    {
        if( Info.Movements[Source].Analog != 0 )
        {
            Info.Movements[Target].Analog = Info.Movements[Source].Analog;
            Info.Movements[Source].Analog = 0;
        }
        Info.Movements[Target].Differential += Info.Movements[Source].Differential;
        Info.Movements[Source].Differential = 0;
    }
}

template<class T>
static inline void Swap( T & Value1, T & Value2)
{
    const T Saved1 = Value1;
    Value1 = Value2;
    Value2 = Saved1;
}

//----------------------------------------------------------------------------
//                  Transform one action into another
//----------------------------------------------------------------------------
static void Transform
(
    FAction             & Info
,   FAction::TAction      SourceAction
,   FAction::TAction      TargetAction   
)
{
    FAction::TActionStatus & Source = Info.Status[SourceAction];
    FAction::TActionStatus & Target = Info.Status[TargetAction];
    const EMovementAxis SourceAxis = FAction::Axis(SourceAction);
    const EMovementAxis TargetAxis = FAction::Axis(TargetAction);
    if( FAction::CheckStatus(Source,FAction::IsActiveStatus) ) //tbd:&& !FAction::CheckStatus(Target,FAction::IsActiveStatus) )
    {
        Target |= Source;
        TransformAxis( SourceAxis, TargetAxis, Info );
        Source = 0;
    }
}

//----------------------------------------------------------------------------
//                  Swap the meanings of two Status
//----------------------------------------------------------------------------
static void Swap
(
    FAction             & Info
,   FAction::TAction      Action1
,   FAction::TAction      Action2
)
{
//tbd:    FAction::TActionStatus & Info1 = Info.Status[Action1];
//tbd:    FAction::TActionStatus & Info2 = Info.Status[Action2];
//tbd:    FAction::TActionStatus Save1 = Info1;
//tbd:    Info1 = Info2;
//tbd:    Info2 = Save1;
    Swap( Info.Status[Action1], Info.Status[Action2] );
    const EMovementAxis Axis1 = FAction::Axis(Action1);
    const EMovementAxis Axis2 = FAction::Axis(Action2);
    if( Axis1 != 0 && Axis2 != 0 )
    {
        Swap( Info.Movements[Axis1].Analog, Info.Movements[Axis2].Analog );
        Swap( Info.Movements[Axis1].Differential, Info.Movements[Axis2].Differential );
    }
}

//----------------------------------------------------------------------------
//            Transform a movement-activated action into another action
//----------------------------------------------------------------------------
static void TransformMovement
(
    FAction             & Info
,   FAction::TAction      SourceAction
,   FAction::TAction      TargetAction   
)
{
    FAction::TActionStatus & Source = Info.Status[SourceAction];
    FAction::TActionStatus & Target = Info.Status[TargetAction];
    const EMovementAxis SourceAxis = FAction::Axis(SourceAction);
    const EMovementAxis TargetAxis = FAction::Axis(TargetAction);
    if( FAction::CheckStatus(Source,FAction::IsActiveStatus) && !FAction::CheckStatus(Target,FAction::IsActiveStatus) )
    {
        FAction::SetStatus( Target, FAction::IsActiveStatus );
        TransformAxis( SourceAxis, TargetAxis, Info );
        if( !FAction::CheckBySwitch(Source) ) // If the source wasn't also switch-activated...
        {
            Source = 0;
        }
    }
}

//----------------------------------------------------------------------------
//      Transform an action into another, except for mouse movements.
//----------------------------------------------------------------------------
static void TransformExceptForMovements
(
    FAction             & Info
,   FAction::TAction      SourceAction
,   FAction::TAction      TargetAction   
)
{
    FAction::TActionStatus & Source = Info.Status[SourceAction];
    FAction::TActionStatus & Target = Info.Status[TargetAction];
    if( FAction::CheckStatus( Source, FAction::IsActiveStatus ) && !FAction::CheckStatus( Target, FAction::IsActiveStatus ) )
    {
        Target = Source;
        FAction::ClearStatus( Source, FAction::BySwitchLikeStatus );
    }
}

//----------------------------------------------------------------------------
//            Transform a keyboard-activated action into another action
//----------------------------------------------------------------------------
static void TransformKeyboard
(
    FAction             & Info
,   FAction::TAction      SourceAction
,   FAction::TAction      TargetAction   
)
{
    FAction::TActionStatus & Source = Info.Status[SourceAction];
    FAction::TActionStatus & Target = Info.Status[TargetAction];
    if( FAction::CheckStatus(Source,FAction::IsActiveStatus) && !FAction::CheckStatus(Target,FAction::IsActiveStatus) && FAction::CheckStatus(Source,FAction::ByKeyboardStatus) )
    {
        Target = Source & (FAction::IsActiveStatus|FAction::ByKeyboardStatus);
        FAction::ClearStatus( Source, FAction::ByKeyboardStatus);
    }
}

//----------------------------------------------------------------------------
//              Empty any movement values.
//----------------------------------------------------------------------------
void FAction::EmptyMovements()
{
    PPlayerMotion::Empty( Movements, PlayerAxis_Count );
}

//----------------------------------------------------------------------------
//              Empty the status of all actions.
//----------------------------------------------------------------------------
void FAction::EmptyActions()
{
    // Empty all active actions (but remember the previous state).
    for( int Action = 1; Action < ActionCount; ++Action )
    {
        EmptyStatus( Status[Action], TRUE );
    }
}

//----------------------------------------------------------------------------
//              See if any mouse switches are active.
//----------------------------------------------------------------------------
void FAction::CheckMouseState(FInput & Input)
{
    // We code a list of switches below just for efficiency.
    // To be general, we should scan all switches and look for those
    // originating from the mouse.
    State.MouseIsActive = 
        State.UsePlainMouseMovements
    ||  Input.IsOn(FInput::S_LeftMouse)
    ||  Input.IsOn(FInput::S_RightMouse)
    ||  Input.IsOn(FInput::S_MiddleMouse)
    ;
}

//----------------------------------------------------------------------------
//              Update action status for actions triggered by switches.
//----------------------------------------------------------------------------
void FAction::UpdateSwitchActions(FInput & Input, BOOL IgnoreTypingKeys )
{
    for( int Switch_ = 1; Switch_ < FInput::SwitchCount; ++Switch_ )
    {
        const FInput::TSwitch Switch = FInput::TSwitch(Switch_);
        TActionCombos & ActionCombos = SwitchCombos[Switch];
        if( !ActionCombos.IsEmpty() && ( !IgnoreTypingKeys || !FInput::IsTypingKey(Switch) ) )
        {
            BOOL ModifiedActionDone = FALSE;
            // First, do any modified action...
            {
                for( int Which = 0; Which < ActionCombos.Count; Which++ )
                {
                    const TActionCombo & ActionCombo = ActionCombos.List[Which];
                    TActionStatus & Status = this->Status[ActionCombo.Action];
                    const FInput::TModifiers & Modifiers = ActionCombo.Modifiers;
                    const FInput::TMetaSwitch & Modifier1 = Modifiers.MetaSwitch1;
                    if( Modifier1.Switch != 0 && Input.IsOn(Modifier1.Switch,Modifier1.IsDouble,Modifier1.IsToggle) )
                    {
                        SetStatus( Status, IsReadyStatus );
                        if( !CheckBySwitch(Status) && Input.IsPending(Switch,ActionCombo.MetaSwitch.IsDouble,ActionCombo.MetaSwitch.IsToggle) )
                        {
                            SetStatus( Status, IsActiveStatus );
                            if( FInput::Device(Switch) == FInput::KeyboardDevice )
                            {
                                SetStatus( Status, ByKeyboardStatus );
                            }
                            else
                            {
                                SetStatus( Status, ByButtonStatus );
                            }
                            ModifiedActionDone  = TRUE;
                            if( ShowActions ) { debugf(LOG_Info,"Action: %s",ActionName(ActionCombo.Action)); }
                        }
                    }
                }
            }

            // If no modified actions were done, do any unmodified actions.
            if( !ModifiedActionDone )
            {
                for( int Which = 0; Which < ActionCombos.Count; Which++ )
                {
                    const TActionCombo & ActionCombo = ActionCombos.List[Which];
                    TActionStatus & Status = this->Status[ActionCombo.Action];
                    const FInput::TModifiers & Modifiers = ActionCombo.Modifiers;
                    const FInput::TMetaSwitch & Modifier1 = Modifiers.MetaSwitch1;
                    if( Modifier1.Switch == 0 ) // Unmodified action?
                    {
                        if( !CheckBySwitch(Status) && Input.IsPending(Switch,ActionCombo.MetaSwitch.IsDouble,ActionCombo.MetaSwitch.IsToggle) )
                        {
                            //todo: Clean this test up. Temporary fix(?) for intel version - disable left mouse
                            // button when running in a window.
                            if( State.UsePlainMouseMovements || Switch!=FInput::S_LeftMouse || ActionCombo.MetaSwitch.IsDouble )
                            {
                                SetStatus( Status, IsActiveStatus );
                                if( FInput::Device(Switch) == FInput::KeyboardDevice )
                                {
                                    SetStatus( Status, ByKeyboardStatus );
                                }
                                else
                                {
                                    SetStatus( Status, ByButtonStatus );
                                }
                                if( ShowActions ) { debugf(LOG_Info,"Action: %s",ActionName(ActionCombo.Action)); }
                            }
                        }
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
//              Update action status for actions triggered by movements.
//----------------------------------------------------------------------------
void FAction::UpdateMovementActions(FInput & Input)
{
    for( int Movement_ = 1; Movement_ < FInput::MovementCount; ++Movement_ )
    {
        const FInput::TMovement Movement = FInput::TMovement(Movement_);
        TActionCombos & ActionCombos = MovementCombos[Movement];
        const BOOL UseIt =
            !FInput::IsMouseMovement( Movement )
        ||  State.MouseIsActive
        ;
        if( !ActionCombos.IsEmpty() )
        {
            BOOL ModifiedActionDone = FALSE;
            BOOL ModifiedActionReady = FALSE;
            // First, do any modified movements.
            {
                for( int Which = 0; Which < ActionCombos.Count; Which++ )
                {
                    const TActionCombo & ActionCombo = ActionCombos.List[Which];
                    TActionStatus & Status = this->Status[ActionCombo.Action];
                    const FInput::TModifiers & Modifiers = ActionCombo.Modifiers;
                    const FInput::TMetaSwitch & Modifier1 = Modifiers.MetaSwitch1;
                    if( Modifier1.Switch != 0 && Input.IsOn(Modifier1.Switch,Modifier1.IsDouble,Modifier1.IsToggle) )
                    {
                        FLOAT Magnitude;
                        BOOL MovementIsActive;
                        Input.InterpretMovement(Magnitude,MovementIsActive,Movement,ActionCombo.MetaMovement.Kind);
                        SetStatus( Status, IsReadyStatus );
                        ModifiedActionReady = TRUE;
                        if( UseIt && MovementIsActive )
                        {
                            SetStatus( Status, IsActiveStatus );
                            EMovementAxis Axis = this->Axis(ActionCombo.Action);
                            if( Magnitude == 0 || Axis == 0 )
                            {
                                SetStatus( Status, ByDigitalMovement );
                            }
                            else if( FInput::IsAnalog(ActionCombo.MetaMovement.Kind) )
                            {
                                Movements[Axis].Analog = Magnitude * AnalogFactor(ActionCombo.Action);
                            }
                            else if( FInput::IsDifferential(ActionCombo.MetaMovement.Kind) )
                            {
                                Movements[Axis].Differential += Magnitude * DifferentialFactor(ActionCombo.Action);
                            }
                            if( ShowActions ) { debugf(LOG_Info,"Action: %s %3.3f (%s)",ActionName(ActionCombo.Action),Magnitude, FInput::Abbreviation(ActionCombo.MetaMovement.Kind)); }
                            ModifiedActionDone = TRUE;
                        }
                    }
                }
            }
            // If no modified actions were done (or are "ready"), do any unmodified actions.
            if( !ModifiedActionDone && !ModifiedActionReady )
            {
                for( int Which = 0; Which < ActionCombos.Count; Which++ )
                {
                    const TActionCombo & ActionCombo = ActionCombos.List[Which];
                    TActionStatus & Status = this->Status[ActionCombo.Action];
                    const FInput::TModifiers & Modifiers = ActionCombo.Modifiers;
                    const FInput::TMetaSwitch & Modifier1 = Modifiers.MetaSwitch1;
                    if( UseIt && Modifier1.Switch == 0 ) // Unmodified action?
                    {
                        FLOAT Magnitude;
                        BOOL MovementIsActive;
                        Input.InterpretMovement(Magnitude,MovementIsActive,Movement,ActionCombo.MetaMovement.Kind);
                        if( MovementIsActive )
                        {
                            SetStatus( Status, IsActiveStatus );
                            EMovementAxis Axis = this->Axis(ActionCombo.Action);
                            if( Magnitude == 0 || Axis == 0 )
                            {
                                SetStatus( Status, ByDigitalMovement );
                            }
                            else if( FInput::IsAnalog(ActionCombo.MetaMovement.Kind) )
                            {
                                Movements[Axis].Analog = Magnitude * AnalogFactor(ActionCombo.Action);
                            }
                            else if( FInput::IsDifferential(ActionCombo.MetaMovement.Kind) )
                            {
                                Movements[Axis].Differential += Magnitude * DifferentialFactor(ActionCombo.Action);
                            }
                            if( ShowActions ) { debugf(LOG_Info,"Action: %s %3.3f (%s) %s",ActionName(ActionCombo.Action),Magnitude, FInput::Abbreviation(ActionCombo.MetaMovement.Kind), FInput::DeviceName(FInput::Device(Movement)) ); }
                        }
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
//              Update actions which are always on.
//----------------------------------------------------------------------------
void FAction::UpdateFixedActions()
{
    if( GPreferences.MouseLookAlwaysOn && State.MouseIsActive )
    {
        const TAction Action = MotionLookShift;
        TActionStatus & Info = Status[Action];
        if( !CheckStatus( Info, IsActiveStatus ) )
        {
            SetStatus( Info, IsActiveStatus );
            SetStatus( Info, ByButtonStatus ); // Pretend it was activated by a button.
        }
    }

    if( GPreferences.RunAlwaysOn )
    {
        const TAction Action = RunShift;
        TActionStatus & Info = Status[Action];
        if( !CheckStatus( Info, IsActiveStatus ) )
        {
            SetStatus( Info, IsActiveStatus );
            SetStatus( Info, ByButtonStatus ); // Pretend it was activated by a button.
        }
    }

}

//----------------------------------------------------------------------------
//                Do transformations on actions
//----------------------------------------------------------------------------
void FAction::TransformActions()
{
    if( CheckStatus( Status[MotionLookShift], IsActiveStatus ) )
    {
        // Transforms movement-activated moves and runs into turns, spins, and looks.
        TransformMovement( *this, MoveForward  , LookUp      );
        TransformMovement( *this, MoveBackward , LookDown    );
        TransformMovement( *this, RunForward   , LookUp      );
        TransformMovement( *this, RunBackward  , LookDown    );
    }
    if( CheckStatus( Status[KeyboardLookShift], IsActiveStatus ) )
    {
        // Transforms keyboard-activated moves and runs into turns, spins, and looks.
        TransformKeyboard( *this, MoveForward  , LookUp      );
        TransformKeyboard( *this, MoveBackward , LookDown    );
        TransformKeyboard( *this, RunForward   , LookUp      );
        TransformKeyboard( *this, RunBackward  , LookDown    );
    }
    if( CheckStatus( Status[SlideShift], IsActiveStatus ) )
    {
        // Converts turns and spins to left/right movements
        Transform( *this, TurnLeft    , MoveLeft     );
        Transform( *this, TurnRight   , MoveRight    );
        Transform( *this, SpinLeft    , RunLeft      );
        Transform( *this, SpinRight   , RunRight     );
    }
    if( CheckStatus( Status[RunShift], IsActiveStatus ) )
    {
        // Transform movements into corresponding runs.
        Transform( *this, MoveBackward  , RunBackward    );
        Transform( *this, MoveDown      , RunDown        );
        Transform( *this, MoveForward   , RunForward     );
        Transform( *this, MoveLeft      , RunLeft        );
        Transform( *this, MoveRight     , RunRight       );
        Transform( *this, MoveUp        , RunUp          );
    }
    if( CheckStatus( Status[SpinShift], IsActiveStatus ) )
    {
        // Transform turns into spins (which are faster).
        TransformExceptForMovements( *this, TurnLeft    , SpinLeft     );
        TransformExceptForMovements( *this, TurnRight   , SpinRight    );
        TransformExceptForMovements( *this, TurnUp      , SpinUp       );
        TransformExceptForMovements( *this, TurnDown    , SpinDown     );
    }
    if( GPreferences.ReverseUpAndDown )
    {
        Swap( *this, LookUp, LookDown );
        Swap( *this, TurnUp, TurnDown );
        Swap( *this, SpinUp, SpinDown );
    }
}

//----------------------------------------------------------------------------
//        Check inputs and update the status of actions.
//----------------------------------------------------------------------------
void FAction::UpdateStatus(FInput & Input, BOOL IgnoreTypingKeys )
{
    Input.GatherInput();

    EmptyActions();
    EmptyMovements();
    CheckMouseState(Input);

    UpdateSwitchActions(Input,IgnoreTypingKeys);
    UpdateMovementActions(Input);
    UpdateFixedActions();

    // Mark all newly activated actions as pending.
    for( int Action_ = 1; Action_ < ActionCount; ++Action_ )
    {
        const TAction Action = TAction( Action_ );
        TActionStatus & Info = Status[Action];
        if( CheckStatus( Info, IsActiveStatus ) && !CheckStatus( Info, WasActiveStatus ) )
        {
            SetStatus( Info, IsPendingStatus );
        }
    }

    TransformActions();

    Input.StartNextInputCycle();

}
