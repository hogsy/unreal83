/*=============================================================================
	UnModel.cpp: Unreal model functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

//
// Model spiffing parameters:
//
#define SPIFF_MIN 		256		/* Minimum available before more are allocated */
#define SPIFF_ALLOC		2048	/* Number to alloc during */
#define SPIFF_MAX		4096	/* Maximum available before shrinking */
#define SPIFF_LIMIT		65535	/* Overflow */

/*---------------------------------------------------------------------------------------
	UBspNodes resource implementation
---------------------------------------------------------------------------------------*/

void UBspNodes::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UBspNodes);
	Type->RecordSize = sizeof (FBspNode);
	Type->Version    = 1;
	strcpy (Type->Descr,"BspNodes");
	UNGUARD("UBspNodes::Register");
	};
void UBspNodes::InitHeader(void)
	{
	GUARD;
	Num				= 0;
	Max				= 0;
	NumZones		= 0;
	NumUniquePlanes	= 0;
	for (int i=0; i<64; i++) Zones[i].Connectivity=((QWORD)1)<<i;
	UNGUARD("UBspNodes::InitHeader");
	};
void UBspNodes::InitData(void)
	{
	GUARD;
	Num  = 0;
	for (int i=0; i<64; i++) Zones[i].Connectivity=((QWORD)1)<<i;
	UNGUARD("UBspNodes::InitData");
	};
AUTOREGISTER_RESOURCE(RES_BspNodes,UBspNodes,0xB2D90860,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UBspSurfs resource implementation
---------------------------------------------------------------------------------------*/

//
// Resource functions
//
void UBspSurfs::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UBspSurfs);
	Type->RecordSize = sizeof (FBspSurf);
	Type->Version    = 1;
	strcpy (Type->Descr,"BspSurfs");
	UNGUARD("UBspSurfs::Register");
	};
void UBspSurfs::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	for (INDEX i=0; i < Num; i++)
		{
		Callback.Resource (this,(UResource **)&Element(i).Texture,0);
		Callback.Resource (this,(UResource **)&Element(i).Brush  ,0);
		};
	UNGUARD("UBspSurfs::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_BspSurfs,UBspSurfs,0xB2D90861,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	ULightMesh implementation
---------------------------------------------------------------------------------------*/

void ULightMesh::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (ULightMesh);
	Type->RecordSize = 0;
	Type->Version    = 1;
	strcpy (Type->Descr,"LightMesh");
	UNGUARD("ULightMesh::Register");
	};
void ULightMesh::InitHeader(void)
	{
	GUARD;
	NumIndices   = 0;
	NumDataBytes = 0;
	UNGUARD("ULightMesh::InitHeader");
	};
void ULightMesh::InitData(void)
	{
	GUARD;
	NumIndices   = 0;
	NumDataBytes = 0;
	UNGUARD("ULightMesh::InitData");
	};
int ULightMesh::QuerySize(void)
	{
	GUARD;
	return NumDataBytes;
	UNGUARD("ULightMesh::QuerySize");
	};
int ULightMesh::QueryMinSize(void)
	{
	GUARD;
	return QuerySize();
	UNGUARD("ULightMesh::QueryMinSize");
	};
AUTOREGISTER_RESOURCE(RES_LightMesh,ULightMesh,0xB2D90862,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UVectors implementation
---------------------------------------------------------------------------------------*/

void UVectors::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UVectors);
	Type->RecordSize = sizeof (FVector);
	Type->Version    = 1;
	strcpy (Type->Descr,"Vectors");
	UNGUARD("UVectors::Register");
	};
AUTOREGISTER_RESOURCE(RES_Vectors,UVectors,0xB2D90863,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UVertPool implementation
---------------------------------------------------------------------------------------*/

void UVertPool::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UVertPool);
	Type->RecordSize = sizeof (FVertPool);
	Type->Version    = 1;
	strcpy (Type->Descr,"VertPool");
	UNGUARD("UVertPool::Register");
	};
void UVertPool::InitHeader(void)
	{
	GUARD;
	Max				= 0;
	Num				= 0;
	NumSharedSides  = 4; // First 4 shared sides are view frustrum sides
	UNGUARD("UVertPool::InitHeader");
	};
void UVertPool::InitData(void)
	{
	GUARD;
	Num				= 0;
	NumSharedSides  = 4;
	UNGUARD("UVertPool::InitData");
	};
AUTOREGISTER_RESOURCE(RES_VertPool,UVertPool,0xB2D90864,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UBound implementation
---------------------------------------------------------------------------------------*/

void UBounds::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UBounds);
	Type->RecordSize = sizeof (FBoundingVolume);
	Type->Version    = 1;
	strcpy (Type->Descr,"Bound");
	UNGUARD("UBound::Register");
	};
AUTOREGISTER_RESOURCE(RES_Bounds,UBounds,0xB2D90865,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UTerrain implementation
---------------------------------------------------------------------------------------*/

void UTerrain::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UTerrain);
	Type->RecordSize = sizeof (FTerrainIndex);
	Type->Version    = 1;
	strcpy (Type->Descr,"Terrain");
	UNGUARD("UTerrain::Register");
	};
void UTerrain::InitHeader(void)
	{
	GUARD;
	Max = 0;
	Num = 0;
	for (int i=0; i<MAX_TERRAIN_LAYERS; i++)
		{
		LayerFlags[i] = 0;
		HeightMaps[i] = NULL;
		TileMaps  [i] = NULL;
		};
	for (int i=0; i<MAX_TILE_REFERENCES; i++)
		{
		TileRefs  [i] = NULL;
		};
	UNGUARD("UTerrain::InitHeader");
	};
void UTerrain::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	for (int i=0; i<MAX_TERRAIN_LAYERS; i++)
		{
		Callback.Resource (this,(UResource **)&LayerFlags[i],0);
		Callback.Resource (this,(UResource **)&HeightMaps[i],0);
		Callback.Resource (this,(UResource **)&TileMaps  [i],0);
		};
	for (int i=0; i<MAX_TILE_REFERENCES; i++)
		{
		Callback.Resource (this,(UResource **)&TileRefs  [i],0);
		};
	UNGUARD("UTerrain::QueryHeaderReferences");
	};
AUTOREGISTER_RESOURCE(RES_Terrain,UTerrain,0xB2D90867,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UModel resource implementation
---------------------------------------------------------------------------------------*/

void UModel::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UModel);
	Type->RecordSize = 0;
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Model");
	UNGUARD("UModel::Register");
	};
void UModel::InitHeader(void)
	{
	GUARD;
	//
	// Init resource header to defaults:
	//
	Vectors    		= NULL;
	Points     		= NULL;
	BspNodes    	= NULL;
	BspSurfs		= NULL;
	VertPool		= NULL;
	Polys	     	= NULL;
	LightMesh   	= NULL;
	Terrain			= NULL;
	Bounds			= NULL;
	//
	LockType		= LOCK_None;
	//
	Location      	= GMath.ZeroVector;
	Rotation		= GMath.ZeroRotation;
	PrePivot		= GMath.ZeroVector;
	PostPivot		= GMath.ZeroVector;
	Scale			= GMath.UnitScale;
	CsgOper			= CSG_Active;
	ModelFlags		= 0;
	//
	Bound[0].Init();
	Bound[1].Init();
	//
	UNGUARD("UModel::InitHeader");
	};
const char *UModel::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	const char		*StrPtr;
	char			StrLine[256];
	//
	Init(1);
	//
	while (GetLINE (&Buffer,StrLine,256)==0)
		{
		StrPtr = StrLine;
		//
		if (GetEND(&StrPtr,"BRUSH"))
			{
			break; // End of brush polys
			}
		else if (GetBEGIN (&StrPtr,"POLYLIST"))
			{
			if (Polys==NULL) Polys = new(Name,CREATE_Replace)UPolys;
			Buffer = Polys->Import(Buffer,BufferEnd,FileType);
			if (!Buffer) return NULL;
			}
		else if (GetCMD(&StrPtr,"SETTINGS"))
			{
			GetDWORD(StrPtr,"CSG=",			(DWORD *)&CsgOper);
			GetDWORD(StrPtr,"FLAGS=",		&ModelFlags);
			GetDWORD(StrPtr,"POLYFLAGS=",	&PolyFlags);
			GetDWORD(StrPtr,"COLOR=",		&Color);
			//
			ModelFlags &= ~(MF_NOIMPORT);
			}
		else if (GetCMD(&StrPtr,"LOCATION"))		GetFVECTOR 	(StrPtr,&Location);
		else if (GetCMD(&StrPtr,"PREPIVOT"))		GetFVECTOR 	(StrPtr,&PrePivot);
		else if (GetCMD(&StrPtr,"POSTPIVOT"))		GetFVECTOR 	(StrPtr,&PostPivot);
		else if (GetCMD(&StrPtr,"SCALE"))			GetFSCALE 	(StrPtr,&Scale);
		else if (GetCMD(&StrPtr,"POSTSCALE"))		GetFSCALE 	(StrPtr,&PostScale);
		else if (GetCMD(&StrPtr,"ROTATION"))		GetFROTATION(StrPtr,&Rotation,1);
		};
	if (GEditor) GEditor->bspValidateBrush(this,0,0);
	//
	return Buffer;
	UNGUARD("UModel::Import");
	};
char *UModel::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	char TempStr[256];
	//
	Buffer += sprintf (Buffer,"%sBegin Brush Name=%s\r\n",spc(Indent),Name);
	//
	// Save all brush properties:
	//
	Buffer += sprintf(Buffer,"%s   Settings  CSG=%i Flags=%u PolyFlags=%u Color=%i\r\n",spc(Indent),CsgOper,ModelFlags,PolyFlags,Color);
	Buffer += sprintf(Buffer,"%s   Location  %s\r\n",spc(Indent),SetFVECTOR   (TempStr,&Location));
	Buffer += sprintf(Buffer,"%s   PrePivot  %s\r\n",spc(Indent),SetFVECTOR   (TempStr,&PrePivot));
	Buffer += sprintf(Buffer,"%s   PostPivot %s\r\n",spc(Indent),SetFVECTOR   (TempStr,&PostPivot));
	Buffer += sprintf(Buffer,"%s   Scale     %s\r\n",spc(Indent),SetFSCALE    (TempStr,&Scale));
	Buffer += sprintf(Buffer,"%s   PostScale %s\r\n",spc(Indent),SetFSCALE    (TempStr,&PostScale));
	Buffer += sprintf(Buffer,"%s   Rotation  %s\r\n",spc(Indent),SetROTATION  (TempStr,&Rotation));
	//
	// Export edpolys:
	//
	Buffer = Polys->Export(Buffer,FileType,Indent+3);
	if (!Buffer) return NULL;
	//
	Buffer += sprintf (Buffer,"%sEnd Brush\r\n",spc(Indent));
	return Buffer;
	//
	UNGUARD("UModel::Export");
	};
int UModel::QuerySize(void)
	{
	GUARD;
	return 0;
	UNGUARD("UModel::QuerySize");
	};
int UModel::QueryMinSize(void)
	{
	GUARD;
	return 0;
	UNGUARD("UModel::QueryMinSize");
	};
void UModel::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	Callback.Resource (this,(UResource **)&Vectors  ,0);
	Callback.Resource (this,(UResource **)&Points   ,0);
	Callback.Resource (this,(UResource **)&BspNodes ,0);
	Callback.Resource (this,(UResource **)&BspSurfs ,0);
	Callback.Resource (this,(UResource **)&VertPool ,0);
	Callback.Resource (this,(UResource **)&LightMesh,0);
	Callback.Resource (this,(UResource **)&Polys    ,0);
	Callback.Resource (this,(UResource **)&Terrain  ,0);
	Callback.Resource (this,(UResource **)&Bounds   ,0);
	UNGUARD("UModel::QueryHeaderReferences");
	};
AUTOREGISTER_RESOURCE(RES_Model,UModel,0xB2D90868,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UModel custom implementation
---------------------------------------------------------------------------------------*/

//
// Lock a model and obtain a ModelInfo structure for accessing it.
// This prevents the model from being swapped.  It also makes the
// model resource unstable (possibly invalid) until you unlock it.
// Return 0 if success, nonzero if problem.
//
void UModel::Lock (IModel *ModelInfo,int NewLockType)
	{
	GUARD;
	if (LockType != LOCK_None) appError ("modelLock: Already locked!");
	//
	if (GTrans && (NewLockType == LOCK_Trans)) // Save all headers for transactional undo/redo:
		{
		GTrans->NoteResHeader (this);
		GTrans->NoteResHeader (BspNodes);
		GTrans->NoteResHeader (BspSurfs);
		GTrans->NoteResHeader (Polys);
		ModelInfo->Trans=1;
		}
	else ModelInfo->Trans=0;
	//
	LockType=NewLockType; // Lock the model after headers are saved transactionally
	//
	ModelInfo->Model				= this;
	ModelInfo->BspNodesResource		= BspNodes;
	ModelInfo->BspSurfsResource		= BspSurfs;
	ModelInfo->PolysResource		= Polys;
	ModelInfo->LightMesh		 	= LightMesh;
	ModelInfo->TerrainResource		= Terrain;
	ModelInfo->BoundsResource		= Bounds;
	//
	// Set main info:
	//
	if (Vectors)
		{
		ModelInfo->FVectors    = Vectors->GetData();
		ModelInfo->MaxVectors  = Vectors->Max;
		ModelInfo->NumVectors  = Vectors->Num;
		}
	else ModelInfo->FVectors = NULL;
	//
	if (Points)
		{
		ModelInfo->FPoints     = Points->GetData();
		ModelInfo->NumPoints   = Points->Num;
		ModelInfo->MaxPoints   = Points->Max;
		}
	else ModelInfo->FPoints = NULL;
	//
	if (BspNodes)
		{
		ModelInfo->NumBspNodes		= BspNodes->Num;
		ModelInfo->MaxBspNodes		= BspNodes->Max;
		ModelInfo->NumZones			= BspNodes->NumZones;
		ModelInfo->NumUniquePlanes	= BspNodes->NumUniquePlanes;
		ModelInfo->BspNodes			= BspNodes->GetData();
		}
	else
		{
		ModelInfo->NumBspNodes		= 0;
		ModelInfo->MaxBspNodes		= 0;
		ModelInfo->NumZones			= 0;
		ModelInfo->NumUniquePlanes	= 0;
		ModelInfo->BspNodes			= NULL;
		};
	//
	if (BspSurfs)
		{
		ModelInfo->NumBspSurfs = BspSurfs->Num;
		ModelInfo->MaxBspSurfs = BspSurfs->Max;
		ModelInfo->BspSurfs    = BspSurfs->GetData();
		}
	else
		{
		ModelInfo->NumBspSurfs = 0;
		ModelInfo->MaxBspSurfs = 0;
		ModelInfo->BspSurfs    = NULL;
		};
	//
	if (VertPool)
		{
		ModelInfo->NumVertPool		= VertPool->Num;
		ModelInfo->MaxVertPool		= VertPool->Max;
		ModelInfo->NumSharedSides	= VertPool->NumSharedSides;
		ModelInfo->VertPool			= VertPool->GetData();
		}
	else
		{
		ModelInfo->NumVertPool		= 0;
		ModelInfo->MaxVertPool		= 0;
		ModelInfo->NumSharedSides	= 4;
		ModelInfo->VertPool			= NULL;
		};
	//
	if (Polys)
		{
		ModelInfo->NumFPolys   = Polys->Num;
		ModelInfo->MaxFPolys   = Polys->Max;
		ModelInfo->FPolys      = Polys->GetData();
		}
	else
		{
		ModelInfo->NumFPolys   = 0;
		ModelInfo->MaxFPolys   = 0;
		ModelInfo->FPolys      = NULL;
		};
	//
	if (Terrain)
		{
		ModelInfo->TerrainIndex = Terrain->GetData();
		ModelInfo->NumTerrain   = Terrain->Num;
		ModelInfo->MaxTerrain   = Terrain->Max;
		}
	else
		{
		ModelInfo->TerrainIndex = NULL;
		ModelInfo->NumTerrain   = 0;
		ModelInfo->MaxTerrain   = 0;
		};
	//
	if (Bounds)
		{
		ModelInfo->Bounds    = Bounds->GetData();
		ModelInfo->NumBounds = Bounds->Num;
		ModelInfo->MaxBounds = Bounds->Max;
		}
	else
		{
		ModelInfo->Bounds    = NULL;
		ModelInfo->NumBounds = 0;
		ModelInfo->MaxBounds = 0;
		};
	ModelInfo->Location		= Location;
	ModelInfo->Rotation		= Rotation;
	ModelInfo->Scale		= Scale;
	ModelInfo->PostScale	= PostScale;
	ModelInfo->TempScale	= TempScale;
	ModelInfo->PrePivot		= PrePivot;
	ModelInfo->PostPivot	= PostPivot;
	ModelInfo->CsgOper		= CsgOper;
	ModelInfo->ModelFlags	= ModelFlags;
	ModelInfo->PolyFlags	= PolyFlags;
	//
	UNGUARD("UModel::Lock");
	};

//
// Unlock a model.  Updates the necessary info, i.e. number of Bsp
// nodes, number of SPoints and FPoints.
//
void UModel::Unlock (IModel *ModelInfo)
	{
	GUARD;
	if (LockType==LOCK_None) appError ("modelUnlock: Not locked");
	LockType=LOCK_None;
	//
	if (Vectors)	Vectors	->Num				= ModelInfo->NumVectors;
	if (Points)		Points	->Num				= ModelInfo->NumPoints;
	if (BspNodes)	BspNodes->Num				= ModelInfo->NumBspNodes;
	if (BspNodes)	BspNodes->NumZones			= ModelInfo->NumZones;
	if (BspNodes)	BspNodes->NumUniquePlanes	= ModelInfo->NumUniquePlanes;
	if (BspSurfs)	BspSurfs->Num				= ModelInfo->NumBspSurfs;
	if (Polys)		Polys	->Num				= ModelInfo->NumFPolys;
	if (VertPool)	VertPool->Num				= ModelInfo->NumVertPool;
	if (VertPool)	VertPool->NumSharedSides	= ModelInfo->NumSharedSides;
	if (Bounds)		Bounds  ->Num				= ModelInfo->NumBounds;
	if (Terrain)	Terrain	->Num				= ModelInfo->NumTerrain;
	//
	Bounds		= ModelInfo->BoundsResource;
	Terrain     = ModelInfo->TerrainResource;
	LightMesh   = ModelInfo->LightMesh;
	Location	= ModelInfo->Location;
	Rotation	= ModelInfo->Rotation;
	Scale		= ModelInfo->Scale;
	PostScale	= ModelInfo->PostScale;
	TempScale	= ModelInfo->TempScale;
	PrePivot	= ModelInfo->PrePivot;
	PostPivot	= ModelInfo->PostPivot;
	CsgOper		= (ECsgOper)ModelInfo->CsgOper;
	ModelFlags	= ModelInfo->ModelFlags;
	PolyFlags	= ModelInfo->PolyFlags;
	//
	UNGUARD("UModel::Unlock");
	};

//
// Init a model's parameters to defaults without affecting what's in it
//
void UModel::Init(int InitPositionRotScale)
	{
	GUARD;
	FModelCoords	TempCoords;
	FVector			Temp;
	//
	CsgOper	    = CSG_Active;
	ModelFlags	= MF_PostScale;
	PolyFlags	= 0;
	Color		= 0;
	//
	if (InitPositionRotScale)
		{
		Scale		= GMath.UnitScale;
		PostScale	= GMath.UnitScale;
		Location	= GMath.ZeroVector;
		Rotation	= GMath.ZeroRotation;
		}
	else
		{
		if (GEditor) GEditor->constraintApply(NULL,NULL,&Location,&Rotation,&GEditor->Constraints);
		BuildCoords (&TempCoords,NULL);
		//
		Temp = GMath.ZeroVector - PrePivot;
		Temp.TransformVector(TempCoords.PointXform);
		Location += Temp + PostPivot;
		};
	PrePivot	= GMath.ZeroVector;
	PostPivot	= GMath.ZeroVector;
	UNGUARD("UModel::Init");
	};

//
// Create a new model and allocate all resources needed for it.
// Call with Editor=1 to allocate editor structures for it, also.  Returns
// model's ID if ok, NULL if error.
//
UModel::UModel(int Editable)
	{
	GUARD;
	char	VName[NAME_SIZE],PName[NAME_SIZE];
	//
	// Figure out how much stuff we need:
	//
	int MaxEdPolys  = 20000;
	int MaxBspNodes = 20000;
	int MaxBspSurfs = 12000;
	int MaxVectors  = 8192;
	int MaxPoints   = 32768+16383;
	int MaxVertPool = 65535*2;
	int MaxBounds   = MaxBspNodes/2;
	int MaxTerrain  = 4095;
	//
	strcpy (VName,"V"); mystrncat (VName,Name,NAME_SIZE); // Vector table name
	strcpy (PName,"P"); mystrncat (PName,Name,NAME_SIZE); // Point table name
	//
	// Allocate all resources for model (indentation shows hierarchy):
	// [doesn't allocate light mesh; that's allocated when built]
	//
	BspNodes	= new(Name, CREATE_Replace)UBspNodes	(MaxBspNodes);
	BspSurfs	= new(Name, CREATE_Replace)UBspSurfs	(MaxBspSurfs);
	VertPool	= new(Name, CREATE_Replace)UVertPool	(MaxVertPool);
	Polys		= new(Name, CREATE_Replace)UPolys		(MaxEdPolys);
	Terrain		= new(Name, CREATE_Replace)UTerrain		(MaxEdPolys);
	Bounds	    = new(Name, CREATE_Replace)UBounds		(MaxEdPolys);
	Vectors		= new(VName,CREATE_Replace)UVectors		(MaxVectors);
	Points		= new(PName,CREATE_Replace)UVectors		(MaxPoints);
	//
	Init(1);
	UNGUARD("UModel::UModel");
	};

//
// Kill a model and all stuff it relies on:
//
void UModel::Kill (void)
	{
	GUARD;
	UNGUARD("UModel::Kill");
	};

//
// Build an optional coordinate system and anticoordinate system
// for a model.  Returns orientation, 1.0 if scaling is normal,
// -1.0 if scaling mirrors the brush.
//
FLOAT UModel::BuildCoords (FModelCoords *Coords,FModelCoords *Uncoords)
	{
	GUARD;
	if (Coords!=NULL)
		{
		Coords->PointXform = GMath.UnitCoords;
		if (ModelFlags & MF_PostScale) Coords->PointXform.TransformByScale(PostScale);
		Coords->PointXform.TransformByRotation (Rotation);
		Coords->PointXform.TransformByScale    (Scale);
		//
		Coords->VectorXform = GMath.UnitCoords;
		Coords->VectorXform.DeTransformByScale (Scale);
		Coords->VectorXform.DeTransformByRotation(Rotation);
		if (ModelFlags&MF_PostScale) Coords->VectorXform.DeTransformByScale (PostScale);
		Coords->VectorXform = Coords->VectorXform.Transposition();
		//
		// Note: The NormalCoords system is orthogonal but not orthonormal.
		//
		};
	if (Uncoords!=NULL)
		{
		Uncoords->PointXform = GMath.UnitCoords;
		Uncoords->PointXform.DeTransformByScale (Scale);
		Uncoords->PointXform.DeTransformByRotation(Rotation);
		if (ModelFlags&MF_PostScale) Uncoords->PointXform.DeTransformByScale (PostScale);
		//
		Uncoords->VectorXform = GMath.UnitCoords;
		if (ModelFlags&MF_PostScale) Uncoords->VectorXform.TransformByScale (PostScale);
		Uncoords->VectorXform.TransformByRotation (Rotation);
		Uncoords->VectorXform.TransformByScale	  (Scale);
		Uncoords->VectorXform = Uncoords->VectorXform.Transposition();
		};
	return Scale.Orientation();
	UNGUARD("UModel::BuildCoords");
	};

//
// Build the model's bounds (min and max):
//
void UModel::BuildBound(int Transformed)
	{
	GUARD;
	//
	if ((!Polys) || (Polys->Num==0))
		{
		Bound[Transformed].Init();
		return;
		};
	FModelCoords Coords;
	FVector *FirstPt	= (FVector *)GMem.Get(0);
	FPoly	*Poly		= &Polys->Element(0);
	FLOAT   Orientation = 0.0;
	int		NumPts		= 0;
	//
	if (Transformed) Orientation = BuildCoords(&Coords,NULL);
	//
	for (int i=0; i<Polys->Num; i++)
		{
		FPoly Temp = *Poly;
		if (Transformed) Temp.Transform(Coords,&PrePivot,&Location,Orientation);
		//
		for (int j=0; j<Temp.NumVertices; j++)
			{
			*(FVector *)GMem.GetFast(sizeof(FVector)) = Temp.Vertex[j];
			};
		NumPts += Temp.NumVertices;
		Poly++;
		};
	Bound[Transformed].Init(FirstPt,NumPts);
	GMem.Release(FirstPt);
	//
	UNGUARD("UModel::BuildBound");
	};

void UModel::Transform (void)
	{
	GUARD;
	IModel			ModelInfo;
	FModelCoords	Coords,Uncoords;
	FLOAT			Orientation;
	//
	Lock(&ModelInfo,LOCK_Trans);
	Orientation = BuildCoords(&Coords,&Uncoords);
	//
	ModelInfo.Location += ModelInfo.PostPivot;
	for (INDEX i=0; i<ModelInfo.NumFPolys; i++)
		{
		ModelInfo.FPolys[i].Transform(Coords,&ModelInfo.PrePivot,&ModelInfo.Location,Orientation);
		};
	ModelInfo.Location -= ModelInfo.PostPivot;
	//
	Unlock(&ModelInfo);
	UNGUARD("UModel::Transform");
	};

//
// Set a brush's absolute pivot location, without affecting the brush's
// post-transformation location.  Usually called in map edit mode to force
// all brushes to share a common pivot.
//
// Assumes that brush location and rotation are in their desired positions
// and that any desired snaps have already been applied to it.
//
void UModel::SetPivotPoint (FVector *PivotLocation,int SnapPivotToGrid)
	{
	GUARD;
	FModelCoords	Coords,Uncoords;
	FVector			NewPrePivot;
	//
	GTrans->NoteResHeader (this);
	//
	BuildCoords(&Coords,&Uncoords);
	//
	NewPrePivot  = *PivotLocation - (Location + PostPivot);
	NewPrePivot.TransformVector (Uncoords.PointXform);
	PrePivot += NewPrePivot;
	//
	Location = *PivotLocation;
	if (GEditor) GEditor->constraintApply(NULL,NULL,&Location,&Rotation,&GEditor->Constraints);
	//
	if (SnapPivotToGrid) PostPivot = GMath.ZeroVector;
	else PostPivot = *PivotLocation - Location;
	UNGUARD("UModel::SetPivotPoint");
	};

void UModel::CopyPosRotScaleFrom(UModel *OtherModel)
	{
	GUARD;
	//
	Location	= OtherModel->Location;
	Rotation	= OtherModel->Rotation;
	PrePivot	= OtherModel->PrePivot;
	Scale		= OtherModel->Scale;
	PostPivot	= OtherModel->PostPivot;
	PostScale	= OtherModel->PostScale;
	//
	BuildBound(0);
	BuildBound(1);
	//
	UNGUARD("UModel::CopyPosRotScaleFrom");
	};

/*---------------------------------------------------------------------------------------
	IModel basic implementation (not including physics)
---------------------------------------------------------------------------------------*/

//
// Empty the contents of a model
//
void IModel::Empty(int EmptyPolyInfo)
	{
	NumBspNodes		= 0;
	NumVertPool		= 0;
	NumBounds		= 0;
	NumZones		= 0;
	NumSharedSides	= 4; // First 4 shared sides are view frustrum edges
	NumUniquePlanes	= 0;
	//
	if (EmptyPolyInfo)
		{
		NumVectors		= 0;
		NumPoints		= 0;
		NumBspSurfs		= 0;
		};
	};

/*---------------------------------------------------------------------------------------
	The End
---------------------------------------------------------------------------------------*/
