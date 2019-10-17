/*===========================================================================
	C++ "Root" actor class definitions exported from UnrealEd
===========================================================================*/
#pragma pack (push,1) /* Actor class data must be unaligned */

///////////////////////////////////////////////////////
// Actor class ARoot
///////////////////////////////////////////////////////

enum EDrawType {
    DT_None                 =0,
    DT_Sprite               =1,
    DT_MeshMap              =2,
    DT_Brush                =3,
    DT_ParticleSystem       =4,
};

enum EBlitType {
    BT_None                 =0,
    BT_Normal               =1,
    BT_Transparent          =2,
    BT_Ghost                =3,
    BT_Glow                 =4,
    BT_Fuzzy                =5,
};

enum EParticleType {
    PT_None                 =0,
};

enum ELightType {
    LT_None                 =0,
    LT_Steady               =1,
    LT_Pulse                =2,
    LT_Blink                =3,
    LT_Flicker              =4,
    LT_Strobe               =5,
    LT_Explode2             =6,
    LT_Explode3             =7,
    LT_DayLight             =8,
    LT_NightLight           =9,
};

enum ELightEffect {
    LE_None                 =0,
    LE_TorchWaver           =1,
    LE_FireWaver            =2,
    LE_WateryShimmer        =3,
    LE_Searchlight          =4,
    LE_SlowWave             =5,
    LE_FastWave             =6,
    LE_CloudCast            =7,
    LE_StormCast            =8,
    LE_Shock                =9,
    LE_Disco                =10,
    LE_Warp                 =11,
    LE_NotImplemented       =12,
    LE_StolenQuakeWater     =13,
    LE_ChurningWater        =14,
    LE_NegativeLight        =15,
    LE_Interference         =16,
    LE_Cylinder             =17,
    LE_Rotor                =18,
};

enum EAI_Task {
    EAI_TaskNone            =0,
    EAI_TaskMove            =1,
    EAI_TaskSearch          =2,
    EAI_TaskWait            =3,
    EAI_TaskAttack          =4,
};

#ifndef AUTOREGISTER_CLASS
#define AUTOREGISTER_CLASS(ignore)
#endif

class ARoot : public AActorBase {
public:
    UClass     *Class;
    FVector    Location;
    FVector    Velocity;
    FRotation  DrawRot;
    FRotation  ViewRot;
    INDEX      iParent;
    INDEX      iWeapon;
    UTexture   *Texture;
    UMeshMap   *MeshMap;
    UModel     *Brush;
    USound     *AmbientSound;
    FLOAT      DrawScale;
    BYTE       AnimSeq;
    FLOAT      AnimRate;
    FLOAT      AnimBase;
    BYTE       AnimCount;
    BYTE       AnimFirst;
    BYTE       AnimLast;
    int        AnimMessage;
    FLOAT      CollisionRadius;
    FLOAT      CollisionHeight;
    BYTE       DrawType                 /* EDrawType            */;
    BYTE       BlitType                 /* EBlitType            */;
    BYTE       ParticleType             /* EParticleType        */;
    BYTE       ParticleCount;
    BYTE       ParticleRate;
    BYTE       SoundRadius;
    BYTE       LightType                /* ELightType           */;
    BYTE       LightEffect              /* ELightEffect         */;
    BYTE       LightBrightness;
    BYTE       LightHue;
    BYTE       LightSaturation;
    BYTE       LightRadius;
    BYTE       LightPeriod;
    BYTE       LightPhase;
    BYTE       LightCone;
    BYTE       InherentBrightness;
    DWORD      bAnimate:1;
    DWORD      bMeshWet:1;
    DWORD      bMeshShadowCast:1;
    DWORD      bMeshEnviroMap:1;
    DWORD      bSpriteRotates:1;
    DWORD      bUnused0:1;
    DWORD      bActorShadows:1;
    DWORD      bShinyReflect:1;
    DWORD      bSpecialLight:1;
    DWORD      bStaticActor:1;
    DWORD      bHidden:1;
    DWORD      bHiddenEd:1;
    DWORD      bDirectional:1;
    DWORD      bCollideActors:1;
    DWORD      bCollideWorld:1;
    DWORD      bBlocksActors:1;
    DWORD      bBehindView:1;
    DWORD      bSelected:1;
    DWORD      bTempDynamicLight:1;
    DWORD      bDrawOnHorizon:1;
    DWORD      bTemplateClass:1;
    DWORD      bTempLightChanged:1;
    DWORD      bUnlit:1;
    DWORD      bNoSmooth:1;
    DWORD      bUnused4:1;
    DWORD      bUnused5:1;
    DWORD      bUnused6:1;
    DWORD      bUnused7:1;
    DWORD      bUnused8:1;
    UCamera    *User;
    FName      Name;
    FName      EventName;
    INDEX      iTarget;
    INDEX      iInventory;
    INDEX      iFloor;
    FLOAT      Mass;
    DWORD      bInactive:1;
    DWORD      bPegged:1;
    DWORD      bGravity:1;
    DWORD      bMomentum:1;
    DWORD      bProjTarget:1;
    DWORD      bCanPossess:1;
    DWORD      bDifficulty1:1;
    DWORD      bDifficulty2:1;
    DWORD      bDifficulty3:1;
    DWORD      bDifficulty4:1;
    DWORD      bNetCooperative:1;
    DWORD      bNetDeathMatch:1;
    DWORD      bNetPersistent:1;
    DWORD      bNetNoMonsters:1;
    DWORD      bTempEditor:1;
    DWORD      bJustDeleted:1;
    DWORD      bCanBeTeleported:1;
    DWORD      bUnused12:1;
    DWORD      bUnused13:1;
    DWORD      bUnused14:1;
    DWORD      bUnused15:1;
    INDEX      iMe;
    BYTE       Zone;
    BYTE       ChanceOfExistence;
    int        LifeSpan;
    int        Age;
    int        Era;
    BYTE       TriggerSequences[10];
    BYTE       TriggerFrames[10];
    BYTE       TriggerValues[10];
    BYTE       WhichTriggers;
    int        TextureList;
    BYTE       TextureCount;
    BYTE       ScriptCountdown;
    int        TimerCountdown;
    int        TimerMessage;
    FLOAT      WaterSinkRate;
    BYTE       AITask                   /* EAI_Task             */;
    INDEX      iPendingTeleporter;
    BYTE       TeleportDelay;
    FName      DefaultEdCategory;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ARoot);

