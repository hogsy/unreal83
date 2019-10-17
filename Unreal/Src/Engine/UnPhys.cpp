/*=============================================================================
    UnPhys.cpp: Simple physics and occlusion testing for editor

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*---------------------------------------------------------------------------------------
   Parameters
---------------------------------------------------------------------------------------*/

#define COLLISION_FUDGE 0.03 /* Threshold for pushing points back extra behind planes in SphereFilter*/

/*---------------------------------------------------------------------------------------
   Point classification (for particles/projectiles)
---------------------------------------------------------------------------------------*/

//
// Recursive minion of IModel::PointClass
//
int TestPointClass (const IModel *ModelInfo, const FVector *Location, INDEX *iDestNode, int Class)
	{
	FBspNode	*Node;
	FBspSurf	*Surf;
	FLOAT		Dist;
	INDEX		iNode,TempNode;
	int			CSG;
	//
	iNode = 0;
	do	{
		TempNode = iNode;
		Node 		= &ModelInfo->BspNodes [iNode];
		Surf 		= &ModelInfo->BspSurfs [Node->iSurf];
		CSG  		= Node->IsCsg();
		//
		Dist=FPointPlaneDist
			(
			*Location,
			ModelInfo->FPoints [Surf->pBase],
			ModelInfo->FVectors[Surf->vNormal]
			);
		if (Dist>0)
			{
			if (CSG) Class = 1;
			iNode = Node->iFront;
			}
		else
			{
			if (CSG) Class = 0;
			iNode = Node->iBack;
			};
		} while (iNode != INDEX_NONE);
	if (iDestNode != NULL) *iDestNode = TempNode;
	return Class;
	};

//
// Classify a point as inside (0) or outside (1), and also set its node location
// (INDEX_NONE if Bsp is empty):
//
int IModel::PointClass (const FVector *Location, INDEX *iDestNode) const
	{
	GUARD;
	if (NumBspNodes!=0)
		{
   		return TestPointClass(this,Location,iDestNode,1);
		}
	else
		{
		if (iDestNode != NULL) *iDestNode = INDEX_NONE;
   		return 1;
		};
	UNGUARD("IModel::PointClass");
	};

/*---------------------------------------------------------------------------------------
   Zone determination
---------------------------------------------------------------------------------------*/

//
// Figure out which zone a point is in, and return it.  A value of
// zero indicates that the point doesn't fall into any zone.
//
BYTE IModel::PointZone (const FVector *Location) const
	{
	GUARD;
	ENodePlace	Place;
	FBspNode	*Node;
	FBspSurf	*Surf;
	FLOAT		Dist;
	INDEX		iNode;
	int			CSG,Class;
	//
	if ((NumZones==0)||(NumBspNodes==0)) return 0;
	//
	Class	= 1;
	iNode	= 0;
	Place   = NODE_Front;
	Node	= &BspNodes[0];
	//
	while (iNode!=INDEX_NONE)
		{
		Node = &BspNodes [iNode];
		Surf = &BspSurfs [Node->iSurf];
		CSG  = Node->IsCsg();
		//
		Dist=FPointPlaneDist
			(
			*Location,
			FPoints [Surf->pBase],
			FVectors[Surf->vNormal]
			);
		if (Dist>=0.0)
			{
			if (CSG) Class = 1;
			iNode = Node->iFront;
			Place = NODE_Front;
			}
		else
			{
			if (CSG) Class = 0;
			iNode = Node->iBack;
			Place = NODE_Back;
			};
		};
	if		(!Class)			return 0;
	else if (Place==NODE_Front) return Node->iZone;
	else						return Node->iBackZone;
	UNGUARD("IModel::PointClassPointZone");
	};

/*---------------------------------------------------------------------------------------
   Fuzzy point classification (for corner radiosity)
---------------------------------------------------------------------------------------*/

//
// Recusive minion of IModel::FuzzyPointClass
//
FLOAT FuzzyPointClass (const IModel *ModelInfo, const FVector *Location, FLOAT RRadius, int Class, INDEX iNode)
	{
	FBspNode *Node;
	FBspSurf *Surf;
	FVector  *Base,*Normal;
	FLOAT    Dist;
	FLOAT	 FrontFrac;
	int      CSG;
	//
	while (iNode!=INDEX_NONE)
		{
		Node 	= &ModelInfo->BspNodes [iNode];
		Surf 	= &ModelInfo->BspSurfs [Node->iSurf];
		CSG  	= Node->IsCsg();
		Base	= &ModelInfo->FPoints  [Surf->pBase];
		Normal	= &ModelInfo->FVectors [Surf->vNormal];
		//
		switch (ModelInfo->FVectors[Surf->vNormal].Align)
			{
			case FVA_Z:
				Dist = (Location->Z-Base->Z)*Normal->Z * RRadius;
				break;
			case FVA_X:
				Dist = (Location->X-Base->X)*Normal->X * RRadius;
				break;
			case FVA_Y:
				Dist = (Location->Y-Base->Y)*Normal->Y * RRadius;
				break;
			case FVA_None:
				Dist =
					(Location->X-Base->X)*Normal->X * RRadius +
					(Location->Y-Base->Y)*Normal->Y * RRadius +
					(Location->Z-Base->Z)*Normal->Z * RRadius;
				break;
			default:
				appError("FuzzyPointClass: Bad Alignment");
				return 0.0;
			};
		if (Dist >= 1.0)
			{
			Class = Class || CSG;
			iNode = Node->iFront;
			}
		else if (Dist <= -1.0)
	        {
			Class = Class && !CSG;
			iNode = Node->iBack;
			}
		else
			{
			FrontFrac = 0.5 * (Dist + 1.0);
			FrontFrac *= FrontFrac;
			return
				(FrontFrac    )*FuzzyPointClass(ModelInfo,Location,RRadius,Class|| CSG,Node->iFront) +
				(1.0-FrontFrac)*FuzzyPointClass(ModelInfo,Location,RRadius,Class&&!CSG,Node->iBack);
			};
		};
	return (Class ? 1.0 : 0.0);
	};

