/*=============================================================================
	UnLight.cpp: Bsp light mesh illumination builder code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"
#include "UnDynBsp.h"

/*---------------------------------------------------------------------------------------
   Globals
---------------------------------------------------------------------------------------*/

enum {UNITS_PER_METER=32}; // Number of world units per meter

//
// Class used for storing all globally-accessible light generation parameters:
//
class FMeshIlluminator
{
public:

	// Variables:
	UCamera			*Camera;
	ICamera			CameraInfo;
	ILevel			*LevelInfo;
	IModel			*ModelInfo;
	UActorList		*Actors;
	ULightMesh		*LightMesh;
	FVector			*FVectors;
	FVector			*FPoints;
	int				NumLights,PolysLit,ActivePolys,RaysTraced,Pairs,Oversample;

	// Functions:
	void	AllocateLightCamera (ULevel *Level);
	void	FreeLightCamera (void);
	void	SetCameraView (int ViewNum,FVector *Location);
	void	ComputeLightVisibility (INDEX iLightActor,AActor *Actor);
	int		ComputeAllLightVisibility(int Selected);
	void	LightBspSurf(INDEX iSurf);
	void	LightAllSurfs(void);
	void	BuildSurfList(INDEX iNode);
	void	InitLightMeshIndices(void);
};

/*---------------------------------------------------------------------------------------
   Functions for managing temporary (non-windowed) cameras
---------------------------------------------------------------------------------------*/

//
// Allocate a temporary camera for lighting purposes,
//
void FMeshIlluminator::AllocateLightCamera (ULevel *Level)
{
	GUARD;
	AActor *Actor;

	Camera = new("Raytracer",CREATE_Replace)UCamera(Level);
	if (!Camera) appError ("AllocateLightCamera: NewCamera failed");

	Actor							= &Camera->GetActor();
	Camera->SXR						= 128;
	Camera->SYR						= 128;
	Actor->CameraStatus.ShowFlags	= 0;
	Actor->CameraStatus.RendMap		= REN_PlainTex;

	Camera->OpenWindow(0,1); // Open a temporary (non-visible) camera

	Camera->Lock(&CameraInfo);
	LevelInfo = &CameraInfo.Level;
	ModelInfo = &CameraInfo.Level.ModelInfo;
	Actors    = CameraInfo.Level.Actors;

	UNGUARD("FMeshIlluminator::AllocateLightCamera");
}

void FMeshIlluminator::FreeLightCamera (void)
{				
	GUARD;
	Camera->Unlock (&CameraInfo,0);
	Camera->Kill();
	UNGUARD("FMeshIlluminator::FreeLightCamera");
}

void FMeshIlluminator::SetCameraView (int ViewNum,FVector *Location)
{
	GUARD;
	CameraInfo.Actor->ViewRot  = GMath.SixViewRotations [ViewNum]; // Up/down/north/south/east/west
	CameraInfo.Actor->Location = *Location;
	Camera->Unlock(&CameraInfo,0);
	Camera->Lock(&CameraInfo);
	UNGUARD("FMeshIlluminator::SetCameraView");
}

/*---------------------------------------------------------------------------------------
   Light visibility computation
---------------------------------------------------------------------------------------*/

