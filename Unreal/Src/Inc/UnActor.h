/*=============================================================================
	UnActor.h: Actor-related functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
        * Aug 30, 1996: Mark added PLevel
=============================================================================*/

#ifndef _INC_UNACTOR
#define _INC_UNACTOR

#include "UnMsgs.h"
#include "UnArray.h"

class ILevel;

/*-----------------------------------------------------------------------------
	AActorBase
-----------------------------------------------------------------------------*/

//
// Base class of all actors
//
#pragma pack (push,1) /* Turn off alignment so C and script code agree on member offsets */
class UNREAL_API AActorBase
	{
	public:
    int Process(ILevel *Level, FName Message, void *Params);
	};
#pragma pack (pop) /* Restore alignment */

//
// Pointer to actor class's 'Process' member function.  This is used for all
// communication with actors.
//
typedef int (AActorBase::*ACTOR_CLASS_FUNC)(ILevel *Level, FName Message, void *Params);

/*-----------------------------------------------------------------------------
	AActorDraw
-----------------------------------------------------------------------------*/

#ifdef _DO_NOT_DEFINE_CLASS_ENUMS /* Just declare the enums, UnRoot.h will define them */

enum EBlitType;
enum ELightType;
enum ELightEffect;
enum EDrawType;
enum EParticleType;

#else /* Define the enums */

//
// Blitting effects for blitting sprites and chunks of actor meshmaps.
//
enum EBlitType
	{
	BT_None				= 0x00,			// Do not display
	BT_Normal			= 0x01,			// Normal (masked), no special effects
	BT_Transparent		= 0x02,			// Transparent, uses color of sprite pixels
	BT_Ghost			= 0x03,			// Ghosting, uses color of background
	BT_Glow				= 0x04,			// Glow
	BT_Fuzzy			= 0x05,			// Fuzzed out
	BT_MAX				= 0x06,
	};

//
// Overall lighting types which modulate a light's brightness as a function of time.
//
enum ELightType
	{
	LT_None				= 0x00,			// This actor casts no light
	LT_Steady			= 0x01, 		// Steady global lighting
	LT_Pulse			= 0x02,			// Sine pulsing
	LT_Blink			= 0x03,			// Blinking on and off
	LT_Flicker			= 0x04,			// Strobes
	LT_Strobe			= 0x05, 		// Little explosion pattern
	LT_Explode2			= 0x06, 		// Medium explosion pattern
	LT_Explode3			= 0x07,			// Huge explosion pattern with radial shock waves
	LT_DayLight			= 0x08,			// Day lighting effect based on CPU clock
	LT_NightLight		= 0x09,			// Night lighting effect
	LT_MAX				= 0x0A,			// Maximum effect number
	};

//
// Localized lighting effects which modulate a light's brightness on a point by
// point basis.
//
enum ELightEffect
	{
	LE_None				= 0x00,			// No special effect
	LE_TorchWaver		= 0x01,			// Slight torch wavering effect
	LE_FireWaver		= 0x02,			// Big fire-like wavering
	LE_WateryShummer	= 0x03,			// Shimmers as if reflected off water
	LE_SearchLight		= 0x04,			// Yawing searchlight effect
	LE_SlowWave			= 0x05,			// Slow radial waves
	LE_FastWave			= 0x06,			// Fast radial waves
	LE_CloudCast		= 0x07,			// Shadows of light clouds
	LE_StormCast		= 0x08,			// Shadows of stormclouds
	LE_Shock			= 0x09,			// Shock wave
	LE_Disco			= 0x0A,			// Disco ball
	LE_Warp				= 0x0B,			// Warp texture U
	LE_NotImplemented	= 0x0C,			// Warp texture V
	LE_CalmWater		= 0x0D,			// Calm water
	LE_ChurningWater	= 0x0E,			// Churning water
	LE_NegativeLight	= 0x0F,			// Light should be subtracted
	LE_Interference		= 0x10,			// Interference pattern
	LE_Cylinder			= 0x11,			// Cylinder light
	LE_Rotor			= 0x12,			// Yawing rotor
	LE_UNUSED4			= 0x13,			// Available
	LE_MAX				= 0x14,			// Maximum effect number
	};

//
// Drawing type; determines which member of AActorDraw should be used for
// drawing the actor.
//
enum EDrawType
	{
	DT_None				= 0x00,			// Actor is not drawn
	DT_Sprite			= 0x01,			// Actor is a sprite  (uses Texture member)
	DT_MeshMap			= 0x02,			// Actor is a meshmap (uses MeshMap member)
	DT_Brush			= 0x03,			// Actor is a brush   (uses Brush member)
	DT_ParticleSystem	= 0x04,			// Actor is a particle system
	};

//
// Particle system effects.
//
enum EParticleType
	{
	PT_None				= 0x00,			// Particle system types - not yet defined
	};

#endif /* _DO_NOT_DEFINE_CLASS_ENUMS */

//
// Information for an actor that's associated with a camera.  These actors
// define a view which can be rendered.
//
class UNREAL_API FActorCameraStatus
	{
	public:
	FLOAT	OrthoZoom;		// Orthogonal/map view zoom factor
	FLOAT	FOVAngle;   	// X field of view angle in degrees (normally 100)
	WORD	ShowFlags;
	WORD	RendMap;
	WORD	Misc1;
	WORD	Misc2;
	};

//
// Information for an actor that's being controlled by AI.
//
class UNREAL_API FActorAIStatus	
	{
	public:
	BYTE	Pad[16];
	};

class FActorServerInfo
	{
	public:
	enum {MAX_TOUCHING_ACTORS=4};
	INDEX	iTouchingActors[MAX_TOUCHING_ACTORS];
	BYTE Reserved[64];
	};

//
// Minimal properties needed for clients to render and process autonomous 
// player movement relative to actors.  All actors are derived from this.
// This contains only client-required rendering and collision properties,
// not server specific information.
//
// WARNING!  All member variables declared here must be binary-compatible with the
// corresponding member variables in the definition ARoot (in Root.h), which
// is automatically generated by UnrealEd based on the root script.  This class
// is somewhat redundent, but exists in order to shield the Unreal engine from
// the details of the actor system scripting language in general.
//
#pragma pack (push,1) /* Turn off alignment so C and script code agree on member offsets */
class UNREAL_API AActorDraw : public AActorBase
	{
	public:
	//
	// Class the actor belongs to:
	//
	UClass		*Class;      		// NULL if this actor entry in a level's actor list is empty
	//
	// Positional info:
	//
	FVector		Location;			// Location in world
	FVector		Velocity;			// Velocity vector
	FRotation	DrawRot;			// Rotation to draw the actor with
	FRotation   ViewRot;     		// View rotation (actors with cameras/players)
	//
	// Links to other actors which may affect rendering:
	//
	INDEX       iParent;      		// Index of parent actor or INDEX_NONE
	INDEX       iWeapon;      		// Weapon actor or INDEX_NONE
	//
	// Drawing info:
	//
	UTexture	*Texture;			// Texture to draw, for sprites where DrawType==DT_Sprite
	UMeshMap	*MeshMap;			// Meshmap to draw, for meshes where DrawType==DT_MeshMap
	UModel		*Brush;				// Model to draw, for moving brushes where DrawType==DT_Brush
	USound      *AmbientSound;		// Ambient sound associated with actor
	FLOAT		DrawScale;			// Scaling, 1.0=default
	BYTE		AnimSeq;			// Animation sequence number being drawn
	FLOAT		AnimRate;			// Animation play rate, 1.0=default
	FLOAT		AnimBase;			// Base animation frame to draw
    BYTE        AnimCount;          // Number of times to do animation.
    BYTE        AnimFirst;          // First frame of animation.
    BYTE        AnimLast;           // Last frame of animation.
    INT         AnimMessage;
	FLOAT		CollisionRadius;	// Sphere radius during collision
	FLOAT		CollisionHeight;	// Height of collision sphere above floor
	BYTE		DrawType;			// EDrawType: Type of thing to draw
	BYTE		BlitType;			// EBlitType: Blit type affecting meshes and sprites
	BYTE		ParticleType;		// EParticleType: Type of particle system effect
	BYTE		ParticleCount;		// Number of particles to animate, sprites only
	BYTE		ParticleRate;		// How fast the particle system animates
	BYTE		SoundRadius;		// Radius of ambient sound, 1 unit = 25 world units
	BYTE		LightType;			// ELightType: Overall light effect
	BYTE		LightEffect;		// ELightEffect: Localized lighting effect
	BYTE		LightBrightness;	// Brightness magnitude at center, 0=fully dark, 256 = 1.5 times maximum
	BYTE		LightHue;			// Hue value, according to GGfx.HueTable
	BYTE		LightSaturation;	// Saturation, 0=fully colored, 255-white (backwards)
	BYTE		LightRadius;		// Constant light radius, 1 unit = 25 world units
	BYTE		LightPeriod;		// Period of light for certain LT_* effects, in 35ths sec.
	BYTE		LightPhase;			// Phase of light; 256 units defines a complete circular phase
	BYTE		LightCone;			// Cone angle, 0=no cone effect
	BYTE		InherentBrightness;	// Inherent (zero-lighting) brightnes of sprites. meshmaps
	//
	// Drawing flags:
	//
	DWORD	bAnimate            :1;	// Animation loops at end, else animation holds at end
	DWORD	bMeshWet			:1;	// Mesh reflects light, looks wet
	DWORD	bMeshShadowCast		:1;	// ctor mesh casts a shadow, only for LightG_ActorShadows lights
	DWORD	bMeshEnviroMap		:1;	// Actor mesh casts a shadow, only for LightG_ActorShadows lights
	DWORD	bSpriteRotates		:1;	// Sprite rotates about screen axis
	DWORD	bUnused0			:1;	// Actor is a potential event source (otherwise an event sink)
	DWORD	bActorShadows		:1;	// Casts actor shadows on surroundings
	DWORD	bShinyReflect		:1;	// Light's sprite reflects off shiny floors
	DWORD	bSpecialLit			:1;	// Only affects PF_SPECIALLIT polys
	DWORD	bStaticActor		:1;	// Can't be destroyed, assumes that it won't move during play
	DWORD	bHidden				:1;	// Hide during gameplay
	DWORD	bHiddenEd			:1;	// Hide in editor
	DWORD	bDirectional		:1;	// Directional, show arrow in 2D views
	DWORD	bCollideActors		:1;	// Collides with other actors
	DWORD	bCollideWorld		:1;	// Collides with the world
	DWORD	bBlocksActors		:1;	// Blocks other actors during collision
	DWORD	bBehindView			:1;	// Behind-the-player view
	DWORD	bSelected			:1;	// Selected in UnrealEd
	DWORD   bTempDynamicLight	:1; // Light is temporarily non-shadowed and dynamic (UnrealEd)
	DWORD	bDrawOnHorizon		:1; // Draw on far-off horizon, for lights/moon sprites
	DWORD	bTemplateClass		:1; // UnrealEd can't add actors to the world if this class flag is set
	DWORD	bTempLightChanged	:1; // UnrealEd internal
	DWORD	bUnlit				:1; // Actor is unlit
	DWORD	bNoSmooth			:1; // Actor textures should not be smoothed
	DWORD	bUnused4			:1;
	DWORD	bUnused5			:1;
	DWORD	bUnused6			:1;
	DWORD	bUnused7			:1;
	DWORD	bUnused8			:1;
	//
	// Functions:
	//
	FLOAT inline WorldLightRadius(void){return 1.0 + 25.0 * LightRadius;};
	FLOAT inline WorldSoundRadius(void){return 1.0 + 25.0 * SoundRadius;};
	void UpdateBrushPosition(ILevel *Level,INDEX iActor,int Editor);
	int IsBrush(void);
	int IsMovingBrush(void);
	};