///////////////////////////////////////////////////////
// Actor class AProjectile:ARoot
///////////////////////////////////////////////////////

enum EDamageType {
    DMT_Basic               =0,
    DMT_Water               =1,
    DMT_Fire                =2,
    DMT_Electric            =3,
    DMT_Count               =4,
    DMT_None                =5,
};

class AProjectile : public ARoot {
public:
    DWORD      bVerticalSeek:1;
    DWORD      bFollowFloor:1;
    DWORD      bBounce:1;
    DWORD      bIsInstantHit:1;
    UClass     *EffectAtLifeSpan;
    UClass     *EffectOnWallImpact;
    UClass     *EffectOnPawnImpact;
    UClass     *EffectOnImpact;
    FLOAT      Speed;
    FLOAT      Acceleration;
    FLOAT      MaxSpeed;
    BYTE       MaxBounceCount;
    BYTE       BounceCount;
    FLOAT      BounceIncidence;
    FLOAT      FollowFloorHeight;
    UTexture   *Textures[20];
    FLOAT      Damage[4];
    FLOAT      DamageDecay[4];
    BYTE       ExplosiveTransfer;
    BYTE       Hack                     /* EDamageType          */;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AProjectile);

///////////////////////////////////////////////////////
// Actor class AFireball:AProjectile:ARoot
///////////////////////////////////////////////////////

class AFireball : public AProjectile {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AFireball);

///////////////////////////////////////////////////////
// Actor class AFireball2:AProjectile:ARoot
///////////////////////////////////////////////////////

class AFireball2 : public AProjectile {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AFireball2);

///////////////////////////////////////////////////////
// Actor class ABulletProjectile:AProjectile:ARoot
///////////////////////////////////////////////////////

class ABulletProjectile : public AProjectile {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ABulletProjectile);

///////////////////////////////////////////////////////
// Actor class AShellProjectile:AProjectile:ARoot
///////////////////////////////////////////////////////

class AShellProjectile : public AProjectile {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AShellProjectile);

///////////////////////////////////////////////////////
// Actor class AStingerProjectile:AProjectile:ARoot
///////////////////////////////////////////////////////

class AStingerProjectile : public AProjectile {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AStingerProjectile);

///////////////////////////////////////////////////////
// Actor class ATentacleProjectile:AProjectile:ARoot
///////////////////////////////////////////////////////

class ATentacleProjectile : public AProjectile {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ATentacleProjectile);

///////////////////////////////////////////////////////
// Actor class APawn:ARoot
///////////////////////////////////////////////////////

enum EAI_Move {
    EAI_MoveNone            =0,
    EAI_MoveApproach        =1,
    EAI_MoveBackOff         =2,
    EAI_MoveRunAway         =3,
};

enum ELifeState {
    LS_None                 =0,
    LS_Alive                =1,
    LS_Dying                =2,
    LS_Dead                 =3,
};

