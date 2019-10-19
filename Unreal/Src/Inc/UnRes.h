/*=============================================================================
	UnRes.h: Standard Unreal resource definitions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNRES // Prevent header from being included multiple times
#define _INC_UNRES

/*-----------------------------------------------------------------------------
	Definitions
-----------------------------------------------------------------------------*/

//
// A unique value assigned to each resource type recognized by Unreal:
//
enum EResourceType
	{
	RES_None			= 0,	// No resource
	RES_Buffer			= 1,	// A large binary object holding misc saveable data
	RES_Array			= 3,	// An array of resources
	RES_TextBuffer		= 4,	// A text buffer
	RES_Texture			= 5,	// Texture or compound texture
	RES_Font			= 6,	// Font for use in game
	RES_Palette			= 7,	// A palette
	RES_Script			= 9,	// Script
	RES_Class			= 10,	// An actor class
	RES_ActorList		= 11,	// An array of actors
	RES_Sound			= 12,	// Sound effect
	RES_Mesh			= 14,	// Animated mesh
	RES_Vectors			= 16,	// 32-bit floating point vector list
	RES_BspNodes		= 17,	// Bsp node list
	RES_BspSurfs		= 18,	// Bsp polygon list
	RES_LightMesh		= 19,	// Bsp polygon lighting mesh
	RES_Polys			= 20,	// Editor polygon list
	RES_Model			= 21,	// Model or level map
	RES_Level			= 22,	// A game level
	RES_Camera			= 25,	// A rendering camera on this machine
	RES_Player			= 28,	// A remote player logged into the local server
	RES_VertPool		= 29,	// A vertex pool corresponding to a Bsp and FPoints/FVectors table
	RES_Ambient			= 30,	// An ambient sound definition
	RES_TransBuffer		= 31,	// Transaction tracking buffer
	RES_MeshMap			= 32,	// MeshMap
	RES_Bounds			= 33,	// Bounding structure
	RES_Terrain			= 34,	// Terrain
	RES_Enum			= 35,	// Enumeration (array of FName's)
	RES_All				= 255,	// Matches all types
	};

/*-----------------------------------------------------------------------------
	Forward declarations
-----------------------------------------------------------------------------*/

//
// All resource classes
//
class UBuffer;
class UArray;
class UTextBuffer;
class UTexture;
class UFont;
class UPalette;
class UScript;
class UClass;
class UActorList;
class USound;
class UAmbient;
class UMesh;
class UVector;
class UBspNodes;
class ULightMesh;
class UPolys;
class UModel;
class ULevel;
class UCamera;
class UPlayer;
class UVertPool;
class UMeshMap;
class UBounds;
class UTerrain;
class UEnum;

//
// Locked info classes:
//
class UNREAL_API IModel;
class UNREAL_API ILevel;
class UNREAL_API ICamera;
class UNREAL_API ITexture;

//
// Other classes:
//
class FSocket;
class FConstraints;

/*-----------------------------------------------------------------------------
	UTextBuffer
-----------------------------------------------------------------------------*/

//
// A database that holds a bunch of text.  The text is contiguous and, if
// of nonzero length, is terminated by a NULL at the very last position.
//
class UNREAL_API UTextBuffer : public UDatabase
	{
	RESOURCE_DB_CLASS(UTextBuffer,char,RES_TextBuffer)
	//
	// Variables:
	//
	INT Pos{ 0 }; // Saved cursor position, i.e. for scripts in UnrealEd's script editor
	//
	// Resource operations:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	};

/*-----------------------------------------------------------------------------
	UBuffer
-----------------------------------------------------------------------------*/

//
// A database that holds raw byte data.
//
class UNREAL_API UBuffer : public UDatabase
	{
	RESOURCE_DB_CLASS(UBuffer,BYTE *,RES_Buffer)
	//
	// Resource operations:
	//
	void Register				(FResourceType *Type);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	};

/*-----------------------------------------------------------------------------
	UArray
-----------------------------------------------------------------------------*/

//
// A database that holds references to untyped resources.
//
class UNREAL_API UArray : public UDatabase
	{
	RESOURCE_DB_CLASS(UArray,UResource *,RES_Array)
	//
	// Resource operations:
	//
	void Register				(FResourceType *Type);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom operations:
	//
	int  Add	(UResource *NewElement);
	void Delete	(UResource *Element);
	void Empty	(void);
	};

//
// A template that deals with resource arrays type-safely.
//
template<class TResClass> class TArray : public UArray
	{
	public:
	inline int  Add (TResClass *NewElement) {return UArray::Add ((UResource *)NewElement);};
	inline void Delete (TResClass *Element) {UArray::Delete((UResource *)Element);};
	inline TResClass *&Element(int i) {return ((TResClass **)Data)[i];};
	inline TArray<TResClass>(int MaxElements,int Occupy=0)
		{
		GUARD;
		Num=0; Max=MaxElements; AllocData(0);
		if (Occupy) Num=Max;
		UNGUARD("TArray::Constructor");
		};
	};

/*-----------------------------------------------------------------------------
	UEnum
-----------------------------------------------------------------------------*/

//
// An enumeration, a database of names.  Used for enumerationsin
// Unreal scripts.
//
class UNREAL_API UEnum : public UDatabase
	{
	RESOURCE_DB_CLASS(UEnum,FName,RES_Enum)
	//
	// Resource operations:
	//
	void Register				(FResourceType *Type);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom operations:
	//
	int  Add	(FName NewName);
	int  AddTag	(const char *NewName);
	void Delete	(FName Element);
	void Empty	(void);
	};

/*-----------------------------------------------------------------------------
	UFont
-----------------------------------------------------------------------------*/

//
// Index of information about one font character.
//
class UNREAL_API FFontCharacter
	{
	public:
	int	StartU;			// Starting coordinate of character in texture
	int	StartV;			// Ending coordinate of character in texture
	int	USize;			// U-length of character, 0=none
	int	VSize;			// V-length of character
	};

//
// A font resource, containing a bunch of graphical ASCII characters.
//
class UNREAL_API UFont : public UDatabase
	{
	RESOURCE_DB_CLASS(UFont,FFontCharacter,RES_Font)
	//
	// Variables:
	//
	UTexture	*Texture{ nullptr };	// Texture containing font chars
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	};

/*-----------------------------------------------------------------------------
	UMesh
-----------------------------------------------------------------------------*/

//
// Packed mesh vertex vector for skinned meshes
//
class FMeshVertex
	{
	public:
	INT		X:11;
	INT		Y:11;
	INT 	Z:10;
	};

//
// Texture coordinates associated with a vertex and a mesh triangle.  All
// triangles sharing a vertex do not necessarily have the same texture
// coordinates there.
//
class FMeshUV
	{
	public:
	BYTE U;
	BYTE V;
	};

//
// One triangular polygon in a mesh, which references three vertices,
// and various drawing/texturing information.
//
class FMeshTriangle
	{
	public:
	WORD		iVertex[3];		// Vertex indices
	BYTE		Type;			// James' mesh type
	BYTE		Color;			// Color for flat and Gouraud shaded
	FMeshUV		Tex [3];		// Texture UV coordinates
	BYTE		TextureNum;		// Source texture offset
	BYTE		Flags;			// Unreal mesh flags (currently unused)
	};

//
// Flags associated with a mesh triangle
//
enum FMeshTriangleFlags
	{
	MT_Textured		= 0,		// Texture map the triangle
	MT_Flat			= 1,		// Draw it black and flat-shaded
	MT_Transparent	= 2,		// Draw it transparently
	MT_Masked		= 3,		// Draw it with masking
	};

//
// Information about one animation sequence associated with a mesh,
// a group of contiguous frames.
//
class FMeshAnimSeq
	{
	public:
	FName	SeqName;			// Sequence's name, so that scripts can access it easily
	INT		SeqStartFrame;		// Starting frame number
	INT		SeqNumFrames;		// Number of frames in sequence
	INT		Rate;				// Playback rate (scale still undefined)
	};

//
// Says which triangles a particular mesh vertex is associated with.
// Precomputed so that mesh triangles can be shaded with Gouraud-style
// shared, interpolated normal shading.
//
class FMeshVertLinkIndex
	{
	public:
	INT		NumVertTriangles;
	INT		TriangleListOffset;
	};

//
// A mesh, completely describing a 3D object (creature, weapon, etc) and
// its animation sequences.  Does not reference textures.
//
class UNREAL_API UMesh : public UResource
	{
	RESOURCE_CLASS(UMesh,BYTE,RES_Mesh)
	//
	// Variables:
	//
	FVector 		Origin;		// Origin in original coordinate system
	FRotation		RotOrigin;	// Amount to rotate when importing (mostly for yawing)
	FBoundingVolume	Bound;		// Bounding volume of all animation frames combined
	//
	INT NumTriangles;	INT MaxTriangles;   
	INT NumVertices;	INT MaxVertices{ 0 };
	INT NumVertLinks;	INT MaxVertLinks;
	INT NumAnimFrames;	INT MaxAnimFrames;	 
	INT NumAnimSeqs;	INT MaxAnimSeqs;	 
	//
	// Resource Functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	int  QuerySize				(void);
	int  QueryMinSize			(void);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	void SetSequence			(const char *SeqName, int StartFrame, int NumFrames);
	void GetInfo				(class IMesh *MeshInfo);
	};

void meshImport			(const char *MeshName, const char *AnivFname, const char *DataFname);

//
// Data size = MaxTriangles                * sizeof (FMeshTriangle) +
//             MaxVertices                 * sizeof (FMeshVertLinkIndex) +
//             MaxVertLinks                * sizeof (WORD) +
//             MaxAnimFrames * MaxVertices * sizeof (FMeshVertex) +
//             MaxAnimFrames               * sizeof (FBoundingVolume) +
//             MaxAnimSeqs                 * sizeof (FMeshAnimSeq)
//

//
// Information about a locked mesh, crammed into one structure for fast
// access.
//
class IMesh
	{
	public:
	INT TriMax,VertexMax,VertLinkIndexMax,VertLinkMax,AnimMax,BoundMax,TotalMax;
	//
	FMeshTriangle			*Triangles;
	FMeshVertex				*Vertex;
	FMeshVertLinkIndex		*VertLinkIndex;
	WORD					*VertLinks;
	FBoundingVolume			*Bound;
	FMeshAnimSeq			*AnimSeqs;
	};

/*-----------------------------------------------------------------------------
	UMeshMap
-----------------------------------------------------------------------------*/

//
// A mesh map, which describes a set of textures to apply to a mesh.  This
// is separated from the mesh structure itself so that one mesh can exist
// in several differently-textured forms.
//
class UNREAL_API UMeshMap : public UResource
	{
	RESOURCE_CLASS(UMeshMap,UTexture*,RES_MeshMap)
	//
	// Variables:
	//
	UMesh		*Mesh;
	DWORD		MaxTextures;
	DWORD		AndFlags;
	DWORD		OrFlags;
	FVector		Scale;
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	int  QuerySize				(void);
	int  QueryMinSize			(void);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom functions:
	//
	void Lock					(class IMeshMap *MeshMapInfo);
	void Unlock					(class IMeshMap *MeshMapInfo);
	};

//
// Information structure returned by locking a meshmap:
//
class IMeshMap
	{
	public:
	UMeshMap				*MeshMap;
	UMesh					*Mesh;
	TArray<UTexture>		*TextureArray;
	//
	INT						NumTriangles;
	INT	 					NumVertices;
	INT						NumVertLinks;
	INT						NumAnimFrames;
	INT						NumAnimSeqs;
	INT						MaxTextures;
	//
	FMeshTriangle			*Triangles;
	FMeshVertLinkIndex		*VertLinkIndex;
	FMeshVertex				*Vertex;
	FMeshAnimSeq			*AnimSeqs;
	FBoundingVolume			*Bound;
	UTexture				**Textures;
	WORD					*VertLinks;
	//
	FVector					Origin;
	FVector					Scale;
	FRotation				RotOrigin;
	};

/*-----------------------------------------------------------------------------
	UBspNode
-----------------------------------------------------------------------------*/

//
// Flags associated with a Bsp node.
//
enum EBspNodeFlags
	{
	NF_NotCsg			= 0x0001, // Node is not a Csg splitter, i.e. is a transparent poly
	NF_Sporadic			= 0x0002, // Node is sporadic (changes from time to time)
	NF_UniquePlane		= 0x0004, // Node's iUniquePlane is valid (otherwise is undefined)
	NF_Modified			= 0x0008, // Editor: Node was modified during editorCleanupNodes
	NF_TagForEmpty	 	= 0x0010, // Editor: Node's polys should be emptied after Csg
	NF_IsNew 		 	= 0x0020, // Editor: Node was newly-added
	NF_TerrainFront		= 0x0040, // Node front contains terrain, NumVertices=0, iVertPool = terrain index
	NF_PolyOccluded		= 0x0080, // Node's poly was occluded on the previously-drawn frame
	NF_AllOccluded		= 0x0100, // Node and all its children were occluded on previously-drawn frame
	NF_NegativePoly		= 0x0200, // The polygon at this node is negative and should be subtracted
	NF_SingleZone		= 0x0400, // This node and all nodes below it are all completely in iZone
	NF_NoMerge			= 0x0800, // Don't merge this node's polys
	NF_SeeThrough		= 0x1000, // Can see through node (for line-of-sight solid ops)
	NF_ShootThrough		= 0x2000, // Can shoot through (for projectile solid ops)
	NF_Bounded			= 0x4000, // Bound is valid (otherwise is undefined)
	NF_Portal			= 0x8000, // Temporary NodeFlags tag for zone portals
	//
	NF_NEVER_MOVE		= NF_TerrainFront, // Bsp cleanup must not move nodes with this tag
	};

//
//	FBspNode defines one node in the Bsp, including the front and back
//	pointers and the polygon data itself.  A node may have 0, 3, 4, 5, or 6
//	vertices. If the node has zero vertices, it's only used for splitting and
//	doesn't contain a polygon (this happens in the editor).
//
//	vNormal, vTextureU, vTextureV, and others are indices into the level's
//	vector table.  iFront,iBack should be INDEX_NONE to indicate no children.
//
//	Coplanar nodes: If iPlane==INDEX_NONE, a node has no coplanars.  Otherwise iPlane
//	is an index to a coplanar polygon in the Bsp.  All polygons that are iPlane
//	children can *only* have iPlane children themselves, not fronts or backs.
//
class FBspNode // 32 bytes
	{
	public:
	//
	enum {MAX_NODE_VERTICES=16};	// Max vertices in a Bsp node, pre clipping
	enum {MAX_FINAL_VERTICES=24};	// Max vertices in a Bsp node, post clipping
	enum {MANY_CHILDREN=8};			// A Bsp Node with this many children is occlusion-rejected carefully
	//
	// Persistent information:
	//
	QWORD			ZoneMask;		// 8 Bit mask for all zones at or below this node (up to 64)
	DWORD			NumVertices:8;	// 1 Number of vertices in node
	DWORD			iVertPool:24;	// 3 Index of first vertex in vertex pool, =iTerrain if NumVertices==0 and NF_TerrainFront
	INDEX			NodeFlags;		// 2 Bsp-related node bit flags
	INDEX			iSurf;			// 2 Index to surface information
	INDEX			iFront;			// 2 Index to node in front (in direction of Normal)
	INDEX			iBack;			// 2 Index to node in back  (opposite direction as Normal)
	INDEX			iPlane;			// 2 Index to next coplanar poly in coplanar list
	INDEX			iBound;			// 2 Optional Bounding volume index, valid only if NF_Bounded
	INDEX			iUniquePlane;	// 2 Unique plane number
	BYTE			iZone;			// 1 Visibility zone this node's front/poly falls into, 0 if none
	BYTE			iBackZone;		// 1 Visibility zone on back, 0 if none
	//
	// Valid in memory only:
	//
	INDEX			iDynamic[2];	// 2 Index to dynamic contents in front (0) and Back (1)
	//
	// Functions:
	//
	int inline IsCsg(void) {return (NumVertices>0) && !(NodeFlags & (NF_IsNew | NF_NotCsg));};
	int inline IsCsg(INDEX ExtraFlags) {return (NumVertices>0) && !(NodeFlags & (NF_IsNew | NF_NotCsg | ExtraFlags));};
	};

enum EZoneFlags
	{
	ZF_MAX	= 0, // We don't have any zone flags yet
	};

//
// Properties of a zone
//
class UNREAL_API FZoneProperties
	{
	public:
	//
	// General zone properties:
	//
	INDEX	iZoneActor;		// Actor defining the zone's property, or INDEX_NONE
	INDEX	Unusedl;
	DWORD	Unused2;
	//
	// Connectivity and visibility bit masks:
	//
	QWORD	Connectivity;	// (Connect[i]&(1<<j))==1 if zone i is adjacent to zone j
	QWORD	Visibility;		// (Connect[i]&(1<<j))==1 if zone i can see zone j
	QWORD	Unused3;
	};

//
// A database of Bsp nodes associated with a model.
//
class UNREAL_API UBspNodes : public UDatabase
	{
	RESOURCE_DB_CLASS(UBspNodes,FBspNode,RES_BspNodes)
	//
	enum {MAX_ZONES=64};				// Maximum zones in a Bsp, limited by QWORD bitmask size
	//
	// Variables:
	//
	DWORD			NumZones;			// Number of rendering zones
	DWORD			NumUniquePlanes;	// Number of unique planes in world
	DWORD			Unused1;
	DWORD			Unused2;
	FZoneProperties	Zones[MAX_ZONES];	// Properties of each zone
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	};

/*-----------------------------------------------------------------------------
	UBspSurf
-----------------------------------------------------------------------------*/

//
// One Bsp polygon.  Lists all of the properties associated with the
// polygon's plane.  Does not include a point list; the actual points
// are stored along with Bsp nodes, since several nodes which lie in the
// same plane may reference the same poly.
//
class FBspSurf
	{
	public:
	//
	// Persistent info:
	//
	UTexture	*Texture;	// 4 Texture map
	UModel		*Brush;		// 4 Editor brush
	DWORD		PolyFlags;  // 4 Polygon flags
	INDEX		pBase;      // 2 Polygon & texture base point index (where U,V==0,0)
	INDEX		vNormal;    // 2 Index to polygon normal
	INDEX		vTextureU;  // 2 Texture U-vector index
	INDEX		vTextureV;  // 2 Texture V-vector index
	INDEX		iLightMesh;	// 2 Light mesh
	INDEX		iBrushPoly; // 2 Editor brush polygon index
	BYTE		PanU;		// 1 U-Panning value
	BYTE		PanV;		// 1 V-Panning value
	INDEX		iActor;		// 2 Moving brush actor owning this Bsp surface, if any
	//
	// Valid in memory only (PreSave/PostLoad):
	//
	SWORD		LastStartY;	// Last span buffer starting Y value or 0
	SWORD		LastEndY;	// Last span buffer ending Y value or 0
	};

//
// Flags describing effects and properties of a Bsp polygon.
//
enum EPolyFlags
	{
	//
	// Regular in-game flags:
	//
	PF_Invisible		= 0x00000001,	// Poly is invisible
	PF_Masked			= 0x00000002,	// Poly should be drawn masked
	PF_Transparent	 	= 0x00000004,	// Poly is transparent
	PF_NotSolid			= 0x00000008,	// Poly is not solid, doesn't block
	PF_Environment   	= 0x00000010,	// Poly should be drawn environment mapped
	PF_Semisolid	  	= 0x00000020,	// Poly is semi-solid = collision solid, Csg nonsolid
	PF_Hurt 			= 0x00000040,	// Floor hurts player
	PF_FakeBackdrop		= 0x00000080,	// Poly looks exactly like backdrop
	PF_TwoSided			= 0x00000100,	// Poly is visible from both sides
	PF_AutoUPan		 	= 0x00000200,	// Automatically pans in U direction
	PF_AutoVPan 		= 0x00000400,	// Automatically pans in V direction
	PF_NoSmooth			= 0x00000800,	// Don't smooth textures
	PF_BigWavy 			= 0x00001000,	// Poly has a big wavy pattern in it
	PF_SmallWavy		= 0x00002000,	// Small wavy pattern (for water/enviro reflection)
	PF_Ghost			= 0x00004000,	// Poly is ghost (transparent nocolorized) 1-sided
	PF_LowShadowDetail	= 0x00008000,	// Low detaul shadows
	PF_NoMerge			= 0x00010000,	// Don't merge poly's nodes before lighting when rendering
	PF_FarCeiling		= 0x00020000,	// Polygon appears to be really far away, parallaxing
	PF_DirtyShadows		= 0x00040000,	// Dirty shadows
	PF_HighLedge		= 0x00080000,	// High ledge, blocks player
	PF_SpecialLit		= 0x00100000,	// Only speciallit lights apply to this poly
	PF_Glow				= 0x00200000,	// Glows
	PF_Unlit			= 0x00400000,	// Unlit
	PF_HighShadowDetail	= 0x00800000,	// High detail shadows
	PF_Portal			= 0x04000000,	// Portal between iZones
	PF_NoLook			= 0x08000000,	// Player shouldn't automatically look up/down these surfaces
	//
	// Editor:
	//
	PF_Memorized     	= 0x01000000,	// Editor: Poly is remembered
	PF_Selected      	= 0x02000000,	// Editor: Poly is selected
	PF_IsFront     		= 0x10000000,	// Filter operation bounding-sphere precomputed and guaranteed to be front
	PF_IsBack      		= 0x20000000,	// Guaranteed back
	PF_InternalUnused1	= 0x40000000,	// FPoly has been split by SplitPolyWithPlane      
	PF_DynamicLight		= 0x80000000,	// Polygon is dynamically lit
	//
	// FPoly flags:
	//
	PF_EdProcessed 		= 0x40000000,	// FPoly was already processed in editorBuildFPolys
	PF_EdCut       		= 0x80000000,	// FPoly has been split by SplitPolyWithPlane      
	//
	// Combinations of flags:
	//
	PF_NoOcclude		= PF_Masked | PF_Transparent | PF_Ghost | PF_Invisible | PF_Glow, // Doesn't obscure stuff beneath it
	PF_NoEdit			= PF_Memorized | PF_Selected | PF_IsFront | PF_IsBack | PF_EdProcessed | PF_EdCut, // Can't change these flags in UnrealEd
	PF_NoImport			= PF_NoEdit | PF_NoMerge,
	PF_NoShadows		= PF_Unlit | PF_Invisible | PF_Portal,
	PF_AddLast			= PF_Semisolid | PF_NotSolid,
	PF_NoAddToBSP		= PF_EdCut | PF_EdProcessed | PF_Selected | PF_Memorized | PF_IsFront | PF_IsBack,
	};

//
// A database of Bsp polygons associated with a model.
//
class UNREAL_API UBspSurfs : public UDatabase
	{
	RESOURCE_DB_CLASS(UBspSurfs,FBspSurf,RES_BspSurfs)
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void QueryDataReferences	(FResourceCallback &Callback);
	};

/*-----------------------------------------------------------------------------
	UBounds
-----------------------------------------------------------------------------*/

//
// A bonuding volume resource is a database of bounding box associated with
// node in a model's Bsp.
//
class UNREAL_API UBounds : public UDatabase
	{
	RESOURCE_DB_CLASS(UBounds,FBoundingVolume,RES_Bounds)
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	};

/*-----------------------------------------------------------------------------
	UTerrain
-----------------------------------------------------------------------------*/

//
// Information about a particular chunk terrain residing in an axis-aligned
// front leaf of the Bsp.
//
class FTerrainIndex
	{
	//
	// Variables:
	//
	INT GridX0,GridX1; // 256 grid-aligned bounds of this terrain area (0-256)
	INT GridY0,GridY1;
	};

//
// Terrain information associated with a level.
//
class UNREAL_API UTerrain : public UDatabase
	{
	RESOURCE_DB_CLASS(UTerrain,FTerrainIndex,RES_Terrain)
	//
	enum {MAX_TERRAIN_LAYERS  = 8};		// Maximum layers of terrain that may be stacked
	enum {MAX_TILE_REFERENCES = 128};	// Maximum number of tile textures that may be referenced
	//
	// Variables:
	//
	DWORD    LayerFlags  [MAX_TERRAIN_LAYERS];
	UTexture *HeightMaps [MAX_TERRAIN_LAYERS];
	UTexture *TileMaps   [MAX_TERRAIN_LAYERS];
	UTexture *TileRefs	 [MAX_TILE_REFERENCES];
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	};

/*-----------------------------------------------------------------------------
	ULightMesh
-----------------------------------------------------------------------------*/

//
// Describes the mesh-based lighting applied to a Bsp poly.
//
class FLightMeshIndex
	{
	public:
	//
	enum {MAX_POLY_LIGHTS=16};	// Number of lights that may be applied to a single poly
	//
	INT		DataOffset;			// Data offset for first shadow mesh
	INT		TextureUStart;  	// Minimum texture U coordinate that is visible
	INT		TextureVStart;  	// Minimum texture U coordinate that is visible
	WORD    MeshUSize;   	  	// Lighting mesh U-size = (USize+15)/16
	WORD    MeshVSize;   	  	// Lighting mesh V-size = (VSize+15)/16
	WORD	MeshSpacing;    	// Mesh spacing, usually 32
	BYTE	MeshShift;		  	// Mesh shifting value, LogTwo(MeshSpacing)
	BYTE	Unused;				// Unused
	INDEX	NumShadowLights;	// Number of lights that cast shadows
	INDEX	NumStaticLights;	// Number of static, shadowing lights attached
	INDEX	NumDynamicLights;	// Number of dynamic, non-shadowing lights attached
	WORD	iLightActor[MAX_POLY_LIGHTS]; // Actors controlling the lights
	//
	// What the numbers mean:
	//
	// * The Mesh U and V sizes are always rounded up to powers of 2 for quadtree
	//   compression.
	//
	// * The mesh origin (0,0) corresponds to the lowest visible Texture U and V values.
	//   When applying lighting to the texture Mesh starts at 0,0 and Texture U and V
	//   start at TextureUStart, TextureVStart.
	//
	// * The mesh axes are fixed on the texture's axes.  A 1x1 mesh unit corresponds to
	//   a MeshUDensity x MeshVDensity square in texture space (usually 16x16).  Lighting is
	//   applied to textures in texture space.
	//
	// * Textures can tile on any power-of-two grid.  Lighting meshes do not tile.
	//
	// * TextureUStart (and V) can be computed with or without tiling.
	//   MeshUSize (and V) must be computed based on the actual size, before
	//   tiling.
	//
	// * If you distort a texture-mapped polygon in the world, the lighting mesh
	//   is distorted exactly the same (stretched, flipped, enlarged, reduced).
	//   This means you can put really big textures in the world with high stretching
	//   and the lighting meshes will be relatively small.  Or, use a reduced texture
	//   for more accurate lighting.
	//
	};

//
// A light mesh resource associated with a level.
//
class UNREAL_API ULightMesh : public UResource
	{
	RESOURCE_CLASS(ULightMesh,FLightMeshIndex,RES_LightMesh)
	//
	// Variables:
	//
	INT	NumIndices;
	INT NumDataBytes;
	//
	// Resource functions:
	//
	void Register		(FResourceType *Type);
	void InitHeader		(void);
	void InitData		(void);
	int  QuerySize		(void);
	int  QueryMinSize	(void);
	};

/*-----------------------------------------------------------------------------
	UPolys
-----------------------------------------------------------------------------*/

//
// Results from FPoly.SplitWithPlane, describing the result of splitting
// an arbitrary FPoly with an arbitrary plane.
//
enum ESplitType
	{
	SP_Coplanar		= 0, // Poly wasn't split, but is coplanar with plane
	SP_Front		= 1, // Poly wasn't split, but is entirely in front of plane
	SP_Back			= 2, // Poly wasn't split, but is entirely in back of plane
	SP_Split		= 3, // Poly was split into two new editor polygons
	};

//
// A general-purpose polygon used by the editor.  An FPoly is a free-standing
// class which exists independently of any particular level, unlike the polys
// associated with Bsp nodes which rely on scads of other resources.  FPolys are
// used in UnrealEd for internal work, such as building the Bsp and performing
// boolean operations.
//
class UNREAL_API FPoly
	{
	public:
	//
	enum {MAX_FPOLY_VERTICES=16}; // Maximum vertices an EdPoly may have
	enum {FPOLY_VERTEX_THRESHOLD=MAX_FPOLY_VERTICES-2}; // Threshold for splitting into two
	//
	FVector     Base;        	// Base point of polygon
	FVector     Normal;			// Normal of polygon
	FVector     TextureU;		// Texture U vector
	FVector     TextureV;		// Texture V vector
	FVector     Vertex[MAX_FPOLY_VERTICES];  // Actual vertices
	DWORD       PolyFlags;		// FPoly & Bsp poly bit flags (PF_)
	UModel		*Brush;			// Brush where this originated, or NULL
	UTexture	*Texture;		// Texture map
	FName		GroupName;		// Group name
	FName		ItemName;		// Item name
	INDEX       NumVertices;	// Number of vertices
	INDEX		iLink;			// iBspSurf, or brush fpoly index of first identical polygon, or MAXWORD
	INDEX		iBrushPoly;		// Index of editor solid's polygon this originated from
	BYTE		PanU,PanV;		// Texture panning values
	//
	// Custom functions:
	//
	void  Init				(void);
	void  Flip				(void);
	void  SplitInHalf		(FPoly *OtherHalf);
	void  Transform			(FModelCoords &Coords, FVector *PreSubtract,FVector *PostAdd, FLOAT Orientation);
	int   Fix				(void);
	int   CalcNormal		(void);
	int   SplitWithPlane	(const FVector	&Base,const FVector &Normal,FPoly *FrontPoly,FPoly *BackPoly,int VeryPrecise);
	int   SplitWithPlaneFast(const FVector	&Base,const FVector &Normal,FPoly *FrontPoly,FPoly *BackPoly);
	int   RemoveColinears	(void);
	int   Finalize			(int NoError);
	FLOAT Area				(void);
	};

class UNREAL_API UPolys : public UDatabase
	{
	RESOURCE_DB_CLASS(UPolys,FPoly,RES_Polys)
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	void QueryDataReferences	(FResourceCallback &Callback);
	//
	// Custom functions:
	//
	int ParseFPolys(const char **Stream,int More, int CmdLine);
	int	CountFPolys(const char *Stream);
	};

/*-----------------------------------------------------------------------------
	UVectors
-----------------------------------------------------------------------------*/

//
// A table of floating point vectors.  Used within levels to store all points
// and vectors in the world.
//
class UNREAL_API UVectors : public UDatabase
	{
	RESOURCE_DB_CLASS(UVectors,FVector,RES_Vectors)
	//
	// Resource functions:
	//
	public:
	void Register				(FResourceType *Type);
	};

/*-----------------------------------------------------------------------------
	UVertPool
-----------------------------------------------------------------------------*/

//
// One vertex associated with a Bsp node's polygon.  Contains a vertex index
// into the level's FPoints table, and a unique number which is common to all
// other sides in the level which are cospatial with this side.
//
class FVertPool
	{
	public:
	INDEX 	pVertex;	// Index of vertex
	INDEX	iSide;		// If shared, index of unique side. Otherwise INDEX_NONE.
	};

//
// Vertex pool resource, containing all point lists referenced by Bsp polygons,
// as well as connectivity information linking sides of adjacent polys.
//
class UNREAL_API UVertPool : public UDatabase
	{
	RESOURCE_DB_CLASS(UVertPool,FVertPool,RES_VertPool)
	//
	// Variables:
	//
	INT NumSharedSides; // Number of unique iSideIndex's
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	void InitData				(void);
	};

/*-----------------------------------------------------------------------------
	UModel
-----------------------------------------------------------------------------*/

//
// Flags associated with models.
//
enum EModelFlags
	{
	MF_Selected			= 0x00000001,	// Brush is selected
	MF_Temp				= 0x00000002,	// Temporary work flag (editor)
	MF_InvalidBsp		= 0x00000004,	// Model's Bsp has been invalidated by map operations
	MF_ShouldSnap		= 0x00000008,	// Editor: Should grid snap this world brush
	MF_Color			= 0x00000010,	// Color contains a valid brush color
	MF_Linked			= 0x00000020,	// Model's iLinks have been set
	MF_PostScale		= 0x00000040,	// Model's post-scale is valid
	//
	MF_NOIMPORT			= MF_Selected | MF_Temp | MF_ShouldSnap, // Clear these flags upon importing a map
	};

//
// A Csg brush operation.
// Must agree with definitions in editorGetCsgName (unedcsg.cpp)
//
enum ECsgOper
	{
	CSG_Active      	= 0,	// Active brush (always brush 0), no effect on world
	CSG_Add         	= 1, 	// Add brush to world
	CSG_Subtract    	= 2, 	// Subtract brush from world
	CSG_Intersect   	= 3, 	// Rebuild brush from brush/world intersection
	CSG_Deintersect 	= 4, 	// Rebuild brush from brush/inverse world intersection
	CSG_Cutaway			= 5,	// Cutaway region for overhead view
	CSG_NoTerrain		= 6,	// Add no-terrain zone, cut into world before everything else
	CSG_NoCut       	= 7, 	// Don't cut anything within brush when building Bsp
	CSG_PlaceHolder 	= 9, 	// Add to level brush list but don't affect geometry
	};

//
// Model resources are used for: The level map and the editor solid.
//
class UNREAL_API UModel : public UResource
	{
	RESOURCE_CLASS(UModel,BYTE,RES_Model) // Model resource header
	//
	// References:
	//
	UVectors	*Vectors;		// Floating point vector table
	UVectors	*Points;		// Floating point point table
	UBspNodes	*BspNodes;		// Bsp Node list
	UBspSurfs	*BspSurfs;		// Bsp Polygon list
	UVertPool	*VertPool;		// Bsp vertex pool
	UPolys		*Polys;			// Editor polys
	ULightMesh  *LightMesh;		// Bsp poly light mesh
	UBounds		*Bounds;		// Bsp node bounding volumes
	UTerrain	*Terrain;		// Terrain information
	//
	// The following are only used by brush models, not by level models:
	//
	FVector		Location;		// Location of origin within level
	FRotation	Rotation;       // Rotation about (0,0,0)
	FVector		PrePivot;       // Pivot point for editing
	FScale		Scale;			// Scaling and sheering
	//
	ECsgOper	CsgOper; 		// Csg operation (CSG_Add,etc)
	DWORD		LockType;		// LOCK_TYPE (None, read, write, notrans)
	DWORD		Color;			// Base color of brush (0-7) if MF_Color is set.
	DWORD		PolyFlags;		// Poly flags for Csg operation
	DWORD		ModelFlags;		// Model bit flags
	FVector		PostPivot;		// Post-rotation pivot
	FScale		PostScale;		// Post-rotation scaling
	FScale		TempScale;		// Working scale value during MF_Scaling
	FBoundingVolume Bound[2];	// World bounding volumes, 0=untransformed, 1=transformed
	//
	// For expansion:
	//
	BYTE		Pad[32];		// Reserved, must be 0
	//
	// UModel resource functions:
	//
	void Register				(FResourceType *Type);
	void InitHeader				(void);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	char *Export				(char *Buffer,const char *FileType,int Indent);
	int  QuerySize				(void);
	int  QueryMinSize			(void);
	void QueryHeaderReferences	(FResourceCallback &Callback);
	//
	// Functions
	//
	UModel				(int Editable);
	void	Lock		(class IModel *ModelInfo,int LockType);
	void	Unlock		(class IModel *ModelInfo);
	void	Kill		(void);
	void	Init		(int InitPositionRotScale);
	FLOAT	BuildCoords	(FModelCoords *Coords,FModelCoords *Uncoords);
	void	BuildBound	(int Transformed);
	void	Transform	(void);
	void	SetPivotPoint(FVector *PivotLocation,int SnapPivotToGrid);
	void	CopyPosRotScaleFrom(UModel *OtherModel);
	};

//
// Information structure for a locked model.  This contains essential
// the same information as the UModel resource, plus extra members
// to make it easier to access information quickly.
//
class UNREAL_API IModel
	{
	public:
	UModel		*Model;
	ULightMesh  *LightMesh;
	UBspNodes	*BspNodesResource;
	UBspSurfs	*BspSurfsResource;
	UPolys		*PolysResource;
	UBounds		*BoundsResource;
	UTerrain	*TerrainResource;
	//
	INT			NumVectors,  	MaxVectors;
	INT			NumPoints,   	MaxPoints;
	INT			NumBspNodes, 	MaxBspNodes;
	INT			NumBspSurfs, 	MaxBspSurfs;
	INT			NumFPolys,  	MaxFPolys;
	INT			NumBounds,		MaxBounds;
	INT			NumVertPool,	MaxVertPool;
	INT			NumTerrain,		MaxTerrain;
	INT			NumSharedSides;
	INT			NumZones;
	INT			NumUniquePlanes;
	//
	FVector		*FVectors;
	FVector		*FPoints;
	FPoly		*FPolys;
	FBspNode	*BspNodes;
	FBspSurf	*BspSurfs;
	FVertPool	*VertPool;
	FBoundingVolume *Bounds;
	FTerrainIndex *TerrainIndex;
	//
	FVector		Location;
	FRotation	Rotation;
	FVector		PrePivot,PostPivot;
	FScale		Scale,PostScale,TempScale;
	ECsgOper	CsgOper;
	DWORD		PolyFlags;
	DWORD		ModelFlags;
	INT			Trans;
	//
	// General functions (UnModel.cpp):
	//
	void    Empty		(int EmptyPolyInfo);
	//
	// Physics functions (UnPhys.cpp):
	//
	typedef void (*PLANE_FILTER_CALLBACK)(IModel *ModelInfo, INDEX iNode, int Param);
	typedef void (*SPHERE_FILTER_CALLBACK)(IModel *ModelInfo, INDEX iNode, int IsBack, int Outside, int Param);
	typedef int  (*RAYTRACE_CALLBACK)(IModel *ModelInfo, INDEX iLeafNode, int IsBack, int Param);
	//
	int		PointClass   		(const FVector *Location, INDEX *iDestNode) const;
	BYTE	PointZone   		(const FVector *Location)  const;
	int		LineClass    		(const FVector *V1, const FVector *V2) const;
	int		SphereClass  		(const FVector *Location, FLOAT Radius)  const;
	int		SphereTestMove  	(const FVector *Location, const FVector *Delta, FVector *ResultAdjustment, FLOAT Radius, int FavorSmoothness) const;
	int		SphereNearMove  	(FVector *Location, const FVector *Delta, FLOAT Radius, int FavorSmoothness) const;
	int		SphereMove  		(FVector *Location, const FVector *Delta, FLOAT Radius, int FavorSmoothness) const;
	int		SphereReflect		(FVector *Location,FLOAT Radius, FVector *ReflectionVector) const;
	INDEX 	ZCollision 			(const FVector *V, FVector *Hit, INDEX *iActorHit) const;
	FLOAT	FindNearestVertex	(const FVector *SourcePoint, FVector *DestPoint, FLOAT MinRadius, INDEX *pVertex) const;
	void	PlaneFilter			(const FVector *Location,FLOAT Radius, PLANE_FILTER_CALLBACK Callback, DWORD SkipNodeFlags,int Param);
	void	SphereLeafFilter	(const FVector *Location,FLOAT Radius, SPHERE_FILTER_CALLBACK Callback, DWORD SkipNodeFlags,int Param);
	FLOAT	FuzzyLineClass		(const FVector *V1, const FVector *V2, FLOAT Radius) const;
	FLOAT	FuzzyPointClass		(const FVector *Location, FLOAT Radius) const;
	FLOAT	SoundDamping		(const FVector *Listener, const FVector *Emitter) const;
	int		Raytrace			(const FVector *V1, const FVector *V2, FLOAT ConeRadiusFactor,
								 RAYTRACE_CALLBACK Callback, int Param,
								 FVector *HitLocation, FVector *HitNormal, INDEX *HitNode);
	void	PrecomputeSphereFilter(FVector *Sphere);
	};

/*-----------------------------------------------------------------------------
	UPalette
-----------------------------------------------------------------------------*/

//
// A 24-bit color value
//
class UNREAL_API FColor
	{
	public:
	union
		{
		struct						// Separate RGB components for easy access
			{
			BYTE	Red;
			BYTE	Green;
			BYTE	Blue;
			union
				{
				BYTE Flags;			// Can hold windows palette flags
				BYTE RemapIndex;	// In texture palettes, holds index of closest color match in GGfx.Default
				};
			};
		DWORD D;					// DWORD value for easy access
		};
	inline int Brightness(void)		// Greyscale 0-255 brightness
		{
		return (3*(int)Red + 3*(int)Green + 2*(int)Blue)>>3;
		};
	inline DWORD TrueColor(void)
		{
		return ((D&0xff)<<16) + (D&0xff00) + ((D&0xff0000)>>16);
		};
	inline WORD HiColor565(void)
		{
		return ((D&0xf8) << 8) + ((D&0xfC00) >> 5) + ((D&0xf80000) >> 19);
		};
	inline WORD HiColor555(void)
		{
		return ((D&0xf8) << 7) + ((D&0xf800) >> 6) + ((D&0xf80000) >> 19);
		};
	inline FVector Vector(void)
		{
		FVector Temp;
		Temp.R = Red;
		Temp.G = Green;
		Temp.B = Blue;
		return Temp;
		};
	inline void Set(FVector &Vector)
		{
		ftob(Red,  OurClamp(Vector.R,(FLOAT)0.0,(FLOAT)255.0));
		ftob(Green,OurClamp(Vector.G,(FLOAT)0.0,(FLOAT)255.0));
		ftob(Blue, OurClamp(Vector.B,(FLOAT)0.0,(FLOAT)255.0));
		};
	inline void SetFast(FVector &Vector)
		{
		ftob(Red,   Vector.R);
		ftob(Green, Vector.G);
		ftob(Blue,  Vector.B);
		};
	};

enum
	{
	EHiColor565_RedMask		= 0xf800,
	EHiColor565_GreenMask	= 0x07e0,
	EHiColor565_BlueMask	= 0x001f,
	//
	EHiColor555_RedMask		= 0x7c00,
	EHiColor555_GreenMask	= 0x03e0,
	EHiColor555_BlueMask	= 0x001f,
	//
	ETrueColor_RedMask		= 0x00ff0000,
	ETrueColor_GreenMask	= 0x0000ff00,
	ETrueColor_BlueMask		= 0x000000ff,
	};

//
// Precomputed palette information:
// - Remap index for closest remap to game's main palette
// - Table of several nearest matching color for fast diffusion dithering
//
class FPalettePrecalc
	{
	public:
	enum {NUM_NEAREST=12};
	BYTE	RemapIndex;
	BYTE	Brightness;
	BYTE	Pad[2];
	BYTE	Nearest[NUM_NEAREST];
	};

//
// A palette resource.  Holds 256 unique FColor values, forming a 256-color palette
// which can be referenced by textures.
//
class UNREAL_API UPalette : public UDatabase
	{
	RESOURCE_DB_CLASS(UPalette,FColor,RES_Palette)
	//
	enum{NUM_PAL_COLORS=256};	// Number of colors in a standard palette
	//
	// Resource functions:
	//
	void Register				(FResourceType *Type);
	void InitData				(void);
	int  QuerySize				(void);
	int  QueryMinSize			(void);
	const char *Import			(const char *Buffer, const char *BufferEnd,const char *FileType);
	//
	// Custom functions:
	//
	BYTE BestMatch (FColor Color,int SystemPalette=1);
	BYTE BestMatchInRange(FColor MatchColor, BYTE RangeColor);
	UPalette *ReplaceWithExisting(void);
	void BuildPaletteRemapIndex(int Masked);
	void BuildBrightnessTable(void);
	void Smooth(void);
	};

/*-----------------------------------------------------------------------------
	Textures
-----------------------------------------------------------------------------*/

enum{MAX_MIPS=8};

//
// Information about a locked texture. Used for ease of rendering.
//
class ITexture
	{
	public:
	//
	// Standard data members for a locked texture:
	//
	UTexture *Texture;			// The texture that was locked
	UPalette *Palette;			// The texture's palette
	//
	int		Mip;				// Mipmapping level
	int		USize;				// Width of texture
	int		VSize;				// Height of texture
	BYTE	*Data;				// Texture data for mip level 0
	BYTE	*MipData[MAX_MIPS];	// Texture data for all mip levels
	FColor	*Colors;			// Regular palette
	DWORD	*Palette32;			// Truecolor palette if Camera->ColorBytes==4
	WORD	*Palette16;			// Highcolor palette if Camera->ColorBytes==2
	//
	BYTE	Platform[256];		// Space reserved for platform-specific locked texture info
	};

//
// Flags that can be specified when locking textures:
//
enum ETextureLockFlags
	{
	TL_Normal			= 0,	// No flags
	TL_Remapped			= 1,	// Return remapped texture data if in 256-color mode
	TL_Renderable		= 2,	// Return in a hardware renderable form
	};

extern void DiffusionDither(BYTE *Src, BYTE *Dest, UPalette *Palette,int USize,int VSize,int FixDiffusionC,int Masked);

//
// A 256-color, BYTE-based texture map.  Texture maps reference an optional palette,
// and can either be a simple rectangular array of pixels, or a hierarchy of mipmaps
// where each successive mipmap is half the dimensions of the mipmap above it.  Paletted
// textures must be remapped into the game's palette during play.
//
// Must parallel TextureRes class defined in Classes/Engine.tcx.
//
class UNREAL_API UTexture : public UResource
	{
	RESOURCE_CLASS(UTexture,BYTE,RES_Texture)
	//
	// Variables:
	//
	UClass		*Class;				// Must be GClasses.TextureRes
	UPalette	*Palette;			// Custom texture palette, NULL = use default palette
	UTexture	*Microtexture;		// Microtexture for up-close view, NULL = none
	UBuffer		*FireParams;		// FireEngine parameters if this is a fire texture
	FName		FamilyName;			// Name of texture family
	FName		UnusedName;			// Unused
	//
	// Editable parameters:
	//
	FLOAT		PalDiffusionC;		// For smooth remapping to the game palette 0.0 (none) - 1.0 (full)
	FLOAT		DiffuseC;			// Diffuse lighting coefficient (0.0-1.0)
	FLOAT		SpecularC;			// Specular lighting coefficient (0.0-1.0)
	FLOAT		FrictionC;			// Surface friction coefficient, 1.0=none, 0.95=some
	//
	USound		*FootstepSound;		// Footstep sound
	USound		*HitSound;			// Sound when the texture is hit with a projectile
	DWORD		PolyFlags{ 0 };		// Polygon flags to be applied to Bsp polys with texture (See PF_*)
	DWORD		bNoTile			:1;	// Texture is a non-power-of-two size and thus doesn't tile
	//
	DWORD		LockCount;			// Number of locks that are active on the texture
	DWORD		CameraCaps;			// If a camera, its caps flags, otherwise 0
	DWORD		Pad3;				// Reserved
	DWORD		Pad4;				// Reserved
	DWORD		Pad5;				// Reserved
	DWORD		Pad6;				// Reserved
	DWORD		Pad7;				// Reserved
	DWORD		Pad8;				// Reserved
	//
	// Internal information:
	//
	INT			USize;				// Width, must be power of 2.
	INT			VSize;				// Height, must be power of 2.
	INT			DataSize;			// Size of texture data (texture + mip sizes)
	INT			ColorBytes;			// Bytes per texel, must be 1 for regular textures
	FColor		MipZero;			// Overall average color of texture
	DWORD		MipOfs[MAX_MIPS];	// Offset of mipmaps into texture's data, MAXDWORD=not avail
	//
	// Misc info:
	//
	BYTE		UBits;				// # of bits in USize, i.e. 8 for 256
	BYTE		VBits;				// # of bits in VSize
	BYTE		Unused1;			// Obsolete
	BYTE		Unused2;			// Obsolete
	//
	// Reserved for expansion:
	//
	BYTE		Pad[32];			// Must be 0
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
	//
	// Custom functions:
	//
	public:
	//
	void Lock(ITexture *TextureInfo,ICamera *Camera,int Mip,int Flags); // Flags are ETextureLockFlags
	void Unlock(ITexture *TextureInfo);
	//
	void Remap (UPalette *SourcePalette, UPalette *DestPalette);
	void BuildPaletteRemapIndex(int Masked);
	void CreateMips(int FullMips);
	void Fixup (void);
	//
	inline BYTE *GetOriginalData (int *MipLevel, int *MipUSize, int *MipVSize)
		{
		GUARD;
		BYTE *Data = GetData();
		//
		if (*MipLevel>7) *MipLevel=7;
		while (*MipLevel > 0) {if (MipOfs[*MipLevel]!=MAXDWORD) break; (*MipLevel)--;};
		//
		*MipUSize = USize >> *MipLevel;
		*MipVSize = VSize >> *MipLevel;
		//
		#ifdef PARANOID
		if ((*MipUSize<=0)||(*MipVSize<=0)||(MipOfs[*MipLevel]==MAXDWORD)) appErrorf("Consistency error %i %s, %ix%i: Error",*MipLevel,Name,USize,VSize);
		#endif
		//
		return &Data[MipOfs[*MipLevel]];
		UNGUARD("UTexture::GetOriginalData");
		};
	inline BYTE *GetData(int *MipLevel,int ColorBytes,int *USize,int *VSize)
		{
		GUARD;
		BYTE *Src = GetOriginalData (MipLevel,USize,VSize);
		if (ColorBytes==1)
			{
			int CacheID = ((int)Index << 16) + *MipLevel + 0x100;
			BYTE *Dest  = GCache.Get(CacheID);
			if (!Dest)
				{
				Dest = GCache.Create(CacheID,DataSize);
				DiffusionDither (Src,Dest,Palette, *USize, *VSize, (int)(PalDiffusionC*64.0),PolyFlags & PF_Masked);
				};
			return Dest;
			}
		else return Src;
		UNGUARD("UTexture::GetData");
		};
	private:
	void DoRemap (BYTE *Remap);
	void InitRemap(BYTE *Remap);
	};

/*----------------------------------------------------------------------------
	The End
----------------------------------------------------------------------------*/
#endif // _INC_UNRES

