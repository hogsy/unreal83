/*=============================================================================
	UnBlastC.cpp: C++ Texture blasting code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#define _DO_NOT_DEFINE_CLASS_ENUMS
#include "Unreal.h"
#include "UnRender.h"
#include "Root.h"

/*-----------------------------------------------------------------------------
	Compile options
-----------------------------------------------------------------------------*/

//#undef ASM /* To generate slow C code for everything */
//#define MMX

/*-----------------------------------------------------------------------------
	Types
-----------------------------------------------------------------------------*/

enum {LIGHT_XR=512}; /* Duplicated in UnRender.inc */
enum {LIGHT_X_TOGGLE=LIGHT_XR*4}; /* Duplicated in UnRender.inc */

typedef void (*POST_EFFECT)(void);
typedef void (*TEX_INNER)(int SkipIn,FTexLattice *T);

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

extern "C"
	{
	//
	// Globals this file:
	//
	DWORD		GLight[LIGHT_XR * 5];
	BYTE		*GBits;
	TEX_INNER	GTexInner=NULL;
	POST_EFFECT GEffect=NULL;
	//
	void PostMask_8P (void); void PostBlend_8P (void); void PostFire_8P (void);
	void PostMask_16P(void); void PostBlend_16P(void); void PostFire_16P(void);
	void PostMask_32P(void); void PostBlend_32P(void); void PostFire_32P(void);
	POST_EFFECT GEffectTable[5][FGlobalRender::DRAWRASTER_MAX] =
		{
			{NULL,	NULL,			NULL,			NULL},			// 0-bit
			{NULL,	PostMask_8P,	PostBlend_8P,	PostFire_8P},	// 8-bit
			{NULL,	PostMask_16P,	PostBlend_16P,	PostFire_16P},	// 16-bit
			{NULL,	NULL,			NULL,			NULL},			// 24-bit
			{NULL,	NULL,			NULL,			NULL},			// 32-bit
		};
	//
	// Texture mapper inner loop:
	//
	ASMVAR	QWORD	TMI_DTex;
	ASMVAR	DWORD	TMI_DiffLight;
	ASMVAR	DWORD	TMI_DiffDest;
	ASMVAR  DWORD	*TMI_ProcBase;
	ASMVAR  DWORD	*TMI_LineProcBase;
	ASMVAR  FTexLattice *TMI_TopLattice;
	ASMVAR  FTexLattice **TMI_LatticeBase;
	ASMVAR  FTexLattice **TMI_NextLatticeBase;
	ASMVAR	DWORD	TMI_Shader;
	ASMVAR	BYTE	*TMI_Dest;
	ASMVAR	BYTE	*TMI_FinalDest;
	ASMVAR	BYTE	*TMI_RectFinalDest;
	//
	ASMVAR	DWORD	TMI_ProcTable8P[],TMI_ProcTable16P[],TMI_ProcTable32P[];
	ASMVAR	DWORD	TMI_ModTable8P[],TMI_ModTable16P[],TMI_ModTable32P[];
	//
	// Texture mapper outer loop:
	//
	ASMVAR	BYTE	*TMO_Dest;
	ASMVAR	BYTE	*TMO_DestBits;
	ASMVAR	BYTE	**TMO_DestOrDestBitsPtr;
	ASMVAR  FSpan	*TMO_Span,*TMO_OrigSpan;
	ASMVAR  FSpan	*TMO_RectSpan;
	ASMVAR  FSpan	*TMO_NextRectSpan;
	ASMVAR  INT		TMO_Stride;
	//
	// Light mapper inner loop.
	//
	ASMVAR	FLOAT	*TLI_MeshFloat;
	ASMVAR	FLOAT	*TLI_Sinc;
	ASMVAR	DWORD	TLI_AddrMask;
	ASMVAR	DWORD	TLI_Temp;
	ASMVAR	DWORD	*TLI_ProcBase;
	ASMVAR  FTexLattice *TLI_TopLattice;
	ASMVAR	DWORD	*TLI_Dest;
	ASMVAR	DWORD	*TLI_DestEnd;
	ASMVAR	DWORD	TLI_ProcTable[];
	ASMVAR  DWORD	TLI_SkipIn;
	//
	ASMVAR	DWORD	*TLO_TopBase;
	ASMVAR	DWORD	*TLO_BotBase;
	ASMVAR	DWORD	*TLO_FinalDest;
	ASMVAR  FSpan	*TLO_RectSpan;
	ASMVAR  void	(*TLO_LightInnerProc);
	ASMVAR  FTexLattice **TLO_LatticeBase;
	//
	// TexRect outer loop
	//
	ASMVAR	INT		TRO_Y;
	ASMVAR	INT		TRO_SubRectEndY;
	ASMVAR	DWORD	**TRO_ThisLightBase;
	ASMVAR	FSpan	**TRO_SpanIndex;
	//
	// Light mapper mid loop:
	//
	ASMVAR	DWORD	TLM_GBlitInterX;
	ASMVAR	BYTE	TLM_GBlitInterXBits2;
	ASMVAR	DWORD	TRO_OuterProc;
	//
	void __cdecl TLM_8P_Unlit(void);
	void __cdecl TLM_8P_Lit(void);
	void __cdecl TRO_Outer8P(void);
	void __cdecl TRO_Outer16P(void);
	void __cdecl TRO_Outer32P(void);
	void __cdecl LightOuter(void);
	//
	void __cdecl LightVInterpolate_8P_1(void);
	void __cdecl LightVInterpolate_8P_2(void);
	void __cdecl LightVInterpolate_8P_4(void);
	};

#ifndef ASM
	void (*GLightInnerProc)(void);
#endif

/*-----------------------------------------------------------------------------
	Postprocessing effects
-----------------------------------------------------------------------------*/

//
// 8-bit color
//

// Blit with no postprocessing effects
void PostBlit_8P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		BYTE *Src		= &TMO_DestBits	[Span->Start];
		BYTE *Dest		= &TMO_Dest		[Span->Start];
		BYTE *DestEnd	= &TMO_Dest		[Span->End];
		while (Dest<DestEnd)
			{
			*Dest++ = *Src++;
			};
		Span = SpanNext;
		};
	};

// Blit with color 0 = see-through
void PostMask_8P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		BYTE *Src		= &TMO_DestBits	[Span->Start];
		BYTE *Dest		= &TMO_Dest		[Span->Start];
		BYTE *DestEnd	= &TMO_Dest		[Span->End];
		while (((int)Dest & 3) && (Dest<DestEnd))
			{
			if (*Src) *Dest = *Src; Src++; Dest++;
			};
		while ((Dest+3)<DestEnd)
			{
			if (*(DWORD *)Src)
				{
				if (Src[0]&&Src[1]&&Src[2]&&Src[3])
					{
					*(DWORD *)Dest = *(DWORD *)Src;
					}
				else
					{
					if (Src[0]) Dest[0]=Src[0];
					if (Src[1]) Dest[1]=Src[1];
					if (Src[2]) Dest[2]=Src[2];
					if (Src[3]) Dest[3]=Src[3];
					};
				};
			Src+=4;
			Dest+=4;
			};
		while (Dest<DestEnd)
			{
			if (*Src) *Dest = *Src; Src++; Dest++;
			};
		Span = SpanNext;
		};
	};

// Blit with 64K palette blend table
void PostBlend_8P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		BYTE *Src		= &TMO_DestBits	[Span->Start];
		BYTE *Dest		= &TMO_Dest		[Span->Start];
		BYTE *DestEnd	= &TMO_Dest		[Span->End];
		while (((int)Dest & 3) && (Dest<DestEnd))
			{
			*Dest = GBlit.BlendTable[(int)*Src+((int)*Dest<<8)]; Src++; Dest++;
			};
		while ((Dest+3)<DestEnd)
			{
			*Dest = GBlit.BlendTable[(int)*Src+((int)*Dest<<8)]; Src++; Dest++;
			*Dest = GBlit.BlendTable[(int)*Src+((int)*Dest<<8)]; Src++; Dest++;
			*Dest = GBlit.BlendTable[(int)*Src+((int)*Dest<<8)]; Src++; Dest++;
			*Dest = GBlit.BlendTable[(int)*Src+((int)*Dest<<8)]; Src++; Dest++;
			};
		while (Dest<DestEnd)
			{
			*Dest = GBlit.BlendTable[(int)*Src+((int)*Dest<<8)]; Src++; Dest++;
			};
		Span = SpanNext;
		};
	};

// Blit with remapping to a specified palette, i.e. for fire
void PostFire_8P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		BYTE *Src		= &TMO_DestBits	[Span->Start];
		BYTE *Dest		= &TMO_Dest		[Span->Start];
		BYTE *DestEnd	= &TMO_Dest		[Span->End];
		while (Dest < DestEnd)
			{
			*Dest++ = *Src++;
			};
		Span = SpanNext;
		};
	};

//
// 16-bit color
//

// Blit with no postprocessing effects
void PostBlit_16P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		BYTE *Src		= &TMO_DestBits	[Span->Start];
		BYTE *Dest		= &TMO_Dest		[Span->Start];
		BYTE *DestEnd	= &TMO_Dest		[Span->End];
		while (Dest<DestEnd)
			{
			*Dest++ = *Src++;
			};
		Span = SpanNext;
		};
	};

// Blit with color 0 = see-through
void PostMask_16P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		WORD *Src		= &((WORD*)TMO_DestBits	)[Span->Start];
		WORD *Dest		= &((WORD*)TMO_Dest		)[Span->Start];
		WORD *DestEnd	= &((WORD*)TMO_Dest		)[Span->End];
		while (((int)Dest & 3) && (Dest<DestEnd))
			{
			if (*Src) *Dest = *Src; Src++; Dest++;
			};
		while ((Dest+3)<DestEnd)
			{
			if (*(DWORD *)Src)
				{
				if (Src[0]&&Src[1]&&Src[2]&&Src[3])
					{
					*(DWORD *)Dest = *(DWORD *)Src;
					}
				else
					{
					if (Src[0]) Dest[0]=Src[0];
					if (Src[1]) Dest[1]=Src[1];
					if (Src[2]) Dest[2]=Src[2];
					if (Src[3]) Dest[3]=Src[3];
					};
				};
			Src+=4;
			Dest+=4;
			};
		while (Dest<DestEnd)
			{
			if (*Src) *Dest = *Src; Src++; Dest++;
			};
		Span = SpanNext;
		};
	};

// Blit with blending effects
void PostBlend_16P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		WORD *Src		= (WORD*)(((int)TMO_DestBits+Span->Start)<<1);
		WORD *Dest		= (WORD*)(((int)TMO_Dest    +Span->Start)<<1);
		WORD *DestEnd	= (WORD*)(((int)TMO_Dest    +Span->End  )<<1);
		//
		while (((int)Dest & 6) && (Dest<DestEnd))
			{
			*Dest = *Src & 0xfff0; Src++; Dest++;
			};
		while ((Dest+3)<DestEnd)
			{
			// Can do one DWORD at a time
			*Dest = *Src & 0xfff0; Src++; Dest++;
			*Dest = *Src & 0xfff0; Src++; Dest++;
			*Dest = *Src & 0xfff0; Src++; Dest++;
			*Dest = *Src & 0xfff0; Src++; Dest++;
			};
		while (Dest<DestEnd)
			{
			*Dest = *Src & 0xfff0; Src++; Dest++;
			};
		Span = SpanNext;
		};
	};

// Blit with remapping to a specified palette, i.e. for fire
void PostFire_16P(void)
	{
	static FSpan *SpanNext;
	FSpan *Span = TMO_OrigSpan;
	while (Span)
		{
		SpanNext	= Span->Next;
		//
		BYTE *Src		= &TMO_DestBits	[Span->Start];
		BYTE *Dest		= &TMO_Dest		[Span->Start];
		BYTE *DestEnd	= &TMO_Dest		[Span->End];
		while (Dest < DestEnd)
			{
			*Dest++ = *Src++;
			};
		Span = SpanNext;
		};
	};

/*-----------------------------------------------------------------------------
	Setup
-----------------------------------------------------------------------------*/

void rendDrawAcrossSetup(ICamera *Camera, UTexture *Texture, DWORD ThesePolyFlags, DWORD NotPolyFlags)
	{
	GUARD;
	//
	// Set up globals:
	//
	if (Texture)	GBlit.Texture = Texture;
	else			GBlit.Texture = GGfx.DefaultTexture;
	//
	// Handle poly flags:
	//
	DWORD PolyFlags = (Texture->PolyFlags & ~NotPolyFlags) | ThesePolyFlags;
	//
	if (!(PolyFlags & (PF_Transparent | PF_Ghost | PF_Glow)))
		{
		if (!(PolyFlags & PF_Masked))			GBlit.DrawKind = FGlobalRender::DRAWRASTER_Normal;
		else									GBlit.DrawKind = FGlobalRender::DRAWRASTER_Masked;
		}
	else
		{
		GBlit.DrawKind = FGlobalRender::DRAWRASTER_Blended;
		//
		if		(PolyFlags & PF_Transparent)	GBlit.BlendKind = BLEND_Transparent;
		else if (PolyFlags & PF_Glow)			GBlit.BlendKind = BLEND_Glow;
		else									GBlit.BlendKind = BLEND_Ghost;
		//
		GBlit.BlendTable = GGfx.Blenders[GBlit.BlendKind];
		};
	GEffect = GEffectTable[Camera->ColorBytes][GBlit.DrawKind];
	//
	// Handle dithering:
	//
	if (GRender.DoDither && !(PolyFlags & PF_NoSmooth )) GBlit.DitherBase = &GDither256;
	else GBlit.DitherBase = &GNoDither256;
	//
	// Set up mipmapping:
	//
	GBlit.UBits = Texture->UBits;
	GBlit.VBits = Texture->VBits;
	//
	FBlitMipInfo *Mip = &GBlit.Mips[0];
	for (int i=0; i<8; i++)
		{
		int MipLevel = i;
		int USize,VSize;
		Mip->Data		= Texture->GetData(&MipLevel,Camera->ColorBytes,&USize,&VSize);
		Mip->MipLevel	= MipLevel;
		Mip->UBits		= Texture->UBits - MipLevel;
		Mip->VBits		= Texture->VBits - MipLevel;
		Mip->VMask		= VSize-1;
		Mip->Dither		= &(*GBlit.DitherBase)[Mip->UBits];
		Mip++;
		};
	UNGUARD("rendDrawAcrossSetup");
	};

/*-----------------------------------------------------------------------------
	Lighting vertical interpolators
-----------------------------------------------------------------------------*/

#ifdef ASM

typedef void (*LIGHT_V_INTERPOLATE_PROC)(void);

#else

typedef void (*LIGHT_V_INTERPOLATE_PROC)(FSpan *);

