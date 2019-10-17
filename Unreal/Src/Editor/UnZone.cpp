/*=============================================================================
	UnZone.cpp: Zone visibility computation code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney

	Description:
		* This is all the code needed for assigning visibility zones to nodes
		  and filling in all of the zone-related visibility information structures
		  in the Bsp.  This zone information is entirely optional during editing
		  and rendering, but it accelerates rendering significantly because it allows
		  the renderer to discard occluded polygons and regions far more
		  efficiently.

	Definitions:
		* A zone is a logical area of the Bsp which is grouped together for visibility
		  determinated, planned out by the level designer.  A zone consists of one or
		  more adjacent, connected leaf nodes and all of the Bsp node polys which fall
		  into the zone.  A level can have up to MAX_ZONES (64) zones.  During rendering, visible
		  zones are tracked and all polys which are known to be in non-visible zones are
		  skipped.
		* Zones are bounded by solid, occluding polygons, and invisibile zone portals.
		  Zone portals are added in by level designers and typically inserted in the
		  middle of doors and hallways to logically divide space into areas.
		* The routine bears no resemblance to Quake's PCVS (precomputed visibilty set)
		  method, which actually computes node-to-node visibility.  Zone visibility only
		  computes zones and their connecting portals, and visibility is computed on
		  the fly during rendering.
		* Any node can contain two leaves: A front leaf (if the node's front is outside
		  and iFront is empty), and a back leaf (if the back is outside and iBack is empty).
=============================================================================*/

#include "Unreal.h"

//
// Options:
//
//#define PARANOID /* Turn this on for careful debugging checks */
//#define HULL_BOUNDS /* Node bounding boxes correspond to interior hulls */

//
// Constants:
//
#define WORLD_MAX 65536.0		/* Maximum size of the world */
#define MAX_DEPTH 4096			/* Maximum depth of the Bsp from root to the lowest leaf */

enum EZoneFilterState			// States during FilterToLeaf routine
	{
	FILTERING_DOWN_FRONT,		// First, poly is filtered through the front to leaves
	FILTERING_DOWN_BACK,		// Then, poly chunks are filtered through the back to leaves
	};

class FZoneFilterCuttingPlane	// A remembered node cutting plane during MergeAdjacentZones
	{
	public:
	int		IsFront;
	FVector *Normal;
	FVector	*Base;
	};

class FZoneFilter				// Class passed around during zone building operations
	{
	public:
	IModel					ModelInfo;	// Model information for the Bsp
	ULevel					*Level;		// Level this Bsp resides in
	FZoneFilterCuttingPlane *Planes;	// List of cutting planes in MergeAdjacentZones
	INDEX					*NodeZones;	// Full zone numbers (0-65535), later reduced to byte iZone's
	INDEX					*BackZones;	// Full zone numbers (0-65535), later reduced to byte iZone's
	int						NumPlanes;	// Number of cutting planes in MergeAdjacentZones
	int						NumPortals;	// Number of zone portals found on polys with PF_ZONE_PORTAL tag
	int						PortalChunks; // Number of portal polys filtered down to leaves
	int						SingleZones; // Nodes tagged as NF_SINGLE_ZONE
	//
	// Functions:
	//
	void FilterToLeaf(FPoly *EdPoly,INDEX iOtherNode,INDEX iPrevNode,INDEX iNode,EZoneFilterState State, int Outside, int BackOutside, ENodePlace PrevPlace);
	void AssignUniqueLeafZones(INDEX iPrev,INDEX iNode, int &iTopZone,int Outside,ENodePlace PrevPlace);
	void MergeAdjacentZones(INDEX iNode, int Outside);
	void PerformMergeAdjacentZones(INDEX iNode, int Outside, FPoly OnceInfiniteEdPoly, int iParentCuttingPlane, int NumPlanes);
	void MergeTwoZones(INDEX iZone1, INDEX iZone2);
	void RemapZones(void);
	void FillZone(INDEX iNode,INDEX iZone);
	void AssignAllZones(INDEX iNode,int Outside);
	void FindSingleZone(INDEX iPrevNode,INDEX iNode,FPoly *EdPoly,BYTE &iThisZone,int &MultipleZones,int Outside,ENodePlace PrevPlace);
	void FilterMultiZonePoly(INDEX iSourceNode,INDEX iPrevNode,INDEX iNode,FPoly *EdPoly,int Outside,ENodePlace PrevPlace);
	void FilterPortalToLeaves(INDEX iPrevNode,INDEX iNode,FPoly *EdPoly);
	void MergeZonePortals(void);
	void BuildConnectivity(void);
	void BuildVisibility(int SampleDensity, int MinSampleSize);
	void BuildZoneDescriptors(void);
	QWORD BuildZoneMasks(INDEX iNode);
	};

/*-----------------------------------------------------------------------------
	Non-class functions
-----------------------------------------------------------------------------*/

//
// Find good arbitrary axis vectors to represent U and V axes of a plane
// given just the normal:
//
void FindBestAxisVectors(FVector &Normal,FVector &Axis1,FVector &Axis2)
	{
	GUARD;
	FLOAT NX=OurAbs(Normal.X);
	FLOAT NY=OurAbs(Normal.Y);
	FLOAT NZ=OurAbs(Normal.Z);
	//
	// Find best basis vectors:
	//
	if ((NZ>NX)&&(NZ>NY))	Axis1 = GMath.XAxisVector;
	else					Axis1 = GMath.ZAxisVector;
	//
	Axis1 -= Normal * (Axis1 | Normal); Axis1.Normalize();
	Axis2  = Axis1 ^ Normal;
	//
	#ifdef PARANOID // Check results
	if (
		(OurAbs(Axis1 | Normal)>0.0001) ||
		(OurAbs(Axis2 | Normal)>0.0001) ||
		(OurAbs(Axis1 | Axis2 )>0.0001)
		) appError ("FindBestAxisVectors: Failed");
	#endif
	UNGUARD("FindBestAxisVectors");
	};

//
// Build an FPoly representing an "infinite" plane (which exceeds the maximum
// dimensions of the world in all directions) for a particular Bsp node.
//
void BuildInfiniteFPoly(IModel &ModelInfo, INDEX iNode, FPoly &EdPoly)
	{
	GUARD;
	FBspNode *Node   = &ModelInfo.BspNodes [iNode        ];
	FBspSurf *Poly   = &ModelInfo.BspSurfs [Node->iSurf  ];
	FVector  *Base   = &ModelInfo.FPoints  [Poly->pBase  ];
	FVector  *Normal = &ModelInfo.FVectors [Poly->vNormal];
	FVector	 Axis1,Axis2;
	//
	// Find two non-problematic axis vectors:
	//
	FindBestAxisVectors(*Normal,Axis1,Axis2);
	//
	// Set up the FPoly:
	//
	EdPoly.Init();
	EdPoly.NumVertices = 4;
	EdPoly.Normal      = *Normal;
	EdPoly.Base        = *Base;
	EdPoly.Vertex[0]   = *Base + Axis1*WORLD_MAX + Axis2*WORLD_MAX;
	EdPoly.Vertex[1]   = *Base - Axis1*WORLD_MAX + Axis2*WORLD_MAX;
	EdPoly.Vertex[2]   = *Base - Axis1*WORLD_MAX - Axis2*WORLD_MAX;
	EdPoly.Vertex[3]   = *Base + Axis1*WORLD_MAX - Axis2*WORLD_MAX;
	//
	#ifdef PARANOID // Validate the poly
	if (EdPoly.SplitWithPlane (*Base,*Normal,NULL,NULL,0)!=SP_Coplanar) appError ("BuildInfinitePoly screwed up");
	#endif
	//
	UNGUARD("BuildInfiniteFPoly");
	};

