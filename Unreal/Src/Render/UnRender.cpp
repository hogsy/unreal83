/*=============================================================================
	UnRender.cpp: Main Unreal rendering functions and pipe

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney

	Rearrange:
		* Merge Bsp Poly and lighting info
		* Separate Bsp Node and bound info
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"
#include "UnRaster.h"
#include "UnRenDev.h"

FGlobalRender	GRender;
FGlobalRaster	GRaster;

UNRENDER_API FGlobalRender *GRendPtr = &GRender;

#ifdef STATS
FRenderStats GStat;
#endif

#ifdef ASM
	#define ASM_LATTICE
#endif

class FMipTable
	{
	public:
	BYTE MipLevel; 
	BYTE RoutineOfs;
	} GSmallMipTable[512*4],GLargeMipTable[512*4],GNoMipTable[512*4];

extern "C"
	{
	ASMVAR FMipTable	*TRL_MipTable;
	ASMVAR FLOAT		TRL_FBaseU;
	ASMVAR FLOAT		TRL_FBaseV;
	ASMVAR DWORD		TRL_TexBaseU;
	ASMVAR DWORD		TRL_TexBaseV;
	ASMVAR DWORD		TRL_LightBaseU;
	ASMVAR DWORD		TRL_LightBaseV;
	ASMVAR DWORD		TRL_LightVMask;
	ASMVAR BYTE			*TRL_MipRef;
	ASMVAR FBlitMipInfo	*TRL_MipPtr[8];
	ASMVAR BYTE			TRL_LightMeshShift;
	ASMVAR BYTE			TRL_LightMeshUShift;
	ASMVAR BYTE			TRL_RoutineOfsEffectBase;
	void __cdecl TRL_RectLoop(void);
	void __cdecl TRL_SelfModRect(void);
	};

/*-----------------------------------------------------------------------------
	FGlobalRender init & exit
-----------------------------------------------------------------------------*/

//
// Initialize the rendering engine and allocate all temporary buffers.
// Calls appError if failure.
//
void FGlobalRender::Init (void)
	{
	GUARD;
	static int TempStatic=0;
	//
	// Validate defined constants:
	//
	if (FBspNode::MAX_NODE_VERTICES > FPoly::MAX_FPOLY_VERTICES) appError ("Vertex problem");
	//
	MaxTransforms		= 32768+16383;
	MaxDynamics			= 1024;
	MaxPostDynamics		= 768;
	//
	// Allocate rendering stuff
	//
	PointTransform  = appMallocArray(MaxTransforms,FTransform,		"PointCache");
	VectorTransform = appMallocArray(MaxTransforms,FVector,			"VectorCache");
	DynamicsIndex   = appMallocArray(MaxDynamics,FDynamicsIndex,	"DynamicsCache");
	PostDynamics    = appMallocArray(MaxPostDynamics,INDEX,			"PostDynCache");
	//
	mymemset(LatticePtr,0,sizeof(LatticePtr));
	//
	InitDither();
	GCache.Flush();
	//
	// Initialize dynamics info:
	//
	DynamicsLocked  = 0;
	//
	// Misc params:
	//
	Toggle			= 0;
	RendIter		= 0;
	ShowLattice		= 0;
	Extra			= 0;
	Fog				= 0;
	DoDither		= 1;
	ShowChunks		= 0;
	LeakCheck		= 0;
	Extra1			= 0;
	Extra2			= 0;
	Extra3			= 0;
	Extra4			= 0;
	AllStats		= 0;
	MeshStatPtr		= &TempStatic;
	//
	// Byte multiply table:
	//
	int i;
	for (i=0; i<256; i++) for (int j=0; j<256; j++) ByteMult[i][j]=(WORD)i*j;
	//
	void DrawNormalRaster		 (BYTE*,QWORD,QWORD,int);
	void DrawMaskedRaster		 (BYTE*,QWORD,QWORD,int);
	void DrawBlendedRaster		 (BYTE*,QWORD,QWORD,int);
	void DrawMaskedBlendedRaster (BYTE*,QWORD,QWORD,int);
	//
	Raster256Table[0] = DrawNormalRaster;			// DRAW_RASTER_NORMAL
	Raster256Table[1] = DrawMaskedRaster;			// DRAW_RASTER_MASKED
	Raster256Table[2] = DrawBlendedRaster;			// DRAW_RASTER_BLENDED
	Raster256Table[3] = DrawMaskedBlendedRaster;	// DRAW_RASTER_MASKED_BLENDED
	//
	VectorTransformList = NULL;
	PointTransformList	= NULL;
	NumVectorTransforms = 0;
	NumPointTransforms  = 0;
	//
	for (i=0; i<MaxTransforms; i++)	VectorTransform[i].Align = FVA_Uncached;
	for (i=0; i<MaxTransforms; i++)	PointTransform [i].Align = FVA_Uncached;
	//
	// Init stats:
	//
	#ifdef STATS
	mymemset(&GStat,0,sizeof(GStat));
	Stat = &GStat;
	#endif
	//
	debug(LOG_Init,"Rendering initialized");
	//
	GRaster.Init();
	//
	for (i=0; i<8; i++)
		{
		TRL_MipPtr[i] = &GBlit.Mips[i];
		GBlit.MipRef[i] = GBlit.PrevMipRef[i] = 0;
		};
	for (i=0; i<512*4; i++)
		{
		int Mip = (i/4) - 127;
		int Ofs = (i&3);
		if (Mip<0)
			{
			GSmallMipTable[i].MipLevel    = 0;
			GSmallMipTable[i].RoutineOfs  = 1;
			}
		else if (Mip<7)
			{
			GSmallMipTable[i].MipLevel    = Mip;
			GSmallMipTable[i].RoutineOfs  = Mip*16 + (i&3)*4 + 1;
			}
		else
			{
			GSmallMipTable[i].MipLevel    = 7;
			GSmallMipTable[i].RoutineOfs  = 7*16 + 1;
			};
		GLargeMipTable[i].MipLevel   = OurClamp(Mip,0,7);
		GLargeMipTable[i].RoutineOfs = OurClamp(Mip,0,7)*16 + 1;
		//
		GNoMipTable[i].MipLevel      = 0;
		GNoMipTable[i].RoutineOfs    = 1;
		};
	GLightManager->Init();
	UNGUARD("FGlobalRender::Init");
	};

//
// Shut down the rendering engine
//
void FGlobalRender::Exit (void)
	{
	GUARD;
	//
	GRaster.Exit();
	//
	appFree(PointTransform);
	appFree(VectorTransform);
	appFree(DynamicsIndex);
	appFree(PostDynamics);
	//
	debug(LOG_Exit,"Rendering closed");
	//
	UNGUARD("FGlobalRender::Exit");
	};

/*-----------------------------------------------------------------------------
	FGlobalRender Stats display
-----------------------------------------------------------------------------*/

enum {STAT_Y = 16};

void FGlobalRender::DrawStats(ICamera *Camera)
	{
	GUARD;
	#ifdef STATS
	//
	char TempStr[256];
	GStat.ThisEndTime = GApp->TimeMSec();
	FLOAT FrameTime   = ((FLOAT)(SQWORD)GStat.ThisEndTime - (FLOAT)(SQWORD)GStat.LastEndTime  );
	FLOAT RenderTime  = ((FLOAT)(SQWORD)GStat.ThisEndTime - (FLOAT)(SQWORD)GStat.ThisStartTime);
	//
	if (QuickStats)
		{
		int XL,YL;
		sprintf (TempStr,"Fps=%3.1f (%3.1f) Nodes=%03i Polys=%03i Lites=%04i Thrash=%02i%%%% Zones=%02i Mem=%03iK",
			1000.0/(FLOAT)FrameTime,
			1000.0/(FLOAT)RenderTime,
			Stat->NodesDrawn,Stat->PolysDrawn,Stat->PolyLitesDrawn,
			(100*Stat->NodesDone)/(Stat->NodesTotal+1),
			Stat->VisibleZones,
			Stat->GDynMem>>10);
		GGfx.StrLen	  (&XL,&YL,0,0,GGfx.SmallFont,TempStr);
		//
		int Y=Camera->SYR;
		GGfx.BurnRect(Camera->Texture,0,Camera->SXR,Y-YL-3,Y,0);
		GGfx.Printf(Camera->Texture,(Camera->SXR-XL)/2,Y-YL-2,0,GGfx.SmallFont,NormalFontColor,TempStr);
		};
	if (AllStats)
		{
		int	 StatYL=0;
		//
		sprintf     (TempStr,"       FPS=%3.1f (%3.1f) MSec=%3.1f Tmap=%3.1f Draw=%3.1f Illum=%3.1f",1000.0/FrameTime,1000.0/RenderTime,RenderTime,GStat.TextureMap*Cyc2Msec,GCameraManager->DrawTime*Cyc2Msec,GStat.IllumTime*Cyc2Msec);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"       TICK Server=%3.1f Level=%3.1f Actor=%3.1f Audio=%3.1f Frags=%i",GServer.TaskTickTime*Cyc2Msec,GServer.LevelTickTime*Cyc2Msec,GServer.ActorTickTime*Cyc2Msec,GServer.AudioTickTime*Cyc2Msec,GServer.ActorCollisionFrags);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf		(TempStr,"     CACHE");
		GCache.Status(TempStr+strlen(TempStr));
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"      LITE MESHGEN=%i, LIGHTAGE=%iK, LIGHTMEM=%iK, TEX=%i, TEXMEM=%iK, MODS=%i",GStat.MeshesGen,GStat.Texelage>>10,GStat.TexelMem>>10,GStat.UniqueTextures,GStat.UniqueTextureMem>>10,GStat.CodePatches);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"  TRAVERAL VISIT=%i/%i BEHIND=%i EXTRA=%i",GStat.NodesDone,GStat.NodesTotal,GStat.NodesBehind,GStat.Extra1);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr," OCCLUSION Checks=%04i Back=%03i In=%03i PyrOut=%03i SpanOcc=%03i",GStat.BoxChecks,GStat.BoxBacks,GStat.BoxIn,GStat.BoxOutOfPyramid,GStat.BoxSpanOccluded);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"    MEMORY GMem=%iK GDynMem=%iK",GStat.GMem>>10,GStat.GDynMem>>10);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"      CLIP Accept=%05i Reject=%05i Nil=%05i",GStat.ClipAccept,GStat.ClipOutcodeReject,GStat.ClipNil);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"      ZONE Cur=%02i Visible=%02i/%02i Reject=%i",GStat.CurZone,GStat.VisibleZones,GStat.NumZones,GStat.MaskRejectZones);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"    RASTER BoxRej=%04i Rasterized=%04i Nodes=%04i Polys=%04i",GStat.NumRasterBoxReject,GStat.NumRasterPolys,GStat.NodesDrawn,GStat.PolysDrawn);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"    TIME Gen=%02.2f Lat=%02.2f TMap=%02.2f Asm=%02.2f",GStat.Generate*Cyc2Msec,GStat.CalcLattice*Cyc2Msec,GStat.Tmap*Cyc2Msec,GStat.Asm*Cyc2Msec);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"    TIME Setup=%02.2f SetupC=%02.2f Trans=%02.2f Clip=%02.2f",GStat.Setup*Cyc2Msec,GStat.SetupCached*Cyc2Msec,GStat.Transform*Cyc2Msec,GStat.Clip*Cyc2Msec);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"     SIDES Setup=%04i Cached=%04i",GStat.NumSides,GStat.NumSidesCached);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"   SPRITES NUM=%i MESH=%i CHNKS=%i FINAL=%i DRAWN=%i",
			GStat.NumSprites,GStat.MeshMapsDrawn,GStat.NumChunks,GStat.NumFinalChunks,GStat.ChunksDrawn);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"      SPAN CHURN=%i REJIG=%03i",GStat.SpanTotalChurn,GStat.SpanRejig);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"      SPAN XV=%02.2f OV=%02.2f CRU=%02.2f CR=%02.2f CIF=%02.2f (MSEC)",
			GStat.BoxIsVisible*Cyc2Msec,GStat.BoundIsVisible*Cyc2Msec,GStat.CopyFromRasterUpdate*Cyc2Msec,
			GStat.CopyFromRaster*Cyc2Msec,GStat.CopyIndexFrom*Cyc2Msec);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"      SPAN CF=%02.2f MW=%02.2f MF=%02.2f CRF=%02.2f CLF=%02.2f",
			GStat.CopyFromRange*Cyc2Msec,GStat.MergeWith*Cyc2Msec,GStat.MergeFrom*Cyc2Msec,
			GStat.CalcRectFrom*Cyc2Msec,GStat.CalcLatticeFrom*Cyc2Msec);
		ShowStat	(Camera,&StatYL,TempStr);
		//
		sprintf     (TempStr,"     ILLUM DYNLITES=%05i LATS=%05i LATLIGHTS=%05i",GStat.DynLightActors,GStat.LatsMade,GStat.LatLightsCalc);
		ShowStat	(Camera,&StatYL,TempStr);
		};
	#endif // STATS
	UNGUARD("FCameraConsole::DrawStats");
	};

//
// Show one statistic and update the pointer:
//
void FGlobalRender::ShowStat (ICamera *Camera,int *StatYL,const char *Str)
	{
	GUARD;
	int	XL,YL;
	if (*StatYL==0)
		{
		GGfx.Printf (Camera->Texture,16,STAT_Y + *StatYL,0,GGfx.SmallFont,NormalFontColor,"Statistics:");
		GGfx.StrLen (&XL,&YL,0,1,GGfx.SmallFont,"Statistics:");
		*StatYL += YL;
		};
	GGfx.Printf (Camera->Texture,16,STAT_Y + *StatYL,0,GGfx.SmallFont,NormalFontColor,"%s",Str);
    GGfx.StrLen (&XL,&YL,0,1,GGfx.SmallFont,Str);
	*StatYL += YL;
	UNGUARD("FCameraConsole::ShowStat");
	};

/*-----------------------------------------------------------------------------
	FGlobalRender PreRender & PostRender
-----------------------------------------------------------------------------*/

//
// Set up for rendering a frame.
//
void FGlobalRender::PreRender(ICamera *Camera)
	{
	GApp->EnableFastMath(1); // Set precision to 24 bit
	//
	#ifdef STATS
	QWORD Temp = GStat.ThisEndTime;
	//
	memset (&GStat,0,sizeof(GStat));
	//
	GStat.LastEndTime   = Temp;
	GStat.ThisStartTime = GApp->TimeMSec();
	#endif
	};

