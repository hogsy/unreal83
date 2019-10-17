/*=============================================================================
	UnMath.cpp: Unreal math routines, implementation of FGlobalMath class

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "Float.h"

/*-----------------------------------------------------------------------------
	FGlobalMath class functions
-----------------------------------------------------------------------------*/

//
// Init math subsystem
//
void FGlobalMath::Init (void)
	{
	GUARD;
	double	Angle		= 0.0;
	double	AngleInc	= 2.0*PI / (FLOAT)NUM_ANGLES;
	int		i;
	//
	for (i=0; i<NUM_ANGLES; i++) // Init angular sine and cosine tables
		{
		SinFLOAT [i]=sin(Angle);
		CosFLOAT [i]=cos(Angle);
		Angle+=AngleInc;
		};
	for (i=0; i<NUM_SQRTS; i++)
		{
		FLOAT S				= sqrt((FLOAT)(i+1) * (1.0/(FLOAT)NUM_SQRTS));
		FLOAT Temp			= (1.0-S);// Was (2*S*S*S-3*S*S+1);
		LightSqrtFLOAT[i]	= Temp/S;
		SqrtFLOAT[i]		= sqrt((FLOAT)i / 16384.0);
		};
	//
	// Init all constants:
	//
	UnitVector.Make				(1.0,1.0,1.0);
	ZeroVector.Make				(0.0,0.0,0.0);
	XAxisVector.Make			(1.0,0.0,0.0);
	YAxisVector.Make			(0.0,1.0,0.0);
	ZAxisVector.Make			(0.0,0.0,1.0);
	WorldMin.Make				(-32700.0,-32700.0,-32700.0);
	WorldMax.Make				(32700.0,32700.0,32700.0);
	VectorMax.Make				(+(FLOAT)MAXSWORD,+(FLOAT)MAXSWORD,+(FLOAT)MAXSWORD);
	VectorMin.Make				(-(FLOAT)MAXSWORD,-(FLOAT)MAXSWORD,-(FLOAT)MAXSWORD);
	UnitScaleVect.Make			(1.0,1.0,1.0);
	UnitCoords.Make				(ZeroVector,XAxisVector,YAxisVector,ZAxisVector);
	UnitScale.Make				(UnitVector,0.0,SHEER_ZX);
	ZeroRotation.Make			(0,0,0);
	CameraViewCoords.Make 		(ZeroVector,YAxisVector,-ZAxisVector,XAxisVector);
	UncameraViewCoords.Make		(ZeroVector,ZAxisVector,XAxisVector,YAxisVector);
	DefaultCameraStart.Make		(-500.0,-300.0,300.0);
	DefaultCameraRotation.Make	(0     ,0x2000,0);
	SixViewRotations[0].Make	(0x4000,0     ,0); // Up (pitch, yaw, roll)
	SixViewRotations[1].Make	(0xC000,0     ,0); // Down
	SixViewRotations[2].Make	(0     ,0     ,0); // North
	SixViewRotations[3].Make	(0     ,0x8000,0); // South
	SixViewRotations[4].Make	(0     ,0xC000,0); // East
	SixViewRotations[5].Make	(0     ,0x4000,0); // West
	//
	UNGUARD("FGlobalMath::Init");
	};

//
// Shut down math subsystem
//
void FGlobalMath::Exit (void)
	{
	GUARD;
	UNGUARD("FGlobalMath::Exit");
	};

/*-----------------------------------------------------------------------------
	Conversion functions
-----------------------------------------------------------------------------*/

//
// Return the FRotation corresponding to the direction that the vector
// is pointing in.  Sets Yaw and Pitch to the proper numbers, and sets
// roll to zero because the roll can't be determined from a vector.
//
FRotation FVector::Rotation(void)
	{
	FRotation R;
	//
	// Find yaw:
	//
	R.Yaw = atan2(Y,X) * (FLOAT)MAXWORD / (2.0*PI);
	//
	// Find pitch:
	//
	R.Pitch = atan2(Z,sqrt(X*X+Y*Y)) * (FLOAT)MAXWORD / (2.0*PI);
	//
	// Find roll:
	//
	R.Roll = 0;
	//
	return R;
	};

//
// Convert a rotation to a normal vector.
//
FVector FRotation::Vector(void)
	{
	FCoords Coords = GMath.UnitCoords;
	Coords.DeTransformByRotation(*this);
	//
	return Coords.XAxis;
	};

/*-----------------------------------------------------------------------------
	Matrix inversion
-----------------------------------------------------------------------------*/

