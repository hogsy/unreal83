/*==============================================================================
UnInputP.h: Unreal platform-specific input 

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:

    This has the platform-dependent Unreal input specifics.
    A switch is an input which is on or off, such as:
       - a joystick button
       - a keyboard key
       - a mouse button
       - a digital joystick (in a particular direction)
    A movement is a device which provides a relative or absolute
    motion, such as:
       - a mouse
       - a joystick

    The different kinds of switches and movements are platform-specific
    and specified in this file. However, certain fundamental values
    and types must be provided for the platform-independent input
    processing. Mostly, the values needed are counts of the
    number of different inputs. 

    Parts that are expected by other layers (and must be provided for
    each platform) are marked with [Required].

Revision history:
    * 06/01/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnInputP
#define _INC_UnInputP

#include "Unreal.h"

class UNREAL_API FPlatformInput
{
public:
    enum { MaxJoystickButtonCount = 10 };
    typedef enum
    {
        // Note: All TSwitch values must fit in a BYTE
        // Be careful if you add, remove, or rearrange these enumeration values.
        // There are arrays which expect this order (look for arrays of size SwitchCount.
        S_None      = 0     // [Required] No switch (always 0)
        //----------------------------------------------------------
        //            Platform-specific switches
        //----------------------------------------------------------
    ,   S_Space // Space bar
        // Keyboard function keys... (must be sequential)
    ,   S_F1    
    ,   S_F2   
    ,   S_F3   
    ,   S_F4
    ,   S_F5   
    ,   S_F6   
    ,   S_F7   
    ,   S_F8
    ,   S_F9   
    ,   S_F10  
    ,   S_F11  
    ,   S_F12
        // Keyboard number keys: (must be sequential)
    ,   S_0   
    ,   S_1   
    ,   S_2   
    ,   S_3
    ,   S_4   
    ,   S_5   
    ,   S_6   
    ,   S_7
    ,   S_8   
    ,   S_9
        // Keyboard letter keys: (must be sequential)
    ,   S_A   
    ,   S_B   
    ,   S_C   
    ,   S_D   
    ,   S_E
    ,   S_F   
    ,   S_G   
    ,   S_H   
    ,   S_I   
    ,   S_J
    ,   S_K   
    ,   S_L   
    ,   S_M   
    ,   S_N   
    ,   S_O
    ,   S_P   
    ,   S_Q   
    ,   S_R   
    ,   S_S   
    ,   S_T
    ,   S_U   
    ,   S_V   
    ,   S_W   
    ,   S_X   
    ,   S_Y
    ,   S_Z
        // Keyboard numeric keypad keys:
    ,   S_NumLock
    ,   S_N0   
    ,   S_N1   
    ,   S_N2   
    ,   S_N3
    ,   S_N4   
    ,   S_N5   
    ,   S_N6   
    ,   S_N7
    ,   S_N8   
    ,   S_N9
    ,   S_NDivide
    ,   S_NTimes
    ,   S_NMinus
    ,   S_NPlus
    ,   S_NPeriod
    ,   S_NEnter
        // Keyboard punctuation/bracket keys:
    ,   S_Tilde             // Back tick / tilde key
    ,   S_Minus             // Underscore/minus key
    ,   S_Equals            // Plus/equals key
    ,   S_LeftBracket       // Left square bracket key
    ,   S_RightBracket      // Right square bracket key
    ,   S_BackSlash         // Backslash/vertical bar key
    ,   S_SemiColon         // Semicolon/colon key
    ,   S_Quote             // Single quote/double quote key
    ,   S_Comma             // Less than/comma key
    ,   S_Period            // Greater than/period key
    ,   S_Slash             // Question mark/slash key
        // Keyboard shift keys:
    ,   S_CapsLock          // Caps lock key
    ,   S_Alt               // Unified Alt (represents either Alt key)
    ,   S_Ctrl              // Unified Ctrl (represents either Ctrl key)
    ,   S_Shift             // Unified Shift (represents either Shift key)
        // Keyboard editing/cursor/scrolling keys:
    ,   S_Backspace
    ,   S_Insert
    ,   S_Delete
    ,   S_Home
    ,   S_End
    ,   S_PageUp
    ,   S_PageDown
    ,   S_CursorLeft
    ,   S_CursorRight
    ,   S_CursorUp
    ,   S_CursorDown
        // Keyboard special keys:
    ,   S_PrintScrn
    ,   S_ScrollLock
    ,   S_Pause
    ,   S_Enter
    ,   S_Escape
    ,   S_Tab
        // Mouse buttons:
    ,   S_LeftMouse        
    ,   S_RightMouse        
    ,   S_MiddleMouse        
        // Joystick #1 buttons: These must be contiguous and there must be MaxJoystickButtonCount of them
    ,   S_J1B1
    ,   S_J1B2
    ,   S_J1B3
    ,   S_J1B4
    ,   S_J1B5
    ,   S_J1B6
    ,   S_J1B7
    ,   S_J1B8
    ,   S_J1B9
    ,   S_J1B10
        // Joystick #1 Hat Switch, discrete positions. These must be contiguous and in the same order as those for joystick #2.
    ,   S_J1HatOn        // Hat set to any position (not off).
    ,   S_J1HatN
    ,   S_J1HatS
    ,   S_J1HatW
    ,   S_J1HatE
    ,   S_J1HatNW
    ,   S_J1HatNE
    ,   S_J1HatSW
    ,   S_J1HatSE
        // Joystick #2 Buttons: These must be contiguous, and there must be MaxJoystickButtonCount of them
    ,   S_J2B1
    ,   S_J2B2
    ,   S_J2B3
    ,   S_J2B4
    ,   S_J2B5
    ,   S_J2B6
    ,   S_J2B7
    ,   S_J2B8
    ,   S_J2B9
    ,   S_J2B10
        // Joystick #2 Hat Switch, discrete positions. These must be contiguous and in the same order as those for joystick #1.
    ,   S_J2HatOn    // Hat set to any position (not off).
    ,   S_J2HatN
    ,   S_J2HatS
    ,   S_J2HatW
    ,   S_J2HatE
    ,   S_J2HatNW
    ,   S_J2HatNE
    ,   S_J2HatSW
    ,   S_J2HatSE
        //----------------------------------------------------------
        //            End of platform-specific switches
        //----------------------------------------------------------
    ,   SwitchCount     // [Required] Number of kinds of switches, including S_None.
    }
    TSwitch; // [Required]

    // The abbreviations are used in .ini files where brevity may be important to keep 
    // line lengths reasonable. The descriptive names are used in the user interface.
    static const char * Abbreviation(TSwitch Switch); // [Required]
    static const char * Description(TSwitch Switch); // [Required]
    static BOOL IsTypingKey(TSwitch Switch); // [Required] Is the key used for console typing?
    
    typedef enum
    {
        // Note: All TMovement values must fit in a BYTE
        // Be careful if you add, remove, or rearrange these enumeration values.
        // There are arrays which expect this order (look for arrays of size MovementCount.
        M_None      = 0     // [Required] No movement (always 0)
        //----------------------------------------------------------
        //            Platform-specific movements
        //----------------------------------------------------------
        // All movements are listed in consecutive pairs of positive/negative motions,
        // the positive motion being first.
        // The mouse movements must be consecutive from FirstMouseMovement to LastMouseMovement:
    ,   FirstMouseMovement // Keep this accurate! //todo: Delete!
    ,   M_MouseR        = FirstMouseMovement // Mouse right.
    ,   M_MouseL        // Mouse left.
    ,   M_MouseB        // Mouse backward.
    ,   M_MouseF        // Mouse forward.
    ,   LastMouseMovement = M_MouseF // Keep this accurate! //todo:Delete!
        // Joystick #1 movements, positive/negative values.
        // These must be contiguous in positive/negative pairs, and must match the similar values for joystick #2.
        // The negative value for an axis immediately follows the positive axis.
    ,   M_J1XP          // Positive values for axis 1 (X)
    ,   M_J1XN          // Negative values for axis 1 (X)
    ,   M_J1YP          // Positive values for axis 2 (Y)
    ,   M_J1YN          // Negative values for axis 2 (Y)
    ,   M_J1ZP          // Positive values for axis 3 (Z)
    ,   M_J1ZN          // Negative values for axis 3 (Z)
    ,   M_J1RP          // Positive values for axis 4 (Rudder)
    ,   M_J1RN          // Negative values for axis 4 (Rudder)
        // Joystick #2 movements, positive/negative values.
        // See the comments above for joystick #1 movements - 
        // the following values are organized the same way.
    ,   M_J2XP          // Positive values for axis 1 (X)
    ,   M_J2XN          // Negative values for axis 1 (X)
    ,   M_J2YP          // Positive values for axis 2 (Y)
    ,   M_J2YN          // Negative values for axis 2 (Y)
    ,   M_J2ZP          // Positive values for axis 3 (Z)
    ,   M_J2ZN          // Negative values for axis 3 (Z)
    ,   M_J2RP          // Positive values for axis 4 (Rudder)
    ,   M_J2RN          // Negative values for axis 4 (Rudder)
        //----------------------------------------------------------
        //            End of platform-specific movements
        //----------------------------------------------------------
    ,   MovementCount       // [Required] Number of kinds of movements, including M_None.
    }
    TMovement; // [Required]

    static BOOL IsMouseMovement(TMovement Movement)
    {
        return Movement >= FirstMouseMovement && Movement <= LastMouseMovement;
    }

    // The abbreviations are used in .ini files where brevity may be important to keep 
    // line lengths reasonable. The descriptive names are used in the user interface.
    static const char * Abbreviation(TMovement Movement); // [Required]
    static const char * Description(TMovement Movement); // [Required]
    static const char * NeutralDescription(TMovement Movement); // [Required]
    static BOOL IsPositive(TMovement Movement); // Is the movement a positive (not negative) movement? [Required]


    static TMovement OppositeMovement(TMovement Movement); // Returns the similar but opposite movement.    
    // Map a Windows VK_... key identifier to a switch. A value of 0
    // means the key has no corresponding switch.
    static TSwitch WindowsKeySwitches[256];

    FPlatformInput(); 
    void Initialize(); // [Required]
    void Finalize();   // [Required]
    void Reset(); // [Required] Reset the state of all switches, movements.

    typedef enum
    {
        // Note: All device values must fit in a BYTE
        // Be careful if you add, remove, or rearrange these enumeration values.
        // There are arrays which expect this order (look for arrays of size DeviceCount.
        //----------------------------------------------------------
        //            Platform-specific devices
        //----------------------------------------------------------
        KeyboardDevice
    ,   MouseDevice
    ,   Joystick1Device
    ,   Joystick2Device
        //----------------------------------------------------------
        //            End of platform-specific device
        //----------------------------------------------------------
    ,   DeviceCount       // [Required] Number of kinds of devices.
    }
    TDevice; // [Required]
    static const char * DeviceName(TDevice Device);
    void UseDevice( TDevice Device, BOOL Use = TRUE ); // Use or ignore a device for input.
    BOOL UsingDevice( TDevice Device ) const; // Are we using Device for input?
    void CaptureDevices(BOOL Capture = TRUE); // For all devices in use, start or stop capturing input.
    BOOL CapturingDevice(TDevice Device) const; // Are we using and capturing input from a device?
    BOOL CapturingMouse() const { return CapturingDevice(MouseDevice); }
    BOOL CapturingKeyboard() const { return CapturingDevice(KeyboardDevice); }
    BOOL CapturingJoystick1() const { return CapturingDevice(Joystick1Device); }
    BOOL CapturingJoystick2() const { return CapturingDevice(Joystick2Device); }
    BOOL CapturingJoystick() const { return CapturingJoystick1() || CapturingJoystick2(); }

    FLOAT AnalogThreshold      (TMovement Movement) const { return MovementsInfo[Movement].Sensitivity.AnalogThreshold      ; }
    FLOAT DifferentialThreshold(TMovement Movement) const { return MovementsInfo[Movement].Sensitivity.DifferentialThreshold; }
    FLOAT DigitalThreshold     (TMovement Movement) const { return MovementsInfo[Movement].Sensitivity.DigitalThreshold     ; }
    FLOAT Speed                (TMovement Movement) const { return MovementsInfo[Movement].Sensitivity.Speed                ; }

    void SetAnalogThreshold       (TMovement Movement, FLOAT Value) { MovementsInfo[Movement].Sensitivity.AnalogThreshold        = Value; }
    void SetDifferentialThreshold (TMovement Movement, FLOAT Value) { MovementsInfo[Movement].Sensitivity.DifferentialThreshold  = Value; }
    void SetDigitalThreshold      (TMovement Movement, FLOAT Value) { MovementsInfo[Movement].Sensitivity.DigitalThreshold       = Value; }
    void SetSpeed                 (TMovement Movement, FLOAT Value) { MovementsInfo[Movement].Sensitivity.Speed                  = Value; }

    static TDevice Device(TMovement Movement); // Which device owns Movement?
    static TDevice Device(TSwitch Switch); // Which device owns Switch?
private:
    void CaptureDevice(TDevice Device, BOOL Capture = TRUE);
    void GatherInput(TDevice Device); // Gather input for a device.
public:
    void GatherInput(); // Gather input from devices.
    void StartNextInputCycle(); // Checkpoint, call after input gathered and processed.
        // This saves all movement new positions as the "old" positions, for later comparisons.

    // Platform state information
    void * DeviceInfo;

    //todo: [Mark]
    // Perhaps some of the following could be moved to the 
    // platform-independent class FInput? Some of these members are not
    // use and probably are not needed. Some of these are needed but not
    // yet used:
    //   - when switches are processed, copy IsPending to IsOn so that
    //     switch presses are not missed
    struct TSwitchState
    {  
        BOOL    Changed         : 1 ;   // The switch state just change, from IsOn to !IsOn or from !IsOn to IsOn. 
        BOOL    IsOn            : 1 ;   // The switch is currently on.
        BOOL    IsPending       : 1 ;   // The switch was on, but may not be now. Processing is pending.
        BOOL    IsDouble        : 1 ;   // The switch has been double-pressed, and is currently held down.
        BOOL    DoubleIsPending : 1 ;   // The switch was double-pressed, but may not be held down.
        BOOL    IsToggledOn     : 1 ;   // The switch, treated as a toggle, is considered on (even though the switch might not be on).
        BOOL    IsDoubleToggledOn:1 ;   // The double switch, treated as a toggle, is considered on (even though the switch might not be on).
        void    Empty(BOOL ResetToggles = TRUE);
        // Notes:
        //   1. The IsPending and DoubleIsPending flags are usually reset when the corresponding
        //      switch action has been processed. This is done by the higher layers, if they care.
        //   2. IsDouble ==> IsOn
    };

    TSwitchState    Switches[SwitchCount];
    int             SwitchPressTimes[SwitchCount];  // Time switch was last pressed.

    typedef enum
    {
        NoMovementKind      = 0     // Always 0.
    ,   DigitalMovementKind         // Movement is on or off.
    ,   AnalogMovementKind          // Movement position is used.
    ,   DifferentialMovementKind    // Change in movement position is used.
    ,   MovementKindCount           // Number of kinds of movements, including NoMovementKind.
    }
    TMovementKind;

    // The abbreviations are used in .ini files where brevity may be important to keep 
    // line lengths reasonable. The descriptive names are used in the user interface.
    // The neutral description removes any positive/negative connotation from the description.
    // The help gives a detailed English description.
    static const char * Abbreviation(TMovementKind MovementKind); // [Required]
    static const char * Description(TMovementKind MovementKind); // [Required]
    static const char * Help(TMovementKind MovementKind); // [Required]

    static BOOL IsAnalog(TMovementKind MovementKind)
    {
        return MovementKind == AnalogMovementKind;
    }
    static BOOL IsDifferential(TMovementKind MovementKind)
    {
        return MovementKind == DifferentialMovementKind;
    }

    //tbi: These can be int's:
    FLOAT   NewPosition[MovementCount]      ;
    FLOAT   OldPosition[MovementCount]      ; // Copied from NewPosition when ResetGatheredInput() is called.

    struct TSensitivity
    {
        FLOAT   Speed                 ; // Movement is multiplied by this.
        FLOAT   AnalogThreshold       ; // Relative value needed to trigger analog action. In [0.0,1.0)
        FLOAT   DifferentialThreshold ; // Relative value needed to trigger differential action. In [0.0,1.0)
        FLOAT   DigitalThreshold      ; // Relative value needed to trigger digital action. In [0.0,1.0)
    };

    struct TMovementInfo 
    {
        TSensitivity Sensitivity           ;
        FLOAT        MiddlePosition        ; // Value of middle point in movement.
        FLOAT        HalfRange             ; // Range of movement above or below MiddlePosition.
        // Notes:
        //   1. Speed>1: Changes in movement translate to larger actions.
        //      Speed<1: Changes in movement translate to smaller actions. 
        //   2. Since movements typically come in pairs (such as mouse forward, mouse backward),
        //      the TMovementInfo will usually be the same for the two movements in the pair.
        //      Certainly this->MiddlePosition and this->HalfRange should be the same.
        //      We could allow this->Speed and this->AnalogThreshold to differ between
        //      the paired movements.
        //   3. The threshold defines a "dead zone" in which movements are not recognized.
        //      This is necessary, for example, for joysticks (which are hard to keep 
        //      perfectly centered. A value of 0.5 means the first 1/2 of the range is 
        //      ignored. (0.5 is probably much too large, unless the movement is going
        //      to be used as a digital movement).
    };

    TMovementInfo MovementsInfo[MovementCount];

    void SetMovementRange // Sets MiddlePosition and HalfRange for the specified movements.
    (
        TMovement       PositiveMovement
    ,   TMovement       NegativeMovement
    ,   FLOAT           MinValue
    ,   FLOAT           MaxValue
    );

    void SetDefaultMovementInfo(); // Fills in MovementsInfo with default values.

    static TMovementKind DefaultMovementKind(TMovement Movement);

    //tbi: These can be int's:
    //todo: Actually, I think they can be deleted! Check the other mouse functions too.
    FLOAT OldMouseX() const { return OldPosition[M_MouseR]; }
    FLOAT OldMouseY() const { return OldPosition[M_MouseB]; }
    FLOAT NewMouseX() const { return NewPosition[M_MouseR]; }
    FLOAT NewMouseY() const { return NewPosition[M_MouseB]; }

    // Handling the mouse sometimes requires playing around with the
    // old and new positions, so these functions are provided:

    void SetOldMouse(FLOAT X, FLOAT Y)
    {
        OldPosition[M_MouseR] = X;
        OldPosition[M_MouseL] = X;
        OldPosition[M_MouseB] = Y;
        OldPosition[M_MouseF] = Y;
    }
    void SetNewMouse(FLOAT X, FLOAT Y)
    {
        NewPosition[M_MouseR] = X;
        NewPosition[M_MouseL] = X;
        NewPosition[M_MouseB] = Y;
        NewPosition[M_MouseF] = Y;
    }
    // Resets the old position to (X,Y), and uses the current
    // changes to define the new position.
    void ResetMouse(FLOAT X, FLOAT Y);

    void SetSwitch(TSwitch Switch, BOOL IsPressed = TRUE)
    {
        if(IsPressed) 
        { 
            Press(Switch); 
        } 
        else 
        { 
            Release(Switch); 
        }
    }
    void Press(TSwitch Switch); // Press the specified switch.
    void DoublePress(TSwitch Switch); // Double-press the specified switch.
    void Release(TSwitch Switch); // Release the specified switch.
    BOOL IsOn( TSwitch Switch ) const // Is the given switch on?
    {
        return Switches[Switch].IsOn;
    }
    BOOL IsPending( TSwitch Switch ) const // Is the given switch pending?
    {
        return Switches[Switch].IsPending;
    }

    BOOL IsToggledOn( TSwitch Switch ) const // Is the given switch toggled on?
    {
        return Switches[Switch].IsToggledOn;
    }

    BOOL IsDoubleToggledOn( TSwitch Switch ) const // Is the given switch double-toggled on?
    {
        return Switches[Switch].IsDoubleToggledOn;
    }

    // Is the given switch currently pressed, and if Doubled, is the given switch doubled-pressed?
    BOOL IsOn( TSwitch Switch, BOOL Doubled ) const 
    {
        return IsOn(Switch) && (!Doubled || Switches[Switch].IsDouble);
    }
    // Is the given switch currently pending, and if Doubled, is the given switch doubled-pending?
    BOOL IsPending( TSwitch Switch, BOOL Doubled ) const 
    {
        return IsPending(Switch) && (!Doubled || Switches[Switch].DoubleIsPending);
    }

    // Is the given switch toggled on or, if Doubled, is the switch double-toggled on?
    BOOL IsToggledOn( TSwitch Switch, BOOL Doubled ) const 
    {
        return 
            Doubled
        ?   Switches[Switch].IsDoubleToggledOn
        :   Switches[Switch].IsToggledOn
        ;
    }

    // Is the given switch on, subject to the following interpretations:
    //   !Doubled && !Toggled:
    //      Is the switch currently pressed?
    //   Doubled && !Toggled:
    //      Is the switch currently doubled-pressed?
    //   !Doubled && Toggled:
    //      Is the switch currently toggled on (regardless of whether the switch is actually on or off.)
    //   Doubled && Toggled:
    //      Is the switch currently double-toggled on (regardless of whether the switch is actually on or off.)
    BOOL IsOn( TSwitch Switch, BOOL Doubled, BOOL Toggled ) const 
    {
        return 
            ( !Toggled && IsOn(Switch,Doubled) )
        ||  ( Toggled && IsToggledOn(Switch,Doubled) )
        ;
    }

    // Is the given switch pending, subject to the following interpretations:
    //   !Doubled && !Toggled:
    //      Is the switch currently pending?
    //   Doubled && !Toggled:
    //      Is the switch currently doubled-pending?
    //   !Doubled && Toggled:
    //      Is the switch currently toggled on (regardless of whether the switch is actually on or off.)
    //   Doubled && Toggled:
    //      Is the switch currently double-toggled on (regardless of whether the switch is actually on or off.)
    BOOL IsPending( TSwitch Switch, BOOL Doubled, BOOL Toggled ) const 
    {
        return 
            ( !Toggled && IsPending(Switch,Doubled) )
        ||  ( Toggled && IsToggledOn(Switch,Doubled) )
        ;
    }

    //                  NoteMovement
    // This makes note of a movement pair's current position, and updates
    // MovementPosition[] of the given movements, based on the values.
    void NoteMovement
    (
        TMovement       PositiveMovement    // The movement for position motion.
    ,   TMovement       NegativeMovement    // The movement for negative motion.
    ,   FLOAT           Value               // The current absolute position.
    );

    //              InterpretMovement
    // Interpret the current NewPosition of the specified movement.
    // Interpret it as a MovementKind kind of movement.
    // On output, the following are set:
    //   Magnitude: 
    //     Holds the magnitude of the movement, 0.0..1.0, or 0.0 for an active digital movement.
    //   MovementIsActive:
    //     TRUE if the movement is active, FALSE otherwise.
    void InterpretMovement
    (
        FLOAT         & Magnitude        
    ,   BOOL          & MovementIsActive
    ,   TMovement       Movement
    ,   TMovementKind   MovementKind
    ) const;
};

#endif
