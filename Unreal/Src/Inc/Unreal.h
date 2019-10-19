/*=============================================================================
	Unreal.h: Main header for the Unreal engine

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNREAL
#define _INC_UNREAL

/*----------------------------------------------------------------------------
	Low level includes
----------------------------------------------------------------------------*/


//
// Version info and compile options specific to this build of the Unreal engine
//
#include "UnBuild.h"

//
// Unreal Platform-specific defines for portability
//
#ifndef  _INC_UNPORT
#include "UnPort.h" 
#endif

//
// All platform-independent hooks visible to Unreal which
// call platform-specific routines elsewhere
//
#ifndef __PLATFORM__
#include "UnPlatfm.h"
#endif

/*-----------------------------------------------------------------------------
	Global variables
-----------------------------------------------------------------------------*/

//
// Every major, Unreal subsystem has a global class associated with it,
// and a global variable named GSomthing.
//
UNREAL_API extern class FUnrealEngine			GUnreal;
UNREAL_API extern class FGlobalResourceManager	GRes;
UNREAL_API extern class FUnrealServer			GServer;
UNREAL_API extern class FGlobalDefaults			GDefaults;
UNREAL_API extern class FGlobalPlatform			*GApp;
UNREAL_API extern class FGlobalTopicTable		GTopics;
UNREAL_API extern class FGlobalClasses			GClasses;
UNREAL_API extern class FMemoryCache			GCache;
UNREAL_API extern class FMemPool				GMem,GDynMem;
UNREAL_API extern class FGlobalGfx				GGfx;
UNREAL_API extern class FGlobalMath				GMath;
UNREAL_API extern class FGlobalAudio			GAudio;
UNREAL_API extern class FGlobalRender			*GRend;
UNREAL_API extern class FCameraManager			*GCameraManager;
UNREAL_API extern class UTransBuffer			*GTrans;
UNREAL_API extern class FEditor					*GEditor;
UNREAL_API extern class FVirtualGame			*GVirtualGame;
UNREAL_API extern class NManager				*GNetManager;

/*-----------------------------------------------------------------------------
	High-level includes
-----------------------------------------------------------------------------*/

// Low-level, platform-independent file functions
#include "UnFile.h"

// Vector math functions
#include "UnMath.h"

// Parameter parsing routines
#include "UnParams.h"

// In-memory object caching
#include "UnCache.h"

// Fast memory pool allocation
#include "UnMem.h"

// Resource type definitions
#include "UnResTyp.h"

// Resource manager
#include "UnRes.h"

// Topic handlers for editor/server communication
#include "UnTopics.h"

// Actor subsystem
#include "UnActor.h"

// Level resource
#include "UnLevel.h"

// Camera subsystem
#include "UnCamera.h"

// Audio subsystem FGlobalAudio
#include "UnFGAud.h"

// Audio subsystem main
#include "UnSound.h"

// Graphics subsystem
#include "UnGfx.h"

// Unreal server
#include "UnServer.h"

// Unreal defaults
#include "UnDeflts.h"

// Unreal engine
#include "UnEngine.h"

// Virtual game class
#include "UnVGame.h"

//
// Files required by the Unreal editor
//
#ifdef EDITOR

	#ifndef _INC_UNEDITOR	// Unreal editor
	#include "UnEditor.h"
	#endif

	#ifndef _INC_UNEDTRAN	// Transaction tracking system
	#include "UnEdTran.h"
	#endif

#endif
/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNREAL
