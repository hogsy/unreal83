/*=============================================================================
	UnMath.h: Unreal math routines

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNMATH // Prevent header from being included multiple times
#define _INC_UNMATH

/*-----------------------------------------------------------------------------
	Declarations
-----------------------------------------------------------------------------*/

class  FVector;		// Floating point vector
class  FCoords;		// Floating point coordinate system
class  FRotation;	// Fixed point pitch-yaw-roll rotation
class  FScale;		// Floating point scaling value
class  FGlobalMath;	// Global math table class

/*-----------------------------------------------------------------------------
	FVector
-----------------------------------------------------------------------------*/

//
// Information associated with a floating point vector, describing its
// status as a point in a rendering context:
//
enum EVectorFlags
	{
	FVF_OutXMin		= 0x04,	// Outcode rejection, off left hand side of screen
	FVF_OutXMax		= 0x08,	// Outcode rejection, off right hand side of screen
	FVF_OutYMin		= 0x10,	// Outcode rejection, off top of screen
	FVF_OutYMax		= 0x20,	// Outcode rejection, off bottom of screen
	FVF_OutZBack	= 0x40,	// Outcode rejection, behind viewer
	FVF_OutReject   = (FVF_OutXMin | FVF_OutXMax | FVF_OutYMin | FVF_OutYMax | FVF_OutZBack), // Outcode rejectable
	FVF_OutSkip		= (FVF_OutXMin | FVF_OutXMax | FVF_OutYMin | FVF_OutYMax), // Outcode clippable
	};

//
// Floating point vector alignment flags, for directional vectors.  Aids in
// optimizing trivial transformations.
//
enum EVectorAlign
	{
	FVA_None		= 0,	// Vector isn't axis-aligned
	FVA_X			= 1,	// Vector is parallel to the X-Axis
	FVA_Y			= 2,	// Vector is parallel to the Y-Axis
	FVA_Z			= 3,	// Vector is parallel to the Z-Axis
	FVA_Uncached	= 4,	// Indicates that the thing is not cached
	};

//
// 32-bit floating point point, vector, plane, and color
//
class UNREAL_API FVector 
	{
	public:
	union
		{
		struct {FLOAT X,Y,Z;}; // Coordinates
		struct {FLOAT R,G,B;}; // Color components
		struct {FLOAT V[3];};  // Indexed components
		};
	union
		{
		struct // Special flags for 3-component (X,Y,Z) vectors
			{
			WORD	iTransform;	// Transformation cache index
			BYTE	Flags;		// Engine-internal misc flags from EVectorFlags
			BYTE	Align;		// Engine-internal alignment from EVectorAlign
			};
		FLOAT W;				// Fourth coordinate for 4-component (X,Y,Z,D) planes
		};
	//
	// Math operators:
	//
	inline FLOAT& operator() (const int i) {return V[i];};
	FVector inline operator+  (const FVector &V) const;
	FVector inline operator+= (const FVector &V);
	FVector inline operator-  (const FVector &V) const;
	FVector inline operator-  (void) const;
	FVector inline operator-= (const FVector &V);
	FVector inline operator*  (FLOAT Scale) const;
	FVector inline operator^  (const FVector &V) const; // Cross product
	FLOAT   inline operator|  (const FVector &V) const; // Dot product
	friend FVector inline operator*  (FLOAT Scale,const FVector &V);
	FVector inline operator*= (FLOAT Scale);
	FVector inline operator/  (FLOAT Scale) const;
	FVector inline operator/= (FLOAT Scale);
	int     inline operator== (const FVector &V) const;
	int     inline operator!= (const FVector &V) const;
	//
	// Simple functions:
	//
	FVector inline Make(FLOAT CX, FLOAT CY, FLOAT CZ);
	FLOAT	inline Size(void) const;
	FLOAT	inline SizeSquared(void) const;
	FLOAT	inline Size2D(void) const;
	int     inline IsNearlyZero(void) const;
	int  	inline IsZero(void) const;
	int		inline Normalize (void);
	void	inline NormalizeApprox (void);
	void	inline AddYawed (FLOAT Forward,FLOAT Strafe,WORD Yaw);
	FLOAT	inline GetYawedNormal (WORD Yaw);
	FLOAT   inline GetYawedTangent(WORD Yaw);
	void	inline MoveBounded (const FVector &Delta);
	FVector inline GridSnap (const FVector &Grid);
	//
	// Transformation:
	//
	void	inline TransformVector	(const FCoords &Coords);
	void	inline TransformPoint	(const FCoords &Coords);
	FVector inline Mirror			(const FVector &MirrorNormal) const;
	//
	// Complicated functions:
	//
	FRotation		Rotation					(void);
	//
	// Friends:
	//
	friend FLOAT   inline FDist					(const FVector &V1, const FVector &V2);
	friend FLOAT   inline FDistSquared			(const FVector &V1, const FVector &V2);
	friend FLOAT   inline FDistApprox			(const FVector &V1, const FVector &V2);
	friend FVector inline FMidpoint				(const FVector &V1, const FVector &V2);
	friend int     inline FPointsAreSame        (const FVector &P, const FVector &Q);
	friend int     inline FPointsAreNear        (const FVector &Point1,const FVector &Point2, FLOAT Dist);
	friend FLOAT   inline FPointPlaneDist       (const FVector &Point,const FVector &PlaneBase,const FVector &PlaneNormal);
	friend FLOAT   inline FEllipsePlaneDist     (const FVector &Point,const FVector &Weight,const FVector &PlaneOrigin,const FVector &PlaneNormal);
	friend FVector inline FLinePlaneIntersection(const FVector &Point1,const FVector &Point2,const FVector &PlaneOrigin,const FVector &PlaneNormal);
	friend int     inline FParallel             (const FVector &Normal1, const FVector &Normal2);
	friend int     inline FCoplanar             (const FVector &Base1, const FVector &Normal1, const FVector &Base2, const FVector &Normal2);
	friend void	UNREAL_API InvertVectors (FVector &V1,FVector &V2,FVector &V3,FVector &R1,FVector &R2,FVector &R3);
	friend void UNREAL_API PerturbNormalVector(FVector &V,const FCoords &C,FLOAT Ratio);
	//
	// Macros to perform an operation on each of a vector's 3 or 4 components. Example:
	//
	//	FVector A,B,C;
	//	forv3(i,A(i)+=B(i)*C(i));
	//
	#define forv3(_var,_expr) {{const int _var=0;_expr;}{const int _var=1;_expr;}{const int _var=2;_expr;}}
	#define forv4(_var,_expr) {{const int _var=0;_expr;}{const int _var=1;_expr;}{const int _var=2;_expr;}{const int _var=3;_expr;}}
	};

/*-----------------------------------------------------------------------------
	FScale
-----------------------------------------------------------------------------*/

//
// An axis along which sheering is performed
//
enum ESheerAxis
	{
	SHEER_None = 0,
	SHEER_XY   = 1,
	SHEER_XZ   = 2,
	SHEER_YX   = 3,
	SHEER_YZ   = 4,
	SHEER_ZX   = 5,
	SHEER_ZY   = 6,
	};

