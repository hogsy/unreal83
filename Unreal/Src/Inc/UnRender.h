/*=============================================================================
	UnRender.h: Rendering functions and structures

	Copyright 1995 Epic MegaGames, Inc.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNRENDER // Prevent header from being included multiple times
#define _INC_UNRENDER

/*------------------------------------------------------------------------------------
	Other includes
------------------------------------------------------------------------------------*/

#ifndef _INC_UNSPAN
#include "UnSpan.h"
#endif

/*------------------------------------------------------------------------------------
	Types for rendering engine
------------------------------------------------------------------------------------*/

//
// Point/vector transformation cache:
//
class FTransform : public FVector // Floating-point screen coordinates
{
public:
	FLOAT   ScreenX;	// Projected Screen X
	FLOAT	ScreenY;	// Projected Screen Y
	INT		IntX;		// Integer Screen X
	INT		IntY;		// Integer Screen Y
	DWORD	iSide;		// Side link (Bsp polys only)

	void inline ComputeOutcode(ICamera *Camera)
	{
		static BYTE OutZBackTab[2] = {0,FVF_OutZBack};
		static BYTE OutXMinTab [2] = {0,FVF_OutXMin };
		static BYTE OutXMaxTab [2] = {FVF_OutXMax,0 };
		static BYTE OutYMinTab [2] = {0,FVF_OutYMin };
		static BYTE OutYMaxTab [2] = {FVF_OutYMax,0 };

		#ifdef ASM /* 40 cycles */
		static DWORD XP,XM,YP,YM;
		__asm
		{
			mov		edi,[this]				; edi = this
			mov		esi,[Camera]			; esi = Camera
			;
			mov		ebx,[edi]FVector.Z		; ebx = Z
			;
			xor		edx,edx					; edx = 0
			add		ebx,0x80000000			; Set carry when Z<0
			;
			adc		edx,0					; edx = (Z<0) ? 1 : 0
			xor		al,al					; al = Flags = 0
			;
			fld		[edi]FVector.X			; st=X
			fld		[edi]FVector.Y			; st=Y,X
			fxch							; st=X,Y
			fmul	[esi]ICamera.ProjZRSX2	; st=X*ProjZRSX2,Y
			fxch							; st=Y,X*ProjZRSX2
			fmul	[esi]ICamera.ProjZRSY2	; st=Y*ProjZRSY2,X*ProjZRSX2
			;
			mov		cl,OutZBackTab[edx]		; cl = (Z<0) ? FVF_OutZBack : 0
			;
			or		al,cl					; Flags = Flags | ((Z<0) ? FVF_OutZBack : 0)
			;
			fld		st(0)					; st=Y*ProjZRSY2,Y*ProjZRSY2,X*ProjZRSX2
			fadd	[edi]FVector.Z			; st=Y*ProjZRSY2+Z,Y*ProjZRSY2,X*ProjZRSX2
			fxch							; st=Y*ProjZRSY2,Y*ProjZRSY2+Z,X*ProjZRSX2
			fsub	[edi]FVector.Z			; st=Y*ProjZRSY2-Z,Y*ProjZRSY2+Z,X*ProjZRSX2
			fld		st(2)					; st=X*ProjZRSX2,Y*ProjZRSY2-Z,Y*ProjZRSY2+Z,X*ProjZRSX2
			fadd	[edi]FVector.Z			; st=X*ProjZRSX2+Z,Y*ProjZRSY2-Z,Y*ProjZRSY2+Z,X*ProjZRSX2
			fxch	st(3)					; st=X*ProjZRSX2,Y*ProjZRSY2-Z,Y*ProjZRSY2+Z,X*ProjZRSX2+Z
			fsub	[edi]FVector.Z			; st=X*ProjZRSX2-Z,Y*ProjZRSY2-Z,Y*ProjZRSY2+Z,X*ProjZRSX2+Z
			fxch	st(2)					; st=Y*ProjZRSY2+Z,Y*ProjZRSY2-Z,X*ProjZRSX2-Z,X*ProjZRSX2+Z
			;
			fstp	[YP]					; st=Y*ProjZRSY2-Z,X*ProjZRSX2-Z,X*ProjZRSX2+Z
			fstp	[YM]					; st=X*ProjZRSX2-Z,X*ProjZRSX2+Z
			fstp	[XM]					; st=X*ProjZRSX2+Z
			fstp	[XP]					; st=(empty)
			;
			mov		ebx,[YP]				; ebx=Y*ProjZRSY2+Z
			mov		esi,[YM]				; esi=Y*ProjZRSY2-Z
			;
			xor		edx,edx					; edx=0
			add		ebx,0x80000000			; Set carry when Y*ProjZRSY2<-Z
			;
			adc		edx,0					; edx=Y*ProjZRSY2<-Z ? 1 : 0
			;
			xor		ecx,ecx					; ecx=0
			add		esi,0x80000000			; Set carry when Y*ProjZRSY2>Z
			;
			adc		ecx,0					; ecx=Y*ProjZRSY2>Z ? 1 : 0
			mov		bl,OutYMinTab[edx]		; bl=Y*ProjZRSY2<-Z ? FVF_OutYMin : 0
			;
			mov		dl,OutYMaxTab[ecx]		; dl=Y*ProjZRSY2>Z ? FVF_OutYMax : 0
			or		al,bl					; Update flags
			;
			or		al,dl					; Update flags
			;
			mov		ebx,[XP]				; ebx=X*ProjZRSX2+Z
			mov		esi,[XM]				; esi=X*ProjZRSX2-Z
			;
			xor		edx,edx					; edx=0
			add		ebx,0x80000000			; Set carry when X*ProjZRSX2<-Z
			;
			adc		edx,0					; edx=Y*ProjZRSY2<-Z ? 1 : 0
			;
			xor		ecx,ecx					; ecx=0
			add		esi,0x80000000			; Set carry when X*ProjZRSX2>Z
			;
			adc		ecx,0					; ecx=Y*ProjZRSY2>Z ? 1 : 0
			mov		bl,OutXMinTab[edx]		; bl=X*ProjZRSX2<-Z ? FVF_OutXMin : 0
			;
			mov		dl,OutXMaxTab[ecx]		; dl=X*ProjZRSX2>Z ? FVF_OutXMax : 0
			or		al,bl					; Update flags
			;
			or		al,dl					; Update flags
			;
			mov		[edi]FVector.Flags,al	; Store flags
		}
		#else
			FLOAT Temp;
			//
			Flags = 0;
			if (Z < 0.0) Flags = FVF_OutZBack;
			//
			Temp = X * Camera->ProjZRSX2;
			if 	(Temp < -Z) Flags |= FVF_OutXMin;
			if	(Temp > +Z) Flags |= FVF_OutXMax;
			//
			Temp = Y * Camera->ProjZRSY2;
			if 	(Temp < -Z) Flags |= FVF_OutYMin;
			if	(Temp > +Z) Flags |= FVF_OutYMax;
		#endif
	}
	FTransform inline operator+ (const FTransform &V) const
	{
		FTransform Temp;
		Temp.X = X + V.X; Temp.Y = Y + V.Y; Temp.Z = Z + V.Z;
		return Temp;
	}
	FTransform inline operator- (const FTransform &V) const
	{
		FTransform Temp;
		Temp.X = X - V.X; Temp.Y = Y - V.Y; Temp.Z = Z - V.Z;
		return Temp;
	}
	FTransform inline operator* (FLOAT Scale ) const
	{
		FTransform Temp;
		Temp.X = X * Scale; Temp.Y = Y * Scale; Temp.Z = Z * Scale;
		return Temp;
	}
};