//
// Classify a point as inside=0.0, outside=1.0, or kinda=inbetween.
//
FLOAT IModel::FuzzyPointClass (const FVector *Location, FLOAT Radius) const
	{
	GUARD;
	//
	if (NumBspNodes>0) return ::FuzzyPointClass(this,Location,1.0/Radius,1,0);
	else return 1.0;
	//
	UNGUARD("IModel::FuzzyPointClass");
	};

/*---------------------------------------------------------------------------------------
   Fuzzy raytracing
---------------------------------------------------------------------------------------*/

/* Discrete
FLOAT FuzzyLineOfSight (IModel *ModelInfo, WORD iNode,
	FVECTOR *V1, FVECTOR *V2, FLOAT Radius, FLOAT RRadius,FLOAT Class)
	{
	FBspNode   *Node;
	FBspSurf   *Poly;
	FVECTOR    MidVector;
	FLOAT      TimeDenominator,Time,Dist1,Dist2,FrontSize,BackSize,FrontFraction;
	int        CSG;
	//
	Loop:
	//
	if (iNode==MAXWORD) return Class;
	//
	Node = &ModelInfo->BspNodes [iNode];
	Poly = &ModelInfo->BspSurfs [Node->iSurf];
	CSG  = NODE_IS_CSG(Node);
	//
	// Check side-of-plane for both points:
	//
	Dist1=FPointPlaneDist(V1,
		&ModelInfo->FPoints [Poly->pBase],
		&ModelInfo->FVectors[Poly->vNormal]);
	Dist2=FPointPlaneDist(V2,
		&ModelInfo->FPoints [Poly->pBase],
		&ModelInfo->FVectors[Poly->vNormal]);
	//
	// Classify line based on both distances:
	//
	if ((Dist1 > -0.001) && (Dist2 > -0.001)) // Both points are in front
		{
		if (CSG) Class = 1.0;
		iNode  = Node->iFront;
		goto Loop;
		}
	else if ((Dist1 < 0.001)&&(Dist2 < 0.001)) // Both points are in back
		{
		if (CSG) Class = 0.0;
		iNode  = Node->iBack;
		goto Loop;
		}
	else // Line is split (guranteed to be non-parallel to plane, so TimeDenominator != 0)
		{
		//
		// Compute Fraction = % in front
		//
		//FrontSize = 
		//FrontFraction = 0.5 * (
		//
		// Should special case axis-aligned stuff (=majority of world!)
		// Most of these computations are redundent due to Dist1,Dist2.
		//
		Time        = (0.0-Dist1)/(Dist2-Dist1);
		MidVector.X = V1->X + (V2->X - V1->X) * Time;
		MidVector.Y = V1->Y + (V2->Y - V1->Y) * Time;
		MidVector.Z = V1->Z + (V2->Z - V1->Z) * Time;
		//
		if (Dist1 > 0.0) // Dist2 < 0
			{
			if (!FuzzyLineOfSight (ModelInfo,Node->iFront,V1,&MidVector,Radius,RRadius,CSG?1.0:Class)) return 0;
			return FuzzyLineOfSight (ModelInfo,Node->iBack,&MidVector,V2,Radius,RRadius,CSG?0.0:Class);
			}
		else // Dist1 < 0, Dist2 > 0
			{
			if (!FuzzyLineOfSight (ModelInfo,Node->iFront,V2,&MidVector,Radius,RRadius,CSG?1.0:Class)) return 0;
			return FuzzyLineOfSight (ModelInfo,Node->iBack,&MidVector,V1,Radius,RRadius,CSG?0.0:Class);
			};
		};
	};
*/

//
// Optimizations:
// - Don't raytrace player
// - Don't raytrace surfs that are all-invisible or
//   maybe all-visible (though this loses the inside corner shading).
// - Sphere filter through static lights' iteration-based shadow
//   volume Bsp instead of raytracing the world Bsp.
// - If using world Bsp, do a coherent Z-array based raytracer
//

//
// Line of sight globals
//
static class FPhysGLOS
	{
	public:
	const IModel	*ModelInfo;
	FBspNode		*BspNodes;
	FBspSurf		*BspSurfs;
	FVector			*FVectors;
	FVector			*FPoints;
	FVector			V0;		// Starting point, where parameter = 0.0
	FVector			V1;		// Ending point, where parameter = 1.0
	FVector			RayVector;
	FLOAT			Length;
	FLOAT			Radius,RRadius;
	} GLOS;

