/*==============================================================================
UnPrefer.h: Unreal user preferences
Used by: Actor code, Engine code(?)

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    
    Here we encapsulate the miscellaneous configurable user preferences.
    This package is not very coherent - the preferences span
    various different kinds of behaviour. However, there is some 
    convenience in collecting these preferences into a single place.

Revision history:
    * 07/17/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnPrefer
#define _INC_UnPrefer

#include "Unreal.h"
#include "UnKeyVal.h"

class UNREAL_API FPreferences
    : public FKeyValues     // We provide a key-value interface.
{
public:

    BOOL AllowNudity                ; // Nudity is allowed.
    BOOL AllowProfanity             ; // Profanity is allowed.
    BOOL AllowBlood                 ; // Blood is allowed.
    BOOL WeaponsSway                ; // Weapons sway back and forth.
    BOOL StillViewBobs              ; // View bobs up and down when player is still.
    BOOL MovingViewBobs             ; // View bobs up and down when player is moving.
    BOOL ViewFollowsIncline         ; // The view looks up or down ramps and stairs.
    BOOL SwitchFromEmptyWeapon      ; // Switch to non-empty weapon when current weapon is emptied.
    BOOL SwitchToNewWeapon          ; // Switch to new (better) weapon.
    BOOL ReverseUpAndDown           ; // Reverse up and down directions (looking, turning).
    BOOL MouseLookAlwaysOn          ; // Is the mouse-look action always on?
    BOOL RunAlwaysOn                ; // Is the run action always on?
    BOOL ViewRolls                  ; // Does the player's view roll (during turn or left/right movement).

    //tba: startup screen size and kind
    //tba: kind of status display
    //tba: name of player

    FPreferences();
    void SetDefaults(); // Change all values to their defaults.

    //            Key-Value pairs for input.
    // The recognized keys are not documented here because they tend to
    // change and any list here will soon be obsolete. Instead, the
    // Keys() function can be used to get a list.

    // Implementations of the FKeyValues interface:
    TStringList Keys() const; // What are all the keys?
    char * Value(const char * Key) const; // What is the current value for the given key? 
    BOOL SetValue(const char * Key, const char * Value); // Set the value associated with a key.

};

extern UNREAL_API FPreferences GPreferences;

#endif