// 4 pixels high
void LightVInterpolate_8P_4(FSpan *SubRectSpan)
	{
	while (SubRectSpan) // Sublattice is 4 pixels high
		{
		for (int i=SubRectSpan->Start; i<=SubRectSpan->End; i++)
			{
			DWORD Mid              = (TLO_TopBase[i]+TLO_BotBase[i])>>1;
			GLight[i + 2*LIGHT_XR] = Mid;
			GLight[i + 1*LIGHT_XR] = (TLO_TopBase[i]+Mid)>>1;
			GLight[i + 3*LIGHT_XR] = (TLO_BotBase[i]+Mid)>>1;
			};
		SubRectSpan = SubRectSpan->Next;
		};
	};

// 2 pixels high
void LightVInterpolate_8P_2(FSpan *SubRectSpan)
	{
	while (SubRectSpan) // Sublattice is 2 pixels high
		{
		for (int i=SubRectSpan->Start; i<=SubRectSpan->End; i++)
			{
			GLight[i + 1*LIGHT_XR] = (TLO_TopBase[i]+TLO_BotBase[i])>>1;
			};
		SubRectSpan = SubRectSpan->Next;
		};
	};

// 1 pixel high
void LightVInterpolate_8P_1(FSpan *) {};
#endif

LIGHT_V_INTERPOLATE_PROC LightVInterpolateProcs[3] =
	{
	LightVInterpolate_8P_1,
	LightVInterpolate_8P_2,
	LightVInterpolate_8P_4
	};

/*-----------------------------------------------------------------------------
	Texture setup
-----------------------------------------------------------------------------*/

//
// Set up the self-modifying code for all versions of the
// texture mapper (8 mips, 4 lines, 4 trilinear phases).
//
void TexSetup( ICamera *Camera )
{
	DWORD *ModPtr;
	FDitherSet *Dither	= &(*GBlit.DitherBase)[GBlit.UBits];
	FVector Base,Scale,BaseDelta;

	// Set up colordepth-specific info and make sure a palette lookup table exists:
	if( Camera->ColorBytes==1 )
	{
		// 256-color hardware paletized
		TMI_ProcBase	= TMI_ProcTable8P;
		TMI_Shader		= (DWORD)GGfx.ShadeData;
		TRO_OuterProc	= (DWORD)TRO_Outer8P;
		ModPtr			= &TMI_ModTable8P[1];
	}
	else
	{
		// Truecolor or hicolor software paletized
		UPalette* Palette = GBlit.Texture->Palette;
		FColor Color[256],*TempColor,*LastColor,*C;

		int PaletteCacheID =
		+	((int)Palette->Index<<16) 
		+	0x0D00 
		+	(Camera->ColorBytes<<12)
		+	(Camera->Caps<<8)
		+	(GBlit.iZone)
		;

		INDEX iActor = Camera->Level.GetZoneDescriptor(GBlit.iZone);
		int ZoneScalerCacheID =
		+	((int)iActor<<16) 
		+	0x2300 
		+	(Camera->ColorBytes<<12)
		+	(Camera->Caps<<8)
		;
		
		TMI_Shader = (DWORD)GCache.Get(PaletteCacheID);
		if( !TMI_Shader )
		{
			TempColor = &Palette->Element(0);
			C		  = &Color[0];
			LastColor = &Color[256];

			for( int j=0; j<256; j++ )
			{
				C->Red		= TempColor->Red;
				C->Green	= TempColor->Green;
				C->Blue		= TempColor->Blue;
				C++; TempColor++;
			}
		}

		void* ZoneScaler = (WORD*)GCache.Get(ZoneScalerCacheID);
		if( !ZoneScaler )
		{
			// Set up one-per-zone palette scaler info for all texture palettes in this zone
			Base  = GMath.ZeroVector;
			Scale = GMath.UnitVector;

			if( iActor != INDEX_NONE)
			{
				AZoneDescriptor& Actor = (AZoneDescriptor&)Camera->Level.Actors->Element(iActor);
				GGfx.RGBtoHSV(Base, Actor.AmbientHue,Actor.AmbientSaturation,Actor.AmbientBrightness,Camera->ColorBytes);
				GGfx.RGBtoHSV(Scale,Actor.RampHue,Actor.RampSaturation,255,Camera->ColorBytes);
			}
			FLOAT Gamma = 0.5 + 5.0 * GGfx.GammaLevel / GGfx.NumGammaLevels;
			BaseDelta	= Gamma * 0x10000 * (Scale/64.0);
			Base		= Gamma * 0x10000 * Base;
		}

		// Handle the specific realcolor color depth
		if( Camera->ColorBytes==2 )
		{
			if( !ZoneScaler )
			{
				// Build one-per-zone palette scaler
				ZoneScaler   = GCache.Create(ZoneScalerCacheID,64*256*3*2);
				WORD *Ptr    = (WORD*)ZoneScaler;

				for( int i=0; i<63; i++ )
				{
					int R = 0, DR = Base.R;
					int G = 0, DG = Base.G;
					int B = 0, DB = Base.B;

					if( Camera->Caps & CC_RGB565 )
					{
						// RGB 5-6-5
						for( int j=0; j<256; j++ )
						{
							Ptr[0*256*64] = (R & 0xf80000) >> 8;
							Ptr[1*256*64] = (G & 0xfc0000) >> 13;
							Ptr[2*256*64] = (B & 0xf80000) >> 19;
							Ptr++;

							R+=DR; if (R>0xfff000) {R=0xfff000; DR=0;};
							G+=DG; if (G>0xfff000) {G=0xfff000; DG=0;};
							B+=DB; if (B>0xfff000) {B=0xfff000; DB=0;};
						}
					}
					else
					{
						// RGB 5-5-5
						for( int j=0; j<256; j++ )
						{
							Ptr[0*256*64] = (R & 0xf80000) >> 9;
							Ptr[1*256*64] = (G & 0xf80000) >> 14;
							Ptr[2*256*64] = (B & 0xf80000) >> 19;
							Ptr++;

							R+=DR; if (R>0xfff000) {R=0xfff000; DR=0;};
							G+=DG; if (G>0xfff000) {G=0xfff000; DG=0;};
							B+=DB; if (B>0xfff000) {B=0xfff000; DB=0;};
						}
					}
					Base += BaseDelta;
				}
			}

			// Set up info needed for blitting.
			TMI_Shader			= TMI_Shader >> 1;
			TMI_ProcBase		= TMI_ProcTable16P;
			TRO_OuterProc		= (DWORD)TRO_Outer16P;
			ModPtr				= &TMI_ModTable16P[1];
			WORD* Scaler		= (WORD*)ZoneScaler;

			// Build texture palette lookup table.
			if (!TMI_Shader)
			{
				TMI_Shader		= (DWORD)GCache.Create(PaletteCacheID,256*64*2,65536*2) >> 1;
				WORD *HiColor	= (WORD *)(TMI_Shader*2);

				for( int j=0; j<64; j++)
				{
					C = &Color[0];
					while( C < LastColor )
					{
						*HiColor++ =
						+	Scaler[C->Red   + 0*256*64]
						+	Scaler[C->Green + 1*256*64]
						+	Scaler[C->Blue  + 2*256*64]
						;
						C++;
					}
					Scaler += 256;
				}
			}
		}
		else if( (Camera->ColorBytes==3) || (Camera->ColorBytes==4) )
		{
			// RGB 8-8-8.
			if( !ZoneScaler )
			{
				// Build one-per-zone palette scaler.
				ZoneScaler   = GCache.Create(ZoneScalerCacheID,64*256*3*4);
				DWORD *Ptr   = (DWORD*)ZoneScaler;

				for( int i=0; i<63; i++ )
				{
					int R = 0, DR = Base.R;
					int G = 0, DG = Base.G;
					int B = 0, DB = Base.B;

					for( int j=0; j<256; j++ )
					{
						Ptr[0*256*64] = (R & 0xff0000) >> 0;
						Ptr[1*256*64] = (G & 0xff0000) >> 8;
						Ptr[2*256*64] = (B & 0xff0000) >> 16;
						Ptr++;

						R+=DR; if (R>0xfff000) {R=0xfff000; DR=0;};
						G+=DG; if (G>0xfff000) {G=0xfff000; DG=0;};
						B+=DB; if (B>0xfff000) {B=0xfff000; DB=0;};
					}
					Base += BaseDelta;
				}
			}

			// Set up info needed for blitting.
			TMI_Shader			= TMI_Shader >> 2;
			TMI_ProcBase		= TMI_ProcTable32P;
			TRO_OuterProc		= (DWORD)TRO_Outer32P;
			ModPtr				= &TMI_ModTable32P[1];

			// Build texture palette lookup table.
			if( !TMI_Shader )
			{
				TMI_Shader			= (int)GCache.Create(PaletteCacheID,256*64*4,65536*4) >> 2;
				DWORD *Scaler		= (DWORD*)ZoneScaler;
				DWORD *TrueColor	= (DWORD *)(TMI_Shader * 4);

				for( int j=0; j<64; j++ )
				{
					C = &Color[0];
					while( C < LastColor )
					{
						*TrueColor++ =
						+	Scaler[C->Red   + 0*256*64]
						+	Scaler[C->Green + 1*256*64]
						+	Scaler[C->Blue  + 2*256*64]
						;
						C++;
					}
					Scaler += 256;
				}
			}
		}
		else
		{
		appError("Invalid color depth");
		}
	}

#ifdef ASM

	// Set up all self-modifying code:
	FBlitMipInfo *MipInfo  = &GBlit.Mips[0];
	FBlitMipInfo *PrevInfo = &GBlit.Mips[1];

	for( int Mip=0; Mip<8; Mip++ )
	{
		if( GBlit.MipRef[Mip] && !GBlit.PrevMipRef[Mip] )
		{
			BYTE		*TexBase	= MipInfo->Data;
			BYTE		*PrevBase	= PrevInfo->Data;
			DWORD		AddrMask	= (0xffff >> (16-MipInfo->VBits)) + (0xffff0000 << (16-MipInfo->UBits));
			DWORD		PrevMask	= (0xffff >> (16-MipInfo->VBits)) + (0xffff0000 << (16-PrevInfo->UBits));
			BYTE		UBits		= MipInfo->UBits;
			BYTE		PrevUBits	= PrevInfo->UBits;
			FDitherPair	*Pair		= &Dither->Pair[MipInfo->MipLevel][0][0];

			if (GRend->Extra3) AddrMask = PrevMask = 0;

			static DWORD *ModPtr1;
			ModPtr1 = ModPtr;

			STAT(GStat.CodePatches += 16);
			for( int Line=0; Line<4; Line++ )
			{
				FDitherPair	*Pair = &Dither->Pair[Mip][Line][0];
				__asm
				{
					pushad

					mov eax, [Pair]
					mov ebx, [AddrMask]
					mov cl,  [UBits]
					mov esi, [TexBase]
					mov edi, [PrevBase]
					mov ch,  [PrevUBits]
					mov edx, [PrevMask]

					mov ebp, [ModPtr1] ; Frame base pointer not valid from here on

					call [ebp]

					popad
				}
				ModPtr1++;
			}
			GBlit.PrevMipRef[Mip] = 1;
		}
		MipInfo++;
		if (Mip<7) PrevInfo++;
		ModPtr += 4;
	}
#endif
}

/*-----------------------------------------------------------------------------
	Texture inner loops
-----------------------------------------------------------------------------*/

//
// C texture mapper inner loops.
//
// The C versions of these loops are extremely inefficient. The design goal
// of the C texture loops was to precisely model the inputs and outputs of the 
// assembly language loops rather than to be efficient or readable.
//

#ifndef ASM

inline DWORD LightValAt(BYTE *Dest) {return *(DWORD *)(Dest + TMI_DiffLight);};

void TexInner8(int SkipIn,FTexLattice *T)
	{
	FBlitMipInfo *Mip		= &GBlit.Mips[T->RoutineOfs >> 4];
	FDitherSet *Set			= Mip->Dither;
	BYTE *Texture			= Mip->Data;
	BYTE  UBits				= Mip->UBits;
	DWORD VMask				= Mip->VMask << UBits;
	DWORD MipLevel			= Mip->MipLevel;
	BYTE *Dest				= TMI_Dest;
	BYTE *AlignedDest		= (BYTE *)((int)TMI_Dest & ~3);
	QWORD Tex				= T->Q;
	QWORD DTex				= T->QX;
	//
	if (SkipIn) Tex += SkipIn * DTex;
	Tex  = (Tex  & ~(QWORD)0xffff) + LightValAt(AlignedDest);
	DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(AlignedDest+4) - (Tex&0xffff))>>2);
	if (SkipIn&3) Tex += (SkipIn&3) * (WORD)(DTex & 0xffff);
	//
	while (Dest < TMI_FinalDest)
		{
		QWORD ThisTex = Tex + Set->Pair[MipLevel][(int)Dest&3][TRO_Y&1].Offset;
		*Dest++ = GGfx.ShadeData
			[
			(int)(ThisTex&0x3f00)+
			(int)Texture
				[
				((ThisTex >> (64-UBits))        ) +
				((ThisTex >> (32-UBits)) & VMask)
				]
			];
		Tex += DTex;
		if (((int)Dest & 3)==0) DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(Dest+4) - (Tex&0xffff))>>2);
		};
	};
void TexInner16(int SkipIn,FTexLattice *T)
	{
	FBlitMipInfo *Mip	= &GBlit.Mips[T->RoutineOfs >> 4];
	FDitherSet *Set		= Mip->Dither;
	DWORD MipLevel			= Mip->MipLevel;
	BYTE *Texture		= Mip->Data;
	BYTE  UBits			= Mip->UBits;
	DWORD VMask			= Mip->VMask << UBits;
	BYTE *Dest			= TMI_Dest;
	BYTE *AlignedDest   = (BYTE *)((int)TMI_Dest & ~3);
	QWORD Tex			= T->Q;
	QWORD DTex			= T->QX;
	//
	if (SkipIn) Tex += SkipIn * DTex;
	Tex  = (Tex  & ~(QWORD)0xffff) + LightValAt(AlignedDest);
	DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(AlignedDest+4) - (Tex&0xffff))>>2);
	if (SkipIn&3) Tex += (SkipIn&3) * (WORD)(DTex & 0xffff);
	//
	while (Dest < TMI_FinalDest)
		{
		QWORD ThisTex = Tex + Set->Pair[MipLevel][(int)Dest&3][TRO_Y&1].Offset;
		*(WORD *)((int)Dest++ * 2) = ((WORD *)((int)TMI_Shader*2))
			[
			(int)(ThisTex&0x3f00)+
			(int)Texture
				[
				((ThisTex >> (64-UBits))        ) +
				((ThisTex >> (32-UBits)) & VMask)
				]
			];
		Tex += DTex;
		if (((int)Dest & 3)==0) DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(Dest+4) - (Tex&0xffff))>>2);
		};
	};
