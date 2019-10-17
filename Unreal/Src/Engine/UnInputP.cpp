/*==============================================================================
UnInputP.cpp: Platform-specific input information

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

* N O T E *
    todo: This is to be moved to the Windows project!

Revision history:
    * 06/04/96: Created by Mark
==============================================================================*/

#include "UnInputP.h"
#include <windows.h>
#include "UnChecks.h"
#include "unaction.h" //tbd!

#define ShowSwitchChanges   0 // 1 to show each switch change, 0 otherwise.
#define ShowMovements       0 // 1 to show each movement, 0 otherwise.
#define ShowMovementRanges  0 // 1 to show each movement, 0 otherwise.
#define ShowJoystickChanges 0 // 1 to show each change in joystick state, 0 otherwise.

#define Debugging 0 

static const int MaxHatPosition = 35900; // Maximum position of hat switch

#if Debugging
    static void __cdecl Debug(const char * Message, ...)
    {
        char Text[1000];
        va_list ArgumentList;
        va_start(ArgumentList,Message);
        vsprintf(Text,Message,ArgumentList);
        va_end(ArgumentList);
        debugf(LOG_INFO,Text);
    }
#else
    static inline void __cdecl Debug(const char * Message, ...)
    {
    }
#endif


static const int MouseHalfRangeX = 320; 
static const int MouseHalfRangeY = 200; 

#define arrayCount_(Array) ( sizeof(Array) / sizeof((Array)[0]) )

FPlatformInput::TSwitch FPlatformInput::WindowsKeySwitches[256];

struct TDeviceInfo
{
    FPlatformInput::TDevice Device; // Kind of device
    BOOL        IsUsed      ; // TRUE iff the device is being used for input.
    BOOL        WasChecked  ; // TRUE when the device has been checked and found to be okay.
    BOOL        IsCaptured  ; // TRUE iff input from the device is being captured.
    // Notes:
    //   1. IsUsed==TRUE && !IsCaptured: This means that although the device is meant to
    //      be used for input, it is not currently being used. This could be the case, for 
    //      example, if the program loses the input focus.

    union
    {
        struct // For when Kind==Joystick1Device or JoyStick2Device
        {
            JOYCAPS     Capabilities    ;
            JOYINFOEX   State           ;
        }
        Joystick; // For when Kind==MouseDevice
        struct
        {
            WORD X  ;  // Mouse's X position
            WORD Y  ;  // Mouse's Y position
            int  dX ;  // Change in X position
            int  dY ;  // Change in Y position
        }
        Mouse;
    };
    void Reset();
};

void TDeviceInfo::Reset()
{
    IsUsed      = FALSE ;
    WasChecked  = FALSE ;
    IsCaptured  = FALSE ;
}

static TDeviceInfo * GetDeviceInfo(FPlatformInput & Input)
{
    return (TDeviceInfo *)Input.DeviceInfo;
}
static const TDeviceInfo * GetDeviceInfo(const FPlatformInput & Input)
{
    return (TDeviceInfo *)Input.DeviceInfo;
}
//----------------------------------------------------------------------------
//                 Information about switches
//----------------------------------------------------------------------------
struct TSwitchInfo
{
    FPlatformInput::TSwitch    Switch        ; // Unnecessary, but used for sanity check.
    FPlatformInput::TDevice    Device        ; // The device which owns the switch.
    BOOL                       IsTypingKey   ; // The key is used for console typing.
    const char               * Abbreviation  ; // Used in .ini files.
    const char               * Description   ; // Used in user interface menus.
};