//
// Clean up after rendering a frame.
//
void FGlobalRender::PostRender(ICamera *Camera)
	{
	DrawStats(Camera);
	GApp->EnableFastMath(0); // Restore default precision
	};

/*-----------------------------------------------------------------------------
	FGlobalRender command line
-----------------------------------------------------------------------------*/

//
// Execute a command line.
//
int FGlobalRender::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	//
	if (GetCMD(&Str,"STATUS"))
		{
		if (GetCMD(&Str,"REN") || !Str[0])
			{
			Out->Logf("   REN - Alive and well");
			return Str[0]!=0;
			}
		else return 0;
		}
	else if (GetCMD(&Str,"STATS"))
		{
		AllStats ^= 1; Out->Logf("Stats are %s",AllStats?"on":"off"); return 1;
		}
	else if (GetCMD(&Str,"LEAK"))
		{
		LeakCheck ^= 1; Out->Logf("Leak check is %s",LeakCheck?"on":"off"); return 1;
		}
	else if (GetCMD(&Str,"REND"))
		{
		if 		(GetCMD(&Str,"SMOOTH"))		GGfx.Smooth		^= 1;
		else if (GetCMD(&Str,"STRETCH"))	GGfx.Stretch	^= 1;
		else if (GetCMD(&Str,"BILINEAR"))	DoDither		^= 1;
		else if (GetCMD(&Str,"CUTS"))		ShowChunks		^= 1;
		else if (GetCMD(&Str,"FOG"))		Fog				^= 1;
		else if (GetCMD(&Str,"EXTRA"))		Extra			^= 1;
		else if (GetCMD(&Str,"LATTICE"))	ShowLattice		^= 1;
		else if (GetCMD(&Str,"EXTRA1"))		Extra1			^= 1;
		else if (GetCMD(&Str,"EXTRA2"))		Extra2			^= 1;
		else if (GetCMD(&Str,"EXTRA3"))		Extra3			^= 1;
		else if (GetCMD(&Str,"T"))			Toggle			^= 1;
		else return 0;
		Out->Log("Rendering option recognized");
		return 1;
		}
	else return 0; // Not executed
	//
	UNGUARD("FGlobalRender::Exec");
	};

/*-----------------------------------------------------------------------------
	FGlobalRender point and vector transformation cache
-----------------------------------------------------------------------------*/

//
// Initialize the point and vector transform caches for a frame.
//
void FGlobalRender::InitTransforms (IModel *ModelInfo)
	{
	GUARD;
	//
	VectorTransformList = (INDEX *)GMem.Get(ModelInfo->MaxVectors * sizeof(INDEX));
	PointTransformList  = (INDEX *)GMem.Get(ModelInfo->MaxPoints  * sizeof(INDEX));
	NumVectorTransforms = 0;
	NumPointTransforms  = 0;
	//
	UNGUARD("FGlobalRender::InitTransforms");
	};

//
// Free and empty the point and vector transform caches.
//
void FGlobalRender::ExitTransforms(void)
	{
	GUARD;
	//
	INDEX *T;
	//
	T = &VectorTransformList[0];
	int i;
	for (i=0; i<NumVectorTransforms; i++)
		{
		VectorTransform[*T++].Align = FVA_Uncached;
		};
	T = &PointTransformList[0];
	for (i=0; i<NumPointTransforms; i++)
		{
		PointTransform[*T++].Align = FVA_Uncached;
		};
	GMem.Release(VectorTransformList);
	//
	UNGUARD("FGlobalRender::ExitTransforms");
	};

//
// Return the transformed and possibly projected value of a point.
//
inline FTransform *FGlobalRender::GetPoint(IModel *ModelInfo, ICamera *Camera, INDEX pPoint)
	{
	FTransform 	*T = &PointTransform[pPoint];
	if (T->Align==FVA_Uncached)
		{
		//
		// Transform into screenspace
		//
		*(FVector *)T	= ModelInfo->FPoints[pPoint];
		PointTransformList[NumPointTransforms++] = pPoint;
		//
		T->iTransform	= pPoint;
		T->IntX			= MAXINT;
		T->TransformPoint (Camera->Coords);
		T->ComputeOutcode (Camera);
		};
	return T;
	};

//
// Return the transformed value of a vector.
//
inline FVector *FGlobalRender::GetVector(IModel *ModelInfo, const FCoords *Coords, INDEX vVector)
	{
	FVector *T = &VectorTransform[vVector];
	if (T->Align==FVA_Uncached)
		{
		FVector *V = &ModelInfo->FVectors[vVector];
		VectorTransformList[NumVectorTransforms++] = vVector;
		switch (V->Align)
			{
			case FVA_None:
				T->X = V->X * Coords->XAxis.X + V->Y * Coords->XAxis.Y + V->Z * Coords->XAxis.Z;
				T->Y = V->X * Coords->YAxis.X + V->Y * Coords->YAxis.Y + V->Z * Coords->YAxis.Z;
				T->Z = V->X * Coords->ZAxis.X + V->Y * Coords->ZAxis.Y + V->Z * Coords->ZAxis.Z;
				break;
			case FVA_X:
				T->X = V->X * Coords->XAxis.X;
				T->Y = V->X * Coords->YAxis.X;
				T->Z = V->X * Coords->ZAxis.X;
				break;
			case FVA_Y:
				T->X = V->Y * Coords->XAxis.Y;
				T->Y = V->Y * Coords->YAxis.Y;
				T->Z = V->Y * Coords->ZAxis.Y;
				break;
			case FVA_Z:
				T->X = V->Z * Coords->XAxis.Z;
				T->Y = V->Z * Coords->YAxis.Z;
				T->Z = V->Z * Coords->ZAxis.Z;
				break;
			};
		};
	return T;
	};

/*--------------------------------------------------------------------------
	Clippers
--------------------------------------------------------------------------*/

//
// Clipping macro used by ClipBspSurf and ClipTexPts.
//
#define CLIPPER(DOT1EXPR,DOT2EXPR,CACHEDVIEWPLANE)\
	{\
	Num1    = 0;\
	SrcPtr  = &SrcList  [0];\
	DestPtr = &DestList [0];\
	P1		= SrcList   [Num0-1];\
	Dot1	= DOT1EXPR;\
	while (Num0-- > 0)\
		{\
		P2   = *SrcPtr++;\
		Dot2 = DOT2EXPR;\
		if (Dot1>=0.0) /* P1 is unclipped */ \
			{\
			*DestPtr++ = P1;\
			Num1++;\
			if (Dot2<0.0) /* P1 is unclipped, P2 is clipped. */ \
				{\
				*Top = *P2 + (*P1-*P2) * (Dot2/(Dot2-Dot1));\
				Top->iTransform = INDEX_NONE;\
				Top->iSide      = P2->iSide; /* Cache partially clipped side rasterization */ \
				*DestPtr++		= Top++;\
				Num1++;\
				};\
			}\
		else if (Dot2>0.0) /* P1 is clipped, P2 is unclipped. */ \
			{\
			*Top = *P2 + (*P1-*P2) * (Dot2/(Dot2-Dot1));\
			Top->iTransform = INDEX_NONE;\
			Top->iSide      = CACHEDVIEWPLANE; /* Fully clipped side - use cache from view planes 0-3 */ \
			*DestPtr++      = Top++;\
			Num1++;\
			};\
		P1   = P2;\
		Dot1 = Dot2;\
		};\
	if (Num1<3) {STAT(GStat.ClipNil++); ENDTIME(Clip); return 0;};\
	Num0     = Num1;\
	TempList = SrcList;\
	SrcList  = DestList;\
	DestList = TempList;\
	};

//
// Transform a Bsp poly into a list of points, and return number of points or zero
// if outcode rejected.
//
int inline FGlobalRender::TransformBspSurf(IModel *ModelInfo,ICamera *Camera,
	INDEX iNode, FTransform **Pts, BYTE &AllCodes)
	{
	BEGINTIME(Transform);
	//
	FBspNode   		*Node		= &ModelInfo->BspNodes[iNode];
	FVertPool		*VertPool	= &ModelInfo->VertPool[Node->iVertPool];
	BYTE			Outcode		= FVF_OutReject;
	FTransform		**DestPtr	= &Pts[0];
	int				Num			= Node->NumVertices;;
	//
	// Transform, outcode reject, and build initial point list:
	//
	AllCodes = 0;
	for (int i=0; i<Num; i++)
		{
		*DestPtr			 = GetPoint(ModelInfo,Camera,VertPool->pVertex);
		(*DestPtr)->iSide	 = VertPool->iSide;
		Outcode				&= (*DestPtr)->Flags;
		AllCodes			|= (*DestPtr)->Flags;
		//
		#ifdef PARANOID
			if (VertPool->iSide<4) appErrorf("Bad iSide %i",VertPool->iSide);
		#endif
		//
		VertPool++;
		DestPtr++;
		};
	ENDTIME(Transform);
	//
	if (Outcode) return 0;
	else return Num;
	};

//
// Transform and clip a Bsp poly.  Outputs the transformed points to OutPts and
// returns the number of points, which will be zero or >=3.
//
int FGlobalRender::ClipBspSurf (IModel *ModelInfo, ICamera *Camera, INDEX iNode, FTransform *OutPts)
	{
	GUARD;
	BEGINTIME(Clip);
	//
	static FTransform WorkPts[FBspNode::MAX_FINAL_VERTICES],*List0[FBspNode::MAX_FINAL_VERTICES],*List1[FBspNode::MAX_FINAL_VERTICES];
	FTransform		*Top,*P1,*P2;
	FTransform		**SrcList,**DestList,**TempList,**SrcPtr,**DestPtr;
	FLOAT			Dot1,Dot2,Factor;
	BYTE			AllCodes;
	int				Num0,Num1;
	//
	Num0 = TransformBspSurf(ModelInfo,Camera,iNode,List0,AllCodes);
	if (!Num0)
		{
		STAT(GStat.ClipOutcodeReject++;)
		ENDTIME(Clip);
		return 0;
		};
	Top = &WorkPts[0];
	SrcList	 = List0;
	DestList = List1;
	//
	// Clip point list by each view frustrum clipping plane:
	//
	if (AllCodes & FVF_OutXMin) CLIPPER
		(
		P1->X * Camera->ProjZRSX2 + P1->Z,
		P2->X * Camera->ProjZRSX2 + P2->Z,0
		); 
	if (AllCodes & FVF_OutXMax) CLIPPER
		(
		P1->Z - P1->X * Camera->ProjZRSX2,
		P2->Z - P2->X * Camera->ProjZRSX2,1
		);
	if (AllCodes & FVF_OutYMin) CLIPPER
		(
		P1->Y * Camera->ProjZRSY2 + P1->Z,
		P2->Y * Camera->ProjZRSY2 + P2->Z,2
		); 
	if (AllCodes & FVF_OutYMax) CLIPPER
		(
		P1->Z - P1->Y * Camera->ProjZRSY2,
		P2->Z - P2->Y * Camera->ProjZRSY2,3
		);
	P2     = &OutPts  [0];
	SrcPtr = &SrcList [0];
	for (int i=0; i<Num0; i++)
		{
		P1 = *SrcPtr++;
		if ((P1->iTransform==INDEX_NONE)||(P1->IntX==MAXINT))
			{
			Factor      = Camera->ProjZ / P1->Z;
			//
			P1->ScreenX = 65536.0 * (P1->X * Factor + Camera->FSXR15);
			P1->ScreenY = P1->Y * Factor + Camera->FSYR15;
			//
			ftoi(P1->IntX,P1->ScreenX);
			ftoi(P1->IntY,P1->ScreenY-0.5);
			//
			// Prevent numerical imprecision errors (tiny Z) from killing clipper.
			// This happens once every few hours of operation when the player happens
			// to be very, very close to a polygon.
			//
			if		(P1->IntX < 0			)	{P1->ScreenX=0.0;			P1->IntX=0;}
			else if (P1->IntX > Camera->FixSXR)	{P1->ScreenX=Camera->FSXR1;	P1->IntX=Camera->FixSXR;};
			//
			if		(P1->IntY < 0			)	{P1->ScreenY=0.0;			P1->IntY=0;}
			else if (P1->IntY > Camera->SYR	)	{P1->ScreenY=Camera->FSYR1;	P1->IntY=Camera->SYR;};
			};
		*P2++ = *P1;
		};
	STAT(GStat.ClipAccept++;)
	ENDTIME(Clip);
	return Num0;
	//
	UNGUARD("FGlobalRender::ClipBspSurf");
	};

//
// Clip a set of points with vertex texture coordinates and vertex lighting values.
// Outputs the points to OutPts and returns the number of poiints, which will be
// 0 or >=3.
//
int FGlobalRender::ClipTexPoints (ICamera *Camera, FTransTex *InPts, FTransTex *OutPts, int Num0)
	{
	GUARD;
	BEGINTIME(Clip);
	//
	static FTransTex WorkPts[FBspNode::MAX_FINAL_VERTICES],*List0[FBspNode::MAX_FINAL_VERTICES],*List1[FBspNode::MAX_FINAL_VERTICES];
	FTransTex		*Top,*P1,*P2;
	FTransTex		**SrcList,**DestList,**TempList,**SrcPtr,**DestPtr;
	FLOAT			Dot1,Dot2,Factor;
	BYTE			AllCodes,Outcode;
	int				Num1;
	//
	DestPtr	 = &List0[0];
	Outcode  = FVF_OutReject;
	AllCodes = 0;
	int i;
	for (i=0; i<Num0; i++)
		{
		*DestPtr     = &InPts[i];
		Outcode		&= (*DestPtr)->Flags;
		AllCodes	|= (*DestPtr)->Flags;
		DestPtr++;
		};
	if (Outcode) return 0;
	//
	Top      = &WorkPts[0];
	SrcList	 = List0;
	DestList = List1;
	//
	// Clip point list by each view frustrum clipping plane:
	//
	if (AllCodes & FVF_OutXMin) CLIPPER
		(
		P1->X * Camera->ProjZRSX2 + P1->Z,
		P2->X * Camera->ProjZRSX2 + P2->Z,INDEX_NONE
		); 
	if (AllCodes & FVF_OutXMax) CLIPPER
		(
		P1->Z - P1->X * Camera->ProjZRSX2,
		P2->Z - P2->X * Camera->ProjZRSX2,INDEX_NONE
		);
	if (AllCodes & FVF_OutYMin) CLIPPER
		(
		P1->Y * Camera->ProjZRSY2 + P1->Z,
		P2->Y * Camera->ProjZRSY2 + P2->Z,INDEX_NONE
		); 
	if (AllCodes & FVF_OutYMax) CLIPPER
		(
		P1->Z - P1->Y * Camera->ProjZRSY2,
		P2->Z - P2->Y * Camera->ProjZRSY2,INDEX_NONE
		);
	P2     = &OutPts  [0];
	SrcPtr = &SrcList [0];
	for (i=0; i<Num0; i++)
		{
		P1 = *SrcPtr++;
		if (P1->iTransform==INDEX_NONE)
			{
			Factor      = Camera->ProjZ / P1->Z;
			//
			P1->ScreenX = P1->X * Factor + Camera->FSXR15;
			P1->ScreenY = P1->Y * Factor + Camera->FSYR15;
			};
		ftoi(P1->IntX,P1->ScreenX-0.5);
		ftoi(P1->IntY,P1->ScreenY-0.5);
		//
		// Prevent numerical imprecision errors (tiny Z) from killing clipper:
		//
		if		(P1->IntX < 0			)	{P1->ScreenX=0.0;			P1->IntX=0;				debug(LOG_Info,"FlowX0");}
		else if (P1->IntX > Camera->SXR	)	{P1->ScreenX=Camera->FSXR;	P1->IntX=Camera->SXR;	debug(LOG_Info,"FlowX1");};
		if		(P1->IntY < 0			)	{P1->ScreenY=0.0;			P1->IntY=0;				debug(LOG_Info,"FlowY0");}
		else if (P1->IntY > Camera->SYR	)	{P1->ScreenY=Camera->FSYR;	P1->IntY=Camera->SYR;	debug(LOG_Info,"FlowY1");};
		//
		*P2++ = *P1;
		};
	ENDTIME(Clip);
	return Num0;
	//
	UNGUARD("FGlobalReneder::ClipTexPoints");
	};

/*-----------------------------------------------------------------------------
	Bsp occlusion functions
-----------------------------------------------------------------------------*/

//
// Checks whether the node's bouding box is totally occluded.  Returns 0 if
// total occlusion, 1 if all or partial visibility.
//
int FGlobalRender::BoundVisible (ICamera *Camera, FBoundingVolume *Bound, FSpanBuffer *SpanBuffer,
	FScreenBounds *Results, FCoords *Rotation)
	{
	GUARD;
	//
	FCoords			*CameraCoords;
	FCoords			TempCoords;
	FCoords       	BoxDot[2];
	FTransform		Pts[8],*Pt;
	FVector 		CameraLoc;
	FVector       	NewMin,NewMax;
	FLOAT         	Persp1,Persp2,Temp,BoxMinZ,BoxMaxZ;
	int         	BoxX,BoxY;
	int 			BoxMinX,BoxMaxX,BoxMinY,BoxMaxY;
	int				i,OutCode;
	//
	STAT(GStat.BoxChecks++);
	//
	if (!Rotation)
		{
		//
		// Box is axis-aligned; use camera coordinate system as-is:
		//
		CameraCoords = &Camera->Coords;
		}
	else
		{
		//
		// Generate a new, rotated camera coordinate system to make up for the box's rotation:
		//
		TempCoords   = Camera->Coords;
		CameraCoords = &TempCoords;
		//
		TempCoords.TransformByCoords(*Rotation);
		//
		TempCoords.Origin = Camera->Coords.Origin - Rotation->Origin;
		TempCoords.Origin.TransformVector (*Rotation);
		TempCoords.Origin += Rotation->Origin;
		};
	//
	// Handle rejection in orthogonal views:
	//
	if (Camera->Camera->IsOrtho())
		{
		FLOAT SX1,SY1,SX2,SY2;
		Project (Camera,&Bound->Min,&SX1,&SY1,NULL);
		Project (Camera,&Bound->Max,&SX2,&SY2,NULL);
		//
		if  (
			((SX1<0.0         )&&(SX2<0.0         )) ||
			((SY1<0.0         )&&(SY2<0.0         )) ||
			((SX1>Camera->FSXR)&&(SX2>Camera->FSXR)) ||
			((SY1>Camera->FSYR)&&(SY2>Camera->FSYR))
			) return 0; // Occluded
		else return 1; // Visible
		};
	//
	// Trivial accept: If camera is within bouding box, skip calculations because
	// the bounding box is definitely in view:
	//
	CameraLoc = CameraCoords->Origin;
	NewMin    = Bound->Min - CameraLoc;
	NewMax    = Bound->Max - CameraLoc;
	//
	if ((NewMin.X < 0.0) && (NewMax.X > 0.0) &&
		(NewMin.Y < 0.0) && (NewMax.Y > 0.0) &&
		(NewMin.Z < 0.0) && (NewMax.Z > 0.0))
		{
		if (Results) Results->Valid = 0;
		STAT(GStat.BoxIn++);
		return 1; // Visible
		};
	//
	// Test bounding-box side-of-camera rejection.  Since box is axis-aligned,
	// this can be optimized: Box can only be rejected if all 8 dot products of the
	// 8 box sides are less than zero.  This is the case iff each dot product
	// component is less than zero.
	//
	BoxDot[0].ZAxis.X = NewMin.X * CameraCoords->ZAxis.X; BoxDot[1].ZAxis.X = NewMax.X * CameraCoords->ZAxis.X;
	BoxDot[0].ZAxis.Y = NewMin.Y * CameraCoords->ZAxis.Y; BoxDot[1].ZAxis.Y = NewMax.Y * CameraCoords->ZAxis.Y;
	BoxDot[0].ZAxis.Z = NewMin.Z * CameraCoords->ZAxis.Z; BoxDot[1].ZAxis.Z = NewMax.Z * CameraCoords->ZAxis.Z;
	//
	if ((BoxDot[0].ZAxis.X<=0.0) && (BoxDot[0].ZAxis.Y<=0.0) && (BoxDot[0].ZAxis.Z<=0.0) &&
		(BoxDot[1].ZAxis.X<=0.0) && (BoxDot[1].ZAxis.Y<=0.0) && (BoxDot[1].ZAxis.Z<=0.0))
		{
		if (Results) Results->Valid = 0;
		STAT(GStat.BoxBacks++);
		return 0; // Totally behind camera - invisible
		};
	//
	// Transform bounding box min and max coords into screenspace
	//
	BoxDot[0].XAxis.X = NewMin.X * CameraCoords->XAxis.X; BoxDot[1].XAxis.X = NewMax.X * CameraCoords->XAxis.X;
	BoxDot[0].XAxis.Y = NewMin.Y * CameraCoords->XAxis.Y; BoxDot[1].XAxis.Y = NewMax.Y * CameraCoords->XAxis.Y;
	BoxDot[0].XAxis.Z = NewMin.Z * CameraCoords->XAxis.Z; BoxDot[1].XAxis.Z = NewMax.Z * CameraCoords->XAxis.Z;
	//
	BoxDot[0].YAxis.X = NewMin.X * CameraCoords->YAxis.X; BoxDot[1].YAxis.X = NewMax.X * CameraCoords->YAxis.X;
	BoxDot[0].YAxis.Y = NewMin.Y * CameraCoords->YAxis.Y; BoxDot[1].YAxis.Y = NewMax.Y * CameraCoords->YAxis.Y;
	BoxDot[0].YAxis.Z = NewMin.Z * CameraCoords->YAxis.Z; BoxDot[1].YAxis.Z = NewMax.Z * CameraCoords->YAxis.Z;
	//
	// View-pyramid reject (outcode test):
	//
	int ThisCode,AllCodes;
	//
	BoxMinZ=Pts[0].Z;
	BoxMaxZ=Pts[0].Z;
	OutCode  = 1|2|4|8;
	AllCodes = 0;
	#define CMD(i,j,k,First,P)\
		\
		ThisCode = 0;\
		\
		P.Z = BoxDot[i].ZAxis.X + BoxDot[j].ZAxis.Y + BoxDot[k].ZAxis.Z;\
		if (First || (P.Z < BoxMinZ)) BoxMinZ = P.Z;\
		if (First || (P.Z > BoxMaxZ)) BoxMaxZ = P.Z;\
		\
		P.X = BoxDot[i].XAxis.X + BoxDot[j].XAxis.Y + BoxDot[k].XAxis.Z;\
		Temp = P.X * Camera->ProjZRSX2;\
		if (Temp < -P.Z) ThisCode |= 1;\
		if (Temp >= P.Z) ThisCode |= 2;\
		\
		P.Y = BoxDot[i].YAxis.X + BoxDot[j].YAxis.Y + BoxDot[k].YAxis.Z;\
		Temp = P.Y * Camera->ProjZRSY2;\
		if (Temp <  -P.Z) ThisCode |= 4;\
		if (Temp >=  P.Z) ThisCode |= 8;\
		\
		OutCode  &= ThisCode;\
		AllCodes |= ThisCode;
	CMD(0,0,0,1,Pts[0]); CMD(1,0,0,0,Pts[1]); CMD(0,1,0,0,Pts[2]); CMD(1,1,0,0,Pts[3]);
	CMD(0,0,1,0,Pts[4]); CMD(1,0,1,0,Pts[5]); CMD(0,1,1,0,Pts[6]); CMD(1,1,1,0,Pts[7]);
	#undef CMD
	//
	if (OutCode)
		{
		STAT(GStat.BoxOutOfPyramid++;);
		return 0; // Invisible - pyramid reject
		};
	//
	// Calculate projections of 8 points and take X,Y min/max bounded to Span X,Y window:
	//
	Persp1 = Camera->ProjZ / BoxMinZ;
	Persp2 = Camera->ProjZ / BoxMaxZ;
	//
	Pt = &Pts[0];
	BoxMinX = BoxMaxX = Camera->SXR2 + ftoi(Pt->X * Persp1);
	BoxMinY = BoxMaxY = Camera->SYR2 + ftoi(Pt->Y * Persp1);
	//
	if (AllCodes & 1) BoxMinX = 0;
	if (AllCodes & 2) BoxMaxX = Camera->SXR;
	if (AllCodes & 4) BoxMinY = 0;
	if (AllCodes & 8) BoxMaxY = Camera->SYR;
	//
	for (i=0; i<8; i++)
		{
		//
		// Check with MinZ:
		//
		BoxX = Camera->SXR2 + ftoi(Pt->X * Persp1);
		BoxY = Camera->SYR2 + ftoi(Pt->Y * Persp1);
		if (BoxX < BoxMinX) BoxMinX = BoxX; else if (BoxX > BoxMaxX) BoxMaxX = BoxX;
		if (BoxY < BoxMinY) BoxMinY = BoxY; else if (BoxY > BoxMaxY) BoxMaxY = BoxY;
		//
		// Check with MaxZ:
		//										
		BoxX = Camera->SXR2 + ftoi(Pt->X * Persp2);
		BoxY = Camera->SYR2 + ftoi(Pt->Y * Persp2);
		if (BoxX < BoxMinX) BoxMinX = BoxX; else if (BoxX > BoxMaxX) BoxMaxX = BoxX;
		if (BoxY < BoxMinY) BoxMinY = BoxY; else if (BoxY > BoxMaxY) BoxMaxY = BoxY;
		//
		Pt++;
		};
	if (Results) // Set bounding box size
		{
		Results->Valid = 1;
		Results->MinZ  = BoxMinZ;
		Results->MaxZ  = BoxMaxZ;
		Results->MinX  = BoxMinX;
		Results->MinY  = BoxMinY;
		Results->MaxX  = BoxMaxX;
		Results->MaxY  = BoxMaxY;
		};
	if ((!SpanBuffer) || SpanBuffer->BoxIsVisible(BoxMinX,BoxMinY,BoxMaxX,BoxMaxY))
		{
		return 1; // Visible
		}
	else
		{
		STAT(GStat.BoxSpanOccluded++);
		return 0; // Invisible
		};
	UNGUARD("FGlobalRender::BoundVisible");
	};

enum ENodePass
	{
	PASS_FRONT,
	PASS_NODE,
	PASS_BACK,
	};

class FNodeStack
	{
	public:
	int			iNode;
	int			iFarNode;
	int			FarOutside;
	int			Outside;
	int			DrewStuff;
	ENodePass	Pass;
	};

enum EPlaneStatusCache // For unqiue plane status cache
	{
	PSC_Cached		= 0x01,	// Plane status has been calculated
	PSC_Front		= 0x02, // Viewer is in front of plane (Dot>=0.0)
	PSC_Backfaced	= 0x04,	// Plane is backfaced with certainty (Dot<=1.0)
	PSC_FarOut		= 0x08,	// Far side of the node is entirely behind the viewer
	};

BYTE GetPlaneStatus(ICamera *Camera,IModel *ModelInfo,BYTE *PlaneStatusCache,FVector &Origin,FBspNode *Node)
	{
	BYTE *Status,NewStatus;
	//
	if (Node->NodeFlags & NF_UniquePlane) // Look up cacheable status
		{
		Status = &PlaneStatusCache[Node->iUniquePlane]; // Look up status in cache
		if (*Status & PSC_Cached) return *Status; // Status is in cache so return it
		NewStatus	= PSC_Cached; // Create a new cache entry
		}
	else // Look up noncacheable status
		{
		Status    = &NewStatus;
		NewStatus = 0;
		};
	FBspSurf	*Poly	= &ModelInfo->BspSurfs[Node->iSurf];
	FVector		*Base	= &ModelInfo->FPoints [Poly->pBase];
	FVector		*Normal	= &ModelInfo->FVectors[Poly->vNormal];
	FLOAT		Dot		= (Origin - *Base) | *Normal;
	//
	if (Dot>=0.0)
		{
		NewStatus |= PSC_Front;
		if (((*Normal | Camera->ViewSides[0])>0.0) &&
			((*Normal | Camera->ViewSides[1])>0.0) &&
			((*Normal | Camera->ViewSides[2])>0.0) &&
			((*Normal | Camera->ViewSides[3])>0.0))
			{
			NewStatus |= PSC_FarOut;
			STAT(GStat.NodesBehind++);
			};
		}
	else
		{
		if (Dot<-1.0) NewStatus |= PSC_Backfaced;
		if (((*Normal | Camera->ViewSides[0])<0.0) &&
			((*Normal | Camera->ViewSides[1])<0.0) &&
			((*Normal | Camera->ViewSides[2])<0.0) &&
			((*Normal | Camera->ViewSides[3])<0.0))
			{
			NewStatus |= PSC_FarOut;
			STAT(GStat.NodesBehind++);
			};
		};
	*Status = NewStatus;
	return NewStatus;
	};