#pragma pack (pop) /* Restore alignment */

/*-----------------------------------------------------------------------------
	AActorRoot
-----------------------------------------------------------------------------*/

//
// Class containing *all* properties available in the root actor class,
// from which all actors are derived.
//
#pragma pack (push,1) /* Turn off alignment so C and script code agree on member offsets */
class UNREAL_API AActorRoot : public AActorDraw
	{
	public:
	//
	// General:
	//
	UCamera		*Camera;		  	// UCamera controlling actor, or NULL if none
	FName		Name;				// Name of actor
	FName		EventName;			// Tag for bulk events
	//
	// Indices into actor list, referencing other actors:
	//
	INDEX       iTarget;      		// Actor target or INDEX_NONE
	INDEX       iInventory;      	// Next inventory item in 'linked list'
	INDEX		iFloor;				// Moving brush actor this actor is standing on
	//
	// Other:
	//
	FLOAT		Mass;				// Actor's mass, for momentum calculations
	//
	// Bit flags:
	//
	DWORD		bInactive		:1;	// Execution is frozen
	DWORD		bPegged			:1;	// Actor isn't affected by physics
	DWORD		bGravity		:1;	// Actor is affected by gravity
	DWORD		bMomentum		:1;	// Actor has momentum effects
	DWORD		bProjTarget		:1;	// Projectiles should consider targeting this actor
	DWORD		bCanPossess		:1;	// Players can possess it
	DWORD		bDifficulty1	:1;	// Appears on difficulty level 1
	DWORD		bDifficulty2	:1;	// Appears on difficulty level 2
	DWORD		bDifficulty3	:1;	// Appears on difficulty level 3
	DWORD		bDifficulty4	:1;	// Appears on difficulty level 4
	DWORD		bNetCooperative	:1;	// Appears in small cooperative play game
	DWORD		bNetDeathMatch	:1;	// Appears in small deathmatch game
	DWORD		bNetPersistent	:1;	// Appears in persistent-server (Internet) game
	DWORD		bNetNoMonsters	:1;	// Appears in net game with monsters turned off
	DWORD		bTempEditor		:1;	// Used by UnrealEd for temporary work
	DWORD		bJustDeleted	:1; // Actor with Class=NULL was deleted during this frame
    DWORD       bCanBeTeleported:1;
	DWORD		bUnused12		:1;
	DWORD		bUnused13		:1;
	DWORD		bUnused14		:1;
	DWORD		bUnused15		:1;
	INDEX		iMe;				// Index of this actor
	BYTE		Zone;				// Zone the actor resides in, 0=zone not known
    BYTE        ChanceOfExistence;
	//
    INT         LifeSpan;
    INT         Age;
    INT         Era;
    BYTE        TriggerSequences[10];
    BYTE        TriggerFrames[10];
    BYTE        TriggerValues[10];
    BYTE        WhichTriggers;
    INT         TextureList;
    BYTE        TextureCount;
    BYTE        ScriptCountdown;
    INT         TimerCountdown;
    INT         TimerMessage;
    FLOAT       WaterSinkRate;
    BYTE        AITask;
    INDEX       iPendingTeleporter;
    BYTE        TeleportDelay;
	FName		DefaultEdCategory;
	};