//
// Recursive minion of IModel::FuzzyLineClass
//
FLOAT FuzzyLineOfSight (WORD iNode,FLOAT T0,FLOAT T1,int Class)
	{
	FBspNode	*Node;
	FBspSurf	*Surf;
	FVector		*Normal,*Base;
	FLOAT		T,Tmin,Tmax,Dist0,Dist1,Factor,Dist,BaseNorm;
	int			CSG;
	//
	Factor = 1.0;
	while (iNode!=INDEX_NONE)
		{
		Node	= &GLOS.BspNodes [iNode];
		Surf	= &GLOS.BspSurfs [Node->iSurf];
		CSG		= Node->IsCsg();
		Base	= &GLOS.FPoints  [Surf->pBase];
		Normal	= &GLOS.FVectors [Surf->vNormal];
		//
		// Check side-of-plane for both points:
		//
		switch (GLOS.FVectors[Surf->vNormal].Align)
			{
			case FVA_Z:
				Dist0 = (GLOS.V0.Z-Base->Z)*Normal->Z;
				Dist1 = (GLOS.V1.Z-Base->Z)*Normal->Z;
				break;
			case FVA_X:
				Dist0 = (GLOS.V0.X-Base->X)*Normal->X;
				Dist1 = (GLOS.V1.X-Base->X)*Normal->X;
				break;
			case FVA_Y:
				Dist0 = (GLOS.V0.Y-Base->Y)*Normal->Y;
				Dist1 = (GLOS.V1.Y-Base->Y)*Normal->Y;
				break;
			case FVA_None:
				BaseNorm = (*Base   | *Normal); // Lookup 
				Dist0    = (GLOS.V0 | *Normal) - BaseNorm;
				Dist1    = (GLOS.V1 | *Normal) - BaseNorm;
				break;
			default:
				appError("FuzzyLineOfSight: Bad alignment");
				return 0.0;
			};
		Dist  = Dist1-Dist0;
		T     = -Dist0;
		//
		if (Dist < 0)
			{
			Tmin = T + GLOS.Radius;
			Tmax = T - GLOS.Radius;
			//
			if (Tmax > T0*Dist) // Both points are on Dist1 side
				{
				if (Dist1 > 0.0)	{iNode = Node->iFront; Class = Class ||  CSG;}
				else					{iNode = Node->iBack;  Class = Class && !CSG;};
				}
			else if (Tmin < T1*Dist) // Both points are on Dist0 side
				{
				if (Dist0 > 0.0)	{iNode = Node->iFront; Class = Class ||  CSG;}
				else					{iNode = Node->iBack;  Class = Class && !CSG;};
				}
			else // Line is split
				{
				Tmin /= Dist;
				Tmax /= Dist;
				//
				Factor *= FuzzyLineOfSight (Node->iFront,T0,OurMin(Tmax,T1),Class||CSG);
				if (Factor==0.0) return 0.0;
				iNode  = Node->iBack;
				Class  = Class && !CSG;
				T0     = OurMax(Tmin,T0);
				};
			}
		else
			{
			Tmin = T - GLOS.Radius;
			Tmax = T + GLOS.Radius;
			//
			if (Tmax<T0*Dist) // Both points are on Dist1 side
				{
				if (Dist1 > 0.0)	{iNode = Node->iFront; Class = Class ||  CSG;}
				else					{iNode = Node->iBack;  Class = Class && !CSG;};
				}
			else if (Tmin>T1*Dist) // Both points are on Dist0 side
				{
				if (Dist0 > 0.0)	{iNode = Node->iFront; Class = Class ||  CSG;}
				else					{iNode = Node->iBack;  Class = Class && !CSG;};
				}
			else // Line is split
				{
				Tmin /= Dist;
				Tmax /= Dist;
				//
				Factor *= FuzzyLineOfSight (Node->iBack,T0,OurMin(Tmax,T1),Class && !CSG);
				if (Factor==0.0) return 0.0;
				iNode	= Node->iFront;
				Class	= Class || CSG;
				T0		= OurMax(Tmin,T0);
				};
			};
		};
	if (!Class)
		{
		Factor *= 1.0 - 0.5 * (T1-T0) * (T1-T0) * GLOS.Length * GLOS.RRadius;
		if (Factor<0.0) Factor = 0.0;
		};
	return Factor;
	};

//
// Classify a line as unobstructed (1.0), obstructed (0.0), or somewhere
// in between, based on a radius.  Note that this function can penetrate
// walls of thickness less than Radius.
//
FLOAT IModel::FuzzyLineClass (const FVector *V1, const FVector *V2, FLOAT Radius) const
	{
	GUARD;
	//
	GLOS.V0			= *V1;
	GLOS.V1			= *V2;
	GLOS.ModelInfo	= this;
	GLOS.FVectors	= FVectors;
	GLOS.FPoints	= FPoints;
	GLOS.BspNodes	= BspNodes;
	GLOS.BspSurfs	= BspSurfs;
	GLOS.Radius		= Radius;
	GLOS.RRadius	= 1.0/Radius;
	//
	GLOS.RayVector = *V2 - *V1;
	GLOS.Length    = GLOS.RayVector.Size();
	//
	if (NumBspNodes) return FuzzyLineOfSight (0,0.0,1.0,1);
	else return 1.0;
	//
	UNGUARD("IModel::FuzzyLineClass");
	};

/*---------------------------------------------------------------------------------------
   Line classification (line-of-sight)
---------------------------------------------------------------------------------------*/

//
// Recursive minion of IModel::LineClass
//
int LineOfSight (const IModel *ModelInfo, INDEX iNode, const FVector *V1, const FVector *V2, int Class)
	{
	FBspNode   *Node;
	FBspSurf   *Surf;
	FVector    MidVector;
	FLOAT      Time,Dist1,Dist2;
	int        CSG;
	//
	Loop:
	//
	if (iNode==INDEX_NONE) return Class;
	//
	Node = &ModelInfo->BspNodes [iNode];
	Surf = &ModelInfo->BspSurfs [Node->iSurf];
	CSG  = Node->IsCsg();
	//
	// Check side-of-plane for both points:
	//
	Dist1=FPointPlaneDist(*V1,
		ModelInfo->FPoints [Surf->pBase],
		ModelInfo->FVectors[Surf->vNormal]);
	Dist2=FPointPlaneDist(*V2,
		ModelInfo->FPoints [Surf->pBase],
		ModelInfo->FVectors[Surf->vNormal]);
	//
	// Classify line based on both distances:
	//
	if ((Dist1 > -0.001) && (Dist2 > -0.001)) // Both points are in front
		{
		Class |= CSG;
		iNode  = Node->iFront;
		goto Loop;
		}
	else if ((Dist1 < 0.001)&&(Dist2 < 0.001)) // Both points are in back
		{
		Class &= !CSG;
		iNode  = Node->iBack;
		goto Loop;
		}
	else // Line is split (guranteed to be non-parallel to plane, so TimeDenominator != 0)
		{
		Time = (0.0-Dist1)/(Dist2-Dist1);
		//
		MidVector.X = V1->X + (V2->X - V1->X) * Time;
		MidVector.Y = V1->Y + (V2->Y - V1->Y) * Time;
		MidVector.Z = V1->Z + (V2->Z - V1->Z) * Time;
		//
		if (Dist1 > 0.0) // Dist2 < 0
			{
			if (!LineOfSight (ModelInfo,Node->iFront,V1,&MidVector,Class | CSG)) return 0;
			return LineOfSight (ModelInfo,Node->iBack,&MidVector,V2,Class  & !CSG);
			}
		else // Dist1 < 0, Dist2 > 0
			{
			if (!LineOfSight (ModelInfo,Node->iFront,V2,&MidVector,Class | CSG)) return 0;
			return LineOfSight (ModelInfo,Node->iBack,&MidVector,V1,Class & !CSG);
			};
		};
	};

//
// Classify a line as unobstructed (1) or obstructed (0).
//
int IModel::LineClass(const FVector *V1, const FVector *V2) const
	{
	GUARD;
	//
	if (NumBspNodes!=0)	return LineOfSight (this,0,V1,V2,1);
	else				return 1;
	//
	UNGUARD("IModel::LineClass");
	};

/*---------------------------------------------------------------------------------------
   Raytracing with an optional leaf callback
---------------------------------------------------------------------------------------*/

class FRaytraceParams
	{
	public:
	IModel::RAYTRACE_CALLBACK Callback;
	FVector				*HitLocation;
	FVector				*HitNormal;
	INDEX				*HitNode;
	FLOAT				ConeRadiusFactor;
	int					Param;
	int					Stop;
	};

//
// Recursive minion of IModel::Raytrace
//
int Raytrace (IModel *ModelInfo, INDEX iNode, FVector V1, FVector V2, int Class,
	FRaytraceParams *Params)
	{
	int TestResult;
	//
	Loop:
	if (Params->Stop) return Class;
	//
	FBspNode	*Node	= &ModelInfo->BspNodes	[iNode];
	FBspSurf	*Surf	= &ModelInfo->BspSurfs	[Node->iSurf];
	FVector		*Base	= &ModelInfo->FPoints	[Surf->pBase];
	FVector		*Normal	= &ModelInfo->FVectors	[Surf->vNormal];
	int			CSG		= Node->IsCsg();
	//
	// Check side-of-plane for both points:
	//
	FLOAT		Dist1	= FPointPlaneDist(V1,*Base,*Normal);
	FLOAT		Dist2	= FPointPlaneDist(V2,*Base,*Normal);
	//
	// Classify line based on both distances:
	//
	if ((Dist1 > -0.001) && (Dist2 > -0.001)) // Both points are in front
		{
		DoFront:
		Class = Class || CSG;
		if (Node->iFront==INDEX_NONE) // Front leaf
			{
			if (Class) Params->Stop = Params->Callback(ModelInfo,iNode,0,Params->Param); // Outside leaf
			else Params->Stop       = 1; // Inside leaf
			return Class;
			};
		iNode = Node->iFront;
		goto Loop;
		}
	else if ((Dist1 < 0.001)&&(Dist2 < 0.001)) // Both points are in back
		{
		DoBack:
		Class = Class && !CSG;
		if (Node->iBack==INDEX_NONE) // Back leaf
			{
			if (Class) Params->Stop = Params->Callback(ModelInfo,iNode,1,Params->Param); // Outside leaf
			else       Params->Stop = 1; // Inside leaf
			return Class;
			};
		iNode = Node->iBack;
		goto Loop;
		}
	else
		{
		//
		// Line is split.
		// It's guranteed to be non-parallel to plane, so TimeDenominator != 0.
		// Now we go and raytrace the nearest part then the furthest part.
		//
		FVector MidVector = V1 + (V1 - V2) * (Dist1/(Dist2-Dist1));
		//
		if (Dist1 > 0.0) // Dist2 < 0
			{
			// Process nearest part (front):
			int FrontClass = Class || CSG;
			if (Node->iFront!=INDEX_NONE)
				{
				// Recurse down front
				TestResult = Raytrace (ModelInfo,Node->iFront,V1,MidVector,FrontClass,Params);
				if (Params->Stop) return TestResult;
				}
			else
				{
				// Outside leaf
				if (FrontClass) Params->Stop = Params->Callback(ModelInfo,iNode,0,Params->Param);
				else			Params->Stop = 1; // Inside leaf
				if (Params->Stop) return FrontClass;
				};
			// Handle CSG outside->inside transition:
			if (CSG)
				{
				*(Params->HitLocation)	= MidVector;
				*(Params->HitNormal)	= *Normal;
				*(Params->HitNode)		= iNode;
				};
			// Process furthest part (back):
			V1 = MidVector;
			goto DoBack;
			}
		else // Dist1 < 0, Dist2 > 0
			{
			// Process nearest part (back):
			int BackClass = Class && !CSG;
			if (Node->iBack!=INDEX_NONE)
				{
				// Recurse down back:
				TestResult = Raytrace (ModelInfo,Node->iBack,V1,MidVector,BackClass,Params);
				if (Params->Stop) return TestResult;
				}
			else
				{
				// Inside leaf
				if (BackClass)	Params->Stop = Params->Callback(ModelInfo,iNode,1,Params->Param);
				else			Params->Stop = 1; // Inside leaf
				if (Params->Stop) return BackClass;
				};
			// Handle CSG inside->outside transition:
			// Process furthest part (front):
			V1 = MidVector;
			goto DoFront;
			};
		};
	};