//
// Point plus texture transformation.  First entries must be identical to
// FTransform because FTransTex records are often typecast to FTransform's.
//
class FTransTex : public FTransform // Transform plus texture coordinates
{
public:
	FLOAT		U;  // Texture U value at point * 65536.0
	FLOAT		V;  // Texture V value at point * 65536.0
	FLOAT		G;  // Gouraud value at point   * 65536.0

	FTransTex inline operator+ (const FTransTex &T) const
	{
		FTransTex Temp;
		Temp.X = X + T.X; Temp.Y = Y + T.Y; Temp.Z = Z + T.Z;
		Temp.U = U + T.U; Temp.V = V + T.V; Temp.G = G + T.G;
		return Temp;
	}
	FTransTex inline operator- (const FTransTex &T) const
	{
		FTransTex Temp;
		Temp.X = X - T.X; Temp.Y = Y - T.Y; Temp.Z = Z - T.Z;
		Temp.U = U - T.U; Temp.V = V - T.V; Temp.G = G - T.G;
		return Temp;
	}
	FTransTex inline operator* (FLOAT Scale) const
	{
		FTransTex Temp;
		Temp.X = X * Scale; Temp.Y = Y * Scale; Temp.Z = Z * Scale;
		Temp.U = U * Scale; Temp.V = V * Scale; Temp.G = G * Scale;
		return Temp;
	}
};

