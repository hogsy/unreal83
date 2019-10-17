/*==============================================================================
UnChecks.h: Help simplify and unify checking code
Used by: Various

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

Summary:
    Below is a summary of the function declarations for the checking 
    functions (although they might actually be implemented as macros).
        void checkInput  (BOOL Condition);
        void checkLogic  (BOOL Condition);
        void checkOutput (BOOL Condition);
        void checkState  (BOOL Condition);
        void checkVital  (BOOL Condition, const char * Message);
    The following debug checks are similar to the corresponding
    simple checks, but they are active only if _DEBUG is defined.
    Use the debug checks for checks you refuse to have in the
    release code.
        void debugInput  (BOOL Condition);
        void debugLogic  (BOOL Condition);
        void debugOutput (BOOL Condition);
        void debugState  (BOOL Condition);

Description:

    This header declares and defines objects to ease the addition of
    checking code to your programs. assert() is a standard way of 
    doing checks. The disadvantages of assert() are:
       - Assertions are usually turned off for release builds, but you
         might want to keep some important assertions. Other assertions
         are too expensive to keep. There is no standard way to 
         distinguish between these two and handle them differently.
       - You usually don't have much control over how a failed assertion
         is handled. You might want a project-specific behaviour for all
         failed assertions. You might want a different behaviour in a debug 
         build than the behaviour in a release build.
       - Assertions can be costly when left on (in particular, a lot of
         strings are created - the larger the assertion expressions, the
         larger the string space needed).
    The intent of this package is to provide a simple checking interface 
    whose implementation details can be tailored to a specific project.

    Some things are just too costly to check in released code.
    We introduce the notions of a simple check and a debug check, 
    and we make an arbitrary distinction between the two.

Definitions:
    debug check:
        A check which does not belong in release code (for whatever reason
        you decide, such as prohibitive cost).
    input check:
        A check for valid argments passed to a called function. Sometimes
        called the pre-conditions for a function. Examples:
          - A list index function checks that the index is not too large.
    logic check:
         A localized check of the logic within a function or the correctness 
         of a function's local state. The expectation is that such checks 
         are normally removed from the released code.
    output check:
        A check for correct output from a called function. Sometimes
        called the post-conditions for a function. Examples:
          - A sort() function checks that it does indeed sort an input array.
    simple check:
        A check which would be reasonable to leave in the release code.
    state check:
        A check of the persistent state of an object or of the relationships
        between a persistent set of objects.
    vital check:
        An important check for critical, unrecoverable errors that could
        happen at run-time. Such checks are expected to be left in the 
        release code, although their behaviour in that code might differ
        from their behaviour in the debug code.
        Examples:
          - A function malloc's a vital object, and checks to make sure
            there was enough free store to hold the object.

Notes:
  1. A vital check is the only check for which text may be given.
     The other kinds of checks, when they fail, provide implicit information 
     such as the text of the failed condition, the source file name, and
     the source file line number.

Requirements for this package:
  1. There should only be a few kinds of assertions - 5 or less. Any more
     would be too complicated and hard to remember.
  2. The interface to the user of this package should be the same regardless
     of the actual implementation.
  3. When turned off, checks should cause little or no overhead.

Revision history:
    * 03/22/96, Created by Mark
==============================================================================*/

#ifndef _INC_CHECKS
#define _INC_CHECKS

#ifdef _DEBUG
    // Debug code: Use 1 to enable a check, or 0 to disable it.
    // You may use these macros in your own code: if(CHECK_INPUT) ...
    #define CHECK_INPUT  1
    #define CHECK_LOGIC  1
    #define CHECK_OUTPUT 1
    #define CHECK_STATE  1
#else
    // Release code: Use 1 to enable a check, or 0 to disable it.
    // You may use these macros in your own code: if(CHECK_INPUT) ...
    #define CHECK_INPUT  1
    #define CHECK_LOGIC  0
    #define CHECK_OUTPUT 1
    #define CHECK_STATE  1
#endif

// Pick one of these - whichever is best for you.
// The more information provided to checkFailed, the more useful
// the information, but the larger the code.
#define CHECK_FAILED(Condition,SourceFileName,LineNumber) checkFailed( #Condition, SourceFileName, LineNumber )
//#define CHECK_FAILED(Condition,SourceFileName,LineNumber) checkFailed( __FILE__, __LINE__ )
//#define CHECK_FAILED(Condition,SourceFileName,LineNumber) checkFailed( #Condition )
//#define CHECK_FAILED(Condition,SourceFileName,LineNumber) checkFailed( __LINE__ )
//#define CHECK_FAILED(Condition,SourceFileName,LineNumber) checkFailed()

// Here is a convenience definition for when you want to do something
// conditionally for debug code only, but you don't want to use
// #if and the macro preprocessor:
#if _DEBUG
    static const int DebugCode = 1;
#else
    static const int DebugCode = 0;
#endif

#if CHECK_INPUT
    #define checkInput(Condition) (void) ( (Condition) || ( CHECK_FAILED( (Condition), __FILE__, __LINE__ ), 0 ) )
#else
    #define checkInput(Condition) 
#endif

#if CHECK_LOGIC
    #define checkLogic(Condition) (void) ( (Condition) || ( CHECK_FAILED( (Condition), __FILE__, __LINE__ ), 0 ) )
#else
    #define checkLogic(Condition) 
#endif

#if CHECK_OUTPUT
    #define checkOutput(Condition) (void) ( (Condition) || ( CHECK_FAILED( (Condition), __FILE__, __LINE__ ), 0 ) )
#else
    #define checkOutput(Condition) 
#endif

#if CHECK_STATE
    #define checkState(Condition) (void) ( (Condition) || ( CHECK_FAILED( (Condition), __FILE__, __LINE__ ), 0 ) )
#else
    #define checkState(Condition) 
#endif

#ifdef _DEBUG
    // Debug checks are the same as simple checks when debugging:
    #define debugInput(Condition)  checkInput(Condition)  
    #define debugLogic(Condition)  checkLogic(Condition)  
    #define debugOutput(Condition) checkOutput(Condition)  
    #define debugState(Condition)  checkState(Condition)  
#else
    // Debug checks are always turned off for release code:
    #define debugInput(Condition)
    #define debugLogic(Condition)
    #define debugOutput(Condition)
    #define debugState(Condition)
#endif

// The vital check is special - it might have a message constructed by the caller.
#define checkVital(Condition,Message) (void) ( (Condition) || ( checkFailed( Message ), 0 ) )

// These are the functions called when a check fails. Don't call these directly.
// The fewer arguments provided to checkFailed, the less helpful the error 
// message but the smaller the code. The message or source name each should not
// exceed 200 characters.
extern void UNREAL_API checkFailed( const char * Message, const char * SourceFileName, int LineNumber );
extern void UNREAL_API checkFailed( const char * SourceFileName, int LineNumber );
extern void UNREAL_API checkFailed( const char * Message );
extern void UNREAL_API checkFailed( int LineNumber );
extern void UNREAL_API checkFailed( );

#endif // _INC_CHECKS