int FGlobalRender::OccludeBsp( ICamera *Camera, FSpanBuffer *Backdrop )
{
	static IModel 				*ModelInfo;
	static FSpanBuffer			ZoneSpanBuffer[UBspNodes::MAX_ZONES],*SpanBuffer;
	static FBspDrawList			*TopDrawList;
	static FRasterSetup			WorkingRaster;
	static FBspNode      		*Node,*BspNodes;
	static FBspSurf      		*Poly,*BspSurfs;
	static FNodeStack			*Stack,NodeStack[MAX_VISIBLE_NODES];
	static FTransform 			Pts[FBspNode::MAX_FINAL_VERTICES];
	static FRasterSideSetup		**SideCache;
	static FVector				Origin;
	static DWORD				PolyFlags;
	static QWORD				ActiveZoneMask;
	static INDEX          		*AllPolys,iNode,iSurf,iOriginalNode,iThingZone;
	static FLOAT				MaxZ,MinZ;
	static BYTE					iViewZone,iZone,iOppositeZone,*PlaneStatusCache,Status,OriginalStatus;
	static int           		Visible,Merging,Mergeable,Outside,Pass,DrewStuff,NumPts;
	static int					NodeCount,CoplanarPass;

	GUARD;
	if ( Camera->Level.ModelInfo.NumBspNodes==0 ) return 0;

	ModelInfo			= &Camera->Level.ModelInfo;
	AllPolys            = (INDEX             *)GMem.GetOned  (ModelInfo->MaxBspSurfs   *sizeof(INDEX));
	SideCache	        = (FRasterSideSetup **)GMem.GetZeroed(ModelInfo->NumSharedSides*sizeof(FRasterSideSetup *));
	PlaneStatusCache	= (BYTE				 *)GMem.GetZeroed(ModelInfo->MaxBspSurfs   *sizeof(BYTE));
	BspNodes			= ModelInfo->BspNodes;
	BspSurfs			= ModelInfo->BspSurfs;
	Origin				= Camera->Coords.Origin;
	TopDrawList			= DrawList;
	Stack				= &NodeStack[0];
	NodeCount			= 0;
	iViewZone			= ModelInfo->PointZone(&Origin);

	for( int qq=0; qq<ModelInfo->MaxBspSurfs; qq++ ) AllPolys[qq]=INDEX_NONE;//!!
	/* For debugging
	bug	("%i",
		ModelInfo->MaxBspSurfs   *sizeof(INDEX) +
		ModelInfo->NumSharedSides*sizeof(FRasterSideSetup *) +
		ModelInfo->MaxBspSurfs   *sizeof(BYTE));
	*/

	//for( int zz=0; zz<ModelInfo->MaxBspNodes; zz++ ) ModelInfo->BspNodes[zz].NodeFlags &= ~NF_AllOccluded;

	// Init first four units of the rasterization side setup cache so that they
	// represent the setups for the four view frustrum clipping planes.
	int i;
	for ( i=0; i<4; i++ ) SideCache[i]=(FRasterSideSetup *)GDynMem.GetFast(sizeof(FRasterSideSetup));
	SideCache[0]->P.X = 0;					SideCache[0]->DP.X = 0; // X min clipping plane
	SideCache[1]->P.X = FIX(Camera->SXR);	SideCache[1]->DP.X = 0; // X max clipping plane
	SideCache[2]->P.X = 0;					SideCache[2]->DP.X = 0; // Y min clipping plane
	SideCache[3]->P.X = FIX(Camera->SYR);	SideCache[3]->DP.X = 0; // Y max clipping plane

	// Init zone span buffers.
	for ( i=0; i<UBspNodes::MAX_ZONES; i++ ) ZoneSpanBuffer[i].AllocIndex(0,0,&GDynMem);
	ZoneSpanBuffer[iViewZone].AllocIndexForScreen( Camera->SXR,Camera->SYR,&GDynMem );
	ActiveZoneMask = ((QWORD)1) << iViewZone;

	// Init unrolled recursion stack.
	Stack				= &NodeStack[0];
	iNode				= 0;
	Outside				= 1;
	DrewStuff			= 0;
	Pass				= PASS_FRONT;

	for( ;; )
	{
		Node = &BspNodes[iNode];

		// Pass 1: Process node for the first time and optionally recurse with front node.
		if( Pass==PASS_FRONT )
		{
			if( iViewZone )
			{
				// Use pure zone rejection.
				if( !(Node->ZoneMask & ActiveZoneMask) )
				{
					STAT(GStat.MaskRejectZones++);
					goto PopStack;
				}
			}
			if( (Node->NodeFlags & (NF_AllOccluded|NF_Bounded))==(NF_AllOccluded|NF_Bounded) )
			{
				// Use bounding box rejection if occluded on the previous frame.
				FScreenBounds Results;
				if( !BoundVisible (Camera,&ModelInfo->Bounds[Node->iBound],iViewZone?NULL:&ZoneSpanBuffer[0],&Results,NULL) )
				{
					goto PopStack;
				}
				if(Results.Valid && iViewZone)
				{
					QWORD ZoneMask=1;
					FSpanBuffer *ZoneSpan = &ZoneSpanBuffer[0];

					for( int iZone=0; iZone<64; iZone++)
					{
						if( ZoneSpan->ValidLines && (Node->ZoneMask & ZoneMask))
						{
							if( ZoneSpan->BoundIsVisible(Results) ) goto Visible;
						}
						ZoneMask = ZoneMask << 1;
						ZoneSpan++;
					}
					STAT( GStat.BoxSpanOccluded++; );
					goto PopStack;
					Visible:;
				}
			}
			if( Node->iDynamic[0]!=INDEX_NONE )
			{
				dynamicsFilter (Camera,iNode,1,Outside);
			}
			Status = GetPlaneStatus( Camera,ModelInfo,PlaneStatusCache,Origin,Node );
			if( Status & PSC_Front )
			{
				Stack->iFarNode   = Node->iBack;
				Stack->FarOutside = Outside && !Node->IsCsg();

				if ( Node->iFront != INDEX_NONE )
				{
					Stack->iNode		= iNode;
					Stack->Outside		= Outside;
					Stack->DrewStuff	= 0;
					Stack->Pass  		= PASS_NODE;
					Stack++;

					iNode				= Node->iFront;
					Outside				= Outside || Node->IsCsg();
					Pass				= PASS_FRONT;

					continue;
				}
			}
			else
			{
				Stack->iFarNode   = Node->iFront;
				Stack->FarOutside = Outside || Node->IsCsg();
				if( Node->iBack  != INDEX_NONE )
				{
					Stack->iNode		= iNode;
					Stack->Outside		= Outside;
					Stack->DrewStuff	= 0;
					Stack->Pass			= PASS_NODE;
					Stack++;

					iNode		= Node->iBack;
					Outside		= Outside && !Node->IsCsg();
					Pass		= PASS_FRONT;

					continue;
				}
			}
			Pass		= PASS_NODE;
		}

		// Pass 2: Process polys within this node and optionally recurse with back.
		if (Pass==PASS_NODE)
		{
			iOriginalNode	= iNode;
			Status			= GetPlaneStatus(Camera,ModelInfo,PlaneStatusCache,Origin,Node);
			OriginalStatus  = Status;
			iThingZone      = iViewZone?((Status&PSC_Front)?Node->iZone:Node->iBackZone):0;

			if( (Node->iDynamic[(Status&PSC_Front)?0:1]!=INDEX_NONE) && ZoneSpanBuffer[iThingZone].ValidLines )
			{
				dynamicsPreRender
				(
					Camera,
					&ZoneSpanBuffer[iThingZone],
					iNode,
					(Status&PSC_Front) ? 0 : 1
				);
			}
			if ( Status & PSC_FarOut ) goto PrePopStack;

			// Make two passes through this list of coplanars.  Draw regular (solid) polys on
			// first pass, semisolids and nonsolids on second pass.
			for( CoplanarPass=0; CoplanarPass<2; CoplanarPass++ )
				{
				for( ;; )
				{
					// Process node and all of its coplanars.
					if( iViewZone && !(Node->ZoneMask&ActiveZoneMask) )
					{
						STAT(GStat.MaskRejectZones++);
						break;
					}
					iSurf		= Node->iSurf;
					Poly		= &BspSurfs[iSurf];
					PolyFlags	= Poly->PolyFlags | Camera->ExtraPolyFlags;

					if( CoplanarPass ^ !(PolyFlags & (PF_NotSolid|PF_Semisolid)) ) goto NextCoplanar;
					STAT(GStat.NodesDone++);

					if( Status & PSC_Front )
					{
						iZone         = Node->iZone;
						iOppositeZone = Node->iBackZone;
					}
					else
					{
						if( (Status&PSC_Backfaced) && !(Poly->PolyFlags & (PF_TwoSided|PF_Portal)) )
							goto NextCoplanar;
						iZone         = Node->iBackZone;
						iOppositeZone = Node->iZone;
					}
					if( iViewZone==0 )
					{
						iZone         = 0;
						iOppositeZone = 0;
					}
					SpanBuffer = &ZoneSpanBuffer[iZone];
					if( SpanBuffer->ValidLines <= 0 ) goto NextCoplanar;

					// Clip it:
					NumPts = ClipBspSurf( ModelInfo,Camera,iNode,Pts );
					if( !NumPts ) goto NextCoplanar;

					// Box reject this poly if it was entirely occluded last frame.
					if( Node->NodeFlags & NF_PolyOccluded )
					{
						FScreenBounds Box;
						WorkingRaster.CalcBound(Pts,NumPts,Box);
						if( !SpanBuffer->BoundIsVisible(Box) )
						{
							STAT(GStat.NumRasterBoxReject++);
							goto NextCoplanar;
						}
					}
					if( (Poly->PolyFlags & PF_Portal)&&(iViewZone==0) ) goto NextCoplanar;

					// Rasterize it.
					WorkingRaster.SetupCached(Camera,Pts,NumPts,&GMem,&GDynMem,SideCache);
					WorkingRaster.Generate(GRaster.Raster);

					if( Poly->Texture ) PolyFlags |= Poly->Texture->PolyFlags;
					if( (PolyFlags & (PF_TwoSided | PF_Portal)) && !(Status & PSC_Front) )
						GRaster.Raster->ForceForwardFace();

					Mergeable = !
					(
						(PolyFlags & ( PF_NoOcclude|PF_NoMerge) )
					||	(Node->NodeFlags & NF_NoMerge)
					||	(GApp->RenDev && GApp->RenDev->Active)
					);
					Merging = 
					(
						(Mergeable && AllPolys[iSurf]!=INDEX_NONE))
					||	((PolyFlags&PF_Portal) && (Camera->RendMap!=REN_Zones)
					);
					if( !Merging )
					{
						if( Mergeable )
						{
							// Look up cached bounds to exploit frame-to-frame coherence.
							int GuessStartY = OurMax(0,          OurMin(GRaster.Raster->StartY,Poly->LastStartY-4));
							int GuessEndY	= OurMin(Camera->SYR,OurMax(GRaster.Raster->EndY,  Poly->LastEndY  +4));
							TopDrawList->Span.AllocIndex(GuessStartY,GuessEndY,&GDynMem);
						}
						else if( GApp->RenDev && GApp->RenDev->Active )
						{
							TopDrawList->Span.AllocIndex(GRaster.Raster->StartY,GRaster.Raster->EndY,&GMem);	
						}
						else TopDrawList->Span.AllocIndex(GRaster.Raster->StartY,GRaster.Raster->EndY,&GDynMem);
					}
					else
					{
						TopDrawList->Span.AllocIndex(GRaster.Raster->StartY,GRaster.Raster->EndY,&GMem);
					}

					if( !(PolyFlags&PF_NoOcclude) )	Visible = TopDrawList->Span.CopyFromRasterUpdate(*SpanBuffer,*GRaster.Raster);
					else							Visible = TopDrawList->Span.CopyFromRaster(*SpanBuffer,*GRaster.Raster);

					if ( Visible && (PolyFlags & PF_Portal) )
					{
						if( iOppositeZone!=0 )
						{
							FSpanBuffer *OppositeSpan = &ZoneSpanBuffer[iOppositeZone];
							if( OppositeSpan->ValidLines <= 0 )
							{
								ActiveZoneMask |= ((QWORD)1)<<iOppositeZone;
							}
							OppositeSpan->MergeWith(TopDrawList->Span);
						}
						if( Camera->RendMap==REN_Zones ) goto DrawNoMerge; // Actually display zone portals
					}
					else if( Visible && ((!(Camera->ShowFlags&SHOW_PlayerCtrl)) || !(PolyFlags & PF_Invisible)) )
					{
						// Compute Z range.
						MaxZ = MinZ = Pts[0].Z;
						for( i=1; i<NumPts; i++ )
						{
							if(			Pts[i].Z > MaxZ ) MaxZ = Pts[i].Z;
							else if(	Pts[i].Z < MinZ ) MinZ = Pts[i].Z;
						}
						if( ( PolyFlags & PF_FakeBackdrop ) && (Camera->ShowFlags&SHOW_PlayerCtrl) )
						{
							if (Backdrop) Backdrop->MergeWith(TopDrawList->Span);
						}
						else if( !Merging )
						{
							// Create new draw-list entry.
							DrawNoMerge:
							if( GApp->RenDev && GApp->RenDev->Active )
							{
								TopDrawList->NumPts = NumPts;
								TopDrawList->Pts    = (FTransform *)GDynMem.Get(NumPts * sizeof(FTransform));
								memcpy(TopDrawList->Pts,Pts,NumPts * sizeof(FTransform));

								TopDrawList->Span.Release();
							}
							AllPolys[Node->iSurf]   = NodeCount++;
							TopDrawList->iNode		= iNode;
							TopDrawList->iZone		= (Status & PSC_Front)?Node->iZone:Node->iBackZone;
							TopDrawList->iSurf		= iSurf;
							TopDrawList->PolyFlags	= PolyFlags;
							TopDrawList->MaxZ		= MaxZ;
							TopDrawList->MinZ		= MinZ;
							TopDrawList->Texture	= Poly->Texture;
							TopDrawList++;
							STAT(GStat.PolysDrawn++;)
						}
						else
						{
							// Add to existing draw-list entry:
							FBspDrawList *Existing = &DrawList[AllPolys[iSurf]];
							Existing->MaxZ = OurMax(MaxZ,Existing->MaxZ);
							Existing->MinZ = OurMin(MinZ,Existing->MinZ);
							Existing->Span.MergeWith(TopDrawList->Span);
							TopDrawList->Span.Release();
						}
						DrewStuff = 1;
						Node->NodeFlags &= ~NF_PolyOccluded;
						STAT(GStat.NodesDrawn++;)

						if( SpanBuffer->ValidLines <= 0 )
						{
							ActiveZoneMask &= ~(((QWORD)1)<<iZone);
							if (!ActiveZoneMask)
							{
								// Screen is now completely filled.
								BspNodes[iOriginalNode].NodeFlags &= ~NF_AllOccluded;
								while( --Stack >= &NodeStack[0])
								{
									BspNodes[Stack->iNode].NodeFlags &= ~NF_AllOccluded;
								}
								goto DoneRendering;
							}
						}
					}
					else
					{
						// Rejected, span buffer wasn't affected.
						Node->NodeFlags |= NF_PolyOccluded;
						TopDrawList->Span.Release();
					}
					WorkingRaster.Release(); // Releases unlinked raster setups in GMem, not linked raster setups in GDynMem

					NextCoplanar:
					if( Node->iPlane==INDEX_NONE ) break;

					iNode	= Node->iPlane;
					Node	= &BspNodes[iNode];
					Status	= GetPlaneStatus(Camera,ModelInfo,PlaneStatusCache,Origin,Node);
				}
				iNode  = iOriginalNode;
				Node   = &BspNodes[iNode];
				Status = GetPlaneStatus(Camera,ModelInfo,PlaneStatusCache,Origin,Node);
			}
			iThingZone = iViewZone?((Status&PSC_Front)?Node->iBackZone:Node->iZone):0;
			if( (Node->iDynamic[(Status&PSC_Front)?1:0]!=INDEX_NONE)
				&& ZoneSpanBuffer[iThingZone].ValidLines )
			{
				dynamicsPreRender
				(
					Camera,
					&ZoneSpanBuffer[iThingZone],
					iNode,
					(Status&PSC_Front) ? 1 : 0
				);
			}
			if( Stack->iFarNode != INDEX_NONE )
			{
				Stack->iNode		= iOriginalNode;
				Stack->Outside		= Outside;
				Stack->Pass			= PASS_BACK;
				Stack->DrewStuff    = DrewStuff;
				Stack++;

				iNode				= Stack[-1].iFarNode;
				Outside				= Stack[-1].FarOutside;
				Pass				= PASS_FRONT;
				DrewStuff			= 0;

				continue;
			}
			Pass  = PASS_BACK;
			iNode = iOriginalNode;
		}

		// Pass 3: Done processing all stuff at and below iNode in the tree, now update visibility information.
		// Assert: Pass==PASS_BACK.

		PrePopStack:
		Node = &BspNodes[iNode];
		if( !DrewStuff )	Node->NodeFlags |=  NF_AllOccluded;
		else				Node->NodeFlags &= ~NF_AllOccluded;

		// Return from recursion, noting that the node we're returning to is guaranteed visible if the
		// child we're processing now is visible.

		PopStack:
		if( --Stack < &NodeStack[0] ) break;
		iNode		= Stack->iNode;
		Outside		= Stack->Outside;
		Pass		= Stack->Pass;
		DrewStuff  |= Stack->DrewStuff;
		}
	DoneRendering:
	TopDrawList->iNode = INDEX_NONE; // Tag last entry.

	// Build backdrop by merging all of the non-empty parts of the remaining span buffers
	// together.
	if( Backdrop )
	{
		for (i=0; i<UBspNodes::MAX_ZONES; i++) if (ZoneSpanBuffer[i].ValidLines>0)
		{
			Backdrop->MergeWith(ZoneSpanBuffer[i]);
		}
	}

	// Update stats:
	STAT(GStat.NumZones=ModelInfo->NumZones);
	STAT(GStat.CurZone =iViewZone);
	for (i=0; i<UBspNodes::MAX_ZONES; i++) if (ZoneSpanBuffer[i].EndY) STAT(GStat.VisibleZones++);

	return NodeCount;

	UNGUARD("FGlobalRender::OccludeBsp");
}

