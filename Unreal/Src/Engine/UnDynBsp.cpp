/*=============================================================================
	UnDynBsp.cpp: Unreal dynamic Bsp object support

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnDynBsp.h"

/*
#define PARANOID // Perform slow validity checks
#undef SLOW_GUARD
#undef SLOW_UNGUARD
#define SLOW_GUARD GUARD
#define SLOW_UNGUARD UNGUARD
*/

#define PRECOMPUTE_FILTER /* Precompute sphere filter for optimization */

/*---------------------------------------------------------------------------------------
	This code handles all moving brushes with a level.  These are objects which move
	once in a while and are added to and maintained within the level's Bsp whenever they 
	move.

	Two forms of information are maintained about moving brushes during gameplay:

	1.	Permanent information.  Information that is allocated once for each moving
		brush: Bsp surfaces and vectors.  These records are updated as
		the brush moves, but they are not deleted/reallocated as the brush moves.  This info
		need only be allocated once because each brush requires a constant amount of this
		stuff, and the amount is known in advance.

	2.	Sporadic information.  Information that is deleted/reallocated for a brush each 
		time the brush moves: Bsp nodes, poly points, vertex pools.  This needs to be 
		reallocated dynamically because a brush requires a changing amount of it based on the 
		amount a moving brush is cut by the Bsp in its current position.

	Safely handles overflow conditions.

	Implementation:

		Uses unlinked databases for all permanent and sporadic elements; any moving brush
		operation requires that each element of each database be traversed to see what needs
		to be updated.  This is fast because the databases and element sizes (WORDs) are small.
		New element allocation is performed via a roving feeler.

	Major optimizations that could be performed:

+		Reduce FlushActorBrush thrash via iParent
		Brush bounding boxes and bounding spheres for filtering precompute.
		Store each brush as a brep with its own point and vector tables.  Then:
		- Preallocate space for all of the brep's points and vectors, so that only
		  points on clipped poly lines need to be sporadically maintained.
		- Point and vector usage will be minimized.
		- Duplicate points and vectors won't be transformed during rendering.
		- VertPools can be added with side linking.
		- Filtering work can be minimized by filtering the entire brep through the
		  bsp in one pass, instead of a per-poly pass.
		Store each brush as a Bsp and use tree merging.

	This uses the following members of other classes for purposes other than what
	they are named/intended:

	* UModel::Color - First node index in the linked list of sporadic brush nodes.
	* FBspNode::iUniquePlane - Next node in the linked list of sporadic brush nodes.
	* FPoly::iLink - Bsp surface index corresponding to the FPoly
	* FPoly::iBrushPoly - Light mesh index that applies to the FPoly, or INDEX_NONE if none
	* FBspSurf::iActor - Index of actor whose moving brush owns the surface

---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
	Globals
---------------------------------------------------------------------------------------*/

//
// Tracks moving brushes within a level.  One of these structures is kept for each 
// active level.  The structure is not saved with levels, but is rather rebuild at 
// load-time.
//
// Though multiple FMovingBrushTrackers may exist simultaneously, only one may be
// locked at any time.
//
class FMovingBrushTracker
	{
	public:
	//
	// Constructor/Init/Exit:
	//
	FMovingBrushTracker() {Initialized=0; Locked=0;};
	//
	void Init(ULevel *ThisLevel);
	void Exit(void);
	//
	// Lock:
	//
	void Lock(ILevel *ThisLevel);
	void Unlock(void);
	//
	// Operations that can be done while locked:
	//
	void SetupAllBrushes(void);
	void RemoveAllBrushes(void);
	void UpdateBrushes(INDEX *iActors,int Num);
	//
	// Helper functions:
	//
	static int Locked;
	int Initialized;
	void AssertInitialized(void)	{if (!Initialized)appError("Not initialized");};
	void AssertLocked(void)			{if (!Initialized)appError("Not initialized"); if (!Locked)appError("Not locked");};
	//
	// Private functions:
	//
	inline int  NewPointIndex(INDEX iActor);
	inline int  NewVectorIndex(INDEX iActor);
	inline int  NewNodeIndex(INDEX iActor,INDEX iParent);
	inline int  NewSurfIndex(INDEX iActor);
	inline int  NewVertPoolIndex(INDEX iActor,int NumVerts);
	inline int	NewBrushMapIndex(INDEX iActor);
	//
	inline void FreePointIndex(INDEX i);
	inline void FreeVectorIndex(INDEX i);
	inline void FreeNodeIndex(INDEX i);
	inline void FreeSurfIndex(INDEX i);
	inline void FreeVertPoolIndex(DWORD i,int NumVerts);
	inline void FreeBrushMapIndex(INDEX i);
	//
	void SetupActorBrush(INDEX iActor);
	void RemoveActorBrush(INDEX iActor);
	//
	void AddActorBrush(INDEX iActor);
	void FlushActorBrush(INDEX iActor,int Group);
	//
	inline void ForceGroupFlush(INDEX iNode);
	void ForceTouchFlush(INDEX iDynamic);
	//
	void FilterFPoly(INDEX iNode, INDEX iCoplanarParent, FPoly *EdPoly, int Outside);
	void AddPolyFragment(INDEX iNode, INDEX iCoplanarParent, int IsFront, FPoly *EdPoly);
	//
	// Private variables:
	//
	//private:
	//
	enum {MAX_MOVING_BRUSH_POLYS =4096}; // Maximum moving brush polys per level
	enum {MAX_MOVING_BRUSH_ACTORS=512};  // Maximum moving brush actors per level
	enum {MAX_TOUCHING_ACTORS=512};		 // Maximum actors touched by a moving brush during update
	//
	ULevel *LevelRes;
	static ILevel *Level;
	static AActor *Actors;
	static IModel ModelInfo;
	static FVector FPolyNormal;
	//
	int iTopNode,iTopSurf,iTopPoint,iTopVector,iTopVertPool,iTopBrushMap;
	INDEX iAddActor,iAddSurf;
	FBspSurf *AddSurf;
	//
	// Tracking which actors' brushes are valid:
	UModel **ActorBrushes;
	//
	// Mechanism to pair actor indices with Bsp surfaces:
	INDEX *iBrushMapOwners;
	INDEX *iBrushMapSurfs;
	//
	// Owner actor indices for things that are deleted and realllocated whenever a brush moves:
	INDEX *iNodeOwners;
	INDEX *iNodeParents;
	INDEX *iPointOwners;
	DWORD *iVertPoolOwners;
	// 
	// Owner actor indices for things that are only updated (not deleted/reallocated) whenever a brush moves:
	INDEX *iSurfOwners;
	INDEX *iVectorOwners;
	//
	// List of actors to update in a multi-brush update:
	static INDEX iGroupActors[MAX_MOVING_BRUSH_ACTORS];
	static int NumGroupActors;
	//
	static INDEX iTouchActors[MAX_MOVING_BRUSH_ACTORS];
	static int NumTouchActors;
	//
	// Used by AddPolyFragment
	static INDEX *iActorNodePrevLink;
	};
