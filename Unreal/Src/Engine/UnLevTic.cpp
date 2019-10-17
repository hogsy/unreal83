/*=============================================================================
	UnLevTic.cpp: Level timer tick function

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
        * July 21, 1996: Mark added GLevel
        * Aug  31, 1996: Mark added GRestartLevelAfterTick
        * Aug  31, 1996: Mark added GJumpToLevelAfterTick
=============================================================================*/

#include "Unreal.h"
#include "UnDynBsp.h"
#include "UnAction.h"

ILevel * GLevel = 0; // Global level pointer, available when level is locked.
BOOL GRestartLevelAfterTick = FALSE; // Set TRUE to cause level to be restarted at next end of "tick" processing.
char GJumpToLevelAfterTick[64] = { 0 } ; // Set to a level name to go to at next end of "tick" processing.

/*-----------------------------------------------------------------------------
	Main level timer tick handler
-----------------------------------------------------------------------------*/

//
// Update the level after timer ticks have passed.  TicksPassed is usually 1,
// and will only be higher if the server is too slow and starts getting behind
// the 35 fps base frame rate.
//
// All child actors are ticked after their parents have been ticked.
//
void ULevel::Tick(int CamerasOnly, INDEX iActiveLocalPlayer)
	{
	GUARD;
	ILevel		Level;
	PPlayerTick	MovementPacket,NoMovementPacket;
	BYTE		*Ticked = (BYTE *)GMem.GetZeroed(ActorList->Max * sizeof(BYTE));
	int			NumUpdated,NumSkipped,NumIter=0;
	//
	ALWAYS_BEGINTIME(GServer.LevelTickTime);
	//
	NoMovementPacket.BuildAllMovement(NULL); // Null movement packet
	//
	// Lock everything we need:
	//
	Lock(&Level,LOCK_NoTrans);
	//
	if (!CamerasOnly) Level.Dynamics.AddAllActors(&Level);
	//
	// Go through actor list, updating actors who either have no parent, or whose
	// parent has been updated.  The result is that parent actors are always updated
	// before their children.
	//
	ALWAYS_BEGINTIME(GServer.ActorTickTime);
	do	{
		NumUpdated = 0;
		NumSkipped = 0;
		//
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
		BYTE *TickPtr = &Ticked[0];
		AActor *Actor = &Level.Actors->Element(0);
		//
		for (INDEX i=0; i<Level.Actors->Max; i++)
#endif
    	for( int Which = 0; Which < Level.Actors->DynamicActors->Count(); Which++)
			{
            AActor * const Actor = (*Level.Actors->DynamicActors)[Which];
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
			{
			if (Actor->Class && !*TickPtr)
#endif
			if( Actor != 0 && !Ticked[Actor->iMe] )
				{
                const INDEX i = Actor->iMe;
				if (CamerasOnly)
					{
					if (!Actor->Camera) goto Skip;
					if (!(Actor->CameraStatus.ShowFlags & SHOW_PlayerCtrl)) goto Skip;
					};
				if (((Actor->iParent==INDEX_NONE) || (Ticked[Actor->iParent])) && !Actor->bStaticActor)
					{
					if (!Actor->Camera)
						{
						Level.SendMessage(i,ACTOR_Tick,NULL);
						}
					else
						{
						if (i==iActiveLocalPlayer) // Camera: Add keystrokes/mouse from local input
							{
                            if( GEditor != 0 )
                                {
                                    // Update status - done here only when running the editor
                                    // since the editor does not call the server tick function.
                                    GAction.UpdateStatus(GInput,Actor->Camera->Console->IsTyping());
                                }
							GAudio.SetOrigin(&Actor->Location, &Actor->ViewRot);
							MovementPacket.BuildAllMovement(Actor->Camera);
							Level.SendMessage(i,ACTOR_PlayerTick,&MovementPacket);
							}
						else Level.SendMessage(i,ACTOR_PlayerTick,&NoMovementPacket);
						}
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
					*TickPtr=1;
#endif
                    Ticked[i] = TRUE;
					NumUpdated++;
					}
				else
					{
					Skip:
					NumSkipped++;
					};
				};
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
			Actor++;
			TickPtr++;
#endif
			};
		NumIter++;
		} while (NumUpdated && NumSkipped);
	ALWAYS_ENDTIME(GServer.ActorTickTime);
	//
	// Update the audio:
	//
	ALWAYS_BEGINTIME(GServer.AudioTickTime);
	GAudio.Tick(&Level);
	ALWAYS_ENDTIME(GServer.AudioTickTime);
	//
	// Unlock everything:
	//
	Unlock(&Level);
	GMem.Release(Ticked);
	//
	ALWAYS_ENDTIME(GServer.LevelTickTime);
	//
	UNGUARD("ULevel::Tick");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
