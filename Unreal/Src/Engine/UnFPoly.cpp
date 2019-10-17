/*=============================================================================
	UnFPoly.cpp: FPoly implementation (Editor polygons)

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "Unreal.h"

/*---------------------------------------------------------------------------------------
	FPoly class implementation
---------------------------------------------------------------------------------------*/

//
// Initialize everything in an  editor polygon structure to defaults
//
void FPoly::Init (void)
	{
	GUARD;
	Base.X       	= MAXSWORD; // No origin
	Normal.X        = MAXSWORD; // No normal
	TextureU		= GMath.ZeroVector;
	TextureV		= GMath.ZeroVector;
	PanU			= 0;
	PanV			= 0;
	NumVertices     = 0;
	PolyFlags       = 0;
	iLink           = INDEX_NONE;
	Brush			= NULL;
	Texture         = NULL;
	GroupName       = NAME_NONE;
	ItemName        = NAME_NONE;
	UNGUARD("FPoly::Init");
	};

//
// Reverse an FPoly by revesing the normal and reversing the order of its
// vertices.
//
void FPoly::Flip (void)
	{
	GUARD;
	FVector Temp;
	int i,c;
	//
	Normal *= -1;
	//
	c=NumVertices/2;
	for (i=0; i<c; i++) // Flip all points except middle if odd number of points.
		{
		Temp      = Vertex[i];
		Vertex[i] = Vertex[(NumVertices-1)-i];
		Vertex[(NumVertices-1)-i] = Temp;
		};
	UNGUARD("FPoly::Flip");
	};

//
// Fix up an editor poly by deleting vertices that are identical.  Sets
// vertex count to zero if it collapses.  Returns number of vertices, 0 or >=3.
//
int FPoly::Fix (void)
	{
	GUARD;
	int i,j,prev;
	//
	j=0; prev=NumVertices-1;
	for (i=0; i<NumVertices; i++)
		{
		if (!FPointsAreSame(Vertex[i],Vertex[prev]))
			{
			if (j!=i) Vertex[j] = Vertex[i]; // j<i
			prev = j;
			j    ++;
			}
		else debug(LOG_Bsp,"FPoly::Fix: Collapsed a point");
		};
	if (j>=3) NumVertices = j;
	else      NumVertices = 0;
	return NumVertices;
	UNGUARD("FPoly::Fix");
	};

FLOAT FPoly::Area(void)
	{
	GUARD;
	FVector Side1,Side2;
	FLOAT Area;
	int i;
	//
	Area  = 0.0;
	Side1 = Vertex[1] - Vertex[0];
	for (i=2; i<NumVertices; i++)
		{
		Side2 = Vertex[i] - Vertex[0];
		Area += (Side1 ^ Side2).Size();
		Side1 = Side2;
		};
	return Area;
	UNGUARD("FPoly::Area");
	};

//
// Split with plane. Meant to be numerically stable.
//
int FPoly::SplitWithPlane (
	const FVector	&PlaneBase,
	const FVector	&PlaneNormal,
	FPoly			*FrontPoly,
	FPoly			*BackPoly,
	int				VeryPrecise)
	{
	GUARD;
	FVector 	Intersection;
	FLOAT   	Dist,MaxDist=0,MinDist=0,PrevDist,Thresh;
	enum 	  	{V_FRONT,V_BACK,V_EITHER} Status,PrevStatus;
	int     	i,j;
	//
	if (VeryPrecise)	Thresh = THRESH_SPLIT_POLY_PRECISELY;	
	else				Thresh = THRESH_SPLIT_POLY_WITH_PLANE;
	//
	// Find number of vertices:
	//
	if      (NumVertices<3)                  appErrorf("FPoly::SplitWithPlane: NumVertices underflow %i",NumVertices);
	else if (NumVertices>MAX_FPOLY_VERTICES) appErrorf("FPoly::SplitWithPlane: Vertex overflow %i",NumVertices);
	//
	// See if the polygon is split by SplitPoly, or it's on either side, or the
	// polys are coplanar.  Go through all of the polygon points and
	// calculate the minimum and maximum signed distance (in the direction
	// of the normal) from each point to the plane of SplitPoly.
	//
	for (i=0; i<NumVertices; i++)
		{
		Dist = FPointPlaneDist(Vertex[i],PlaneBase,PlaneNormal);
		//
		if ((i==0)||(Dist>MaxDist)) MaxDist=Dist;
		if ((i==0)||(Dist<MinDist)) MinDist=Dist;
		//
		if      (Dist > +Thresh) PrevStatus = V_FRONT;
		else if (Dist < -Thresh) PrevStatus = V_BACK;
		};
	if ((MaxDist < Thresh) && (MinDist > -Thresh))
		{
		return SP_Coplanar;
		}
	else if (MaxDist<Thresh)
		{
		return SP_Back;
		}
	else if (MinDist > -Thresh)
		{
		return SP_Front;
		}
	else // Split
		{
		if (FrontPoly==NULL) return SP_Split; // Caller only wanted status
		if (NumVertices >= MAX_FPOLY_VERTICES) appError ("FPoly::SplitWithPlane: Vertex overflow");
		//
		*FrontPoly = *this;                 // Copy all info
		FrontPoly->PolyFlags  |= PF_EdCut; // Mark as cut
		FrontPoly->NumVertices =  0;
		//
		*BackPoly = *this;                  // Copy all info
		BackPoly->PolyFlags   |= PF_EdCut; // Mark as cut
		BackPoly->NumVertices  = 0;
		//
		j = NumVertices-1;      // Previous vertex; have PrevStatus already
		//
		for (i=0; i<NumVertices; i++)
			{
			PrevDist	= Dist;
      		Dist		= FPointPlaneDist(Vertex[i],PlaneBase,PlaneNormal);
			//
			if      (Dist > +Thresh)  	Status = V_FRONT;
			else if (Dist < -Thresh)  	Status = V_BACK;
			else						Status = PrevStatus;
			//
			if (Status != PrevStatus)
	            {
				//
				// Crossing.  Either Front-to-Back or Back-To-Front.
				// Intersection point is naturally on both front and back polys.
				//
				if ((Dist >= -Thresh) && (Dist < +Thresh))
					{
					//
					// This point lies on plane
					//
					if (PrevStatus == V_FRONT)
						{
						FrontPoly->Vertex[FrontPoly->NumVertices++]       = Vertex[i];
						BackPoly ->Vertex[BackPoly ->NumVertices  ]       = Vertex[i];
						BackPoly ->Vertex[BackPoly ->NumVertices++].Flags = 0;
						}
					else
						{
						BackPoly ->Vertex[BackPoly ->NumVertices++]       = Vertex[i];
						FrontPoly->Vertex[FrontPoly->NumVertices  ]       = Vertex[i];
						FrontPoly->Vertex[FrontPoly->NumVertices++].Flags = 0;
						};
					}
				else if ((PrevDist >= -Thresh) && (PrevDist < +Thresh))
					{
					//
					// Previous point lies on plane
					//
					if (Status == V_FRONT)
						{
						FrontPoly->Vertex[FrontPoly->NumVertices  ]       = Vertex[j];
						FrontPoly->Vertex[FrontPoly->NumVertices++].Flags = 0;
						FrontPoly->Vertex[FrontPoly->NumVertices++]       = Vertex[i];
						}
					else
						{
						BackPoly ->Vertex[BackPoly ->NumVertices  ]       = Vertex[j];
						BackPoly ->Vertex[BackPoly ->NumVertices++].Flags = 0;
						BackPoly ->Vertex[BackPoly ->NumVertices++]       = Vertex[i];
						};
					}
				else
					{
					//
					// Intersection point is in between
					//
					Intersection = FLinePlaneIntersection(Vertex[j],Vertex[i],PlaneBase,PlaneNormal);
					//
					if (PrevStatus == V_FRONT)
						{
						FrontPoly->Vertex[FrontPoly->NumVertices  ]       = Intersection;
						FrontPoly->Vertex[FrontPoly->NumVertices++].Flags = Vertex[i].Flags;
						BackPoly ->Vertex[BackPoly ->NumVertices  ]       = Intersection;
						BackPoly ->Vertex[BackPoly ->NumVertices++].Flags = 0;
						BackPoly ->Vertex[BackPoly ->NumVertices++]		  = Vertex[i];
						}
					else
						{
						BackPoly ->Vertex[BackPoly ->NumVertices  ]       = Intersection;
						BackPoly ->Vertex[BackPoly ->NumVertices++].Flags = Vertex[i].Flags;
						FrontPoly->Vertex[FrontPoly->NumVertices  ]       = Intersection;
						FrontPoly->Vertex[FrontPoly->NumVertices++].Flags = 0;
						FrontPoly->Vertex[FrontPoly->NumVertices++]		  = Vertex[i];
						};
					};
				}
			else
				{
        		if (Status==V_FRONT) FrontPoly->Vertex[FrontPoly->NumVertices++] = Vertex[i];
        		else                 BackPoly ->Vertex[BackPoly ->NumVertices++] = Vertex[i];
				};
			j          = i;
			PrevStatus = Status;
			};
		//
		// Handle possibility of sliver polys due to precision errors:
		//
		if (FrontPoly->Fix()<3)
			{
			debug (LOG_Bsp,"FPoly::SplitWithPlane: Ignored front sliver");
			return SP_Back;
			}
		else if (BackPoly->Fix()<3)
	        {
			debug (LOG_Bsp,"FPoly::SplitWithPlane: Ignored back sliver");
			return SP_Front;
			}
		else return SP_Split;
		};
	UNGUARD("FPoly::SplitWithPlane");
	};