/*------------------------------------------------------------------------------------
	Basic types
------------------------------------------------------------------------------------*/

//
// A packed MMX data structure
//
class FMMX
{
public:
	union
	{
		struct {SWORD  B,G,R,A;};		// RGB colors
		struct {SWORD  U1,V1,U2,V2;};	// Texture coordinates pair
		struct {INT    U,V;};			// Texture coordinates
	};
};

//
// Sample lighting and texture values at a lattice point on a rectangular grid
// 68 bytes
//
class FTexLattice // Blit globals - **WARNING** Mirrored in UnRender.inc
{
public:
	union
	{
		struct // 12 bytes
		{
			// 256-color texture and lighting info:
			BYTE Pad1[32];
			INT  U,V,G,Pad2;
			FVector Loc;
		};
		struct // 24 bytes
		{
			// MMX texture and lighting info:
			FMMX Tex;
			FMMX Color;
			FMMX Fog;
		};
		struct // 64 bytes
		{
			// Bilinear rectangle setup:
			DWORD L,  H;
			DWORD LY, HY;
			DWORD LX, HX;
			DWORD LXY,HXY;

			// Bilinear subrectangle setup:
			DWORD SubL,  SubH;
			DWORD SubLY, SubHY;
			DWORD SubLX, SubHX;
			DWORD SubLXY,SubHXY;
		};
		struct // 64 bytes
		{
			// Bilinear rectangle setup:
			QWORD Q;
			QWORD QY;
			QWORD QX;
			QWORD QXY;
			// Bilinear subrectangle setup:
			QWORD SubQ;
			QWORD SubQY;
			QWORD SubQX;
			QWORD SubQXY;
		};
	};
	DWORD RoutineOfs;
	DWORD AlignPad;
};

/*------------------------------------------------------------------------------------
	Dynamics rendering
------------------------------------------------------------------------------------*/

//
// Temporary sprite structure
//
class FSprite
{
public:
	EBlitType	BlitType;				// How to draw it
	int			iActor;					// Actor that generated this sprite
	int			USize,VSize;			// Texture size
	int			X1,Y1;					// Top left corner on screen
	int			X2,Y2;					// Bottom right corner, X2 must be >= X1, Y2 >= Y1.
	int			iFrame;					// Frame number to draw
	UTexture	*Texture;				// Source texture
	BYTE		*Buffer;				// Texture data to draw
	AActorDraw	*Actor;					// Actor draw info
	FLOAT 		ScreenX,ScreenY;		// Fractionally-accurate screen location of center
	FLOAT		Scale;					// Scaling value, 1.0 = 1 texel to 1 world unit
	FLOAT		Z;						// Screenspace Z location
	FTransform	Verts[4];				// Vertices of projection plane for filtering
};

/*------------------------------------------------------------------------------------
	Dynamic Bsp contents
------------------------------------------------------------------------------------*/

class FDynamicsIndex
{
public:
	int 				Type;		// Type of contents
	FSprite				*Sprite;	// Pointer to dynamic data, NULL=none
	FSpanBuffer			*Span;		// Pointer to dynamic saved span, NULL=none
	class FRasterPoly	*Raster;	// Rasterization this is clipped to, NULL=none
	FLOAT				Z;			// For sorting
	INDEX				iNode;		// Node the contents correspond to
	INDEX				iNext;		// Index of next dynamic contents in list or INDEX_NONE
};

