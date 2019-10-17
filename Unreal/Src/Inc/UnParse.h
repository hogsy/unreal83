/*==============================================================================
UnParse.h: General string parsing
Used by: Whoever needs general string parsing.

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Here are some general functions to help you parse strings.

    If you want to add a function here, consider first if it is
    truly general of if it is biased towards Unreal-specific string
    parsing. If the latter, consider adding the function to the
    Get... routines defined elsewhere (such as GetBYTE). The Get..
    routines expect Unreal-particular syntax, such as in "INDEX=12".


Revision history:
    * 06/10/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnParse
#define _INC_UnParse

#include "Unreal.h"

struct UNREAL_API FParse
{
    //              Skip white space
    // Skip over any leading whitespace in Text, and
    // leave Text pointing at the first non-whitespace
    // character (or the terminating null).
    static void SkipWhiteSpace(const char * & Text);

    //                 Is the text empty?
    // Text is empty if it has no characters or only whitespace characters.
    static BOOL IsEmpty(const char * Text);

    //                     Get a word
    // Remove any leading whitespace and get the next word from Text. 
    // Text is left pointing the at the text after the parsed word,
    // with any whitespace skipped. If there is no next word,
    // Word is set to the empty string.
    // 
    // A word is one of the following:
    //   - a run of letters, numbers, and '_'
    //   - a single character which is not a letter, number, or '_'
    //
    // Up to MaxWordLength characters of the next word are copied into Word,
    // so Word must have room for MaxWordLength+1 characters (including the
    // trailing null). Regardless of how much of the word is copied to Word,
    // the entire word is parsed (Text is left pointing beyond the word).
    //
    // Return the length of the word (not necessarily the amount put into Word), 
    // or 0 if a word was not found.
    static int GetWord(const char * & Text, char * Word, int MaxWordLength );

    //              Get a single character in single quotes, such as 'x'
    // Remove any leading whitespace and get a quoted single character
    // (the character itself in singe quotes). To be clear, 3 characters are
    // expected in this order:
    //    - a single quote character '\'' 
    //    - a non-0 character
    //    - a single quote character '\'' 
    // Text is left pointing the at the text after the parsed character,
    // with any trailing whitespace skipped. 
    // The character found is returned, or if Text does not start with
    // a properly quoted single character, 0 is returned.
    static char GetQuotedCharacter(const char * & Text);

    //             Get a floating point value
    // Remove the leading whitespace and get a floating point number from Text.
    // Text is left pointing after the floating point value and any trailing whitespace.
    // If a valid floating point value is found, TRUE is returned and the value is
    // saved in Value. Otherwise, FALSE is returned and Value is undefined.
    static BOOL GetFloat(const char * & Text, float & Value );

    //        Does the text start with a given character?
    // Return TRUE if the first non-whitespace character in the Text 
    // is the given character. Return FALSE otherwise.
    // On output, Text has any leading whitespace removed, and
    // is left pointing after the given character (if it is found
    // at the beginning).
    static BOOL StartsWith(const char * & Text, char C );

    //        Does the text end with a given string?
    // Return TRUE if the text ends with the given string, FALSE otherwise.
    // Leading and trailing whitespace is ignored in both strings, and the comparison
    // is case-insensitive.
    static BOOL EndsWith(const char * Text, const char * Suffix );

    //    Make a string with room for N characters.
    // N need not include space for the trailing null.
    // The string is initially empty.
    // Free the string later with FreeString().
    //tbi: This isn't really directly related to parsing, and could perhaps have a better home elsewhere.
    static char * MakeString(int N);

    //  Make a string from the given value.
    // Free the string later with FreeString(). If Value==0, 0 is returned.
    //tbi: This isn't really directly related to parsing, and could perhaps have a better home elsewhere.
    static char * MakeString(const char * Value);

    // Create a string by catenating two strings.
    // Free the resulting string with FreeString(). If both strings are 0, 0 is returned.
    static char * MakeString(const char * String1, const char * String2 );

    // Catenate one string onto another.
    // The target string is reallocated and the old storage is freed, unless
    // the added String is 0 or empty.
    static void AddString( char * & Target, const char * String );

    //      Free a string made with a MakeString() function.
    // Does nothing if Value==0, otherwise frees the storage for Value and sets Value==0.
    //tbi: This isn't really directly related to parsing, and could perhaps have a better home elsewhere.
    static void FreeString(char * & Value);

    //           Interpret text as a true or false value.
    // This returns TRUE if Text represents a TRUE boolean value, FALSE otherwise.
    // Use InterpretBoolean() defined below if you need to know if the text represents
    // a valid boolean value.
    static BOOL InterpretBoolean(const char * Text); 

    //           Interpret text as a true or false value.
    // This returns TRUE if Text represents a TRUE boolean value, FALSE otherwise.
    // In addition, IsOkay is set to TRUE if Text is a valid boolean value, FALSE otherwise.
    static BOOL InterpretBoolean 
    (
        const char * Text
    ,   BOOL       & IsOkay
    );

    // Return text representing the boolean value Value. 
    static const char * BooleanText(BOOL Value); 

};


#endif
