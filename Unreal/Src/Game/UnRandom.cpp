/*
==============================================================================
UnRandom.cpp: Random values

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 05/27/96: Created by Mark
==============================================================================
*/

#include "UnRandom.h"
#include <stdlib.h>
#include <sys\types.h>
#include <sys\timeb.h>

//----------------------------------------------------------------------------
//               Initialize the random number generator.
//----------------------------------------------------------------------------
void FRandom::Initialize()
{
    // Seed the random-number generator with a value that will give us
    // different random number sequences.
    struct _timeb TimeInfo;
    _ftime( &TimeInfo );
    const unsigned Seed = 1000*(unsigned)TimeInfo.time + (unsigned)TimeInfo.millitm;
    srand( Seed );
}

//----------------------------------------------------------------------------
//              Returns a random value in 0.0 .. 1.0
//----------------------------------------------------------------------------
float FRandom::Value()
{
    return float(rand()) / float(RAND_MAX) ;
}

//----------------------------------------------------------------------------
//          Returns a value in First..Last (inclusive)
//----------------------------------------------------------------------------
int FRandom::Value(int First, int Last)
{
    return int( First + float(Last-First+.999) * Value() );
}

//----------------------------------------------------------------------------
//         Returns a value in First..Last (inclusive)
//----------------------------------------------------------------------------
float FRandom::Value(float First, float Last)
{
    return First + float(Last-First) * Value();
}

//----------------------------------------------------------------------------
//           Returns TRUE Percentage% of the time.
//----------------------------------------------------------------------------
BOOL FRandom::Percent(int Percentage)
{
    return Value(1,100) <= Percentage;
}

//----------------------------------------------------------------------------
//              Return TRUE 50% of the time.
//----------------------------------------------------------------------------
BOOL FRandom::Boolean()
{
    return rand()&1;
}