//
// Compute per-polygon visibility of one light:
//
void FMeshIlluminator::ComputeLightVisibility (INDEX iLightActor,AActor *Actor)
{
	GUARD;
	FLightMeshIndex *LightMeshIndex = ModelInfo->LightMesh->GetData();

	// Render six span occlusion frames looking up/down/n/s/e/w and tag all
	// visibly polygons:

	for (int i=0; i<6; i++)
	{
		void *MemTop    = GMem.Get(0);
		void *DynMemTop = GDynMem.Get(0);

		SetCameraView      	(i,&Actors->Element(iLightActor).Location);
		GRend->InitTransforms(ModelInfo);
		GRend->OccludeBsp(&CameraInfo,NULL);

		FBspDrawList *DrawList = &GRend->DrawList[0];
		while (DrawList->iNode != INDEX_NONE)
		{
			FBspSurf *Poly = &ModelInfo->BspSurfs[DrawList->iSurf];
			if ((Poly->iLightMesh!=INDEX_NONE) && 
				(Actor->bSpecialLit ? (Poly->PolyFlags&PF_SpecialLit) : !(Poly->PolyFlags&PF_SpecialLit)))
			{
				if ((Actor->LightRadius==0) || (FPointPlaneDist
					(
					Actor->Location,
					FPoints[Poly->pBase],
					FVectors[Poly->vNormal]
					) <= Actor->WorldLightRadius()))
				{
					FLightMeshIndex *Index = &LightMeshIndex[Poly->iLightMesh];
					if (Index->NumStaticLights	< Index->MAX_POLY_LIGHTS)
					{
						for (INDEX j=0; j<Index->NumStaticLights; j++)
						{
							if (Index->iLightActor[j] == iLightActor) goto Skip;
						};
						Pairs++;
						Index->iLightActor [Index->NumStaticLights++] = iLightActor;
						Skip:;
					}
				}
			}
		DrawList++;
		}
		GRend->ExitTransforms();

		GDynMem.Release(DynMemTop);
		GMem.Release(MemTop);
		}
	UNGUARD("FMeshIlluminator::ComputeLightVisibility");
}

//
// Compute visibility between each light in the world and each polygon.
// Returns number of lights to be applied.
//
int FMeshIlluminator::ComputeAllLightVisibility(int Selected)
	{
	GUARD;
	AActor *Actor = &Actors->Element(0);
	INDEX i,n=0;
	//
	for (i=0; i<Actors->Max; i++)
		{
		if (Actor->Class && (Actor->LightType!=LT_None) && Actor->bStaticActor)
			{
			if (Actor->bSelected || !Selected)
				{
				n++;
				ComputeLightVisibility (i,Actor);
				};
			};
		Actor++;
		};
	return n;
	UNGUARD("FMeshIlluminator::ComputeAllLightVisibility");
	};

/*---------------------------------------------------------------------------------------
   Polygon lighting
---------------------------------------------------------------------------------------*/

//
// Apply all lights to one poly, generating its lighting mesh and updating
// the tables:
//
void FMeshIlluminator::LightBspSurf (INDEX iSurf)
	{
	GUARD;
	FBspSurf			*Surf		= &ModelInfo->BspSurfs [iSurf];
	INDEX				iLightMesh	= Surf->iLightMesh;
	if (iLightMesh==INDEX_NONE) appError("Invalid lightmesh");
	//
	FLightMeshIndex		*Index		= &ModelInfo->LightMesh->GetData()[iLightMesh];
	AActor				*Actor		= NULL;
	UModel				*Brush		= NULL;
	FVector				InverseUAxis,InverseVAxis,InverseNAxis;
	//
	if (iSurf >= ModelInfo->NumBspSurfs)
		{
		if (Surf->iActor==INDEX_NONE) appError("Bad dynamic surface actor");
		AActor *Actor = &Actors->Element(Surf->iActor);
		//
		if (!Actor->Brush) appError("Bad dynamic surface brush");
		Brush = Actor->Brush;
		//
		if (!LevelInfo->SendMessage(Surf->iActor,ACTOR_RaytraceBrush,NULL))
			{
			iLightMesh = INDEX_NONE;
			};
		};
	FVector		*Base     = &FPoints  [Surf->pBase];
	FVector		*Normal   = &FVectors [Surf->vNormal];
	FVector		TextureU  =  FVectors [Surf->vTextureU];
	FVector		TextureV  =  FVectors [Surf->vTextureV];
	//
	FLOAT		MinU	  = +10000000.0;
	FLOAT		MinV	  = +10000000.0;
	FLOAT		MaxU	  = -10000000.0;
	FLOAT		MaxV	  = -10000000.0;
	//
	if (iSurf < ModelInfo->NumBspSurfs)
		{
		//
		// Find extent of static world surface from all of the Bsp polygons
		// that use the surface:
		//
		FBspNode *Node = &ModelInfo->BspNodes[0];
		for (INDEX i=0; i<ModelInfo->NumBspNodes; i++)
			{
			if ((Node->iSurf == iSurf) && (Node->NumVertices>0))
				{
				FVertPool *VertPool = &ModelInfo->VertPool[Node->iVertPool];
				for (BYTE B=0; B < Node->NumVertices; B++)
					{
					FVector Vertex	= FPoints[VertPool[B].pVertex] - *Base;
					FLOAT	U		= (Vertex | TextureU) / 65536.0;
					FLOAT	V		= (Vertex | TextureV) / 65536.0;
					//
					if (U < MinU) MinU = U; if (U > MaxU) MaxU = U;
					if (V < MinV) MinV = V; if (V > MaxV) MaxV = V;
					};
				};
			Node++;
			};
		}
	else
		{
		//
		// Find extent of moving brush polygon from the original EdPoly that
		// generated the surface:
		//
		FPoly *Poly = &Brush->Polys->Element(0);
		for (int i=0; i<Brush->Polys->Num; i++)
			{
			if (Poly->iLink == iSurf)
				{
				for (int j=0; j<Poly->NumVertices; j++)
					{
					// Find extent in untransformed brush space
					FVector Vertex	= Poly->Vertex[j] - Poly->Base;
					FLOAT	U		= (Vertex | Poly->TextureU) / 65536.0;
					FLOAT	V		= (Vertex | Poly->TextureV) / 65536.0;
					//
					if (U < MinU) MinU = U; if (U > MaxU) MaxU = U;
					if (V < MinV) MinV = V; if (V > MaxV) MaxV = V;
					};
				Poly->iBrushPoly = iLightMesh;
				break;
				};
			Poly++;
			};
		if (i>=Brush->Polys->Num) appError("Dissociated brush surface");
		};
	//
	// Compute mesh density:
	//
	DWORD PolyFlags = ModelInfo->BspSurfs[iSurf].PolyFlags;
	if (PolyFlags & PF_HighShadowDetail)
		{
		Index->MeshSpacing   = 16;
		Index->MeshShift     = 4;
		}
	else if (PolyFlags & PF_LowShadowDetail)
		{
		Index->MeshSpacing   = 64;
		Index->MeshShift     = 6;
		}
	else
		{
		Index->MeshSpacing   = 32;
		Index->MeshShift     = 5;
		};
	//
	// Set light mesh index values, forcing to lattice for coplanar mesh alignment:
	//
	while (1)
		{
		Index->TextureUStart = FIX(((INT)(MinU+0.01) & ~(Index->MeshSpacing-1))-2*Index->MeshSpacing);
		Index->TextureVStart = FIX(((INT)(MinV+0.01) & ~(Index->MeshSpacing-1))-2*Index->MeshSpacing);
		//
		Index->MeshUSize = (((INT)(MaxU-0.01) - UNFIX(Index->TextureUStart)) >> Index->MeshShift) + 4;
		Index->MeshVSize = (((INT)(MaxV-0.01) - UNFIX(Index->TextureVStart)) >> Index->MeshShift) + 4;
		//
		if ((Index->MeshUSize <= 1024) 
			&& (Index->MeshVSize <= 1024) 
			&& (Index->MeshUSize * Index->MeshVSize <= 128*128))
			break;
		Index->MeshShift++;
		Index->MeshSpacing *= 2;
		};
	//
	// Update data byte count and reallocate the data:
	//
	Index->DataOffset        = LightMesh->NumDataBytes;
	LightMesh->NumDataBytes += Index->NumStaticLights * ((Index->MeshUSize+7)>>3) * Index->MeshVSize;
	//
	ModelInfo->LightMesh->Realloc();
	//
	Index						= &ModelInfo->LightMesh->GetData()[Surf->iLightMesh];
	BYTE *DataStart				= (BYTE *)ModelInfo->LightMesh->GetData() + Index->DataOffset;
	int Size					= ((Index->MeshUSize+7)>>3) * Index->MeshVSize;
	//
	// Calculate new base point by moving polygon's base point forward by 4 units:
	//
	FVector		NewBase			= FPoints [Surf->pBase] + (*Normal)*4.0;
	FLOAT		FMeshSpacing	= (FLOAT)Index->MeshSpacing;
	//
	// Calculate inverse U & V axes which map texels onto pixels:
	//
	InvertVectors(TextureU,TextureV,*Normal,InverseUAxis,InverseVAxis,InverseNAxis);
	InverseUAxis *= 65536.0;
	InverseVAxis *= 65536.0;
	//
	// Raytrace each lightsource
	//
	for (int i=0; i<Index->NumStaticLights; i++)
		{
		AActor  *Actor      = &Actors->Element(Index->iLightActor[i]);
		FVector *Light		= &Actor->Location;
		BYTE	*Data		= DataStart;
		//
		// Go through all lattice points and build lighting mesh.
		// U,V units are texels.
		//
		FLOAT	U			= (FLOAT)UNFIX(Index->TextureUStart);
		FLOAT	V			= (FLOAT)UNFIX(Index->TextureVStart);
		FVector	Vertex0		= NewBase + InverseUAxis*U + InverseVAxis*V;
		FVector VertexDU	= InverseUAxis * FMeshSpacing;
		FVector VertexDV	= InverseVAxis * FMeshSpacing;
		//
		for (int VCounter = 0; VCounter < Index->MeshVSize; VCounter++)
			{
			FVector Vertex = Vertex0;
			for (int UCounter = 0; UCounter < Index->MeshUSize; UCounter+=8)
				{
				BYTE B = 0;
				BYTE M = 1;
				for (int ByteUCounter=0; ByteUCounter < 8; ByteUCounter++)
					{					
					if (ModelInfo->LineClass(&Vertex, Light)) B |= M;
					M = M << 1;
					RaysTraced++;
					Vertex += VertexDU;
					};
				*Data++ = B;
				};
			Vertex0 += VertexDV;
			};
		DataStart += Size;
		};
	if (iSurf >= ModelInfo->NumBspSurfs)
		{
		LevelInfo->SendMessage(Surf->iActor,ACTOR_RaytraceWorld,NULL);
		};
	UNGUARD("FMeshIlluminator::LightBspSurf");
	};