void TexInner24(int SkipIn,FTexLattice *T)
	{
	FBlitMipInfo *Mip	= &GBlit.Mips[T->RoutineOfs >> 4];
	BYTE *Texture		= Mip->Data;
	FDitherSet *Set		= Mip->Dither;
	DWORD MipLevel		= Mip->MipLevel;
	BYTE  UBits			= Mip->UBits;
	DWORD VMask			= Mip->VMask << UBits;
	BYTE *Dest			= TMI_Dest;
	BYTE *Dest3			= (BYTE *)((int)TMI_Dest * 3);
	BYTE *AlignedDest   = (BYTE *)((int)TMI_Dest & ~3);
	QWORD Tex			= T->Q;
	QWORD DTex			= T->QX;
	//
	if (SkipIn) Tex += SkipIn * DTex;
	Tex  = (Tex  & ~(QWORD)0xffff) + LightValAt(AlignedDest);
	DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(AlignedDest+4) - (Tex&0xffff))>>2);
	if (SkipIn&3) Tex += (SkipIn&3) * (WORD)(DTex & 0xffff);
	//
	union
		{
		BYTE B[4];
		DWORD D;
		} Temp;
	while (Dest < TMI_FinalDest)
		{
		QWORD ThisTex = Tex + Set->Pair[MipLevel][(int)Dest&3][TRO_Y&1].Offset;
		Temp.D = ((DWORD *)((int)TMI_Shader*4))
			[
			(int)(ThisTex&0x3f00)+
			(int)Texture
				[
				((ThisTex >> (64-UBits))        ) +
				((ThisTex >> (32-UBits)) & VMask)
				]
			];
		Tex += DTex;
		Dest3[0] = Temp.B[0];
		Dest3[1] = Temp.B[1];
		Dest3[2] = Temp.B[2];
		Dest3 += 3;
		Dest++;
		if (((int)Dest & 3)==0) DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(Dest+4) - (Tex&0xffff))>>2);
		};
	};
void TexInner32(int SkipIn,FTexLattice *T)
	{
	FBlitMipInfo *Mip	= &GBlit.Mips[T->RoutineOfs >> 4];
	BYTE *Texture		= Mip->Data;
	BYTE  UBits			= Mip->UBits;
	FDitherSet *Set		= Mip->Dither;
	DWORD MipLevel			= Mip->MipLevel;
	DWORD VMask			= Mip->VMask << UBits;
	BYTE *Dest			= TMI_Dest;
	BYTE *AlignedDest   = (BYTE *)((int)TMI_Dest & ~3);
	QWORD Tex			= T->Q;
	QWORD DTex			= T->QX;
	//
	#ifdef MMX___this_code_is_broken
	static int SavedESP,SavedEBP,AddrMask,templ,temph;
	static QWORD TempQ;
	static BYTE UBits1;
	UBits1 = UBits;
	templ=0x80000000;
	temph=1<<(31-UBits);
	AddrMask = (0x0000ffff >> (16-Mip->VBits)) + (0xffff0000 << (16-Mip->UBits));
	TMI_DTex = DTex;
	if (SkipIn) Tex += SkipIn * DTex;
	__asm
		{
		mov			cl,[UBits]
		mov			ecx,[MMX_Palette]
		;
		mov			[SavedESP],esp
		mov			[SavedEBP],ebp
		;
		mov			edi,[Dest]
		mov			ebx,DWORD PTR [Tex]
		;
		mov			esi,DWORD PTR [Tex+4]
		mov			ebp,[Texture]
		;
		mov			edx,ecx
		;
		mov			eax,[AddrMask]
		mov			edx,[templ]
		;
		MMXLoop:
		;
		add			edx,ebx
		mov			edx,[temph]
		;
		adc			edx,esi
		and			eax,esi
		;
		mov			cl,[UBits1]
		rol			eax,cl
		and			edx,[AddrMask]
		;
		inc			edi
		add			ebx,DWORD PTR [TMI_DTex]
		;
		adc			esi,DWORD PTR [TMI_DTex+4]
		mov			cl,[ebp+eax]
		;
		xchg		cl,[UBits1]
		rol			edx,cl
		xchg		cl,[UBits1]
		mov			eax,[AddrMask]
		;
		movq		mm0,[ecx*8]
		mov			cl,[ebp+edx]
		;
		mov			edx,[templ]
		;...
		;
		paddw		mm0,[ecx*8]
		;
		psrlw		mm0,4
		packuswb	mm0,mm0
		movd		[edi*4-4],mm0
		;
		cmp			edi,[TMI_FinalDest]
		jl			MMXLoop
		;
		mov			esp,[SavedESP]
		mov			ebp,[SavedEBP]
		emms
		};
	#else
	//
	if (SkipIn) Tex += SkipIn * DTex;
	Tex  = (Tex  & ~(QWORD)0xffff) + LightValAt(AlignedDest);
	DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(AlignedDest+4) - (Tex&0xffff))>>2);
	if (SkipIn&3) Tex += (SkipIn&3) * (WORD)(DTex & 0xffff);
	//
	while (Dest < TMI_FinalDest)
		{
		QWORD ThisTex = Tex + Set->Pair[MipLevel][(int)Dest&3][TRO_Y&1].Offset;
		*(DWORD *)((int)Dest++ * 4) = ((DWORD *)((int)TMI_Shader*4))
			[
			(int)(ThisTex&0x3f00)+
			(int)Texture
				[
				((ThisTex >> (64-UBits))        ) +
				((ThisTex >> (32-UBits)) & VMask)
				]
			];
		Tex += DTex;
		if (((int)Dest & 3)==0) DTex = (DTex & ~(QWORD)0xffff) + (WORD)((LightValAt(Dest+4) - (Tex&0xffff))>>2);
		};
	#endif
	};
TEX_INNER GTexInnerTable[5]={NULL,TexInner8,TexInner16,TexInner24,TexInner32};
#endif

/*-----------------------------------------------------------------------------
	Texture outer loops
-----------------------------------------------------------------------------*/

//
// Texture mapper outer loop
//
#ifndef ASM
void TexOuter(void)
	{
	FSpan *SpanNext;
	int SpanStart,SpanEnd,StartX,EndX;
	//
	// Draw all spans:
	//
	while (TMO_Span)
		{
		SpanStart = TMO_Span->Start;
		SpanEnd   = TMO_Span->End;
		SpanNext  = TMO_Span->Next;
		//
		FTexLattice *T	= TMI_LatticeBase[SpanStart >> GBlit.LatticeXBits];
		if ((SpanStart ^ (SpanEnd-1)) & GBlit.LatticeXNotMask) // Draw multiple rectspans
			{
			StartX = (SpanStart & GBlit.LatticeXNotMask) + GBlit.LatticeX;
			EndX   = StartX + GBlit.LatticeX;
			//
			// Draw first, left-clipped rectspan:
			//
			TMI_Dest		= TMO_DestBits + SpanStart;
			TMI_FinalDest	= TMO_DestBits + StartX;
			GTexInner(SpanStart & GBlit.LatticeXMask,T++);
			//
			// Draw middle, unclipped rectspans:
			//
			while (EndX < SpanEnd)
				{
				TMI_Dest		= TMO_DestBits + StartX;
				TMI_FinalDest	= TMO_DestBits + EndX;
				GTexInner(0,T++);
				StartX  = EndX;
				EndX   += GBlit.LatticeX;
				};
			//
			// Draw last, right-clipped rectspan:
			//
			TMI_Dest		= TMO_DestBits + StartX;
			TMI_FinalDest	= TMO_DestBits + SpanEnd;
			GTexInner (0,T);
			}
		else // Draw single left- and right-clipped span
			{
			if ((SpanStart ^ SpanEnd) & ~3) // Separated
				{
				TMI_Dest		= TMO_DestBits + SpanStart;
				TMI_FinalDest	= TMO_DestBits + SpanEnd;
				GTexInner(SpanStart & GBlit.LatticeXMask,T);
				}
			else // Together
				{
				TMI_Dest		= TMO_DestBits + SpanStart;
				TMI_FinalDest	= TMO_DestBits + SpanEnd;
				GTexInner(SpanStart & GBlit.LatticeXMask,T);
				};
			};
		TMO_Span = SpanNext;
		};
	//
	// Update all rects:
	//
	FSpan *RectSpan = TMO_RectSpan;
	while (RectSpan)
		{
		FTexLattice *T	= TMI_LatticeBase[RectSpan->Start];
		int RectXCount	= RectSpan->End - RectSpan->Start;
		//
		while (RectXCount-- > 0)
			{
			T->Q  += T->QY;
			T->QX += T->QXY;
			T++;
			};
		RectSpan = RectSpan->Next;
		};
	};