class APawn : public ARoot {
public:
    int        YawSpeed;
    int        PitchSpeed;
    int        RollSpeed;
    int        FixFrame;
    FLOAT      BaseEyeHeight;
    FLOAT      EyeHeight;
    FLOAT      Bob;
    FName      TeamName;
    int        TargetYaw;
    int        TargetPitch;
    int        TargetRoll;
    BYTE       KillCount;
    BYTE       DeathCount;
    BYTE       SecretCount;
    int        ShotCount;
    int        HitCount;
    int        DamagedCount;
    int        DamageCount;
    int        HealthCount;
    int        LowHealthCount;
    int        HighHealthCount;
    BYTE       ApproachPeriod;
    BYTE       ApproachRandomization;
    BYTE       AttackPeriod;
    BYTE       AttackRandomization;
    BYTE       BackOffPeriod;
    BYTE       BackOffRandomization;
    BYTE       BackOffThreshold;
    BYTE       TargetCloseness;
    BYTE       TargetZCloseness;
    BYTE       ApproachDirectness;
    DWORD      bHasAI:1;
    DWORD      bSensesTargets:1;
    DWORD      bRespondsToNoise:1;
    DWORD      bRespondsToSights:1;
    DWORD      bHarmsMonsters:1;
    DWORD      bNeverStill:1;
    DWORD      bIsXenophobic:1;
    BYTE       LurkingDistance;
    FLOAT      AuralAcuity;
    BYTE       RunAwayHealthThreshold;
    BYTE       AIMove                   /* EAI_Move             */;
    BYTE       AIPreviousTask           /* EAI_Task             */;
    BYTE       AIPreviousMove           /* EAI_Move             */;
    int        AmmoCount[10];
    USound     *MinorInjurySounds[3];
    USound     *MajorInjurySounds[3];
    USound     *DeathSounds[2];
    USound     *QuestingSounds[2];
    USound     *StillSounds[2];
    USound     *VictorySounds[2];
    USound     *AttackSounds[4];
    BYTE       SoundTimer;
    UClass     *AttackEffects[2];
    UClass     *ExplosionEffect;
    FLOAT      NaturalArmor[4];
    FLOAT      HealRate;
    FLOAT      Armor[4];
    FLOAT      SightRadius;
    FLOAT      PeripheralVision;
    BYTE       HearingAccuracy;
    BYTE       Visibility;
    FLOAT      Noise;
    FLOAT      LungeSpeed;
    FLOAT      RunSpeed;
    FLOAT      NormalSpeed;
    FLOAT      MaxStrength;
    FLOAT      Strength;
    FLOAT      Stamina;
    FLOAT      Health;
    FLOAT      DesiredSpeed;
    BYTE       LifeState                /* ELifeState           */;
    INDEX      iKiller;
    UClass     *DeathSpawn;
    BYTE       HitDisplayTimer;
    BYTE       ExplorationTimer;
    int        TargetLostTime;
    FVector    TargetLastLocation;
    FVector    TargetLastVelocity;
    DWORD      bHasDistantMovingAttack:1;
    DWORD      bHasDistantStillAttack:1;
    DWORD      bHasCloseUpAttack:1;
    FLOAT      LastNonFallZ;
    int        InvisibilityTimeLeft;
    int        SilenceTimeLeft;
    int        InvincibilityTimeLeft;
    int        SuperStaminaTimeLeft;
    int        SuperStrengthTimeLeft;
    FLOAT      ExplosiveCharge;
    FLOAT      DelayedDamage;
    BYTE       DamageDelay;
    DWORD      bHasInvisibility:1;
    DWORD      bHasSilence:1;
    DWORD      bHasInvincibility:1;
    DWORD      bHasSuperStrength:1;
    DWORD      bHasSuperStamina:1;
    DWORD      bIsQuiescent:1;
    DWORD      bTargetIsNear:1;
    DWORD      bTargetWasHere:1;
    DWORD      bTargetWasNear:1;
    DWORD      bTargetWasLost:1;
    DWORD      bLimitRotation:1;
    DWORD      bLookingAlongStair:1;
    DWORD      bIsOnSurface:1;
    DWORD      bCheated:1;
    DWORD      bStatusChanged:1;
    DWORD      bIsAlarmed:1;
    DWORD      bCannotTurn:1;
    DWORD      bCannotMove:1;
    BYTE       MaxStepUpHeight;
    BYTE       MaxStepDownHeight;
    int        AmmoCapacity[10];
    INDEX      iRecentWeapons[2];
    BYTE       HurtEffectCount;
    BYTE       PickupEffectCount;
    BYTE       HealthEffectCount;
    BYTE       TeleportEffectCount;
    int        bRenderUnderwater;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(APawn);

///////////////////////////////////////////////////////
// Actor class ACamera:APawn:ARoot
///////////////////////////////////////////////////////

class ACamera : public APawn {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ACamera);

///////////////////////////////////////////////////////
// Actor class AWoman:APawn:ARoot
///////////////////////////////////////////////////////

class AWoman : public APawn {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AWoman);

///////////////////////////////////////////////////////
// Actor class ADragon:APawn:ARoot
///////////////////////////////////////////////////////

class ADragon : public APawn {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ADragon);

///////////////////////////////////////////////////////
// Actor class ASkaarj:APawn:ARoot
///////////////////////////////////////////////////////

enum ESkaarjAnimationTriggers {
    SkaarjAT_None           =0,
    SkaarjAT_Lunge          =1,
    SkaarjAT_Spin           =2,
    SkaarjAT_Fire           =3,
    SkaarjAT_ClawLeft       =4,
    SkaarjAT_ClawRight      =5,
};

enum ESkaarjAnimations {
    SkaarjA_None            =0,
    SkaarjA_Squat           =1,
    SkaarjA_Blade           =2,
    SkaarjA_TwoClaw         =3,
    SkaarjA_Death           =4,
    SkaarjA_Fighter         =5,
    SkaarjA_HeadUp          =6,
    SkaarjA_Firing          =7,
    SkaarjA_Looking         =8,
    SkaarjA_Jog             =9,
    SkaarjA_Lunge           =10,
    SkaarjA_Spin            =11,
    SkaarjA_T1              =12,
    SkaarjA_T2              =13,
    SkaarjA_T3              =14,
    SkaarjA_T4              =15,
    SkaarjA_T5              =16,
    SkaarjA_T6              =17,
    SkaarjA_TakeHit         =18,
};

class ASkaarj : public APawn {
public:
    BYTE       Hack1                    /* ESkaarjAnimationTriggers */;
    BYTE       Hack2                    /* ESkaarjAnimations    */;
    USound     *LungeSound;
    USound     *SpinSound;
    USound     *ClawSound;
    USound     *ShootSound;
    BYTE       LungeDamage;
    BYTE       SpinDamage;
    BYTE       ClawDamage;
    BYTE       ShootDamage;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ASkaarj);

///////////////////////////////////////////////////////
// Actor class ABigMan:APawn:ARoot
///////////////////////////////////////////////////////

enum EBigManAnimationTriggers {
    BigManAT_None           =0,
    BigManAT_StillFireLeft  =1,
    BigManAT_StillFireRight =2,
    BigManAT_PistolWhip     =3,
    BigManAT_GutFire        =4,
    BigManAT_WalkFireLeft   =5,
    BigManAT_WalkFireRight  =6,
};

enum EBigManAnimations {
    BigManA_None            =0,
    BigManA_StillLook       =1,
    BigManA_StillFire       =2,
    BigManA_PistolWhip      =3,
    BigManA_Sleep           =4,
    BigManA_GutShot         =5,
    BigManA_DieForward      =6,
    BigManA_ShootLeft       =7,
    BigManA_Walk            =8,
    BigManA_WalkLeft        =9,
    BigManA_WalkRight       =10,
    BigManA_ShootRight      =11,
    BigManA_T1              =12,
    BigManA_T2              =13,
    BigManA_T3              =14,
    BigManA_T4              =15,
    BigManA_T5              =16,
    BigManA_TakeHit         =17,
    BigManA_DieBackward     =18,
};

class ABigMan : public APawn {
public:
    BYTE       Hack1                    /* EBigManAnimationTriggers */;
    BYTE       Hack2                    /* EBigManAnimations    */;
    USound     *ShootSound;
    USound     *WhipSound;
    BYTE       ShootDamage;
    BYTE       WhipDamage;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ABigMan);

///////////////////////////////////////////////////////
// Actor class AArchAngel:ABigMan:APawn:ARoot
///////////////////////////////////////////////////////

class AArchAngel : public ABigMan {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AArchAngel);

///////////////////////////////////////////////////////
// Actor class AGasbag:APawn:ARoot
///////////////////////////////////////////////////////

enum EGasBagAnimationTriggers {
    GasBagAT_None           =0,
    GasBagAT_PunchLeft      =1,
    GasBagAT_PunchRight     =2,
    GasBagAT_Pound          =3,
    GasBagAT_Belch          =4,
};

enum EGasBagAnimations {
    GasBagA_None            =0,
    GasBagA_TwoPunch        =1,
    GasBagA_Belch           =2,
    GasBagA_Deflate         =3,
    GasBagA_Fiddle          =4,
    GasBagA_Fighter         =5,
    GasBagA_Float           =6,
    GasBagA_Grab            =7,
    GasBagA_Pound           =8,
    GasBagA_T1              =9,
    GasBagA_T2              =10,
    GasBagA_T3              =11,
    GasBagA_T4              =12,
    GasBagA_TakeHit         =13,
};

class AGasbag : public APawn {
public:
    BYTE       Hack1                    /* EGasBagAnimationTriggers */;
    BYTE       Hack2                    /* EGasBagAnimations    */;
    USound     *BelchSound;
    USound     *PunchSound;
    USound     *PoundSound;
    BYTE       BelchDamage;
    BYTE       PunchDamage;
    BYTE       PoundDamage;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AGasbag);

///////////////////////////////////////////////////////
// Actor class AManta:APawn:ARoot
///////////////////////////////////////////////////////

enum EMantaAnimationTriggers {
    MantaAT_None            =0,
    MantaAT_Sting           =1,
    MantaAT_Whip            =2,
};

enum EMantaAnimations {
    MantaA_None             =0,
    MantaA_Fly              =1,
    MantaA_Sting            =2,
    MantaA_Whip             =3,
    MantaA_Die              =4,
    MantaA_Land             =5,
    MantaA_Launch           =6,
};

class AManta : public APawn {
public:
    BYTE       Hack1                    /* EMantaAnimationTriggers */;
    BYTE       Hack2                    /* EMantaAnimations     */;
    USound     *StingSound;
    USound     *WhipSound;
    BYTE       StingDamage;
    BYTE       WhipDamage;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AManta);

///////////////////////////////////////////////////////
// Actor class ATentacle:APawn:ARoot
///////////////////////////////////////////////////////

enum ETentacleAnimationTriggers {
    TentacleAT_None         =0,
    TentacleAT_Shoot        =1,
};

enum ETentacleAnimations {
    TentacleA_None          =0,
    TentacleA_Waver         =1,
    TentacleA_Shoot         =2,
    TentacleA_Mebax         =3,
    TentacleA_Death         =4,
};

class ATentacle : public APawn {
public:
    BYTE       Hack1                    /* ETentacleAnimationTriggers */;
    BYTE       Hack2                    /* ETentacleAnimations  */;
    USound     *ShootSound;
    USound     *MebaxSound;
    UClass     *Projectile;
    BYTE       WhipDamage;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ATentacle);

///////////////////////////////////////////////////////
// Actor class ALight:ARoot
///////////////////////////////////////////////////////

class ALight : public ARoot {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ALight);

///////////////////////////////////////////////////////
// Actor class AKeypoint:ARoot
///////////////////////////////////////////////////////

class AKeypoint : public ARoot {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AKeypoint);

///////////////////////////////////////////////////////
// Actor class APlayerStart:AKeypoint:ARoot
///////////////////////////////////////////////////////

class APlayerStart : public AKeypoint {
public:
    FName      TeamName;
    UClass     *PlayerSpawnClass;
};
AUTOREGISTER_CLASS(APlayerStart);

///////////////////////////////////////////////////////
// Actor class ATeleporter:AKeypoint:ARoot
///////////////////////////////////////////////////////

class ATeleporter : public AKeypoint {
public:
    char       TeleportURL[64];
    DWORD      bChangesVelocity:1;
    DWORD      bChangesYaw:1;
    DWORD      bReversesX:1;
    DWORD      bReversesY:1;
    DWORD      bReversesZ:1;
    FName      ProductRequired;
    int        TargetYaw;
    FVector    TargetVelocity;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ATeleporter);

///////////////////////////////////////////////////////
// Actor class ABlockMonsters:AKeypoint:ARoot
///////////////////////////////////////////////////////

class ABlockMonsters : public AKeypoint {
public:
};
AUTOREGISTER_CLASS(ABlockMonsters);

///////////////////////////////////////////////////////
// Actor class ABlockAll:AKeypoint:ARoot
///////////////////////////////////////////////////////

class ABlockAll : public AKeypoint {
public:
};
AUTOREGISTER_CLASS(ABlockAll);

///////////////////////////////////////////////////////
// Actor class ALevelDescriptor:AKeypoint:ARoot
///////////////////////////////////////////////////////

class ALevelDescriptor : public AKeypoint {
public:
    FVector    LevelGravity;
    char       LevelTitle[64];
    char       LevelAuthor[64];
    char       LevelMusic[64];
    FLOAT      Lightning;
};
AUTOREGISTER_CLASS(ALevelDescriptor);

///////////////////////////////////////////////////////
// Actor class AAmbientSound:AKeypoint:ARoot
///////////////////////////////////////////////////////

class AAmbientSound : public AKeypoint {
public:
};
AUTOREGISTER_CLASS(AAmbientSound);

///////////////////////////////////////////////////////
// Actor class ATextMessage:AKeypoint:ARoot
///////////////////////////////////////////////////////

class ATextMessage : public AKeypoint {
public:
};
AUTOREGISTER_CLASS(ATextMessage);

///////////////////////////////////////////////////////
// Actor class AZoneDescriptor:ARoot
///////////////////////////////////////////////////////

class AZoneDescriptor : public ARoot {
public:
    DWORD      bWaterZone:1;
    DWORD      bFogZone:1;
    DWORD      bRainZone:1;
    DWORD      bKillZone:1;
    DWORD      bEchoZone:1;
    DWORD      bNeutralZone:1;
    DWORD      bGravityZone:1;
    DWORD      bVelocityZone:1;
    FVector    ZoneGravity;
    FVector    ZoneVelocity;
    char       ZoneTitle[64];
    USound     *ZoneAmbientSound;
    BYTE       AmbientBrightness;
    BYTE       AmbientHue;
    BYTE       AmbientSaturation;
    BYTE       RampHue;
    BYTE       RampSaturation;
    BYTE       FogThickness;
    BYTE       FogHue;
    BYTE       FogSaturation;
    FLOAT      Lightning;
};
AUTOREGISTER_CLASS(AZoneDescriptor);

///////////////////////////////////////////////////////
// Actor class AMover:ARoot
///////////////////////////////////////////////////////

enum EMoverBumpType {
    MB_StopWhenBump         =0,
    MB_ReturnWhenBump       =1,
    MB_CrushWhenBump        =2,
};

enum EMoverTriggerType {
    MT_None                 =0,
    MT_TriggerOpenTimed     =1,
    MT_TriggerToggle        =2,
    MT_TriggerControl       =3,
    MT_TriggerCycleOn       =4,
    MT_TriggerCycleOff      =5,
    MT_TriggerInstant       =6,
    MT_ProximityOpenTimed   =7,
    MT_ProximityControl     =8,
    MT_StandOpenTimed       =9,
};

enum EMoverGlideType {
    MV_MoveByTime           =0,
    MV_GlideByTime          =1,
    MV_Sinusoid             =2,
};

class AMover : public ARoot {
public:
    BYTE       MoverBumpType            /* EMoverBumpType       */;
    BYTE       MoverTriggerType         /* EMoverTriggerType    */;
    BYTE       MoverGlideType           /* EMoverGlideType      */;
    BYTE       KeyNum;
    BYTE       WorldRaytraceKey;
    BYTE       BrushRaytraceKey;
    BYTE       PrevKeyNum;
    int        MoverTime;
    int        RemainOpenTime;
    FLOAT      BumpPlayerDamage;
    DWORD      bCanInterruptMove:1;
    DWORD      bSlave:1;
    DWORD      bTrigger:1;
    DWORD      bAdded:1;
    DWORD      bMoving:1;
    DWORD      bReverseWhenDone:1;
    DWORD      bDynamicShadows:1;
    DWORD      bTriggerOnceOnly:1;
    USound     *OpenSound;
    USound     *ClosedSound;
    USound     *MoveAmbientSound;
    FVector    KeyPos[4];
    FRotation  KeyRot[4];
    FRotation  FreeRotation;
    FVector    BasePos;
    FVector    OldPos;
    FRotation  BaseRot;
    FRotation  OldRot;
    int        CurTime;
    int        HoldTime;
    INDEX      iSlaves[16];
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AMover);

///////////////////////////////////////////////////////
// Actor class AInventory:ARoot
///////////////////////////////////////////////////////

enum EInvState {
    INV_None                =0,
    INV_Active              =1,
    INV_Activating          =2,
    INV_DeActivating        =3,
    INV_Using1              =4,
    INV_Using2              =5,
    INV_UsingCloseUp        =6,
    INV_Reloading           =7,
    INV_Playing             =8,
};

enum EInventorySet {
    INV_NoSet               =0,
    INV_WeaponSet1          =1,
    INV_WeaponSet2          =2,
    INV_WeaponSet3          =3,
    INV_WeaponSet4          =4,
    INV_WeaponSet5          =5,
    INV_WeaponSet6          =6,
    INV_WeaponSet7          =7,
    INV_WeaponSet8          =8,
    INV_WeaponSet9          =9,
    INV_WeaponSet10         =10,
};

class AInventory : public ARoot {
public:
    BYTE       InvState                 /* EInvState            */;
    INDEX      iNextActive;
    DWORD      bInPickupState:1;
    DWORD      bActiveInSet:1;
    DWORD      bNeedsReloading:1;
    USound     *PickupSound;
    USound     *RespawnSound;
    DWORD      bRespawnNetOnly:1;
    int        RespawnTime;
    char       PickupMessage[64];
    DWORD      bTakesDamage:1;
    UClass     *EffectWhenDestroyed;
    BYTE       OwningSet                /* EInventorySet        */;
    BYTE       AutoSwitchPriority;
    FLOAT      DrawForward;
    FLOAT      DrawDown;
    int        DrawPitch;
    int        DrawRoll;
    int        DrawYaw;
    UMeshMap   *PlayerViewMesh;
    FLOAT      PlayerViewScale;
    UMeshMap   *PickupMesh;
    FLOAT      PickupScale;
    UTexture   *AmmoStatusIcon;
    int        YawSpeed;
    int        PitchSpeed;
    int        RollSpeed;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AInventory);

///////////////////////////////////////////////////////
// Actor class AWeapon:AInventory:ARoot
///////////////////////////////////////////////////////

enum EAmmoType {
    AmmoType_Bullets        =0,
    AmmoType_Shells         =1,
    AmmoType_StingerAmmo    =2,
    AmmoType_FlameGunAmmo   =3,
    AmmoType_Reserved2      =4,
    AmmoType_Reserved3      =5,
    AmmoType_Reserved4      =6,
    AmmoType_Reserved5      =7,
    AmmoType_Reserved6      =8,
    AmmoType_Reserved7      =9,
    AmmoType_Count          =10,
    AmmoType_None           =11,
};

class AWeapon : public AInventory {
public:
    DWORD      bAutoVTarget:1;
    DWORD      bAutoHTarget:1;
    DWORD      bWasReleased:1;
    FLOAT      MaxTargetRange;
    FLOAT      SeekDamping;
    BYTE       AmmoType                 /* EAmmoType            */;
    int        AmmoUsed[2];
    BYTE       Discharges[2];
    BYTE       ReloadCount;
    FLOAT      Noise[2];
    BYTE       ReusePeriod[2];
    FLOAT      RecoilForce[2];
    BYTE       RecoilPitch[2];
    UClass     *MuzzleEffectClass[2];
    USound     *DischargeSounds[2];
    BYTE       bRepeatSounds[2];
    USound     *ReloadSound;
    USound     *CloseUpSound;
    int        PickupAmmoCount;
    BYTE       CloseUpDamage;
    FLOAT      CloseUpStrengthFactor;
    int        UseTime;
    int        LastUseTime;
    UClass     *ProjectileClass[2];
    FLOAT      ProjStartDist;
    FLOAT      Dispersion[2];
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AWeapon);

///////////////////////////////////////////////////////
// Actor class AAutoMag:AWeapon:AInventory:ARoot
///////////////////////////////////////////////////////

enum EAutoMagAnimations {
    EAMA_None               =0,
    EAMA_Still              =1,
    EAMA_Shoot              =2,
    EAMA_Shoot2             =3,
    EAMA_Twirl              =4,
    EAMA_Whip               =5,
    EAMA_T1                 =6,
    EAMA_T2                 =7,
};

enum EAutoMagAnimationTriggers {
    AutoMagAT_None          =0,
    AutoMagAT_Fire1         =1,
    AutoMagAT_Fire2         =2,
};

class AAutoMag : public AWeapon {
public:
    BYTE       Hack1                    /* EAutoMagAnimations   */;
    BYTE       Hack2                    /* EAutoMagAnimationTriggers */;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AAutoMag);

///////////////////////////////////////////////////////
// Actor class AQuadShot:AWeapon:AInventory:ARoot
///////////////////////////////////////////////////////

enum EQuadShotAnimations {
    EQSA_None               =0,
    EQSA_Fire               =1,
    EQSA_Reload             =2,
};

enum EQuadShotAnimationTriggers {
    QuadShotAT_None         =0,
    QuadShotAT_Fire         =1,
};

class AQuadShot : public AWeapon {
public:
    BYTE       Hack1                    /* EQuadShotAnimations  */;
    BYTE       Hack2                    /* EQuadShotAnimationTriggers */;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AQuadShot);

///////////////////////////////////////////////////////
// Actor class AFlameGun:AWeapon:AInventory:ARoot
///////////////////////////////////////////////////////

enum EFlameGunAnimations {
    EFGA_None               =0,
    EFGA_Still              =1,
    EFGA_Drop               =2,
    EFGA_Fire               =3,
};

class AFlameGun : public AWeapon {
public:
    BYTE       Hack1                    /* EFlameGunAnimations  */;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AFlameGun);

///////////////////////////////////////////////////////
// Actor class AStinger:AWeapon:AInventory:ARoot
///////////////////////////////////////////////////////

enum EStingerAnimations {
    EStingA_None            =0,
    EStingA_Still           =1,
    EStingA_Fire1           =2,
    EStingA_Fire3           =3,
};

enum EStingerAnimationTriggers {
    StingerAT_None          =0,
    StingerAT_Fire1         =1,
    StingerAT_Fire2         =2,
    StingerAT_Fire3         =3,
};

class AStinger : public AWeapon {
public:
    BYTE       Hack1                    /* EStingerAnimations   */;
    BYTE       Hack2                    /* EStingerAnimationTriggers */;
    BYTE       PendingShots[5];
    BYTE       PendingShotCount;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AStinger);

///////////////////////////////////////////////////////
// Actor class APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class APickup : public AInventory {
public:
    BYTE       Unused;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(APickup);

///////////////////////////////////////////////////////
// Actor class APowerUp:APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class APowerUp : public APickup {
public:
    FLOAT      Strength;
    FLOAT      Stamina;
    FLOAT      Health;
    FLOAT      Armor[4];
    int        TimeLimit;
    DWORD      bInvisibility:1;
    DWORD      bSilence:1;
    DWORD      bInvincibility:1;
    DWORD      bSuperStrength:1;
    DWORD      bSuperStamina:1;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(APowerUp);

///////////////////////////////////////////////////////
// Actor class AArmor:APowerUp:APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class AArmor : public APowerUp {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AArmor);

///////////////////////////////////////////////////////
// Actor class AHealth:APowerUp:APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class AHealth : public APowerUp {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AHealth);

///////////////////////////////////////////////////////
// Actor class AAmmo:APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class AAmmo : public APickup {
public:
    int        AmmoCount[10];
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AAmmo);