#pragma pack (pop) /* Restore alignment */

/*-----------------------------------------------------------------------------
	AActor
-----------------------------------------------------------------------------*/

//
// Standard actor class from which all actors are derived.  This is the
// complete structure maintained by the server and editor.
//
// WARNING!  All member variables declared here must be binary-compatible with the
// corresponding member variables in the definition ARoot (in Root.h), which
// is automatically generated by UnrealEd based on the root script.  This class
// is somewhat redundent, but exists in order to shield the Unreal engine from
// the details of the actor system scripting language in general.
//
#pragma pack (push,1) /* Turn off alignment so C and script code agree on member offsets */
class UNREAL_API AActor : public AActorRoot
	{
	public:
	//
	enum {AI_INFO_SIZE        = 200}; // Size of information structure needed for AI
	enum {CLASS_PROP_EXTRA    = 512}; // Size of extra class property info usable by ARoot-derived classes
	enum {CLASS_PROP_MAX_SIZE = sizeof(AActorRoot) + CLASS_PROP_EXTRA}; // Total size of available actor properties data
	//
	// Class-specific storage, specific to each actor class:
	//
	BYTE				ClassInfo[CLASS_PROP_EXTRA];	// Class-specific information, including locals and stack
	FActorCameraStatus	CameraStatus;					// Info for cameras
	FActorServerInfo	ServerInfo;						// Networking info for the server
	BYTE				AIInfo[AI_INFO_SIZE];			// AI info for the server
	//
	// Functions:
	//
	void QueryReferences(UResource *ParentRes,FResourceCallback &Callback, DWORD ContextFlags);
	void InitServerInfo(void);
	void GetDrawCoords(FCoords *Coords) const;
	void GetViewCoords(FCoords *Coords) const;
	void TransformPoint(FVector *LocalPoint, const FVector *WorldPoint);
	inline void *GetPropertyPtr(int iProperty,int iElement=0);
	};
#pragma pack (pop) /* Restore alignment */

/*-----------------------------------------------------------------------------
	UActorList
-----------------------------------------------------------------------------*/

//
// A list of actors associated with a level.  The actor list is sparse, in that
// actors with Class==NULL are treated as empty/nonexistant.
//
class UNREAL_API UActorList : public UDatabase
	{
	RESOURCE_DB_CLASS(UActorList,AActor,RES_ActorList)
	//
	DWORD		LockType;		// ELockType (None, read, write, notrans)
	DWORD		Trans;			// 1=transaction tracking is active
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	void QueryDataReferences	(FResourceCallback &Callback);
	void PostLoad				(void);
	//
	// Custom functions:
	//
	void Lock(int NewLockType);
	void Unlock(void);
    //    Redundant compact actor arrays provided for performance:
    struct CompactList : public DArray<AActor*,200,100> 
    {
        BOOL NeedsCompressing; // TRUE when the list might have elements with value 0.
        CompactList() { NeedsCompressing = FALSE; }

        // Remove an actor from the list: The actor is not removed immediately but is
        // instead replaced with the value 0. This allows uninterrupted iteration over
        // the list. It is assumed the actor is on the list at most once.
        void RemoveActor(AActor * Actor)
        {
            ChangeElement(Actor,0);
            NeedsCompressing = TRUE;
        }
        void Compress() // Compress the list if NeedsCompressing==TRUE.
        {
            if( NeedsCompressing )
            {
                RemoveElements(0); // Clear out the null entries.
                NeedsCompressing = FALSE;
            }
        }
    };
    // These are the main lists:
    CompactList * StaticActors        ; // List of all actors with Class != 0 && bStaticActor==TRUE.
    CompactList * DynamicActors       ; // List of all actors with Class != 0 && bStaticActor==FALSE.
    CompactList * CollidingActors     ; // List of all actors with Class != 0 && bCollideActors==TRUE.
    // These are the administrative lists:
    CompactList * ActiveActors        ; // List of all actors with Class != 0.
    CompactList * UnusedActors        ; // List of all actors with Class == 0 && bJustDeleted==FALSE.
    CompactList * JustDeletedActors   ; // List of all actors with Class == 0 && bJustDeleted==TRUE.
    void RelistActors(); // Rebuild the above redundant lists from scratch by scanning the actor list.
    void UnlistActor(AActor * Actor); // Remove an actor from the appropriate main lists (based on its properties).
        // Note: UnlistActor does *not* remove the actor from the administrative lists.
    void ListActor(AActor * Actor); // Add an actor to appropriate main lists (based on its properties).
        // Note: ListActor does *not* remove the actor from the administrative lists.
    void CheckLists(); // Debug: Check the redundant lists for correctness and completeness.
	};

/*-----------------------------------------------------------------------------
	UClass
-----------------------------------------------------------------------------*/