enum EDynamicsType
{
	DY_NOTHING,			// Nothing
	DY_SPRITE,			// A sprite
	DY_CHUNK,			// A sprite chunk
	DY_FINALCHUNK,		// A non moving chunk
};

/*------------------------------------------------------------------------------------
	Dynamic node contents
------------------------------------------------------------------------------------*/

void dynamicsLock 		(IModel *ModelInfo);
void dynamicsUnlock		(IModel *ModelInfo);
void dynamicsSetup		(ICamera *Camera, INDEX iExclude);
void dynamicsFilter		(ICamera *Camera, INDEX iNode,int FilterDown,int Outside);
void dynamicsPreRender	(ICamera *Camera, FSpanBuffer *SpanBuffer, INDEX iNode,int IsBack);
void dynamicsFinalize	(ICamera *Camera, int SpanRender);

/*------------------------------------------------------------------------------------
	Bsp & Occlusion
------------------------------------------------------------------------------------*/

class FBspDrawList
{
	public:
	int					iNode,iSurf,iZone,PolyFlags,NumPts;
	FLOAT				MaxZ,MinZ;
	FSpanBuffer			Span;
	FTransform			*Pts;
	UTexture			*Texture;
};

/*------------------------------------------------------------------------------------
	Debugging stats
------------------------------------------------------------------------------------*/

//
// General-purpose statistics:
//
#ifdef STATS
//
// Macro to execute an optional statistics-related command:
//
#define STAT(cmd) cmd
//
// All stats:
//
class FRenderStats
{
public:

	// Bsp traversal:
	int NodesDone;			// Nodes traversed during rendering
	int NodesTotal;			// Total nodes in Bsp

	// Occlusion stats
	int NodesBehind;		// Nodes totally behind viewer
	int BoxChecks;			// Number of bounding boxes checked
	int BoxSkipped;			// Boxes skipped due to nodes being visible on the prev. frame
	int BoxBacks;			// Bsp node bounding boxes behind the player
	int BoxIn;				// Boxes the player is in
	int BoxOutOfPyramid;	// Boxes out of view pyramid
	int BoxSpanOccluded;	// Boxes occluded by span buffer

	// Actor drawing stats:
	int	NumSprites;			// Number of sprites filtered
	int	NumChunks;			// Number of final chunks filtered
	int	NumFinalChunks;		// Number of final chunks
	int	ChunksDrawn;		// Chunks drawn
	int MeshMapsDrawn;		// Mesh maps drawn

	// Texture subdivision stats
	int	LatsMade;			// Number of lattices generated
	int LatLightsCalc;		// Number of lattice light computations
	int	DynLightActors;		// Number of actors shining dynamic light

	// Polygon/rasterization stats
	int	NumSides;			// Number of sides rasterized
	int NumSidesCached;		// Number of sides whose setups were cached
	int	NumRasterPolys;		// Number of polys rasterized
	int NumRasterBoxReject;	// Number of polygons whose bounding boxes were span rejected

	// Poly drawing:
	int NodesDrawn;			// Number of Bsp nodes draw
	int PolysDrawn;			// Number of merged Bsp polys drawn
	int PolyLitesDrawn;		// Number of lights applied to polys

	// Span buffer:
	int SpanTotalChurn;		// Total spans added
	int SpanRejig;			// Number of span index that had to be reallocated during merging

	// Clipping:
	int ClipAccept;			// Polygons accepted by clipper
	int ClipOutcodeReject;	// Polygons outcode-rejected by clipped
	int ClipNil;			// Polygons clipped into oblivion

	// Memory:
	int GMem;				// Bytes used in global memory pool
	int GDynMem;			// Bytes used in dynamics memory pool

	// Zone rendering:
	int CurZone;			// Current zone the player is in
	int NumZones;			// Total zones in world
	int VisibleZones;		// Zones actually processed
	int MaskRejectZones;	// Zones that were mask rejected

