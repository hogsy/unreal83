/*=============================================================================
    UnInv.cpp: Inventory class actor code

    Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
    Compiled with Visual C++ 4.0.

    Revision history:
        * Created by Tim Sweeney
        05/--/96 Many changes by Mark
=============================================================================*/

#include "UnGame.h"
#include "UnFActor.h"
#include "UnPrefer.h"
#include "UnCheat.h"

static const int WeaponActivationTime   = 8; // Time taken to activate or deactivate a weapon.
static const int CloseUpAttackTime      = 16; // Time taken to do close-up attack.
//----------------------------------------------------------------------------
//       Place active weapon relative to current player position and view
//----------------------------------------------------------------------------
static void SynchronizeInventoryItem
(
    AInventory  & Item
,   APawn       & ParentPawn
)
{
    Item.DrawRot         = ParentPawn.DrawRot;
    Item.ViewRot.Yaw     = ParentPawn.ViewRot.Yaw;
    Item.ViewRot.Pitch   = (SWORD)ParentPawn.ViewRot.Pitch - ( (SWORD)ParentPawn.ViewRot.Pitch > 0 ? 0 : ((SWORD)ParentPawn.ViewRot.Pitch >> 2) );
    //todo: Add this back after Intel version: ViewRot.Pitch   = (SWORD)ParentPawn.ViewRot.Pitch - ((SWORD)Parent.ViewRot.Pitch >> 2);
    Item.ViewRot.Roll    = 0.0;
    Item.ViewRot.Pitch   += Item.DrawPitch;

    FCoords Coords = GMath.CameraViewCoords;
    Coords.DeTransformByRotation(ParentPawn.ViewRot);

    Item.Location       = ParentPawn.Location;
    Item.Location.Z    += ParentPawn.EyeHeight + ParentPawn.Bob * 0.10;
    Item.Location      += Coords.YAxis * Item.DrawDown;
    Item.Location      += Coords.ZAxis * Item.DrawForward;

    switch (Item.InvState)
    {
        case INV_None:
        {
            break;
        }
        case INV_Active:
        {
            Item.ViewRot.Yaw += Item.DrawYaw;
            Item.ViewRot.Roll += Item.DrawRoll;
            break;
        }
        case INV_DeActivating:
        case INV_Activating:
        {
            // Activating: Pull the weapon up and away from the body.
            // Deactivating: Pull the weapon down and towards the body.
            // Do this by pretending to start from a location
            // a little below the parent location, and move towards or
            // away from the weapon's Location.
            FVector StartedAt = ParentPawn.Location;
            StartedAt.Z -= 30;
            FVector Direction = Item.Location - StartedAt;
            Direction *= float(Item.TimerCountdown)/WeaponActivationTime;
            if( Item.InvState == INV_DeActivating )
            {
                Item.Location = StartedAt + Direction;
                //ViewRot.Yaw   += Era * 4000 / WeaponActivationTime;
                //ViewRot.Pitch -= Era * 500 / WeaponActivationTime;
            }
            else
            {
                Item.Location -= Direction;
                Item.ViewRot.Yaw   += Item.TimerCountdown * 6000 / WeaponActivationTime;
                //ViewRot.Pitch -= TimerCountdown * 500 / WeaponActivationTime;
            }
            break;
        }
        case INV_Using1: // Primary attack
        case INV_Using2: // Secondary attack
        {
            break;
        }
    }
}

//----------------------------------------------------------------------------
//                 Inventory processing
//----------------------------------------------------------------------------
int AInventory::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    if(bInPickupState)
    {
        switch (Message.Index) // Item is sitting waiting to be picked up
        {
            //----------------------------------------------------------------
            //                 Time passes (as pickup)
            //----------------------------------------------------------------
            case ACTOR_Tick:
            case ACTOR_PlayerTick:
            {
                AnimSeq      = 0;
                MeshMap      = PickupMesh;
                DrawScale    = PickupScale;
                DrawRot.Yaw += 0x200;
                if( Actor.bGravity )
                {
                    IModel & Model = GLevel->ModelInfo;
                    INDEX iActorHit;
                    FVector Floor;
                    if ( Model.ZCollision(&Location,&Floor,&iActorHit) != MAXWORD )
                    {
                        const FLOAT dZ = Location.Z - Actor.CollisionHeight - Floor.Z;
                        const BOOL IsOnSurface = dZ<=2; //tbi: 2
                        if( IsOnSurface )
                        {
                            Velocity = GMath.ZeroVector;
                            bGravity = FALSE;
                            Location.Z = Floor.Z + Actor.CollisionHeight;
                        }
                        else
                        {
                            Velocity += Level->GetZoneGravityAcceleration(iMe);
                            if (Velocity.Z < -20 )
                            {
                                Velocity.Z = -20;
                            }
                            Level->MoveActor(iMe,&Velocity);
                        }
                    }
                    else
                    {
                        bGravity = FALSE;
                    }
                }
                return ProcessParent;
                break;
            }
            //----------------------------------------------------------------
            //                 Pickup item is touched
            //----------------------------------------------------------------
            case ACTOR_Touch:
            {
                PTouch & TouchInfo = *(PTouch *)Params;
                FActor & TouchingActor = FActor::Actor(TouchInfo.iActor);

                if( TouchingActor.Class->IsKindOf(GClasses.Pawn) )
                {
                    APawn & Toucher = TouchingActor.Pawn();
                    PPickupQuery PickupInfo;
                    PickupInfo.iActor = iMe;
                    INDEX iInvActor = Toucher.iInventory;
                    while(iInvActor != INDEX_NONE)
                    {
                        if (Level->SendMessage(iInvActor,ACTOR_PreemptPickupQuery,&PickupInfo)==1)
                        {
                            return ProcessDone;
                        }
                        iInvActor = FActor::Actor(iInvActor).iInventory;
                    }
                    if (Level->SendMessage(TouchInfo.iActor,ACTOR_PickupQuery,&PickupInfo)!=1)
                    {
                        // Toucher did not want the pickup.
                    }
                    else
                    {
                        // Toucher agreed to accept the pickup.
                        // Spawn an actor of this type and add it to the toucher's inventory.
                        FActor::Actor(*this).MakeSound( PickupSound );
                        PActor AddInfo;
                        AddInfo.iActor = Level->SpawnActor(Class,Name,&Location);
                        FActor & NewActor = FActor::Actor(AddInfo.iActor);
                        //todo: This doesn't copy the attributes of the pickup, since
                        // the attributes of the class (not the pickup) are used.
                        AInventory & NewInventory = NewActor.Inventory();
                        NewInventory.bInPickupState = 0     ;
                        NewInventory.bHidden        = 1     ;
                        NewActor.SetActorCollision(FALSE)   ;
                        NewInventory.bCollideWorld  = 0     ;
                        NewInventory.bAnimate       = TRUE  ;
                        NewInventory.AnimSeq        = 0     ;
                        NewInventory.MeshMap        = NewInventory.PlayerViewMesh;
                        NewInventory.DrawScale      = NewInventory.PlayerViewScale;
                        NewActor.TriggerEvent(); // Trigger any event associated with picking up this item.
                        if (Level->SendMessage(TouchInfo.iActor,ACTOR_AddInventory,&AddInfo)==1)
                        {
                            Level->SendMessage(TouchInfo.iActor,ACTOR_PickupNotify,&AddInfo);
                        }
                        if( RespawnTime == 0 ) //tba: Test for bRespawnNetOnly
                        {
                            // The item is never respawned - destroy it.
                            Level->DestroyActor(iMe);
                        }
                        else
                        {
                            // The item will respawn itself.
                            Actor.StartTimer(RespawnTime);
                            bHidden         = TRUE          ;
                            Actor.SetActorCollision(FALSE)  ;
                        }
                    }
                }
                else if( TouchingActor.IsA( GClasses.Projectile ) && bTakesDamage )
                {
                    //todo: Duplicate respawning logic above.
                    FActor * Effect = Actor.Spawn( EffectWhenDestroyed, this->Location, 0, GMath.ZeroVector );
                    Level->DestroyActor(iMe);
                }
                return ProcessDone;
            }
            //----------------------------------------------------------------
            //                 Timer expired
            //----------------------------------------------------------------
            case ACTOR_Timer:
            {
                // The timer is used to respawn:
                bHidden         = FALSE     ;
                Actor.SetActorCollision(TRUE);
                FActor::Actor(*this).MakeSound( RespawnSound );
                return ProcessDone;
            }
        }
    }
    else if (iParent!=INDEX_NONE)  // Item is in a player's inventory?
    {
        switch (Message.Index)
        {
            //----------------------------------------------------------------
            //                 Time passes (in inventory)
            //----------------------------------------------------------------
            case ACTOR_Tick:
            {
                // Synchronize the item with its owner:
                FActor & Parent = Actor.Parent()    ;
                APawn & ParentPawn  = Parent.Pawn();
                Location        = Parent.Location   ;
                DrawRot         = Parent.DrawRot    ;

                if( Parent.iWeapon == iMe )
                { 
                    const FLOAT ParentSpeed = ParentPawn.Velocity.Size2D(); //todo: Efficiency
                    if( InvState == INV_Active && ParentSpeed >= 0.2 && GPreferences.WeaponsSway )
                    {
                        static int MaxYawFactor = 150;
                        static int MaxRollFactor = 300;
                        static int AccelerationRatio = 16;
                        if( GCheat->Adjustment==FCheat::AdjustWeaponMotion )
                        {
                            FLOAT Values[3];
                            Values[0] = MaxYawFactor;
                            Values[1] = MaxRollFactor;
                            Values[2] = AccelerationRatio;
                            GCheat->DoAdjustments(Parent.iMe, Values );
                            MaxYawFactor            = Values[0];
                            MaxRollFactor           = Values[1];
                            AccelerationRatio       = Values[2];
                        }
                        WORD MaxYaw = ParentSpeed/ParentPawn.NormalSpeed * MaxYawFactor;
                        WORD MaxRoll = ParentSpeed/ParentPawn.NormalSpeed * MaxRollFactor;
                        YawSpeed = YawSpeed >= 0 ? +MaxYaw/8 : -MaxYaw/AccelerationRatio;
                        RollSpeed = RollSpeed >= 0 ? +MaxRoll/8 : -MaxRoll/AccelerationRatio;
                        DrawYaw += YawSpeed;
                        DrawRoll += RollSpeed;
                        if( DrawYaw > MaxYaw )
                        {
                            DrawYaw = MaxYaw;
                            if( YawSpeed > 0 ) { YawSpeed = -YawSpeed; }
                        }
                        else if( DrawYaw < -MaxYaw )
                        {
                            DrawYaw = -MaxYaw;
                            if( YawSpeed < 0 ) { YawSpeed = -YawSpeed; }
                        }
                        if( DrawRoll > MaxRoll )
                        {
                            DrawRoll = MaxRoll;
                            if( RollSpeed > 0 ) { RollSpeed = -RollSpeed; }
                        }
                        else if( DrawRoll < -MaxRoll )
                        {
                            DrawRoll = -MaxRoll;
                            if( RollSpeed < 0 ) { RollSpeed = -RollSpeed; }
                        }
                    }
                    else
                    {
                        DrawRoll = 0;
                        DrawYaw  = 0;
                    }
                }
                    #if 0 //tbd: obsolete 
                    ViewRot.Yaw     = Parent.ViewRot.Yaw;
                    ViewRot.Pitch   = (SWORD)Parent.ViewRot.Pitch - ( (SWORD)Parent.ViewRot.Pitch > 0 ? 0 : ((SWORD)Parent.ViewRot.Pitch >> 2) );
                    //todo: Add this back after Intel version: ViewRot.Pitch   = (SWORD)Parent.ViewRot.Pitch - ((SWORD)Parent.ViewRot.Pitch >> 2);
                    ViewRot.Roll    = 0.0;
                    ViewRot.Pitch   += DrawPitch;

                    FCoords Coords = GMath.CameraViewCoords;
                    Coords.DeTransformByRotation(ParentPawn.ViewRot);

                    Location       = ParentPawn.Location;
                    Location.Z    += ParentPawn.EyeHeight + ParentPawn.Bob * 0.10;
                    Location      += Coords.YAxis * DrawDown;
                    Location      += Coords.ZAxis * DrawForward;

                    switch (InvState)
                    {
                        case INV_None:
                        {
                            break;
                        }
                        case INV_Active:
                        {
                            const FLOAT ParentSpeed = ParentPawn.Velocity.Size2D();
                            static int MaxYawFactor = 150;
                            static int MaxRollFactor = 300;
                            static int AccelerationRatio = 16;
                            if( GCheat->Adjustment==FCheat::AdjustWeaponMotion )
                            {
                                FLOAT Values[3];
                                Values[0] = MaxYawFactor;
                                Values[1] = MaxRollFactor;
                                Values[2] = AccelerationRatio;
                                GCheat->DoAdjustments(Parent.iMe, Values );
                                MaxYawFactor            = Values[0];
                                MaxRollFactor           = Values[1];
                                AccelerationRatio       = Values[2];
                            }
                            if( ParentSpeed >= 0.2 && GPreferences.WeaponsSway )
                            {
                                WORD MaxYaw = ParentSpeed/ParentPawn.NormalSpeed * MaxYawFactor;
                                WORD MaxRoll = ParentSpeed/ParentPawn.NormalSpeed * MaxRollFactor;
                                YawSpeed = YawSpeed >= 0 ? +MaxYaw/8 : -MaxYaw/AccelerationRatio;
                                RollSpeed = RollSpeed >= 0 ? +MaxRoll/8 : -MaxRoll/AccelerationRatio;
                                DrawYaw += YawSpeed;
                                DrawRoll += RollSpeed;
                                if( DrawYaw > MaxYaw )
                                {
                                    DrawYaw = MaxYaw;
                                    if( YawSpeed > 0 ) { YawSpeed = -YawSpeed; }
                                }
                                else if( DrawYaw < -MaxYaw )
                                {
                                    DrawYaw = -MaxYaw;
                                    if( YawSpeed < 0 ) { YawSpeed = -YawSpeed; }
                                }
                                ViewRot.Yaw += DrawYaw;
                                if( DrawRoll > MaxRoll )
                                {
                                    DrawRoll = MaxRoll;
                                    if( RollSpeed > 0 ) { RollSpeed = -RollSpeed; }
                                }
                                else if( DrawRoll < -MaxRoll )
                                {
                                    DrawRoll = -MaxRoll;
                                    if( RollSpeed < 0 ) { RollSpeed = -RollSpeed; }
                                }
                                ViewRot.Roll += DrawRoll;
                            }
                            else
                            {
                                DrawRoll = 0;
                                DrawYaw  = 0;
                            }
                            break;
                        }
                        case INV_DeActivating:
                        case INV_Activating:
                        {
                            // Activating: Pull the weapon up and away from the body.
                            // Deactivating: Pull the weapon down and towards the body.
                            // Do this by pretending to start from a location
                            // a little below the parent location, and move towards or
                            // away from the weapon's Location.
                            FVector StartedAt = ParentPawn.Location;
                            StartedAt.Z -= 30;
                            FVector Direction = Location - StartedAt;
                            Direction *= float(TimerCountdown)/WeaponActivationTime;
                            if( InvState == INV_DeActivating )
                            {
                                Location = StartedAt + Direction;
                                //ViewRot.Yaw   += Era * 4000 / WeaponActivationTime;
                                //ViewRot.Pitch -= Era * 500 / WeaponActivationTime;
                            }
                            else
                            {
                                Location -= Direction;
                                ViewRot.Yaw   += TimerCountdown * 6000 / WeaponActivationTime;
                                //ViewRot.Pitch -= TimerCountdown * 500 / WeaponActivationTime;
                            }
                            break;
                        }
                        case INV_Using1: // Primary attack
                        case INV_Using2: // Secondary attack
                                break;
                    }
                    #endif
                return ProcessParent;
                break;
            }
            //----------------------------------------------------------------
            //                 Calculate view
            //----------------------------------------------------------------
            case ACTOR_InventoryCalcView: // Compute view relative to parent
            {
                FActor & Parent = Actor.Parent();
                SynchronizeInventoryItem( *this, Parent.Pawn() );
                return ProcessDone;
            }
            //----------------------------------------------------------------
            //             Pickup preemption query
            //----------------------------------------------------------------
            case ACTOR_PreemptPickupQuery:
            {
                const PPickupQuery & PickupInfo = *(PPickupQuery *)Params;
                FActor & Pickup = FActor::Actor(PickupInfo.iActor);
                BOOL PickItUp = TRUE;
                if( Pickup.Class == Class )
                {
                    // Prevent the owner from picking up an item he already has,
                    // unless there is ammo that can be used by the parent.
                    PickItUp = FALSE;
                    if( Actor.IsA(GClasses.Weapon) && Actor.HasParent() )
                    {
                        AWeapon & Weapon = Actor.Weapon();
                        FActor & Parent = Actor.Parent();
                        if( Parent.CanUseAmmo( EAmmoType(Weapon.AmmoType), Weapon.PickupAmmoCount, FALSE ) )
                        {
                            PickItUp = TRUE;
                        }
                    }
                }
                return PickItUp ? -1 : 1;
                break;
            }
            //----------------------------------------------------------------
            //                  Activation
            //----------------------------------------------------------------
            case ACTOR_Activate:
            {
                // Start sequence of raising the weapon up.
                bActiveInSet    = TRUE              ;
                InvState        = INV_Activating    ;
                Era             = 0                 ;
                Actor.StartTimer( WeaponActivationTime, ACTOR_Activated );
                Actor.UseFrame( 1, 1 );
                APawn & Parent = Actor.Parent().Pawn();
                if( Parent.iRecentWeapons[0] != iMe )
                {
                    Parent.iRecentWeapons[1] = Parent.iRecentWeapons[0];
                    Parent.iRecentWeapons[0] = iMe;
                }
                Parent.bStatusChanged = TRUE;

                // Initially locate the weapon out of sight. Later, ACTOR_Tick processing
                // will place the weapon in view more intelligently.
                Location         = Parent.Location;
                Location.Z      -= Parent.CollisionHeight;
                ViewRot.Yaw      = 0;
                ViewRot.Pitch    = 0;
                ViewRot.Roll     = 0.0;

                return ProcessDone;
            }
            //----------------------------------------------------------------
            //                 Deactivation
            //----------------------------------------------------------------
            case ACTOR_DeActivate:
            {
                // Start sequence of lowering the weapon.
                InvState = INV_DeActivating;
                Actor.StartTimer( WeaponActivationTime, ACTOR_Deactivated );
                Actor.UseFrame( 1, 1 );
                // Prepare to switch to another item, if requested.
                PActor & Info = *(PActor*)Params;
                iNextActive = Info.iActor;
                if( iNextActive != INDEX_NONE )
                {
                    // Request to switch to another item.
                    AInventory & NextActive = FActor::Inventory(iNextActive);
                    if (NextActive.Class->IsKindOf(GClasses.Inventory))
                    {
                        if (OwningSet==NextActive.OwningSet)
                        {
                            // These two weapons are in the same set. Make sure the next weapon is tagged
                            // as the active weapon for this key.
                            bActiveInSet = FALSE;
                        }
                    }
                }
                return ProcessDone;
            }
            //----------------------------------------------------------------
            //                 End of animation
            //----------------------------------------------------------------
            // This marks the end of some action sequence, such as raising the
            // weapon (activating it), firing the weapon, reloading the weapon,
            // and so on.
            case ACTOR_Activated:
            case ACTOR_Deactivated:
            case ACTOR_EndAnimation:
            {
                APawn & Parent = Actor.Parent().Pawn();
                AnimRate    = 0 ;
                bHidden = TRUE;
                switch (InvState)
                {
                    case INV_None:
                    {
                        break;
                    }
                    case INV_Active:
                    case INV_Activating:
                    case INV_Using1:
                    case INV_Using2:
                    case INV_UsingCloseUp:
                    case INV_Playing:
                    case INV_Reloading:
                    {
                        const EInvState OldState = EInvState(InvState);
                        AWeapon & Weapon = Actor.Weapon(); // We assume all inventory items are weapons.
                        LightType = LT_None; // Turn off lighting caused by firing.
                        Actor.UseFrame( 1, 1 ); //tbi? Use Property
                        InvState    = INV_Active;
                        if( bNeedsReloading )
                        {
                            Actor.Send_Reload();
                        }
                        if( bNeedsReloading && OldState != INV_Activating && GPreferences.SwitchFromEmptyWeapon )
                        {
                            FActor::Send_ChooseWeapon(iParent);
                        }
                        else if( OldState == INV_Using1 && !Weapon.bWasReleased )
                        {
                            Actor.Send_Use();
                        }
                        else if( OldState == INV_Using2 && !Weapon.bWasReleased )
                        {
                            Actor.Send_UseExtra();
                        }
                        break;
                    }
                    case INV_DeActivating:
                    {
                        Parent.iWeapon = iNextActive;
                        Parent.bStatusChanged = TRUE;
                        if(iNextActive!=INDEX_NONE)
                        {
                            FActor::Send_Activate(iNextActive);
                        }
                        InvState = INV_None;
                    }
                    break;
                }
                return ProcessDone;
            }
        }
    }
    return ProcessParent;
    UNGUARD("AInventory::Process");
}

//----------------------------------------------------------------------------
//                 Health processing
//----------------------------------------------------------------------------
int AHealth::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "AHealth::Process" );
}

//----------------------------------------------------------------------------
//                 Ammo processing
//----------------------------------------------------------------------------
int AAmmo::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "AAmmo::Process" );
}

//----------------------------------------------------------------------------
//                 Armor processing
//----------------------------------------------------------------------------
int AArmor::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "AArmor::Process" );
}

//----------------------------------------------------------------------------
//                 Pickup processing
//----------------------------------------------------------------------------
int APickup::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "APickup::Process" );
}

//----------------------------------------------------------------------------
//                 Powerup processing
//----------------------------------------------------------------------------
int APowerUp::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "APowerUp::Process" );
}

//----------------------------------------------------------------------------
//                 Clip processing
//----------------------------------------------------------------------------
int AClip::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "AClip::Process" );
}

//----------------------------------------------------------------------------
//                 Shells processing
//----------------------------------------------------------------------------
int AShells::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "AShells::Process" );
}


//----------------------------------------------------------------------------
//                 Stinger ammo processing
//----------------------------------------------------------------------------
int AStingerAmmo::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD( "AStingerAmmo::Process" );
}

//----------------------------------------------------------------------------
//                 The End
//----------------------------------------------------------------------------
