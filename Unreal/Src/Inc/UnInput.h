/*==============================================================================
UnInput.h: Unreal input 

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:

    This has the platform-independent Unreal input specifics.
    This depends on the platform-dependent input functions
    defined elsewhere.

Revision history:
    * 06/01/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnInput
#define _INC_UnInput

#include "Unreal.h"
#include "UnInputP.h"
#include "UnKeyVal.h"

class UNREAL_API FInput
    : public FPlatformInput // Platform-specific input information
    , public FKeyValues     // We provide a key-value interface.
{
public:

    void Initialize();
    void Finalize();
    void Reset(); // Reset input information.

    struct UNREAL_API TMetaSwitch // A switch with some interpretations attached to it.
    {
        TSwitch Switch   :16; // Switch, 0 if none.
        BOOL    IsDouble :1 ; // Is the switch treated as a double-switch (activated with a double-press)?
        BOOL    IsToggle :1 ; // Is the switch treated as a toggle, being alternately turned on and off as it is pressed?
        void Empty() { Switch = S_None; IsDouble = FALSE; IsToggle = FALSE; }
        BOOL IsEmpty() const { return Switch==0; }
        const char * Text(BOOL Descriptive) const; // Return a textual description of the meta-switch, "" if IsEmpty().
            // If Descriptive, return a longer, user-readable form.
            // Note: The returned string is in a buffer possibly reused by this or other functions.

        BOOL Parse(const char * & Text); // Make a meta-switch from the given Text.
            // Remove any leading whitespace from Text.
            // If the text starts with one of these definitions:
            //      SwitchName
            //      SwitchName*2
            //      SwitchName*2(T)
            // then do the following:
            //      Set Text to point beyond the definition, after any trailing whitespace.
            //      Set *this to reflect the values implied by the text.
            //      Return TRUE.
            // Otherwise:
            //    this->Empty()
            //    Return FALSE.

        BOOL operator == (const TMetaSwitch & Value) const;
        void Normalize(); // Normalizes this by clearing unused information.
    };

    struct UNREAL_API TMetaMovement // A movement with some interpretations attached to it.
    {
        TMovement     Movement :8 ; // Movement, 0 if none.
        TMovementKind Kind     :8 ; // How to interpret the movement.
        void Empty() { Movement = M_None; Kind = NoMovementKind; }
        BOOL IsEmpty() const { return Movement==0; }
        const char * Text(BOOL Descriptive) const; // Return a textual description of the meta-movement, "" if IsEmpty()..
            // If Descriptive, return a longer, user-readable form.
            // Note: The returned string is in a buffer possibly reused by this or other functions.

        BOOL Parse(const char * & Text); // Make a meta-Movement from the given Text.
            // Remove any leading whitespace from Text.
            // If the text starts with one of these definitions:
            //      MovementName
            //      MovementName(MovementKind)
            // then do the following:
            //      Set Text to point beyond the definition, after any trailing whitespace.
            //      Set *this to reflect the values implied by the text.
            //      Return TRUE.
            // Otherwise:
            //    this->Empty()
            //    Return FALSE.
        BOOL operator == (const TMetaMovement & Value) const;
        void Normalize() {} // Normalizes this by clearing unused information.
    };

    struct UNREAL_API TModifiers // Modifier switches, which modify a main input.
    {
        TMetaSwitch MetaSwitch1; // Currently, we allow only 1 modifying switch.
        TSwitch Switch1() const { return MetaSwitch1.Switch; } // A notational convenience.
        BOOL Has1() const { return Switch1()!=0; } // Is there a modifier?
        BOOL IsEmpty() const { return !Has1(); }
        void Empty() { MetaSwitch1.Empty(); }
        const char * Text(BOOL Descriptive) const; // Return a textual description of the modifiers, "" if IsEmpty()..
            // If Descriptive, return a longer, user-readable form.
            // Note: The returned string is in a buffer possibly reused by this or other functions.

        BOOL Parse(const char * & Text); // Make a TModifier from the given Text.
            // Remove any leading whitespace from Text.
            // If the text starts with this definition:
            //      MetaSwitch +
            // where MetaSwitch is as expected in TMetaSwitch::Parse, 
            // then do the following:
            //      Set Text to point beyond the definition, after any trailing whitespace.
            //      Set *this to reflect the values implied by the text.
            //      Return TRUE.
            // Otherwise:
            //    this->Empty()
            //    Return FALSE.
        BOOL operator == (const TModifiers & Value) const;
        void Normalize(); // Normalizes this by clearing unused information.
    };

    struct UNREAL_API TCombo // An input combo - combination of modifiers and a switch or movement.
    // Note: Combo is a bit of a misnomer, since if there are no modifiers, 
    // it really isn't a "combination".
    {
        BOOL        IsASwitch    :1 ; // TRUE if the main input for the combo is a switch. FALSE for a movement.
        TModifiers  Modifiers       ; // The modifiers that are part of the combo.
        union
        {
            TMetaSwitch     MetaSwitch      ; // For when IsSwitch.
            TMetaMovement   MetaMovement    ; // For when !IsSwitch.
        };
        BOOL IsSwitch     () const { return IsASwitch;  }
        BOOL IsMovement   () const { return !IsASwitch; }
        TSwitch   Switch  () const { return MetaSwitch.Switch    ; } // Call only when IsSwitch().
        TMovement Movement() const { return MetaMovement.Movement; } // Call only when IsMovement().
        void Empty() { Modifiers.Empty(); }
        const char * Text(BOOL Descriptive) const; // Return a textual description of the combo.
            // If Descriptive, return a longer, user-readable form.
            // Note: The returned string is in a buffer possibly reused by this or other functions.
        const char * MainInputText(BOOL Descriptive) const; // Return a textual description of the main input.
            // If Descriptive, return a longer, user-readable form.
            // Note: The returned string is in a buffer possibly reused by this or other functions.

        BOOL Parse(const char * & Text); // Parse a combo out of the given Text.
            // Remove any leading whitespace from Text.
            // If the text starts with one of these definitions:
            //      MetaSwitch
            //      Modifiers MetaSwitch
            //      MetaMovement
            //      Modifiers MetaMovement
            // where MetaSwitch is as expected in TMetaSwitch::Parse, 
            // MetaMovement is as expected in TMetaMovement::Parse,
            // and Modifiers is as expected in TModifiers::Parse,
            // then do the following:
            //      Set Text to point beyond the definition, after any trailing whitespace.
            //      Set *this to reflect the values implied by the text.
            //      Return TRUE.
            // Otherwise:
            //    this->Empty()
            //    Return FALSE.
        BOOL operator == (const TCombo & Value) const;
        void Normalize(); // Normalizes this by clearing unused information.
    };

    //         Parse a movement name out of a text string.    
    // Look for a movement name at the beginning of Text, after any whitespace.
    // If found, move Text beyond the movement name and return the identified movement.
    // Otherwise, leading whitespace is removed and 0 is returned.
    static TMovement FInput::ParseMovement( const char * & Text );
    // FindMovement is like ParseMovement, without any adjustments in the text.
    static TMovement FInput::FindMovement( const char * Text );

    //         Parse a switch name out of a text string.    
    // Look for a switch name at the beginning of Text, after any whitespace.
    // If found, move Text beyond the switch name and return the identified switch.
    // Otherwise, leading whitespace is removed and 0 is returned.
    static TSwitch FInput::ParseSwitch( const char * & Text );
    // FindSwitch is like ParseSwitch, without any adjustments in the text.
    static TSwitch FInput::FindSwitch( const char * Text );

    //            Key-Value pairs for input.
    // The recognized keys are not documented here because they tend to
    // change and any list here will soon be obsolete. Instead, the
    // Keys() function can be used to get a list.

    // Implementations of the FKeyValues interface:
    TStringList Keys() const; // What are all the keys?
    char * Value(const char * Key) const; // What is the current value for the given key? 
    BOOL SetValue(const char * Key, const char * Value); // Set the value associated with a key.

    static BOOL SetValue(const char * Key, const char * Value, TMovementInfo * Info );
        // Fill in Info (an array of MovementCount items) with the value interpreted from Key=Value.
        // Return TRUE if there was one or more errors, FALSE otherwise.
    static BOOL SetValue(const char * Pair, TMovementInfo * Info );
        // Fill in Info (an array of MovementCount items) with the value interpreted from Key=Value in Pair.
        // Return TRUE if there was one or more errors, FALSE otherwise.
    static BOOL SetValues(TStringList PairList, TMovementInfo * Info );
        // Fills in Info (an array of MovementCount items) with parsed values from PairList.
        // Return TRUE if there was one or more errors, FALSE otherwise.
    
    static const char * Name(TSwitch Switch, BOOL Descriptive) // Cryptic or descriptive switch name
    {
        return Descriptive ? Description(Switch) : Abbreviation(Switch);
    }

    static const char * Name(TMovement Movement, BOOL Descriptive) // Cryptic or descriptive movement name
    {
        return Descriptive ? Description(Movement) : Abbreviation(Movement);
    }

    static const char * Name(TMovementKind MovementKind, BOOL Descriptive) // Cryptic or descriptive TMovementKind name
    {
        return Descriptive ? Description(MovementKind) : Abbreviation(MovementKind);
    }
};

extern UNREAL_API FInput GInput;

#endif