//
// Split with plane quickly for in-game geometry operations.
// Results are always valid. May return sliver polys.
//
int FPoly::SplitWithPlaneFast(
	const FVector	&PlaneBase,
	const FVector	&PlaneNormal,
	FPoly			*FrontPoly,
	FPoly			*BackPoly)
	{
	GUARD;
	enum {V_FRONT=0,V_BACK=1} Status,PrevStatus,VertStatus[MAX_FPOLY_VERTICES],*StatusPtr;
	int Front=0,Back=0;
	//
	StatusPtr = &VertStatus[0];
	for (int i=0; i<NumVertices; i++)
		{
		FLOAT Dist = FPointPlaneDist(Vertex[i],PlaneBase,PlaneNormal);
		//
		if (Dist>=0.0)
			{
			*StatusPtr++ = V_FRONT;
			if (Dist > +THRESH_SPLIT_POLY_WITH_PLANE) Front=1;
			}
		else
			{
			*StatusPtr++ = V_BACK;
			if (Dist < -THRESH_SPLIT_POLY_WITH_PLANE) Back=1;
			};
		};
	if (!Front)
		{
		if (Back) return SP_Back;
		return SP_Coplanar;
		};
	if (!Back) return SP_Front;
	//
	// Handle split case:
	//
	FVector *V  = &Vertex            [0];
	FVector *W  = &Vertex            [NumVertices-1];
	FVector *V1 = &FrontPoly->Vertex [0];
	FVector *V2 = &BackPoly ->Vertex [0];
	PrevStatus  = VertStatus         [NumVertices-1];
	StatusPtr   = &VertStatus        [0];
	//
	int N1=0, N2=0;
	for (int i=0; i<NumVertices; i++)
		{
		Status = *StatusPtr++;
		//
		if (Status != PrevStatus) // Crossing
	        {
			*V1++ = *V2++ = FLinePlaneIntersection(*W,*V,PlaneBase,PlaneNormal);
			if (PrevStatus == V_FRONT)	{*V2++ = *V; N1++; N2+=2;}
			else						{*V1++ = *V; N2++; N1+=2;};
			}
		else if (Status==V_FRONT) {*V1++ = *V; N1++;}
        else                      {*V2++ = *V; N2++;};
		//
		PrevStatus = Status;
		W          = V++;
		};
	FrontPoly->NumVertices = N1;
	BackPoly ->NumVertices = N2;
	return SP_Split;
	//
	UNGUARD("FPoly::SplitWithPlaneFast");
	};

//
// Split an FPoly in half
//
void FPoly::SplitInHalf (FPoly *OtherHalf)
	{
	GUARD;
	int m = NumVertices/2;
	int i;
	//
	if ((NumVertices<=3) || (NumVertices>MAX_FPOLY_VERTICES)) appErrorf ("FPoly::SplitInHalf: %i Vertices",NumVertices);
	//
	*OtherHalf = *this;
	//
	OtherHalf->NumVertices = (NumVertices-m) + 1;
	NumVertices            = (m            ) + 1;
	//
	for (i=0; i<(OtherHalf->NumVertices-1); i++)
		{
		OtherHalf->Vertex[i] = Vertex[i+m];
		};
	OtherHalf->Vertex[OtherHalf->NumVertices-1] = Vertex[0];
	//
	PolyFlags            |= PF_EdCut;
	OtherHalf->PolyFlags |= PF_EdCut;
	UNGUARD("FPoly::SplitInHalf");
	};

