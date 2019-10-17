/*==============================================================================
UnKeyVal.h: Unreal key-value pair processing.
Used by: Interfaces that deal with keyed values in text form.

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Keyed values, or key-value pairs, look like this:
       Key = Value
    For example, we might have something like this:
       Mass = 10.0

    A class might find the declarations here useful if it wants to 
    provide a uniform way of interpreting text input from a user
    interface (command-line or .ini file), or a text-based way of
    representing the class state (for debugging or writing to a .ini file).

Revision history:
    * 06/15/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnKeyVal
#define _INC_UnKeyVal

#include "Unreal.h"
#include "UnParse.h"

// A class for managing key-value pairs:
class UNREAL_API FKeyValues
{
public:
    enum
    {
        MaxKeyLength    = 100    // Maximum length of Key in "Key=Value" pair.
    ,   MaxValueLength  = 1000   // Maximum length of Value in "Key=Value" pair.
    };

    // Definitions:
    //   A string list is a list of strings where each string is terminated
    //   by a null character and last string is terminated with two null characters.
    //   Note that there can't be an empty string in the list, since such a
    //   string would cause 2 nulls and be considered as the end of the list.
    struct UNREAL_API TStringList
    {
        char * Strings;
        TStringList() { Strings = 0; }
        void Add(const char * String); // Add the string to the list. If String is 0 or empty, add a blank string.
        void Add(TStringList List); // Add the string list to the list.
        const TStringList Next() const; // Return the rest of the list after the first string in the list.
            // If there are no more strings, a list with IsEmpty() is returned.
            // Note: Next() returns a list pointing within the list *this, not a newly allocated list.
        int Size() const; // How many bytes in the whole list, including the nulls at the end of the strings but not the 2nd terminating null.
        void Free() { FParse::FreeString(Strings); }
        const char * First() const { return Strings; }
        BOOL IsEmpty() const { return Strings==0 || Strings[0]==0; }
    };

    //                   What are all the keys?
    // Returns a string list of all the keys. The list should be freed.
    virtual TStringList Keys() const = 0;

    //        What is the current value for the given key? 
    // Returns 0 if the key is not recognized. Otherwise, the returned value should be freed.
    virtual char * Value(const char * Key) const = 0;

    //        What are the current values for the given keys? 
    // Return a string list with one string for each Key in the KeyList (which is a string list).
    // The returned value should be freed. There will be the same number of strings in the 
    // returned list as there are in KeyList.
    TStringList Values(TStringList KeyList) const;

    //        Set the value associated with a text key and value.
    // Return TRUE if the value is set okay, FALSE if there is an error
    // (invalid Key, or invalid Value).
    virtual BOOL SetValue(const char * Key, const char * Value) = 0;

    //        Set the value associated with a text Key=Value pair.
    // Return TRUE if the value is set okay, FALSE if there is an error.
    // The Pair is expected to be in the form: Key = Value, or 
    // a comment starting with ';'.
    BOOL SetValue(const char * Pair); 

    //        Set the values associated with a list of Key=Value pairs.
    // Return TRUE if the values were all set okay, FALSE otherwise.
    // If FALSE is returned, some values may have been set successfully
    // Pairs is a string list of strings with this form: Key = Value.
    BOOL SetValues(TStringList PairList); 

    //       Return the key-value pair for a given key.
    // Returns 0 if the key is not recognized. The returned text is in
    // the form: Key = Value. The returned value should be freed if not 0.
    char * Pair(const char * Key) const;

    //    Return a list of key-value pairs for all the given keys.
    // Unrecognized keys are not included in the resulting list.
    // The returned list should be freed.
    TStringList Pairs(TStringList KeyList) const;

    //   Return a list of all the key-value pairs.
    // The returned list should be freed.
    TStringList Pairs() const;

};

#endif