//
// Filter a line through the Bsp, calling the specified callback for each leaf reached
// (in order, starting at V1's leaf and ending at V2's).  Continues until the entire
// line has been raytraced, or the callback returns a zero, signaling that tracing should
// stop.
//
// Returns 1 if unobstructed, 0 if obstructed.
//
// If obstructed, sets HitLocation, HitNormal, and HitNode according to the closest point
// that was hit.  If the ray is obstructed without hitting anything (i.e. if the source
// point is obstructed), sets HitLocation to the source location, HitNormal to the Z-Axis 
// Vector, and sets HitNode to INDEX_NONE.
//
// If ConeRadiusFactor is zero, does a normal raytrace operation.  If ConeRadiusFactor
// is positive, the raytrace is performed as a cone trace, where the cone's distance
// is equal to ConeRadiusFactor times the distance of the test point from V1 along the line
// from V1 to V2.
//
// Purpose: Actor/projectile collision, actor weapon targeting.
//
int IModel::Raytrace(const FVector *V1, const FVector *V2, FLOAT ConeRadiusFactor,
	RAYTRACE_CALLBACK Callback, int Param,
	FVector *HitLocation, FVector *HitNormal, INDEX *iHitNode)
	{
	GUARD;
	if (NumBspNodes==0) return 1;
	//
	// Defaults:
	//
	*HitLocation = *V1;
	*HitNormal   = GMath.ZAxisVector;
	*iHitNode    = INDEX_NONE;
	//
	// Set up Params:
	//
	FRaytraceParams Params;
	Params.Callback				= Callback;
	Params.HitLocation			= HitLocation;
	Params.HitNormal			= HitNormal;
	Params.HitNode				= iHitNode;
	Params.ConeRadiusFactor		= ConeRadiusFactor;
	Params.Param				= Param;
	Params.Stop					= 0;
	//
	// Perform raytrace:
	//
	int Result = ::Raytrace (this,0,*V1,*V2,1,&Params);
	//
	// If raytracer ran into a fake backdrop surf, treat it as unoccluded.
	//
	//debugging: for (int i=0; i<NumBspSurfs; i++) BspSurfs[i].PolyFlags &= ~PF_SELECTED;
	if ((Result==0) && (*iHitNode != INDEX_NONE))
		{
		FBspNode *Node = &BspNodes[*iHitNode];
		FBspSurf *Surf = &BspSurfs[Node->iSurf];
		// Debugging: Surf->PolyFlags |= PF_SELECTED;
		if (Surf->PolyFlags & PF_FakeBackdrop) return 1; // Unoccluded
		else return 0; // Occluded
		}
	else return 1; // Unoccluded
	//
	UNGUARD("IModel::Raytrace");
	};

/*---------------------------------------------------------------------------------------
	Z axis-aligned ray filtering for player collision
---------------------------------------------------------------------------------------*/

//
// Recursive minion of IModel::ZCollision
//
INDEX FilterZRay (const IModel *ModelInfo, INDEX iNode, INDEX iHit, FVector V, FLOAT EndZ, FVector *Hit, 
	INDEX *iActorHit, int Outside)
	{
	FBspNode   	*Node;
	FBspSurf   	*Surf;
	FVector    	*Normal,*Base;
	FLOAT    	Dist,DZ;
	INDEX		Result;
	int        	CSG,Split;
	//
	while (1)
		{
		if (iNode==INDEX_NONE)
	   		{
			if (Outside)	return INDEX_NONE;
			else			return iHit;
   			};
		Node 		= &ModelInfo->BspNodes 	[iNode];
		Surf 		= &ModelInfo->BspSurfs 	[Node->iSurf];
		Base		= &ModelInfo->FPoints 	[Surf->pBase];
		Normal		= &ModelInfo->FVectors	[Surf->vNormal];
		CSG  		= Node->IsCsg();
		//
		// Check side-of-plane for point, then find Z intersection based on it:
		//
		Dist = FPointPlaneDist(V,*Base,*Normal);
		//
		if (OurAbs(Normal->Z) > 0.001)
			{
			DZ 	= -Dist / Normal->Z;
			Split = (DZ < 0.0) && ((V.Z + DZ) > EndZ);
			}
		else
			{
			Split = 0;
			DZ    = 0.0;
			};
		//
		// Classify line based on both distances:
		//
		if (!Split)
			{
			if (Dist > 0.0) // Entire ray is in front of node, no intersection
				{
				Outside |= CSG;
				iNode    = Node->iFront;
				}
			else // Dist < 0.0, Entire ray is in back of node, no intersection
				{
				Outside &= !CSG;
				iNode    = Node->iBack;
				if (CSG) *iActorHit = Surf->iActor;
				}
			}
		else // Ray is split.  Process both halves in nearest-to-furthest order
			{
			if (Dist > 0.0) // Front then back
				{
				Result = FilterZRay(ModelInfo,Node->iFront,iHit,V,V.Z + DZ,Hit,iActorHit,Outside | CSG);
				if (Result != INDEX_NONE) return Result;
				//
				V.Z      += DZ;
				if (CSG) {*Hit = V; iHit = iNode;};
				if (CSG) *iActorHit = Surf->iActor;
				Outside  &= !CSG;
				iNode     = Node->iBack;
				}
			else // Back then front
				{
				INDEX iTemp = *iActorHit;
				if (CSG) *iActorHit = Surf->iActor;
				Result = FilterZRay(ModelInfo,Node->iBack,iHit,V,V.Z + DZ,Hit,iActorHit,Outside & !CSG);
				if (Result != INDEX_NONE) return Result;
				*iActorHit = iTemp;
				//
				V.Z      += DZ;
				if (CSG) {*Hit = V; iHit = iNode;};
				Outside  |= CSG;
				iNode     = Node->iFront;
				};
			};
		};
	};

//
// Filter a Z-aligned ray down the Bsp starting at a point, and find the first thing
// it hits.  Returns the Bsp node index for the collision, or INDEX_NONE of no collision.
// If collision, sets Hit vector to location of intersecti
//
INDEX IModel::ZCollision(const FVector *V, FVector *Hit, INDEX *iActorHit) const
	{
	GUARD;
	*iActorHit	= INDEX_NONE;
	*Hit		= *V; // Default
	//
	if (NumBspNodes!=0)	return FilterZRay (this,0,INDEX_NONE,*V,V->Z - 100000.0,Hit,iActorHit,1);
	else				return INDEX_NONE;
	//
	UNGUARD("IModel::ZCollision");
	};

/*---------------------------------------------------------------------------------------
   Sphere collision
---------------------------------------------------------------------------------------*/

class FCollisionLocals
	{
	public:
	FLOAT 		BackAdjustmentSize;
	FVector 	BackAdjustment;
	};

class FCollisionGlobals
	{
	public:
	FLOAT	AdjustmentSize;
	FVector	Adjustment;
	FVector	MoveVector;
	int		FavorSmoothness;
	};