int			FMovingBrushTracker::Locked;
ILevel*		FMovingBrushTracker::Level;
AActor*		FMovingBrushTracker::Actors;
IModel		FMovingBrushTracker::ModelInfo;
FVector		FMovingBrushTracker::FPolyNormal;
INDEX		FMovingBrushTracker::iGroupActors[MAX_MOVING_BRUSH_ACTORS];
int			FMovingBrushTracker::NumGroupActors;
INDEX		FMovingBrushTracker::iTouchActors[MAX_MOVING_BRUSH_ACTORS];
int			FMovingBrushTracker::NumTouchActors;
INDEX*		FMovingBrushTracker::iActorNodePrevLink;

//
// Temporary early development hacks:
//
FMovingBrushTracker GLST;
UNREAL_API void sporeInit(ULevel *Level)	{GLST.Init(Level);};
UNREAL_API void sporeExit(void)				{GLST.Exit();};
UNREAL_API void sporeLock(ILevel *Level)	{if (GLST.Initialized) GLST.Lock(Level);};
UNREAL_API void sporeUnlock(void)			{if (GLST.Initialized) GLST.Unlock();};
UNREAL_API void sporeFlush(INDEX iActor)	{if (GLST.Initialized) GLST.FlushActorBrush(iActor,0);};
UNREAL_API int sporeSurfIsDynamic(INDEX iSurf)
	{
	if (iSurf<GLST.ModelInfo.NumBspSurfs) return 0;
	return GLST.iSurfOwners[iSurf - GLST.ModelInfo.NumBspSurfs] != INDEX_NONE;
	};
UNREAL_API void sporeUpdate(INDEX iActor)
	{
	if (GLST.Initialized)
		{
		AActor *Actor = &GLST.Actors[iActor];
		UModel *Brush = Actor->Brush;
		//
		if (Brush && (Brush->Location!=Actor->Location) || (Brush->Rotation != Actor->DrawRot))
			{
			// Must handle collision response
			Brush->Location		= Actor->Location;
			Brush->Rotation		= Actor->DrawRot;
			GLST.UpdateBrushes(&iActor,1);
			};
		};
	};

/*---------------------------------------------------------------------------------------
	FMovingBrushTracker init & exit
---------------------------------------------------------------------------------------*/

//
// Allocate an array of INDEX's for the elements in a database resource from
// Num to Max.  These elements of level resources are reserved for use by moving 
// brush pieces.
//
INDEX *AllocDbIndex(UDatabase *Res,char *Descr)
	{
	GUARD;
	//
	INDEX *Result = (INDEX *)appMallocArray(Res->Max - Res->Num,INDEX,Descr);
	for (int i=Res->Num; i<Res->Max; i++)
		{
		Result[i - Res->Num] = INDEX_NONE;
		};
	return Result;
	//
	UNGUARD("AllocDbIndex");
	};

//
// Set a vector in the Bsp points or vectors table to a specified
// vector, initializing its transformation index, flags, and alignment info.
//
inline void SetVector(FVector &Dest,FVector &Value)
	{
	Dest            = Value;
	Dest.iTransform = INDEX_NONE;
	Dest.Flags      = 0;
	Dest.Align      = 0;
	};

//
// Shorten a database resource's maximum element count to prevent
// moving brush data from trashing it as a sparse array.  Returns
// the number of active elements in the resource.
//
int inline ShortenDb(UDatabase *Res)
	{
	if (!GEditor) Res->Max = OurMin(Res->Max,Res->Num + 2048 + (Res->Num>>1));
	return Res->Num;
	};

//
// Initialize or reinitialize everything, and allocate all working tables.  Must be 
// followed by a call to UpdateAllBrushes to actually add moving brushes to the world 
// Bsp.  This function assumes that the Bsp is clean when it is called, i.e. it has no 
// references to dynamic Bsp nodes in it.
//
void FMovingBrushTracker::Init(ULevel *ThisLevel)
	{
	GUARD;
	//
	if (Initialized) appError("Already initialized");
	//
	LevelRes = ThisLevel;
	//
	iTopNode			= ShortenDb(LevelRes->Model->BspNodes);
	iTopSurf			= ShortenDb(LevelRes->Model->BspSurfs);
	iTopPoint			= ShortenDb(LevelRes->Model->Points  );
	iTopVector			= ShortenDb(LevelRes->Model->Vectors );
	iTopVertPool		= ShortenDb(LevelRes->Model->VertPool);
	iTopBrushMap		= 0;
	//
	// Allocate memory:
	//
	ActorBrushes		= (UModel **)appMallocArray(LevelRes->ActorList->Max,UModel **,"ActorBrushes");
	int i;
	for (i=0; i<LevelRes->ActorList->Max; i++) ActorBrushes[i]=NULL;
	//
	iBrushMapOwners		= (INDEX *)appMallocArray(MAX_MOVING_BRUSH_POLYS,INDEX,"iBrushMapOwners");
	for (i=0; i<MAX_MOVING_BRUSH_POLYS; i++) iBrushMapOwners[i]=INDEX_NONE;
	//
	iBrushMapSurfs		= (INDEX *)appMallocArray(MAX_MOVING_BRUSH_POLYS,INDEX,"BrushMapSurfs");
	//
	iVertPoolOwners		= (DWORD *)appMallocArray(LevelRes->Model->VertPool->Max - LevelRes->Model->VertPool->Num,DWORD,"iVertPoolOwners");
	for (i=0; i<(LevelRes->Model->VertPool->Max - LevelRes->Model->VertPool->Num); i++) iVertPoolOwners[i]=MAXDWORD;
	//
	iNodeOwners			= AllocDbIndex(LevelRes->Model->BspNodes,"iNodeOwners");
	iNodeParents		= AllocDbIndex(LevelRes->Model->BspNodes,"iNodeParents");
	iSurfOwners			= AllocDbIndex(LevelRes->Model->BspSurfs,"iSurfOwners");
	iPointOwners		= AllocDbIndex(LevelRes->Model->Points,  "iPointOwners");
	iVectorOwners		= AllocDbIndex(LevelRes->Model->Vectors, "iVectorOwners");
	//
	// Successfully initialized structure:
	//
	Locked				= 0;
	Initialized			= 1;
	//
	debugf(LOG_Bsp,"Initialized moving brush tracker for %s",LevelRes->Name);
	//
	// Now setup and update all brushes:
	//
	ILevel LevelInfo;
	LevelRes->Lock(&LevelInfo,LOCK_NoTrans);
	UpdateBrushes(NULL,0);
	LevelRes->Unlock(&LevelInfo);
	//
	// Make sure that multiple actors don't share the same brush.
	//
	#ifdef PARANOID
	AActor *Actor1 = &Level->Actors->Element(0);
	for (i=0; i<Level->Actors->Max; i++)
		{
		if (Actor1->IsMovingBrush())
			{
			AActor *Actor2 = &Level->Actors->Element(0);
			for (int j=0; j<Level->Actors->Max; j++)
				{
				if (Actor2->IsMovingBrush() && (i!=j))
					{
					if (Actor1->Brush==Actor2->Brush) appErrorf("Shared brush %s",Actor1->Brush->Name);
					};
				Actor2++;
				};
			};
		Actor1++;
		};
	#endif
	//
	UNGUARD("FMovingBrushTracker::Init");
	};

