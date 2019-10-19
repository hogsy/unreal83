/*=============================================================================
	UnEdCSG.cpp: High-level CSG tracking functions for editor

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

//
// Globals:
//
BYTE GFlags1 [MAXWORD+1]; // For fast polygon selection
BYTE GFlags2 [MAXWORD+1];

/*-----------------------------------------------------------------------------
	Level brush tracking
-----------------------------------------------------------------------------*/

//
// Duplicate the specified brush's edpolys and return a new brush.
// Returns NULL if the brush is empty.
//
UModel *FEditor::csgDuplicateBrush(ULevel *Level,UModel *Brush, DWORD PolyFlags, BYTE ModelFlags)
	{
	if (Brush->Polys->Num==0) return NULL;
	//
	char Name[NAME_SIZE];
	GRes.MakeUniqueName(Name,Level->Name,"_S",RES_Model);
	//
	UPolys	*NewUPolys		= new(Name,CREATE_Replace)UPolys(Brush->Polys->Num);
	UModel	*NewBrush		= new(Name,CREATE_Replace)UModel;
	//
	// Copy polys over:
	//
	NewUPolys->Num = Brush->Polys->Num;
	FPoly *Poly    = &NewUPolys->Element(0);
	for (INDEX i=0; i<NewUPolys->Num; i++)
		{
		*Poly = Brush->Polys->Element(i);
		//
		Poly->iBrushPoly = INDEX_NONE;
		if (!Poly->Texture) Poly->Texture = GUnrealEditor.CurrentTexture;
		//
		Poly++;
		};
	//
	// Set new brush's properties:
	//
	NewBrush->CopyHeaderFrom(Brush);
	NewBrush->Polys    = NewUPolys;
	NewBrush->Vectors  = NULL;
	NewBrush->Points   = NULL;
	NewBrush->BspNodes = NULL;
	NewBrush->BspSurfs = NULL;
	NewBrush->VertPool = NULL;
	NewBrush->Terrain  = NULL;
	NewBrush->Bounds   = NULL;
	//
	NewBrush->PolyFlags  = PolyFlags;
	NewBrush->ModelFlags = ModelFlags | (Brush->ModelFlags & MF_PostScale);
	//
	NewBrush->BuildBound(1);
	//
	return NewBrush;
	};

//
// Add a brush to the list of CSG brushes in the level, using a CSG operation, and return 
// a newly-created copy of it.
//
UModel *FEditor::csgAddOperation (UModel *Brush,ULevel *Level, DWORD PolyFlags, ECsgOper CsgOper, BYTE ModelFlags)
	{
	GUARD;
	UArray	*BrushArray	= Level->BrushArray;
	UModel  *NewBrush;
	//
	NewBrush = csgDuplicateBrush(Level,Brush,PolyFlags,ModelFlags);
	if (!NewBrush) return NULL;
	//
	// Save existing level brush array:
	//
	GTrans->NoteBrushArray (Level->BrushArray);
	NewBrush->CsgOper = CsgOper;
	//
	Level->BrushArray->Add(NewBrush);
	//
	return NewBrush;
	UNGUARD("csgAddOperation");
	};

/*-----------------------------------------------------------------------------
	Misc
-----------------------------------------------------------------------------*/

const char *FEditor::csgGetName (ECsgOper CSG)
	{
	GUARD;
	switch (CSG)
		{
		case CSG_Active:		return "Active Brush";
		case CSG_Add:			return "Add Brush";
		case CSG_Subtract:		return "Subtract Brush";
		case CSG_Intersect:		return "Intersect Brush";
		case CSG_Deintersect:	return "Deintersect Brush";
		case CSG_NoCut:			return "No-Cut Brush";
		case CSG_NoTerrain:		return "No-Terrain Brush";
		case CSG_Cutaway:		return "Cutaway-View Brush";
		default:				return "Unknown Brush";
		};
	UNGUARD("csgGetName");
	};

void FEditor::csgInvalidateBsp (ULevel *Level)
	{
	GUARD;
	Level->Model->ModelFlags |= MF_InvalidBsp;
	UNGUARD("csgInvalidateBsp");
	};

/*-----------------------------------------------------------------------------
	CSG Rebuilding
-----------------------------------------------------------------------------*/

//
// Rebuild the level's Bsp from the level's CSG brushes
//
// Note: Needs to be expanded to defragment Bsp polygons as needed (by rebuilding
// the Bsp), so that it doesn't slow down to a crawl on complex levels.
//
void FEditor::csgRebuild (ULevel *Level)
	{
	GUARD;
	UModel		     	*Model = Level->Model;
	UModel				*Brush;
	IModel				ModelInfo;
	int 				NodeCount,PolyCount,LastPolyCount,i,n;
	char				TempStr [80];
	//
	GApp->BeginSlowTask ("Rebuilding geometry",1,0);
	//
	constraintFinishAllSnaps(Level);
	Model->ModelFlags &= ~MF_InvalidBsp;	// Revalidate the Bsp
	MapEdit = 0;							// Turn map editing off
	//
	// Empty the model out:
	//
	Model->Lock(&ModelInfo,LOCK_Trans);
	ModelInfo.Empty(1);
	Model->Unlock(&ModelInfo);
	//
	LastPolyCount = 0;
	n             = Level->BrushArray->Num;
	for (i=1; i<n; i++)
		{
		sprintf(TempStr,"Applying brush %i of %i",i,n);
		GApp->StatusUpdate(TempStr,i,n);
		//
		// See if the Bsp has become badly fragmented and, if so, rebuild:
		//
		PolyCount = Model->BspSurfs->Num;
		NodeCount = Model->BspNodes->Num;
		if ((PolyCount > 2000) && (PolyCount >= 3*LastPolyCount))
			{
			strcat (TempStr,": Refreshing Bsp...");
			GApp->StatusUpdate (TempStr,i,n);
			//
			debug 				(LOG_Bsp,"Map: Rebuilding Bsp");
			bspBuildFPolys		(Level->Model,1);
			bspMergeCoplanars	(Level->Model,0);
			bspBuild			(Level->Model,BSP_Lame,25,0);
			debugf				(LOG_Bsp,"Map: Reduced nodes by %i%%, polys by %i%%",(100*(NodeCount-Model->BspNodes->Num))/NodeCount,(100*(PolyCount-Model->BspSurfs->Num))/PolyCount);
			//
			LastPolyCount = Model->BspSurfs->Num;
			};
		//
		// Perform this CSG operation:
		//
		Brush = Level->BrushArray->Element(i);
		bspBrushCSG (Brush,Level->Model,Brush->PolyFlags,(ECsgOper)Brush->CsgOper,0);
		};
	Model->Lock(&ModelInfo,LOCK_Trans);
	bspBuildBounds(&ModelInfo);
	Model->Unlock(&ModelInfo);
	//
	GApp->EndSlowTask ();
	UNGUARD("csgRebuild");
	};

