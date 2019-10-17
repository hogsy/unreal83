/*=============================================================================
	UnRaster.h: Rasterization template

	Copyright 1995 Epic MegaGames, Inc.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney

	Optimizations needed:
		* Convert textured poly rasterizer to assembly
=============================================================================*/

#ifndef _INC_UNRASTER // Prevent header from being included multiple times
#define _INC_UNRASTER

/*-----------------------------------------------------------------------------
	Generic templates for polygon pipeline
-----------------------------------------------------------------------------*/

template <class TRasterPointT> class TRasterLine
	{
	public:
	union
		{
		struct {TRasterPointT Start,End;};
		TRasterPointT Point[2]; // Start point and end point
		};
	inline void DrawFlatSpan(BYTE *CameraLine, BYTE Color) const
		{
		int Pixels = End.GetX() - Start.GetX();
		if (Pixels>0) mymemset(CameraLine + Start.GetX(), Color, Pixels);
		};
	};
template <class TRasterLineT,class TRasterPointT> class TRasterPoly
	{
	public:
	int				StartY;
	int				EndY;
	TRasterLineT	Lines[1];
	//
	inline void ForceForwardFace(void);
	inline void DrawFlat(const ICamera *Camera,BYTE Color,BYTE BorderColor);
	inline void Draw(const ICamera *Camera);
	};
template <class TRasterPointT,class TThisT> class TRasterSideSetup
	{
	public:
	int					DY;			// Y length
	TThisT				*Next;		// Next side in list, NULL=none
	TRasterPointT		P;			// Value at this point
	TRasterPointT		DP;			// X-derivative at this point
	};
template <class TRasterSideSetupT, class TRasterLineT, class TRasterPolyT, class TThisT, class TTransformT> class TRasterSetup
	{
	public:
	int					StartY;		// Starting Y value
	int					EndY;		// Ending Y value + 1
	void				*MemTop;	// Top of allocated memory
	TRasterSideSetupT	*LeftSide;	// Left side rasterization setup, NULL = none
	TRasterSideSetupT	*RightSide;	// Right side rasterization setup, NULL = none
	FMemPool			*Mem;		// Memory pool
	//
	// Functions:
	//
	inline void CalcBound(const TTransformT *Pts, int NumPts, FScreenBounds &BoxBounds);
	inline void Setup(const ICamera *Camera, const TTransformT *Pts, int NumPts, FMemPool *MemPool);
	inline void SetupCached(const ICamera *Camera, const TTransformT *Pts, int NumPts, FMemPool *TempPool, FMemPool *CachePool, TRasterSideSetupT **Cache);
	inline void Generate(TRasterPolyT *Raster) const;
	inline void Release(void)
		{
		GUARD;
		Mem->Release(MemTop);
		UNGUARD("TRasterSetup::Release");
		};
	};

/*-----------------------------------------------------------------------------
	Flat shaded polygon implementation
-----------------------------------------------------------------------------*/

class FRasterPoint
	{
	public:
	int X;
	inline int GetX(void) const {return X;};
	};
class FRasterLine : public TRasterLine<FRasterPoint>
	{
	public:
	inline void DrawSpan(const ICamera *Camera,BYTE *CameraLine) const
		{
		DrawFlatSpan(CameraLine,SelectColor);
		};
	inline int IsBackwards(void) {return Start.X > End.X;};
	};
