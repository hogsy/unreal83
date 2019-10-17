/*=============================================================================
    UnPawns.cpp: Specific pawn actor code

    Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
    Compiled with Visual C++ 4.0.

    Revision history:
        * Created by Tim Sweeney
=============================================================================*/

#include "UnGame.h"

#include "UnFActor.h"
#include "UnRandom.h"

//tbm: Convert degrees to WORD angle measurements.
static inline int Degrees(int Degrees)
{
    return Degrees*65536/360;
}

//----------------------------------------------------------------------------
//                    Skaarj Pawn processing
//----------------------------------------------------------------------------
int ASkaarj::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                     Animation request.
        //--------------------------------------------------------------------
        case ACTOR_Animate:
        {
            const PAnimate & Info = PAnimate::Convert(Params);
            switch( Info.Kind )
            {
                case PAnimate::DeathAnimation:
                {
                    Actor.AddAnimation( SkaarjA_Death, 1 );
                    break;
                }
                case PAnimate::StillAnimation:
                {
                    Actor.UseFrame( SkaarjA_Squat, 1 );
                    break;
                }
                case PAnimate::IdleAnimation:
                {
                    Actor.AddAnimation( SkaarjA_Squat, 1 );
                    break;
                }
                case PAnimate::RunAnimation:
                case PAnimate::MoveAnimation:
                {
                    //tbi: Assume we are in the fighter position.
                    Actor.AddAnimation( SkaarjA_T6   , 1 );
                    Actor.AddAnimation( SkaarjA_Jog      );
                    Actor.AddAnimation( SkaarjA_T5   , 1 );
                    break;
                }
                case PAnimate::HitAnimation:
                {
                    Actor.AddAnimation( SkaarjA_Death, 1 );
                    break;
                }
                case PAnimate::SearchAnimation:
                {
                    //tbi: Assume we are in the fighter position.
                    Actor.AddAnimation( SkaarjA_T2, 1 );
                    Actor.AddAnimation( SkaarjA_Squat, 1 );
                    Actor.AddAnimation( SkaarjA_HeadUp, 1 );
                    Actor.AddAnimation( SkaarjA_Looking );
                    break;
                }
                case PAnimate::DistantStillAttackAnimation  :
                {
                    Actor.AddAnimation( SkaarjA_Firing );
                    break;
                }
                case PAnimate::CloseUpAttackAnimation     :
                {
                    if( bTargetIsNear )
                    {
                        Actor.AddAnimation( Random(SkaarjA_TwoClaw,SkaarjA_Spin) );
                    }
                    break;
                }
                case PAnimate::DistantMovingAttackAnimation    :
                {
                    Actor.AddAnimation( SkaarjA_Lunge );
                    break;
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
            const ESkaarjAnimationTriggers Trigger = ESkaarjAnimationTriggers(Info.Trigger);
            switch( Trigger )
            {
                case SkaarjAT_Lunge:
                {
                    if( Actor.CanSeeTarget() && bTargetIsNear )
                    {
                        Actor.MakeSound( LungeSound );
                        Actor.CauseDamage(Actor.Target(),LungeDamage,1000);
                    }
                    return ProcessDone;
                    break;
                }
                case SkaarjAT_Spin:
                {
                    //Debug( "Skaarj: Spin-slice!" );
                    if( bTargetIsNear )
                    {
                        Actor.MakeSound( SpinSound );
                        Actor.CauseDamage(Actor.Target(),SpinDamage,1000);
                    }
                    return ProcessDone;
                    break;
                }
                case SkaarjAT_Fire:
                {
                    //Debug( "Skaarj: Bang!" );
                    if( Actor.CanSeeTarget() )
                    {
                        Actor.SpawnPyrotechnic( AttackEffects[0], 30.0, 20.0,  40.0 );
                        Actor.SpawnPyrotechnic( AttackEffects[0], 30.0, 20.0, -40.0 );
                        Actor.MakeSound( ShootSound );
                        Actor.CauseRayDamage(Actor.Target(),ShootDamage,100.0);
                        return ProcessDone;
                    }
                    break;
                }
                case SkaarjAT_ClawLeft:
                case SkaarjAT_ClawRight:
                {
                    //Debug( "Skaarj: Slash!" );
                    if( bTargetIsNear )
                    {
                        Actor.MakeSound( ClawSound );
                        Actor.CauseDamage(Actor.Target(),ClawDamage,1000);
                    }
                    return ProcessDone;
                    break;
                }
            }
            return ProcessDone;
            break;
        }
    }
    return ProcessParent;
    UNGUARD("ASkaarj::Process");
};