//
// Filter through all nodes that the sphere lies in, down to leaves.
// If within any inside leaf, classify as inside.
//
// Returns 1 if collision (sphere is inside or sphere has collided with a polygon),
// or 0 if no collison.  If collision, Globals.AdjustmentSize is a suggested adjustment
// vector to get the sphere out of the wall.
//
int SphereCollision (const IModel *ModelInfo, INDEX iNode, const FVector *Dest,
	FCollisionLocals Locals, FCollisionGlobals *Globals, FLOAT Radius, int Outside)
	{
	FBspNode 			*Node;
	FBspSurf 			*Surf;
	FLOAT    			Dist,TempAdjustmentSize;
	FVector				Normal,TempAdjustment;
	int      			CSG,Collision,Update;
	//
	// Loop through convex volumes (backs) while recursing with fronts:
	//
	Collision = 0;
	//
	BackLoop:
	//
	Node = &ModelInfo->BspNodes [iNode];
	Surf = &ModelInfo->BspSurfs [Node->iSurf];
	CSG  = Node->IsCsg();
	//
	if (!(Surf->PolyFlags & PF_HighLedge))
		{
		Dist = FPointPlaneDist(*Dest,ModelInfo->FPoints [Surf->pBase],ModelInfo->FVectors[Surf->vNormal]);
		}
	else
		{
		TempAdjustment = ModelInfo->FVectors[Surf->vNormal] * 56.0 + ModelInfo->FPoints[Surf->pBase];
		Dist = FPointPlaneDist(*Dest,TempAdjustment,ModelInfo->FVectors[Surf->vNormal]);
		};
	//
	// Calculate collision adjustment.  Will be replaced if sphere collides
	// with a poly further down the tree's back.
	//
	Update = 0;
	if (CSG && (Dist >= -Radius) && (Dist <= Radius))
		{
		Normal          = ModelInfo->FVectors[Surf->vNormal];
		TempAdjustment  = Normal * (Radius + COLLISION_FUDGE - Dist);
		//
		if (Globals->FavorSmoothness) // Favor small movements (looking for minimum adjustment size)
			{
			TempAdjustmentSize = TempAdjustment.Size();
			if (Globals->FavorSmoothness!=2) TempAdjustmentSize /= Normal | Globals->MoveVector;
			TempAdjustmentSize = OurAbs(TempAdjustmentSize);
			}
		else // Favor large, non-floor movements
			{
			TempAdjustmentSize = -(1.0+OurAbs(TempAdjustment.X)+OurAbs(TempAdjustment.Y));
			};
		//
		// Fix up adjustment so it doesn't try to move the player up/down:
		//
		if ((Normal.Z!=0.0) && (OurAbs(Normal.Z)<0.85) && (Globals->FavorSmoothness != 2))
			{
			FLOAT Temp = 1.02 / Normal.Size2D();
			TempAdjustment.X *= Temp;
			TempAdjustment.Y *= Temp;
			TempAdjustment.Z  = 0;
			};
		if ((Locals.BackAdjustmentSize==0.0) || (TempAdjustmentSize <= Locals.BackAdjustmentSize))
			{
      		Locals.BackAdjustmentSize	= TempAdjustmentSize;
			Update						= 1;
			};
		};
	//
	// Check front (outside)
	//
	if ((Dist >= -Radius) && (Node->iFront!=INDEX_NONE))
		{
		Collision |= SphereCollision (ModelInfo,Node->iFront,Dest,Locals,Globals,Radius,Outside | CSG);
		};
	if (Update) Locals.BackAdjustment = TempAdjustment;
	//
	// Loop with back
	//
	if (Dist <= Radius)
		{
		Outside &= !CSG;
		iNode    = Node->iBack;
		//
		if (iNode!=INDEX_NONE) goto BackLoop;
		//
	   	if (!Outside)
			{
      		if (
				(( Globals->FavorSmoothness) && (Locals.BackAdjustmentSize >= Globals->AdjustmentSize)) ||
				((!Globals->FavorSmoothness) && ((Globals->AdjustmentSize==0.0)||(Locals.BackAdjustmentSize < Globals->AdjustmentSize)))
				)
				{
         		Globals->Adjustment     = Locals.BackAdjustment;
         		Globals->AdjustmentSize = Locals.BackAdjustmentSize;
         		};
			return 1;
			};
		};
	return Collision;
	};

//
// Find reflection vector of a sphere colliding with the world.
// Assumes that sphere has collided with a wall.
// Returns 1 if reflected successfully, 0 if unreflectable.
//
int IModel::SphereReflect (FVector *Location,FLOAT Radius, FVector *ReflectionVector) const 
	{
	GUARD;
	FVector				TempLocation = *Location;
	FCollisionGlobals	Globals;
	FCollisionLocals	Locals;
	//
	if (NumBspNodes!=0)
		{
		Globals.FavorSmoothness		= 0;
		Globals.AdjustmentSize 		= 0.0;
		Globals.MoveVector			= GMath.ZeroVector;
		Locals.BackAdjustmentSize 	= 0.0;
		Locals.BackAdjustment 		= GMath.ZeroVector;
		//
		if (!SphereCollision(this,0,&TempLocation,Locals,&Globals,Radius,1)) return 0;
		//
		*ReflectionVector = Globals.Adjustment;
		return 1;
		}
	else return 1;
	//
	UNGUARD("IModel::SphereReflect");
	};

//
// Try to move a sphere by a movement vector, which must be less
// than the sphere's radius.  If fits, returns 1.  If blocked, returns
// 0, and sets Adjustment to a suggested adjustment which might move
// the sphere out of the wall.
//
int IModel::SphereTestMove(const FVector *Location, const FVector *Delta, FVector *ResultAdjustment,
	FLOAT Radius, int FavorSmoothness) const
	{
	if (NumBspNodes!=0)
		{
		FCollisionGlobals			Globals;
		Globals.FavorSmoothness		= FavorSmoothness;
		Globals.Adjustment			= GMath.ZeroVector;
		Globals.AdjustmentSize 		= 0.0;
		Globals.MoveVector			= *Delta;
		//
		FCollisionLocals			Locals;
		Locals.BackAdjustment 		= GMath.ZeroVector;
		Locals.BackAdjustmentSize 	= 0.0;
		//
		int Result = !SphereCollision(this,0,Location,Locals,&Globals,Radius,1);
		*ResultAdjustment = Globals.Adjustment;
		return Result;
		}
	else return 1;
	};

