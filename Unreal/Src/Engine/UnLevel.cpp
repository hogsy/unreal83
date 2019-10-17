/*=============================================================================
	UnLevel.cpp: Level-related functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
        * July 21, 1996: Mark added global GLevel
=============================================================================*/

#include "Unreal.h"
#include "UnChecks.h"
#include "UnDynBsp.h"

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

//
// Level state description text
//
UNREAL_API const char *GLevelStateDescr[LEVEL_MAX]=
	{
	"Down","Up for play","Up for editing","Up for demo"
	};

/*-----------------------------------------------------------------------------
	Level creation & emptying
-----------------------------------------------------------------------------*/

//
//	Create a new level and allocate all resources needed for it.
//	Call with Editor=1 to allocate editor structures for it, also.
//
ULevel::ULevel (int Editable)
	{
	GUARD;
	//
	State=LEVEL_Down;
	//
	// Figure out how much stuff we need:
	//
	INDEX MaxActors		= 1200;
	INDEX MaxTextSize	= 20000;
	INDEX MaxBrushes	= 5000;
	//
	// Allocate all resources (indentation shows hierarchy):
	//
	Model			= new(Name,CREATE_Replace)   UModel         (Editable);
	ActorList		= new(Name,CREATE_Replace)   UActorList     (MaxActors);
	ActorList->RelistActors();
	BrushArray		= new(Name,CREATE_Replace)   TArray<UModel> (MaxBrushes);
	BrushArray->Add  (new("Brush",CREATE_Replace)UModel         (Editable));
	//
	// Stick level in world:
	//
	GServer.Levels->Add(this);
	//
	// Bring level up:
	//
	if (Editable) SetState (LEVEL_UpEdit,NULL);
	//
	UNGUARD("ULevel::ULevel");
	};

//
// Empty the contents of a level
//
void ULevel::Empty (void)
	{
	GUARD;
	IModel		ModelInfo;
	//
	// Clear the brush array:
	// (Must go through and actually delete them!)
	//
	BrushArray->Num = OurMin(1,BrushArray->Num); // Keep only the brush
	//
	// Clear the model:
	//
	Model->Lock(&ModelInfo,LOCK_NoTrans);
	ModelInfo.NumVectors  = 0;
	ModelInfo.NumPoints   = 0;
	ModelInfo.NumBspNodes = 0;
	ModelInfo.NumBspSurfs = 0;
	ModelInfo.NumVertPool = 0;
	ModelInfo.ModelFlags  = 0;
	Model->Unlock (&ModelInfo);
	//
	UNGUARD("ULevel::Empty");
	};

//
// Delete all stuff related to a level
//
void ULevel::KillAll (void)
	{
	GUARD;
	SetState(LEVEL_Down,NULL);
	for (int i=1; i<BrushArray->Num; i++) BrushArray->Element(i)->Kill();
	Kill();
	UNGUARD("ULevel::KillAll");
	};

/*-----------------------------------------------------------------------------
	Level locking and unlocking
-----------------------------------------------------------------------------*/

void ULevel::Lock (ILevel *LevelInfo, int NewLockType)
	{
	GUARD;
	checkVital( GLevel==0, "Level: Multiple locks" );
	GLevel = LevelInfo;
	//
	LevelInfo->State		= State;
	LevelInfo->Level		= this;
	LevelInfo->Model		= Model;
	LevelInfo->PlayerList	= PlayerList;
	LevelInfo->BrushArray	= BrushArray;
	LevelInfo->Brush        = BrushArray->Element(0);
	LevelInfo->Model->Lock	(&LevelInfo->ModelInfo,NewLockType);
	LevelInfo->Actors		= ActorList;
	LevelInfo->Actors->Lock(NewLockType);
	LevelInfo->Dynamics.Locked = 0;
    if( ActorList->ActiveActors != 0 ) // Safety check in case we haven't set up the lists yet.
        {
            ActorList->StaticActors        ->Compress();
            ActorList->DynamicActors       ->Compress();
            ActorList->CollidingActors     ->Compress();
            ActorList->ActiveActors        ->Compress();
            ActorList->UnusedActors        ->Compress();
            ActorList->JustDeletedActors   ->Compress();
        }
	//
	// Go through all actors and clear 'bJustDeleted' so that actors destroyed
	// on the previous frame can be recycled on this frame:
	//
    if( ActorList->JustDeletedActors != 0 )
        {
        while( ActorList->JustDeletedActors->Count() > 0 )
            {
            AActor * Actor = (*ActorList->JustDeletedActors)[0];
            Actor->bJustDeleted = FALSE;
            Actor->Class = 0;
            ActorList->JustDeletedActors->RemoveIndex(0);
            ActorList->UnusedActors->Add(Actor);
            }
        }
	sporeLock(LevelInfo);
	LevelInfo->Dynamics.Lock(&GMem,LevelInfo,OurMax(1000,LevelInfo->ModelInfo.NumBspNodes*5));
	//
	UNGUARD("ULevel::Lock");
	};

void ULevel::Unlock (ILevel *LevelInfo)
	{
	GUARD;
	//
	LevelInfo->Dynamics.Unlock();
	sporeUnlock();
	//
	checkVital( GLevel!=0, "Level: Unlock without lock" );
	GLevel = 0;
	State  = LevelInfo->State;
	LevelInfo->Actors->Unlock();
	LevelInfo->Model->Unlock (&LevelInfo->ModelInfo);
	//
	UNGUARD("ULevel::Unlock");
	};

/*-----------------------------------------------------------------------------
	Level state transitions
-----------------------------------------------------------------------------*/

//
// Return the level's current state (LEVEL_UP_PLAY, etc).
//
ELevelState ULevel::GetState (void)
	{
	GUARD;
	return State;
	UNGUARD("ULevel::GetState");
	};

//
// Set the level's state.  Notifies all actors of the state change.
// If you're setting the state to LEVEL_UP_PLAY, you must specify
// the network mode and difficulty level.
//
void ULevel::SetState (ELevelState NewState,PBeginPlay *PlayInfo)
	{
	GUARD;
	ILevel		Level;
	ELevelState	OldState;
	//
	OldState	= State;
	State		= NewState;
	//
	if (NewState == OldState) return;
	//
	// Send messages to all level actors notifying state we're exiting:
	//
	Lock (&Level,LOCK_NoTrans);
	//
	if (OldState==LEVEL_UpPlay)	Level.BroadcastMessage (ACTOR_EndPlay,NULL);
	if (OldState==LEVEL_UpEdit)	Level.BroadcastMessage (ACTOR_EndEdit,NULL);
	//
	// Send message to actors notifying them of new level state:
	//
	if (NewState == LEVEL_UpPlay)
		{
		if (!PlayInfo) appError ("Missing PlayInfo");
		debugf (LOG_Server,"Level %s is now up for play",Name);
		Level.BroadcastMessage(ACTOR_PreBeginPlay,PlayInfo);
		Level.BroadcastMessage(ACTOR_BeginPlay,PlayInfo);
		//
		for (INDEX i=0; i<Level.Actors->Max; i++)
			{
			AActor *Actor = &Level.Actors->Element(i);
			if (Actor->Class) Level.SetActorZone(i);
			};
		Level.BroadcastMessage(ACTOR_PostBeginPlay,PlayInfo);
		}
	else if (NewState == LEVEL_UpEdit)
		{
		debugf (LOG_Server,"Level %s is now up for play",Name);
		Level.BroadcastMessage(ACTOR_BeginEdit,NULL);
		}
	else if (NewState == LEVEL_Down)
		{
		debugf (LOG_Server,"Level %s is now down",Name);
		}
	else appErrorf ("SetLevelState: Bad state %i",NewState);
	Unlock (&Level);
	UNGUARD("ULevel::SetState");
	};

/*-----------------------------------------------------------------------------
	Level resource implementation
-----------------------------------------------------------------------------*/

void ULevel::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (ULevel);
	Type->RecordSize = 0;
	Type->Version    = 1;
	strcpy (Type->Descr,"Level");
	UNGUARD("ULevel::Register");
	};
void ULevel::InitHeader(void)
	{
	GUARD;
	//
	// Init resource header to defaults:
	//
	State			= LEVEL_Down;
	BrushArray		= NULL;
	Model		    = NULL;
	ActorList		= NULL;
	//
	// Clear all text blocks:
	//
	for (int i=0; i<NUM_LEVEL_TEXT_BLOCKS; i++) TextIDs[i]=NULL;
	UNGUARD("ULevel::InitHeader");
	};
const char *ULevel::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	int ImportedActiveBrush=0,NumBrushes=0;
	char StrLine[256],BrushName[NAME_SIZE];
	const char *StrPtr;
	//
	if (BrushArray->Num==0) appError ("No active brush in level");
	//
	// Assumes data is being imported over top of a new, valid map
	//
	GetNEXT  (&Buffer);
	if       (!GetBEGIN (&Buffer,"MAP")) return NULL;
	GetINT(Buffer,"Brushes=",&NumBrushes);
	//
	while (GetLINE (&Buffer,StrLine,256)==0)
		{
		StrPtr = StrLine;
		if (GetEND(&StrPtr,"MAP"))
			{
			break; // End of brush polys
			}
		else if (GetBEGIN(&StrPtr,"BRUSH"))
			{
			GApp->StatusUpdate("Importing Brushes",BrushArray->Num,NumBrushes);
			//
			if (GetSTRING(StrPtr,"NAME=",BrushName,NAME_SIZE))
				{
				UModel *TempModel;
				if (!ImportedActiveBrush)
					{
					// Parse the active brush, which has already been allocated:
					TempModel = BrushArray->Element(0);
					Buffer    = TempModel->Import(Buffer,BufferEnd,FileType);
					if (!Buffer) return NULL;
					ImportedActiveBrush = 1;
					}
				else
					{
					// Parse a new brush, which has not yet been allocated:
					GRes.MakeUniqueName(BrushName,Name,"_S",RES_Model);
					TempModel = new(BrushName,CREATE_Unique)UModel;
					Buffer = TempModel->Import(Buffer,BufferEnd,FileType);
					if (!Buffer) return NULL;
					BrushArray->Add(TempModel);						
					};
				TempModel->ModelFlags |= MF_Selected;
				};
			}
		else if (GetBEGIN(&StrPtr,"ACTORLIST"))
			{
			Buffer = ActorList->Import(Buffer,BufferEnd,FileType);
			if (!Buffer) return NULL;
			};
		};
	return Buffer;
	UNGUARD("ULevel::Import");
	};
char *ULevel::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	//
	Buffer += sprintf (Buffer,"%s;\r\n",spc(Indent));
	Buffer += sprintf (Buffer,"%s; Unreal World Editor\r\n",spc(Indent));
	Buffer += sprintf (Buffer,"%s; Version: %s\r\n",spc(Indent),ENGINE_VERSION);
	Buffer += sprintf (Buffer,"%s; Exported map \r\n",spc(Indent));
	Buffer += sprintf (Buffer,"%s;\r\n",spc(Indent));
	//
	Buffer += sprintf (Buffer,"%sBegin Map Name=%s Brushes=%i\r\n",spc(Indent),Name,BrushArray->Num);
	Buffer += sprintf (Buffer,"%s   ;\r\n",spc(Indent));
	if (TextIDs[0])	Buffer += sprintf (Buffer,"%s   ;          Title: %s\r\n",spc(Indent),TextIDs[0]->GetData());
	if (TextIDs[1])	Buffer += sprintf (Buffer,"%s   ;        Creator: %s\r\n",spc(Indent),TextIDs[1]->GetData());
	if (TextIDs[2])	Buffer += sprintf (Buffer,"%s   ; Release Status: %s\r\n",spc(Indent),TextIDs[2]->GetData());
	Buffer += sprintf (Buffer,"%s   ;\r\n",spc(Indent));
	//
	// Export brushes:
	//
	for (int i=0; i<BrushArray->Num; i++)
		{
		Buffer = BrushArray->Element(i)->Export(Buffer,FileType,Indent+3);
		if (!Buffer) return NULL;
		};
	//
	// Export actors:
	//
	Buffer  = ActorList->Export(Buffer,FileType,Indent+3);
	if (!Buffer) return NULL;
	//
	Buffer += sprintf (Buffer,"%sEnd Map\r\n",spc(Indent));
	//
	return Buffer;
	UNGUARD("ULevel::Export");
	};