//
// 4x4 matrix for internal use.
//
class FMatrix4
	{
	public:
	FLOAT element[4][4];
	};

//
// 4x4 matrix inversion.
// From "Graphics Gems I", Academic Press, 1990
//
float Det2x2(FLOAT a, FLOAT b, FLOAT c, FLOAT d)
	{
    return a * d - b * c;
	};

//
// Determinant of 3x3 matrix.
//
float Det3x3(
	FLOAT a1, FLOAT a2, FLOAT a3,
	FLOAT b1, FLOAT b2, FLOAT b3,
	FLOAT c1, FLOAT c2, FLOAT c3)
	{
    return
		+ a1 * Det2x2( b2, b3, c2, c3 )
        - b1 * Det2x2( a2, a3, c2, c3 )
        + c1 * Det2x2( a2, a3, b2, b3 );
	};

//
// Determinant of 4x4 matrix.
//
FLOAT Det4x4(FMatrix4 *m)
	{
    FLOAT a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;
	//
    // assign to individual variable names to aid selecting
	//  correct elements
	//
	a1 = m->element[0][0]; b1 = m->element[0][1]; 
	c1 = m->element[0][2]; d1 = m->element[0][3];
	//
	a2 = m->element[1][0]; b2 = m->element[1][1]; 
	c2 = m->element[1][2]; d2 = m->element[1][3];
	//
	a3 = m->element[2][0]; b3 = m->element[2][1]; 
	c3 = m->element[2][2]; d3 = m->element[2][3];
	//
	a4 = m->element[3][0]; b4 = m->element[3][1]; 
	c4 = m->element[3][2]; d4 = m->element[3][3];
	//
    return
		+ a1 * Det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
        - b1 * Det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
        + c1 * Det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
        - d1 * Det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
	};

//
// Adjoint of 4x4 matrix.
//
void Adjoint(FMatrix4 *in, FMatrix4 *out)
	{
    FLOAT a1, a2, a3, a4, b1, b2, b3, b4;
    FLOAT c1, c2, c3, c4, d1, d2, d3, d4;
	//
    // Assign to individual variable names to aid selecting correct values
	//
	a1 = in->element[0][0]; b1 = in->element[0][1]; 
	c1 = in->element[0][2]; d1 = in->element[0][3];
	//
	a2 = in->element[1][0]; b2 = in->element[1][1]; 
	c2 = in->element[1][2]; d2 = in->element[1][3];
	//
	a3 = in->element[2][0]; b3 = in->element[2][1];
	c3 = in->element[2][2]; d3 = in->element[2][3];
	//
	a4 = in->element[3][0]; b4 = in->element[3][1]; 
	c4 = in->element[3][2]; d4 = in->element[3][3];
	//
    // row column labeling reversed since we transpose rows & columns
	//
    out->element[0][0]  =   Det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
    out->element[1][0]  = - Det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
    out->element[2][0]  =   Det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
    out->element[3][0]  = - Det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
	//        
    out->element[0][1]  = - Det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
    out->element[1][1]  =   Det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
    out->element[2][1]  = - Det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
    out->element[3][1]  =   Det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
	//  
    out->element[0][2]  =   Det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
    out->element[1][2]  = - Det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
    out->element[2][2]  =   Det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
    out->element[3][2]  = - Det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
	//        
    out->element[0][3]  = - Det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
    out->element[1][3]  =   Det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
    out->element[2][3]  = - Det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
    out->element[3][3]  =   Det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
	};

//
// Invert a 4x4 matrix.
//
void Inverse(FMatrix4 *in, FMatrix4 *out)
	{
    int i, j;
    FLOAT det, det4x4();
	//
    // calculate the adjoint matrix
	//
    Adjoint(in,out);
	//
    //  calculate the 4x4 determinant
    //  if the determinant is zero, 
    //  then the inverse matrix is not unique.
    //
    det = Det4x4(in);
    if (fabs(det) < SMALL_NUMBER) appErrorf("Non-singular matrix, no inverse!");
	//
    // scale the adjoint matrix to get the inverse */
	//
    for (i=0; i<4; i++) for(j=0; j<4; j++) out->element[i][j] = out->element[i][j] / det;
	};

//
// Compute the inverse transpose of the 3x3 matrix defined by (V1,V2,V3) and return it
// in (R1,R2,R3):
//
void UNREAL_API InvertVectors (
	FVector &V1,FVector &V2,FVector &V3,
	FVector &R1,FVector &R2,FVector &R3)
	{
	GUARD;
	//
	FMatrix4 In,Out;
	//
	In.element[0][0] = V1.X; In.element[0][1]=V1.Y; In.element[0][2]=V1.Z; In.element[0][3]=0.0;
	In.element[1][0] = V2.X; In.element[1][1]=V2.Y; In.element[1][2]=V2.Z; In.element[1][3]=0.0;
	In.element[2][0] = V3.X; In.element[2][1]=V3.Y; In.element[2][2]=V3.Z; In.element[2][3]=0.0;
	In.element[3][0] = 0.0;  In.element[3][1]=0.0;  In.element[3][2]=0.0;  In.element[3][3]=1.0;
	//
	Inverse(&In,&Out);
	//
	R1.X = Out.element[0][0]; R1.Y = Out.element[1][0]; R1.Z=Out.element[2][0];
	R2.X = Out.element[0][1]; R2.Y = Out.element[1][1]; R2.Z=Out.element[2][1];
	R3.X = Out.element[0][2]; R3.Y = Out.element[1][2]; R3.Z=Out.element[2][2];
	//
	UNGUARD("InvertVectors");
	};

//
// Randomly perturb a vector using a coordinate system for scaling.
//
void UNREAL_API PerturbNormalVector(FVector &V,const FCoords &C,FLOAT Ratio)
	{
	Ratio *= 2.0;
	V += C.XAxis * (Ratio * (0.5 - (FLOAT)rand() / (FLOAT)RAND_MAX)) +
		 C.YAxis * (Ratio * (0.5 - (FLOAT)rand() / (FLOAT)RAND_MAX)) +
		 C.ZAxis * (Ratio * (0.5 - (FLOAT)rand() / (FLOAT)RAND_MAX));
	V.Normalize();
	};

/*-----------------------------------------------------------------------------
	FBoundingVolume implementation
-----------------------------------------------------------------------------*/

//
// Initialize to an empty bound centered at 0,0,0.
//
void FBoundingVolume::Init(void)
	{
	GUARD;
	//
	Min      = GMath.ZeroVector;
	Max      = GMath.ZeroVector;
	Sphere   = GMath.ZeroVector;
	Sphere.W = 0.0;
	//
	Min.iTransform = 0; // Note that it's invalid
	//
	UNGUARD("FBoundingVolume::Init");
	};

//
// Set the bounding volume with a list of points.
// Graphics Gems I, Jack Ritter p. 301
//
void FBoundingVolume::Init(FVector *Points,int NumPts)
	{
	GUARD;
	//
	if (NumPts==0)
		{
		Init();
		return;
		};
	FVector MinX = Points[0];
	FVector MaxX = Points[0];
	FVector MinY = Points[0];
	FVector MaxY = Points[0];
	FVector MinZ = Points[0];
	FVector MaxZ = Points[0];
	//
	Min = Points[0];
	Max = Points[0];
	for (int i=1; i<NumPts; i++)
		{
		FVector *P = &Points[i];
		//
		UPDATE_MIN(&Min,P);
		UPDATE_MAX(&Max,P);
		//
		if (P->X < MinX.X) MinX = *P;
		if (P->X > MaxX.X) MaxX = *P;
		if (P->Y < MinY.Y) MinY = *P;
		if (P->Y > MaxY.Y) MaxY = *P;
		if (P->Z < MinZ.Z) MinZ = *P;
		if (P->Z > MaxZ.Z) MaxZ = *P;
		};
	FLOAT DX = FDist(MinX,MaxX);
	FLOAT DY = FDist(MinY,MaxY);
	FLOAT DZ = FDist(MinZ,MaxZ);
	//
	FLOAT RSquared;
	if		((DX>DY) && (DX>DZ))	{Sphere=FMidpoint(MinX,MaxX); RSquared=DX*DX/4.0;}
	else if (DY>DZ)					{Sphere=FMidpoint(MinY,MaxY); RSquared=DY*DY/4.0;}
	else							{Sphere=FMidpoint(MinZ,MaxZ); RSquared=DZ*DZ/4.0;};
	//
	for (int i=0; i<NumPts; i++)
		{
		FLOAT DistSquared = FDistSquared(Points[i],Sphere);
		if (DistSquared > RSquared) RSquared = DistSquared;
		};
	Sphere.W = sqrt(RSquared) * 1.01; // For guaranteed safety
	//
	Min.iTransform = INDEX_NONE; // Note that it's now valid
	//
	UNGUARD("FBoundingVolume::Init");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