//
// Clear out the world and free all moving brush data.
//
void FMovingBrushTracker::Exit(void)
	{
	GUARD;
	AssertInitialized();
	if (Locked) appError("Locked");
	//
	// Remove all moving brushes:
	//
	ILevel LevelInfo;
	LevelRes->Lock(&LevelInfo,LOCK_NoTrans);
	RemoveAllBrushes();
	LevelRes->Unlock(&LevelInfo);
	//
	Initialized=0;
	//
	// Free memory:
	//
	appFree(ActorBrushes);
	appFree(iBrushMapOwners);
	appFree(iBrushMapSurfs);
	appFree(iNodeOwners);
	appFree(iNodeParents);
	appFree(iSurfOwners);
	appFree(iPointOwners);
	appFree(iVectorOwners);
	appFree(iVertPoolOwners);
	//
	debugf(LOG_Bsp,"Shut down moving brush tracker for %s",LevelRes->Name);
	//
	UNGUARD("FMovingBrushTracker::Exit");
	};

/*---------------------------------------------------------------------------------------
	Index functions
---------------------------------------------------------------------------------------*/

//
// Get a new index for a particular item.  Returns <0 if point table fills up.
// Unoptimized. Will be optimized with the use of a roving feeler.
//
inline int NewThingIndex(int &TopThing, const int &NumThings,const int &MaxThings,
	INDEX *iThingOwners,INDEX iActor)
	{
	#ifdef PARANOID
	if ((TopThing<NumThings) || (TopThing>=MaxThings)) 
		{
		appErrorf ("TopThing inconsistency %i<%i>%i",NumThings,TopThing,MaxThings);
		};
	#endif
	//
	int   StartThing   = TopThing;
	INDEX *iThingOwner = &iThingOwners[TopThing-NumThings];
	while (TopThing < MaxThings)
		{
		if (*iThingOwner==INDEX_NONE) {*iThingOwner = iActor; return TopThing;};
		TopThing++;
		iThingOwner++;
		};
	TopThing    = NumThings;
	iThingOwner = &iThingOwners[0];
	while (TopThing<StartThing)
		{
		if (*iThingOwner==INDEX_NONE) {*iThingOwner = iActor; return TopThing;};
		TopThing++;
		iThingOwner++;
		};
	#ifdef PARANOID
		appError("NewThingIndex overflow");
	#endif
	//
	return -1;
	};

//
// Routines to allocate new elements of particular types, for moving brush usage.
// These all call NewThingIndex to do their work.
//
inline int FMovingBrushTracker::NewNodeIndex(INDEX iActor,INDEX iParent)
	{
	SLOW_GUARD;
	//
	INDEX Result = NewThingIndex(iTopNode,ModelInfo.NumBspNodes,ModelInfo.MaxBspNodes,iNodeOwners,iActor);
	//
	#ifdef PARANOID
	if (iNodeParents[Result - ModelInfo.NumBspNodes]!=INDEX_NONE) appError("Parent duplicate");
	#endif
	//
	if (Result!=INDEX_NONE) iNodeParents[Result - ModelInfo.NumBspNodes] = iParent;
	return Result;
	//
	SLOW_UNGUARD("FMovingBrushTracker::NewNodeIndex");
	};
inline int FMovingBrushTracker::NewSurfIndex(INDEX iActor)
	{
	SLOW_GUARD;
	return NewThingIndex(iTopSurf,ModelInfo.NumBspSurfs,ModelInfo.MaxBspSurfs,iSurfOwners,iActor);
	SLOW_UNGUARD("FMovingBrushTracker::NewSurfIndex");
	};
inline int FMovingBrushTracker::NewPointIndex(INDEX iActor)
	{
	SLOW_GUARD;
	return NewThingIndex(iTopPoint,ModelInfo.NumPoints,ModelInfo.MaxPoints,iPointOwners,iActor);
	SLOW_UNGUARD("FMovingBrushTracker::NewPointIndex");
	};
inline int FMovingBrushTracker::NewVectorIndex(INDEX iActor)
	{
	SLOW_GUARD;
	return NewThingIndex(iTopVector,ModelInfo.NumVectors,ModelInfo.MaxVectors,iVectorOwners,iActor);
	SLOW_UNGUARD("FMovingBrushTracker::NewVectorIndex");
	};
inline int FMovingBrushTracker::NewBrushMapIndex(INDEX iActor)
	{
	SLOW_GUARD;
	return NewThingIndex(iTopBrushMap,0,MAX_MOVING_BRUSH_POLYS,iBrushMapOwners,iActor);
	SLOW_UNGUARD("FMovingBrushTracker::NewBrushMapIndex");
	};
inline int FMovingBrushTracker::NewVertPoolIndex(INDEX iActor,int NumVerts)
	{
	SLOW_GUARD;
	//
	#ifdef PARANOID
	if ((iTopVertPool<ModelInfo.NumVertPool) || (iTopVertPool>=ModelInfo.MaxVertPool))
		{
		appError("TopThing inconsistency");
		};
	#endif
	//
	int		iStart  = iTopVertPool;
	int		NumFree = 0;
	DWORD	*iOwner	= &iVertPoolOwners[iTopVertPool - ModelInfo.NumVertPool];
	//
	while (iTopVertPool < (ModelInfo.MaxVertPool - NumVerts))
		{
		if (*iOwner==MAXDWORD)
			{
			if (++NumFree >= NumVerts)
				{
				while (NumFree-- > 0) *iOwner-- = iActor;
				return iTopVertPool+1-NumVerts;
				}
			}
		else NumFree=0;
		//
		iTopVertPool++;
		iOwner++;
		};
	iTopVertPool	= ModelInfo.NumVertPool;
	NumFree			= 0;
	iOwner			= &iVertPoolOwners[0];
	//
	while (iTopVertPool < (iStart - NumVerts))
		{
		if (*iOwner==MAXDWORD)
			{
			if (++NumFree >= NumVerts)
				{
				while (NumFree-- > 0) *iOwner-- = iActor;
				return iTopVertPool+1-NumVerts;
				}
			}
		else NumFree=0;
		//
		iTopVertPool++;
		iOwner++;
		};
	#ifdef PARANOID
		appError("NewVertPoolIndex overflow");
	#endif
	return -1;
	//
	SLOW_UNGUARD("FMovingBrushTracker::NewVertPoolIndex");
	};

//
// Functions to free things
//
inline void FMovingBrushTracker::FreePointIndex(INDEX i)
	{
	#ifdef PARANOID
		if (iPointOwners[i-ModelInfo.NumPoints]==INDEX_NONE) appError("FreePointIndex inconsistency");
	#endif
	iPointOwners[i-ModelInfo.NumPoints] = INDEX_NONE;
	};
inline void FMovingBrushTracker::FreeVectorIndex(INDEX i)
	{
	#ifdef PARANOID
		if (iVectorOwners[i-ModelInfo.NumVectors]==INDEX_NONE) appError("FreeVectorIndex inconsistency");
	#endif
	iVectorOwners[i-ModelInfo.NumVectors] = INDEX_NONE;
	};
inline void FMovingBrushTracker::FreeNodeIndex(INDEX i)
	{
	#ifdef PARANOID
		if (iNodeOwners [i-ModelInfo.NumBspNodes]==INDEX_NONE) appError("FreeNodeIndex node inconsistency");
		if (iNodeParents[i-ModelInfo.NumBspNodes]==INDEX_NONE) appError("FreeNodeIndex parent inconsistency");
	#endif
	iNodeOwners [i-ModelInfo.NumBspNodes] = INDEX_NONE;
	iNodeParents[i-ModelInfo.NumBspNodes] = INDEX_NONE;
	};
inline void FMovingBrushTracker::FreeSurfIndex(INDEX i)
	{
	#ifdef PARANOID
		if (iSurfOwners[i-ModelInfo.NumBspSurfs]==INDEX_NONE) appError("FreeSurfIndex inconsistency");
	#endif
	iSurfOwners[i-ModelInfo.NumBspSurfs] = INDEX_NONE;
	};
inline void FMovingBrushTracker::FreeVertPoolIndex(DWORD i,int NumVerts)
	{
	DWORD *iVertPoolOwner = &iVertPoolOwners[i - ModelInfo.NumVertPool];
	for (int j=0; j<NumVerts; j++)
		{
		#ifdef PARANOID
			if (*iVertPoolOwner==MAXDWORD) appError("FreeVertPoolIndex inconsistency");
		#endif
		*iVertPoolOwner++ = MAXDWORD;
		};
	};
inline void FMovingBrushTracker::FreeBrushMapIndex(INDEX i)
	{
	#ifdef PARANOID
		if (iBrushMapOwners[i]==INDEX_NONE) appError("FreeBrushMapIndex inconsistency");
	#endif
	iBrushMapOwners[i]=INDEX_NONE;
	};

/*---------------------------------------------------------------------------------------
	Lock & Unlock
---------------------------------------------------------------------------------------*/

//
// Lock the moving brush tracker for a particular level.
// Assumes that the tracker is initialized.
//
void FMovingBrushTracker::Lock(ILevel *ThisLevel)
	{
	GUARD;
	if (Locked) appError("Already locked");
	//
	Level		= ThisLevel;
	ModelInfo	= ThisLevel->ModelInfo;
	Actors		= &ThisLevel->Actors->Element(0);
	//
	Locked      = 1;
	//
	UNGUARD("FMovingBrushTracker::Lock");
	};

//
// Unlock the moving brush tracker for a particular level.
//
void FMovingBrushTracker::Unlock(void)
	{
	GUARD;
	AssertLocked();
	//
	Locked = 0;
	//
	UNGUARD("FMovingBrushTracker::Unlock");
	};

/*---------------------------------------------------------------------------------------
	Private, permanent per brush operations
---------------------------------------------------------------------------------------*/

//
// Setup permanenent information (surfaces, vectors, base points) for a moving
// brush.
//
void FMovingBrushTracker::SetupActorBrush(INDEX iActor)
	{
	GUARD;
	AssertLocked();
	if (ActorBrushes [iActor]) appError("Brush already setup");
	//
	AActor *Actor  = &Actors[iActor];
	UModel *Brush  = Actor->Brush;
	if (Brush==NULL) appError("Null brush");
	//
	ActorBrushes [iActor] = Brush;
	*(INDEX *)&Brush->Color=INDEX_NONE;
	//
	// Create permanent maps for all moving brush FPolys:
	//
	FPoly    *Poly = &Brush->Polys->Element(0);
	FBspSurf *Surf;
	INDEX    iSurf;
	//
	for (int i=0; i<Brush->Polys->Num; i++)
		{
		//
		// Create new surface elements:
		//
		int iBrushMap = NewBrushMapIndex(iActor);
		if (iBrushMap==-1) goto Over1; // Safe overflow
		//
		iSurf = NewSurfIndex(iActor);
		if (iSurf==INDEX_NONE) goto Over2;
		//
		iBrushMapSurfs[iBrushMap] = iSurf;
		Surf = &ModelInfo.BspSurfs[iSurf];
		//
		Surf->vNormal = NewVectorIndex(iActor);
		if (Surf->vNormal==INDEX_NONE) goto Over3;
		//
		Surf->vTextureU = NewVectorIndex(iActor);
		if (Surf->vTextureU==INDEX_NONE) goto Over4;
		//
		Surf->vTextureV = NewVectorIndex(iActor);
		if (Surf->vTextureV==INDEX_NONE) goto Over5;
		//
		Surf->pBase = NewPointIndex(iActor);
		if (Surf->pBase==INDEX_NONE) goto Over6;
		//
		Surf->iLightMesh = Poly->iBrushPoly; // May be INDEX_NONE
		//
		// Set all other surface properties:
		//
		Surf->Texture  		= Poly->Texture;
		Surf->iActor		= INDEX_NONE;
		Surf->PanU 		 	= Poly->PanU;
		Surf->PanV 		 	= Poly->PanV;
		Surf->PolyFlags 	= Poly->PolyFlags & ~PF_NoAddToBSP;
		Surf->Brush	 		= Brush;
		Surf->iBrushPoly	= i;
		Surf->LastStartY	= 0;
		Surf->LastEndY		= 0;
		Surf->iActor		= iActor;
		//
		// Link FPoly to this Bsp surface:
		//
		Poly->iLink         = iSurf;
		//
		// Go to next FPoly:
		//
		Poly++;
		continue;
		//
		// Overflow cleanup:
		//
		Over6:	FreeVectorIndex(Surf->vTextureV);
		Over5:	FreeVectorIndex(Surf->vTextureU);
		Over4:	FreeVectorIndex(Surf->vNormal);
		Over3:	FreeSurfIndex(iSurf);
		Over2:	FreeBrushMapIndex(iBrushMap);
		Over1:	;
		#ifdef PARANOID
			appErrorf("Overflowed",Actors[iActor].Class->Name);
		#endif
		break;
		};
	#ifdef PARANOID
		debugf(LOG_Bsp,"SetupActorBrush for %s",Actors[iActor].Class->Name);
	#endif
	//
	UNGUARD("FMovingBrushTracker::SetupActorBrush");
	};

