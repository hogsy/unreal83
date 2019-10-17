/*=============================================================================
	UnScrRTL.cpp: UnrealScript runtime library implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	Description

	This contains the implementation of intrinsic UnrealScript functions and
	operators which are general purpose and non-Unreal specific.

	Verify:
	- Function names
	- Parameters
	- AActor

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Integer operators
-----------------------------------------------------------------------------*/

class PBinaryIntOper : public PMessageParams {
	public:
	int A;
	int B;
	int Result;
	};

class PUnaryIntOper : public PMessageParams {
	public:
	int A;
	int Result;
	};

class PFunctionStr : public PMessageParams {
	public:
	int A;
	char Result[];
	};

class PFunctionReal : public PMessageParams {
	public:
	int A;
	FLOAT Result;
	};

// Operator +    (A as Integer               ) as Integer   Intrinsic Fast
void OperatorPlus_I_I(AActor *Actor,PUnaryIntOper *Params) {
	Params->Result = Params->A;
	};

// Operator -    (A as Integer               ) as Integer   Intrinsic Fast
void OperatorMinus_I_I(AActor *Actor,PUnaryIntOper *Params) {
	Params->Result = -Params->A;
	};

// Operator Not  (A as Integer               ) as Integer   Intrinsic Fast
void OperatorNot_I_I(AActor *Actor,PUnaryIntOper *Params) {
	Params->Result = !Params->A;
	};

// Operator +    (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorPlus_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A + Params->B;
	};

// Operator -    (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorMinus_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A - Params->B;
	};

// Operator *    (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorTimes_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A * Params->B;
	};

// Operator /    (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorDivide_II_I(AActor *Actor,PBinaryIntOper *Params) {
	if (Params->B!=0) Params->Result = Params->A / Params->B;
	else              Params->Result = 0;
	};

// Operator <    (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorLess_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A < Params->B;
	};

// Operator >    (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorGreater_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A > Params->B;
	};

// Operator <=   (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorLessEquals_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A <= Params->B;
	};

// Operator >=   (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorGreaterEquals_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A <= Params->B;
	};

// Operator =    (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorEquals_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A == Params->B;
	};

// Operator <>   (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorLessGreater_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A != Params->B;
	};

// Operator And  (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorAnd_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A && Params->B;
	};

// Operator Or   (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorOr_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A || Params->B;
	};

// Operator BitAnd(A as Integer, B as Integer) as Integer   Intrinsic Fast
void OperatorBitAnd_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A & Params->B;
	};

// Operator BitOr(A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorBitOr_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A | Params->B;
	};

// Operator BitXor(A as Integer, B as Integer) as Integer   Intrinsic Fast
void OperatorBitXor_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A ^ Params->B;
	};

// Operator Mod  (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorMod_II_I(AActor *Actor,PBinaryIntOper *Params) {
	if (Params->B != 0) Params->Result = Params->A % Params->B;
	else                Params->Result = 0;
	};

// Operator <<   (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorLessLess_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A << Params->B;
	};

// Operator >>   (A as Integer, B as Integer ) as Integer   Intrinsic Fast
void OperatorGreaterGreater_II_I(AActor *Actor,PBinaryIntOper *Params) {
	Params->Result = Params->A >> Params->B;
	};

// Function Sqrt (A as Integer               ) as Integer   Intrinsic Fast
void FunctionSqrt_I_I(AActor *Actor,PUnaryIntOper *Params) {
	if (Params->A >= 0) Params->Result = (int)sqrt((FLOAT)Params->A);
	else                Params->Result = 0;
	};

// Function Str  (A as Integer               ) as String    Intrinsic Fast
void FunctionStr_I_S(AActor *Actor,PFunctionStr *Params) {
	itoa(Params->A,Params->Result,10);
	};

// Function Real (A as Integer               ) as Real      Intrinsic Fast
void FunctionAbs_I_R(AActor *Actor,PFunctionReal *Params) {
	Params->Result = (FLOAT)Params->A;
	};

// Function RandomInt(Max as Integer         ) as Integer   Intrinsic Fast
void FunctionRandomInt_I_I(AActor *Actor,PUnaryIntOper *Params) {
	if (Params->A > 0) Params->Result = rand() % Params->A;
	else               Params->Result = 0;
	};

/*-----------------------------------------------------------------------------
	Real operators and functions
-----------------------------------------------------------------------------*/

class PBinaryRealOper : public PMessageParams {
	public:
	FLOAT A;
	FLOAT B;
	FLOAT Result;
	};

class PUnaryRealOper : public PMessageParams {
	public:
	FLOAT A;
	FLOAT Result;
	};

class PBinaryRealBooleanOper : public PMessageParams {
	public:
	FLOAT A;
	FLOAT B;
	int Result;
	};

class PFunctionStrReal : public PMessageParams {
	public:
	FLOAT A;
	char Result[];
	};

class PFunctionInt : public PMessageParams {
	public:
	FLOAT A;
	int Result;
	};

class PReal : public PMessageParams {
	public:
	FLOAT Result;
	};

// Operator +    (A as Real                  ) as Real      Intrinsic Fast
void OperatorPlus_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = Params->A;
	};

// Operator -    (A as Real                  ) as Real      Intrinsic Fast
void OperatorMinus_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = -Params->A;
	};

// Operator +    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorPlus_RR_R(AActor *Actor,PBinaryRealOper *Params) {
	Params->Result = Params->A + Params->B;
	};

// Operator -    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorMinus_RR_R(AActor *Actor,PBinaryRealOper *Params) {
	Params->Result = Params->A - Params->B;
	};

// Operator *    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorTimes_RR_R(AActor *Actor,PBinaryRealOper *Params) {
	Params->Result = Params->A * Params->B;
	};

// Operator /    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorDivide_RR_R(AActor *Actor,PBinaryRealOper *Params) {
	Params->Result = Params->A / Params->B;
	};

// Operator ^    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorBitXor_RR_R(AActor *Actor,PBinaryRealOper *Params) {
	Params->Result = exp(Params->B * log(Params->A)); // a^b = exp(b*log(a)
	};

// Operator <    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorLess_RR_I(AActor *Actor,PBinaryRealBooleanOper *Params) {
	Params->Result = Params->A < Params->B;
	};

// Operator >    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorGreater_RR_I(AActor *Actor,PBinaryRealBooleanOper  *Params) {
	Params->Result = Params->A > Params->B;
	};

// Operator <=   (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorLessEquals_RR_I(AActor *Actor,PBinaryRealBooleanOper  *Params) {
	Params->Result = Params->A <= Params->B;
	};

// Operator >=   (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorGreaterEquals_RR_I(AActor *Actor,PBinaryRealBooleanOper  *Params) {
	Params->Result = Params->A >= Params->B;
	};

// Operator =    (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorEquals_RR_I(AActor *Actor,PBinaryRealBooleanOper  *Params) {
	Params->Result = Params->A == Params->B;
	};

// Operator <>   (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorLessGreater_RR_I(AActor *Actor,PBinaryRealBooleanOper  *Params) {
	Params->Result = Params->A != Params->B;
	};

// Operator Mod  (A as Real, B as Real       ) as Real      Intrinsic Fast
void OperatorMod_RR_R(AActor *Actor,PBinaryRealOper *Params) {
	Params->Result = fmod(Params->A,Params->B);
	};

// Function Abs  (A as Real                  ) as Real      Intrinsic Fast
void FunctionAbs_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = OurAbs(Params->A);
	};

// Function Sqrt (A as Real                  ) as Real      Intrinsic Fast
void FunctionSqrt_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = sqrt(Params->A);
	};

// Function Str  (A as Real                  ) as String    Intrinsic Fast
void FunctionStr_R_S(AActor *Actor,PFunctionStrReal *Params) {
	sprintf(Params->Result,"%f",Params->A);
	};

// Function Int  (A as Real                  ) as Integer   Intrinsic Fast
void FunctionInt_R_I(AActor *Actor,PFunctionInt *Params) {
	Params->Result = (int)Params->A;
	};

// Function Sin  (A as Real                  ) as Real      Intrinsic Fast
void FunctionSin_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = sin(Params->A);
	};

// Function Cos  (A as Real                  ) as Real      Intrinsic Fast
void FunctionCos_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = cos(Params->A);
	};

// Function Tan  (A as Real                  ) as Real      Intrinsic Fast
void FunctionTan_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = tan(Params->A);
	};

// Function Atn  (A as Real                  ) as Real      Intrinsic Fast
void FunctionAtn_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = atan(Params->A);
	};

// Function Exp  (A as Real                  ) as Real      Intrinsic Fast
void FunctionExp_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = exp(Params->A);
	};

// Function Log  (A as Real                  ) as Real      Intrinsic Fast
void FunctionLog_R_R(AActor *Actor,PUnaryRealOper *Params) {
	Params->Result = log(Params->A);
	};

// Function RandomReal(                      ) as Real      Intrinsic Fast
void FunctionSin_R_R(AActor *Actor,PReal *Params) {
	Params->Result = (FLOAT)rand()/(FLOAT)RAND_MAX;
	};

/*-----------------------------------------------------------------------------
	String operators and functions
-----------------------------------------------------------------------------*/

class PBinaryStringOper : public PMessageParams {
	public:
	char		A[256];
	char		B[256];
	char Result	[256];
	};

class PBinaryStringBooleanOper : public PMessageParams {
	public:
	char		A[256];
	char		B[256];
	int			Result;
	};

class PStringInteger : public PMessageParams {
	public:
	char		A[256];
	int			Result;
	};

// Operator +    (A as String, B as String   ) as String    Intrinsic Fast
void OperatorPlus_SS_S(AActor *Actor,PBinaryStringOper *Params) {
	int NumToCopy = 256-strlen(Params->A)-strlen(Params->B);
	int NewLength = strlen(Params->A)+NumToCopy;
	//
	strcpy  (Params->Result,Params->A);
	strncpy (Params->Result+strlen(Params->A),Params->B,NumToCopy);
	Params->Result[NewLength]=0;
	};

// Operator <    (A as String, B as String   ) as Integer   Intrinsic Fast
void OperatorLess_SS_I(AActor *Actor,PBinaryStringBooleanOper *Params) {
	Params->Result = strcmp(Params->A,Params->B)<0;
	};

// Operator >    (A as String, B as String   ) as Integer   Intrinsic Fast
void OperatorGreater_SS_I(AActor *Actor,PBinaryStringBooleanOper *Params) {
	Params->Result = strcmp(Params->A,Params->B)>0;
	};

// Operator <=   (A as String, B as String   ) as Integer   Intrinsic Fast
void OperatorLessEquals_SS_I(AActor *Actor,PBinaryStringBooleanOper *Params) {
	Params->Result = strcmp(Params->A,Params->B)<=0;
	};

// Operator >=   (A as String, B as String   ) as Integer   Intrinsic Fast
void OperatorGreaterEquals_SS_I(AActor *Actor,PBinaryStringBooleanOper *Params) {
	Params->Result = strcmp(Params->A,Params->B)>=0;
	};

// Operator =    (A as String, B as String   ) as Integer   Intrinsic Fast
void OperatorEquals_SS_I(AActor *Actor,PBinaryStringBooleanOper *Params) {
	Params->Result = strcmp(Params->A,Params->B)==0;
	};

// Operator <>   (A as String, B as String   ) as Integer   Intrinsic Fast
void OperatorLessGreater_SS_I(AActor *Actor,PBinaryStringBooleanOper *Params) {
	Params->Result = strcmp(Params->A,Params->B)!=0;
	};

// Function Val  (A as String, B as String   ) as Real      Intrinsic Fast
void FuncVal_S_I(AActor *Actor,PStringInteger *Params) {
	Params->Result = atoi(Params->A);
	};

/*-----------------------------------------------------------------------------
	Vector operators and functions
-----------------------------------------------------------------------------*/

class PBinaryVectorOper : public PMessageParams {
	public:
	FVector A;
	FVector B;
	FVector Result;
	};

class PUnaryVectorOper : public PMessageParams {
	public:
	FVector A;
	FVector Result;
	};

class PBinaryVectorBooleanOper : public PMessageParams {
	public:
	FVector A;
	FVector B;
	int Result;
	};

class PFunctionStrVector : public PMessageParams {
	public:
	FVector A;
	char Result[];
	};

class PVectorRealVector : public PMessageParams {
	public:
	FVector A;
	FLOAT   B;
	FVector Result;
	};

class PRealVectorVector : public PMessageParams {
	public:
	FLOAT   A;
	FVector B;
	FVector Result;
	};

class PVectorReal : public PMessageParams {
	public:
	FVector A;
	FLOAT   Result;
	};

class PVectorVectorReal : public PMessageParams {
	public:
	FVector A;
	FVector B;
	FLOAT   Result;
	};

// Operator +    (A as Vector                ) as Vector    Intrinsic Fast
void OperatorPlus_V_V(AActor *Actor,PUnaryVectorOper *Params) {
	Params->Result = Params->A;
	};

// Operator -    (A as Vector                ) as Vector    Intrinsic Fast
void OperatorMinus_V_V(AActor *Actor,PUnaryVectorOper *Params) {
	Params->Result = -Params->A;
	};

// Operator +    (A as Vector, B as Vector   ) as Vector    Intrinsic Fast
void OperatorPlus_VV_V(AActor *Actor,PBinaryVectorOper *Params) {
	Params->Result = Params->A + Params->B;
	};

// Operator -    (A as Vector, B as Vector   ) as Vector    Intrinsic Fast
void OperatorMinus_VV_V(AActor *Actor,PBinaryVectorOper *Params) {
	Params->Result = Params->A - Params->B;
	};

// Operator *    (A as Vector, B as Real     ) as Vector    Intrinsic Fast
void OperatorTimes_VR_V(AActor *Actor,PVectorRealVector *Params) {
	Params->Result = Params->A * Params->B;
	};

// Operator *    (A as Real,   B as Vector   ) as Vector    Intrinsic Fast
void OperatorTimes_RV_V(AActor *Actor,PRealVectorVector *Params) {
	Params->Result = Params->A * Params->B;
	};

// Operator /    (A as Vector, B as Real     ) as Vector    Intrinsic Fast
void OperatorDivide_VR_V(AActor *Actor,PVectorRealVector *Params) {
	Params->Result = Params->A / Params->B;
	};

// Operator =    (A as Vector, B as Vector   ) as Integer   Intrinsic Fast
void OperatorEquals_VV_I(AActor *Actor,PBinaryVectorBooleanOper *Params) {
	Params->Result = Params->A == Params->B;
	};

// Operator <>   (A as Vector, B as Vector   ) as Integer   Intrinsic Fast
void OperatorLessGreater_VV_I(AActor *Actor,PBinaryVectorBooleanOper *Params) {
	Params->Result = Params->A != Params->B;
	};

// Function Size (A as Vector                ) as Real      Intrinsic Fast
void FunctionSize_V_I(AActor *Actor,PVectorReal *Params) {
	Params->Result = Params->A.Size();
	};

// Function Str  (A as Integer               ) as Real      Intrinsic Fast
void FunctionStr_V_S(AActor *Actor,PFunctionStrVector *Params) {
	sprintf(Params->Result,"(X=%f,Y=%f,Z=%f)",Params->A.X,Params->A.Y,Params->A.Z);
	};

// Function Dot  (A as Vector, B as Vector   ) as Real      Intrinsic Fast
void FunctionDot_VV_R(AActor *Actor,PVectorVectorReal *Params) {
	Params->Result = Params->A | Params->B;
	};

// Function Cross(A as Vector, B as Vector   ) as Vector    Intrinsic Fast
void FunctionCross_VV_V(AActor *Actor,PBinaryVectorOper *Params) {
	Params->Result = Params->A ^ Params->B;
	};

/*-----------------------------------------------------------------------------
	Rotation operators and functions
-----------------------------------------------------------------------------*/

class PBinaryRotationOper : public PMessageParams {
	public:
	FRotation A;
	FRotation B;
	FRotation Result;
	};

class PRotationVector : public PMessageParams {
	public:
	FRotation A;
	FVector   Result;
	};

class PRotation : public PMessageParams {
	public:
	FRotation Result;
	};

// Operator +    (A as Rotation, B as Rotation) as Rotation Intrinsic Fast
void OperatorPlus_TT_T(AActor *Actor,PBinaryRotationOper *Params) {
	Params->Result = Params->A + Params->B;
	};

// Operator -    (A as Rotation, B as Rotation) as Rotation Intrinsic Fast
void OperatorMinus_TT_T(AActor *Actor,PBinaryRotationOper *Params) {
	Params->Result = Params->A - Params->B;
	};

// Function GetXAxis (A as Rotation           ) as Vector   Intrinsic Fast
void FunctionGetXAxis_T_V(AActor *Actor,PRotationVector *Params) {
	FCoords Coords = GMath.UnitCoords;
	Coords.TransformByRotation(Params->A);
	Params->Result = Coords.XAxis;
	};

// Function GetYAxis (A as Rotation           ) as Vector   Intrinsic Fast
void FunctionGetYAxis_T_V(AActor *Actor,PRotationVector *Params) {
	FCoords Coords = GMath.UnitCoords;
	Coords.TransformByRotation(Params->A);
	Params->Result = Coords.YAxis;
	};

// Function GetZAxis (A as Rotation           ) as Vector   Intrinsic Fast
void FunctionGetZAxis_T_V(AActor *Actor,PRotationVector *Params) {
	FCoords Coords = GMath.UnitCoords;
	Coords.TransformByRotation(Params->A);
	Params->Result = Coords.ZAxis;
	};

// Function GetUnXAxis (A as Rotation         ) as Vector   Intrinsic Fast
void FunctionGetUnXAxis_T_V(AActor *Actor,PRotationVector *Params) {
	FCoords Coords = GMath.UnitCoords;
	Coords.DeTransformByRotation(Params->A);
	Params->Result = Coords.XAxis;
	};

// Function GetUnYAxis (A as Rotation         ) as Vector   Intrinsic Fast
void FunctionGetUnYAxis_T_V(AActor *Actor,PRotationVector *Params) {
	FCoords Coords = GMath.UnitCoords;
	Coords.DeTransformByRotation(Params->A);
	Params->Result = Coords.YAxis;
	};

// Function GetUnZAxis (A as Rotation         ) as Vector   Intrinsic Fast
void FunctionGetUnZAxis_T_V(AActor *Actor,PRotationVector *Params) {
	FCoords Coords = GMath.UnitCoords;
	Coords.DeTransformByRotation(Params->A);
	Params->Result = Coords.ZAxis;
	};

// Function RandomRotation (                  ) as Rotation Intrinsic Fast
void FunctionRandomRotation__T(AActor *Actor,PRotation *Params) {
	Params->Result.Pitch = rand();
	Params->Result.Yaw   = rand();
	Params->Result.Roll  = rand();
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/