/*---------------------------------------------------------------------------------------
	Flag setting and searching
---------------------------------------------------------------------------------------*/

//
// Sets and clears all Bsp node flags.  Affects all nodes, even ones that don't
// really exist.
//
void FEditor::polySetAndClearNodeFlags (IModel *ModelInfo, DWORD SetBits, DWORD ClearBits)
	{
	FBspNode 	*Node = &ModelInfo->BspNodes [0];
	BYTE		NewFlags;
	//
	GUARD;
	for (INDEX i=0; i<ModelInfo->NumBspNodes; i++)
		{
		NewFlags = (Node->NodeFlags & ~ClearBits) | SetBits;
		if (NewFlags != Node->NodeFlags)
			{
			if (ModelInfo->Trans) GTrans->NoteBspNode (ModelInfo,i);
			Node->NodeFlags = NewFlags;
			};
		Node++;
		};
	UNGUARD("polySetAndClearNodeFlags");
	};

//
// Sets and clears all Bsp node flags.  Affects all nodes, even ones that don't
// really exist.
//
void FEditor::polySetAndClearPolyFlags (IModel *ModelInfo, DWORD SetBits, DWORD ClearBits,int SelectedOnly, int UpdateMaster)
	{
	FBspSurf 	*Poly = &ModelInfo->BspSurfs [0];
	DWORD		NewFlags;
	//
	GUARD;
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if ((!SelectedOnly) || (Poly->PolyFlags & PF_Selected))
			{
			NewFlags = (Poly->PolyFlags & ~ClearBits) | SetBits;
			if (NewFlags != Poly->PolyFlags)
				{
				if (ModelInfo->Trans) GTrans->NoteBspSurf(ModelInfo,i,UpdateMaster);
				Poly->PolyFlags = NewFlags;
				//
				if (UpdateMaster) polyUpdateMaster (ModelInfo,i,0,0);
				};
			};
		Poly++;
		};
	UNGUARD("polySetAndClearPolyFlags");
	};

/*-----------------------------------------------------------------------------
	Polygon searching
-----------------------------------------------------------------------------*/

typedef void (*POLY_CALLBACK)(IModel *ModelInfo, INDEX iSurf);

//
// Find the Brush EdPoly corresponding to a given Bsp poly:
//
FPoly *FEditor::polyFindMaster (IModel *ModelInfo, INDEX iSurf)
	{
	FPoly						*BrushFPolys;
	FBspSurf 					*Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[iSurf];
	if (!Poly->Brush) return NULL;
	//
	BrushFPolys  = Poly->Brush->Polys->GetData();
	//
	return &BrushFPolys[Poly->iBrushPoly];
	UNGUARD("polyFindMaster");
	};

//
// Update a the master brush EdPoly corresponding to a newly-changed
// poly to reflect its new properties.
//
// Doesn't do any transaction tracking.  Assumes you've called transSelectedBspSurfs.
//
void FEditor::polyUpdateMaster (IModel *ModelInfo,INDEX iSurf,int UpdateTexCoords,int UpdateBase)
	{
	FBspSurf		*Poly;
	FPoly			*MasterEdPoly,*BrushEdPolys;
	UModel			*MasterBrush;
	UPolys			*EdPolysHeader;
	FModelCoords	Uncoords;
	//
	GUARD;
	Poly  			= &ModelInfo->BspSurfs[iSurf];
	if (!Poly->Brush) return;
	//
	MasterBrush		= Poly->Brush;
	EdPolysHeader	= MasterBrush->Polys;
	BrushEdPolys	= MasterBrush->Polys->GetData();
	//
	if (UpdateTexCoords || UpdateBase) Poly->Brush->BuildCoords(NULL,&Uncoords);
	//
	INDEX iEdPoly = Poly->iBrushPoly;
	while (iEdPoly < EdPolysHeader->Num)
		{
		MasterEdPoly = &BrushEdPolys[iEdPoly];
		//
		MasterEdPoly->Texture   = Poly->Texture;
		MasterEdPoly->PanU      = Poly->PanU;
		MasterEdPoly->PanV      = Poly->PanV;
		MasterEdPoly->PolyFlags = Poly->PolyFlags & ~(PF_EdCut|PF_EdProcessed|PF_Selected|PF_Memorized);
		//
		if (UpdateTexCoords || UpdateBase)
			{
			if (UpdateTexCoords)
				{
				MasterEdPoly->TextureU = ModelInfo->FVectors[Poly->vTextureU];
				MasterEdPoly->TextureV = ModelInfo->FVectors[Poly->vTextureV];
				//
				MasterEdPoly->TextureU.TransformVector(Uncoords.VectorXform);
				MasterEdPoly->TextureV.TransformVector(Uncoords.VectorXform);
				};
			if (UpdateBase)
				{
				MasterEdPoly->Base  = ModelInfo->FPoints[Poly->pBase] - MasterBrush->Location;
				MasterEdPoly->Base -= MasterBrush->PostPivot;
				MasterEdPoly->Base.TransformVector(Uncoords.PointXform);
				MasterEdPoly->Base += MasterBrush->PrePivot;
				};
			};
		while (++iEdPoly < EdPolysHeader->Num)
			{
			if (BrushEdPolys[iEdPoly].iLink == Poly->iBrushPoly) break;
			};
		};
	UNGUARD("polyUpdateMaster");
	};

//
// Find all Bsp polys with flags such that SetBits are clear or ClearBits are set.
//
void FEditor::polyFindByFlags (IModel *ModelInfo, DWORD SetBits, DWORD ClearBits, POLY_CALLBACK Callback)
	{
	FBspSurf 	*Poly = &ModelInfo->BspSurfs [0];
	//
	GUARD;
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (((Poly->PolyFlags&SetBits)!=0) || ((Poly->PolyFlags&~ClearBits)!=0))
			{
			Callback (ModelInfo,i);
			};
		Poly++;
		};
	UNGUARD("polyFindByFlags");
	};

//
// Find all Bsp polys corresponding to a particular editor brush resource and polygon index.
// Call with BrushPoly set to INDEX_NONE to find all Bsp polys corresponding to the brush.
//
void FEditor::polyFindByBrush (IModel *ModelInfo, UModel *Brush, INDEX iBrushPoly, POLY_CALLBACK Callback)
	{
	FBspSurf 	*Poly = &ModelInfo->BspSurfs [0];
	//
	GUARD;
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (
			(Poly->Brush == Brush) && 
			((iBrushPoly == INDEX_NONE) || (Poly->iBrushPoly == iBrushPoly))
			)
			{
			Callback (ModelInfo,i);
			};
		Poly++;
		};
	UNGUARD("polyFindByBrush");
	};

//
// Find all Bsp polys corresponding to a particular editor brush resource and polygon index.
// Call with BrushPoly set to INDEX_NONE to find all Bsp polys corresponding to the brush.
//
void FEditor::polyFindByBrushGroupItem (
	IModel *ModelInfo,
	UModel *Brush, INDEX iBrushPoly,
	FName GroupName, FName ItemName,
	POLY_CALLBACK Callback)
	{
	FBspSurf 	*Poly = &ModelInfo->BspSurfs [0];
	//
	GUARD;
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (
			(Poly->Brush == Brush) && 
			((iBrushPoly == INDEX_NONE) || (iBrushPoly  == Poly->iBrushPoly)) &&
			((GroupName.IsNone()) || (GroupName  == polyFindMaster(ModelInfo,i)->GroupName)) &&
			((ItemName.IsNone())  || (ItemName   == polyFindMaster(ModelInfo,i)->ItemName))
			)
			{
			Callback (ModelInfo,i);
			};
		Poly++;
		};
	UNGUARD("polyFindByBrushGroupItem");
	};

/*-----------------------------------------------------------------------------
   All transactional polygon selection functions
-----------------------------------------------------------------------------*/

void FEditor::polyResetSelection (IModel *ModelInfo)
	{
	FBspSurf	*Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		Poly->PolyFlags |= ~(PF_Selected | PF_Memorized);
		Poly++;
		};
	UNGUARD("polyResetSelection");
	};

void FEditor::polySelectAll (IModel *ModelInfo)
	{
	GUARD;
	polySetAndClearPolyFlags 	(ModelInfo,PF_Selected,0,0,0);
	UNGUARD("polySelectAll");
	};

void FEditor::polySelectNone (IModel *ModelInfo)
	{
	GUARD;
	polySetAndClearPolyFlags 	(ModelInfo, 0, PF_Selected,0,0);
	UNGUARD("polySelectNone");
	};

void FEditor::polySelectMatchingGroups (IModel *ModelInfo)
	{
	FBspSurf	*Poly;
	INDEX		i;
	//
	GUARD;
	mymemset (GFlags1,0,sizeof(GFlags1));
	//
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->PolyFlags&PF_Selected) GFlags1[polyFindMaster(ModelInfo,i)->GroupName.Index]=1;
		Poly++;
		};
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if ((GFlags1[polyFindMaster(ModelInfo,i)->GroupName.Index]) && (!(Poly->PolyFlags & PF_Selected)))
			{
			GTrans->NoteBspSurf (ModelInfo,i,0);
			Poly->PolyFlags |= PF_Selected;
			};
		Poly++;
		};
	UNGUARD("polySelectMatchingGroups");
	};

void FEditor::polySetItemNames (IModel *ModelInfo,FName ItemName)
	{
	FPoly		*EdPoly;
	//
	GUARD;
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (ModelInfo->BspSurfs[i].PolyFlags & PF_Selected)
			{
			EdPoly = polyFindMaster(ModelInfo,i);
			EdPoly->ItemName = ItemName;
			};
		};
	UNGUARD("polySetItemNames");
	};

void FEditor::polySelectMatchingItems (IModel *ModelInfo)
	{
	FBspSurf *Poly;
	//
	GUARD;
	mymemset (GFlags1,0,sizeof(GFlags1));
	mymemset (GFlags2,0,sizeof(GFlags2));
	//
	Poly = &ModelInfo->BspSurfs[0];
	INDEX i;
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->Brush)
			{
			if (Poly->PolyFlags&PF_Selected) GFlags2[Poly->Brush->Index]=1;
			};
		if (Poly->PolyFlags&PF_Selected) GFlags1[polyFindMaster(ModelInfo,i)->ItemName.Index]=1;
		Poly++;
		};
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->Brush)
			{
			if ((GFlags1[polyFindMaster(ModelInfo,i)->ItemName.Index]) &&
				( GFlags2[Poly->Brush->Index]) &&
				(!(Poly->PolyFlags & PF_Selected)))
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags |= PF_Selected;
				};
			};
		Poly++;
		};
	UNGUARD("polySelectMatchingItems");
	};

enum EAdjacentsType
	{
	ADJACENT_ALL,		// All adjacent polys
	ADJACENT_COPLANARS,	// Adjacent coplanars only
	ADJACENT_WALLS,		// Adjacent walls
	ADJACENT_FLOORS,	// Adjacent floors or ceilings
	ADJACENT_SLANTS,	// Adjacent slants
	};

//
// Select all adjacent polygons (only coplanars if Coplanars==1) and
// return number of polygons newly selected.
//
int TagAdjacentsType (IModel *ModelInfo, EAdjacentsType AdjacentType)
	{
	GUARD;
	FBspNode	*Node;
	FBspSurf 	*Poly;
	FVertPool	*VertPool;
	FVector		*Base,*Normal;
	BYTE		b;
	INDEX		i;
	int			Selected,Found;
	//
	Selected = 0;
	mymemset (GFlags1,0,sizeof(GFlags1));
	//
	// Find all points corresponding to selected vertices:
	//
	Node = &ModelInfo->BspNodes[0];
	for (i=0; i<ModelInfo->NumBspNodes; i++)
		{
		Poly = &ModelInfo->BspSurfs[Node->iSurf];
		if (Poly->PolyFlags & PF_Selected)
			{
			VertPool = &ModelInfo->VertPool [Node->iVertPool];
			//
			for (b=0; b<Node->NumVertices; b++) GFlags1[(VertPool++)->pVertex] = 1;
			};
		Node++;
		};
	//
	// Select all unselected nodes for which two or more vertices are selected:
	//
	Node = &ModelInfo->BspNodes[0];
	for (i=0; i<ModelInfo->NumBspNodes; i++)
		{
		Poly = &ModelInfo->BspSurfs[Node->iSurf];
		if (!(Poly->PolyFlags & PF_Selected))
			{
			Found    = 0;
			VertPool	= &ModelInfo->VertPool [Node->iVertPool];
			//
			Base   = &ModelInfo->FPoints [Poly->pBase];
			Normal = &ModelInfo->FVectors[Poly->vNormal];
			//
			for (b=0; b<Node->NumVertices; b++) Found += GFlags1[(VertPool++)->pVertex];
			//
			if (AdjacentType == ADJACENT_COPLANARS)
				{
				if (!GFlags2[Node->iSurf]) Found=0;
				}
			else if (AdjacentType == ADJACENT_FLOORS)
				{
				if (OurAbs(Normal->Z) <= 0.85) Found = 0;
				}
			else if (AdjacentType == ADJACENT_WALLS)
				{
				if (OurAbs(Normal->Z) >= 0.10) Found = 0;
				}
			else if (AdjacentType == ADJACENT_SLANTS)
				{
				if (OurAbs(Normal->Z) > 0.85) Found = 0;
				if (OurAbs(Normal->Z) < 0.10) Found = 0;
				};
			if (Found > 0)
				{
				GTrans->NoteBspSurf (ModelInfo,Node->iSurf,0);
				Poly->PolyFlags |= PF_Selected;
				Selected++;
				};
			};
		Node++;
		};
	return Selected;
	UNGUARD("TagEAdjacentsType");
	};