//
// Scaling and sheering info associated with a brush.  This is 
// easily-manipulated information which is built into a transformation
// matrix later.
//
class UNREAL_API FScale 
	{
	public:
	FVector		Scale;
	FLOAT		SheerRate;
	ESheerAxis	SheerAxis;
	//
	// Functions:
	//
	FScale inline Make(FVector CScale,FLOAT CSheerRate,ESheerAxis CSheerAxis);
	FLOAT  inline Orientation (void);
	};

/*-----------------------------------------------------------------------------
	FRotation & FFloatRotation
-----------------------------------------------------------------------------*/


//
// Unsigned word rotation info used throughout Unreal
//
class UNREAL_API FRotation
	{
	public:
	//
	ANGLE_TYPE Pitch;   // Looking up and down (0=Straight Ahead, +Up, -Down).
	ANGLE_TYPE Yaw;     // Rotating around (running in circles), 0=East, +North, -South
	ANGLE_TYPE Roll;    // Rotation about axis of screen, 0=Straight, +Clockwise, -CCW
	//
	// Operators:
	//
	FRotation	inline operator+  (const FRotation &V) const;
	FRotation	inline operator+= (const FRotation &V);
	FRotation	inline operator-  (const FRotation &V) const;
	FRotation	inline operator-  (void) const;
	FRotation	inline operator-= (const FRotation &V);
	FRotation inline operator*  (FLOAT Scale) const;
	friend FRotation inline operator*  (FLOAT Scale,const FRotation &R);
	FRotation inline operator*= (FLOAT Scale);
	int			inline operator== (const FRotation &R) const;
	int			inline operator!= (const FRotation &R) const;
	//
	// Functions:
	//
	FRotation	inline Make			(WORD CPitch,WORD CYaw,WORD CRoll);
	void		inline Add			(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll);
	void		inline AddBounded	(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll);
	FRotation	inline GridSnap		(const FRotation &RotGrid);
	FVector		Vector				(void);
	};

//
// Floating point rotation used for high-precision rotation in UnrealEd
//
class FFloatRotation
	{
	public:
	FLOAT	Pitch;
	FLOAT	Yaw;
	FLOAT	Roll;
	};

/*-----------------------------------------------------------------------------
	FCoords, FModelCoords
-----------------------------------------------------------------------------*/

//
// A coordinate system matrix
//
class UNREAL_API FCoords
	{
	public:
	FVector	Origin;
	FVector	XAxis;
	FVector YAxis;
	FVector ZAxis;
	//
	// Simple functions:
	//
	FCoords inline Make(FVector COrigin, FVector CXAxis, FVector CYAxis, FVector CZAxis);
	FCoords	inline Transposition(void);
	//
	// Transformation:
	//
	void     inline TransformByCoords				(const FCoords   &TransformCoords);
	void	 inline TransformByRotation				(const FRotation &Rot);
	void	 inline DeTransformByRotation			(const FRotation &Rot);
	void	 inline TransformByScale				(const FScale    &Scale);
	void	 inline DeTransformByScale				(const FScale    &Scale);
	FCoords  inline Mirror							(const FVector   &MirrorNormal) const; // Mirror must be normal
	};

//
// A model coordinate system, describing both the covariant and contravariant
// transformation matrices to transform points and normals by.
//
class UNREAL_API FModelCoords
	{
	public:
	FCoords PointXform;		// Coordinates to transform points by (covariant)
	FCoords VectorXform;	// Coordinates to transform normals by (contravariant)
	};

/*-----------------------------------------------------------------------------
	FBoundingVolume
-----------------------------------------------------------------------------*/

//
// A bounding volume in space. Contains an axis-aligned bounding box 
// (extendeding from Min to Max) and a bounding sphere Sphere.
//
class UNREAL_API FBoundingVolume
	{
	public:
	FVector Min;	// X,Y,Z = Box minima
	FVector Max;	// X,Y,Z = Box maxima
	FVector Sphere;	// X,Y,Z = Center, D = radius
	//
	// Call Init() to initialize to an empty bound centered at 0,0,0.
	//
	// To set bounding box and sphere to meaningful values, cycle through all
	// your points twice (Pass = 0-1) and call Update() with each vertex and
	// the pass number, then call Finalize() when done.
	//
	void Init(void);
	void Init(FVector *Points,int NumPts);
	};

/*-----------------------------------------------------------------------------
	FGlobalMath
-----------------------------------------------------------------------------*/

//
// Global math structure
//
class UNREAL_API FGlobalMath
	{
	public:
	//
	enum {ANGLE_SHIFT 	= 4};		// Bits to shift to get lookup value
	enum {ANGLE_BITS	= 12};		// Number of valid bits in angles
	enum {NUM_ANGLES 	= 4096}; 	// Number of angles that are in lookup table (was 1024)
	enum {NUM_SQRTS		= 16384};	// Number of square roots in lookup table
	//
	// Tables:
	//
	private:
	FLOAT  SinFLOAT			[NUM_ANGLES];
	FLOAT  CosFLOAT			[NUM_ANGLES];
	FLOAT  SqrtFLOAT		[NUM_SQRTS];
	FLOAT  LightSqrtFLOAT	[NUM_SQRTS];
	//
	public:
	inline FLOAT Sqrt( int i )		{ return SqrtFLOAT[i]; };
	inline FLOAT LightSqrt( int i ) { return LightSqrtFLOAT[i]; };
	inline FLOAT SinTab( int i )	{ return SinFLOAT[(i>>ANGLE_SHIFT)&(NUM_ANGLES-1)]; };
	inline FLOAT CosTab( int i )	{ return CosFLOAT[(i>>ANGLE_SHIFT)&(NUM_ANGLES-1)]; };
	//
	// Constants:
	//
	FVector		VectorMin;
	FVector		VectorMax;
	FVector  	UnitVector;
	FVector  	ZeroVector;
	FVector  	XAxisVector;
	FVector  	YAxisVector;
	FVector  	ZAxisVector;
	FVector  	WorldMin;
	FVector  	WorldMax;
	FVector		UnitScaleVect;
	FCoords  	UnitCoords;
	FScale   	UnitScale;
	FRotation	ZeroRotation;
	FCoords		CameraViewCoords;
	FCoords		UncameraViewCoords;
	FVector		DefaultCameraStart;
	FRotation	DefaultCameraRotation;
	FRotation	SixViewRotations[6];
	//
	// Startup and shutdown:
	//
	void Init(void);
	void Exit(void);
	//
	// Functions:
	//
	DWORD LogTwo (DWORD N);
	DWORD NextPowerOfTwo (DWORD N);
	};

/*-----------------------------------------------------------------------------
	Floating point constants
-----------------------------------------------------------------------------*/

//
// Standard
//
#define PI 								((FLOAT)3.1415926535897932)
#define SQRT2							((FLOAT)1.4142135623730950)
#define SQRT3							((FLOAT)1.7320508075688772)
#define LOGE2							((FLOAT)0.6931471805599453)
#define FLOAT_NONE						((FLOAT)MAXSWORD)
#define SMALL_NUMBER					((FLOAT)1.e-8)
//
// Lengths of normalized vectors (These are half their maximum values
// to assure that dot products with normalized vectors don't overflow):
//
#define FLOAT_NORMAL_THRESH				(0.0001f)
//
// Magic numbers required for CSG to work stably:
//
#define THRESH_POINT_ON_PLANE			(0.10)		/* Thickness of plane for front/back/inside test */
#define THRESH_POINT_ON_SIDE			(0.20)		/* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define THRESH_POINTS_ARE_SAME			(0.002)		/* Two points are same if within this distance */
#define THRESH_POINTS_ARE_NEAR			(0.015)		/* Two points are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_NORMALS_ARE_SAME			(0.00002)	/* Two normal points are same if within this distance */
													/* Making this too large results in incorrect CSG classification and disaster */
#define THRESH_VECTORS_ARE_NEAR			(0.0004)	/* Two vectors are near if within this distance and can be combined if imprecise math is ok */
													/* Making this too large results in lighting problems due to inaccurate texture coordinates */
#define THRESH_SPLIT_POLY_WITH_PLANE	(0.25)		/* A plane splits a polygon in half */
#define THRESH_SPLIT_POLY_PRECISELY		(0.01f)		/* A plane exactly splits a polygon */
#define THRESH_ZERO_NORM_SQUARED		(0.01*0.01)	/* Size of a unit normal that is considered "zero", squared */
#define THRESH_VECTORS_ARE_PARALLEL		(0.02)		/* Vectors are parallel if dot product varies less than this */

#define THRESH_OPTGEOM_COPLANAR			(0.05)		/* Threshold for Bsp geometry optimization */
#define THRESH_OPTGEOM_COSIDAL			(0.05)		/* Threshold for Bsp geometry optimization */

/*-----------------------------------------------------------------------------
	Fast, global inline functions
-----------------------------------------------------------------------------*/

inline	int FIX		(int A)			{return A<<16;};
inline	int FIX		(FLOAT A)		{return A*65536.0;};
inline	int FIXFRAC	(int A,int F)   {return FIX(A)+F;};
inline	int UNFIX	(int A)			{return A>>16;};

template <class T> inline	T   OurAbs	(T A)			{return (A>=(T)0) ? A : -A;};
template <class T> inline	T   OurSgn	(T A)			{return (A>0) ? 1 : ((A<0) ? -1 : 0);};
template <class T> inline	T	OurMax	(T A, T B)		{return (A>=B) ? A : B;};
template <class T> inline	T	OurMin	(T A, T B)		{return (A<=B) ? A : B;};
template <class T> inline	T	OurSquare(T A)			{return A*A;};
template <class T> inline   T   Align4  (T A)			{return (A+3)&~3;};

template <class T> inline void OurExchange(T A, T B)
	{
	T Temp = A;
	A      = B;
	B      = Temp;
	};

template <class T> inline T OurClamp (T X, T Min, T Max)
	{
	if		((X>=Min) && (X<=Max))	return X;
	else if	(X<Min)					return Min;
	else							return Max;
	};

inline void UPDATE_MIN(FVector *Min,FVector *V)
	{
	if (V->X < Min->X) Min->X = V->X;
	if (V->Y < Min->Y) Min->Y = V->Y;
	if (V->Z < Min->Z) Min->Z = V->Z;
	};

inline void UPDATE_MAX(FVector *Max,FVector *V)
	{
	if (V->X > Max->X) Max->X = V->X;
	if (V->Y > Max->Y) Max->Y = V->Y;
	if (V->Z > Max->Z) Max->Z = V->Z;
	};

//
// Convert a floating point number to an integer.
// Workaround to VC++'s unbelievably slow conversion routine.
//
inline void ftoi(int &I,FLOAT F)
	{
	#ifdef ASM
		__asm fld   [F]				// Load as floating point number
		__asm mov   eax,[I]			// Address of destination
		__asm fistp [DWORD PTR eax]	// Store as integer and pop
	#else
		I = (int)floor(F+0.5);
	#endif
	};

//
// Convert a floating point number to a byte.
// Workaround to VC++'s unbelievably slow conversion routine.
//
inline void ftob(BYTE &B,FLOAT F)
	{
	#ifdef ASM
		INT Temp;
		__asm fld   [F]				// Load as floating point number
		__asm mov   eax,[B]			// Address of destination
		__asm fistp [Temp]			// Store as integer and pop
		__asm mov   bl,BYTE PTR [Temp]
		__asm mov   [eax],bl
	#else
		B = (BYTE)floor(F+0.5);
	#endif
	};

inline int ftoi(FLOAT F)
	{
	#ifdef ASM
		int I;
		__asm fld   [F]				// Load as floating point number
		__asm fistp [I]				// Store as integer and pop
		return I;
	#else
		return (int)floor(F+0.5);
	#endif
	};

/*-----------------------------------------------------------------------------
	IEEE floating point class
-----------------------------------------------------------------------------*/

class FFloatIEEE
	{
	public:
	union
		{
		FLOAT F;
		DWORD D;
		struct // Individual fields
			{
			DWORD	Mantissa:23;	// Fractional part, not including implied leading '1'
			DWORD	Exponent:8;		// Exponent, biased by 127
			DWORD	Sign:1;			// Sign, 0=positive, 1=negative
			};
		};
	inline int GetExponent(void)			{return Exponent-127;};
	inline int GetMantissa(void)			{return Mantissa;};
	inline int GetMantissaBits(int Bits)	{return Mantissa >> (23 - Bits);};
	};

class FDoubleIEEE
	{
	public:
	union
		{
		DOUBLE F;
		QWORD Q;
		struct // Individual fields
			{
			QWORD	Mantissa:52;	// Fractional part, not including implied leading '1'
			QWORD	Exponent:11;	// Exponent, biased by 127
			QWORD	Sign:1;			// Sign, 0=positive, 1=negative
			};
		};
	inline int GetExponent() {return Exponent-1023;};
	};

/*-----------------------------------------------------------------------------
	Global functions 
-----------------------------------------------------------------------------*/

FLOAT	inline FSnap		(FLOAT Location, FLOAT Grid);
FLOAT	inline FSheerSnap	(FLOAT Sheer);
DWORD	inline FNextPowerOfTwo (DWORD N);
DWORD	inline FLogTwo		(DWORD N);
FLOAT	inline FCalcFOV		(FLOAT SXR, FLOAT FOVAngle);
WORD	inline FAddAngleConfined(WORD Angle,INT Delta, WORD MinThresh, WORD MaxThresh);
void	inline FCalcViewCoords (const FVector &Location, const FRotation &Rotation, FCoords &Coords, FCoords &Uncoords);

/*-----------------------------------------------------------------------------
	Big global functions
-----------------------------------------------------------------------------*/

//
// Snap a value to the nearest grid multiple.
//
FLOAT FSnap (FLOAT Location, FLOAT Grid)
	{
	if (Grid==0)	return Location;
	else			return floor((Location + 0.5*Grid)/Grid)*Grid;
	};

//
// Internal sheer adjusting function so it snaps nicely at 0 and 45 degrees
//
FLOAT inline FSheerSnap (FLOAT Sheer)
	{
	if		(Sheer < -0.65)	return Sheer + 0.15;
	else if (Sheer > +0.65)	return Sheer - 0.15;
	else if (Sheer < -0.55)	return -0.50;
	else if (Sheer > +0.55)	return 0.50;
	else if (Sheer < -0.05)	return Sheer + 0.05;
	else if (Sheer > +0.05)	return Sheer - 0.05;
	else					return 0.0;
	};

