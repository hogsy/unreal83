/*=============================================================================
	UnMsgs.h: Header file registering all predefined Unreal actor messages
	Used by: Engine, Actor code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
        * July 21, 1996: Added kinds of messages used for each message.
        * Aug 30, 1996: Mark added PLevel, PreTeleport, PostTeleport.
=============================================================================*/

//
// Macro to define a message as an enumeration
//
#ifndef REGISTER_MESSAGE
	#define REGISTER_MESSAGE(num,msg) ACTOR_##msg = num,
	#define REGISTERING_ENUM
	enum EActorMessage {
#endif

/*-----------------------------------------------------------------------------
	All standard actor messages (must be 1-15 characters)
-----------------------------------------------------------------------------*/

// In the description for each message, the type of parameter associated with the
// message is shown in square brackets, such as [PTouch], or [null] if there are no
// parameters. These parameter types are found in UnActor.h.
// Message parameters marked with [????] are unknown. Please correct it if you know the parameter kind!

//
// Kernel-generated general messages
//
REGISTER_MESSAGE( 400,Null				) // [null] No effect
REGISTER_MESSAGE( 401,Spawn				) // [null] Reverse-sent to actor immediately after spawning
REGISTER_MESSAGE( 402,Destroy			) // [null] Called immediately before actor is removed from actor list
REGISTER_MESSAGE( 403,Debug				) // [????] Sent to an actor when debugging info is desired
REGISTER_MESSAGE( 404,Exec				) // [PExec] Execute a text command (ignored by root)
REGISTER_MESSAGE( 405,GetProp			) // [????] Ask for a property (handled by root)
REGISTER_MESSAGE( 406,SetProp			) // [????] Set a property (handled by root)
REGISTER_MESSAGE( 407,PostEditChange	) // [null] Called after editing properties
REGISTER_MESSAGE( 408,PostEditMove		) // [????] Called after an actor is moved in the editor
REGISTER_MESSAGE( 409,GainedChild		) // [????] Sent to a parent actor when another actor attaches to it
REGISTER_MESSAGE( 410,LostChild			) // [????] Sent to a parent actor when another actor detaches from it
REGISTER_MESSAGE( 411,QueryCallback		) // [????] Called by kernel in response to query messages
REGISTER_MESSAGE( 412,PreRaytrace		) // [null] Called by editor before raytracing begins
REGISTER_MESSAGE( 413,PostRaytrace		) // [null] Called by editor after raytracing ends
REGISTER_MESSAGE( 414,RaytraceWorld		) // [null] Tell actor to position itself for world raytracing
REGISTER_MESSAGE( 415,RaytraceBrush		) // [null] Tell actor to position itself for brush raytracing
REGISTER_MESSAGE( 416,PreEditChange		) // [null] Called before editing properties
//
// Kernel-generated level state messages
//
REGISTER_MESSAGE( 420,BeginPlay			) // [PBeginPlay] Play has just begin
REGISTER_MESSAGE( 421,EndPlay			) // [null] Play has just ended
REGISTER_MESSAGE( 422,BeginEdit			) // [null] Editing has just begin
REGISTER_MESSAGE( 423,EndEdit			) // [null] Editing has just ended
REGISTER_MESSAGE( 424,LevelClosing		) // [????] Level is closing and play will end soon
REGISTER_MESSAGE( 425,SetPlayMode		) // [????] Called with difficulty/net flags so undesired actors can kill themselves
REGISTER_MESSAGE( 426,PreBeginPlay		) // [PBeginPlay] Sent right before BeginPlay, for root initialization
REGISTER_MESSAGE( 427,PostBeginPlay		) // [PBeginPlay] Sent right after BeginPlay, for root processing
REGISTER_MESSAGE( 428,RestartLevel      ) // [null] Sent to restart the current level.
REGISTER_MESSAGE( 429,GoToLevel         ) // [PLevel] Sent to start up another level.
//
// Kernel-generated timer tick messages
//
REGISTER_MESSAGE( 440,Tick				) // [null] Clock tick update, no player controls
REGISTER_MESSAGE( 441,PlayerTick		) // [PPlayerTick] Clock tick update, player controls
REGISTER_MESSAGE( 442,PlayerCalcView	) // [PCalcView] Calculate player view
REGISTER_MESSAGE( 443,InventoryCalcView	) // [null] Calculate inventory/weapon view for player
// 
// Actor timing:
//
REGISTER_MESSAGE( 455,Timer				) // [null] Actor timer has fired
REGISTER_MESSAGE( 456,Minute			) // [????] A minute has passed
REGISTER_MESSAGE( 457,Hour				) // [????] An hour has passed
REGISTER_MESSAGE( 458,Day				) // [????] Daytime has just begun
REGISTER_MESSAGE( 459,Night				) // [????] Nighttime has just begun
//
// Inventory:
//
REGISTER_MESSAGE( 460,PickupQuery		) // [PPickupQuery] Pickup actor asks its toucher if it wants to pick the thing up (PPickupQuery)
// 461 is available for use
REGISTER_MESSAGE( 462,PreemptPickupQuery) // [PPickupQuery] Ask existing inventory whether a pickup should be allowed to occur, give them the action of overriding it
REGISTER_MESSAGE( 463,PickupNotify      ) // [PActor] Actor just picked up this inventory actor
REGISTER_MESSAGE( 464,AddInventory		) // [PActor] Add an actor to recipient's inventory (PTouch)
REGISTER_MESSAGE( 465,DeleteInventory	) // [PActor] Remove an actor from recipient's inventory (PTouch)
REGISTER_MESSAGE( 466,Activate			) // [null] Activate an inventory actor
REGISTER_MESSAGE( 467,DeActivate		) // [PActor] Deactivate an inventory actor
REGISTER_MESSAGE( 468,Use				) // [PUse] Use an inventory actor
REGISTER_MESSAGE( 469,UseExtra			) // [PUse] Secondary fire button
REGISTER_MESSAGE( 470,SwitchInventory	) // [PActor] Sent to all pawns when they need to switch inventory items
REGISTER_MESSAGE( 471,Reload            ) // [null] Sent to weapons to reload if empty and if ammo is available.
REGISTER_MESSAGE( 472,ChooseWeapon      ) // [null] Sent to player to cause automatic selection of appropriate weapon.
REGISTER_MESSAGE( 473,Release           ) // [null] Sent to a weapon when Use/UseExtra actions are released.
REGISTER_MESSAGE( 474,UseCloseUp        ) // [PActor] Sent to a weapon to use a close-up attack.
REGISTER_MESSAGE( 475,Activated         ) // [null] Sent to a weapon when it is raised and ready.
REGISTER_MESSAGE( 476,Deactivated       ) // [null] Sent to a weapon after it has been lowered.
//
// User-callable messages:
//
REGISTER_MESSAGE( 480,SetParent			) // [????] Sets this actors parent to the specified actor
REGISTER_MESSAGE( 481,QueryChildActors	) // [????] Causes kernel to call QueryActorCallback for all child actors
REGISTER_MESSAGE( 482,QueryEventActors	) // [????] Causes kernel to call QueryActorCallback for all actors using an event
REGISTER_MESSAGE( 483,QueryNameActors	) // [????] Causes kernel to call QueryActorCallback for all actors using a name
//
// Commonly used messages
//
REGISTER_MESSAGE( 500,TextMsg			) // [PText] Text chat message from somewhere
REGISTER_MESSAGE( 501,Target			) // [????] Target has become active (must be requested)
REGISTER_MESSAGE( 502,UnTarget			) // [null] Target has been destroyed
REGISTER_MESSAGE( 503,Weapon			) // [????] Weapon has become active
REGISTER_MESSAGE( 504,UnWeapon			) // [null] Weapon has been destroyed
REGISTER_MESSAGE( 505,Touch				) // [PTouch] Actor was just touched by another actor
REGISTER_MESSAGE( 506,UnTouch			) // [PTouch] Actor touch just ended, always sent sometime after Touch
REGISTER_MESSAGE( 507,Possess			) // [UCamera] Actor was just possessed by a user
REGISTER_MESSAGE( 508,UnPossess			) // [null] Actor was just unpossessed
REGISTER_MESSAGE( 509,Trigger			) // [PTouch] Message sent by Trigger actors
REGISTER_MESSAGE( 510,UnTrigger			) // [PTouch] Message sent by Trigger actors
REGISTER_MESSAGE( 511,Bump				) // [PTouch] Actor was just touched and blocked. No interpenetration. No UnBump.
REGISTER_MESSAGE( 512,MoverBump			) // [????] Actor was bumped by a keyframe mover and moved.
REGISTER_MESSAGE( 513,PlayerEntered		) // [????] Sent to a zone descriptor actor when a player enters
REGISTER_MESSAGE( 514,StandMover		) // [PTouch] Sent to actor when it first stands on a mover
REGISTER_MESSAGE( 515,UnStandMover		) // [PTouch] Sent to actor when it gets off a mover
REGISTER_MESSAGE( 516,SteppedOn			) // [PTouch] Sent to mover when actor first steps on it
REGISTER_MESSAGE( 517,UnSteppedOn		) // [PTouch] Sent to mover when actor steps off it
//
// Feedback from other actors or the world
//
REGISTER_MESSAGE( 520,Hit				) // [PHit] Actor was just damaged by something
REGISTER_MESSAGE( 521,KillCredit		) // [null] Actor has just received credit for a kill
REGISTER_MESSAGE( 522,HitNotify			) // [PHitNotify] Sent to actor during RadialHit, RayHit
REGISTER_MESSAGE( 523,WallNotify		) // [PWallNotify] Sent to actor during RayHit
REGISTER_MESSAGE( 524,PreTeleport       ) // [PTouch] Send to actor about to be teleported. Return -1 to abort teleportation, +1 to accept.
REGISTER_MESSAGE( 525,PostTeleport      ) // [PTouch] Sent to actor after being teleported.
//
// Physics
//
REGISTER_MESSAGE( 540,HitWall			) // [null] Ran into a wall (monster, projectile)
REGISTER_MESSAGE( 541,NearWall			) // [????] Actor is near a wall (must be requested)
REGISTER_MESSAGE( 542,Falling			) // [????] Actor is falling
REGISTER_MESSAGE( 543,Landed			) // [????] Actor has landed
REGISTER_MESSAGE( 544,ZoneChange		) // [PTouch] Actor has changed into a new zone
//
// AI hints
//
// Notes:
//   1. "Actor's target" is the actor's chosen target stored in AActor.iTarget.
REGISTER_MESSAGE( 620,AllIsQuiet      ) // [null] Not much is happening nearby. Actor should take it easy.
REGISTER_MESSAGE( 621,DoSomething     ) // [null] Do something.
REGISTER_MESSAGE( 622,Animate         ) // [PAnimate] Sent when starting new behaviour requiring a particular kind of animation.
REGISTER_MESSAGE( 623,EndAnimation    ) // [null] Sent at the end of the current animation.
REGISTER_MESSAGE( 624,FrameTrigger    ) // [PFrame] Animation frame trigger.
REGISTER_MESSAGE( 625,HarmTarget      ) // [null] Actor's target should be harmed.
REGISTER_MESSAGE( 626,SensedSomething ) // [PSense] Actor sensed something (player, explosion, light, ...).
REGISTER_MESSAGE( 627,Search          ) // [null] Actor should look around, see what's happening.
REGISTER_MESSAGE( 628,TargetIsNear    ) // [null] Actor's target is close enough for melee attack.
REGISTER_MESSAGE( 629,TargetMovedAway ) // [null] Actor's target moved out of melee range.

//
// Kills
//
REGISTER_MESSAGE( 640,Die				) // [null] Actor died (sent if specific die message not processed)
REGISTER_MESSAGE( 641,DieProjHit		) // [null] Die because of projectile hit
REGISTER_MESSAGE( 642,DieMeleeHit		) // [null] Die because of melee hit
REGISTER_MESSAGE( 643,DieInstantHit		) // [null] Die because of instantaneous hit
REGISTER_MESSAGE( 644,DieFloorHit		) // [null] Die because of damaging floor
REGISTER_MESSAGE( 645,DieMoverHit		) // [null] Die because of crushing mover
//
// Keyframers
//
REGISTER_MESSAGE( 660,KeyMoveTo			) // [PMoveTo] Move to a keyframe
REGISTER_MESSAGE( 661,KeyStop			) // [????] Stop moving immediately
REGISTER_MESSAGE( 662,KeyReverse		) // [null] Reverse course of KeyMoveTo
REGISTER_MESSAGE( 663,SetKeyPoint		) // [????] Set a keyframe position to that of a named keypoint actor

#ifdef REGISTERING_ENUM
	};
#endif

/*-----------------------------------------------------------------------------
	Closing
-----------------------------------------------------------------------------*/

#ifdef REGISTERING_ENUM
	#undef REGISTER_MESSAGE
	#undef REGISTERING_ENUM
#endif

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
