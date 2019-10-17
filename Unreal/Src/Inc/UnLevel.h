/*=============================================================================
	UnLevel.h: Standard Unreal resource definitions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
        * July 21, 1996: Mark added GLevel
        * Aug  31, 1996: Mark added GRestartLevelAfterTick
        * Aug  31, 1996: Mark added GJumpToLevelAfterTick
=============================================================================*/

#ifndef _INC_UNLEVEL
#define _INC_UNLEVEL

/*-----------------------------------------------------------------------------
	ULevel resource
-----------------------------------------------------------------------------*/

//
// What's happening with a level.
//
enum ELevelState
	{
	LEVEL_Down    		= 0, 		// Level is down and unloaded, no players
	LEVEL_UpPlay 		= 1, 		// Level is up and running
	LEVEL_UpEdit 		= 2, 		// Level is up for one-player editing
	LEVEL_UpDemo		= 3,		// Up for demo
	LEVEL_MAX			= 4,		// Maximum value+1
	};

UNREAL_API extern const char *GLevelStateDescr[LEVEL_MAX];
UNREAL_API extern BOOL GRestartLevelAfterTick; // Set TRUE to cause level to be restarted at next end of "tick" processing.
UNREAL_API extern char GJumpToLevelAfterTick[64]; // Set to a level name to go to at next end of "tick" processing.

//
// The level resource.  Contains the level's actor list, Bsp information, and brush list.
//
class UNREAL_API ULevel : public UResource
	{
	RESOURCE_CLASS(ULevel,BYTE,RES_Level)
	//
	private:
	ELevelState		State;
	PBeginPlay		PlayInfo;
	//
	public:
	enum {NUM_LEVEL_TEXT_BLOCKS=16}; // Number of blocks of descriptive text to allocate with levels
	//
	UModel			*Model;
	UActorList		*ActorList;
	TArray<UPlayer>	*PlayerList;
	TArray<UModel>	*BrushArray;
	UTextBuffer		*TextIDs [NUM_LEVEL_TEXT_BLOCKS]; // Text blocks for editor and misc tools
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom functions:
	//
	ULevel			(int Editable);
	void	Lock 	(class ILevel *LevelInfo, int LockType);
	void	Unlock 	(class ILevel *LevelInfo);
	void	Empty 	(void);
	void	KillAll	(void);
	void	SetState(ELevelState State,class PBeginPlay *PlayInfo);
	void	Tick	(int CamerasOnly, INDEX iActiveActor);
	void	PreReconcileActors(void);
	ELevelState GetState(void);
	void	ReconcileActors (int Remembered);
	void	RememberActors(void);
	void	DissociateActors(void);
	int		Exec(const char *Cmd,FOutputDevice *Out=GApp);
	};

/*-----------------------------------------------------------------------------
	Level dynamics
-----------------------------------------------------------------------------*/

//
// A generic dynamic item that can be added to the Bsp.  Subclassed
// by dynamic actor item and dynamic rendering item structures.
//
class UNREAL_API FBspDynamicItem
	{
	public:
	INDEX	iNext;
	};

//
// An actor reference stored at a Bsp node.  Used for actor-to-actor 
// collision during server level processing.
//
class UNREAL_API FBspDynamicActor : public FBspDynamicItem
	{
	public:
	INDEX iActor;
	};

//
// Generic class for handling dynamic items added to the Bsp.  Subclassed by 
// the server for actor-actor collision, and by the rendering engine for 
// dynamic objects.
//
class UNREAL_API FBspDynamicsTracker
	{
	public:
	//
	// Variables; only valid when locked:
	//
	int				Locked;			// =1 if locked
	int				Overflowed;		// =1 if overflowed during Lock session
	FMemPool		*Mem;			// Memory pool used for adding dynamics references
	IModel			*Model;			// Model being worked on
	ILevel			*Level;			// Level being worked on
	DWORD			*AlteredNodes;	// One bit for each Bsp node indicating that it was altered
	INDEX			NumDynamics;	// Number of dynamics already active
	INDEX			MaxDynamics;	// How many dynamics objects can be active at a time
	FBspDynamicItem	**Dynamics;		// Pointers to all dynamics, up to MaxDynamics
	FBspDynamicItem	*TempItem;		// Used for callbacks
	int				TempItemSize;	// Size of TempItem (may be a child class)
	int				AlterSize;		// Number of DWORD's in AlteredNodes
	void			*MemStart;		// Start of memory at Lock() time
	//
	// Functions:
	//
	void Lock		(FMemPool *MemPool, ILevel *Level,int MaxDynamics);
	void Unlock		(void);
	void Add		(INDEX iNode,FBspDynamicItem *Item, int AddToBack);
	void AddSphere	(FBspDynamicItem *ItemTemplate, int ItemSize, FVector &Location, FLOAT Radius);
	};

