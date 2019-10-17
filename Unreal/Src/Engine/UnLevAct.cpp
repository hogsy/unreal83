/*=============================================================================
	UnLevActor.cpp: Level actor functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#define _DO_NOT_DEFINE_CLASS_ENUMS
#include "Unreal.h"
#include "Root.h"

#define COLLISION_SPHERE_FACTOR 1.0

/*-----------------------------------------------------------------------------
	Notes:

	Actor references are added to all Bsp leaves that the actor's collision
	sphere falls into.  This serves as a first-pass filter to determine
	potential actors that a given actor may collide with.  Actual collision
	is determined by a distance check with potential colliders.
	
	Note that actor references must be added to all leaves, not just outside 
	leaves, because of the case where two actors separated by a wall just barely 
	touch. This case often occurs with moving brush triggers placed inside 
	moving brushes.
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Level actor messaging
-----------------------------------------------------------------------------*/

//
// Send a message to an actor immediately and get a response into Result.  Returns 
// result if sent successfully and a value was returned, 0 if not.  When a message
// is sent to an actor's class function but not processed, it's routed to the actor's
// parent class (and so on) until either it is processed, or a base class has been
// reached.  This provides an easy way of expanding actor logic, by only replacing
// the message handlers that you want to change.
//
// Handles the case where an actor kills itself somewhere during a SendMessage call hierarchy.
//
int ILevel::SendMessage (INDEX iActor,FName Msg,void *Params,UClass *Class)
	{
	GUARD;
	//
	if (iActor==INDEX_NONE)  appError("INDEX_NONE");
	if (iActor>=Actors->Max) appErrorf("Invalid actor %i/%i",iActor,Actors->Max);
	//
	AActor	*Actor = &Actors->Element(iActor);
	if (Class==NULL) Class=Actor->Class;
	//
	#ifdef PARANOID
		if (Actor->iMe != iActor) appErrorf("iMe mismatch: %s (%i,%i)",Actor->Class->Name,iActor,Actor->iMe);
	#endif
	//
	// Send message to actor class's handler function.  If the handler doesn't process
	// the message, pass it to the parent class's handler until we've passed the root class.
	//
	while (Class && Actor->Class)
		{
		//
		// This is the weird C++ syntax to call the class function pointer Class->ActorFunc 
		// referencing the *Actor object.
		//
		//bug ("Calling %s",Class->Name);
		int Result = (Actor->*Class->ActorFunc)(this,Msg,Params);
		if (Result) return Result; // Message was finally processed
		Class = Class->ParentClass;
		};
	return 0; // Nobody processed the message
	UNGUARD("ILevel::SendMessage");
	};

//
// Send a message without routing it to the actor's parent class.
//
int ILevel::SendMessageDirect (INDEX iActor,FName Msg,void *Params)
	{
	GUARD;
	AActor *Actor  = &Actors->Element(iActor);
	UClass *Class  = Actor->Class;
	//
	#ifdef PARANOID
		if (Actor->iMe != iActor) appErrorf("iMe mismatch: %s",Actor->Class->Name);
	#endif
	//
	// Send message to actor class's handler function.
	//
	return (Actor->*Class->ActorFunc)(this,Msg,Params);
	UNGUARD("ILevel::SendMessageDirect");
	};

//
// Send a message to each class in an actor's parent hierarchy, starting at the
// root and ending at the current actor.  This is used for Spawn and for Validate.
//
int ILevel::SendMessageReverse (INDEX iActor,FName Msg,void *Params,UClass *SomeClass)
	{
	GUARD;
	AActor *Actor  = &Actors->Element(iActor);
	//
	// Recursively send to parents, starting at root:
	//
	if (!Actor->Class)			return 0; // Actor destroyed itself already
	if (SomeClass==NULL)		SomeClass = Actor->Class; // Start at the actor's class
	if (SomeClass->ParentClass) SendMessageReverse(iActor,Msg,Params,SomeClass->ParentClass);
	//
	// Send message to actor class's handler function.
	//
	if (!Actor->Class) return 0; // Actor destroyed itself already
	return (Actor->*SomeClass->ActorFunc)(this,Msg,Params);
	//
	UNGUARD("ILevel::SendMessageReverse");
	};

//
// Broadcast a message to all actors.  Returns count of messages actually processed.
//
int ILevel::BroadcastMessage (FName Msg,void *Params)
	{
	GUARD;
	INDEX  Count = 0;
	for( int Which = 0; Which < Actors->ActiveActors->Count(); Which++)
    {
        AActor * const Actor = (*Actors->ActiveActors)[Which];
        if(Actor != 0)
        {
            if (SendMessage (Actor->iMe,Msg,Params)) Count++;
        }
    }
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
	AActor *Actor = &Actors->Element(0);
	for (INDEX iActor = 0; iActor < Actors->Max; iActor++)
		{
		if (Actor->Class)
			{
			if (SendMessage (iActor,Msg,Params)) Count++;
			};
		Actor++;
		};
#endif
	return Count;
	UNGUARD("ILevel::BroadcastMessage");
	};

//
// Send a message to all actors matching certain criteria.  Returns number of messages
// sent and processed.  A message is sent to all actors that meet *all* of the following
// criteria:
//
// iActor    = Actor index to send to, INDEX_NONE = all actors.
// Name      = Name of actor, NAME_NONE = all names.
// Class     = Actor's class or its arbitrary-level superclass, NULL or GClasses.Root = all classes.
//
int	ILevel::SendMessageEx(FName Message,void *Params,INDEX iActor,FName Name,UClass *Class)
	{
	GUARD;
	INDEX Count = 0;
	for( int Which = 0; Which < Actors->ActiveActors->Count(); Which++)
    {
        AActor * const Actor = (*Actors->ActiveActors)[Which];
        if
        (
                Actor != 0
            &&  ( iActor==INDEX_NONE || iActor==Actor->iMe  ) 
            &&  ( Name==NAME_NONE    || Name==Actor->Name   ) 
            &&  ( Class==NULL        || Class==Actor->Class )
        )
        {
            if (SendMessage (Actor->iMe,Message,Params)) Count++;
        }
    }
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
	AActor *Actor = &Actors->Element(0);
	for (INDEX i = 0; i < Actors->Max; i++)
		{
		if  (
			(Actor->Class) &&
			((iActor==INDEX_NONE)   || (iActor==i)) &&
			((Name==NAME_NONE)		|| (Name==Actor->Name)) &&
			((Class==NULL)          || (Class==Actor->Class))
			)
			{
			if (SendMessage (i,Message,Params)) Count++;
			};
		Actor++;
		};
#endif
	return Count;
	UNGUARD("ILevel::SendMessageEx");
	};

/*-----------------------------------------------------------------------------
	Level actor possession
-----------------------------------------------------------------------------*/

//
// Hook a camera or user up to an actor.  Updates the actor.  You must update
// the camera or user with the actor's index yourself.  This only fails if the 
// actor is already hooked up to a camera or user; actors can't refuse 
// possession.  Returns 1 if success, 0 if failure.
//
int ILevel::PossessActor (INDEX iActor, UCamera *Camera)
	{
	GUARD;
	AActor *Actor = &Actors->Element(iActor);
	if (Actor->Camera)
		{
		//bug ("Possess failed");
		return 0;
		};
	Actor->Camera = Camera;
	SendMessage (iActor,ACTOR_Possess,Camera);
	//bug ("Possess succeeded");
	return 1;
	UNGUARD("ILevel::PossessActor");
	};

//
// Unpossess an actor, unhooking the actor's camera or user.  You must update
// the possessor yourself.  Returns the ID of the old posessor, or NULL if none.
//
UCamera *ILevel::UnpossessActor (INDEX iActor)
	{
	GUARD;
	AActor  *Actor			= &Actors->Element(iActor);
	UCamera	*OldPossessor	= Actor->Camera;
	//
	if (!Actor->Camera) return NULL; // Nothing to do
	Actor->Camera = NULL;
	//
	SendMessage (iActor,ACTOR_UnPossess,NULL);
	return OldPossessor;
	UNGUARD("ILevel::UnpossessActor");
	};

/*-----------------------------------------------------------------------------
	Level actor management
-----------------------------------------------------------------------------*/

//
// Create a new actor and sends it the ACTOR_SPAWN message.  Returns the
// new actor's index, or INDEX_NONE if either the actor list is full or
// the actor refuses to spawn.
//
// Does not recycle actors that were destroyed during this frame.
//
INDEX ILevel::SpawnActor (UClass *Class,FName ActorName,const FVector *Location,AActor *Template)
	{
	GUARD;
	if (Template==NULL) Template = &Class->DefaultActor;
	//
    if( Actors->UnusedActors == 0 )
		{
        Actors->RelistActors();
		}
    if( Actors->UnusedActors->Count() > 0 )
		{
        AActor * Actor = (*Actors->UnusedActors)[0];
        Actors->UnusedActors->RemoveIndex(0);
        Actors->ActiveActors->Add(Actor);
        const INDEX i = Actor - &Actors->Element(0);
		//
		// Save previous (empty) entry, if transactional:
		//
		if (Actors->Trans) GTrans->NoteActor (Actors,i);
		//
		// Initialize to defaults:
		//
		memcpy (Actor,Template,sizeof(AActor));
		if (Actor->Class != Class) appErrorf ("Class %s is not %s",Actor->Class->Name,Class->Name);
		Actor->InitServerInfo();
		//
		Actor->iMe		= i;
		Actor->Name     = ActorName;
		Actor->Camera   = NULL;
		Actor->Zone     = 0;
        Actors->ListActor(Actor); // Put actor on appropriate lists based on properties.
		//
		Actor->Location = *Location;
		Actor->ViewRot  = GMath.ZeroRotation;
		Actor->DrawRot  = GMath.ZeroRotation;
		//
		if (Actor->Brush) Actor->Brush = NULL;
		//
		// Send "spawn" message to all classes in its derivation hierarchy,
		// from the root (first) to its actual class (last):
		//
		SendMessageReverse(i,ACTOR_Spawn,NULL);
		if (!Actor->Class) return INDEX_NONE; // Actor destroyed itself in ACTOR_Spawn
		//
		if (State==LEVEL_UpPlay) SendMessage(i,ACTOR_BeginPlay,NULL);
		if (!Actor->Class) return INDEX_NONE; // Actor destroyed itself in ACTOR_BeginPlay
		//
		SetActorZone(i);
		if (!Actor->Class) return INDEX_NONE; // Actor destroyed itself in ACTOR_ZoneChange
		//
		Actors->Num = Actors->Max;
		return i;
		}
	else return INDEX_NONE; // Actor list is full
	UNGUARD("ILevel::SpawnActor");
	};

//
// Delete an actor from an actor list cleanly.  Upon exit, this actor no longer
// exists, and the level's actor list is in a clean state. Performs destruction 
// cleanup in this order:
//
// 1. Sends 'Destroy' to this actor.
// 2. Sends UnTouch to all actors that think they're touching this.
// 3. Recursively destroys all child actors.
// 4. If this actor is in its parent's inventory chain, removes it cleanly.
// 5. If this actor has a parent, notify it that is has lost a child.
// 6. Removes any references that other actors make to this actor.
// 7. Removes this actor from the actor list.
// 8. Destroys any users/cameras associated with this actor.
//
// Can safely handle the case where a message sent during DestroyActor() results
// in another call to DestroyActor.  Actors destroyed during this frame won't be
// recycled till the next frame.
//
// If the actor is static (bStaticActor), ignores the Destroy call entirely.
// Caller must either check for bStaticActor, or not assume that DestroyActor will
// definitly destroy the actor.
//
void ILevel::DestroyActor (INDEX iActor)
	{
	GUARD;
	AActor *ThisActor = &Actors->Element(iActor);
	if (ThisActor->bStaticActor && (State==LEVEL_UpPlay)) return;
	//
	if (Actors->Trans) GTrans->NoteActor (Actors,iActor);
	//
	// 1. Tell this actor it's about to be destroyed:
	//
	SendMessage (iActor,ACTOR_Destroy,NULL);
	if (!ThisActor->Class) return; // In case actor's Destroy logic redestroyed this actor.
	//
	// 2. Tell all touching actors that they're no longer touching this:
	//
	for( int Which = 0; Which < Actors->ActiveActors->Count(); Which++)
    {
        AActor * const Actor = (*Actors->ActiveActors)[Which];
        if(Actor != 0)
        {
			for (int j=0; j<FActorServerInfo::MAX_TOUCHING_ACTORS; j++)
				{
				if (Actor->ServerInfo.iTouchingActors[j]!=INDEX_NONE)
					{
					EndTouch(Actor->iMe,iActor);
					};
				};
        }
    }
	if (!ThisActor->Class) return; // In case any of those UnTouch's redestroyed this actor.
	//
	// 3. Recursively destroy all child actors:
	//
	for( int Which = 0; Which < Actors->ActiveActors->Count(); Which++)
    {
        AActor *Actor = (*Actors->ActiveActors)[Which];
        if( Actor != 0 && Actor->iParent==iActor )
        {
			DestroyActor(Actor->iMe);
        }
    }
	if (!ThisActor->Class) return; // In case any of those Destroy's redestroyed this actor.
	//
	// 4. If this actor is in its parent's inventory chain, delink it smoothly:
	//
	INDEX iInvActor = ThisActor->iParent;
	while (iInvActor != INDEX_NONE)
		{
		AActor *InvActor = &Actors->Element(iInvActor);
		if (InvActor->iInventory == iActor)
			{
			InvActor->iInventory = ThisActor->iInventory;
			break;
			};
		iInvActor = InvActor->iInventory;
		};
	//
	// 5. If this actor has a parent, notify it that is has lost a child:
	//
	if (ThisActor->iParent != INDEX_NONE)
		{
		SendMessage(ThisActor->iParent,ACTOR_LostChild,&iActor);
		};
	//
	// 6 Remove this actor from any other actors that reference it.  Send explicit
	// UnTarget and UnWeapon messages for Target references.
	//
	UnlinkActor(iActor);
	//
	// 7 Mark the actor as nonexistant, so that this actor is not recognized
	// from this point on:
	//
	ThisActor->Class		= NULL;
	ThisActor->bJustDeleted = TRUE;
    Actors->UnlistActor(ThisActor);
    Actors->ActiveActors->RemoveActor(ThisActor);
    Actors->JustDeletedActors->Add(ThisActor);
	//
	// 8 Scrap the actor's camera, if any:
	//
	if (ThisActor->Camera) ThisActor->Camera->Kill();
	UNGUARD("ILevel::DestroyActor");
	};

