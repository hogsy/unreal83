/*=============================================================================
	UnServer.cpp: Player login/logout/information functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "time.h"

#include "Unreal.h"
#include "UnAction.h"
#include "UnCheat.h"

enum {MAX_SERVER_PLAYERS	= 256};
enum {MAX_WORLD_LEVELS		= 64};

/*-----------------------------------------------------------------------------
	FUnrealServer init & exit
-----------------------------------------------------------------------------*/

//
// Start up the server
//
void FUnrealServer::Init (void)
	{
	GUARD;
	//
	Ticks  = 0;
	Rate   = 35; 
	IsPaused = FALSE;
	//
	// Allocate server array:
	//
	ServerArray = new("Server",CREATE_Unique)UArray(64);
	//
	// Allocate all resources the server needs:
	//
	Players = new("PlayerList",CREATE_Unique)TArray<UPlayer>(MAX_SERVER_PLAYERS);
	//
	// Add all newly-allocated resources to the server array for tracking:
	//
	ServerArray->Add(Players);
	GRes.Root->Add(ServerArray);
	//
	// Allocate the world:
	//
	Levels = new("World",CREATE_Unique)TArray<ULevel >(MAX_WORLD_LEVELS);
	GRes.Root->Add(Levels);
	//
	// Allocate first level:
	//
	ULevel *Level = new("TestLev",CREATE_Unique)ULevel(1);
	//
	debug (LOG_Init,"Server initialized");
	//
	// Start task:
	//
	GTaskManager->AddTask(this,NULL,GApp,PRIORITY_Realtime,TASK_NoUserKill);
	//
	UNGUARD("FUnrealServer::Init");
	};

//
// Shut down the server
//
void FUnrealServer::Exit (void)
	{
	GUARD;
	//
	// End task:
	//
	GTaskManager->KillTask(this);
	//
	// Remove all players
	//
	int PlayerCount = Players->Num;
	for (int i=PlayerCount-1; i>=0; i--)
		{
		LogoutPlayer(Players->Element(i));
		};
	Players->Kill();
	//
	// Close and kill all levels
	//
	for (int i=0; i<Levels->Num; i++)
		{
		Levels->Element(i)->Kill();
		};
	//
	// Remove from root array:
	//
	GRes.Root->Delete(ServerArray); ServerArray->Kill();
	GRes.Root->Delete(Levels);
	Levels->Kill();
	//
	debugf (LOG_Exit,"Server shut down, %i player(s) logged off",PlayerCount);
	UNGUARD("FUnrealServer::Init");
	};

/*-----------------------------------------------------------------------------
	FUnrealServer command line
-----------------------------------------------------------------------------*/

int FUnrealServer::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (GetCMD(&Str,"STATUS"))
		{
		if (GetCMD(&Str,"SERVER") || !Str[0])
			{
			Out->Logf("   SERVER - Ok, Ticks=%i",Ticks);
			return Str[0]!=0;
			}
		else return 0;
		}
	else if (GetCMD(&Str,"LEVELS"))
		{
		Out->Log("Levels:");
		for (int i=0; i<Levels->Num; i++)
			{
			ULevel *Level = Levels->Element(i);
			//
			int n=0;
			for (int i=0; i<Players->Num; i++) if (Players->Element(i)->Level==Level) n++;
			//
			Out->Logf("   %s (%i players): %s",Level->Name,n,GLevelStateDescr[Level->GetState()]);
			};
		Out->Logf("%i level(s)",Levels->Num);
		return 1;
		}
	else if (GetCMD(&Str,"PLAYERSONLY"))
		{
		PlayersOnly ^= 1;
		Out->Log(PlayersOnly ? "Updating players only" : "Updating all actors");
		return 1;
		}
	else if (GetCMD(&Str,"PLAYERS"))
		{
		Out->Log("Players:");
		for (int i=0; i<Players->Num; i++)
			{
			UPlayer *Player = Players->Element(i);
			Out->Logf("   %s (%s)",Player->Name,Player->Level ? Player->Level->Name : "none");
			};
		Out->Logf("%i player(s)",Players->Num);
		return 1;
		}
	else return 0; // Not executed
	//
	UNGUARD("FUnrealServer::Exec");
	};

/*-----------------------------------------------------------------------------
	FUnrealServer player login/logout
-----------------------------------------------------------------------------*/

//
// Log a new player in and return his UPlayer resource or NULL if not accepted.
//
UPlayer *FUnrealServer::Login (ULevel *Level,const char *Name, FSocket *Socket)
	{
	GUARD;
	UPlayer *NewPlayer;
	//
	if (new(Name,FIND_Optional)UPlayer) return NULL; // Already logged in
	//
	// Add new player resource:
	//
	NewPlayer = new(Name,CREATE_Unique)UPlayer;
	//
	// Set player's information:
	//
	NewPlayer->Socket	= Socket;
	NewPlayer->Level	= Level;
	NewPlayer->iActor	= INDEX_NONE;
	//
	// Add to player list:
	//
	Players->Add(NewPlayer);
	//
	debugf (LOG_Server,"Player %s logged in",NewPlayer->Name);
	return NewPlayer;
	//
	UNGUARD("FUnrealServer::Login");
	};