//
// Class for tracking all actors stored dynamically in a Bsp.
//
class UNREAL_API FBspActorTracker : public FBspDynamicsTracker
	{
	public:
	enum {MAX_COLLISION_ACTORS=512};
	INDEX TempActors[MAX_COLLISION_ACTORS];
	int	TempNum;
	//
	void AddAllActors(ILevel *Level);
	void AddActorSphere(INDEX iActor);
	int CheckActorTouch(FVector &Location, FLOAT Radius, INDEX *TouchedActors, int MaxTouchedActors);
	};

/*-----------------------------------------------------------------------------
	ILevel
-----------------------------------------------------------------------------*/

//
// Information structure for a locked level.  This contains essentially the
// same information as the ULevel resource, but in a format that's easier and
// faster to manipulate.
//
class UNREAL_API ILevel
	{
	public:
	//
	// Variables:
	//
	ELevelState		State;
	ULevel			*Level;
	UModel			*Model;
	UActorList		*Actors;
	UModel			*Brush;
	TArray<UModel>	*BrushArray;
	TArray<UPlayer>	*PlayerList;
	//
	IModel				ModelInfo;
	FBspActorTracker	Dynamics;
	//
	// Actor-related functions:
	//
	int		SendMessage			(INDEX iActor,FName Message,void *Params,UClass *Class=NULL);
	int		SendMessageDirect	(INDEX iActor,FName Message,void *Params);
	int		SendMessageEx		(FName Message,void *Params,INDEX iActor=INDEX_NONE,FName TagName=NAME_NONE,UClass *Class=NULL);
	int		SendMessageReverse	(INDEX iActor,FName Message,void *Params,UClass *Class=NULL);
	int		BroadcastMessage	(FName Message,void *Params);
	int		MoveActor			(INDEX iActor,FVector *Delta);
	int		FarMoveActor		(INDEX iActor, FVector *DestLocation);
	void	DestroyActor		(INDEX iActor);
	void	UnlinkActor			(INDEX iActor);
	int		PossessActor		(INDEX iActor, UCamera *Camera);
	UCamera *UnpossessActor		(INDEX iActor);
	INDEX	SpawnActor			(UClass *Class, FName ActorName,const FVector *Location,AActor *Template=NULL);
	INDEX	SpawnCameraActor	(UCamera *Camera,FName MatchName);
	INDEX	SpawnPlayActor		(UCamera *Camera);
	//
	void	BeginTouch(INDEX iActor,INDEX iTouchActor);
	void	EndTouch(INDEX iActor,INDEX iTouchActor);
	//
	int		RayHit(INDEX iActorSource, const FVector *Location, const FVector *Direction,
				FLOAT Momentum, FLOAT ConeRadiusFactor, class PHit *HitMessageTemplate,int Notify,
				FVector *HitLocation, FVector *HitNormal, INDEX *iHitNode);
	void	RadialHit(INDEX iActorSource, FVector *Location, FLOAT Radius, FLOAT Momentum,
				class PHit *HitMessageTemplate, int Notify);
	INDEX	GetZoneDescriptor(int iZone);
	INDEX	GetLevelDescriptor();
	int		GetActorZone(INDEX iActor);
	void	SetActorZone(INDEX iActor);
	void	GetSoundProperties(INDEX iActor,FLOAT *ZoneBreadth,FLOAT *ZoneReflectivity);
	FVector GetZoneVelocity(INDEX iActor);
	FVector GetZoneGravityAcceleration(INDEX iActor);
	//
	// Private. Do not call for normal actor AI.
	//
	int		PrivateTestMoveActor(INDEX iActor,FVector *NewLocation,FVector *Delta,FVector *Adjustment,DWORD *BumpMask);
	int		PrivateNearMoveActor(INDEX iActor,FVector *Delta,DWORD *BumpMask);
	};

extern UNREAL_API ILevel * GLevel; // Global level pointer, available during level Tick processing.

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNLEVEL
