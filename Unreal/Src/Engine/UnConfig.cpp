/*==============================================================================
UnConfig.cpp: Unreal configuration processing

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 06/14/96: Created by Mark
==============================================================================*/

#include "Unreal.h"

#include "UnConfig.h"
#include "UnParse.h"
#include "UnKeyVal.h"
#include "UnInput.h"
#include "UnAction.h"
#include "UnPrefer.h"
#include "UnChecks.h"
#include "UnFGAud.h"
#include <stdlib.h>
#include <direct.h>

FConfiguration GConfiguration;

static const BOOL Debugging = FALSE; // TRUE to enable debugging, FALSE otherwise.

static const int MaxSectionLength = 32000; // Maximum length of text in a profile section.
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

#define arrayCount_(Array) ( sizeof(Array) / sizeof((Array)[0]) )

//----------------------------------------------------------------------------
//                Initialization
//----------------------------------------------------------------------------
void FConfiguration::Initialize()
{
    GUARD;
    //tbi: These belong elsewhere.
    GInput.Initialize();
    GAction.Initialize();
    InterpretValuesFromFile(FConfiguration::NoSection);
    UNGUARD( "FConfiguration::Initialize" );
}

//----------------------------------------------------------------------------
//                Finalize
//----------------------------------------------------------------------------
void FConfiguration::Finalize()
{
    //tbi: These belong elsewhere.
    GInput.Finalize();
    GAction.Finalize();
}

//----------------------------------------------------------------------------
//                Map a section to its name
//----------------------------------------------------------------------------
static const char * SectionNames[FConfiguration::SectionCount] =
{
    0               // NoSection
,   "Input"         // InputSection
,   "Action"        // ActionSection
,   "Preferences"   // PreferencesSection
,   "Audio"         // AudioSection
};

//----------------------------------------------------------------------------
//                What is the name of a section?
//----------------------------------------------------------------------------
const char * FConfiguration::SectionName(TSection Section)
{
    return SectionNames[Section];
}

//----------------------------------------------------------------------------
//       Get all the values for a section from an initialization file.
//----------------------------------------------------------------------------
FKeyValues::TStringList FConfiguration::GetSection
(
    TSection        Section         // Get from this section.
,   const char *    FileName        // 0 means use the default file name
)
{
    FKeyValues::TStringList List;
    List.Strings = FParse::MakeString(MaxSectionLength);
    GApp->GetProfileSection( SectionName(Section), List.Strings, MaxSectionLength, FileName );
    return List;
}

//----------------------------------------------------------------------------
//       Get a particular value from an initialization file
//----------------------------------------------------------------------------
char * FConfiguration::Get
(
        TSection        Section         // Get from this section.
    ,   const char *    Key             // Get the value for this key.
    ,   const char *    FileName        // 0 means use the default file name
)
{
    char * Value = 0;
    char Text[FKeyValues::MaxValueLength+1]; // +1 for trailing null.
    if
    ( 
        GApp->GetProfileValue
        (
            SectionName(Section)
        ,   Key
        ,   0
        ,   Text
        ,   sizeof(Text)
        ,   FileName
        )
    )
    {
        Value = FParse::MakeString(Text);
    }
    return Value;  
}

//----------------------------------------------------------------------------
//         Get an integer value from an initialization file.
//----------------------------------------------------------------------------
int FConfiguration::GetInteger
(
    TSection        Section         // Get from this section.
,   const char *    Key             // Get the value for this key.
,   int             DefaultValue    // The default value
,   const char *    FileName        // 0 means use the default file name
)
{
    return GApp->GetProfileInteger
    (
        SectionName(Section)
    ,   Key
    ,   DefaultValue
    ,   FileName
    );
}

//----------------------------------------------------------------------------
//           Get a boolean value from an initialization file.
//----------------------------------------------------------------------------
BOOL FConfiguration::GetBoolean
(
    TSection        Section         // Get from this section.
,   const char *    Key             // Get the value for this key.
,   BOOL            DefaultValue    // The default value
,   const char *    FileName        // 0 means use the default file name
)
{
    return GApp->GetProfileBoolean
    (
        SectionName(Section)
    ,   Key
    ,   DefaultValue
    ,   FileName
    );
}

//----------------------------------------------------------------------------
//                What is the section started by Text?
//----------------------------------------------------------------------------
FConfiguration::TSection FConfiguration::Section(const char * Text)
{
    TSection Section = NoSection;
    if( FParse::StartsWith(Text,'[') )
    {
        char Name[101];
        if( FParse::GetWord(Text,Name,100) )
        {
            for( int CheckSection_ = 1; Section==0 && CheckSection_ < SectionCount; ++CheckSection_ )
            {
                const TSection CheckSection = TSection(CheckSection_);
                if( stricmp( Name, SectionName(CheckSection) ) == 0 )
                {
                    Section = CheckSection;
                }
            }
        }
    }
    return Section;
}

//----------------------------------------------------------------------------
//             Intepret the value in a section.
//----------------------------------------------------------------------------
void FConfiguration::InterpretValue
(
    TSection        Section
,   const char *    Pair        // Text should be in this form: Key = Value
)
{
    GUARD;
    const char * Text = Pair;
    if( FParse::StartsWith(Text,';') )
    {
        // Comment line - ignore it.
    }
    else
    {
        Debug( "InterpretValue(%s,%s)", SectionName(Section), Text );
        char Key[FKeyValues::MaxKeyLength+1]; // +1 for trailing null.
        if( FParse::GetWord(Text,Key,FKeyValues::MaxKeyLength) && FParse::StartsWith(Text,'=') )
        {
            FParse::SkipWhiteSpace(Text);
            Debug( "Found key: |%s| value: |%s|", Key, Text );
            InterpretValue( Section, Key, Text );
        }
        else
        {
            if( CurrentFileName != 0 )
            {
                debugf( LOG_Critical, "Error in file %s:", CurrentFileName );
            }
            debugf( LOG_Critical, "Error: %s", Pair );
        }
    }
    UNGUARD( "FConfiguration::InterpretValue" );
}