static const TSwitchInfo SwitchesInfo[FPlatformInput::SwitchCount] =
{
    // Cryptic abbreviations for table:
    #define I       FPlatformInput
    #define T       TRUE
    #define F       FALSE
    #define KEY     FPlatformInput::KeyboardDevice
    #define MOUSE   FPlatformInput::MouseDevice
    #define JOY1    FPlatformInput::Joystick1Device
    #define JOY2    FPlatformInput::Joystick2Device
    // Switch            Device  Is.. Abbreviation  Description
    { I::S_None         , KEY   , F, "None"        , "None"                }
,   { I::S_Space        , KEY   , T, "Space"       , "Space"               }
,   { I::S_F1           , KEY   , F, "F1"          , "F1"                  }
,   { I::S_F2           , KEY   , F, "F2"          , "F2"                  }
,   { I::S_F3           , KEY   , F, "F3"          , "F3"                  }
,   { I::S_F4           , KEY   , F, "F4"          , "F4"                  }
,   { I::S_F5           , KEY   , F, "F5"          , "F5"                  }
,   { I::S_F6           , KEY   , F, "F6"          , "F6"                  }
,   { I::S_F7           , KEY   , F, "F7"          , "F7"                  }
,   { I::S_F8           , KEY   , F, "F8"          , "F8"                  }
,   { I::S_F9           , KEY   , F, "F9"          , "F9"                  }
,   { I::S_F10          , KEY   , F, "F10"         , "F10"                 }
,   { I::S_F11          , KEY   , F, "F11"         , "F11"                 }
,   { I::S_F12          , KEY   , F, "F12"         , "F12"                 }
,   { I::S_0            , KEY   , T, "0"           , "0"                   }
,   { I::S_1            , KEY   , T, "1"           , "1"                   }
,   { I::S_2            , KEY   , T, "2"           , "2"                   }
,   { I::S_3            , KEY   , T, "3"           , "3"                   }
,   { I::S_4            , KEY   , T, "4"           , "4"                   }
,   { I::S_5            , KEY   , T, "5"           , "5"                   }
,   { I::S_6            , KEY   , T, "6"           , "6"                   }
,   { I::S_7            , KEY   , T, "7"           , "7"                   }
,   { I::S_8            , KEY   , T, "8"           , "8"                   }
,   { I::S_9            , KEY   , T, "9"           , "9"                   }
,   { I::S_A            , KEY   , T, "A"           , "A"                   }
,   { I::S_B            , KEY   , T, "B"           , "B"                   }
,   { I::S_C            , KEY   , T, "C"           , "C"                   }
,   { I::S_D            , KEY   , T, "D"           , "D"                   }
,   { I::S_E            , KEY   , T, "E"           , "E"                   }
,   { I::S_F            , KEY   , T, "F"           , "F"                   }
,   { I::S_G            , KEY   , T, "G"           , "G"                   }
,   { I::S_H            , KEY   , T, "H"           , "H"                   }
,   { I::S_I            , KEY   , T, "I"           , "I"                   }
,   { I::S_J            , KEY   , T, "J"           , "J"                   }
,   { I::S_K            , KEY   , T, "K"           , "K"                   }
,   { I::S_L            , KEY   , T, "L"           , "L"                   }
,   { I::S_M            , KEY   , T, "M"           , "M"                   }
,   { I::S_N            , KEY   , T, "N"           , "N"                   }
,   { I::S_O            , KEY   , T, "O"           , "O"                   }
,   { I::S_P            , KEY   , T, "P"           , "P"                   }
,   { I::S_Q            , KEY   , T, "Q"           , "Q"                   }
,   { I::S_R            , KEY   , T, "R"           , "R"                   }
,   { I::S_S            , KEY   , T, "S"           , "S"                   }
,   { I::S_T            , KEY   , T, "T"           , "T"                   }
,   { I::S_U            , KEY   , T, "U"           , "U"                   }
,   { I::S_V            , KEY   , T, "V"           , "V"                   }
,   { I::S_W            , KEY   , T, "W"           , "W"                   }
,   { I::S_X            , KEY   , T, "X"           , "X"                   }
,   { I::S_Y            , KEY   , T, "Y"           , "Y"                   }
,   { I::S_Z            , KEY   , T, "Z"           , "Z"                   }
,   { I::S_NumLock      , KEY   , F, "NumLock"     , "NumLock"             }
,   { I::S_N0           , KEY   , T, "NP0"         , "Pad 0"               }
,   { I::S_N1           , KEY   , T, "NP1"         , "Pad 1"               }
,   { I::S_N2           , KEY   , T, "NP2"         , "Pad 2"               }
,   { I::S_N3           , KEY   , T, "NP3"         , "Pad 3"               }
,   { I::S_N4           , KEY   , T, "NP4"         , "Pad 4"               }
,   { I::S_N5           , KEY   , T, "NP5"         , "Pad 5"               }
,   { I::S_N6           , KEY   , T, "NP6"         , "Pad 6"               }
,   { I::S_N7           , KEY   , T, "NP7"         , "Pad 7"               }
,   { I::S_N8           , KEY   , T, "NP8"         , "Pad 8"               }
,   { I::S_N9           , KEY   , T, "NP9"         , "Pad 9"               }
,   { I::S_NDivide      , KEY   , T, "NPDivide"    , "Pad /"               }
,   { I::S_NTimes       , KEY   , T, "NPTimes"     , "Pad *"               }
,   { I::S_NMinus       , KEY   , T, "NPMinus"     , "Pad -"               }
,   { I::S_NPlus        , KEY   , T, "NPPlus"      , "Pad +"               }
,   { I::S_NPeriod      , KEY   , T, "NPPeriod"    , "Pad ."               }
,   { I::S_NEnter       , KEY   , T, "NPEnter"     , "Pad Enter"           }
,   { I::S_Tilde        , KEY   , F, "`"           , "`"                   }
,   { I::S_Minus        , KEY   , T, "-"           , "-"                   }
,   { I::S_Equals       , KEY   , T, "="           , "="                   }
,   { I::S_LeftBracket  , KEY   , T, "["           , "["                   }
,   { I::S_RightBracket , KEY   , T, "]"           , "]"                   }
,   { I::S_BackSlash    , KEY   , T, "\\"          , "\\"                  }
,   { I::S_SemiColon    , KEY   , T, ";"           , ";"                   }
,   { I::S_Quote        , KEY   , T, "'"           , "'"                   }
,   { I::S_Comma        , KEY   , T, ","           , ","                   }
,   { I::S_Period       , KEY   , T, "."           , "."                   }
,   { I::S_Slash        , KEY   , T, "/"           , "/"                   }
,   { I::S_CapsLock     , KEY   , F, "Caps"        , "CapsLock"            }
,   { I::S_Alt          , KEY   , F, "Alt"         , "Alt"                 }
,   { I::S_Ctrl         , KEY   , F, "Ctrl"        , "Ctrl"                }
,   { I::S_Shift        , KEY   , F, "Shift"       , "Shift"               }
,   { I::S_Backspace    , KEY   , T, "Backspace"   , "Backspace"           }
,   { I::S_Insert       , KEY   , F, "Insert"      , "Insert"              }
,   { I::S_Delete       , KEY   , F, "Delete"      , "Delete"              }
,   { I::S_Home         , KEY   , F, "Home"        , "Home"                }
,   { I::S_End          , KEY   , F, "End"         , "End"                 }
,   { I::S_PageUp       , KEY   , T, "PageUp"      , "PageUp"              }
,   { I::S_PageDown     , KEY   , T, "PageDown"    , "PageDown"            }
,   { I::S_CursorLeft   , KEY   , T, "CursorLeft"  , "CursorLeft"          }
,   { I::S_CursorRight  , KEY   , T, "CursorRight" , "CursorRight"         }
,   { I::S_CursorUp     , KEY   , T, "CursorUp"    , "CursorUp"            }
,   { I::S_CursorDown   , KEY   , T, "CursorDown"  , "CursorDown"          }
,   { I::S_PrintScrn    , KEY   , F, "PrintScrn"   , "PrintScrn"           }
,   { I::S_ScrollLock   , KEY   , F, "ScrollLock"  , "ScrollLock"          }
,   { I::S_Pause        , KEY   , F, "Pause"       , "Pause"               }
,   { I::S_Enter        , KEY   , T, "Enter"       , "Enter"               }
,   { I::S_Escape       , KEY   , T, "Esc"         , "Esc"                 }
,   { I::S_Tab          , KEY   , F, "Tab"         , "Tab"                 }
,   { I::S_LeftMouse    , MOUSE , F, "mbl"         , "Mouse Left Button"   }
,   { I::S_RightMouse   , MOUSE , F, "mbr"         , "Mouse Right Button"  }
,   { I::S_MiddleMouse  , MOUSE , F, "mbm"         , "Mouse Middle Button" }
,   { I::S_J1B1         , JOY1  , F, "j1b1"        , "Joystick 1 Button 1" }
,   { I::S_J1B2         , JOY1  , F, "j1b2"        , "Joystick 1 Button 2" }
,   { I::S_J1B3         , JOY1  , F, "j1b3"        , "Joystick 1 Button 3" }
,   { I::S_J1B4         , JOY1  , F, "j1b4"        , "Joystick 1 Button 4" }
,   { I::S_J1B5         , JOY1  , F, "j1b5"        , "Joystick 1 Button 5" }
,   { I::S_J1B6         , JOY1  , F, "j1b6"        , "Joystick 1 Button 6" }
,   { I::S_J1B7         , JOY1  , F, "j1b7"        , "Joystick 1 Button 7" }
,   { I::S_J1B8         , JOY1  , F, "j1b8"        , "Joystick 1 Button 8" }
,   { I::S_J1B9         , JOY1  , F, "j1b9"        , "Joystick 1 Button 9" }
,   { I::S_J1B10        , JOY1  , F, "j1b10"       , "Joystick 1 Button 10"}
,   { I::S_J1HatOn      , JOY1  , F, "j1bh"        , "Joystick 1 Hat"      }
,   { I::S_J1HatN       , JOY1  , F, "j1bhn"       , "Joystick 1 Hat N"    }
,   { I::S_J1HatS       , JOY1  , F, "j1bhs"       , "Joystick 1 Hat S"    }
,   { I::S_J1HatW       , JOY1  , F, "j1bhw"       , "Joystick 1 Hat W"    }
,   { I::S_J1HatE       , JOY1  , F, "j1bhe"       , "Joystick 1 Hat E"    }
,   { I::S_J1HatNW      , JOY1  , F, "j1bhnw"      , "Joystick 1 Hat NW"   }
,   { I::S_J1HatNE      , JOY1  , F, "j1bhne"      , "Joystick 1 Hat NE"   }
,   { I::S_J1HatSW      , JOY1  , F, "j1bhsw"      , "Joystick 1 Hat SW"   }
,   { I::S_J1HatSE      , JOY1  , F, "j1bhse"      , "Joystick 1 Hat SE"   }
,   { I::S_J2B1         , JOY2  , F, "j2b1"        , "Joystick 2 Button 1" }
,   { I::S_J2B2         , JOY2  , F, "j2b2"        , "Joystick 2 Button 2" }
,   { I::S_J2B3         , JOY2  , F, "j2b3"        , "Joystick 2 Button 3" }
,   { I::S_J2B4         , JOY2  , F, "j2b4"        , "Joystick 2 Button 4" }
,   { I::S_J2B5         , JOY2  , F, "j2b5"        , "Joystick 2 Button 5" }
,   { I::S_J2B6         , JOY2  , F, "j2b6"        , "Joystick 2 Button 6" }
,   { I::S_J2B7         , JOY2  , F, "j2b7"        , "Joystick 2 Button 7" }
,   { I::S_J2B8         , JOY2  , F, "j2b8"        , "Joystick 2 Button 8" }
,   { I::S_J2B9         , JOY2  , F, "j2b9"        , "Joystick 2 Button 9" }
,   { I::S_J2B10        , JOY2  , F, "j2b10"       , "Joystick 2 Button 10"}
,   { I::S_J2HatOn      , JOY2  , F, "j2bh"        , "Joystick 2 Hat"      }
,   { I::S_J2HatN       , JOY2  , F, "j2bhn"       , "Joystick 2 Hat N"    }
,   { I::S_J2HatS       , JOY2  , F, "j2bhs"       , "Joystick 2 Hat S"    }
,   { I::S_J2HatW       , JOY2  , F, "j2bhw"       , "Joystick 2 Hat W"    }
,   { I::S_J2HatE       , JOY2  , F, "j2bhe"       , "Joystick 2 Hat E"    }
,   { I::S_J2HatNW      , JOY2  , F, "j2bhnw"      , "Joystick 2 Hat NW"   }
,   { I::S_J2HatNE      , JOY2  , F, "j2bhne"      , "Joystick 2 Hat NE"   }
,   { I::S_J2HatSW      , JOY2  , F, "j2bhsw"      , "Joystick 2 Hat SW"   }
,   { I::S_J2HatSE      , JOY2  , F, "j2bhse"      , "Joystick 2 Hat SE"   }
    // Remove cryptic abbreviations
    #undef I       
    #undef T       
    #undef F       
    #undef KEY     
    #undef MOUSE   
    #undef JOY1    
    #undef JOY2    
};

FPlatformInput::TDevice FPlatformInput::Device(TSwitch Switch)
{
    return SwitchesInfo[Switch].Device;
}

BOOL FPlatformInput::IsTypingKey(TSwitch Switch)
{
    return SwitchesInfo[Switch].IsTypingKey;
}

const char * FPlatformInput::Abbreviation(TSwitch Switch)
{
    return SwitchesInfo[Switch].Abbreviation;
}