//
// Remove references to target, parent (and other actors) from an actor.
//
void ILevel::UnlinkActor (INDEX iActor)
	{
	GUARD;
	AActor *ThisActor = &Actors->Element(iActor);
	if (Actors->Trans) GTrans->NoteActor (Actors,iActor);
	//
	for( int Which = 0; Which < Actors->ActiveActors->Count(); Which++)
		{
        AActor * const Actor = (*Actors->ActiveActors)[Which];
        if( Actor != 0 )
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
		if (Actor->Class)
#endif
			{
			if (Actor->iTarget==iActor)
				{
				if (Actors->Trans) GTrans->NoteActor (Actors,Actor->iMe);
				Actor->iTarget = INDEX_NONE;
				SendMessage (Actor->iMe,ACTOR_UnTarget,NULL);
				if (!ThisActor->Class) return; // Actor was destroyed in SendMessage
				};
			if (Actor->iWeapon==iActor)
				{
				if (Actors->Trans) GTrans->NoteActor (Actors,Actor->iMe);
				Actor->iTarget = INDEX_NONE;
				SendMessage (Actor->iMe,ACTOR_UnWeapon,NULL);
				if (!ThisActor->Class) return; // Actor was destroyed in SendMessage
				};
			FClassProperty *Property = &Actor->Class->Element(0);
			for (int j=0; j<Actor->Class->Num; j++)
				{
				if ((Property->PropertyType==CPT_Actor) && stricmp(Property->PropertyName.Name(),"Me"))
					{
					INDEX *iLinkedActor = (INDEX *)(&((BYTE *)Actor)[Property->PropertyOffset]);
					if (*iLinkedActor==iActor)
						{
						if (Actors->Trans) GTrans->NoteActor (Actors,Actor->iMe);
						*iLinkedActor = INDEX_NONE;
						};
					};
				Property++;
				};
			};
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
		Actor++;
#endif
		};
	UNGUARD("ILevel::UnlinkActor");
	};

/*-----------------------------------------------------------------------------
	Player spawning
-----------------------------------------------------------------------------*/

//
// Find an available camera actor in the level and return it, or spawn a new
// one if none are available.  Returns actor number or INDEX_NONE if none are
// available.
//
INDEX ILevel::SpawnCameraActor (UCamera *Camera,FName MatchName)
	{
	GUARD;
	INDEX 	iActor = INDEX_NONE;
	//
	// Find an existing actor:
	//
	for( int Which = 0; Which < Actors->ActiveActors->Count(); Which++)
    {
        AActor * const Actor = (*Actors->ActiveActors)[Which];
		if ( Actor != 0 && (Actor->Class == GClasses.Camera) && (!Actor->Camera) &&
			((MatchName.IsNone())||(MatchName==Actor->Name))) 
        {
            iActor = Actor->iMe;
            break;
        }
    }
    if( iActor == INDEX_NONE )
		{
		//
		// None found, spawn a new one and set default position
		//
		iActor = SpawnActor(GClasses.Camera,NAME_NONE,&GMath.DefaultCameraStart);
		if (iActor==INDEX_NONE) return INDEX_NONE;
		//
		AActor * Actor	= &Actors->Element(iActor);
		Actor->ViewRot  = GMath.DefaultCameraRotation;
		};
	//
	// Successfully spawned an actor
	//
	if (!PossessActor(iActor,Camera)) // Posess failed
		{
		DestroyActor(iActor);
		return INDEX_NONE;
		};
	Actors->Element(iActor).Name.Add(Camera->Name);
	//
	return iActor;
	UNGUARD("ILevel::SpawnCameraActor");
	};

//
// Spawn an actor for gameplay.
// Places at an appropriate PlayerStart point using the PlayerStart's yaw.
//
INDEX ILevel::SpawnPlayActor (UCamera *Camera)
	{
	GUARD;
	//
	Camera->Level = Level;
	for (INDEX i=0; i<Actors->Max; i++)
		{
		AActor *PlayerStart = &Actors->Element(i);
		if (PlayerStart->Class == GClasses.PlayerStart)
			{
			FName PlayerName; PlayerName.Add("Player");
			INDEX iActor = SpawnActor(GClasses.Player,PlayerName,&PlayerStart->Location);
			if (iActor!=INDEX_NONE)
				{
				AActor *NewActor		= &Actors->Element(iActor);
				NewActor->ViewRot.Yaw	= PlayerStart->DrawRot.Yaw;
				NewActor->DrawRot.Yaw	= PlayerStart->DrawRot.Yaw;
				};
			return iActor;
			};
		};
	return INDEX_NONE;
	UNGUARD("ILevel::SpawnPlayActor");
	};

/*-----------------------------------------------------------------------------
	Level actor moving/placing
-----------------------------------------------------------------------------*/