//----------------------------------------------------------------------------
//              Interpret a key/value pair for a section.
//----------------------------------------------------------------------------
void FConfiguration::InterpretValue
(
    TSection        Section
,   const char *    Key
,   const char *    Value
)
{
    GUARD;
    Debug( "InterpretValue(%s,%s,%s)", SectionName(Section), Key, Value );
    BOOL IsOkay = TRUE; // Set to false if unsuccessful.
    switch(Section)
    {
        case NoSection:
        {
            break;
        }
        case InputSection:
        {
            IsOkay = GInput.SetValue(Key,Value);
            break;
        }
        case ActionSection:
        {
            IsOkay = GAction.SetValue(Key,Value);
            break;
        }
        case PreferencesSection:
        {
            IsOkay = GPreferences.SetValue(Key,Value);
            break;
        }
        case AudioSection:
        {
            // We don't interpret the audio information here, instead the
            // audio management class fetches the values it needs.
            IsOkay = TRUE;
            break;
        }
        default:
        {
            debugf(LOG_Info,"Bad section");
            break;
        }
    }
    if( !IsOkay )
    {
        if( CurrentFileName != 0 )
        {
            debugf( LOG_Critical, "Error in file %s:", CurrentFileName );
        }
        debugf( LOG_Critical, "Error: %s = %s", Key, Value );
    }
    UNGUARD( "FConfiguration::InterpretValue" );
}

//----------------------------------------------------------------------------
//                      Set values from a file.
//----------------------------------------------------------------------------
void FConfiguration::InterpretValuesFromFile
(
    TSection        Section         // Read values for this section, NoSection for all sections.
,   const char *    FileName        // 0 means use the default file name
)
{
    GUARD;
    CurrentFileName = FileName==0 ? GApp->DefaultProfileFileName() : FileName;
    CurrentLineNumber = 0;
    if( Section == 0 ) // Do all sections?
    {
        for( int Section_ = 1; Section_ < SectionCount; ++Section_ )
        {
            const TSection Section = TSection(Section_);
            InterpretValuesFromFile(Section, FileName);
        }
        WriteValuesToFile(NoSection,"\\unreal\\system\\Debug.Ini"); //tbd!
    }
    else
    {
        char * Values = FParse::MakeString(MaxSectionLength);
        const char * SectionName = this->SectionName(Section);
        // Get a null-separated, double-null-terminated list of "Key=Value" pairs:
        GApp->GetProfileSection( SectionName, Values, MaxSectionLength, FileName );
        const char * Value = Values; // Value points into Values as we process each new value.
        while( Value[0] != 0 || Value[1] != 0 ) // While not at end of double-null-terminated list
        {
            if( Value[0] == 0 )
            {
                Value++; // Skip over separating null.
            }
            else
            {
                InterpretValue( Section, Value );
                Value += strlen(Value);
            }
        }
        FParse::FreeString( Values );
    }
    CurrentFileName   = 0;
    CurrentLineNumber = 0;
    UNGUARD( "FConfiguration::InterpretValuesFromFile" );
}

//----------------------------------------------------------------------------
//                   Value pairs for the Audio section
//----------------------------------------------------------------------------
static FKeyValues::TStringList AudioPairs()
{
    FKeyValues::TStringList Pairs;
    char Pair[FKeyValues::MaxKeyLength+FKeyValues::MaxValueLength+1+1]; // +1 for '=', +1 for trailing null
    sprintf( Pair, "SoundVolume=%i", GAudio.SfxVolumeGet() );
    Pairs.Add(Pair);
    sprintf( Pair, "MusicVolume=%i", GAudio.MusicVolumeGet() );
    Pairs.Add(Pair);
    sprintf( Pair, "UseDirectSound=%s", FParse::BooleanText(GAudio.DirectSoundFlagGet()) );
    Pairs.Add(Pair);
    //todo? Add sampling rate value...
    return Pairs;
}

//----------------------------------------------------------------------------
//                      Save values into a file
//----------------------------------------------------------------------------
void FConfiguration::WriteValuesToFile
(
    TSection        Section         // Write values for this section, NoSection for all sections.
,   const char *    FileName        // Write values into this file.
)
{
    GUARD;
    CurrentFileName = FileName==0 ? GApp->DefaultProfileFileName() : FileName;
    CurrentLineNumber = 0;
    if( Section == 0 ) // Do all sections?
    {
        for( int Section_ = 1; Section_ < SectionCount; ++Section_ )
        {
            const TSection Section = TSection(Section_);
            WriteValuesToFile( Section, FileName );
        }
    }
    else
    {
        FKeyValues::TStringList Pairs;
        switch(Section)
        {
            case InputSection       : { Pairs = GInput.Pairs()      ; break; }
            case ActionSection      : { Pairs = GAction.Pairs()     ; break; }
            case PreferencesSection : { Pairs = GPreferences.Pairs(); break; }
            case AudioSection       : { Pairs = AudioPairs()        ; break; }
            default: checkVital(FALSE,"Logic");
        }
        GApp->PutProfileSection( SectionName(Section), Pairs.Strings, FileName );
        Pairs.Free();
    }
    CurrentFileName   = 0;
    CurrentLineNumber = 0;
    UNGUARD( "FConfiguration::WriteValuesToFile" );
}