void FMeshIlluminator::LightAllSurfs(void)
	{
	GUARD;
	//
	int n=0,c=0;
	for (INDEX i=0; i<ModelInfo->MaxBspSurfs; i++)
		{
		n += (ModelInfo->BspSurfs[i].iLightMesh != INDEX_NONE);
		};
	for (i=0; i<ModelInfo->MaxBspSurfs; i++)
		{
		if (ModelInfo->BspSurfs[i].iLightMesh != INDEX_NONE)
			{
			GApp->StatusUpdate ("Raytracing",c++,n);
			LightBspSurf(i);
			};
		};
	UNGUARD("FMeshIlluminator::LightAllSurfs");
	};

/*---------------------------------------------------------------------------------------
   Index building
---------------------------------------------------------------------------------------*/

//
// Recursively go through the Bsp nodes and build a list of active Bsp polys,
// allocating their light mesh indices.
//
void FMeshIlluminator::BuildSurfList(INDEX iNode)
	{
	GUARD;
	FBspNode			*Node       = &ModelInfo->BspNodes [iNode];
	INDEX				iSurf       = Node->iSurf;
	FBspSurf			*Surf       = &ModelInfo->BspSurfs [iSurf];
	//
	if ((Node->NumVertices > 0) && 
		(iSurf != INDEX_NONE) &&
		(Surf->vTextureU != MAXWORD) && 
		(Surf->vTextureV != MAXWORD) &&
		(!(Surf->PolyFlags & PF_NoShadows)))
		{
		if (Surf->iLightMesh==INDEX_NONE)
			{
			PolysLit++;
			Surf->iLightMesh = LightMesh->NumIndices++; // Link polygon to light mesh
			FLightMeshIndex *Index = &ModelInfo->LightMesh->GetData()[Surf->iLightMesh];
			};
		};
	if (Node->iFront != INDEX_NONE) BuildSurfList(Node->iFront);
	if (Node->iBack  != INDEX_NONE) BuildSurfList(Node->iBack);
	if (Node->iPlane != INDEX_NONE) BuildSurfList(Node->iPlane);
	//
	if (iNode==0)
		{
		for (int i=ModelInfo->NumBspSurfs; i<ModelInfo->MaxBspSurfs; i++)
			{
			if ((Surf->iLightMesh==INDEX_NONE) && sporeSurfIsDynamic(i))
				{
				PolysLit++;
				Surf->iLightMesh = LightMesh->NumIndices++; // Link polygon to light mesh
				};
			};
		};
	UNGUARD("FMeshIlluminator::BuildSurfList");
	};