///////////////////////////////////////////////////////
// Actor class AClip:AAmmo:APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class AClip : public AAmmo {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AClip);

///////////////////////////////////////////////////////
// Actor class AShells:AAmmo:APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class AShells : public AAmmo {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AShells);

///////////////////////////////////////////////////////
// Actor class AStingerAmmo:AAmmo:APickup:AInventory:ARoot
///////////////////////////////////////////////////////

class AStingerAmmo : public AAmmo {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AStingerAmmo);

///////////////////////////////////////////////////////
// Actor class ATriggers:ARoot
///////////////////////////////////////////////////////

class ATriggers : public ARoot {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ATriggers);

///////////////////////////////////////////////////////
// Actor class ATrigger:ATriggers:ARoot
///////////////////////////////////////////////////////

enum ETriggerType {
    TT_Proximity            =0,
    TT_Use                  =1,
    TT_Shoot                =2,
};

class ATrigger : public ATriggers {
public:
    BYTE       TriggerType              /* ETriggerType         */;
    UClass     *ClassesToDetect;
    char       Message[80];
    DWORD      bShowMessage:1;
    DWORD      bTriggerOnceOnly:1;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ATrigger);

///////////////////////////////////////////////////////
// Actor class ACounter:ATriggers:ARoot
///////////////////////////////////////////////////////

