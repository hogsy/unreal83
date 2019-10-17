/*==============================================================================
UnFActor.cpp: <Description>

Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
Compiled with Visual C++ 4.0.

Description:
    Refer to the associated header file.

Revision history:
    * 07/04/96: Created by Mark
==============================================================================*/

#include "UnFActor.h"
#include "UnRandom.h"
#include "UnCheat.h"

#define DebugAI 0 // 1 to put AI debugging information into log, 0 otherwise.

//tbi? (Think up a smarter way to determine angle changes.)
static WORD ArcCosValues[21] = // arc cosine(x) = ArcCosValues[ 10*x + 10 ]
{
    32768   // arccos(-1.0)
,   28064   // arccos(-0.9)
,   26056   // arccos(-0.8)
,   24472   // arccos(-0.7)
,   23096   // arccos(-0.6)
,   21845   // arccos(-0.5)
,   20676   // arccos(-0.4)
,   19562   // arccos(-0.3)
,   18484   // arccos(-0.2)
,   17429   // arccos(-0.1)
,   16384   // arccos( 0.0)
,   15339   // arccos(+0.1)
,   14284   // arccos(+0.2)
,   13206   // arccos(+0.3)
,   12092   // arccos(+0.4)
,   10923   // arccos(+0.5)
,    9672   // arccos(+0.6)
,    8296   // arccos(+0.7)
,    6712   // arccos(+0.8)
,    4704   // arccos(+0.9)
,       0   // arccos(+1.0)
};

// For an angle near 0 (cos near 1), provide smaller increments for greater accuracy.
// This is done so that when an angle is near a target angle, we don't turn wide
// of the target.
static WORD NearUnitArcCosValues[11] = 
{
    4704    // arccos(0.90)
,   4459    // arccos(0.91)
,   4200    // arccos(0.92)
,   3926    // arccos(0.93)
,   3632    // arccos(0.94)
,   3312    // arccos(0.95)
,   2960    // arccos(0.96)
,   2561    // arccos(0.97)
,   2090    // arccos(0.98)
,   1476    // arccos(0.99)
,   0       // arccos(1.00)
};

static inline WORD ArcCos(FLOAT Value)
{
    return
        Value <= 0.90 ? ArcCosValues[ int(10*Value + 10) ]
    :                   NearUnitArcCosValues[ int(100*(Value-0.9)) ]
    ;
}

//tbm: Convert degrees to WORD angle measurements.
static inline int Degrees(int Degrees)
{
    return Degrees*65536/360;
}


//----------------------------------------------------------------------------
//             Convenience functions.
//----------------------------------------------------------------------------
static inline FLOAT Max( FLOAT Value1, FLOAT Value2 )
{
    return Value1 >= Value2 ? Value1 : Value2;
}

static inline FLOAT Min( FLOAT Value1, FLOAT Value2 )
{
    return Value1 <= Value2 ? Value1 : Value2;
}

#define arrayCount_(Array) ( sizeof(Array) / sizeof((Array)[0]) )

static inline FLOAT Meters(FLOAT Meters) // Convert meters to world units.
{
    return 52.5*Meters; // About 52 world units per meter.
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

//----------------------------------------------------------------------------
//             Values for tailoring and fine-tuning the behavior:
//----------------------------------------------------------------------------
static const FLOAT FractionOfDamageAbsorbedByExternalArmor = 0.75;
   // For every unit of damage directed at a pawn, this many units are absorbed
   // by any external armor (of the type which protects against that kind of
   // damage).


// Turn the vector 90 degrees to the right. Works only in the X-Y plane.
static inline void Turn90Right(FVector & This)
{
    // (X,Y,Z) --> (-Y,X,Z)
    FLOAT OldX = This.X;
    This.X = -This.Y;
    This.Y = OldX;
}


//----------------------------------------------------------------------------
//                    Empty the AI task
//----------------------------------------------------------------------------
void FActor::EmptyAI(BOOL CancelAnimations)
{
    AActorAI & AI = this->AI();
    AITask = EAI_TaskNone;
    StopTimer();
    AI.BasicMotion.Empty();
    if( CancelAnimations )
    {
        AI.Animations.Empty();
    }
    else
    {
        AI.Animations.FinishSoon();
    }
}

//----------------------------------------------------------------------------
//                       Dump AI info to log
//----------------------------------------------------------------------------
void FActor::DumpAI() const
{
    //tbc: Make this code conditional to reduce space in released version.
    const AActorAI & AI = this->AI();
    const FActor & Actor = *this;
    char Info[100];
    char * Text = Info;
    Text += sprintf( Text, "%s AITask:", Actor.Class->Name );
    // Task kind:
    {
        char * KindText;
        switch(AITask)
        {
            case EAI_TaskNone       : KindText = "None"     ; break;
            case EAI_TaskMove       : KindText = "Move"     ; break;
            case EAI_TaskSearch     : KindText = "Search"   ; break;
            case EAI_TaskWait       : KindText = "Wait"     ; break;
            case EAI_TaskAttack     : KindText = "Attack"   ; break;
            default: checkFailed();
        }
        Text += sprintf( Text, " %s", KindText );
    }

    Text += sprintf( Text, " T:%i Age:%i", TimerCountdown, Era );

    Text += sprintf( Text, " Mess(%i)", TimerMessage );

    if( AITask == EAI_TaskMove )
    {
        Text += sprintf(Text," Motion(%i,%i)", AI.MotionGoal.Location.Kind, AI.MotionGoal.Rotation.Kind );
    }
    debug( LOG_Info, Info );
    
    Text = Info;
    Text += sprintf( Text, "  %i animations:", AI.Animations.Count() );
    debug( LOG_Info, Info );
    for( int Which = 1; Which <= AI.Animations.Count(); ++Which )
    {
        const AIAnimation & Animation = AI.Animations[Which];
        Text = Info;
        Text += sprintf
        ( 
            Text
        ,   "  [%i] %i %i-%i (%i times)"
        ,   Which
        ,   Animation.Sequence
        ,   Animation.First
        ,   Animation.Last
        ,   Animation.Count
        );
        debug( LOG_Info, Info );
    }
    Text = Info;
    Text += sprintf
    ( 
        Text
    ,   "  bAnimate: %i Anim...Seq:%i Rate:%3.2f Base:%i Count:%i First:%i Last:%i Message: %i"
    ,   int(Actor.bAnimate) 
    ,   int(Actor.AnimSeq) 
    ,   float(Actor.AnimRate) 
    ,   int(Actor.AnimBase) 
    ,   int(Actor.AnimCount) 
    ,   int(Actor.AnimFirst) 
    ,   int(Actor.AnimLast) 
    ,   int(Actor.AnimMessage) 
    );
    debug( LOG_Info, Info );
}

//----------------------------------------------------------------------------
//                      Initialize AI info
//----------------------------------------------------------------------------
void FActor::InitializeAI()
{
    EmptyAI(TRUE);
    if( IsA(GClasses.Pawn) )
    {
        APawn & Pawn = this->Pawn();
        Pawn.bTargetWasHere  = FALSE;
        Pawn.bTargetWasNear  = FALSE;
    }
}

//----------------------------------------------------------------------------
//                 Set the bCollideActors property 
//----------------------------------------------------------------------------
void FActor::SetActorCollision(BOOL CollideActors)
{
    const BOOL DidCollide  = this->bCollideActors;
    const BOOL Collides = CollideActors;
    this->bCollideActors = Collides;
    if( DidCollide && !Collides )
    {
        // Actor use to collide, but collides no longer.
        GLevel->Actors->CollidingActors->RemoveElement(this);
    }
    else if( !DidCollide && Collides )
    {
        // Actor now collides, but didn't before.
        GLevel->Actors->CollidingActors->Add(this);
    }
}

//----------------------------------------------------------------------------
//                 Can the pawn see a location?
//----------------------------------------------------------------------------
BOOL FActor::CanSee(const FVector & Location) const
{
    BOOL CanSee = FALSE;
    const FVector & TargetLocation = Location;
    const APawn & SensorPawn = this->Pawn();
    IModel & Model = GLevel->ModelInfo;
    if( Model.LineClass( &SensorPawn.Location, &TargetLocation ) == 1 ) // Unobstructed line?
    {
        FVector SightVector = TargetLocation - SensorPawn.Location;
        if( SightVector.SizeSquared() <= SensorPawn.SightRadius*SensorPawn.SightRadius)
        {
            SightVector.Normalize();
            // Create a unit vector from the sensor's Yaw.
            //tbi: This must be corrected for actors with pitch:
            FVector YawVector;
            const int Yaw = SensorPawn.DrawRot.Yaw;
            YawVector.X = GMath.CosTab(Yaw);
            YawVector.Y = GMath.SinTab(Yaw);
            YawVector.Z = 0;
            if( (YawVector|SightVector) >= SensorPawn.PeripheralVision )
            {
                CanSee = TRUE;
            }
        }
    }
    return CanSee;
}

//----------------------------------------------------------------------------
//          Is *this (a weapon) ready to fire?
//----------------------------------------------------------------------------
//todo: Make this a Weapon message
BOOL FActor::CanFire(BOOL Primary) const
{
    BOOL CanFire = FALSE;
    if( this->IsA( GClasses.Weapon) && this->HasParent() )
    {
        const AWeapon & Weapon = this->Weapon();
        const APawn & Parent = this->Parent().Pawn();
        const int Usage = Primary ? 0 : 1; // For indexing arrays.
        {
            const int AmmoAvailable  = Parent.AmmoCount[Weapon.AmmoType];
            // AmmoHeld is the ammo we consider to be held by the weapon, although
            // for standard ammo it is really held by the weapon holder.
            int AmmoHeld = 
                Weapon.ReloadCount==0 ? AmmoAvailable 
            :   (AmmoAvailable-1)%Weapon.ReloadCount + 1
            ;
            if( AmmoAvailable > 0 && AmmoHeld > 0 )
            {
                CanFire = TRUE;
            }
        }
    }
    return CanFire;
}

//----------------------------------------------------------------------------
//          Have *this (a weapon) fire a single projectile.
//----------------------------------------------------------------------------
//todo: What on earth is this doing here!! This is a weapon-specific function
// and belongs in unweapon.cpp or maybe uninv.cpp. Move it!
int FActor::Fire
(
    BOOL    PrimaryUse     // TRUE if this is the primary use of the weapon (FALSE for secondary).
,   int     YawDeviation   // Deviation from weapon's direction of projectile's yaw
,   int     PitchDeviation // Deviation from weapon's direction of projectile's pitch
)
{
    FActor & Actor = *this;
    const int Usage = PrimaryUse ? 0 : 1;
    int DischargeCount = 0;
    if( this->IsA( GClasses.Weapon) && this->HasParent() )
    {
        AWeapon & Weapon = this->Weapon();
        APawn & Parent = this->Parent().Pawn();
        {
            FVector RecoilVelocity;
            FCoords Coords;
            this->GetViewCoords(&Coords);
            RecoilVelocity = Coords.ZAxis * (-Weapon.RecoilForce[Usage] / Parent.Mass);
            USound *    DischargeSound  = Weapon.DischargeSounds[Usage]    ;
            UClass *    ProjectileClass = Weapon.ProjectileClass[Usage]    ;
            const BOOL  RepeatSound     = Weapon.bRepeatSounds[Usage]      ;
            const FLOAT Noise           = Weapon.Noise[Usage]        ;
            int         DischargeLimit  = Weapon.Discharges[Usage]         ;
            const int   AmmoPerDischarge= Weapon.AmmoUsed[Usage]           ;
            int         AmmoAvailable   = Parent.AmmoCount[Weapon.AmmoType];
            const int   RecoilPitch     = Degrees(Weapon.RecoilPitch[Usage])/2;
            // AmmoHeld is the ammo we consider to be held by the weapon, although
            // for standard ammo it is really held by the weapon holder.
            int AmmoHeld = 
                Weapon.ReloadCount==0 ? AmmoAvailable 
            :   (AmmoAvailable-1)%Weapon.ReloadCount + 1
            ;
            if( Weapon.AmmoType == AmmoType_None || ProjectileClass == 0 )
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
                AmmoAvailable -= AmmoPerDischarge;
                AmmoHeld -= AmmoPerDischarge;
                DischargeCount++;
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
                    FActor::Spawn(Weapon.MuzzleEffectClass[Usage],Weapon.Location);
                }
                if( DefaultProjectile.bIsInstantHit )
                {
                    // Need to build this into a function (get view coords, get draw coords):
                    // Temporarily, remove DrawPitch from the view pitch...
                    Weapon.ViewRot.Pitch -= Weapon.DrawPitch;
                    FCoords Coords; ((AActor *)this)->GetViewCoords(&Coords);
                    ViewRot.Pitch += Weapon.DrawPitch;
                    FVector Direction = Coords.ZAxis;
                    if( Weapon.Dispersion[Usage] != 0 )
                    {
                        PerturbNormalVector(Direction,Coords,Weapon.Dispersion[Usage]);
                    }
                    if( YawDeviation != 0 )
                    {
                        Yaw( Direction, YawDeviation );                                            
                    }
                    //todo: Use PitchDeviation
                    FVector HitLocation;
                    FVector HitNormal;
                    INDEX   iHitNode;
                    GLevel->RayHit
                    (
                        iMe,&Location,&Direction,
                        DefaultProjectile.Mass*DefaultProjectile.Speed,0.0, 
                        0,TRUE,
                        &HitLocation,&HitNormal,&iHitNode
                    );
                }
                else
                {
                    GGame.SpawnForwardProjectile( Weapon.iMe, ProjectileClass, Weapon.ProjStartDist, YawDeviation, PitchDeviation );
                }
            }
            Weapon.bNeedsReloading = AmmoHeld < Weapon.AmmoUsed[0]*int(Weapon.Discharges[0]);
            Parent.AmmoCount[Weapon.AmmoType] = AmmoAvailable;
            Parent.bStatusChanged = TRUE;
        }
    }
    return DischargeCount;
}