//
// Remove all permanent information (surfaces, vectors, base points) for a moving brush from 
// the level.
//
void FMovingBrushTracker::RemoveActorBrush(INDEX iActor)
	{
	GUARD;
	AssertLocked();
	if (!ActorBrushes[iActor]) appError("No brush found for actor");
	//
	// Find all surfaces owned by this actor, and free them and their contents:
	//
	INDEX *iBrushMapOwner = &iBrushMapOwners[0];
	for (int i=0; i<MAX_MOVING_BRUSH_POLYS; i++)
		{
		if (*iBrushMapOwner==iActor)
			{
			//
			// Free all stuff owned by this surface:
			//
			INDEX    iSurf = iBrushMapSurfs[i];
			FBspSurf *Surf = &ModelInfo.BspSurfs[iSurf];
			//
			#ifdef PARANOID
				if (Surf->vNormal  ==INDEX_NONE) appError("Bad vNormal");
				if (Surf->vTextureU==INDEX_NONE) appError("Bad vTextureU");
				if (Surf->vTextureV==INDEX_NONE) appError("Bad vTextureV");
				if (Surf->pBase    ==INDEX_NONE) appError("Bad pBase");
			#endif
			//
			FreeSurfIndex		(iSurf);
			FreePointIndex		(Surf->pBase);
			FreeVectorIndex		(Surf->vTextureV);
			FreeVectorIndex		(Surf->vTextureU);
			FreeVectorIndex		(Surf->vNormal);
			FreeBrushMapIndex	(i);
			};
		iBrushMapOwner++;
		};
	ActorBrushes [iActor]=NULL;
	//
	UNGUARD("FMovingBrushTracker::RemoveActorBrush");
	};

/*---------------------------------------------------------------------------------------
	Polygon filtering
---------------------------------------------------------------------------------------*/

void FMovingBrushTracker::AddPolyFragment(INDEX iParent, INDEX iCoplanarParent,
	int IsFront, FPoly *EdPoly)
	{
	GUARD;
	FBspNode	*Node,*Parent;
	FVertPool	*VertPool;
	INDEX		iNode;
	int			i,iVertPool;
	//
	// If this node is meant to be added as a coplanar, handle it now:
	//
	if (iCoplanarParent!=INDEX_NONE)
		{
		iParent = iCoplanarParent;
		IsFront = 2;
		};
	//
	// Find parent:
	//
	Parent = &ModelInfo.BspNodes[iParent];
	if (IsFront==2) // Coplanar
		{
		while (Parent->iPlane!=INDEX_NONE)
			{
			iParent = Parent->iPlane;
			Parent = &ModelInfo.BspNodes[iParent];
			};
		};
	//
	// Create a new sporadic node:
	//
	iNode = NewNodeIndex(iAddActor,iParent);
	if (iNode==INDEX_NONE) goto Over1;
	Node = &ModelInfo.BspNodes[iNode];
	//
	// Set node's info:
	//
	Node->iSurf       	= iAddSurf;
	Node->iDynamic[0]	= INDEX_NONE;
	Node->iDynamic[1] 	= INDEX_NONE;
	Node->NodeFlags   	= NF_IsNew | NF_Sporadic;
	Node->iBound		= INDEX_NONE;
	Node->iZone			= IsFront ? Parent->iZone : Parent->iBackZone;
	Node->iBackZone		= Node->iZone;
	Node->ZoneMask		= Parent->ZoneMask;
	Node->NumVertices	= EdPoly->NumVertices;
	Node->iUniquePlane	= INDEX_NONE;
	//
	Node->iFront		= INDEX_NONE;
	Node->iBack			= INDEX_NONE;
	Node->iPlane		= INDEX_NONE;
	//
	// Allocate this node's vertex pool and vertices:
	//
	iVertPool = NewVertPoolIndex(iAddActor,EdPoly->NumVertices);
	if (iVertPool==MAXDWORD) goto Over2;
	//
	Node->iVertPool = iVertPool;
	VertPool        = &ModelInfo.VertPool[iVertPool];
	//
	for (i=0; i<EdPoly->NumVertices; i++)
		{
		INDEX pVertex		= NewPointIndex(iAddActor);
		if (pVertex==INDEX_NONE) goto Over3;
		//
		VertPool->iSide		= INDEX_NONE;
		VertPool->pVertex	= pVertex;
		SetVector(ModelInfo.FPoints[pVertex],EdPoly->Vertex[i]);
		//
		VertPool++;
		};
	//
	// Success; link this node to its parent and return.
	// (Can't fail past this point)
	//
	if (Node->iZone) Parent->ZoneMask |= (QWORD)1 << Node->iZone;
	//
	#ifdef PARANOID
	if		((IsFront==2)&&(Parent->iPlane!=INDEX_NONE)) appError("iPlane exists");
	else if ((IsFront==1)&&(Parent->iFront!=INDEX_NONE)) appError("iFront exists");
	else if ((IsFront==0)&&(Parent->iBack !=INDEX_NONE)) appError("iBack exists");
	#endif
	//
	if		(IsFront==2) Parent->iPlane = iNode;
	else if (IsFront==1) Parent->iFront = iNode;
	else				 Parent->iBack  = iNode;
	//
	*iActorNodePrevLink = iNode;
	iActorNodePrevLink  = &Node->iUniquePlane;
	//
	return;
	//
	// Overflow handlers:
	//
	Over3:	while(--i >= 0) FreePointIndex((--VertPool)->pVertex);
	Over2:	FreeNodeIndex(iNode);
	Over1:	;
	#ifdef PARANOID
		appError("Overflowed");
	#endif
	//
	UNGUARD("FMovingBrushTracker::AddPolyFragment");
	};