void TagCoplanars (IModel *ModelInfo)
	{
	FBspSurf	*SelectedPoly,*Poly;
	FVector		*SelectedBase,*SelectedNormal,*Base,*Normal;
	//
	GUARD;
	mymemset (GFlags2,0,sizeof(GFlags2));
	//
	SelectedPoly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (SelectedPoly->PolyFlags & PF_Selected)
			{
			SelectedBase   = &ModelInfo->FPoints [SelectedPoly->pBase];
			SelectedNormal = &ModelInfo->FVectors[SelectedPoly->vNormal];
			//
			Poly = &ModelInfo->BspSurfs[0];
			for (INDEX j=0; j<ModelInfo->NumBspSurfs; j++)
				{
				Base   = &ModelInfo->FPoints [Poly->pBase];
				Normal = &ModelInfo->FVectors[Poly->vNormal];
				//
				if (FCoplanar(*Base,*Normal,*SelectedBase,*SelectedNormal) && (!(Poly->PolyFlags & PF_Selected)))
					{
					GFlags2[j]=1;
					};
				Poly++;
				};
			};
		SelectedPoly++;
		};
	UNGUARD("TagCoplanars");
	};

void FEditor::polySelectAdjacents (IModel *ModelInfo)
	{
	GUARD;
	do {} while (TagAdjacentsType (ModelInfo,ADJACENT_ALL) > 0);
	UNGUARD("polySelectAdjancents");
	};

void FEditor::polySelectCoplanars (IModel *ModelInfo)
	{
	GUARD;
	TagCoplanars(ModelInfo);
	do {} while (TagAdjacentsType(ModelInfo,ADJACENT_COPLANARS) > 0);
	UNGUARD("polySelectCoplanars");
	};

void FEditor::polySelectMatchingBrush (IModel *ModelInfo)
	{
	FBspSurf	*Poly;
	INDEX		i;
	//
	GUARD;
	mymemset (GFlags1,0,sizeof(GFlags1));
	//
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->Brush)
			{
			if (Poly->PolyFlags&PF_Selected) GFlags1[Poly->Brush->Index]=1;
			};
		Poly++;
		};
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->Brush)
			{
			if ((GFlags1[Poly->Brush->Index])&&(!(Poly->PolyFlags&PF_Selected)))
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags |= PF_Selected;
				};
			};
		Poly++;
		};
	UNGUARD("polySelectMatchingBrush");
	};

void FEditor::polySelectMatchingTexture (IModel *ModelInfo)
	{
	FBspSurf	*Poly;
	INDEX		i,Blank=0;
	//
	GUARD;
	mymemset (GFlags1,0,sizeof(GFlags1));
	//
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->Texture && (Poly->PolyFlags&PF_Selected)) GFlags1[Poly->Texture->Index]=1;
		else if (!Poly->Texture) Blank=1;
		Poly++;
		};
	Poly = &ModelInfo->BspSurfs[0];
	for (i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->Texture && (GFlags1[Poly->Texture->Index]) && (!(Poly->PolyFlags&PF_Selected)))
			{
			GTrans->NoteBspSurf (ModelInfo,i,0);
			Poly->PolyFlags |= PF_Selected;
			}
		else if (Blank & !Poly->Texture) Poly->PolyFlags |= PF_Selected;
		Poly++;
		};
	UNGUARD("polySelectMatchingTexture");
	};

void FEditor::polySelectAdjacentWalls (IModel *ModelInfo)
	{
	GUARD;
	do {} while (TagAdjacentsType  (ModelInfo,ADJACENT_WALLS) > 0);
	UNGUARD("polySelectAdjacentWalls");
	};

void FEditor::polySelectAdjacentFloors (IModel *ModelInfo)
	{
	GUARD;
	do {} while (TagAdjacentsType (ModelInfo,ADJACENT_FLOORS) > 0);
	UNGUARD("polySelectAdjacentFloors");
	};

void FEditor::polySelectAdjacentSlants (IModel *ModelInfo)
	{
	GUARD;
	do {} while (TagAdjacentsType  (ModelInfo,ADJACENT_SLANTS) > 0);
	UNGUARD("polySelectEAdjacentsTypelants");
	};

void FEditor::polySelectReverse (IModel *ModelInfo)
	{
	FBspSurf	*Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		GTrans->NoteBspSurf (ModelInfo,i,0);
		Poly->PolyFlags ^= PF_Selected;
		//
		Poly++;
		};
	UNGUARD("polySelectReverse");
	};

void FEditor::polyMemorizeSet (IModel *ModelInfo)
	{
	FBspSurf *Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->PolyFlags & PF_Selected) 
			{
			if (!(Poly->PolyFlags & PF_Memorized))
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags |= (PF_Memorized);
				};
			}
		else
			{
			if (Poly->PolyFlags & PF_Memorized)
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags &= (~PF_Memorized);
				};
			};
		Poly++;
		};
	UNGUARD("polyMemorizeSet");
	};

void FEditor::polyRememberSet (IModel *ModelInfo)
	{
	FBspSurf *Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->PolyFlags & PF_Memorized) 
			{
			if (!(Poly->PolyFlags & PF_Selected))
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags |= (PF_Selected);
				};
			}
		else
			{
			if (Poly->PolyFlags & PF_Selected)
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags &= (~PF_Selected);
				};
			};
		Poly++;
		};
	UNGUARD("polyRememberSet");
	};

void FEditor::polyXorSet (IModel *ModelInfo)
	{
	FBspSurf	*Poly;
	int			Flag1,Flag2;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		Flag1 = (Poly->PolyFlags & PF_Selected ) != 0;
		Flag2 = (Poly->PolyFlags & PF_Memorized) != 0;
		//
		if (Flag1 ^ Flag2)
			{
			if (!(Poly->PolyFlags & PF_Selected))
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags |= PF_Selected;
				};
			}
		else
			{
			if (Poly->PolyFlags & PF_Selected)
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags &= (~PF_Selected);
				};
			};
		Poly++;
		};
	UNGUARD("polyXorSet");
	};

void FEditor::polyUnionSet (IModel *ModelInfo)
	{
	FBspSurf	*Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (!(Poly->PolyFlags & PF_Memorized))
			{
			if (Poly->PolyFlags | PF_Selected)
				{
				GTrans->NoteBspSurf (ModelInfo,i,0);
				Poly->PolyFlags &= (~PF_Selected);
				};
			};
		Poly++;
		};
	UNGUARD("polyUnionSet");
	};