//
// Try to place an actor that has moved a long way.  This is for
// moving actors through teleporters, adding them to levels, and
// starting them out in levels.  The results of this function is independent
// of the actor's current location and rotation.
//
// If the actor doesn't fit exactly in the location specified, tries
// to slightly move it out of walls and such.
//
// If bGravity and the actor is subject to gravity, places the actor on the
// floor below.  May move the actor a long way down.
//
// Returns 1 if the actor has been successfully moved, or 0 if it couldn't
// fit.
//
// Updates the actor's Zone and sends ZoneChange if it changes.
//
int ILevel::FarMoveActor (INDEX iActor, FVector *Location)
	{
	GUARD;
	AActor *Actor = &Actors->Element(iActor);
	//
	// Make sure destination isn't inside wall:
	//
	if (Actor->bCollideWorld && !ModelInfo.PointClass(Location,NULL)) return 0;
	//
	// Try to move the player's bounding sphere into a position near the destination
	// where it fits:
	//
	if (Actor->bCollideWorld)
		{
		FVector Delta = GMath.ZAxisVector;
		if (!ModelInfo.SphereMove (Location,&Delta,Actor->CollisionRadius,2)) return 0;
		};
	Actor->Location = *Location;
	//
	// Fall with gravity:
	//
	FVector Floor;
	INDEX iCollisionActor;
	if (Actor->bCollideWorld && Actor->bGravity && 
		(ModelInfo.ZCollision(&Actor->Location,&Floor,&iCollisionActor) != INDEX_NONE))
		{
		//
		// Move bounding sphere to floor, accounting for collision radius
		//
		FVector Delta	= GMath.ZeroVector;
		Delta.Z			= Floor.Z - Actor->Location.Z + Actor->CollisionHeight;
		ModelInfo.SphereMove (&Actor->Location,&Delta,Actor->CollisionRadius,1);
		};
	SetActorZone(iActor);
	return 1;
	UNGUARD("ILevel::FarMoveActor");
	};

//
// Try to move an actor to a nearby new location specified by NewLocation, using a movement
// vector Delta.  See ILevel::NearMoveActor for the complete definition of what this does.
//
// Returns 1 if the actor fits at NewLocation.  Otherwise, returns 0 and sets Adjustment
// to a suggested adjustment vector that may move the actor out the collision situation.
//
// This does *not* affect the actor or its location!
//
int	ILevel::PrivateTestMoveActor(INDEX iActor,FVector *NewLocation,FVector *Delta,FVector *Adjustment, DWORD *BumpMask)
	{
	AActor	*Actor	= &Actors->Element(iActor);
	int		Moved	= 0;
	//
	// Process collision and optional rebound with other actors:
	//
	if (Actor->bCollideActors)
		{
		INDEX iActorTouch[16];
		int NumTouched = Dynamics.CheckActorTouch
			(
			*NewLocation,COLLISION_SPHERE_FACTOR * Actor->CollisionRadius,
			iActorTouch,16
			);
		int LostTouch[FActorServerInfo::MAX_TOUCHING_ACTORS];
		for (int i=0; i<FActorServerInfo::MAX_TOUCHING_ACTORS; i++)
			{
			LostTouch[i] = (Actor->ServerInfo.iTouchingActors[i]!=INDEX_NONE);
			};
		for (int i=0; i<NumTouched; i++)
			{
			INDEX  iTouchActor = iActorTouch[i];
			if (iTouchActor != iActor)
				{
				AActor *TouchActor = &Actors->Element(iTouchActor);
				//
				if (Actor->bBlocksActors && TouchActor->bBlocksActors &&
					(strcmp(TouchActor->Class->Name,"BlockMonsters") || (!Actor->Camera) ||
					(!Actor->Class->IsKindOf(GClasses.Pawn))))
					{
					// Do touch and collision rebound:
					*Adjustment		= *NewLocation - TouchActor->Location;
					FLOAT OutSize	= Adjustment->Size();
					FLOAT OutDist   = 1.001 * COLLISION_SPHERE_FACTOR * (Actor->CollisionRadius + TouchActor->CollisionRadius) - OutSize;
					*Adjustment    *= OutDist/OutSize;
					//
					// Note that the touch occured:
					//
					BumpMask[iTouchActor>>5] |= 1 << (iTouchActor & 31);
					//
					return 0; // Movement failed (blocked by actor)
					}
				else
					{
					// Allow interpenetration and do touch/untouch
					for (int j=0; j<FActorServerInfo::MAX_TOUCHING_ACTORS; j++)
						{
						INDEX iOtherActor = Actor->ServerInfo.iTouchingActors[j];
						if (iOtherActor == iTouchActor)
							{
							LostTouch[j]=0; // We're still touching it
							goto Next; // Don't generate a new Touch message
							};
						};
					//
					// Set both actors as mutually touching, and send Touch messages.
					//
					BeginTouch(iActor,iTouchActor);
					BeginTouch(iTouchActor,iActor);
					};
				};
			Next:;
			};
		for (int i=0; i<FActorServerInfo::MAX_TOUCHING_ACTORS; i++)
			{
			INDEX iTouchActor = Actor->ServerInfo.iTouchingActors[i];
			if (LostTouch[i] && (iTouchActor != INDEX_NONE) && (iTouchActor!=iActor))
				{
				//
				// Set both actors as mutually untouching, and send Untouch messages:
				//
				EndTouch(iActor,iTouchActor);
				EndTouch(iTouchActor,iActor);
				};
			};
		};
	//
	// Check collision and do collision rebound with the world:
	//
	if (Actor->bCollideWorld) return ModelInfo.SphereTestMove
		(
		NewLocation,Delta,Adjustment,
		Actor->CollisionRadius,1
		);
	else return 1;
	};

//
// Worker function for MoveActor. Doesn't send ZoneChange messages.
// Doesn't actually dispatch Touch messages.
//
int ILevel::PrivateNearMoveActor(INDEX iActor, FVector *Delta,DWORD *BumpMask)
	{
	GUARD;
	AActor		*Actor			= &Actors->Element(iActor);
	FVector		NewLocation		= Actor->Location + *Delta;
	int			Iteration		= 0;
	int			Moved			= 0;
	//
	while (Iteration++ < 4)
		{
		FVector Adjustment;
		if (PrivateTestMoveActor(iActor,&NewLocation,Delta,&Adjustment,BumpMask))
			{
			// Moved without collision on this iteration - we have a valid destination
			Actor->Location  = NewLocation;
			Moved            = 1; 
			break;
			};
		// Collided; adjust and try again:
		NewLocation += Adjustment;
		};
	if (Moved) SetActorZone(iActor);
	return Moved;
	//
	UNGUARD("ILevel::MoveActor");
	};