class ACounter : public ATriggers {
public:
    BYTE       NumToCount;
    char       CountMessage[80];
    char       CompleteMessage[80];
    DWORD      bShowMessage:1;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ACounter);

///////////////////////////////////////////////////////
// Actor class ADispatcher:ATriggers:ARoot
///////////////////////////////////////////////////////

class ADispatcher : public ATriggers {
public:
    FName      OutEvents[8];
    int        OutDelays[8];
    DWORD      bActive:1;
    int        Count;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ADispatcher);

///////////////////////////////////////////////////////
// Actor class ADecorations:ARoot
///////////////////////////////////////////////////////

class ADecorations : public ARoot {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ADecorations);

///////////////////////////////////////////////////////
// Actor class AVase:ADecorations:ARoot
///////////////////////////////////////////////////////

class AVase : public ADecorations {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AVase);

///////////////////////////////////////////////////////
// Actor class AChandelier:ADecorations:ARoot
///////////////////////////////////////////////////////

class AChandelier : public ADecorations {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AChandelier);

///////////////////////////////////////////////////////
// Actor class AHammok:ADecorations:ARoot
///////////////////////////////////////////////////////

class AHammok : public ADecorations {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AHammok);

///////////////////////////////////////////////////////
// Actor class AFlame:ADecorations:ARoot
///////////////////////////////////////////////////////

