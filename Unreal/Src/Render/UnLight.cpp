/*=============================================================================
	UnLight.cpp: Unreal global lighting subsystem implementation.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

Description:
	Computes all point lighting information and builds surface light meshes 
	based on light actors and shadow maps.

Definitions:
	attenuation:
		The amount by which light diminishes as it travells from a point source
		outward through space.  Physically correct attenuation is propertional to
		1/(distance*distance), but for speed, Unreal uses a lookup table 
		approximation where all light ceases after a light's predefined radius.
	diffuse lighting:
		Viewpoint-invariant lighting on a surface that is the result of a light's
		brightness and a surface texture's diffuse lighting coefficient.
	dynamic light:
		A light that does not move, but has special effects.
	illumination map:
		A 2D array of floating point or MMX Red-Green-Blue-Unused values which 
		represent the illumination that a light applies to a surface. An illumination
		map is the result of combining a light's spatial effects, attenuation,
		incidence factors, and shadow map.
	incidence:
		The angle at which a ray of light hits a point on a surface. Resulting brightness
		is directly proportional to incidence.
    light:
		Any actor whose LightType member has a value other than LT_None.
	meshel:
		A mesh element; a single point in the rectangular NxM mesh containing lighting or
		shadowing values.
	moving light:
		A light that moves. Moving lights do not cast shadows.
	radiosity:
		The process of determining the surface lighting resulting from 
		propagation of light through an environment, accounting for interreflection
		as well as direct light propagation. Radiosity is a computationally
		expensive preprocessing step but generates physically correct lighting.
	raytracing:
		The process of tracing rays through a level between lights and map lattice points
		to precalculate shadow maps, which are later filtered to provide smoothing. 
		Raytracing generates cool looking though physically unrealistic lighting.
	resultant map:
		The final 2D array of floating point or MMX values which represent the total
		illumination resulting from all of the lights (and hence illumination maps) 
		which apply to a surface.
	shadow map:
		A 2D array of floating point values which represent the amount of shadow
		occlusion between a light and a map lattice point, from 0.0 (fully occluded)
		to 1.0 (fully visible).
	shadow hypervolume:
		The six-dimensional hypervolume of space which is not affected by a volume
		lightsource.
	shadow volume:
		The volume of space which is not affected by a point lightsource. The inverse of light
		volume.
	shadow z-buffer:
		A 2D z-buffer representing a perspective projection depth view of the world from a
		lightsource. Often used in dynamic shadowing computations.
	spatial lighting effect:
		A lighting effect that is a function of a location in space, usually relative to
		a light's location.
	specular lighting:
		Viewpoint-varient lighting on a shiny surface that is the result of a
		light's brightness and a surface texture's specular lighting
		coefficient.
	static illumination map:
		An illumination map that represents the total of all static light illumination
		maps that apply to a surface. Static illumination maps do not change in time
		and thus they can be cached.
	static light:
		A light that is constantly on, does not move, and has no special effects.
	surface map:
		Any 2D map that applies to a surface, such as a shadow map or illumination
		map.  Surface maps are always aligned to the surface's U and V texture
		coordinates and are bilinear filtered across the extent of the surface.
	volumetric lighting:
		Lighting that is visible as a result of light interacting with a volume in
		space due to an interacting media such as fog. Volumetric lighting is view
		variant and cannot be associated with a particular surface.

Design notes:
 *	Uses a multi-tiered system for generating the resultant map for a surface,
	where all known constant intermediate and resulting meshes that may be needed 
	in the future are cached, and all known variable intermediate and resulting
	meshes are allocated temporarily.
 *  All floating point light meshes are shadow maps are represented as floating point
    values from 0.0 to 1.0, all-inclusive.  This allows for fast mixing of components A 
	and B via (A+B-A*B), which produces a valid result given valid inputs. However, typical
	lighting values are small (<0.2) so that (A+B-A*B) is approximately linear; nonlinearity
	only occurs with unnaturally bright lights.

Notes:
	No specular lighting support.
	No volumetric lighting support.
	No radiosity.
	No dynamic shadows.
	No shadow hypervolumes.
	No shadow volumes.
	No shadow z-buffers.

todo: / optimize:
 *  Time everything and don't bother optimizing the seldom-used functions.  I have only a vague
    idea of where lighting CPU time is being spent...
 *	Idea: When building illumination map, compute bounding rectangle of valid lighting area,
	and clip all lighting operations to that rectangle.  I think that >50% of the lighting 
	computation is being done for meshels which are outside of the lightsource's radius.
 *	Prevent the tons of unnecessary matrix inversions per frame by caching or precomputing them.
 *	See 'optimize:' notes for list of what can be optimized.
 *  A signicant percent of lighting activity occurs in fully-shadowed areas (where the shadow
    map value equals 0.0).  When I find a 0.0 shadow map, I skip a lot of the illumination map math
	but it would be nice to find a way to reject large areas of 0.0's with a single check.  Note that
	since the shadow map is generated from a bit mask of binary 1's and 0's filtered with a 5x5 filter,
	it is possible to determine an exact list of all 0.0 shadow maps from just the bit map. Perhaps this
	could be used to build a cached span buffer representing the valid x-spans in the (x,y) shadow maps.
	This would work with the existing loops like spatial_None, but it's complex and may or may not be
	worthwhile.
 *	Don't worry about the bizarre special effects like searchlight which do stuff like multiple atan2's
    per meshel. These will only be used in small rooms where performance isn't an issue.
 *  Feel free to play with additional special effects and techniques; this is set up very modularly and
    it fun to experiment with.
 *	Keep the code clean and simple; it will be extended.

Revision history:
    9-23-96, Tim: Rewritten from the ground up.
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

#define SHADOW_SMOOTHING 1	/* Smooth shadows (should be 1) */

//
// Testing macros:
//
#if 0
	#define AssertInRange(f,r) {if(f<0.0||f>1.0)appErrorf(r ": Range(%f)",f);}
#else
	#define AssertInRange(f,r)
#endif

/*------------------------------------------------------------------------------------
	Subsystem definition
------------------------------------------------------------------------------------*/

//
// Lighting manager definition.
//
class FGlobalLightManager : public FVirtualGlobalLightManager
{
public:
	// FVirtualGlobalLightManager functions.
	void Init();
	void Exit();
	void Tick();
	void SetupForActor(ICamera *Camera, INDEX iActor);
	void SetupForPoly(ICamera *Camera, FVector &Normal, FVector &Base, INDEX iThisSurf);
	void SetupForNothing(ICamera *Camera);
	void ApplyLatticeEffects(FTexLattice* TopLattice, int Num);
	void ReleaseLightBlock();
	void DoDynamicLighting(ILevel *Level);
	void UndoDynamicLighting(ILevel *Level);

	// Forward declarations
	class FLightInfo;

	// Constants.
	enum {GCycle=16};
	enum {N_RANDS=256};
	enum {MAX_LIGHTS=16};
	enum {MAX_DYN_LIGHT_POLYS=2048};

	// Function pointer types.
	typedef void (*LIGHT_MERGE_FUNC)   (int Key, int MeshUSize, FLOAT* Src,FLOAT* Dest,FLOAT Scale);
	typedef void (*LIGHT_SPATIAL_FUNC) (int MeshUSize, class FLightInfo* Info, FLOAT* Src, FLOAT* Dest, FVector Vertex, FVector VertexDU);
	typedef void (*LIGHT_LATTICE_FUNC) (FLightInfo& Light, FTexLattice* TopLattice, int Num);
	typedef void (*LIGHT_TYPE_FUNC)    (FLightInfo& Light, AActorDraw &Actor);

	// Information about one special lighting effect.
	struct FLocalEffectEntry
	{
		LIGHT_MERGE_FUNC	MergeFxFunc;		// Function to perform merge lighting
		LIGHT_SPATIAL_FUNC	SpatialFxFunc;		// Function to perform spatial lighting
		LIGHT_LATTICE_FUNC	LatticeFxFunc;		// Function to perform lattice warping
		int					MinXBits,MaxXBits;	// Horizontal lattice size bounds, 0=n/a
		int					MinYBits,MaxYBits;	// Vertical lattice size bounds, 0=n/a
		int					IsSpatialDynamic;	// Indicates whether light spatiality changes over time
	};

	// Information about a lightsource.
	class FLightInfo
	{
	public:

		// Variables.
		AActorDraw			*Actor;			// All actor drawing info
		FVector				Location;		// Transformed screenspace location of light
		FVector				Reflection;		// Reflection about current polygon
		FLOAT				Radius;			// Maximum effective radius
		FLOAT				RRadius;		// 1.0 / Radius
		FLOAT				RRadiusMult;	// 16383.0 / (Radius * Radius)
		FLOAT				Brightness;		// Center brightness at this instance, 1.0=max, 0.0=none
		FLOAT				LocationNormal;	// light location dot poly normal
		FLOAT				BaseNormal;		// base dot normal
		FLOAT				BaseNormalDelta;// (light location - poly base) dot poly normal
		FLOAT				Diffuse;		// BaseNormalDelta * RRadius
		FLOAT*				IlluminationMap;// Temporary illumination map pointer

		// Information about the lighting effect.
		FLocalEffectEntry	Effect;

		// Functions.
		void inline ComputeFromActor  (ICamera *Camera, INDEX iActor);
		void inline SetupRectSampling (FVector &Base, FVector &Normal);
	};

	// Global light effects.
	static void global_None				( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Steady			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Pulse			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Blink			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Flicker			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Strobe			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Explode2			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Explode3			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Daylight			( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Nightlight		( class FLightInfo &Light, AActorDraw &Actor );
	static void global_Test				( class FLightInfo &Light, AActorDraw &Actor );

	// Simple lighting functions.
	static void merge_F_None			(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale);
	static void merge_F_TorchWaver		(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale);
	static void merge_F_FireWaver		(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale);
	static void merge_F_WaterShimmer	(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale);
	static void merge_F_Test			(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale);

	// Spatial lighting functions.
	static void spatial_None			( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_SearchLight		( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_SlowWave		( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_FastWave		( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_CloudCast		( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_StormCast		( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_Shock			( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_Disco			( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_NegativeLight	( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_Interference	( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_Cylinder		( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_Rotor			( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );
	static void spatial_Test			( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU );

	// Lattice lighting functions.
	static void lattice_None			( FLightInfo& Info, FTexLattice* TopLattice, int Num );
	static void lattice_WarpU			( FLightInfo& Info, FTexLattice* TopLattice, int Num );
	static void lattice_WarpV			( FLightInfo& Info, FTexLattice* TopLattice, int Num );
	static void lattice_CalmWater		( FLightInfo& Info, FTexLattice* TopLattice, int Num );
	static void lattice_ChurningWater	( FLightInfo& Info, FTexLattice* TopLattice, int Num );
	static void lattice_Test			( FLightInfo& Info, FTexLattice* TopLattice, int Num );

	// Functions.
	void BuildTemporaryTables();
	void ShadowMapGen_F(BYTE *SrcBits,FLOAT *Dest1);
	static void LightingSpanInit_F(FLOAT *Dest);
	void StaticLightingMapGen(FLOAT *Result);
	void IlluminationMapGen(FLightInfo *Info,FLOAT *Src,FLOAT *Dest);

	// Level of optimization that can be applied to a lightsource, ranging from 0 (the most)
	// to 3 (the least).
	enum EActorLightOptimization
	{
		LC_NotLight		= 0,	// Actor is not a lightsource
		LC_StaticLight	= 1,	// Actor is a non-moving, non-changing lightsource
		LC_DynamicLight	= 2,	// Actor is a non-moving, changing lightsource
		LC_MovingLight	= 3,	// Actor is a moving, changing lightsource
	};

	// Return the level of lighting optimization that can be applied to an actor.
	static EActorLightOptimization LightOptimization(AActorDraw *Actor)
	{
		if( Actor->LightType==LT_None )
			return LC_NotLight;
		else if( Actor->bTempDynamicLight )
			return LC_MovingLight;
		else if( Actor->Class->IsKindOf(GClasses.Light) )
		{
			if( Actor->LightType==LT_Steady && Actor->LightEffect==LE_None )
				return LC_StaticLight;
			else if( Actor->Class->IsKindOf(GClasses.Light) )
				return LC_DynamicLight;
		}
		return LC_MovingLight;
	}

	// Look up a Bsp Node's light volume index and return a pointer to it or NULL if none.
	inline static FLightMeshIndex *GetLightMeshIndex (const IModel *ModelInfo, INDEX iSurf)
	{
		GUARD;
		FBspSurf *Poly = &ModelInfo->BspSurfs[iSurf];
		if( Poly->iLightMesh==INDEX_NONE || !ModelInfo->LightMesh ) return NULL;
		return &(ModelInfo->LightMesh->GetData())[Poly->iLightMesh];
		UNGUARD("FGlobalLightManager::GetLightMeshIndex");
	}

	// Public lighting variables.
	static FLightInfo*		LastLight;
	static FLightInfo*const FinalLight;
	static FLightInfo		FirstLight[MAX_LIGHTS];

	static INT				LastTicks,NumDynLightPolys,TemporaryTablesBuilt;
	static FLOAT*			ActorBrightness;

	// Tables.
	static INDEX			DynLightPolys[MAX_DYN_LIGHT_POLYS];
	static DWORD			FilterTab[8][32];
	static INT				MultiplyTab[256+2+2];

	// Arrays
	static const FLocalEffectEntry FGlobalLightManager::Effects[LE_MAX];
	static const LIGHT_TYPE_FUNC FGlobalLightManager::GLightTypeFuncs[LT_MAX];
	static FLOAT RandomBases[N_RANDS], Randoms[N_RANDS], RandomDeltas[N_RANDS];
};

//
// FGlobalLightManager statics.
//
FGlobalLightManager::FLightInfo*		FGlobalLightManager::LastLight;
FGlobalLightManager::FLightInfo*const	FGlobalLightManager::FinalLight = &FirstLight[MAX_LIGHTS];
FGlobalLightManager::FLightInfo			FGlobalLightManager::FirstLight[MAX_LIGHTS];

INT					FGlobalLightManager::LastTicks;
INT					FGlobalLightManager::NumDynLightPolys;
INT					FGlobalLightManager::TemporaryTablesBuilt;
FLOAT*				FGlobalLightManager::ActorBrightness;

INDEX				FGlobalLightManager::DynLightPolys[MAX_DYN_LIGHT_POLYS];
DWORD				FGlobalLightManager::FilterTab[8][32];
INT					FGlobalLightManager::MultiplyTab[256+2+2];

const FGlobalLightManager::FLocalEffectEntry FGlobalLightManager::Effects[LE_MAX] =
{
	// Simple func			Spatial func			Lattice func		MinXBit	MaxXBit	MinYBit	MaxYBit	SpacDyn
	//---------------------	-----------------------	-------------------	------- ------- ------- -------	-------
	{merge_F_None,			spatial_None,			lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_TorchWaver,	spatial_None,			lattice_None,		0,		0,		0,		0,		0		},
	{merge_F_FireWaver,		spatial_None,			lattice_None,		0,		0,		0,		0,		0		},
	{merge_F_WaterShimmer,	spatial_None,			lattice_None,		0,		0,		0,		0,		0		},
	{merge_F_None,			spatial_SearchLight,	lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_SlowWave,		lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_FastWave,		lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_CloudCast,		lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_StormCast,		lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_Shock,			lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_Disco,			lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_None,			lattice_WarpU,		0,		0,		0,		0,		0		},
	{merge_F_None,			spatial_None,			lattice_WarpV,		0,		0,		0,		0,		0		},
	{merge_F_None,			spatial_None,			lattice_CalmWater,	0,		0,		0,		0,		0		},
	{merge_F_None,			spatial_None,			lattice_ChurningWater,0,	0,		0,		0,		0		},
	{merge_F_None,			spatial_NegativeLight,	lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_Interference,	lattice_None,		0,		0,		0,		0,		1		},
	{merge_F_None,			spatial_Cylinder,		lattice_None,		0,		0,		0,		0,		0		},
	{merge_F_None,			spatial_Rotor,			lattice_None,		0,		0,		0,		0,		1		},
};

const FGlobalLightManager::LIGHT_TYPE_FUNC FGlobalLightManager::GLightTypeFuncs[LT_MAX] =
{
	global_None,
	global_Steady,
	global_Pulse,
	global_Blink,
	global_Flicker,
	global_Strobe,
	global_Explode2,
	global_Explode3,
	global_Daylight,
	global_Nightlight
};

FLOAT FGlobalLightManager::RandomBases[N_RANDS];
FLOAT FGlobalLightManager::Randoms[N_RANDS];
FLOAT FGlobalLightManager::RandomDeltas[N_RANDS];

//
// FVirtualLightManager statics.
//
FVector				FVirtualGlobalLightManager::Base;
FVector				FVirtualGlobalLightManager::Normal;
FVector				FVirtualGlobalLightManager::TextureU;
FVector				FVirtualGlobalLightManager::TextureV;
FVector				FVirtualGlobalLightManager::InverseUAxis;
FVector				FVirtualGlobalLightManager::InverseVAxis;
FVector				FVirtualGlobalLightManager::InverseNAxis;
ICamera*			FVirtualGlobalLightManager::Camera;
ILevel*				FVirtualGlobalLightManager::Level;
IModel*				FVirtualGlobalLightManager::ModelInfo;
ULightMesh*			FVirtualGlobalLightManager::LightMesh;
FLightMeshIndex*	FVirtualGlobalLightManager::Index;
FBspSurf*			FVirtualGlobalLightManager::Surf;
BYTE*				FVirtualGlobalLightManager::ShadowBase;
void*				FVirtualGlobalLightManager::MemTop;
VOID*				FVirtualGlobalLightManager::MeshVoid;
QWORD				FVirtualGlobalLightManager::MeshAndMask;
DWORD				FVirtualGlobalLightManager::PolyFlags;
DWORD				FVirtualGlobalLightManager::MaxSize;
INT					FVirtualGlobalLightManager::MeshUSize;
INT					FVirtualGlobalLightManager::MeshVSize;
INT					FVirtualGlobalLightManager::MeshSpace;
INT					FVirtualGlobalLightManager::MeshUTile;
INT					FVirtualGlobalLightManager::MeshVTile;
INT					FVirtualGlobalLightManager::MeshTileSpace;
INT					FVirtualGlobalLightManager::MeshUByteSize;
INT					FVirtualGlobalLightManager::MeshByteSpace;
INT					FVirtualGlobalLightManager::MeshSkipSize;
INT					FVirtualGlobalLightManager::LatticeEffects;
INT					FVirtualGlobalLightManager::MinXBits;
INT					FVirtualGlobalLightManager::MaxXBits;
INT					FVirtualGlobalLightManager::MinYBits;
INT					FVirtualGlobalLightManager::MaxYBits;
INDEX				FVirtualGlobalLightManager::iLightMesh;
INDEX				FVirtualGlobalLightManager::iSurf;
BYTE				FVirtualGlobalLightManager::MeshUBits;
BYTE				FVirtualGlobalLightManager::MeshVBits;

/*------------------------------------------------------------------------------------
	Init & Exit
------------------------------------------------------------------------------------*/

//
// Initialize the global lighting subsystem.
//
void FGlobalLightManager::Init()
{
	GUARD;

	// Globals.
	NumDynLightPolys = 0;
	LastTicks = 0;

	// Filtering table.
	int FilterBase[3][3] = 
	{
		{64,24,12},
		{24,20, 8},
		{12, 8, 5}
	};

	// Generate filter weights.
	int FilterWeight[8][8];
		for( int i=0; i<8; i++ ) for( int j=0; j<8; j++ )
		{
#if SHADOW_SMOOTHING
			FilterWeight[i][j]=((i>4)||(j>4))?0:FilterBase[OurAbs(i-2)][OurAbs(j-2)];
#else
			FilterWeight[i][j]=(i==2)&&(j==2);
#endif
		}

	// Generate filter lookup table
	int FilterSum=0;
	for( i=0; i<5; i++ ) for( int j=0; j<8; j++ ) FilterSum += FilterWeight[i][j];

	for( i=0; i<32; i++ )
	{
		for( int j=0; j<8; j++ )
		{
			int FilterAccumulator = 0;
			int M=1;
			for( int Bit=0; Bit<5; Bit++ )
			{
				if( i&M ) FilterAccumulator += FilterWeight[j][Bit];
				M = M << 1;
			}
			FilterTab[j][i] = (1024 * FilterAccumulator) / FilterSum;
		}
	}
	debugf(LOG_Init,"Lighting subsystem initialized");

	UNGUARD("FGlobalLightManager::Init");
}

//
// Shut down the global lighting system.
//
void FGlobalLightManager::Exit()
{
	debugf(LOG_Exit,"Lighting subsystem shut down");
}

/*------------------------------------------------------------------------------------
	Tick
------------------------------------------------------------------------------------*/

//
// Update rendering tables, called once per frame rendered.
//
void FGlobalLightManager::Tick()
{
	// optimize: This code would benefit greatly from a fast random number generator
	// that generates floating point randoms from 0.0 to 1.0. This could be done with
	// an integer random generator and ieee-specific code like:
	//
	// float temp=1.0, *result;
	// int randombits = (some completely random bit pattern);
	// *(int*)&result = (*(int*)&temp & 0xff800000) | (randombits & 0x007fffff);
	//
	if( !LastTicks )
	{
		for( int i=0; i<N_RANDS; i++ )
			Randoms[i] = (FLOAT)rand()/RAND_MAX;
	}
	for( int i=0; i<N_RANDS; i++ )
		RandomBases[i] = (FLOAT)rand()/RAND_MAX;
	if( (GServer.Ticks^LastTicks) & ~(GCycle-1) )
	{
		for( int i=0; i<N_RANDS; i++ )
			RandomDeltas[i] = (RandomBases[i] - Randoms[i]) / GCycle;
		LastTicks = GServer.Ticks & ~(GCycle-1);
	}
	for( i=0; i<N_RANDS; i++ )
		Randoms[i] += RandomDeltas[i] * (GServer.Ticks - LastTicks);
	LastTicks = GServer.Ticks;
}

/*------------------------------------------------------------------------------------
	FGlobalLightManager::FLightInfo implementation.
------------------------------------------------------------------------------------*/

//
// Precompute properties for a lightsource.
//
void FGlobalLightManager::FLightInfo::SetupRectSampling(FVector &Base, FVector &Normal)
	{
	FLOAT MirrorDist;
	//
	LocationNormal	= Location | Normal;
	BaseNormal		= Base     | Normal;
	BaseNormalDelta	= LocationNormal - BaseNormal;
	MirrorDist		= 2.0 * BaseNormalDelta;
	Reflection		= Location - Normal * MirrorDist;
	Diffuse			= OurAbs(BaseNormalDelta * RRadius);
	};

/*------------------------------------------------------------------------------------
	Table management
------------------------------------------------------------------------------------*/

//
// Function to make sure that the temporary tables for this light mesh have been built.
//
void FGlobalLightManager::BuildTemporaryTables()
{
	if( !TemporaryTablesBuilt )
	{
		TemporaryTablesBuilt=1;
		
		// Build saturated multiply table:
		
		MultiplyTab[0] = MultiplyTab[1] = 0;
		MultiplyTab[MeshVSize+2] = MultiplyTab[MeshVSize+3] = (MeshVSize-1) * MeshUByteSize;
		
		int Count=0;
		for( int i=0; i<MeshVSize; i++ )
		{
			MultiplyTab[i+2] = Count; 
			Count += MeshUByteSize;
		}

	// Build inverse vectors:
	TextureU	= ModelInfo->FVectors[Surf->vTextureU];
	TextureV	= ModelInfo->FVectors[Surf->vTextureV];
	Normal		= ModelInfo->FVectors[Surf->vNormal];
	
	// optimize: Cache the inverted vectors somewhere (how many are active per frame?) or
	// precompute them. This is a big CPU drain.
	Base = ModelInfo->FPoints [Surf->pBase];
	InvertVectors(TextureU,TextureV,Normal,InverseUAxis,InverseVAxis,InverseNAxis);

	InverseUAxis *= 65536.0;
	InverseVAxis *= 65536.0;
	}
}

/*------------------------------------------------------------------------------------
	Intermediate map generation code
------------------------------------------------------------------------------------*/

//
// Generate the shadow map for one lightsource that applies to a Bsp surface.
//
// Input: 1-bit-deep occlusion bitmask.
//
// Output: Floating point representation of the fraction of occlusion
// between each point and the lightsource, and range from 0.0 (fully occluded) to
// 1.0 (fully unoccluded).  These values refer only to occlusion and say nothing
// about resultant lighting (brightness, incidence, etc) which IlluminationMapGen handles.
//
void FGlobalLightManager::ShadowMapGen_F(BYTE *SrcBits,FLOAT *Dest1)
{
	static int DirtAccumulator;
	BuildTemporaryTables(); // Build temporary tables if they're not already built
	//
	// Smooth and convert to floating point:
	//
	FLOAT *Dest = Dest1;
	for( int V=0; V<MeshVSize; V++ )
	{
#if SHADOW_SMOOTHING
		// Generate smooth shadow map
		// optimize: Convert to assembly. This is called a lot.
		for( int U=0; U<MeshUSize; U+=2 )
		{
			int UPos	= (U-2) >> 3;
			int Shift1	= (U-2) &  7;
			int Shift2	= (U-1) &  7;
			int A		= 0;
			int B		= 0;
			//
			for( int V2=0; V2<5; V2++ )
			{
				int		Pos		= UPos + MultiplyTab[V+V2];
				DWORD	*Filter = FilterTab[V2];

				int		D		= (DWORD)(SrcBits[Pos]) + ((DWORD)(SrcBits[Pos+1]) << 8);

				A     += Filter [(D >> Shift1) & 0x1f];
				B     += Filter [(D >> Shift2) & 0x1f];
			}
			*Dest++ = (FLOAT)A * (1.0/1024.0);
			*Dest++ = (FLOAT)B * (1.0/1024.0);
		}
#else
		// Generate abrupt shadow map, for testing/debugging only
		for( int U=0; U<MeshUSize; U++ )
		{
			int Base = U>>3;
			int Ofs  = U&7;
			*Dest++ = (SrcBits[V*MeshUByteSize + Base] & (1<<Ofs)) ? 1.0 : 0.0;
		}
#endif
	}
	if( PolyFlags & PF_DirtyShadows )
	{
		// Apply noise to shadow map
		// optimize: Convert to assembly.
		Dest = Dest1;
		FLOAT* End = &Dest[MeshSpace];
		while( Dest < End )
		{
			*Dest++ *= 0.80 + 0.20 * RandomBases[DirtAccumulator++ & (N_RANDS-1)];
		}
	}
}

//
// Generate the illumination map for one lightsource that applies to a Bsp surface,
// based on a shadow map and positional actor information.  This illumination map accounts
// for distance from lightsource and incidence, but not the lightsource's brightness, color,
// or lattice effects, which ScaledSpanGen() handle.
//
// Input: Floating point shadow map, light info.
//
// Output: Floating point illumination map.
//
void FGlobalLightManager::IlluminationMapGen(FLightInfo *Info,FLOAT *Src,FLOAT *Dest)
{
	BuildTemporaryTables(); // Build temporary tables if they're not already built
	STAT(GStat.MeshesGen+=MeshSpace);

	// Compute values for stepping through mesh points:
	FLOAT	U			= (FLOAT)UNFIX(Index->TextureUStart);
	FLOAT	V			= (FLOAT)UNFIX(Index->TextureVStart);
	FVector	Vertex		= Base + InverseUAxis*U + InverseVAxis*V;
	FVector VertexDU	= InverseUAxis * (FLOAT)Index->MeshSpacing;
	FVector VertexDV	= InverseVAxis * (FLOAT)Index->MeshSpacing;

	for( int VCounter = 0; VCounter < MeshVSize; VCounter++ )
	{
		Info->Effect.SpatialFxFunc(MeshUSize,Info,Src,Dest,Vertex,VertexDU);
		Vertex += VertexDV;
		Src    += MeshUSize;
		Dest   += MeshUSize;
	}
}

//
// Init a lighting span to zero.
//
void FGlobalLightManager::LightingSpanInit_F(FLOAT *Dest)
{
	for( int i=MeshUSize; i>0; i-- ) *Dest++ = 0;
}

//
// Build or retrieve a cached light mesh corresponding to all of the static lightsources that affect
// a surface.  This light mesh is time-invariant.
//
void FGlobalLightManager::StaticLightingMapGen(FLOAT *Result)
{
	GUARD;
	BYTE	*ShadowLoc	= ShadowBase;
	void	*DynMemTop  = GDynMem.Get(0);
	int		Offset		= 0;

	// Go through all lights and generate temporary shadow maps and illumination maps for the static ones:
	for( FLightInfo *Info = FirstLight; Info < LastLight; Info++ )
	{
		EActorLightOptimization Opt = LightOptimization(Info->Actor);
		if( Opt==LC_StaticLight ) // This light is static
		{
			// Build the light's temporary shadow map from the raytraced shadow bits:
			FLOAT *ShadowMap = (FLOAT *)GMem.Get(MeshSpace * sizeof(FLOAT));
			ShadowMapGen_F(ShadowLoc,ShadowMap);
			
			// Build the light's temporary illumination map:
			Info->IlluminationMap = (FLOAT *)GDynMem.Get(MeshSpace * sizeof(FLOAT));
			IlluminationMapGen(Info,ShadowMap,Info->IlluminationMap);

			// Release that temporary shadow map:
			GMem.Release(ShadowMap);
		}
		ShadowLoc += MeshByteSpace;
	}

	// Go through span-by-span and add all static lights to the cumulative static map:
	FLOAT *Dest = Result;
	int Key = iSurf;
	for( int k=MeshVSize; k>0; k-- )
	{
		LightingSpanInit_F(Dest);

		for( Info=FirstLight; Info<LastLight; Info++ )
		{
			EActorLightOptimization Opt = LightOptimization(Info->Actor);
			if( Opt == LC_StaticLight ) // This light is static
			{			
				// Scale by brightness and add to destination
				FGlobalLightManager::merge_F_None(Key,MeshUSize,Info->IlluminationMap,Dest,Info->Brightness);
				Info->IlluminationMap += MeshUSize;
			}
		}
		Dest += MeshUSize;
		Key  += iLightMesh + k;
	}
	GDynMem.Release(DynMemTop);
	UNGUARD("FGlobalLightManager::StaticLightingMapGen");
}

/*------------------------------------------------------------------------------------
	Light blocks
------------------------------------------------------------------------------------*/

//
// Release any temporary memory that was allocated by calls to BuildForPoly().  Temporary
// memory will typically have been allocated for the final light map when there are dynamic
// components to the lighting.
//
void FGlobalLightManager::ReleaseLightBlock()
{
	GUARD;

	GMem.Release(MemTop);

	STAT(GStat.Texelage += MeshUSize * MeshVSize);
	STAT(GStat.TexelMem += MeshUSize * MeshVSize * sizeof(FLOAT) * (LastLight-FirstLight));

	UNGUARD("FGlobalLightManager::ReleaseLightBlock");
}

/*------------------------------------------------------------------------------------
	Applying lattice effects
------------------------------------------------------------------------------------*/

//
// Apply all lattice special effects to a span of texture lattices. Called after
// lattice screenspace and texture coordinates are generated and before texture 
// rectangles are generated.
//
void FGlobalLightManager::ApplyLatticeEffects( FTexLattice* TopLattice, int Num )
{
	GUARD;
	for( FLightInfo* Info=FirstLight; Info<LastLight; Info++ )
	{
		if( Info->Effect.LatticeFxFunc != lattice_None )
		{
			Info->Effect.LatticeFxFunc( *Info, TopLattice, Num );
		}
	}
	UNGUARD("FGlobalLightManager::ApplyLatticeEffects");
}

/*------------------------------------------------------------------------------------
	All local light effects functions
------------------------------------------------------------------------------------*/

/////////////////////////////
// Simple effect functions //
/////////////////////////////

// No special effects, floating point
void FGlobalLightManager::merge_F_None(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale)
{
	// Optimize: convert to assembly.
	while( MeshUSize-- > 0 )
	{
		FLOAT Value = Scale * *Src;
		*Dest = Value + *Dest - Value * Dest[0]; 
		AssertInRange(*Dest,"merge_F_None");
		Src++; Dest++;
	}
}

// Torch wavering, floating point
void FGlobalLightManager::merge_F_TorchWaver(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale)
{
	// Optimize: convert to assembly.
	while( MeshUSize-- > 0 )
	{
		FLOAT Value = Scale * *Src * (0.95 + 0.05 * RandomBases[Key++ & (N_RANDS-1)]);
		*Dest = Value + *Dest - Value * Dest[0]; 
		AssertInRange(*Dest,"merge_F_TorchWaver");
		Src++; Dest++;
	}
}

// Fire wavering, floating point
void FGlobalLightManager::merge_F_FireWaver(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale)
{
	// Optimize: convert to assembly.
	while( MeshUSize-- > 0 )
	{
		FLOAT Value = Scale * *Src * (0.80 + 0.20 * RandomBases[Key++ & (N_RANDS-1)]);
		*Dest = Value + *Dest - Value * Dest[0]; 
		AssertInRange(*Dest,"merge_F_FireWaver");
		Src++; Dest++;
	}
}

// Water shimmering, floating point
void FGlobalLightManager::merge_F_WaterShimmer(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale)
{
	// Optimize: convert to assembly.
	while( MeshUSize-- > 0 )
	{
		FLOAT Value = *Src * Scale * (0.6 + 0.4 * Randoms[Key++ & (N_RANDS-1)]);
		*Dest = Value + *Dest - Value * Dest[0]; 
		AssertInRange(*Dest,"merge_F_WaterShimmer4");
		Src++; Dest++;
	}
}

// Merge routine for testing
void FGlobalLightManager::merge_F_Test(int Key, int MeshUSize, FLOAT *Src,FLOAT *Dest,FLOAT Scale)
{
	while( MeshUSize-- > 0 )
	{
		FLOAT Value = Scale * *Src;
		*Dest = Value + *Dest - Value * Dest[0]; 
		AssertInRange(*Dest,"merge_F_Test");
		Src++; Dest++;
	}
}

//////////////////////////////
// Spatial effect functions //
//////////////////////////////

//
// Convenience macros that give you access to the following parameters easily:
// Info			= FLightInfo pointer
// Vertex		= This point in space
// Location		= Location of light in space
// RRadiusMult	= Inverse radius multiplier
//
#define SPATIAL_BEGIN \
	FVector Location    = Info->Actor->Location; \
	FLOAT	RRadiusMult = Info->RRadiusMult; \
	FLOAT   Diffuse     = Info->Diffuse; \
	int		SqrtOfs; \
	for( ; MeshUSize>0; MeshUSize--,Vertex+=VertexDU,Src++,Dest++ ) { \
		ftoi( SqrtOfs, FDistSquared(Vertex,Location) * RRadiusMult ); \
		if( SqrtOfs<16384 && *Src!=0.0) {
#define SPATIAL_END } else *Dest = 0.0; }

// No effects, floating point
void FGlobalLightManager::spatial_None( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	//optimize: Covert function to assembly; this is used for 90% of all lights in the world.
	SPATIAL_BEGIN
		*Dest = *Src * Diffuse * GMath.LightSqrt(SqrtOfs);
		AssertInRange(*Dest,"spatial_None");
	SPATIAL_END
}

// Yawing searchlight effect
void FGlobalLightManager::spatial_SearchLight( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		FLOAT Angle = fmod
		(
			PI + 4.0 * atan2(Vertex.X-Location.X,Vertex.Y-Location.Y) 
		+	GServer.Ticks * 0.06,
			8.*PI
		);
		if( Angle<PI || Angle > PI*3.)
		{
			*Dest = 0.0;
		}
		else
		{
			FLOAT Scale = 0.5 + 0.5 * cos(Angle);
			FLOAT D     = 0.00006 * (OurSquare(Vertex.X-Location.X) + OurSquare(Vertex.Y-Location.Y));
			if (D<1.0) Scale *= D;
			*Dest = *Src * Scale * Diffuse * GMath.LightSqrt(SqrtOfs);
		}
		AssertInRange(*Dest,"spatial_Searchlight");
	SPATIAL_END
}

// Yawing rotor effect
void FGlobalLightManager::spatial_Rotor( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		FLOAT Angle = 6.0 * atan2(Vertex.X-Location.X,Vertex.Y-Location.Y);
		FLOAT Scale = 0.5 + 0.5 * cos(Angle + GServer.Ticks*0.09);
		FLOAT D     = 0.0001 * (OurSquare(Vertex.X-Location.X) + OurSquare(Vertex.Y-Location.Y));
		if (D<1.0) Scale = 1.0 - D + Scale * D;
		*Dest		= *Src * Scale * Diffuse * GMath.LightSqrt(SqrtOfs);
		AssertInRange(*Dest,"spatial_Rotor");
	SPATIAL_END
}

// Slow radial waves
void FGlobalLightManager::spatial_SlowWave( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		FLOAT Scale	= 0.7 + 0.3 * GMath.SinTab(((int)FDist(Vertex,Location) - GServer.Ticks)<<10);
		*Dest		= *Src * Scale * Diffuse * GMath.LightSqrt(SqrtOfs);
		AssertInRange(*Dest,"spatial_SlowWave");
	SPATIAL_END
}

// Fast radial waves
void FGlobalLightManager::spatial_FastWave( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		FLOAT Scale	= 0.7 + 0.3 * GMath.SinTab((((int)FDist(Vertex,Location)>>2) - GServer.Ticks)<<11);
		*Dest		= *Src * Scale * Diffuse * GMath.LightSqrt(SqrtOfs);
		AssertInRange(*Dest,"spatial_FastWave");
	SPATIAL_END
}

// Scrolling clouds
void FGlobalLightManager::spatial_CloudCast( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	BYTE	*Data	= GGfx.BackdropTexture->GetData() + GGfx.BackdropTexture->MipOfs[0];
	BYTE	VShift	= GGfx.BackdropTexture->UBits;
	int		UMask	= GGfx.BackdropTexture->USize-1;
	int		VMask	= GGfx.BackdropTexture->VSize-1;

	// optimize: Convert to assembly and optimize reasonably well. This routine is often used
	// for large outdoors areas.
	SPATIAL_BEGIN
		int		FixU	= ftoi((Vertex.X+Vertex.Z) * 256.0 * 0.2);
		int		FixV	= ftoi((Vertex.Y+Vertex.Z) * 256.0 * 0.2);
		int		U0		= ((FixU >> 8) + GServer.Ticks) & UMask;
		int		U1		= (U0          + 1            ) & UMask;
		int		V0		= ((FixV >> 8) + GServer.Ticks) & VMask;
		int		V1		= (V0          + 1            ) & VMask;
		FLOAT	Alpha	= (FLOAT)(FixU & 255);
		FLOAT	Beta	= (FLOAT)(FixV & 255);

		FLOAT Brightness = 0.5 + 0.5 * 
			(
				(FLOAT)Data[U0 + (V0<<VShift)] * (Alpha      ) * (Beta      ) +
				(FLOAT)Data[U1 + (V0<<VShift)] * (256.0-Alpha) * (Beta      ) +
				(FLOAT)Data[U0 + (V1<<VShift)] * (Alpha      ) * (256.0-Beta) +
				(FLOAT)Data[U1 + (V1<<VShift)] * (256.0-Alpha) * (256.0-Beta)
			) / (256.0 * 256.0 * 256.0);

		*Dest = *Src * Diffuse * GMath.LightSqrt(SqrtOfs) * Brightness;
		AssertInRange(*Dest,"spatial_CloudCast");
	SPATIAL_END
}

// Scrolling parallax cloud layers
void FGlobalLightManager::spatial_StormCast( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	BYTE	*Data	= GGfx.BackdropTexture->GetData() + GGfx.BackdropTexture->MipOfs[0];
	BYTE	VShift	= GGfx.BackdropTexture->UBits;
	int		UMask	= GGfx.BackdropTexture->USize-1;
	int		VMask	= GGfx.BackdropTexture->VSize-1;

	SPATIAL_BEGIN
		int		U		= ftoi((Vertex.X+Vertex.Z)*0.2);
		int		V		= ftoi((Vertex.Y+Vertex.Z)*0.2);

		FLOAT Brightness =
			(
			5.0*(FLOAT)Data[((U+U+GServer.Ticks)&UMask) + (((V-3*GServer.Ticks)&VMask)<<VShift)] +
			7.0*(FLOAT)Data[((U-3*GServer.Ticks)&UMask) + (((U+V+GServer.Ticks)&VMask)<<VShift)] +
			4.0*(FLOAT)Data[((V  +GServer.Ticks)&UMask) + (((U+2*GServer.Ticks)&VMask)<<VShift)]
			) / (256.0 * 16.0);

		*Dest = *Src * Diffuse * GMath.LightSqrt(SqrtOfs) * Brightness;
		AssertInRange(*Dest,"spatial_StormCast");
	SPATIAL_END
}

// Shock wave
void FGlobalLightManager::spatial_Shock( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		int Dist = 8.0 * (Vertex - Location).Size();
		FLOAT Brightness  = 0.9 + 0.1 * GMath.SinTab(((Dist<<1) - (GServer.Ticks << 7))<<4);
		Brightness *= 0.9 + 0.1 * GMath.CosTab(((Dist   ) + (GServer.Ticks << 7))<<4);
		Brightness *= 0.9 + 0.1 * GMath.SinTab(((Dist>>1) - (GServer.Ticks << 7))<<4);
		*Dest = *Src * Diffuse * GMath.LightSqrt(SqrtOfs) * Brightness;
		AssertInRange(*Dest,"spatial_Shock");
	SPATIAL_END
}

// Disco ball
void FGlobalLightManager::spatial_Disco( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		FLOAT Yaw	= 11.0 * atan2(Vertex.X-Location.X,Vertex.Y-Location.Y);
		FLOAT Pitch = 11.0 * atan2(sqrt(OurSquare(Vertex.X-Location.X)+OurSquare(Vertex.Y-Location.Y)),Vertex.Z-Location.Z);

		FLOAT Scale1 = 0.50 + 0.50 * cos(Yaw   + GServer.Ticks*0.15);
		FLOAT Scale2 = 0.50 + 0.50 * cos(Pitch + GServer.Ticks*0.15);

		FLOAT Scale  = Scale1 + Scale2 - Scale1 * Scale2;

		FLOAT D     = 0.00005 * (OurSquare(Vertex.X-Location.X) + OurSquare(Vertex.Y-Location.Y));
		if (D<1.0) Scale *= D;

		*Dest = *Src * (1.0-Scale) * Diffuse * GMath.LightSqrt(SqrtOfs);
		AssertInRange(*Dest,"spatial_Disco");
	SPATIAL_END
}

// Unused
void FGlobalLightManager::spatial_NegativeLight( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	// Dynamic shadow test
	FVector ActorLocation = Camera->Actor->Location;
	//Temp.X += 200.0; Temp.Y += 200.0;
	SPATIAL_BEGIN
		//FVector Temp = Vertex; Temp.TransformPoint(Camera->Uncoords);
		//FLOAT Dist = 0.01 * (Temp - Camera->Actor->Location).Size2D();
		FLOAT Dist = 0.016 * (Vertex - ActorLocation).Size2D();
		if (Dist > 1.0) Dist = 1.0;
		FLOAT V = (Dist > 1.0) ? 1.0 : Dist*Dist;
		*Dest = *Src * Diffuse * GMath.LightSqrt(SqrtOfs) * V;
		AssertInRange(*Dest,"spatial_NegativeLight");
	SPATIAL_END
}

// Cylinder lightsource
void FGlobalLightManager::spatial_Cylinder( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		FLOAT Modulate = OurMin(GMath.LightSqrt(SqrtOfs) * 50.f,1.f);
		ftoi( SqrtOfs, 0.5 * OurMax((FLOAT)10.0,(OurSquare(Vertex.X-Location.X) + OurSquare(Vertex.Y-Location.Y)) * RRadiusMult) );
		if ( SqrtOfs>=0 && SqrtOfs<16384 )
		{
			*Dest = OurClamp(*Src * Diffuse * GMath.LightSqrt(SqrtOfs),0.f,1.f) * Modulate;
		}
		else *Dest = 0.0;
		AssertInRange(*Dest,"spatial_Cylinder");
	SPATIAL_END
}

// Interference pattern
void FGlobalLightManager::spatial_Interference( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	SPATIAL_BEGIN
		FLOAT Pitch = 11.0 * atan2(sqrt(OurSquare(Vertex.X-Location.X)+OurSquare(Vertex.Y-Location.Y)),Vertex.Z-Location.Z);
		FLOAT Scale = 0.50 + 0.50 * cos(Pitch + GServer.Ticks*0.15);

		*Dest = *Src * Scale * Diffuse * GMath.LightSqrt(SqrtOfs);
		AssertInRange(*Dest,"spatial_Interference");
	SPATIAL_END
}

// Spatial routine for testing
void FGlobalLightManager::spatial_Test( int MeshUSize, FLightInfo *Info, FLOAT *Src, FLOAT *Dest, FVector Vertex, FVector VertexDU )
{
	FVector Location    = Info->Actor->Location; \
	FLOAT	RRadiusMult = Info->RRadiusMult; \
	FLOAT   Diffuse     = Info->Diffuse; \
	for( ; MeshUSize>0; MeshUSize--,Vertex+=VertexDU,Src++,Dest++ )
	{
		FVector	M		= (Info->Actor->Location - Base).Mirror(Normal) + Base;
		FVector	A		= Vertex - M;
		FVector	B		= Vertex - Camera->Actor->Location;
		FLOAT	Temp	= OurSquare(OurSquare(OurSquare((A|B)/(A.Size() * B.Size()))));
		*Dest			= *Src * Temp; // * GMath.LightSqrt(SqrtOfs);
		AssertInRange(*Dest,"spatial_Test");
	}
}

/////////////////////////////
// Lattice warping effects //
/////////////////////////////

// No lattice effect.
void FGlobalLightManager::lattice_None( FLightInfo& Info, FTexLattice* TopLattice, int Num )
{
}

// Warping effect.
void FGlobalLightManager::lattice_WarpU( FLightInfo& Info, FTexLattice* TopLattice, int Num )
{
	while ( Num-- > 0 )
	{
		FVector Temp = TopLattice->Loc;
		Temp.TransformVector(Camera->Uncoords);
		Temp += Camera->Coords.Origin;

		TopLattice->U += 65536.0 * 16.0 * cos(Temp.X * 0.01 + GServer.Ticks * 0.1);
		TopLattice->V += 65536.0 * 16.0 * sin(Temp.Y * 0.01 + GServer.Ticks * 0.1);
		TopLattice++;
	}
}

// Warping effect.
void FGlobalLightManager::lattice_WarpV( FLightInfo& Info, FTexLattice* TopLattice, int Num )
{
}

// Wavy watter effect.
void FGlobalLightManager::lattice_CalmWater( FLightInfo& Info, FTexLattice* TopLattice, int Num )
{
	while ( Num-- > 0 )
	{
		FVector Temp = TopLattice->Loc;
		Temp.TransformVector(Camera->Uncoords);
		Temp += Camera->Coords.Origin;

		FLOAT Dist = (TopLattice->Loc - Info.Location).Size();
		if( Dist < Info.Radius )
		{
			Dist = 65536.0 * 6.0 * OurSquare((Info.Radius-Dist)/Info.Radius);
			TopLattice->U += Dist * (0.6*cos(Temp.X * 0.01 + GServer.Ticks * 0.1)+cos(Temp.X * 0.006 + GServer.Ticks * 0.06));
			TopLattice->V += Dist * (0.6*sin(Temp.Y * 0.01 + GServer.Ticks * 0.1)+sin(Temp.Z * 0.006 + GServer.Ticks * 0.06));
		}
		TopLattice++;
	}
}

// Churning water effect.
void FGlobalLightManager::lattice_ChurningWater( FLightInfo& Info, FTexLattice* TopLattice, int Num )
{
	while ( Num-- > 0 )
	{
		FVector Temp = TopLattice->Loc - Info.Location;
		Temp.TransformVector(Camera->Uncoords);

		FLOAT D = Temp.Size();
		Temp *= 1.0/D;

		TopLattice->U   += 65536.0 * 16.0 * Temp.Y * GMath.SinTab((ftoi(16.0*D) - (GServer.Ticks << 4)) << 4);
		TopLattice->V   += 65536.0 * 16.0 * Temp.X * GMath.CosTab((ftoi(16.0*D) - (GServer.Ticks << 4)) << 4);

		TopLattice++;
	}
}

void FGlobalLightManager::lattice_Test( FLightInfo& Info, FTexLattice* TopLattice, int Num )
{
}

///////////////////////////////////
// Global light effect functions //
///////////////////////////////////

// No global lighting
void FGlobalLightManager::global_None( class FLightInfo &Light, AActorDraw &Actor )
{
	Light.Brightness=0.0;
}

// Steady global lighting
void FGlobalLightManager::global_Steady( class FLightInfo &Light, AActorDraw &Actor )
{
}

// Global light pulsing effect
void FGlobalLightManager::global_Pulse( class FLightInfo &Light, AActorDraw &Actor )
{
	Light.Brightness *= 0.700 + 0.299 * GMath.SinTab
	(
		(GServer.Ticks<<16) / OurMax((int)Actor.LightPeriod,1) + (Actor.LightPhase << 8)
	);
}

// Global blinking effect
void FGlobalLightManager::global_Blink( class FLightInfo &Light, AActorDraw &Actor )
{
	if( ((GServer.Ticks<<16)/(Actor.LightPeriod+1) + 
		(Actor.LightPhase << 8)) & 1 )
		Light.Brightness = 0.0;
}

// Global flicker effect
void FGlobalLightManager::global_Flicker( class FLightInfo &Light, AActorDraw &Actor )
{
	FLOAT Random = (FLOAT)rand()/RAND_MAX;

	if( Random < 0.5 )	Light.Brightness = 0.0;
	else				Light.Brightness *= Random;
}

// Strobe light.
void FGlobalLightManager::global_Strobe( class FLightInfo &Light, AActorDraw &Actor )
{
	static int LastTicks=0,Toggle=0;
	if( LastTicks != GServer.Ticks )
	{
		LastTicks = GServer.Ticks;
		Toggle ^= 1;
	}
	if( Toggle ) Light.Brightness = 0.0;
}

// Unused.
void FGlobalLightManager::global_Explode2( class FLightInfo &Light, AActorDraw &Actor )
{
}

// Unused.
void FGlobalLightManager::global_Explode3( class FLightInfo &Light, AActorDraw &Actor )
{
}

// Sun lighting
void FGlobalLightManager::global_Daylight( class FLightInfo &Light, AActorDraw &Actor )
{
	int Base = GServer.Minute + GServer.Hour*60;
	int NoonDistance = OurAbs(Base - 12*60);

	if ( NoonDistance > 9*60 )		Light.Brightness = 0.0;
	else							Light.Brightness *= 1.0 - OurSquare(NoonDistance / (9.0 * 60.0));
}

// Moon lighting
void FGlobalLightManager::global_Nightlight( class FLightInfo &Light, AActorDraw &Actor )
{
	int Base = GServer.Minute + GServer.Hour*60;
	int MidnightDistance = (Base < 12*60)
		?	(Base)
		:	(24*60 - Base);

	if ( MidnightDistance > 9*60 )	Light.Brightness = 0.0;
	else							Light.Brightness *= 1.0 - OurSquare(MidnightDistance / (9.0 * 60.0));
}

/*------------------------------------------------------------------------------------
	Implementation of FLightInfo class
------------------------------------------------------------------------------------*/

//
// Compute lighting information based on an actor lightsource.
//
void FGlobalLightManager::FLightInfo::ComputeFromActor(ICamera *Camera, INDEX iActor)
{
	GUARD;
	STAT(GStat.PolyLitesDrawn++);

	// Should precompute all of this once per actor-frame, not once per actor-poly-frame!!
	Actor			= &Camera->Level.Actors->Element(iActor);
	Radius			= Actor->WorldLightRadius();
	RRadius			= 1.0/OurMax((FLOAT)1.0,Radius);
	RRadiusMult		= 16383.0 * RRadius * RRadius;
	Location		= Actor->Location;
	Location.TransformPoint (Camera->Coords);

	// Figure out global dynamic lighting effect:
	ELightType Type = (ELightType)Actor->LightType;
	if(!(Camera->ShowFlags&SHOW_PlayerCtrl)) Type=LT_Steady;

	// Only compute global lighting effect once per actor per frame, so that
	// lights with random functions produce consistent lighting on all surfaces they hit.
	FLOAT *CachedBrightness = &ActorBrightness[iActor];
	if ( *CachedBrightness!=-1.0 )
	{
		Brightness = *CachedBrightness;
	}
	else
	{
		Brightness = (FLOAT)Actor->LightBrightness;
		if( Type<LT_MAX ) GLightTypeFuncs[Type](*this,*Actor);
		*CachedBrightness = Brightness;
	}

	// Find local dynamic lighting effect:
	Effect = Effects[(Actor->LightEffect<LE_MAX) ? Actor->LightEffect : 0];
	if( Effect.LatticeFxFunc != lattice_None ) LatticeEffects=1;

	// Compute brightness:
	if( Camera->ColorBytes == 1 ) Brightness *= (0.5 + (0.5/256.0) * Actor->LightSaturation);
	Brightness = OurClamp(Brightness * (1./256.),0.,1.);
	Brightness *= sqrt(Brightness);

	UNGUARD("FGlobalLightManager::FLightInfo::ComputeFromActor");
}

/*------------------------------------------------------------------------------------
	Implementation of FLightList class
------------------------------------------------------------------------------------*/

//
// Init basic properties.
//
void FGlobalLightManager::SetupForNothing( ICamera *ThisCamera )
{
	// Set overall parameters.
	Camera					= ThisCamera;
	Level					= &Camera->Level;
	ModelInfo				= &Level->ModelInfo;

	// Init variables.
	LatticeEffects			= 0;
	TemporaryTablesBuilt	= 0;	
	MemTop					= GMem.Get(0);

	// Init lattice sizing.
	MinXBits = MinYBits = 0;
	MaxXBits = MaxYBits = 8;
}

//
// Compute fast light list for a Bsp polygon:
//
void FGlobalLightManager::SetupForPoly(ICamera *ThisCamera, FVector &Normal, FVector &Base, INDEX iThisSurf)
{
	GUARD;
	ALWAYS_BEGINTIME(GStat.IllumTime);

	SetupForNothing( ThisCamera );

	iSurf		= iThisSurf;
	Surf		= &Level->ModelInfo.BspSurfs[iSurf];
	iLightMesh	= Surf->iLightMesh;
	LightMesh	= Level->ModelInfo.LightMesh;

	static int TempTicks=0;
	if (TempTicks!=GServer.Ticks)
	{
		TempTicks=GServer.Ticks;
		Tick();
	}
	int Key=0;
	
	int StaticLights=0,DynamicLights=0,MovingLights=0,StaticLightingChange=0;

	// Empty out the list of lights:	
	LastLight   = FirstLight;

	// Get light mesh index, if any:
	Index		= GetLightMeshIndex (ModelInfo,iSurf);
	if( !Index )
	{
		ALWAYS_ENDTIME(GStat.IllumTime);
		return;
	}

	// Set up parameters for each light and count lights of each type:
	int n = Index->NumStaticLights + Index->NumDynamicLights;
	for( int i=0; i<n; i++ )
	{
		INDEX  iActor = Index->iLightActor[i];
		AActor *Actor = &Camera->Level.Actors->Element(iActor);
		if( Actor->Class && Actor->LightType!=LT_None )
		{
			LastLight->ComputeFromActor  (Camera,iActor);
			LastLight->SetupRectSampling (Base,Normal);

			EActorLightOptimization Opt = LightOptimization(LastLight->Actor);
			if( Opt==LC_StaticLight )
			{
				StaticLights++;
				StaticLightingChange += LastLight->Actor->bTempLightChanged;
			}
			else if( Opt==LC_DynamicLight )
			{
				DynamicLights++;
			}
			else if( Opt==LC_MovingLight)
			{
				MovingLights++;
			}

			if( ++LastLight >= &FirstLight[MAX_LIGHTS] ) break;
		}
	}
	
	// Set FLightList values:
	PolyFlags		= Surf->PolyFlags;
	ShadowBase		= &((BYTE *)LightMesh->GetData())[Index->DataOffset];

	MeshUSize		= (Index->MeshUSize+7) & ~7;
	MeshVSize		= Index->MeshVSize;
	MeshUByteSize	= MeshUSize >> 3;

	MeshUBits		= FLogTwo(MeshUSize);
	MeshVBits		= FLogTwo(MeshVSize);

	MeshUTile		= 1 << MeshUBits;
	MeshVTile		= 1 << MeshVBits;

	MeshSpace		= MeshUSize * MeshVSize;
	MeshTileSpace	= MeshUTile * MeshVTile;
	MeshByteSpace	= MeshUByteSize * MeshVSize;

	MeshAndMask		= ((MeshVSize-1) + ((MeshUSize-1) << (32-MeshUBits)));

	MeshSkipSize	= 1 << (MeshUBits +  MeshVBits);
	
	// Generate static lighting, if any:
	FLOAT *Static=NULL;
	if( StaticLights )
	{
		// Look up cached static light map:
		int CacheID   = ((int)iLightMesh << 16) + 0x700;
		Static        = (FLOAT *)GCache.Get(CacheID);

		if( !Static || StaticLightingChange )
		{
			// Generate new static light map and cache it:
			if ( !Static ) Static = (FLOAT *)GCache.Create(CacheID,MeshSpace * sizeof(FLOAT));
			StaticLightingMapGen(Static);
		}
	}

	// Generate intermediate meshes for dynamic lights, if any:
	if( DynamicLights )
	{
		BYTE *ShadowLoc = ShadowBase;
		for( FLightInfo* Info=FirstLight; Info<LastLight; Info++ )
		{
			EActorLightOptimization Opt = LightOptimization(Info->Actor);
			if( Opt==LC_DynamicLight )
			{
				// Get the light's illumination map:
				if( Info->Effect.IsSpatialDynamic )
				{
					// This light has spatial effects, so we must cache its shadow map since
					// we will be generating its illumination map per frame.

					int CacheID			= ((int)iLightMesh << 16) + 0x1A00 + (Info-FirstLight);
					FLOAT *ShadowMap	= (FLOAT *)GCache.Get(CacheID);
					if( !ShadowMap || Info->Actor->bTempLightChanged )
					{
						// Create and generate its shadow map:
						if (!ShadowMap) ShadowMap = (FLOAT *)GCache.Create(CacheID,MeshSpace * sizeof(FLOAT));
						ShadowMapGen_F(ShadowLoc,ShadowMap);
					}

					// Build a temporary illumination map:
					Info->IlluminationMap = (FLOAT *)GDynMem.Get(MeshSpace * sizeof(FLOAT));
					IlluminationMapGen(Info,ShadowMap,Info->IlluminationMap);
				}
				else
				{
					// No spatial lighting. We use a cached illumination map generated from a temporary
					// shadow map. See if the illumination map is already cached:

					int CacheID				= ((int)iLightMesh << 16) + 0x1900 + (Info-FirstLight);
					Info->IlluminationMap	= (FLOAT *)GCache.Get(CacheID);

					if( !Info->IlluminationMap || Info->Actor->bTempLightChanged )
					{
						// Build a temporary shadow map.
						FLOAT *ShadowMap = (FLOAT *)GDynMem.Get(MeshSpace * sizeof(FLOAT));
						ShadowMapGen_F(ShadowLoc,ShadowMap);

						// Build and cache an illumination map
						if ( !Info->IlluminationMap ) Info->IlluminationMap = (FLOAT *)GCache.Create(CacheID,MeshSpace * sizeof(FLOAT));
						IlluminationMapGen(Info,ShadowMap,Info->IlluminationMap);

						// Release the temporary shadow map.
						GDynMem.Release(ShadowMap);
					}
				}
			}
			ShadowLoc += MeshByteSpace;
		}
	}

	// Generate temporary meshes for moving lights:
	if( MovingLights )
	{
		for( FLightInfo* Info=FirstLight; Info<LastLight; Info++ )
		{
			EActorLightOptimization Opt = LightOptimization(Info->Actor);
			if( Opt == LC_MovingLight )
			{

				// Build a temporary shadow map and fill it with 1.0.
				FLOAT *ShadowMap = (FLOAT *)GMem.Get(MeshSpace * sizeof(FLOAT));
				FLOAT *Temp      = ShadowMap;
				for( int i=MeshSpace; i>0; i--) *Temp++ = 1.0;

				// Build a temporary illumination map.
				Info->IlluminationMap = (FLOAT *)GDynMem.Get(MeshSpace * sizeof(FLOAT));
				IlluminationMapGen(Info,ShadowMap,Info->IlluminationMap);

				// Release the temporary shadow map.
				GMem.Release(ShadowMap);
			}
		}
	}

	// Copy static map to power-of-two area of memory and merge any dynamic light maps in:
	int IsDynamic		= DynamicLights || MovingLights;
	int CacheID			= ((int)iLightMesh << 16) + 0x900;

	if( IsDynamic )		MeshVoid = NULL;
	else				MeshVoid = MeshVoid = GCache.Get(CacheID);

	if( !MeshVoid || (Surf->PolyFlags & PF_DynamicLight))
	{
		Surf->PolyFlags &= ~PF_DynamicLight;
		if( IsDynamic ) Surf->PolyFlags |= PF_DynamicLight;

		int CopyLeft	= (MeshUTile-MeshUSize) >> 1; // CopyLeft+USize+CopyRight = USizeTiled
		int CopyRight	= (MeshUTile-MeshUSize) - CopyLeft;

		// Generate cache entry if needed
		if( !MeshVoid )
		{
			int Size = ((MeshUTile)*(MeshVTile+1))*sizeof(FLOAT);
			if ( IsDynamic )	MeshVoid = GMem.Get(Size);
			else				MeshVoid = GCache.Create(CacheID,Size);
		}
		FLOAT *Dest = (FLOAT*)MeshVoid;
		
		// Loop through the source mesh area:
		FLOAT *Src = Static;
		Key = iSurf;
		for( int V=0; V<MeshVSize; V++ )
		{
			FLOAT *StartDest = Dest;

			// Move our static lighting, if any, to the result map:
			if( Static )
			{
				// Copy static lighting to the result map:
				// optimize: Convert to assembly if needed.
				for( int U=MeshUSize; U>0; U-- ) *Dest++ = *Src++;
			}
			else
			{
				// There is no static lighting, so zero out the result map:
				// optimize: Convert to assembly if needed.
				for( int U=MeshUSize; U>0; U-- ) *Dest++ = 0.0;
			}
			
			// Add dynamic lighting to the result map:
			if( DynamicLights || MovingLights )
			{
				for( FLightInfo* Info=FirstLight; Info<LastLight; Info++ )
				{
					EActorLightOptimization Opt = LightOptimization(Info->Actor);
					if( Opt==LC_DynamicLight || Opt==LC_MovingLight )
					{
						Info->Effect.MergeFxFunc( Key,MeshUSize,Info->IlluminationMap,StartDest,Info->Brightness);
						Info->IlluminationMap += MeshUSize;
					}
				}
			}

			// Scale the light mesh values to a range amicable to the mapping inner loop.
			for( int U=0; U<MeshUSize; U++ )
			{
				// optimize: Convert to assembly.
				StartDest[U] = (FLOAT)((3<<22) + 0x10) + OurMin(2.8*StartDest[U],1.0) * (FLOAT)0x3cf0;
			}

			// Pad the result map's left and right edges for clean wraparound:
			if( CopyRight )
			{
				// optimize: Convert to assembly.
				FLOAT Right = Dest[-1];
				for( int i=0; i<CopyRight; i++ ) *Dest++ = Right;

				FLOAT Left = StartDest[0];
				for ( i=0; i<CopyLeft; i++ ) *Dest++ = Left;
			}
			Key += iLightMesh + V;
		}

		// Fill in the overflow area past the end of the source mesh:
		for( ; V<=MeshVTile; V++ )
		{
			// optimize: Convert to assembly.
			Src = (FLOAT*)MeshVoid;
			for( int U=0; U<MeshUTile; U++ ) *Dest++ = *Src++;
		}
	}
	ALWAYS_ENDTIME(GStat.IllumTime);
	UNGUARD("FGlobalLightManager::SetupForPoly");
}

//
// Compute fast light list for an actor.
//
void FGlobalLightManager::SetupForActor(ICamera* ThisCamera, INDEX iActor)
{
	GUARD;
	
	// Init per actor variables.
	SetupForNothing( ThisCamera );

	LastLight = FirstLight;
	AActor* Actor = &Camera->Level.Actors->Element(iActor);
	AActor *TestActor = &Camera->Level.Actors->Element(0);

	for( INDEX i=0; i<Camera->Level.Actors->Max; i++ )
	{
		if( TestActor->LightType!=LT_None && i!=iActor)
		{
			FLOAT Radius = TestActor->WorldLightRadius();
			if( !TestActor->LightRadius || FDistSquared(Actor->Location,TestActor->Location)<Radius * Radius)
			{
				if( Camera->Level.ModelInfo.LineClass(&Actor->Location,&TestActor->Location) )
				{
					LastLight->ComputeFromActor(Camera,i);
					if( ++LastLight >= FinalLight ) break;
				}
			}
		}
		TestActor++;
	}
	UNGUARD("FGlobalLightManager::SetupForActor");
}

/*-----------------------------------------------------------------------------
	Per-frame dynamic lighting pass
-----------------------------------------------------------------------------*/

//
// Callback for applying a dynamic light to a node.
//
AActor *GTempLightActor;
void ApplyLightCallback (IModel *ModelInfo, INDEX iNode, int iActor)
{
	while( iNode != INDEX_NONE )
	{
		FBspNode			*Node	= &ModelInfo->BspNodes [iNode];
		FBspSurf			*Poly	= &ModelInfo->BspSurfs [Node->iSurf];
		FLightMeshIndex		*Index	= FGlobalLightManager::GetLightMeshIndex(ModelInfo,Node->iSurf);

		if( !(Node->NodeFlags & NF_PolyOccluded )
			&& Index && FGlobalLightManager::NumDynLightPolys < FGlobalLightManager::MAX_DYN_LIGHT_POLYS
			&& (GTempLightActor->bSpecialLit ? (Poly->PolyFlags&PF_SpecialLit) : !(Poly->PolyFlags&PF_SpecialLit)) )
		{
			int n = Index->NumStaticLights + Index->NumDynamicLights;
			if( n < Index->MAX_POLY_LIGHTS )
			{
				for( int i=0; i<n; i++ )
				{
					if( Index->iLightActor[i]==iActor ) goto NextNode; // Don't apply a light twice
				}
				Index->iLightActor[n] = iActor;
				if( !Index->NumDynamicLights )
				{
					FGlobalLightManager::DynLightPolys[FGlobalLightManager::NumDynLightPolys++] = Node->iSurf;
				}
				Index->NumDynamicLights++;
			}
		}
		NextNode:
		iNode = Node->iPlane;
	}
}

//
// Apply all dynamic lighting.
//
void FGlobalLightManager::DoDynamicLighting( ILevel *Level )
{
	GUARD;
	FLOAT *Brightness = FGlobalLightManager::ActorBrightness = (FLOAT*)GMem.Get(Level->Actors->Max * sizeof(FLOAT));

	FGlobalLightManager::NumDynLightPolys = 0;
	AActor *Actor = &Level->Actors->Element(0);
	for( INDEX iActor=0; iActor<Level->Actors->Max; iActor++ )
	{
		*Brightness++ = -1.0;

		if( Actor->Class 
			&& Actor->LightType!=LT_None 
			&& (Actor->bTempDynamicLight || !Actor->bStaticActor) 
			&& Level->ModelInfo.NumBspNodes)
		{
			STAT(GStat.DynLightActors++);
			GTempLightActor = Actor;
			FLOAT Radius    = Actor->WorldLightRadius();
			if( Radius>0.0 )
			{
				Level->ModelInfo.PlaneFilter(&GTempLightActor->Location,Radius,ApplyLightCallback,NF_AllOccluded,iActor);
			}
		}
		Actor++;
	}
	UNGUARD("FGlobalLightManager::DoDynamicLighting");
}

//
// Remove all dynamic lighting.
//
void FGlobalLightManager::UndoDynamicLighting( ILevel *Level )
{
	GUARD;

	GMem.Release(FGlobalLightManager::ActorBrightness);
	if( !Level->ModelInfo.NumBspNodes ) return;
	
	for( int i=0; i<FGlobalLightManager::NumDynLightPolys; i++ )
	{
		FLightMeshIndex	*Index	= FGlobalLightManager::GetLightMeshIndex(&Level->ModelInfo,FGlobalLightManager::DynLightPolys[i]);
		Index->NumDynamicLights = 0;
	}
	
	AActor *Actor = &Level->Actors->Element(0);
	for( i=0; i<Level->Actors->Max; i++ )
	{
		(Actor++)->bTempLightChanged = 0;
	}
	UNGUARD("FGlobalLightManager::UndoDynamicLighting");
}

/*------------------------------------------------------------------------------------
	Light subsystem instantiation
------------------------------------------------------------------------------------*/

FGlobalLightManager			GLightManagerInstance;
FVirtualGlobalLightManager	*GLightManager = &GLightManagerInstance;

/*------------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------------*/