/*-----------------------------------------------------------------------------
	Lattice builders
-----------------------------------------------------------------------------*/

//
// Lattice setup loop globals:
//
FLOAT	LSL_RZ,		LSL_RZGradSX,	LSL_RZGradSY;
FLOAT	LSL_URZ,	LSL_URZGradSX,	LSL_URZGradSY;
FLOAT	LSL_VRZ,	LSL_VRZGradSX,	LSL_VRZGradSY;
FLOAT	LSL_XRZ,	LSL_XRZInc;
FLOAT	LSL_YRZ,	LSL_YRZInc;
FLOAT	LSL_BaseU,	LSL_BaseV;
FLOAT	LSL_MipMult, LSL_RBaseNormal;
FVector	LSL_Normal,LSL_TextureU,LSL_TextureV,LSL_Base;

inline void LatticeSetupLoop( FTexLattice** LatticeBase,FTexLattice* TopLattice,int Start,int End )
{

	GUARD;
	FTexLattice *Original    = TopLattice;

#ifdef PARANOID
	if (End<=Start)appError("SpanLoop inconsistency 1");
#endif

#ifndef ASM_LATTICE
	FTexLattice **LatticePtr = &LatticeBase[Start];

	FLOAT StepIn	= (FLOAT)Start;
	FLOAT RZ		= LSL_RZ  + StepIn * LSL_RZGradSX;
	FLOAT URZ		= LSL_URZ + StepIn * LSL_URZGradSX;
	FLOAT VRZ		= LSL_VRZ + StepIn * LSL_VRZGradSX;
	FLOAT XRZ		= LSL_XRZ + StepIn * LSL_XRZInc;

	int n = End - Start;
	while (n-- > 0)
	{
		FLOAT Z						= 1.0/RZ;
		TopLattice->Loc.Z			= Z;
		TopLattice->Loc.X			= Z * XRZ;
		TopLattice->Loc.Y			= Z * LSL_YRZ;
		TopLattice->Loc.W			= Z * LSL_MipMult;
		*(FLOAT *)&TopLattice->U	= Z * URZ + LSL_BaseU;
		*(FLOAT *)&TopLattice->V	= Z * VRZ + LSL_BaseV;
		TopLattice->RoutineOfs		= 0;

		TopLattice->U				= TopLattice->U << 8;
		TopLattice->V				= TopLattice->V << 8;

		TopLattice->G = 0;

		RZ  += LSL_RZGradSX;
		URZ += LSL_URZGradSX;
		VRZ += LSL_VRZGradSX;
		XRZ += LSL_XRZInc;

		*LatticePtr++ = TopLattice++;
	}

#else
	static FLOAT StepIn;
	__asm
	{
		///////////////////////////////////////////////
		// Assembly language lattice span setup loop //
		///////////////////////////////////////////////

		mov ecx,[Start]
		mov ebx,[LatticeBase]

		mov eax,[End]
		mov edi,[TopLattice]

		lea ebx,[ebx + ecx*4]
		sub eax,ecx

		xor ecx,ecx

		// Load & perform step-in:
		fild [Start]			; Start
		fld  [LSL_YRZ]			; YRZ Start
		fxch					; Start YRZ
		fstp [StepIn]			; YRZ

		fld  [LSL_VRZGradSX]	; VRZGradSX YRZ
		fmul [StepIn]			; VRZGradSX' YRZ

		fld  [LSL_URZGradSX]	; URZGradSX VRZGradSX' YRZ
		fmul [StepIn]			; URZGradSX' VRZGradSX' YRZ

		fld  [LSL_RZGradSX]		; RZGradSX URZGradSX' VRZGradSX' YRZ
		fmul [StepIn]			; RZGradSX' URZGradSX' VRZGradSX' YRZ

		fld  [LSL_XRZInc]		; XRZInc RZGradSX' URZGradSX' VRZGradSX' YRZ
		fmul [StepIn]			; XRZInc' RZGradSX' URZGradSX' VRZGradSX' YRZ

		fxch st(3)				; VRZGradSX' RZGradSX' URZGradSX' XRZInc' YRZ
		fadd [LSL_VRZ]			; VRZ+VRZGradSX' RZGradSX' URZGradSX' XRZInc' YRZ

		fxch st(2)				; URZGradSX' RZGradSX' VRZ+VRZGradSX' XRZInc' YRZ
		fadd [LSL_URZ]			; URZ+URZGradSX' RZGradSX' VRZ+VRZGradSX' XRZInc' YRZ

		fxch st(1)				; RZGradSX' URZ+URZGradSX' VRZ+VRZGradSX' XRZInc' YRZ
		fadd [LSL_RZ]			; RZ+RZGradSX' URZ+URZGradSX' VRZ+VRZGradSX' XRZInc' YRZ

		fxch st(3)				; XRZInc' URZ+URZGradSX' VRZ+VRZGradSX' RZ+RZGradSX' YRZ
		fadd [LSL_XRZ]			; X+XRZInc' URZ+URZGradSX' VRZ+VRZGradSX' RZ+RZGradSX' YRZ

		fxch st(3)				; RZ+RZGradSX' URZ+URZGradSX' VRZ+VRZGradSX' XRZ+XRZInc' YRZ

		fld1					; 1 RZ+RZGradSX' URZ+URZGradSX' VRZ+VRZGradSX' XRZ+XRZInc' YRZ
		fdiv st,st(1)			; 1/RZ+RZGradSX' RZ+RZGradSX' URZ+URZGradSX' VRZ+VRZGradSX' XRZ+XRZInc' YRZ

		// Loop:
		ALIGN 16
		MainLoop:

		; TopLattice->U = URZ * Z + BaseU;
		; TopLattice->V = VRZ * Z + BaseV;
		; URZ += URZGradSX;
		; VRZ += VRZGradSX;
		; RZ  += RZGradSX;
		
		;							;   st(0)   st(1)   st(2)   st(3)   st(4)   st(5)   st(6)   st(7)
		;							;   ------- ------- ------- ------- ------- ------- ------- -------
		fst [edi]TopLattice.Loc.Z	;   Z       RZ      URZ     VRZ		XRZ		YRZ
		fld st(0)					;   Z		Z       RZ      URZ     VRZ		XRZ		YRZ
		fmul [LSL_MipMult]			;   Z*Mip	Z       RZ      URZ     VRZ		XRZ		YRZ
		fld  st(3)					;-  URZ     Z*MipZ	Z       RZ      URZ     VRZ		XRZ		YRZ
		fmul st,st(2)				;-  URZ*Z   Z*MipZ	Z       RZ      URZ     VRZ		XRZ		YRZ
		fxch st(1)					;   Z*MipZ	URZ*Z   Z       RZ      URZ     VRZ		XRZ		YRZ
		fstp [edi]TopLattice.Loc.W	;   URZ*Z   Z       RZ      URZ     VRZ		XRZ		YRZ

		fld  st(4)					;-  VRZ     URZ*Z   Z       RZ      URZ     VRZ		XRZ		YRZ
		fmul st,st(2)				;-  VRZ*Z   URZ*Z   Z       RZ      URZ     VRZ		XRZ		YRZ
		fxch st(3)					;   RZ      URZ*Z   Z       VRZ*Z   URZ     VRZ		XRZ		YRZ
		fadd [LSL_RZGradSX]			;-  RZ'     URZ*Z   Z       VRZ*Z   URZ     VRZ		XRZ		YRZ
		fxch st(1)					;   URZ*Z   RZ      Z       VRZ*Z   URZ     VRZ		XRZ		YRZ
		fadd [LSL_BaseU]			;-  URZ*Z+B RZ      Z       VRZ*Z   URZ	    VRZ		XRZ		YRZ
		fxch st(3)					;   VRZ*Z   RZ      Z       URZ*Z+B URZ     VRZ		XRZ		YRZ
		fadd [LSL_BaseV]			;-  VRZ*Z+B RZ      Z       URZ*Z+B URZ     VRZ		XRZ		YRZ
		fxch st(5)					;   VRZ     RZ      Z       URZ*Z+B URZ     VRZ*Z+B	XRZ		YRZ
		fadd [LSL_VRZGradSX]		;-  VRZ'    RZ      Z       URZ*Z+B URZ     VRZ*Z+B	XRZ		YRZ
		fxch st(4)					;   URZ     RZ      Z       URZ*Z+B VRZ     VRZ*Z+B	XRZ		YRZ
		fadd [LSL_URZGradSX]		;-  URZ'    RZ      Z       URZ*Z+B VRZ     VRZ*Z+B	XRZ		YRZ
		fxch st(3)					;   URZ*Z+B RZ      Z       URZ     VRZ     VRZ*Z+B	XRZ		YRZ
		fstp [edi]FTexLattice.U		;-  RZ      Z       URZ     VRZ     VRZ*Z+B	XRZ		YRZ
		fxch st(4)					;   VRZ*Z+B Z       URZ     VRZ     RZ		XRZ		YRZ
		fstp [edi]FTexLattice.V		;-  Z       URZ     VRZ     RZ		XRZ		YRZ
		fxch st(3)					;   RZ      URZ     VRZ		Z		XRZ		YRZ
		fld  st(4)					;-  XRZ		RZ      URZ     VRZ		Z		XRZ		YRZ
		fadd [LSL_XRZInc]			;-  XRZ'	RZ      URZ     VRZ		Z		XRZ		YRZ

		fxch st(4)					;   Z		RZ      URZ     VRZ		XRZ'	XRZ		YRZ
		fmul st(5),st				;-	Z		RZ      URZ     VRZ		XRZ'	XRZ*Z	YRZ
		fld  st(6)					;-	YRZ		Z		RZ      URZ     VRZ		XRZ'	XRZ*Z	YRZ
		fxch st(1)					;	Z		YRZ		RZ      URZ     VRZ		XRZ'	XRZ*Z	YRZ
		fmul st,st(7)				;-	YRZ*Z	YRZ		RZ      URZ     VRZ		XRZ'	XRZ*Z	YRZ
		fxch st(6)					;	XRZ*Z	YRZ		RZ      URZ     VRZ		XRZ'	YRZ*Z	YRZ
		fstp [edi]TopLattice.Loc.X	;-	YRZ		RZ      URZ     VRZ		XRZ'	YRZ*Z	YRZ
		fxch st(5)					;	YRZ*Z	RZ      URZ     VRZ		XRZ'	YRZ		YRZ
		fstp [edi]TopLattice.Loc.Y	;-	RZ      URZ     VRZ		XRZ'	YRZ		YRZ

		fld1						;-  1       RZ      URZ     VRZ		XRZ		YRZ		YRZ
		ffree st(6)					; Discard
		fdiv st,st(1)				;-  Z'      RZ      URZ     VRZ		XRZ		YRZ

		// Fix up lattice values:
		mov edx,[edi]FTexLattice.U
		mov esi,[edi]FTexLattice.V

		shl edx,8
		mov [edi]FTexLattice.G,ecx ; ecx=0

		shl esi,8
		mov [edi]FTexLattice.U,edx

		mov [edi]FTexLattice.RoutineOfs,ecx ; ecx=0
		mov [edi]FTexLattice.V,esi

		// End of loop:
		mov [ebx],edi
		add ebx,4
		add edi,SIZE FTexLattice

		dec eax
		jg  MainLoop

		// Done:
		fcompp	; Pop 6 registers in 3 cycles
		fcompp
		fcompp
	}
#endif

	UNGUARD("SpanLoop");
}