//
// Tries to move the actor by a movement vector, expressed in "World units 
// per 1/35th second frame".  If no collision occurs, this function 
// just does a Location+=Move.
//
// Assumes that the actor's Location is valid and that the actor
// does fit in its current Location. Assumes that the level's 
// Dynamics member is locked, which will always be the case during
// a call to ILevel::Tick; if not locked, no actor-actor collision
// checking is performed.
//
// If this move places the actor somewhere that is shouldn't be,
// adjusts the velocity vector.  Upon return, Velocity is set to the
// actual velocity vector through which the actor was moved.
//
// If bCollideWorld, checks collision with the world.
//
// If bNoLedgeFall variable is set, prevents the actor from
//    falling off ledges which are higher than climbable stairs.
//
// For every actor-actor collision pair:
//
// If both have bCollideActors and bBlocksActors, performs collision
//    rebound, and dispatches Touch messages to touched-and-rebounded 
//    actors.  If both actors have nonzero masses, updates both
//    actors' velocities according to conservation of momentum:
//    (M1*V1 + M2*V2) before = (M1*V1 + M2*V2) after.
//
// If both have bCollideActors but either one doesn't have bBlocksActors,
//    checks collision with other actors (but lets this actor 
//    interpenetrate), and dispatches Touch and UnTouch messages.
//
// Returns 1 if some movement occured, 0 if no movement occured.
//
// Updates actor's Zone and sends ZoneChange if it changes.
//
int ILevel::MoveActor(INDEX iActor, FVector *Delta)
	{
	GUARD;
	DWORD *BumpMask = (DWORD *)GMem.GetZeroed((Actors->Max+31)>>3);
	//
	AActor *Actor = &Actors->Element(iActor);
	//
	int Steps = (int)(Delta->Size() * 4.00 / Actor->CollisionRadius);
	//
	if (Steps<1) Steps = 1;
	FVector NewDelta = *Delta/(FLOAT)Steps;
	//
	int Moved = 0;
	while (Steps-- > 0) Moved |= PrivateNearMoveActor(iActor,&NewDelta,BumpMask);
	//
	// Send messages for all bumps that occurred:
	//
	int j = 0;
	int n = (Actors->Max+31) >> 5;
	for (int i=0; i<n; i++)
		{
		if (BumpMask[i])
			{
			DWORD BitMask=1;
			for (int k=0; k<32; k++)
				{
				if (BumpMask[i] & BitMask)
					{
					PTouch TouchParams;
					//
					TouchParams.iActor = j;
					SendMessage (iActor,ACTOR_Bump,&TouchParams);
					if (!Actor->Class) goto DoneBump;
					//
					TouchParams.iActor = iActor;
					SendMessage (j,ACTOR_Bump,&TouchParams);
					if (!Actor->Class) goto DoneBump;
					};
				BitMask = BitMask << 1;
				j++;
				};
			}
		else j += 32;
		};
	DoneBump:
	GMem.Release(BumpMask);
	//
	return Moved;
	UNGUARD("ILevel::MoveActor");
	};

/*-----------------------------------------------------------------------------
	Actor touch minions
-----------------------------------------------------------------------------*/

void ILevel::BeginTouch(INDEX iActor,INDEX iTouchActor)
	{
	AActor *Actor = &Actors->Element(iActor);
	for (int i=0; i<FActorServerInfo::MAX_TOUCHING_ACTORS; i++)
		{
		if (Actor->ServerInfo.iTouchingActors[i]==INDEX_NONE)
			{
			Actor->ServerInfo.iTouchingActors[i] = iTouchActor;
			PTouch TouchParams;
			TouchParams.iActor = iTouchActor;
			SendMessage (iActor,ACTOR_Touch,&TouchParams);
			return;
			};
		};
	};

void ILevel::EndTouch(INDEX iActor,INDEX iTouchActor)
	{
	AActor *Actor = &Actors->Element(iActor);
	for (int i=0; i<FActorServerInfo::MAX_TOUCHING_ACTORS; i++)
		{
		if (Actor->ServerInfo.iTouchingActors[i] == iTouchActor)
			{
			Actor->ServerInfo.iTouchingActors[i] = INDEX_NONE;
			PTouch TouchParams;
			TouchParams.iActor = iTouchActor;
			SendMessage (iActor,ACTOR_UnTouch,&TouchParams);
			return;
			};
		};
	};

/*-----------------------------------------------------------------------------
	Radial effects
-----------------------------------------------------------------------------*/

//
// Impart a radial effect (momentum and a message) to all visible actors within a 
// certain radius.  The momentum's direction is outwards, as would be desired from
// an explosion (though you can supply a negative number for inwards).  Momentum
// decreases linearly from its maximum value at the specified location, to zero
// at the edge of the radius.  This only affects actors which have nonzero mass.
//
// If you supply a non-NULL HitMessageTemplate (such as a pre-built ExplodeHit 
// message), it will be sent to all actors to whom momentum was imparted.  It 
// will be sent with the appropriate HitLocation parameter; you specify the rest.
//
// If Notify is true, iSourceActor is sent a HitNotify (iHitActor,HitLocation) message 
// for every actor that is hit.  No WallNotify messages are generated by RadialHit.
//
// Does line-of-sight checks to prevent occluded actors from being affected.
//
// iSourceActor and iSourceActor's parent are ignored.
//
// Assumes: The level's dynamic structure is locked.
// Main purpose: Explosions.
//
void ILevel::RadialHit(INDEX iActorSource, FVector *Location, FLOAT Radius, FLOAT Momentum,
	PHit *HitMessageTemplate, int Notify)
	{
	};

/*-----------------------------------------------------------------------------
	Ray effects
-----------------------------------------------------------------------------*/

//
// Class used for tracking information for RayHit and its callback.
//
class FRayHitInfo
	{
	public:
	enum {MAX_RAY_HIT_ACTORS=256};
	//
	ILevel		*Level;
	int			NumHitActors;
	INDEX		HitActors[MAX_RAY_HIT_ACTORS];
	//
	};

//
// Raytracing callback function for RayHit.
//
// Concept: This callback is called for Bsp leaves in front-to-back order as a ray
// is traced from a starting point to an ending point.  We build up a list of these
// actors, skipping duplicates, as the raytrace proceeds.  When the ray terminates in 
// a wall or reaches its destination unoccluded) we then figure out which actor, if
// any, was hit first.
//
int RayHitCallback(IModel *ModelInfo, INDEX iLeafNode, int IsBack, int Param)
	{
	GUARD;
	FRayHitInfo *RayInfo	= (FRayHitInfo *)Param;
	ILevel		*Level		= RayInfo->Level;
	FBspNode	*Node		= &ModelInfo->BspNodes[iLeafNode];
	//
	INDEX iDynamic = Node->iDynamic[IsBack];
	while (iDynamic != INDEX_NONE)
		{
		FBspDynamicActor	*ActorReference = (FBspDynamicActor *)Level->Dynamics.Dynamics[iDynamic];
		INDEX				iActor			= ActorReference->iActor;
		AActor				*Actor			= &Level->Actors->Element(iActor);
		//
		if (Actor->Class && Actor->bCollideActors && Actor->bProjTarget &&
			(RayInfo->NumHitActors < FRayHitInfo::MAX_RAY_HIT_ACTORS))
			{
			for (int i=0; i<RayInfo->NumHitActors; i++) if (RayInfo->HitActors[i]==iActor) goto Skip;
			RayInfo->HitActors[RayInfo->NumHitActors++] = iActor;
			};
		Skip:;
		iDynamic = ActorReference->iNext;
		};
	return 0; // Allow raytrace to continue
	UNGUARD("RayHitCallback");
	};

