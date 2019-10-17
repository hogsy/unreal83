/*=============================================================================
    UnInv.cpp: Inventory class actor code

    Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
    Compiled with Visual C++ 4.0.

    Revision history:
        * Created by Tim Sweeney
        * 05/--/96 Modified by Mark, many changes
=============================================================================*/

#include "UnGame.h"
#include "UnFActor.h"
#include "UnRandom.h"

//----------------------------------------------------------------------------
//                   Weapon processing
//----------------------------------------------------------------------------
int AWeapon::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    // If you need to, check bInPickupState to determine if the weapon
    // is lying around waiting to be picked up (bInPickupState==TRUE).
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                     Time passes...
        //--------------------------------------------------------------------
        case ACTOR_Tick:
        case ACTOR_PlayerTick:
        {
            if( !bInPickupState && !bWasReleased )
            {
                UseTime++;
            }
            return ProcessParent;
            break;
        }
        //--------------------------------------------------------------------
        //                     Release firing mechanism
        //--------------------------------------------------------------------
        case ACTOR_Release:
        {
            bWasReleased = TRUE;
            return ProcessParent;
            break;
        }
        #if 0 //tbd or //tba, depending on how close-up attacks are handled
        //--------------------------------------------------------------------
        //                   Use close-up attack
        //--------------------------------------------------------------------
        case ACTOR_UseCloseUp:
        {
            if( InvState==INV_Active )
            {   
                PActor & Info = PActor::Convert(Params);
                InvState = INV_UsingCloseUp;
                Era = 0;
                actorAnimate( *this, 1, 1, 0.05, ACTOR_EndAnimation );  //tbi: sequence 1
                
            }
            return ProcessDone;
            break;
        }
        #endif
        //--------------------------------------------------------------------
        //                     Fire weapon
        //--------------------------------------------------------------------
        case ACTOR_Use:
        case ACTOR_UseExtra:
        {
            LastUseTime = GServer.Ticks;
            bWasReleased = FALSE;
            // Light up the weapon:
            //todo: Should we use properties?
            LightType = LT_Steady;
            LightBrightness = 150    ;
            LightHue        = 40    ;
            LightSaturation = 192   ;
            LightRadius     = 30    ;
            LightPeriod     = 16    ;

            // Remove any yaw/roll used to animate weapon(?)
            {
                ViewRot.Yaw     += DrawYaw;
                ViewRot.Roll    += DrawRoll;
                DrawYaw         = 0 ;
                DrawRoll        = 0 ;
                YawSpeed        = 0 ;
                RollSpeed       = 0 ;
            }
            #if 0 //tba: Moved to unfactor
            const int Usage = Message.Index==ACTOR_Use ? 0 : 1;
            if (InvState==INV_Active && ( ReusePeriod[Usage]==0 || (GServer.Ticks-LastUseTime)>=ReusePeriod[Usage] ) )
            {
                bWasReleased = FALSE;
                PUse & UseInfo = PUse::Convert(Params);
                APawn & Parent = FActor::Pawn(iParent);
                UseInfo.Count = 0; // Will be set to number of discharges.
                FVector RecoilVelocity;
                {
                    FCoords Coords;
                    ((AActor*)this)->GetViewCoords(&Coords); //tbi: Conversion
                    RecoilVelocity = Coords.ZAxis * (-RecoilForce[Usage] / Parent.Mass);
                }
                USound *    DischargeSound  = DischargeSounds[Usage]    ;
                UClass *    ProjectileClass = this->ProjectileClass[Usage]    ;
                const BOOL  RepeatSound     = bRepeatSounds[Usage]      ;
                const FLOAT Noise           = this->Noise[Usage]        ;
                int         DischargeLimit  = Discharges[Usage]         ;
                const int   AmmoPerDischarge= AmmoUsed[Usage]           ;
                int         AmmoAvailable   = Parent.AmmoCount[AmmoType];
                const int   RecoilPitch     = Degrees(this->RecoilPitch[Usage])/2;
                // AmmoHeld is the ammo we consider to be held by the weapon, although
                // for standard ammo it is really held by the weapon holder.
                int AmmoHeld = 
                    ReloadCount==0 ? AmmoAvailable 
                :   (AmmoAvailable-1)%ReloadCount + 1
                ;
                if( AmmoType == AmmoType_None || ProjectileClass == 0 )
                {
                    appError( "Weapon has no ammo type or no projectile class." );
                }
                BOOL SoundPlayed = FALSE;
                const AProjectile & DefaultProjectile = FActor::Actor( ProjectileClass->DefaultActor ).Projectile();
                for
                (
                    int WhichShot=1; 
                    AmmoAvailable >= AmmoPerDischarge 
                    &&  AmmoHeld >= AmmoPerDischarge 
                    &&  WhichShot<=DischargeLimit
                    ;
                    WhichShot++
                )
                {
                    LastUseTime = GServer.Ticks;
                    InvState = Usage==0 ? INV_Using1 : INV_Using2;
                    AmmoAvailable -= AmmoPerDischarge;
                    AmmoHeld -= AmmoPerDischarge;
                    UseInfo.Count++;
                    Parent.ViewRot.Pitch += RecoilPitch;
                    Parent.Noise += Noise;
                    Parent.Velocity += RecoilVelocity;
                    if( !SoundPlayed || RepeatSound )
                    { 
                        Actor.MakeSound(DischargeSound);
                        SoundPlayed = TRUE;
                    }
                    if( WhichShot==1 )
                    {
                        FActor::Spawn(MuzzleEffectClass[Usage],Location);
                    }
                    if( DefaultProjectile.bIsInstantHit )
                    {
                        // Need to build this into a function (get view coords, get draw coords):
                        // Temporarily, remove DrawPitch from the view pitch...
                        ViewRot.Pitch -= DrawPitch;
                        FCoords Coords; ((AActor *)this)->GetViewCoords(&Coords);
                        ViewRot.Pitch += DrawPitch;
                        FVector Direction = Coords.ZAxis;
                        PerturbNormalVector(Direction,Coords,Dispersion[Usage]);
                        FVector HitLocation;
                        FVector HitNormal;
                        INDEX   iHitNode;
                        Level->RayHit
                        (
                            iMe,&Location,&Direction,
                            DefaultProjectile.Mass*DefaultProjectile.Speed,0.0, 
                            0,TRUE,
                            &HitLocation,&HitNormal,&iHitNode
                        );
                    }
                    else
                    {
                        GGame.SpawnForwardProjectile( iMe, ProjectileClass, ProjStartDist );
                    }
                }
                bNeedsReloading = AmmoHeld < AmmoUsed[0]*int(Discharges[0]);
                Parent.AmmoCount[AmmoType] = AmmoAvailable;
                Parent.bStatusChanged = TRUE;
            }
            #endif
            return ProcessParent;
            break;
        }
        //--------------------------------------------------------------------
        //                     Reload weapon
        //--------------------------------------------------------------------
        // On output, bNeedsReloading (bnr) and InvState tell you what the
        // status of the reload operation is:
        //       bnr   InvState        Meaning 
        //      -----  --------------- ------------------------
        //   A. FALSE  INV_Reloading   Weapon is proceeding with reload
        //   B. FALSE  !INV_Reloading  Weapon didn't need to be reloaded
        //   C. TRUE                   Weapon needs reloading, but there is not enough ammo.
        case ACTOR_Reload:
        {
            APawn & Parent = FActor::Pawn(iParent);
            //tbi: Make a weapon function to determine if weapon has enough ammo to be useful.
            int         AmmoAvailable   = Parent.AmmoCount[AmmoType];
            const BOOL  HasEnoughAmmo   = AmmoAvailable >= int(AmmoUsed[0]) * int(Discharges[0]);
            bNeedsReloading = bNeedsReloading || !HasEnoughAmmo;
            if( bNeedsReloading )
            {
                if( HasEnoughAmmo )
                {
                    Actor.MakeSound(ReloadSound);
                    Parent.Noise += 2.0; //tbi: Hard-coded noise.
                    bNeedsReloading = FALSE;
                    InvState = INV_Reloading;
                }
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                 Instant-hit weapon struck an actor
        //--------------------------------------------------------------------
        case ACTOR_HitNotify: // Instant-hit weapon struck an actor.
        {
            //tbi: Can we find a better way of conveying projectile info...
            UClass * ProjectileClass = this->ProjectileClass[InvState==INV_Using2 ? 1 : 0]    ;
            const AProjectile & DefaultProjectile = FActor::Actor( ProjectileClass->DefaultActor ).Projectile();
            const PHitNotify & Info = PHitNotify::Convert(Params);
            FActor & VictimActor = FActor::Actor(Info.iHitActor);
            APawn & Parent = FActor::Pawn(iParent); 
            // Send ACTOR_Touch to the hit actor, in case it is interested...
            {
                FActor * Projectile = FActor::Spawn(DefaultProjectile.Class,Info.HitLocation);
                if( Projectile != 0 )
                {
                    VictimActor.Send_Touch(Projectile->iMe);
                    GLevel->DestroyActor(Projectile->iMe);
                }
            }
            if( VictimActor.IsA(GClasses.Pawn) )
            {
                APawn & Victim = VictimActor.Pawn();
                const FVector & HitLocation = Info.HitLocation;
                PHit HitInfo;
                HitInfo.Empty();
                HitInfo.iSourceActor    = iParent       ;
                HitInfo.iSourceWeapon   = iMe        ;
                HitInfo.HitLocation     = HitLocation   ;
                //tbc? The hit location is currently just the hit pawn's location.
                // Let's move that location a little towards the source of the hit,
                // so that any drawn effects don't appear inside the hit pawn.
                const FVector Direction = HitLocation-Location;
                const FLOAT Distance = Direction.Size();
                HitInfo.HitLocation -= Direction/Distance*20.0; // 20 units away from hit pawn (towards source of harm).
                for( int Which = 0; Which < DMT_Count; Which++ )
                {
                    FLOAT DamageAmount = DefaultProjectile.Damage[Which] - DefaultProjectile.DamageDecay[Which]*Distance;
                    if( DamageAmount < 0 ) { DamageAmount = 0.0; }
                    HitInfo.Damage[Which] = DamageAmount;
                }
                FActor::Spawn(DefaultProjectile.EffectOnPawnImpact != 0 ? DefaultProjectile.EffectOnPawnImpact : DefaultProjectile.EffectOnImpact,HitInfo.HitLocation);
                FActor::Send_Hit(Info.iHitActor,HitInfo);
                // Handle any explosive charge imparted to target:
                if( DefaultProjectile.ExplosiveTransfer != 0 )
                {
                    //tbi: This is duplicated in unweapon.cpp!
                    VictimActor.AddExplosiveCharge(DefaultProjectile.ExplosiveTransfer);
                }
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //              Instant-hit weapon struck a wall
        //--------------------------------------------------------------------
        case ACTOR_WallNotify:
        {
            //tbi: Can we find a better way of conveying projectile info...
            UClass * ProjectileClass = this->ProjectileClass[InvState==INV_Using2 ? 1 : 0]    ;
            const AProjectile & DefaultProjectile = FActor::Actor( ProjectileClass->DefaultActor ).Projectile();
            const PWallNotify & Info = *(PWallNotify*)Params; //tbi: Conversion
            const FVector & HitLocation = Info.WallLocation;
            FActor::Spawn(DefaultProjectile.EffectOnWallImpact != 0 ? DefaultProjectile.EffectOnWallImpact : DefaultProjectile.EffectOnImpact,HitLocation);
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                     Activation/deactivation
        //--------------------------------------------------------------------
        case ACTOR_Activate:
        case ACTOR_DeActivate:
        {
            UseTime = 0;
            bWasReleased = TRUE;
            return ProcessParent;
            break;
        }
    }
    return ProcessParent;
    UNGUARD( "AWeapon::Process" );
}

//----------------------------------------------------------------------------
//                   AutoMag processing
//----------------------------------------------------------------------------
int AAutoMag::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    if( bInPickupState )
    {
    }
    else
    {
        switch (Message.Index)
        {   
        //--------------------------------------------------------------------
        //                     Use the weapon
        //--------------------------------------------------------------------
        //todo: Unify all the ACTOR_Use and ACTOR_UseExtra processing, both for
        // this weapon and for all weapons.
            case ACTOR_Use:
            {
                const int Usage = 0;
                if( InvState == INV_Active )
                { 
                    FActor & Parent = Actor.Parent();
                    FActor * NearbyPawn = Parent.NearbyFacedPawn(Parent.ViewRot);
                    if( NearbyPawn != 0 )
                    {
                        Actor.Send_UseCloseUp(NearbyPawn->iMe);
                        Parent.CauseDamage( *NearbyPawn, CloseUpDamage, 1600 );
                        return ProcessDone; // <=== unstructured return //todo: fix
                    } 
                    else if( ReusePeriod[Usage]==0 || (GServer.Ticks-LastUseTime) >= ReusePeriod[Usage] )
                    {
                        if( Actor.CanFire(TRUE) )
                        {
                            InvState = INV_Using1;
                            Actor.ReplaceAnimation( EAMA_Shoot, 1, 1.0, ACTOR_EndAnimation ); 
                            return ProcessParent; // <=== unstructured return //todo: fix
                        }
                    }
                }
                return ProcessDone;
                break;
            }
            case ACTOR_UseExtra:
            {
                const int Usage = 1;
                if( InvState == INV_Active )
                { 
                    FActor & Parent = Actor.Parent();
                    FActor * NearbyPawn = Parent.NearbyFacedPawn(Parent.ViewRot);
                    if( NearbyPawn != 0 )
                    {
                        Actor.Send_UseCloseUp(NearbyPawn->iMe);
                        Parent.CauseDamage( *NearbyPawn, CloseUpDamage, 1600 );
                        return ProcessDone; // <=== unstructured return //todo: fix
                    } 
                    else if( ReusePeriod[Usage]==0 || (GServer.Ticks-LastUseTime) >= ReusePeriod[Usage] )
                    {
                        if( Actor.CanFire(FALSE) )
                        {
                            InvState = INV_Using2;
                            Actor.ReplaceAnimation( EAMA_Shoot2, 1, 1.0, ACTOR_EndAnimation ); 
                            return ProcessParent; // <=== unstructured return //todo: fix
                        }
                    }
                }
                return ProcessDone;
                break;
            }
                #if 0 //tbd: obsolete
                const int Usage = Message.Index==ACTOR_Use ? 0 : 1;
                PUse & UseInfo = *(PUse *)Params; //tbi: conversion
                if (InvState==INV_Active)
                {
                    AWeapon::Process(Level,Message,&UseInfo); //tbc!
                    if( UseInfo.Count > 0 )
                    {
                        // The weapon was used, select the appropriate animation.
                        if( Usage==0 ) // Normal use
                        {
                            Actor.Animate( EAMA_Shoot, 1, 1.0, ACTOR_EndAnimation ); 
                        }
                        else // Secondary use
                        {
                            Actor.Animate( EAMA_Shoot2, 1, 1.0, ACTOR_EndAnimation ); 
                        }
                    }
                }
                    return ProcessDone;
                break;
            }
                #endif
            //--------------------------------------------------------------------
            //                   Use close-up attack
            //--------------------------------------------------------------------
            case ACTOR_UseCloseUp:
            {
                if( InvState==INV_Active )
                {
                    InvState = INV_UsingCloseUp;
                    //tbc: When close-up animations are availabe
                    Actor.ReplaceAnimation( EAMA_Whip, 1, 1, ACTOR_EndAnimation ); 
                }
                return ProcessDone;
                break;
            }
            //----------------------------------------------------------------
            //                     Reload the weapon
            //----------------------------------------------------------------
            case ACTOR_Reload:
            {
                if( InvState == INV_Active )
                {
                    AWeapon::Process(Level,Message,Params); //tbi
                    if( InvState == INV_Reloading )
                    {
                        Actor.ReplaceAnimation( EAMA_Twirl, 1, 1.0, ACTOR_EndAnimation ); 
                    }
                }
                return ProcessDone;
                break;
            }
            //--------------------------------------------------------------------
            //                     Animation frame trigger.
            //--------------------------------------------------------------------
            case ACTOR_FrameTrigger:
            {
                const PFrame & Info = PFrame::Convert(Params);
                const EAutoMagAnimationTriggers Trigger = EAutoMagAnimationTriggers(Info.Trigger);
                switch( Trigger )
                {
                    case AutoMagAT_Fire1:
                    {
                        Actor.Fire( InvState == INV_Using1, 0, 0 );
                        break;
                    }
                    case AutoMagAT_Fire2:
                    {
                        Actor.Fire( InvState == INV_Using1, 0, 0 );
                        break;
                    }
                }
            }
        }
    }
    return ProcessParent;
    UNGUARD( "AAutoMag::Process" );
}

//----------------------------------------------------------------------------
//                   QuadShot processing
//----------------------------------------------------------------------------
int AQuadShot::Process(ILevel *Level, FName Message, void *Params)
    {
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
        {
        //--------------------------------------------------------------------
        //                     Use the weapon
        //--------------------------------------------------------------------
            case ACTOR_Use:
            {
                const int Usage = 0;
                if( InvState == INV_Active )
                { 
                    FActor & Parent = Actor.Parent();
                    FActor * NearbyPawn = Parent.NearbyFacedPawn(Parent.ViewRot);
                    if( NearbyPawn != 0 )
                    {
                        Actor.Send_UseCloseUp(NearbyPawn->iMe);
                        Parent.CauseDamage( *NearbyPawn, CloseUpDamage, 1600 );
                        return ProcessDone; // <=== unstructured return //todo: fix
                    } 
                    else if( ReusePeriod[Usage]==0 || (GServer.Ticks-LastUseTime) >= ReusePeriod[Usage] )
                    {
                        if( Actor.CanFire(TRUE) )
                        {
                            InvState = INV_Using1;
                            Actor.ReplaceAnimation( EQSA_Fire, 1, 0.5, ACTOR_EndAnimation );
                            return ProcessParent; // <=== unstructured return //todo: fix
                        }
                    }
                }
                return ProcessDone;
                break;
            }
            case ACTOR_UseExtra:
            {
                const int Usage = 1;
                if( InvState == INV_Active )
                { 
                    FActor & Parent = Actor.Parent();
                    FActor * NearbyPawn = Parent.NearbyFacedPawn(Parent.ViewRot);
                    if( NearbyPawn != 0 )
                    {
                        Actor.Send_UseCloseUp(NearbyPawn->iMe);
                        Parent.CauseDamage( *NearbyPawn, CloseUpDamage, 1600 );
                        return ProcessDone; // <=== unstructured return //todo: fix
                    } 
                    else if( ReusePeriod[Usage]==0 || (GServer.Ticks-LastUseTime) >= ReusePeriod[Usage] )
                    {
                        if( Actor.CanFire(FALSE) )
                        {
                            InvState = INV_Using2;
                            Actor.ReplaceAnimation( EQSA_Fire, 1, 0.5, ACTOR_EndAnimation );
                            return ProcessParent; // <=== unstructured return //todo: fix
                        }
                    }
                }
                return ProcessDone;
                break;
            }
            //--------------------------------------------------------------------
            //                   Use close-up attack
            //--------------------------------------------------------------------
            case ACTOR_UseCloseUp:
            {
                if( InvState==INV_Active )
                {
                    InvState = INV_UsingCloseUp;
                    //tbc: When close-up animations are availabe
                    Actor.ReplaceAnimation( 1, 1, 1.0, ACTOR_EndAnimation ); 
                }
                return ProcessDone;
                break;
            }
            #if 0 //tbd: obsolete
        case ACTOR_Use:
        case ACTOR_UseExtra:
        {
            PUse & UseInfo = *(PUse *)Params; //tbi: conversion
            if (InvState==INV_Active)
            {
                {
                    AWeapon::Process(Level,Message,&UseInfo); //tbc!
                    if( UseInfo.Count > 0 )
                    {
                        Actor.Animate( EQSA_Fire, 1, 0.5, ACTOR_EndAnimation );
                    }
                }
            }
            return ProcessDone;
            break;
        }
            #endif
        //--------------------------------------------------------------------
        //                     Reload the weapon
        //--------------------------------------------------------------------
        case ACTOR_Reload:
        {
            if( InvState == INV_Active )
            {
                AWeapon::Process(Level,Message,Params); //tbi
                if( InvState == INV_Reloading )
                {
                    Actor.ReplaceAnimation( EQSA_Reload, 1, 0.5, ACTOR_EndAnimation ); 
                }
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                     Animation frame trigger.
        //--------------------------------------------------------------------
        case ACTOR_FrameTrigger:
        {
            const PFrame & Info = PFrame::Convert(Params);
            const EQuadShotAnimationTriggers Trigger = EQuadShotAnimationTriggers(Info.Trigger);
            switch( Trigger )
            {
                case QuadShotAT_Fire:
                {
                    Actor.Fire( InvState == INV_Using1, 0, 0 );
                    break;
                }
            }
            break;
        }
    }
    return ProcessParent;
    UNGUARD( "AQuadShot::Process" );
    }

//----------------------------------------------------------------------------
//                   FlameGun processing
//----------------------------------------------------------------------------
int AFlameGun::Process(ILevel *Level, FName Message, void *Params)
    {
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    #if 0 //tbd: obsolete
    switch (Message.Index)
        {
        //--------------------------------------------------------------------
        //                     Use the weapon
        //--------------------------------------------------------------------
        case ACTOR_Use:
        case ACTOR_UseExtra:
        {
            PUse & UseInfo = *(PUse *)Params; //tbi: conversion
            if (InvState==INV_Active)
            {
                {
                    AWeapon::Process(Level,Message,&UseInfo); //tbc!
                    if( UseInfo.Count > 0 )
                    {
                        Actor.Animate( EFGA_Fire, 1, 0.5, ACTOR_EndAnimation ); 
                    }
                    else if( Message.Index==ACTOR_UseExtra ) // If special firing failed, revert to normal firing.
                    {
                        Level->SendMessage(iMe,ACTOR_Use,Params);
                    }
                }
            }
            return ProcessDone;
            break;
        }
        }
    #endif
    return ProcessParent;
    UNGUARD( "AFlameGun::Process" );
    }

//----------------------------------------------------------------------------
//                   Stinger processing
//----------------------------------------------------------------------------
int AStinger::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                     Use the weapon
        //--------------------------------------------------------------------
        case ACTOR_Use:
        {
            const int Usage = 0;
            if( InvState == INV_Active )
            { 
                    FActor & Parent = Actor.Parent();
                    FActor * NearbyPawn = Parent.NearbyFacedPawn(Parent.ViewRot);
                if( NearbyPawn != 0 )
                {
                        Actor.Send_UseCloseUp(NearbyPawn->iMe);
                        Parent.CauseDamage( *NearbyPawn, CloseUpDamage, 1600 );
                    return ProcessDone; // <=== unstructured return //todo: fix
                } 
                else if( ReusePeriod[Usage]==0 || (GServer.Ticks-LastUseTime) >= ReusePeriod[Usage] )
                {
                    if( Actor.CanFire(TRUE) )
                    {
                        InvState = INV_Using1;
                        Actor.ReplaceAnimation( EStingA_Fire1, 1, 1.0, ACTOR_EndAnimation ); 
                        return ProcessParent; // <=== unstructured return //todo: fix
                    }
                }
            }
            return ProcessDone;
            break;
        }
        case ACTOR_UseExtra:
        {
            const int Usage = 1;
            if( InvState == INV_Active )
            { 
                FActor & Parent = Actor.Parent();
                FActor * NearbyPawn = Parent.NearbyFacedPawn(Parent.ViewRot);
                if( NearbyPawn != 0 )
                {
                    Actor.Send_UseCloseUp(NearbyPawn->iMe);
                    Parent.CauseDamage( *NearbyPawn, CloseUpDamage, 1600 );
                    return ProcessDone; // <=== unstructured return //todo: fix
                } 
                else if ( ReusePeriod[Usage]==0 || (GServer.Ticks-LastUseTime) >= ReusePeriod[Usage] )
                {
                    if( Actor.CanFire(FALSE) )
                    {
                        InvState = INV_Using2;
                        Actor.ReplaceAnimation( EStingA_Fire3, 1, 1.0, ACTOR_EndAnimation ); 
                        // Fill up PendingShots with a list of angles at which to shoot:
                        // The stored values are 1/10 of the angle which will be used
                        // (done only because PendingShots was poorly chosen to be an 
                        // array of bytes. //todo: Fix this.
                        PendingShots[0] = 130;
                        PendingShots[1] = BYTE(-130);
                        PendingShots[2] = 0;
                        PendingShotCount = 3;
                        return ProcessParent; // <=== unstructured return //todo: fix
                    }
                }
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                   Use close-up attack
        //--------------------------------------------------------------------
        case ACTOR_UseCloseUp:
        {
            if( InvState==INV_Active )
            {
                InvState = INV_UsingCloseUp;
                //tbc: When close-up animations are availabe
                Actor.ReplaceAnimation( 1, 1, 0.5, ACTOR_EndAnimation ); 
            }
            return ProcessDone;
            break;
        }
        #if 0 //tbd: obsolete
        case ACTOR_Use:
        case ACTOR_UseExtra:
        {
            //todo: Change this once stinger animations are available:
            PUse & UseInfo = *(PUse *)Params; //tbi: conversion
            if (InvState==INV_Active)
            {
                {
                    AWeapon::Process(Level,Message,&UseInfo); //tbc!
                    if( UseInfo.Count > 0 )
                    {
                        Actor.Animate( EStingA_Fire, 1, 1.0, ACTOR_EndAnimation ); 
                    }
                    else if( Message.Index==ACTOR_UseExtra ) // If special firing failed, revert to normal firing.
                    {
                        Level->SendMessage(iMe,ACTOR_Use,Params);
                    }
                }
            }
            return ProcessDone;
            break;
        }
        #endif
        //--------------------------------------------------------------------
        //                     Animation frame trigger.
        //--------------------------------------------------------------------
        case ACTOR_FrameTrigger:
        {
            const PFrame & Info = PFrame::Convert(Params);
            const EStingerAnimationTriggers Trigger = EStingerAnimationTriggers(Info.Trigger);
            const BOOL IsPrimaryUse = InvState == INV_Using1;
            if( IsPrimaryUse )
            {
                switch( Trigger )
                {
                    case StingerAT_Fire1:
                    {
                        Actor.Fire( IsPrimaryUse, 0, 0 );
                        break;
                    }
                }
            }
            else
            {
                switch( Trigger )
                {
                    case StingerAT_Fire1:
                    case StingerAT_Fire2:
                    case StingerAT_Fire3:
                    {
                        if( PendingShotCount > 0 )
                        {
                            // Pick a shot at random:
                            const int Choice = Random(0,PendingShotCount-1);
                            signed char Value = (signed char)PendingShots[Choice]; //todo: gross misuse of sign extension follows
                            const int Yaw = int(Value) * 10;
                            Actor.Fire( IsPrimaryUse, Yaw, 0 );
                            // Move the last pending shot into the one we just used:
                            PendingShots[Choice] = PendingShots[PendingShotCount-1];
                            PendingShotCount--;
                        }
                        break;
                    }
                }
            }
        }
    }
    return ProcessParent;
    UNGUARD( "AStinger::Process" );
}

//----------------------------------------------------------------------------
//                         The End
//----------------------------------------------------------------------------