//
// Try to move a sphere by a movement vector, which must be less
// than the sphere's radius.  If it doesn't fit, retries a few times
// in order to properly collide with multiple simultaneous walls.
//
// Glides along walls.
//
// Returns 1 if full or partial movement occured, 0 if no movement.
//
int IModel::SphereNearMove(FVector *Location,const FVector *Delta, FLOAT Radius, int FavorSmoothness) const
	{
	GUARD;
	FVector	NewLocation	= *Location + *Delta;
	int		Iteration	= 0;
	int		Moved		= 0;
	//
	while (Iteration++ < 4)
		{
		FVector Adjustment;
		if (SphereTestMove(&NewLocation,Delta,&Adjustment,Radius,FavorSmoothness))
			{
			// Moved without collision on this iteration - we have a valid destination
			*Location = NewLocation;
			Moved     = 1; 
			break;
			};
		// Collided; adjust and try again:
		NewLocation += Adjustment;
		};
	return Moved;
	UNGUARD("IModel::SphereNearMove");
	};

//
// Try to move a sphere by a movement vector, which is allowed to
// be larger than the sphere's radius.  This function works by
// subdividing the movement vector into intervals of size less than
// the radius, and calling IModel::SphereNearMove to do the world.
//
// Glides along walls.
// Returns 1 if full or partial movement occured, 0 if no movement.
//
int IModel::SphereMove (FVector *Location,const FVector *Delta, FLOAT Radius, int FavorSmoothness) const
	{
	GUARD;
	//
	if (NumBspNodes==0) {*Location += *Delta; return 1;};
	int Steps = (int)(Delta->Size() * 4.00 / Radius);
	//
	if (Steps<1) Steps = 1;
	FVector NewDelta = *Delta/(FLOAT)Steps;
	//
	int Moved = 0;
	while (Steps-- > 0) Moved |= SphereNearMove (Location,&NewDelta,Radius,FavorSmoothness);
	///
	return Moved;
	UNGUARD("IModel::SphereMove");
	};

/*---------------------------------------------------------------------------------------
   Sphere plane filtering
---------------------------------------------------------------------------------------*/

//
// Recursive minion of IModel::PlaneFilter
//
void PlaneFilter (IModel *ModelInfo,INDEX iNode, const FVector *Location,FLOAT Radius, IModel::PLANE_FILTER_CALLBACK Callback, DWORD SkipNodeFlags, int Param)
	{
	FLOAT Dist;
	do  {
		FBspNode *Node	= &ModelInfo->BspNodes [iNode];
		if (Node->NodeFlags & SkipNodeFlags) return;
		//
		FBspSurf *Surf	= &ModelInfo->BspSurfs [Node->iSurf];
		Dist			= FPointPlaneDist(*Location,ModelInfo->FPoints[Surf->pBase],ModelInfo->FVectors[Surf->vNormal]);
		//
		if (Dist >= -Radius)
			{
			if (Node->iFront!=INDEX_NONE) // Recurse with front
				{
				PlaneFilter (ModelInfo,Node->iFront,Location,Radius,Callback,SkipNodeFlags,Param); 
				};
			if (Dist <= Radius) // Within bounds: Do callback
				{
				Callback (ModelInfo,iNode,Param);
				};
			};
		iNode = Node->iBack;
		} while ((Dist <= Radius) && (iNode != INDEX_NONE));
	};

//
// Filter a sphere through the Bsp and call the specified callback for each Bsp node whose
// plane is touching the specified sphere.
//
// This only calls the callback for the first node in each coplanar chain; if the
// callback cares about coplanars, it must iterate through them itself.
//
void IModel::PlaneFilter(const FVector *Location,FLOAT Radius, PLANE_FILTER_CALLBACK Callback, DWORD SkipNodeFlags,int Param)
	{
	GUARD;
	if (NumBspNodes>0) ::PlaneFilter(this,0,Location,Radius,Callback,SkipNodeFlags,Param);
	UNGUARD("IModel::PlaneFilter");
	};

/*---------------------------------------------------------------------------------------
   Sphere leaf filtering
---------------------------------------------------------------------------------------*/

//
// Recursive minion of IModel::SphereLeafFilter
//
void SphereLeafFilter (IModel *ModelInfo,INDEX iNode, const FVector *Location,FLOAT Radius, 
	IModel::SPHERE_FILTER_CALLBACK Callback, int Outside, DWORD SkipNodeFlags,int Param)
	{
	while (1)
		{
		FBspNode *Node	= &ModelInfo->BspNodes [iNode];
		FBspSurf *Surf	= &ModelInfo->BspSurfs [Node->iSurf];
		FLOAT    Dist	= FPointPlaneDist(*Location,ModelInfo->FPoints[Surf->pBase],ModelInfo->FVectors[Surf->vNormal]);
		//
		if (Dist >= -Radius)
			{
			if (Node->iFront != INDEX_NONE) // Recurse with front
				{
				::SphereLeafFilter(ModelInfo,Node->iFront,Location,Radius,Callback,Outside || Node->IsCsg(),SkipNodeFlags,Param);
				}
			else // Call callback with front
				{
				Callback(ModelInfo,iNode,0,Outside || Node->IsCsg(), Param);
				};
			};
		if (Dist>Radius) break;
		//
		Outside = Outside && !Node->IsCsg();
		if (Node->iBack == INDEX_NONE) // Call callback with back
			{
			Callback(ModelInfo,iNode,1,Outside,Param);
			break;
			};
		iNode = Node->iBack;
		};
	};

//
// Filter a sphere through the Bsp and call the specified callback for each Bsp leaf that
// a part of the sphere falls in.  This uses inexact radius-based comparison, which
// can treat the sphere as in a wedge leaf that it's not actually in.
//
void IModel::SphereLeafFilter(const FVector *Location,FLOAT Radius,SPHERE_FILTER_CALLBACK Callback, DWORD SkipNodeFlags,int Param)
	{
	GUARD;
	if (NumBspNodes>0) ::SphereLeafFilter(this,0,Location,Radius,Callback,1,SkipNodeFlags,Param);
	UNGUARD("IModel::SphereLeafFilter");
	};