//
// Initialize all light mesh indices
//
void FMeshIlluminator::InitLightMeshIndices(void)
	{
	GUARD;
	//
	FLightMeshIndex *Index = &ModelInfo->LightMesh->GetData()[0];
	for (int i=0; i<LightMesh->NumIndices; i++)
		{
		Index->NumStaticLights 	= 0;
		Index->NumDynamicLights	= 0;
		//
		Index++;
		};
	UNGUARD("FMeshIlluminator::InitLightMeshIndices");
	};

/*---------------------------------------------------------------------------------------
   High-level lighting routine
---------------------------------------------------------------------------------------*/

void FEditor::shadowIlluminateBsp (ULevel *Level, int Selected)
	{
	GUARD;
	FMeshIlluminator Illum;
	//
	// Invalidate texture/illumination cache:
	//
	GCache.Flush();
	//
	GApp->BeginSlowTask ("Raytracing",1,0);
	GApp->StatusUpdate  ("Allocating meshes",0,0);
	//
	sporeInit					(Level);
	Illum.AllocateLightCamera	(Level);
	//
	Illum.FVectors = Illum.ModelInfo->FVectors;
	Illum.FPoints  = Illum.ModelInfo->FPoints;
	//
	if (Illum.ModelInfo->NumBspNodes!=0)
		{
		if (Illum.ModelInfo->LightMesh) Illum.ModelInfo->LightMesh->Kill();
		//
		// Allocate a new lighting mesh
		//
		Illum.ModelInfo->LightMesh		= new(Level->Model->Name,CREATE_Replace)ULightMesh;
		Illum.LightMesh					= Illum.ModelInfo->LightMesh;
		//
		Illum.LightMesh->NumDataBytes	= 0;
		Illum.PolysLit					= 0;
		Illum.RaysTraced				= 0;
		Illum.ActivePolys				= 0;
		Illum.Pairs						= 0;
		//
		// Clear all poly light mesh indices:
		//
		for (INDEX i=0; i<Illum.ModelInfo->MaxBspSurfs; i++)
			{
			Illum.ModelInfo->BspSurfs[i].iLightMesh = INDEX_NONE;
			};
		//
		// Tell all actors that we're about to raytrace the world.
		// This enables movable brushes to set their positions for raytracing.
		//
		AActor *Actor = &Illum.CameraInfo.Level.Actors->Element(0);
		for (i=0; i<Illum.Actors->Max; i++)
			{
			if (Actor->Class) Illum.LevelInfo->SendMessage(i,ACTOR_PreRaytrace,NULL);
			if (Actor->Class) Illum.LevelInfo->SendMessage(i,ACTOR_RaytraceWorld,NULL);
			Actor++;
			};
		//
		// Allocate enough data to hold light mesh index:
		//
		Illum.LightMesh->NumIndices   = Illum.ModelInfo->MaxBspSurfs;
		Illum.LightMesh->NumDataBytes = Illum.LightMesh->NumIndices * sizeof(FLightMeshIndex);
		Illum.ModelInfo->LightMesh->AllocData(1);
		//
		// Recursively update list of polys with min/max U,V's and visibility flags:
		//
		Illum.LightMesh->NumIndices = 0;
		Illum.BuildSurfList(0);
		Illum.InitLightMeshIndices();
		//
		Illum.LightMesh->NumDataBytes = Illum.LightMesh->NumIndices * sizeof(FLightMeshIndex);
		//
		// Compute light visibility and update index with it:
		//
		Illum.NumLights = Illum.ComputeAllLightVisibility(Selected);
		//
		// Apply light to each polygon:
		//
		Illum.LightAllSurfs();
		//
		// Tell all actors that we're done raytracing the world.
		//
		Actor = &Illum.CameraInfo.Level.Actors->Element(0);
		for (i=0; i<Illum.Actors->Max; i++)
			{
			if (Actor->Class)
				{
				Actor->bTempDynamicLight=0;
				Illum.LevelInfo->SendMessage(i,ACTOR_PostRaytrace,NULL);
				};
			Actor++;
			};
		debugf (LOG_Ed,"%i Lights, %i Polys, %i Pairs, %i Rays",
			Illum.NumLights,Illum.PolysLit,Illum.Pairs,Illum.RaysTraced);
		};
	Illum.FreeLightCamera	();
	GApp->EndSlowTask		();
	sporeExit				();
	GCache.Flush			();
	//
	UNGUARD("lightIlluminateBsp");
	};

/*---------------------------------------------------------------------------------------
   Light link topic handler
---------------------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Light",LightTopicHandler);
void LightTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UActorList			*ActorList		= Level->ActorList;
	UModel				*Model			= Level->Model;
	ULightMesh			*LightMesh		= Model->LightMesh;
	AActor				*Actor;
	FLightMeshIndex		*Index;
	INDEX				i;
	int					Meshes,MeshPts,Size,MaxSize,CacheSize,Meters,LightCount,SelCount;
	//
	Meters		= 0;
	LightCount  = 0;
	SelCount	= 0;
	MeshPts		= 0;
	MaxSize		= 0;
	CacheSize	= 0;
	//
	for (i=0; i<ActorList->Max; i++)
		{
		Actor = &ActorList->Element(i);
		if (Actor->Class==GClasses.Light)
			{
			LightCount++;
			if (Actor->bSelected) SelCount++;
			};
		};
	if ((!Level)||(!Model)||(!LightMesh))
		{
		Meshes = 0;
		}
	else
		{
		Index  = LightMesh->GetData();
		Meshes = LightMesh->NumIndices;
		//
		for (int i=0; i<Meshes; i++)
			{
			Size       = (int)Index->MeshUSize * (int)Index->MeshVSize;
			MeshPts   += Size;
	  		CacheSize += Size * (int)Index->MeshSpacing * (int)Index->MeshSpacing;
			if (Size>MaxSize) MaxSize = Size;
			//
			Index++;
			};
		Meters = CacheSize / (UNITS_PER_METER * UNITS_PER_METER);
		};
    if      (stricmp(Item,"Meshes")==0) 	itoa (Meshes,Data,10);
    else if (stricmp(Item,"MeshPts")==0) 	itoa (MeshPts,Data,10);
    else if (stricmp(Item,"MaxSize")==0) 	itoa (MaxSize,Data,10);
    else if (stricmp(Item,"Meters")==0) 	itoa (Meters,Data,10);
    else if (stricmp(Item,"Count")==0) 		sprintf(Data,"%i (%i)",LightCount,SelCount);
    else if (stricmp(Item,"AvgSize")==0) 	itoa (MeshPts/OurMax(1,Meshes),Data,10);
    else if (stricmp(Item,"CacheSize")==0) {itoa(CacheSize/1000,Data,10); strcat (Data,"K");}
	//
	UNGUARD("LightTopicHandler::Get");
	};
void LightTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("LightTopicHandler::Set");
	};

/*---------------------------------------------------------------------------------------
   The End
---------------------------------------------------------------------------------------*/