class FRasterSideSetup : public TRasterSideSetup<FRasterPoint,FRasterSideSetup>
	{
	public:
	inline void SetupSide(const FTransform *P1, const FTransform *P2,int NewY)
		{
		FLOAT YAdjust	= (FLOAT)(NewY+1) - P1->ScreenY;
		FLOAT FloatDX 	= (P2->ScreenX - P1->ScreenX) / (P2->ScreenY - P1->ScreenY);
		//
		ftoi(DP.X,FloatDX);
		ftoi(P.X,P1->ScreenX + FloatDX * YAdjust);
		};
	inline void GenerateSide(FRasterLine *Line,int Offset,int TempDY) const
		{
		int TempFixX	= P.X;
		int TempFixDX	= DP.X;
		#ifndef ASM
		while (TempDY-- > 0)
			{
			(Line++)->Point[Offset].X  = UNFIX(TempFixX);
			TempFixX += TempFixDX;
			};
		#else
		FRasterPoint *Point = &Line->Point[Offset];
		if (TempDY>0) __asm 
			{
			//
			// 4-unrolled, 2 cycle rasterizer loop
			//
			mov eax,[TempFixX]
			push ebp
			mov ecx,[TempFixDX]
			mov ebx,eax
			mov edx,ecx
			sar eax,16
			mov edi,[Point]
			sar ecx,16
			mov esi,[TempDY]
			shl ebx,16
			mov ebp,edi
			shl edx,16
			add ebp,(SIZE FRasterLine)
			jmp RasterLoop
			;
			ALIGN 16
			RasterLoop: ; = 2 cycles per raster side
				add ebx,edx
				mov [edi],eax
				adc eax,ecx
				add ebx,edx
				;
				mov [ebp],eax
				adc eax,ecx
				mov [edi+2*(SIZE FRasterLine)],eax
				add ebx,edx
				;
				adc eax,ecx
				add edi,4*(SIZE FRasterLine)
				mov [ebp+2*(SIZE FRasterLine)],eax
				add ebx,edx
				;
				adc eax,ecx
				add ebp,4*(SIZE FRasterLine)
				;
				sub esi,4
				jg  RasterLoop
			pop ebp				
			};
		#endif
		};
	};
class FRasterPoly : public TRasterPoly<FRasterLine,FRasterPoint> {};
class FRasterSetup : public TRasterSetup<FRasterSideSetup,FRasterLine,FRasterPoly,class FRasterSetup,FTransform> {};

/*-----------------------------------------------------------------------------
	Texture coordinates polygon implementation
-----------------------------------------------------------------------------*/

class FRasterTexPoint
	{
	public:
	FLOAT	FloatX;
	FLOAT	FloatU;
	FLOAT	FloatV;
	FLOAT	FloatG;
	inline int GetX(void) const {return ftoi(FloatX);};
	};
class FRasterTexLine : public TRasterLine<FRasterTexPoint>
	{
	public:
	inline int IsBackwards(void) {return Start.FloatX > End.FloatX;};
	//
	// Should break up into PreDrawSpan and DrawSpan.  PreDrawSpan goes
	// through and transforms all floating points stuff into QWORD integer
	// texture coordinates (or packed MMX coordinates), with pipelined divides.
	// DrawSpan draws it.  This will help a lot on MMX, where floating point and
	// MMX don't mix well.
	//
	// Should optimize by generating ASM code for all possible combinations of UBits and VBits.
	//
	// Maximum allowable texture size is 1024x1024.
	//
	inline void DrawSpan(const ICamera *Camera,BYTE *CameraLine) const
		{
		if (Camera->ColorBytes!=1) return; //!! Temporary
		int StartX  = ftoi(Start.FloatX); if (StartX<0) StartX=0;
		int Pixels  = ftoi(End.FloatX) - StartX;
		if (Pixels>0)
			{
			FLOAT RLength	= 1.0/(End.FloatX - Start.FloatX);
			FLOAT XAdjust	= (FLOAT)StartX + 0.5 - Start.FloatX;
			//
			FLOAT FloatDU	= (End.FloatU - Start.FloatU) * RLength;
			FLOAT FloatDV	= (End.FloatV - Start.FloatV) * RLength;
			FLOAT FloatDG	= (End.FloatG - Start.FloatG) * RLength;
			//
			QWORD StartQ,IncQ;
			//
			StartQ =
				(
				((QWORD)(ftoi(Start.FloatG + XAdjust * FloatDG) & 0x3fff                   )) +
				((QWORD)(ftoi(Start.FloatV + XAdjust * FloatDV) & ((1<<(GBlit.VBits+16))-1)) << 16) +
				((QWORD)(ftoi(Start.FloatU + XAdjust * FloatDU) & 0xfffffffc               ) << (48-GBlit.UBits))
				);
			IncQ =
				(
				((QWORD)(ftoi(FloatDG) & 0xffff                   )) +
				((QWORD)(ftoi(FloatDV) & ((1<<(GBlit.VBits+16))-1)) << 16) +
				((QWORD)(ftoi(FloatDU) & 0xfffffffc               ) << (48-GBlit.UBits))
				);
			GRender.Raster256Table[GBlit.DrawKind](CameraLine + StartX, StartQ, IncQ, Pixels);
			};
		};
	};

class FRasterTexSideSetup : public TRasterSideSetup<FRasterTexPoint,FRasterTexSideSetup>
	{
	public:
	inline void SetupSide(const FTransTex *P1, const FTransTex *P2,int NewY)
		{
		FLOAT FloatRDY		= 1.0 / (P2->ScreenY - P1->ScreenY);
		FLOAT FloatYAdjust	= (FLOAT)(NewY+1) - P1->ScreenY;
		//
		DP.FloatX 			= (P2->ScreenX  - P1->ScreenX ) * FloatRDY;
		DP.FloatU			= (P2->U        - P1->U       ) * FloatRDY;
		DP.FloatV			= (P2->V        - P1->V       ) * FloatRDY;
		DP.FloatG			= (P2->G        - P1->G       ) * FloatRDY;
		//
		P.FloatX			= P1->ScreenX  + DP.FloatX * FloatYAdjust;
		P.FloatU			= P1->U        + DP.FloatU * FloatYAdjust;
		P.FloatV			= P1->V        + DP.FloatV * FloatYAdjust;
		P.FloatG			= P1->G        + DP.FloatG * FloatYAdjust;
		};
	inline void GenerateSide(FRasterTexLine *Line,int Offset,int TempDY) const
		{
		#ifdef ASM
		FRasterTexPoint *Point		= &Line->Point[Offset];
		const FRasterTexPoint *LP   = &P;
		const FRasterTexPoint *LDP  = &DP;
		__asm
			{
			// Optimized by Erik de Nieve
            mov edx,[TempDY]
            mov eax,[Point]
            mov ebx,[LP]
            mov ecx,[LDP]
            cmp edx,0
            jle Done
            ;
            fld [ecx]LDP.FloatG ; DG
            fld [ecx]LDP.FloatV ; DV
            fld [ecx]LDP.FloatU ; DU
            fld [ecx]LDP.FloatX ; DX
			;
            fld [ebx]LDP.FloatG ; Values
            fld [ebx]LDP.FloatV
            fld [ebx]LDP.FloatU
            fld [ebx]LDP.FloatX
            ;
            RasterLoop:  ; Stack 0-7 :  X U V G - dX dU dV dG
            ;
            fst     [eax]LDP.FloatX		; X			; XUVG
            fxch	st(1)
            fst     [eax]LDP.FloatU		; U			; UXVG
            fxch	st(2)
            fst     [eax]LDP.FloatV		; V			; VXUG
            fxch	st(3)
            fst     [eax]LDP.FloatG		; G			; GXUV
			;
            fadd	st,st(7)			; G += DG    ; GXUV
            fxch	st(3)
            fadd	st,st(6)			; V += DV    ; VXUG
            fxch	st(2)
            fadd	st,st(5)			; U += DU    ; UXVG
            fxch	st(1)
            fadd	st,st(4)			; X += DX    ; XUVG
            ;
            add eax,SIZE FRasterTexLine
            dec edx
            ;
            jne RasterLoop
            ;
            fcompp ; pop 8 registers in 4 cycles
            fcompp
            fcompp
            fcompp
            ;
            Done:
			};
		#else
			FLOAT TempFloatX	= P.FloatX;
			FLOAT TempFloatU	= P.FloatU;
			FLOAT TempFloatV	= P.FloatV;
			FLOAT TempFloatG	= P.FloatG;
			while (TempDY-- > 0)
				{
				Line->Point[Offset].FloatX = TempFloatX; TempFloatX += DP.FloatX;
				Line->Point[Offset].FloatU = TempFloatU; TempFloatU += DP.FloatU;
				Line->Point[Offset].FloatV = TempFloatV; TempFloatV += DP.FloatV;
				Line->Point[Offset].FloatG = TempFloatG; TempFloatG += DP.FloatG;
				Line++;
				};
		#endif
		};
	};
class FRasterTexPoly : public TRasterPoly<FRasterTexLine,FRasterTexPoint> {};
class FRasterTexSetup : public TRasterSetup<FRasterTexSideSetup,FRasterTexLine,FRasterTexPoly,class FRasterTexSetup,FTransTex> {};

class FGlobalRaster
	{
	public:
	//
	enum {MAX_RASTER_LINES	= 1024}; // Maximum scanlines in rasterization
	//
	class FRasterPoly *Raster; // Polygon rasterization data
	//
	void Init(void);
	void Exit(void);
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNRASTER
