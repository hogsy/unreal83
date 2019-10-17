/*=============================================================================
    UnPlayer.cpp: Player handling routines

    Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
    Compiled with Visual C++ 4.0.

    Revision history:
        * Created by Tim Sweeney
=============================================================================*/

#include "UnGame.h"
#include "UnFActor.h"
#include "UnAction.h"
#include "UnPrefer.h"
#include "UnRandom.h"
#include "UnCheat.h"

//todo: Now that pitch is a DWORD value, we can dispense with 
// TPositiveAngle and TSignedAngle.

typedef WORD TPositiveAngle;
    // Values of TPositiveAngle range from 0 (0 degrees) to 65535 (a little
    // less than 360 degrees). This representation is convenience for 
    // add/subtracting angles, since it automatically "wraps" at 360 degrees
    // back to 0. However, it makes comparisons difficult, because of the
    // discontinuity near 0.
typedef SWORD TSignedAngle;
    // Values of TSignedAngle range from 0 (0 degrees) to 32767 (180 degrees)
    // and from 0 to -32768 (-180 degrees). Sometimes, it is more convenient
    // to work with angles symmetrically centered around 0 like this.
//----------------------------------------------------------------------------
//                  Player noise parameters
//----------------------------------------------------------------------------
static const FLOAT RunNoise  = 1.0;
static const FLOAT MoveNoise = 0.5;

//----------------------------------------------------------------------------
//                  Player movement parameters
//----------------------------------------------------------------------------
// Notes:
//   1. When considering a friction (damping) factor F and an acceleration
//      A, combining these iteratively leads to a velocity V(k) at tick k:
//          V(n+1) = F*V(n) + A
//      This means that:
//          V(infinity) = A/(1-F)

static const FLOAT RightwardFactor = 0.8; // Rightward motions are generally this fraction of forward motions.

static const FLOAT VelocitySurfaceDamping     = 0.90  ; // Velocity of pawn on surfaces is damped by this factor.
static const FLOAT VelocityFlyDamping         = 0.97  ; // Velocity of flying pawn in air is damped by this factor.
static const FLOAT VelocityAirDamping         = 0.999 ; // Velocity of non-flying pawn in air is damped by this factor.
static const FLOAT UsualForwardAcceleration   = 0.50; // The usual forward or backward acceleration.
static const FLOAT UsualRightwardAcceleration = RightwardFactor*UsualForwardAcceleration; // The usual rightward or leftward acceleration.
static const FLOAT UsualUpwardAcceleration    = 0.40; // The usual upwards or downward acceleration.


static const TPositiveAngle MaxPitchSpeed   = 0x600; 
static const TPositiveAngle LevelPitch      = 0x000;
static const int PitchAccelerationRatio = 8; // Value x where pitch is accelerated by P/x, where P is the change in pitch.
static const TSignedAngle PitchLimit = 0x3400 ; // About 73 degrees upward/downward limit on pitch.
// The change in pitch for changes in a differential analog movement.
#define DifferentialPitchChange (PositiveAngle(PitchLimit)/4)

static const FLOAT DifferentialMoveFactor = 20.0; //tbe:


static const FLOAT StopThresholdSpeed = 0.04; // Speeds below this are set to 0.

//
// Rotation parameters
//
#define PLAYER_ROLL_SNAPBACK_RATIO  4
#define PLAYER_MAX_YAW_SPEED        0x2C0
#define PLAYER_YAW_ACCELERATION     0x78
#define PLAYER_PITCH_ACCELERATION   0x50
#define PLAYER_PITCH_ZERO           0x20
#define PLAYER_ROLL_ZERO            0x20


//
// Eye parameters
//
#define PLAYER_WALK_BOB_HEIGHT      4.5
#define PLAYER_MAX_BOB_HEIGHT       6.0
#define PLAYER_BOB_PER_SEC          1.0
//tbd:#define EYEHEIGHT_DECELERATION_CF   0.21    /* Deceleration coefficient of eye going up/down stairs */
#define EYEHEIGHT_DECELERATION_CF   0.10     /* Deceleration coefficient of eye going up/down stairs */
#define STILL_BOB_HEIGHT            2.0        /* Changed TS 5-5. Still bobbing sucks */
#define PLAYER_CROUCH_HEIGHT       -20.0

//
// Automatic view adjustment on stairs/inclines
//
#define CANE_LENGTH                    96.0
#define MIN_STAIR_SLOPE                0.20
#define MAX_STAIR_SLOPE                1.0
#define PLAYER_STAIR_UP_PITCH         TPositiveAngle(-0x0C00)
#define PLAYER_STAIR_DOWN_PITCH       TPositiveAngle(+0x0C00)

//
// Ramps and stairs
//
#define RAMP_MAX_Z_NORM             0.90    /* Player should slide down ramps in this range */
#define RAMP_MIN_Z_NORM             0.10    /* Something sloped at this grade or lower is a wall */
#define MAX_FALLING_Z_SPEED           30.0  /* Prevents player from falling down too fast */
#define MAX_SMOOTH_STAIR_DIST         25.0    /* Let player climb stairs and descend smoothly */

static const FLOAT LungeStaminaThreshold = 0.10     ; // The minimum amount of stamina to allow a lunge:
static const FLOAT StaminaDecayRate      = 0.015    ; // Amount stamina decays each tick while lunging.
static const FLOAT StaminaRechargeRate   = 0.005    ; // Amount stamina recharges each tick not lunging.

static inline FLOAT Min(FLOAT Value1, FLOAT Value2)
{
    return Value1 <= Value2 ? Value1 : Value2;
}

static inline FLOAT Max(FLOAT Value1, FLOAT Value2)
{
    return Value1 >= Value2 ? Value1 : Value2;
}

static inline int Min(int Value1, int Value2)
{
    return Value1 <= Value2 ? Value1 : Value2;
}

static inline int Max(int Value1, int Value2)
{
    return Value1 >= Value2 ? Value1 : Value2;
}


static inline SWORD SignedAngle(TPositiveAngle Angle)
{
    return TSignedAngle(Angle);
}

static inline WORD PositiveAngle(TSignedAngle Angle)
{
    return TPositiveAngle(Angle);
} 

static const TSignedAngle OneDegree  = 65536/360;
static const TSignedAngle HalfDegree = OneDegree/2;

//tbm: Convert degrees to WORD angle measurements.
static inline int Degrees(int Degrees)
{
    return Degrees*65536/360;
}

//tbm: Convert degrees to WORD angle measurements.
static inline TPositiveAngle PositiveDegrees(int Degrees)
{
    return Degrees*65536/360;
}


//tbd: This is temporary until we have a status bar and full status indicators (invisibility...)
// Return an indicator for the status of a special ability.
// Return a blank character if the ability is not active.
// If the ability is active, return the given indicator, except
// if the ability is almost gone, in which case return alternating
// values (blank, indicator).
static char * abilityStatus( char Indicator, BOOL IsActive, int TimeLeft )
{
    static char Text[5]; // Cheap hack - use of static string.
    strcpy( Text, " " );
    if( IsActive && (TimeLeft >= 5*35 || (GServer.Ticks&0x04)==0 ) )
    {
        Text[0] = Indicator;
    }
    return Text;
}

static void ShowPlayerStat( const APawn & Pawn ) //tbd: This is temporary until we have a status bar.
{
    if( FALSE && GInput.IsToggledOn( FInput::S_ScrollLock ) )
    {
        static char LastStatusText[100] = { 0 };
        char StatusText[100];
        char * Text = StatusText;
        Text += sprintf( Text, "Health: %3i%%%%", int(Pawn.Health) );
        Text += sprintf
        ( 
            Text
        ,   " Armor( %i %i %i %i )"
        ,   int(Pawn.Armor[0])
        ,   int(Pawn.Armor[1])
        ,   int(Pawn.Armor[2])
        ,   int(Pawn.Armor[3])
        );
        // Ammo:
        {
                Text += sprintf( Text, " Ammo( %i %i %i %i )", Pawn.AmmoCount[0], Pawn.AmmoCount[1], Pawn.AmmoCount[2], Pawn.AmmoCount[3] );
        }
        // Kills:
        Text += sprintf( Text, " Kill: %i", Pawn.KillCount );
        // Special abilities:
        {
            strcat( Text, " (" );
            strcat( Text, abilityStatus( 'I', Pawn.bHasInvisibility, Pawn.InvisibilityTimeLeft ) );
            strcat( Text, abilityStatus( 'S', Pawn.bHasSilence, Pawn.SilenceTimeLeft ) );
            strcat( Text, abilityStatus( 'C', Pawn.bHasInvincibility, Pawn.InvincibilityTimeLeft ) );
            strcat( Text, abilityStatus( 'R', Pawn.bHasSuperStrength, Pawn.SuperStrengthTimeLeft ) );
            strcat( Text, abilityStatus( 'M', Pawn.bHasSuperStamina, Pawn.SuperStaminaTimeLeft ) );
            strcat( Text, ")" );
        }
        if( strcmp( StatusText, LastStatusText ) != 0 )
        {
            debugf( LOG_Info, StatusText );
            strcpy( LastStatusText, StatusText );
        }
    }
}

static void LimitPitch( TPositiveAngle & Pitch )
{
    if( Pitch <= PositiveDegrees(180) )
    {
        Pitch = Min( Pitch, PitchLimit );
    }
    else
    {
        Pitch = Max( Pitch, PositiveAngle(-PitchLimit) );
    }
}

static void LimitPitch( int & Pitch )
{
    TPositiveAngle AdjustedPitch = Pitch;
    LimitPitch( AdjustedPitch );
    Pitch = AdjustedPitch;
}




static inline BOOL IsActivated( FAction::TActionStatus Status )
{
    return (Status & (FAction::IsActiveStatus|FAction::WasActiveStatus)) == FAction::IsActiveStatus;
}

void FGame::PlayerYaw( APawn & Pawn, FLOAT Analog, FLOAT Differential )
{
    if( Analog != 0 )
    {
        Pawn.ViewRot.Yaw += Analog * Degrees(3);
    }
    Pawn.ViewRot.Yaw += Differential * Degrees(20);
}

void FGame::PlayerPitch( APawn & Pawn, FLOAT Analog, FLOAT Differential )
{
    if( Analog != 0 )
    {
        Pawn.TargetPitch = TPositiveAngle( Analog*PitchLimit );
    }
    Pawn.ViewRot.Pitch += TPositiveAngle( Differential * DifferentialPitchChange );
    Pawn.TargetPitch   += TPositiveAngle( Differential * DifferentialPitchChange );
    if( Pawn.bLimitRotation )
    {
        LimitPitch( Pawn.TargetPitch );
        LimitPitch( Pawn.ViewRot.Pitch );
    }
}

/*-----------------------------------------------------------------------------
    Standard player controls
-----------------------------------------------------------------------------*/

//
// Actor function to handle all generic player stuff. Called by actorPawn when
// pawn is possessed with player.
//
int FGame::PlayerTick(INDEX iActor, void *Params)
{
    //tbi? Performance?

    GUARD;
    ILevel            & Level       = *GLevel;
    IModel            & Model       = Level.ModelInfo;
    FActor            & Actor       = FActor::Actor(iActor);
    APawn             & Pawn        = Actor.Pawn();
    PPlayerTick       & PlayerInfo  = PPlayerTick::Convert(Params);
    FVector             Floor;
    FVector             MoveVector;
    FVector             Gravity = GLevel->GetZoneGravityAcceleration(iActor);
    FLOAT               DZ;
    FLOAT               RunMult             = 1.0   ;
    const BOOL          IsDead = Pawn.LifeState != LS_Alive;

    // Here are the two primary motion components that we build in this function.
    FLOAT               ForwardAcceleration         = 0.0;
    FLOAT               RightwardAcceleration       = 0.0;
    FLOAT               UpwardAcceleration          = 0.0;
    FLOAT               ForwardAnalogAcceleration   = 0.0;
    FLOAT               RightwardAnalogAcceleration = 0.0;
    FLOAT               UpwardAnalogAcceleration    = 0.0;
    FLOAT               TargetForwardSpeed          = 0.0;
    FLOAT               TargetRightwardSpeed        = 0.0;
    FLOAT               TargetUpwardSpeed           = 0.0;

    FLOAT               BobAmplitude                ;
    SWORD               YawAccel            = 0     ;
//tbd: [obsolete due to new rotation handling]    SWORD               AnalogYawSpeed      = 0     ;
    SWORD               AnalogRollSpeed     = 0     ;

    const FLOAT VelocityDamping = 
        Pawn.bIsOnSurface   ? VelocitySurfaceDamping 
    :   !Pawn.bGravity      ? VelocityFlyDamping
    :                         VelocityAirDamping
    ;

    Pawn.Velocity *= VelocityDamping;

    typedef FAction::TActionStatus TActionStatus;
    TActionStatus * Actions = &PlayerInfo.Actions[0];
    
    #if 0 // Zone debugging code
    INDEX iTemp = Level.GetZoneDescriptor(Level.GetActorZone(iActor));
    if (iTemp!=INDEX_NONE)
        {        
        AZoneDescriptor *Temp = (AZoneDescriptor *)&Level.Actors->Element(iTemp);
        PText Msg;
        Msg.MsgType = LOG_Play;
        sprintf(Msg.Message,"Zone %s",Temp->ZoneTitle);
        Level.SendMessage(iActor,ACTOR_TextMsg,&Msg);
        }
	#endif
    
    // *** Warning *** 
    // Be careful about the order of processing actions, since 
    // some actions affect the behaviour of other actions.


    const BOOL IsCrouching      = Pawn.bGravity && FAction::CheckStatus( Actions[FAction::Crouch], FAction::IsActiveStatus );
    const FLOAT MovementFactor   = IsCrouching ? 0.6 : 1.0;

    const FLOAT NormalSpeed = MovementFactor * Pawn.NormalSpeed   ;
    const FLOAT RunSpeed    = MovementFactor * Pawn.RunSpeed      ;
    const FLOAT LungeSpeed  = MovementFactor * Pawn.LungeSpeed    ;
    FLOAT MaxSpeed = NormalSpeed;

    const FLOAT DefaultForwardAcceleration   = UsualForwardAcceleration   * MovementFactor;
    const FLOAT DefaultRightwardAcceleration = UsualRightwardAcceleration * MovementFactor;
    const FLOAT DefaultUpwardAcceleration = UsualUpwardAcceleration * MovementFactor;

    const SWORD TurnYawAcceleration = IsCrouching ? PLAYER_YAW_ACCELERATION / 2 : PLAYER_YAW_ACCELERATION;
    const SWORD SpinYawAcceleration = 2*TurnYawAcceleration;
    const SWORD MaxTurnYawSpeed = IsCrouching ? PLAYER_MAX_YAW_SPEED / 2 : PLAYER_MAX_YAW_SPEED;
    const SWORD MaxSpinYawSpeed = 2*MaxTurnYawSpeed;
    SWORD MaxYawSpeed = MaxTurnYawSpeed;

    const TPositiveAngle MaxPitchSpeed = IsCrouching ? ::MaxPitchSpeed / 2 : ::MaxPitchSpeed;
    BOOL PlayerChoosesPitch = !Pawn.bGravity && !IsDead; 
    BOOL Lunging = FALSE;
    //------------------------------------------------------
    //                        Notes
    //------------------------------------------------------
    //  1. Pawn.DesiredSpeed will be the desired speed, the speed at
    //     which the pawn "wants" to move. The actual speed we use may be
    //     less or more, depending on other influences. Pawn.DesiredSpeed is
    //     reset each time there is a change in the motion speed (start moving,
    //     stop moving, start running, stop running, etc), but it is kept
    //     while maintaining a particular motion so we remember how fast
    //     we want to go. If Pawn.DesiredSpeed==0, this has a special meaning:
    //     it means to use the speed appropriate to the current actions (such
    //     as running).

    GCheat->DoAdjustments(Actor.iMe);

    //---------------------------------------
    //  LungeForward/RunForward/MoveForward
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Lunge  = Actions[FAction::LungeForward ];
        const TActionStatus & Run    = Actions[FAction::RunForward   ];
        const TActionStatus & Move   = Actions[FAction::MoveForward  ];
        if( FAction::CheckStatus( Lunge, FAction::IsActiveStatus ) )
        {
            Lunging = TRUE; 
            if( Pawn.Stamina >= LungeStaminaThreshold )
            {
                ForwardAcceleration += 2*DefaultForwardAcceleration;
                MaxSpeed = LungeSpeed;
            }
            Pawn.DesiredSpeed = 0;
        }
        else 
        {
            if( FAction::CheckStatus( Run, FAction::IsActiveStatus ) )
            {
                MaxSpeed = Max(MaxSpeed,RunSpeed);
                if( IsActivated(Run) )
                {
                    Pawn.DesiredSpeed = MaxSpeed;
                }
                Pawn.Noise += RunNoise;
                if( FAction::CheckStatus( Run, FAction::BySwitchLikeStatus ) )
                {
                    ForwardAcceleration += 2.0*DefaultForwardAcceleration;
                }
            }
            if( FAction::CheckStatus( Move, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += MoveNoise;
                if( IsActivated(Move) )
                {
                    Pawn.DesiredSpeed = MaxSpeed;
                }
                if( FAction::CheckStatus( Move, FAction::BySwitchLikeStatus ) )
                {
                    ForwardAcceleration += DefaultForwardAcceleration;
                }
            }
            else
            {
                Pawn.DesiredSpeed = 0;
            }
        }
    }

    //---------------------------------------
    // LungeBackward/RunBackward/MoveBackward
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Lunge  = Actions[FAction::LungeBackward ];
        const TActionStatus & Run    = Actions[FAction::RunBackward   ];
        const TActionStatus & Move   = Actions[FAction::MoveBackward  ];
        if( FAction::CheckStatus( Lunge, FAction::IsActiveStatus ) )
        {
            Lunging = TRUE; 
            if( Pawn.Stamina >= 2*LungeStaminaThreshold )
            {
                ForwardAcceleration -= 2.0*DefaultForwardAcceleration;
                MaxSpeed = LungeSpeed;
            }
        }
        else 
        {
            if( FAction::CheckStatus( Run, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += RunNoise;
                MaxSpeed = RunSpeed;
                if( FAction::CheckStatus( Run, FAction::BySwitchLikeStatus ) )
                {
                    ForwardAcceleration -= 2.0*DefaultForwardAcceleration;
                }
            }
            if( FAction::CheckStatus( Move, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += MoveNoise;
                if( FAction::CheckStatus( Move, FAction::BySwitchLikeStatus ) )
                {
                    ForwardAcceleration -= DefaultForwardAcceleration;
                }
            }
        }
    }

    {
        const FLOAT & Analog        = PlayerInfo.Movements[PlayerAxis_Forward].Analog;
        const FLOAT & Differential  = PlayerInfo.Movements[PlayerAxis_Forward].Differential;
        if( Analog != 0 )
        {
            Pawn.DesiredSpeed = 0.0;
            // Use the magnitude as a fraction of the maximum speed
            TargetForwardSpeed = Analog*MaxSpeed;
        }
        if( Differential != 0.0 )
        {
            Pawn.DesiredSpeed = 0.0;
            // Use the magnitude as a change in acceleration:
            ForwardAcceleration += Differential*DifferentialMoveFactor*DefaultForwardAcceleration; 
        }
    }

    //---------------------------------------
    //     LungeLeft/RunLeft/MoveLeft
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Lunge  = Actions[FAction::LungeLeft ];
        const TActionStatus & Run    = Actions[FAction::RunLeft   ];
        const TActionStatus & Move   = Actions[FAction::MoveLeft  ];
        if( FAction::CheckStatus( Lunge, FAction::IsActiveStatus ) )
        {
            Lunging = TRUE; 
            if( Pawn.Stamina >= LungeStaminaThreshold )
            {
                RightwardAcceleration -= 2*DefaultRightwardAcceleration;
                MaxSpeed = LungeSpeed;
            }
        }
        else 
        {
            if( FAction::CheckStatus( Run, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += RunNoise;
                MaxSpeed = Max(MaxSpeed,RunSpeed);
                RightwardAcceleration -= 2.0*DefaultRightwardAcceleration;
            }
            if( FAction::CheckStatus( Move, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += MoveNoise;
                RightwardAcceleration -= DefaultRightwardAcceleration;
            }
        }
    }

    //---------------------------------------
    //     LungeRight/RunRight/MoveRight
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Lunge  = Actions[FAction::LungeRight ];
        const TActionStatus & Run    = Actions[FAction::RunRight   ];
        const TActionStatus & Move   = Actions[FAction::MoveRight  ];
        if( FAction::CheckStatus( Lunge, FAction::IsActiveStatus ) )
        {
            Lunging = TRUE; 
            if( Pawn.Stamina >= LungeStaminaThreshold )
            {
                RightwardAcceleration += 2.0*DefaultRightwardAcceleration;
                MaxSpeed = LungeSpeed;
            }
        }
        else 
        {
            if( FAction::CheckStatus( Run, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += RunNoise;
                MaxSpeed = Max(MaxSpeed,RunSpeed);
                RightwardAcceleration += 2.0*DefaultRightwardAcceleration;
            }
            if( FAction::CheckStatus( Move, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += MoveNoise;
                RightwardAcceleration += DefaultRightwardAcceleration;
            }
        }
    }

    {
        const FLOAT & Analog        = PlayerInfo.Movements[PlayerAxis_Rightward].Analog;
        const FLOAT & Differential  = PlayerInfo.Movements[PlayerAxis_Rightward].Differential;
        if( Analog != 0 )
        {
            TargetRightwardSpeed = Analog*MaxSpeed; // Use the magnitude as a fraction of the maximum speed.
        }
        RightwardAcceleration += Differential*DifferentialMoveFactor*DefaultRightwardAcceleration;
    }

    //---------------------------------------
    //        LungeUp/RunUp/MoveUp
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Lunge  = Actions[FAction::LungeUp ];
        const TActionStatus & Run    = Actions[FAction::RunUp   ];
        const TActionStatus & Move   = Actions[FAction::MoveUp  ];
        if( Pawn.bGravity )
        {
            // The pawn can't fly, so he can't move up.
        }
        else if( FAction::CheckStatus( Lunge, FAction::IsActiveStatus ) )
        {
            Lunging = TRUE; 
            if( Pawn.Stamina >= LungeStaminaThreshold )
            {
                UpwardAcceleration += 2.0*DefaultUpwardAcceleration;
                MaxSpeed = LungeSpeed;
            }
        }
        else
        {
            if( FAction::CheckStatus( Run, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += RunNoise;
                MaxSpeed = Max(MaxSpeed,RunSpeed);
                UpwardAcceleration += 2.0*DefaultUpwardAcceleration;
            }
            if( FAction::CheckStatus( Move, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += MoveNoise;
                UpwardAcceleration += DefaultUpwardAcceleration;
            }
        }
    }


    //---------------------------------------
    //      LungeDown/RunDown/MoveDown
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Lunge  = Actions[FAction::LungeDown ];
        const TActionStatus & Run    = Actions[FAction::RunDown   ];
        const TActionStatus & Move   = Actions[FAction::MoveDown  ];
        if( Pawn.bGravity )
        {
            // The pawn can't fly, so he can't move down.
        }
        else if( FAction::CheckStatus( Lunge, FAction::IsActiveStatus ) )
        {
            Lunging = TRUE; 
            if( Pawn.Stamina >= LungeStaminaThreshold )
            {
                UpwardAcceleration -= 2.0*DefaultUpwardAcceleration;
                MaxSpeed = LungeSpeed;
            }
        }
        else 
        {
            if( FAction::CheckStatus( Run, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += RunNoise;
                MaxSpeed = Max(MaxSpeed,RunSpeed);
                UpwardAcceleration -= 2.0*DefaultUpwardAcceleration;
            }
            if( FAction::CheckStatus( Move, FAction::IsActiveStatus ) )
            {
                Pawn.Noise += MoveNoise;
                UpwardAcceleration -= DefaultUpwardAcceleration;
            }
        }
    }

    {
        const FLOAT & Analog        = PlayerInfo.Movements[PlayerAxis_Upward].Analog;
        const FLOAT & Differential  = PlayerInfo.Movements[PlayerAxis_Upward].Differential;
        if( Analog != 0 )
        {
            TargetUpwardSpeed = Analog*MaxSpeed; // Use the magnitude as a fraction of the maximum speed.
        }
        UpwardAcceleration += Differential*DifferentialMoveFactor*DefaultUpwardAcceleration;
    }

    //---------------------------------------
    //      SpinLeft/TurnLeft
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Spin = Actions[FAction::SpinLeft ];
        const TActionStatus & Turn = Actions[FAction::TurnLeft ];
        if( FAction::CheckStatus( Spin, FAction::IsActiveStatus ) )
        {
            if( FAction::CheckStatus( Spin, FAction::BySwitchLikeStatus ) )
            {
                YawAccel -= SpinYawAcceleration;
            }
            MaxYawSpeed = MaxSpinYawSpeed;
        }
        if( FAction::CheckStatus( Turn, FAction::IsActiveStatus ) )
        {
            if( FAction::CheckStatus( Turn, FAction::BySwitchLikeStatus ) )
            {
                YawAccel -= TurnYawAcceleration;
            }
        }
    }

    //---------------------------------------
    //     SpinRight/TurnRight
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Spin = Actions[FAction::SpinRight ];
        const TActionStatus & Turn = Actions[FAction::TurnRight ];
        if( FAction::CheckStatus( Spin, FAction::IsActiveStatus ) )
        {
            if( FAction::CheckStatus( Spin, FAction::BySwitchLikeStatus ) )
            {
                YawAccel += SpinYawAcceleration;
            }
            MaxYawSpeed = MaxSpinYawSpeed;
        }
        if( FAction::CheckStatus( Turn, FAction::IsActiveStatus ) )
        {
            if( FAction::CheckStatus( Turn, FAction::BySwitchLikeStatus ) )
            {
                YawAccel += TurnYawAcceleration;
            }
        }
    }

    {
        const FLOAT & Analog        = PlayerInfo.Movements[PlayerAxis_Yaw].Analog;
        const FLOAT & Differential  = PlayerInfo.Movements[PlayerAxis_Yaw].Differential;
        GGame.PlayerYaw( Pawn, Analog, Differential );
//tbd: [obsolete due to new rotation handling]
#if 0
        if( Analog != 0 )
        {
            AnalogYawSpeed = Analog * Degrees(3);
        }
        AnalogYawSpeed += Differential * Degrees(30);
#endif
    }

    //---------------------------------------
    //      SpinUp/TurnUp
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Spin = Actions[FAction::SpinUp ];
        const TActionStatus & Turn = Actions[FAction::TurnUp ];
        if( Pawn.bGravity )
        {
            // Do nothing - pawn is not flying.
        }
        else if( FAction::CheckStatus( Spin, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
        else if( FAction::CheckStatus( Turn, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
    }

    //---------------------------------------
    //      SpinDown/TurnDown
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Spin = Actions[FAction::SpinDown ];
        const TActionStatus & Turn = Actions[FAction::TurnDown ];
        if( Pawn.bGravity )
        {
            // Do nothing - pawn is not flying.
        }
        else if( FAction::CheckStatus( Spin, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
        else if( FAction::CheckStatus( Turn, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
    }

    //---------------------------------------
    //          LookUp
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Look = Actions[FAction::LookUp];
        if( FALSE && !Pawn.bGravity ) //todo: ignore action if flying
        {
            // Do nothing - pawn is flying.
        }
        else if( FAction::CheckStatus( Look, FAction::IsActiveStatus ) )
        {
            if( FAction::CheckStatus( Look, FAction::BySwitchLikeStatus ) )
            {
                Pawn.TargetPitch = Pawn.ViewRot.Pitch + TPositiveAngle( Degrees(5) );
            }
            if( Pawn.bLimitRotation )
            {
                LimitPitch( Pawn.TargetPitch );
            }
            PlayerChoosesPitch = TRUE;
        }
        else if( FAction::CheckStatus( Look, FAction::IsReadyStatus )  )
        {
            PlayerChoosesPitch = TRUE;
        }
        else if( FAction::CheckStatus( Actions[FAction::MotionLookShift], FAction::IsActiveStatus ) )
        {
            PlayerChoosesPitch = TRUE;
        }
    }

    //---------------------------------------
    //          LookDown
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Look = Actions[FAction::LookDown];
        if( FALSE && !Pawn.bGravity ) //todo: ignore action if flying
        {
            // Do nothing - pawn is flying.
        }
        else if( FAction::CheckStatus( Look, FAction::IsActiveStatus ) )
        {
            if( FAction::CheckStatus( Look, FAction::BySwitchLikeStatus ) )
            {
                Pawn.TargetPitch = Pawn.ViewRot.Pitch - TPositiveAngle( Degrees(5) );
            }
            if( Pawn.bLimitRotation )
            {
                LimitPitch( Pawn.TargetPitch );
            }
            PlayerChoosesPitch = TRUE;
        }
        else if( FAction::CheckStatus( Look, FAction::IsReadyStatus ) )
        {
            PlayerChoosesPitch = TRUE;
        }
        else if( FAction::CheckStatus( Actions[FAction::MotionLookShift], FAction::IsActiveStatus ) )
        {
            PlayerChoosesPitch = TRUE;
        }
    }

    {
        const FLOAT & Analog        = PlayerInfo.Movements[PlayerAxis_Pitch].Analog;
        const FLOAT & Differential  = PlayerInfo.Movements[PlayerAxis_Pitch].Differential;
        GGame.PlayerPitch( Pawn, Analog, Differential );
#if 0 //tbd: obsolete 
        if( Analog != 0 )
        {
            Pawn.TargetPitch = TPositiveAngle( Analog*PitchLimit );
        }
        Pawn.TargetPitch += TPositiveAngle( Differential * DifferentialPitchChange );
#endif
    }

    //---------------------------------------
    //              WeaponSwap
    //---------------------------------------
    if( !Pawn.bCannotMove && IsActivated(Actions[FAction::WeaponSwap]) )
    {
        const INDEX iWeapon1 = Pawn.iRecentWeapons[0]; // Most recent weapon.
        const INDEX iWeapon2 = Pawn.iRecentWeapons[1]; // Second most recent weapon.
        if( Pawn.iWeapon == INDEX_NONE )
        {
            // The player is not holding a weapon, so switch to the
            // most recent one.
            FActor::Send_SwitchInventory(iActor,iWeapon1);
        }
        // The most recent weapon should logically be the current one,
        // but we'll handle things more generally.
        else if( iWeapon1 != Pawn.iWeapon && iWeapon1 != INDEX_NONE )
        {
            FActor::Send_SwitchInventory(iActor,iWeapon1);
        }
        else if( iWeapon2 != Pawn.iWeapon && iWeapon2 != INDEX_NONE )
        {
            FActor::Send_SwitchInventory(iActor,iWeapon2);
        }
    }

    //---------------------------------------
    //              WeaponNext
    //---------------------------------------
    if( !Pawn.bCannotMove && IsActivated(Actions[FAction::WeaponNext]) )
    {
        //tbi: Move this to a helper function.
        const EInventorySet CurrentSet =
            Pawn.iWeapon != INDEX_NONE         ? EInventorySet(FActor::Inventory(Pawn.iWeapon).OwningSet)
        :   Pawn.iRecentWeapons[0]!=INDEX_NONE ? EInventorySet(FActor::Inventory(Pawn.iRecentWeapons[0]).OwningSet)
        :   INV_NoSet
        ;
        // Find an inventory item whose set is the smallest one
        // greater than the current set. Also keep track of the smallest
        // set from which the player has an item, in case there is no "next" set.
        INDEX iCheck = Pawn.iInventory;
        EInventorySet BestSet; // Best set so far, defined only if iBest!=INDEX_NONE.
        BOOL iBest = INDEX_NONE; // Best item found so far, or INDEX_NONE.
        EInventorySet LowestSet; // Lowest set so far, defined only if iLowest!=INDEX_NONE.
        BOOL iLowest = INDEX_NONE; // Lowest item found so far, or INDEX_NONE.
        while( iCheck != INDEX_NONE )
        {
            AInventory & Item = FActor::Inventory(iCheck);
            const EInventorySet CheckSet =  EInventorySet(Item.OwningSet);
            if( iLowest == INDEX_NONE || CheckSet < LowestSet )
            {
                iLowest = iCheck;
                LowestSet = CheckSet;
            }
            if( CheckSet > CurrentSet )
            {
                // Is this item better than the previous "best" one?
                if( iBest==INDEX_NONE || CheckSet < BestSet )
                {
                    iBest = iCheck;
                    BestSet = CheckSet;
                }
            }
            iCheck = Item.iInventory;
        }
        const EInventorySet NewSet =
            iBest != INDEX_NONE     ? BestSet
        :   iLowest != INDEX_NONE   ? LowestSet
        :   INV_NoSet
        ;
        if( NewSet != INV_NoSet && ( NewSet != CurrentSet || Pawn.iWeapon==INDEX_NONE )  )
        {
            Actor.SelectInventorySet(NewSet,FALSE,FALSE);
        }
    }

    //---------------------------------------
    //              WeaponPrevious
    //---------------------------------------
    if( !Pawn.bCannotMove && IsActivated(Actions[FAction::WeaponPrevious]) )
    {
        //tbi: Move this to a helper function.
        const EInventorySet CurrentSet =
            Pawn.iWeapon != INDEX_NONE         ? EInventorySet(FActor::Inventory(Pawn.iWeapon).OwningSet)
        :   Pawn.iRecentWeapons[0]!=INDEX_NONE ? EInventorySet(FActor::Inventory(Pawn.iRecentWeapons[0]).OwningSet)
        :   INV_NoSet
        ;
        // Find an inventory item whose set is the largest one
        // greater than the current set. Also keep track of the largest
        // set from which the player has an item, in case there is no "previous" set.
        INDEX iCheck = Pawn.iInventory;
        EInventorySet BestSet; // Best set so far, defined only if iBest!=INDEX_NONE.
        BOOL iBest = INDEX_NONE; // Best item found so far, or INDEX_NONE.
        EInventorySet HighestSet; // Highest set so far, defined only if iLowest!=INDEX_NONE.
        BOOL iHighest = INDEX_NONE; // Highest item found so far, or INDEX_NONE.
        while( iCheck != INDEX_NONE )
        {
            AInventory & Item = FActor::Inventory(iCheck);
            const EInventorySet CheckSet =  EInventorySet(Item.OwningSet);
            if( iHighest == INDEX_NONE || CheckSet > HighestSet )
            {
                iHighest = iCheck;
                HighestSet = CheckSet;
            }
            if( CheckSet < CurrentSet )
            {
                // Is this item better than the previous "best" one?
                if( iBest==INDEX_NONE || CheckSet > BestSet )
                {
                    iBest = iCheck;
                    BestSet = CheckSet;
                }
            }
            iCheck = Item.iInventory;
        }
        const EInventorySet NewSet =
            iBest != INDEX_NONE     ? BestSet
        :   iHighest != INDEX_NONE  ? HighestSet
        :   INV_NoSet
        ;
        if( NewSet != INV_NoSet && ( NewSet != CurrentSet || Pawn.iWeapon==INDEX_NONE )  )
        {
            Actor.SelectInventorySet(NewSet,FALSE,FALSE);
        }
    }

    //---------------------------------------
    //              WeaponReady
    //---------------------------------------
    if( !Pawn.bCannotMove && IsActivated(Actions[FAction::WeaponReady]) )
    {
        if( Pawn.iWeapon != INDEX_NONE )
        {
            // Lower the weapon
            FActor::Send_SwitchInventory( iActor, INDEX_NONE );
        }
        else if( Pawn.iRecentWeapons[0] != INDEX_NONE )
        {
            // Raise the previous weapon
            FActor::Send_SwitchInventory( iActor, Pawn.iRecentWeapons[0] );
        }
    }

    //---------------------------------------
    //    WeaponSet1, ..., WeaponSet10    
    //---------------------------------------
    
    // Select the highest weapon selection (for which the player has a weapon).
    if( !Pawn.bCannotMove )
    {
        BOOL WeaponSelected = FALSE;
        for( int Which = FAction::WeaponSet10; !WeaponSelected && Which >= FAction::WeaponSet1; --Which )
        {
            const EInventorySet Set = EInventorySet( Which-FAction::WeaponSet1 + INV_WeaponSet1 );
            const FAction::TAction Action = FAction::TAction(Which);
            if( IsActivated(Actions[Action])  )
            {
                const INDEX iCurrent = Pawn.iWeapon;
                const BOOL UseCurrentSet = // TRUE iff the chosen inventory set holds the current active item.
                    iCurrent != INDEX_NONE
                &&  FActor::Inventory(  iCurrent ).OwningSet == Set
                ;
                WeaponSelected = 
                    Actor.SelectInventorySet( Set, !UseCurrentSet, UseCurrentSet )
                !=  0
                ; 
            }
        }
    }


    //---------------------------------------
    //           RollLeft/BankLeft
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Roll = Actions[FAction::RollLeft];
        const TActionStatus & Bank = Actions[FAction::BankLeft];
        if( FAction::CheckStatus( Roll, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
        else if( FAction::CheckStatus( Bank, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
    }

    //---------------------------------------
    //           RollRight/BankRight
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        const TActionStatus & Roll = Actions[FAction::RollRight];
        const TActionStatus & Bank = Actions[FAction::BankRight];
        if( FAction::CheckStatus( Roll, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
        else if( FAction::CheckStatus( Bank, FAction::IsActiveStatus ) )
        {
            //todo: [flying]
        }
    }

    //---------------------------------------
    //              Accelerate
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Accelerate = Actions[FAction::Accelerate];
        if( FAction::CheckStatus( Accelerate, FAction::IsActiveStatus ) )
        {
            FLOAT Magnitude = 0.01;
            if( Pawn.DesiredSpeed == 0 )
            {
                Pawn.DesiredSpeed = MaxSpeed;
            }
            Pawn.DesiredSpeed = Pawn.DesiredSpeed + Magnitude * RunSpeed;
            if( Pawn.DesiredSpeed > RunSpeed )
            {
                Pawn.DesiredSpeed = RunSpeed;
            }
        }
    }

    //---------------------------------------
    //              Decelerate
    //---------------------------------------
    if( !Pawn.bCannotMove )
    {
        const TActionStatus & Decelerate = Actions[FAction::Decelerate];
        if( FAction::CheckStatus( Decelerate, FAction::IsActiveStatus ) )
        {
            FLOAT Magnitude = 0.01;
            if( Pawn.DesiredSpeed == 0 )
            {
                Pawn.DesiredSpeed = MaxSpeed;
            }
            Pawn.DesiredSpeed = Pawn.DesiredSpeed - Magnitude * RunSpeed;
            if( Pawn.DesiredSpeed < 0.1 ) 
            { 
                Pawn.DesiredSpeed = 0.1; 
            }
        }
    }

    //---------------------------------------
    //               Jump
    //---------------------------------------
    const BOOL IsJumping = Pawn.bGravity && IsActivated(Actions[FAction::Jump]);
    if( !Pawn.bCannotMove )
    {
        if( IsJumping )
        {
            //tba
        }
    }

    //---------------------------------------
    //              Kick
    //---------------------------------------
    if( !Pawn.bCannotMove && FAction::CheckStatus( Actions[FAction::Kick], FAction::IsActiveStatus ) )
    {
        //tba
    }

    //---------------------------------------
    //              Zoom
    //---------------------------------------
    const BOOL Zoom = !Pawn.bCannotTurn && FAction::CheckStatus( Actions[FAction::Zoom], FAction::IsActiveStatus );
    if( Zoom )
    {
        if( Actor.CameraStatus.FOVAngle > 0.3 * GDefaults.FOV)
        {
            Actor.CameraStatus.FOVAngle = Max(Actor.CameraStatus.FOVAngle-4,0.3 * GDefaults.FOV);
        }
    }
    else if( Actor.CameraStatus.FOVAngle < GDefaults.FOV )
    {
        Actor.CameraStatus.FOVAngle = Min(Actor.CameraStatus.FOVAngle+4,GDefaults.FOV);
    }

    //---------------------------------------
    //              Level
    //---------------------------------------
    if( !Pawn.bCannotTurn )
    {
        if( FAction::CheckStatus( Actions[FAction::LookStraight], FAction::IsActiveStatus ) )
        {
            Pawn.TargetPitch = LevelPitch; // Return player to level pitch.
        }
    }

    //---------------------------------------
    //   WeaponFire/WeaponSpecial/WeaponCloseUp
    //---------------------------------------
    if( !Pawn.bCannotMove && Pawn.iWeapon!=INDEX_NONE ) 
    {
        const BOOL Use1 = FAction::CheckStatus( Actions[FAction::WeaponFire], FAction::IsActiveStatus );
        const BOOL Use2 = FAction::CheckStatus( Actions[FAction::WeaponSpecial], FAction::IsActiveStatus );
        const BOOL Use3 = FAction::CheckStatus( Actions[FAction::WeaponCloseUp], FAction::IsActiveStatus );
        //tba: Tertiary weapon use
        AWeapon & Weapon = FActor::Weapon(Pawn.iWeapon);
        if( Use1 || Use2 )
        {
            if
            ( 
                    Use1 && Weapon.InvState==INV_Using2
                ||  Use2 && Weapon.InvState==INV_Using1
            )
            {
                // The requested use of the weapon is different from its
                // current use. Release the current use.
                FActor::Send_Release(Pawn.iWeapon);
                //tbi!
                // A single switch might be used for both primary and
                // secondary use of a weapon (for example, Ctrl => primary, 
                // Ctrl+Ctrl [twice quickly] => secondary). We should be able
                // to quickly cancel a primary weapon use and change to a 
                // secondary use.
            }
            else
            {
                Level.SendMessage(Pawn.iWeapon,Use2?ACTOR_UseExtra:ACTOR_Use,&PUse());
            }
        }
        else if( Weapon.InvState==INV_Using1 || Weapon.InvState==INV_Using2 )
        {
            FActor::Send_Release(Pawn.iWeapon);
        }
    }

    if( Lunging )
    {
        if( Pawn.Stamina > 0 )
        {
            Pawn.Stamina -= StaminaDecayRate;
        }
    }
    else
    {
        if( Pawn.Stamina < 1.0 )
        {
            Pawn.Stamina += StaminaRechargeRate;
        }
    }

    //
    // Adjust view for stairs/inclines
    //
    FVector CanePoint = Pawn.Location; 
    CanePoint.AddYawed(CANE_LENGTH,0,Pawn.ViewRot.Yaw);
    CanePoint.Z += 10.0; // This prevents unstability during angled view of stairs.
    BOOL LookUpStair = FALSE;
    BOOL LookDownStair = FALSE;
    INDEX iHitActor;
    if 
    (
            GPreferences.ViewFollowsIncline
        &&  (Model.LineClass (&Pawn.Location,&CanePoint) 
        &&  (Model.ZCollision(&Pawn.Location,&Floor,&iHitActor)!=MAXWORD))
    )
    {
        INDEX        iFloorNode;
        FVector        CaneFloor;
        iFloorNode=Model.ZCollision(&CanePoint,&CaneFloor,&iHitActor);
        if( iFloorNode != INDEX_NONE )
        {
            FBspNode * FloorNode = &Model.BspNodes [iFloorNode];
            FBspSurf * FloorPoly;
            FloorPoly = &Model.BspSurfs [FloorNode->iSurf];
            if( !( FloorPoly->PolyFlags & (PF_HighLedge|PF_NoLook) ) )
            {
                FVector * FloorNormal = &Model.FVectors [FloorPoly->vNormal];
                if( FloorNormal->Z > 0.707 ) // Upward angle of floor normal >= 45 degrees?
                {
                    FLOAT Slope = (Floor.Z - CaneFloor.Z)/CANE_LENGTH; // Slope of view.
                    if( (Slope > +MIN_STAIR_SLOPE) && (Slope <= +MAX_STAIR_SLOPE) ) 
                    {
                        LookUpStair = TRUE;
                    }
                    else if( (Slope < -MIN_STAIR_SLOPE) && (Slope >= -MAX_STAIR_SLOPE) )
                    {
                        LookDownStair = TRUE;
                    }
                }
            }
        }
    }
    if( PlayerChoosesPitch )
    {
        Pawn.bLookingAlongStair = FALSE;
    }
    else if( LookUpStair )
    {
        Pawn.TargetPitch = PLAYER_STAIR_UP_PITCH;
        Pawn.bLookingAlongStair = TRUE;
    }
    else if( LookDownStair )
    {
        Pawn.TargetPitch = PLAYER_STAIR_DOWN_PITCH;
        Pawn.bLookingAlongStair = TRUE;
    }
    else if( Pawn.bLookingAlongStair )
    {
        // No longer looking up/down stairs - level the view.
        Pawn.TargetPitch = 0;
        Pawn.bLookingAlongStair = FALSE;
    }

    //if (Pawn.Velocity.Z != 0.0) TargetPitch = (SWORD)(8192.0*(Pawn.Velocity.Z/MAX_FALLING_Z_SPEED));
    //
    // Update yaw rotation:
    //
    if (OurSgn(Pawn.YawSpeed)!=OurSgn(YawAccel)) Pawn.YawSpeed = 0;
    Pawn.YawSpeed += YawAccel;
    //
    if (OurAbs(Pawn.YawSpeed) > MaxYawSpeed) Pawn.YawSpeed = OurSgn (Pawn.YawSpeed) * MaxYawSpeed;
    Pawn.ViewRot.Yaw += Pawn.YawSpeed;

    const TSignedAngle TargetPitch = SignedAngle(Pawn.TargetPitch);
    const TSignedAngle ExpectedPitch = SignedAngle(Pawn.ViewRot.Pitch+Pawn.PitchSpeed);
    const int Change = int(TargetPitch) - int(ExpectedPitch);
    if( OurAbs(Change) <= PLAYER_PITCH_ZERO )
    {
        Pawn.ViewRot.Pitch = Pawn.TargetPitch;        
        Pawn.PitchSpeed = 0;
    }
    else
    {
        Pawn.PitchSpeed += Change/PitchAccelerationRatio;
    }    

    const TSignedAngle PreviousPitch = SignedAngle(Pawn.ViewRot.Pitch);
    // Add in the pitch motion:
    if (OurAbs(Pawn.PitchSpeed) > MaxPitchSpeed) Pawn.PitchSpeed = OurSgn (Pawn.PitchSpeed) * MaxPitchSpeed;
    Pawn.ViewRot.Pitch = Pawn.ViewRot.Pitch + Pawn.PitchSpeed;
    if( Pawn.bLimitRotation )
    {
        LimitPitch( Pawn.ViewRot.Pitch );
    }

    if
    ( 
        OurAbs(Pawn.ViewRot.Pitch-Pawn.TargetPitch) <= PLAYER_PITCH_ZERO 
    ||  OurSgn(SignedAngle(Pawn.ViewRot.Pitch)-SignedAngle(Pawn.TargetPitch)) != OurSgn(PreviousPitch-SignedAngle(Pawn.TargetPitch)) // Did we overshoot?
    )
    {
        Pawn.ViewRot.Pitch = Pawn.TargetPitch;        
        Pawn.PitchSpeed = 0; 
    }
    else
    {
        Pawn.PitchSpeed = Pawn.PitchSpeed * 3 / 4; // Damp the pitch speed
    }

//tbd:[obsolete due to new rotation handling]    Pawn.ViewRot.Yaw  += AnalogYawSpeed;
    //
    // Now move with collision
    //
    //tbi? Performance:
    FVector Acceleration = GMath.ZeroVector;

//tba?    const int AccelerationPitchFactor = 1000; //tbm
//tba?    int ForwardAccelerationPitchAdjustment = 0;

//tba?    if( ForwardAcceleration != 0 )
//tba?    {
//tba?        // Pitch the view a little forward when accelerating forward
//tba?        // (as the player leans into a run) or a little backward when
//tba?        // accelerating backward (as the player leans back to stop).
//tba?        ForwardAccelerationPitchAdjustment = -ForwardAcceleration * AccelerationPitchFactor;
//tba?    }

    Acceleration.AddYawed
    ( 
        ForwardAcceleration
    ,   RightwardAcceleration
    ,   Pawn.ViewRot.Yaw 
    );
    Acceleration.Z += UpwardAcceleration; //tbi: This should be relative to the pawn's orientation.    
    FVector AnalogAcceleration = GMath.ZeroVector;
    AnalogAcceleration.AddYawed
    (
        ForwardAnalogAcceleration
    ,   RightwardAnalogAcceleration
    ,   Pawn.ViewRot.Yaw
    );
    AnalogAcceleration.Z += UpwardAnalogAcceleration; //tbi: This should be relative to the pawn's orientation.    


    // Update pawn's velocity
    // We do this even though the pawn might be falling, and he really shouldn't
    // be able to change direction or speed.
    {

        Pawn.Velocity += Acceleration;
        Pawn.Velocity += AnalogAcceleration;

        //------------------------------------------------------------------------
        //     Adjust impulse towards target speeds, if needed
        //------------------------------------------------------------------------
        if ( TargetForwardSpeed != 0 || TargetRightwardSpeed != 0 || TargetUpwardSpeed != 0 )
        {
            // We have a target speed - a speed which we are trying to achieve, as opposed
            // to simple impulsive accelerations. But these targets are relative to the
            // direction the pawn is oriented, so we need to know the current velocity
            // of the pawn relative to its orientation.

            //tbi: Is this too inefficient?...
            FCoords Coords = GMath.CameraViewCoords;
            Coords.DeTransformByRotation( Pawn.ViewRot );
            FVector CurrentSpeeds = Pawn.Velocity;
            CurrentSpeeds.TransformVector( Coords ); // Speeds along line of view.
            // This is a little weird, but CurrentSpeeds now has the following
            // components:
            const FLOAT CurrentForwardSpeed   =  CurrentSpeeds.Z;
            const FLOAT CurrentRightwardSpeed =  CurrentSpeeds.X;
            const FLOAT CurrentUpwardSpeed    = -CurrentSpeeds.Y;

            FLOAT ForwardAdjustment   = 0.0;
            FLOAT RightwardAdjustment = 0.0;
            FLOAT UpwardAdjustment    = 0.0;

            if( TargetForwardSpeed != 0 ) // (0 means there is no target)
            {
                const FLOAT Difference = TargetForwardSpeed - CurrentForwardSpeed;
                if( Difference > 0 ) // Do we need to speed up towards the target?
                {
                    ForwardAdjustment += Min(2.0*DefaultForwardAcceleration,Difference);    
                }
                else if( Difference < 0 ) // Do we need to slow down towards the target?
                {
                    ForwardAdjustment -= Min(2.0*DefaultForwardAcceleration,-Difference);    
                }
            }
            if( TargetRightwardSpeed != 0 ) // (0 means there is no target)
            {
                const FLOAT Difference = TargetRightwardSpeed - CurrentRightwardSpeed;
                if( Difference > 0 ) // Do we need to speed up towards the target?
                {
                    RightwardAdjustment += Min(2.0*DefaultRightwardAcceleration,Difference);    
                }
                else if( Difference < 0 ) // Do we need to slow down towards the target?
                {
                    RightwardAdjustment -= Min(2.0*DefaultRightwardAcceleration,-Difference);    
                }
            }
            Pawn.Velocity.AddYawed( ForwardAdjustment, RightwardAdjustment, Pawn.ViewRot.Yaw );
            //tba: UpwardAdjustment

        }

        //------------------------------------------------------------------------
        // Adjust the velocity back towards MaxSpeed or towards Pawn.DesiredSpeed
        //------------------------------------------------------------------------
        if( Pawn.DesiredSpeed != 0 && MaxSpeed > Pawn.DesiredSpeed )
        {
            MaxSpeed = Pawn.DesiredSpeed;
        }
        //tbi: We must consider his .Z speed, and take care in limiting the
        // X- and Y- speeds of a falling object, otherwise, the large Z
        // speed will dominate the total speed and force reduction of the
        // X- and Y- speeds. This is okay for an upward speed, but not for
        // a downward speed.
        FLOAT Speed = Pawn.Velocity.Size2D();
        if( Speed > MaxSpeed )
        {
            FLOAT SlowDownFactor = Max( MaxSpeed / Speed, VelocityDamping );
            Pawn.Velocity.X *= SlowDownFactor;
            Pawn.Velocity.Y *= SlowDownFactor;
            Speed *= SlowDownFactor;
        }
        if( Pawn.DesiredSpeed == 0 )
        {
        }
        else if( Speed > Pawn.DesiredSpeed )
        {
            const FLOAT SlowDownFactor = Max( Pawn.DesiredSpeed / Speed, VelocityDamping );
            Pawn.Velocity.X *= SlowDownFactor;
            Pawn.Velocity.Y *= SlowDownFactor;
            Speed *= SlowDownFactor;
        }
        else // Speed < Pawn.DesiredSpeed
        {
            const FLOAT SpeedUpFactor = Min( Pawn.DesiredSpeed / Speed, 1.1 );
            Pawn.Velocity.X *= SpeedUpFactor;
            Pawn.Velocity.Y *= SpeedUpFactor;
            Speed *= SpeedUpFactor;
        }

        if( Speed < StopThresholdSpeed )
        {
            const FLOAT SlowDownFactor = 0;
            Pawn.Velocity.X *= SlowDownFactor;
            Pawn.Velocity.Y *= SlowDownFactor;
            Speed *= SlowDownFactor;
        }

   
        // Use the current speed to determine if the view should be leveled:
        if( !PlayerChoosesPitch && !Pawn.bLookingAlongStair && Speed > NormalSpeed/2.0)
        {
            if( Speed > 1.0 )
            {
                Pawn.TargetPitch = 0;
            }
        }
    }

    FLOAT FallingVelocity = Pawn.Velocity.Z;
    if (Model.PointClass (&Pawn.Location,NULL) != 0)
    {
        INDEX iCollisionNode;
		INDEX iHitActor;

		// Here we add new logic to handle message dispatching when players
		// move on and off of moving brushes.

        iCollisionNode = Model.ZCollision(&Pawn.Location,&Floor,&iHitActor);
        if (Pawn.bGravity && iCollisionNode != MAXWORD) // Don't fall if standing on air
        {
            FBspNode * FloorNode    = &Model.BspNodes [iCollisionNode];
            FBspSurf * FloorPoly    = &Model.BspSurfs [FloorNode->iSurf];
            FVector  * FloorNormal  = &Model.FVectors [FloorPoly->vNormal];
            //
            if ((FloorNormal->Z < RAMP_MAX_Z_NORM) && (FloorNormal->Z > RAMP_MIN_Z_NORM)) // Ramp
                {
                Pawn.Velocity.X += 0.30 * (FloorNormal->X - 0.10) / FloorNormal->Z;
                Pawn.Velocity.Y += 0.30 * (FloorNormal->Y - 0.10) / FloorNormal->Z;
                }
            DZ = Pawn.Location.Z - Floor.Z - Pawn.CollisionHeight; // +0.0 means exactly on floor
            if ((DZ <= MAX_SMOOTH_STAIR_DIST) && (Pawn.Velocity.Z <= 0.0)  )
            {                        
                // Standing on ground (may be climbing or descending stairs):
                if (IsJumping)            
                {
                    Pawn.Velocity.Z = +40.0;
                }
                else
                {
                    Pawn.EyeHeight    += DZ;
                    Pawn.Location.Z    -= DZ;
                    Pawn.Velocity.Z    = 0.0;
                }
            }
            else
            {
                // Falling:
                Pawn.Velocity += Gravity;
                Pawn.Velocity *= 0.92; 
                FallingVelocity = Pawn.Velocity.Z;
				// Detach from moving floor if we're falling but not jumping
				if (!IsJumping)
				{
					iHitActor = INDEX_NONE;
				}
            }
        }
		else
		{
			// Detach from moving floor
			iHitActor = INDEX_NONE;
		}

		// Handle detaching from moving floor
		if ( (iHitActor != Actor.iFloor) && (Actor.iFloor != INDEX_NONE))
		{
			// Detach from current moving floor
			Level.SendMessage(Actor.iMe,   ACTOR_UnStandMover,&Actor.iFloor);
			Level.SendMessage(Actor.iFloor,ACTOR_UnSteppedOn, &Actor.iMe);
			Actor.iFloor = INDEX_NONE;
			//bug("Moved off of floor");
		}

		// Handle attaching to moving floor
		if ( iHitActor != Actor.iFloor)
		{
			// Attach to moving floor
			Actor.iFloor = iHitActor;
			Level.SendMessage(Actor.iMe,   ACTOR_StandMover,&Actor.iFloor);
			Level.SendMessage(Actor.iFloor,ACTOR_SteppedOn, &Actor.iMe);
			//bug ("Moved onto floor");
		}
		//!!TIM - END CHANGES

        if (Pawn.bGravity && Pawn.Velocity.Z < -MAX_FALLING_Z_SPEED)
            {
            Pawn.Velocity.Z = -MAX_FALLING_Z_SPEED; // Drag
            }
        //debugf(LOG_Info, " %p:%s [%3f %3f %3f]", &Actor, Actor.Class->Name,Actor.Location.X,Actor.Location.Y, Actor.Location.Z );
        Level.MoveActor(iActor,&Pawn.Velocity);
        //debugf(LOG_Info, "    ==> %p:%s [3f %3f %3f]", &Actor, Actor.Class->Name,Actor.Location.X,Actor.Location.Y, Actor.Location.Z );
        //
        // Keep player from falling on her butt (where bounding sphere is resting
        // on floor and line is dangling in midair):
        //
        iCollisionNode = Model.ZCollision(&Pawn.Location,&Floor,&iHitActor);
        if( Pawn.bGravity  && ( iCollisionNode != MAXWORD))
        {
            const BOOL WasOnSurface = Pawn.bIsOnSurface;
            DZ = Pawn.Location.Z - Floor.Z - Pawn.CollisionHeight; // +0.0 means exactly on floor
            if (DZ > MAX_SMOOTH_STAIR_DIST)
            {
                MoveVector = Level.GetZoneGravityAcceleration(iActor);
                Level.MoveActor(iActor,&MoveVector);
                Pawn.bIsOnSurface = FALSE;
            }
            else
            {
                Pawn.bIsOnSurface = TRUE;
                // See if the surface is harmful:
                FBspNode * FloorNode    = &Model.BspNodes [iCollisionNode];
                FBspSurf * FloorPoly    = &Model.BspSurfs [FloorNode->iSurf];
                if( FloorPoly->PolyFlags & PF_Hurt )
                {
                    //debugf( LOG_Info, "Player is hurt!" );
                    PHit HitInfo;
                    HitInfo.Empty();
                    HitInfo.Damage[DMT_Basic] = 0.142; //tbi: Hard-coded value
                    if( (GServer.Ticks&0x1f)==0 )
                    {
                        Pawn.EyeHeight += Random( 5.0, 8.0 ) * ( FRandom::Boolean() ? -1.0 : 1.0 );
                        Pawn.ViewRot.Roll += 0x200;
                    }
                    Actor.Send_Hit(HitInfo);
                }
            }
            if( !WasOnSurface && Pawn.bIsOnSurface )
            {
                // The pawn just landed - check to see how far it fell.
                const FLOAT FallDistance = Pawn.LastNonFallZ - Pawn.Location.Z;
                const FLOAT SafeDistance = 700;
                if( FallDistance > SafeDistance )
                {
                    PHit HitInfo;
                    HitInfo.Empty();
                    HitInfo.Damage[DMT_Basic] = (FallDistance-SafeDistance)/50;
                    Actor.Send_Hit(HitInfo);
                }
            }
        }
    }
    else Pawn.Location.MoveBounded(Pawn.Velocity); // Trapped in wall
    //
    // Add slight acceleration/deceleration effect with mouse:
    //
    //tbd?Pawn.Velocity += AnalogAcceleration * 0.10;
    //
    // Head bobbing:
    //
    BobAmplitude = 
    (
        (GPreferences.StillViewBobs ? STILL_BOB_HEIGHT : 0 )
    +   (GPreferences.MovingViewBobs ? PLAYER_WALK_BOB_HEIGHT : 0 )
         * Pawn.Velocity.Size()
    ) / Pawn.NormalSpeed
    ;
    Pawn.Bob   = (IsCrouching?PLAYER_CROUCH_HEIGHT:0.0) +
        OurMin(BobAmplitude,(FLOAT)PLAYER_MAX_BOB_HEIGHT) *
        GMath.CosTab(((WORD)(65536.0 * PLAYER_BOB_PER_SEC * (FLOAT)GServer.Ticks/(FLOAT)GServer.Rate)));
    //
    // Final eyeheight computation: Combine components, smoothly move to destination,
    // and force within collision sphere:
    //
    DZ  = Pawn.EyeHeight - (Pawn.BaseEyeHeight + Pawn.Bob);
    //
    // Update camera roll - slight bobbing roll with motion, plus smoothly-accelerated
    // roll when cornering.
    //
    FVector AdjustedVelocity = Pawn.Velocity + AnalogAcceleration;
    //tbd: [obsolete] SWORD   AdjustedYawSpeed = Pawn.YawSpeed + AnalogYawSpeed;
    //
    // Update pitch:
    //
    //TargetPitch += 128.0*AdjustedVelocity.GetYawedNormal(Pawn.ViewRot.Yaw); /* Annoying */
    //
    // Update roll:
    //
    static FLOAT RollFactor = 1.2;
    if( GCheat->Adjustment == FCheat::AdjustPlayerViewRoll )
    {
        GCheat->DoAdjustments(Actor.iMe, &RollFactor );
    }

    if( !IsDead )
    {
        Pawn.DrawRot.Roll = 0;
        if( GPreferences.ViewRolls )
        {
            Pawn.DrawRot.Roll = RollFactor *
            (
                +50.00 * AdjustedVelocity.GetYawedTangent(Pawn.ViewRot.Yaw) 
            //tbd: [deleted because this causes performance hit in texture mapping] +   -00.35 * AdjustedYawSpeed
            );
        }
        Pawn.ViewRot.Roll += ((SWORD)Pawn.DrawRot.Roll - (SWORD)Pawn.ViewRot.Roll)/PLAYER_ROLL_SNAPBACK_RATIO;
        if (OurAbs((SWORD)Pawn.ViewRot.Roll)<PLAYER_ROLL_ZERO) Pawn.ViewRot.Roll = 0;
        Pawn.EyeHeight -= DZ * EYEHEIGHT_DECELERATION_CF;
        if (Pawn.EyeHeight < -Pawn.CollisionRadius*0.80) Pawn.EyeHeight = -Pawn.CollisionRadius*0.80;
        if (Pawn.EyeHeight > +Pawn.CollisionRadius*0.80) Pawn.EyeHeight = +Pawn.CollisionRadius*0.80;
    }
    if( !Pawn.bGravity || Pawn.bIsOnSurface )
    {
        //tbi: This is duplicated here and in APawn::Process
        Pawn.LastNonFallZ = Pawn.Location.Z;
    }

    ShowPlayerStat(Pawn);

    return ProcessDone; // Successfully processed message
    UNGUARD("playerTick");
}

/*-----------------------------------------------------------------------------
    The End
-----------------------------------------------------------------------------*/