	// Illumination cache:
	int IllumTime;			// Time spent in illumination
	int MeshGen;			// Mesh points generated this frame

	// Available:
	int Texelage,TexelMem,UniqueTextures;
	int	Extra1,MeshesGen,UniqueTextureMem,CodePatches;

	// Routine timings:
	int GetValidRange;
	int BoxIsVisible;
	int BoundIsVisible;
	int CopyFromRasterUpdate;
	int CopyFromRaster;
	int CopyIndexFrom;
	int CopyFromRange;
	int MergeWith;
	int MergeFrom;
	int CalcRectFrom;
	int CalcLatticeFrom;
	int Generate;
	int CalcLattice;
	int Tmap;
	int Asm;
	int Setup;
	int SetupCached;
	int Transform;
	int Clip;
	int TextureMap;

	// Timing:
	QWORD	LastEndTime;	// Time when last sample was ended
	QWORD	ThisStartTime;	// Time when this sample was started
	QWORD	ThisEndTime;	// Time when this sample was ended
};
extern FRenderStats GStat;
#else
	#define STAT(x) /* Do nothing */
#endif // STATS

/*------------------------------------------------------------------------------------
	Lighting
------------------------------------------------------------------------------------*/

/*
//
// Information about a lightsource, set up for fast access:
//
class FLightInfo
{
friend class FLightList;

public:
	// Variables
	AActorDraw			*Actor;			// All actor drawing info
	FVector				Location;		// Transformed screenspace location of light
	FVector				Reflection;		// Reflection about current polygon
	FLOAT				Radius;			// Maximum effective radius
	FLOAT				RRadius;		// 1.0 / Radius
	FLOAT				RRadiusMult;	// 16383.0 / (Radius * Radius)
	FLOAT				Brightness;		// Center brightness at this instance, 1.0=max, 0.0=none
	FLOAT				Brightness1024;	// Brightness * 1024.0
	FLOAT				R,G,B;			// Color if in truecolor mode
	FLOAT				LocationNormal;	// light location dot poly normal
	FLOAT				BaseNormal;		// base dot normal
	FLOAT				BaseNormalDelta;// (light location - poly base) dot poly normal
	FLOAT				SpecularDot;	// Precomputed specular lighting parameter
	FLOAT				ReflectionSize;	// Reflection.SizeSquared()
	FLOAT				Diffuse;		// BaseNormalDelta * RRadius
	FLOAT				VolRadius;		// 0.03 * LightInfo->Radius
	FLOAT				LocSizeSquared;	// Location.SizeSquared()
	//LIGHT_SIMPLE_FUNC   SimpleFxFunc;	// Simple effects function
	//LIGHT_SPATIAL_FUNC  SpatialFxFunc;	// Spatial effects function
	//LIGHT_LATTICE_FUNC  LatticeFxFunc;	// Light lattice effects function
	FLOAT*				MeshBase;		// Base address of this light's meshes (not accounting for mip offsets)

	FLOAT*				IlluminationMap;// Temporary illumination map pointer

protected:
	// Functions:
	void inline ComputeFromActor  (ICamera *Camera, INDEX iActor);
	void inline SetupRectSampling (FVector &Base, FVector &Normal);
};
*/

/*------------------------------------------------------------------------------------
	Lighting
------------------------------------------------------------------------------------*/

//
// Class encapsulating the dynamic lighting subsystem.
//
class FVirtualGlobalLightManager
{
public:
	// Functions:
	virtual void Init()=0;
	virtual void Exit()=0;
	virtual void Tick()=0;
	virtual void SetupForActor(ICamera *Camera, INDEX iActor)=0;
	virtual void SetupForPoly(ICamera *Camera, FVector &Normal, FVector &Base, INDEX iThisSurf)=0;
	virtual void SetupForNothing(ICamera *Camera)=0;
	virtual void ApplyLatticeEffects(FTexLattice* TopLattice, int Num)=0;
	virtual void ReleaseLightBlock()=0;
	virtual void DoDynamicLighting(ILevel *Level)=0;
	virtual void UndoDynamicLighting(ILevel *Level)=0;

