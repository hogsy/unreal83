/*==============================================================================
UnPrefer.cpp: User Preferences

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 07/17/96: Created by Mark
==============================================================================*/

#include "UnPrefer.h"

FPreferences GPreferences;

//----------------------------------------------------------------------------
//           Table of information for BOOL preferences:
//----------------------------------------------------------------------------
struct TBoolInfo
{
    const char            * Key             ; // Text key representing value.
    BOOL FPreferences::   * Value           ; // Pointer to member which holds value.
};
static const TBoolInfo BoolInfo[] = // List of info, terminated by entry with .Key==0 and .Value==0.
{
    //  Key                Value            DefaultValue
    { "Nudity"            , &FPreferences::AllowNudity          }
,   { "Profanity"         , &FPreferences::AllowProfanity       }
,   { "Blood"             , &FPreferences::AllowBlood           }
,   { "WeaponsSway"       , &FPreferences::WeaponsSway          }
,   { "StillViewBobs"     , &FPreferences::StillViewBobs        }
,   { "MovingViewBobs"    , &FPreferences::MovingViewBobs       }
,   { "ViewFollowsIncline", &FPreferences::ViewFollowsIncline   }
,   { "SwitchEmptyWeapon" , &FPreferences::SwitchFromEmptyWeapon}
,   { "SwitchToNewWeapon" , &FPreferences::SwitchToNewWeapon    }
,   { "ReverseUpAndDown"  , &FPreferences::ReverseUpAndDown     }
,   { "MouseLookAlwaysOn" , &FPreferences::MouseLookAlwaysOn    }
,   { "RunAlwaysOn"       , &FPreferences::RunAlwaysOn          }
,   { "ViewRolls"         , &FPreferences::ViewRolls            }
,   { 0                   , 0                                   } // Terminates list
};

//----------------------------------------------------------------------------
//                          Constructor
//----------------------------------------------------------------------------
FPreferences::FPreferences()
{
    SetDefaults();
}

//----------------------------------------------------------------------------
//                   Set all values to their defaults.
//----------------------------------------------------------------------------
void FPreferences::SetDefaults()
{
    // Set the Boolean values:
    {
        const TBoolInfo * Info = &BoolInfo[0];
        while( Info->Key != 0 )
        {
            this->*(Info->Value) = FALSE;
            Info++;
        }
    }
}

//----------------------------------------------------------------------------
//              What are all the keys?
//----------------------------------------------------------------------------
FPreferences::TStringList FPreferences::Keys() const
{
    TStringList Keys;
    // Add the Boolean values:
    {
        const TBoolInfo * Info = &BoolInfo[0];
        while( Info->Key != 0 )
        {
            Keys.Add( Info->Key );
            Info++;
        }
    }
    return Keys;
}

//----------------------------------------------------------------------------
//           What is the current value for the given key? 
//----------------------------------------------------------------------------
char * FPreferences::Value(const char * Key) const
{
    char Value[FKeyValues::MaxValueLength+1]; // +1 for trailing null.
    Value[0] = 0;
    char * Text = Value; // Place in Value where we are adding text.
    BOOL Found = FALSE;
    // Check the Boolean values:
    {
        const TBoolInfo * Info = &BoolInfo[0];
        while( !Found && Info->Key != 0 )
        {
            if( stricmp(Key,Info->Key) == 0 )
            {
                Found = TRUE;
                Text += sprintf( Text, "%s", FParse::BooleanText(this->*(Info->Value)) );
            }
            Info++;
        }
    }
    return Value[0] ==0 ? 0 : FParse::MakeString(Value);
}

//----------------------------------------------------------------------------
//            Set the value associated with a text key.
//----------------------------------------------------------------------------
BOOL FPreferences::SetValue(const char * Key, const char * Value)
{
    BOOL Found = FALSE;
    BOOL IsOkay = FALSE;
    // Check the Boolean values:
    {
        const TBoolInfo * Info = &BoolInfo[0];
        while( !Found && Info->Key != 0 )
        {
            if( stricmp(Key,Info->Key) == 0 )
            {
                Found = TRUE;
                BOOL ValueIsOkay;
                BOOL NewValue = FParse::InterpretBoolean(Value,ValueIsOkay);
                if( ValueIsOkay )
                {
                    this->*(Info->Value) = NewValue;
                    IsOkay = TRUE;
                }
            }
            Info++;
        }
    }
    return IsOkay;
}