//----------------------------------------------------------------------------
//           If *this is a pawn, add an explosive charge to it.
//----------------------------------------------------------------------------
void FActor::AddExplosiveCharge(FLOAT Charge)
{
    if( this->IsA(GClasses.Pawn) )
    {
        APawn & Pawn = this->Pawn();
        Pawn.ExplosiveCharge += Charge;
    }
}

//----------------------------------------------------------------------------
//       Have *this cause one kind of damage to the specified target.
//----------------------------------------------------------------------------
void FActor::CauseDamage 
(
    FActor    & Target
,   FLOAT       Damage
,   FLOAT       Momentum    
,   EDamageType DamageType  
)
const
{
    PHit Info;
    Info.Empty();
    if( this->iMe != Target.iMe )
    {
        Info.iSourceActor = iMe;
    }
    Info.Damage[DamageType] = Damage;
    Info.Momentum = Momentum;
    FActor::Send_Hit(Target.iMe,Info);
}

//----------------------------------------------------------------------------
//       Have *this cause basic damage to the specified target.
//----------------------------------------------------------------------------
void FActor::CauseRayDamage
(
    FActor    & Target
,   FLOAT       Damage
,   FLOAT       Momentum
)
{
    FActor & Source = *this;
    PHit HitInfo;
    HitInfo.Empty();
    HitInfo.iSourceActor = Source.iMe;
    HitInfo.Damage[DMT_Basic] = Damage;
    HitInfo.Momentum = 0; // 0 since the ::RayHit routine below will handle momentum.

    FVector HitLocation ;
    FVector HitNormal   ;
    INDEX   iHitNode    ;
    
    GLevel->RayHit
    (
        Source.iMe
    ,   &Source.Location
    ,   &(Target.Location-Source.Location)
    ,   Momentum
    ,   0.0
    ,   &HitInfo
    ,   0.0
    ,   &HitLocation
    ,   &HitNormal
    ,   &iHitNode
    );
}

//----------------------------------------------------------------------------
// Does *this (must be a pawn) have vision good enough to see as far as Location?
//----------------------------------------------------------------------------
// Ignore the fact that the view might be obstructed, instead just determine
// if the location is too far away or not.
BOOL FActor::CanSeeAsFarAs(const FVector & Location) const
{
    const APawn & Sensor = this->Pawn() ;
    return (Location - Sensor.Location).SizeSquared() <= Sensor.SightRadius*Sensor.SightRadius;
}

//----------------------------------------------------------------------------
// Does *this (must be a pawn) have hearing good enough to hear Noise at Location
//----------------------------------------------------------------------------
BOOL FActor::CanHear(const FVector & Location, FLOAT Noise) const
{
    BOOL CanHear = FALSE;
    IModel & Model = GLevel->ModelInfo;
    if( Model.LineClass( &this->Location, &Location ) == 1 ) // Unobstructed path?
    {
        const FLOAT Distance = (this->Location-Location).Size();
        CanHear = this->CanHear(Distance,Noise);
    }
    return CanHear;
}

//----------------------------------------------------------------------------
// Does *this (must be a pawn) have hearing good enough to hear Noise at Distance?
//----------------------------------------------------------------------------
BOOL FActor::CanHear(FLOAT Distance, FLOAT Noise) const
{
    const FLOAT Acuity = this->Pawn().AuralAcuity;
    return Acuity > 0 && 100.0*Noise/Distance >= Acuity;
}

//----------------------------------------------------------------------------
//      Is an actor near another actor? Use their collision radii.
//----------------------------------------------------------------------------
BOOL FActor::IsNear( const FActor & Actor ) const
{
    FLOAT NearLimit = this->CollisionRadius + Actor.CollisionRadius;
    NearLimit *= 1.30; // Add a percentage.
    FVector Direction = Actor.Location - this->Location;
    return Direction.SizeSquared() <= NearLimit * NearLimit;
}

//----------------------------------------------------------------------------
//             Can *this pawn hear Pawn?
//----------------------------------------------------------------------------
BOOL FActor::CanHear( const APawn & Pawn ) const
{
    IModel & Model = GLevel->ModelInfo;
    const APawn &   Sensor = this->Pawn() ;
    const APawn &   Target = Pawn;
    return
        !Target.bHasSilence
    &&  CanHear( Target.Location, Target.Noise )
    ;
}

//----------------------------------------------------------------------------
//             Can *this pawn sense Pawn?
//----------------------------------------------------------------------------
BOOL FActor::CanSense( const APawn & Pawn ) const
{
    //tbi? This involves computing Model.LineClass twice, is this okay?
    return CanSee(Pawn) || CanHear(Pawn);
}

//----------------------------------------------------------------------------
//                Stop turning (*this must be a pawn).
//----------------------------------------------------------------------------
void FActor::StopTurning()
{
    APawn & Pawn = this->Pawn();
    Pawn.YawSpeed    = 0 ;
    Pawn.PitchSpeed  = 0 ;
    Pawn.RollSpeed   = 0 ;
}

//----------------------------------------------------------------------------
//                Stop moving (*this must be a pawn).
//----------------------------------------------------------------------------
void FActor::StopMoving() 
{
    APawn & Pawn = this->Pawn();
    AActorAI & AI = this->AI();
    AI.MotionGoal.Location.Empty();
}


//----------------------------------------------------------------------------
//                    Take damage. *this must be a pawn.
//----------------------------------------------------------------------------
void FActor::TakeDamage( PHit & Info )
{
    APawn & Pawn = this->Pawn();
    FLOAT HealthDamage = 0;
    FActor & Actor = *this;
    char Message[100];
    char * Text = Message;

    const FLOAT OriginalHealth = Pawn.Health;
    if( !Pawn.bHasInvincibility && Pawn.LifeState == LS_Alive )
    {
        if( GCheat->ShowDamage ) 
        { 
            Text += sprintf
            ( 
                Text
            ,   "  %s:%2.1f,Damage[" 
            ,   Pawn.Class->Name
            ,   Pawn.Health
            );
			int Which;
            for( Which = 0; Which < DMT_Count; ++Which )
            {
                Text += sprintf( Text, " %2.1f", Info.Damage[Which] );
            }
            Text += sprintf( Text, "] Armor[" );
            for( Which = 0; Which < DMT_Count; ++Which )
            {
                Text += sprintf( Text, " %2.1f", Pawn.Armor[Which] );
            }
            Text += sprintf( Text, "] Mom:%3.1f", Info.Momentum );
        }
        FLOAT RawDamage = 0; // The total damaged, unchanged by armor.

        // Absorb some of the damage if the actor has armor of the
        // appropriate type. The armor is damaged.
		int WhichDamage;
        for( WhichDamage = 0; WhichDamage < DMT_Count; ++WhichDamage )
        {
            FLOAT & Damage = Info.Damage[WhichDamage];
            FLOAT & Armor  = Pawn.Armor[WhichDamage];
            if( Damage < 0 ) { Damage = 0.0; }
            RawDamage += Damage;
            if( Damage > 0 && Armor > 0 )
            {
                FLOAT DamageAbsorbed = Damage * FractionOfDamageAbsorbedByExternalArmor;
                if( DamageAbsorbed > Armor )
                {
                    DamageAbsorbed = Armor;
                }
                Armor  -= DamageAbsorbed;
                Damage -= DamageAbsorbed;
            }
        }

        // Absorb some of the damage by the actor's natural armor.
        for( WhichDamage = 0; WhichDamage < DMT_Count; ++WhichDamage )
        {
            FLOAT & Damage = Info.Damage[WhichDamage];
            FLOAT & Armor  = Pawn.NaturalArmor[WhichDamage];
            if( Damage > 0 && Armor > 0 )
            {
                const FLOAT DamageAbsorbed = Armor*Damage;
                Damage -= DamageAbsorbed;
            }
        }

        // Figure out the adjusted total damage to health:
        for( WhichDamage = 0; WhichDamage < DMT_Count; ++WhichDamage )
        {
            HealthDamage += Info.Damage[WhichDamage];
        }

        // Impart some momentum to actor:
        if( Info.Momentum != 0 && Info.iSourceActor != INDEX_NONE )
        {
            if( Actor.bMomentum && Actor.Mass != 0 )
            {
                FActor & Source = FActor::Actor(Info.iSourceActor);
                FVector dVelocity = Actor.Location - Source.Location;
                dVelocity.Normalize();
                dVelocity *= Info.Momentum / Actor.Mass;
                Actor.Velocity += dVelocity;
            }
        }
        // Cheat: Lethal hit
        if( Info.iSourceActor != INDEX_NONE )
        {
            FActor & Source = FActor::Actor(Info.iSourceActor);
            if( Source.IsA(GClasses.Pawn) )
            {
                APawn & SourcePawn = Source.Pawn();
                if( GCheat->LethalHit || GCheat->LethalHits )
                {
                    HealthDamage = Pawn.Health + 1; // Just enough to kill.
                    GCheat->LethalHit = FALSE;
                }
            }
            
        }
        // Add extra damage triggered by pawn with an explosive charge in it:
        if( Pawn.ExplosiveCharge > 3 && HealthDamage > 3 )
        {
            // Create some delayed damage.. If there is no delayed damage, make
            // the delay small.
            const FLOAT ExplosiveDamage = Pawn.ExplosiveCharge;
            if( Pawn.DelayedDamage == 0 )
            {
                Pawn.DamageDelay = 2;
            }
            Pawn.DelayedDamage += ExplosiveDamage;
            Pawn.ExplosiveCharge -= ExplosiveDamage;
        }
        const FLOAT NewHealth = Pawn.Health - ( GCheat->NoDamage ? 0 : HealthDamage );
        if( NewHealth < 1.0 && Pawn.Health >= 1.0 )
        {
            Pawn.LifeState = LS_Dead;
            Pawn.iKiller = Info.iSourceActor;
            Pawn.Health = 0;
            FActor::Send_Die(iMe);
        }
        else if( NewHealth >= 1.0 )
        {
            Pawn.Health = NewHealth;
        }
        Pawn.bStatusChanged = TRUE;
        if( GCheat->ShowDamage ) 
        {
            Text += sprintf( Text, "==> -%2.1f", HealthDamage ); 
            debug( LOG_Info, Message );
        }
    }
    Info.ActualDamage = HealthDamage;
}

//----------------------------------------------------------------------------
//   Is the path from *this pawn to Pawn unobstructed and within a sense radius?
//----------------------------------------------------------------------------
BOOL FActor::MightSense( const APawn & Pawn ) const
{
    const IModel & Model = GLevel->ModelInfo;
    const APawn & Sensor = this->Pawn();
    const APawn & Target = Pawn;
    const FLOAT MaxRadius = Sensor.SightRadius;
    return 
        Model.LineClass( &Sensor.Location, &Target.Location ) == 1 // Unobstructed line?
    &&  (Target.Location - Sensor.Location).SizeSquared() <= MaxRadius*MaxRadius
    ;
}


//----------------------------------------------------------------------------
// Spawn an effect, located relative to this->Location.
//----------------------------------------------------------------------------
FActor * FActor::SpawnPyrotechnic
( 
    UClass      * Class
,   FLOAT         Forward  // Distance forward from this->Location, relative to this->DrawRot.
,   FLOAT         Up       // Distance upward from this->Location, relative to this->DrawRot.
,   FLOAT         Right    // Distance to right of this->Location, relative to this->DrawRot.
)
const
{
    FActor * Effect = 0;
    if( Class != 0 )
    {
        FVector Location = this->Location;
        // Adjust location.
        //tbi: We just use Yaw, for now.
        FVector ForwardVector;
        FVector RightVector;
        const int Yaw = DrawRot.Yaw;
        ForwardVector.X = GMath.CosTab(Yaw);
        ForwardVector.Y = GMath.SinTab(Yaw);
        ForwardVector.Z = 0;
        RightVector = ForwardVector;
        Turn90Right( RightVector );
        Location += ForwardVector * Forward;
        Location += RightVector * Right;
        Location.Z += Up;

        INDEX iEffect = GLevel->SpawnActor(Class,NAME_NONE,&Location);
        Effect = Handle(iEffect);
	    if( Effect != 0 )
        {
            APyrotechnics & Pyrotechnic = Effect->Pyrotechnic();
            Pyrotechnic.Velocity        = GMath.ZeroVector   ;
            Pyrotechnic.Texture         = Pyrotechnic.Textures[0] ;
        }
    }    
    return Effect;
}

//----------------------------------------------------------------------------
//                 Can the actor see a pawn?
//----------------------------------------------------------------------------
BOOL FActor::CanSee(const APawn & Pawn) const
{
    return !Pawn.bHasInvisibility && CanSee( Pawn.Location );
}

//----------------------------------------------------------------------------
//                 Make the actor emit a sound
//----------------------------------------------------------------------------
void FActor::MakeSound(USound * Sound) const
{
    //todo: Improve this pawn-specific stuff..
    if( IsA(GClasses.Pawn) )
    {
        APawn & Pawn = (APawn &)this->Pawn(); //todo: Yech - gross cast. Fix this.
        if( Pawn.SoundTimer == 0 || Pawn.LifeState != LS_Alive )
        {
            MakeSound( Sound, this->Location );
            Pawn.SoundTimer = 10;
        }
    }
    else
    {
        MakeSound( Sound, this->Location );
    }
}

//----------------------------------------------------------------------------
//                 Make the actor emit a sound
//----------------------------------------------------------------------------
void FActor::MakeSound(USound * Sound, const FVector & Location) const
{
    if( Sound != 0 ) //tbi: This can be removed when the sound routines handle Sound==0
    {
        //todo: The following check for pawns is robust but
        // it seems a waste to do it here.
        INDEX Index = 
            this->IsA(GClasses.Pawn) && this->Pawn().LifeState != LS_Alive
        ?   -1          // Audio routines expect -1 for a dying monster to avoid cutting off his scream.
        :   this->iMe;
        ;
        GAudio.PlaySfxLocated(&Location, Sound, Index );
    }
}

//----------------------------------------------------------------------------
//                 The actor makes noise
//----------------------------------------------------------------------------
// The noise is just a numeric value indicating how loud the noise is.
void FActor::MakeNoise(FLOAT Noise)
{
    MakeNoise( Noise, this->Location );
}

//----------------------------------------------------------------------------
//                 The actor makes noise at a distant location
//----------------------------------------------------------------------------
// The noise is just a numeric value indicating how loud the noise is.
void FActor::MakeNoise(FLOAT Noise, const FVector & Location)
{
    if( Noise != 0 )
    {
        // Check all pawn actors and see if the sound should alarm them.
        //todo: Improve efficiency?
        UActorList & Actors = *GLevel->Actors;
    	for( int Which = 0; Which < Actors.DynamicActors->Count(); Which++ )
        {
            AActor * const CheckActor = (*Actors.DynamicActors)[Which];
            if( CheckActor != 0 && CheckActor->Class->IsKindOf(GClasses.Pawn) )
            {
                FActor & Actor = FActor::Actor( *CheckActor );
                APawn & Pawn = Actor.Pawn();
                if( Pawn.bRespondsToNoise && Actor.CanHear( Location, Noise ) )
                {
                    if( !Actor.HasTarget() )
                    {
                        // Let's just pretend the monster was targeting the source 
                        // of the noise. This gives a better monster response.
                        //todo: Encapsulate these settings into a function
                        Pawn.bIsQuiescent = FALSE;
                        Pawn.bTargetWasLost = TRUE;
                        Pawn.TargetLostTime = GServer.Ticks;
                        Pawn.TargetLastLocation = this->Location;
                    }
                    else
                    {
                        Pawn.bIsAlarmed = TRUE;
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
// Randomly pick a sound and play it.
//----------------------------------------------------------------------------
void FActor::PickSound 
(
    USound   ** Sounds      // A list of Count sounds. There can be trailing null sounds, which are not selected.
,   int         Count  
)
const
{
    int LastSound = Count-1; // Index of last non-null sound.
    while( LastSound >= 0 && Sounds[LastSound]==0 )
    {
        LastSound--;
    }
    if( LastSound >= 0 )
    {
        const int WhichSound = FRandom::Value(0,LastSound);
        MakeSound(Sounds[ WhichSound ]);
    }
}


//----------------------------------------------------------------------------
//              Create a new actor based on the given info.
//----------------------------------------------------------------------------
FActor * FActor::Spawn 
(
    UClass        * Class       // Class of actor to create. 0 if none.
,   const FVector & Location    // Location of new actor.
,   int             LifeSpan    // Life span of new actor, or 0 to use default class lifespan.
,   const FVector & Velocity    // Velocity of new actor.
)
{
    FActor * Actor = 0;
    if( Class != 0 )
    {
        INDEX iActor = GLevel->SpawnActor(Class,NAME_NONE,&Location);
        if (iActor != INDEX_NONE)
        {
            Actor = &FActor::Actor(iActor);
            Actor->Velocity = Velocity;
            if( LifeSpan != 0 )
            {
                Actor->LifeSpan = LifeSpan;
            }
        }
    }
    return Actor;
}

//----------------------------------------------------------------------------
//     Determine the first inventory item of the given class.
//----------------------------------------------------------------------------
// If found, return the item. Otherwise, return 0.
FActor * FActor::InventoryItem
(
    UClass        * Class       // Look for an item of this kind.
)
{
    FActor * Item = 0;
    INDEX iItem = INDEX_NONE;
    INDEX iCheck = iInventory;
    while( Item==0 && iCheck != INDEX_NONE )
    {
        FActor & CheckItem = FActor::Actor(iCheck);
        if( CheckItem.Class->IsKindOf( Class ) )
        {
            Item = &CheckItem;
        }
        iCheck = CheckItem.iInventory;
    }
    return Item;
}


//---------------------------------------------------------------------------
// Determine if a pawn can use some ammo (and optionally apply the ammo)
//---------------------------------------------------------------------------
BOOL FActor::CanUseAmmo 
(
    AAmmo   & Ammo       // Can *this make use Ammo?
,   BOOL      Apply      // If TRUE, actually give the ammo to the pawn.
)
{
    GUARD;
    BOOL CanUse = FALSE;
    APawn & Pawn    = this->Pawn();
    for( int Which = 0; Which < AmmoType_Count; ++Which )
    {
        const EAmmoType AmmoType = EAmmoType(Which);
        if( CanUseAmmo( AmmoType, Ammo.AmmoCount[Which], Apply ) )
        {
            CanUse = TRUE;
        }
    }
    return CanUse;
    UNGUARD("FActor::CanUseAmmo[1]");
}
//---------------------------------------------------------------------------
// Determine if a pawn can use some ammo (and optionally apply the ammo)
//---------------------------------------------------------------------------
BOOL FActor::CanUseAmmo
(
    EAmmoType AmmoType      // Can *this use more of this kind of ammo?
,   int       AmmoCount     // Amount of ammo available.
,   BOOL      Apply         // If TRUE, actually give the ammo to the pawn.
)
{
    GUARD;
    BOOL CanUse = FALSE;
    APawn & Pawn = this->Pawn();
    int & PawnAmmo      = Pawn.AmmoCount[AmmoType]    ;
    int   PawnAmmoLimit = Pawn.AmmoCapacity[AmmoType] ;
    if( PawnAmmo < PawnAmmoLimit && AmmoCount > 0 )
    {
        CanUse = TRUE;
        if( Apply )
        {
            PawnAmmo = OurMin( int(PawnAmmo)+AmmoCount, int(PawnAmmoLimit) );
            Pawn.bStatusChanged = TRUE;
        }
    }
    return CanUse;
    UNGUARD("FActor::CanUseAmmo[2]");
}

//----------------------------------------------------------------------------
// Determine if a pawn can use a power-up (and optionally apply the power-up).
//----------------------------------------------------------------------------
BOOL FActor::CanUsePowerUp
(
    APowerUp      & Power
,   BOOL            Apply       // If TRUE, actually apply the power-up to the pawn.
)
{
    BOOL CanUse = FALSE;
    APawn     & Pawn    = this->Pawn();

    // Strength:
    if( Power.Strength > 0 && Pawn.Strength < 1.0 )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.Strength = Min( Pawn.MaxStrength, Pawn.Strength+Power.Strength );
            Pawn.bStatusChanged = TRUE;
        }
    }

    // Stamina:
    if( Power.Stamina > 0 && Pawn.Stamina < 1.0 )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.Stamina = Min( 1.0, Pawn.Stamina+Power.Stamina );
            Pawn.bStatusChanged = TRUE;
        }
    }

    // Health:
    if( Power.Health > 0 && Pawn.Health < 100.0 )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.Health = Min( 100.0, Pawn.Health+Power.Health );
            Pawn.bStatusChanged = TRUE;
        }
    }

    // Armor:
    for( int WhichArmor = 0; WhichArmor < DMT_Count; ++WhichArmor )
    {
        FLOAT & PawnArmor   = Pawn.Armor  [WhichArmor];
        FLOAT & PowerArmor  = Power.Armor [WhichArmor];
        if( PowerArmor > 0 && PawnArmor < 100 )
        {
            CanUse = TRUE;
            if( Apply )
            {
                PawnArmor = Min( 100.0, PawnArmor + PowerArmor );
                Pawn.bStatusChanged = TRUE;
            }
        }
    }

    // Invisibility:
    if( Power.bInvisibility && !Pawn.bHasInvisibility )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.bHasInvisibility = TRUE;
            Pawn.InvisibilityTimeLeft = Power.TimeLimit;
            Pawn.bStatusChanged = TRUE;
        }
    }

    // Silence:
    if( Power.bSilence && !Pawn.bHasSilence )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.bHasSilence = TRUE;
            Pawn.SilenceTimeLeft = Power.TimeLimit;
            Pawn.bStatusChanged = TRUE;
        }
    }

    // Invincibility:
    if( Power.bInvincibility && !Pawn.bHasInvincibility )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.bHasInvincibility = TRUE;
            Pawn.InvincibilityTimeLeft = Power.TimeLimit;
            Pawn.bStatusChanged = TRUE;
        }
    }

    // SuperStrength:
    if( Power.bSuperStrength && !Pawn.bHasSuperStrength )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.bHasSuperStrength = TRUE;
            Pawn.SuperStrengthTimeLeft = Power.TimeLimit;
            Pawn.bStatusChanged = TRUE;
        }
    }

    // SuperStamina:
    if( Power.bSuperStamina && !Pawn.bHasSuperStamina )
    {
        CanUse = TRUE;
        if( Apply )
        {
            Pawn.bHasSuperStamina = TRUE;
            Pawn.SuperStaminaTimeLeft = Power.TimeLimit;
            Pawn.bStatusChanged = TRUE;
        }
    }

    return CanUse;
}

//----------------------------------------------------------------------------
//                   Stop the actor's timer.
//----------------------------------------------------------------------------
void FActor::StopTimer()
{
    TimerCountdown  = 0   ;
    TimerMessage    = 0   ;
}

//----------------------------------------------------------------------------
//                   Start the actor's timer.
//----------------------------------------------------------------------------
void FActor::StartTimer
(
    int             Time    // How many ticks until the timer expires.
,   EActorMessage   Message // Message to send when timer expires.
)
{
    TimerCountdown  = Time      ;
    TimerMessage    = Message   ;
}

//----------------------------------------------------------------------------
//                  Set the actor's image.
//----------------------------------------------------------------------------
void FActor::UseFrame
(
    int     Sequence            // The animation sequence number.
,   int     Frame               // Index (from 0) of the frame to use frame.
)
{
    CancelAnimations();
    bAnimate   = FALSE      ;
    AnimSeq    = Sequence   ;
    AnimBase   = Frame      ;
}

//---------------------------------------------------------------------------
//        Cancel all animations in progress (or suspended).
//---------------------------------------------------------------------------
void FActor::CancelAnimations()
{
    AActorAI & AI = this->AI();
    ARoot & Root = this->Root();
    while( !AI.Animations.IsEmpty() )
    {
        AI.Animations.MoveNextTo(Root);
    }
    if( bAnimate )
    {
        // Set the pawn to the last animation.
        AnimBase = AnimLast;
    }
    bAnimate   = FALSE         ;
    AnimSeq    = 0             ;
}

//---------------------------------------------------------------------------
//        Is this actor animated?
//---------------------------------------------------------------------------
BOOL FActor::IsAnimated() const
{
    const AActorAI & AI = this->AI();
    return bAnimate || !AI.Animations.IsEmpty();
}

//---------------------------------------------------------------------------
//        Finish animations as soon as possible without losing any.
//---------------------------------------------------------------------------
void FActor::FinishAnimations()
{
    AActorAI & AI = this->AI();
    AI.Animations.FinishSoon();
    if( bAnimate )
    {
        AnimCount = 1; // Causes current animation to finish soon.
    }
}

//---------------------------------------------------------------------------
// Start a mesh animation. Replace any existing animations.
//---------------------------------------------------------------------------
void FActor::ReplaceAnimation
(
    int             Sequence    // The animation sequence number.
,   BYTE            Count       // Number of times to do animation (0 means forever)
,   FLOAT           Rate        // Rate of animation (1.0=normal)
,   EActorMessage   Message     // Message to send at end of animation.
)
{
    CancelAnimations();
    AddAnimation(Sequence,Count,Rate,Message);
    AnimationTick(); // To get the first frame going.
}

//---------------------------------------------------------------------------
//             Prepare an actor for a new animation.
//---------------------------------------------------------------------------
static void PrepareForNewAnimation(FActor & Actor)
{
    AActorAI & AI = Actor.AI();
    if( AI.Animations.Count() == AIAnimations::MaxCount )
    {
        // The animation list is full. 
        // Let's copy the first animation (the oldest) into the actor's current
        // animation. This will create lost frames and jerky animation.
        AI.Animations.MoveNextTo(Actor.Root());
    }
}

//---------------------------------------------------------------------------
//             Add a mesh animation to the actor's list of animations.
//---------------------------------------------------------------------------
void FActor::AddAnimation 
(
    AIAnimation::TSequence  Sequence    // Sequence number (1..)
,   BYTE                    Count       // Number of times to repeat sequence. 0=forever
,   FLOAT                   Rate        // Animation rate.   
,   EActorMessage           Message     // Message to send at end of animation.
)
{
    AActorAI & AI = this->AI();
    PrepareForNewAnimation(*this);
    AI.Animations.Add( Sequence, Count, Rate );
    AI.Animations.First().Message = Message; //tbc: This should be the last animation, not the first.
    if( !bAnimate )
    {
        // Clear any existing frame:
        AnimSeq = 0;
    }
    bAnimate = TRUE;

}

//---------------------------------------------------------------------------
//          Returns an actor near *this, or 0 if none.
//---------------------------------------------------------------------------
FActor * FActor::NearbyFacedPawn(const FRotation & View) const
{
    const FActor & Actor = *this;
    INDEX Touchers[10];
    FVector Location = this->Location; //todo: Delete this when CheckActorTouch uses const parameters
    const int TouchersCount = GLevel->Dynamics.CheckActorTouch(Location, this->CollisionRadius*1.80, Touchers, arrayCount_(Touchers) );
    FActor * NearbyActor = 0;
    if( TouchersCount > 0 )
    {
        FCoords Coords;
        Actor.GetViewCoords(&Coords);
        FVector & ViewVector = Coords.ZAxis;
        INDEX BestChoice = INDEX_NONE;
        FLOAT BestDot    = 0; // Dot product of direction to BestChoice and view vector.
        // Note: The ideal dot product is 1.0 - smaller values are not as good.
        for( int Which = 0; Which < TouchersCount; ++Which )
        {
            const INDEX CheckIndex = Touchers[Which];
            FActor & Check = FActor::Actor(CheckIndex);
            if( CheckIndex != this->iMe && Check.IsA(GClasses.Pawn) )
            {
                FVector Direction = Check.Location - this->Location;
                Direction.Normalize();
                const FLOAT Dot = ViewVector | Direction;
                if( (BestChoice == INDEX_NONE && Dot >= .5 ) || Dot > BestDot )
                {
                    BestChoice = CheckIndex;
                    BestDot = Dot;
                }
            }
        }
        NearbyActor = FActor::Handle(BestChoice);
    }
    return NearbyActor;
}
 
//---------------------------------------------------------------------------
//   Advance animation by 1 tick. Send any ACTOR_FrameTrigger message.
//---------------------------------------------------------------------------
void FActor::AnimationTick()
{
    FActor & Actor = *this;
    if( IsAnimated() )
    {
        if( Actor.AnimLast==0 && Actor.AnimSeq != 0 && Actor.DrawType==DT_MeshMap && Actor.MeshMap != 0 )
        {
            // Determine the last frame from the mesh info.
            UMeshMap  * MeshMap     = Actor.MeshMap  ;
            UMesh     * Mesh        = MeshMap->Mesh  ;
            IMesh       MeshInfo;
            Mesh->GetInfo(&MeshInfo);
            Actor.AnimLast = MeshInfo.AnimSeqs[Actor.AnimSeq].SeqNumFrames-1;
        }
        if( Actor.DrawType==DT_Sprite )
        {
            Actor.AnimSeq = 1; //tbi: This is a hack because we sometimes use AnimSeq to detect animations,
                               // even though AnimSeq is not used for sprite animations.
        }
        const int PreviousFrame = Actor.AnimBase;
        const EActorMessage SavedMessage = EActorMessage(Actor.AnimMessage);
        BOOL ConsiderFrameTrigger   = FALSE     ; // TRUE iff we should consider sending ACTOR_FrameTrigger.
        BOOL Restart    = !bAnimate || Actor.AnimSeq==0; // TRUE iff we should start another animation sequence (maybe just repeating one).
        BOOL Ended      = FALSE     ; // TRUE iff the current animation was ended.
        if( !Restart )
        {
            int PreviousFrame = int(Actor.AnimBase);
            Actor.AnimBase += Actor.AnimRate;
            if( int(Actor.AnimBase) > Actor.AnimLast )
            {
                Actor.AnimBase -= Actor.AnimRate; // Restore AnimBase to previous value - just to keep it valid.
                Restart = TRUE;
            }
            else
            {
                ConsiderFrameTrigger = PreviousFrame != int(Actor.AnimBase); // Change in frame?
            }
        }
        if( Restart && Actor.AnimSeq != 0 )
        {
            if( Actor.AnimCount == 1 )
            {
                Ended = TRUE;
            }
            else
            {
                if( Actor.AnimCount > 0 ) { Actor.AnimCount--; }
                // Restart the sequence:
                Actor.AnimBase   = Actor.AnimFirst  ;
                ConsiderFrameTrigger = TRUE;
                Restart = FALSE; // No need for further restarting.
            }
        }
        if( Restart )
        {
            ConsiderFrameTrigger = TRUE;
            Actor.bAnimate = FALSE;
            // We need to find a new animation:
            if( TRUE )
            {
                AActorAI & AI = Actor.AI();
                if( !AI.Animations.IsEmpty() )
                {
                    Actor.bAnimate = TRUE;
                    AI.Animations.MoveNextTo(Actor.Root());
                }
            }
        }
        if( Ended && SavedMessage != ACTOR_Null )
        {
            GLevel->SendMessage( Actor.iMe, SavedMessage, 0 );
        }
        if( ConsiderFrameTrigger && Actor.TriggerSequences[0] != 0 )
        {
            //tbi: Efficiency
            // Set First to index into TriggerSequences of first occurence of AnimSeq.
            BOOL Found = FALSE;
            int First = 0;
            while( !Found && First < arrayCount_(Actor.TriggerSequences) )
            {
                if( Actor.TriggerSequences[First] == Actor.AnimSeq )
                {
                    Found = TRUE;
                }
                else
                {
                    First++;
                }
            }
            if( Found )
            {
                // For each TriggerSequence matching AnimSeq, check the frame:
                BOOL FrameFound = FALSE;
                for( int Which=First; !FrameFound && Which < arrayCount_(Actor.TriggerSequences) && Actor.TriggerSequences[Which] == Actor.AnimSeq ; Which++ )
                {
                    if( Actor.TriggerFrames[Which] == int(Actor.AnimBase) )
                    {
                        FrameFound = TRUE;
                        FActor::Send_FrameTrigger(Actor.iMe,Actor.AnimSeq,Actor.AnimBase,Actor.TriggerValues[Which]);
                    }
                }
            }
        }
        UTexture ** Textures = (UTexture**)Actor.TextureList; //tbi: conversion
        if( Actor.DrawType==DT_Sprite && Textures != 0 )
        {
            Actor.Texture = Textures[int(Actor.AnimBase)];
        }
        //todo: Improve this (duplicated at start of function)...
        if( Actor.AnimLast==0 && Actor.AnimSeq != 0 && Actor.DrawType==DT_MeshMap && Actor.MeshMap != 0 )
        {
            // Determine the last frame from the mesh info.
            UMeshMap  * MeshMap     = Actor.MeshMap  ;
            UMesh     * Mesh        = MeshMap->Mesh  ;
            IMesh       MeshInfo;
            Mesh->GetInfo(&MeshInfo);
            Actor.AnimLast = MeshInfo.AnimSeqs[Actor.AnimSeq].SeqNumFrames-1;
        }
    }
}

//---------------------------------------------------------------------------
//        Find the next inventory item from the specified set.
//---------------------------------------------------------------------------
FActor * FActor::FindInventoryItemFromSet
(
    EInventorySet   WhichSet    // Find item from this set.
)
{
    FActor * Check   = this;
    FActor * Matched = 0; // Set to the next matching item, once it is found.
    while( Check != 0 && Matched==0 )
    {
        AInventory & Item = Check->Inventory();
        if( Item.OwningSet == WhichSet )
        {
            Matched = Check;
        }
        else
        {
            Check = FActor::Handle(Item.iInventory);
        }
    }
    return Matched;
}

//---------------------------------------------------------------------------
//       Activate an inventory item from the actor's inventory set.
//---------------------------------------------------------------------------
FActor * FActor::SelectInventorySet
(
    EInventorySet   WhichSet    // Find item from this set.
,   BOOL            Reuse       // TRUE to reuse the previously active item from the set.
,   BOOL            UseNext     // TRUE to use the item after the previously active item from the set.
)
{
    FActor * Chosen = 0;
    FActor * InventoryList = FActor::Handle(this->iInventory);
    if( InventoryList != 0 )
    {
        FActor * FirstMatch = InventoryList->FindInventoryItemFromSet( WhichSet );
        if( FirstMatch != 0 ) // Is there at least one weapon in the set?
        {
            AInventory & FirstInventoryItem = FirstMatch->Inventory();
            if( Reuse || UseNext )
            {
                // If Reuse: 
                //     We want to find an existing item from inventory set WhichSet which 
                //     was previously in use. We choose that item.
                // If UseNext:
                //     We want to find an existing item from inventory set WhichSet which 
                //     was previously in use. We choose the item after that item (in the same set).
                BOOL Found = FALSE;
                FActor * CheckItem = FirstMatch;
                while( !Found && CheckItem != 0 )
                {
                    AInventory & Check = CheckItem->Inventory();
                    if( Check.bActiveInSet )
                    {
                        Found = TRUE;
                        if( Reuse )
                        {
                            Chosen = CheckItem;
                        }
                        else if( Check.iInventory != INDEX_NONE )
                        {
                            Chosen = FActor::Actor(Check.iInventory).FindInventoryItemFromSet(WhichSet);
                        }
                    }
                    else if( Check.iInventory == INDEX_NONE ) // End of inventory chain?
                    {
                        CheckItem = 0;
                    }
                    else
                    {
                        CheckItem = FActor::Actor( Check.iInventory).FindInventoryItemFromSet(WhichSet);
                    }
                }
            }
            if( Chosen == 0 )
            {
                Chosen = FirstMatch;
            }
            if( Chosen != 0 && Chosen->iMe != this->iWeapon )
            {
                FActor::Send_SwitchInventory(this->iMe,Chosen->iMe);
            }
        }
    }
    return Chosen;
}

//---------------------------------------------------------------------------
//               Vector in direction that actor is looking.
//---------------------------------------------------------------------------
FVector FActor::ViewVector() const
{
    //tbi: Performance. We use GetViewCoordinates for convenience, but
    // that routine does a lot of unnecessary calculations.
    FCoords Coordinates;
    GetViewCoords(&Coordinates); // Leaves Z-axis pointing along the line of sight.
    return Coordinates.ZAxis;
}

//---------------------------------------------------------------------------
//       Vector in direction that actor is oriented (ignores actor's view).
//---------------------------------------------------------------------------
FVector FActor::ForwardVector() const
{
    //tbi: Performance. We use GetDrawCoordinates for convenience, but
    // that routine does a lot of unnecessary calculations.
    FCoords Coordinates;
    GetDrawCoords(&Coordinates); // Leaves Z-axis pointing along the line of sight.
    return Coordinates.ZAxis;
}

//---------------------------------------------------------------------------
//       Vector in direction to right of actor's orientation.
//---------------------------------------------------------------------------
FVector FActor::RightwardVector() const
{
    //tbi: Performance. We use GetDrawCoordinates for convenience, but
    // that routine does a lot of unnecessary calculations.
    FCoords Coordinates;
    GetDrawCoords(&Coordinates);
    return Coordinates.XAxis;
}


//---------------------------------------------------------------------------
//       Vector in upwards direction relative to the actor's orientation.
//---------------------------------------------------------------------------
FVector FActor::UpwardVector() const
{
    //tbi: Performance. We use GetDrawCoordinates for convenience, but
    // that routine does a lot of unnecessary calculations.
    FCoords Coordinates;
    GetDrawCoords(&Coordinates);
    return -Coordinates.YAxis;
}

//---------------------------------------------------------------------------
//                  Is an actor facing a location?
//---------------------------------------------------------------------------
BOOL FActor::IsFacing(const FVector & Location) const
{
    const FVector Orientation   = ForwardVector();
    const FVector Direction     = Location-this->Location;
    return ( Orientation | Direction ) > 0;
}

//---------------------------------------------------------------------------
//                Turn this actor towards the specified location.
//---------------------------------------------------------------------------
void FActor::TurnTowards( const FVector & TargetLocation, int MaxChange )
{
    FActor & Actor = *this;
    FVector Direction = TargetLocation - Actor.Location; // Vector from actor to target.

    // We want to yaw the actor towards the target, if necessary.
    {
        FVector UnitDirection = Direction;
        UnitDirection.Z = 0; // For yawing, we only care about x-y coordinates.
        UnitDirection.Normalize();
        FVector Forward = ForwardVector();
        Forward.Z = 0; // For yawing, we only care about x-y coordinates.
        const FLOAT Cos = Forward|UnitDirection;
        if( Cos <= .97 ) // Are we pointing close enough to the target?
        {
            // We want to turn. To figure out which way to turn, create a direction 
            // vector 90 degrees right of the forward vector. The sign of the dot product 
            // of this new vector with the direction vector tells us which way to turn.
            const FVector Rightward = Actor.RightwardVector();
            const WORD Change = Min( ArcCos(Cos), MaxChange);
            if( ( Rightward | UnitDirection ) > 0.0 )
            {
                Actor.DrawRot.Yaw += Change; // Turn right.
            }
            else
            {
                Actor.DrawRot.Yaw -= Change; // Turn left.
            }
        }
    }

    // We want to pitch the actor towards the target, if appropriate.
    if( FALSE && !Actor.bGravity ) // We use bGravity to indicate "pitchability"
    {
        FVector UnitDirection = Direction;
        UnitDirection.Y = 0; // For pitching, we only care about x-z coordinates.
        UnitDirection.Normalize();
        FVector PitchVector;
        const int Pitch = Actor.DrawRot.Roll; //tbc!! Roll->Pitch
        PitchVector.X = GMath.CosTab(Pitch);
        PitchVector.Y = 0;
        PitchVector.Z = GMath.SinTab(Pitch);
        if( ( PitchVector | UnitDirection ) <= .97 ) //tbi: constants
        {
            // We want to turn. To figure out which way to turn,
            // create a direction vector 90 degrees above
            // the Pitch vector. The sign of the dot product of this
            // new vector with the direction vector tells us 
            // which way to turn.
            const FVector Upward = Actor.UpwardVector();
            if( ( Upward | UnitDirection ) < 0.0 )
            {
                Actor.DrawRot.Roll -= 2000; // Turn up.//tbc!! Roll->Pitch
            }
            else
            {
                Actor.DrawRot.Roll += 2000; // Turn down.//tbc!! Roll->Pitch
            }
        }
    }
}

//---------------------------------------------------------------------------
//                Trigger an actor's event
//---------------------------------------------------------------------------
void FActor::TriggerEvent()
{
    if( EventName != NAME_NONE )
    {
        PTouch Info;
        Info.iActor = iMe;
        GLevel->SendMessageEx(ACTOR_Trigger,&Info,INDEX_NONE,EventName,NULL);
    }
}

//---------------------------------------------------------------------------
//                Teleport the actor to the given target
//---------------------------------------------------------------------------
void FActor::TeleportTo( INDEX iTeleporter )
{
    if( iTeleporter != INDEX_NONE )
    {
        FActor & TeleporterActor = FActor::Actor(iTeleporter);
        ATeleporter & Teleporter = TeleporterActor.Teleporter();
        const char * TargetLocation = Teleporter.TeleportURL;
        const char * Separator = strchr( TargetLocation, '/' );
        if( Separator != 0 ) // Teleportation to a different level?
        {
            FActor::Send_GoToLevel(iMe,TargetLocation);
        }
        else // Teleportation within the level.
        {
            // Find the matching teleporter (or other target):
            INDEX iTarget = TargetLocation[0] != 0 ? INDEX_NONE : iTeleporter;
            for( int WhichActor = 0; iTarget==INDEX_NONE && WhichActor < GLevel->Actors->Max; ++WhichActor )
            {
                AActor & Actor = GLevel->Actors->Element(WhichActor);
                if( stricmp( Actor.Name.Name(), TargetLocation ) == 0 ) 
                {
                    iTarget = WhichActor;
                }
            }
            if( iTarget == INDEX_NONE )
            {
                debugf( LOG_Info, "Unable to find teleport target: %s", TargetLocation );
            }
            else
            {          
                TeleporterActor.TriggerEvent(); // Trigger event associated with teleporter.
                FActor & Target = FActor::Actor(iTarget);
                if( iTarget == iTeleporter || GLevel->FarMoveActor(iMe,&Target.Location) )
                {
                    Send_PostTeleport(iTarget);
                    if( Teleporter.bChangesYaw )
                    {
                        this->ViewRot.Yaw = Teleporter.TargetYaw;
                        this->DrawRot.Yaw = Teleporter.TargetYaw;
                    }
                    if( Teleporter.bChangesVelocity )
                    {
                        this->Velocity = Teleporter.TargetVelocity;
                    }
                    if( Teleporter.bReversesX )
                    {
                        this->Velocity.X = -this->Velocity.X;
                    }
                    if( Teleporter.bReversesY )
                    {
                        this->Velocity.Y = -this->Velocity.Y;
                    }
                    if( Teleporter.bReversesZ )
                    {
                        this->Velocity.Z = -this->Velocity.Z;
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
//              Clear the actor's task.
//---------------------------------------------------------------------------
void FActor::ClearTask(BOOL CancelAnimations)
{ 
    AActorAI & AI = this->AI();
    EmptyAI(CancelAnimations);
    if( CancelAnimations )
    {
        this->CancelAnimations();
    }
    else 
    {
        FinishAnimations();
    }
}

//---------------------------------------------------------------------------
//                 Set a new task.
//---------------------------------------------------------------------------
void FActor::SetTask(EAI_Task Task, int Ticks)
{
    AActorAI & AI = this->AI();
    if( DebugAI )
    {
        debug( LOG_Info, "===>Changing tasks from:" );
        DumpAI();
    }
    AI.Animations.FinishSoon();
    StopTimer();
    if( Ticks != 0 )
    {
        StartTimer(Ticks);
    }
    AITask = Task;
    if( bAnimate )
    {
        AnimCount = 1;
    }
    if( DebugAI )
    {
        debug( LOG_Info, "to:" );
        DumpAI();
    }
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void FActor::DoHarm(int Ticks)   
{ 
    SetTask(EAI_TaskAttack  ,Ticks); 
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void FActor::DoMotion(int Ticks)
{ 
    AActorAI & AI = this->AI();
    SetTask(EAI_TaskMove,Ticks); 
    AI.MotionGoal.Empty(); 
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void FActor::DoSearch(int Ticks)   
{ 
    SetTask(EAI_TaskSearch,Ticks); 
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void FActor::DoWait(int Ticks)
{ 
    SetTask(EAI_TaskWait  ,Ticks); 
}

//---------------------------------------------------------------------------
//                       Add an animation task.
//---------------------------------------------------------------------------
void FActor::AddAnimation(const AIAnimation & Animation)
{
    PrepareForNewAnimation(*this);
    AI().Animations.Add(Animation);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void FActor::GoAlongVector(const FVector & Direction, int Ticks )
{
    AActorAI & AI = this->AI();
    SetTask(EAI_TaskMove,Ticks); 
    AI.MotionGoal.Empty();
    AI.MotionGoal.GoAlongVector(Direction);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void FActor::Near(const FVector & Location, int Ticks )
{
    AActorAI & AI = this->AI();
    SetTask(EAI_TaskMove,Ticks); 
    AI.MotionGoal.Empty(); //tbd? (redundant)
    AI.MotionGoal.GoTo(Location);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void FActor::Near(INDEX iActor, int Ticks )
{
    AActorAI & AI = this->AI();
    SetTask(EAI_TaskMove,Ticks); 
    AI.MotionGoal.Empty(); //tbd? (redundant)
    AI.MotionGoal.GoTo(iActor);
}

//---------------------------------------------------------------------------
//               Send ACTOR_Die to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Die(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Die,0);
    UNGUARD("FActor::Die");
}

//---------------------------------------------------------------------------
//               Send ACTOR_HitWall to an actor.
//---------------------------------------------------------------------------
int FActor::Send_HitWall(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_HitWall,0);
    UNGUARD("FActor::HitWall");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Hit to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Hit(INDEX iActor, PHit & Info)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Hit,&Info);
    UNGUARD("FActor::Hit");
}

//---------------------------------------------------------------------------
//               Send ACTOR_RestartLevel to an actor.
//---------------------------------------------------------------------------
int FActor::Send_RestartLevel(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_RestartLevel,0);
    UNGUARD("FActor::RestartLevel");
}

//---------------------------------------------------------------------------
//               Send ACTOR_GoToLevel to an actor.
//---------------------------------------------------------------------------
int FActor::Send_GoToLevel(INDEX iActor, const char *LevelName)
{
    GUARD;
    PLevel Info;
    Info.Name[0] = 0;
    strncat( Info.Name, LevelName, PLevel::MAX_NAME_LENGTH );
    return GLevel->SendMessage(iActor,ACTOR_GoToLevel,&Info);
    UNGUARD("FActor::GoToLevel");
}

//---------------------------------------------------------------------------
//               Send ACTOR_PostTeleport to an actor.
//---------------------------------------------------------------------------
int FActor::Send_PostTeleport(INDEX iActor, INDEX iTeleporter)
{
    GUARD;
    PTouch Info;
    Info.iActor = iTeleporter;
    return GLevel->SendMessage(iActor,ACTOR_PostTeleport,&Info);
    UNGUARD("FActor::PostTeleport");
}

//---------------------------------------------------------------------------
//               Send ACTOR_PreTeleport to an actor.
//---------------------------------------------------------------------------
int FActor::Send_PreTeleport(INDEX iActor, INDEX iTeleporter)
{
    GUARD;
    PTouch Info;
    Info.iActor = iTeleporter;
    return GLevel->SendMessage(iActor,ACTOR_PreTeleport,&Info);
    UNGUARD("FActor::PreTeleport");
}

//---------------------------------------------------------------------------
//               Send ACTOR_HarmTarget to an actor.
//---------------------------------------------------------------------------
int FActor::Send_HarmTarget(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_HarmTarget,0);
    UNGUARD("FActor::HarmTarget");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Search to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Search(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Search,0);
    UNGUARD("FActor::Search");
}

//---------------------------------------------------------------------------
//               Send ACTOR_UnTarget to an actor.
//---------------------------------------------------------------------------
int FActor::Send_UnTarget(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_UnTarget,0);
    UNGUARD("FActor::UnTarget");
}

//---------------------------------------------------------------------------
//               Send ACTOR_TargetIsNear to an actor.
//---------------------------------------------------------------------------
int FActor::Send_TargetIsNear(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_TargetIsNear,0);
    UNGUARD("FActor::TargetIsNear");
}

//---------------------------------------------------------------------------
//               Send ACTOR_TargetMovedAway to an actor.
//---------------------------------------------------------------------------
int FActor::Send_TargetMovedAway(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_TargetMovedAway,0);
    UNGUARD("FActor::TargetMovedAway");
}

//---------------------------------------------------------------------------
//               Send ACTOR_DoSomething to an actor.
//---------------------------------------------------------------------------
int FActor::Send_DoSomething(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_DoSomething,0);
    UNGUARD("FActor::DoSomething");
}

//---------------------------------------------------------------------------
//               Send ACTOR_AllIsQuiet to an actor.
//---------------------------------------------------------------------------
int FActor::Send_AllIsQuiet(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_AllIsQuiet,0);
    UNGUARD("FActor::AllIsQuiet");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Animate to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Animate(INDEX iActor, PAnimate::TKind Animation)
{
    GUARD;
    PAnimate Info;
    Info.Kind = Animation;
    return GLevel->SendMessage(iActor,ACTOR_Animate,&Info);
    UNGUARD("FActor::Animate");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Release to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Release(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Release,0);
    UNGUARD("FActor::Release");
}

//---------------------------------------------------------------------------
//               Send ACTOR_KillCredit to an actor.
//---------------------------------------------------------------------------
int FActor::Send_KillCredit(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_KillCredit,0);
    UNGUARD("FActor::KillCredit");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Reload to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Reload(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Reload,0);
    UNGUARD("FActor::Reload");
}

//---------------------------------------------------------------------------
//               Send ACTOR_ChooseWeapon to an actor.
//---------------------------------------------------------------------------
int FActor::Send_ChooseWeapon(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_ChooseWeapon,0);
    UNGUARD("FActor::ChooseWeapon");
}

//---------------------------------------------------------------------------
//               Send ACTOR_EndAnimation to an actor.
//---------------------------------------------------------------------------
int FActor::Send_EndAnimation(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_EndAnimation,0);
    UNGUARD("FActor::EndAnimation");
}

//---------------------------------------------------------------------------
//               Send ACTOR_FrameTrigger to an actor.
//---------------------------------------------------------------------------
int FActor::Send_FrameTrigger(INDEX iActor, int Sequence, int Frame, int Trigger )
{
    GUARD;
    PFrame Info;
    Info.Sequence   = Sequence  ;
    Info.Frame      = Frame     ;
    Info.Trigger    = Trigger   ;
    return GLevel->SendMessage(iActor,ACTOR_FrameTrigger,&Info);
    UNGUARD("FActor::FrameTrigger");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Timer to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Timer(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Timer,0);
    UNGUARD("FActor::Timer");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Activate to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Activate(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Activate,0);
    UNGUARD("FActor::Activate");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Use to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Use(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_Use,0);
    UNGUARD("FActor::Send_Use");
}

//---------------------------------------------------------------------------
//               Send ACTOR_UseCloseUp to an actor.
//---------------------------------------------------------------------------
int FActor::Send_UseCloseUp(INDEX iActor, INDEX iTarget)
{
    GUARD;
    PActor Info;
    Info.iActor = iTarget;
    return GLevel->SendMessage(iActor,ACTOR_UseCloseUp,&Info);
    UNGUARD("FActor::UseCloseUp");
}

//---------------------------------------------------------------------------
//               Send ACTOR_UseExtra to an actor.
//---------------------------------------------------------------------------
int FActor::Send_UseExtra(INDEX iActor)
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_UseExtra,0);
    UNGUARD("FActor::Send_UseExtra");
}

//---------------------------------------------------------------------------
//               Send ACTOR_AddInventory to an actor.
//---------------------------------------------------------------------------
int FActor::Send_AddInventory(INDEX iActor, INDEX iInventory)
{
    GUARD;
    PActor Info;
    Info.iActor = iInventory;
    return GLevel->SendMessage(iActor,ACTOR_AddInventory,&Info);
    UNGUARD("FActor::AddInventory");
}

//---------------------------------------------------------------------------
//               Send ACTOR_DeleteInventory to an actor.
//---------------------------------------------------------------------------
int FActor::Send_DeleteInventory(INDEX iActor, INDEX iInventory)
{
    GUARD;
    PActor Info;
    Info.iActor = iInventory;
    return GLevel->SendMessage(iActor,ACTOR_DeleteInventory,&Info);
    UNGUARD("FActor::DeleteInventory");
}

//---------------------------------------------------------------------------
//               Send ACTOR_PickupNotify to an actor.
//---------------------------------------------------------------------------
int FActor::Send_PickupNotify(INDEX iActor, INDEX iPickup)
{
    GUARD;
    PActor Info;
    Info.iActor = iPickup;
    return GLevel->SendMessage(iActor,ACTOR_PickupNotify,&Info);
    UNGUARD("FActor::PickupNotify");
}

//---------------------------------------------------------------------------
//               Send ACTOR_SwitchInventory to an actor.
//---------------------------------------------------------------------------
int FActor::Send_SwitchInventory(INDEX iActor, INDEX iNextInventory )
{
    GUARD;
    PActor Info;
    Info.iActor = iNextInventory;
    return GLevel->SendMessage(iActor,ACTOR_SwitchInventory,&Info);
    UNGUARD("FActor::SwitchInventory");
}

//---------------------------------------------------------------------------
//               Send ACTOR_DeActivate to an actor.
//---------------------------------------------------------------------------
int FActor::Send_DeActivate(INDEX iActor, INDEX iNextInventory )
{
    GUARD;
    PActor Info;
    Info.iActor = iNextInventory;
    return GLevel->SendMessage(iActor,ACTOR_DeActivate,&Info);
    UNGUARD("FActor::Deactivate");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Bump to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Bump(INDEX iActor, INDEX iToucher)
{
    GUARD;
    PTouch Info;
    Info.iActor = iToucher;
    return GLevel->SendMessage(iActor,ACTOR_Bump,&Info);
    UNGUARD("FActor::Bump");
}

//---------------------------------------------------------------------------
//               Send ACTOR_Touch to an actor.
//---------------------------------------------------------------------------
int FActor::Send_Touch(INDEX iActor, INDEX iToucher)
{
    GUARD;
    PTouch Info;
    Info.iActor = iToucher;
    return GLevel->SendMessage(iActor,ACTOR_Touch,&Info);
    UNGUARD("FActor::Touch");
}

//---------------------------------------------------------------------------
//               Send ACTOR_UnTouch to an actor.
//---------------------------------------------------------------------------
int FActor::Send_UnTouch(INDEX iActor, INDEX iToucher)
{
    GUARD;
    PTouch Info;
    Info.iActor = iToucher;
    return GLevel->SendMessage(iActor,ACTOR_UnTouch,&Info);
    UNGUARD("FActor::UnTouch");
}

//---------------------------------------------------------------------------
//               Send ACTOR_TextMsg to an actor.
//---------------------------------------------------------------------------
int FActor::Send_TextMsg(INDEX iActor, const char * Message, BYTE MessageType)
{
    GUARD;
    PText Info;
    Info.MsgType = MessageType;
    strcpy( Info.Message, Message );
    return GLevel->SendMessage(iActor,ACTOR_TextMsg,&Info);
    UNGUARD("FActor::TextMsg");
}


//---------------------------------------------------------------------------
//               Send ACTOR_SensedSomething to an actor.
//---------------------------------------------------------------------------
int FActor::Send_SensedSomething(INDEX iActor, PSense & Info )
{
    GUARD;
    return GLevel->SendMessage(iActor,ACTOR_SensedSomething,&Info);
    UNGUARD("FActor::SensedSomething");
}