//
// Flags associated with each property in a class, overriding the
// property's default behavior.
//
enum EClassPropertyFlags
	{
	CPF_Edit			= 0x0001,	// Property is user-settable in the editor
	CPF_EditInPlay		= 0x0002,	// Property can safely be edited during gameplay
	CPF_Const			= 0x0004,	// Actor's property always matches class's default actor property
	CPF_Private			= 0x0008,	// Private, inaccessible to scripts
	CPF_NoSaveResource  = 0x0010,	// Don't save this CPT_Resource on disk
	CPF_ExportResource	= 0x0020,	// Resource can be exported with actor
	CPF_Param			= 0x0040,	// Function/When call parameter
	CPF_OptionalParam	= 0x0080,	// Optional parameter (if CPF_Param is set)
	CPF_ReturnValue		= 0x0100,	// Return value
	CPF_Initialized		= 0x0200,	// UnrealScript has specified an initializer value
	};

//
// Class property data types.
//
enum EClassPropertyType
	{
	CPT_None			= 0x00,		// No property							(0 bytes)
	CPT_Byte			= 0x01,		// Byte value, 0 to 256					(1 byte)
	CPT_Integer			= 0x02,		// 32-bit signed integer				(4 bytes)
	CPT_Boolean			= 0x03,		// Bit flag within 32-bit dword			(0 or 4 bytes)
	CPT_Real			= 0x04,		// Floating point number				(4 bytes)
	CPT_Actor			= 0x05,		// Actor index							(2 bytes)
	CPT_Resource		= 0x06,		// Resource pointer						(4 bytes)
	CPT_Name			= 0x07,		// FName, a global name					(2 bytes)
	CPT_String			= 0x08,		// Null terminated string				(0+ bytes)
	CPT_Vector			= 0x09,		// Floating point vector				(16 bytes)
	CPT_Rotation		= 0x0A,		// Pitch-yaw-roll rotation				(6 bytes)
	CPT_MAX				= 0x0B,
	};

//
// One class property that resides in a given class.  A property describes
// a variable name, an offset within the actor's data, a data type, property flags,
// and optional, type-dependent data.
//
class UNREAL_API FClassProperty // 32 bytes
	{
	public:
	FName		PropertyName;				// Name of property, i.e. "Strength", "Intelligence"
	FName		PropertyCategory;			// Property category name
	EClassPropertyType PropertyType;		// Type of property, from EClassPropertyType
	DWORD		PropertyOffset;				// Offset into property data for fast access
	DWORD		PropertyFlags;				// From EClassPropertyFlags
	DWORD		PropertyArrayDim;			// Dimension of array, 0 = a single variable, not an array
	DWORD		PropertyElementSize;		// Length of one element of the array (=PropertyLength if ArraySize=0 or 1)
	DWORD		PropertySize;				// Total length in bytes (accounting for array)
	union
		{
		UEnum			*Enum;				// Enum (or NULL=none) if BYTE or INT
		UClass			*Class;				// Optional IsKindOf class for CPT_Actor's (NULL=any kind is ok)
		EResourceType	PropertyResType;	// Property resource ID for AP_RESOURCE
		DWORD			BitMask;			// Bit mask for bit if type is AP_BITFLAG
		};
	void Init(void);
	int  SetFlags(const char *Override);
	int  SetType(const char *Type,int ArraySize,char *ErrorMessage);
	FClassProperty *AddToFrame(int *NumFrameProperties,FClassProperty *FrameProperties);
	int  InitPropertyData(BYTE *FrameDataStart,class FToken *OptionalInitToken=NULL);
	int  IsTrue(const BYTE *FrameDataStart);
	char *ExportTCX(char *Buffer,BYTE *Data);
	char *ExportH(char *Buffer);
	void DebugDump(char *Str);
	//
	int Compare(const AActor *RawActor1, const AActor *RawActor2,int Element);
	//
	inline int ComputePropertySize(void) {return PropertyElementSize * (PropertyArrayDim ? PropertyArrayDim : 1);};
	};

//
// An actor class.  Describes all of the attributes of a kind of actor, but
// does not refer to an actual instance of an actor.
//
class UNREAL_API UClass : public UDatabase
	{
	RESOURCE_DB_CLASS(UClass,FClassProperty,RES_Class)
	//
	enum {MAX_CLASS_PROPERTIES=256}; // Predefined limit of properties
	//
	// General variables:
	//
	UClass		*ParentClass;		// Parent class of the actor, or NULL if this class is the root
	UTextBuffer	*ScriptText;		// Optional script source code, NULL if none
	UScript		*Script;			// Optional compiled script object code, NULL if not valid
	DWORD		Intrinsic;			// 0=scripted, nonzero=intrinsic
	//
	// Pointer to actor's ::Process member function:
	//
	ACTOR_CLASS_FUNC ActorFunc;		// Valid only in memory, associated in PostLoad
	//
	// Make info for recompiling class scripts when needed:
	//
	DWORD		ScriptTextCRC;		// CRC of ScriptText data after most recent successful compile
	DWORD		ParentTextCRC;		// CRC of parent's ScriptText data after most recent successful compile
	DWORD		Pad1,Pad2,Pad3;
	//
	// Default actor properties.  These are copied to new actors when new actors of this 
	// class are spawned.
	//
	AActor		DefaultActor;		// Default actor
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	int  QuerySize				(void);
	int  QueryMinSize			(void);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	void PostLoad				(void);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom functions (UnClass.cpp):
	//
	UClass						(UClass *ParentClass);
	void AddParentProperties	(void);
	void Delete					(void);
	int  IsKindOf				(UClass *SomeParent);
	void FindActorFunc			(void);
	};

/*-----------------------------------------------------------------------------
	UScript
-----------------------------------------------------------------------------*/

//
// A tokenized, interpretable script resource referenced by a class.
// First element of the script's data is the root of the script's stack tree.
// Everything else in the script's data can be determined from the stack tree.
//
class UNREAL_API UScript : public UResource
	{
	RESOURCE_CLASS(UScript,BYTE,RES_Script)
	//
	// Variables:
	//
	UClass *Class;		// Class the script applies to
	INT Length;			// Total number of bytes in script
	INT NumStackTree;	// Number of entries in script's stack tree
	INT CodeOffset;		// Offset of start-of-code into script's data
	INT	Pad[5];
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	int  QuerySize				(void);
	int  QueryMinSize			(void);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom functions:
	//
	char *Decompile				(char *Buffer,int ParentLinks);
	};

/*-----------------------------------------------------------------------------
	Actor message parameter classes
-----------------------------------------------------------------------------*/

//
// Child classes of FActorMessageParams are all sent to actors using the 
// "void *Params" pointer in various actor messages.
// Refer to UnMsgs.h for the parameter types expected for particular messages.
//
class PMessageParams
	{
	public:
	// Contains nothing
	};

class PSense : public PMessageParams
{
public:
    INDEX       SensedActor     ; // Sensed actor, or INDEX_NONE.
    BOOL        LocationKnown   ; // TRUE if Location is known.
    union
    {
        FVector Location        ; // LocationKnown==TRUE: Location of sensed object
        FVector Direction       ; // LocationKnown==FALSE: Direction of sensed object
    };
    // Notes:
    //   1. The accuracy of this->Direction depends on the accuracy of the sensing actor.
    static PSense & Convert(void * Params) { return *(PSense*)Params; }
};

class PFrame : public PMessageParams
{
public:
    int     Sequence    ; // Animation sequence (1..)
    int     Frame       ; // Animation frame (0..)
    int     Trigger     ; // Trigger value associated with the frame.
    static PFrame & Convert(void * Params) { return *(PFrame*)Params; }
};

//
// Player movement packet information structure:
//
enum EMovementAxis // An axis of player movement:
	{
	PlayerAxis_None         , // No axis (always 0).
    // Movements:
	PlayerAxis_Forward      , // Forward (>0) or backward (<0)
	PlayerAxis_Rightward    , // Rightward (>0) or leftward (<0)
	PlayerAxis_Upward       , // Upward (>0) or downward (<0)
    // Rotations:
	PlayerAxis_Yaw          , // Yaw to the right (>0) or left (<0)
	PlayerAxis_Pitch        , // Pitch upward (>0) or downward (<0)
	PlayerAxis_Roll         , // Roll clockwise (>0) or counterclockwise (<0)
    // Administrative:
    PlayerAxis_Count        , // Number of axes, including PlayerAxis_None.

    // Keep the following synchronized with the above values:
    PlayerAxis_FirstMovement    = PlayerAxis_Forward    ,
    PlayerAxis_LastMovement     = PlayerAxis_Upward     ,
    PlayerAxis_MovementCount    = PlayerAxis_LastMovement - PlayerAxis_FirstMovement + 1,
    PlayerAxis_FirstRotation    = PlayerAxis_Yaw        ,
    PlayerAxis_LastRotation     = PlayerAxis_Roll       ,
    PlayerAxis_RotationCount    = PlayerAxis_LastRotation - PlayerAxis_FirstRotation + 1
	};

struct PPlayerMotion
    {
        FLOAT   Analog          ; // Analog position of motion.
        FLOAT   Differential    ; // Differential change in motion.
        void Empty() { Analog = 0; Differential = 0; }
        static void Empty( PPlayerMotion * Motions, int Count ) // Empty an array of player motions.
        {
            memset( Motions, 0, Count * sizeof(Motions[0]) ); // Note: This assumes 0-ness of floating point 0.
        }
    };

class PPlayerTick : public PMessageParams
	{
	public:
    PPlayerMotion Movements[PlayerAxis_Count]; 
    //todo: Make Movements an array of PlayerAxis_MovementCount elements once the
    // rotation handling has been moved to PCalcView.
    //todo: [add] FAction::TActionStatus Actions[FAction::ActionCount];
    // We use the declaration below to avoid many many many recompilations
    // after changes in FAction. Eventually, we should use the above declaration
    // instead of this:
    BYTE Actions[100];
	int	 BuildAllMovement(UCamera *Camera);
    static PPlayerTick & Convert(void * Params) { return *(PPlayerTick*)Params; }
	};

//
// View calculation information structure, sent to actors
// with active cameras immediately before rendering, to allow the actors
// to move the viewpoint for effects such as bouncing the player's view
// while she's walking, or implementing a behind-the-player view.
//
class PCalcView : public PMessageParams
	{
	public:
	FVector	  ViewLocation;
	FRotation ViewRotation;
	FCoords	  *Coords;
	FCoords	  *Uncoords;
    PPlayerMotion Rotations[PlayerAxis_RotationCount];  // Indexed by (EMovementAxis-PlayerAxis_FirstRotation)
    const PPlayerMotion & Rotation(EMovementAxis RotationAxis) const // RotationAxis must be a rotation.
        {
        return Rotations[RotationAxis - PlayerAxis_FirstRotation];
        }
    PPlayerMotion & Rotation(EMovementAxis RotationAxis) // RotationAxis must be a rotation.
        {
        return Rotations[RotationAxis - PlayerAxis_FirstRotation];
        }
    static PCalcView & Convert(void * Params) { return *(PCalcView*)Params; }
	};

// Actor identification:
class PActor : public PMessageParams
	{
	public:
	INDEX iActor;
    static PActor & Convert(void * Params) { return *(PActor*)Params; }
	};

// Touch identification:
class PTouch : public PActor
	{
	public:
    static PTouch & Convert(void * Params) { return *(PTouch*)Params; }
	};

// Pickup identification:
class PPickupQuery : public PActor
	{
	public:
    static PPickupQuery & Convert(void * Params) { return *(PPickupQuery*)Params; }
	};

// Hit identification:
class PHit : public PMessageParams
	{
	public:
	INDEX iSourceActor;		// Actor (possibly player) who originated the hit or INDEX_NONE
	INDEX iSourceWeapon;	// Weapon who originated the hit or INDEX_NONE
    //tbi: It would be nice to use DMT_Count below instead of "4".
	FLOAT Damage[4]; // Damage, indexed with EDamageType.
    FLOAT ActualDamage  ; // Output value: actual total damage inflicted on pawn.
	FVector HitLocation;	// Exact location of hit
    FLOAT Momentum;  // Momentum to impart (based on direction of the source actor), 0 for none.
    void Empty()
        {
            iSourceActor    = INDEX_NONE    ;
            iSourceWeapon   = INDEX_NONE    ;
            for( int Which = 0; Which < 4; ++Which )
                {
                    Damage[Which] = 0;
                }
            ActualDamage = 0;
            Momentum = 0;
            HitLocation = GMath.ZeroVector;
        }
    static PHit & Convert(void * Params) { return *(PHit*)Params; }
	};

//
// Parameters for HitNotify message:
//
class PHitNotify : public PMessageParams
	{
	public:
	FVector		HitLocation;
	INDEX		iHitActor;
    static PHitNotify & Convert(void * Params) { return *(PHitNotify*)Params; }
	};

//
// Parameters for WallNotify message:
//
class PWallNotify : public PMessageParams
	{
	public:
	FVector		WallLocation;
	FVector		WallNormal;
	INDEX		iWallNode;
    static PWallNotify & Convert(void * Params) { return *(PWallNotify*)Params; }
	};

// Using inventory items:
class PUse : public PMessageParams
{
public:
    BOOL    Count       ; // Output value - number of times item did something.
    // Notes:
    //   1. Count for a weapon is the number of discharges effected.
    static PUse & Convert(void * Params) { return *(PUse*)Params; }
};

//
// KeyMoveTo
//
class PKeyMove : public PMessageParams
	{
	public:
	BYTE KeyNum; // Keyframe number to move to
    static PKeyMove & Convert(void * Params) { return *(PKeyMove*)Params; }
	};

//
// A reference to a name
//
class PName : public PMessageParams
	{
	public:
	FName Name;
    static PName & Convert(void * Params) { return *(PName*)Params; }
	};


//
// Level information:
//
class PLevel : public PMessageParams
    {
    public:
        enum { MAX_NAME_LENGTH = 63 };
        char Name[MAX_NAME_LENGTH+1]; // +1 for trailing null.
        static PLevel & Convert(void * Params) { return *(PLevel*)Params; }
    };

//
// A text message structure used by TextMsg message
//
enum {TEXTMSG_LENGTH=120};
class PText
	{
	public:
	INDEX iSourceActor;
	BYTE  MsgType /* ETextMsgType */;
	char Message[TEXTMSG_LENGTH];
    static PText & Convert(void * Params) { return *(PText*)Params; }
	};

//
// Exec structure for actor Exec, GetProp, SetProp messages
//
class PExec
	{
	public:
	INDEX iSourceActor;
	char Arg[TEXTMSG_LENGTH];
    static PExec & Convert(void * Params) { return *(PExec*)Params; }
	};

//
// Gameplay mode information for BeginPlay
//
class PBeginPlay
	{
	public:
	DWORD	bNetCooperative		:1;
	DWORD	bNetDeathMatch		:1;
	DWORD	bNetPersistent		:1;
	DWORD	bNetNoMonsters		:1;
	//
	BYTE	Difficulty;
    static PBeginPlay & Convert(void * Params) { return *(PBeginPlay*)Params; }
	};


// Information about a requested animation:
class PAnimate : public PMessageParams
{
public:
    typedef enum // Kinds of animations
    {
        NoAnimation    = 0           // Always 0.
    ,   StillAnimation               // Animation to use when the actor is still.
    ,   IdleAnimation                // Animation to use when actor is doing nothing.
    ,   MoveAnimation                // Animation to use for normal movement.
    ,   RunAnimation                 // Animation to use for faster movement.
    ,   FlyAnimation                 // Animation to use when flying, if different from MoveAnimation.
    ,   HitAnimation                 // Animation to use when actor is hit.
    ,   PainAnimation                // Animation to use when actor is in pain.
    ,   SearchAnimation              // Animation to use when actor is searching.
    ,   ThreatenAnimation            // Animation to threaten (bellow, roar, menace).
    ,   DistantStillAttackAnimation  // Animation for long-distance non-moving attack.
    ,   DistantMovingAttackAnimation // Animation for long-distance moving attack.
    ,   CloseUpAttackAnimation       // Animation for nearby attack.
    ,   DeathAnimation               // Animation to use when actor dies.
        // Notes:
        //   1. It might seem silly to have a StillAnimation, since why would you
        //      animate an actor which is still. However, actors are almost always
        //      playing an animation, even if it is a single frame.
        //   2. StillAnimation and IdleAnimation are similar but their separation allows
        //      us to have monsters which, although doing nothing really important, 
        //      nonetheless have some movement (for realism or entertainment). A monster
        //      might treat StillAnimation and IdleAnimation as the same.
    }
    TKind;  
    TKind Kind;  
    static PAnimate & Convert(void * Params) { return *(PAnimate*)Params; }
};

/*-----------------------------------------------------------------------------
	Class-related global variables
-----------------------------------------------------------------------------*/

//
// Actor types that are built into the engine.
// This structure is temporary and will become obsolete.
//
class UNREAL_API FGlobalClasses
	{
	public:
	TArray<UClass>	*IntrinsicClasses;	// List of built-in, always-active classes
	TArray<UClass>	*AllClasses;		// All classes, for UnrealEd
	UClass			*AddClass;			// Currently selected class for adding actors
	//
	// Built-in actor classes: All of these classes are built into the engine and
	// are always loaded.  They are referenced in the IntrinsicClasses array, which
	// is referenced by to Root resource, which prevents them from being freed during
	// a resource purged.  Do not add additional classes here unless absolutely
	// necessary.  If you do add a class, make sure it's referenced by IntrinsicClasses
	// or else it will be purged and its pointer will be invalid!
	//
	UClass			*TextureRes,*SoundRes,*AmbientRes,*LevelRes;
	UClass			*Root,*Camera,*Light,*Pawn,*Player,*PlayerStart;
	UClass			*LevelDescriptor,*ZoneDescriptor;
	UClass			*Inventory,*Weapon,*Pickup,*Mover;
	UClass			*Ammo,*PowerUp;
	UClass			*Projectile;
	//
	// Functions:
	//
	void Init(void);
	void Exit(void);
	void Associate(void);
	};

/*-----------------------------------------------------------------------------
	Global actor and class functions
-----------------------------------------------------------------------------*/

//
// Actor import/export functions (UnActLst.cpp):
//
void ExportActorProperty(char *Type,char *Name, char *Value, FClassProperty *ClassProperty,
	AActor *Actor, int Flags, int Descriptive, int ArrayElement, AActor *Delta, FName Category);
int ExportActor(AActor *Actor,char *Ptr,FName PropertyName,
	int Indent,int Descriptive,int Flags, AActor *Delta, int Resources, int ArrayElement,FName Name);
int ExportMultipleActors (UActorList *Actors,char *Ptr,FName PropertyName,
	int Indent,int Descriptive,FName Category);
const char *ImportActorProperties(AActor *Actor, const char *Data);

//
// Script compiler functions (UnScript.cpp):
//
int UNREAL_API MakeScripts(UClass *Class,int MakeAll);
int UNREAL_API CompileScript(UClass *Class,int ActorPropertiesAreValid);

//
// Misc:
//
char *GetPropertyTypeName(EClassPropertyType Type);

inline void *AActor::GetPropertyPtr(int iProperty,int iElement)
	{
	FClassProperty *Property = &Class->Element(iProperty);
	return &((BYTE *)this)[Property->PropertyOffset + iElement*Property->PropertyElementSize];
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNACTOR

