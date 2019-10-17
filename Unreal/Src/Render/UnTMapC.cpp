/*=============================================================================
	UnTmap.cpp: Unreal texture mapping functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

//
// Temp:
//
#ifdef ASM
#define XXX extern
extern "C"
	{
#else
#define XXX 
#endif
	XXX INT		TM_AndMask;
	XXX INT		TM_End4;
	XXX INT		TM_StartLine,TM_EndLine,TM_RectY1,TM_RectY2,TM_RectX1,TM_RectX2;
	XXX INT		TM_DestInc,TM_Line,TM_DitherPtr,TM_BlenderDiff;
	XXX INT		TM_GG,TM_UU,TM_DG,TM_DU,TM_GInc,TM_UInc,TM_DGInc,TM_DUInc;
	XXX BYTE	*TM_TexAddr,*TM_EndAddr,*TM_ShadeAddr,*TM_Dest,*TM_SpanDest;
	XXX FSpan	**TM_SpanIndex;
	XXX BYTE	TM_PixelShift;
#ifdef ASM
	extern void (__cdecl *TextureSpanTable [13][4])(void);
	extern void (__cdecl *TextureBlockTable[13][4])(void);
	};
#endif

/*-----------------------------------------------------------------------------
	Raster drawers
-----------------------------------------------------------------------------*/

#ifdef ASM
void inline CallAsm(BYTE *Dest, QWORD Start, QWORD Inc, int Pixels, int Routine)
	{
	TM_Line			= GBlit.Line;
	TM_SpanDest		= Dest;
	TM_GG			= Start;
	TM_UU			= Start >> 32;
	TM_DG			= Inc;
	TM_DU			= Inc >> 32;
	TM_EndAddr		= Dest + Pixels;
	void (VARARGS *Func)(void) = TextureSpanTable[GBlit.UBits][Routine+1];
	__asm
		{
		push esi
		push edi
		push ebx
		push ecx
		call Func
		pop ecx
		pop ebx
		pop edi
		pop esi
		};
	};
#endif

void DrawNormalRaster(BYTE *Dest, QWORD Start, QWORD Inc, int Pixels)
	{
	#ifdef ASM
		CallAsm(Dest,Start,Inc,Pixels,0);
	#else
		QWORD VMask = (GBlit.VSize-1) << GBlit.UBits;
		while (Pixels-- > 0)
			{
			*Dest++ = GGfx.ShadeData
				[
				(int)(Start&0x3f00)
				+
				(int)GBlit.TextureData
					[
					((Start >> (64-GBlit.UBits))        )+
					((Start >> (32-GBlit.UBits)) & VMask)
					]
				];
			Start += Inc;
			};
	#endif
	};

void DrawMaskedRaster(BYTE *Dest, QWORD Start, QWORD Inc, int Pixels)
	{
	#ifdef ASM
		CallAsm(Dest,Start,Inc,Pixels,1);
	#else
		QWORD VMask = (GBlit.VSize-1) << GBlit.UBits;
		BYTE B;
		while (Pixels-- > 0)
			{
			B = GGfx.ShadeData
				[
				(int)(Start&0x3f00)
				+
				(int)GBlit.TextureData
					[
					((Start >> (64-GBlit.UBits))        )+
					((Start >> (32-GBlit.UBits)) & VMask)
					]
				];
			if (B) *Dest = B;
			Dest++;
			Start += Inc;
			};
	#endif
	};

void DrawBlendedRaster(BYTE *Dest, QWORD Start, QWORD Inc, int Pixels)
	{
	#ifdef ASM
		CallAsm(Dest,Start,Inc,Pixels,2);
	#else
		QWORD VMask = (GBlit.VSize-1) << GBlit.UBits;
		while (Pixels-- > 0)
			{
			*Dest++ = GBlit.BlendTable
				[
				((int)*Dest << 8)
				+
				GGfx.ShadeData
					[
					(int)(Start&0x3f00)
					+
					(int)GBlit.TextureData
						[
						((Start >> (64-GBlit.UBits))        )+
						((Start >> (32-GBlit.UBits)) & VMask)
						]
					]
				];
			Start += Inc;
			};
	#endif
	};

void DrawMaskedBlendedRaster(BYTE *Dest, QWORD Start, QWORD Inc, int Pixels)
	{
	#ifdef ASM
		CallAsm(Dest,Start,Inc,Pixels,3);
	#else
		QWORD VMask = (GBlit.VSize-1) << GBlit.UBits;
		BYTE B;
		while (Pixels-- > 0)
			{
			B = GBlit.BlendTable
				[
				((int)*Dest << 8)
				+
				GGfx.ShadeData
					[
					(int)(Start&0x3f00)
					+
					(int)GBlit.TextureData
						[
						((Start >> (64-GBlit.UBits))        )+
						((Start >> (32-GBlit.UBits)) & VMask)
						]
					]
				];
			if (B) *Dest = B;
			Dest++;
			Start += Inc;
			};
	#endif
	};

/*-----------------------------------------------------------------------------
	Overlay highlight drawer
-----------------------------------------------------------------------------*/

void FGlobalRender::DrawHighlight( ICamera *Camera,FSpanBuffer *SpanBuffer,BYTE Color )
{
	GUARD;
	FSpan	*Span,**Index;
	BYTE	*Screen,*Line;
	int		X,Y,XOfs;

	Y     = (SpanBuffer->StartY+1)&~1;
	Index = &SpanBuffer->Index [0];

	if ( Camera->ColorBytes==1 )
	{
		Line  = &Camera->Screen [Y * Camera->SXStride];
		while( Y < SpanBuffer->EndY )
		{
			Span = *Index;
			while ( Span )
			{
				XOfs   = (Y & 2) * 2;
				X      = (((int)Span->Start + XOfs + 7) & ~7) - XOfs;
				Screen = &Line[X];
				while ( X < Span->End )
				{
					*Screen = Color;
					Screen += 8;
					X      += 8;
				}
				Span = Span->Next;
			}
			Line  += Camera->SXStride * 2;
			Y     +=2;
			Index +=2;
		}
	}
	else if( Camera->ColorBytes==2 )
	{
		Line = &Camera->Screen [Y * Camera->SXStride * 2];
		WORD HiColor;
		if( Camera->Caps & CC_RGB565 ) HiColor = GGfx.DefaultColors[SelectColor].HiColor565();
		else HiColor = GGfx.DefaultColors[SelectColor].HiColor555();
		while ( Y < SpanBuffer->EndY )
		{
			Span = *Index;
			while ( Span )
			{
				XOfs   = (Y & 2) * 2;
				X      = (((int)Span->Start + XOfs + 7) & ~7) - XOfs;
				Screen = &Line[X*2];
				while ( X < Span->End )
				{
					*(WORD *)Screen = HiColor;
					Screen += 16;
					X      += 8;
				}
				Span = Span->Next;
			}
			Line  += Camera->SXStride * 4;
			Y     += 2;
			Index += 2;
		}
	}
	else
	{
		Line			= &Camera->Screen [Y * Camera->SXStride * 4];
		DWORD TrueColor	= *(DWORD *)&GGfx.TrueColors[SelectColor];
		while ( Y < SpanBuffer->EndY )
		{
			Span = *Index;
			while ( Span )
			{
				XOfs   = (Y & 2) * 2;
				X      = (((int)Span->Start + XOfs + 7) & ~7) - XOfs;
				Screen = &Line[X*4];
				while ( X < Span->End )
				{
					*(DWORD *)Screen = TrueColor;
					Screen += 32;
					X      += 8;
				}
				Span = Span->Next;
			}
			Line  += Camera->SXStride * 8;
			Y     += 2;
			Index += 2;
		}
	}
	UNGUARD("FGlobalRender::DrawHighlight");
}

/*-----------------------------------------------------------------------------
	Dither variables & functions
-----------------------------------------------------------------------------*/

FDitherTable GDither256,GBlur256,GNoDither256;

void BuildDitherTable (FDitherSet *G, int UBits, FDitherOffsets &Offsets,FDitherOffsets &OtherOffsets)
	{
	FDitherOffsets *Ofs = &Offsets;
	int Mip,Line,Pixel;
	for (Mip=0; Mip<8; Mip++)
		{
		for (Line=0; Line<4; Line++)
			{
			for (Pixel=0; Pixel<2; Pixel++)
				{
				G->Pair[Mip][Line][Pixel].Offset =	
					((QWORD)((Ofs->G[Line][Pixel] >> (8           )) & (0x0000ffff))   ) +
					((QWORD)((Ofs->V[Line][Pixel] << (16-Mip      )) & (0xffff0000))   ) +
					((QWORD)((Ofs->V[Line][Pixel] >> (16+Mip      )) & 0x3ff) << 32) +
					((QWORD)((Ofs->U[Line][Pixel] << (16-Mip-UBits)) &~0x3ff) << 32);
				};
			for (Pixel=0; Pixel<2; Pixel++)
				{
				G->Pair[Mip][Line][Pixel].Delta =
					G->Pair[Mip][Line][(Pixel+1)&1].Offset - 
					G->Pair[Mip][Line][Pixel].Offset;
				};
			};
		Ofs = &OtherOffsets;
		};
	};

void InitDither(void)
	{
	GUARD;
	FDitherOffsets NoDitherOffsets = // Table for no texture dithering, only light dithering:
		{
			{	// U256Offsets:
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000}
			},
			{	// V256Offsets:	
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000}
			},		
			{	// G256Offsets:
				{+0x2000,+0x8000},
				{+0xC000,+0x6000},
				{+0x0000,+0xA000},
				{+0xE000,+0x4000}
			},
		};
	FDitherOffsets AbsolutelyNoDitherOffsets = // Table for absolutely no dithering
		{
			{	// U256Offsets:
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000}
			},
			{	// V256Offsets:	
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000}
			},		
			{	// G256Offsets:
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000},
				{+0x0000,+0x0000}
			},
		};
	FDitherOffsets DitherOffsets = // Regular texture and light dithering:
		{		
/*
			{	// U256Offsets:
				{+0x0000,+0x6200},
				{+0xEC00,+0x5000},
				{+0x1300,+0x7300},
				{+0xD200,+0x2A00}
			},		
			{	// V256Offsets:
				{+0xD700,+0x3200},
				{+0x0020,+0x7300},
				{+0xA500,+0x4300},
				{+0x0E00,+0x6F00}
			},
			{	// G256Offsets:
				{+0x2000,+0x8000},
				{+0xC000,+0x6000},
				{+0x0000,+0xA000},
				{+0xE000,+0x4000}
			},
*/
			{	// U256Offsets:
				{+0x0000,+0x6200},
				{+0xEC00,+0x5000},
				{+0x0000,+0x6200},
				{+0xEC00,+0x5000},
			},		
			{	// V256Offsets:
				{+0xD700,+0x3200},
				{+0x0020,+0x7300},
				{+0xD700,+0x3200},
				{+0x0020,+0x7300},
			},
			{	// G256Offsets:
				{+0x2000,+0x8000},
				{+0xC000,+0x6000},
				{+0x0000,+0xA000},
				{+0xE000,+0x4000}
			},
		};
	FDitherOffsets BlurOffsets = // Texture blurring and light dithering:
		{
			{	// U256Offsets:
				{+16*0x0000,+16*0x6200},
				{+16*0xEC00,+16*0x5000},
				{+16*0x1300,+16*0x7300},
				{+16*0xD200,+16*0x2A00}
			},		
			{	// V256Offsets:
				{+16*0xD700,+16*0x3200},
				{+16*0x0020,+16*0x7300},
				{+16*0xA500,+16*0x4300},
				{+16*0x0E00,+16*0x6F00}
			},		
			{	// G256Offsets:
				{+0x2000,+0x8000},
				{+0xC000,+0x6000},
				{+0x0000,+0xA000},
				{+0xE000,+0x4000}
			},
		};
	//
	// Build dither tables:
	//
	for (int UBits=0; UBits<16; UBits++)
		{
		BuildDitherTable(&GDither256	[UBits],UBits,DitherOffsets,	NoDitherOffsets);
		BuildDitherTable(&GBlur256		[UBits],UBits,BlurOffsets,		BlurOffsets);
		BuildDitherTable(&GNoDither256	[UBits],UBits,NoDitherOffsets,	NoDitherOffsets);
		};
	UNGUARD("InitDither");
	};

void FGlobalBlit::Setup(ICamera *Camera, UTexture *ThisTexture, DWORD ThesePolyFlags, DWORD NotPolyFlags)
	{
	GUARD;
	//
	// Set up general blitting parameters:
	//
	if (ThisTexture)	Texture = ThisTexture;
	else				Texture = GGfx.DefaultTexture;
	//
	int OurMipLevel = 0,USize,VSize;
	TextureData		= Texture->GetData(&OurMipLevel,Camera->ColorBytes,&USize,&VSize);
	UBits			= Texture->UBits;
	VBits			= Texture->VBits;
	//
	DWORD PolyFlags = (Texture->PolyFlags & ~NotPolyFlags) | ThesePolyFlags;
	//
	// Set up dithering:
	//
	if (GRender.DoDither && !(PolyFlags & PF_NoSmooth )) DitherBase = &GDither256;
	else DitherBase = &GNoDither256;
	//
	// Set up mipmapping:
	//
	FBlitMipInfo *Mip = &Mips[0];
	for (int i=0; i<8; i++)
		{
		int MipLevel = i;
		int USize,VSize;
		Mip->Data		= Texture->GetData(&MipLevel,Camera->ColorBytes,&USize,&VSize);
		Mip->MipLevel	= MipLevel;
		Mip->UBits		= UBits - MipLevel;
		Mip->VBits		= VBits - MipLevel;
		Mip->VMask		= VSize-1;
		Mip->AndMask	= GRender.Extra3 ? 0 : ((VSize-1) + ((USize-1) << (32-Mip->UBits)));
		Mip->Dither		= &(i ? GNoDither256 : *DitherBase)[Mip->UBits];
		Mip++;
		};
	//
	// Set up general effects:
	//
	if (!(PolyFlags & (PF_Transparent | PF_Ghost | PF_Glow)))
		{
		if (!(PolyFlags & PF_Masked))			DrawKind = FGlobalRender::DRAWRASTER_Normal;
		else									DrawKind = FGlobalRender::DRAWRASTER_Masked;
		BlendKind = BLEND_Transparent; // Must be something
		}
	else
		{
		DrawKind = FGlobalRender::DRAWRASTER_Blended;
		//
		if		(PolyFlags & PF_Transparent)	BlendKind = BLEND_Transparent;
		else if (PolyFlags & PF_Glow)			BlendKind = BLEND_Glow;
		else									BlendKind = BLEND_Ghost;
		};
	//
	// Assembly setup for span (non-mipmapped) texture mappers:
	//
	#ifdef ASM
		TM_DitherPtr	= (INT)&(*GBlit.DitherBase)[GBlit.UBits];
		TM_ShadeAddr	= GGfx.ShadeData;
		TM_TexAddr		= GBlit.TextureData;
		TM_BlenderDiff	= (INT)GGfx.Blenders[GBlit.BlendKind] - (INT)GGfx.ShadeData;
		TM_AndMask		= Mips[0].AndMask;
	#endif
	//
	UNGUARD("FGlobalBlit::Setup");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
