#ifndef _INC_unrandom
#define _INC_unrandom
/*
==============================================================================
UnRandom.h: Randomization support
Used by: Various

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    This package provides convenience routines for the
    selection of random values.

Revision history:
    * 05/27/96, Created by Mark
==============================================================================
*/

#include "Unreal.h"

struct FRandom
{
    static float Value(); // Returns a random value in 0.0 .. 1.0 (inclusive)
    static int Value(int First, int Last); // Returns a value in First..Last (inclusive)
    static float Value(float First, float Last); // Returns a value in First..Last (inclusive)
    static BOOL Percent(int Percentage); // Returns TRUE Percentage% of the time.
    static BOOL Boolean(); // Return TRUE 50% of the time.
    static void Initialize();
};

// Randomly select from 2 choices:
template<class T> inline T Random( T Choice1, T Choice2 )
{
    const int Selector = FRandom::Value(1,2);
    return 
        Selector == 1   ? Choice1
    :                     Choice2
    ;
}

// Randomly select from 3 choices:
template<class T> inline T Random( T Choice1, T Choice2, T Choice3 )
{
    const int Selector = FRandom::Value(1,3);
    return 
        Selector == 1   ? Choice1
    :   Selector == 2   ? Choice2
    :                     Choice3
    ;
}

// Randomly select from 4 choices:
template<class T> inline T Random( T Choice1, T Choice2, T Choice3, T Choice4 )
{
    const int Selector = FRandom::Value(1,4);
    return 
        Selector == 1   ? Choice1
    :   Selector == 2   ? Choice2
    :   Selector == 3   ? Choice3
    :                     Choice4
    ;
}

#endif
