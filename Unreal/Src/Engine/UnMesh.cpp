/*=============================================================================
	UnMesh.cpp: Unreal mesh animation functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Note: See DRAGON.MAC for a sample import macro

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Mesh info (only used in this file, not globally)
-----------------------------------------------------------------------------*/

void GetMeshResInfo (IMesh *MeshInfo, UMesh *Mesh, BYTE *MeshData)
	{
	GUARD;
	//
	// Get maximums:
	//
	MeshInfo->TriMax      		= (int)Mesh->MaxTriangles  * sizeof (FMeshTriangle);
	MeshInfo->VertexMax   		= (int)Mesh->MaxAnimFrames * (int)Mesh->MaxVertices * sizeof (FMeshVertex);
	MeshInfo->VertLinkIndexMax	= (int)Mesh->MaxVertices   * sizeof (FMeshVertLinkIndex);
	MeshInfo->VertLinkMax		= (int)Mesh->MaxVertLinks  * sizeof (WORD);
	MeshInfo->BoundMax      	= (int)Mesh->MaxAnimFrames * sizeof (FBoundingVolume);
	MeshInfo->AnimMax     		= (int)Mesh->MaxAnimSeqs   * sizeof (FMeshAnimSeq);
	//
	// Calc total:
	//
	MeshInfo->TotalMax			= 	MeshInfo->TriMax + MeshInfo->VertexMax +
									MeshInfo->VertLinkIndexMax + MeshInfo->VertLinkMax +
									MeshInfo->BoundMax + MeshInfo->AnimMax;
	//
	// Get pointers:
	//
	MeshInfo->Triangles			= (FMeshTriangle		*) (MeshData + 0);
	MeshInfo->Vertex			= (FMeshVertex			*) (MeshData + MeshInfo->TriMax);
	MeshInfo->VertLinkIndex		= (FMeshVertLinkIndex	*) (MeshData + MeshInfo->TriMax + MeshInfo->VertexMax);
	MeshInfo->VertLinks			= (WORD 				*) (MeshData + MeshInfo->TriMax + MeshInfo->VertexMax + MeshInfo->VertLinkIndexMax);
	MeshInfo->Bound        		= (FBoundingVolume		*) (MeshData + MeshInfo->TriMax + MeshInfo->VertexMax + MeshInfo->VertLinkIndexMax + MeshInfo->VertLinkMax);
	MeshInfo->AnimSeqs			= (FMeshAnimSeq			*) (MeshData + MeshInfo->TriMax + MeshInfo->VertexMax + MeshInfo->VertLinkIndexMax + MeshInfo->VertLinkMax + MeshInfo->BoundMax);
	//
	UNGUARD("GetMeshResInfo");
	};

//
// Lock a mesh for reading (doesn't handle writing).
// This just fills in the MeshMapInfo structure for easy manipulation.
//
void UMesh::GetInfo (IMesh *MeshInfo)
	{
	GUARD;
	GetMeshResInfo (MeshInfo,this,GetData());
	UNGUARD("UMesh::GetInfo");
	};

/*-----------------------------------------------------------------------------
	UMesh resource implementation
-----------------------------------------------------------------------------*/

void UMesh::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UMesh);
	Type->RecordSize = 0;
	Type->Version    = 1;
	strcpy (Type->Descr,"Mesh");
	UNGUARD("UMesh::Register");
	};
void UMesh::InitHeader(void)
	{
	GUARD;
	//
	// Init resource header to defaults:
	//
	NumAnimFrames	= 0;	MaxAnimFrames = 0;
	NumVertices		= 0;	MaxVertices   = 0;
	NumVertLinks	= 0;	MaxVertLinks  = 0;
	NumTriangles	= 0;	MaxTriangles  = 0;
	//
	Origin			= GMath.ZeroVector;
	RotOrigin		= GMath.ZeroRotation;
	UNGUARD("UMesh::InitHeader");
	};
void UMesh::InitData(void)
	{
	GUARD;
	NumAnimFrames	= 0;
	NumVertices		= 0;
	NumVertLinks	= 0;
	NumTriangles	= 0;
	UNGUARD("UMesh::InitData");
	};
int UMesh::QuerySize(void)
	{
	GUARD;
	IMesh MeshInfo;
	GetMeshResInfo (&MeshInfo,this,GetData());
	return MeshInfo.TotalMax;
	UNGUARD("UMesh::QuerySize");
	};
int UMesh::QueryMinSize(void)
	{
	GUARD;
	return QuerySize();
	UNGUARD("UMesh::QueryMinSize");
	};
void UMesh::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	IMesh		MeshInfo;
	GetMeshResInfo (&MeshInfo,this,GetData());
	for (int i=0; i<NumAnimSeqs; i++)
		{
		Callback.Name (this,&MeshInfo.AnimSeqs[i].SeqName,0);
		};
	UNGUARD("UMesh::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_Mesh,UMesh,0xB2D90858,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	UMeshMap resource implementation
-----------------------------------------------------------------------------*/

void UMeshMap::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UMeshMap);
	Type->RecordSize = 0;
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"MeshMap");
	UNGUARD("UMeshMap::Register");
	};
void UMeshMap::InitHeader(void)
	{
	GUARD;
	Mesh			= NULL;
	MaxTextures		= 0;
	AndFlags		= MAXWORD;
	OrFlags			= 0;
	Scale			= GMath.UnitScaleVect;
	UNGUARD("UMeshMap::InitHeader");
	};
void UMeshMap::InitData(void)
	{
	GUARD;
	for (DWORD i=0; i<MaxTextures; i++) GetData()[i] = NULL;
	UNGUARD("UMeshMap::InitData");
	};
int UMeshMap::QuerySize(void)
	{
	GUARD;
	return MaxTextures * sizeof (UTexture *);
	UNGUARD("UMeshMap::QuerySize");
	};
int UMeshMap::QueryMinSize(void)
	{
	GUARD;
	return QuerySize();
	UNGUARD("UMeshMap::QueryMinSize");
	};
void UMeshMap::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	Callback.Resource (this,(UResource **)&Mesh,0);
	UNGUARD("UMeshMap::QueryHeaderReferences");
	};
void UMeshMap::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	for (DWORD i=0; i<MaxTextures; i++)
		{
		Callback.Resource (this,(UResource **)&GetData()[i],0);
		};
	UNGUARD("UMeshMap::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_MeshMap,UMeshMap,0xB2D90859,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	Mesh sprite functions
-----------------------------------------------------------------------------*/

//
// Lock a sprite for reading (doesn't handle writing).
// This just fills in the MeshMapInfo structure for easy manipulation
//
void UMeshMap::Lock (IMeshMap *MeshMapInfo)
	{
	GUARD;
	IMesh MeshInfo;
	//
	// Get mesh info:
	//
	Mesh->GetInfo(&MeshInfo);
	//
	// Set all MeshMapInfo properties:
	//
	MeshMapInfo->MeshMap		= this;
	MeshMapInfo->Mesh			= Mesh;
	//
	MeshMapInfo->NumTriangles	= Mesh->NumTriangles;
	MeshMapInfo->NumVertices	= Mesh->NumVertices;
	MeshMapInfo->NumVertLinks	= Mesh->NumVertLinks;
	MeshMapInfo->NumAnimFrames	= Mesh->NumAnimFrames;
	MeshMapInfo->NumAnimSeqs	= Mesh->NumAnimSeqs;
	MeshMapInfo->MaxTextures	= MaxTextures;
	//
	MeshMapInfo->Triangles		= MeshInfo.Triangles;
	MeshMapInfo->Vertex			= MeshInfo.Vertex;
	MeshMapInfo->VertLinkIndex  = MeshInfo.VertLinkIndex;
	MeshMapInfo->VertLinks      = MeshInfo.VertLinks;
	MeshMapInfo->Bound          = MeshInfo.Bound;
	MeshMapInfo->AnimSeqs		= MeshInfo.AnimSeqs;
	MeshMapInfo->Textures		= (UTexture **)GetData();
	//
	MeshMapInfo->Origin			= Mesh->Origin;
	MeshMapInfo->RotOrigin      = Mesh->RotOrigin;
	MeshMapInfo->Scale			= Scale;
	//
	UNGUARD("UMeshMap::Lock");
	};

//
// Unlock a mesh map
//
void UMeshMap::Unlock (IMeshMap *MeshMapInfo)
	{
	GUARD;
	UNGUARD("UMeshMap::Unlock");
	};

void UMesh::SetSequence (const char *SeqName, int StartFrame, int NumFrames)
	{
	GUARD;
	FMeshAnimSeq 	*AnimSeq;
	IMesh			MeshInfo;
	//
	if ((NumAnimSeqs+1) >= MaxAnimSeqs)
		{
		debugf (LOG_Info,"Animation sequence table full");
		}
	else
		{
		GetInfo (&MeshInfo);
		//
		AnimSeq = &MeshInfo.AnimSeqs [NumAnimSeqs++];
		//
		AnimSeq->SeqName.Add(SeqName);			// Sequence's name
		AnimSeq->SeqStartFrame 	= StartFrame;	// Starting animation frame
		AnimSeq->SeqNumFrames  	= NumFrames;	// Number of frames in sequence
		AnimSeq->Rate          	= 0;			// Playback rate (scale still undefined)
		};
	UNGUARD("UMesh::SetSequence");
	};

/*-----------------------------------------------------------------------------
	Mesh link topic function
-----------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Mesh",MeshTopicHandler);
void MeshTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UMeshMap		*MeshMap;
	IMesh			MeshInfo;
	FMeshAnimSeq 	*AnimSeq;
	WORD			SeqNum;
	//
	if (!_strnicmp(Item,"NUMANIMSEQS",11))
		{
		if (GetUMeshMap(Item,"NAME=",&MeshMap))
			{
			itoa(MeshMap->Mesh->NumAnimSeqs,Data,10);
			};
		}
	else if (!_strnicmp(Item,"ANIMSEQ",7))
		{
		if (GetUMeshMap(Item,"NAME=",&MeshMap) &&
			(GetWORD(Item,"NUM=",&SeqNum)))
			{
			MeshMap->Mesh->GetInfo(&MeshInfo);
			AnimSeq = &MeshInfo.AnimSeqs [SeqNum];
			//
			if (!AnimSeq->SeqName.IsNone())
				{
				sprintf(Data,"%s                                        %03i %03i",
					AnimSeq->SeqName.Name(),SeqNum,AnimSeq->SeqNumFrames);
				};
			};
		};
	UNGUARD("MeshTopicHandler::Get");
	};
void MeshTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("MeshTopicHandler::Set");
	};