//
// Find the closest power of 2 that is >= N.
//
DWORD inline FNextPowerOfTwo (DWORD N)
	{
	if (N<=0L		) return 0L;
	if (N<=1L		) return 1L;
	if (N<=2L		) return 2L;
	if (N<=4L		) return 4L;
	if (N<=8L		) return 8L;
	if (N<=16L	    ) return 16L;
	if (N<=32L	    ) return 32L;
	if (N<=64L 	    ) return 64L;
	if (N<=128L     ) return 128L;
	if (N<=256L     ) return 256L;
	if (N<=512L     ) return 512L;
	if (N<=1024L    ) return 1024L;
	if (N<=2048L    ) return 2048L;
	if (N<=4096L    ) return 4096L;
	if (N<=8192L    ) return 8192L;
	if (N<=16384L   ) return 16384L;
	if (N<=32768L   ) return 32768L;
	if (N<=65536L   ) return 65536L;
	else			  return 0;
	};

//
// Find the largest number whose base-2 log is <= N.
//
DWORD inline FLogTwo (DWORD N)
	{
	if (N<=0L		) return 0;
	if (N<=1L		) return 0;
	if (N<=2L		) return 1;
	if (N<=4L		) return 2;
	if (N<=8L		) return 3;
	if (N<=16L	    ) return 4;
	if (N<=32L	    ) return 5;
	if (N<=64L 	    ) return 6;
	if (N<=128L     ) return 7;
	if (N<=256L     ) return 8;
	if (N<=512L     ) return 9;
	if (N<=1024L    ) return 10;
	if (N<=2048L    ) return 11;
	if (N<=4096L    ) return 12;
	if (N<=8192L    ) return 13;
	if (N<=16384L   ) return 14;
	if (N<=32768L   ) return 15;
	if (N<=65536L   ) return 16;
	else			  return 0;
	};

//
// Calculate projection plane distance in pixels,
// given field of view angle and screen X resolution.
//
FLOAT inline FCalcFOV (FLOAT SXR, FLOAT FOVAngle)
	{
	WORD TempAngle = (0.5 * 65536.0 / 360.0) * FOVAngle;
	return SXR / (2.0*GMath.SinTab(TempAngle) / GMath.CosTab(TempAngle));
	};

//
// Add to a word angle, constraining it within a min (not to cross)
// and a max (not to cross).  Accounts for funkyness of word angles.
// Assumes that angle is initially in the desired range.
//
WORD inline FAddAngleConfined(WORD Angle,INT Delta, WORD MinThresh, WORD MaxThresh)
	{
	if (Delta<0)
		{
		if ((Delta<=-0x10000L) || (Delta<=-(INT)((WORD)(Angle-MinThresh)))) return MinThresh;
		}
	else if (Delta>0)
		{
		if ((Delta>=0x10000L) || (Delta>=(INT)((WORD)(MaxThresh-Angle)))) return MaxThresh;
		};
	return (WORD)((INT)Angle+Delta);
	};

//
// Compute view transformation and detransformation coordinate systems
// for a given location and rotation.
//
void inline FCalcViewCoords (const FVector &Location, const FRotation &Rotation,
	FCoords &Coords, FCoords &Uncoords)
	{
	Coords = GMath.CameraViewCoords;
	Coords.DeTransformByRotation (Rotation);
	Coords.Origin = Location;
	//
	Uncoords.XAxis.X = Coords.XAxis.X;
	Uncoords.XAxis.Y = Coords.YAxis.X;
	Uncoords.XAxis.Z = Coords.ZAxis.X;
	//
	Uncoords.YAxis.X = Coords.XAxis.Y;
	Uncoords.YAxis.Y = Coords.YAxis.Y;
	Uncoords.YAxis.Z = Coords.ZAxis.Y;
	//
	Uncoords.ZAxis.X = Coords.XAxis.Z;
	Uncoords.ZAxis.Y = Coords.YAxis.Z;
	Uncoords.ZAxis.Z = Coords.ZAxis.Z;
	//
	Uncoords.Origin  = Location;
	//
	Uncoords.Origin *= -1.0;
	Uncoords.Origin.TransformVector(Coords);
	};

/*-----------------------------------------------------------------------------
	FVector implementation
-----------------------------------------------------------------------------*/

///////////////////////
// FVector Operators //
///////////////////////

FVector inline FVector::operator+= (const FVector &V)
	{
	X += V.X; Y += V.Y; Z += V.Z;
	return *this;
	};

FVector inline FVector::operator-= (const FVector &V)
	{
	X -= V.X; Y -= V.Y; Z -= V.Z;
	return *this;
	};

FVector inline FVector::operator*= (FLOAT Scale)
	{
	X *= Scale; Y *= Scale; Z *= Scale;
	return *this;
	};

//
// Cross product operator: The vector cross product of two vectors.  
// The result is perpendicular to both input vectors and its magnitude
// is |V1| |V2| cos (Angle), zero if either vector is 
// zero or they are parallel.
//
FVector inline FVector::operator^ (const FVector &V) const
	{
	FVector Result;
	Result.X = Y * V.Z - Z * V.Y;
	Result.Y = Z * V.X - X * V.Z;
	Result.Z = X * V.Y - Y * V.X;
	return Result;
	};

//
// Dot product operator: The scalar dot product of two vectors.  
// The magnitude is |V1| |V2| sin (Angle), zero if either vector 
// is zero or they are perpendicular.
//
FLOAT inline FVector::operator| (const FVector &V) const
	{
	return X*V.X + Y*V.Y + Z*V.Z;
	};

FVector inline operator* (FLOAT Scale,const FVector &V)
	{
	FVector Result;
	Result.X = V.X * Scale;
	Result.Y = V.Y * Scale;
	Result.Z = V.Z * Scale;
	return Result;
	};

FVector inline FVector::operator/= (FLOAT V)
	{
	FLOAT RV = 1.0/V;
	X *= RV; Y *= RV; Z *= RV;
	return *this;
	};

FVector inline FVector::operator+ (const FVector &V) const
	{
	FVector Temp;
	Temp.X = X + V.X; Temp.Y = Y + V.Y; Temp.Z = Z + V.Z;
	return Temp;
	};

FVector inline FVector::operator- (const FVector &V) const
	{
	FVector Temp;
	Temp.X = X - V.X; Temp.Y = Y - V.Y; Temp.Z = Z - V.Z;
	return Temp;
	};

FVector inline FVector::operator- (void) const
	{
	FVector Temp;
	Temp.X = -X; Temp.Y = -Y; Temp.Z = -Z;
	return Temp;
	};

FVector inline FVector::operator* (FLOAT Scale ) const
	{
	FVector Temp;
	Temp.X = X * Scale; Temp.Y = Y * Scale; Temp.Z = Z * Scale;
	return Temp;
	};

FVector inline FVector::operator/ (FLOAT Scale) const
	{
	FVector Temp;
	FLOAT	RScale = 1.0/Scale;
	Temp.X = X * RScale; Temp.Y = Y * RScale; Temp.Z = Z * RScale;
	return Temp;
	};

int inline FVector::operator== (const FVector &V) const
	{
	return (X==V.X) && (Y==V.Y) && (Z==V.Z);
	};

int inline FVector::operator!= (const FVector &V) const
	{
	return (X!=V.X) || (Y!=V.Y) || (Z!=V.Z);
	};

///////////////////////
// FVector Functions //
///////////////////////

