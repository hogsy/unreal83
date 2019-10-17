/*=============================================================================
	UnActors.cpp: Main actor DLL file

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#define  _COMPILING_UNGAME	/* Read UnActDll.h for explanation */
#include "UnGame.h"
#include "UnCon.h"
#include "UnRandom.h"
#include "UnFActor.h"

//
// The global game object:
//
UNGAME_API FGame GGame;
UNGAME_API FVirtualGame *GGamePtr=&GGame;

/*-----------------------------------------------------------------------------
	Game init and exit
-----------------------------------------------------------------------------*/

//
// Initialized all global game info.  Called once per run.
//
void FGame::Init(void)
	{
	GUARD;
	//
	/////////////////////////////
	// Required initialization //
	/////////////////////////////
	//
	// Register all built-in messages as global names
	//
	ActorMessages = new("ActorMessages",CREATE_Unique)UEnum(1024);
	GRes.Root->Add(ActorMessages);
	//
	FName Name;
	#define  REGISTER_MESSAGE(num,msg) \
		Name.AddHardcoded(num,#msg); \
		ActorMessages->Add(ACTOR_##msg);
	#include "UnMsgs.h"
	#undef   REGISTER_MSG
	//
    FRandom::Initialize(); //tbm: Is there a better place for this?
	debug(LOG_Init,GAME_DLL " initialized");
	//
	//////////////////////////////////
	// Game-specific initialization //
	//////////////////////////////////
	//
	// (None is needed here)
	//
	UNGUARD("FGame::Init");
	};

//
// Check state of game info.
// Called between Init() and Exit().
//
void FGame::CheckState(void)
	{
	//
	// Make sure built-in and generated structures match up properly:
	//
    if(sizeof(AActorRoot)!=sizeof(ARoot))
		{
		appError("Size mismatch: AActorRoot and ARoot");
		};
    if( sizeof(AActor) != sizeof(FActor) )
        {
        appError( "Size mismatch: AActor and FActor" );
        }
	};

//
// Shut down and deallocate all game-specific stuff
//
void FGame::Exit(void)
	{
	GUARD;
	//
	// Delete all actor messages:
	//
	GRes.Root->Delete(ActorMessages);
	ActorMessages->Kill();
	//
	debug(LOG_Exit,GAME_DLL " shut down");
	//
	UNGUARD("FGame::Exit");
	};

/*-----------------------------------------------------------------------------
	Creating and destroying camera consoles
-----------------------------------------------------------------------------*/

//
// Create a new game-specific console for the specific camera.  The console
// includes a status bar, text console, and whatever other stuff is desired.
//
class FVirtualCameraConsole *FGame::CreateCameraConsole(UCamera *Camera)
	{
	GUARD;
	//
	FCameraConsole *Result = new FCameraConsole;
	Result->Old = new FCameraConsole;
	return Result;
	//
	UNGUARD("FGame::CreateCameraConsole");
	};

//
// Destroy a camera console.
//
void FGame::DestroyCameraConsole(class FVirtualCameraConsole *Console)
	{
	GUARD;
	//
	delete ((FCameraConsole *)Console)->Old;
	delete Console;
	//
	UNGUARD("FGame::DestroyCameraConsole");
	};

/*-----------------------------------------------------------------------------
	Game command line
-----------------------------------------------------------------------------*/

int FGame::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	return 0; // Not executed
	UNGUARD("FGame::Exec");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