//
// Impart a ray effect (momentum and a message) to the first actor hit by a
// ray.  The ray begins at Location, and continues in Direction until
// it either hits an actor tagged with bCollideActors, hits a wall, or zooms off
// into infinity.
//
// Momentum, if nonzero, will be applied to the actor that is hit
// by the ray, in Direction.
//
// If you specify a non-NULL HitMessageTemplate, that message is sent to the
// first actor that is hit along the ray, in order to damage it.  HitLocation
// is set by this function; you must supply all other variables.
//
// If Notify is true, iSourceActor is sent a HitNotify (HitLocation,iHitActor)
// message if an actor is hit, and a WallNotify message (WallLocation,WallNormal,
// iWallNode) if a wall is hit.
//
// Returns 1 if the ray has hit a wall				(in this case, no actor was notified)
// Returns 0 if the ray hits an actor				(in this case, the actor was notified)
// Returns 0 if the ray zooms off into infinity		(in this case, no actor was notified)
// Returns 0 if the ray hits a fake backdrop wall	(in this case, no actor was notified)
//
// Ignores iActorSource and iActorSource's iParent.
//
// Call with AutoTargetSlope=0.0 for no autotarget, 1.0 for 45-degree autotarget.
//
// Assumes: Direction is a vector (not necessarily normal).
// Main purpose: Instant-hit projectiles, like guns.
//
int ILevel::RayHit(INDEX iActorSource, const FVector *Location, const FVector *Direction,
	FLOAT Momentum, FLOAT AutoTargetSlope, 
	PHit *HitMessageTemplate, int Notify,
	FVector *WallLocation, FVector *WallNormal, INDEX *iWallNode
	)
	{
	GUARD;
	AActor *SourceActor  = (iActorSource!=INDEX_NONE) ? &Actors->Element(iActorSource) : NULL;
	INDEX  iSourceParent = SourceActor ? SourceActor->iParent : INDEX_NONE;
	FVector End,UnitDirection;
	//
	UnitDirection = *Direction;
	UnitDirection.Normalize();
	End = *Location + UnitDirection * 65536.0;
	//
	// Set up information class:
	//
	FRayHitInfo RayInfo;
	RayInfo.Level			= this;
	RayInfo.NumHitActors	= 0;
	//
	// Perform raytrace:
	//
	int HitAPoly = !ModelInfo.Raytrace
		(
		Location,&End,0,RayHitCallback,(int)&RayInfo,
		WallLocation,WallNormal,iWallNode
		);
	//
	// Sort through all potentially hit actors and find the nearest one:
	//
	FLOAT PolyDist;
	if (HitAPoly)	PolyDist = (*WallLocation - *Location) | UnitDirection;
	else			PolyDist = 1000000.0; // Past the valid end of the world
	//
	INDEX iHitActor    = INDEX_NONE;
	FLOAT HitActorDist = 1000000.0;
	for (int i=0; i<RayInfo.NumHitActors; i++)
		{
		INDEX  iActor = RayInfo.HitActors[i];
		AActor *Actor = &Actors->Element(iActor);
		//
		// Process this actor if it's behind the wall-hit location and in front of the ray:
		//
		FVector ActorVector  = Actor->Location - *Location;
		FLOAT   ActorDist	 = (ActorVector | UnitDirection) - Actor->CollisionRadius;
		//
		if (
			(ActorDist < PolyDist) &&
			(ActorDist > 0.0) &&
			(iActor != iActorSource) && (iActor != iSourceParent)
			)
			{
			// Compute minimum distance from actor's center to the ray:
			FVector NearestPointOnLine = UnitDirection * (ActorVector | UnitDirection);
			FLOAT   ActorRayDist       = FDist(NearestPointOnLine,ActorVector);
			// See if this actor is within hitting distance of the ray:
			if ((ActorRayDist < Actor->CollisionRadius) && (ActorRayDist < HitActorDist))
				{
				ActorRayDist = HitActorDist;
				iHitActor    = iActor;
				};
			};
		};
	if (iHitActor != INDEX_NONE) // Hit an actor
		{
		AActor		*HitActor	= &Actors->Element(iHitActor);
		FVector		HitLocation	= HitActor->Location;
		//
		if (HitActor->bMomentum && (Momentum!=0.0) && (HitActor->Mass!=0.0))
			{
			// Transfer momentum to the actor we've hit:
			HitActor->Velocity += UnitDirection * (Momentum / HitActor->Mass);
			};
		if (HitMessageTemplate)
			{
			// Send hit message to actor:
			HitMessageTemplate->HitLocation = HitLocation;
			SendMessage(iHitActor,ACTOR_Hit,HitMessageTemplate);
			};
		if (Notify)
			{
			// Send HitNotify message to source:
			PHitNotify HitNotify;
			HitNotify.iHitActor		= iHitActor;
			HitNotify.HitLocation	= HitLocation;
			SendMessage(iActorSource,ACTOR_HitNotify,&HitNotify);
			};
		return 0;
		}
	else if (HitAPoly) // Hit a poly
		{
		if (Notify)
			{
			// Send WallNotify message to source:
			PWallNotify WallNotify;
			WallNotify.WallLocation = *WallLocation;
			WallNotify.WallNormal   = *WallNormal;
			WallNotify.iWallNode    = *iWallNode;
			SendMessage(iActorSource,ACTOR_WallNotify,&WallNotify);
			};
		return 1;
		}
	else // Zoomed off into infinity
		{
		return 0;
		};
	UNGUARD("ILevel::RayHit");
	};

/*-----------------------------------------------------------------------------
	FBspDynamicsTracker implementation (Level dynamics)
-----------------------------------------------------------------------------*/

//
// Note that if an actor is destroyed, the actor's reference will still be
// stored in the dynamics structure for the remainder of the frame.  Therefore,
// all routines that use the dynamics information must make sure that Actor->Class's
// are non-NULL.  Logic in SpawnActor prevents an actor index from being recycled
// during a single frame.
//

//
// Lock the dynamics tracker.
//
void FBspDynamicsTracker::Lock(FMemPool *MemPool, ILevel *NewLevel,int NewMaxDynamics)
	{
	GUARD;
	if (Locked) appError("Already locked");
	//
	Mem				= MemPool;
	MemStart		= Mem->Get(0);
	Level			= NewLevel;
	Model			= &Level->ModelInfo;
	MaxDynamics		= NewMaxDynamics;
	NumDynamics		= 0;
	Locked			= 1;
	Overflowed		= 0;
	//
	AlterSize		= (Model->MaxBspNodes+31) >> 5;
	AlteredNodes	= (DWORD			 *)Mem->GetZeroed(AlterSize   * sizeof(DWORD));
	Dynamics		= (FBspDynamicItem  **)Mem->GetZeroed(MaxDynamics * sizeof(FBspDynamicItem *));
	UNGUARD("FBspDynamicsTracker::Lock");
	};

//
// Unlock the dynamics tracker.  Restores everything in the Bsp to its original
// state.
//
void FBspDynamicsTracker::Unlock(void)
	{
	GUARD;
	FBspNode *Node = &Model->BspNodes[0];
	if (!Locked) appError("Not locked");
	//
	#ifdef PARANOID
	if (Overflowed) debugf(LOG_Info,"Dynamics overflowed (%i)",MaxDynamics);
	#endif
	//
	Locked = 0;
	//
	for (int i=0; i<AlterSize; i++)
		{
		if (AlteredNodes[i])
			{
			int BitMask = 1;
			for (int j=0; j<32; j++)
				{
				if (AlteredNodes[i] & BitMask)
					{
					Node->iDynamic[0] = INDEX_NONE;
					Node->iDynamic[1] = INDEX_NONE;
					};
				BitMask = BitMask << 1;
				Node++;
				};
			}
		else Node += 32;
		};
	Mem->Release(MemStart);
	UNGUARD("FBspDynamicsTracker::Unlock");
	};

//
// Add a dynamic item reference to a Bsp node.  If the dynamics
// table is full, the item is not added.
//
void FBspDynamicsTracker::Add(INDEX iNode,FBspDynamicItem *Item,int AddToBack)
	{
	GUARD;
	//
	if (NumDynamics >= MaxDynamics)
		{
		Overflowed=1;
		return;
		};
	INDEX iDynamic = NumDynamics++;
	//
	AlteredNodes[iNode >> 5] |= 1 << ((int)iNode & 31);
	//
	FBspNode *Node = &Model->BspNodes[iNode];
	//
	Dynamics[iDynamic]			= Item;
	Item->iNext					= Node->iDynamic[AddToBack];
	Node->iDynamic[AddToBack]	= iDynamic;
	//
	UNGUARD("FBspDynamicsTracker::Add");
	};

void AddSphereCallback(IModel *ModelInfo, INDEX iNode, int IsBack, int Outside, int Param)
	{
	GUARD;
	//
	FBspDynamicsTracker *This = (FBspDynamicsTracker *)Param;
	//
	FBspDynamicItem *NewItem = (FBspDynamicItem *)This->Mem->Get(This->TempItemSize);
	memcpy(NewItem,This->TempItem,This->TempItemSize);
	//
	This->Add(iNode,NewItem,IsBack);
	GServer.ActorCollisionFrags++;
	//
	UNGUARD("AddSphereCallback");
	};

//
// Add a dynamic item reference to all leaves within a bounding sphere.
// If the dynamics table fills up, some items are not added.
//
void FBspDynamicsTracker::AddSphere(FBspDynamicItem *Item, int ItemSize, FVector &Location, FLOAT Radius)
	{
	GUARD;
	//
	TempItem     = Item;
	TempItemSize = ItemSize;
	//
	Model->SphereLeafFilter(&Location,Radius,AddSphereCallback,0,(int)this);
	//
	UNGUARD("FBspDynamicsTracker::AddSphere");
	};

/*-----------------------------------------------------------------------------
	FBspDynamicActor implementation (Level actor dynamics)
-----------------------------------------------------------------------------*/

//
// Add a reference to an actor at all leaves which the specified bounding sphere (assumed
// to be the actor's bounding sphere) lies in.
//
void FBspActorTracker::AddActorSphere(INDEX iActor)
	{
	GUARD;
	//
	AActor *Actor = &Level->Actors->Element(iActor);
	//
	FBspDynamicActor ActorReference;
	ActorReference.iActor = iActor;
	//
	AddSphere
		(
		&ActorReference,sizeof(ActorReference),
		Actor->Location,
		COLLISION_SPHERE_FACTOR * Actor->CollisionRadius
		);
	UNGUARD("FBspActorTracker::AddActorSphere");
	};

//
// Add references to all actors in the level, for collision.
//
void FBspActorTracker::AddAllActors(ILevel *Level)
	{
	GUARD;
	//
	int n=0;
	for( int Which = 0; Which < Level->Actors->CollidingActors->Count(); Which++)
        {
        AActor * const Actor = (*Level->Actors->CollidingActors)[Which];
		if( Actor != 0 && !Actor->bStaticActor)
			{
			AddActorSphere(Actor->iMe);
            n++;
			};
        }
#if 0 //todo: [Mark] Delete (obsolete due to new actor list processing)
	AActor *Actor = &Level->Actors->Element(0);
	for (INDEX i=0; i<Level->Actors->Max; i++)
		{
		if (Actor->Class && Actor->bCollideActors && !Actor->bStaticActor)
			{
			AddActorSphere(i);
			};
		Actor++;
		};
#endif
	//debugf("Added %i actors, %i fragments",n,NumDynamics);
	UNGUARD("FBspActorTracker::AddAllActors");
	};

//
// Callback for building a list of actors that collide with a given actor.
//
void CheckActorTouchCallback(IModel *ModelInfo, INDEX iNode, int IsBack, int Outside, int Param)
	{
	GUARD;
	//
	FBspNode         *Node		= &ModelInfo->BspNodes[iNode];
	FBspActorTracker *This		= (FBspActorTracker *)Param;
	INDEX            iDynamic	= Node->iDynamic[IsBack];
	//
	while ((iDynamic != INDEX_NONE) && (This->TempNum<FBspActorTracker::MAX_COLLISION_ACTORS))
		{
		FBspDynamicActor *ActorReference = (FBspDynamicActor *)This->Dynamics[iDynamic];
		//
		INDEX  iTempActor = ActorReference->iActor;
		AActor *TempActor = &This->Level->Actors->Element(iTempActor);
		if (TempActor->Class) // Only process it if the actor wasn't already destroyed
			{
			for (int i=0; i<This->TempNum; i++) // Prevent duplicates
				{
				if (This->TempActors[i] == iTempActor) goto Next;
				};
			This->TempActors[This->TempNum++] = ActorReference->iActor;
			};
		Next:
		iDynamic = ActorReference->iNext;
		};
	UNGUARD("CheckActorTouchCallback");
	};

//
// Return a list of all actors touched by a sphere (assumed to be an actor).  Each actor index
// in the list is guaranteed to be unique.
//
int FBspActorTracker::CheckActorTouch(FVector &Location, FLOAT Radius, INDEX *TouchedActors, int MaxTouchedActors)
	{
	GUARD;
	//
	// Build a list of all prospective collision actors == all actors that share at least
	// one Bsp leaf with this sphere:
	//
	TempNum	= 0;
	Model->SphereLeafFilter(&Location,Radius,CheckActorTouchCallback,0,(int)this);
	//
	// Keep all actors which actually touch this sphere:
	//
	int		Num			= 0;
	INDEX	*iActorPtr	= &TempActors[0];
	for (int i=0; i<TempNum; i++)
		{
		AActor *Actor = &Level->Actors->Element(*iActorPtr);
		//
		if (FDist(Location,Actor->Location)<=(Radius + COLLISION_SPHERE_FACTOR * Actor->CollisionRadius))
			{
			TouchedActors[Num++] = *iActorPtr;
			if (Num>=MaxTouchedActors) break;
			};
		iActorPtr++;
		};
	return Num;
	UNGUARD("FBspActorTracker::CheckActorTouch");
	};

/*-----------------------------------------------------------------------------
	ILevel descriptor getters
-----------------------------------------------------------------------------*/

//
// Return the index of the actor which contains the summary information
// for a particular zone.  A zone descriptor actor can optionally be
// placed in a particular zone in UnrealEd in order to specify custom
// properties of a zone - its name, water effects, fog effects, etc.
//
// Returns INDEX_NONE if no zone descriptor exists for this zone.
// In this case, the zone should be treated as if all properties are default.
//
// Returns INDEX_NONE if you pass a zone value of zero (which indicates 'no zone').
//
INDEX ILevel::GetZoneDescriptor(int iZone)
	{
	GUARD;
	//
	if ((iZone==0)||(ModelInfo.NumBspNodes==0)||(ModelInfo.NumZones==0)) return INDEX_NONE;
	//
	FZoneProperties	*Props = &Model->BspNodes->Zones[iZone];
	return Props->iZoneActor;
	//
	UNGUARD("ILevel::GetZoneDescriptor");
	};

//
// Return the index of the level descriptor actor for a zone,
// or INDEX_NONE if the level contains no level descriptor.
// The level descriptor, if present, must be actor 0.
//
INDEX ILevel::GetLevelDescriptor()
	{
	if( Actors->Element(0).Class==GClasses.LevelDescriptor ) return 0;
	else return INDEX_NONE;
	};

/*-----------------------------------------------------------------------------
	ILevel zone functions
-----------------------------------------------------------------------------*/

//
// Return zone number that the specified actor resides in.
// A result of zero indicates that the actor isn't in a particular
// zone, meaning that it's inside of a wall or the level's zones have
// not been built.
//
// Returns 0 if you pass INDEX_NONE.
//
int ILevel::GetActorZone(INDEX iActor)
	{
	GUARD;
	//
	if (iActor==INDEX_NONE) return 0;
	AActor *Actor = &Actors->Element(iActor);
	return ModelInfo.PointZone(&Actor->Location);
	//
	UNGUARD("ILevel::GetActorZone");
	};

//
// Figure out which zone an actor is in, update the actor's iZone,
// and notify the actor of the zone change.  Skips the zone notification
// if the zone hasn't changed.
//
void ILevel::SetActorZone(INDEX iActor)
	{
	GUARD;
	//
	AActor *Actor	= &Actors->Element(iActor);
	int    iZone	= GetActorZone(iActor);
	//
	if (iZone != Actor->Zone)
		{
		Actor->Zone = iZone;
		SendMessage(iActor,ACTOR_ZoneChange,NULL);
		};
	UNGUARD("ILevel::SetActorZone");
	};

//
// Return the ambient velocity of a zone; this velocity
// is added to all players every tick.
//
// Returns a zero vector if you pass INDEX_NONE.
//
FVector ILevel::GetZoneVelocity(INDEX iActor)
{
	if( iActor != INDEX_NONE )
	{
		AActor& Actor = Actors->Element(iActor);
		if( Actor.Zone )
		{
			INDEX iZoneDescriptor = GetZoneDescriptor( Actor.Zone );
			if( iZoneDescriptor != INDEX_NONE )
			{
				AZoneDescriptor& ZoneDescriptor = (AZoneDescriptor&)Actors->Element(iZoneDescriptor);
				if( ZoneDescriptor.bVelocityZone )
				return ZoneDescriptor.ZoneVelocity;
			}
		}
	}
	return GMath.ZeroVector;
}

//
// Get gravity acceleration for the current zone/level.
//
FVector ILevel::GetZoneGravityAcceleration(INDEX iActor)
	{
		if( iActor != INDEX_NONE )
	{
		AActor& Actor = Actors->Element(iActor);
		if( Actor.Zone )
		{
			INDEX iZoneDescriptor = GetZoneDescriptor( Actor.Zone );
			if( iZoneDescriptor != INDEX_NONE )
			{
				AZoneDescriptor& ZoneDescriptor = (AZoneDescriptor&)Actors->Element(iZoneDescriptor);
				if( ZoneDescriptor.bGravityZone )
					return ZoneDescriptor.ZoneGravity;
			}
		}
	}
	INDEX iLevelDescriptor = GetLevelDescriptor();
	if( iLevelDescriptor != INDEX_NONE )
	{
		ALevelDescriptor& LevelDescriptor = (ALevelDescriptor&)Actors->Element(iLevelDescriptor);
		return LevelDescriptor.LevelGravity;
	}
	FVector Result;
	Result.Make(0.f,0.f,-3.f);
	return Result;
	};

/*-----------------------------------------------------------------------------
	Sound properties
-----------------------------------------------------------------------------*/

//
// Set the sound properties for the zone surrounding the specified actor. Sets:
//
// ZoneBreadth = a measure of the size of the zone in world units (affects reverb 
// timing).  Think of the zone as a cube, and this represents the length of one side.
// Ranges from 0.0 (infinitesimal zone) to 65536.0 (maximum allowable size).
// 
// ZoneReflectivity = how much the zone walls reflect sound.  0=Absorbs all sound
// hitting walls, 1.0 = reflects all sound hitting walls.  This should have no
// effect on first-order sounds played, but should dampen second-order and successive
// reverbs.  The nth reverb should be dampened by ZoneReflectivity to the nth power.
//
void GetSoundProperties(INDEX iActor,FLOAT *ZoneBreadth,FLOAT *ZoneReflectivity)
	{
	GUARD;
	//
	*ZoneBreadth		= 100.0;
	*ZoneReflectivity	= 0.5;
	//
	UNGUARD("GetSoundProperties");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