//
// Log a player out
//
void FUnrealServer::LogoutPlayer (UPlayer *Player)
	{
	GUARD;
	//
	//	Remove entry from player list:
	//
	Players->Delete(Player);
	//
	// Kill player resource:
	//
	debugf (LOG_Server,"Player %s logged out",Player->Name);
	Player->Kill();
	//
	UNGUARD("FUnrealServer::LogoutPlayer");
	};

//
// Log a player on a particular socket out:
//
void FUnrealServer::LogoutSocket (FSocket *Socket)
	{
	};

/*-----------------------------------------------------------------------------
	Server tasking functions
-----------------------------------------------------------------------------*/

//
// Server task exit function.
//
void FUnrealServer::TaskExit(void)
	{
	GUARD;
	// Does nothing
	UNGUARD("FUnrealServer::TaskExit");
	};

//
// Server task status function.
//
char *FUnrealServer::TaskStatus(char *Name,char *Desc)
	{
	GUARD;
	//
	sprintf(Name,"UnrealServer");
	sprintf(Desc,"Levels=%i, Players=%i",Levels->Num,Players->Num);
	return Name;
	//
	UNGUARD("FUnrealServer::TaskStatus");
	};

/*-----------------------------------------------------------------------------
	Server timer tick function
-----------------------------------------------------------------------------*/

//
// Calls ILevel::Tick for all levels that are up for playing.  Doesn't do anything
// with levels that are being edited, because the editor cameras take care
// of all updating (when editing a level with player controls, the editor
// cameras handle kTick calls).
//
void FUnrealServer::TaskTick(void)
{
	GUARD;

	UCamera	*Camera,*ActiveCamera;
	ULevel	*Level,*ActiveLevel;
	INDEX	iActiveActor;

	LevelTickTime			= 0;
	GServer.ActorTickTime	= 0;
	AudioTickTime			= 0;
	ActorCollisionFrags		= 0;

    // Slow-motion (useful for debugging)
    static int SlowMotionTicks = 0;
    BOOL IsPaused = this->IsPaused;
    if(!IsPaused && GCheat->SlowMotion)
    {
        SlowMotionTicks++;
        if (SlowMotionTicks < 10) IsPaused = TRUE;
        else SlowMotionTicks = 0;
    }
    else SlowMotionTicks = 0;

	// Update virtual (server-based) ticks:
	if (!IsPaused) Ticks++;

	// Refresh memory cache:
	if(!IsPaused) GCache.Tick();

	// Find active (input) camera:
	ActiveCamera 	= NULL;
	ActiveLevel 	= NULL;
	iActiveActor	= INDEX_NONE;
	for (int i=0; i<GCameraManager->CameraArray->Num; i++)
	{
		Camera = GCameraManager->CameraArray->Element(i);
		if (Camera->Current)
		{
			ActiveCamera	= Camera;
			ActiveLevel		= Camera->Level;
			iActiveActor	= Camera->iActor;
			break;
		}
	}

	// Set time info:
	time_t Time;	time(&Time);
	tm LocalTime;	LocalTime = *localtime(&Time);

	MSec	= 0;
	Second	= LocalTime.tm_sec;
	Minute	= LocalTime.tm_min;
	Hour	= LocalTime.tm_hour;
	Day		= LocalTime.tm_mday;
	Month	= LocalTime.tm_mon+1;
	Year	= LocalTime.tm_year;

	// Update all in-play levels:
	for (int i=0; i<Levels->Num; i++)
	{
		Level = Levels->Element(i);

		int IsPlay = Level->GetState()==LEVEL_UpPlay;
		int IsEdit = Level->GetState()==LEVEL_UpEdit;

		if (IsPlay || (IsEdit && ActiveCamera && ActiveCamera->IsRealtime()))
		{
            if( Level==ActiveLevel && IsPlay && iActiveActor != INDEX_NONE && ActiveCamera != 0 )
            {
                GAction.UpdateStatus(GInput,ActiveCamera->Console->IsTyping());
            }
            if( !IsPaused )
            {
			    Level->Tick(IsEdit || PlayersOnly,(Level==ActiveLevel)?iActiveActor:INDEX_NONE);
            }
		}
	}
	UNGUARD("FUnrealServer::Tick");
}

/*-----------------------------------------------------------------------------
	UPlayer resource implementation
-----------------------------------------------------------------------------*/

void UPlayer::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UPlayer);
	Type->RecordSize = 0;
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Player");
	UNGUARD("UPlayer::Register");
	};
void UPlayer::InitHeader(void)
	{
	GUARD;
	Socket	= NULL;
	Level   = NULL;
	iActor	= INDEX_NONE;
	UNGUARD("UPlayer::InitHeader");
	};
AUTOREGISTER_RESOURCE(RES_Player,UPlayer,0xB2D90874,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