class AFlame : public ADecorations {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AFlame);

///////////////////////////////////////////////////////
// Actor class APyrotechnics:ARoot
///////////////////////////////////////////////////////

class APyrotechnics : public ARoot {
public:
    FLOAT      GravityMult;
    FLOAT      AccelerationFactor;
    BYTE       Noise;
    USound     *InitialSound;
    UTexture   *Textures[20];
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(APyrotechnics);

///////////////////////////////////////////////////////
// Actor class AWallHit:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class AWallHit : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AWallHit);

///////////////////////////////////////////////////////
// Actor class APawnHit:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class APawnHit : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(APawnHit);

///////////////////////////////////////////////////////
// Actor class AExplode1:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class AExplode1 : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AExplode1);

///////////////////////////////////////////////////////
// Actor class AExplode3:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class AExplode3 : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AExplode3);

///////////////////////////////////////////////////////
// Actor class ABigManGunFlash:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class ABigManGunFlash : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ABigManGunFlash);

///////////////////////////////////////////////////////
// Actor class ASkaarjGunFlash:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class ASkaarjGunFlash : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(ASkaarjGunFlash);

///////////////////////////////////////////////////////
// Actor class AGasBagBelchFlash:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class AGasBagBelchFlash : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AGasBagBelchFlash);

///////////////////////////////////////////////////////
// Actor class APlayerRespawn:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class APlayerRespawn : public APyrotechnics {
public:
};
AUTOREGISTER_CLASS(APlayerRespawn);