	// Global variables:
	static FVector			Base,Normal,TextureU,TextureV;
	static FVector			InverseUAxis,InverseVAxis,InverseNAxis;
	static ICamera*			Camera;
	static ILevel*			Level;
	static IModel*			ModelInfo;
	static ULightMesh*		LightMesh;
	static FLightMeshIndex*	Index;
	static FBspSurf*		Surf;
	static BYTE*			ShadowBase;
	static VOID*			MemTop;
	static VOID*			MeshVoid;
	static QWORD			MeshAndMask;
	static DWORD			PolyFlags,MaxSize;
	static INT				MeshUSize,MeshVSize,MeshSpace;
	static INT				MeshUTile,MeshVTile,MeshTileSpace;
	static INT				MeshUByteSize,MeshByteSpace,MeshSkipSize,LatticeEffects;
	static INT				MinXBits,MaxXBits,MinYBits,MaxYBits;
	static INDEX			iLightMesh,iSurf;
	static BYTE				MeshUBits,MeshVBits;

	// Inlines:
	inline static FLOAT*& GetMeshFloat() { return (FLOAT *&)MeshVoid; };
};

/*------------------------------------------------------------------------------------
	Dithering
------------------------------------------------------------------------------------*/

//
// Dither offsets: Hand-tuned parameters for texture and illumination dithering.
//
class FDitherOffsets
{
	public:
	INT U[4][2];
	INT V[4][2];
	INT G[4][2];
};

class FDitherPair
{
	public:
	QWORD Delta;
	QWORD Offset;
};

//
// Dither table: Table generated by BuildDitherTable for 256-color
// texture mapper, including dither parameters for all allowable
// texture sizes.
//
class FDitherSet
{
public:
	FDitherPair Pair[8][4][2];
};
typedef FDitherSet FDitherTable[12];
extern  FDitherTable GDither256,GBlur256,GNoDither256;

void InitDither();
typedef void (*RASTER_DRAW_FUNC)(BYTE *Dest, QWORD Start, QWORD Inc, int Pixels);

/*------------------------------------------------------------------------------------
	Blit globals
------------------------------------------------------------------------------------*/
 
class UNRENDER_API FBlitMipInfo
{
public:
	DWORD		VMask;
	DWORD		AndMask;
	BYTE		*Data;
	FDitherSet	*Dither;

	BYTE		MipLevel;
	BYTE		UBits;
	BYTE		VBits;
	BYTE		Pad;
};

class FGlobalBlit // Blit globals - **WARNING** Mirrored in UnRender.inc
{
public:
	INT				LatticeX,LatticeY;			// Lattice grid size
	INT				SubX,SubY;					// Sublattice grid size
	INT				InterX,InterY;				// Interlattice size
	INT				LatticeXMask,LatticeXNotMask,LatticeXMask4;
	INT				InterXMask,InterXNotMask;

	BYTE			LatticeXBits,LatticeYBits;	// Lattice shifters, corresponding to LatticeX,LatticeY
	BYTE			SubXBits,SubYBits;			// Sublattice shifters, corresponding to LatticeX,LatticeY
	BYTE			InterXBits,InterYBits;		// LatticeXBits-SubXBits etc
	BYTE			UBits,VBits;

	INT				Line;

	UTexture		*Texture;
	BYTE			*TextureData;
	BYTE			*BlendTable;
	FDitherTable	*DitherBase;
	FBlitMipInfo	Mips[8];
	BYTE			MipRef[8+1],PrevMipRef[8+1];

	INT				BlendKind;					// See BLEND_ in UnGfx.h
	INT				DrawKind;					// See DRAW_RASTER_ above
	INT				iZone;						// Zone the blitting is occuring in

	// Functions:
	void Setup (ICamera *Camera, UTexture *Texture, DWORD PolyFlags, DWORD NotPolyFlags);
};
extern "C" extern FGlobalBlit GBlit;

/*------------------------------------------------------------------------------------
	FGlobalRender
------------------------------------------------------------------------------------*/

enum {MAX_XR = 256};		// Maximum subdivision lats horizontally
enum {MAX_YR = 256};		// Maximum subdivision lats vertically