/*---------------------------------------------------------------------------------------
   Sound propagation
---------------------------------------------------------------------------------------*/

//
// Compute the amount of sound propagation between two points, from 0.0 (silence) to
// 1.0 (maximum volume).  This will be expanded to consider level geometry and
// occlusion, but right now it just returns 1.0.
//
FLOAT IModel::SoundDamping (const FVector *Listener, const FVector *Emitter) const
	{
	GUARD;
	return 1.0;
	UNGUARD("IModel::SoundDamping");
	};

/*---------------------------------------------------------------------------------------
   Point searching
---------------------------------------------------------------------------------------*/

//
// Find closest vertex to a point at or below a node in the Bsp.  If no vertices
// are closer than MinRadius, returns -1.
//
FLOAT FindNearestVertex (const IModel *ModelInfo, const FVector *SourcePoint,
	FVector *DestPoint, FLOAT MinRadius, INDEX iNode, INDEX *pVertex)
	{
	FLOAT ResultRadius = -1.0;
	while (iNode != INDEX_NONE)
		{
		FBspNode	*Node	= &ModelInfo->BspNodes [iNode];
		FBspSurf	*Surf	= &ModelInfo->BspSurfs [Node->iSurf];
		INDEX		iBack   = Node->iBack;
		//
		FLOAT PlaneDist = FPointPlaneDist
			(
			*SourcePoint,
			ModelInfo->FPoints [Surf->pBase],
			ModelInfo->FVectors[Surf->vNormal]
			);
		if ((PlaneDist >= -MinRadius) && (Node->iFront!=INDEX_NONE)) // Check front
			{
			FLOAT TempRadius = FindNearestVertex (ModelInfo,SourcePoint,DestPoint,MinRadius,Node->iFront,pVertex);
			if (TempRadius >= 0.0) {ResultRadius = TempRadius; MinRadius = TempRadius;};
			};
		if ((PlaneDist > -MinRadius) && (PlaneDist <= MinRadius)) // Check this node's poly's vertices
			{
			while (iNode != INDEX_NONE) // Loop through all coplanars
				{
				Node				= &ModelInfo->BspNodes [iNode];
				Surf				= &ModelInfo->BspSurfs [Node->iSurf];
				//
				FVector *Base		= &ModelInfo->FPoints[Surf->pBase];
				FLOAT   TempRadius	= FDistApprox (*SourcePoint,*Base);
				//
				if (TempRadius < MinRadius)
					{
					*pVertex     = Surf->pBase;
					ResultRadius = TempRadius;
					MinRadius    = TempRadius;
					*DestPoint   =	*Base;
					};
				FVertPool *VertPool = &ModelInfo->VertPool[Node->iVertPool];
				for (BYTE B=0; B<Node->NumVertices; B++)
					{
					FVector *Vertex		= &ModelInfo->FPoints [VertPool->pVertex];
					FLOAT   TempRadius	= FDistApprox (*SourcePoint,*Vertex);
					if (TempRadius < MinRadius)
						{
						*pVertex     = VertPool->pVertex;
						ResultRadius = TempRadius;
						MinRadius    = TempRadius;
						*DestPoint   =	*Vertex;
						};
					VertPool++;
					};
				iNode = Node->iPlane;
				};
			};
		if (PlaneDist > MinRadius) break; // Don't go down back
		iNode = iBack;
		};
	return ResultRadius;
	};

//
// Find Bsp node vertex nearest to a point (within a certain radius) and
// set the location.  Returns distance, or -1.0 if no point was found.
//
FLOAT IModel::FindNearestVertex (const FVector *SourcePoint,FVector *DestPoint,
	FLOAT MinRadius, INDEX *pVertex) const
	{
	GUARD;
	//
	if (NumBspNodes>0) return ::FindNearestVertex (this,SourcePoint,DestPoint,MinRadius,0,pVertex);
	else return -1.0;
	//
	UNGUARD("IModel::FindNearestVertex");
	};

/*---------------------------------------------------------------------------------------
   Bound filter precompute
---------------------------------------------------------------------------------------*/

//
// Recursive worker function for IModel::PrecomputeSphereFilter
//
void PrecomputeFilter(IModel *ModelInfo,INDEX iNode,FVector *Sphere)
	{
	FBspNode *Node;
	FBspSurf *Surf;
	//
	while (iNode!=INDEX_NONE)
		{
		Node = &ModelInfo->BspNodes[iNode];
		Surf = &ModelInfo->BspSurfs[Node->iSurf];
		//
		FLOAT Dist = FPointPlaneDist
			(
			*Sphere,
			ModelInfo->FPoints  [Surf->pBase],
			ModelInfo->FVectors [Surf->vNormal]
			);
		if (Dist < -Sphere->W) // All back
			{
			Surf->PolyFlags = (Surf->PolyFlags & ~PF_IsFront) | PF_IsBack;
			iNode = Node->iBack;
			}
		else if (Dist > Sphere->W) // All front
			{
			Surf->PolyFlags = (Surf->PolyFlags & ~PF_IsBack) | PF_IsFront;
			iNode = Node->iFront;
			}
		else // Both front and back
			{
			Surf->PolyFlags &= ~(PF_IsFront | PF_IsBack);
			//
			if (Node->iBack!=INDEX_NONE) PrecomputeFilter(ModelInfo,Node->iBack,Sphere);
			iNode = Node->iFront;
			};
		};
	};

//
// Precompute the front/back test for a bounding sphere.  Tags all nodes that
// the sphere falls into with a PF_IsBack tag (if the sphere is entirely in back
// of the node), a PF_IsFront tag (if the sphere is entirely in front of the node),
// or neither (if the sphere is split by the node).  This only affects nodes
// that the sphere falls in.  Thus, it is not necessary to perform any cleanup
// after precomputing the filter as long as you're sure the sphere completely
// encloses the object whose filter you're precomputing.
//
void IModel::PrecomputeSphereFilter(FVector *Sphere)
	{
	if (NumBspNodes>0) PrecomputeFilter(this,0,Sphere);
	};

/*---------------------------------------------------------------------------------------
   The End
---------------------------------------------------------------------------------------*/