void FMovingBrushTracker::FilterFPoly(INDEX iNode, INDEX iCoplanarParent,
	FPoly *EdPoly, int Outside)
	{
	FPoly		*TempFrontEdPoly, *TempBackEdPoly;
	FBspNode	*Node;
	FBspSurf	*Surf;
	int			SplitResult;
	//
	TempFrontEdPoly	= (FPoly *)GMem.GetFast(sizeof(FPoly));
	TempBackEdPoly	= (FPoly *)GMem.GetFast(sizeof(FPoly));
	//
	FilterLoop:
	Node  = &ModelInfo.BspNodes[iNode];
	Surf  = &ModelInfo.BspSurfs[Node->iSurf];
	//
	if (EdPoly->NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD) // Must split to avoid vertex overflow
		{
		TempFrontEdPoly = (FPoly *)GMem.Get(sizeof(FPoly));
		EdPoly->SplitInHalf(TempFrontEdPoly);
		FilterFPoly(iNode,iCoplanarParent,TempFrontEdPoly,Outside);
		};
	#ifndef PARANOID
	#ifdef  PRECOMPUTE_FILTER
	if (Surf->PolyFlags & (PF_IsFront | PF_IsBack)) // Filter status was precomputed based on bounding sphere
		{
		if (Surf->PolyFlags & PF_IsFront) goto Front;
		else goto Back;
		};
	#endif
	#endif
	//
	SplitResult = EdPoly->SplitWithPlaneFast
		(
		ModelInfo.FPoints  [Surf->pBase],
		ModelInfo.FVectors [Surf->vNormal],
		TempFrontEdPoly,
		TempBackEdPoly
		);
	if (SplitResult==SP_Front)
		{
		#ifdef PARANOID
			if (Surf->PolyFlags & PF_IsBack) appError("Precompute error 1");
		#endif
		//
		Front:
		Outside = Outside || Node->IsCsg();
		if (Node->iFront != INDEX_NONE)
			{
			iNode = Node->iFront;
			goto FilterLoop;
			}
		else if (Outside) AddPolyFragment(iNode,iCoplanarParent,1,EdPoly);
		}
	else if (SplitResult==SP_Back)
		{
		#ifdef PARANOID
			if (Surf->PolyFlags & PF_IsFront) appError("Precompute error 2");
		#endif
		//
		Back:
		Outside = Outside && !Node->IsCsg();
		if (Node->iBack != INDEX_NONE)
			{
			iNode = Node->iBack;
			goto FilterLoop;
			}
		else if (Outside) AddPolyFragment(iNode,iCoplanarParent,0,EdPoly);
		}
	else if (SplitResult==SP_Coplanar)
		{
		#ifdef PARANOID
			if (Surf->PolyFlags & (PF_IsFront | PF_IsBack)) appError("Precompute error 3");
		#endif
		//
		if ((ModelInfo.FVectors[Surf->vNormal] | FPolyNormal) >= 0.0) iCoplanarParent = iNode;
		goto Front;
		}
	else if (SplitResult==SP_Split)
		{
		#ifdef PARANOID
			if (Surf->PolyFlags & (PF_IsFront | PF_IsBack)) appError("Precompute error 4");
		#endif
		//
		// Handle front fragment:
		//
		if (Node->iFront != INDEX_NONE) FilterFPoly(Node->iFront,iCoplanarParent,TempFrontEdPoly,Outside || Node->IsCsg());
		else if (Outside || Node->IsCsg()) AddPolyFragment(iNode,iCoplanarParent,1,TempFrontEdPoly);
		//
		// Handle back fragment:
		//
		EdPoly			= TempBackEdPoly;
		TempBackEdPoly	= (FPoly *)GMem.GetFast(sizeof(FPoly));
		goto Back;
		};
	};

/*---------------------------------------------------------------------------------------
	Private, sporadic per brush operations
---------------------------------------------------------------------------------------*/

//
// Add all sporadic information (nodes, poly points) for a moving brush to the
// level.  Assumes that no Bsp nodes coming from this actor's brush already exist in the 
// level (those Bsp nodes, if any, must have already been cleaned out by FlushActorBrush).
//
void FMovingBrushTracker::AddActorBrush(INDEX iActor)
	{
	GUARD;
	AssertLocked();
	if (!ActorBrushes[iActor]) appError("No brush found for actor");
	//
	iActorNodePrevLink = (INDEX *)&ActorBrushes[iActor]->Color;
	//
	iAddActor = iActor;
	//
	AActor *Actor = &Actors[iActor];
	UModel *Brush = Actor->Brush;
	if (!Brush) appError("Null brush");
	//
	#ifdef PARANOID
		{
		//
		// Verify that no Bsp nodes coming from this actor's brush already exist in the level.
		//
		for (int i=ModelInfo.NumBspNodes; i<ModelInfo.MaxBspNodes; i++)
			{
			if (iNodeOwners[i-ModelInfo.NumBspNodes]==iActor) appError("Brush nodes already exist");
			};
		//
		// Verify that that each FPoly in the brush is referenced once and only once by iBrushMapOwners.
		//
		for (i=0; i<Brush->Polys->Num; i++)
			{
			int Found=0;
			for (int j=0; j<MAX_MOVING_BRUSH_POLYS; j++)
				{
				if (iBrushMapOwners[j]==iActor)
					{
					FBspSurf *Surf = &ModelInfo.BspSurfs[iBrushMapSurfs[j]];
					if (Surf->iBrushPoly == i) Found++;
					};
				};
			if (Found!=1) appErrorf("Surf/FPoly dissociation %i",Found);
			};
		};
	#endif
	//
	// Build coordinate system to transform Brush's original polygons into the world according
	// to the Brush's current rotation:
	//
	FModelCoords Coords;
	FLOAT    Orientation = Brush->BuildCoords(&Coords,NULL);
	FVector  Location    = Brush->Location + Brush->PostPivot;
	//
	int     NumPolys          = Brush->Polys->Num;
	FPoly	*TransformedPolys = (FPoly *)GMem.Get(NumPolys * sizeof(FPoly));
	FPoly	*SrcPoly          = &Brush->Polys->Element(0);
	FPoly	*DestPoly         = &TransformedPolys[0];
	FVector *FirstPt          = (FVector *)GMem.Get(0);
	FVector *Pt               = FirstPt;
	int     NumPts            = 0;
	//
	for (int i=0; i<NumPolys; i++)
		{
		*DestPoly = *SrcPoly++;
		DestPoly->Transform(Coords,&Brush->PrePivot,&Location,Orientation);
		//
		#ifdef PRECOMPUTE_FILTER
			for (int j=0; j<DestPoly->NumVertices; j++) *Pt++ = DestPoly->Vertex[j];
			NumPts += DestPoly->NumVertices;
		#endif
		//
		DestPoly++;
		};
	#ifdef PRECOMPUTE_FILTER
	FBoundingVolume Bound;
	Bound.Init(FirstPt,NumPts);
	ModelInfo.PrecomputeSphereFilter(&Bound.Sphere);
	#endif
	//
	// Go through list of all brush FPolys and update their corresponding Bsp surfaces.
	//
	FPoly *Poly = &TransformedPolys[0];
	for (int i=0; i<NumPolys; i++)
		{
		INDEX    iSurf  = Poly->iLink;
		FBspSurf *Surf  = &ModelInfo.BspSurfs[iSurf];
		//
		#ifdef PARANOID
			if (iSurfOwners[iSurf-ModelInfo.NumBspSurfs]!=iActor) appError("iSurfOwner mismatch");
		#endif
		//
		FPolyNormal = Poly->Normal;
		SetVector(ModelInfo.FPoints [Surf->pBase    ],Poly->Base);
		SetVector(ModelInfo.FVectors[Surf->vNormal  ],Poly->Normal);
		SetVector(ModelInfo.FVectors[Surf->vTextureU],Poly->TextureU);
		SetVector(ModelInfo.FVectors[Surf->vTextureV],Poly->TextureV);
		//
		// Filter the brush's FPoly through the Bsp, creating new sporadic Bsp nodes (and their 
		// corresponding VertPools and points) for all outside leaves the FPoly fragments fall into:
		//
		GUARD;
			{
			//
			// Scrub precompute info from surface:
			//
			Surf->PolyFlags &= ~(PF_IsFront | PF_IsBack);
			//
			void *MemTop=GMem.GetFast(0);
			//
			iAddSurf = iSurf;
			if (ModelInfo.NumBspNodes>0) FilterFPoly(0,INDEX_NONE,Poly,1);
			//
			GMem.Release(MemTop);
			};
		UNGUARD("Filtering");
		//
		Poly++;
		};
	*iActorNodePrevLink = INDEX_NONE;
	//
	// Tag all newly-added nodes as non-new.
	//
	INDEX iNode = *(INDEX *)&Brush->Color;
	while (iNode!=INDEX_NONE)
		{
		FBspNode *Node   = &ModelInfo.BspNodes[iNode];
		//
		#ifdef PARANOID
			if (iNodeOwners[iNode-ModelInfo.NumBspNodes]!=iActor) appError("Add node inconsistency");
		#endif
		//
		Node->NodeFlags &= ~NF_IsNew;
		iNode            = Node->iUniquePlane;
		};
	UNGUARD("FMovingBrushTracker::AddActorBrush");
	};

//
// Force the brush that owns a specified Bsp node to be flushed and later
// updated as part of a group flushing operation.
//
void FMovingBrushTracker::ForceGroupFlush(INDEX iNode)
	{
	GUARD;
	//
	INDEX iOwnerActor = iNodeOwners[iNode - ModelInfo.NumBspNodes];
	if (iOwnerActor==INDEX_NONE) return; // Already flushed it
	//
	for (int i=0; i<NumGroupActors; i++)
		{
		if (iGroupActors[i]==iOwnerActor) return; // Already in flush list
		};
	if (NumGroupActors < MAX_MOVING_BRUSH_ACTORS)
		{
		iGroupActors[NumGroupActors++] = iOwnerActor;
		};
	UNGUARD("FMovingBrushTracker::ForceGroupFlush");
	};

//
// Force all actor references in a dynamics chain to be added to the list
// iTouchActors so that they can be added back in after the operation is 
// complete.  If this isn't done, then the inter-actor collision system will
// generate false UnTouch messages and will fail to recognize that actors
// are touching depending on their ordering in the actor list relative to
// moving brush whose nodes they reside in.
//
void FMovingBrushTracker::ForceTouchFlush(INDEX iDynamic)
	{
	while (iDynamic!=INDEX_NONE)
		{
		FBspDynamicActor	*ActorReference = (FBspDynamicActor *)Level->Dynamics.Dynamics[iDynamic];
		INDEX				iActor			= ActorReference->iActor;
		//
		for (int i=0; i<NumTouchActors; i++)
			{
			if (iTouchActors[i]==iActor) goto Next;
			};
		if (NumTouchActors<MAX_TOUCHING_ACTORS)
			{
			iTouchActors[NumTouchActors++]=iActor;
			};
		Next:
		iDynamic = ActorReference->iNext;
		};
	};