// Big wavy effect
void BigWavy( ICamera* Camera, FTexLattice* TopLattice, int Num )
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

// Small wavy effect
void SmallWavy( ICamera* Camera, FTexLattice* TopLattice, int Num )
{
	while ( Num-- > 0 )
	{
		FVector Temp = TopLattice->Loc;
		Temp.TransformVector(Camera->Uncoords);
		Temp += Camera->Coords.Origin;
		TopLattice->U += 65536.0 * 4.0 * cos(Temp.X * 0.03 + GServer.Ticks * 0.12);
		TopLattice->V += 65536.0 * 4.0 * sin(Temp.Y * 0.03 + GServer.Ticks * 0.17);
		TopLattice++;
	}
}

void inline RectLoop(FTexLattice **LatticeBase,int Start, int End)
	{
#ifdef ASM_LATTICE
	__asm
		{
		pushad					; Save regs
		mov edi,[LatticeBase]	; Get lattice base pointer
		mov esi,[Start]			; Get start
		mov ebp,[End]			; Get end
		;
		call TRL_RectLoop		; Setup
		;
		popad					; Restore regs
		};
#else
	GUARD;
	//
	FTexLattice **LatticePtr = &LatticeBase[Start];
	while (Start++ < End)
		{
		FTexLattice *T0		= LatticePtr[0];
		FTexLattice *T1		= LatticePtr[1];
		FTexLattice *B0		= LatticePtr[MAX_XR];
		FTexLattice *B1		= LatticePtr[MAX_XR+1];
		//
		FMipTable *T = &TRL_MipTable[*(DWORD *)&T0->Loc.W >> 21];
		//
		T0->RoutineOfs = T->RoutineOfs + TRL_RoutineOfsEffectBase;
		//
		GBlit.MipRef[T->MipLevel  ] = 1;
		GBlit.MipRef[T->MipLevel+1] = 1;
		//
		FBlitMipInfo *Mip	= &GBlit.Mips[OurMax(T->MipLevel,T->MipLevel)];
		INT  VMask			= Mip->VMask;
		INT  NotVMask		= ~VMask;
		//
		BYTE GP			    = GBlit.LatticeXBits;
		BYTE GL			    = GBlit.LatticeYBits;
		BYTE GM			    = T->MipLevel;
		BYTE GMU		    = GM + Mip->UBits;
		//
		// Lattice setup:
		//
		int B_IU,B_IUX,B_IUY,B_IUXY;
		int B_IV,B_IVX,B_IVY,B_IVXY;
		//
		B_IU			= (T0->U + TRL_TexBaseU         ) >> GMU;
		B_IUX			= (T1->U - T0->U                ) >> GMU;
		B_IUY			= (B0->U - T0->U                ) >> GMU;
		B_IUXY			= (T0->U - T1->U - B0->U + B1->U) >> GMU;
		//
		B_IV			= (T0->V + TRL_TexBaseV         ) >> GM;
		B_IVX			= (T1->V - T0->V                ) >> GM;
		B_IVY			= (B0->V - T0->V                ) >> GM;
		B_IVXY			= (T0->V - T1->V - B0->V + B1->V) >> GM;
		//
		T0->L			= (B_IV   << (16      )&0xffff0000);
		T0->LY			= (B_IVY  << (16-GL   )&0xffff0000);
		T0->LX			= (B_IVX  << (16-GP   )&0xffff0000);
		T0->LXY			= (B_IVXY << (16-GP-GL)&0xffff0000);
		//
		T0->H			= ((B_IU   << (16      ))&NotVMask  )+(((B_IV   >> (16      )))&VMask);
		T0->HY			= ((B_IUY  << (16-GL   ))&NotVMask  )+(((B_IVY  >> (16+   GL)))&VMask);
		T0->HX			= ((B_IUX  << (16-GP   ))&NotVMask  )+(((B_IVX  >> (16+GP   )))&VMask);
		T0->HXY			= ((B_IUXY << (16-GP-GL))&NotVMask  )+(((B_IVXY >> (16+GP+GL)))&VMask);
		//
		// Sublattice setup:
		//
#if 0 // todo: restore?
		if (GRender.LightList.Index)
			{
			int B_IG, B_IGX, B_IGY, B_IGXY;

			INT MeshBaseU	= GRender.LightList.Index->TextureUStart;
			INT MeshBaseV	= GRender.LightList.Index->TextureVStart;
			VMask			= (1 << GRender.LightList.MeshVBits) - 1;
			NotVMask		= ~VMask;
			//
			GP				= GBlit.InterXBits;
			GL				= GBlit.InterYBits;
			GM				= GRender.LightList.Index->MeshShift;
			GMU				= GM + GRender.LightList.MeshUBits;
			//
			// Can conditionally skip parts if GP=0 or GM=0 (which can occur a lot)
			//
			B_IU			= (T0->U - MeshBaseU - FIX(16)   ) >> GMU;
			B_IUX			= (T1->U - T0->U                 ) >> GMU;
			B_IUY			= (B0->U - T0->U                 ) >> GMU;
			B_IUXY			= (T0->U - T1->U - B0->U + B1->U ) >> GMU;
			//
			B_IV			= (T0->V - MeshBaseV + FIX(16)   ) >> GM;
			B_IVX			= (T1->V - T0->V                 ) >> GM;
			B_IVY			= (B0->V - T0->V                 ) >> GM;
			B_IVXY			= (T0->V - T1->V - B0->V + B1->V ) >> GM;
			//
			B_IG			= (T0->G                         );
			B_IGX			= (T1->G - T0->G                 );
			B_IGY			= (B0->G - T0->G                 );
			B_IGXY			= (T0->G - T1->G - B0->G + B1->G );
			//
			T0->SubL		= (((B_IG                ))&0x0000ffff)+((B_IV   << (16      ))&0xffff0000);
			T0->SubLY		= (((B_IGY  >> (      GL)))&0x0000ffff)+((B_IVY  << (16-GL   ))&0xffff0000);
			T0->SubLX		= (((B_IGX  >> (   GP   )))&0x0000ffff)+((B_IVX  << (16-GP   ))&0xffff0000);
			T0->SubLXY		= (((B_IGXY >> (   GP+GL)))&0x0000ffff)+((B_IVXY << (16-GP-GL))&0xffff0000);
			//
			T0->SubH		= (((B_IV   >> (16      )))&VMask     )+((B_IU   << (16      ))&NotVMask  );
			T0->SubHY		= (((B_IVY  >> (16+   GL)))&VMask     )+((B_IUY  << (16-GL   ))&NotVMask  );
			T0->SubHX		= (((B_IVX  >> (16+GP   )))&VMask     )+((B_IUX  << (16-GP   ))&NotVMask  );
			T0->SubHXY		= (((B_IVXY >> (16+GP+GL)))&VMask     )+((B_IUXY << (16-GP-GL))&NotVMask  );
			};
#endif
		LatticePtr++;
		};
	UNGUARD("RectLoop");
#endif
	};

/*-----------------------------------------------------------------------------
	Software textured Bsp surface rendering
-----------------------------------------------------------------------------*/