//
// Make a floating point vector from three floating point values.
//
FVector inline FVector::Make(FLOAT CX, FLOAT CY, FLOAT CZ)
	{
	X          = CX;
	Y          = CY;
	Z          = CZ;
	iTransform = INDEX_NONE;
	Flags      = 0;
	Align      = 0;
	return *this;
	};

//
// The euclidean size of a vector.
//
FLOAT inline FVector::Size(void) const
	{
	return sqrt(X*X+Y*Y+Z*Z);
	};

//
// The size of a vector, squared.
//
FLOAT inline FVector::SizeSquared(void) const
	{
	return X*X+Y*Y+Z*Z;
	};

//
// The 2D size of the X and Y components of a vector, disregarding Z.
//
FLOAT inline FVector::Size2D(void) const 
	{
	return sqrt(X*X+Y*Y);
	};

//
// See if a vector is nearly zero.
//
int inline FVector::IsNearlyZero(void) const
	{
	return (OurAbs(X)<0.0001)&&(OurAbs(Y)<0.0001)&&(OurAbs(Z)<0.0001);
	};

//
// See if a vector is exactly zero.
//
int inline FVector::IsZero(void) const
	{
	return (X==0.0)&&(Y==0.0)&&(Z==0.0);
	};

//
// Normalize a vector.  Returns 1 if ok, or 0 if the vector was
// too small to be normalized properly.
//
inline int FVector::Normalize (void)
	{
	FLOAT SquareSum;
	FLOAT Scale;
	//
	SquareSum = X*X+Y*Y+Z*Z;
	if (SquareSum >= 0.0000001)
		{
		Scale     = 1.0/sqrt (SquareSum);
		X        *= Scale;
		Y        *= Scale;
		Z        *= Scale;
		return 1;
		}
	else return 0;
	};

//
// Approximately normalize a vector, preserving its direction exactly.
// +/- 9% size error.
//
void inline FVector::NormalizeApprox (void)
	{
	FLOAT	Min,Med,Max,Temp,Scale;
	//
	// Returns distance plus or minus 8% error, from Jack Ritter, Graphics Gems I.
	//
	Min = OurAbs(X);
	Med = OurAbs(Y);
	Max = OurAbs(Z);
	//
	if (Max<Med) {Temp = Max; Max = Med; Med = Temp;};
	if (Max<Min) {Temp = Max; Max = Min; Min = Temp;};
	//
	Scale = 1.0 / (Max + (11.0/32.0)*Med + (1.0/4.0)*Min);
	//
	X *= Scale;
	Y *= Scale;
	Z *= Scale;
	};

//
// Snap a point to the nearest grid lattice point.
//
FVector inline FVector::GridSnap (const FVector &Grid)
	{
	FVector Result;
	Result.X = FSnap (X,Grid.X);
	Result.Y = FSnap (Y,Grid.Y);
	Result.Z = FSnap (Z,Grid.Z);
	return Result;
	};

//
// For moving the player forward and sideways.
//
void inline FVector::AddYawed (FLOAT Forward,FLOAT Strafe,WORD Yaw)
	{
	X +=	Forward * +GMath.CosTab(Yaw) +
			Strafe  * -GMath.SinTab(Yaw);
	Y +=	Forward * +GMath.SinTab(Yaw) +
   			Strafe  * +GMath.CosTab(Yaw);
	};

//
// For finding the forward velocity of the player.
//
FLOAT inline FVector::GetYawedNormal (WORD Yaw)
	{
	return X * +GMath.CosTab(Yaw) + Y * +GMath.SinTab(Yaw);
	};

//
// For finding the sideways velocity of the player.
//
FLOAT inline FVector::GetYawedTangent(WORD Yaw)
	{
	return X * -GMath.SinTab(Yaw) + Y * +GMath.CosTab(Yaw);
	};

////////////////////////////
// FVector Transformation //
////////////////////////////

//
// Transform a directional vector by a coordinate system.
// Ignore's the coordinate system's origin.
//
void inline FVector::TransformVector (const FCoords &Coords)
	{
	FVector TempPoint = *this;
	//
	X = TempPoint.X * Coords.XAxis.X +
		TempPoint.Y * Coords.XAxis.Y +
		TempPoint.Z * Coords.XAxis.Z;
	Y = TempPoint.X  * Coords.YAxis.X +
		TempPoint.Y * Coords.YAxis.Y +
		TempPoint.Z * Coords.YAxis.Z;
	Z = TempPoint.X * Coords.ZAxis.X +
		TempPoint.Y * Coords.ZAxis.Y +
		TempPoint.Z * Coords.ZAxis.Z;
	};

//
// Transform a point by a coordinate system, moving
// it by the coordinate system's origin if nonzero.
//
void inline FVector::TransformPoint (const FCoords &Coords)
	{
	FVector TempPoint;
	//
	TempPoint.X = X - Coords.Origin.X;
	TempPoint.Y = Y - Coords.Origin.Y;
	TempPoint.Z = Z - Coords.Origin.Z;
	//
	X = TempPoint.X * Coords.XAxis.X +
		TempPoint.Y * Coords.XAxis.Y +
		TempPoint.Z * Coords.XAxis.Z;
	Y = TempPoint.X * Coords.YAxis.X +
		TempPoint.Y * Coords.YAxis.Y +
		TempPoint.Z * Coords.YAxis.Z;
	Z = TempPoint.X * Coords.ZAxis.X +
		TempPoint.Y * Coords.ZAxis.Y +
		TempPoint.Z * Coords.ZAxis.Z;
	};

//
// Mirror a vector about a normal vector.
//
FVector inline FVector::Mirror (const FVector &MirrorNormal) const
	{
	FLOAT OutFactor = 2.0 * (X * MirrorNormal.X + Y * MirrorNormal.Y + Z * MirrorNormal.Z);
	//
	FVector Result;
	Result.X = X - OutFactor * MirrorNormal.X;
	Result.Y = Y - OutFactor * MirrorNormal.Y;
	Result.Z = Z - OutFactor * MirrorNormal.Z;
	return Result;
	};

//
// Move a point vector, restricting it to within the world box:
//
void inline FVector::MoveBounded (const FVector &Delta)
	{
	FVector New;
	//
	New.X = X+Delta.X;
	New.Y = Y+Delta.Y;
	New.Z = Z+Delta.Z;
	//
	if ((New.X >= -MAXSWORD) && (New.X <= MAXSWORD)) X = New.X;
	if ((New.Y >= -MAXSWORD) && (New.Y <= MAXSWORD)) Y = New.Y;
	if ((New.Z >= -MAXSWORD) && (New.Z <= MAXSWORD)) Z = New.Z;
	};

/////////////////////
// FVector friends //
/////////////////////

//
// Compare two points and see if they're the same, using a threshold.
// Returns 1=yes, 0=no.  Uses fast distance approximation.
//
int inline FPointsAreSame (const FVector &P, const FVector &Q)
	{
	FLOAT Temp;
	Temp=P.X-Q.X;
	if ((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
		{
		Temp=P.Y-Q.Y;
		if ((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
			{
			Temp=P.Z-Q.Z;
			if ((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
				{
				return 1;
				};
			};
		};
	return 0;
	};

//
// Compare two points and see if they're the same, using a threshold.
// Returns 1=yes, 0=no.  Uses fast distance approximation.
//
int inline FPointsAreNear (const FVector &Point1,const FVector &Point2, FLOAT Dist)
	{
	FLOAT Temp;
	Temp=(Point1.X - Point2.X); if (OurAbs(Temp)>=Dist) return 0;
	Temp=(Point1.Y - Point2.Y); if (OurAbs(Temp)>=Dist) return 0;
	Temp=(Point1.Z - Point2.Z); if (OurAbs(Temp)>=Dist) return 0;
	return 1;
	};

//
// Calculate the signed distance (in the direction of the normal) between
// a point and a plane.
//
FLOAT inline FPointPlaneDist (
	const FVector &Point,
	const FVector &PlaneBase,
	const FVector &PlaneNormal)
	{
	return
		(
		(Point.X - PlaneBase.X)*PlaneNormal.X +
		(Point.Y - PlaneBase.Y)*PlaneNormal.Y +
		(Point.Z - PlaneBase.Z)*PlaneNormal.Z
		);
	};

//
// Find the weighted distance between a point and a plane based on elliptical
// scaling factors.  (1,1,1) = sphere, (1,1,0.5) = ellipse twice as high as it is wide.
//
FLOAT inline FEllipsePlaneDist (
	const FVector &Point,
	const FVector &Weight,
	const FVector &PlaneOrigin,
	const FVector &PlaneNormal)
	{
	return
		(
		(Point.X - PlaneOrigin.X) * PlaneNormal.X * Weight.X +
		(Point.Y - PlaneOrigin.Y) * PlaneNormal.Y * Weight.Y +
		(Point.Z - PlaneOrigin.Z) * PlaneNormal.Z * Weight.Z
		);
	};

//
// Find the intersection of an infinite line (defined by two points) and
// a plane.  Assumes that the line and plane do indeed intersect; you must
// make sure they're not parallel before calling.
//
FVector inline FLinePlaneIntersection (
	const FVector &Point1,
	const FVector &Point2,
	const FVector &PlaneOrigin,
	const FVector &PlaneNormal)
	{
	FVector	Delta,Intersection;
	FLOAT	Time,TimeNumerator,TimeDenominator;
	//
	Delta.X = Point2.X - Point1.X;
	Delta.Y = Point2.Y - Point1.Y;
	Delta.Z = Point2.Z - Point1.Z;
	//
	TimeNumerator=
		((PlaneOrigin.X - Point1.X) * PlaneNormal.X +
		( PlaneOrigin.Y - Point1.Y) * PlaneNormal.Y +
		( PlaneOrigin.Z - Point1.Z) * PlaneNormal.Z);
	TimeDenominator=
		((Delta.X * PlaneNormal.X) +
		( Delta.Y * PlaneNormal.Y) +
		( Delta.Z * PlaneNormal.Z));
	if (TimeDenominator==0) Time=0.5; // Prevent divide by zero
	else Time=TimeNumerator/TimeDenominator;
	//
	Intersection.X = Point1.X + Time*Delta.X;
	Intersection.Y = Point1.Y + Time*Delta.Y;
	Intersection.Z = Point1.Z + Time*Delta.Z;
	//
	return Intersection;
	};

//
// Euclidean distance between two points.
//
FLOAT inline FDist (const FVector &V1, const FVector &V2)
	{
	FLOAT DX,DY,DZ;
	//
	DX = V2.X - V1.X;
	DY = V2.Y - V1.Y;
	DZ = V2.Z - V1.Z;
	//
	return sqrt (DX*DX+DY*DY+DZ*DZ);
	};

//
// Squared distance between two points.
//
FLOAT inline FDistSquared (const FVector &V1, const FVector &V2)
	{
	FLOAT DX,DY,DZ;
	//
	DX = V2.X - V1.X;
	DY = V2.Y - V1.Y;
	DZ = V2.Z - V1.Z;
	//
	return DX*DX + DY*DY + DZ*DZ;
	};

//
// Returns distance plus or minus 8% error, from Jack Ritter, Graphics Gems I.
//
FLOAT inline FDistApprox (const FVector &V1, const FVector &V2) 
	{
	FLOAT	Min,Med,Max,Temp;
	// 
	Min = OurAbs(V1.X - V2.X);
	Med = OurAbs(V1.Y - V2.Y);
	Max = OurAbs(V1.Z - V2.Z);
	//
	if (Max<Med) {Temp = Max; Max = Med; Med = Temp;};
	if (Max<Min) {Temp = Max; Max = Min; Min = Temp;};
	//
	return Max + (11.0/32.0)*Med + (1.0/4.0)*Min;
	};

//
// The midpoint of two points.
//
FVector inline FMidpoint (const FVector &V1, const FVector &V2)
	{
	FVector Result;
	Result.X = 0.5 * (V1.X + V2.X);
	Result.Y = 0.5 * (V1.Y + V2.Y);
	Result.Z = 0.5 * (V1.Z + V2.Z);
	return Result;
	};

//
// See if two normal vectors (or plane normals) are nearly parallel.
//
int inline FParallel (const FVector &Normal1, const FVector &Normal2)
	{
	FLOAT NormalDot = Normal1 | Normal2;
	return (OurAbs (NormalDot - 1.0) <= THRESH_VECTORS_ARE_PARALLEL);
	};

//
// See if two planes are coplanar.
//
int inline FCoplanar (const FVector &Base1, const FVector &Normal1, const FVector &Base2, const FVector &Normal2)
	{
	if      (!FParallel(Normal1,Normal2)) return 0;
	else if (FPointPlaneDist (Base2,Base1,Normal1) > THRESH_POINT_ON_PLANE) return 0;
	else    return 1;
	};

/*-----------------------------------------------------------------------------
	FCoords implementation
-----------------------------------------------------------------------------*/

///////////////////////
// FCoords Functions //
///////////////////////

//
// Make a coordinate system out of an origin point and three axis vectors.
//
FCoords inline FCoords::Make(FVector COrigin, FVector CXAxis, FVector CYAxis, FVector CZAxis)
	{
	Origin     = COrigin;
	XAxis      = CXAxis;
	YAxis      = CYAxis;
	ZAxis      = CZAxis;
	return *this;
	};

//
// Return the transposition of this coordinate system.
//
FCoords inline FCoords::Transposition(void)
	{
	FCoords Result;
	//
	Result.XAxis.X = XAxis.X;
	Result.XAxis.Y = YAxis.X;
	Result.XAxis.Z = ZAxis.X;
	//
	Result.YAxis.X = XAxis.Y;
	Result.YAxis.Y = YAxis.Y;
	Result.YAxis.Z = ZAxis.Y;
	//
	Result.ZAxis.X = XAxis.Z;
	Result.ZAxis.Y = YAxis.Z;
	Result.ZAxis.Z = ZAxis.Z;
	//
	return Result;
	};

//
// Transform a coordinate system's vectors by another coordinate
// system's vectors.  Disregards all origins.
//
void inline FCoords::TransformByCoords (const FCoords &TransformCoords)
	{
	XAxis.TransformVector (TransformCoords);
	YAxis.TransformVector (TransformCoords);
	ZAxis.TransformVector (TransformCoords);
	};

/////////////////////////////
// FCoords Transformations //
/////////////////////////////

//
// Apply a yaw/pitch/roll transformation to a coordinate system.  Slow and
// unoptimized.  This is okay, since it's called infrequently.
//
void inline FCoords::TransformByRotation (const FRotation &Rot)
	{
	FCoords TempCoords;
	//
	// * Rule of thumb for pitch, yaw, roll components: When a component's
	// * value is zero, the corresponding matrix should be the identity matrix.
	// * All of these matrices should always be orthogonal.
	// * Do DeTransforms in reverse order (roll, pitch, yaw)
	//
	// Apply yaw rotation:
	//
	TempCoords.XAxis.X = + GMath.CosTab(Rot.Yaw);
	TempCoords.XAxis.Y = + GMath.SinTab(Rot.Yaw);
	TempCoords.XAxis.Z = + 0.0;
	//
	TempCoords.YAxis.X = - GMath.SinTab(Rot.Yaw);
	TempCoords.YAxis.Y = + GMath.CosTab(Rot.Yaw);
	TempCoords.YAxis.Z = + 0.0;
	//
	TempCoords.ZAxis.X = + 0.0;
	TempCoords.ZAxis.Y = + 0.0;
	TempCoords.ZAxis.Z = + 1.0;
	//
	TransformByCoords (TempCoords);
	//
	// Apply pitch rotation:
	//
	TempCoords.XAxis.X = + GMath.CosTab(Rot.Pitch);
	TempCoords.XAxis.Y = + 0.0;
	TempCoords.XAxis.Z = + GMath.SinTab(Rot.Pitch);
	//
	TempCoords.YAxis.X = + 0.0;
	TempCoords.YAxis.Y = + 1.0;
	TempCoords.YAxis.Z = + 0.0;
	//
	TempCoords.ZAxis.X = - GMath.SinTab(Rot.Pitch);
	TempCoords.ZAxis.Y = + 0.0;
	TempCoords.ZAxis.Z = + GMath.CosTab(Rot.Pitch);
	//
	TransformByCoords (TempCoords);
	//
	// Apply roll rotation:
	//
	TempCoords.XAxis.X = + 1.0;
	TempCoords.XAxis.Y = + 0.0;
	TempCoords.XAxis.Z = + 0.0;
	//
	TempCoords.YAxis.X = + 0.0;
	TempCoords.YAxis.Y = + GMath.CosTab(Rot.Roll);
	TempCoords.YAxis.Z = - GMath.SinTab(Rot.Roll);
	//
	TempCoords.ZAxis.X = + 0.0;
	TempCoords.ZAxis.Y = + GMath.SinTab(Rot.Roll);
	TempCoords.ZAxis.Z = + GMath.CosTab(Rot.Roll);
	//
	TransformByCoords (TempCoords);
	};

//
// Apply a yaw/pitch/roll detransformation to a coordinate system.  Slow and
// unoptimized.  This is okay, since it's only called 1-2x per frame.
//
void inline FCoords::DeTransformByRotation (const FRotation &Rot)
	{
	FCoords TempCoords;
	//
	// These rotation matrices are just the negative pitch, yaw, and roll matrices.
	//
	// Apply inverse roll rotation:
	//
	TempCoords.XAxis.X = + 1.0;
	TempCoords.XAxis.Y = - 0.0;
	TempCoords.XAxis.Z = + 0.0;
	//
	TempCoords.YAxis.X = - 0.0;
	TempCoords.YAxis.Y = + GMath.CosTab(Rot.Roll);
	TempCoords.YAxis.Z = + GMath.SinTab(Rot.Roll);
	//
	TempCoords.ZAxis.X = + 0.0;
	TempCoords.ZAxis.Y = - GMath.SinTab(Rot.Roll);
	TempCoords.ZAxis.Z = + GMath.CosTab(Rot.Roll);
	//
	TransformByCoords (TempCoords);
	//
	// Apply inverse pitch rotation:
	//
	TempCoords.XAxis.X = + GMath.CosTab(Rot.Pitch);
	TempCoords.XAxis.Y = + 0.0;
	TempCoords.XAxis.Z = - GMath.SinTab(Rot.Pitch);
	//
	TempCoords.YAxis.X = + 0.0;
	TempCoords.YAxis.Y = + 1.0;
	TempCoords.YAxis.Z = - 0.0;
	//
	TempCoords.ZAxis.X = - -GMath.SinTab(Rot.Pitch);
	TempCoords.ZAxis.Y = + 0.0;
	TempCoords.ZAxis.Z = + GMath.CosTab(Rot.Pitch);
	//
	TransformByCoords (TempCoords);
	//
	// Apply inverse yaw rotation:
	//
	TempCoords.XAxis.X = + GMath.CosTab(Rot.Yaw);
	TempCoords.XAxis.Y = - GMath.SinTab(Rot.Yaw);
	TempCoords.XAxis.Z = - 0.0;
	//
	TempCoords.YAxis.X = + GMath.SinTab(Rot.Yaw);
	TempCoords.YAxis.Y = + GMath.CosTab(Rot.Yaw);
	TempCoords.YAxis.Z = + 0.0;
	//
	TempCoords.ZAxis.X = - 0.0;
	TempCoords.ZAxis.Y = + 0.0;
	TempCoords.ZAxis.Z = + 1.0;
	//
	TransformByCoords (TempCoords);
	};

//
// Note: Will return coordinate system of opposite handedness if
// Scale.X*Scale.Y*Scale.Z is negative.  You must check for this
// yourself, i.e. to prevent polygons vertices of the wrong clockness.
//
void inline FCoords::TransformByScale (const FScale &Scale)
	{
	FCoords TempCoords;
	FLOAT Sheer = FSheerSnap (Scale.SheerRate);
	//
	// Apply sheering
	//
	TempCoords = GMath.UnitCoords;
	switch (Scale.SheerAxis)
		{
		case SHEER_XY:
			TempCoords.XAxis.Y = Sheer;
			break;
		case SHEER_XZ:
			TempCoords.XAxis.Z = Sheer;
			break;
		case SHEER_YX:
			TempCoords.YAxis.X = Sheer;
			break;
		case SHEER_YZ:
			TempCoords.YAxis.Z = Sheer;
			break;
		case SHEER_ZX:
			TempCoords.ZAxis.X = Sheer;
			break;
		case SHEER_ZY:
			TempCoords.ZAxis.Y = Sheer;
			break;
		default: // SHEER_NONE
			break;
		};
	TransformByCoords(TempCoords);
	//
	// Apply scaling
	//
	TempCoords = GMath.UnitCoords;
	TempCoords.XAxis.X *= Scale.Scale.X;
	TempCoords.YAxis.Y *= Scale.Scale.Y;
	TempCoords.ZAxis.Z *= Scale.Scale.Z;
	TransformByCoords(TempCoords);
	};

//
// Inversely transform a coordinate system by a scale.
//
void inline FCoords::DeTransformByScale (const FScale &Scale)
	{
	FCoords TempCoords;
	FLOAT	Sheer = FSheerSnap (Scale.SheerRate);
	//
	// Deapply scaling
	//
	TempCoords = GMath.UnitCoords;
	if (Scale.Scale.X!=0) TempCoords.XAxis.X /= Scale.Scale.X;
	if (Scale.Scale.Y!=0) TempCoords.YAxis.Y /= Scale.Scale.Y;
	if (Scale.Scale.Z!=0) TempCoords.ZAxis.Z /= Scale.Scale.Z;
	TransformByCoords(TempCoords);
	//
	// Deapply sheering
	//
	TempCoords = GMath.UnitCoords;
	switch (Scale.SheerAxis)
		{
		case SHEER_XY:
			TempCoords.XAxis.Y = -Sheer;
			break;
		case SHEER_XZ:
			TempCoords.XAxis.Z = -Sheer;
			break;
		case SHEER_YX:
			TempCoords.YAxis.X = -Sheer;
			break;
		case SHEER_YZ:
			TempCoords.YAxis.Z = -Sheer;
			break;
		case SHEER_ZX:
			TempCoords.ZAxis.X = -Sheer;
			break;
		case SHEER_ZY:
			TempCoords.ZAxis.Y = -Sheer;
			break;
		default: // SHEER_NONE
			break;
		};
	TransformByCoords(TempCoords);
	};

//
// Mirror a coordinate system about a normal vector.  MirrorNormal must
// have a length of 1 to avoid distortion.
//
FCoords inline FCoords::Mirror (const FVector &MirrorNormal) const
	{
	FCoords Result;
	Result.XAxis  = XAxis.Mirror(MirrorNormal);
	Result.YAxis  = YAxis.Mirror(MirrorNormal);
	Result.ZAxis  = ZAxis.Mirror(MirrorNormal);
	Result.Origin = Origin; // Should mirror
	return Result;
	};

/*-----------------------------------------------------------------------------
	FRotation operators
-----------------------------------------------------------------------------*/

///////////////////////
// FVector Operators //
///////////////////////

FRotation inline FRotation::operator+= (const FRotation &R)
	{
	Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
	return *this;
	};

FRotation inline FRotation::operator-= (const FRotation &R)
	{
	Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
	return *this;
	};

FRotation inline FRotation::operator+ (const FRotation &R) const
	{
	FRotation Temp;
	Temp.Pitch = Pitch + R.Pitch; Temp.Yaw = Yaw + R.Yaw; Temp.Roll = Roll + R.Roll;
	return Temp;
	};

FRotation inline FRotation::operator- (const FRotation &R) const
	{
	FRotation Temp;
	Temp.Pitch = Pitch - R.Pitch; Temp.Yaw = Yaw - R.Yaw; Temp.Roll = Roll - R.Roll;
	return Temp;
	};

FRotation inline FRotation::operator- (void) const
	{
	FRotation Temp;
	Temp.Pitch = -Pitch; Temp.Yaw = -Yaw; Temp.Roll = -Roll;
	return Temp;
	};

FRotation inline FRotation::operator* (FLOAT Scale) const
	{
	FRotation Temp;
	Temp.Pitch = Scale * (FLOAT)(SWORD)Pitch;
	Temp.Yaw   = Scale * (FLOAT)(SWORD)Yaw;
	Temp.Roll  = Scale * (FLOAT)(SWORD)Roll;
	return Temp;
	};

FRotation inline operator* (FLOAT Scale,const FRotation &R)
	{
	FRotation Temp;
	Temp.Pitch = Scale * (FLOAT)(SWORD)R.Pitch;
	Temp.Yaw   = Scale * (FLOAT)(SWORD)R.Yaw;
	Temp.Roll  = Scale * (FLOAT)(SWORD)R.Roll;
	return Temp;
	};

FRotation inline FRotation::operator*= (FLOAT Scale)
	{
	Pitch = Scale * (FLOAT)(SWORD)Pitch;
	Yaw   = Scale * (FLOAT)(SWORD)Yaw;
	Roll  = Scale * (FLOAT)(SWORD)Roll;
	return *this;
	};

int inline FRotation::operator== (const FRotation &V) const
	{
	return (Pitch==V.Pitch) && (Yaw==V.Yaw) && (Roll==V.Roll);
	};

int inline FRotation::operator!= (const FRotation &V) const
	{
	return (Pitch!=V.Pitch) || (Yaw!=V.Yaw) || (Roll!=V.Roll);
	};

/*-----------------------------------------------------------------------------
	FRotation class functions
-----------------------------------------------------------------------------*/

//
// Make a rotation from a pitch, yaw, and roll.
//
FRotation inline FRotation::Make(WORD CPitch,WORD CYaw,WORD CRoll)
	{
	Pitch	= CPitch;
	Yaw		= CYaw;
	Roll	= CRoll;
	return *this;
	};

//
// Add signed pitch, yaw, and roll values to this rotation.
// Since the components are WORD's, they automatically wrap around
// as they should.
//
void inline FRotation::Add (INT DeltaPitch, INT DeltaYaw, INT DeltaRoll)
	{
	Yaw   += DeltaYaw;
	Pitch += DeltaPitch;
	Roll  += DeltaRoll;
	};

//
// Add signed pitch, yaw, and roll values to this rotation, making
// sure the resulting camera view rotation doesn't become disorienting.
//
void inline FRotation::AddBounded (INT DeltaPitch, INT DeltaYaw, INT DeltaRoll)
	{
	Yaw  += (WORD)DeltaYaw;
	Pitch = FAddAngleConfined(Pitch,DeltaPitch,192*0x100,64*0x100);
	Roll  = FAddAngleConfined(Roll, DeltaRoll, 192*0x100,64*0x100);
	};

//
// Snap this rotation to a rotational grid value.
//
FRotation inline FRotation::GridSnap (const FRotation &RotGrid)
	{
	FRotation Result;
	Result.Pitch = FSnap (Pitch,RotGrid.Pitch);
	Result.Yaw   = FSnap (Yaw,  RotGrid.Yaw);
	Result.Roll  = FSnap (Roll, RotGrid.Roll);
	return Result;
	};

/*-----------------------------------------------------------------------------
	FScale class functions
-----------------------------------------------------------------------------*/

//
// Make a scale from a vector, sheer rate, and sheer axis.
//
FScale inline FScale::Make(FVector CScale,FLOAT CSheerRate,ESheerAxis CSheerAxis)
	{
	Scale		= CScale;
	SheerRate	= CSheerRate;
	SheerAxis   = CSheerAxis;
	return *this;
	};

//
// Return orientation of a scale transform (1 = regular, -1 = opposite clockness,
// 0=collapsed).
//
FLOAT inline FScale::Orientation (void)
	{
	FLOAT Temp = Scale.X * Scale.Y * Scale.Z;
	return OurSgn(Temp);
	};

//
// Calculate view coordinate system for a camera.
//
void inline FCalcViewCoords (const FVector &Location, const FRotation &Rotation,
	FCoords *Coords, FCoords *Uncoords)
	{
	*Coords = GMath.CameraViewCoords;
	Coords->DeTransformByRotation (Rotation);
	Coords->Origin = Location;
	//
	if (Uncoords) // Compute Screen->World coordinates (orthogonal inverse = transpose)
		{
		Uncoords->XAxis.X = Coords->XAxis.X;
		Uncoords->XAxis.Y = Coords->YAxis.X;
		Uncoords->XAxis.Z = Coords->ZAxis.X;
		//
		Uncoords->YAxis.X = Coords->XAxis.Y;
		Uncoords->YAxis.Y = Coords->YAxis.Y;
		Uncoords->YAxis.Z = Coords->ZAxis.Y;
		//
		Uncoords->ZAxis.X = Coords->XAxis.Z;
		Uncoords->ZAxis.Y = Coords->YAxis.Z;
		Uncoords->ZAxis.Z = Coords->ZAxis.Z;
		//
		Uncoords->Origin  = Location * -1;
		//
		Uncoords->Origin.TransformVector(*Coords);
		};
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNMATH
