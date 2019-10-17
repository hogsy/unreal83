/*
==============================================================================
UnPyro.cpp: Pyrotechnics actor code

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    This defines various functions related to pyrotechnic actors.

Revision history:
    * 05/24/96: Created by Mark
==============================================================================
*/

#include "UnGame.h"
#include "UnFActor.h"

//---------------------------------------------------------------------------
//              APyrotechnics::Process
//---------------------------------------------------------------------------
int APyrotechnics::Process(ILevel *Level, FName Message, void *Params)
{
   GUARD;
   FActor & Actor = FActor::Actor(*this);
   switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                      Initialization
        //--------------------------------------------------------------------
        case ACTOR_Spawn:
        {
            TextureList = int(&Textures[0]); //tbi: Conversion            
            AnimBase = AnimFirst;
            Actor.MakeSound( InitialSound );
            Actor.MakeNoise( Noise );
            break;
        }
        //--------------------------------------------------------------------
        //                      End of animation
        //--------------------------------------------------------------------
        case ACTOR_EndAnimation:
        {
            Actor.Send_Die();
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                      Time passes...
        //--------------------------------------------------------------------
        case ACTOR_Tick:
        {
            Velocity *= AccelerationFactor;
            Level->MoveActor(iMe,&Velocity);
            if (bGravity) Velocity += Level->GetZoneGravityAcceleration(iMe) * GravityMult;
            break;
        }
    }
    return ProcessParent;
    UNGUARD( "APyrotechnics::Process" );
}

//---------------------------------------------------------------------------
//              ABigManGunFlash::Process
//---------------------------------------------------------------------------
int ABigManGunFlash::Process(ILevel *Level, FName Message, void *Params)
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}

//---------------------------------------------------------------------------
//              ASkaarjGunFlash::Process
//---------------------------------------------------------------------------
int ASkaarjGunFlash::Process(ILevel *Level, FName Message, void *Params)
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}

//---------------------------------------------------------------------------
//              AGasBagBelchFlash::Process
//---------------------------------------------------------------------------
int AGasBagBelchFlash::Process(ILevel *Level, FName Message, void *Params)
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}

//---------------------------------------------------------------------------
//              AWallHit::Process
//---------------------------------------------------------------------------
int AWallHit::Process(ILevel *Level, FName Message, void *Params) 
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}

//---------------------------------------------------------------------------
//              APawnHit::Process
//---------------------------------------------------------------------------
int APawnHit::Process(ILevel *Level, FName Message, void *Params)
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}

//---------------------------------------------------------------------------
//              AExplode1::Process
//---------------------------------------------------------------------------
int AExplode1::Process(ILevel *Level, FName Message, void *Params)
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}

//---------------------------------------------------------------------------
//              AExplode2::Process
//---------------------------------------------------------------------------
int AExplode2::Process(ILevel *Level, FName Message, void *Params)
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}

//---------------------------------------------------------------------------
//              AExplode3::Process
//---------------------------------------------------------------------------
int AExplode3::Process(ILevel *Level, FName Message, void *Params)
{
    //tbi? For efficiency, remove ::Process function.
    return ProcessParent;
}