//
// Draw a software-textured Bsp surface.
//
void DrawSoftwareTexturedBspSurf ( ICamera* Camera, FBspDrawList* Draw )
{
	GUARD;

	ILevel		*Level		= &Camera->Level;
	IModel		*ModelInfo	= &Level->ModelInfo;
	FCoords		*Coords		= &Camera->Coords;

	FTexLattice	**LatticeBase,*FirstLattice,*TopLattice;
	FSpanBuffer	SubRectSpan,SubLatticeSpan,RectSpan,LatticeSpan;
	FSpan		*Span,**SpanIndex;
	FLOAT		FSXR2,FSYR2,RProjZ;
	INT			iLightMesh,Sampled,PolyFlags,YR,LatticeEffects;

	for ( int i=0; i<8; i++ ) 
		GBlit.MipRef[i]=0;

	if ( Draw->iSurf!=INDEX_NONE )
	{
		FBspSurf *Surf	= &ModelInfo->BspSurfs[Draw->iSurf];
		GBlit.Texture	= Surf->Texture ? Surf->Texture : GGfx.DefaultTexture;
		GBlit.iZone		= Draw->iZone;

		PolyFlags		= Surf->PolyFlags;
		iLightMesh		= Surf->iLightMesh;

		// Compute largest lattice size required to capture the essence of this Bsp surface's
		// perspective correction and special effects.

		GBlit.LatticeXBits	= 7;
		GBlit.LatticeYBits	= 6;

		FVector	*Normal     = GRender.GetVector(ModelInfo,Coords,Surf->vNormal);
		FVector	*Base     	= GRender.GetPoint(ModelInfo,Camera,Surf->pBase);
		LSL_XRZInc			= Camera->RProjZ * (FLOAT)(1 << GBlit.LatticeXBits);
		LSL_YRZInc			= Camera->RProjZ * (FLOAT)(1 << GBlit.LatticeYBits);
		LSL_RBaseNormal		= 1.0/(*Normal | *Base);
		LSL_RZGradSX		= LSL_XRZInc * Normal->X * LSL_RBaseNormal;
		LSL_RZGradSY   		= LSL_YRZInc * Normal->Y * LSL_RBaseNormal;

		FLOAT RZGradSX		= OurAbs(LSL_RZGradSX);
		FLOAT RZGradSY		= OurAbs(LSL_RZGradSY);

		// Shrink lattice vertically as necessary to mainain the illusion of perspective correctness.
		static const FLOAT LineThresh[8] = { 0.0f,1.80f,1.50f,1.00f,0.60f,0.36f,0.20f,0.08f };
		while ( GBlit.LatticeYBits>0 )
		{
			FLOAT Factor1   = Draw->MaxZ * RZGradSY;
			FLOAT Factor2   = Draw->MinZ * RZGradSY;
			GBlit.LatticeY	= 1<<GBlit.LatticeYBits;

			if ( (GBlit.LatticeY*Factor1/(4.0+2.0/Factor1)<LineThresh[GBlit.LatticeYBits])
			&&	(GBlit.LatticeY*Factor2/(4.0+2.0/Factor2)<LineThresh[GBlit.LatticeYBits]) )
				break;
			RZGradSY *= 0.5;
			GBlit.LatticeYBits--;
		}

		// Shrink lattice horizontally as necessary to mainain the illusion of perspective correctness.
		static const FLOAT PixelThresh[8]={0.0f,0.0f,0.0f,1.5f,0.80f,0.45f,0.28f,0.06f}; // 0.50
		while ( GBlit.LatticeXBits>2 )
		{
			FLOAT Factor1      = Draw->MaxZ * RZGradSX;
			FLOAT Factor2      = Draw->MinZ * RZGradSX;
			GBlit.LatticeX	   = 1<<GBlit.LatticeXBits;

			if ( (GBlit.LatticeX*Factor1/(4.0+2.0/Factor1)<PixelThresh[GBlit.LatticeXBits])
			&&	(GBlit.LatticeX*Factor2/(4.0+2.0/Factor2)<PixelThresh[GBlit.LatticeXBits]) )
				break;
			RZGradSX *= 0.5;
			GBlit.LatticeXBits--;
		}
		PolyFlags		= Surf->PolyFlags;
		iLightMesh		= Surf->iLightMesh;

		LSL_TextureU 	= *GRender.GetVector (ModelInfo,Coords,Surf->vTextureU);
		LSL_TextureV 	= *GRender.GetVector (ModelInfo,Coords,Surf->vTextureV);
		LSL_Normal   	= *GRender.GetVector (ModelInfo,Coords,Surf->vNormal  );
		LSL_Base     	= *GRender.GetPoint  (ModelInfo,Camera,Surf->pBase    );

		TRL_TexBaseU	= FIX(Surf->PanU);
		TRL_TexBaseV	= FIX(Surf->PanV);
	}
	else
	{
		GBlit.LatticeXBits = 4;
		GBlit.LatticeYBits  = 2;

		PolyFlags		= 0;
		iLightMesh		= INDEX_NONE;

		LSL_TextureU 	= GMath.XAxisVector*16384.0;	LSL_TextureU.TransformVector(*Coords);
		LSL_TextureV 	= GMath.YAxisVector*16384.0;	LSL_TextureV.TransformVector(*Coords);
		LSL_Normal   	= GMath.ZAxisVector;			LSL_Normal.TransformVector(*Coords);
		LSL_Base     	= GMath.ZAxisVector*384.0;		LSL_Base.TransformVector(*Coords);

		TRL_TexBaseU	= 0;
		TRL_TexBaseV	= 0;

		GBlit.Texture = GGfx.BackdropTexture;
		GBlit.iZone   = 0;
	}

	// Far-ceiling effect.
	if ( (PolyFlags & PF_FarCeiling) && (Camera->ShowFlags&SHOW_PlayerCtrl) ) 
		LSL_Base -= LSL_Normal * 512.0;

	// Setup dynamic lighting, required before lattice size is finalized.
	GLightManager->Index = NULL;
	if ( (Camera->RendMap==REN_DynLight) && (iLightMesh!=INDEX_NONE) && !(PolyFlags & PF_Unlit) )
	{
		Sampled = 1;
		GLightManager->SetupForPoly(Camera,LSL_Normal,LSL_Base,Draw->iSurf);
		LatticeEffects = GLightManager->LatticeEffects;
	}
	else
	{
		GLightManager->SetupForNothing(Camera);
		Sampled = 0;
		LatticeEffects = 0;
	}
	if( PolyFlags & PF_BigWavy )
	{
		LatticeEffects = 1;
		GLightManager->MaxXBits = OurMin(GLightManager->MaxXBits,5);
		GLightManager->MaxYBits = OurMin(GLightManager->MaxYBits,3);
	}
	if( PolyFlags & PF_SmallWavy )
	{
		LatticeEffects = 1;
		GLightManager->MaxXBits = OurMin(GLightManager->MaxXBits,4);
		GLightManager->MaxYBits = OurMin(GLightManager->MaxYBits,3);
	}

	// Clamp lattice size to the min/max specified by lighting code:
	GBlit.LatticeXBits = OurClamp((int)GBlit.LatticeXBits,GLightManager->MinXBits,GLightManager->MaxXBits);
	GBlit.LatticeYBits = OurClamp((int)GBlit.LatticeYBits,GLightManager->MinYBits,GLightManager->MaxYBits);

	// Keep precision under control.
	while ( GBlit.LatticeXBits + GBlit.LatticeYBits > 12 )
	{
		if ( GBlit.LatticeXBits >= GBlit.LatticeYBits )	GBlit.LatticeXBits--;
		else											GBlit.LatticeYBits--;
	}
	if ( Camera->SXR<420 && GBlit.LatticeYBits>0 ) GBlit.LatticeYBits--;

	// Prevent from being larger than storage will allow.
	while ( (Camera->SXR >> GBlit.LatticeXBits) > (MAX_XR-2) )	GBlit.LatticeXBits++;
	while ( (Camera->SYR >> GBlit.LatticeYBits) > (MAX_YR-2) )	GBlit.LatticeYBits++;

	// Prevent from being larger than precision will reasonably allow.
	if ( GBlit.LatticeXBits>7 ) GBlit.LatticeXBits=7;
	if ( GBlit.LatticeYBits>7 ) GBlit.LatticeYBits=7;

	GBlit.LatticeX			= 1 << GBlit.LatticeXBits;
	GBlit.LatticeY			= 1 << GBlit.LatticeYBits;
	GBlit.LatticeXMask		= (GBlit.LatticeX-1);
	GBlit.LatticeXMask4		= (GBlit.LatticeX-1) & ~3;
	GBlit.LatticeXNotMask	= ~(GBlit.LatticeX-1);

	// Compute sublattice size needed for mesh lighting.
	GBlit.SubXBits = OurMin(GBlit.LatticeXBits,(BYTE)2);
	GBlit.SubYBits = OurClamp(GBlit.LatticeYBits-2,0,(Camera->SXR>400) ? 2 : 1);

	GBlit.SubX = 1 << GBlit.SubXBits;
	GBlit.SubY = 1 << GBlit.SubYBits;

	// Compute interlattice.
	GBlit.InterXBits		= GBlit.LatticeXBits - GBlit.SubXBits;
	GBlit.InterYBits		= GBlit.LatticeYBits - GBlit.SubYBits;
	GBlit.InterX			= 1 << GBlit.InterXBits;
	GBlit.InterY			= 1 << GBlit.InterYBits;
	GBlit.InterXMask		= (GBlit.InterX-1);
	GBlit.InterXNotMask		= ~(GBlit.InterX-1);

	// Set up texture mapping.
	rendDrawAcrossSetup(Camera,GBlit.Texture,PolyFlags,0);

	// Compute lattice hierarchy.
	if (!SubRectSpan.CalcRectFrom	(Draw->Span,GBlit.SubXBits,GBlit.SubYBits,&GMem)) return;
	SubLatticeSpan.CalcLatticeFrom	(SubRectSpan,&GMem);

	if (!RectSpan.CalcRectFrom		(SubRectSpan,GBlit.InterXBits,GBlit.InterYBits,&GMem)) return;
	LatticeSpan.CalcLatticeFrom		(RectSpan,&GMem);

	FirstLattice	= (FTexLattice *)GMem.Get(0);
	TopLattice		= FirstLattice;

	// Setup magic numbers.
	FSXR2  			= Camera->FSXR2;
	FSYR2  			= Camera->FSYR2;
	RProjZ 			= Camera->RProjZ;

	LSL_XRZ    		= (1.0 - FSXR2) * RProjZ; // 1.0 is subpixel adjustment based on rasterizer convention
	LSL_YRZ 		= (LatticeSpan.StartY * GBlit.LatticeY + 1.0 - FSYR2) * RProjZ;

	LSL_XRZInc		= RProjZ * (FLOAT)GBlit.LatticeX;
	LSL_YRZInc		= RProjZ * (FLOAT)GBlit.LatticeY;

	LSL_RBaseNormal	= 1.0 / (LSL_Normal | LSL_Base);

	LSL_MipMult = (2.8/Camera->SXR) * (0.5 / 65536.0) * sqrt
		(
		(OurSquare(0.75) + OurSquare(RProjZ * FSXR2 * LSL_RBaseNormal * 75.0)) *
		(LSL_TextureU.SizeSquared() + LSL_TextureV.SizeSquared())
		);
	//LSL_MipMult = 1.0/Camera->SXR; // To disable angular mip adjustment.
	//if (Camera->SXR > 420) LSL_MipMult *= 1.4; // To adapt mipmapping to resolution
	LSL_MipMult *= 1.4f;

	if ( (PolyFlags & (PF_AutoUPan | PF_AutoVPan)) && (Camera->ShowFlags&SHOW_PlayerCtrl) )
	{
		if ( PolyFlags & PF_AutoUPan ) TRL_TexBaseU += FIX(GServer.Ticks);
		if ( PolyFlags & PF_AutoVPan ) TRL_TexBaseV += FIX(GServer.Ticks);
	}
	LSL_RZGradSX   	= LSL_XRZInc * LSL_Normal.X * LSL_RBaseNormal;	
	LSL_RZGradSY	= LSL_YRZInc * LSL_Normal.Y * LSL_RBaseNormal;
	LSL_URZGradSX  	= LSL_XRZInc * LSL_TextureU.X;	
	LSL_URZGradSY	= LSL_YRZInc * LSL_TextureU.Y;
	LSL_VRZGradSX  	= LSL_XRZInc * LSL_TextureV.X;	
	LSL_VRZGradSY	= LSL_YRZInc * LSL_TextureV.Y;

	LSL_RZ     		= (LSL_Normal  .X * LSL_XRZ + LSL_Normal  .Y * LSL_YRZ + LSL_Normal  .Z) * LSL_RBaseNormal;
	LSL_URZ    		= (LSL_TextureU.X * LSL_XRZ + LSL_TextureU.Y * LSL_YRZ + LSL_TextureU.Z);
	LSL_VRZ    		= (LSL_TextureV.X * LSL_XRZ + LSL_TextureV.Y * LSL_YRZ + LSL_TextureV.Z);

	LSL_BaseU = (FLOAT)0xC0000000 - (LSL_TextureU | LSL_Base);
	LSL_BaseV = (FLOAT)0xC0000000 - (LSL_TextureV | LSL_Base);

#ifdef ASM_LATTICE
	TRL_RoutineOfsEffectBase = 0;
	if (GLightManager->Index)
	{
		TRL_LightVMask      = (DWORD)0xffffffff >> (32-GLightManager->MeshVBits);
		TRL_LightBaseU		= -GLightManager->Index->TextureUStart;
		TRL_LightBaseV		= -GLightManager->Index->TextureVStart;
		TRL_LightMeshShift	= GLightManager->Index->MeshShift;
		TRL_LightMeshUShift	= GLightManager->Index->MeshShift + GLightManager->MeshUBits;
	}
	TRL_MipRef	= &GBlit.MipRef[0];
	__asm
	{
		mov al,[GBlit]GBlit.LatticeXBits
		mov bl,[GBlit]GBlit.LatticeYBits
		mov cl,[GBlit]GBlit.InterXBits
		mov dl,[GBlit]GBlit.InterYBits
		call TRL_SelfModRect
	}
#endif

	// Interpolate affine values RZ, URZ, VRZ, XRZ, YRZ.
	BEGINTIME(CalcLattice);
	if		(GBlit.Texture->MipOfs[1]==MAXDWORD)	TRL_MipTable = GNoMipTable;
	else if (GBlit.VBits<=8)						TRL_MipTable = GSmallMipTable;
	else											TRL_MipTable = GLargeMipTable;

	SpanIndex	= &LatticeSpan.Index [0];
	LatticeBase	= &GRender.LatticePtr[LatticeSpan.StartY+1][1];
	for ( YR=LatticeSpan.StartY; YR<LatticeSpan.EndY; YR++ )
	{
		Span = *SpanIndex++;
		while ( Span )
		{
#ifdef STATS
			int Len = Span->End-Span->Start;
			if (GLightManager->Index) GStat.LatLightsCalc += Len * (
				GLightManager->Index->NumStaticLights +
				GLightManager->Index->NumDynamicLights);
			GStat.LatsMade += Len;
#endif
			LatticeSetupLoop( LatticeBase, TopLattice, Span->Start, Span->End );

			if( LatticeEffects )
			{
				if( GLightManager->LatticeEffects	) GLightManager->ApplyLatticeEffects( TopLattice, Span->End - Span->Start );
				if( PolyFlags & PF_SmallWavy		) SmallWavy( Camera, TopLattice, Span->End - Span->Start );
				if( PolyFlags & PF_BigWavy			) BigWavy  ( Camera, TopLattice, Span->End - Span->Start );
			}
			TopLattice += Span->End - Span->Start;
			Span        = Span->Next;
		}
		LSL_RZ 		+= LSL_RZGradSY;
		LSL_URZ		+= LSL_URZGradSY;
		LSL_VRZ		+= LSL_VRZGradSY;
		LSL_YRZ		+= LSL_YRZInc;
		LatticeBase	+= MAX_XR;
	}

	SpanIndex	= &RectSpan.Index[0];
	LatticeBase	= &GRender.LatticePtr[RectSpan.StartY+1][1];
	for ( YR=RectSpan.StartY; YR<RectSpan.EndY; YR++ )
	{
		Span = *SpanIndex++;
		while ( Span )
		{
			RectLoop(LatticeBase,Span->Start,Span->End);
			Span = Span->Next;
		}
		LatticeBase += MAX_XR;
	}
	GMem.GetFast( (int)TopLattice-(int)FirstLattice );

	ENDTIME( CalcLattice );

	// Draw it.
	rendDrawAcross( Camera,&Draw->Span,&RectSpan,&LatticeSpan,&SubRectSpan,&SubLatticeSpan );

	// Clean up.
	if ( GRender.ShowLattice ) GRender.DrawLatticeGrid( Camera,&LatticeSpan );
	GRender.CleanupLattice( LatticeSpan );
	if (Sampled) GLightManager->ReleaseLightBlock();

	UNGUARD_BEGIN;
	UNGUARD_MSGF("MakeSurf (%s %ix%i)",GBlit.Texture->Name,GBlit.Texture->USize,GBlit.Texture->VSize);
	UNGUARD_END;
}

/*-----------------------------------------------------------------------------
	Drawers
-----------------------------------------------------------------------------*/

//
// Draw the lattice grid, for lattice debugging.
//
void FGlobalRender::DrawLatticeGrid(ICamera *Camera,FSpanBuffer *LatticeSpan)
	{
	GUARD;
	//
	int Y         = LatticeSpan->StartY * GBlit.LatticeY;
	BYTE *Dest1   = &Camera->Screen[Y * Camera->SXStride];
	FSpan **Index = &LatticeSpan->Index[0];
	for (int i=LatticeSpan->StartY; i<LatticeSpan->EndY; i++)
		{
		if (Y>=Camera->SYR) break;
		FSpan *Span = *Index++;
		while (Span)
			{
			BYTE *Dest = Dest1 + Span->Start * GBlit.LatticeX;
			for (int j=Span->Start; j<Span->End; j++)
				{
				*Dest  = BrushWireColor;
				Dest  += GBlit.LatticeX;
				};
			Span = Span->Next;
			};
		Y     += GBlit.LatticeY;
		Dest1 += Camera->SXStride * GBlit.LatticeY;
		};
	UNGUARD("FGlobalRender::DrawLatticeGrid");
	};

//
// Draw a flatshaded poly.
//
void FGlobalRender::DrawFlatPoly (ICamera *Camera,FSpanBuffer *SpanBuffer, BYTE Color)
	{
	GUARD;
	FSpan			*Span,**Index;
	int				m,n;
	//
	m     = SpanBuffer->EndY - SpanBuffer->StartY;
	Index = &SpanBuffer->Index [0];
	//
	if (Camera->ColorBytes==1)
		{
		BYTE *Line = &Camera->Screen [SpanBuffer->StartY * Camera->SXStride];
		while (m-- > 0)
			{
			Span = *Index++;
			while (Span)
				{
				mymemset(&Line[Span->Start],Color,Span->End - Span->Start);
				Span   = Span->Next;
				};
			Line += Camera->SXStride;
			};
		}
	else if (Camera->ColorBytes==2)
		{
		WORD *Screen,*Line,HiColor;
		//
		if (Camera->Caps & CC_RGB565)	HiColor = GGfx.DefaultColors[Color].HiColor565();
		else							HiColor = GGfx.DefaultColors[Color].HiColor555();
		//
		Line = &((WORD *)Camera->Screen)[SpanBuffer->StartY * Camera->SXStride];
		while (m-- > 0)
			{
			Span = *Index++;
			while (Span)
				{
				Screen = &Line[Span->Start];
				n      = Span->End - Span->Start;
				while (n-- > 0) *Screen++ = HiColor;
				Span = Span->Next;
				};
			Line += Camera->SXStride;
			};
		}
	else
		{
		DWORD TrueColor = GGfx.TrueColors[Color].D;
		DWORD *Screen,*Line;
		//
		Line = &((DWORD *)Camera->Screen)[SpanBuffer->StartY * Camera->SXStride];
		while (m-- > 0)
			{
			Span = *Index++;
			while (Span)
				{
				Screen = &Line[Span->Start];
				n      = Span->End - Span->Start;
				while (n-- > 0) *Screen++ = TrueColor;
				Span = Span->Next;
				};
			Line += Camera->SXStride;
			};
		};
	UNGUARD("FGlobalRender::DrawFlatPoly");
	};

//
// Draw a raster side.
//
void FGlobalRender::DrawRasterSide (BYTE *Line,int SXStride,FRasterSideSetup *Side,BYTE Color)
	{
	BYTE *Temp;
	INT  FixX,FixDX,YL,L;
	//
	while (Side)
		{
		FixX  = Side->P.X;
		FixDX = Side->DP.X;
		YL    = Side->DY;
		//
		if ((FixDX>=-65535) && (FixDX<=65535)) // Vertical major
			{
			while (YL-- > 0)
				{
				Line[UNFIX(FixX)]  = Color;
				Line              += SXStride;
				FixX              += FixDX;
				}
			}
		else if (FixDX>0) // Horizontal major, positive slope
			{
			while (YL-- > 0)
				{
				L    = UNFIX(FixX+FixDX) - UNFIX(FixX);
				Temp = &Line[UNFIX(FixX)];
				while (L-- > 0) *Temp++ = Color;
				FixX += FixDX;
				Line += SXStride;
				};
			}
		else // Horizontal major, negative slope
			{
			while (YL-- > 0)
				{
				L    = UNFIX(FixX) - UNFIX(FixX+FixDX);
				Temp = &Line[UNFIX(FixX+FixDX+0xffff)];
				while (L-- > 0) *Temp++ = Color;
				FixX += FixDX;
				Line += SXStride;
				};
			};
		Side = Side->Next;
		};
	};

//
// Draw a raster outline.
//
void FGlobalRender::DrawRasterOutline (ICamera *Camera,FRasterSetup *Raster, BYTE Color)
	{
	GUARD;
	BYTE *StartScreenLine = &Camera->Screen[Raster->StartY * Camera->SXStride];
	//
	DrawRasterSide(StartScreenLine,Camera->SXStride,Raster->LeftSide, Color);
	DrawRasterSide(StartScreenLine,Camera->SXStride,Raster->RightSide,Color);
	//
	UNGUARD("FGlobalRender::DrawRasterOutline");
	};

//
// Clean up LatticePtr by zeroing out all pointers
// specified by LatticeSpan.  This is needed because we assume that
// all NULL LatticeSpan pointers don't fall into the currently generated
// lattice.
//
void FGlobalRender::CleanupLattice (FSpanBuffer &LatticeSpan)
	{
	GUARD;
	FSpan		**Index			= &LatticeSpan.Index [0];
	FTexLattice **LatticePtr1	= &LatticePtr  [LatticeSpan.StartY+1][1];
	//
	for (int i=LatticeSpan.StartY; i<LatticeSpan.EndY; i++)
		{
		FSpan *Span = *Index++;
		while (Span)
			{
			FTexLattice **Lattice = &LatticePtr1[Span->Start];
			for (int j=Span->Start; j<Span->End; j++) *Lattice++ = NULL;
			Span = Span->Next;
			};
		LatticePtr1 += MAX_XR;
		};
	#ifdef PARANOID
		for (i=0; i<MAX_YR; i++) for (int j=0; j<MAX_XR; j++) if (LatticePtr[i][j]) appErrorf("Failed %i,%i",i,j);
	#endif
	//
	UNGUARD("FGlobalRender::CleanupLattice");
	};

/*-----------------------------------------------------------------------------
	General Bsp surface rendering
-----------------------------------------------------------------------------*/

//
// Draw a Bsp surface
//
void FGlobalRender::DrawBspSurf (ICamera *Camera, FBspDrawList *Draw)
	{
	GUARD;
	//
	IModel 		*ModelInfo 		= &Camera->Level.ModelInfo;
	FBspNode	*Node 			= &ModelInfo->BspNodes [Draw->iNode];
	FBspSurf	*Poly 			= &ModelInfo->BspSurfs [Node->iSurf];
	DWORD		PolyFlags		= Draw->PolyFlags;
	void		*MemTop			= GMem.GetFast(0);
	FSpanBuffer	TempLinearSpan;
	//
	UTexture *Texture = ModelInfo->BspSurfs[Draw->iSurf].Texture;
	if (!Texture) Texture=GGfx.DefaultTexture;
	//
	if (GEditor && GEditor->Scan.Active) GEditor->Scan.PreScan ();
	if (PolyFlags & PF_Selected) TempLinearSpan.CopyIndexFrom(Draw->Span,&GMem);
	//
	if (GApp->RenDev && GApp->RenDev->Active)
		{
		int  Index		  = (Camera->RendMap==REN_Polys) ? Draw->iSurf : Draw->iNode;
		BYTE iColor       = Texture->MipZero.RemapIndex;
		//
		if (iColor>0x80) iColor -= (Index%6) << 3;
		else			 iColor += (Index%6) << 3;
		//
		FVector	*TextureU	= GetVector(ModelInfo,&Camera->Coords,Poly->vTextureU);
		FVector	*TextureV 	= GetVector(ModelInfo,&Camera->Coords,Poly->vTextureV);
		FVector	*Normal   	= GetVector(ModelInfo,&Camera->Coords,Poly->vNormal  );
		FVector	*Base     	= GetPoint (ModelInfo,Camera,         Poly->pBase    );
		//
		FColor Color = GGfx.DefaultColors[iColor];
		GApp->RenDev->DrawPolyV
			(
			Camera,Poly->Texture ? Poly->Texture : GGfx.DefaultTexture,
			Draw->Pts,Draw->NumPts,
			*Base,*Normal,
			*TextureU,*TextureV
			);
		}
	else if ((Camera->RendMap!=REN_Polys)&&(Camera->RendMap!=REN_PolyCuts)&&(Camera->RendMap!=REN_Zones))
		{
		DrawSoftwareTexturedBspSurf(Camera,Draw);
		}
	else
		{
		BYTE Color = Texture->MipZero.RemapIndex;
		//
		int Index;
		if ((Camera->RendMap==REN_Zones)&&(ModelInfo->NumZones>0))
			{
			if (Node->iZone==MAXBYTE) Color = 0x67 + ((Draw->iNode&3)<<3);
			else Color = 0x28 + (Node->iZone&0x07) + ((Draw->iNode&3)<<3) + ((Node->iZone&0x28)<<2);
			//
			if (PolyFlags & PF_Portal) DrawHighlight (Camera,&Draw->Span,BrushSnapColor);
			else DrawFlatPoly (Camera,&Draw->Span,Color);
			}
		else
			{
			if (Camera->RendMap==REN_Polys)	Index=Draw->iSurf;
			else Index=Draw->iNode;
			//
			if (Color>0x80) Color -= (Index%6) << 3;
			else			Color += (Index%6) << 3;
			//
			DrawFlatPoly (Camera,&Draw->Span,Color);
			};
		};
	if (GEditor && (PolyFlags & PF_Selected)) DrawHighlight (Camera,&TempLinearSpan,SelectColor);
	if (GEditor && GEditor->Scan.Active) GEditor->Scan.PostScan (EDSCAN_BspNodePoly,Draw->iNode,0,0,NULL);
	GMem.Release(MemTop);
	//
	UNGUARD("FGlobalRender::DrawBspSurf");
	};

/*-----------------------------------------------------------------------------
	Span buffer Bsp rendering
-----------------------------------------------------------------------------*/

int __cdecl DrawTexCompare(const void *A, const void *B)
	{
	return (*(FBspDrawList **)A)->Texture - (*(FBspDrawList **)B)->Texture;
	};

void FGlobalRender::DrawWorld(ICamera *Camera)
	{
	GUARD;
	IModel		*ModelInfo = &Camera->Level.ModelInfo;
	void		*MemTop    = GMem.Get(0);
	FSpanBuffer	Backdrop;
	INDEX		iActorExclude;
	int			i,n;
	//
	if (GRend->LeakCheck) memset(Camera->Screen,BrushWireColor,Camera->SXR*Camera->SYR);
	RendIter++;
	//
	dynamicsLock		(&Camera->Level.ModelInfo);
	InitTransforms		(&Camera->Level.ModelInfo);
	//
	if (Camera->ShowFlags & SHOW_Actors)
		{
		iActorExclude = Camera->iActor;
		if (Camera->Actor->bBehindView) iActorExclude = INDEX_NONE;
		dynamicsSetup (Camera,iActorExclude);
		};
	if (ModelInfo->NumBspNodes)
		{
		Backdrop.AllocIndex(0,0,&GDynMem);
		n = OccludeBsp(Camera,&Backdrop);
		}
	else
		{
		Backdrop.AllocIndexForScreen(Camera->SXR,Camera->SYR,&GDynMem);
		n = 0;
		};
	GLightManager->DoDynamicLighting( &Camera->Level );
	//
	// Show backdrop if desired:
	//
	if ((Camera->ShowFlags & SHOW_Backdrop) && (Backdrop.ValidLines>0)) DrawBackdrop(Camera,&Backdrop);
	//
	if (GApp->RenDev && GApp->RenDev->Active)
		{
		GApp->RenDev->Lock(Camera);
		//
		// Draw everything in the world from front to back to hardware. Nothing has been
		// merged. There can be no sorting errors.
		//
		FBspDrawList *Draw = &GRender.DrawList[n-1];
		for (i=0; i<n; i++) DrawBspSurf (Camera,Draw--);
		//
		GApp->RenDev->Unlock(Camera);
		}
	else
		{
		//
		// Now draw all fully-occluding polygons.  Sort order is irrelevant here, so
		// we take this opportunity to merge polys with identical textures.
		//
		FBspDrawList **DrawPtrs = (FBspDrawList **)GMem.Get(n * sizeof(FBspDrawList *));
		FBspDrawList *TempDraw=&GRender.DrawList[0],**TempDrawPtr=&DrawPtrs[0];
		for (i=0; i<n; i++) *TempDrawPtr++ = TempDraw++;
		qsort(&DrawPtrs[0],n,sizeof(FBspDrawList*),DrawTexCompare);
		//
		TempDrawPtr = &DrawPtrs[0];
		UTexture *PrevTex = NULL;
		for (i=0; i<n; i++)
			{
			FBspDrawList *Draw = *TempDrawPtr++;
			if (!(Draw->PolyFlags & (PF_NoOcclude|PF_Portal)))
				{
				// Handle texture:
				if ((Draw->Texture!=PrevTex) || !PrevTex)
					{
					STAT(GStat.UniqueTextures++);
					if (Draw->Texture) STAT(GStat.UniqueTextureMem+=Draw->Texture->USize*Draw->Texture->VSize);
					for (int j=0; j<8; j++) GBlit.PrevMipRef[j]=0;
					PrevTex = Draw->Texture;
					};
				// Update span bounds:
				FBspSurf *Poly = &ModelInfo->BspSurfs[Draw->iSurf];
				Draw->Span.GetValidRange(&Poly->LastStartY,&Poly->LastEndY);
				// Draw it:
				DrawBspSurf (Camera,Draw);
				// Must draw actors interspersed here as GDrawList entries!
				};
			};
		//
		// Now draw all non-occluding polygons in back-to-front order. For proper
		// rendering, these must be interleaved with dynamics.
		//
		FBspDrawList *Draw = &GRender.DrawList[n-1];
		for (i=0; i<n; i++)
			{
			if (Draw->PolyFlags & (PF_NoOcclude|PF_Portal))
				{
				STAT(GStat.UniqueTextures++);
				if (Draw->Texture) STAT(GStat.UniqueTextureMem+=Draw->Texture->USize*Draw->Texture->VSize);
				for (int j=0; j<8; j++) GBlit.PrevMipRef[j]=0;
				DrawBspSurf (Camera,Draw);
				};
			Draw--;
			};
		};
	if (Camera->ShowFlags & SHOW_Actors) dynamicsFinalize(Camera,1);
	//
	// Draw all moving brushes as wireframes:
	//
	if (Camera->ShowFlags & SHOW_MovingBrushes) DrawMovingBrushWires(Camera);
	//
	// Finish up:
	//
	STAT(GStat.GMem			= (int)(GMem.Top-GMem.Start));
	STAT(GStat.GDynMem		= (int)(GDynMem.Top-GDynMem.Start));
	STAT(GStat.NodesTotal	= ModelInfo->NumBspNodes);
	//
	GLightManager->UndoDynamicLighting( &Camera->Level );
	//
	ExitTransforms		();
	dynamicsUnlock		(&Camera->Level.ModelInfo);
	GMem.Release        (MemTop);
	//
	UNGUARD("FGlobalRender::DrawWorld");
	};

void FScreenBounds::DebugDump(void)
	{
	debugf("Box (%i,%i)-(%i,%i)",MinX,MinY,MaxX,MaxY);
	};

void FGlobalRender::DrawBackdrop (ICamera *Camera, FSpanBuffer *SpanBuffer)
	{
	GUARD;
	void *MemTop = GMem.Get(0);
	//
	for (int j=0; j<8; j++) GBlit.PrevMipRef[j]=0;
	//
	FBspDrawList Draw;
	Draw.iNode		= INDEX_NONE;
	Draw.iSurf		= INDEX_NONE;
	Draw.PolyFlags	= 0;
	Draw.NumPts		= 0;
	Draw.Span		= *SpanBuffer;
	Draw.Texture	= GGfx.BackdropTexture;
	//
	DrawSoftwareTexturedBspSurf(Camera,&Draw);
	//
	GMem.Release(MemTop);
	UNGUARD("FGlobalRender::DrawBackdrop");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