const char * FPlatformInput::Description(TSwitch Switch)
{
    return SwitchesInfo[Switch].Description;
}


//----------------------------------------------------------------------------
//                 Information about movements:
//----------------------------------------------------------------------------
struct TMoveInfo
{
    FPlatformInput::TMovement       Movement            ; // Unnecessary, but used for sanity check.
    FPlatformInput::TMovement       OppositeMovement    ; // Movement in opposite direction.
    BOOL                            IsPositive          ; // Is the movement "positive"?
    FPlatformInput::TDevice         Device              ; // The device which owns the movement.
    FPlatformInput::TMovementKind   DefaultKind         ; // Default kind.
    const char                    * Abbreviation        ; // Used in .ini files.
    const char                    * Description         ; // Used in user interface menus.
    const char                    * NeutralDescription  ; // Used in user interface menus for direction-neutral description.
};

static const TMoveInfo MovesInfo[FPlatformInput::MovementCount] =
{
    // Cryptic abbreviations for table:
    #define I       FPlatformInput
    #define T       TRUE
    #define F       FALSE
    #define KEY     FPlatformInput::KeyboardDevice
    #define MOUSE   FPlatformInput::MouseDevice
    #define JOY1    FPlatformInput::Joystick1Device
    #define JOY2    FPlatformInput::Joystick2Device
    #define A       FPlatformInput::AnalogMovementKind
    #define C       FPlatformInput::DifferentialMovementKind
    #define D       FPlatformInput::DigitalMovementKind
    // Movement   OppositeMove.. IsP Device DM Abbrev  Description     NeutralD...
    { I::M_None   , I::M_None   , F, KEY  , A, "None", "None"                   , "None"            }
,   { I::M_MouseR , I::M_MouseL , T, MOUSE, C, "mr"  , "Mouse Right"            , "Mouse Left/Right"   }
,   { I::M_MouseL , I::M_MouseR , F, MOUSE, C, "ml"  , "Mouse Left"             , "Mouse Left/Right"   }
,   { I::M_MouseB , I::M_MouseF , T, MOUSE, C, "mb"  , "Mouse Backward"         , "Mouse Forward/Back" }
,   { I::M_MouseF , I::M_MouseB , F, MOUSE, C, "mf"  , "Mouse Forward"          , "Mouse Forward/Back" }
,   { I::M_J1XP   , I::M_J1XN   , T, JOY1 , A, "j1xp", "Joystick 1 {X} Right"   , "Joystick 1 X-axis"      }
,   { I::M_J1XN   , I::M_J1XP   , F, JOY1 , A, "j1xn", "Joystick 1 {X} Left"    , "Joystick 1 X-axis"      }
,   { I::M_J1YP   , I::M_J1YN   , T, JOY1 , A, "j1yp", "Joystick 1 {Y} Backward", "Joystick 1 Y-axis"      }
,   { I::M_J1YN   , I::M_J1YP   , F, JOY1 , A, "j1yn", "Joystick 1 {Y} Forward" , "Joystick 1 Y-axis"      }
,   { I::M_J1ZP   , I::M_J1ZN   , T, JOY1 , A, "j1zp", "Joystick 1 {Z} Up"      , "Joystick 1 Z-axis"      }
,   { I::M_J1ZN   , I::M_J1ZP   , F, JOY1 , A, "j1zn", "Joystick 1 {Z} Down"    , "Joystick 1 Z-axis"      }
,   { I::M_J1RP   , I::M_J1RN   , T, JOY1 , A, "j1rp", "Joystick 1 Rudder Right", "Joystick 1 Rudder"      }
,   { I::M_J1RN   , I::M_J1RP   , F, JOY1 , A, "j1rn", "Joystick 1 Rudder Left" , "Joystick 1 Rudder"      }
,   { I::M_J2XP   , I::M_J2XN   , T, JOY2 , A, "j2xp", "Joystick 2 {X} Right"   , "Joystick 2 X-axis"      }
,   { I::M_J2XN   , I::M_J2XP   , F, JOY2 , A, "j2xn", "Joystick 2 {X} Left"    , "Joystick 2 X-axis"      }
,   { I::M_J2YP   , I::M_J2YN   , T, JOY2 , A, "j2yp", "Joystick 2 {Y} Backward", "Joystick 2 Y-axis"      }
,   { I::M_J2YN   , I::M_J2YP   , F, JOY2 , A, "j2yn", "Joystick 2 {Y} Forward" , "Joystick 2 Y-axis"      }
,   { I::M_J2ZP   , I::M_J2ZN   , T, JOY2 , A, "j2zp", "Joystick 2 {Z} Up"      , "Joystick 2 Z-axis"      }
,   { I::M_J2ZN   , I::M_J2ZP   , F, JOY2 , A, "j2zn", "Joystick 2 {Z} Down"    , "Joystick 2 Z-axis"      }
,   { I::M_J2RP   , I::M_J2RN   , T, JOY2 , A, "j2rp", "Joystick 2 Rudder Right", "Joystick 2 Rudder"      }
,   { I::M_J2RN   , I::M_J2RP   , F, JOY2 , A, "j2rn", "Joystick 2 Rudder Left" , "Joystick 2 Rudder"      }
    // Remove cryptic abbreviations
    #undef I       
    #undef T       
    #undef F       
    #undef KEY     
    #undef MOUSE   
    #undef JOY1    
    #undef JOY2    
    #undef A       
    #undef NA      
    #undef C       
    #undef NC      
    #undef D       
};

FPlatformInput::TDevice FPlatformInput::Device(TMovement Movement)
{
    return MovesInfo[Movement].Device;
}

const char * FPlatformInput::Abbreviation(TMovement Movement)
{
    return MovesInfo[Movement].Abbreviation;
}

const char * FPlatformInput::Description(TMovement Movement)
{
    return MovesInfo[Movement].Description;
}

const char * FPlatformInput::NeutralDescription(TMovement Movement)
{
    return MovesInfo[Movement].NeutralDescription;
}

FPlatformInput::TMovement FPlatformInput::OppositeMovement(TMovement Movement)
{
    return MovesInfo[Movement].OppositeMovement;
}

BOOL FPlatformInput::IsPositive(TMovement Movement)
{
    return MovesInfo[Movement].IsPositive;
}

FPlatformInput::TMovementKind FPlatformInput::DefaultMovementKind(TMovement Movement)
{
    return MovesInfo[Movement].DefaultKind;
}



//----------------------------------------------------------------------------
//                 The textual names for devices.
//----------------------------------------------------------------------------
static const char * const DeviceNames[FPlatformInput::DeviceCount] =
{
    "Keyboard"      // KeyboardDevice
,   "Mouse"         // MouseDevice
,   "Joystick1"     // Joystick1Device
,   "Joystick2"     // Joystick2Device
};
    
//----------------------------------------------------------------------------
//                 The textual name for a device.
//----------------------------------------------------------------------------
const char * FPlatformInput::DeviceName(TDevice Device)
{
    return DeviceNames[Device];
}
//----------------------------------------------------------------------------
//                 The textual names for movement kinds.
//----------------------------------------------------------------------------
static const char * const MovementKindAbbreviations[FPlatformInput::MovementKindCount] =
{
    "None"  // NoMovementKind      (not used)
,   "D"     // DigitalMovementKind         
,   "A"     // AnalogMovementKind          
,   "C"     // DifferentialMovementKind    
};

static const char * const MovementKindDescriptions[FPlatformInput::MovementKindCount] =
{
    "None"            // NoMovementKind      (not used)
,   "Digital"         // DigitalMovementKind         
,   "Analog"          // AnalogMovementKind          
,   "Change"          // DifferentialMovementKind    
};

const char * FPlatformInput::Abbreviation(TMovementKind MovementKind)
{
    return MovementKindAbbreviations[MovementKind];
}

const char * FPlatformInput::Description(TMovementKind MovementKind)
{
    return MovementKindDescriptions[MovementKind];
}




static struct // List of mappings, used to fill in WindowsKeySwitches
{
    BYTE    VK_Value    ; // The Windows VK_... value.
    BYTE    Switch      ; // The associated switch.        
}

WindowsKeyMappings[] =
{
    #define I FPlatformInput // Local cryptic abbreviation to make tables more manageable.
    { VK_LBUTTON        ,   I::S_LeftMouse      }
,   { VK_RBUTTON        ,   I::S_RightMouse     }
,   { VK_MBUTTON        ,   I::S_MiddleMouse    }
,   { VK_BACK           ,   I::S_Backspace      }
,   { VK_PAUSE          ,   I::S_Pause          }
,   { VK_CAPITAL        ,   I::S_CapsLock       }
,   { VK_SPACE          ,   I::S_Space          }
,   { VK_END            ,   I::S_End            }
,   { VK_HOME           ,   I::S_Home           }
,   { VK_LEFT           ,   I::S_CursorLeft     }
,   { VK_UP             ,   I::S_CursorUp       }
,   { VK_RIGHT          ,   I::S_CursorRight    }
,   { VK_DOWN           ,   I::S_CursorDown     }
,   { VK_INSERT         ,   I::S_Insert         }
,   { VK_DELETE         ,   I::S_Delete         }
,   { VK_NUMPAD0        ,   I::S_N0             }
,   { VK_NUMPAD1        ,   I::S_N1             }
,   { VK_NUMPAD2        ,   I::S_N2             }
,   { VK_NUMPAD3        ,   I::S_N3             }
,   { VK_NUMPAD4        ,   I::S_N4             }
,   { VK_NUMPAD5        ,   I::S_N5             }
,   { VK_NUMPAD6        ,   I::S_N6             }
,   { VK_NUMPAD7        ,   I::S_N7             }
,   { VK_NUMPAD8        ,   I::S_N8             }
,   { VK_NUMPAD9        ,   I::S_N9             }
,   { VK_MULTIPLY       ,   I::S_NTimes         }
,   { VK_ADD            ,   I::S_NPlus          }
,   { VK_SUBTRACT       ,   I::S_NMinus         }
,   { VK_DECIMAL        ,   I::S_NPeriod        }
,   { VK_DIVIDE         ,   I::S_NDivide        }
//,   { VK_F1             ,   I::S_F1             }
//,   { VK_F2             ,   I::S_F2             }
//,   { VK_F3             ,   I::S_F3             }
//,   { VK_F4             ,   I::S_F4             }
//,   { VK_F5             ,   I::S_F5             }
//,   { VK_F6             ,   I::S_F6             }
//,   { VK_F7             ,   I::S_F7             }
//,   { VK_F8             ,   I::S_F8             }
//,   { VK_F9             ,   I::S_F9             }
//,   { VK_F10            ,   I::S_F10            }
//,   { VK_F11            ,   I::S_F11            }
//,   { VK_F12            ,   I::S_F12            }
,   { VK_NUMLOCK        ,   I::S_NumLock        }
,   { VK_SCROLL         ,   I::S_ScrollLock     }
,   { VK_RETURN         ,   I::S_Enter          }
,   { VK_ESCAPE         ,   I::S_Escape         }
,   { VK_TAB            ,   I::S_Tab            }
,   { VK_MENU           ,   I::S_Alt            }
,   { VK_CONTROL        ,   I::S_Ctrl           }
,   { VK_SHIFT          ,   I::S_Shift          }
,   { VK_PRIOR          ,   I::S_PageUp         }
,   { VK_NEXT           ,   I::S_PageDown       }
    #undef I
};

static void PrepareWindowsKeyMappings()
{
    for( int Which = 0; Which < arrayCount_(WindowsKeyMappings); ++Which )
    {
        FPlatformInput::WindowsKeySwitches
        [ 
            WindowsKeyMappings[Which].VK_Value 
        ] 
        = FPlatformInput::TSwitch(WindowsKeyMappings[Which].Switch);
    }
}

static struct // List of ascii mappings, used to fill in WindowsKeySwitches
{
    BYTE    Ascii       ; // Ascii value.
    BYTE    Switch      ; // The associated switch.        
}
AsciiMappings[] =
{
    #define I FPlatformInput // Local cryptic abbreviation to make tables more manageable.

    // The values for 0-9, a-z, A-Z are filled in separately, so they
    // don't need to be part of this table.
    { '`'   , I::S_Tilde            }
,   { '~'   , I::S_Tilde            }
,   { '!'   , I::S_1                }
,   { '@'   , I::S_2                }
,   { '#'   , I::S_3                }
,   { '$'   , I::S_4                }
,   { '%'   , I::S_5                }
,   { '^'   , I::S_6                }
,   { '&'   , I::S_7                }
,   { '*'   , I::S_8                }
,   { '('   , I::S_9                }
,   { ')'   , I::S_0                }
,   { '-'   , I::S_Minus            }
,   { '_'   , I::S_Minus            }
,   { '='   , I::S_Equals           }
,   { '+'   , I::S_Equals           }
,   { '['   , I::S_LeftBracket      }
,   { '{'   , I::S_LeftBracket      }
,   { ']'   , I::S_RightBracket     }
,   { '}'   , I::S_RightBracket     }
,   { '\\'  , I::S_BackSlash        }
,   { '|'   , I::S_BackSlash        }
,   { ';'   , I::S_SemiColon        }
,   { ':'   , I::S_SemiColon        }
,   { '\''  , I::S_Quote            }
,   { '"'   , I::S_Quote            }
,   { ','   , I::S_Comma            }
,   { '<'   , I::S_Comma            }
,   { '.'   , I::S_Period           }
,   { '>'   , I::S_Period           }
,   { '/'   , I::S_Slash            }
,   { '?'   , I::S_Slash            }

    #undef I
};

static int VirtualKey(char C)
{
    const SHORT Key = VkKeyScan(C);
    return Key==-1 ? 0 : Key & 0xff;
}

static void PrepareAsciiMappings()
{
    for( int Which = 0; Which < arrayCount_(AsciiMappings); ++Which )
    {
        const int Key = VirtualKey( AsciiMappings[Which].Ascii );
        if( Key != 0 )
        {
            FPlatformInput::WindowsKeySwitches[Key]
                = FPlatformInput::TSwitch(AsciiMappings[Which].Switch);
        }
    }
    for( int Which = '0'; Which <= '9'; ++Which )
    {
        const int Key = VirtualKey( Which );
        if( Key != 0 )
        {
            FPlatformInput::WindowsKeySwitches[Key]
                = FPlatformInput::TSwitch( FPlatformInput::S_0 + (Which - int('0')) );
        }
    }
    for( int Which = 'a'; Which <= 'z'; ++Which )
    {
        const int Key = VirtualKey( Which );
        if( Key != 0 )
        {
            FPlatformInput::WindowsKeySwitches[Key]
                = FPlatformInput::TSwitch( FPlatformInput::S_A + (Which - int('a')) );
        }
    }
    for( int Which = 'A'; Which <= 'Z'; ++Which )
    {
        const int Key = VirtualKey( Which );
        if( Key != 0 )
        {
            FPlatformInput::WindowsKeySwitches[Key]
                = FPlatformInput::TSwitch( FPlatformInput::S_A + (Which - int('A')) );
        }
    }
}

//----------------------------------------------------------------------------
//                    Constructor
//----------------------------------------------------------------------------
FPlatformInput::FPlatformInput()
{
    DeviceInfo = 0;
}

//----------------------------------------------------------------------------
//                    Initialize
//----------------------------------------------------------------------------
void FPlatformInput::Initialize()
{
    GUARD;
    // Sanity check the arrangement of the movement info:
    for( int Movement_ = 1; Movement_ < MovementCount; Movement_++ )
    {
        const TMovement Movement = TMovement(Movement_);
        if( MovesInfo[Movement].Movement != Movement )
        {
            appError( "Bad movement array" );
        }
    }
    // Sanity check the arrangement of the switch info:
    for( int Switch_ = 1; Switch_ < SwitchCount; Switch_++ )
    {
        const TSwitch Switch = TSwitch(Switch_);
        if( SwitchesInfo[Switch].Switch != Switch )
        {
            appError( "Bad witch array" );
        }
    }
    PrepareWindowsKeyMappings();
    PrepareAsciiMappings();
    DeviceInfo = appMallocArray( DeviceCount, TDeviceInfo, "Input devices" );
    TDeviceInfo * Devices = GetDeviceInfo(*this);
    for( int Device_ = 0; Device_ < DeviceCount; Device_++ )
    {
        const TDevice Device = TDevice(Device_);
        Devices[Device].Reset();
        Devices[Device].Device      = Device    ;
    }
    SetDefaultMovementInfo();
    UseDevice( KeyboardDevice ); // We always enable the keyboard.
    //tba:
    // We have to do something to initialize movements so that the
    // first real movement doesn't have an artifically large change.
    UNGUARD( "FPlatformInput::Initialize" );
}

//----------------------------------------------------------------------------
//                    Reset
//----------------------------------------------------------------------------
void FPlatformInput::Reset()
{
}

//----------------------------------------------------------------------------
//                    Finalize
//----------------------------------------------------------------------------
void FPlatformInput::Finalize()
{
    GUARD;
    appFree( GetDeviceInfo(*this) );
    DeviceInfo = 0;
    UNGUARD( "FPlatformInput::Finalize" );
}

//----------------------------------------------------------------------------
//                 For debugging: show joystick capabilities
//----------------------------------------------------------------------------
static void ShowJoystickCapabilities(const JOYCAPS & Capabilities)
{
    char Message[100];
    char * Text = Message;
    const JOYCAPS & C = Capabilities; // For brevity.
    Text += sprintf( Text, "Joystick:" );
    if(C.szPname!=0)
    {
        Text += sprintf( Text, " %s", C.szPname );
    }
    debug( LOG_Info, Message );
    Text = Message;
    Text += sprintf( Text, "   Buttons: %i", C.wNumButtons );
    Text += sprintf( Text, " Axes: %i/%i", C.wNumAxes, C.wMaxAxes );
    if( ( C.wCaps & JOYCAPS_HASPOV ) == 0 )
    {
        Text += sprintf( Text, " POV: none" );
    }
    else if( ( C.wCaps & JOYCAPS_POV4DIR ) != 0 )
    {
        Text += sprintf( Text, " POV: discrete" );
    }
    else if( ( C.wCaps & JOYCAPS_POV4DIR ) != 0 )
    {
        Text += sprintf( Text, "POV: continuous" );
    }
    Text += sprintf( Text, " Period: %li-%li", (unsigned long)C.wPeriodMin, (unsigned long)C.wPeriodMax );
    debug( LOG_Info, Message );
    Text = Message;
    Text += sprintf( Text, "   X:0x%lx-0x%lx", (unsigned long)C.wXmin, (unsigned long)C.wXmax );
    Text += sprintf( Text, " Y:0x%lx-0x%lx", (unsigned long)C.wYmin, (unsigned long)C.wYmax );
    if( (C.wCaps&JOYCAPS_HASZ) != 0 )
    {
        Text += sprintf( Text, " Z:0x%lx-0x%lx", (unsigned long)C.wZmin, (unsigned long)C.wZmax );
    }
    if( (C.wCaps&JOYCAPS_HASR) != 0 )
    {
        Text += sprintf( Text, " R:0x%lx-0x%lx", (unsigned long)C.wRmin, (unsigned long)C.wRmax );
    }
    if( (C.wCaps&JOYCAPS_HASU) != 0 )
    {
        Text += sprintf( Text, " U:0x%lx-0x%lx", (unsigned long)C.wUmin, (unsigned long)C.wUmax );
    }
    if( (C.wCaps&JOYCAPS_HASV) != 0 )
    {
        Text += sprintf( Text, " V:0x%lx-0x%lx", (unsigned long)C.wVmin, (unsigned long)C.wVmax );
    }
    debug( LOG_Info, Message );
}

//----------------------------------------------------------------------------
//                 For debugging: show joystick state
//----------------------------------------------------------------------------
static void ShowJoystickState(const JOYCAPS & Capabilities, const JOYINFOEX & State)
{
    static JOYINFOEX PreviousState;
    if( memcmp(&PreviousState,&State,sizeof(State)) == 0 )
    {
        // No change since last time - don't show it.
    }
    else
    {
        memmove( &PreviousState, &State, sizeof(State) );
        char Message[400]; //tbi: 100
        char * Text = Message;
        Text += sprintf( Text, "Joystick:" );
        if( State.dwFlags & JOY_RETURNBUTTONS )
        {
            Text += sprintf( Text, " B:%x", int(State.dwButtons) );
        }
        if( State.dwFlags & JOY_RETURNX )
        {
            Text += sprintf( Text, " X:%lx", (unsigned long)State.dwXpos );
        }
        if( State.dwFlags & JOY_RETURNY )
        {
            Text += sprintf( Text, " Y:%lx", (unsigned long)State.dwYpos );
        }
        if( State.dwFlags & JOY_RETURNZ )
        {
            Text += sprintf( Text, " Z:%lx", (unsigned long)State.dwZpos );
        }
        if( State.dwFlags & JOY_RETURNR )
        {
            Text += sprintf( Text, " R:%lx", (unsigned long)State.dwRpos );
        }
        if( State.dwFlags & JOY_RETURNU )
        {
            Text += sprintf( Text, " U:%lx", (unsigned long)State.dwUpos );
        }
        if( State.dwFlags & JOY_RETURNV )
        {
            Text += sprintf( Text, " V:%lx", (unsigned long)State.dwVpos );
        }
        if( State.dwFlags & (JOY_RETURNPOV|JOY_RETURNPOVCTS ) )
        {
            Text += sprintf( Text, " POV:%lx", (unsigned long)State.dwPOV );
        }
        debug( LOG_Info, Message );
    }
}

//----------------------------------------------------------------------------
//                      Check a joystick
//----------------------------------------------------------------------------
// Fills in appropriate fields in DeviceInfo.
// Returns TRUE if the joystick can be used, FALSE otherwise.
static BOOL CheckJoystick
(
    int               WhichJoystick     // 1 or 2
,   UINT              JoystickId
,   TDeviceInfo     & DeviceInfo
,   FPlatformInput  & Input
)
{
    Debug( "Check joystick #%i %i %p %p", int(WhichJoystick), int(JoystickId), &DeviceInfo, &Input );
    BOOL IsOkay = FALSE;

    if( DeviceInfo.WasChecked )
    {
        // We have already checked this device, and it is okay.
        IsOkay = TRUE;
    }
    else if( int(joyGetNumDevs()) < WhichJoystick )
    {
        debugf( LOG_Init, "Driver does not support joystick #%i", WhichJoystick );
    }
    else
    {
        JOYCAPS & Capabilities = DeviceInfo.Joystick.Capabilities;
        JOYINFOEX & State = DeviceInfo.Joystick.State;
        JOYINFO Info;
        State.dwSize = sizeof( State );
        State.dwFlags = JOY_RETURNALL;
        MMRESULT Result = joyGetPos(JoystickId, &Info); 
        if( Result == JOYERR_NOERROR )
        {
            Result = joyGetDevCaps(JoystickId, &Capabilities, sizeof(Capabilities) ); 
        }
        if( Result == JOYERR_NOERROR )
        {
            const JOYCAPS & C = Capabilities; // For brevity.
            DeviceInfo.WasChecked = TRUE;
            IsOkay = TRUE;
            ShowJoystickCapabilities(Capabilities);
            // We start out with axis 1 (X) positive/negative movements (P/N):
            FPlatformInput::TMovement P = WhichJoystick==1 ? FPlatformInput::M_J1XP : FPlatformInput::M_J2XP; 
            FPlatformInput::TMovement N = FPlatformInput::TMovement(P+1);
            Input.SetMovementRange( P, N, FLOAT(C.wXmin), FLOAT(C.wXmax) );
            // Axis 2 (Y):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            Input.SetMovementRange( P, N, FLOAT(C.wYmin), FLOAT(C.wYmax) );
            // Axis 3 (Z):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            if( C.wCaps & JOYCAPS_HASZ )
            {
                Input.SetMovementRange( P, N, FLOAT(C.wZmin), FLOAT(C.wZmax) );
            }
            // Axis 4 (R):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            if( C.wCaps & JOYCAPS_HASR )
            {
                Input.SetMovementRange( P, N, FLOAT(C.wRmin), FLOAT(C.wRmax) );
            }
            // Hat switch:
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            Input.SetMovementRange( P, N, 0, FLOAT(MaxHatPosition) );
        }
        else if( Result == MMSYSERR_NODRIVER )
        {
            debugf( LOG_Init, "Joystick #%i capabilities, failed (no driver).", WhichJoystick );
        } 
        else if( Result == JOYERR_UNPLUGGED )
        {
            debugf( LOG_Init, "Joystick #%i is unplugged.", WhichJoystick );
        } 
        else
        {
            debugf( LOG_Init, "Joystick #%i is unavailable.", WhichJoystick, Result );
        } 
    }
    return IsOkay;
}

//----------------------------------------------------------------------------
//                      Use a device.
//----------------------------------------------------------------------------
void FPlatformInput::UseDevice( TDevice Device, BOOL Use )
{
    GUARD;
    TDeviceInfo * Devices = GetDeviceInfo(*this);
    TDeviceInfo & DeviceInfo = Devices[Device];

    if( Device == KeyboardDevice && !Use )
    {
        // We don't allow disabling the keyboard.
    }
    else if( DeviceInfo.IsUsed ) // Is the device already in use?
    {
        if( Use ) // Use a device already in use?
        {
            CaptureDevice(Device,TRUE); // Start capturing the device.
        }
        else // Stop using a device which is in use.
        {
            CaptureDevice(Device,FALSE); // Stop capturing the device.
            DeviceInfo.Reset();    
        }
    }
    else if( Use ) // Use a device which is not already in use.
    {
        switch( Device )
        {
            case KeyboardDevice:
            {
                DeviceInfo.IsUsed = TRUE;
                break;
            }
            case MouseDevice:
            {
                DeviceInfo.IsUsed = TRUE;
                break;
            }
            case Joystick1Device:
            {
                DeviceInfo.IsUsed = CheckJoystick(1,JOYSTICKID1,DeviceInfo,*this);
                break;
            }
            case Joystick2Device:
            {
                DeviceInfo.IsUsed = CheckJoystick(2,JOYSTICKID2,DeviceInfo,*this);
                break;
            }
        }
        if( DeviceInfo.IsUsed )
        {
            CaptureDevice(Device,TRUE);
        }
    }
    UNGUARD("FPlatformInput::UseDevice");
}

//----------------------------------------------------------------------------
//          Are we using input from a device?
//----------------------------------------------------------------------------
BOOL FPlatformInput::UsingDevice(TDevice Device) const
{
    const TDeviceInfo * Devices = GetDeviceInfo(*this);
    BOOL Using = FALSE;
    if( Devices != 0 )
    {
        const TDeviceInfo & DeviceInfo = Devices[Device];
        Using = DeviceInfo.IsUsed;
    }
    return Using;
}

//----------------------------------------------------------------------------
//          Are we using and capturiung input from a device?
//----------------------------------------------------------------------------
BOOL FPlatformInput::CapturingDevice(TDevice Device) const
{
    BOOL Capturing = FALSE;
    const TDeviceInfo * Devices = GetDeviceInfo(*this);
    if( Devices != 0 )
    {
        const TDeviceInfo & DeviceInfo = Devices[Device];
        Capturing = DeviceInfo.IsCaptured;
    }
    return Capturing;
}

//----------------------------------------------------------------------------
//               Start or stop capturing all devices
//----------------------------------------------------------------------------
void FPlatformInput::CaptureDevices(BOOL Capture)
{
    if( GetDeviceInfo(*this) != 0 )
    {
        for( int Device_ = 0; Device_ < DeviceCount; ++Device_ )
        {
            const TDevice Device = TDevice(Device_);
            CaptureDevice(Device,Capture);
        }
    }
}

//----------------------------------------------------------------------------
//               Start or stop capturing a device
//----------------------------------------------------------------------------
void FPlatformInput::CaptureDevice(TDevice Device, BOOL Capture)
{
    TDeviceInfo * Devices = GetDeviceInfo(*this);
    TDeviceInfo & DeviceInfo = Devices[Device];
    if( !DeviceInfo.IsUsed )
    {
        // The device is not being used.
    }
    else if( DeviceInfo.IsCaptured == Capture )
    {
        // The state of capturing is already as desired.
    }
    else if( Capture ) // Start capturing?
    {
        DeviceInfo.IsCaptured = TRUE;
        switch( Device )
        {
            case KeyboardDevice:
            {
                break;
            }
            case MouseDevice:
            {
                break;
            }
            case Joystick1Device:
            {
                break;
            }
            case Joystick2Device:
            {
                break;
            }
        }
    }
    else // Stop capturing:
    {
        DeviceInfo.IsCaptured = FALSE;
        switch( Device )
        {
            case KeyboardDevice:
            {
                break;
            }
            case MouseDevice:
            {
                break;
            }
            case Joystick1Device:
            {
                break;
            }
            case Joystick2Device:
            {
                break;
            }
        }
    }
}

//----------------------------------------------------------------------------
//               Gather input for all devices in use
//----------------------------------------------------------------------------
void FPlatformInput::GatherInput()
{
    for( int Device_ = 0; Device_ < DeviceCount; ++Device_ )
    {
        const TDevice Device = TDevice(Device_);
        GatherInput(Device);
    }

    // Sometimes keystroke-up messages are missed, especially when Alt is used to
    // activate Windows menu items. Periodically, let's check the status of keys
    // and make sure they are correctly reflected in our state.
    if( (GServer.Ticks&0xf)==0 )
    {
        const SHORT AltStatus   = GetAsyncKeyState(VK_MENU); 
        const SHORT ShiftStatus = GetAsyncKeyState(VK_SHIFT); 
        const SHORT CtrlStatus  = GetAsyncKeyState(VK_CONTROL); 
        // GetAsyncKeyState returns a non-negative number if the key
        // is released (or 0 if the window does not have the input focus).
        if( AltStatus   >= 0 ) { Release( S_Alt     ); }
        if( ShiftStatus >= 0 ) { Release( S_Shift ); }
        if( CtrlStatus  >= 0 ) { Release( S_Ctrl  ); }
    }
}

//----------------------------------------------------------------------------
//       Map a joystick button number to the associated windows constant.
//----------------------------------------------------------------------------
// The button number (1..MaxJoystickButtonCount) is used to index the array.
static const UINT WindowsJoystickButtons[1+FPlatformInput::MaxJoystickButtonCount] =
{
    0           // First entry is unused.
,   JOY_BUTTON1
,   JOY_BUTTON2
,   JOY_BUTTON3
,   JOY_BUTTON4
,   JOY_BUTTON5
,   JOY_BUTTON6
,   JOY_BUTTON7
,   JOY_BUTTON8
,   JOY_BUTTON9
,   JOY_BUTTON10
};

//----------------------------------------------------------------------------
//                      Gather input for a joystick
//----------------------------------------------------------------------------
// Fills in appropriate fields in DeviceInfo.
// Returns TRUE if the joystick can be used, FALSE otherwise.
static void GatherJoystickInput
(
    UINT             JoystickId
,   TDeviceInfo    & DeviceInfo
,   FPlatformInput & Input
)
{
    JOYCAPS   & Capabilities = DeviceInfo.Joystick.Capabilities     ;
    JOYINFOEX & State        = DeviceInfo.Joystick.State            ;
    BOOL Gather = DeviceInfo.IsUsed && DeviceInfo.IsCaptured;
    if( Gather )
    {
        const MMRESULT Result = joyGetPosEx( JoystickId, &State ); 
        Gather = Result == JOYERR_NOERROR;
    }
    if( Gather )
    {
        if( ShowJoystickChanges )
        {
            ShowJoystickState(Capabilities,State);
        }
        // Update the positions of all the axes:
        //tbi: Efficiency and clean-up organization
        {
            const JOYCAPS & C = Capabilities; // For brevity.
            // We start out with axis 1 (X):
            // Positive movement:
            FPlatformInput::TMovement P = 
                DeviceInfo.Device==FPlatformInput::Joystick1Device 
            ?   FPlatformInput::M_J1XP
            :   FPlatformInput::M_J2XP
            ; 
            // Negative movement:
            FPlatformInput::TMovement N = FPlatformInput::TMovement(P+1);
            Input.NoteMovement( P, N, FLOAT(State.dwXpos) );
            // Axis 2 (Y):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            Input.NoteMovement( P, N, FLOAT(State.dwYpos) );
            // Axis 3 (Z):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            if( C.wCaps&JOYCAPS_HASZ )
            {
                Input.NoteMovement( P, N, FLOAT(State.dwZpos) );
            }
            // Axis 4 (R):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            if( C.wCaps&JOYCAPS_HASR )
            {
                Input.NoteMovement( P, N, FLOAT(State.dwRpos) );
            }
            // Axis 5 (U):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            if( C.wCaps&JOYCAPS_HASU )
            {
                Input.NoteMovement( P, N, FLOAT(State.dwUpos) );
            }
            // Axis 6 (V):
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            if( C.wCaps&JOYCAPS_HASV )
            {
                Input.NoteMovement( P, N, FLOAT(State.dwVpos) );
            }
            // Hat switch:
            // If centered, convert this to backwards.
            P = FPlatformInput::TMovement(P+2); N = FPlatformInput::TMovement(N+2);
            if( C.wCaps&JOYCAPS_HASPOV )
            {
                const DWORD HatValue = State.dwPOV == JOY_POVCENTERED ? JOY_POVBACKWARD : State.dwPOV;
                Input.NoteMovement( P, N, FLOAT(HatValue) );
            }
        }
        // Update the buttons:
        {
            FPlatformInput::TSwitch Switch = 
                DeviceInfo.Device==FPlatformInput::Joystick1Device 
            ?   FPlatformInput::S_J1B1 
            :   FPlatformInput::S_J2B1
            ;
            for
            ( 
                int Which = 1; 
                Which <= FPlatformInput::MaxJoystickButtonCount && Which <= int(Capabilities.wNumButtons); 
                ++Which 
            )
            {
                Input.SetSwitch 
                (
                    Switch
                ,   (State.dwButtons&WindowsJoystickButtons[Which]) != 0
                );
                Switch = FPlatformInput::TSwitch(Switch+1);
            }
        }
        // Update the POV (hat) switch:
        if( Capabilities.wCaps & JOYCAPS_HASPOV )
        {
            // The hat switch (or POV switch) has positions arranged in a circle.
            // We divide the circle into even segments as depicted below.
            //                   0
            //               P   A   B
            //           O               C
            //                              
            //        N                     D
            //                                
            // 27000 M                       E 9000
            //
            //        L                     F
            //                              
            //           K               G
            //               J   I   H
            //                 18000
            // We do this to decide when particular switches are on or off.
            // A joystick can support a continuous position in this circle or
            // only the discrete positions (A, E, I, and M).
            // We'll use the above names to identify the points.
            // Be careful about comparing values around 0 (due to the discontinuity in the values near there).
            const int A  = JOY_POVFORWARD   ;
            const int A0 = MaxHatPosition   ; // To handle discontinuity at A.
            const int E  = JOY_POVRIGHT     ;
            const int I  = JOY_POVBACKWARD  ;
            const int M  = JOY_POVLEFT      ;
            const int C  = (A+E)/2;
            const int B  = (A+C)/2;
            const int D  = (C+E)/2;
            const int G  = (E+I)/2;
            const int F  = (E+G)/2;
            const int H  = (G+I)/2;
            const int K  = (I+M)/2;
            const int J  = (I+K)/2;
            const int L  = (K+M)/2;
            const int O  = (M+A0)/2;
            const int N  = (M+O)/2;
            const int P  = (O+A0)/2;
            //tbi: Performance
            const int JoystickSelector = // To convert hat switch values from joystick #1 to joystick being considered.
                DeviceInfo.Device==FPlatformInput::Joystick1Device 
            ?   0
            :   FPlatformInput::S_J2HatOn - FPlatformInput::S_J1HatOn
            ; 
            const DWORD HatValue = State.dwPOV;
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatOn + JoystickSelector )
            ,   HatValue != JOY_POVCENTERED
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatW + JoystickSelector )
            ,   HatValue > L && HatValue < N
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatE + JoystickSelector )
            ,   HatValue > D && HatValue < F
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatN + JoystickSelector )
            ,   HatValue != JOY_POVCENTERED && (HatValue > P || HatValue < B ) // Note the discontinuity around point A.
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatS + JoystickSelector )
            ,   HatValue > H && HatValue < J
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatNW + JoystickSelector )
            ,   HatValue >= N && HatValue <= P
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatNE + JoystickSelector )
            ,   HatValue >= B && HatValue <= D
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatSW + JoystickSelector )
            ,   HatValue >= J && HatValue <= L
            ); 
            Input.SetSwitch
            ( 
                FPlatformInput::TSwitch( FPlatformInput::S_J1HatSE + JoystickSelector )
            ,   HatValue >= F && HatValue <= H
            ); 
            if( FALSE )//tbd: debugging stuff
            {
                static int Last = -2;
                if( Last != int(HatValue) )
                {
                    debugf( LOG_Info, "Hat: %i", int(HatValue) );
                    Last = int(HatValue);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
//                   Gather input from a device.
//----------------------------------------------------------------------------
void FPlatformInput::GatherInput(TDevice Device)
{
    GUARD;
    TDeviceInfo * Devices = GetDeviceInfo(*this);
    TDeviceInfo & DeviceInfo = Devices[Device];
    if( !DeviceInfo.IsUsed || !DeviceInfo.IsCaptured )
    {
        // Don't do anything.
    }
    else
    {
        switch( Device )
        {
            case KeyboardDevice:
            {
                break;
            }
            case MouseDevice:
            {
                break;
            }
            case Joystick1Device:
            {
                GatherJoystickInput( JOYSTICKID1, DeviceInfo, *this );
                break;
            }
            case Joystick2Device:
            {
                GatherJoystickInput( JOYSTICKID2, DeviceInfo, *this );
                break;
            }
        }
    }

    UNGUARD( "FPlatformInput::GatherInput");
}

//----------------------------------------------------------------------------
//                   Start the next input cycle.
//----------------------------------------------------------------------------
void FPlatformInput::StartNextInputCycle()
{
    //tbi: Performance (some of this is only needed for the mouse)
    // Reset any out-of-range positions.
    for( int Movement_ = 1; Movement_ < MovementCount; Movement_++ )
    {
        const TMovement Movement = TMovement(Movement_);
        TMovementInfo & Info = MovementsInfo[Movement];
        if( NewPosition[Movement] > Info.MiddlePosition + Info.HalfRange )
        {
            NewPosition[Movement] = Info.MiddlePosition + Info.HalfRange;
        }
        else if( NewPosition[Movement] < Info.MiddlePosition - Info.HalfRange )
        {
            NewPosition[Movement] = Info.MiddlePosition - Info.HalfRange;
        }
    }
    // Mark all pending but inactive switches as non-pending:
    for( int Switch_ = 1; Switch_ < SwitchCount; Switch_++ )
    {
        const TSwitch Switch = TSwitch(Switch_);
        TSwitchState & Info = Switches[Switch];
        Info.IsPending = Info.IsOn;
        Info.DoubleIsPending = Info.IsDouble;
    }
    memmove( OldPosition, NewPosition, sizeof(OldPosition) );
}
//----------------------------------------------------------------------------
//                      Reset a switch.
//----------------------------------------------------------------------------
void FPlatformInput::TSwitchState::Empty(BOOL ResetToggles)
{
    Changed         = FALSE;
    IsOn            = FALSE;
    IsPending       = FALSE;
    IsDouble        = FALSE;
    DoubleIsPending = FALSE;
    if( ResetToggles )
    {
        IsToggledOn       = FALSE;
        IsDoubleToggledOn = FALSE;
    }
}

//----------------------------------------------------------------------------
//                      Press a switch
//----------------------------------------------------------------------------
void FPlatformInput::Press(TSwitch Switch)
{
    TSwitchState & State = Switches[Switch];
    if( Switch != 0 && !State.IsOn )
    {
        // The switch was not on, and is now pressed.

        // Check for double-press, based on time of last switch change.
        const int Time = GServer.Ticks;
        if( Time-SwitchPressTimes[Switch] < 10 ) //tbi: constant time delta
        {
            DoublePress(Switch);
        }
        else
        {
            State.Changed   = TRUE;
            State.IsOn      = TRUE;
            State.IsPending = TRUE; // New press has not yet been processed.
            State.IsToggledOn = !State.IsToggledOn;
            SwitchPressTimes[Switch] = Time;
            if( ShowSwitchChanges )
            {
                debugf( LOG_Info, "+%s%s", Abbreviation(Switch), State.IsToggledOn ? "[T]" : "" );
            }
        }
    }
}

//----------------------------------------------------------------------------
//                      Double-press a switch
//----------------------------------------------------------------------------
void FPlatformInput::DoublePress(TSwitch Switch)
{
    TSwitchState & State = Switches[Switch];
    if( Switch != 0 && !State.IsOn )
    {
        State.Changed           = TRUE;
        State.IsOn              = TRUE;
        State.IsPending         = TRUE; // New press has not yet been processed.
        State.IsDouble          = TRUE;
        State.DoubleIsPending   = TRUE; // New double-press has not yet been processed.
        State.IsDoubleToggledOn = !State.IsDoubleToggledOn;
        SwitchPressTimes[Switch] = GServer.Ticks;
        if( ShowSwitchChanges )
        {
            debugf( LOG_Info, "+%s*2%s", Abbreviation(Switch), State.IsDoubleToggledOn ? "[T]" : "" );
        }
    }
}

//----------------------------------------------------------------------------
//                      Release a switch.
//----------------------------------------------------------------------------
void FPlatformInput::Release(TSwitch Switch)
{
    TSwitchState & State = Switches[Switch];
    if( Switch != 0 && State.IsOn )
    {
        State.Changed           = TRUE  ;
        State.IsOn              = FALSE ;
        State.IsDouble          = FALSE ;
        if( ShowSwitchChanges )
        {
            debugf( LOG_Info, "-%s", Abbreviation(Switch) );
        }
    }
}

//----------------------------------------------------------------------------
//                   Set default movement info.
//----------------------------------------------------------------------------
void FPlatformInput::SetDefaultMovementInfo()
{
    for( int Movement_ = 1; Movement_ < MovementCount; Movement_++ )
    {
        const TMovement Movement = TMovement(Movement_);
        TMovementInfo & Info = MovementsInfo[Movement];
        Info.MiddlePosition = 0.0       ;
        Info.HalfRange      = 1000.0    ;
        Info.Sensitivity.Speed                  = 1.0       ;
        Info.Sensitivity.AnalogThreshold        = IsMouseMovement(Movement) ? 0.01 : 0.10;
        Info.Sensitivity.DifferentialThreshold  = IsMouseMovement(Movement) ? 0.01 : 0.01;
        Info.Sensitivity.DigitalThreshold       = IsMouseMovement(Movement) ? 0.20 : 0.40;
    }
}

//----------------------------------------------------------------------------
//                   Set range information for a movement pair
//----------------------------------------------------------------------------
void FPlatformInput::SetMovementRange
(
    TMovement       PositiveMovement
,   TMovement       NegativeMovement
,   FLOAT           MinValue
,   FLOAT           MaxValue
)
{
    TMovementInfo & PositiveInfo = MovementsInfo[PositiveMovement];
    TMovementInfo & NegativeInfo = MovementsInfo[NegativeMovement];

    PositiveInfo.MiddlePosition = (MinValue+MaxValue)/2.0;

    if( IsMouseMovement(PositiveMovement) ) //tbd?
    {
        // We pretend all mouse movements have the same range. This
        // normalizes behaviour across a variety of screen resolutions.
        if( PositiveMovement == M_MouseR )
        {
            PositiveInfo.HalfRange = MouseHalfRangeX;
        }
        else
        {
            PositiveInfo.HalfRange = MouseHalfRangeY;
        }
    }
    else
    {
        PositiveInfo.HalfRange = MaxValue-PositiveInfo.MiddlePosition;
    }

    NegativeInfo.MiddlePosition = PositiveInfo.MiddlePosition   ;
    NegativeInfo.HalfRange      = PositiveInfo.HalfRange        ;

    if( ShowMovementRanges )
    {
        debugf
        ( 
            LOG_Info
        ,   "Movement range for %s: %3.2f +- %3.2f"
        ,   Abbreviation(PositiveMovement)
        ,   PositiveInfo.MiddlePosition
        ,   PositiveInfo.HalfRange
        );
    }
}

//----------------------------------------------------------------------------
//                         NoteMovement
//----------------------------------------------------------------------------
void FPlatformInput::NoteMovement
(
    TMovement       PositiveMovement    // The movement for position motion.
,   TMovement       NegativeMovement    // The movement for negative motion.
,   FLOAT           Value               // The current absolute position.
)
{
    
    // We update both the positive and negative movement information with
    // the position. Later, when the movement is interpreted, we will sort out
    // the positiveness/negativeness.
    NewPosition[NegativeMovement] = Value;
    NewPosition[PositiveMovement] = Value;
    if( ShowMovements && ( IsMouseMovement(PositiveMovement) || (GServer.Ticks&0x1f)==0 ) )
    {
        debugf( LOG_Info, "NoteMovement - %s: %3.2f", Abbreviation(PositiveMovement), Value );
    }
}

//----------------------------------------------------------------------------
//                      Interpret a movement
//----------------------------------------------------------------------------
void FPlatformInput::InterpretMovement
(
    FLOAT         & Magnitude        
,   BOOL          & MovementIsActive
,   TMovement       Movement
,   TMovementKind   MovementKind
) 
const
{
    //tbi: Performance
    MovementIsActive    = FALSE ;
    Magnitude           = 0.0   ;
    const TMovementInfo & Info = MovementsInfo[Movement];
    const BOOL MovementIsPositive = MovesInfo[Movement].IsPositive;
    if( CapturingDevice( Device(Movement) ) )
    {
        // Normalized value, except possibly for the sign:
        const FLOAT NormalizedValue = (NewPosition[Movement]-Info.MiddlePosition)/Info.HalfRange;
        switch(MovementKind)
        {
            case DigitalMovementKind:
            {
                const FLOAT Threshold = Info.Sensitivity.DigitalThreshold;
                MovementIsActive = 
                    MovementIsPositive && NormalizedValue >= Threshold
                || !MovementIsPositive && NormalizedValue <= -Threshold
                ;
                if( MovementIsActive && ShowMovements ) 
                {
                    debugf
                    (
                        LOG_Info
                    ,   "Digital movement: %s %3.2f->%3.2f(norm) (thr:%3.2f) in [%3.2f+-%3.2f]"
                    ,   Abbreviation(Movement)
                    ,   NewPosition[Movement]
                    ,   NormalizedValue
                    ,   Threshold
                    ,   Info.MiddlePosition
                    ,   Info.HalfRange 
                    );
                }
                break;
            }
            case AnalogMovementKind:
            {
                const FLOAT Threshold = Info.Sensitivity.AnalogThreshold;
                const FLOAT AliveZone = 1.0-Threshold; // Amount of movement that is alive.
                if( MovementIsPositive && NormalizedValue > Threshold )
                {
                    MovementIsActive = TRUE;
                    Magnitude = (NormalizedValue-Threshold)/AliveZone*Info.Sensitivity.Speed;
                }
                else if( !MovementIsPositive && NormalizedValue < -Threshold )
                {
                    MovementIsActive = TRUE;
                    Magnitude = -(NormalizedValue+Threshold)/AliveZone*Info.Sensitivity.Speed;
                }
                if( Magnitude > 1.0 ) { Magnitude = 1.0; }
                else if( Magnitude < 0 ) { Magnitude = 0; }
                if( FALSE && MovementIsActive ) //tbd:
                {
                    //tbe: Fit a quadratic into interval [a,1] and we get this:
                    // y = (x*x - 2*x + a)/(a-1):
                    const FLOAT A = 0.2f;
                    if( Magnitude >= A )
                    {
                        Magnitude = (Magnitude*Magnitude - 2*Magnitude + A)/(A-1);
                    }
                    //tbd:Magnitude = sqrt(Magnitude); //tbi? Performance?
                }
                break;
            }
            case DifferentialMovementKind:
            {
                const FLOAT Threshold = Info.Sensitivity.DifferentialThreshold;
                const FLOAT AliveZone = 1.0-Threshold; // Amount of movement that is alive.
                const FLOAT Change = NewPosition[Movement]-OldPosition[Movement];
                const FLOAT NormalizedValue = Change/Info.HalfRange;
                if( MovementIsPositive && NormalizedValue > Threshold )
                {
                    MovementIsActive = TRUE;
                    Magnitude = (NormalizedValue-Threshold)/AliveZone*Info.Sensitivity.Speed;
                }
                else if( !MovementIsPositive && NormalizedValue < -Threshold )
                {
                    MovementIsActive = TRUE;
                    Magnitude = -(NormalizedValue+Threshold)/AliveZone*Info.Sensitivity.Speed;
                    if( ShowMovements ) debugf(LOG_Info,"Differential %s: Change: %6.4f->%6.4f in [%3.2f+-%3.2f]", Abbreviation(Movement), Change, Magnitude, Info.MiddlePosition, Info.HalfRange );
                }
                if( Magnitude > 1.0 ) { Magnitude = 1.0; }
                else if( Magnitude < 0 ) { Magnitude = 0; }
                if( FALSE && MovementIsActive ) //tbd:
                {
                    //tbe: Fit a quadratic into interval [a,1] and we get this:
                    // y = (x*x - 2*x + a)/(a-1):
                    const FLOAT A = 0.2f;
                    if( Magnitude >= A )
                    {
                        Magnitude = (Magnitude*Magnitude - 2*Magnitude + A)/(A-1);
                    }
                    //tbd: Magnitude = sqrt(Magnitude); //tbi? Performance?
                }
                if( MovementIsActive && ShowMovements ) 
                {
                    debugf
                    (
                        LOG_Info
                    ,   "Differential %s: New-Old: %3.3f-%3.3f Magn:%3.3f in [%3.1f+-%3.1f]"
                    ,   Abbreviation(Movement)
                    ,   NewPosition[Movement]
                    ,   OldPosition[Movement]
                    ,   Magnitude
                    ,   Info.MiddlePosition
                    ,   Info.HalfRange 
                    );
                }
                break;
            }
        }   
    }
}

#if 0 //tbd:
void FPlatformInput::ResetMouse(FLOAT X, FLOAT Y)
{
    const FLOAT dX = NewMouseX() - OldMouseX();
    const FLOAT dY = NewMouseY() - OldMouseY();
    SetOldMouse( X, Y );
    SetNewMouse( X+dX, Y+dY );
    debugf( LOG_Info, "ResetMouse: Old(%3.1f,%3.1f) New(%3.1f,%3.1f)", OldMouseX(), OldMouseY(), NewMouseX(), NewMouseY() );
}
#endif