void ULevel::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	Callback.Resource (this,(UResource **)&Model      ,0);
	Callback.Resource (this,(UResource **)&ActorList  ,0);
	Callback.Resource (this,(UResource **)&BrushArray ,0);
	// Maybe PlayerListID
	UNGUARD("ULevel::QueryHeaderReferences");
	};
void ULevel::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	for (INDEX i=0; i<NUM_LEVEL_TEXT_BLOCKS; i++) Callback.Resource (this,(UResource **)&TextIDs[i],0);
	UNGUARD("ULevel::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_Level,ULevel,0xB2D90857,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	Level link topic
-----------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Lev",LevTopicHandler);
void LevTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UTextBuffer	*Text;
	int			ItemNum;
	//
	if (!isdigit (Item[0]))	return; // Item isn't a number
	//
	ItemNum = atoi (Item);
	if ((ItemNum < 0) || (ItemNum >= ULevel::NUM_LEVEL_TEXT_BLOCKS)) return; // Invalid text block number
	//
	Text = Level->TextIDs[ItemNum];
	//
	if (Text) strcpy (Data,Text->GetData()); // Must watch out for overflows
	//
	UNGUARD("LevTopicHandler::Get");
	};
void LevTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UTextBuffer	*Text;
	char		Name[NAME_SIZE];
	int			ItemNum;
	//
	if (!isdigit (Item[0]))	return; // Item isn't a number
	//
	ItemNum = atoi (Item);
	if ((ItemNum < 0) || (ItemNum >= ULevel::NUM_LEVEL_TEXT_BLOCKS)) return; // Invalid text block number
	//
	GRes.CombineName (Name,Level->Name,"T",ItemNum);
	//
	Text = new(Name,CREATE_Replace)UTextBuffer(strlen(Data)+1,1);
	strcpy (Text->GetData(),Data);
	//
	Level->TextIDs[ItemNum] = Text;
	//
	UNGUARD("LevTopicHandler::Set");
	};

/*-----------------------------------------------------------------------------
	Reconcile actors and cameras after loading or creating a new level
-----------------------------------------------------------------------------*/

//
// These functions provide the basic mechanism by which UnrealEd associates
// cameras and actors together, even when new maps are loaded which contain
// an entirely different set of actors which must be mapped onto the existing cameras.
//

//
// Step 1: Remember actors.  This is called prior to loading a level, and it
// places temporary camera actor location/status information into each camera,
// which can be used to reconcile the actors once the new level is loaded and the
// current actor list is replaced with an entirely new one.
//
void ULevel::RememberActors(void)
	{
	GUARD;
	UCamera	*Camera;
	//
	for (int i=0; i<GCameraManager->CameraArray->Num; i++)
		{
		Camera = GCameraManager->CameraArray->Element(i);
		Camera->RememberedActor = Camera->GetActor();
		};
	UNGUARD("ULevel::RememberActors");
	};

//
// Remove all camera references from all actors in this level.
//
void ULevel::DissociateActors(void)
	{
	GUARD;
	for (INDEX i=0; i<ActorList->Max; i++) ActorList->Element(i).Camera = NULL;
	UNGUARD("ULevel::DissociateActors");
	};