///////////////////////////////////////////////////////
// Actor class ATeleportIn:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class ATeleportIn : public APyrotechnics {
public:
};
AUTOREGISTER_CLASS(ATeleportIn);

///////////////////////////////////////////////////////
// Actor class ATeleportOut:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class ATeleportOut : public APyrotechnics {
public:
};
AUTOREGISTER_CLASS(ATeleportOut);

///////////////////////////////////////////////////////
// Actor class AExplode2:APyrotechnics:ARoot
///////////////////////////////////////////////////////

class AExplode2 : public APyrotechnics {
public:
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AExplode2);

///////////////////////////////////////////////////////
// Actor class AExplosion:ARoot
///////////////////////////////////////////////////////

class AExplosion : public ARoot {
public:
    UClass     *Effect;
    USound     *InitialSound;
    BYTE       Noise;
    UClass     *Debris[10];
    int        DebrisCount;
    FLOAT      Radius;
    FLOAT      RadiusIncrement;
    BYTE       Damage[4];
    FLOAT      DamageIncrement;
    FLOAT      Momentum;
    FLOAT      MomentumIncrement;
    int Process(ILevel *Level, FName Message, void *Params);
};
AUTOREGISTER_CLASS(AExplosion);

///////////////////////////////////////////////////////
// Actor class AClipExplosion:AExplosion:ARoot
///////////////////////////////////////////////////////

class AClipExplosion : public AExplosion {
public:
};
AUTOREGISTER_CLASS(AClipExplosion);

///////////////////////////////////////////////////////
// Actor class AShellExplosion:AExplosion:ARoot
///////////////////////////////////////////////////////

class AShellExplosion : public AExplosion {
public:
};
AUTOREGISTER_CLASS(AShellExplosion);

///////////////////////////////////////////////////////
// Actor class ATarydiumExplosion:AExplosion:ARoot
///////////////////////////////////////////////////////

class ATarydiumExplosion : public AExplosion {
public:
};
AUTOREGISTER_CLASS(ATarydiumExplosion);

#pragma pack (pop) /* Restore alignment to previous setting */
