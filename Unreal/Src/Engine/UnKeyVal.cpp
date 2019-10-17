/*==============================================================================
UnKeyVal.cpp: Unreal key-value pair processing

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 06/15/96: Created by Mark
==============================================================================*/

#include "UnKeyVal.h"
#include "UnParse.h"

static const BOOL Debugging = FALSE; // TRUE to enable debugging, FALSE otherwise.

static inline void __cdecl Debug(const char * Message, ...)
{
    if( Debugging )
    {
        char Text[1000];      
        va_list ArgumentList;
        va_start(ArgumentList,Message);
        vsprintf(Text,Message,ArgumentList);
        va_end(ArgumentList);
        debug(LOG_Info,Text);
    }
}

static void Debug( const char * Intro, FKeyValues::TStringList List )
{
    if( Debugging )
    {
        debug( LOG_Info, Intro );
        int Which = 1;
        while( !List.IsEmpty() )
        {
            debugf( LOG_Info, " [%2i] %s", Which, List.First() );
            List = List.Next();
            Which++;
        }
    }
}

#if 0 //tbd:
//----------------------------------------------------------------------------
//                Make a pair out of text in the form Key=Value
//----------------------------------------------------------------------------
void FKeyValue::TPair::Make(const char * Pair)
{
    Free();
    PairText = FParse::MakeString(Pair);
    Debug( "TPair::Make(%s)", Pair );
}

//----------------------------------------------------------------------------
//                Make a pair out of Key and Value
//----------------------------------------------------------------------------
void FKeyValue::TPair::Make(const char * Key, const char * Value)
{
    const char Length = 
        ( Key==0 ? 0 : strlen(Key) )
    +   1                            // For separating '='
    +   ( Value==0 ? 0 : strlen(Value) )
    ;
    char * Pair = FParse::MakeString(Length);
    Pair[0] = 0;
    if( Key != 0 ) { strcat(Pair,Key); }
    strcat(Pair,"=");
    if( Value != 0 ) { strcat(Pair,Value); }
    Make(Pair);
    FParse::FreeString(Pair);
}

//----------------------------------------------------------------------------
//                Make room for a new pair-list
//----------------------------------------------------------------------------
void FKeyValue::TPairs::MakeRoomFor(int PairLength)
{
    const int ApproximateUnusedSpace = Space==0 ? 0 : Space - Next;
    if( ApproximateUnusedSpace <= PairLength + 20 ) // +20 for laziness and robustness
    {
        const int AdditionalSpace = OurMax(PairLength,400);
        const int NewSpace = Space + AdditionalSpace + 20; // +20 for nulls and general laziness and robustness.
        char * NewString = FParse::MakeString(NewSpace);
        if( Space > 0 )
        {
            memmove( NewString, PairsText, Space );
        }
        else 
        {
            Next = 0;
            // Double-null-terminate the list:
            NewString[Next] = 0;
            NewString[Next+1] = 0;
        }
        Free(); // Delete old pair.
        Space = NewSpace;
        PairsText = NewString;
    }
}

//----------------------------------------------------------------------------
//                Add a key-value pair to the list
//----------------------------------------------------------------------------
void FKeyValue::TPairs::Add(const TPair & Pair)
{
    if( Pair() != 0 )
    {
        const int PairLength = strlen(Pair());
        MakeRoomFor( PairLength );
        memmove( &PairsText[Next], Pair(), PairLength+1 ); // +1 to grab trailing null.
        Next += PairLength+1; // Next now points beyond trailing null just added.
        PairsText[Next] = 0; // Double-null-terminate the list.
        Debug( "TPairs::Add(%s)", Pair() );
    }
}

//----------------------------------------------------------------------------
//                Add a key-value pair to the list
//----------------------------------------------------------------------------
void FKeyValue::TPairs::Add(const char * Key, const char * Value)
{
    TPair Pair;
    Pair.Make(Key,Value);
    Add( Pair );
}
#endif 

//----------------------------------------------------------------------------
//        What are the current values for the given keys? 
//----------------------------------------------------------------------------
FKeyValues::TStringList FKeyValues::Values(TStringList KeyList) const
{
    TStringList Values;
    TStringList Keys = KeyList; // Keys.String will iterate over each key.
    while( !Keys.IsEmpty() )
    {
        Values.Add( this->Value(Keys.First()) );
        Keys = Keys.Next();
    }
    return Values;
}

//----------------------------------------------------------------------------
//        Set the value associated with a text Key=Value pair.
//----------------------------------------------------------------------------
BOOL FKeyValues::SetValue(const char * Pair)
{
    const char * Text = Pair;
    BOOL IsOkay = FALSE;
    if( FParse::StartsWith(Text,';') )
    {
        // Comment line - ignore it.
        IsOkay = TRUE;
    }
    else
    {
        char Key[MaxKeyLength+1]; // +1 for trailing null.
        if( FParse::GetWord(Text,Key,MaxKeyLength) && FParse::StartsWith(Text,'=') )
        {
            FParse::SkipWhiteSpace(Text);
            Debug( "Found key: |%s| value: |%s|", Key, Text );
            IsOkay = SetValue( Key, Text );
        }
    }
    return IsOkay;
}

//----------------------------------------------------------------------------
//        Set the values associated with a list of Key=Value pairs.
//----------------------------------------------------------------------------
BOOL FKeyValues::SetValues(TStringList PairList)
{
    TStringList Pairs = PairList; // Pairs will iterate over each pair.
    BOOL IsOkay = TRUE;
    while( !Pairs.IsEmpty() )
    {
        if( !SetValue(Pairs.First()) )
        {
            IsOkay = FALSE;
        }
        Pairs = Pairs.Next();
    }
    return IsOkay;
}

//----------------------------------------------------------------------------
//       Return the key-value pair for a given key.
//----------------------------------------------------------------------------
char * FKeyValues::Pair(const char * Key) const
{
    char * Pair = 0;
    if( Key != 0 )
    {
        char * Value = this->Value(Key);
        Pair = FParse::MakeString
        ( 
                strlen(Key) 
            +   1                // For '='
            +   ( Value==0 ? 0 : strlen(Value) )
        );
        strcpy( Pair, Key );
        strcat( Pair, "=" );
        if( Value != 0 )
        {
            strcat( Pair, Value );
        }
        FParse::FreeString(Value);
    }
    return Pair;
}

//----------------------------------------------------------------------------
//    Return a list of key-value pairs for all the given keys.
//----------------------------------------------------------------------------
FKeyValues::TStringList FKeyValues::Pairs(TStringList KeyList) const
{
    TStringList Keys = KeyList;
    TStringList Pairs;
    while( !Keys.IsEmpty() )
    {
        char * Pair = this->Pair(Keys.First());
        if( Pair != 0 )
        {
            Pairs.Add( Pair );
            FParse::FreeString(Pair);
        }        
        Keys = Keys.Next();
    }
    Debug( "::Pairs(Keys), Keys are:", Keys );
    Debug( "::Pairs(Keys), result is:", Pairs );
    return Pairs;
}

//----------------------------------------------------------------------------
//          Return a list of all the key-value pairs.
//----------------------------------------------------------------------------
FKeyValues::TStringList FKeyValues::Pairs() const
{
    TStringList AllKeys = this->Keys();
    TStringList Pairs = this->Pairs(AllKeys);
    AllKeys.Free();
    return Pairs;
}

#if 0 //tbd:
//----------------------------------------------------------------------------
//                   Returns a new empty list.
//----------------------------------------------------------------------------
FKeyValues::TStringList FKeyValues::TStringList::New()
{
    TStringList List;
    List.Strings = FParse::MakeString(2);
    memmove(List.Strings, "\0\0", 2 ); // Empty list has 2 null terminators.
    return List;
}
#endif

//----------------------------------------------------------------------------
//    Return a pointer to the next string in the list, or 0 if none.
//----------------------------------------------------------------------------
const FKeyValues::TStringList FKeyValues::TStringList::Next() const
{
    char * Check = this->Strings;
    if( Check != 0 )
    {
        Check += strlen(Check)+1; // +1 to skip string's trailing null.
        if( Check[0] == 0 ) // We reached the end of the list.
        {
            Check = 0;
        }
    }
    TStringList Next;
    Next.Strings = Check;
    return Next;
}

//----------------------------------------------------------------------------
//              How many bytes in the whole list?
//----------------------------------------------------------------------------
int FKeyValues::TStringList::Size() const
{
    int Size = 0;
    if( !IsEmpty() )
    {
        char * Check = Strings;
        while( Check[0] != 0 ) // Look for (second) terminating null.
        {
            Check += strlen(Check)+1; // +1 to skip string's single terminating null.
        }
        Size = Check - Strings;
    }
    return Size;
}

//----------------------------------------------------------------------------
//              Add a string to a list.
//----------------------------------------------------------------------------
void FKeyValues::TStringList::Add(const char * String)
{
    GUARD;
    if( String==0 || String[0]==0 )
    {
        String =  " "; // Add a blank string (instead of an empty string).
    }
    const int AddLength = strlen(String)+1; // +1 to add trailing null.
    const int OldSize = Size();
    const int NewSize = OldSize + AddLength;
    char * OldStrings = Strings;
    char * NewStrings = FParse::MakeString(NewSize+1); //+1 for second null in double-null termination.
    memmove( &NewStrings[0], OldStrings, OldSize ); // Copy old strings.
    memmove( &NewStrings[OldSize], String, AddLength ); // Add new string (with null).
    NewStrings[NewSize] = 0; // Add the second null terminator to terminate the whole list.
    Strings = NewStrings;
    FParse::FreeString(OldStrings);
    UNGUARD( "TStringList::Add" );
}

//----------------------------------------------------------------------------
//              Add a list to a list.
//----------------------------------------------------------------------------
void FKeyValues::TStringList::Add(TStringList List)
{
    GUARD;
    UNGUARD( "TStringList::Add" );
}