//
// Flush all sporadic information (nodes, poly points) for a moving brush from 
// the level.  If Group is true, expects that NumGroupActors and iGroupActors 
// are valid.
//
void FMovingBrushTracker::FlushActorBrush(INDEX iActor,int Group)
	{
	GUARD;
	AssertLocked();
	if (!ActorBrushes[iActor]) appError("No brush found for actor");
	//
	// Go through all sporadic nodes in the Bsp and find ones owned by this actor:
	//
	INDEX iNode = ActorBrushes[iActor]->Color;
	while(iNode!=INDEX_NONE)
		{
		FBspNode *Node = &ModelInfo.BspNodes[iNode];
		INDEX iParent    = iNodeParents        [iNode-ModelInfo.NumBspNodes];
		FBspNode *Parent = &ModelInfo.BspNodes [iParent];
		//
		#ifdef PARANOID
			if (iNodeOwners[iNode-ModelInfo.NumBspNodes]!=iActor) appError("Flush node inconsistency");
		#endif
		//
		// Remove references to this node from its parents:
		//
		if		(Parent->iFront==iNode) Parent->iFront=INDEX_NONE;
		else if (Parent->iBack ==iNode) Parent->iBack =INDEX_NONE;
		else if (Parent->iPlane==iNode) Parent->iPlane=INDEX_NONE;
		#ifdef PARANOID
			else appError("Parent mismatch");
		#endif
		//
		// If those node has any children belonging to moving brushes that
		// aren't in our group list, tag them for subsequent flushing. This
		// prevents them from being orphaned.
		//
		if (Group)
			{
			if (Node->iFront!=INDEX_NONE)		ForceGroupFlush(Node->iFront);
			if (Node->iBack !=INDEX_NONE)		ForceGroupFlush(Node->iBack );
			if (Node->iPlane!=INDEX_NONE)		ForceGroupFlush(Node->iPlane);
			//
			if (Node->iDynamic[0]!=INDEX_NONE)	ForceTouchFlush(Node->iDynamic[0]);
			if (Node->iDynamic[1]!=INDEX_NONE)	ForceTouchFlush(Node->iDynamic[1]);
			};
		//
		// Free all sporadic data:
		//
		FVertPool *VertPool = &ModelInfo.VertPool[Node->iVertPool];
		for (DWORD j=0; j<Node->NumVertices; j++)
			{
			FreePointIndex(VertPool->pVertex);
			VertPool++;
			};
		FreeVertPoolIndex    (Node->iVertPool,Node->NumVertices);
		FreeNodeIndex        (iNode);
		//
		iNode = Node->iUniquePlane;
		};
	UNGUARD("FMovingBrushTracker::FlushActorBrush");
	};

/*---------------------------------------------------------------------------------------
	Public operations
---------------------------------------------------------------------------------------*/


//
// Setup all brushes in the level.  Cleans out all permanent and sporadic data, then
// sets up permanent data (surfaces, vectors) for all moving brushes.  Sporadic data
// isn't added until the next call to UpdateAllBrushes.
//
void FMovingBrushTracker::SetupAllBrushes(void)
	{
	GUARD;
	AssertLocked();
	//
	RemoveAllBrushes();
	//
	AActor *Actor = &Level->Actors->Element(0);
	for (int i=0; i<Level->Actors->Max; i++)
		{
		if (Actor->Class && Actor->IsMovingBrush()) SetupActorBrush(i);
		Actor++;
		};
	UNGUARD("FMovingBrushTracker::SetupAllBrushes");
	};

//
// Remove all permanent and sporadic moving brush data.
// Note that this must be called before a level is saved, or else Bsp nodes that
// should be leaves will reference nodes in the range from Num-Max that don't exist 
// in the saved file resulting.
//
void FMovingBrushTracker::RemoveAllBrushes(void)
	{
	GUARD;
	AssertLocked();
	//
	// Flush all sporadic data:
	//
	AActor *Actor = &Level->Actors->Element(0);
	for (int i=0; i<Level->Actors->Max; i++)
		{
		if (Actor->Class && Actor->IsMovingBrush() && ActorBrushes[i]) FlushActorBrush(i,0);
		Actor++;
		};
	//
	// Verify that all dynamic nodes were actually removed:
	//
	#ifdef PARANOID
	int n = LevelRes->Model->BspNodes->Num;
	for (i=0; i<n; i++)
		{
		FBspNode *Node = &LevelRes->Model->BspNodes->Element(i);
		//
		if ((Node->iFront!=INDEX_NONE) && (Node->iFront >= n))	appErrorf("Bad iFront %i",i);
		if ((Node->iBack !=INDEX_NONE) && (Node->iBack  >= n))	appErrorf("Bad iBack %i",i);
		if ((Node->iPlane!=INDEX_NONE) && (Node->iPlane >= n))	appErrorf("Bad iPlane %i",i);
		};
	#endif
	//
	// Remove all permanent data:
	//
	Actor = &Level->Actors->Element(0);
	for (int i=0; i<Level->Actors->Max; i++)
		{
		if (Actor->Class && Actor->IsMovingBrush() && ActorBrushes[i]) RemoveActorBrush(i);
		Actor++;
		};
	UNGUARD("FMovingBrushTracker::RemoveAllBrushes");
	};

//
// Update certain moving brushes in the level.  Call with Actors=NULL to update all moving
// brushes, or with a list of specific brushes to be updated in lock-step.  All of the
// specified moving brushes (and brushes interwoved with their Bsp's) are cleaned out of the 
// Bsp, and then re-added.
//
void FMovingBrushTracker::UpdateBrushes(INDEX *iActors,int Num)
	{
	GUARD;
	AssertLocked();
	int Group;
	//
	if (iActors==NULL) // Update all actor brushes
		{
		Group = 0;
		NumGroupActors = 0;
		AActor *Actor = &Actors[0];
		for (int i=0; i<Level->Actors->Max; i++)
			{
			if (Actor->Class && Actor->IsMovingBrush() && 
				(NumGroupActors < MAX_MOVING_BRUSH_ACTORS))
				{
				iGroupActors[NumGroupActors++] = i;
				};
			Actor++;
			};
		}
	else // Update specified actor brushes
		{
		Group = 1;
		NumGroupActors = OurMin(Num,(int)MAX_MOVING_BRUSH_ACTORS);
		memcpy(iGroupActors,iActors,Num*sizeof(INDEX));
		};
	NumTouchActors;
	//
	// Flush all actor brushes.  Calls to FlushActorBrush with Group set to true
	// cause iGroupActors to be expanded to include all brushes interwoven with the
	// specified brushes.
	//
	for (int i=0; i<NumGroupActors; i++)
		{
		INDEX  iActor = iGroupActors [i];
		AActor *Actor = &Actors      [iActor];
		//
		if (Actor->Class && Actor->IsMovingBrush() && ActorBrushes[iActor])
			{
			FlushActorBrush(iActor,Group);
			};
		};
	//
	// Add all moving brush sporadic data to the world:
	//
	for (int i=0; i<NumGroupActors; i++)
		{
		INDEX  iActor = iGroupActors [i];
		AActor *Actor = &Actors      [iActor];
		//
		if (Actor->Class && Actor->IsMovingBrush())
			{
			if (!ActorBrushes[iActor]) SetupActorBrush(iActor);
			AddActorBrush(iActor);
			};
		};
	//
	// Go through list of all actors whose collision info was lost
	// while the brushes were moved and update their collision info:
	//
	if (Group)
		{
		for (int i=0; i<NumTouchActors; i++)
			{
			Level->Dynamics.AddActorSphere(iTouchActors[i]);
			};
		};
	UNGUARD("FMovingBrushTracker::UpdateBrushes");
	};

/*---------------------------------------------------------------------------------------
	The End
---------------------------------------------------------------------------------------*/
