/*=============================================================================
	UnEngine.cpp: Unreal engine main

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"
#include "UnDynBsp.h"
#include "Net.h"
#include "UnConfig.h"

//
// All global variables:
//
UNREAL_API FUnrealEngine			GUnreal;
UNREAL_API FGlobalResourceManager	GRes;
UNREAL_API FUnrealServer			GServer;
UNREAL_API FGlobalDefaults			GDefaults;
UNREAL_API FGlobalPlatform			*GApp;
UNREAL_API FGlobalTopicTable		GTopics;
UNREAL_API FGlobalClasses			GClasses;
UNREAL_API FMemoryCache				GCache;
UNREAL_API FMemPool					GMem,GDynMem;
UNREAL_API FGlobalMath				GMath;
UNREAL_API FGlobalGfx				GGfx;

UNREAL_API FGlobalRender			*GRend				= NULL;
UNREAL_API FCameraManager			*GCameraManager		= NULL;
UNREAL_API FTaskManager				*GTaskManager		= NULL;
UNREAL_API FEditor					*GEditor			= NULL;
UNREAL_API NManager					*GNetManager		= NULL;
UNREAL_API UTransBuffer				*GTrans				= NULL;
UNREAL_API FVirtualGame				*GVirtualGame		= NULL;

UNREAL_API float Cyc2Msec = 1.0f / (166.0f * 1000.0f); // Replace 166.0 with your mHz CPU speed

int UNREAL_API GEngineExec(const char *Cmd,FOutputDevice *Out)
	{return GUnreal.Exec(Cmd,Out);};

/*-----------------------------------------------------------------------------
	Unreal Init
-----------------------------------------------------------------------------*/

int FUnrealEngine::Init
	(
	class FGlobalPlatform	*Platform,
	class FTaskManager		*TaskManager,
	class FCameraManager	*CameraManager, 
	class FGlobalRender		*Rend,
	class FVirtualGame		*Game,
	class NManager			*NetManager,
	class FEditor			*Editor)
	{
	GUARD;
	//
	GDefaults.Init(Platform->CmdLine); // Init defaults based on command-line
	//
	GApp				= Platform;
	GTaskManager		= TaskManager;
	GCameraManager		= CameraManager;
	GEditor				= Editor;
	GNetManager			= NetManager;
	GVirtualGame		= Game;
	GRend				= Rend;
	//
	GMem.AllocatePool			(GDefaults.GlobalsMemSize,"Globals");
	GDynMem.AllocatePool		(GDefaults.DynamicsMemSize,"Dynamics");
	GCache.Init					(1024*1024*(GEditor ? 8 : 3),2000); // Allocate a 3 or 8 meg object cache
	//
	GRes.Init					();         // Start resource manager
	GTaskManager->Init			();			// Start task manager
	GTopics.Init				();			// Start link topic handler
	GMath.Init					(); 		// Init math tables
	GClasses.Init				();			// Init actor classes
	GServer.Init				(); 		// Init server
	GCameraManager->Init		();			// Init camera manager
	if (GEditor) GEditor->Init	();			// Init editor
	if (GNetManager) GNetManager->Init();	// Initialize networking
	GGfx.Init					(); 		// Init graphics subsystem
	GRend->Init					(); 		// Init rendering subsystem
	GAudio.Init					(GDefaults.AudioActive); // Init music and sound
	GVirtualGame->Init			();			// Initialize game-specific info and actor messages
	GClasses.Associate			();			// Associate classes with other resources
	GVirtualGame->CheckState	();			// Verify that game state is valid
    GConfiguration.Initialize();
	//
	if (GEditor)
		{	
		GApp->ServerAlive = 1; 		// Server is now alive
		GEditor->Exec ("SERVER OPEN");
		debug (LOG_Init,"UnrealServer " ENGINE_VERSION " launched for editing!");
		}
	else
		{
		InitGame(); 				// Init game
		GApp->ServerAlive = 1; 		// Server is now alive
		OpenCamera();
		debug (LOG_Init,"UnrealServer " ENGINE_VERSION " launched for gameplay!");
		};
	return 1;
	//
	UNGUARD("FUnrealEngine::Init");
	};

/*-----------------------------------------------------------------------------
	Unreal Exit
-----------------------------------------------------------------------------*/

void FUnrealEngine::Exit(void)
	{
	GUARD;
	//
	GApp->ServerAlive = 0;
    GConfiguration.Finalize();
	//
	if (GEditor)	GEditor->Exit();
	else			ExitGame();
	//
	GVirtualGame->Exit		();		// Shut down game-specific code
	GAudio.Exit				();		// Shut down music and sound
	GRend->Exit				();		// Shut down rendering subsystem
	GGfx.Exit				();		// Shut down graphics subsystem
	if (GNetManager) GNetManager->Exit();	// Initialize networking
	if (GEditor) GEditor->Exit();			// Init editor
	GCameraManager->Exit	();		// Exit camera manager
	GServer.Exit			();		// Shut down the server
	GClasses.Exit			();		// Shut down actor classes
	GMath.Exit				();		// Shut down math routines
	GTopics.Exit			();		// Shut down link topic handler
	GTaskManager->Exit		();		// Shut down trask manager
	GRes.Exit				();		// Shut down resource manager
	GCache.Exit				();		// Shut down the memory cache
	GMem.FreePool			();		// Free memory pool
	GDynMem.FreePool		();		// Free memory pool
	GDefaults.Exit			();		// Shut down global parameters
	//
	debug (LOG_Exit,"Unreal engine shut down");
	//
	UNGUARD("FUnrealEngine::Exit");
	};

/*-----------------------------------------------------------------------------
	Game init/exit functions
-----------------------------------------------------------------------------*/

void FUnrealEngine::InitGame(void)
	{
	GUARD;
	//
	// Init defaults:
	//
	GGfx.DefaultCameraFlags  = SHOW_Backdrop | SHOW_Actors | SHOW_Menu | SHOW_PlayerCtrl | SHOW_RealTime;
	GGfx.DefaultRendMap      = REN_DynLight;
	//
	// Open default game world:
	//
	if (GRes.AddFile(GDefaults.AutoLevel)==FILE_NONE) GApp->Error ("resAddFile failed");
	//
	ULevel *Level = (ULevel *)GServer.Levels->Element(0);
	if (!Level) appError ("Can't find level");
	//
	if (!MakeScripts(GClasses.Root,0)) appError("Script compilation failed");
	//
	PBeginPlay PlayInfo;
	PlayInfo.bNetCooperative	= 0;
	PlayInfo.bNetDeathMatch		= 0;
	PlayInfo.bNetPersistent		= 1;
	PlayInfo.bNetNoMonsters		= 0;
	PlayInfo.Difficulty         = 1;
	//
	Level->ReconcileActors(0);
	//
	GAudio.InitLevel(Level->ActorList->Max);
	//
	Level->SetState(LEVEL_UpPlay,&PlayInfo);
	//
	sporeInit(Level);
	//
	GRes.Purge(1);
	//
	UNGUARD("FUnrealEngine::InitGame");
	};

void FUnrealEngine::ExitGame(void)
	{
	GUARD;
	sporeExit();
	GAudio.ExitLevel();
	UNGUARD("FUnrealEngine::ExitGame");
	};

/*-----------------------------------------------------------------------------
	Camera functions
-----------------------------------------------------------------------------*/

//
// Open a normal camera for gameplay or editing.
//
UCamera *FUnrealEngine::OpenCamera(void)
	{
	GUARD;
	//
	UCamera *Camera = new(NULL,CREATE_Unique)UCamera(GServer.Levels->Element(0));
	Camera->OpenWindow(NULL,0);
	return Camera;
	//
	UNGUARD("FUnrealEngine::OpenCamera");
	};

//
// Draw a camera view.
//
void FUnrealEngine::Draw(UCamera *Camera, int Scan)
	{
	GUARD;
	AActor				*Actor;
	ICamera 			CameraInfo;
	FVector				OriginalLocation;
	FRotation			OriginalRotation;
	PCalcView			ViewInfo;
	DWORD				ShowFlags;
	//
	if (Camera->Level->Model->ModelFlags & MF_InvalidBsp)
		{
		debug (LOG_Ed,"Can't draw game view - invalid Bsp");
		return;
		};
	if (!Camera->Lock(&CameraInfo))
		{
		debug (LOG_Ed,"Couldn't lock camera for drawing");
		return;
		};
	GRend->PreRender(&CameraInfo);
	//
	// Do game-specific prerendering; handles adjusting the view location
	// according to the actor's status, rendering the status bar, etc.
	//
	CameraInfo.Camera->Console->PreRender(&CameraInfo);
	if ((Camera->SXR>0) && (Camera->SYR>0))
		{
		//
		// Handle graphics-mode prerendering; handles special preprocessing
		// and postprocessing special effects and stretching.
		//
		GGfx.PreRender(&CameraInfo);
		if ((Camera->SXR>0) && (Camera->SYR>0))
			{
			//
			// Adjust viewing location based on the player's response to ACTOR_PlayerCalcView:
			//
			Actor					= CameraInfo.Actor;
			ShowFlags				= Actor->CameraStatus.ShowFlags;
			//
			ViewInfo.Coords			= &CameraInfo.Coords;
			ViewInfo.Uncoords		= &CameraInfo.Uncoords;
			ViewInfo.ViewLocation	= Actor->Location;
			ViewInfo.ViewRotation	= Actor->ViewRot;
			CameraInfo.Level.SendMessage(CameraInfo.iActor,ACTOR_PlayerCalcView,&ViewInfo);
			OriginalLocation		= Actor->Location;
			OriginalRotation		= Actor->ViewRot;
			Actor->Location			= ViewInfo.ViewLocation;
			Actor->ViewRot			= ViewInfo.ViewRotation;
			//
			CameraInfo.BuildCoords();
			//
			Actor->Location = OriginalLocation;
			Actor->ViewRot  = OriginalRotation;
			//
			// Draw the level:
			//
			GRend->DrawWorld(&CameraInfo);
			//
			// Draw the player's weapon:
			//
			if ((CameraInfo.Actor->iWeapon !=INDEX_NONE) && (!CameraInfo.Actor->bBehindView) && 
				(CameraInfo.ShowFlags & SHOW_Actors))
				{
				INDEX iWeapon = CameraInfo.Actor->iWeapon;
				AActor *Weapon = &CameraInfo.Level.Actors->Element(iWeapon);
				//
				CameraInfo.Level.SendMessage(iWeapon,ACTOR_InventoryCalcView,NULL);
				//
				FRotation Temp  = Weapon->DrawRot;
				Weapon->DrawRot = Weapon->ViewRot;
				GRend->DrawActor (&CameraInfo,iWeapon);
				Weapon->DrawRot = Temp;
				};
			};
		GGfx.PostRender(&CameraInfo);
		};
	CameraInfo.Camera->Console->PostRender(&CameraInfo,0);
	GRend->PostRender(&CameraInfo);
	//
	Camera->Unlock(&CameraInfo,!Scan);
	//
	UNGUARD("FUnrealEngine::Draw");
	};

/*-----------------------------------------------------------------------------
	World functions
-----------------------------------------------------------------------------*/

void FUnrealEngine::EnterWorld(const char *WorldURL)
	{
	GUARD;
	// Puke if Bsp is invalid
	UNGUARD("FUnrealEngine::EnterWorld");
	};

void FUnrealEngine::GetWorldInfo(char *WorldURL, char *WorldTitle)
	{
	GUARD;
	UNGUARD("FUnrealEngine::GetWorldInfo");
	};

/*-----------------------------------------------------------------------------
	Command line executor
-----------------------------------------------------------------------------*/

//
// This always going to be the last handler in the chain. It
// handles passing the command to all other global handlers.
// There may be console-specific handlers before this in the
// chain, such as a camera console handler or a Telnet console
// handler.
//
int FUnrealEngine::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	// See if any other subsystems claim the command:
	//
	if (GApp && GApp->Exec						(Cmd,Out)) return 1;
	if (GTaskManager && GTaskManager->Exec		(Cmd,Out)) return 1;
	if (GRes.Exec								(Cmd,Out)) return 1;
	if (GCameraManager && GCameraManager->Exec	(Cmd,Out)) return 1;
	if (GServer.Exec							(Cmd,Out)) return 1;
	if (GNetManager && GNetManager->Exec		(Cmd,Out)) return 1;
	if (GEditor && GEditor->Exec				(Cmd,Out)) return 1;
	if (GRend->Exec								(Cmd,Out)) return 1;
	//
	// Handle engine command line:
	//
	if (GetCMD(&Str,"STATUS"))
		{
		if (GetCMD(&Str,"ENGINE") || !Str[0])
			{
			Out->Logf("   ENGINE - Launched for %s",GEditor ? "editing" : "play");
			return 1;
			}
		else return 1;
		}
	else if (GetCMD(&Str,"HELP"))
		{
		Out->Log("   STATUS - Major subsystem status");
		Out->Log("   FLUSH - Flush memory caches");
		Out->Log("   (See the docs for a complete command list)");
		return 1;
		}
	else if (GetCMD(&Str,"FLUSH"))
		{
		GCache.Flush();
		Out->Log("Flushed memory caches");
		return 1;
		}
	else if (GetCMD(&Str,"_HELP"))
		{
		return 1;
		}
	else
		{
		Out->Log(LOG_ExecError,"Unrecognized command");
		return 1;
		};
	UNGUARD("FUnrealEngine::Exec");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