//
// Renderer globals:
//
class UNRENDER_API FGlobalRender
{
public:

	// Public interface:
	enum {MAX_VISIBLE_NODES	= 8192};	// For span rendering

	virtual void	Init();
	virtual void	Exit();
	virtual int		Exec(const char *Cmd,FOutputDevice *Out=GApp);

	virtual void	PreRender(ICamera *Camera);
	virtual void	PostRender(ICamera *Camera);

	virtual void	DrawWorld(ICamera *Camera);
	virtual void	DrawActor(ICamera *Camera,INDEX iActor);
	virtual void	DrawScaledSprite(ICamera *Camera, UTexture *Texture, FLOAT ScreenX, FLOAT ScreenY, FLOAT XSize, FLOAT YSize, EBlitType BlitType,FSpanBuffer *SpanBuffer,int Center,int Highlight);
	virtual void	DrawTiledTextureBlock(ICamera *Camera,UTexture *Texture,int X, int XL, int Y, int YL,int U,int V);

	FBspDrawList	DrawList[MAX_VISIBLE_NODES];
	WORD			ByteMult[256][256];

	// Private implementation
	// ----------------------

	// Variables:
	int				MaxTransforms;
	int				*MeshStatPtr;

	INDEX			NumVectorTransforms,NumPointTransforms;
	INDEX			*VectorTransformList,*PointTransformList;
	FTransform		*PointTransform;
	FVector			*VectorTransform;

	INDEX			NumDynamics;//OLD!!!!!
	int				DynamicsLocked;
	INDEX			MaxDynamics;
	INDEX			NumPostDynamics;
	INDEX			MaxPostDynamics;
	FDynamicsIndex	*DynamicsIndex;
	INDEX			*PostDynamics;

	int				Toggle,RendIter,ShowLattice,Extra;
	int				Fog,DoDither,ShowChunks,Unused2;
	int				Extra1,Extra2,Extra3,Extra4,LeakCheck;
	int				QuickStats,AllStats;

	FTexLattice		*LatticePtr[MAX_YR][MAX_XR];

	class FRenderStats *Stat;

	enum EDrawRaster
	{
		DRAWRASTER_Normal			= 0,	// No effects
		DRAWRASTER_Masked			= 1,	// Masked
		DRAWRASTER_Blended			= 2,	// Blend table
		DRAWRASTER_Fire				= 3,	// Fire table
		DRAWRASTER_MAX				= 4,	// First invalid entry
	};
	RASTER_DRAW_FUNC Raster256Table[4];

	void InitTransforms		(IModel *ModelInfo);
	void ExitTransforms		();
	inline FTransform *GetPoint(IModel *ModelInfo, ICamera *Camera, INDEX pPoint);
	inline FVector *GetVector(IModel *ModelInfo, const FCoords *Coords, INDEX vVector);

	int  OccludeBsp			(ICamera *Camera, FSpanBuffer *Backdrop);

	void DrawBspSurf 		(ICamera *Camera, FBspDrawList *Draw);
	void DrawHighlight		(ICamera *Camera, FSpanBuffer *SpanBuffer, BYTE Color);

	void DrawFlatPoly		(ICamera *Camera,FSpanBuffer *SpanBuffer, BYTE Color);

	void DrawLatticeGrid	(ICamera *Camera,FSpanBuffer *LatticeSpan);
	void CleanupLattice		(FSpanBuffer &LatticeSpan);

	void DrawRasterOutline  (ICamera *Camera,class FRasterSetup *Raster, BYTE Color);
	void DrawRasterSide		(BYTE *Line,int SXR,class FRasterSideSetup *Side,BYTE Color);

