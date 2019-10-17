#ifndef _INC_UNFACTOR
#define _INC_UNFACTOR
/*
==============================================================================
UnFActor.h: Class FActor
Used by: Actor routines

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Class AActor defines the fundamental actor class, but it
    was specifically created for use by the Unreal Engine as
    well as the actor system. For this reason, the class AActor
    is not decorated with a lot of functions which might be
    useful to the actor system, especially where such functions
    might create dependencies on specific actor classes.

    Here we don't care about depending on specific actor classes
    (such as AWeapon, AProjectile, etc). The class FActor is provided
    to overlay AActor objects and provide additional functions.

    ** WARNING ** This is a kludge (redefining AActor objects
    onto FActor objects), but it does give us simpler code and a
    place to compartmentalize actor helper functions.

Revision history:
    * 07/04/96, Created by Mark
==============================================================================
*/

#include "UnGame.h"
#include "AIAnimat.h"
#include "AImotion.h"

struct AActorAI
{
    //Notes: Explain this stuff better.
    // Talk about how the base task shouldn't end until animations are ended.
    // Information relating to specific kinds of tasks:
    union
    {
        AIMotion    MotionGoal; // Motion goal, when the monster's main task is motion.
    };
    AIMotion        BasicMotion ; // Underlying motion goal.
    AIAnimations    Animations  ; // Ongoing animations.
};

extern void actorTick(INDEX iActor); //todo: move this