//
// Step 2: Reconcile actors.  This is called after loading a level.
// It attempts to match each existing camera to an actor in the newly-loaded
// level.  If no decent match can be found, creates a new actor for the camera.
//
void ULevel::ReconcileActors (int Remembered)
	{
	GUARD;
	ILevel				LevelInfo;
	UCamera				*Camera;
	AActor				*Actor;
	FName				Name;
	//
	Lock (&LevelInfo,LOCK_NoTrans);
	//
	// Dissociate all actor Cameras:
	//
	DissociateActors();
	//
	if (LevelInfo.State==LEVEL_UpEdit)
		{
		//
		// Match cameras and camera-actors with identical names.  These cameras
		// will obtain all of their desired display properties from the actors.
		//
		for (int i=0; i<GCameraManager->CameraArray->Num; i++)
			{
			Camera			= GCameraManager->CameraArray->Element(i);
			Camera->Level   = this;
			Camera->iActor	= INDEX_NONE;
			Name.Add(Camera->Name);
			//
			for (INDEX j=0; j<LevelInfo.Actors->Max; j++)
				{
				Actor = &LevelInfo.Actors->Element(j);
				if ((Actor->Name == Name) && (Actor->Class == GClasses.Camera))
					{
					debugf (LOG_Info,"Matched camera %s",Camera->Name);
					Camera->iActor = j;
					//
					// Removed this so that camera state is remembered when a map is loaded:
					//
					// if (Remembered) Actor->CameraStatus = Camera->RememberedActor.CameraStatus;
					//
					goto NextCam;
					};
				};
			NextCam:;
			};
		//
		// Match up all remaining cameras to actors.  These cameras will get default
		// display properties.
		//
		for (int i=0; i<GCameraManager->CameraArray->Num; i++)
			{
			Camera = GCameraManager->CameraArray->Element(i);
			//
			// Hook camera up to an existing (though unpossessed) camera actor, or create
			// a new camera actor for it.  Sends ACTOR_SPAWN and ACTOR_POSSESS.  Returns
			// actor index or INDEX_NONE if failed.
			//
			if (Camera->iActor==INDEX_NONE)
				{
				Name.Add(Camera->Name);
				Camera->iActor = LevelInfo.SpawnCameraActor(Camera,Name);
				if (Camera->iActor == INDEX_NONE)
					{
					debug(LOG_Ed,"cameraReconcileActors: Spawn failed");
					Camera->Kill(); // Spawn failed
					i--;
					}
				else
					{
					debugf (LOG_Info,"Spawned camera %s",Camera->Name);
					if (Remembered)
						{
						Camera->GetActor()     = Camera->RememberedActor;
						Camera->GetActor().iMe = Camera->iActor;
						};
					};
				};
			};
		}
	else // State==LEVEL_UP_PLAY
		{
		for (int i=0; i<GCameraManager->CameraArray->Num; i++)
			{
			Camera          = GCameraManager->CameraArray->Element(i);
			Camera->iActor  = LevelInfo.SpawnPlayActor(Camera);
			//
			if (Camera->iActor==INDEX_NONE) appError (
				"Can't play this level: No 'PlayerStart' actor was found to "
				"specify the player's starting position.");
			};
		};
	Unlock (&LevelInfo);
	//
	// Associate cameras and actors:
	//
	GCameraManager->UpdateActorUsers();
	//
	// Kill any remaining camera actors:
	//
	Lock (&LevelInfo,LOCK_NoTrans);
	for (INDEX i=0; i<LevelInfo.Actors->Max; i++)
		{
		AActor *Actor = &LevelInfo.Actors->Element(i);
		//
		if ((Actor->Class  == GClasses.Camera) && 
			(Actor->Camera == NULL))
			{
			LevelInfo.DestroyActor(i);
			};
		};
	Unlock (&LevelInfo);
	UNGUARD("ULevel::ReconcileActors");
	};

/*-----------------------------------------------------------------------------
	ULevel command-line
-----------------------------------------------------------------------------*/

int ULevel::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (GetCMD(&Str,"STATUS") && (GetCMD(&Str,"LEVEL") || !Str[0]))
		{
		int n=0;
		for (int i=0; i<ActorList->Max; i++) if (ActorList->Element(i).Class) n++;
		//
		Out->Logf("   LEVEL - %s, %i/%i actors",GLevelStateDescr[State],n,ActorList->Max);
		return Str[0]!=0;
		}
	else if (GetCMD(&Str,"LEVEL"))
		{
		if (GetCMD(&Str,"UP"))
			{
			Out->Log("Not implemented");
			return 1;
			}
		else if (GetCMD(&Str,"DOWN"))
			{
			Out->Log("Not implemented");
			return 1;
			}
		else return 0;
		}
	else if (GetCMD(&Str,"KILLACTORS"))
		{
		ILevel LevelInfo;
		Lock(&LevelInfo,LOCK_NoTrans);
		AActor *Actor = &LevelInfo.Actors->Element(0);
		for (int i=0; i<LevelInfo.Actors->Max; i++)
			{
			if (Actor->Class && (Actor->Class!=GClasses.Light) && !Actor->Camera) LevelInfo.DestroyActor(i);
			Actor++;
			};
		Unlock(&LevelInfo);
		Out->Log("Killed all actors");
		return 1;
		}
	else if (GetCMD(&Str,"KILLMONSTERS"))
		{
		ILevel LevelInfo;
		Lock(&LevelInfo,LOCK_NoTrans);
		AActor *Actor = &LevelInfo.Actors->Element(0);
		for (int i=0; i<LevelInfo.Actors->Max; i++)
			{
			if (Actor->Class && Actor->Class->IsKindOf(GClasses.Pawn) && !Actor->Camera) LevelInfo.DestroyActor(i);
			Actor++;
			};
		Out->Log("Killed all NPC's");
		Unlock(&LevelInfo);
		return 1;
		}
	else if (GetCMD(&Str,"LINKS"))
		{
		Out->Log("Level links:");
		AActor *Actor = &ActorList->Element(0);
		for (int i=0; i<ActorList->Max; i++)
			{
			Actor++;
			};
		return 1;
		}
	else if (GetCMD(&Str,"ACTORS"))
		{
		int TotalCount=0,TotalStatic=0,TotalCollision=0;
		//
		int *ActorCount		= (int *)GMem.GetZeroed(GRes.MaxRes * sizeof(int));
		int *StaticCount	= (int *)GMem.GetZeroed(GRes.MaxRes * sizeof(int));
		int *CollisionCount	= (int *)GMem.GetZeroed(GRes.MaxRes * sizeof(int));
		//
		UClass *Class=NULL;
		GetUClass(Str,"CLASS=",&Class);
		//
		AActor *Actor = &ActorList->Element(0);
		for (int i=0; i<ActorList->Max; i++)
			{
			if (Actor->Class)
				{
				ActorCount[Actor->Class->Index]++;
				TotalCount++;
				//
				if (Actor->bStaticActor)
					{
					StaticCount[Actor->Class->Index]++;
					TotalStatic++;
					};
				if (Actor->bCollideActors)
					{
					CollisionCount[Actor->Class->Index]++;
					TotalCollision++;
					};
				};
			Actor++;
			};
		if (TotalCount>0) Out->Logf("Actors:");
		//
		UClass *TestClass;
		FOR_ALL_TYPED_RES(TestClass,RES_Class,UClass)
			{
			if (ActorCount[TestClass->Index] && ((Class==NULL) || (Class==TestClass)))
				{
				Out->Logf
					(
					" %s...%i (%i, %i)",TestClass->Name,
					ActorCount		[TestClass->Index],
					StaticCount		[TestClass->Index],
					CollisionCount	[TestClass->Index]
					);
				};
			}
		END_FOR_ALL_TYPED_RES;
		//
		Out->Logf("%i Actors (%i static, %i collision)",TotalCount,TotalStatic,TotalCollision);
		GMem.Release(ActorCount);
		//
		return 1;
		}
	else return 0;
	//
	UNGUARD("ULevel::Exec");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