/*-----------------------------------------------------------------------------
	FZoneFilter: Main zone merging functions and callbacks
-----------------------------------------------------------------------------*/

//
// Filter a poly through the Bsp, splitting it as we go, until we get to a leaf,
// then process the resulting poly chunks based on the filter State.
//
// iPrevNode = index of previous node in chain (guaranteed not INDEX_NONE)
// If FILTERING_DOWN_FRONT: iOtherNode = index of back node to progress down next
// If FILTERING_DOWN_BACK:  iOtherNode = index of original front node's iZone we encountered
//
void FZoneFilter::FilterToLeaf(FPoly *EdPoly,
	INDEX iOtherNode,INDEX iPrevNode,INDEX iNode,EZoneFilterState State,
	int Outside, int BackOutside, ENodePlace PrevPlace)
	{
	GUARD;
	//
	FilterLoop:
	//
	if (EdPoly->NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
		{
		FPoly TempEdPoly;
		EdPoly->SplitInHalf(&TempEdPoly);
		FilterToLeaf(&TempEdPoly,iOtherNode,iPrevNode,iNode,State,Outside,BackOutside,PrevPlace);
		};
	if (iNode!=INDEX_NONE)
		{
		FBspNode *Node   = &ModelInfo.BspNodes [iNode        ];
		FBspSurf *Poly   = &ModelInfo.BspSurfs [Node->iSurf  ];
		FVector  *Base   = &ModelInfo.FPoints  [Poly->pBase  ];
		FVector  *Normal = &ModelInfo.FVectors [Poly->vNormal];
		FPoly	 FrontPoly,BackPoly;
		//
		switch (EdPoly->SplitWithPlane (*Base,*Normal,&FrontPoly,&BackPoly,0))
			{
			case SP_Coplanar:
				debugf(LOG_Info,"FZoneFilter::FilterToLeaf: Got coplanar");
			case SP_Front:
				Outside   = Outside || Node->IsCsg();
				iPrevNode = iNode;
				iNode     = Node->iFront;
				PrevPlace = NODE_Front;
				goto FilterLoop;
			case SP_Back:
				Outside   = Outside && !Node->IsCsg();
				iPrevNode = iNode;
				iNode     = Node->iBack;
				PrevPlace = NODE_Back;
				goto FilterLoop;
			case SP_Split:
				FilterToLeaf
					(
					&FrontPoly,iOtherNode,iNode,Node->iFront,State,
					Outside ||  Node->IsCsg(),BackOutside,NODE_Front
					);
				FilterToLeaf
					(
					&BackPoly, iOtherNode,iNode,Node->iBack, State,
					Outside && !Node->IsCsg(),BackOutside,NODE_Back
					);
				break;
			default:
				appError("FZoneFilter::FilterToLeaf: Unknown split code");
			};
		}
	else if (State==FILTERING_DOWN_FRONT)
		{
		if (Outside && !(ModelInfo.BspNodes[iPrevNode].NodeFlags & NF_Portal))
			{
			//
			// Now iOtherNode = Lowest common parent node
			//     iPrevNode  = Lowest valid front node
			//     iNode      = INDEX_NONE
			//
			if (iOtherNode==INDEX_NONE)appError("Error3");
			if (iPrevNode ==INDEX_NONE)appError("Error4");
			//
			INDEX    iFrontZone;
			if		(PrevPlace==NODE_Front) iFrontZone = NodeZones[iPrevNode];
			else if (PrevPlace==NODE_Back ) iFrontZone = BackZones[iPrevNode];
			//
			// On the next iteration:
			//     iOtherNode = The front leaf's zone (=PrevNode->Zone)
			//     iPrevNode  = Lowest common parent (=iOtherNode)
			//     iNode      = (iOtherNode -> iBack)
			//
			FBspNode *Parent = &ModelInfo.BspNodes[iOtherNode];
			FilterToLeaf
				(
				EdPoly,iFrontZone,iOtherNode,Parent->iBack,FILTERING_DOWN_BACK,
				BackOutside,BackOutside,NODE_Back
				);
			};
		}
	else
		{
		if (Outside && !(ModelInfo.BspNodes[iPrevNode].NodeFlags & NF_Portal))
			{
			//
			// Here we've reached a destination back leaf and therefore a non-empty polygon has 
			// survived the trip down both the front and the back of the source node, and we
			// know that both original nodes are adjacent and mutually visible.
			//
			// Now iOtherNode  = The front leaf's iZone
			//     iPrevNode   = Lowest valid back node
			//     iNode       = INDEX_NONE
			//
			INDEX iBackZone;
			//
			if		(PrevPlace==NODE_Front) iBackZone = NodeZones[iPrevNode];
			else if (PrevPlace==NODE_Back ) iBackZone = BackZones[iPrevNode];
			//
			if (iBackZone==INDEX_NONE) appError ("Error2");
			//
			MergeTwoZones(iOtherNode,iBackZone);
			};
		};
	UNGUARD("FZoneFilter::FilterToLeaf");
	};

//
// Filter a portal polygon down to leaves, tagging all leaves that its
// chunks fall in as NF_Portal, which blocks adjacency calculations at those
// leaves.  This disregards inside/outside, since the portal FPoly can be
// assumed to be clipped to only the inside of the Bsp.
//
void FZoneFilter::FilterPortalToLeaves(INDEX iPrevNode,INDEX iNode,FPoly *EdPoly)
	{
	GUARD;
	//
	FilterLoop:
	//
	if (EdPoly->NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
		{
		FPoly TempEdPoly;
		EdPoly->SplitInHalf(&TempEdPoly);
		FilterPortalToLeaves(iPrevNode,iNode,&TempEdPoly);
		};
	if (iNode!=INDEX_NONE)
		{
		FBspNode *Node   = &ModelInfo.BspNodes [iNode        ];
		FBspSurf *Poly   = &ModelInfo.BspSurfs [Node->iSurf  ];
		FVector  *Base   = &ModelInfo.FPoints  [Poly->pBase  ];
		FVector  *Normal = &ModelInfo.FVectors [Poly->vNormal];
		FPoly	 FrontPoly,BackPoly;
		//
		switch (EdPoly->SplitWithPlane (*Base,*Normal,&FrontPoly,&BackPoly,0))
			{
			case SP_Coplanar:
				debugf(LOG_Info,"FZoneFilter::FilterPortalToLeaves: Got coplanar");
			case SP_Front:
				iPrevNode = iNode;
				iNode     = Node->iFront;
				goto FilterLoop;
			case SP_Back:
				iPrevNode = iNode;
				iNode     = Node->iBack;
				goto FilterLoop;
			case SP_Split:
				FilterPortalToLeaves(iNode,Node->iFront,&FrontPoly);
				FilterPortalToLeaves(iNode,Node->iBack, &BackPoly);
				break;
			};
		}
	else // At a leaf
		{
		PortalChunks++;
		ModelInfo.BspNodes[iPrevNode].NodeFlags |= NF_Portal;
		};
	UNGUARD("FZoneFilter::FilterPortalToLeaves");
	};

//
// Filter a once-infinte EdPoly through the Bsp, cutting it by each parent cutting plane,
// until we get to the desired node.  Then, begin filtering the poly to all leaves.  This
// is a worker function called by MergeAdjacentZones.
//
void FZoneFilter::PerformMergeAdjacentZones(INDEX iNode, int Outside, FPoly OnceInfiniteEdPoly,
	int iParentCuttingPlane, int NumPlanes)
	{
	GUARD;
	//
	// Clip the infinite poly by all parent cutting planes, so that it's
	// restricted to its proper convex hull.
	//
	while (iParentCuttingPlane < NumPlanes)
		{
		if (OnceInfiniteEdPoly.NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
			{
			FPoly TempEdPoly;
			OnceInfiniteEdPoly.SplitInHalf(&TempEdPoly);
			PerformMergeAdjacentZones(iNode,Outside,TempEdPoly,iParentCuttingPlane,NumPlanes);
			};
		FZoneFilterCuttingPlane *Cut = &Planes[iParentCuttingPlane];
		FPoly FrontPoly,BackPoly;
		switch (OnceInfiniteEdPoly.SplitWithPlane (*Cut->Base,*Cut->Normal,&FrontPoly,&BackPoly,0))
			{
			case SP_Front:
				if (!Cut->IsFront) return; // This piece of the poly was clipped to oblivion
				break;
			case SP_Back:
				if (Cut->IsFront) return; // This piece of the poly was clipped to oblivion
				break;
			case SP_Split: // Keep the one piece we want
				if (Cut->IsFront)	OnceInfiniteEdPoly = FrontPoly;
				else				OnceInfiniteEdPoly = BackPoly;
				break;
			case SP_Coplanar: // Inconsistency due to numerical precision problem
				debugf(LOG_Info,"FZoneFilter::MergeAdjacentZones: Invalid coplanar");
				return;
			};
		iParentCuttingPlane++;
		};
	//
	// Filter it through the Bsp down to leaves and tag any leaves that are proven
	// to be adjacent.
	//
	FBspNode *Node = &ModelInfo.BspNodes[iNode];
	//
	FilterToLeaf
		(
		&OnceInfiniteEdPoly, iNode, iNode, Node->iFront, FILTERING_DOWN_FRONT,
		Outside || Node->IsCsg(), Outside && !Node->IsCsg(), NODE_Front
		);
	UNGUARD("FZoneFilter::PerformMergeAdjacentZones");
	};

//
// Recurse through the Bsp, merging zones that are adjacent.
//
void FZoneFilter::MergeAdjacentZones(INDEX iNode, int Outside)
	{
	GUARD;
	FBspNode	*Node   = &ModelInfo.BspNodes[iNode];
	FBspSurf	*Poly	= &ModelInfo.BspSurfs[Node->iSurf];
	//
	// Recursively merge adjacents in front and back.
	// Tracks a list of cutting planes from the root down to
	// this node so that the hull bounding polys can be clipped
	// to their appropriate parent volumes.
	//
	Planes[NumPlanes].Normal = &ModelInfo.FVectors[Poly->vNormal];
	Planes[NumPlanes].Base   = &ModelInfo.FPoints [Poly->pBase];
	if (Node->iFront != INDEX_NONE)
		{
		Planes[NumPlanes++].IsFront = 1;
		MergeAdjacentZones(Node->iFront,Outside || Node->IsCsg());
		NumPlanes--;
		};
	if (Node->iBack  != INDEX_NONE)
		{
		Planes[NumPlanes++].IsFront = 0;
		MergeAdjacentZones(Node->iBack, Outside && !Node->IsCsg());
		NumPlanes--;
		};
	//
	// For all coplanar PF_ZONE_PORTALS in this node, filter them down to
	// leaves and tag those leaves as NF_Portal.  This causes zone portal
	// polygons to block the adjacency computations.
	//
	for (int i=0; i<ModelInfo.NumBspNodes; i++)
		{
		ModelInfo.BspNodes[i].NodeFlags &= ~NF_Portal;
		};
	INDEX iTempNode = iNode;
	while (iTempNode != INDEX_NONE)
		{
		FBspNode *TempNode = &ModelInfo.BspNodes[iTempNode];
		FBspSurf *TempPoly = &ModelInfo.BspSurfs[TempNode->iSurf];
		if (TempPoly->PolyFlags & PF_Portal)
			{
			NumPortals++;
			FPoly PortalPoly;
			if (GUnrealEditor.bspNodeToFPoly (&ModelInfo,iTempNode,&PortalPoly))
				{
				FilterPortalToLeaves(iNode,Node->iFront,&PortalPoly);
				FilterPortalToLeaves(iNode,Node->iBack, &PortalPoly);
				};
			};
		iTempNode = TempNode->iPlane;
		};
	//
	// Build an "infinite" FPoly representing this node's plane:
	//
	FPoly InfiniteEdPoly;
	BuildInfiniteFPoly(ModelInfo,iNode,InfiniteEdPoly);
	PerformMergeAdjacentZones(iNode,Outside,InfiniteEdPoly,0,NumPlanes);
	//
	UNGUARD("FZoneFilter::MergeAdjacentZones");
	};

/*-----------------------------------------------------------------------------
	FZoneFilter: Misc zone functions
-----------------------------------------------------------------------------*/

//
// Assign a unique zone number to every leaf node of the Bsp.
//
void FZoneFilter::AssignUniqueLeafZones(INDEX iPrev,INDEX iNode,int &iTopZone,int Outside,ENodePlace PrevPlace)
	{
	GUARD;
	if (iNode!=INDEX_NONE)
		{
		FBspNode *Node = &ModelInfo.BspNodes[iNode];
		AssignUniqueLeafZones(iNode,Node->iFront,iTopZone,Outside ||  Node->IsCsg(),NODE_Front);
		AssignUniqueLeafZones(iNode,Node->iBack ,iTopZone,Outside && !Node->IsCsg(),NODE_Back);
		}
	else
		{
		if (Outside)
			{
			if		(PrevPlace==NODE_Front) NodeZones[iPrev] = iTopZone;
			else if (PrevPlace==NODE_Back ) BackZones[iPrev] = iTopZone;
			iTopZone++;
			};
		};
	UNGUARD("FZoneFilter::AssignUniqueLeafZones");
	};


//
// Merge two zones.  Goes through all nodes in the world and replaces
// references to the second zone with the first.
//
void FZoneFilter::MergeTwoZones(INDEX iZone1, INDEX iZone2)
	{
	GUARD;
	if (iZone1!=iZone2) for (int i=0; i<ModelInfo.NumBspNodes; i++)
		{
		if (NodeZones[i]==iZone2) NodeZones[i]=iZone1;
		if (BackZones[i]==iZone2) BackZones[i]=iZone1;
		};
	UNGUARD("FZoneFilter::MergeTwoZones");
	};

//
// Renumber all zones in the Bsp so that zone numbers go from 1 to the maximum
// with no gaps.
//
void FZoneFilter::RemapZones(void)
	{
	GUARD;
	BYTE	*ZoneRemap  = (BYTE *)GMem.GetZeroed(ModelInfo.NumZones * sizeof(BYTE));
	int		NewNumZones = 0;
	//
	FBspNode *Node = &ModelInfo.BspNodes[0];
	for (int i=0; i<ModelInfo.NumBspNodes; i++)
		{
		if (NodeZones[i]!=INDEX_NONE)
			{
			if (ZoneRemap[NodeZones[i]]==0) ZoneRemap[NodeZones[i]] = 1+((NewNumZones++)%63);
			Node->iZone = ZoneRemap[NodeZones[i]];
			}
		else Node->iZone = 0;
		//
		if (BackZones[i]!=INDEX_NONE)
			{
			if (ZoneRemap[BackZones[i]]==0) ZoneRemap[BackZones[i]] = 1+((NewNumZones++)%63);
			Node->iBackZone = ZoneRemap[BackZones[i]];
			}
		else Node->iBackZone = 0;
		//
		Node++;
		};
	debugf(LOG_Info,"visBuild reduced %i zones down to %i (%i)",ModelInfo.NumZones,NewNumZones+1,OurMin(64,NewNumZones+1));
	ModelInfo.NumZones = OurMin(64,NewNumZones+1);
	GMem.Release(ZoneRemap);
	UNGUARD("FZoneFilter::RemapZones");
	};

/*-----------------------------------------------------------------------------
	FZoneFilter: Assigning zone numbers
-----------------------------------------------------------------------------*/

//
// Filter a poly through the Bsp, figure out which zone it lies in, and set iThisZone to
// its number.  This routine hopes that all chunks of the poly lie in the same zone, but
// it sets MultipleZones to 1 if this isn't the case.
//
void FZoneFilter::FindSingleZone(INDEX iPrevNode,INDEX iNode,FPoly *EdPoly,BYTE &iThisZone,int &MultipleZones,int Outside,ENodePlace PrevPlace)
	{
	GUARD;
	//
	FilterLoop:
	//
	if (EdPoly->NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
		{
		FPoly TempEdPoly;
		EdPoly->SplitInHalf(&TempEdPoly);
		FindSingleZone(iPrevNode,iNode,&TempEdPoly,iThisZone,MultipleZones,Outside,PrevPlace);
		};
	if (iNode!=INDEX_NONE) // Filter through
		{
		FBspNode *Node   = &ModelInfo.BspNodes [iNode        ];
		FBspSurf *Poly   = &ModelInfo.BspSurfs [Node->iSurf  ];
		FVector  *Base   = &ModelInfo.FPoints  [Poly->pBase  ];
		FVector  *Normal = &ModelInfo.FVectors [Poly->vNormal];
		FPoly	 FrontPoly,BackPoly;
		//
		switch (EdPoly->SplitWithPlane (*Base,*Normal,&FrontPoly,&BackPoly,0))
			{
			case SP_Coplanar:
				debugf(LOG_Info,"Coplanar");
				/*
				iPrevNode	= iNode;
				iNode		= INDEX_NONE;
				Outside     = Outside || Node->IsCsg();
				goto		FilterLoop;
				*/
			case SP_Front:
				iPrevNode	= iNode;
				iNode		= Node->iFront;
				Outside     = Outside || Node->IsCsg();
				PrevPlace   = NODE_Front;
				goto FilterLoop;
				break;
			case SP_Back:
				iPrevNode	= iNode;
				iNode		= Node->iBack;
				Outside     = Outside && !Node->IsCsg();
				PrevPlace   = NODE_Back;
				goto FilterLoop;
				break;
			case SP_Split:
				FindSingleZone(iNode,Node->iFront,&FrontPoly,iThisZone,MultipleZones,Outside ||  Node->IsCsg(),NODE_Front);
				FindSingleZone(iNode,Node->iBack, &BackPoly, iThisZone,MultipleZones,Outside && !Node->IsCsg(),NODE_Back);
				break;
			};
		}
	else // iPrevNode is a leaf
		{
		if (Outside)
			{
			FBspNode *PrevNode = &ModelInfo.BspNodes[iPrevNode];
			INDEX	 iPrevZone;
			//
			if		(PrevPlace==NODE_Front) iPrevZone = PrevNode->iZone;
			else if	(PrevPlace==NODE_Back ) iPrevZone = PrevNode->iBackZone;
			else							appError("FZoneFilter::FindSingleZone: Place inconsistency");
			//
			if		(iPrevZone==0)  appError("FZoneFilter::FindSingleZone: Unzoned leaf");
			//
			if		(iThisZone==0)			iThisZone     = iPrevZone;
			else if (iThisZone!=iPrevZone)	MultipleZones = 1;
			};
		};
	UNGUARD("FZoneFilter::FindSingleZone");
	};

//
// Filter a polygon down the Bsp and add it to leaves, assigning the proper zone numbers
// to it.  This is called when a Bsp node poly's chunks are found to reside in more than
// one zone.  To render it properly, it must be split up so that each piece lies in one
// and only one zone.
//
void FZoneFilter::FilterMultiZonePoly(INDEX iSourceNode,INDEX iPrevNode,INDEX iNode,FPoly *EdPoly,int Outside,ENodePlace PrevPlace)
	{
	GUARD;
	//
	FilterLoop:
	//
	if (EdPoly->NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
		{
		FPoly TempEdPoly;
		EdPoly->SplitInHalf(&TempEdPoly);
		FilterMultiZonePoly(iSourceNode,iPrevNode,iNode,&TempEdPoly,Outside,PrevPlace);		
		};
	if (iNode!=INDEX_NONE) // Filter through
		{
		FBspNode *Node   = &ModelInfo.BspNodes [iNode        ];
		FBspSurf *Poly   = &ModelInfo.BspSurfs [Node->iSurf  ];
		FVector  *Base   = &ModelInfo.FPoints  [Poly->pBase  ];
		FVector  *Normal = &ModelInfo.FVectors [Poly->vNormal];
		FPoly	 FrontPoly,BackPoly;
		//
		switch (EdPoly->SplitWithPlane (*Base,*Normal,&FrontPoly,&BackPoly,0))
			{
			case SP_Coplanar:
				// Happens occastionally due to precision errors.
				// Arbitrarily treat coplanars as front so they're most likely to get some zone
				// assigned to them:
			case SP_Front:
				iPrevNode	= iNode;
				iNode		= Node->iFront;
				Outside     = Outside || Node->IsCsg();
				PrevPlace	= NODE_Front;
				goto FilterLoop;
			case SP_Back:
				iPrevNode	= iNode;
				iNode		= Node->iBack;
				Outside     = Outside && !Node->IsCsg();
				PrevPlace	= NODE_Back;
				goto FilterLoop;
			case SP_Split:
				FilterMultiZonePoly(iSourceNode,iNode,Node->iFront,&FrontPoly,Outside ||  Node->IsCsg(),NODE_Front);
				FilterMultiZonePoly(iSourceNode,iNode,Node->iBack, &BackPoly, Outside && !Node->IsCsg(),NODE_Back);
				break;
			};
		}
	else // iPrevNode is a leaf
		{
		if (Outside)
			{
			FBspNode	*SourceNode = &ModelInfo.BspNodes[iSourceNode];
			FBspNode	*PrevNode	= &ModelInfo.BspNodes[iPrevNode  ];
			INDEX		iPrevZone;
			//
			if		(PrevPlace==NODE_Front)	iPrevZone = PrevNode->iZone;
			else if (PrevPlace==NODE_Back ) iPrevZone = PrevNode->iBackZone;
			if (iPrevZone==0) appError("FZoneFilter::FilterMultiZonePoly: Zone inconsistency");
			//
			INDEX iNewNode = GUnrealEditor.bspAddNode
				(
				&ModelInfo, iSourceNode, NODE_Plane,
				SourceNode->NodeFlags | NF_IsNew,
				EdPoly
				);
			ModelInfo.BspNodes[iNewNode].iZone			 = iPrevZone;
			ModelInfo.BspNodes[iNewNode].iBackZone		 = iPrevZone;
			//
			INDEX iSurf = SourceNode->iSurf;
			FBspNode *Node = &ModelInfo.BspNodes[0];
			for( INDEX iNode=0; iNode<ModelInfo.NumBspNodes; iNode++)
				{
				if( Node->iSurf == iSurf ) Node->NodeFlags |= NF_NoMerge;
				Node++;
				};
			};
		};
	UNGUARD("FZoneFilter::FilterMultiZonePoly");
	};

//
// Go through the Bsp and assign zone numbers to all nodes.  Prior to this
// function call, only leaves have zone numbers.  The zone numbers for the entire
// Bsp can be determined from leaf zone numbers.
//
void FZoneFilter::AssignAllZones(INDEX iNode,int Outside)
	{
	GUARD;
	INDEX		iOriginalNode	= iNode;
	FBspNode	*OriginalNode	= &ModelInfo.BspNodes[iOriginalNode];
	FBspSurf	*OriginalPoly	= &ModelInfo.BspSurfs[OriginalNode->iSurf];
	FVector		*OriginalNormal	= &ModelInfo.FVectors[OriginalPoly->vNormal];
	FPoly		EdPoly;
	//
	// Recursively assign zone numbers to children:
	//
	if (OriginalNode->iFront!=INDEX_NONE) AssignAllZones(OriginalNode->iFront, Outside ||  OriginalNode->IsCsg());
	if (OriginalNode->iBack !=INDEX_NONE) AssignAllZones(OriginalNode->iBack,  Outside && !OriginalNode->IsCsg());
	//
	// Make sure this node's polygon resides in a single zone.  In other words,
	// find all of the zones belonging to outside Bsp leaves and make sure their
	// zone number is the same, and assign that zone number to this node.  Note that
	// if semisolid polygons exist in the world, polygon fragments may actually fall into
	// inside nodes, and these fragments (and their zones) can be disregarded.
	//
	while (iNode != INDEX_NONE)
		{
		FBspNode	*Node		= &ModelInfo.BspNodes[iNode];
		FBspSurf	*Poly		= &ModelInfo.BspSurfs[Node->iSurf];
		//
		if (Node->NodeFlags & NF_IsNew) break;
		//
		if (GUnrealEditor.bspNodeToFPoly (&ModelInfo,iNode,&EdPoly))
			{
			BYTE	ThisZone;
			int		MultipleZones = 0;
			//
			int Forwards = ((*OriginalNormal | ModelInfo.FVectors[Poly->vNormal])>=0.0);
			if (Poly->PolyFlags & PF_TwoSided) // Two-sided's aren't allowed to be split
				{
				ThisZone = 0;
				FindSingleZone
					(
					iOriginalNode,OriginalNode->iFront,&EdPoly,ThisZone,MultipleZones,
					Outside || OriginalNode->IsCsg(),NODE_Front
					);
				// If 0, this is probably an interior semisolid poly
				if (Forwards)	Node->iZone		= ThisZone;
				else			Node->iBackZone	= ThisZone;
				//
				ThisZone = 0;
				FindSingleZone
					(
					iOriginalNode,OriginalNode->iBack,&EdPoly,ThisZone,MultipleZones,
					Outside && !OriginalNode->IsCsg(),NODE_Back
					);
				// If 0, this is probably an interior semisolid poly
				if (Forwards)	Node->iBackZone = ThisZone; 
				else			Node->iZone     = ThisZone;
				}
			else
				{
				if (Forwards)
					{
					ThisZone = 0;
					MultipleZones = 0;
					FindSingleZone
						(
						iOriginalNode,OriginalNode->iFront,&EdPoly,ThisZone,MultipleZones,
						Outside || OriginalNode->IsCsg(),NODE_Front
						);
					if (MultipleZones)
						{
						FilterMultiZonePoly
							(
							iNode,iOriginalNode,OriginalNode->iFront,
							&EdPoly,Outside || OriginalNode->IsCsg(),NODE_Front
							);
						Node->NodeFlags |= NF_TagForEmpty;
						}
					else Node->iZone = ThisZone; // If 0, this is probably an interior semisolid poly
					}
				else
					{
					ThisZone		= 0;
					MultipleZones	= 0;
					FindSingleZone
						(
						iOriginalNode,OriginalNode->iBack,&EdPoly,ThisZone,MultipleZones,
						Outside && !OriginalNode->IsCsg(),NODE_Back
						);
					if (MultipleZones)
						{
						FilterMultiZonePoly
							(
							iNode,iOriginalNode,OriginalNode->iBack,
							&EdPoly,Outside && !OriginalNode->IsCsg(),NODE_Back
							);
						Node->NodeFlags |= NF_TagForEmpty;
						}
					else Node->iZone = ThisZone; // If 0, this is probably an interior semisolid poly
					};
				};
			};
		iNode = Node->iPlane;
		};
	UNGUARD("FZoneFilter::AssignAllZones");
	};

/*-----------------------------------------------------------------------------
	Portal merging
-----------------------------------------------------------------------------*/

//
// Go through and make sure all zone portal nodes originating from the
// same polygon have the same zone numbers attached to them.
//
void FZoneFilter::MergeZonePortals(void)
	{
	GUARD;
	INDEX *PolyFrontZones = (INDEX *)GMem.GetZeroed(ModelInfo.NumBspSurfs*sizeof(INDEX));
	INDEX *PolyBackZones  = (INDEX *)GMem.GetZeroed(ModelInfo.NumBspSurfs*sizeof(INDEX));
	//
	for (INDEX iNode=0; iNode<ModelInfo.NumBspNodes; iNode++)
		{
		FBspNode *Node = &ModelInfo.BspNodes[iNode];
		INDEX	 iSurf = Node->iSurf;
		FBspSurf *Poly = &ModelInfo.BspSurfs[iSurf];
		//
		if (Poly->PolyFlags & PF_Portal)
			{
			//
			// Merge front zones:
			//
			if (PolyFrontZones[iSurf]!=INDEX_NONE)
				{
				if (NodeZones[iNode]==INDEX_NONE) NodeZones[iNode]=PolyFrontZones[iSurf];
				MergeTwoZones(NodeZones[iNode],PolyFrontZones[iSurf]);
				}
			else PolyFrontZones[iSurf] = NodeZones[iNode];
			//
			// Merge back zones:
			//
			if (PolyBackZones[iSurf]!=INDEX_NONE)
				{
				if (BackZones[iNode]==INDEX_NONE) BackZones[iNode]=PolyBackZones[iSurf];
				MergeTwoZones(BackZones[iNode],PolyBackZones[iSurf]);
				}
			else PolyBackZones[iSurf] = BackZones[iNode];
			};
		};
	GMem.Release(PolyFrontZones);
	UNGUARD("FZoneFilter::MergeZonePortals");
	};

/*-----------------------------------------------------------------------------
	Bsp zone structure building
-----------------------------------------------------------------------------*/

//
// Build a 64-bit zone mask for each node, with a bit set for every
// zone that's referenced by the node and its children.  This is used
// during rendering to reject entire sections of the tree when it's known
// that none of the zones in that section are active.
//
QWORD FZoneFilter::BuildZoneMasks(INDEX iNode)
	{
	GUARD;
	FBspNode	*Node		= &ModelInfo.BspNodes[iNode];
	QWORD		ZoneMask	= 0;
	//
	if (Node->iZone    !=0) ZoneMask |= ((QWORD)1) << Node->iZone;
	if (Node->iBackZone!=0) ZoneMask |= ((QWORD)1) << Node->iBackZone;
	//
	if (Node->iFront != INDEX_NONE)	ZoneMask |= BuildZoneMasks(Node->iFront);
	if (Node->iBack  != INDEX_NONE)	ZoneMask |= BuildZoneMasks(Node->iBack );
	if (Node->iPlane != INDEX_NONE)	ZoneMask |= BuildZoneMasks(Node->iPlane);
	//
	Node->ZoneMask = ZoneMask;
	//
	// See if we can tag Node as NF_SingleZone:
	//
	if (Node->iZone != 0)
		{
		for (int i=0; i<64; i++) if (ZoneMask==(((QWORD)1)<<i))
			{
			Node->NodeFlags |= NF_SingleZone;
			SingleZones++;
			break;
			};
		};
	return ZoneMask;
	UNGUARD("FZoneFilter::BuildZoneMasks");
	};

//
// Build 64x64 zone connectivity matrix.  Entry(i,j) is set if node i is connected
// to node j.  Entry(i,i) is always set by definition.  This structure is built by
// analyzing all portals in the world and tagging the two zones they connect.
//
void FZoneFilter::BuildConnectivity(void)
	{
	GUARD;
	for (int i=0; i<64; i++) // Init to identity
		{
		ModelInfo.BspNodesResource->Zones[i].Connectivity = ((QWORD)1)<<i;
		};
	for (i=0; i<ModelInfo.NumBspNodes; i++) // Process zones connected by portals
		{
		FBspNode *Node = &ModelInfo.BspNodes[i];
		FBspSurf *Poly = &ModelInfo.BspSurfs[Node->iSurf];
		//
		if (Poly->PolyFlags & PF_Portal)
			{
			ModelInfo.BspNodesResource->Zones[Node->iZone]    .Connectivity |= ((QWORD)1) << Node->iBackZone;
			ModelInfo.BspNodesResource->Zones[Node->iBackZone].Connectivity |= ((QWORD)1) << Node->iZone;
			};
		};
	UNGUARD("FZoneFilter::BuildConnectivity");
	};

/*-----------------------------------------------------------------------------
	Zone-to-zone visibility
-----------------------------------------------------------------------------*/

//
// Go through all portals in the world and sample their visibility.
// Call with:
//
// SampleDensity = maximum world-unit distance between lattice-based
//                 samples (i.e. 16 = sample size of 16x16 world units).
//
// MinSampleSize = minimum number of samples to take in each direction (i.e. 8 = don't
//                 sample a lattice less than 8x8).
//
// Visibility is generated by rendering two sets of views at each portal, one set in the
// in-facing direction, and one in the out-facing direction.  At these portals, the
// world is span-buffer rendered and the zones inside of all other visible portals in the
// world are tagged as visible.  This also explicitly tags Entry(i,i) as visible, and
// tags Entry(i,j) as visible if Entry(i,j) is connected.
//
void FZoneFilter::BuildVisibility(int SampleDensity, int MinSampleSize)
	{
	GUARD;
	for (int i=0; i<64; i++)
		{
		ModelInfo.BspNodesResource->Zones[i].Visibility = ModelInfo.BspNodesResource->Zones[i].Connectivity;
		};
	UNGUARD("FZoneFilter::BuildVisibility");
	};

/*-----------------------------------------------------------------------------
	FZoneFilter: Zone descriptors
-----------------------------------------------------------------------------*/

//
// Attach ZoneDescriptor actors to the zones that they belong in.
// ZoneDescriptor actors are a class of actor which level designers may
// place in UnrealEd in order to specify the properties of the zone they
// reside in, such as water effects, zone name, etc.
//
void FZoneFilter::BuildZoneDescriptors(void)
	{
	GUARD;
	//
	int Descriptors=0, Duplicates=0, Zoneless=0;
	//
	for (int i=0; i<64; i++)
		{
		ModelInfo.BspNodesResource->Zones[i].iZoneActor = INDEX_NONE;
		};
	for (i=0; i<Level->ActorList->Max; i++)
		{
		AActor *Actor = &Level->ActorList->Element(i);
		if (Actor->Class == GClasses.ZoneDescriptor)
			{
			int iZone = ModelInfo.PointZone(&Actor->Location);
			if (iZone==0)
				{
				Zoneless++;
				}
			else if (ModelInfo.BspNodesResource->Zones[iZone].iZoneActor != INDEX_NONE)
				{
				Duplicates++;
				}
			else
				{
				Descriptors++;
				ModelInfo.BspNodesResource->Zones[iZone].iZoneActor = i;
				};
			};
		};
	debugf(LOG_Info,"visBuild: %i ZoneDescriptors, %i duplicates, %i zoneless",Descriptors,Duplicates,Zoneless);
	UNGUARD("FZoneFilter::BuildZoneDescriptors");
	};

/*-----------------------------------------------------------------------------
	FZoneFilter: Main routine
-----------------------------------------------------------------------------*/

//
// Build all visibility zones for the Bsp.  Assigns iZone's and zone reject bitmasks
// to each node, sets up zone-to-zone connectivity information for sound propagation, 
// and sets up all visibility portals for proper visibility zone occlusion checking.
//
void FEditor::visBuild(ULevel *Level)
	{
	GUARD;
	UModel *Model = Level->Model;
	FZoneFilter Filter;
	//
	Model->Lock(&Filter.ModelInfo,LOCK_NoTrans);
	Filter.Level = Level;
	Filter.ModelInfo.NumZones = 0;
	//
	int NumPortals=0;
	for (int i=0; i<Filter.ModelInfo.NumBspSurfs; i++)
		{
		if (Filter.ModelInfo.BspSurfs[i].PolyFlags & PF_Portal) NumPortals++;
		};
	if (Filter.ModelInfo.NumBspNodes && NumPortals)
		{
		debug(LOG_Info,"visBuild begin");
		//
		// Allocate working tables:
		//
		Filter.Planes	 = (FZoneFilterCuttingPlane *)GMem.Get(MAX_DEPTH * sizeof(FZoneFilterCuttingPlane));
		Filter.NodeZones = (INDEX                   *)GMem.Get(Filter.ModelInfo.NumBspNodes * sizeof (INDEX));
		Filter.BackZones = (INDEX                   *)GMem.Get(Filter.ModelInfo.NumBspNodes * sizeof (INDEX));
		//
		// Give each leaf in the world a unique zone number.
		//
		for (int i=0; i<Filter.ModelInfo.NumBspNodes; i++)
			{
			Filter.NodeZones         [i]			= INDEX_NONE;
			Filter.BackZones         [i]			= INDEX_NONE;
			Filter.ModelInfo.BspNodes[i].iZone		= 0;
			Filter.ModelInfo.BspNodes[i].NodeFlags &= ~(NF_Portal|NF_IsNew);
			};
		Filter.ModelInfo.NumZones = 1; // Don't assign 0 to any zone
		Filter.AssignUniqueLeafZones(0,0,Filter.ModelInfo.NumZones,1,NODE_Root);
		debugf(LOG_Info,"visBuild assigned %i starting zones",Filter.ModelInfo.NumZones);
		//
		// Go through the entire tree, merging zones which are spatially connected.
		//
		Filter.NumPortals   = 0;
		Filter.PortalChunks = 0;
		Filter.NumPlanes    = 0;
		Filter.SingleZones	= 0;
		Filter.MergeAdjacentZones(0,1);
		//
		// Renumber the zones so we're left with a smaller, more manageable set:
		//
		Filter.MergeZonePortals();
		Filter.RemapZones();
		//
		// Assign zone numbers to all nodes in the world (previous to this, only leaves
		// have zone numbers):
		//
		Filter.AssignAllZones(0,1);
		Filter.BuildConnectivity();
		Filter.BuildVisibility(0,0);
		Filter.BuildZoneDescriptors();
		//
		// Clean up Bsp.  Required since node polys may have been filtered, leaving zero-
		// vertex node polys with possibly non-empty coplanars.  hen rebuild bounding
		// boxes since Bsp node polys were filtered down.
		//
		int NodesMissed=0;
		for (i=0; i<Filter.ModelInfo.NumBspNodes; i++)
			{
			FBspNode *Node = &Filter.ModelInfo.BspNodes[i];
			if ((Node->iZone == 0) && !(Node->NodeFlags&NF_TagForEmpty)) NodesMissed++;
			Filter.ModelInfo.BspNodes[i].NodeFlags &= ~(NF_Portal);
			};
		debugf(LOG_Info,"visBuild end, portals=%i/%i, single=%i%%, missed=%i",
			Filter.NumPortals,Filter.PortalChunks,
			(100*Filter.SingleZones)/Filter.ModelInfo.NumBspNodes,
			NodesMissed);
		GMem.Release(Filter.Planes);
		//
		GUnrealEditor.bspCleanup		(&Filter.ModelInfo);	// Clean up newly-emptied nodes
		GUnrealEditor.bspRefresh		(&Filter.ModelInfo,0);	// Delete unreferenced stuff if near overflow
		GUnrealEditor.bspBuildBounds	(&Filter.ModelInfo);	// Build bounding box structure
		//
		// Build 64-entry rejection mask for each node.  Must call this after
		// bspCleanup:
		//
		Filter.BuildZoneMasks(0);
		}
	else debug(LOG_Info,"visBuild built no zones");
	Model->Unlock(&Filter.ModelInfo);
	//
	UNGUARD("visBuild");
	};

/*-----------------------------------------------------------------------------
	Bsp node bounding volumes
-----------------------------------------------------------------------------*/

#ifdef HULL_BOUNDS

enum {MAX_BOUND_POLYS=96};

//
// Update a bounding volume by expanding it to enclose a list of polys.
//
void UpdateBoundWithPolys(FBoundingVolume *Bound,FPoly **PolyList,int nPolys)
	{
	GUARD;
	for (int i=0; i<nPolys; i++)
		{
		FPoly   *Poly = PolyList[i];
		FVector *V    = &Poly->Vertex[0];
		for (int j=0; j<nPolys; j++)
			{
			UPDATE_MIN(&Bound->Min,V);
			UPDATE_MAX(&Bound->Max,V);
			V++;
			};
		};
	UNGUARD("UpdateBoundWithPolys");
	};

//
// Cut a partitioning poly by a list of polys, and add the resulting inside pieces to the
// front list and back list.
//
void SplitPartitioner(FPoly **PolyList, FPoly **FrontList, FPoly **BackList, int n, int nPolys,
	int &nFront, int &nBack, FPoly InfiniteEdPoly)
	{
	FPoly FrontPoly,BackPoly;
	while (n<nPolys)
		{
		if (InfiniteEdPoly.NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
			{
			FPoly Half;
			InfiniteEdPoly.SplitInHalf(&Half);
			SplitPartitioner(PolyList,FrontList,BackList,n,nPolys,nFront,nBack,Half);
			bug ("Split");
			};
		FPoly *Poly = PolyList[n];
		switch (InfiniteEdPoly.SplitWithPlane(Poly->Base,Poly->Normal,&FrontPoly,&BackPoly,0))
			{
			case SP_Coplanar:
				bug("FilterBound: Got inficoplanar"); // May occasionally happen
				break;
			case SP_Front:
				bug("FilterBound: Got infifront"); // Shouldn't happen if hull is correct
				return;
			case SP_Split:
				InfiniteEdPoly = BackPoly;
				break;
			};
		if ((nFront >= MAX_BOUND_POLYS)||(nBack >= MAX_BOUND_POLYS))
			{
			appError("Convex area is too complex");
			};
		n++;
		};
	FPoly *New = (FPoly *)GMem.Get(sizeof(FPoly));
	*New = InfiniteEdPoly;
	New->Flip();
	FrontList[nFront++] = New;
	//
	New = (FPoly *)GMem.Get(sizeof(FPoly));
	*New = InfiniteEdPoly;
	BackList[nBack++] = New;
	};

//
// Recursively filter a set of polys defining a convex hull down the Bsp,
// splitting it into two halves at each node and adding in the appropriate
// face polys at splits.
//
void FilterBound(IModel *ModelInfo,FBoundingVolume *ParentBound,
	INDEX iNode,FPoly **PolyList, int nPolys, int Outside)
	{
	FBspNode		*Node		= &ModelInfo->BspNodes [iNode];
	FBspSurf		*Surf		= &ModelInfo->BspSurfs [Node->iSurf];
	FVector			*Base		= &ModelInfo->FPoints  [Surf->pBase];
	FVector			*Normal		= &ModelInfo->FVectors [Surf->vNormal];
	void			*MemTop		= GMem.Get(0);
	FBoundingVolume	Bound;
	//
	Bound.Min.X = Bound.Min.Y = Bound.Min.Z = +65536.0;
	Bound.Max.X = Bound.Max.Y = Bound.Max.Z = -65536.0;
	//
	// Split bound into front half and back half:
	//
	FPoly *FrontList[MAX_BOUND_POLYS]; int nFront=0;
	FPoly *BackList [MAX_BOUND_POLYS]; int nBack=0;
	//
	FPoly *FrontPoly = (FPoly *)GMem.Get(sizeof(FPoly));
	FPoly *BackPoly  = (FPoly *)GMem.Get(sizeof(FPoly));
	//
	for (int i=0; i<nPolys; i++)
		{
		FPoly *Poly = PolyList[i];
		switch (Poly->SplitWithPlane(*Base,*Normal,FrontPoly,BackPoly,0))
			{
			case SP_Coplanar:
				bug("FilterBound: Got coplanar");
				FrontList[nFront++] = Poly;
				BackList[nBack++]   = Poly;
				break;
			case SP_Front:
				FrontList[nFront++] = Poly;
				break;
			case SP_Back:
				BackList[nBack++]   = Poly;
				break;
			case SP_Split:
				if (FrontPoly->NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
					{
					FPoly *Half = (FPoly *)GMem.Get(sizeof(FPoly));
					FrontPoly->SplitInHalf(Half);
					FrontList[nFront++] = Half;
					};
				FrontList[nFront++] = FrontPoly;
				//
				if (BackPoly->NumVertices >= FPoly::FPOLY_VERTEX_THRESHOLD)
					{
					FPoly *Half = (FPoly *)GMem.Get(sizeof(FPoly));
					BackPoly->SplitInHalf(Half);
					BackList[nBack++] = Half;
					};
				BackList [nBack++ ] = BackPoly;
				//
				FrontPoly           = (FPoly *)GMem.Get(sizeof(FPoly));
				BackPoly            = (FPoly *)GMem.Get(sizeof(FPoly));
				break;
			default:
				appError("FZoneFilter::FilterToLeaf: Unknown split code");
			};
		if ((nFront >= MAX_BOUND_POLYS)||(nBack >= MAX_BOUND_POLYS))
			{
			appError("Convex area is too complex");
			};
		};
	if (nFront && nBack)
		{
		//
		// Add partitioner plane to front and back:
		//
		FPoly InfiniteEdPoly;
		BuildInfiniteFPoly(*ModelInfo,iNode,InfiniteEdPoly);
		//
		SplitPartitioner(PolyList,FrontList,BackList,0,nPolys,nFront,nBack,InfiniteEdPoly);
		}
	else
		{
		if (!nFront) bug("FilterBound: Empty fronthull");
		if (!nBack ) bug("FilterBound: Empty backhull");
		};
	//
	// Recursively update all our childrens' bounding volumes:
	//
	if (nFront>0)
		{
		if (Node->iFront != INDEX_NONE) FilterBound
			(
			ModelInfo,&Bound,Node->iFront,
			FrontList, nFront,
			Outside ||  Node->IsCsg()
			);
		else if (Outside ||  Node->IsCsg()) UpdateBoundWithPolys(&Bound,FrontList,nFront);
		};
	if (nBack>0)
		{
		if (Node->iBack != INDEX_NONE) FilterBound
			(
			ModelInfo,&Bound,Node->iBack, 
			BackList, nBack,
			Outside && !Node->IsCsg()
			);
		else if (Outside && !Node->IsCsg()) UpdateBoundWithPolys(&Bound,BackList,nBack);
		};
	/* For debugging
	if ((Node->iFront==INDEX_NONE)&&(Node->iBack==INDEX_NONE))
		{
		bug ("%f %f %f - %f %f %f",Bound.Min.X,Bound.Min.Y,Bound.Min.Z,
			Bound.Max.X,Bound.Max.Y,Bound.Max.Z);
		};
	*/
	//
	// Apply this bound to this node:
	//
	Node->NodeFlags |= NF_Bounded;
	Node->iBound     = ModelInfo->NumBounds++;
	if (ModelInfo->NumBounds > ModelInfo->MaxBounds) appError ("Bound table full");
	ModelInfo->Bounds[Node->iBound] = Bound;
	//
	// Update parent bound to enclose this bound:
	//
	if (ParentBound)
		{
		UPDATE_MIN(&ParentBound->Min,&Bound.Min);
		UPDATE_MAX(&ParentBound->Max,&Bound.Max);
		};
	GMem.Release(MemTop);
	};

//
// Build bounding volumes for all Bsp nodes.  The bounding volume of the node
// completely encloses the "outside" space occupied by the nodes.  Note that 
// this is not the same as representing the bounding volume of all of the 
// polygons within the node.
//
void FEditor::bspBuildBounds(IModel *ModelInfo)
	{
	GUARD;
	if (ModelInfo->NumBspNodes==0) return;
	//
	FPoly *PolyList[6];
	for (int i=0; i<6; i++) PolyList[i]=&GGfx.RootHullBrush->Polys->Element(i);
	//
	ModelInfo->NumBounds=0;
	FilterBound(ModelInfo,NULL,0,PolyList,6,1);
	//
	bug ("bspBuildBounds: Generated %i bounds",ModelInfo->NumBounds);
	UNGUARD("FEditor::bspBuildBounds");
	};

#else

//
// Set a node's bounding volumes and all of its children's bounding boxes.
//
int BuildBound (IModel *ModelInfo, INDEX iNode, FBoundingVolume *ParentBound)
	{
	FBspNode		*Node 		= &ModelInfo->BspNodes [iNode];
	FVertPool		*VertPool	= &ModelInfo->VertPool [Node->iVertPool];
	int 			OurChildren = Node->NumVertices>0;
	FBoundingVolume	Bound;
	SWORD			C;
	BYTE			i;
	//
	// Set bounding volume to negative outer limits to guarantee that it'll be replaced:
	//
	Bound.Min.X = (FLOAT)+MAXSWORD;
	Bound.Min.Y = (FLOAT)+MAXSWORD;
	Bound.Min.Z = (FLOAT)+MAXSWORD;
	Bound.Max.X = (FLOAT)-MAXSWORD;
	Bound.Max.Y = (FLOAT)-MAXSWORD;
	Bound.Max.Z = (FLOAT)-MAXSWORD;
	//
	// Recursively update all our childrens' bounding volumes, which updates this
	// node's bounding volume to enclose theirs.
	//
	if (Node->iFront != INDEX_NONE)  OurChildren += BuildBound (ModelInfo, Node->iFront, &Bound);
	if (Node->iBack  != INDEX_NONE)  OurChildren += BuildBound (ModelInfo, Node->iBack,  &Bound);
	if (Node->iPlane != INDEX_NONE)  OurChildren += BuildBound (ModelInfo, Node->iPlane, &Bound);
	//
	// Update the bounding volume to enclose the polygon in this node.  Make it
	// slightly larger than needed to account for rounding errors.
	//
	for (i=0; i<Node->NumVertices; i++)
		{
		C = ModelInfo->FPoints[VertPool[i].pVertex].X;
		if (C <= Bound.Min.X) Bound.Min.X = C;
		if (C >= Bound.Max.X) Bound.Max.X = C;
		//
		C = ModelInfo->FPoints[VertPool[i].pVertex].Y;
		if (C <= Bound.Min.Y) Bound.Min.Y = C;
		if (C >= Bound.Max.Y) Bound.Max.Y = C;
		//
		C = ModelInfo->FPoints[VertPool[i].pVertex].Z;
		if (C <= Bound.Min.Z) Bound.Min.Z = C;
		if (C >= Bound.Max.Z) Bound.Max.Z = C;
		};
	//
	// Update parent's bounding volume:
	//
	if (ParentBound)
		{
		if (Bound.Min.X < ParentBound->Min.X) ParentBound->Min.X = Bound.Min.X;
		if (Bound.Max.X > ParentBound->Max.X) ParentBound->Max.X = Bound.Max.X;
		//
		if (Bound.Min.Y < ParentBound->Min.Y) ParentBound->Min.Y = Bound.Min.Y;
		if (Bound.Max.Y > ParentBound->Max.Y) ParentBound->Max.Y = Bound.Max.Y;
		//
		if (Bound.Min.Z < ParentBound->Min.Z) ParentBound->Min.Z = Bound.Min.Z;
		if (Bound.Max.Z > ParentBound->Max.Z) ParentBound->Max.Z = Bound.Max.Z;
		};
	//
	// Set node's number-of-children flags (used in renderer to optimized node rejection),
	// and decide whether to do bounding volume.  Do bounding volume if we have many
	// children or we're the root.
	//
	if ((OurChildren >= FBspNode::MANY_CHILDREN) || !ParentBound)
		{
		if (Node->iBound==INDEX_NONE)
			{
			Node->iBound = ModelInfo->NumBounds++;
			if (ModelInfo->NumBounds > ModelInfo->MaxBounds) appError ("Bound table full");
			};
		};
	if (Node->iBound != INDEX_NONE)
		{
		Node->NodeFlags |= NF_Bounded;
		ModelInfo->Bounds[Node->iBound] = Bound;
		};
	return OurChildren;
	};

//
// Rebuild all Bsp node bounding volumes.  Changes to Bsp node bounding volumes
// aren't logged in the transaction tracking system, so undo/redo must rebuild
// the boxes themselves.
//
void FEditor::bspBuildBounds (IModel *ModelInfo)
	{
	GUARD;
	if (ModelInfo->NumBspNodes>0) BuildBound (ModelInfo,0,NULL);
	UNGUARD("BspBuildBounds");
	};

#endif

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
