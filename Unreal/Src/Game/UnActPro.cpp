/*=============================================================================
	UnActPro.cpp: Projectile actor code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "UnGame.h"
#include "UnFActor.h"
#include "UnCheat.h"

/*=============================================================================
	Public projectile functions
=============================================================================*/

//
// Spawn a projectile and return its actor index or INDEX_NONE
//
// Velocity  = Velocity vector, usually the direction the parent is facing.
// Class     = Class of projectile
INDEX FGame::SpawnProjectile
(
    INDEX           iParentActor
,   UClass        * Class
,   const FVector & Velocity
,   FLOAT           StartDist
)
{
    GUARD;
    FActor & Parent = FActor::Actor(iParentActor);
    FVector Start = Parent.Location + Velocity * (StartDist/Velocity.Size());
    //
    INDEX iActor = GLevel->SpawnActor(Class,NAME_NONE,&Start);
    if (iActor != INDEX_NONE)
    {
        AProjectile & Projectile = FActor::Projectile(iActor);
        //
        Projectile.Velocity     = Velocity      ;
        Projectile.iParent      = iParentActor  ;
        //tbc: Conversion
        if( Parent.IsPlayer() )
        {
            Projectile.DrawRot = Parent.ViewRot;
        }
        else
        {
            Projectile.DrawRot = ((FVector&)Velocity).Rotation();
        }
        if( GCheat->SlowProjectiles )
        {
            Projectile.Velocity.Normalize();
            Projectile.Velocity *= 2.0;
            Projectile.LifeSpan = 700;
        }
    }
    return iActor;
    UNGUARD("FGame::SpawnProjectile");
}

//tbm:
// Yaw the vector by the specified Angle.
//tbi: This is not really what happens.
static inline void Yaw(FVector & This, WORD Angle) //tbi:word
{
    const FLOAT X1 = This.X;
    const FLOAT Y1 = This.Y;
    const FLOAT Cos = GMath.CosTab(Angle);
    const FLOAT Sin = GMath.SinTab(Angle);
    This.X = Y1*Sin + X1*Cos;
    This.Y = Y1*Cos - X1*Sin;
}

//
// Spawn a projectile based on an actor's rotation
//
INDEX FGame::SpawnForwardProjectile 
(
    INDEX       iParentActor
,   UClass    * Class
,   FLOAT       StartDist
,   int         YawDeviation
,   int         PitchDeviation
)
{
    GUARD;
    FActor & Parent  = FActor::Actor(iParentActor);
    FCoords Coords;
    FVector Velocity;
    //
    // Transform the actor's vector:
    //
    Parent.GetViewCoords(&Coords);
    AProjectile & DefaultProjectile = FActor::Actor( Class->DefaultActor ).Projectile();
    if( YawDeviation != 0 )
    {
        Yaw( Coords.ZAxis, YawDeviation );                                            
    }
    //todo: Use PitchDeviation
    Velocity = Coords.ZAxis * DefaultProjectile.Speed;
    //
    return SpawnProjectile(iParentActor,Class,Velocity,StartDist);
    UNGUARD("FGame::SpawnForwardProjectile");
}

//todo: Delete this when FVector::Mirror is fixed.
static inline FVector TempMirror (const FVector &Vector, const FVector &MirrorNormal) 
	{
	FLOAT   OutFactor = 2.0 * ( Vector.X * MirrorNormal.X + Vector.Y * MirrorNormal.Y + Vector.Z * MirrorNormal.Z );
	FVector Result;
	Result.X = Vector.X - OutFactor * MirrorNormal.X;
	Result.Y = Vector.Y - OutFactor * MirrorNormal.Y;
	Result.Z = Vector.Z - OutFactor * MirrorNormal.Z;
	return Result;
	};