#endif

/*-----------------------------------------------------------------------------
	Light mapping
-----------------------------------------------------------------------------*/

void LightSetup(void)
	{
	TLI_MeshFloat			= GLightManager->GetMeshFloat();
	TLI_AddrMask			= (0xffffffff >> (32-GLightManager->MeshVBits)) + (0xffffffff << (32-GLightManager->MeshUBits));
	TLI_Sinc				= GGfx.SincData;
	TLI_ProcBase			= &TLI_ProcTable[(int)GLightManager->MeshUBits << 6];
	//
	TLM_GBlitInterX			= GBlit.InterX;
	TLM_GBlitInterXBits2	= GBlit.InterXBits+2;
	};

#ifndef ASM
void LightInner_8P_Unlit(void)
	{
	DWORD *Dest = TLI_Dest;
	while (Dest < TLI_DestEnd) *Dest++=0x1800;
	};
void LightInner_8P_Lit(void)
	{
	//
	// Make sure we're in a valid rect.  We may not be, as this inner loop will often
	// be called to render the first lightel of the next rect, when only the previous
	// rect's definition is available.  This could be solved by storing h & v deltas for
	// all lattices where they are valid, but this is not convenient due to the rol texture
	// coordinate encoding format, so we just adjust the rect here.  Adjustment is required,
	// on average, 8% of the time.
	//
	FTexLattice *T = TLI_TopLattice;
	//
	if ((!T) || (!T->RoutineOfs))
		{
		FTexLattice **LatticeBase = &TLO_LatticeBase[(TLI_Dest - TLO_BotBase) >> GBlit.InterXBits];
		//
		TLI_SkipIn = GBlit.InterX;
		T      = LatticeBase[-1];
		if ((!T) || (!T->RoutineOfs))
			{
			TLI_SkipIn = 0;
			T = LatticeBase[-MAX_XR];
			if ((!T) || (!T->RoutineOfs))
				{
				TLI_SkipIn = GBlit.InterX;
				T = LatticeBase[-MAX_XR-1];
				//
				#ifdef PARANOID
					if (!T) appError("LightInner inconsistency 1");
					if (!T->RoutineOfs) appError("LightInner inconsistency 2");
				#endif
				};
			};
		};
	DWORD	*Dest		= TLI_Dest;
	QWORD	Tex			= T->SubQ;
	QWORD	DTex		= T->SubQX;
	BYTE	UBits		= GLightManager->MeshUBits;
	DWORD	VMask		= ((1 << GLightManager->MeshVBits)-1) << UBits;
	DWORD	USize		= 1<<UBits;
	//
	if (TLI_SkipIn) Tex += DTex * TLI_SkipIn;
	//
	QWORD Ofs = (QWORD)1 << (64-UBits);
	while (Dest < TLI_DestEnd)
		{
		FLOAT *Addr1 = &((FLOAT*)GLightManager->MeshVoid)
			[
			((Tex >> (64-UBits))        ) +
			((Tex >> (32-UBits)) & VMask)
			];
		FLOAT *Addr2 = &((FLOAT*)GLightManager->MeshVoid)
			[
			(((Tex+Ofs) >> (64-UBits))  ) +
			((Tex >> (32-UBits)) & VMask)
			];
		FLOAT Alpha = GGfx.SincData[(Tex>>(56-UBits))&0xff];
		//
		FLOAT A		= Addr1[0];
		FLOAT C		= Addr1[USize];
		FLOAT AB	= A+(Addr2[0    ]-A)*Alpha;
		FLOAT CD	= C+(Addr2[USize]-C)*Alpha;
		//
		*Dest++ = ((int)(AB + (CD-AB)*GGfx.SincData[(Tex>>24)&0xff]) & 0x3ff8) + (Tex & 0x3ff8); // 2x2 Precision masked
		Tex += DTex;
		};
	};
#endif

//
// Generate lighting across an entire row of sublattices based on
// the setup info contained in the specified row of rects.
//
#ifndef ASM
inline void LightOuter(FSpan *Span)
	{
	//
	// Light this span
	//
	while (Span)
		{
		FSpan *SpanNext = Span->Next;
		//
		TLI_TopLattice	= TLO_LatticeBase[Span->Start >> GBlit.InterXBits];
		TLI_Dest		= TLO_BotBase + Span->Start;
		TLO_FinalDest	= TLO_BotBase + Span->End;
		TLI_SkipIn		= Span->Start & GBlit.InterXMask;
		//
		if ((Span->Start ^ (Span->End-1)) & GBlit.InterXNotMask) // Draw multiple rectspans
			{
			//
			// Light first, left-clipped rectspan:
			//
			TLI_DestEnd	= TLO_BotBase + (Span->Start & GBlit.InterXNotMask) + GBlit.InterX;
			GLightInnerProc();
			TLI_TopLattice++;
			//
			// Light middle, unclipped rectspans:
			//
			TLI_SkipIn	 = 0;
			TLI_Dest	 = TLI_DestEnd;
			TLI_DestEnd	+= GBlit.InterX;
			//
			while (TLI_DestEnd < TLO_FinalDest)
				{
				GLightInnerProc();
				TLI_Dest = TLI_DestEnd;
				TLI_DestEnd += GBlit.InterX;
				TLI_TopLattice++;
				};
			};
		//
		// Light last, right-clipped rectspan:
		//
		TLI_DestEnd = TLO_FinalDest;
		GLightInnerProc();
		//
		Span = SpanNext;
		};
	//
	// Update all rects:
	//
	FSpan *RectSpan = TLO_RectSpan;
	while (RectSpan)
		{
		FTexLattice *T	= TLO_LatticeBase[RectSpan->Start];
		int Count		= RectSpan->End - RectSpan->Start;
		//
		while (Count-- > 0)
			{
			T->SubQ		+= T->SubQY;
			T->SubQX	+= T->SubQXY;
			T++;
			};
		RectSpan = RectSpan->Next;
		};
	};
#endif

/*-----------------------------------------------------------------------------
	Main texture mapper
-----------------------------------------------------------------------------*/