	void DrawBackdrop		(ICamera *Camera, FSpanBuffer *Backdrop);
	void DrawLevelActors	(ICamera *Camera, INDEX iExclude);
	void DrawMovingBrushWires(ICamera *Camera);
	void DrawLevelBrushes	(ICamera *Camera);
	void DrawActiveBrush	(ICamera *Camera);
	void DrawWireBackground	(ICamera *Camera);
	void DrawBrushPolys		(ICamera *Camera, UModel *Model, int WireColor, int Dotted, FConstraints *Constraints, int DrawPivot, int DrawVertices, int DrawSelected, int DoScan);
	void DrawLine			(ICamera *Camera, BYTE Color,int Dotted,FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2);
	void DrawDepthLine		(ICamera *Camera, BYTE Color,int Dotted,FLOAT X1, FLOAT Y1, FLOAT RZ1, FLOAT X2, FLOAT Y2, FLOAT RZ2);
	void DrawFPoly 			(ICamera *Camera, FPoly *Poly, int WireColor, int FillColor,int Dotted);
	void DrawRect 			(ICamera *Camera, BYTE Color, int X1, int Y1, int X2, int Y2);
	void Draw3DLine 		(ICamera *Camera, const FVector *OrigP, const FVector *OrigQ, int MustTransform, int Color,int DepthShade,int Dotted);
	void DrawBoundingVolume (ICamera *Camera,FBoundingVolume *Bound);
	void DrawCircle			(ICamera *Camera, FVector &Location, int Radius, int Color, int Dotted);

	int	 Deproject			(ICamera *Camera,int ScreenX,int ScreenY,FVector *V,int UseEdScan,FLOAT Radius);
	int	 Project			(ICamera *Camera, FVector *V, FLOAT *ScreenX, FLOAT *ScreenY, FLOAT *Scale);
	int  BoundVisible 		(ICamera *Camera, FBoundingVolume *Bound, FSpanBuffer *SpanBuffer, FScreenBounds *Results,FCoords *Rotation);
	int  OrthoClip			(ICamera *Camera, const FVector *P1, const FVector *P2, FLOAT *ScreenX1, FLOAT *ScreenY1, FLOAT *ScreenX2, FLOAT *ScreenY2);
	void DrawOrthoLine		(ICamera *Camera, const FVector *P1, const FVector *P2,int Color,int Dotted);

	int inline TransformBspSurf(IModel *ModelInfo,ICamera *Camera, INDEX iNode, FTransform **Pts, BYTE &AllCodes);
	int ClipBspSurf (IModel *ModelInfo, ICamera *Camera, INDEX iNode, FTransform *OutPts);
	int ClipTexPoints (ICamera *Camera, FTransTex *InPts, FTransTex *OutPts, int Num0);

	int		SetActorSprite 	(ICamera *Camera, FSprite *Sprite, INDEX iActor);
	void	DrawActorSprite (ICamera *Camera, FSprite *Sprite);
	void	DrawActorChunk  (ICamera *Camera, FSprite *Sprite,FSpanBuffer *Span);
	int		DrawMeshMap		(ICamera *Camera, UMeshMap *MeshMap,FVector *Location,FRotation *Rotation,int Highlight, FSprite *Sprite,int DrawToBuffer,int DrawLit);
	void	DrawSprite		(ICamera *Camera, BYTE *Texture, int USize, int VSize,int X1, int Y1, int Center, EBlitType BlitType, FSpanBuffer *SpanBuffer,int Highlight);

private:
	void ShowStat (ICamera *Camera,int *StatYL,const char *Str);
	void DrawStats(ICamera *Camera);
	void DrawGridSection (ICamera *Camera, int CameraLocX,
		int CameraSXR, int CameraGridY, FVector *A, FVector *B,
		FLOAT *AX, FLOAT *BX,int AlphaCase);
};

extern FGlobalRender GRender;
extern FVirtualGlobalLightManager *GLightManager;

/*------------------------------------------------------------------------------------
	New DrawAcross code:
------------------------------------------------------------------------------------*/

void rendDrawAcrossSetup(ICamera *Camera, UTexture *ThisTexture, DWORD PolyFlags, DWORD NotPolyFlags);
void rendDrawAcross (ICamera *Camera,FSpanBuffer *SpanBuffer,
	FSpanBuffer *RectSpanBuffer, FSpanBuffer *LatticeSpanBuffer,
	FSpanBuffer *SubRectSpanBuffer, FSpanBuffer *SubLatticeSpanBuffer);

/*------------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------------*/
#endif // _INC_UNRENDER