//----------------------------------------------------------------------------
//                 AProjectile base processing
//----------------------------------------------------------------------------
int AProjectile::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FActor & Actor = FActor::Actor(*this);
    switch (Message.Index)
    {
        case ACTOR_Spawn: //tbi: Once all actors get BeginPlay, remove Spawn
        //----------------------------------------------------------
        //                     Initialization
        //----------------------------------------------------------
        case ACTOR_BeginPlay:
        {
            // Establish any texture animation:
            if( TextureCount > 0 )
            {
                TextureList = int(&Textures[0]); //tbi: Conversion            
            }
            return ProcessParent;
            break;
        }
        //----------------------------------------------------------
        //                 Time passes
        //----------------------------------------------------------
        case ACTOR_Tick:
        case ACTOR_PlayerTick:
        {
            ARoot::Process(Level,Message,Params); //tbi: When call to parent is supported
            if (Class) // If the projectile didn't die...
            {
                // Move the projectile and check for wall collision
                Level->MoveActor(iMe,&Velocity);
                if (Level->ModelInfo.FuzzyPointClass (&Location,CollisionRadius*1.01)<0.9999)
                {
                    // The projectile collided with a wall.
                    Actor.Send_HitWall();
                };
            }
            return ProcessDone;
            break;
        }
        //----------------------------------------------------------
        //                 The projectile hit a wall
        //----------------------------------------------------------
        case ACTOR_HitWall:
        {
            const BOOL ShouldBounce = bBounce && ( MaxBounceCount==0 || BounceCount<MaxBounceCount );
            BOOL ShouldDie = !ShouldBounce;
            FActor::Spawn(EffectOnWallImpact != 0 ? EffectOnWallImpact : EffectOnImpact, Actor.Location);
            if (ShouldBounce)
            {
                FVector Reflection;
                BounceCount++;
                const BOOL ReflectsOkay = Level->ModelInfo.SphereReflect(&Location,CollisionRadius*1.01,&Reflection);
                if( ReflectsOkay )
                {
                    Reflection.Normalize();
                    if( BounceIncidence > 0 )
                    {
                        // Make sure the incident angle of the bounce is such that
                        // we want to do the bounce.
                        FVector Direction = Velocity;
                        Direction.Normalize();
                        const FLOAT Cosine = - (Direction | Reflection); // Cosine of incident angle (relative to normal)
                        if( Cosine > BounceIncidence )
                        {
                            ShouldDie = TRUE;
                        }
                    }
                    if( !ShouldDie )
                    {
                        Velocity = TempMirror( Velocity, Reflection);
                        DrawRot = Velocity.Rotation();
                    }
                }
                else
                {
                    ShouldDie = TRUE;
                }
            }
            if( ShouldDie )
            {
                Level->DestroyActor(iMe);
            }
            return ProcessDone;
            break;
        }
        //----------------------------------------------------------
        //           The projectile touched something (someone?)
        //----------------------------------------------------------
        case ACTOR_Touch: 
        {
            PTouch & TouchInfo = *(PTouch*)Params;
            FActor & HitActor = FActor::Actor(TouchInfo.iActor );
            UClass * Effect = (HitActor.IsA(GClasses.Pawn) && EffectOnPawnImpact != 0) ? EffectOnPawnImpact : EffectOnImpact;
            FActor::Spawn(Effect,Location);

            PHit HitInfo;
            HitInfo.Empty();
            //tbi: What if the object struck is not a pawn?
            for( int Which = 0; Which < DMT_Count; Which++ )
            {
                FLOAT DamageAmount = Damage[Which] - Age * DamageDecay[Which];
                if( DamageAmount < 0 ) { DamageAmount = 0.0; }
                HitInfo.Damage[Which] = DamageAmount;
            }
            HitInfo.iSourceActor    = iMe           ;
            HitInfo.iSourceWeapon   = INDEX_NONE    ;
            HitInfo.Momentum        = Mass * Velocity.Size();
            FActor & ParentActor = FActor::Actor(iParent);
            if( ParentActor.Class->IsKindOf(GClasses.Weapon) )
            {
                HitInfo.iSourceActor  = ParentActor.iParent;
                HitInfo.iSourceWeapon = iParent;
            }
            HitActor.Send_Hit( HitInfo );
            // Handle any explosive charge imparted to target:
            if( ExplosiveTransfer != 0 )
            {
                //tbi: This is duplicated in unweapon.cpp!
                HitActor.AddExplosiveCharge(ExplosiveTransfer);
            }

            LifeSpan = 2; 
            Velocity = GMath.ZeroVector;
            return ProcessDone;
            break;
        }
        //----------------------------------------------------------
        //           The projectile should cease to exist.
        //----------------------------------------------------------
        case ACTOR_Die:
        {
            FActor::Spawn( EffectAtLifeSpan, Location );
            return ProcessParent;
            break;
        }
    }
    return ProcessParent;
    UNGUARD("AProjectile::Process");
}

//----------------------------------------------------------------------------
//                 AFireball base processing
//----------------------------------------------------------------------------
int AFireball::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return 0;
    UNGUARD("AFireball::Process");
}

//----------------------------------------------------------------------------
//                 AFireball2 base processing
//----------------------------------------------------------------------------
int AFireball2::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD("AFireball2::Process");
}


//----------------------------------------------------------------------------
//                 AShellProjectile base processing
//----------------------------------------------------------------------------
int AShellProjectile::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD("AShellProjectile::Process");
}

//----------------------------------------------------------------------------
//                 ABulletProjectile base processing
//----------------------------------------------------------------------------
int ABulletProjectile::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD("ABulletProjectile::Process");
}

//----------------------------------------------------------------------------
//                 AStingerProjectile base processing
//----------------------------------------------------------------------------
int AStingerProjectile::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD("AStingerProjectile::Process");
}

//----------------------------------------------------------------------------
//                 ATentacleProjectile base processing
//----------------------------------------------------------------------------
int ATentacleProjectile::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    return ProcessParent;
    UNGUARD("ATentacleProjectile::Process");
}

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