void FEditor::polyIntersectSet (IModel *ModelInfo)
	{
	FBspSurf		*Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if ((Poly->PolyFlags & PF_Memorized) && !(Poly->PolyFlags & PF_Selected))
			{
			Poly->PolyFlags |= PF_Selected;
			};
		Poly++;
		};
	UNGUARD("polyIntersectSet");
	};

/*---------------------------------------------------------------------------------------
   Brush selection functions
---------------------------------------------------------------------------------------*/

//
// Generic selection routines
//

typedef int (*BRUSH_SEL_FUNC)(UModel *Brush,int Tag);

void MapSelect (ULevel *Level,BRUSH_SEL_FUNC Func,int Tag)
	{
	GUARD;
	ILevel	LevelInfo;
	UModel		*Brush;
	//
	Level->Lock (&LevelInfo,LOCK_Trans);
	//
	int n = LevelInfo.BrushArray->Num;
	for (int i=0; i<n; i++)
		{
		Brush = LevelInfo.BrushArray->Element(i);
		//
		if (Func (Brush,Tag)) // Select it
			{
			if (!(Brush->ModelFlags & MF_Selected))
				{
				GTrans->NoteResHeader (Brush);
				Brush->ModelFlags |= MF_Selected;
				};
			}
		else // Deselect it
			{
			if (Brush->ModelFlags & MF_Selected)
				{
				GTrans->NoteResHeader (Brush);
				Brush->ModelFlags &= ~MF_Selected;
				};
			};
		};
	Level->Unlock (&LevelInfo);
	UNGUARD("MapSelect");
	};

//
// Select all
//
int  BrushSelectAllFunc (UModel *Brush,int Tag) {return 1;};
void FEditor::mapSelectAll (ULevel *Level)
	{
	GUARD;
	MapSelect (Level,BrushSelectAllFunc,0);
	UNGUARD("mapSelectAll");
	};

//
// Select none
//
int BrushSelectNoneFunc (UModel *Brush,int Tag) {return 0;};
void FEditor::mapSelectNone (ULevel *Level)
	{
	GUARD;
	MapSelect (Level,BrushSelectNoneFunc,0);
	UNGUARD("mapSelectNone");
	};

//
// Select by CSG operation
//
int  BrushSelectOperationFunc (UModel *Brush,int Tag) {return (Brush->CsgOper == Tag) && !(Brush->PolyFlags & (PF_NotSolid | PF_Semisolid));};
void FEditor::mapSelectOperation (ULevel *Level,ECsgOper CsgOper)
	{
	GUARD;
	MapSelect (Level,BrushSelectOperationFunc,CsgOper);
	UNGUARD("mapSelectOperation");
	};

int  BrushSelectFlagsFunc (UModel *Brush,int Tag) {return (Brush->PolyFlags & Tag);};
void FEditor::mapSelectFlags (ULevel *Level,DWORD Flags)
	{
	GUARD;					   
	MapSelect (Level,BrushSelectFlagsFunc,(int)Flags);
	UNGUARD("mapSelectFlags");
	};

void MapSelectSeq (ULevel *Level,int Delta)
	{
	ILevel 		LevelInfo;
	UModel			*Brush,*OtherBrush;
	int				i,j,n;
	//
	GUARD;
	Level->Lock (&LevelInfo,LOCK_Trans);
	//
	n = LevelInfo.BrushArray->Num;
	for (i=1; i<n; i++)
		{
		Brush = LevelInfo.BrushArray->Element(i);
		Brush->ModelFlags &= ~MF_Temp;
		j 	  = i+Delta;
		//
		if ((j>=1)&&(j<n))
			{
			OtherBrush = LevelInfo.BrushArray->Element(j);
			if (OtherBrush->ModelFlags & MF_Selected) Brush->ModelFlags |= MF_Temp;
			};
		};
	for (i=1; i<n; i++)
		{
		Brush = LevelInfo.BrushArray->Element(i);
		//
		if (Brush->ModelFlags & MF_Temp)
			{
			if (!(Brush->ModelFlags & MF_Selected))
				{
				GTrans->NoteResHeader (Brush);
				Brush->ModelFlags |= MF_Selected;
				};
			}
		else
			{
			if (Brush->ModelFlags & MF_Selected)
				{
				GTrans->NoteResHeader (Brush);
				Brush->ModelFlags &= ~MF_Selected;
				};
			};
		};
	Level->Unlock(&LevelInfo);
	UNGUARD("mapSelectSeq");
	};

//
// Select brushes previous to selected brushes
//
void FEditor::mapSelectPrevious (ULevel *Level)
	{
	GUARD;
	MapSelectSeq (Level,1);
	UNGUARD("mapSelectPrevious");
	};

//
// Select brushes after selected brushes
//
void FEditor::mapSelectNext (ULevel *Level)
	{
	GUARD;
	MapSelectSeq (Level,-1);
	UNGUARD("mapSelectNext");
	};

//
// Select first or last
//
void FEditor::mapSelectFirst (ULevel *Level)
	{
	GUARD;
	ILevel 		LevelInfo;
	UModel			*Brush;
	//
	MapSelect (Level,BrushSelectNoneFunc,0);
	Level->Lock (&LevelInfo,LOCK_Trans);
	//
	if (LevelInfo.BrushArray->Num >= 1)
		{
		Brush = LevelInfo.BrushArray->Element(1);
		GTrans->NoteResHeader(Brush);
		Brush->ModelFlags |= MF_Selected;
		};
	Level->Unlock (&LevelInfo);
	UNGUARD("mapSelectFirst");
	};

void FEditor::mapSelectLast (ULevel *Level)
	{
	GUARD;
	ILevel 		LevelInfo;
	UModel			*Brush;
	int				n;
	//
	MapSelect (Level,BrushSelectNoneFunc,0);
	Level->Lock (&LevelInfo,LOCK_Trans);
	//
	n = LevelInfo.BrushArray->Num;
	if (n >= 1)
		{
		Brush = LevelInfo.BrushArray->Element(n-1);
		GTrans->NoteResHeader (Brush);
		Brush->ModelFlags |= MF_Selected;
		};
	Level->Unlock (&LevelInfo);
	UNGUARD("mapSelectLast");
	};

/*---------------------------------------------------------------------------------------
   Other map brush functions
---------------------------------------------------------------------------------------*/

void CopyBrushEdPolys (UModel *DestBrush,UModel *SourceBrush,int Realloc)
	{
	IModel			ModelInfo;
	FPoly				*SourceFPolys, *DestFPolys;
	//
	GUARD;
	//
	// Save all old destination EdPolys for undo
	//
	DestBrush->Lock		(&ModelInfo,LOCK_Trans);
	GTrans->NoteFPoly	(&ModelInfo,INDEX_NONE);
	DestBrush->Unlock	(&ModelInfo);
	//
	DestBrush->Polys	= new(DestBrush->Name,CREATE_Replace)UPolys(SourceBrush->Polys->Num,1);
	DestFPolys 			= DestBrush->Polys->GetData();
	SourceFPolys 		= SourceBrush->Polys->GetData();
	//
	memcpy (DestFPolys,SourceFPolys,SourceBrush->Polys->QueryMinSize());
	//
	UNGUARD("CopyBrushEdPolys");
	};

//
// Put the first selected brush into the current brush.
//
void FEditor::mapBrushGet (ULevel *Level)
	{
	GUARD;
	ILevel 	LevelInfo;
	UModel		*Brush;
	int			i,Done;
	//
	Level->Lock(&LevelInfo,LOCK_Trans);
	GTrans->NoteBrushArray (LevelInfo.BrushArray);
	//
	Done = 0;
	for (i=1; i<LevelInfo.BrushArray->Num; i++)
		{
		Brush = LevelInfo.BrushArray->Element(i);
		//
		if (Brush->ModelFlags & MF_Selected)
			{
			GTrans->NoteResHeader (Brush);
			//
			CopyBrushEdPolys (LevelInfo.Brush,Brush,0);
			LevelInfo.Brush->CopyPosRotScaleFrom(Brush);
			//
			break;
			};
		};
	Level->Unlock (&LevelInfo);
	UNGUARD("mapBrushGet");
	};

//
// Replace all selected brushes with the current brush.
//
void FEditor::mapBrushPut (ULevel *Level)
	{
	GUARD;
	ILevel 		LevelInfo;
	UModel			*Brush;
	int				i;
	//
	Level->Lock (&LevelInfo,LOCK_Trans);
	//
	GTrans->NoteBrushArray (LevelInfo.BrushArray);
	//
	for (i=1; i<LevelInfo.BrushArray->Num; i++)
		{
		Brush = LevelInfo.BrushArray->Element(i);
		if (Brush->ModelFlags & MF_Selected)
			{
			CopyBrushEdPolys(Brush,LevelInfo.Brush,1);
			Brush->CopyPosRotScaleFrom(LevelInfo.Brush);
			};
		};
	Level->Unlock (&LevelInfo);
	UNGUARD("mapBrushPut");
	};

//
// Delete all selected brushes in a level
//
void FEditor::mapDelete (ULevel *Level)
	{
	GUARD;
	ILevel 		LevelInfo;
	UModel			*Brush;
	int				i,n;
	//
	Level->Lock (&LevelInfo,LOCK_Trans);
	//
	n = LevelInfo.BrushArray->Num;
	//
	GTrans->NoteBrushArray (LevelInfo.BrushArray);
	//
	for (i=1; i<n; i++) // Don't delete active brush
		{
		Brush = LevelInfo.BrushArray->Element(i);
		if (Brush->ModelFlags & MF_Selected)
			{
			LevelInfo.BrushArray->Delete(Brush);
			i--;
			n--;
			};
		};
	Level->Unlock (&LevelInfo);
	UNGUARD("mapDelete");
	};

//
// Duplicate all selected brushes on a level
//
void FEditor::mapDuplicate (ULevel *Level)
	{
	GUARD;
	FVector  		Delta = {32.0,32.0,32.0};
	ILevel 		LevelInfo;
	UModel			*Brush,*NewBrush;
	//
	Level->Lock (&LevelInfo,LOCK_Trans);
	GTrans->NoteBrushArray (LevelInfo.BrushArray);
	//
	int n = LevelInfo.BrushArray->Num;
	for (int i=0; i<n; i++)
		{
		Brush = LevelInfo.BrushArray->Element(i);
		if (Brush->ModelFlags & MF_Selected)
			{
			NewBrush = csgAddOperation (Brush,Level,Brush->PolyFlags,(ECsgOper)Brush->CsgOper,Brush->ModelFlags);
			//
			GTrans->NoteResHeader (Brush);
			Brush->ModelFlags &= ~MF_Selected;
			//
			NewBrush->Location += Delta;
			};
		};
	Level->Unlock (&LevelInfo);
	UNGUARD("mapDuplicate");
	};

//
// Generic private routine for send to front / send to back
//
void SendTo (ULevel *Level,int SendToFirstXOR)
	{
	GUARD;
	ILevel		LevelInfo;
	UModel		*Brush;
	UModel		**BrushArray,**TempArray;
	int			i,Num,Count;
	//
	Level->Lock (&LevelInfo,LOCK_Trans);
	GTrans->NoteBrushArray (LevelInfo.BrushArray);
	//
	Num			= LevelInfo.BrushArray->Num;
	BrushArray	= (UModel **)LevelInfo.BrushArray->GetData();
	TempArray	= (UModel **)GMem.Get(Num*sizeof(UModel *));
	Count       = 0;
	//
	TempArray[Count++] = BrushArray[0]; // Active brush
	//
	// Pass 1: Copy stuff to front
	//
	for (i=1; i<Num; i++)
		{
		Brush = BrushArray[i];
		if ((Brush->ModelFlags ^ SendToFirstXOR) & MF_Selected)
			TempArray[Count++] = BrushArray[i];
		};
	//
	// Pass 2: Copy stuff to back
	//
	for (i=1; i<Num; i++)
		{
		Brush = BrushArray[i];
		if (!((Brush->ModelFlags ^ SendToFirstXOR) & MF_Selected))
			TempArray[Count++] = BrushArray[i];
		};
	//
	// Finish up:
	//
	memcpy(BrushArray,TempArray,Num*sizeof(UModel *));
	GMem.Release(TempArray);
	Level->Unlock (&LevelInfo);
	UNGUARD("SendTo");
	};

//
// Send all selected brushes in a level to the front of the hierarchy
//
void FEditor::mapSendToFirst (ULevel *Level)
	{
	GUARD;
	SendTo(Level,0);
	UNGUARD("mapSendToFirst");
	};

//
// Send all selected brushes in a level to the back of the hierarchy
//
void FEditor::mapSendToLast (ULevel *Level)
	{
	GUARD;
	SendTo(Level,MF_Selected);
	UNGUARD("mapSendToLast");
	};