class FActor 
    : public AActor // FActor is an enhancement of class AActor.
{
public:
    // ** WARNING **
    // Objects of class AActor are cast to ojects of class FActor.
    // Do not add the following kinds of declarations:
    //   virtual functions
    //   member variables

    // Make an FActor from an AActor or ARoot:
    static FActor & Actor(AActor & Actor) { return (FActor &)Actor; }
    static const FActor & Actor(const AActor & Actor) { return (FActor &)Actor; }
    static FActor & Actor(ARoot & Actor) { return (FActor &)Actor; }
    static const FActor & Actor(const ARoot & Actor) { return (FActor &)Actor; }

    // Make an FActor from an index:
    static FActor & Actor( INDEX iActor ) { return Actor(GLevel->Actors->Element(iActor)); }

    // Make an FActor handle from an index, handling the case iActor==INDEX_NONE.
    static FActor * Handle( INDEX iActor ) { return iActor==INDEX_NONE ? 0 : &Actor(GLevel->Actors->Element(iActor)); }

    //------------------------------------------------------------------------
    // Convert a general FActor actor to a specific kind of actor.
    //------------------------------------------------------------------------
    // These functions let you avoid writing explicit type casts. 
    // In addition, these functions could be made safe (raising an error
    // if the actor is not of the expected class.)
    APawn           & Pawn          () { return (APawn          &)*this; }
    ARoot           & Root          () { return (ARoot          &)*this; }
    AProjectile     & Projectile    () { return (AProjectile    &)*this; }
    ALight          & Light         () { return (ALight         &)*this; }
    AInventory      & Inventory     () { return (AInventory     &)*this; }
    AWeapon         & Weapon        () { return (AWeapon        &)*this; }
    APowerUp        & PowerUp       () { return (APowerUp       &)*this; }
    AAmmo           & Ammo          () { return (AAmmo          &)*this; }
    APyrotechnics   & Pyrotechnic   () { return (APyrotechnics  &)*this; }
    ADecorations    & Decoration    () { return (ADecorations   &)*this; }
    AKeypoint       & Keypoint      () { return (AKeypoint      &)*this; }
    APlayerStart    & PlayerStart   () { return (APlayerStart   &)*this; }
    ATeleporter     & Teleporter    () { return (ATeleporter    &)*this; }
    AZoneDescriptor & ZoneDescriptor() { return (AZoneDescriptor&)*this; }
    ABlockMonsters  & BlockMonsters () { return (ABlockMonsters &)*this; }
    AMover          & Mover         () { return (AMover         &)*this; }
    ATrigger        & Trigger       () { return (ATrigger       &)*this; }

    const APawn           & Pawn          () const { return (APawn          &)*this; }
    const ARoot           & Root          () const { return (ARoot          &)*this; }
    const AProjectile     & Projectile    () const { return (AProjectile    &)*this; }
    const ALight          & Light         () const { return (ALight         &)*this; }
    const AInventory      & Inventory     () const { return (AInventory     &)*this; }
    const AWeapon         & Weapon        () const { return (AWeapon        &)*this; }
    const APowerUp        & PowerUp       () const { return (APowerUp       &)*this; }
    const AAmmo           & Ammo          () const { return (AAmmo          &)*this; }
    const APyrotechnics   & Pyrotechnic   () const { return (APyrotechnics  &)*this; }
    const ADecorations    & Decoration    () const { return (ADecorations   &)*this; }
    const AKeypoint       & Keypoint      () const { return (AKeypoint      &)*this; }
    const APlayerStart    & PlayerStart   () const { return (APlayerStart   &)*this; }
    const ATeleporter     & Teleporter    () const { return (ATeleporter    &)*this; }
    const AZoneDescriptor & ZoneDescriptor() const { return (AZoneDescriptor&)*this; }
    const ABlockMonsters  & BlockMonsters () const { return (ABlockMonsters &)*this; }
    const AMover          & Mover         () const { return (AMover         &)*this; }
    const ATrigger        & Trigger       () const { return (ATrigger       &)*this; }

    // Is the specifed actor a kind of actor specified by Class?
    BOOL IsA(UClass * Class) const { return this->Class->IsKindOf(Class); }

    void MakeSound(USound * Sound) const;
    void MakeSound(USound * Sound, const FVector & Location) const;
    void MakeNoise(FLOAT Noise); // Actor makes a noise - check to see if other actors sense it.
    void MakeNoise(FLOAT Noise, const FVector & Location); // Actor makes a noise at a distant location.

    void TriggerEvent(); // If this actor has an EventName, trigger it.

    void PickSound // Randomly pick a sound and play it.
    (
        USound   ** Sounds      // A list of Count sounds. There can be trailing null sounds, which are not selected.
    ,   int         Count  
    ) const;

    // Create a new actor based on the given info. Returns pointer to new actor, or 0 if none created.
    static FActor * Spawn 
    (
        UClass        * Class           // Class of actor to create. 0 if none.
    ,   const FVector & Location        // Location of new actor.
    ,   int             LifeSpan = 0    // Life span of new actor, or 0 to use default class lifespan.
    ,   const FVector & Velocity = GMath.ZeroVector // Velocity of new actor.
    );

    // Determine the first inventory item of the given class.
    // If found, return the item. Otherwise, return 0.
    FActor * InventoryItem 
    (
        UClass        * Class       // Look for an item of this kind.
    );

    // Determine if a pawn can use some ammo (and optionally apply the ammo).
    // *this must be a pawn.
    BOOL CanUseAmmo 
    (
        AAmmo   & Ammo       // Can *this make use of Ammo?
    ,   BOOL      Apply      // If TRUE, actually give the ammo to the pawn.
    );
    BOOL CanUseAmmo
    (
        EAmmoType AmmoType      // Can *this use more of this kind of ammo?
    ,   int       AmmoCount     // Amount of ammo available.
    ,   BOOL      Apply         // If TRUE, actually give the ammo to the pawn.
    );

    // Determine if a pawn can use a power-up (and optionally apply the power-up).
    // *this must be a pawn.
    BOOL CanUsePowerUp // Can the actor make use of a powerup?
    (
        APowerUp      & Power       // Can *this make use of this power-up?
    ,   BOOL            Apply       // If TRUE, actually apply the power-up to the pawn.
    );

    void AddExplosiveCharge(FLOAT Charge); // If *this is a pawn, add the given charge to it.

    // Stop any active timer.
    void StopTimer();

    // Start the actor's timer going.
    void StartTimer 
    (
        int             Time                  // How many ticks until the timer expires.
    ,   EActorMessage   Message = ACTOR_Timer // Message to send when timer expires.
    );

    // Set the actor's image.
    void FActor::UseFrame
    (
        int     Sequence            // The animation sequence number.
    ,   int     Frame               // Index (from 0) of the frame to use frame.
    );

    void CancelAnimations(); // Cancel all animations in progress (or suspended).
    void FinishAnimations(); // Cause animations to finish soon.
    BOOL IsAnimated() const; // Is there an animation in progress or pending?

    void ReplaceAnimation // Start a mesh animation. Replace any existing animations.
    (
        int             Sequence            // The animation sequence number.
    ,   BYTE            Count       = 1     // Number of times to do animation (0 means forever)
    ,   FLOAT           Rate        = 1.0   // Rate of animation (1.0=normal)
    ,   EActorMessage   Message = ACTOR_Null// Message to send at end of animation.
    );

    void AddAnimation // Add a mesh animation to the actor's list of animations.
    (
        AIAnimation::TSequence  Sequence    // Sequence number (1..)
    ,   BYTE                    Count   = 0     // Number of times to repeat sequence. 0=forever
    ,   FLOAT                   Rate    = 1.0   // Animation rate.   
    ,   EActorMessage           Message = ACTOR_Null    // Message to send at end of animation.
    );

    // Find the next inventory item (in the inventory chain) in the specified
    // inventory set *at or after* inventory item *this. Return 0 if not found.
    FActor * FindInventoryItemFromSet
    (
        EInventorySet   WhichSet    // Find item from this set.
    );

    // Activate an inventory item in *this's inventory.
    // Returns the item chosen, or 0 if none.
    FActor * SelectInventorySet
    (
        EInventorySet   WhichSet    // Activate an item from this set.
    ,   BOOL            Reuse       // TRUE to reuse the previously active item from the set.
    ,   BOOL            UseNext     // TRUE to use the item after the previously active item from the set.
        // Notes:
        //   1. Only one of Reuse and UseNext can be TRUE.
        //   2. If Reuse or UseNext is TRUE, and the inventory set doesn't have any
        //      previously active items, the first item in the set is chosen.
    );

    FActor * NearbyFacedPawn(const FRotation & View) const;
        // Returns a nearby pawn near the line of sight defined by View.
        // Return 0 if no such pawn exists. When more than one pawn is found,
        // return the one most "straight ahead".

    void AnimationTick();   // Advance animation by 1 tick. Send any ACTOR_FrameTrigger message.

    BOOL CanFire(BOOL Primary) const; // Is *this (a weapon) ready to fire? 

    int Fire // Make the actor (must be a weapon) fire a projectile
    // Returns the number of discharges.
    (
        BOOL    PrimaryUse     // TRUE if this is the primary use of the weapon (FALSE for secondary).
    ,   int     YawDeviation   // Deviation from weapon's direction of projectile's yaw
    ,   int     PitchDeviation // Deviation from weapon's direction of projectile's pitch
    );

    FActor & Target() const { return Actor(iTarget); }
    FActor & Parent() const { return Actor(iParent); }
    BOOL HasTarget() const { return iTarget != INDEX_NONE; }
    BOOL HasParent() const { return iParent != INDEX_NONE; }

    BOOL IsPlayer() const { return Camera!=0; }

    BOOL CanSeeAsFarAs(const FVector & Location) const; // Does *this (must be a pawn) have vision good enough to see as far as Location, ignoring obstacles?
//tbd?    BOOL CanHearAsFarAs(const FVector & Location) const; // Does *this (must be a pawn) have hearing good enough to hear as far as Location, ignoring obstacles?
    BOOL CanSee(const FVector & Location) const; // Can *this (must be a pawn) see a location?
    BOOL CanSee(const APawn & Pawn) const; // Can *this (must be a pawn) see a pawn?
    BOOL CanSeeTarget() const { return HasTarget() && CanSee(Target().Pawn()); }
    BOOL CanHear(const APawn & Pawn) const; // Can *this (must be a pawn) hear Pawn?
    BOOL CanSense(const APawn & Pawn) const; // Can *this (must be a pawn) sense Pawn?
    BOOL CanHear(const FVector & Location, FLOAT Noise) const; // Can *this (must be a pawn) hear a noise at the specified location.
    BOOL CanHear(FLOAT Distance, FLOAT Noise) const; // Can *this (must be a pawn) hear a noise at the specified distance.

    // Is the path from *this pawn to Pawn unobstructed and either within the sight radius or
    // loud enough to be heard?
    // This is deliberately vague - we are not saying *this does sense Pawn, just that
    // maybe it does.
    BOOL MightSense(const APawn & Pawn ) const;

    // Spawn a pyrotechnic effect, located relative to this->Location. 
    // Returns the created actor, or 0 if none.
    FActor * SpawnPyrotechnic
    ( 
        UClass      * Class    // The class defining the effect to spawn, or 0.
    ,   FLOAT         Forward  // Distance forward from this->Location, relative to this->DrawRot.
    ,   FLOAT         Up       // Distance upward from this->Location, relative to this->DrawRot.
    ,   FLOAT         Right    // Distance to right of this->Location, relative to this->DrawRot.
    )
    const;

    void CauseDamage // Have *this cause a single kind of damage to the specified target.
    (
        FActor    & Target
    ,   FLOAT       Damage
    ,   FLOAT       Momentum    = 0         // Momentum to impart.
    ,   EDamageType DamageType  = DMT_Basic // Kind of damage
    )
    const;

    void CauseRayDamage // Have *this cause basic damage to the first actor hit by a ray.
    (
        FActor    & Target
    ,   FLOAT       Damage
    ,   FLOAT       Momentum    // Amount of momentum to impart to target.
    );

    // Take damage (*this must be a pawn).
    // Return in Info.ActualDamage the total amount of damage sustained by
    // the pawn (not including damage absorbed by the armor). If the pawn is
    // killed, the pawn's LifeState is set to LS_Dead and the pawn is sent
    // an ACTOR_Die message.
    void TakeDamage( PHit & Info );

    void SetActorCollision(BOOL CollideActors);
        // Use SetActorCollision to change the bCollideActors property of an actor.
        // This is necessary because bCollideActors is an important property which
        // defines what actor lists the actor is kept in, so changing bCollideActors
        // requires changing the lists the actor is kept in.
        
    // Is an actor near another actor? (Use their collision radii.)
    BOOL FActor::IsNear( const FActor & Actor ) const;

    void StopTurning(); // Stop turning (*this must be a pawn).
    void StopMoving();  // Stop moving (*this must be a pawn), not including turning.
    void Stop() { StopTurning(); StopMoving(); }

    BOOL IsFacing(const FVector & Location) const; // Is an actor facing a location?
        // By "facing", we mean the location is in front of the actor.
        // Construct two vectors:
        //  A: along the actors orientation (DrawRot)
        //  B: from the actor to Location.
        // If the dot product of A and B is > 0, then the actor
        // is considered to be facing the location.

    void TurnTowards( const FVector & Target, int MaxChange = 5500 );

    void TeleportTo( INDEX iTarget );

    FVector ViewVector() const; // Vector in direction that actor is looking.
    FVector ForwardVector() const; // Vector in direction that actor is oriented (ignores actor's view).
    FVector RightwardVector() const; // Vector in direction to the right of actor's orientation (DrawRot).
    FVector UpwardVector() const; // Vector in direction upwards relative to actor's orientation (DrawRot).

    //------------------------------------------------------------------------
    // Convenience functions: Convert a level and index into various actor types:
    //------------------------------------------------------------------------
    // These functions let you avoid writing explicit type casts. 
    // In addition, these functions could be made safe (raising an error
    // if the actor is not of the expected class.)
    static APawn           & Pawn          ( INDEX iActor ) { return (APawn          &)Actor(iActor).Pawn          (); }
    static ARoot           & Root          ( INDEX iActor ) { return (ARoot          &)Actor(iActor).Root          (); }
    static AProjectile     & Projectile    ( INDEX iActor ) { return (AProjectile    &)Actor(iActor).Projectile    (); }
    static ALight          & Light         ( INDEX iActor ) { return (ALight         &)Actor(iActor).Light         (); }
    static AInventory      & Inventory     ( INDEX iActor ) { return (AInventory     &)Actor(iActor).Inventory     (); }
    static AWeapon         & Weapon        ( INDEX iActor ) { return (AWeapon        &)Actor(iActor).Weapon        (); }
    static APowerUp        & PowerUp       ( INDEX iActor ) { return (APowerUp       &)Actor(iActor).PowerUp       (); }
    static AAmmo           & Ammo          ( INDEX iActor ) { return (AAmmo          &)Actor(iActor).Ammo          (); }
    static APyrotechnics   & Pyrotechnic   ( INDEX iActor ) { return (APyrotechnics  &)Actor(iActor).Pyrotechnic   (); }
    static ADecorations    & Decoration    ( INDEX iActor ) { return (ADecorations   &)Actor(iActor).Decoration    (); }
    static AKeypoint       & Keypoint      ( INDEX iActor ) { return (AKeypoint      &)Actor(iActor).Keypoint      (); }
    static APlayerStart    & PlayerStart   ( INDEX iActor ) { return (APlayerStart   &)Actor(iActor).PlayerStart   (); }
    static ATeleporter     & Teleporter    ( INDEX iActor ) { return (ATeleporter    &)Actor(iActor).Teleporter    (); }
    static AZoneDescriptor & ZoneDescriptor( INDEX iActor ) { return (AZoneDescriptor&)Actor(iActor).ZoneDescriptor(); }
    static ABlockMonsters  & BlockMonsters ( INDEX iActor ) { return (ABlockMonsters &)Actor(iActor).BlockMonsters (); }
    static AMover          & Mover         ( INDEX iActor ) { return (AMover         &)Actor(iActor).Mover         (); }
    static ATrigger        & Trigger       ( INDEX iActor ) { return (ATrigger       &)Actor(iActor).Trigger       (); }

    //------------------------------------------------------------------------
    //      Actor messaging functions. 
    //------------------------------------------------------------------------
    // Use these functions to guarantee you are sending the proper parameters with
    // the message. The general form of the following functions is:
    //      int Send_XXXXX(INDEX iActor, ...)
    // to send a message to actor iActor, or
    //      int Send_XXXXX(...)
    // to send a message to *this.
    // Such functions send message ACTOR_XXXXX to the actor iActor or *this, with
    // any additional parameters supplied. The value returned is the 
    // value returned from the message handler.
    static int Send_Activate       (INDEX iActor);
    static int Send_AddInventory   (INDEX iActor, INDEX iInventory     );
    static int Send_AllIsQuiet     (INDEX iActor);
    static int Send_Animate        (INDEX iActor, PAnimate::TKind Animation);
    static int Send_Bump           (INDEX iActor, INDEX iToucher);
    static int Send_ChooseWeapon   (INDEX iActor);
    static int Send_DeActivate     (INDEX iActor, INDEX iNextInventory );
    static int Send_DeleteInventory(INDEX iActor, INDEX iInventory     );
    static int Send_Die            (INDEX iActor);
    static int Send_DoSomething    (INDEX iActor);
    static int Send_EndAnimation   (INDEX iActor);
    static int Send_FrameTrigger   (INDEX iActor, int Sequence, int Frame, int Trigger );
    static int Send_GoToLevel      (INDEX iActor, const char *LevelName );
    static int Send_HarmTarget     (INDEX iActor);
    static int Send_HitWall        (INDEX iActor);
    static int Send_Hit            (INDEX iActor, PHit & Info);
    static int Send_KillCredit     (INDEX iActor);
    static int Send_PickupNotify   (INDEX iActor, INDEX iPickup        );
    static int Send_PostTeleport   (INDEX iActor, INDEX iTeleporter);
    static int Send_PreTeleport    (INDEX iActor, INDEX iTeleporter);
    static int Send_Release        (INDEX iActor);
    static int Send_Reload         (INDEX iActor);
    static int Send_RestartLevel   (INDEX iActor);
    static int Send_Search         (INDEX iActor);
    static int Send_SensedSomething(INDEX iActor, PSense & Info );
    static int Send_SwitchInventory(INDEX iActor, INDEX iNextInventory );
    static int Send_TargetIsNear   (INDEX iActor);
    static int Send_TargetMovedAway(INDEX iActor);
    static int Send_TextMsg        (INDEX iActor, const char * Message, BYTE MessageType = LOG_Play );
    static int Send_Timer          (INDEX iActor);
    static int Send_Touch          (INDEX iActor, INDEX iToucher);
    static int Send_UnTarget       (INDEX iActor);
    static int Send_UnTouch        (INDEX iActor, INDEX iToucher);
    static int Send_Use            (INDEX iActor);
    static int Send_UseCloseUp     (INDEX iActor, INDEX iTarget);
    static int Send_UseExtra       (INDEX iActor);

    // Functions for an actor to send a message to itself:
    int Send_Activate       ()                      { return Send_Activate       (iMe); }
    int Send_AddInventory   (INDEX iInventory)      { return Send_AddInventory   (iMe,iInventory); }
    int Send_AllIsQuiet     ()                      { return Send_AllIsQuiet     (iMe); }
    int Send_Animate        (PAnimate::TKind Animation) { return Send_Animate        (iMe,Animation); }
    int Send_Bump           (INDEX iToucher)        { return Send_Bump           (iMe,iToucher); }
    int Send_ChooseWeapon   ()                      { return Send_ChooseWeapon   (iMe); }
    int Send_DeActivate     (INDEX iNextInventory ) { return Send_DeActivate     (iMe,iNextInventory); }
    int Send_DeleteInventory(INDEX iInventory     ) { return Send_DeleteInventory(iMe,iInventory); }
    int Send_Die            ()                      { return Send_Die            (iMe); }
    int Send_DoSomething    ()                      { return Send_DoSomething    (iMe); }
    int Send_EndAnimation   ()                      { return Send_EndAnimation   (iMe); }
    int Send_FrameTrigger   ( int Sequence, int Frame, int Trigger ) { return Send_FrameTrigger(iMe,Sequence,Frame,Trigger); }
    int Send_GoToLevel      (const char *LevelName) { return Send_GoToLevel(iMe, LevelName); }
    int Send_HarmTarget     ()                      { return Send_HarmTarget     (iMe); }
    int Send_HitWall        ()                      { return Send_HitWall        (iMe); }
    int Send_Hit            (PHit & Info)           { return Send_Hit            (iMe,Info); }
    int Send_KillCredit     ()                      { return Send_KillCredit     (iMe); }
    int Send_PickupNotify   (INDEX iPickup)         { return Send_PickupNotify   (iMe,iPickup); }
    int Send_PostTeleport   (INDEX iTeleporter)     { return Send_PostTeleport   (iMe,iTeleporter); }
    int Send_PreTeleport    (INDEX iTeleporter)     { return Send_PreTeleport    (iMe,iTeleporter); }
    int Send_Release        ()                      { return Send_Release        (iMe); }
    int Send_Reload         ()                      { return Send_Reload         (iMe); }
    int Send_RestartLevel   ()                      { return Send_RestartLevel   (iMe); }
    int Send_Search         ()                      { return Send_Search         (iMe); }
    int Send_SensedSomething(PSense & Info)         { return Send_SensedSomething(iMe,Info); }
    int Send_SwitchInventory(INDEX iNextInventory)  { return Send_SwitchInventory(iMe,iNextInventory); }
    int Send_TargetIsNear   ()                      { return Send_TargetIsNear   (iMe); }
    int Send_TargetMovedAway()                      { return Send_TargetMovedAway(iMe); }
    int Send_TextMsg        (const char * Message, BYTE MessageType = LOG_Play ) { return Send_TextMsg(iMe,Message,MessageType); }
    int Send_Timer          ()                      { return Send_Timer          (iMe); }
    int Send_Touch          (INDEX iToucher)        { return Send_Touch          (iMe,iToucher); }
    int Send_UnTarget       ()                      { return Send_UnTarget       (iMe); }
    int Send_UnTouch        (INDEX iToucher)        { return Send_UnTouch        (iMe,iToucher); }
    int Send_Use            ()                      { return Send_Use            (iMe); }
    int Send_UseCloseUp     (INDEX iTarget)         { return Send_UseCloseUp     (iMe,iTarget); }
    int Send_UseExtra       ()                      { return Send_UseExtra       (iMe); }

    //------------------------------------------------------------------------
    //                  AI-related functions.
    //------------------------------------------------------------------------
    AActorAI & AI() { return *(AActorAI*)&AIInfo; }
    const AActorAI & AI() const { return *(AActorAI*)&AIInfo; }
    void ClearTask(BOOL CancelAnimations = FALSE);
    void SetTask(EAI_Task Task, int Ticks); // Finish any existing animations, and add a new task.
    //              Functions to set the actor's immediate task.
    // Ticks=0 means the task has no time limitation.
    void DoHarm     (int Ticks = 0);
    void DoMotion   (int Ticks = 0);
    void DoSearch   (int Ticks = 0);
    void DoWait     (int Ticks = 0);

    void AddAnimation(const AIAnimation & Animation);
    //?void AddAnimation(AIAnimation::TSequence Sequence, BYTE Count = 0, FLOAT Rate = 1.0 );

    // Various tasks:
    
    void GoAlongVector(const FVector & Direction, int Ticks = 0 ); // Travel in a particular direction.
    void Near(const FVector & Location, int Ticks = 0 ); // Move towards and turn towards a location
    void Near(INDEX iActor, int Ticks = 0 ); // Move towards and turn towards an actor

    // Post a message back to the actor when the task is done.
    void SendWhenTaskIsDone( enum EActorMessage Message )
    {
        TimerMessage = Message;
    }

    // Set the task underlying motion goal. This goal is used even while completing other tasks.
    void KeepFacing(INDEX iActor) { AI().BasicMotion.Face(iActor); }
    void KeepGoingTo(INDEX iActor) { AI().BasicMotion.GoTo(iActor); }

    // Basic AI management:
    void EmptyAI(BOOL CancelAnimations = FALSE);
    void DumpAI() const; // Dump information to the log. If Actor != 0, dump related info from it.
    void InitializeAI();

};

static inline       AActor & actor(      ARoot & Actor) { return *(AActor*)&Actor; }
static inline const AActor & actor(const ARoot & Actor) { return *(AActor*)&Actor; }

static BOOL actorIs(const ARoot & Actor, UClass * Class) { return Actor.Class->IsKindOf(Class); }


// Return values for actor ::Process functions:
static const int ProcessDone        = 1 ;
static const int ProcessParent      = 0 ;


#endif
