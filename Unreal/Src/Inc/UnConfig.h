/*==============================================================================
UnConfig.h: Unreal configuration and customization
Used by: The Windows interface and initialization code

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    This class deals with the Unreal configuration file.

Revision history:
    * 06/14/96, Created by Mark
==============================================================================*/

#ifndef _INC_UnConfig
#define _INC_UnConfig

#include "UnKeyVal.h"
class UNREAL_API FConfiguration
{

public:

    // Configuration sections
    typedef enum
    {
        NoSection       = 0 // Always 0.
    ,   InputSection        // Section describing input devices, thresholds, and so on.
    ,   ActionSection       // Section describing player game-play actions
    ,   PreferencesSection  // Section describing player preferences.
    ,   AudioSection        // Section describing audio properties.
    ,   SectionCount        // Number of sections, including NoSection.
    }
    TSection;

    static const char * SectionName(TSection Section); // The text name of a section.

    //               The section started by some text
    // Return the section begun with the given Text. Return 0 if no section matches.
    // A line which starts like this:
    //   [SectionName]
    // starts the section named SectionName.
    TSection Section(const char * Text);

    void Initialize();
    void Finalize();

    // Get a particular value from an initialization file:
    // Returns 0 if not found or if the value is empty.
    // Otherwise, free the returned string when it is no longer needed.
    char * Get
    (
        TSection        Section         // Get from this section.
    ,   const char *    Key             // Get the value for this key.
    ,   const char *    FileName = 0    // 0 means use the default file name
    );

    // Get an integer value from an initialization file.
    // Returns the specified default if not found.
    int GetInteger
    (
        TSection        Section         // Get from this section.
    ,   const char *    Key             // Get the value for this key.
    ,   int             DefaultValue    // The default value
    ,   const char *    FileName = 0    // 0 means use the default file name
    );

    // Get a boolean value from an initialization file.
    // Returns the specified default if not found.
    BOOL GetBoolean
    (
        TSection        Section         // Get from this section.
    ,   const char *    Key             // Get the value for this key.
    ,   BOOL            DefaultValue    // The default value
    ,   const char *    FileName = 0    // 0 means use the default file name
    );
    
    // Get all the profile files for a particular section, in "Key=Value" strings.
    // Free the returned list when it is no longer needed.
    static FKeyValues::TStringList GetSection
    (
        TSection        Section         // Get from this section.
    ,   const char *    FileName = 0    // 0 means use the default file name
    );

    // Interpret Text for the given Section.
    void InterpretValue
    (
        TSection        Section
    ,   const char *    Pair            // Pair should be in this form: Key = Value
        // Notes:
        //   1. If Pair starts with a comment ';', it is ignored.
    );

    // Interpret a key/value pair for the given Section.
    void InterpretValue
    (
        TSection        Section
    ,   const char *    Key
    ,   const char *    Value
    );

    // Interpret all the values in file FileName for section Section.
    void InterpretValuesFromFile
    (
        TSection        Section  = NoSection // Read values for this section, NoSection for all sections.
    ,   const char *    FileName = 0         // 0 means use the default file name
    );

    // Save values into a file.
    void WriteValuesToFile
    (
        TSection        Section  = NoSection // Write values for this section, NoSection for all sections.
    ,   const char *    FileName = 0         // 0 means use the default file name
    );


private:
    // For diagnostics:
    const char * CurrentFileName;
    int CurrentLineNumber;
};

extern UNREAL_API FConfiguration GConfiguration;

#endif
