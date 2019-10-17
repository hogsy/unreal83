/*==============================================================================
UnAction.h: Player input actions
Used by: Player processing code

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:

Revision history:
    * 05/30/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnAction
#define _INC_UnAction

#include "Unreal.h"
#include "UnInput.h"
#include <stdio.h>

class UNREAL_API FAction
    : public FKeyValues     // We provide a key-value interface.
{
public:

    // A kind of action...
    // Actions marked with [F] apply only when flying or floating.
    // Actions might have associated values.
    // Be careful about adding, removing, or rearranging these enumeration values.
    // There are arrays expecting this order (look for arrays of size ActionCount).
    typedef enum
    {
        NoAction        = 0     // No action (always 0).
    ,   Accelerate              // Speed up.
    ,   BankLeft                // [F] Bank left at normal speed.
    ,   BankRight               // [F] Bank right at normal speed.
    ,   Chat                    // Send a chat message
    ,   ConsoleFull             // Full-sized console.
    ,   ConsoleHalf             // Half-sized console.
    ,   ConsoleType             // Type a console command
    ,   Crouch                  // Crouch down to the ground.
    ,   Decelerate              // Slow down.
    ,   FullScreen              // Toggles full-screen view
    ,   Jump                    // Jump upwards.
    ,   KeyboardLookShift       // Change moves (originating from keyboard) into looks and turns.
    ,   Kick                    // Kick action.
    ,   LookDown                // Tilt the view up
    ,   LookFromBehind          // Change view to behind player pawn.
    ,   LookStraight            // Look straight ahead (removes any up/down tilt or roll).
    ,   LookUp                  // Tilt the view up
    ,   LungeBackward           // Move backward with a sudden, short burst of speed.
    ,   LungeDown               // [F] Move down with a sudden, short burst of speed.
    ,   LungeForward            // Move forward with a sudden, short burst of speed.
    ,   LungeLeft               // Move left with a sudden, short burst of speed.
    ,   LungeRight              // Move right with a sudden, short burst of speed.
    ,   LungeUp                 // [F] Move up with a sudden, short burst of speed.
    ,   MotionLookShift         // Change mouse/joystick motions into looks and turns.
    ,   MoveBackward            // Move backward at normal speed.
    ,   MoveDown                // [F] Move down at normal speed.
    ,   MoveForward             // Move forward at normal speed.
    ,   MoveLeft                // Move left at normal speed.
    ,   MoveRight               // Move right at normal speed.
    ,   MoveUp                  // [F] Move up at normal speed.
    ,   Pause                   // Pause or unpause the game
    ,   RollLeft                // [F] Bank left at faster speed.
    ,   RollRight               // [F] Bank right at faster speed.
    ,   RunShift                // Changes moves into faster moves.
    ,   RunBackward             // Move backwards fast.
    ,   RunDown                 // [F] Move down fast.
    ,   RunForward              // Move forward fast.
    ,   RunLeft                 // Move left fast.
    ,   RunRight                // Move right fast.
    ,   RunUp                   // [F] Move up fast.
    ,   ScreenEnlarge           // Make the game window larger
    ,   ScreenShot              // Take a snapshot of the screen.
    ,   ScreenShrink            // Make the game window smaller
    ,   SlideShift              // Change turns and looks to moves.
    ,   SpinShift               // Changes turns into spins.
    ,   SpinDown                // [F] Turn down at faster speed.
    ,   SpinLeft                // Turn left at faster speed.
    ,   SpinRight               // Turn right at faster speed.
    ,   SpinUp                  // [F] Turn up at faster speed.
    ,   TurnDown                // [F] Turn down at normal speed.
    ,   TurnLeft                // Turn left at normal speed.
    ,   TurnRight               // Turn right at normal speed.
    ,   TurnUp                  // [F] Turn up at normal speed.
    ,   WeaponFire              // Use current weapon's primary function.
    ,   WeaponSpecial           // Use current weapon's secondary function.
    ,   WeaponCloseUp           // Use current weapon's tertiary function (close-up attack).
    ,   WeaponNext              // Select next weapon (in some well-defined sequence).
    ,   WeaponPrevious          // Select previous weapon (in some well-defined sequence).
    ,   WeaponReady             // Lowers or raises the current weapon.
        // Note: WeaponSet1..WeaponSet10 must be sequential.
    ,   WeaponSet1              // Select weapon set #1, or cycle within set.
    ,   WeaponSet2              // Select weapon set #2, or cycle within set.
    ,   WeaponSet3              // Select weapon set #3, or cycle within set.
    ,   WeaponSet4              // Select weapon set #4, or cycle within set.
    ,   WeaponSet5              // Select weapon set #5, or cycle within set.
    ,   WeaponSet6              // Select weapon set #6, or cycle within set.
    ,   WeaponSet7              // Select weapon set #7, or cycle within set.
    ,   WeaponSet8              // Select weapon set #8, or cycle within set.
    ,   WeaponSet9              // Select weapon set #9, or cycle within set.
    ,   WeaponSet10             // Select weapon set #10, or cycle within set.
    ,   WeaponSwap              // Switch between 2 most recently used weapons.
    ,   Zoom                    // Zooms view (while held)
    ,   ActionCount             // Count of all actions, including NoAction.
    }
    TAction;

    static const char * ActionName(TAction Action);
    static const char * ActionHelp(TAction Action); // Descriptive "help text" for an action.

    // Classification of action... (useful for user interface)
    typedef enum
    {
        NoClassification    // Use to indicate unimplemented (unclassified) action. Always 0.
    ,   MovingAction        // Classifies movement actions (move forward, move left, ... )
    ,   TurningAction       // Classifies turning actions.
    ,   WeaponAction        // Classifies weapon selection and use.
    ,   AdministrativeAction // Classifies administrative actions (save game, ...).
    ,   MiscellaneousAction // Classifies miscellaneous actions
    ,   AdvancedAction      // Classifies advanced actions.
    ,   ClassificationCount // Number of classifications, including NoClassification.
    }
    TClass;

    static TClass Class(TAction Action);
    static TAction Opposite(TAction Action); // What is the "opposite" action of Action (returns NoAction if none).
    static EMovementAxis Axis(TAction Action); // What axis is affected Action (0 if none).
    static BOOL IsPositiveAxis(TAction Action); // TRUE iff Action affects an axis in the positive direction.

    struct UNREAL_API TActionCombo // An input combo with associated action.
        : FInput::TCombo
    {
        TAction Action; // The action to be performed.
        void Empty();
    };

    struct UNREAL_API TActionCombos // A list of action combos.
    {
        int MaxCount : 16 ; // Maximum actions that can currently fit in List.
        int Count    :  8 ; // Number of action combos currently in List[0], List[1], ..., List[Count-1]
        TActionCombo * List;
        void Initialize() { MaxCount = 0; Count = 0; List = 0; }
        BOOL IsEmpty() const { return Count==0; }
        void Add(const TActionCombo & ActionCombo);
        BOOL Has(const TActionCombo & ActionCombo) const; // Is the specified combo somewhere in the list of combos?
        void Empty(TAction Action); // Remove all definitions for the given action.
        void Empty() { Count = 0; }
        void Free(); 
        void Finalize() { Free(); }
    private:
        void MakeRoom();
    };

    // Return a list of all combos for the given action.
    void GetCombos
    (
        TAction              Action    // The action of interest.
    ,   int                & Count     // Output: A count of the number of combos returned.
    ,   FInput::TCombo **    Combos    // Output if !=0: A list of the combos.
        // Note: If Combos!=0 && *Combos!=0, then *Combos should be freed when no longer needed.
        // On output, if Count=0  && Combos != 0, then *Combos = 0.
    )
    const;

    void Add( const TActionCombo & ActionCombo );

    // Add a list of combos for the given action.
    void Add
    (
        TAction                   Action  // The action of interest.
    ,   int                       Count   // A count of the number of combos in Combos.
    ,   const FInput::TCombo *    Combos  // A list of the combos.
    );

    TActionCombos SwitchCombos  [FInput::SwitchCount  ] ; // Action combos associated with switches.
    TActionCombos MovementCombos[FInput::MovementCount] ; // Action combos associated with movements.

    typedef BYTE TActionStatus; // Defines the status of an action.

    enum // Bit flags used to form values of TActionStatus
    {
        IsActiveStatus           = 0x01 // The action is currently active.
    ,   IsReadyStatus            = 0x02 // The action has a modifier which is active, even if the action itself isn't active.
    ,   WasActiveStatus          = 0x04 // Previously, the action was active.
    ,   IsPendingStatus          = 0x08 // The action is pending (it has not been processed yet).
    ,   ByKeyboardStatus         = 0x10 // The action was activated by a keyboard switch.
    ,   ByButtonStatus           = 0x20 // The action was activated by a button (mouse or joystick)
    ,   ByDigitalMovement        = 0x40 // The action was activated by a digital movement.
    ,   ByAnySwitchStatus        = ByKeyboardStatus | ByButtonStatus
    ,   BySwitchLikeStatus       = ByAnySwitchStatus | ByDigitalMovement // By switch, including movements translated to switch-like activation.
    ,   AllStatus                = 0xff
    };

    static BOOL CheckStatus(TActionStatus Status, TActionStatus Flag)
    {
        return ( Status & Flag ) != 0;
    }

    static BOOL CheckBySwitch(TActionStatus Status)
    {
        return CheckStatus(Status,ByAnySwitchStatus);
    }

    static void SetStatus(TActionStatus & Status, TActionStatus Flags)
    {
        Status |= Flags;
    }

    static void ClearStatus(TActionStatus & Status, TActionStatus Flags)
    {
        Status &= ~Flags;
    }

    static BOOL CheckAnyStatus(TActionStatus Status, TActionStatus Flags)
    {
        return ( Status & Flags ) != 0;
    }

    static BOOL CheckAllStatus(TActionStatus Status, TActionStatus Flags)
    {
        return ( Status & Flags ) == Flags;
    }

    TActionStatus Status[ActionCount];
    void EmptyActions(); // Clear out any status for actions in this->Status.

    PPlayerMotion Movements[PlayerAxis_Count];
    void EmptyMovements(); // Clear out any values in this->Movements.

    // Indexing *this by an action gives the status of the action.
    const TActionStatus & operator[](TAction Action) const { return Status[Action]; }
    TActionStatus & operator[](TAction Action) { return Status[Action]; }

    void Initialize();
    void Finalize();
    void Empty(); // Clear out all definitions.
    void Empty(TAction Action); // Remove all definitions for Action.
    void Reset(); // Reset the state of switches, and status of all actions.

    static BOOL Do(TActionStatus & Status) // Use this to check for a pending action and clear the pending condition.
    {
        const BOOL WasPending = CheckStatus(Status,IsPendingStatus);
        ClearStatus( Status, IsPendingStatus );
        return WasPending;
    }

    BOOL Do(TAction Action) // Use this to check for a pending action and clear the pending condition.
    {
        return Do( Status[Action] );
    }

    void EmptyStatus(TActionStatus & Status, BOOL RememberState) // Clear the status, possibly remembering the previous state.
    {
        const TActionStatus OldStatus = Status;
        Status = 0;
        if( RememberState )
        {
            SetStatus( Status, OldStatus & IsPendingStatus );
            if( CheckStatus(OldStatus,IsActiveStatus) )
            {
                SetStatus( Status, WasActiveStatus );
            }
            if( CheckStatus(OldStatus,IsActiveStatus) )
            {
                SetStatus( Status, WasActiveStatus );
            }
        }        
    }

    void UpdateStatus(FInput & Input, BOOL IgnoreTypingKeys = FALSE);
        // Using all of the input gathered to this time, interpret the 
        // input and set this->Status to reflect the status of all actions.
        // If IgnoreTypingKeys, actions triggered with a keyboard "typing key"
        // for the main input are suppressed. See FPlatformInput for a 
        // definition of typing keys.

    //            Key-Value pairs for input.
    // The recognized keys are not documented here because they tend to
    // change and any list here will soon be obsolete. Instead, the
    // Keys() function can be used to get a list.

    // Implementations of the FKeyValues interface:
    TStringList Keys() const; // What are all the keys?
    char * Value(const char * Key) const; // What is the current value for the given key? 
    BOOL SetValue(const char * Key, const char * Value); // Set the value associated with a key.

    void UsePlainMouseMovements( BOOL Use ) { State.UsePlainMouseMovements = Use; }
        // This is a bit of a hack, but sometimes we want to ignore
        // plain (unmodified) mouse movements, such as when playing in
        // a window on the desktop, where mouse movements are greatly
        // exaggerated as the mouse cursor enters or leaves the window.

    void UpdateSwitchActions(FInput & Input, BOOL IgnoreTypingKeys); // Update state of actions triggered by switches.
    void UpdateMovementActions(FInput & Input); // Update movements and state of actions triggered by movements.
    void UpdateFixedActions(); // Update state of actions which are always on.
    void TransformActions(); // Do transformations on actions (move to turn, turn to slide, and so on)

private:
    struct
    {
        BOOL UsePlainMouseMovements ;
        BOOL MouseIsActive          ; // True if UsePlainMouseMovements or if a mouse switch is active.
    }
    State;
    void CheckMouseState(FInput & Input); // Sets State.MouseIsActive if a mouse switch is active or if UsePlainMouseMovements.
    
};

extern UNREAL_API FAction GAction;

#endif
