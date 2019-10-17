/*==============================================================================
UnParse.cpp: General parsing.

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 06/10/96: Created by Mark
==============================================================================*/

#include "UnParse.h"
#include <ctype.h>

//----------------------------------------------------------------------------
//                   Skip any whitespace
//----------------------------------------------------------------------------
void FParse::SkipWhiteSpace(const char * & Text)
{
    while( isspace(Text[0]) )
    {
        Text++;
    }
}

//----------------------------------------------------------------------------
//                     Is the text empty?
//----------------------------------------------------------------------------
BOOL FParse::IsEmpty(const char * Text)
{
    const char * Check = Text;
    SkipWhiteSpace(Check);
    return Check[0] == 0;
}

//----------------------------------------------------------------------------
//                         Get a word
//----------------------------------------------------------------------------
int FParse::GetWord(const char * & Text, char * Word, int MaxWordLength )
{
    int WordLength = 0;
    SkipWhiteSpace(Text);
    Word[0] = 0;
    if( Text[0] == 0 ) // No word found.
    {
    }
    else if( isalnum(Text[0]) || Text[0]=='_' ) // A word, possibly multicharacter
    {
        while( isalnum(Text[0]) || Text[0] == '_' )
        {
            WordLength++;
            if( WordLength <= MaxWordLength )
            {
                Word[WordLength-1]  = Text[0]   ;
                Word[WordLength]    = 0         ;
            }
            Text++;
        }
    }
    else // A special character (single-character word)
    {
        WordLength++;
        if( WordLength <= MaxWordLength )
        {
            Word[WordLength-1]  = Text[0]   ;
            Word[WordLength]    = 0         ;
        }
        Text++;
    }
    SkipWhiteSpace(Text);
    return WordLength;
}

//----------------------------------------------------------------------------
//          Does the text start with a given character?
//----------------------------------------------------------------------------
BOOL FParse::StartsWith(const char * & Text, char C )
{
    SkipWhiteSpace(Text);
    const BOOL Found = Text[0]==C;
    if( Found )
    {
        Text++;
    }
    return Found;
}

//----------------------------------------------------------------------------
//         Get a single character in single quotes, such as 'x'
//----------------------------------------------------------------------------
char FParse::GetQuotedCharacter(const char * & Text)
{
    char Char = 0;
    SkipWhiteSpace(Text);
    if( Text[0]=='\'' && Text[1]!=0 && Text[2]=='\'' )
    {
        Char = Text[1];
        Text += 3; // Skip over '?'
        SkipWhiteSpace(Text);
    }
    return Char;
}

//----------------------------------------------------------------------------
//               Get a floating-point value
//----------------------------------------------------------------------------
static inline int CountDigits(const char * & Text)
{
    int Count = 0;
    while( isdigit( Text[0] ) ) { Count++; Text++; }
    return Count;
}
static inline BOOL SkipChar(const char * & Text, char C)
{
    const BOOL Matched = Text[0] == C;
    if( Matched ) { Text++; }
    return Matched;
}
static inline BOOL SkipChars(const char * & Text, char C1, char C2)
{
    const BOOL Matched = Text[0] == C1 || Text[0] == C2;
    if( Matched ) { Text++; }
    return Matched;
}
BOOL FParse::GetFloat(const char * & Text, float & Value )
{
    SkipWhiteSpace(Text);
    // Check the syntax:
    //  [sign][digits][.digits][{e|E}[sign]digits]
    const char * CheckText = Text;
    SkipChars(CheckText,'-','+');
    int DigitCount = CountDigits(CheckText);
    SkipChar(CheckText,'.');
    DigitCount += CountDigits(CheckText);
    const BOOL HasExponent = SkipChars(CheckText,'e','E');
    int ExponentDigits = 0;
    if( HasExponent )
    {
        SkipChars(CheckText,'+','-');
        ExponentDigits += CountDigits(CheckText);
    }
    BOOL Found = 
        DigitCount > 0
    &&  !HasExponent || ExponentDigits > 0
    ;
    if( Found )
    {
        Value = atof(Text);
        Text = CheckText;
    }
    return Found;
}

//----------------------------------------------------------------------------
//        Does the text end with a given string?
//----------------------------------------------------------------------------
BOOL FParse::EndsWith(const char * Text, const char * Suffix )
{
    BOOL Matches = TRUE;
    SkipWhiteSpace(Text);
    SkipWhiteSpace(Suffix);
    const char * CheckText   = Text   + strlen(Text)   ; // Start at end of text.
    const char * CheckSuffix = Suffix + strlen(Suffix) ; // Start at end of suffix.

    while( CheckText > Text && isspace(CheckText[0]) ) // Remove trailing whitespace
    {
        --CheckText;
    }
    while( CheckSuffix > Suffix && isspace(CheckSuffix[0]) ) // Remove trailing whitespace
    {
        --CheckSuffix;
    }
    // Scan backwards until we exhaust CheckText, CheckSuffix, or until
    // we find a mismatched character:
    while( CheckText > Text && CheckSuffix > Suffix && Matches )
    {
        // Check preceding character.
        CheckText--;
        CheckSuffix--;
        const char TextChar = *CheckText;
        const char SuffixChar = *CheckSuffix;
        Matches = tolower(TextChar) == tolower(SuffixChar);
    }
    // We must have matched all characters and exhausted the suffix:
    return Matches && CheckSuffix <= Suffix;
}


//----------------------------------------------------------------------------
//        Make a string with room for N characters.
//----------------------------------------------------------------------------
char * FParse::MakeString(int N)
{
    char * String = appMallocArray((N > 0 ? N : 0) + 1, char, "MakeString"); //+1 for trailing null.
    String[0] = 0; // Empty the string.
    return String;
}

//----------------------------------------------------------------------------
//          Make a string with a given value.
//----------------------------------------------------------------------------
char * FParse::MakeString(const char * Value)
{
    char * String = 0;
    if( Value != 0 )
    {
        const int Length = strlen(Value);
        String = MakeString( Length );
        memmove( String, Value, Length+1 ); // +1 for trailing null.
    }
    return String;
}
//----------------------------------------------------------------------------
//              Create a string by catenating two strings.
//----------------------------------------------------------------------------
char * FParse::MakeString(const char * String1, const char * String2 )
{
    char * String;
    if( String1 == 0 )
    {
        String = MakeString(String2);
    }
    else if( String2 == 0 )
    {
        String = MakeString(String1);
    }
    else
    {
        const int Length1 = strlen(String1);
        const int Length2 = strlen(String2);
        String = MakeString( Length1 + Length2 + 1 ); // +1 for trailing null.
        memmove( &String[0], String1, Length1 );
        memmove( &String[Length1], String2, Length2+1 ); // +1 to add trailing null.
    }
    return String;
}

//----------------------------------------------------------------------------
//              Catenate one string onto another.
//----------------------------------------------------------------------------
void FParse::AddString( char * & Target, const char * String )
{
    const int AddedLength = String == 0 ? 0 : strlen(String);
    if( AddedLength != 0 )
    {
        char * NewString = MakeString(Target,String);
        FreeString(Target);     
        Target = NewString;  
    }
}

//----------------------------------------------------------------------------
//        Free a string made with a MakeString() function.
//----------------------------------------------------------------------------
void FParse::FreeString(char * & Value)
{
    GUARD;
    if( Value != 0 )
    {
        appFree( Value );
        Value = 0;
    }
    UNGUARD( "FParse::FreeString" );
}

//----------------------------------------------------------------------------
//                      Interpret a boolean value
//----------------------------------------------------------------------------
BOOL FParse::InterpretBoolean(const char * Text)
{
    BOOL IsOkay;
    return InterpretBoolean(Text,IsOkay);
}

//----------------------------------------------------------------------------
//                      Interpret a boolean value
//----------------------------------------------------------------------------
BOOL FParse::InterpretBoolean
(
    const char * Text
,   BOOL       & IsOkay
)
{
    BOOL Value = FALSE;
    IsOkay = FALSE;
    if( Text==0 )
    {
    }
    else if( stricmp(Text,"true") == 0 )
    {
        Value = TRUE;
        IsOkay = TRUE;
    }
    else if( stricmp(Text,"false") == 0 )
    {
        Value = FALSE;
        IsOkay = TRUE;
    }
    return Value;
}

//----------------------------------------------------------------------------
//        Return text representing the boolean value Value. 
//----------------------------------------------------------------------------
const char * FParse::BooleanText(BOOL Value)
{
    // This seems like a pathetic function, but if we use it for expressing
    // all boolean values we are guaranteed consistency.
    return Value ? "True" : "False";
}