//----------------------------------------------------------------------------
//                    BigMan Pawn processing
//----------------------------------------------------------------------------
int ABigMan::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    //DeathSpawn = new("clip",FIND_Optional)UClass; //tbd!
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                     Animation request.
        //--------------------------------------------------------------------
        case ACTOR_Animate:
        {
            const PAnimate & Info = PAnimate::Convert(Params);
            switch( Info.Kind )
            {
                case PAnimate::DeathAnimation:
                {
                    Actor.AddAnimation( Random(BigManA_DieForward,BigManA_DieBackward), 1 );
                    break;
                }
                case PAnimate::IdleAnimation:
                case PAnimate::StillAnimation:
                {
                    Actor.AddAnimation( BigManA_Sleep );
                    break;
                }
                case PAnimate::MoveAnimation:
                {
                    //tbi: Assume we are in the standing position.
                    //tbd?Actor.AddAnimation( BigManA_FootUp  , 1 );
                    Actor.AddAnimation( BigManA_Walk        );
                    //tbd?Actor.AddAnimation( BigManA_FootDown, 1 );
                    break;
                }
                case PAnimate::RunAnimation:
                {
                    break;
                }
                case PAnimate::HitAnimation:
                {
                    Actor.AddAnimation( BigManA_TakeHit, 1 );
                    break;
                }
                case PAnimate::SearchAnimation:
                {
                    //tbi: Assume we are in the standing position.
                    Actor.AddAnimation( BigManA_StillLook );
                    break;
                }
                case PAnimate::DistantStillAttackAnimation  :
                {
                    //tbd?Actor.AddAnimation( BigManA_FootUp  , 1 );
                    Actor.AddAnimation( BigManA_StillFire );
                    //tbd?Actor.AddAnimation( BigManA_FootDown, 1 );
                    break;
                }
                case PAnimate::DistantMovingAttackAnimation :
                {
                    //tbd?Actor.AddAnimation( BigManA_FootUp  , 1 );
                    Actor.AddAnimation( Random(BigManA_ShootRight,BigManA_ShootLeft) );
                    //tbd?Actor.AddAnimation( BigManA_FootDown, 1 );
                    break;
                }
                case PAnimate::CloseUpAttackAnimation     :
                {
                    if( bTargetIsNear )
                    {
                        //tbd?Actor.AddAnimation( BigManA_FootUp  , 1 );
                        Actor.AddAnimation( BigManA_PistolWhip );
                        //tbd?Actor.AddAnimation( BigManA_FootDown, 1 );
                    }
                    break;
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
            const EBigManAnimationTriggers Trigger = EBigManAnimationTriggers(Info.Trigger);
            switch( Trigger )
            {
                case BigManAT_StillFireLeft:
                case BigManAT_StillFireRight:
                {
                    if( Actor.CanSeeTarget() )
                    {
                        Actor.SpawnPyrotechnic( AttackEffects[0], 30.0, 30.0, Trigger==BigManAT_StillFireLeft ? 40.0 : -40.0 );
                        Actor.MakeSound( ShootSound );
                        Actor.CauseRayDamage(Actor.Target(),ShootDamage,100.0);
                    }
                    return ProcessDone;
                    break;
                }
                case BigManAT_PistolWhip:
                {
                    if( Actor.CanSeeTarget() && bTargetIsNear )
                    {
                        Actor.MakeSound( WhipSound );
                        Actor.CauseDamage(Actor.Target(),WhipDamage,1000);
                    }
                    return ProcessDone;
                    break;
                }
            }
            return ProcessDone;
            break;
        }
    }
    return ProcessParent;
    UNGUARD("ABigMan::Process");
};

//----------------------------------------------------------------------------
//                    GasBag Pawn processing
//----------------------------------------------------------------------------
int AGasbag::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                     Animation request.
        //--------------------------------------------------------------------
        case ACTOR_Animate:
        {
            const PAnimate & Info = PAnimate::Convert(Params);
            switch( Info.Kind )
            {
                case PAnimate::DeathAnimation:
                {
                    Actor.AddAnimation( GasBagA_Deflate, 1 );
                    break;
                }
                case PAnimate::StillAnimation:
                {
                    Actor.UseFrame( GasBagA_Float, 1 );
                    break;
                }
                case PAnimate::RunAnimation:
                case PAnimate::MoveAnimation:
                case PAnimate::FlyAnimation:
                {
                    //tbi: Assume we are in the fighter position.
                    Actor.AddAnimation( GasBagA_T2      , 1 );
                    Actor.AddAnimation( GasBagA_Float       );
                    Actor.AddAnimation( GasBagA_T1      , 1 );
                    break;
                }
                case PAnimate::HitAnimation:
                {
                    break;
                }
                case PAnimate::IdleAnimation:
                case PAnimate::SearchAnimation:
                {
                    Actor.AddAnimation( GasBagA_Fiddle );
                    break;
                }
                case PAnimate::DistantStillAttackAnimation  :
                {
                    Actor.AddAnimation( GasBagA_Belch );
                    Actor.AddAnimation( GasBagA_Fighter , 1 );
                    break;
                }
                case PAnimate::CloseUpAttackAnimation     :
                {
                    if( bTargetIsNear )
                    {
                        Actor.AddAnimation( Random(GasBagA_Belch,GasBagA_TwoPunch,GasBagA_Pound) );
                        Actor.AddAnimation( GasBagA_Fighter , 1 );
                    }
                    break;
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
            const EGasBagAnimationTriggers Trigger = EGasBagAnimationTriggers(Info.Trigger);
            switch( Trigger )
            {
                case GasBagAT_Belch:
                {
                    if( Actor.CanSeeTarget() )
                    {
                        Actor.SpawnPyrotechnic(  AttackEffects[0], 30.0, 0.0,  0.0 );
                        Actor.CauseDamage(Actor.Target(),BelchDamage,500);
                    }
                    return ProcessDone;
                    break;
                }
                case GasBagAT_PunchLeft:
                case GasBagAT_PunchRight:
                {
                    if( Actor.CanSeeTarget() && bTargetIsNear )
                    {
                        Actor.CauseDamage(Actor.Target(),PunchDamage,800);
                    }
                    return ProcessDone;
                    break;
                }
                case GasBagAT_Pound:
                {
                    if( Actor.CanSeeTarget() && bTargetIsNear )
                    {
                        Actor.CauseDamage(Actor.Target(),PoundDamage,1000);
                    }
                    return ProcessDone;
                    break;
                }
            }
            return ProcessDone;
            break;
        }
    }
    return ProcessParent;
    UNGUARD("AGasBag::Process");
};

//----------------------------------------------------------------------------
//                    Manta Pawn processing
//----------------------------------------------------------------------------
int AManta::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                     Animation request.
        //--------------------------------------------------------------------
        case ACTOR_Animate:
        {
            const PAnimate & Info = PAnimate::Convert(Params);
            switch( Info.Kind )
            {
                case PAnimate::DeathAnimation:
                {
                    Actor.AddAnimation( MantaA_Die, 1 );
                    break;
                }
                case PAnimate::StillAnimation:
                {
                    Actor.AddAnimation( MantaA_Fly );
                    break;
                }
                case PAnimate::RunAnimation:
                case PAnimate::MoveAnimation:
                case PAnimate::FlyAnimation:
                {
                    Actor.AddAnimation( MantaA_Fly );
                    break;
                }
                case PAnimate::HitAnimation:
                {
                    Actor.AddAnimation( MantaA_Fly );
                    break;
                }
                case PAnimate::IdleAnimation:
                case PAnimate::SearchAnimation:
                {
                    Actor.AddAnimation( MantaA_Fly );
                    break;
                }
                case PAnimate::CloseUpAttackAnimation     :
                {
                    if( bTargetIsNear )
                    {
                        Actor.AddAnimation( Random(MantaA_Sting,MantaA_Whip) );
                    }
                    break;
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
            const EMantaAnimationTriggers Trigger = EMantaAnimationTriggers(Info.Trigger);
            switch( Trigger )
            {
                case MantaAT_Sting:
                {
                    if( Actor.CanSeeTarget() )
                    {
                        Actor.MakeSound( StingSound );
                        Actor.CauseDamage(Actor.Target(),StingDamage,1000.0);
                    }
                    return ProcessDone;
                    break;
                }
                case MantaAT_Whip:
                {
                    if( Actor.CanSeeTarget() && bTargetIsNear )
                    {
                        Actor.MakeSound( WhipSound );
                        Actor.CauseDamage(Actor.Target(),WhipDamage,1000.0);
                    }
                    return ProcessDone;
                    break;
                }
            }
            return ProcessDone;
            break;
        }
    }
    return ProcessParent;
    UNGUARD( "AManta::Process" );
};

//----------------------------------------------------------------------------
//                    Woman Pawn processing
//----------------------------------------------------------------------------
int AWoman::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        case ACTOR_Spawn:
        case ACTOR_BeginPlay:
            AnimSeq = 0;
            return ProcessParent;
        case ACTOR_PlayerTick:
        {

            if ((GServer.Ticks & 3)==0)
            {
                FLOAT Speed = Velocity.Size();
                FixFrame += (int)(Speed * 65536.0 * 0.6);
                while (FixFrame >= FIX(30)) FixFrame -= FIX(30);
            }
            AnimBase = UNFIX(FixFrame);

            if( LifeState == LS_Dead )
            {
                Era++;
                bHasInvisibility = TRUE;
                bCannotMove = TRUE;
                bCannotTurn = TRUE;
                if( Era <= 300 )
                {
                    if( iKiller != INDEX_NONE )
                    {
                        FActor & Killer = FActor::Actor(iKiller);
                        Actor.TurnTowards( Killer.Location, Degrees(10) );
                        ViewRot.Yaw = DrawRot.Yaw;
                    }
                    //Level->SendMessage( iMe, ACTOR_Tick, 0 );
                    return ProcessParent;
                }
                Actor.Send_RestartLevel();
                //todo: Remove this when level restarting is *really* implemented.
                // Move actor back to PlayerStart, and restock his vitals and ammo.
                for (INDEX Which=0; Which<Level->Actors->Max; Which++)
                {
                    FActor & TestActor = FActor::Actor(Which);
                    if(TestActor.Class == GClasses.PlayerStart)
                    {
                        Actor.ViewRot.Yaw   = TestActor.DrawRot.Yaw;
                        Actor.DrawRot.Yaw    = TestActor.DrawRot.Yaw;
                        Health = 100.0;
                        bStatusChanged = TRUE;
                        Actor.Location = TestActor.Location;
                        TargetPitch  = 0;
                        ViewRot = GMath.ZeroRotation;
                        DrawRot = GMath.ZeroRotation;
                    }
                    LifeState = LS_Alive;
                    bHasInvisibility = FALSE;
                    bCannotMove = FALSE;
                    bCannotTurn = FALSE;
                }
                return ProcessDone;
            }
            else if( LifeState == LS_Dying )
            {
                //E3 hack:
                // As the player dies, focus his line of sight on his killer.
                if( iKiller != INDEX_NONE )
                {
                    FActor & Killer = FActor::Actor(iKiller);
                    Actor.TurnTowards( Killer.Location, Degrees(10) );
                    ViewRot.Yaw = DrawRot.Yaw;
                }
                ViewRot.Roll += 200;
                EyeHeight -= 1.0;
                ViewRot.Pitch /= 4;
                if( EyeHeight <= -30.0 )
                {
                    LifeState = LS_Dead;
                    TargetPitch = 0;
                    Era = 0;
                }
                Level->SendMessage( iMe, ACTOR_Tick, 0 );
                return ProcessDone;
            }
            else
            {
                return ProcessParent;
            }
            break;
        }
        case ACTOR_Die:
        {
            LifeState = LS_Dying;
            return ProcessParent;
            break;
        }
    }
    return ProcessParent;
    UNGUARD("Woman::Process");
    };

//----------------------------------------------------------------------------
//                    Dragon Pawn processing
//----------------------------------------------------------------------------
int ADragon::Process(ILevel *Level, FName Message, void *Params)
    {
    GUARD;
    switch (Message.Index)
        {
        case ACTOR_BeginPlay:
            AnimSeq = 0;
            return ProcessParent;
        case ACTOR_Tick:
            AnimSeq=0;
            if ((GServer.Ticks & 1)==0) AnimBase++;
            // Move by velocity. This helps to show off Unreal's momentum system.
            Level->MoveActor(iMe,&Velocity);
            Velocity *= 0.9; // Friction damping
            if (Velocity.SizeSquared()<1.0) Velocity = GMath.ZeroVector;
            return ProcessParent;
        };
    return ProcessParent;
    UNGUARD("ADragon::Process");
    };


//----------------------------------------------------------------------------
//                    ArchAngel Pawn processing
//----------------------------------------------------------------------------
int AArchAngel::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        case ACTOR_Tick:
        {
            break;
        }
    }
    return ProcessParent; 
    UNGUARD( "AArchAngel::Process" );
};

//----------------------------------------------------------------------------
//                    Tentacle Pawn processing
//----------------------------------------------------------------------------
int ATentacle::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        case ACTOR_Spawn:
        case ACTOR_BeginPlay:
        {
            break;
        }
        case ACTOR_Tick:
        {
            break;
        }
        //--------------------------------------------------------------------
        //                     Animation request.
        //--------------------------------------------------------------------
        case ACTOR_Animate:
        {
            const PAnimate & Info = PAnimate::Convert(Params);
            switch( Info.Kind )
            {
                case PAnimate::DeathAnimation:
                {
                    Actor.AddAnimation( TentacleA_Death );
                    break;
                }
                case PAnimate::StillAnimation:
                {
                    Actor.UseFrame( TentacleA_Waver, 1 );
                    break;
                }
                case PAnimate::RunAnimation:
                case PAnimate::MoveAnimation:
                case PAnimate::FlyAnimation:
                {
                    break;
                }
                case PAnimate::HitAnimation:
                {
                    break;
                }
                case PAnimate::IdleAnimation:
                {
                    Actor.AddAnimation( TentacleA_Waver );
                    break;
                }
                case PAnimate::SearchAnimation:
                {
                    Actor.AddAnimation( TentacleA_Mebax );
                    break;
                }
                case PAnimate::CloseUpAttackAnimation     :
                case PAnimate::DistantStillAttackAnimation  :
                {
                    Actor.AddAnimation( TentacleA_Shoot );
                    break;
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
            const ETentacleAnimationTriggers Trigger = ETentacleAnimationTriggers(Info.Trigger);
            switch( Trigger )
            {
                case TentacleAT_Shoot:
                {
                    if( Actor.CanSeeTarget() )
                    {
                        Actor.MakeSound( ShootSound );
                        if( bTargetIsNear )
                        {
                            // Rather than firing a projectile, just whip the target.
                            Actor.CauseDamage(Actor.Target(),WhipDamage,1000.0);
                        }
                        else if( Projectile != 0 )
                        {
                            //todo: encapsulate this function.
                            FVector Velocity = Actor.Target().Location - this->Location;
                            Velocity.Normalize();
                            Velocity *= FActor::Actor(Projectile->DefaultActor).Projectile().Speed;
                            INDEX iProjectile = GGame.SpawnProjectile
                            (
                                iMe
                            ,   Projectile
                            ,   Velocity
                            ,   this->CollisionRadius*1.10 + Projectile->DefaultActor.CollisionRadius
                            );
                        }
                    }
                    return ProcessDone;
                    break;
                }
            }
            return ProcessDone;
            break;
        }
    }
    return ProcessParent; 
    UNGUARD( "ATentacle::Process" );
};