void rendDrawAcross
	(
	ICamera *Camera,FSpanBuffer *SpanBuffer,
	FSpanBuffer *RectSpanBuffer, FSpanBuffer *LatticeSpanBuffer,
	FSpanBuffer *SubRectSpanBuffer, FSpanBuffer *SubLatticeSpanBuffer
	)
	{
	GUARD;
	ALWAYS_BEGINTIME(GStat.TextureMap);
	//
	static int RectEndY,EndY;
	static FSpan*const NullSpan = NULL;
	static FSpan*const* RectSpanDecisionList[4] =
		{
		&NullSpan,&TMO_RectSpan,&TMO_NextRectSpan,&TMO_RectSpan
		};
	static FTexLattice**const* LatticeBaseDecisionList[4] =
		{
		&TMI_NextLatticeBase,&TMI_LatticeBase,&TMI_NextLatticeBase,&TMI_LatticeBase
		};
	static int*const SubRectDecisionList[2] =
		{
		&TRO_SubRectEndY,
		&EndY
		};
	static int*const RectDecisionList[2] =
		{
		&RectEndY,
		&EndY
		};
	static DWORD*const LightAlternator = (DWORD *)((int)&GLight[0] + (int)&GLight[LIGHT_X_TOGGLE]);
	static DWORD*const OriginalLightBase[4]={&GLight[LIGHT_XR*1],&GLight[LIGHT_XR*1],&GLight[LIGHT_XR*2],&GLight[LIGHT_XR*3]};
	//
	static DWORD* LightBase[4];
	static LIGHT_V_INTERPOLATE_PROC LightVInterpolateProc;
	static FSpan **RectIndex,**SubLatticeIndex,**SubRectIndex;
	//
	LightVInterpolateProc = LightVInterpolateProcs[GBlit.SubYBits];
	memcpy(LightBase,OriginalLightBase,sizeof(LightBase));
	//
	#ifdef ASM
		if (!GLightManager->Index)	TLO_LightInnerProc = TLM_8P_Unlit;
		else						TLO_LightInnerProc = TLM_8P_Lit;
	#else
		if (!GLightManager->Index)	GLightInnerProc = LightInner_8P_Unlit;
		else						GLightInnerProc = LightInner_8P_Lit;
		GTexInner = GTexInnerTable[Camera->ColorBytes];
		void (*TexOuterProc)(void) = TexOuter;
	#endif
	//
	// Set up texture mapper inner globals:
	//
	TMI_LatticeBase		= &GRender.LatticePtr[RectSpanBuffer->StartY+1][1];
	TMI_NextLatticeBase	= &TMI_LatticeBase[MAX_XR];
	//
	// Set up texture mapper outer globals:
	//
	TMO_Stride = Camera->SXStride;
	if (!GEffect)
		{
		TMO_DestBits          = NULL;
		TMO_DestOrDestBitsPtr = &TMO_DestBits;
		}
	else // (GEffect)
		{
		TMO_DestBits = (BYTE *)GMem.Get(2048*1);
		TMO_DestOrDestBitsPtr = &TMO_Dest;
		};
	TexSetup(Camera);
	LightSetup();
	//
	// Set up:
	//
	TRO_SpanIndex	= &SpanBuffer->Index[0];
	RectIndex		= &RectSpanBuffer->Index[0];
	SubLatticeIndex	= &SubLatticeSpanBuffer->Index[0];
	SubRectIndex	= &SubRectSpanBuffer->Index[0];
	//
	TRO_Y			= SpanBuffer->StartY;
	EndY			= SpanBuffer->EndY;
	TRO_SubRectEndY	= (SubRectSpanBuffer->StartY + 1) << GBlit.SubYBits;
	RectEndY		= (RectSpanBuffer->StartY    + 1) << GBlit.LatticeYBits;
	//
	// Skip into top rect:
	//
	int SkipY = TRO_Y - (RectSpanBuffer->StartY << GBlit.LatticeYBits);
	if (SkipY > 0)
		{
		int SubSkipY    = SkipY >> GBlit.SubYBits;
		FSpan *RectSpan = *RectSpanBuffer->Index;
		//
		while (RectSpan)
			{
			FTexLattice *T = TMI_LatticeBase[RectSpan->Start];
			if (SubSkipY) for (int i=RectSpan->Start; i<RectSpan->End; i++)
				{
				T->Q		+= SkipY    * T->QY;
				T->QX		+= SkipY    * T->QXY;
				T->SubQ		+= SubSkipY * T->SubQY;
				T->SubQX	+= SubSkipY * T->SubQXY;
				T++;
				}
			else for (int i=RectSpan->Start; i<RectSpan->End; i++)
				{
				T->Q		+= SkipY * T->QY;
				T->QX		+= SkipY * T->QXY;
				T++;
				}
			RectSpan = RectSpan->Next;
			};
		};
	//
	// Prepare to traverse all lattice rows:
	//
	TMO_Dest		= (BYTE *)((DWORD)Camera->Screen / Camera->ColorBytes) + SpanBuffer->StartY * TMO_Stride;
	TMO_RectSpan	= *RectIndex++;
	//
	// Set up for first lighting sublattice:
	//
	TLO_BotBase		= &GLight[0];
	TLO_RectSpan	= *RectSpanBuffer->Index;
	TLO_LatticeBase = TMI_LatticeBase;
	//
	#ifdef ASM
	__asm
		{
		pushad
		;
		; Generate first row's lighting:
		;
		mov eax,[SubLatticeIndex]
		mov esi,[eax]
		add eax,4
		mov [SubLatticeIndex],eax
		call LightOuter
		;
		; Lead into MainYLoop:
		;
		mov		eax,[TRO_Y]				; eax = TRO_Y
		mov		ebx,[EndY]				; ebx = EndY
		mov		ecx,[RectEndY]			; ecx = RectEndY
		xor		edx,edx					; Zero edx
		;
		MainYLoop:
		;
		mov		edi,[RectIndex]			; edi = RectIndex
		cmp		ecx,ebx					; RectEndY <?> EndY
		;
		setg	dl						; dl = RectEndY > EndY
		;
		mov		esi,[edi]				; esi = *RectIndex
		add		edi,4					; edi = RectIndex+1
		;
		mov		[TMO_NextRectSpan],esi	; TMO_NextRectSpan = *RectIndex
		mov		esi,RectDecisionList[edx*4] ; edi = RectDecisionList[RectEndY > EndY]
		;
		mov		[RectIndex],edi			; RectIndex++
		mov		edi,[GBlit].SubY		; edi = GBlit.SubY
		;
		mov		edx,[esi]				; ecx = *RectDecisionList[RectEndY > EndY]
		add		edi,eax					; edi = TRO_Y + GBlit.SubY
		;
		mov		[RectEndY],edx			; RectEndY = *RectDecisionList[RectEndY > EndY]
		;
		; Traverse all sublattices rows in lattice row:
		;
		RectYLoop:
		;
		mov		edx,[TLO_BotBase]		; edx = TLO_BotBase
		mov		ebx,[LightAlternator]	; ebx = LightAlternator
		;
		mov		[TLO_TopBase],edx		; TLO_TopBase = TLO_BotBase
		sub		ebx,edx					; ebx = LightAlternator - TLO_BotBase
		;
		mov		[TLO_BotBase],ebx		; TLO_BotBase = LightAlternator - TLO_BotBase
		mov		esi,[EndY]				; esi = EndY
		;
		mov		[LightBase],edx			; LightBase[0] = TLO_BotBase
		cmp		edi,esi					; (TRO_Y + GBlit.SubY) <?> EndY
		;
		setl	dl						; dl = (TRO_Y + GBlit.SubY) < EndY
		;
		shl		dl,1					; dl = ((TRO_Y + GBlit.SubY) < EndY) * 2
		cmp		edi,ecx					; (TRO_Y + GBlit.SubY) <?> RectEndY
		;
		setl	cl						; cl = (TRO_Y + GBlit.SubY) < RectEndY
		;
		add		cl,dl					; cl = choice index
		xor		ebx,ebx					; ebx = 0
		;
		mov		bl,cl					; ebx = choice index
		mov		ecx,[TRO_SubRectEndY]	; ecx = TRO_SubRectEndY
		;
		xor		eax,eax					; Clear out eax
		cmp		ecx,esi					; TRO_SubRectEndY <?> EndY
		;
		mov		edx,LatticeBaseDecisionList[ebx*4] ; edx = LatticeBaseDecisionList[Choice]
		mov		ecx,RectSpanDecisionList[ebx*4] ; ecx = RectSpanDecisionList[Choice]
		;
		setg	al						; eax = TRO_SubRectEndY > EndY
		;
		mov		esi,[edx]				; esi = *LatticeBaseDecisionList[Choice]
		mov		ebx,[ecx]				; ebx = *RectSpanDecisionList[Choice]
		;
		mov		[TLO_LatticeBase],esi	; TLO_LatticeBase = *LatticeBaseDecisionList[Choice]
		mov		edx,[SubLatticeIndex]	; edx = SubLatticeIndex
		;
		mov		esi,SubRectDecisionList[eax*4] ; esi = SubRectDecisionList[TRO_SubRectEndY > EndY]
		mov		[TLO_RectSpan],ebx		; TLO_RectSpan = *RectSpanDecisionList[Choice]
		;
		mov		ecx,[esi]				; ecx = *SubRectDecisionList[TRO_SubRectEndY > EndY]
		mov		esi,[edx]				; esi = *SubLatticeIndex
		;
		sub		edi,ecx					; edi = TRO_Y + GBlit.SubY - TRO_SubRectEndY
		add		edx,4					; edx = SubLatticeIndex+1
		;
		mov		[TRO_SubRectEndY],ecx	; TRO_SubRectEndY = *SubRectDecisionList[TRO_SubRectEndY > EndY]
		mov		[SubLatticeIndex],edx	; SubLatticeIndex++
		;
		lea		edx,LightBase[edi*4]	; ecx = &LightBase[TRO_Y + GBlit.SubY - TRO_SubRectEndY]
		;
		mov		[TRO_ThisLightBase],edx	; TRO_ThisLightBase	= &LightBase[TRO_Y + GBlit.SubY - TRO_SubRectEndY]
		;
		; Perform sublattice lighting:
		;
		call	LightOuter
		;
		; Interpolate the sublattice lighting:
		;
		mov		eax,[SubRectIndex]
		mov		ebx,4
		;
		add		ebx,eax
		;
		mov		esi,[eax]
		mov		[SubRectIndex],ebx
		;
		test	esi,esi
		jz		SkipLighting
		;
		call	[LightVInterpolateProc]
		SkipLighting:
		;
		; Texture map these 4 lines:
		;
		mov eax,[TRO_Y]
		call [TRO_OuterProc]
		;
		; Next subrect:
		;
		mov edi,[GBlit].SubY
		mov eax,[TRO_Y]
		;
		add edi,eax
		mov ecx,[RectEndY]
		;
		mov [TRO_SubRectEndY],edi
		;
		cmp eax,ecx ; TRO_Y <?> RectEndY
		jl  RectYLoop
		;
		; Next rect:
		;
		mov edi,[TMO_NextRectSpan]		; edi = TMO_NextRectSpan
		mov	esi,[TMI_NextLatticeBase]	; esi = TMI_NextLatticeBase
		;
		mov ecx,[GBlit].LatticeY		; edx = GBlit.LatticeY
		mov [TMO_RectSpan],edi			; TMO_RectSpan = TMO_NextRectSpan;
		;
		mov [TMI_LatticeBase],esi		; TMI_LatticeBase = TMI_NextLatticeBase
		add ecx,eax						; ecx = TRO_Y + GBlit.LatticeY
		;
		add	esi,MAX_XR*4				; esi = TMI_NextLatticeBase + MAX_XR
		mov [RectEndY],ecx				; RectEndY = TRO_Y + GBlit.LatticeY
		;
		mov ebx,[EndY]					; ebx = EndY
		mov [TMI_NextLatticeBase],esi	; TMI_NextLatticeBase += MAX_XR
		;
		xor		edx,edx					; Zero edx
		;
		cmp eax,ebx						; TRO_Y <?> TRO_EndY
		jl MainYLoop					; Next rect
		;
		popad
		};
	#else
	LightOuter(*SubLatticeIndex++);
	while (TRO_Y < EndY)
		{
		RectEndY = *RectDecisionList[RectEndY > EndY]; //Logic version: if (RectEndY > EndY) RectEndY = EndY;
		TMO_NextRectSpan = *RectIndex++;
		//
		while (TRO_Y < RectEndY)
			{
			TLO_TopBase		= LightBase[0] = TLO_BotBase;
			TLO_BotBase		= (DWORD *)((int)LightAlternator - (int)TLO_BotBase);
			//
			// No-branch logic to set up pointers required by LightOuter, equivalant to the following:
			//
			//if (TempY<RectEndY)	{TLO_RectSpan = TMO_RectSpan;		TLO_LatticeBase = TMI_LatticeBase;}
			//else if (TempY<EndY)	{TLO_RectSpan = TMO_NextRectSpan;   TLO_LatticeBase = TMI_NextLatticeBase;}
			//else					{TLO_RectSpan = NULL;				TLO_LatticeBase = TMI_NextLatticeBase;};
			//
			//if (TRO_SubRectEndY > EndY) TRO_SubRectEndY = EndY;
			//
			int TempY = TRO_Y + GBlit.SubY;
			int Choice = (TempY<RectEndY) + (TempY<EndY)*2;
			//
			TLO_RectSpan		= *RectSpanDecisionList[Choice];
			TLO_LatticeBase		= *LatticeBaseDecisionList[Choice];
			//
			TRO_SubRectEndY		= *SubRectDecisionList[TRO_SubRectEndY > EndY];
			TRO_ThisLightBase	= &LightBase[TRO_Y + GBlit.SubY - TRO_SubRectEndY];
			//
			LightOuter(*SubLatticeIndex++);
			LightVInterpolateProc(*SubRectIndex++);
			//
			do	{
				*TMO_DestOrDestBitsPtr	= TMO_Dest; //Translation: if (!GEffect) TMO_DestBits = TMO_Dest;
				TMI_DiffLight			= (int)*TRO_ThisLightBase - (int)TMO_DestBits;
				TMI_DiffDest			= (int)TMO_Dest       - (int)TMO_DestBits;
				TMI_LineProcBase		= &TMI_ProcBase			[(TRO_Y&3)*2];
				//
				// Perform all texture mapping and rect updating:
				//
				TMO_OrigSpan = TMO_Span = *TRO_SpanIndex++;
				TexOuterProc();
				if (GEffect) GEffect();
				//
				// Next line:
				//
				TMO_Dest += TMO_Stride;
				TRO_ThisLightBase++;
				} while (++TRO_Y < TRO_SubRectEndY);
			// Next subrect:
			TRO_SubRectEndY   = TRO_Y + GBlit.SubY;
			};
		// Next rect:
		TMO_RectSpan		 = TMO_NextRectSpan;
		TMI_LatticeBase		 = TMI_NextLatticeBase;
		TMI_NextLatticeBase += MAX_XR;
		RectEndY			 = TRO_Y + GBlit.LatticeY;
		};
		#endif
	ALWAYS_ENDTIME(GStat.TextureMap);
	UNGUARD("rendDrawAcross");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
