/*=============================================================================
    UnPawns.cpp: General pawn actor code

    Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
    Compiled with Visual C++ 4.0.

    Revision history:
        07/04/96: Created by Mark
=============================================================================*/

//todo:
// Check all uses of sqrt (or use of Size() or Size2D() functions) and see
// if the computation can be made faster by eliminating the sqrt().

#include "UnGame.h"

#include "UnFActor.h"
#include "UnRandom.h"
#include "UnPrefer.h"
#include "UnCheat.h"
#include "UnInput.h"
#include "UnAction.h"

#define DebuggingAI          0 // 1 to put AI info in log, 0 otherwise.
#define DebuggingMotion      0 // 1 to put motion info into log, 0 otherwise.
#define DebuggingActions     0 // 1 to put action info into log, 0 otherwise.
#define DebuggingExploration 0 // 1 to put exploration info into log, 0 otherwise.

#define AllowPerformanceMeasurements 1 // 1 to allow performance measurements, 0 otherwise.

#define arrayCount_(Array) ( sizeof(Array) / sizeof((Array)[0]) )

#if DebuggingAI || DebuggingMotion || DebuggingActions || DebuggingExploration
    static void __cdecl Debug(const char * Message, ...)
    {
        static int Count = 0;
        if( Count <= 300 ) // To reduce total messages.
        {
            char Text[300];
            va_list ArgumentList;
            va_start(ArgumentList,Message);
            vsprintf(Text,Message,ArgumentList);
            va_end(ArgumentList);
            debugf( LOG_Info, Text );
        }
    }
    static void DebugVector(const char *Text, const FVector & Vector )
    {
        char VectorText[100];
        char Message[100];
        sprintf( VectorText, "[%4.4f,%4.4f,%4.4f]", Vector.X, Vector.Y, Vector.Z );
        sprintf( Message, Text, VectorText );
        Debug(Message);
    }
#else
    static inline void __cdecl Debug(const char * Message, ...)
    {
    }
    #define DebugVector(Text,Vector)
#endif

static const FLOAT StopThresholdSpeed = 0.04; // Speeds below this are set to 0.
static const FLOAT MotionDamping = 0.9;

//tbm: Convert degrees to WORD angle measurements.
static inline int Degrees(int Degrees)
{
    return Degrees*65536/360;
}

static inline FLOAT Meters(FLOAT Meters) // Convert meters to world units.
{
    return 52.5*Meters; // About 52 world units per meter.
}

//static inline int Squared(int X) { return X*X; }
template<class T> inline T Squared(T X) { return X*X; }

//static inline FLOAT Squared(FLOAT X) { return X*X; }

static inline int UnitsInDekaUnits(int Units) // Convert meters to world units.
{
    return Units * 10;
}

static inline int Seconds(int Seconds) // Convert seconds to timer ticks.
{
    return GServer.Rate*Seconds;
}

static inline int DSeconds(int DSeconds) // Convert deciseconds to timer ticks.
{
    return GServer.Rate*DSeconds/10;
}

#define RAMP_MAX_Z_NORM             0.90    /* Player should slide down ramps in this range */
#define RAMP_MIN_Z_NORM             0.10    /* Something sloped at this grade or lower is a wall */
#define MAX_FALLING_Z_SPEED            12.0    /* Prevents player from falling down too fast */


static void updateTargetInfo(FActor & Actor )
{
    AActorAI & AI = Actor.AI();
    APawn & Pawn = Actor.Pawn();
    if( Actor.iTarget != INDEX_NONE )
    {
        FActor & Target = FActor::Actor(Actor.iTarget);
        Pawn.bTargetWasNear = Pawn.bTargetIsNear;
        Pawn.bTargetIsNear = Actor.IsNear(Target);
        if( !Target.Pawn().bHasInvisibility )
        {
            Pawn.TargetLastLocation = Target.Location;
            Pawn.TargetLastVelocity = Target.Velocity;
        }
    }
    else if( Pawn.bTargetWasLost && ! Pawn.bTargetWasHere )
    {
        BOOL Was = Pawn.bTargetWasHere;
        Pawn.bTargetWasHere = (Actor.Location-Pawn.TargetLastLocation).SizeSquared() <= Meters(2.0)*Meters(2.0);
    }
}

static void SetActorTarget(INDEX iActor, INDEX iTarget = INDEX_NONE )
{
    FActor      &   Actor   = FActor::Actor(iActor);
    AActorAI    &   AI      = Actor.AI();
    APawn       &   Pawn    = Actor.Pawn();
    Pawn.bTargetIsNear = FALSE;
    Pawn.bTargetWasNear = FALSE;
    Actor.iTarget = iTarget;
    Pawn.bTargetWasHere = FALSE;
    if( iTarget == INDEX_NONE )
    {
        Pawn.bTargetWasLost = TRUE;
    }
    else
    {
        Pawn.bTargetWasLost = FALSE;
        updateTargetInfo(Actor);
    }
}



//tbm:
// Yaw the vector by the angle defined by Sine and Cosine.
//tbi: This is not really what happens.
static inline void Yaw(FVector & This, FLOAT Sine, FLOAT Cosine ) //tbi:word
{
    const FLOAT X1 = This.X;
    const FLOAT Y1 = This.Y;
    This.X = Y1*Sine   + X1*Cosine  ;
    This.Y = Y1*Cosine - X1*Sine    ;
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

//tbm:
// Pitch the vector by the specified Angle.
//tbi: This is not really what happens.
static inline void Pitch(FVector & This, WORD Angle) //tbi:word
{
    //tbe:
    const FLOAT Size2D  = This.Size2D()     ;
    const FLOAT Sine    = This.Y / Size2D   ;
    const FLOAT Cosine  = This.X / Size2D   ;
    // First, yaw to align the vector into the XZ plane.
    Yaw( This, Sine, Cosine );
    // Rotate in the XZ plane:
    {
        const FLOAT Cos = GMath.CosTab(Angle);
        const FLOAT Sin = GMath.SinTab(Angle);
        const FLOAT X0 = This.X;
        const FLOAT Z0 = This.Z;
        This.X = X0*Cos + Z0*Sin;
        This.Z = Z0*Cos - X0*Sin;
    }
    // Yaw back:
    Yaw( This, -Sine, Cosine );

}

// What is the change in height (Z) after a Pawn actor is:
//   - moved to NewLocation
//   - dropped down to the floor beneath
// The following cases are handled:
//   - If the NewLocation is inside a solid, a large positive
//     value is returned.
//   - If the NewLocation doesn't have a floor beneath it, a
//     large negative value is returned
// Special cases:
//   1. For an actor unaffected by gravity, no search is made
//      for the floor beneath.
static FLOAT ZChange( FActor & Actor, const FVector & _NewLocation )
{
    IModel & Model = GLevel->ModelInfo;
    FLOAT dZ; // Change in Z location.
    FVector NewLocation = _NewLocation; //tbi!! constness problem
    if( Model.PointClass(&NewLocation,NULL) == 0 )
    {
        // The NewLocation is inside something.
        dZ = +100000.0; // A large value is used in this case.
        if( DebuggingMotion ) DebugVector( "actorZChange: %s inside wall", NewLocation );
    }
    else if( !Actor.bGravity ) 
    {
        // The actor is unaffected by gravity.
        dZ = NewLocation.Z - Actor.Location.Z;
    }
    else
    {
        // Search for the floor below the NewLocation.
        FVector Floor;
        INDEX iActorHit;
        const BOOL FloorFound = Model.ZCollision(&NewLocation,&Floor,&iActorHit) != INDEX_NONE;
        dZ = 
            FloorFound
        ?   Floor.Z - (Actor.Location.Z - Actor.CollisionHeight)
        :   -100000   // Large negative value means no floor.
        ;
    }
    return dZ;
}

//
// Direct the Actor to move towards the given location.
static void DirectActorTowards( FActor & Actor, const FVector & TargetLocation, FLOAT NoCloserThan = 0.0 )
{
    //tbi: performance
    APawn    & Pawn = Actor.Pawn();
    FVector Direction = TargetLocation - Actor.Location; // Vector from actor to target.
    //tbd? Direction.Z = 0; //tbi: assumes 2-D
    FLOAT Distance = Direction.Size();
    if( Distance < NoCloserThan || Pawn.NormalSpeed == 0 )
    {
        //tbd?Pawn.Velocity = GMath.ZeroVector;
    }
    else
    {
        const FLOAT Acceleration = Pawn.NormalSpeed * ( 1.0 - MotionDamping); //tbe
        FVector Change = Direction;
        Change.Normalize();
        Change *= Acceleration;

        Pawn.Velocity += Change;
        // We don't want to step over an edge, so look ahead to see what is ahead of us:
        FVector Probe = Pawn.Location;
        Probe += 2.0 * Change; // Probe a little further to keep the pawn well away from the edge.
        if( ZChange(Actor,Probe) < -FLOAT(Pawn.MaxStepDownHeight) )
        {
            Pawn.Velocity -= Change;
            Pawn.Velocity /= 2; // Slow down pawn so momentum doesn't carry him over the edge.
            //todo: Cancel motion goal
        }
        else
        {
            Probe += 2.0 * Change; // Probe even a little further...
            if( ZChange(Actor,Probe) < -2.0*FLOAT(Pawn.MaxStepDownHeight) )
            {
                Pawn.Velocity -= Change;
                Pawn.Velocity /= 2; // Slow down pawn so momentum doesn't carry him over the edge.
                //todo: Cancel motion goal
            }
        }
#if 0 //tbd?
        FLOAT Speed = Pawn.Velocity.Size2D(); //tba: 3-D moving pawns
//tbd?        Pawn.Velocity = Direction;
        if( Speed > Pawn.NormalSpeed )
        {
            //tbi: Damp the motion instead of clamping it.
            const FLOAT SlowDown = 0.8;
            Pawn.Velocity.X *= SlowDown;
            Pawn.Velocity.Y *= SlowDown;
            Speed *= SlowDown;
        }
#endif
    }
}

static FLOAT SizeSquared2D(const FVector & Vector )
{
    return Vector.X * Vector.X + Vector.Y * Vector.Y;
}

// Has an actor achieved a location goal?
static BOOL actorAchievedLocationGoal( FActor & Actor, const AIMotion::TGoal & Goal )
{
    BOOL GoalWasAchieved = FALSE;
    if( Goal.Kind == 0 )
    {
        GoalWasAchieved = TRUE;
    }
    else if( Goal.Kind == AIMotion::ToActor )
    {
        FActor & Target = FActor::Actor(Goal.iActor);
        GoalWasAchieved = Actor.IsNear(Target);
    }
    else if( Goal.Kind == AIMotion::ToLocation )
    {
        FVector Direction = Goal.Location - Actor.Location;
        FLOAT DistanceSquared = Actor.bGravity ? SizeSquared2D(Direction) : Direction.SizeSquared();
        GoalWasAchieved = DistanceSquared <= Squared(2.0*Actor.CollisionRadius);
    }
    else
    {
        // Other kinds of goals are unachievable.
    }
    return GoalWasAchieved;
}

// Has an actor achieved a rotation goal?
static BOOL actorAchievedRotationGoal( FActor & Actor, const AIMotion::TGoal & Goal )
{
    BOOL GoalWasAchieved = FALSE;
    if( Goal.Kind == 0 )
    {
        GoalWasAchieved = TRUE;
    }
    else if( Goal.Kind==AIMotion::RandomizedChange )
    {
        // Such a goal is never achieved.
    }
    else
    {
        //tbi: performance
        FVector Direction;
        if( Goal.Kind == AIMotion::ToActor )
        {
            FActor & Target = FActor::Actor(Goal.iActor);
            Direction = Target.Location - Actor.Location;
        }
        else if( Goal.Kind == AIMotion::ToLocation )
        {
            Direction = Goal.Location - Actor.Location;
        }
        else
        {
            // Other kinds of goals are unsupported.
        }
        FVector UnitDirection = Direction;
        UnitDirection.Z = 0; // For yawing, we only care about x-y coordinates.
        UnitDirection.Normalize();
        FVector YawVector;
        const int Yaw = Actor.DrawRot.Yaw;
        YawVector.X = GMath.CosTab(Yaw);
        YawVector.Y = GMath.SinTab(Yaw);
        YawVector.Z = 0;
        if( ( YawVector |UnitDirection ) > .95 )
        {
            GoalWasAchieved = TRUE;
        }
    }
    return GoalWasAchieved;
}

// Did an actor achieve a motion goal?
static BOOL actorAchievedMotionGoal( FActor & Actor, const AIMotion & Motion )
{
    return 
        actorAchievedLocationGoal(Actor,Motion.Location )
    &&  actorAchievedRotationGoal(Actor,Motion.Rotation )
    ;
}

// Try to achieve a location goal, at least partly.
static void actorTryToAchieveLocationGoal( FActor & Actor, const AIMotion::TGoal & Goal )
{
    APawn & Pawn = Actor.Pawn();
    if( Pawn.bCannotMove )
    {
    }
    else if( Goal.Kind == AIMotion::ToActor )
    {
        FActor & Target = FActor::Actor(Goal.iActor);
        DirectActorTowards( Actor, Target.Location, Actor.CollisionRadius+Target.CollisionRadius );
    }
    else if( Goal.Kind == AIMotion::ToLocation )
    {
        DirectActorTowards( Actor, Goal.Location );
    }
    else if( Goal.Kind == AIMotion::AlongVector )
    {
        DirectActorTowards( Actor, Actor.Location+Goal.Direction );
    }
    else
    {
        // Other kinds of goals are unsupported.
    }
}

// Try to achieve a rotation goal, at least partly.
static void actorTryToAchieveRotationGoal( FActor & Actor, AIMotion::TGoal & Goal )
{
    APawn & Pawn = Actor.Pawn();
    if( Pawn.bCannotTurn )
    {
    }
    else if( Goal.Kind == AIMotion::ToActor )
    {
        FActor & Target = FActor::Actor(Goal.iActor);
        Actor.TurnTowards( Target.Location );
    }
    else if( Goal.Kind == AIMotion::ToLocation )
    {
        Actor.TurnTowards( Goal.Location );
    }
    else if( Goal.Kind == AIMotion::AlongVector )
    {
        Actor.TurnTowards( Actor.Location+Goal.Direction );
    }
    else if( Goal.Kind == AIMotion::UniformChange )
    {
        Actor.DrawRot.Pitch += Goal.dRotation.Pitch ;
        Actor.DrawRot.Yaw   += Goal.dRotation.Yaw   ;
        Actor.DrawRot.Roll  += Goal.dRotation.Roll  ;
    }
    else if( Goal.Kind == AIMotion::RandomizedChange )
    {
        //todo: Generalize randomized rotation...
        //todo: Performance? (frequent randomizations)
        if( (GServer.Ticks&0x7) == 0 )
        {
            Goal.dRotation.Yaw = Random( 200,1000 );
        }
        Actor.DrawRot.Pitch += Goal.dRotation.Pitch ;
        Actor.DrawRot.Yaw   += Goal.dRotation.Yaw   ;
        Actor.DrawRot.Roll  += Goal.dRotation.Roll  ;
    }
    else
    {
        // Other kinds of goals are unsupported.
    }
}

typedef enum
{
    NormalExploration       // Normal exploration (approach target, keep in sight)
,   BackOffExploration      // Back off from target, but not with fear or panic.
,   RunAwayExploration      // Run away, hide (panic, fear).
}
TExplorationKind;

struct TExploreInfo
{
    FVector Origin          ; // Where the exploration starts from.
    BOOL    HasTarget       ; // TRUE iff there is a target point.
    int     StepCount       ; // How many steps were made.
    int     TargetViewCount ; // How many steps kept the target point in view?
    BOOL    TargetViewedAtFrontier    ; // Can the target be seen on the frontier?
    BOOL    NearsEdge       ; // True if the frontier is close to an edge.
    FLOAT   Distance        ; // How far did the exploration go.
    FVector Frontier        ; // Limits of the exploration.
    FVector Target          ; // Exploration target, if HasTarget.
    void Empty(const FVector & Origin, const FVector * TargetPoint )
    {
        this->Origin            = Origin    ;
        StepCount               = 0         ;
        TargetViewedAtFrontier  = FALSE     ;
        NearsEdge               = FALSE     ;
        TargetViewCount         = 0         ;
        Distance                = 0.0       ;
        HasTarget               = TargetPoint != 0;
        if( HasTarget )
        {
            Target = *TargetPoint;
        }
    }
    #if DebuggingExploration
        void Dump(const char * Intro) const;
    #endif
    int Score(TExplorationKind Kind) const;
};

#if DebuggingExploration
void TExploreInfo::Dump( const char * Intro ) const
{
    char Message[200];
    char * Text = Message;
    Text += sprintf( Text, "%s:", Intro );
    Text += sprintf( Text, " Steps=%i Distance=%3.2f (Score=???)", int(StepCount), Distance /*, Score()*/ );
    debug( LOG_Info, Message );
}
#endif



// Evaluate a score for an exploration.
int TExploreInfo::Score(TExplorationKind Kind) const
{
    const TExploreInfo & Info = *this;
    int Score = 0;
    if( Info.HasTarget )
    {
        const FLOAT OldDistance = (Info.Origin-Info.Target).Size();
        const FLOAT NewDistance = (Info.Frontier-Info.Target).Size();
        const FLOAT dDistance = NewDistance-OldDistance;
        if( dDistance < 0 ) // Score for approaching target...
        {
            Score += Kind==NormalExploration ? 10 : -10;
        }
        else // Score for moving away from target...
        {
            Score += Kind==NormalExploration ? -5 : 10 ;
        }
        if( Info.TargetViewedAtFrontier ) // Score for keeping target in sight...
        {
            if( Kind != RunAwayExploration )
            {
                Score += 20;
            }
        }
        else // Score for losing sight of target...
        {
            if( Kind == RunAwayExploration )
            {
                Score += 20;
            }
        }
        // Lose points for short explorations which near an edge...
        if( Info.NearsEdge && Info.StepCount < 4 )
        {
            Score -= 10;
        }

    }
    Score += StepCount; // Points for distance traveled
    // Points off for very short distances:
    if( StepCount < 4  )
    {
        Score -= 10;
    }
#if 0 //tbd: obsolete
    // 3 points for each unit closer the exploration gets to the target point:
    // -1 point for each unit further away.
    if( Info.HasTarget )
    {
        const FLOAT OldDistance = (Info.Origin-Info.Target).Size();
        const FLOAT NewDistance = (Info.Frontier-Info.Target).Size();
        const FLOAT dDistance = NewDistance-OldDistance;
        if( dDistance < 0 ) // 
        {
            //Debug("   score closer: %i", -3*int(dDistance) );
            Score += 3*int(dDistance/100);
        }
        else
        {
            //Debug("   score further: %i", int(dDistance) );
            Score -= int(dDistance);
        }
        // Points for keeping target in sight.
        if( Info.TargetViewedAtFrontier )
        {
            Score += 100;
        }

    }
    // 2 points for each unit traveled:
    Score += 2*int(Info.Distance);
    // Points off for very short distances:
    if( Info.Distance <= Meters(.4) )
    {
        Score /= 4;
    }
#endif
    //Debug( "    score travel: %i", 2*int(Info.Distance) );
    return Score;
}

// Replace Best with the better of the two explorations (Best,Info).
static void BestExploration(TExploreInfo & Best, const TExploreInfo Info, TExplorationKind Kind)
{
    //tbi: Performance ... Recalculation of best score each time.
    const int InfoScore = Info.Score(Kind);
    const int BestScore = Best.Score(Kind);
    const BOOL Better = InfoScore >= BestScore || (InfoScore>=BestScore-5 && FRandom::Percent(50));
    #if DebuggingExploration
    {
        Best.Dump( "Best of" );
        Info.Dump( "    and" );
    }
    #endif
    if( Better )
    {
        Best = Info;
    }
    #if DebuggingExploration
    {
        Best.Dump( "     is" );
    }
    #endif
}

// Explore the area away from an actor in the given direction.
// Move in the direction in small increments and check various
// things:
//  - Make sure the actor does not go over an edge (unless the actor can fly/float).
//  - Make sure the actor does not move into a wall.
//  - Make sure the actor does not step onto an excessively high raised surface.
//todo: Efficiency
static TExploreInfo Explore
(
    FActor            & Actor
,   const FVector     & Direction   // Direction to go exploring.
,   const FVector     * TargetPoint // Targeted point, or 0 if none.
,   FLOAT               MaxDistance // Maximum distance to explore.
,   TExplorationKind    Kind
)
{
    APawn & Pawn = Actor.Pawn();
    IModel & Model = GLevel->ModelInfo;
    TExploreInfo Info;
    Info.Empty(Actor.Location,TargetPoint);
    FVector ExploreHere = Actor.Location;
    FVector ExploreDirection = Direction;
    if( Actor.bGravity )
    {
        ExploreDirection.Z = 0;
    }
    ExploreDirection.Normalize();
    ExploreDirection *= Actor.CollisionRadius; /*tbd? Meters(.1)/2;*/
    const int StepsInProbe = 3; // Heuristic: number of steps pawn might 
        // make while travelling in ExploreDirection. Used to figure out
        // how to handle stairs and ramps.

    BOOL Stopped = FALSE; // TRUE when exploration is stopped.

    // For long explorations, we keep the results of intermediate
    // explorations. This way, we won't ignore a potentially useful
    // exploration because we looked "too far".
    int BestInfoAge = 0; // Count of how old BestInfo is, in steps.
    TExploreInfo BestInfo = Info;
    for
    ( 
        int Limiter = 0; // To limit the number of iterations.
        Limiter <= 10 && !Stopped && Info.Distance < MaxDistance;
        Limiter++
    )
    {
        FVector NewLocation = ExploreHere;
        NewLocation += ExploreDirection;
        FVector OutOfWallAdjustment;
        Stopped = !Model.SphereTestMove
        (
            &ExploreHere
        ,   &ExploreDirection
        ,   &OutOfWallAdjustment
        ,   Actor.CollisionRadius
        ,   0
        );
        if( Stopped && DebuggingExploration )
        {
            debugf( LOG_Info, "Stopped by failed sphere test" );
        }
        // Adjust the Z position of the actor's new location based
        // on the length of his collision rod:
        if( !Stopped )
        {
            FVector Floor;
            INDEX iActorHit;
            const BOOL FloorFound = Model.ZCollision(&NewLocation,&Floor,&iActorHit) != INDEX_NONE;
            const FLOAT dZ = 
                FloorFound
            ?   Floor.Z - (ExploreHere.Z - Actor.CollisionHeight)
            :   0.0
            ;
            if( !FloorFound )
            {
                // No floor was found - not good unless the actor can fly!
                Stopped = !Actor.bGravity;
            }
            else if( dZ > 0 ) // Upwards movement?
            {
                Stopped = dZ >= StepsInProbe*int(Pawn.MaxStepUpHeight); // Did we move up too far?
                if( Stopped && DebuggingExploration )
                {
                    debugf(LOG_Info,"Exploration stopped by upward change: %3.1f >= %i * %i", dZ, int(StepsInProbe), int(Pawn.MaxStepUpHeight) );
                }
                const FLOAT NewZ = Floor.Z + Actor.CollisionHeight;
                if( Actor.bGravity || NewLocation.Z <= NewZ )
                {
                    NewLocation.Z = NewZ;
                }
            }
            else if( dZ < 0 && Actor.bGravity ) // Downwards movement?
            {
                Stopped = dZ <= -FLOAT(StepsInProbe*int(Pawn.MaxStepDownHeight)); // Did we move down too far?
                if( Stopped && DebuggingExploration ) 
                {
                    debugf(LOG_Info,"Exploration stopped by downward change: %3.1f <= -%i * %i", dZ, int(StepsInProbe), int(Pawn.MaxStepDownHeight) );
                }
                if( Stopped )
                {
                    Info.NearsEdge = TRUE;
                }
                // Now, drop any gravity-affected actor to the floor:
                NewLocation.Z = Floor.Z + Actor.CollisionHeight;
            }
        }
        if( !Stopped )
        {
            //tbi: Performance
            // The move looks okay. Update the exploration information.
            FLOAT LastDistance = (NewLocation-ExploreHere).Size();
            Info.StepCount++;
            Info.Distance += LastDistance;
            Info.Frontier = ExploreHere;
            if( TargetPoint != 0 )
            {
                Info.TargetViewedAtFrontier = Model.LineClass( (FVector*)&NewLocation, (FVector*)TargetPoint ) == 1; // Unobstructed line? //tbi: cast
                if( Info.TargetViewedAtFrontier )
                {
                    Info.TargetViewCount++;
                }
            }
            ExploreHere = NewLocation;
            BestInfoAge++;
            if( BestInfoAge > 5 ) // Just so we don't do too many comparisons.
            {
                BestExploration( BestInfo, Info, Kind );
                BestInfoAge = 0;
            }
        }
    }
    if( BestInfoAge > 0 )
    {
        BestExploration( BestInfo, Info, Kind );
    }
    if( DebuggingExploration )debugf( LOG_Info, "Explore from %3.0f, %3.0f, %3.0f", Actor.Location.X, Actor.Location.Y, Actor.Location.Z );
    if( DebuggingExploration )debugf( LOG_Info, "   direction %3.0f, %3.0f, %3.0f", Direction.X, Direction.Y, Direction.Z );
    if( DebuggingExploration )debugf( LOG_Info, "        info steps=%i distance=%3.2f", int(Info.StepCount), Info.Distance );
    if( BestInfo.NearsEdge && BestInfo.StepCount >= 4 )
    {
        // Back off from the edge:
        const int OldStepCount = BestInfo.StepCount;
        const int NewStepCount = OldStepCount-2;
        BestInfo.StepCount = NewStepCount;        
        BestInfo.Distance = BestInfo.Distance * NewStepCount / OldStepCount; // An approximation only.
        FVector FrontierAdjustment = BestInfo.Frontier - BestInfo.Origin;
        FrontierAdjustment *= FLOAT(NewStepCount/OldStepCount);
        BestInfo.Frontier = BestInfo.Origin + FrontierAdjustment;
    }
    return Info;
}

// Try to achieve a motion goal, at least partly.
static void actorTryToAchieveMotionGoal( FActor & Actor, AIMotion & Motion )
{
    actorTryToAchieveLocationGoal( Actor, Motion.Location );
    actorTryToAchieveRotationGoal( Actor, Motion.Rotation );
}




// Continue the current AI task.
static void continueActorTask( INDEX iActor )
{
    FActor      &   Actor   = FActor::Actor(iActor);
    AActorAI    &   AI      = Actor.AI()       ;
    APawn       &   Pawn    = Actor.Pawn()     ;
    
    if( Actor.AITask != EAI_TaskMove && !AI.BasicMotion.IsEmpty() )
    {
        actorTryToAchieveMotionGoal( Actor, AI.BasicMotion );
    }

    switch( Actor.AITask )
    {
        case EAI_TaskNone:
        {
            break;
        }
        case EAI_TaskAttack:
        {
//?            FActor & Target = FActor::Actor(Actor.iTarget);
//?            FActor::Send_HarmTarget(iActor);
            break;
        }
        case EAI_TaskMove:
        {
            actorTryToAchieveMotionGoal( Actor, AI.MotionGoal );
            if( actorAchievedMotionGoal(Actor,AI.MotionGoal) )
            {
                Actor.FinishAnimations();
                Actor.StopTimer(); // Stops the time limit on the task.
                Actor.AITask = EAI_TaskNone;
                //?endActorTask(iActor);
            }
            break;
        }
        case EAI_TaskSearch:
        {
//?            FActor::Send_Search(iActor);
            break;
        }
        case EAI_TaskWait:
        {
//?            if( !Actor.IsAnimated() ) 
//?            {
//?                endActorTask( iActor );
//?            }
            break;
        }
        default:
        {
            checkLogic( 0 ); // False seems to not be defined in debug version -Tim
        }
    }
}

// Update status of important AI variables. Should be
// called before general AI processing, since general
// AI relies on accurate (current) information.
static void updateAI( FActor & Actor )
{
    GUARD;
    updateTargetInfo(Actor);
    UNGUARD("updateAI");
}


static void actorAI( INDEX iActor )
{
    GUARD;
    #if AllowPerformanceMeasurements
        static DWORD TotalCycleCount = 0; // Total cycles spent handling message
        static int   ExecutionCount  = 0; // Number of times this code was executed
        static DWORD MaxCycleCount   = 0; // Max cycles spent any one time
        const DWORD StartTime = TimeCpuCycles();
    #endif
    FActor      &   Actor   = FActor::Actor(iActor);
    AActorAI    &   AI      = Actor.AI()       ;
    APawn       &   Pawn    = Actor.Pawn()     ;

    // Periodically do checks on all the players to see if the monster can sense
    // a target. Don't do this if we are already concentrating on another task.
    updateAI( Actor );
//?    Actor.Stop();
    if( Pawn.LifeState == LS_Alive )
    {
        // Periodic check slightly randomized so all pawns don't check at once.
        if( (GServer.Ticks&0xf) == (iActor&0xf) || GServer.Ticks&GCheat->MonsterSlowMotionValue != 0 ) 
        {
            if ( Pawn.bSensesTargets && Actor.iTarget == INDEX_NONE )
            {
                // Find all player actors:
                {
                    UActorList & Actors = *GLevel->Actors;
                    INDEX        iTarget  = INDEX_NONE;
	                for( int Which = 0; iTarget == INDEX_NONE && Which < Actors.DynamicActors->Count(); Which++)
                    {
                        AActor * const CheckActor = (*Actors.DynamicActors)[Which];
                        if( CheckActor != 0 && CheckActor->Class->IsKindOf(GClasses.Player) )
                        {
                            FActor & TargetActor = FActor::Actor( *CheckActor );
                            if( Actor.CanSense( TargetActor.Pawn() ) )
                            {
                                if( Pawn.LurkingDistance == 0 || (Pawn.Location-TargetActor.Location).SizeSquared() <= UnitsInDekaUnits(Pawn.LurkingDistance)*UnitsInDekaUnits(Pawn.LurkingDistance) )
                                {
                                    iTarget = TargetActor.iMe;
                                }
                            }
                        }
                    }
                    if( iTarget != INDEX_NONE )
                    {
                        FActor & TargetActor = FActor::Actor( Actors.Element(iTarget) );
                        //tbc: handle seeing/hearing differently
                        Pawn.bIsQuiescent = FALSE;
                        PSense Info;
                        Info.SensedActor    = iTarget               ;
                        Info.LocationKnown  = TRUE                  ;
                        Info.Location       = TargetActor.Location  ;
                        FActor::Send_SensedSomething(iActor,Info);
                    }
                }
            }
            else if
            ( 
                Actor.HasTarget()
            && 
                (
                       Actor.Target().Pawn().bHasInvisibility
                    || !Actor.MightSense( Actor.Target().Pawn() )
                )
         
            )
            {
                Pawn.bTargetWasLost = TRUE;
                FActor::Send_UnTarget(iActor);
            }
        }
    }

    if( Pawn.LifeState == LS_Alive )
    {
        //tbi: Conversion
        if( BYTE(Pawn.bTargetWasNear) != Pawn.bTargetIsNear ) // Change in "nearness" of target?
        {
            if( Pawn.bTargetIsNear )
            {
                FActor::Send_TargetIsNear(iActor);
                //tbi: This should be unnecessary once collision detection
                // is matched up with actor movement.
                //?FActor::Send_Touch(Pawn.iTarget,iActor);
            }
            else
            {
                FActor::Send_TargetMovedAway(iActor);
            }
        }
    }
    

    if( Pawn.AITask != 0 )
    {
        if( Pawn.TimerCountdown == 0 )
        {
            // Task is done - we are wait for the animation to finish...
            if( !Actor.IsAnimated() )
            {
                Pawn.AITask = EAI_TaskNone;
            }
        }
        else
        {
            continueActorTask(iActor);
        }
    }
#if 0 //tbd: obsolete
    // Proceed with any task currently in progress.
    //tbi: Encapsulate this into an FActor function.
    if( !AI.IsEmpty() )
    {
        AI.Tick();
        if( AI.HasExpired() )
        {
            endActorTask( iActor );
        }
        else
        {
            continueActorTask( iActor );
        }
    }
#endif
    if( Pawn.AITask != 0 )
    {
        if( Pawn.AITask == EAI_TaskMove )
        {
            //todo: Yuck. Do this in the right place, not every time here!
            Pawn.AIPreviousMove = Pawn.AIMove;
        }
        //todo: Yuck. Do this in the right place, not every time here!
        Pawn.AIPreviousTask = Pawn.AITask;
    }
    else if( Actor.IsAnimated() && !Pawn.bIsQuiescent )
    {
        // There is no task, but we have some animations to finish.
        //todo: We don't have to do this over and over.
        Actor.FinishAnimations();
        Actor.StopMoving();
    }
    else if( Pawn.LifeState == LS_Alive )
    {
//?        if( Actor.IsAnimated() ) 
//?        {
//?            Actor.FinishAnimations();
//?        }
#if 0 //tbd: obsolete
        else if( Pawn.AINextTask != 0 )
        {
            StartTask( Actor, EAI_Task(Pawn.AINextTask), EAI_Subtask(Pawn.AINextSubtask) );
        }
#endif
//?        else
        {
            Actor.Stop();
            Pawn.AITask = EAI_TaskNone;
            //tbd?if( Pawn.bTargetIsNear )
            //tbd?{
            //tbd?    FActor::Send_TargetIsNear(iActor);
            //tbd?}
            //tbd?else 
            if( Pawn.bIsQuiescent )
            {
                if( Pawn.bIsAlarmed )
                {
                    Actor.DoSearch( 7 );
                    Pawn.bIsAlarmed = FALSE;
                }
            }
            else 
            {
                FActor::Send_DoSomething(iActor);
            }
        }
    }
    #if AllowPerformanceMeasurements
    {
        const DWORD EndTime = TimeCpuCycles();
        const DWORD CycleCount = EndTime-StartTime;
        TotalCycleCount += CycleCount;
        ExecutionCount++;
        if( CycleCount > MaxCycleCount )
        {
            MaxCycleCount = CycleCount;
        }
        if( GCheat->MeasurePerformance && (GServer.Ticks&0x1ff) == 0 )
        {
            const DWORD Average = TotalCycleCount / ExecutionCount;
            debugf( LOG_Info, "ActorAI processing: Average %i cycles [Max:%i]", int(Average), int(MaxCycleCount) );
            TotalCycleCount = 0;
            ExecutionCount  = 0;
            MaxCycleCount   = 0;
        }
    }
    #endif
    UNGUARD("ActorAI");
}


void actorTick (INDEX iActor)
{
    GUARD;
    IModel  &   Model       = GLevel->ModelInfo;
    FActor  &   Actor       = FActor::Actor(iActor);
    APawn   &   Pawn        = Actor.Pawn();
    Pawn.Velocity *= MotionDamping;
    if( Pawn.Velocity.Size2D() < StopThresholdSpeed*StopThresholdSpeed )
    {
        Pawn.Velocity = GMath.ZeroVector;
    }
    if( !GCheat->NoBrains && Pawn.bHasAI )
    {
        actorAI(iActor);
    }
    WORD                iCollisionNode;
    FBspNode            *FloorNode;
    FBspSurf            *FloorPoly;
    FVector                Floor,*FloorNormal,MoveVector,Gravity;
    Pawn.DrawRot.Yaw += Pawn.YawSpeed; // Update yaw.
    if (Model.PointClass (&Pawn.Location,NULL) == 0)
    {
        // Trapped in wall
        Pawn.Location.MoveBounded(Pawn.Velocity);
    }
    else
    {
        INDEX iActorHit;
        iCollisionNode = Model.ZCollision(&Pawn.Location,&Floor,&iActorHit);
        if (iCollisionNode != MAXWORD) // Don't fall if standing on air
        {
            FloorNode   = &Model.BspNodes [iCollisionNode];
            FloorPoly   = &Model.BspSurfs [FloorNode->iSurf];
            FloorNormal = &Model.FVectors [FloorPoly->vNormal];
            //
            if ((FloorNormal->Z < RAMP_MAX_Z_NORM) && (FloorNormal->Z > RAMP_MIN_Z_NORM)) // Ramp
            {
                Pawn.Velocity.X += 0.30 * (FloorNormal->X - 0.10) / FloorNormal->Z;
                Pawn.Velocity.Y += 0.30 * (FloorNormal->Y - 0.10) / FloorNormal->Z;
            }
            // Falling:
            if( Actor.bGravity )
            {
                Gravity = GLevel->GetZoneGravityAcceleration(iActor);
                Pawn.Velocity += Gravity;
            }
        }
        if (Pawn.Velocity.Z < -MAX_FALLING_Z_SPEED)
        {
            Pawn.Velocity.Z = -MAX_FALLING_Z_SPEED; // Drag
        }
        const FVector PreviousLocation = Actor.Location;
        {
            if( Actor.bGravity )
            {
                //tbd?Pawn.Velocity.Z = 0;
            }
            MoveVector = Pawn.Velocity ;
            //tbd? ModelInfo->SphereMove(&Pawn.Location, &MoveVector,COLLISION_SPHERE_RADIUS,1);
//tbd?            if( MoveVector.Size() > 0.1 )
            {
                GLevel->MoveActor(iActor,&MoveVector);
            }
        }

        WORD iCollisionNode = Actor.bGravity ? Model.ZCollision(&Pawn.Location,&Floor,&iActorHit) : MAXWORD;
        if( iCollisionNode != MAXWORD )
        {
            const FLOAT DistanceAboveFloor = Pawn.Location.Z - Actor.CollisionHeight - Floor.Z;
            if( Pawn.bIsOnSurface && DistanceAboveFloor > Pawn.MaxStepDownHeight )
            {
                // The pawn just walked off an edge. Let's try to take back that action
                // by moving him back to his original location and reversing his velocity,
                // to some limited degree (since the pawn might be *forced* off the edge).
                Pawn.Location = PreviousLocation;
                FVector VelocityReversal = Pawn.Velocity;
                VelocityReversal.Normalize();
                VelocityReversal *= -3.0 * Pawn.NormalSpeed;
                Pawn.Velocity += VelocityReversal;
                MoveVector = Pawn.Velocity;
                GLevel->MoveActor(iActor,&MoveVector);
                iCollisionNode = Model.ZCollision(&Pawn.Location,&Floor,&iActorHit);
            }
        }

        if ( iCollisionNode != MAXWORD )
        {
            const FLOAT dZ = Pawn.Location.Z - Actor.CollisionHeight - Floor.Z;
            const BOOL WasOnSurface = Pawn.bIsOnSurface;
            Pawn.bIsOnSurface = dZ<=0;
            if( dZ < 0 ) // Pawn has sunk into floor?
            {
                FVector Adjustment = GMath.ZeroVector;
                Adjustment.Z = -dZ;
                Model.SphereMove (&Pawn.Location,&Adjustment,Actor.CollisionRadius,1);
            }
            if( !WasOnSurface && Pawn.bIsOnSurface )
            {
                // The pawn just landed - check to see how far it fell.
                const FLOAT FallDistance = Pawn.LastNonFallZ - Pawn.Location.Z;
                const FLOAT SafeDistance = 100;
                if( FallDistance > SafeDistance )
                {
                    PHit HitInfo;
                    HitInfo.Empty();
                    HitInfo.Damage[DMT_Basic] = FallDistance/25;
                    Actor.Send_Hit(HitInfo);
                }
            }
        }
    }
    if( !Pawn.bGravity || Pawn.bIsOnSurface )
    {
        //tbi: This is duplicated here and in actorTick.
        Pawn.LastNonFallZ = Pawn.Location.Z;
    }
    UNGUARD("actorTick");
}

// Explore in a direction (possibly yawed then pitched), and save in Info
// the best exploration (best of the existing Info and the new
// new exploration). The following information in the existing
// exploration Info is used to establish the new exploration 
// info:
//    Info.Origin
//    Info.HasTarget
//    Info.Target
static void TryExploration
(
    FActor            & Actor
,   TExploreInfo      & Info
,   const FVector     & Direction
,   int                 dYaw     
,   int                 dPitch     
,   const FVector     * TargetPoint
,   TExplorationKind    Kind
)
{
    IModel & Model = GLevel->ModelInfo;
    FVector NewDirection = Direction;
    if( dYaw != 0 )
    {
        Yaw(NewDirection,dYaw);
    }
    if( dPitch != 0 )
    {
        Pitch(NewDirection,dPitch);
    }
    TExploreInfo NewInfo = Explore
    (
        Actor
    ,   NewDirection
    ,   TargetPoint
    ,   Meters(10.0) //tbd
    ,   Kind
    );
    BestExploration(Info,NewInfo,Kind);
}

// Explore in a direction (possibly yawed), and save in Info
// the best exploration (best of the existing Info and the new
// new exploration). The following information in the existing
// exploration Info is used to establish the new exploration 
// info:
//    Info.Origin
//    Info.HasTarget
//    Info.Target
static void TryExploration
(
    FActor            & Actor
,   TExploreInfo      & Info
,   const FVector     & Direction
,   int                 dYaw     
,   const FVector     * TargetPoint
,   TExplorationKind    Kind
)
{
    IModel & Model = GLevel->ModelInfo;
    FVector NewDirection = Direction;
    if( dYaw != 0 )
    {
        Yaw(NewDirection,dYaw);
    }
    TExploreInfo NewInfo = Explore
    (
        Actor
    ,   NewDirection
    ,   TargetPoint
    ,   Meters(10.0) //tbd
    ,   Kind
    );
    BestExploration(Info,NewInfo,Kind);
}

// Find a good direction of forward exploration. 
// If Directly==TRUE, motion directly towards the  target is preferred if possible.
static BOOL ExploreForward
(
    FActor            & Actor
,   const FVector     & Target          // Desirable location.
,   BOOL                Directly
,   FVector           & ChosenLocation  // Output explored frontier.
,   TExplorationKind    Kind
)
{
    IModel & Model = GLevel->ModelInfo;
    FVector DirectDirection = Target - Actor.Location;
    TExploreInfo Info;
    Info.Empty( Actor.Location, &Target );
    BOOL Found = FALSE;
    if( Directly )
    {
        TryExploration(Actor,Info,DirectDirection,Degrees(  0),&Target,Kind);
        Found = Info.StepCount >= 4;
    }
    if( !Found  )
    {
        const FVector & DD = DirectDirection; // For brevity
        FActor & A = Actor;
        // Try left or right:
        //tbi: Randomize 
        TryExploration(A,Info,DD,Degrees( 30),&Target,Kind);
        TryExploration(A,Info,DD,Degrees(-30),&Target,Kind);
        TryExploration(A,Info,DD,Degrees(-50),&Target,Kind);
        TryExploration(A,Info,DD,Degrees( 50),&Target,Kind);
        TryExploration(A,Info,DD,Degrees(-80),&Target,Kind);
        TryExploration(A,Info,DD,Degrees( 80),&Target,Kind);
        if( !Actor.bGravity )
        {
            //                             dYaw        dPitch
            //                         ------------ ------------
            TryExploration(A,Info,DD,Degrees(  0),Degrees( 40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(  0),Degrees(-40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(  0),Degrees( 80),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(  0),Degrees(-80),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 40),Degrees( 40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-40),Degrees( 40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-40),Degrees(-40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 40),Degrees(-40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 80),Degrees( 40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-80),Degrees( 40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-80),Degrees(-40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 80),Degrees(-40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 80),Degrees( 80),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-80),Degrees( 80),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-80),Degrees(-80),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 80),Degrees(-80),&Target,Kind);
        }
        Found = Info.StepCount >= 4;
    }
    if( !Found && !Directly )
    {
        // Try directly:
        TryExploration(Actor,Info,DirectDirection,Degrees(  0),&Target,Kind);
        Found = Info.StepCount >= 4;
    }
    ChosenLocation = Info.Frontier;
    if( DebuggingExploration )debugf( LOG_Info, "ExploreForward: => %i, %3.0f, %3.0f, %3.0f", int(Found), ChosenLocation.X, ChosenLocation.Y, ChosenLocation.Z );
    return Found;
}

// Find a good direction of backward exploration. 
static BOOL ExploreBackward
(
    FActor            & Actor
,   const FVector     & Target   // Desirable location.
,   BOOL                Directly
,   FVector           & ChosenLocation // Output explored frontier.
,   TExplorationKind    Kind
)
{
    IModel & Model = GLevel->ModelInfo;
    FVector DirectDirection = Target - Actor.Location;
    TExploreInfo Info;
    Info.Empty( Actor.Location, &Target );
    BOOL Found = FALSE;
    if( Directly )
    {
        TryExploration(Actor,Info,DirectDirection,Degrees(180),&Target,Kind);
        Found = Info.StepCount >= 4;
    }
    if( !Found  )
    {
        const FVector & DD = DirectDirection; // For brevity
        IModel & M = Model; // For brevity
        FActor & A = Actor;
        // Try left or right:
        //tbi: Randomize 
        TryExploration(A,Info,DD,Degrees( 100),&Target,Kind);
        TryExploration(A,Info,DD,Degrees(-100),&Target,Kind);
        TryExploration(A,Info,DD,Degrees( 130),&Target,Kind);
        TryExploration(A,Info,DD,Degrees(-130),&Target,Kind);
        TryExploration(A,Info,DD,Degrees( 150),&Target,Kind);
        TryExploration(A,Info,DD,Degrees(-150),&Target,Kind);
        //TryExploration(A,Info,DD,Degrees(180),&Target,Kind);
        if( !Actor.bGravity )
        {
            //                             dYaw        dPitch
            //                         ------------ ------------
            TryExploration(A,Info,DD,Degrees( 100),Degrees(  40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-100),Degrees(  40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 100),Degrees( -40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-100),Degrees( -40),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 180),Degrees(  50),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 180),Degrees( -50),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(   0),Degrees(  90),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(   0),Degrees( -90),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-150),Degrees( -60),&Target,Kind);
            TryExploration(A,Info,DD,Degrees(-150),Degrees(  60),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 150),Degrees( -60),&Target,Kind);
            TryExploration(A,Info,DD,Degrees( 150),Degrees(  60),&Target,Kind);
        }
        Found = Info.StepCount >= 4;
    }
    if( !Found && !Directly )
    {
        // Try directly backwards:
        TryExploration(Actor,Info,DirectDirection,Degrees(180),&Target,Kind);
        Found = Info.StepCount >= 4;
    }
    ChosenLocation = Info.Frontier;
    if( DebuggingExploration )debugf( LOG_Info, "ExploreBackward: => %i, %3.0f, %3.0f, %3.0f", int(Found), ChosenLocation.X, ChosenLocation.Y, ChosenLocation.Z );
    return Found;
}

static FVector ChooseLocation
(
    FActor            & Actor
,   const FVector     & Target
,   BOOL                BackwardsIsOkay
,   BOOL                Directly
,   TExplorationKind    Kind
)
{
    #if AllowPerformanceMeasurements
        static DWORD TotalCycleCount = 0; // Total cycles spent handling message
        static int   ExecutionCount  = 0; // Number of times this code was executed
        static DWORD MaxCycleCount   = 0; // Max cycles spent any one time
        const DWORD StartTime = TimeCpuCycles();
    #endif
    IModel & Model = GLevel->ModelInfo;
    FVector Choice;
    BOOL Found = FALSE;
    if( Kind == NormalExploration )
    {
        Found = ExploreForward(Actor,Target,Directly,Choice,Kind);
    }
    if( !Found && BackwardsIsOkay )
    {
        Found = ExploreBackward(Actor,Target,Directly,Choice,Kind);
    }
    if( !Found && Kind != NormalExploration )
    {
        Found = ExploreForward(Actor,Target,FALSE,Choice,Kind);
    }
    if( !Found )
    {
        Choice = Target;
        Debug( "Could not choose a location" );
    }
    #if AllowPerformanceMeasurements
    {
        const DWORD EndTime = TimeCpuCycles();
        const DWORD CycleCount = EndTime-StartTime;
        TotalCycleCount += CycleCount;
        ExecutionCount++;
        if( CycleCount > MaxCycleCount )
        {
            MaxCycleCount = CycleCount;
        }
        if( GCheat->MeasurePerformance && (GServer.Ticks&0x1ff) == 0 )
        {
            const DWORD Average = TotalCycleCount / ExecutionCount;
            debugf( LOG_Info, "ChooseLocation processing: Average %i cycles [Max:%i]", int(Average), int(MaxCycleCount) );
            TotalCycleCount = 0;
            ExecutionCount  = 0;
            MaxCycleCount   = 0;
        }
    }
    #endif
    return Choice;
}


//----------------------------------------------------------------------------
//                    General Pawn processing
//----------------------------------------------------------------------------
int APawn::Process(ILevel *Level, FName Message, void *Params)
{
    GUARD;
    FVector        BehindAdjustment;
    FActor      &   Actor   = FActor::Actor(*this) ;
    APawn       &   Pawn    = Actor.Pawn()         ;
    IModel      &   Model   = Level->ModelInfo     ;
    AActorAI    &   AI      = Actor.AI()           ;
    const BOOL RunningAway = int(Pawn.Health) < Pawn.RunAwayHealthThreshold;
    //
    switch (Message.Index)
    {
        //--------------------------------------------------------------------
        //                    Initialization
        //--------------------------------------------------------------------
        case ACTOR_Spawn:
        case ACTOR_BeginPlay:
        {
            ARoot::Process(Level,Message,Params); //tbc!
            // Here is where all the generalized pawn initialization belongs:
            Pawn.bStatusChanged = TRUE;
            Pawn.bIsQuiescent = TRUE;
            Pawn.LifeState = LS_Alive;
            Pawn.bTargetIsNear = FALSE;
            Pawn.bTargetWasLost = FALSE;
            Pawn.AITask         = EAI_TaskNone;
            Pawn.AIPreviousTask = EAI_TaskNone;
            Pawn.AIMove = EAI_MoveNone;
            Pawn.AIPreviousMove = EAI_MoveNone;
            Pawn.Stamina = 1.0;
            Pawn.LastNonFallZ = Pawn.Location.Z;
            Pawn.iRecentWeapons[0] = INDEX_NONE;
            Pawn.iRecentWeapons[1] = INDEX_NONE;
            Pawn.TargetPitch = Pawn.DrawRot.Pitch;
            Pawn.TargetYaw = Pawn.DrawRot.Yaw;
            Pawn.TargetRoll = Pawn.DrawRot.Roll;
            Pawn.iPendingTeleporter = INDEX_NONE;
            if( Actor.IsPlayer() )
            {
                bCannotTurn = FALSE;
                bCannotMove = FALSE;
            }
            checkVital( sizeof(AI) <= sizeof(Actor.AIInfo), "AIInfo is too big!" );
            Actor.CancelAnimations();
            Actor.Send_Animate( PAnimate::StillAnimation );
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                    Time passes
        //--------------------------------------------------------------------
        case ACTOR_PlayerTick:
        case ACTOR_Tick:
        {
            #if AllowPerformanceMeasurements
                static DWORD TotalCycleCount = 0; // Total cycles spent handling message
                static int   ExecutionCount  = 0; // Number of times this code was executed
                static DWORD MaxCycleCount   = 0; // Max cycles spent any one time
                const DWORD StartTime = TimeCpuCycles();
            #endif
            if
            ( 
                GCheat->MonsterSlowMotionValue != 0 
            &&  !Actor.IsPlayer()
            &&  (GServer.Ticks&GCheat->MonsterSlowMotionValue) != 0
            )
            {
                return ProcessDone;
            }
            if( GCheat->IsWizard && !Actor.IsPlayer() && Pawn.LifeState==LS_Alive && GInput.Switches[FInput::S_Home].IsOn && (GServer.Ticks&0x3)==0 )
            {
                Actor.DumpAI();
            }
            const BOOL DebugNoise = FALSE; //TRUE to debug changes in player's noise level.
            {
                // Any noise made by the pawn decays with time.
                // Below is an unrealistically slow decay rate, but this lets monsters
                // check for noises infrequently, and can make game-play more interesting
                // as the noise "clings" to the player.
                static FLOAT LastNoise; //tbd: for debugging
                Pawn.Noise *= 0.85; 
                if( DebugNoise && Actor.IsPlayer() )
                {
                    if( (Pawn.Noise >= 0.1 && Pawn.Noise > LastNoise) || (GServer.Ticks&0x1f)==0 )
                    {
                        debugf( LOG_Info, "Noise: %3.3f", Pawn.Noise );
                    }
                    LastNoise = Pawn.Noise;
                }

            }
            if( Pawn.ExplorationTimer > 0 )
            {
                Pawn.ExplorationTimer--;
            }
            if( Pawn.HitDisplayTimer > 0 )
            {
                Pawn.HitDisplayTimer--;
            }
            if( Pawn.SoundTimer > 0 )
            {
                Pawn.SoundTimer--;
            }
            if( Pawn.ExplosiveCharge > 0 )
            {
                Pawn.ExplosiveCharge *= 0.996; // Decay any explosive charge.
                if( Pawn.ExplosiveCharge <= 0.4 )
                {
                    Pawn.ExplosiveCharge = 0;
                }
                if( Pawn.ExplosiveCharge == 0 || (GServer.Ticks&0xf)==(iMe&0xf) )
                {
                    const AActor & Default = Pawn.Class->DefaultActor;
                    int Hue = 1 + int(Default.LightHue) + Pawn.ExplosiveCharge * 2;
                    int Brightness = 1+int(Default.LightBrightness) + Pawn.ExplosiveCharge * 1;
                    int Radius = 1 + int(Default.LightRadius) + Pawn.ExplosiveCharge * 10;
                    Pawn.LightHue = OurMin( 255, Hue );
                    Pawn.LightBrightness = OurMin( 255, Brightness );
                    Pawn.LightRadius = OurMin( 255, Radius );
                    if( Pawn.ExplosiveCharge > 0 )
                    {
                        Pawn.LightType = LT_Pulse;
                        Pawn.LightPeriod = 10;
                    }
                    else
                    {
                        Pawn.LightType = Default.LightType;
                        Pawn.LightPeriod = Default.LightPeriod;
                    }
                }
            }
            //todo: encapsulate into a helper function
            if( Pawn.DelayedDamage != 0 )
            {
                Pawn.DamageDelay--;
                if( Pawn.DamageDelay==0 )
                {
                    // Spit out two explosions, a slow one and a fast one.
                    for( int Which = 1; Which <= 2; ++Which )
                    {
                        FActor * Effect = Actor.SpawnPyrotechnic( Pawn.ExplosionEffect, 0, 0, 0 );
                        if( Effect != 0 )
                        {
                            Effect->Velocity.X = Random(-2.0,2.0);
                            Effect->Velocity.Y = Random(-2.0,2.0);
                            Effect->Velocity.Z = Random(0.0,3.0);
                            if( Which==1 ) // First one?
                            {
                                Effect->Velocity *= 0.2; // Slow it down
                            }
                        }
                    }
                    FLOAT Damage = Pawn.DelayedDamage;
                    if( Damage > 10 ) Damage = 10;
                    Pawn.DelayedDamage -= Damage;
                    Actor.CauseDamage( Actor, Damage, 0 );
                    if( Pawn.DelayedDamage > 0 )
                    {
                        Pawn.DamageDelay = Random(5,10);
                    }
                }
            }
            if( bHasInvisibility )
            {
                InvisibilityTimeLeft--;
                bHasInvisibility = InvisibilityTimeLeft > 0;
                if( !bHasInvisibility )
                {
                    Pawn.bStatusChanged = TRUE;
                }
            }
            if( bHasInvincibility )
            {
                InvincibilityTimeLeft--;
                bHasInvincibility = InvincibilityTimeLeft > 0;
                if( !bHasInvincibility )
                {
                    Pawn.bStatusChanged = TRUE;
                }
            }
            if( bHasSilence )
            {
                SilenceTimeLeft--;
                bHasSilence = SilenceTimeLeft > 0;
                if( !bHasSilence )
                {
                    Pawn.bStatusChanged = TRUE;
                }
            }
            if( bHasSuperStrength )
            {
                SuperStrengthTimeLeft--;
                bHasSuperStrength = SuperStrengthTimeLeft > 0;
                if( !bHasSuperStrength )
                {
                    Pawn.bStatusChanged = TRUE;
                }
            }
            if( bHasSuperStamina )
            {
                SuperStaminaTimeLeft--;
                bHasSuperStamina = SuperStaminaTimeLeft > 0;
                if( !bHasSuperStamina )
                {
                    Pawn.bStatusChanged = TRUE;
                }
            }
            if( Health < 1.0 && LifeState == LS_Alive)
            {
                Actor.Send_Die();
            }
            else if( Health < 100.0 )
            {
                Health += Pawn.HealRate;
            }
            if( Actor.IsPlayer() ) { GCheat->ApplyPendingCheats(Actor.iMe); }
            if( Message.Index == ACTOR_PlayerTick )
            {
                // The playerTick() function manipulates the ViewRot, and we use
                // this to set the DrawRot yaw. For now, we always draw the 
                // player with roll=0 and pitch=0, but this will change when
                // the player can fly as other creatures.
                GGame.PlayerTick(iMe,Params);
                DrawRot.Yaw = ViewRot.Yaw;
                DrawRot.Pitch = 0;
                DrawRot.Roll  = 0;
            }
            else
            {
                actorTick(iMe);
            }

            #if AllowPerformanceMeasurements
            {
                const DWORD EndTime = TimeCpuCycles();
                const DWORD CycleCount = EndTime-StartTime;
                TotalCycleCount += CycleCount;
                ExecutionCount++;
                if( CycleCount > MaxCycleCount )
                {
                    MaxCycleCount = CycleCount;
                }
                if( GCheat->MeasurePerformance && (GServer.Ticks&0x1ff) == 0 )
                {
                    const DWORD Average = TotalCycleCount / ExecutionCount;
                    debugf( LOG_Info, "Pawn Tick processing: Average %i cycles [Max:%i]", int(Average), int(MaxCycleCount) );
                    TotalCycleCount = 0;
                    ExecutionCount  = 0;
                    MaxCycleCount   = 0;
                }
            }
            #endif

            return ProcessParent;
            break;
        }
        //--------------------------------------------------------------------
        //                    
        //--------------------------------------------------------------------
        case ACTOR_PlayerCalcView:
        {
            PCalcView *ViewInfo;
            ViewInfo = (PCalcView *)Params;
            ViewInfo->ViewLocation = Location + GMath.ZAxisVector * EyeHeight;

            //todo: This input handling should be in the engine, and the collected
            // movement info should be put into the message parameters.
            if( !GServer.IsPaused )
            {
                FVector Move;
                FFloatRotation Rotation;
                GCameraManager->GetStoredMove( User, &Move, &Rotation ); // Called just so that dmTick is called.
                //todo: Too much detail here - FAction should hide this stuff from us.
                GInput.GatherInput();
                GAction.UpdateMovementActions(GInput);
                GAction.UpdateFixedActions();
                GAction.TransformActions();
                GGame.PlayerYaw( Pawn, GAction.Movements[PlayerAxis_Yaw].Analog, GAction.Movements[PlayerAxis_Yaw].Differential );
                GGame.PlayerPitch( Pawn, GAction.Movements[PlayerAxis_Pitch].Analog, GAction.Movements[PlayerAxis_Pitch].Differential );
                // Clear those movements which have been added to the player's motion:
                GAction.Movements[PlayerAxis_Yaw].Empty();
                GAction.Movements[PlayerAxis_Pitch].Empty();
                DrawRot.Yaw   = ViewRot.Yaw     ;
                DrawRot.Pitch = ViewRot.Pitch   ;
            }
            if (bBehindView)
            {
                BehindAdjustment = GMath.ZAxisVector * 60.0;
                BehindAdjustment.TransformVector (*ViewInfo->Uncoords);
                ViewInfo->ViewLocation -= BehindAdjustment;
            }
            ViewInfo->ViewRotation = ViewRot;
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                    
        //--------------------------------------------------------------------
        case ACTOR_DoSomething:
        {
            Debug( "%s: Do something.", Class->Name );
            if( Pawn.iTarget != INDEX_NONE  )
            {
                FActor & Target = FActor::Actor(Actor.iTarget);
                if
                ( 
                        Pawn.AIPreviousTask == EAI_TaskMove 
                    &&  Pawn.AttackPeriod != 0 
                    &&  (Pawn.bTargetIsNear || Pawn.bHasDistantMovingAttack || Pawn.bHasDistantStillAttack)
                    &&  FRandom::Percent(80) 
                )
                {
                    // We just moved, now do some damage...
                    Debug( "Cause harm" );
                    Actor.Send_HarmTarget();
                }
                else if( Pawn.ExplorationTimer > 0 )
                {
                    // We don't want to do any exploration yet.
                    Debug( "Wait for exploration timer to expire" );
                    Actor.DoWait( Pawn.ExplorationTimer );
                    Actor.Send_Animate( PAnimate::IdleAnimation );
                }
                else if( RunningAway )
                {
                    const int Period = DSeconds( Random(15,25) );
                    Debug( "Run away!" );
                    // Pawn is hurt and likes to run away ... so run away...
                    Actor.Near
                    (
                        ChooseLocation(Actor,Target.Location,TRUE,FRandom::Percent(30),RunAwayExploration)
                    ,   Period
                    );
                    Actor.Send_Animate( PAnimate::MoveAnimation );
                    Actor.SendWhenTaskIsDone( ACTOR_HarmTarget ); // Do some harm later.
                    Pawn.AIMove = EAI_MoveRunAway;
                    Pawn.ExplorationTimer = 40; //tbi? Hard-coded value?
                }
                else if
                ( 
                        (Pawn.AIPreviousTask==EAI_TaskAttack  || Pawn.AIPreviousMove == EAI_MoveApproach )
                    &&  Pawn.BackOffPeriod != 0 
                    && 
                        (
                            Pawn.BackOffThreshold == 0
                        ||  (Target.Location - Pawn.Location).SizeSquared() <= Squared(UnitsInDekaUnits(Pawn.BackOffThreshold)) 
                        )
                )
                {
                    // Pawn wants to back off a little.
                    Debug( "Back off." );
                    const int Period = DSeconds( Random( int(Pawn.BackOffPeriod)-Pawn.BackOffRandomization, int(Pawn.BackOffPeriod)+Pawn.BackOffRandomization ) );
                    if( Pawn.NormalSpeed == 0 )
                    {
                        // Pawn doesn't move, so just have him idle...
                        Actor.DoWait( Period );
                        Actor.Send_Animate( PAnimate::IdleAnimation );
                    }
                    else
                    {
                        Actor.Near
                        (
                            ChooseLocation(Actor,Target.Location,TRUE,FALSE,BackOffExploration)
                        ,   Period
                        );
                        Pawn.ExplorationTimer = 40; //tbi? Hard-coded value?
                        Actor.Send_Animate( PAnimate::MoveAnimation );
                    }
                    Actor.SendWhenTaskIsDone( ACTOR_HarmTarget ); // Do some harm later.
                    Pawn.AIMove = EAI_MoveBackOff;
                }
                else
                {
                    Debug( "Maybe move towards target" );
                    // Move towards the target.
                    const int Period = Pawn.NormalSpeed == 0 ? 0 : DSeconds( Random( int(Pawn.ApproachPeriod)-Pawn.ApproachRandomization, int(Pawn.ApproachPeriod)+Pawn.ApproachRandomization ) );
                    if( Period != 0 )
                    {
                        Actor.Near
                        (
                            ChooseLocation(Actor,Target.Location,TRUE,FRandom::Percent(50),NormalExploration)
                        ,   Period
                        );
                        Pawn.ExplorationTimer = 40; //tbi? Hard-coded value?
                        Actor.Send_Animate( PAnimate::MoveAnimation );
                        Actor.SendWhenTaskIsDone( ACTOR_HarmTarget ); // Do some harm later.
                        Pawn.AIMove = EAI_MoveApproach;
                    }
                    else
                    {
                        // Don't bother moving - do some harm now.
                        Actor.Send_HarmTarget();
                    }
                }
            }
            else if( Pawn.bTargetWasLost && Pawn.bTargetWasHere )
            {
                Debug( "%s: Search", Class->Name );
                //Actor.DoSearch( DSeconds( Random(20,60) ) );
                Pawn.bTargetWasLost = FALSE; // Forget about the lost target
                Actor.Send_Search();
            }
            else if( Pawn.ExplorationTimer > 0 )
            {
                // We don't want to do any exploration yet.
                Actor.DoWait( Pawn.ExplorationTimer );
                Actor.Send_Animate( PAnimate::IdleAnimation );
            }
            else if( Pawn.bTargetWasLost && !RunningAway )
            {
                Debug( "%s: Hunt", Class->Name );
                Actor.Near
                (
                    ChooseLocation(Actor,Pawn.TargetLastLocation,TRUE,TRUE,NormalExploration)
                ,   Seconds(FRandom::Value(3,4)) 
                );
                Actor.Send_Animate( PAnimate::MoveAnimation );
                Actor.SendWhenTaskIsDone( ACTOR_Search ); // Do some harm later.
            }
            else 
            {
                // No target. Go to sleep.
                Actor.Send_AllIsQuiet();
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                    Actor was hit
        //--------------------------------------------------------------------
        case ACTOR_Hit:
        {
            PHit & Info = PHit::Convert(Params);
            const int PreviousHealthGroup = int(Pawn.Health)/5; //tbe!
            Actor.TakeDamage(Info);
            const INDEX iHarmer = Info.iSourceActor;
            const int ActualDamage = Info.ActualDamage;
            const int NewHealthGroup = int(Pawn.Health)/5; //tbe!
            if( Pawn.LifeState != LS_Alive )
            {
            }
            else if( ActualDamage >= 8 )
            {
                Actor.PickSound(MajorInjurySounds,arrayCount_(MajorInjurySounds));
            }
            else if( ActualDamage >= 4 )
            {
                Actor.PickSound(MinorInjurySounds,arrayCount_(MinorInjurySounds));
            }
            else if( PreviousHealthGroup != NewHealthGroup  )
            {
                Actor.PickSound(MinorInjurySounds,arrayCount_(MinorInjurySounds));
            }
            if( !GCheat->NoBrains && !Actor.IsPlayer() && ActualDamage > 0 )
            {
                //Debug( "Monster was damaged: %i", int(ActualDamage) );
                if( Pawn.LifeState == LS_Alive )
                {
                    // Interrupt the current animations and tasks to take the hit.
                    // Don't always do this, or repeated hits will render the monster
                    // ineffective and easy to kill.
                    if( Pawn.HitDisplayTimer==0 && FRandom::Percent(75) )
                    {
                        Pawn.HitDisplayTimer = DSeconds(30); //todo: Make the delay a property?
                        Actor.ClearTask(TRUE);
                        Actor.StopMoving();
                        Actor.StopTurning();
                        Actor.DoWait(2);
                        Actor.Send_Animate( PAnimate::HitAnimation );
                    }
                }
            }
            if( Actor.IsPlayer() ) // Player-controlled pawn:
            {
                if( Pawn.LifeState == LS_Alive && iHarmer != INDEX_NONE )
                {
                    FActor & Harmer = FActor::Actor(iHarmer);
                    // Change the pitch of the actor:
                    if( Actor.IsFacing(Harmer.Location) )
                    {
                        Actor.ViewRot.Pitch += 1000;
                    }
                    else
                    {
                        Actor.ViewRot.Pitch -= 1000;
                    }
                }
            }
            else if( Pawn.LifeState != LS_Alive )
            {
                // Monster dies.
            }
            else if( iTarget != INDEX_NONE ) // Monster has a target
            {
                // Do some damage to the current target, even if it is
                // not the source of our pain.
                Actor.Send_HarmTarget();
            }
            else  // Monster has no target.
            {
                // Turn towards the source of the damage.
                if( iHarmer != INDEX_NONE )
                {
                    Actor.TurnTowards( FActor::Actor(iHarmer).Location, Degrees(180) );
                }
            }
            return ProcessDone;
        }
        //--------------------------------------------------------------------
        //                Should actor accept pickup?
        //--------------------------------------------------------------------
        case ACTOR_PickupQuery:
        {
            // We touched a pickup and we have a chance to accept it or refuse it.
            // We don't actually pick it up here - if we accept it we will later get
            // an ACTOR_AddInventory message.
            const PPickupQuery & Info = *(PPickupQuery*)Params;
            const INDEX iPickUp = Info.iActor;
            FActor & PickUpActor = FActor::Actor(iPickUp);
            AInventory & PickUp = PickUpActor.Inventory();
            BOOL PickItUp = FALSE;
            if( !Actor.IsPlayer() )
            {
                PickItUp = FALSE; // Pawn is not under user control.
            }
            else if( PickUp.Class->IsKindOf(GClasses.PowerUp) )
            {
                PickItUp = Actor.CanUsePowerUp(PickUpActor.PowerUp(),FALSE);
            }
            else if( PickUp.Class->IsKindOf(GClasses.Ammo) )
            {
                PickItUp = Actor.CanUseAmmo(PickUpActor.Ammo(),FALSE);
            }
            else
            {
                PickItUp = TRUE;
            }
            return PickItUp ? 1 : -1; // Return 1 to accept, -1 to refuse.
            break;
        }
        //--------------------------------------------------------------------
        //            Actor was just given a new inventory item
        //--------------------------------------------------------------------
        case ACTOR_PickupNotify:
        {
            //
            // We were just given a new inventory item.
            //
            PActor        *InvInfo = (PActor*)Params;
            INDEX        iNewInv  = InvInfo->iActor;
            AInventory    &NewInv  = FActor::Inventory(iNewInv);
            if (NewInv.Class->IsKindOf(GClasses.Weapon))
            {
                AWeapon & NewWeapon = FActor::Weapon(iNewInv);
                FActor * CurrentWeapon = FActor::Handle(iWeapon);
                // Also may have grabbed some ammo in the weapon...
                Actor.CanUseAmmo( EAmmoType(NewWeapon.AmmoType), NewWeapon.PickupAmmoCount, TRUE );
                if 
                (
                    (CurrentWeapon==0) 
                ||  (    NewWeapon.AutoSwitchPriority > CurrentWeapon->Weapon().AutoSwitchPriority 
                      && GPreferences.SwitchToNewWeapon 
                    )
                )
                {
                    Actor.Send_SwitchInventory(iNewInv);
                }
                //tbd: done in addinventory handling...Actor.Send_TextMsg( NewInv.PickupMessage);
            }
            Pawn.bStatusChanged = TRUE;
            return ProcessDone;
        }
        //--------------------------------------------------------------------
        //                    Change current inventory item (weapon)
        //--------------------------------------------------------------------
        case ACTOR_SwitchInventory:
        {
            PActor *InvInfo = (PActor*)Params;
            const INDEX iSwitchFrom = iWeapon           ;
            const INDEX iSwitchTo   = InvInfo->iActor   ;
            if (iSwitchFrom != iSwitchTo)
            {
                if (iSwitchFrom != INDEX_NONE)
                {
                    // Tell the current weapon to deactivate and then activate the new weapon.
                    FActor::Send_DeActivate(iSwitchFrom,iSwitchTo);
                }
                else
                {
                    // No current weapon, switch directly to the new weapon.
                    iWeapon = iSwitchTo;
                    if (iSwitchTo != INDEX_NONE)
                    {
                        FActor::Send_Activate(iWeapon);
                    }
                }
                Pawn.bStatusChanged = TRUE;
            }
            return ProcessDone;
        }
        //--------------------------------------------------------------------
        //                    Display a message
        //--------------------------------------------------------------------
        case ACTOR_TextMsg:
        {
            PText *Msg = (PText *)Params;
            if (Actor.IsPlayer())
            {
                User->Console->Log((ELogType)Msg->MsgType, Msg->Message);
            }
            return ProcessDone;
        }
        //--------------------------------------------------------------------
        //                    Actor has just died
        //--------------------------------------------------------------------
        case ACTOR_Die:
        {
            Actor.Send_SwitchInventory( INDEX_NONE ); // Lower the current weapon.
            Debug( "%s: Died", Class->Name );
            Actor.Stop();
            GAudio.SfxStopActor(iMe);
            Actor.ClearTask(TRUE);
            Actor.bGravity = TRUE;
            Actor.PickSound(DeathSounds,arrayCount_(DeathSounds));
            Pawn.DeathCount++;
            if( !Actor.IsPlayer() )
            {
                Pawn.LifeState = LS_Dead;
                Actor.SetActorCollision(FALSE);
                if( Pawn.iKiller != INDEX_NONE )
                {
                    FActor::Send_KillCredit( Pawn.iKiller );
                }
                if( DeathSpawn != 0 )
                {
                    FVector SpawnLocation = Actor.Location;
                    SpawnLocation.Z += 5;
                    INDEX iNewActor = GLevel->SpawnActor(DeathSpawn,NAME_NONE,&SpawnLocation);
                    if( iNewActor != INDEX_NONE ) 
                    {
                        FActor & NewActor = FActor::Actor(iNewActor);
                        NewActor.Velocity.Make( 9.0, 0, 0 );
                        NewActor.Velocity -= Level->GetZoneGravityAcceleration(iNewActor) * 8;
                        const int Angle = Random(0,60000); //tbi: Constants
                        Yaw( NewActor.Velocity, Angle );
                        NewActor.bGravity = TRUE;
                        NewActor.bCollideWorld = TRUE;
                    }
                }
            }
            Actor.DoWait(2);
            Actor.Send_Animate( PAnimate::DeathAnimation );
            Pawn.bStatusChanged = TRUE;
            Actor.TriggerEvent(); // Trigger any event for the death of this pawn.
            #if 0 //tbd: obsolete
            if( EventName != NAME_NONE ) // Trigger other events on death of pawn...
            {
                PTouch Info;
                Info.iActor = iMe;
                Level->SendMessageEx(ACTOR_Trigger,&Info,INDEX_NONE,EventName,NULL);
            }
            #endif
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                    Actor just killed something
        //--------------------------------------------------------------------
        case ACTOR_KillCredit:
        {
            Pawn.KillCount++;
            Actor.PickSound(VictorySounds,arrayCount_(VictorySounds));
            Pawn.bStatusChanged = TRUE;
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                 Actor should cause harm to current target
        //--------------------------------------------------------------------
        case ACTOR_HarmTarget:
        {
            if( Actor.HasTarget() && Pawn.AITask != EAI_TaskAttack )
            {
                const BOOL UsingNearAttack = Pawn.bTargetIsNear && Pawn.bHasCloseUpAttack;
                const BOOL UsingDistantAttack = !UsingNearAttack && (Pawn.bHasDistantMovingAttack || Pawn.bHasDistantStillAttack);
                const BOOL UsingMovingAttack = 
                        RunningAway                     ? FALSE
                    :   !UsingDistantAttack             ? FALSE
                    :   !Pawn.bHasDistantMovingAttack   ? FALSE
                    :   !Pawn.bHasDistantStillAttack    ? TRUE
                    :   FRandom::Percent(30)
                    ;
                Pawn.AITask = EAI_TaskNone;
                if( UsingNearAttack || UsingDistantAttack )
                {
                    Pawn.AITask = EAI_TaskAttack;
                    Actor.PickSound(AttackSounds,arrayCount_(AttackSounds));
                    FActor & Target = Actor.Target();
                    const int HarmPeriod = DSeconds( Random( int(Pawn.AttackPeriod)-Pawn.AttackRandomization, int(Pawn.AttackPeriod)+Pawn.AttackRandomization ) );
                    Actor.DoHarm( HarmPeriod );
                    Actor.TurnTowards(Target.Location,Degrees(180));
                    if( Actor.CanSee(Target.Pawn()) )
                    {
                        if( UsingMovingAttack )
                        {
                            Actor.KeepGoingTo(Actor.iTarget);
                        }
                        else
                        {
                            Actor.KeepFacing(Actor.iTarget);
                            Actor.Stop();
                        }
                        Actor.Send_Animate
                        ( 
                            UsingNearAttack     ? PAnimate::CloseUpAttackAnimation
                           :UsingMovingAttack   ? PAnimate::DistantMovingAttackAnimation
                           :                      PAnimate::DistantStillAttackAnimation
                        );
                    }
                }
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //           Add an inventory item to this actor's inventory
        //--------------------------------------------------------------------
        case ACTOR_AddInventory:
        {
            // If the actor chooses not to add the thing, it must kill the actor passed to it
            // and return -1.
            PActor & Info = *(PActor*)Params;
            INDEX iNew = Info.iActor;
            FActor & InventoryActor = FActor::Actor(iNew);
            AInventory & Inventory = InventoryActor.Inventory();
            Pawn.bStatusChanged = TRUE;
            Actor.Send_TextMsg( Inventory.PickupMessage );
            BOOL DestroyIt = FALSE;
            if( Inventory.Class->IsKindOf(GClasses.PowerUp) )
            {
                Actor.CanUsePowerUp(InventoryActor.PowerUp(),TRUE);
                DestroyIt = TRUE;
            }
            else if( Inventory.Class->IsKindOf(GClasses.Ammo) )
            {
                Actor.CanUseAmmo(InventoryActor.Ammo(),TRUE);
                DestroyIt = TRUE;
            }
            else if( Inventory.Class->IsKindOf(GClasses.Weapon) && Actor.InventoryItem(InventoryActor.Class) != 0 )
            {
                // The pawn already has the weapon, so just grab the ammo.
                const AWeapon & Weapon = InventoryActor.Weapon();
                Actor.CanUseAmmo( EAmmoType(Weapon.AmmoType), Weapon.PickupAmmoCount, TRUE );
                DestroyIt = TRUE;
            }
            if( DestroyIt )
            {
                //tbd: done by inventory handling...FActor::Actor(Inventory).MakeSound(Inventory.PickupSound);
                if (Inventory.Class->IsKindOf(GClasses.Ammo) && iWeapon==INDEX_NONE )
                {
                    Actor.Send_ChooseWeapon();
                }
                Level->DestroyActor(iNew);
                return -1;
            }
            else
            {
                return ProcessParent;
            }
            break;
        }
        //--------------------------------------------------------------------
        //                Find an appropriate weapon to ready
        //--------------------------------------------------------------------
        case ACTOR_ChooseWeapon:
        {
            // We try to find a "comparable" weapon - one with equal or greater
            // priority and enough ammo to be useful. iComparable tells us the *nearest*
            // comparable weapon (the one with the closest priority).
            INDEX   iComparable        = INDEX_NONE ; // Index of nearest comparable weapon, INDEX_NONE if not found.
            int     ComparablePriority ; // Priority of iComparable, or undefined if iComparable==INDEX_NONE.
            // We also keep track of the "best" weapon - the one with the highest priority
            // and enough ammo to be useful.
            INDEX   iBest              = INDEX_NONE ; // Index of best weapon, INDEX_NONE if not found.
            int     BestPriority       ; // Priority of iBest, or undefined if iBest==INDEX_NONE.

            AWeapon * CurrentWeapon = iWeapon==INDEX_NONE ? 0 : &FActor::Weapon(iWeapon);

            // Scan through the inventory chain, maintaining accurate iComparable and
            // iBest values:
            INDEX iCheck = iInventory;
            while( iCheck != INDEX_NONE )
            {
                //tbi: This assumes only weapons are in the inventory.
                AWeapon & Weapon = FActor::Weapon(iCheck);
                APawn & Parent = FActor::Pawn(Weapon.iParent);
                //tbi: Make a weapon function to determine if weapon has enough ammo to be useful.
                int         AmmoAvailable   = Parent.AmmoCount[Weapon.AmmoType];
                const BOOL  HasEnoughAmmo   = AmmoAvailable >= int(Weapon.AmmoUsed[0]) * int(Weapon.Discharges[0]);
                if( HasEnoughAmmo )
                {
                    // Update the best weapon so far:
                    if( iBest==INDEX_NONE || Weapon.AutoSwitchPriority > BestPriority )
                    {
                        iBest         = iCheck                      ;
                        BestPriority  = Weapon.AutoSwitchPriority  ;
                    }
                    // Update the nearest comparable weapon so far:
                    if( CurrentWeapon != 0 && Weapon.AutoSwitchPriority >= CurrentWeapon->AutoSwitchPriority )
                    {
                        if( iComparable==INDEX_NONE || Weapon.AutoSwitchPriority < ComparablePriority )
                        {
                            iComparable         = iCheck                      ;
                            ComparablePriority  = Weapon.AutoSwitchPriority  ;
                        }
                    }
                }
                iCheck = Weapon.iInventory;
            }
            const INDEX SwitchTo = iComparable != INDEX_NONE ? iComparable : iBest;
            if( SwitchTo != INDEX_NONE && SwitchTo != iWeapon )
            {
                Actor.Send_SwitchInventory( SwitchTo );
            }
            return ProcessDone;
        }
        //--------------------------------------------------------------------
        //                    Something just touched the actor
        //--------------------------------------------------------------------
        case ACTOR_Touch:
        {
            const PTouch & Info = *(PTouch*)Params;
            const INDEX iToucher = Info.iActor;
            if( iToucher != INDEX_NONE )
            {
                FActor & TouchingActor = FActor::Actor(iToucher);
                Debug( "A %s just touched a %s", TouchingActor.Class->Name, Actor.Class->Name );
                if( Actor.IsPlayer() )
                {
                    if( TouchingActor.Class->IsKindOf(GClasses.Pawn) )
                    {
                        APawn & Toucher = TouchingActor.Pawn();
                    }
                }
            }
            return ProcessParent;
            break;
        }
        //--------------------------------------------------------------------
        //                  The actor has sensed something
        //--------------------------------------------------------------------
        case ACTOR_SensedSomething:
        {
            const PSense & Info = PSense::Convert(Params);
            if( Info.SensedActor != INDEX_NONE )
            {
                FActor & SensedActor = FActor::Actor(Info.SensedActor);
                if( SensedActor.Class->IsKindOf( GClasses.Pawn ) && SensedActor.Pawn().User !=  0 )
                {
                    Debug( "%s: I sense you.", Class->Name );
		    Pawn.bIsQuiescent = FALSE;
                    Actor.ClearTask();
                    SetActorTarget(iMe,Info.SensedActor);
                }
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //            The actor's current target is no longer viable
        //--------------------------------------------------------------------
        case ACTOR_UnTarget:
        {
            Debug( "%s: Where are you?", Class->Name );
            SetActorTarget(iMe,INDEX_NONE);
            Actor.TurnTowards( Pawn.TargetLastLocation );
            Actor.ClearTask();
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //
        //--------------------------------------------------------------------
        case ACTOR_TargetIsNear:
        {
            Debug( "%s: Gotcha!", Class->Name );
            Actor.ClearTask(); 
            //?Actor.DoHarm(); // Cause continuous harm to the target.
            Actor.Send_HarmTarget();
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //
        //--------------------------------------------------------------------
        case ACTOR_TargetMovedAway:
        {
            Debug( "%s: Going somewhere?", Class->Name );
            // Stop whatever we were doing. The general actor AI
            // will make us follow the target.
            Actor.ClearTask(); 
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //
        //--------------------------------------------------------------------
        case ACTOR_Search:
        {
            Debug( "%s: Searching", Class->Name );
            Actor.DoMotion( DSeconds( FRandom::Value(100,150) ) );
            AI.MotionGoal.Spin( 0, 400, 0 );
            AI.MotionGoal.Rotation.Kind = AIMotion::RandomizedChange;
            Actor.Send_Animate( PAnimate::SearchAnimation );
            Actor.SendWhenTaskIsDone( ACTOR_AllIsQuiet );
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //
        //--------------------------------------------------------------------
        case ACTOR_AllIsQuiet:
        {
            Debug( "%s: Zzzzzz.", Class->Name );
            Actor.Stop();
            Actor.ClearTask(FALSE);
            Actor.Send_Animate( PAnimate::StillAnimation );
            Pawn.bIsQuiescent = TRUE;
            // Cheat a little by facing the direction of the last known location
            // of the target (if there was one).
            Actor.TurnTowards( Pawn.TargetLastLocation );
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //              Timer expired (timer is used to limit AI tasks)
        //--------------------------------------------------------------------
        case ACTOR_Timer:
        {
            // We use the timer to signal the end of the current AI task.
            // However, we will allow the animations to finish.
            //todo: Create a new message ACTOR_EndTask to avoid conflicts
            // with the ACTOR_Timer message.
            if( Actor.IsAnimated() ) 
            {
                Actor.FinishAnimations();
            }
            return ProcessDone;
            break;
        }
        //--------------------------------------------------------------------
        //                    End of animation
        //--------------------------------------------------------------------
        case ACTOR_EndAnimation:
        {
            // We don't do anything with this since we are always polling for
            // the completion of animations.
            return ProcessDone;
            break;
        }
    }
    return ProcessParent;
    UNGUARD("APawn::Process");
}

/*-----------------------------------------------------------------------------
    The End
-----------------------------------------------------------------------------*/