//
// Compute normal of an FPoly.  Works even if FPoly has 180-degree-angled sides (which
// are often created during T-joint elimination).  Returns nonzero result (plus sets
// normal vector to zero) if a problem occurs.
//
int FPoly::CalcNormal (void)
	{
	GUARD;
	FVector 	Side1,Side2;
	FLOAT		SizeSquared;
	int			i;
	//
	i		= 2;
	Normal	= GMath.ZeroVector;
	Side1   = Vertex[1] - Vertex[0];
	//
	while (i < NumVertices)
		{
		Side2   = Vertex[i] - Vertex[0];
		Normal += Side1 ^ Side2;
		//
		SizeSquared = Normal.SizeSquared();
		if (SizeSquared >= 10000.0*(FLOAT)THRESH_ZERO_NORM_SQUARED) // Sufficient precision to normalize
			{
			Normal *= 1.0 / sqrt(SizeSquared);
			return 0; // Successful
			};
		Side1 = Side2;
		i     ++;
		};
	SizeSquared = Normal.SizeSquared();
	if (SizeSquared >= (FLOAT)THRESH_ZERO_NORM_SQUARED) // Sufficient precision to normalize
		{
		Normal *= 1.0 / sqrt(SizeSquared);
		return 0; // Successful
		};
	debug (LOG_Bsp,"FPoly::CalcNormal: Zero-area polygon");
	return 1;
	UNGUARD("FPoly::CalcNormal");
	};

//
// Transform an editor polygon with a coordinate system, a pre-transformation
// addition, and a post-transformation addition:
//
void FPoly::Transform (FModelCoords &Coords, FVector *PreSubtract,FVector *PostAdd, FLOAT Orientation)
	{
	GUARD;
	FVector 	Temp;
	int 		i,m;
	//
	if (TextureU.X!=MAXSWORD) TextureU.TransformVector(Coords.VectorXform);
	if (TextureV.X!=MAXSWORD) TextureV.TransformVector(Coords.VectorXform);
	//
	if (Base.X!=MAXSWORD)
		{
		if (PreSubtract) Base -= *PreSubtract;
		Base.TransformVector (Coords.PointXform);
		if (PostAdd) Base     += *PostAdd;
		};
	for (i=0; i<NumVertices; i++)
		{
		if (PreSubtract) Vertex[i] -= *PreSubtract;
		Vertex[i].TransformVector (Coords.PointXform);
		if (PostAdd) Vertex[i]     += *PostAdd;
		};
	//
	// Flip vertex order if orientation is negative:
	//
	if (Orientation < 0.0)
		{
		m = NumVertices/2;
		for (i=0; i<m; i++)
			{
			Temp 					  = Vertex[i];
			Vertex[i] 		          = Vertex[(NumVertices-1)-i];
			Vertex[(NumVertices-1)-i] = Temp;
			};
		};
	//
	// Transform normal.  Since the transformation coordinate system is
	// orthogonal but not orthonormal, it has to be renormalized here.
	//
	Normal.TransformVector (Coords.VectorXform);
	if (!Normal.Normalize()) appError ("FPoly::Transform: Invalid normal");
	UNGUARD("FPoly::Transform");
	};

//
// Remove colinear vertices and check convexity.  Returns 1 if convex, 0 if
// nonconvex or collapsed.
//
int FPoly::RemoveColinears(void)
	{
	FVector  SidePlaneNormal[MAX_FPOLY_VERTICES];
	FVector  Side;
	int      i,j;
	//
	GUARD;
	for (i=0; i<NumVertices; i++)
		{
		j=i-1; if (j<0) j=NumVertices-1;
		//
		// Create cutting plane perpendicular to both this side and the polygon's normal:
		//
		Side = Vertex[i] - Vertex[j];
		SidePlaneNormal[i] = Side ^ Normal;
		//
		if (!SidePlaneNormal[i].Normalize())
			{
			//
			// Eliminate these nearly identical points:
			//
			memcpy (&Vertex[i],&Vertex[i+1],(NumVertices-(i+1)) * sizeof (FVector));
			if (--NumVertices<3) {NumVertices = 0; return 0;}; // Collapsed
			i--;
			};
		};
	for (i=0; i<NumVertices; i++)
		{
		j=i+1; if (j>=NumVertices) j=0;
		//
		if (FPointsAreNear(SidePlaneNormal[i],SidePlaneNormal[j],FLOAT_NORMAL_THRESH))
	        {
			//
			// Eliminate colinear points:
			//
			memcpy (&Vertex[i],&Vertex[i+1],(NumVertices-(i+1)) * sizeof (FVector));
			memcpy (&SidePlaneNormal[i],&SidePlaneNormal[i+1],(NumVertices-(i+1)) * sizeof (FVector));
			if (--NumVertices<3) {NumVertices = 0; return 0;}; // Collapsed
			i--;
			}
		else
			{
			for (j=0; j<NumVertices; j++)
	            {
				if (j != i)
					{
					switch (SplitWithPlane (Vertex[i],SidePlaneNormal[i],NULL,NULL,0))
						{
						case SP_Front: return 0; // Nonconvex + Numerical precision error
						case SP_Split: return 0; // Nonconvex
						// SP_BACK: Means it's convex
						// SP_COPLANAR: Means it's probably convex (numerical precision)
						};
					};
				};
			};
		};
	return 1; // Ok
	UNGUARD("FPoly::RemoveColinears");
	};

//
// Compute all remaining polygon parameters (normal, etc) that are blank.
// Returns 0 if ok, nonzero if problem.
//
int FPoly::Finalize (int NoError)
	{
	GUARD;
	FVector Temp;
	//
	// Check for problems:
	//
	if (NumVertices<3)
		{
		debugf(LOG_Ed,"FPoly::Finalize: Not enough vertices (%i)",NumVertices);
		if (NoError) return -1;
		else appErrorf("FPoly::Finalize: Not enough vertices (%i)",NumVertices);
		};
	//
	// If no origin, set origin to first point.
	//
	if ((Base.X==MAXSWORD)&&(NumVertices>0))
		{
		Base = Vertex[0];
		};
	//
	// If no normal, compute from cross-product and normalize it:
	//
	if ((Normal.X==MAXSWORD)&&(NumVertices>=3))
		{
		if (CalcNormal())
			{
			debugf(LOG_Ed,"FPoly::Finalize: Normalization failed, verts=%i, size=%f",NumVertices,Normal.Size());
			if (NoError) return -1;
			else appErrorf("FPoly::Finalize: Normalization failed, verts=%i, size=%f",NumVertices,Normal.Size());
			};
		};
	//
	// If texture U and V coordinates weren't specified, fudge them:
	//
	if (TextureU.IsZero() && TextureV.IsZero())
		{
		Temp = Vertex[0] - Vertex[1];
		TextureU = Temp ^ Normal;
		TextureU.Normalize(); // Ignore normalization errors
		//
		TextureV = TextureU ^ Normal;
		TextureV.Normalize(); // Ignore normalization errors
		//
		// Default scaling:
		// V = upside down (for correct mapping onto default rectangle)
		//
		TextureU *= +65536.0;
		TextureV *= -65536.0;
		};
	return 0;
	UNGUARD("FPoly::Finalize");
	};

/*---------------------------------------------------------------------------------------
	FPolys Resource Implementation
---------------------------------------------------------------------------------------*/

void UPolys::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UPolys);
	Type->RecordSize = sizeof (FPoly);
	Type->Version    = 1;
	strcpy (Type->Descr,"Polys");
	UNGUARD("UPolys::Register");
	};
const char *UPolys::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	int New = (Max == 0);
	//
	if (New)
		{
		Num = 0;
		Max = CountFPolys(Buffer);
		Realloc();
		};
	Num = ParseFPolys(&Buffer,0,0); // Note: Changes Buffer
	return Buffer;
	UNGUARD("UPolys::Import");
	};
char *UPolys::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	FPoly	*Poly;
	char	TempStr[256];
	int		i,j;
	//
	Poly    = GetData();
	Buffer += sprintf (Buffer,"%sBegin PolyList Num=%i Max=%i\r\n",spc(Indent),Num,Max);
	//
	for (i=0; i<Num; i++)
		{
		//
		// Start of polygon plus group/item name if applicable:
		//
		Buffer += sprintf (Buffer,"%s   Begin Polygon",spc(Indent));
		if ((!Poly->GroupName.IsNone()) || (!Poly->ItemName.IsNone()))
			{
			if (!Poly->GroupName.IsNone()) Buffer += sprintf (Buffer," Group=%s",Poly->GroupName.Name());
			if (!Poly->ItemName.IsNone())  Buffer += sprintf (Buffer," Item=%s",Poly->ItemName.Name());
			};
		if (Poly->Texture)
			{
			Buffer += sprintf (Buffer," Texture=%s",Poly->Texture->Name);
			};
		if (Poly->PolyFlags != 0)
			{
			Buffer += sprintf (Buffer," Flags=%i",Poly->PolyFlags);
			};
		if (Poly->iLink != INDEX_NONE)
			{
			Buffer += sprintf (Buffer," Link=%i",Poly->iLink);
			};
		Buffer += sprintf (Buffer,"\r\n");
		//
		// All coordinates:
		//
		Buffer += sprintf (Buffer,"%s      Origin   %s\r\n",spc(Indent),SetFVECTOR(TempStr,&Poly->Base));
		Buffer += sprintf (Buffer,"%s      Normal   %s\r\n",spc(Indent),SetFVECTOR(TempStr,&Poly->Normal));
		//
		if ((Poly->PanU!=0)||(Poly->PanV!=0))
			{
			Buffer += sprintf (Buffer,"%s      Pan      U=%i V=%i\r\n",spc(Indent),Poly->PanU,Poly->PanV);
			};
		if ((Poly->TextureU.X != MAXWORD) && (Poly->TextureV.X != MAXWORD))
			{
			Buffer += sprintf (Buffer,"%s      TextureU %s\r\n",spc(Indent),SetFIXFVECTOR(TempStr,&Poly->TextureU));
			Buffer += sprintf (Buffer,"%s      TextureV %s\r\n",spc(Indent),SetFIXFVECTOR(TempStr,&Poly->TextureV));
			};
		for (j=0; j<Poly->NumVertices; j++)
			{
			Buffer += sprintf (Buffer,"%s      Vertex   %s\r\n",spc(Indent),SetFVECTOR(TempStr,&Poly->Vertex[j]));
			};
		Poly++;
		Buffer += sprintf (Buffer,"%s   End Polygon\r\n",spc(Indent));
		};
	Buffer += sprintf (Buffer,"%sEnd PolyList\r\n",spc(Indent));
	//
	return Buffer;
	UNGUARD("UPolys::Export");
	};
void UPolys::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	for (INDEX i=0; i<Num; i++)
		{
		Callback.Resource	(this,(UResource **)&Element(i).Texture,0);
		Callback.Resource	(this,(UResource **)&Element(i).Brush,  0);
		Callback.Name		(this,				&Element(i).GroupName,0);
		Callback.Name		(this,				&Element(i).ItemName, 0);
		};
	UNGUARD("UPolys::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_Polys,UPolys,0xB2D90855,0xCCD211cf,0x91360000,0xC028B992);

/*---------------------------------------------------------------------------------------
	UPoly custom functions
---------------------------------------------------------------------------------------*/

//
// Parse a list of polygons.  Returns the number parsed.
//
int UPolys::ParseFPolys(const char **Stream,int More, int CmdLine)
	{
	GUARD;
	//
	FPoly			*FPolys		= GetData();
	FPoly 			*Poly;
	FVector			*PointPool;
	const char 		*Str;
	char 			StrLine [256],VertText[256],FaceText[256];
	int 			n,i,First,TempVerts=0,NumVerts=0,DXFStart=0,DXFTag,TempNumPolys;
	//
	GetBEGIN (Stream,"POLYLIST"); // Eat up if present
	//
	if (More)
		{
		n    	= Num-1;
		Poly	= &FPolys[Num];
		}
	else
		{
		Num		= 0;
		Poly 	= &FPolys[0];
		n		= -1;
		};
	First=1;
	while (1)
		{
		if (GetLINE (Stream,StrLine,256)!=0)	break;
		if (CmdLine && GEditor) 				GEditor->NoteMacroCommand (StrLine);
		if (strlen(StrLine)==0) 				continue;
		//
		Str=StrLine;
		if (GetEND(&Str,"POLYLIST")) break; // End of brush polys
		else if (strstr(Str,"ENTITIES") && First) DXFStart=1; // AutoCad .DXF file
		else if (strstr(Str,"VERTEX") && First && DXFStart)
			{
			for (n=0; n<Max; n++) FPolys[n].Init();
			//
			debug(LOG_Info,"Reading DXF file with vertices");
			n=0;
			while (GetLINE(Stream,StrLine,256)==0)
				{
				Str=StrLine; while (*Str==' ') Str++; DXFTag=atoi(Str);
				GetLINE(Stream,StrLine,256); strupr(StrLine);
				Str=StrLine; while (*Str==' ') Str++;
				//
				Poly = &FPolys[n];
				//
				if ((DXFTag==0) && (strcmp(Str,"EOF")==0))
					{
					break;
					}
				else if	((DXFTag==8) && strstr (Str,"FACE"))
					{
					n=atoi(Str+4)-1;
					Num = OurMax(Num,n+1);
					if (!(Num&63)) GApp->StatusUpdate ("Importing DXF",Num,Max);
					}
				else if	(DXFTag==10)
					{
					Poly->Vertex[Poly->NumVertices]=GMath.ZeroVector;
					Poly->Vertex[Poly->NumVertices].X=1.0*atof(Str);
					NumVerts++;
					}
				else if	(DXFTag==20)
					{
					Poly->Vertex[Poly->NumVertices].Y=1.0*atof(Str);
					}
				else if	(DXFTag==30)
					{
					Poly->Vertex[Poly->NumVertices].Z=1.0*atof(Str);
					if (!Poly->Vertex[Poly->NumVertices].IsZero()) Poly->NumVertices++;
					};
				};
			for (n=0; n<Num; n++) FPolys[n].Finalize(0);
			debugf(LOG_Info,"Imported %i vertices, %i faces",NumVerts,Num);
			return Num;
			}
		else if (strstr(Str,"3DFACE") && First && DXFStart)
			{
			debug(LOG_Info,"Reading DXF file with 3D faces");
			for (n=0; n<Max; n++) FPolys[n].Init();
			while (GetLINE(Stream,StrLine,256)==0)
				{
				Str=StrLine; while (*Str==' ') Str++; DXFTag=atoi(Str);
				GetLINE(Stream,StrLine,256); strupr(StrLine);
				Str=StrLine; while (*Str==' ') Str++;
				//
				Poly = &FPolys[Num];
				//
				if ((DXFTag==0) && (strcmp(Str,"EOF")==0))
					{
					break;
					}
				else if	((DXFTag==0) && (strcmp(Str,"3DFACE")==0))
					{
					if (!(Num&127)) GApp->StatusUpdate ("Importing DXF",Num,Max);
					if (Poly->Finalize(1)==0)
						{
						Poly->Flip();
						Num++;
						};
					}
				else if	((DXFTag>=10)&&(DXFTag<=19))
					{
					NumVerts++;
					Poly->Vertex[DXFTag-10]=GMath.ZeroVector;
					Poly->Vertex[DXFTag-10].X=atof(Str);
					if (Poly->NumVertices < (1+DXFTag-10)) Poly->NumVertices = 1+DXFTag-10;
					}
				else if	((DXFTag>=20)&&(DXFTag<=29)) Poly->Vertex[DXFTag-20].Y=atof(Str);
				else if	((DXFTag>=30)&&(DXFTag<=39)) Poly->Vertex[DXFTag-30].Z=atof(Str);
				};
			Poly = &FPolys[Num];
			if (Poly->NumVertices!=0) if (Poly->Finalize(1)==0)
				{
				Poly->Flip();
				Num++;
				};
			//for (n=0; n<Num; n++) debugf(LOG_Info,"%i",FPolys[n].NumVertices);
			debugf(LOG_Info,"Imported %i vertices, %i faces",NumVerts,Num);
			return Num;
			}
		else if (strstr(Str,"Tri-mesh,") && First) // 3DS .ASC file
			{
			debug(LOG_Info,"Reading 3D Studio ASC file");
			PointPool = (FVector *)GMem.Get(20000 * sizeof (FVector));
			//
			AscReloop:
			TempNumPolys = 0;
			NumVerts     = 0;
			while (GetLINE(Stream,StrLine,256)==0)
				{
				Str=StrLine;
				sprintf(VertText,"Vertex %i:",NumVerts);
				sprintf(FaceText,"Face %i:",TempNumPolys);
				//
				if (strstr(Str,VertText))
					{
					PointPool[NumVerts].X = atof(strstr(Str,"X:")+2);
					PointPool[NumVerts].Y = atof(strstr(Str,"Y:")+2);
					PointPool[NumVerts].Z = atof(strstr(Str,"Z:")+2);
					NumVerts++;
					TempVerts++;
					}
				else if (strstr(Str,FaceText))
					{
					Poly = &FPolys[Num];
					Num++;
					TempNumPolys++;
					//
					Poly->Init();
					Poly->NumVertices=3;
					Poly->Vertex[0]=PointPool[atoi(strstr(Str,"A:")+2)];
					Poly->Vertex[1]=PointPool[atoi(strstr(Str,"B:")+2)];
					Poly->Vertex[2]=PointPool[atoi(strstr(Str,"C:")+2)];
					Poly->Finalize(0);
					}
				else if (strstr(Str,"Tri-mesh,")) goto AscReloop;
				};
			debugf(LOG_Info,"Imported %i vertices, %i faces",TempVerts,Num);
			GMem.Release(PointPool);
			return Num;
			}
		else if (GetBEGIN(&Str,"POLYGON")) // Unreal .T3D file
			{
			//
			// Start of new polygon
			//
			if (First) First=0;
			else if (Poly->Finalize(1)==0)	Poly++;
			else n--;
			//
			if (++n >= Max)
				{
				debug (LOG_Ed,"fpolyParse: Overflowed MaxFPolys!");
				Num = 0;
				return 0;
				};
			//
			// Init to defaults and get group/item and texture
			//
			Poly->Init();
			GetDWORD	(Str,"FLAGS=",&Poly->PolyFlags);
			GetINDEX	(Str,"LINK=", &Poly->iLink);
			GetNAME		(Str,"GROUP=",&Poly->GroupName);
			GetNAME		(Str,"ITEM=", &Poly->ItemName);
			GetUTexture	(Str,"TEXTURE=",&Poly->Texture);
			//
			Poly->PolyFlags &= ~PF_NoImport;
			}
		else if (GetCMD(&Str,"PAN"))
			{
			GetBYTE (Str,"U=",&Poly->PanU);
			GetBYTE (Str,"V=",&Poly->PanV);
			}
		else if (GetCMD(&Str,"ORIGIN"))
			{
			if (n<0)
				{
				Error:
				debug(LOG_Ed,"Got poly info before 'POLYGON' cmd");
				Num = 0;
				return 0;
				};
			if (GetFVECTOR(Str,&Poly->Base)!=3)
				{
				debug (LOG_Ed,"Origin coordinates not given");
				Num = 0;
				return 0;
				};
			}
		else if (GetCMD(&Str,"NORMAL"))
			{
			// Ignore it - we compute normals for exact accuracy!
			}
		else if (GetCMD(&Str,"VERTEX"))
			{
			if (n<0) goto Error;
			if (Poly->NumVertices >= FPoly::MAX_FPOLY_VERTICES)
				{
				debug (LOG_Ed,"Poly has too many vertices!");
				Num = 0;
				return 0;
				};
			i=Poly->NumVertices;
			if (GetFVECTOR (Str,&Poly->Vertex[i])!=3)
				{
				debug (LOG_Ed,"Vertex didn't specify all coords!");
				Num = 0;
				return 0;
				}
			Poly->NumVertices++;
			}
		else if (GetCMD(&Str,"TEXTUREU"))
			{
			if (n<0) goto Error;
			if (GetFIXFVECTOR (Str,&Poly->TextureU)!=3)
				{
				debug (LOG_Ed,"TextureU didn't specify all coords!");
				Num = 0;
				return 0;
				};
			}
		else if (GetCMD(&Str,"TEXTUREV"))
			{
			if (n<0) goto Error;
			if (GetFIXFVECTOR (Str,&Poly->TextureV)!=3)
				{
				debug (LOG_Ed,"TextureU didn't specify all coords!");
				Num = 0;
				return 0;
				};
			}
		else if (GetEND(&Str, "POLYGON")) {} // Ignore
		};
	if ((Poly->NumVertices>0) && (Poly->Finalize(1)!=0)) n--;
	Num = n+1;
	//
	return Num; // Success
	//
	UNGUARD("UPolys::ParseFPolys");
	};

//
// Count the number of polys in a stream
//
int	UPolys::CountFPolys(const char *Stream)
	{
	GUARD;
	const char	*Ptr,*NewStream;
	char		StrLine[256];
	int 		n = 0;
	//
	NewStream = Stream;
	while (GetLINE (&NewStream,StrLine,255)==0)
		{
		Ptr = StrLine;
		//
		if 		(GetEND(&Ptr,"POLYLIST")) 	break;
		else if (GetBEGIN(&Ptr,"POLYGON"))  n++;
		else if (strstr(Ptr,"Face "))		n++;
		else if (strstr(Ptr,"3DFACE"))		n++;
		else if (strncmp(Ptr,"FACE",4)==0)	n = OurMax(n,1+atoi(Ptr+4));
		};
	return n;
	UNGUARD("UPolys::CountFPolys");
	};

/*---------------------------------------------------------------------------------------
	The End
---------------------------------------------------------------------------------------*/