void FEditor::mapSetBrush (ULevel *Level,EMapSetBrushFlags PropertiesMask,WORD BrushColor,FName GroupName,
	DWORD SetPolyFlags,DWORD ClearPolyFlags)
	{
	GUARD;
	ILevel		LevelInfo;
	UModel			*Brush;
	IModel		BrushInfo;
	//
	Level->Lock(&LevelInfo,LOCK_Trans);
	for (int i=1; i<LevelInfo.BrushArray->Num; i++)
		{
		Brush = LevelInfo.BrushArray->Element(i);
		if (Brush->ModelFlags & MF_Selected)
			{
			GTrans->NoteResHeader (Brush);
			if (PropertiesMask & MSB_BrushColor)
				{
				if (BrushColor==65535) // Remove color
					{
					Brush->ModelFlags &= ~MF_Color;
					}
				else // Set color
					{
					Brush->Color		= BrushColor;
					Brush->ModelFlags  |= MF_Color;
					};
				};
			if (PropertiesMask & MSB_Group)
				{
				Brush->Lock (&BrushInfo,LOCK_Trans);
				GTrans->NoteFPoly (&BrushInfo,INDEX_NONE);
				for (INDEX j=0; j<BrushInfo.NumFPolys; j++)
					{
					BrushInfo.FPolys[j].GroupName = GroupName;
					};
				Brush->Unlock(&BrushInfo);
				};
			if (PropertiesMask & MSB_PolyFlags)
				{
				Brush->PolyFlags = (Brush->PolyFlags & ~ClearPolyFlags) | SetPolyFlags;
				};
			};
		};
	Level->Unlock(&LevelInfo);
	UNGUARD("mapSetBrush");
	};

/*---------------------------------------------------------------------------------------
   Poly texturing operations
---------------------------------------------------------------------------------------*/

//
// Pan textures on selected polys.  Doesn't do transaction tracking.
//
void FEditor::polyTexPan (IModel *ModelInfo,int PanU,int PanV,int Absolute)
	{
	FBspSurf	*Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->PolyFlags & PF_Selected)
			{
			if (Absolute)
				{
				Poly->PanU = PanU;
				Poly->PanV = PanV;
				}
			else // Relative
				{
				Poly->PanU += PanU;
				Poly->PanV += PanV;
				};
			polyUpdateMaster (ModelInfo,i,0,0);
			};
		Poly++;
		};
	UNGUARD("polyTexPan");
	};

//
// Scale textures on selected polys. Doesn't do transaction tracking.
//
void FEditor::polyTexScale (IModel *ModelInfo,FLOAT UU,FLOAT UV, FLOAT VU, FLOAT VV,int Absolute)
	{
	FVector		OriginalU,OriginalV;
	FVector		NewU,NewV;
	FBspSurf	*Poly;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->PolyFlags & PF_Selected)
			{
			OriginalU = ModelInfo->FVectors [Poly->vTextureU];
			OriginalV = ModelInfo->FVectors [Poly->vTextureV];
			//
			if (Absolute)
				{
				OriginalU *= 65536.0/OriginalU.Size();
				OriginalV *= 65536.0/OriginalV.Size();
				};
			//
			// Calc new vectors:
			//
			NewU = OriginalU * UU + OriginalV * UV;
			NewV = OriginalU * VU + OriginalV * VV;
			//
			// Update Bsp poly:
			//
			Poly->vTextureU = bspAddVector (ModelInfo,&NewU,0); // Add U vector
			Poly->vTextureV = bspAddVector (ModelInfo,&NewV,0); // Add V vector
			//
			// Update generating brush poly:
			//
			polyUpdateMaster	(ModelInfo,i,1,0);
			bspRefresh			(ModelInfo,0);
			//
			Poly->iLightMesh	= INDEX_NONE;
			};
		Poly++;
		};
	UNGUARD("polyTexScale");
	};

//
// Align textures on selected polys.  Doesn't do any transaction tracking.
//
void FEditor::polyTexAlign (IModel *ModelInfo,ETexAlign TexAlignType,DWORD Texels)
	{
	FBspSurf		*Poly;
	FPoly			EdPoly;
	FVector			Base,Normal,U,V,Temp,OldBase;
	FModelCoords	Coords,Uncoords;
	FLOAT			BaseZ,Orientation,k;
	//
	GUARD;
	Poly = &ModelInfo->BspSurfs[0];
	for (INDEX i=0; i<ModelInfo->NumBspSurfs; i++)
		{
		if (Poly->PolyFlags & PF_Selected)
			{
			EdPoly = *polyFindMaster (ModelInfo,i);
			Normal = ModelInfo->FVectors [Poly->vNormal];
			//
			switch (TexAlignType)
				{
				case TEXALIGN_Default:
					//
					Orientation = Poly->Brush->BuildCoords (&Coords,&Uncoords);
					//
					EdPoly.TextureU  = GMath.ZeroVector; // Force recalc
					EdPoly.TextureV  = GMath.ZeroVector; // Force recalc
					EdPoly.Base.X    = MAXSWORD;    // Force recalc
					EdPoly.PanU      = 0;
					EdPoly.PanV      = 0;
					EdPoly.Finalize  (0);
					EdPoly.Transform (Coords,&GMath.ZeroVector,&GMath.ZeroVector,Orientation);
					//
		      		Poly->vTextureU 	= bspAddVector (ModelInfo,&EdPoly.TextureU,0);
	      			Poly->vTextureV 	= bspAddVector (ModelInfo,&EdPoly.TextureV,0);
					Poly->PanU			= EdPoly.PanU;
					Poly->PanV			= EdPoly.PanV;
					Poly->iLightMesh	= INDEX_NONE;
					//
					polyUpdateMaster	(ModelInfo,i,1,1);
					break;
				case TEXALIGN_Floor:
					//
					if (OurAbs(Normal.Z) > 0.05)
						{
						//
						// Shouldn't change base point, just base U,V
						//
						Base           	= ModelInfo->FPoints  [Poly->pBase];
						OldBase         = Base;
						BaseZ          	= (Base | Normal) / Normal.Z;
						Base       		= GMath.ZeroVector;
						Base.Z     		= BaseZ;
			      		Poly->pBase 	= bspAddPoint (ModelInfo,&Base,1);
						//
						Temp.X 			= 65536.0;
						Temp.Y			= 0.0;
						Temp.Z			= 0.0;
						Temp			= Temp - Normal * (Temp | Normal);
						Poly->vTextureU	= bspAddVector (ModelInfo,&Temp,0);
						//
						Temp.X			= 0.0;
						Temp.Y 			= 65536.0;
						Temp.Z			= 0.0;
						Temp			= Temp - Normal * (Temp | Normal);
						Poly->vTextureV	= bspAddVector (ModelInfo,&Temp,0);
						//
						Poly->PanU       = 0;
						Poly->PanV       = 0;
						Poly->iLightMesh = INDEX_NONE;
						};
					polyUpdateMaster (ModelInfo,i,1,1);
					break;
				case TEXALIGN_WallDir:
					//
					// Align texture U,V directions for walls
					// U = (Nx,Ny,0)/sqrt(Nx^2+Ny^2)
					// V = (U dot N) normalized and stretched so Vz=1
					//
					if (OurAbs(Normal.Z)<0.95)
						{
						U.X = +Normal.Y;
						U.Y = -Normal.X;
						U.Z = 0.0;
						U  *= 65536.0/U.Size();
						V   = (U ^ Normal);
						V  *= 65536.0/V.Size();
						//
						if (V.Z > 0.0)
							{
							V *= -1.0;
							U *= -1.0;
							};
						Poly->vTextureU = bspAddVector (ModelInfo,&U,0);
						Poly->vTextureV = bspAddVector (ModelInfo,&V,0);
						//
						Poly->PanU			= 0;
						Poly->PanV			= 0;
						Poly->iLightMesh	= INDEX_NONE;
						//
						polyUpdateMaster (ModelInfo,i,1,0);
						};
					break;
				case TEXALIGN_WallPan:
					Base = ModelInfo->FPoints  [Poly->pBase];
					U    = ModelInfo->FVectors [Poly->vTextureU];
					V    = ModelInfo->FVectors [Poly->vTextureV];
					//
					if ((OurAbs(Normal.Z)<0.95) && (OurAbs(V.Z)>0.05))
						{
						k     = -Base.Z/V.Z;
						V    *= k;
						Base += V;
			      		Poly->pBase = bspAddPoint (ModelInfo,&Base,1);
						Poly->iLightMesh = INDEX_NONE;
						//
						polyUpdateMaster(ModelInfo,i,1,1);
						};
					break;
				case TEXALIGN_OneTile:
					Poly->iLightMesh = INDEX_NONE;
					polyUpdateMaster (ModelInfo,i,1,1);
					break;
				};
			bspRefresh (ModelInfo,0);
			};
		Poly++;
		};
	UNGUARD_BEGIN
	UNGUARD_MSGF("polyTexAlign(Type=%i,Texels=%i",TexAlignType,Texels);
	UNGUARD_END
	};

/*---------------------------------------------------------------------------------------
   Map geometry link topic handler
---------------------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Map",MapTopicHandler);
void MapTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	UModel			*Brush;
	UPolys			*BrushEdPolys;
	IModel			ModelInfo;
	int				i,NumBrushes,NumPolys,NumAdd,NumSubtract,NumSpecial;
	//
	if ((!Level)||(!Level->BrushArray)) return;
	//
	NumBrushes  = Level->BrushArray->Num;
	NumAdd	    = 0;
	NumSubtract	= 0;
	NumSpecial  = 0;
	NumPolys    = 0;
	//
	for (i=1; i<NumBrushes; i++)
		{
		Brush        = Level->BrushArray->Element(i);
		BrushEdPolys = Brush->Polys;
		//
		if      (Brush->CsgOper == CSG_Add)			NumAdd++;
		else if (Brush->CsgOper == CSG_Subtract)	NumSubtract++;
		else										NumSpecial++;
		//
		NumPolys += BrushEdPolys->Num;
		};
	Brush = Level->Model;
	Brush->Lock(&ModelInfo,LOCK_Read);
	//
	if      (stricmp(Item,"Brushes"     )==0) itoa (NumBrushes-1,Data,10);
	else if (stricmp(Item,"Add"         )==0) itoa (NumAdd,Data,10);
	else if (stricmp(Item,"Subtract"    )==0) itoa (NumSubtract,Data,10);
	else if (stricmp(Item,"Special"     )==0) itoa (NumSpecial,Data,10);
	else if (stricmp(Item,"AvgPolys"    )==0) itoa (NumPolys/OurMax(1,NumBrushes-1),Data,10);
	else if (stricmp(Item,"TotalPolys"  )==0) itoa (NumPolys,Data,10);
	else if (stricmp(Item,"Points"		)==0) itoa (ModelInfo.NumPoints,Data,10);
	else if (stricmp(Item,"Vectors"		)==0) itoa (ModelInfo.NumVectors,Data,10);
	else if (stricmp(Item,"Sides"		)==0) itoa (ModelInfo.NumSharedSides,Data,10);
	else if (stricmp(Item,"Zones"		)==0) itoa (ModelInfo.NumZones-1,Data,10);
	else if (stricmp(Item,"Bounds"		)==0) itoa (ModelInfo.NumBounds,Data,10);
	else if (stricmp(Item,"Planes"		)==0) itoa (ModelInfo.NumUniquePlanes,Data,10);
	else if (!stricmp(Item,"DuplicateBrush"))
		{
		//
		// Make a unique copy of the current brush and return its name:
		//
		UModel *PlaceAt = NULL;
		GetUModel(Data,"PLACEAT=",&PlaceAt);
		//
		UModel *NewBrush = GEditor->csgDuplicateBrush(Level,Level->BrushArray->Element(0),0,0);
		if (NewBrush) strcpy(Data,NewBrush->Name);
		//
		if(PlaceAt)
			{
			NewBrush->Location  = PlaceAt->Location;
			NewBrush->Rotation  = PlaceAt->Rotation;
			NewBrush->PrePivot  = PlaceAt->PrePivot;
			NewBrush->PostPivot = PlaceAt->PostPivot;
			};
		debugf("Duplicate %s at %s",NewBrush?NewBrush->Name:"",PlaceAt?PlaceAt->Name:"");
		}
	else strcpy(Data,"-1"); // Unknown
	//
	Brush->Unlock(&ModelInfo);
	//
	UNGUARD("MapTopicHandler::Get");
	};
void MapTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("MapTopicHandler::Set");
	};

/*---------------------------------------------------------------------------------------
   Polys link topic handler
---------------------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC("Polys",PolysTopicHandler);
void PolysTopicHandler::Get(ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	GUARD;
	IModel		ModelInfo;
	FBspSurf	*Poly;
	DWORD		OnFlags,OffFlags;
	//
	Level->Model->Lock(&ModelInfo,LOCK_Read);
	//
	INDEX n  = 0;
	OffFlags = (DWORD)~0;
	OnFlags  = (DWORD)~0;
	for (INDEX i=0; i<ModelInfo.NumBspSurfs; i++)
		{
		Poly = &ModelInfo.BspSurfs[i];
		//
		if (Poly->PolyFlags&PF_Selected)
			{
			OnFlags  &=  Poly->PolyFlags;
			OffFlags &= ~Poly->PolyFlags;
			n++;
			};
		};
	if (!stricmp(Item,"NumSelected"))				sprintf(Data,"%i",n);
	else if (!stricmp(Item,"SelectedSetFlags"))		sprintf(Data,"%u",OnFlags  & ~PF_NoEdit);
	else if (!stricmp(Item,"SelectedClearFlags"))	sprintf(Data,"%u",OffFlags & ~PF_NoEdit);
	Level->Model->Unlock(&ModelInfo);
	//
	UNGUARD("PolysTopicHandler::Get");
	};
void PolysTopicHandler::Set(ULevel *Level, const char *Topic, const char *Item, const char *Data)
	{
	GUARD;
	UNGUARD("PolysTopicHandler::Set");
	};
