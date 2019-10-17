/*=============================================================================
	UnGfx.cpp: FGlobalGfx implementation - general-purpose graphics routines

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

/*------------------------------------------------------------------------------
	Palette-related functions
------------------------------------------------------------------------------*/

//
// Gamma correct a palette using the game's current gamma value:
//
void FGlobalGfx::GammaCorrectPalette (UPalette *DestPalette, UPalette *SourcePalette)
	{
	GUARD;
	FColor	*DestColors		= DestPalette->GetData();
	FColor	*SourceColors	= SourcePalette->GetData();
	//
	FLOAT F     = (FLOAT)(NumGammaLevels-GammaLevel)/(FLOAT)NumGammaLevels;
	FLOAT Gamma = 0.75 + 0.50 * F;
	//
	for (int i=0; i<256; i++)
		{
		DestColors->Red   = 255.0 * exp(Gamma * log(0.0001 + (FLOAT)SourceColors->Red   / 255.0));
		DestColors->Green = 255.0 * exp(Gamma * log(0.0001 + (FLOAT)SourceColors->Green / 255.0));
		DestColors->Blue  = 255.0 * exp(Gamma * log(0.0001 + (FLOAT)SourceColors->Blue  / 255.0));
		//
		SourceColors++;
		DestColors++;
		};
	UNGUARD("FGlobalGfx::GammaCorrectPalette");
	};

//
// Set the palette.  This function sets the screen palette to a
// gamma-corrected version of the source palette.
//
void FGlobalGfx::SetPalette(void)
	{
	GUARD;
	//
	GammaCorrectPalette(GammaPalette,DefaultPalette);
	GCameraManager->SetPalette(DefaultPalette,GammaPalette);
	//
	UNGUARD("FGlobalGfx::SetPalette");
	};

//
// Adjust a regular (imported) palette:
//
void FGlobalGfx::FixPalette (UPalette *Palette)
	{
	GUARD;
	//
	FColor  *Colors     = (FColor *)Palette->GetData();
	FColor  *TempColors = (FColor *)GMem.Get(256 * sizeof (FColor));
	int     iColor,iShade,iStart;
	//
	for (int i=0; i<256; i++) TempColors[i] = Colors[0];
	//
	for (iColor=0; iColor<8; iColor++)
		{
		if (iColor==0)	iStart=1;
		else			iStart=32*iColor;
		//
		for (iShade=0; iShade<28; iShade++)
			{
			TempColors[16 + iColor + (iShade<<3)] = Colors[iStart + iShade];
			};
		};
	memcpy (Colors,TempColors,256*sizeof(FColor));
	//
	Colors[0].RemapIndex=0;
	for (int i=1; i<256; i++) Colors[i].RemapIndex = i+0x10;
	//
	UNGUARD("FGlobalGfx::FixPalette");
	};

void* FGlobalGfx::GetPaletteTable(UTexture *CameraTexture,UPalette *Palette)
	{
	GUARD;
	//
	if (CameraTexture->ColorBytes==1)
		{
		return Palette;
		}
	else
		{
		int		CacheID	= ((int)Palette->Index<<16) + 0x0E00 + (CameraTexture->ColorBytes<<8) + CameraTexture->CameraCaps;
		void*	Result	= (void*)GCache.Get(CacheID);
		//
		if (Result) return Result;
		//
		Result			= (void*)GCache.Create(CacheID,256*CameraTexture->ColorBytes);
		FColor *Color	= &Palette->Element(0);
		//
		if ((CameraTexture->ColorBytes==2) && (CameraTexture->CameraCaps & CC_RGB565)) // RGB 5-6-5
			{
			WORD *HiColor = (WORD *)Result;
			for (int i=0; i<256; i++) *HiColor++ = (Color++)->HiColor565();
			}
		else if (CameraTexture->ColorBytes==2) // RGB 5-5-5
			{
			WORD *HiColor = (WORD *)Result;
			for (int i=0; i<256; i++) *HiColor++ = (Color++)->HiColor555();
			}
		else
			{
			DWORD *TrueColor = (DWORD *)Result;
			for (int i=0; i<256; i++) *TrueColor++ = (Color++)->TrueColor();
			};
		return Result;
		};
	UNGUARD("FGlobalGfx::GetPaletteTable");
	};

//
// Convert byte hue-saturation-brightness to floating point red-green-blue.
//
void FGlobalGfx::RGBtoHSV( FVector &Result,BYTE H,BYTE S,BYTE V,int CameraColorBytes )
{
	FLOAT	Brightness	= (FLOAT)V;
	FLOAT	Alpha		= (FLOAT)S / 255.0; // 0=full color, 1=full white

	if( CameraColorBytes == 1 )
	{

		Brightness *= (0.5/255.0) * (Alpha + 1.00);
		Brightness *= 0.70/(0.01 + sqrt(Brightness));
		Brightness = OurClamp(Brightness,(FLOAT)0.0,(FLOAT)1.0);

		Result.R = Result.G = Result.B = Brightness;
	}
	else
	{
		Brightness 		*= (1.4f/255.0f);
		Brightness		*= 0.70f/(0.01f + sqrtf(Brightness));
		Brightness		 = OurClamp(Brightness,(FLOAT)0.0,(FLOAT)1.0);

		FVector *Hue     = &HueData[H];

		Result.R	= (Hue->R + Alpha * (1.0 - Hue->R)) * Brightness;
		Result.G	= (Hue->G + Alpha * (1.0 - Hue->G)) * Brightness;
		Result.B	= (Hue->B + Alpha * (1.0 - Hue->B)) * Brightness;
	}
}

/*------------------------------------------------------------------------------
	FGlobalGfx Text output: Character drawer
------------------------------------------------------------------------------*/

//
// Draw a character onto a texture.
//
void FGlobalGfx::Cout(UTexture *DestTexture, int X, int Y, int XSpace, UFont *Font, int Color, char C, void* PaletteTable)
	{
	GUARD;
	FFontCharacter  *FontIndex  	= &Font->Element(C);
	UTexture		*SourceTexture	= Font->Texture;
	BYTE			*Source			= Font->Texture->GetData();
	BYTE			*Dest			= DestTexture->GetData();
	BYTE			*SourcePtr1,*SourcePtr,*DestPtr1,*DestPtr,B;
	int				U,V,EndX,EndY,XC;
	//
	U=0; if (X<0) {U+=(-X); X=0;}; EndX = X+FontIndex->USize-U; U += FontIndex->StartU;
	V=0; if (Y<0) {V+=(-Y); Y=0;}; EndY = Y+FontIndex->VSize-V; V += FontIndex->StartV;
	//
	if (EndX > DestTexture->USize) EndX = DestTexture->USize;
	if (EndY > DestTexture->VSize) EndY = DestTexture->VSize;
	//
	SourcePtr1 = &Source [U + V*SourceTexture->USize];
	if (DestTexture->ColorBytes==1)
		{
		int ColorOffset	= Color - 16;
		DestPtr1		= &Dest[X + Y*DestTexture->USize];
		while (Y++ < EndY)
			{
			SourcePtr = SourcePtr1;
			DestPtr   = DestPtr1;
			//
			for (XC=X; XC<EndX; XC++)
				{
				B = *SourcePtr++;
				if (B) *DestPtr = B + ColorOffset;
				DestPtr++;
				};
			SourcePtr1 += SourceTexture->USize;
			DestPtr1   += DestTexture  ->USize;
			};
		}
	else if (DestTexture->ColorBytes==2)
		{
		WORD *Table    = (WORD *)(PaletteTable ? PaletteTable : GetPaletteTable(DestTexture,DestTexture->Palette));
		WORD *DestPtr1 = (WORD *)&Dest[2*(X + Y*DestTexture->USize)],*DestPtr;
		while (Y++ < EndY)
			{
			SourcePtr = SourcePtr1;
			DestPtr   = DestPtr1;
			//
			for (XC=X; XC<EndX; XC++)
				{
				B = *SourcePtr++;
				if (B) *DestPtr = Table[B + Color];
				DestPtr++;
				};
			SourcePtr1 += SourceTexture->USize;
			DestPtr1   += DestTexture->USize;
			};
		}
	else if (DestTexture->ColorBytes==3)
		{
		// Not yet implemented !!
		}
	else if (DestTexture->ColorBytes==4)
		{
		DWORD *Table    = (DWORD *)(PaletteTable ? PaletteTable : GetPaletteTable(DestTexture,DestTexture->Palette));
		DWORD *DestPtr1 = (DWORD *)&Dest [4*(X + Y*DestTexture->USize)],*DestPtr;
		while (Y++ < EndY)
			{
			SourcePtr = SourcePtr1;
			DestPtr   = DestPtr1;
			//
			for (XC=X; XC<EndX; XC++)
				{
				B = *SourcePtr++;
				if (B) *DestPtr = *(DWORD *)&Table[B + Color];
				DestPtr++;
				};
			SourcePtr1 += SourceTexture->USize;
			DestPtr1   += DestTexture->USize;
			};
		};
	UNGUARD("FGlobalGfx::Cout");
	};

/*------------------------------------------------------------------------------
	FGlobalGfx Text output: String length functions
------------------------------------------------------------------------------*/

//
// Calculate the length of a string built from a font, starting at a specified
// position and counting up to the specified number of characters (-1 = infinite).
//
void FGlobalGfx_PartialStrLen(int *XL, int *YL, int XSpace, int YSpace, 
	UFont *Font, const char *Text,
	int iStart,int NumChars)
	{
	GUARD;
	FFontCharacter	*Index = Font->GetData();
	const char		*c;
	//
	*XL=0; *YL=0; c=&Text[iStart];
	//
	while(*c && NumChars)
		{
		*XL += Index [*c].USize + XSpace;
		*YL  = OurMax (*YL,Index[*c].VSize);
		c++;
		NumChars--;
		};
	*YL += YSpace;
	UNGUARD("FGlobalGfx::StrLen");
	};

//
// Calculate the size of a string built from a font:
//
void FGlobalGfx::StrLen (int *XL, int *YL, int XSpace, int YSpace, 
	UFont *Font, const char *Text)
	{
	GUARD;
	//
	FGlobalGfx_PartialStrLen(XL,YL,XSpace,YSpace,Font,Text,0,-1);
	//
	UNGUARD("FGlobalGfx::StrLen");
	};

//
// Calculate the size of a string built from a font, word wrapped
// to a specified region.
//
// Algorithm: 
//
void FGlobalGfx::WrappedStrLen(int *XL, int *YL, int XSpace, int YSpace, 
	UFont *Font, int MaxWidth, const char *Text)
	{
	GUARD;
	*XL=0; *YL=0;
	int iLine=0;
	int TestXL,TestYL;
	//
	// Process each output line:
	//
	while (Text[iLine])
		{
		//
		// Process each word until the current line overflows:
		//
		int iWord, iTestWord=iLine;
		do	{
			iWord = iTestWord;
			if (!Text[iTestWord]) break;
			//
			while (Text[iTestWord] && (Text[iTestWord]!=' ')) iTestWord++;
			while (Text[iTestWord]==' ') iTestWord++;
			//
			FGlobalGfx_PartialStrLen(&TestXL,&TestYL,XSpace,YSpace,Font,Text,iLine,iTestWord-iLine);
			} while (TestXL <= MaxWidth);
		if (iWord==iLine)
			{
			//
			// The text didn't fit word-wrapped onto this line, so chop it:
			//
			int iTestWord = iLine;
			do	{
				iWord   = iTestWord;
				if (!Text[iTestWord]) break;
				//
				iTestWord++;
				//
				FGlobalGfx_PartialStrLen(&TestXL,&TestYL,XSpace,YSpace,Font,Text,iLine,iWord-iLine);
				} while (TestXL <= MaxWidth);
			if (iWord==iLine) return; // Word wrap failed
			}
		//
		// Sucessfully split this line:
		//
		FGlobalGfx_PartialStrLen(&TestXL,&TestYL,XSpace,YSpace,Font,Text,iLine,iWord-iLine);
		*YL += TestYL;
		*XL  = OurMax(*XL,TestXL);
		// Go to the next line:
		while (Text[iWord]==' ') iWord++;
		iLine = iWord;
		};
	if (*XL>MaxWidth) appError("Logic error");
	UNGUARD("FGlobalGfx::WrappedStrLen");
	};

/*------------------------------------------------------------------------------
	FGlobalGfx Text output: String printing functions
------------------------------------------------------------------------------*/

//
// Draw a font onto a texture.  Normally called with a camera's
// screen texture.
//
void VARARGS FGlobalGfx::Printf (UTexture *DestTexture, int X, int Y, int XSpace, 
	UFont *Font, int Color, const char *Fmt,...)
	{
	va_list  ArgPtr;
	char	 Text[256];
	//
	va_start (ArgPtr,Fmt);
	vsprintf (Text,Fmt,ArgPtr);
	va_end   (ArgPtr);
	//
	GUARD;
	//
	FFontCharacter	*Index = Font->GetData();
	char *c = &Text[0];
	//
	while(*c != 0)
		{
		Cout (DestTexture,X,Y,XSpace,Font,Color,*c,GetPaletteTable(DestTexture,DestTexture->Palette));
		X += Index [*c].USize + XSpace;
		c ++;
		};
	UNGUARD("FGlobalGfx::Printf");
	};

void VARARGS FGlobalGfx::WrappedPrintf(UTexture *DestTexture,int X, int Y, int XSpace, int YSpace,
	UFont *Font, int Color, int Width, int Center, const char *Fmt,...)
	{
	va_list  ArgPtr;
	char	 Text[256];
	//
	va_start (ArgPtr,Fmt);
	vsprintf (Text,Fmt,ArgPtr);
	va_end   (ArgPtr);
	//
	GUARD;
	//
	int iLine=0;
	int TestXL,TestYL;
	//
	// Process each output line:
	//
	while (Text[iLine])
		{
		//
		// Process each word until the current line overflows:
		//
		int iWord, iTestWord=iLine;
		do	{
			iWord = iTestWord;
			if (!Text[iTestWord]) break;
			//
			while (Text[iTestWord] && (Text[iTestWord]!=' ')) iTestWord++;
			while (Text[iTestWord]==' ') iTestWord++;
			//
			FGlobalGfx_PartialStrLen(&TestXL,&TestYL,XSpace,YSpace,Font,Text,iLine,iTestWord-iLine);
			} while (TestXL <= Width);
		if (iWord==iLine)
			{
			//
			// The text didn't fit word-wrapped onto this line, so chop it:
			//
			int iTestWord = iLine;
			do	{
				iWord = iTestWord;
				if (!Text[iTestWord]) break;
				//
				iTestWord++;
				//
				FGlobalGfx_PartialStrLen(&TestXL,&TestYL,XSpace,YSpace,Font,Text,iLine,iWord-iLine);
				} while (TestXL <= Width);
			if (iWord==iLine) return; // Word wrap failed
			}
		//
		// Sucessfully split this line, now draw it:
		//
		char Temp[256];
		strcpy(Temp,&Text[iLine]);
		Temp[iWord-iLine]=0;
		//
		FGlobalGfx_PartialStrLen(&TestXL,&TestYL,XSpace,YSpace,Font,Text,iLine,iWord-iLine);
		if (TestXL>Width) appError("Logic error");
		//
		Printf
			(
			DestTexture,Center ? (X + ((Width-TestXL)>>1)) : X, Y,
			XSpace,Font,Color,"%s",Temp
			);
		Y += TestYL;
		//
		// Go to the next line:
		//
		while (Text[iWord]==' ') iWord++;
		iLine = iWord;
		};
	UNGUARD("FGlobalGfx::WrappedStrLen");
	};

/*------------------------------------------------------------------------------
	Font importing/processing
------------------------------------------------------------------------------*/

//
//	Fast pixel-lookup macro
//
//	a=screen buffer (byte pointer)
//	b=screen length (such as 320)
//	x=X coordinate
//	y=Y coordinate
//
inline BYTE AT(BYTE *Screen,int SXL,int X,int Y) {return Screen[X+Y*SXL];};

//
//	Find the border around a font character that starts at x,y (it's upper
//	left hand corner).  If it finds a character box, it returns 0 and the
//	character's length (xl,yl).  Otherwise returns -1.
//
// Doesn't check x or y for overflowing.
//
int FGlobalGfx::ScanFontBox (UTexture *Texture, int X,int Y,int *XL,int *YL)
	{
	GUARD;
	BYTE	*TextureData = Texture->GetData();
	int 	FontXL = Texture->USize;
	int 	NewXL,NewYL;
	//
	// Find x-length
	//
	NewXL = 1;
	while
		(
		(AT(TextureData,FontXL,X+NewXL,Y)==255) &&
		(AT(TextureData,FontXL,X+NewXL,Y+1)!=255)
		) NewXL++;
	if (AT(TextureData,FontXL,X+NewXL,Y)!=255) return -1;
	//
	// Find y-length
	//
	NewYL = 1;
	while
		(
		(AT(TextureData,FontXL,X,Y+NewYL)==255)&&
		(AT(TextureData,FontXL,X+1,Y+NewYL)!=255)
		) NewYL++;
	if (AT(TextureData,FontXL,X,Y+NewYL)!=255) return -1;
	//
	*XL=NewXL - 1;
	*YL=NewYL - 1;
	return 0;
	//
	UNGUARD("FGlobalGfx::ScanFontBox");
	};

UFont *FGlobalGfx::MakeFontFromTexture (UTexture *Texture)
	{
	GUARD;
	BYTE			*TextureData	= Texture->GetData();
	UFont			*Font			= new(Texture->Name,CREATE_Replace)UFont(256,1);
	FFontCharacter	*Index			= Font->GetData();
	int				i,X,Y,XL,YL,MaxYL;
	//
	Font->Texture = Texture;
	for (i=0; i<256; i++) // Init all characters to "unavailable"
		{
		Index[i].StartU = 0; Index[i].USize = 0;
		Index[i].StartV = 0; Index[i].VSize = 0;
		};
	//
	// Scan in all fonts, starting at character 32:
	//
	i = 32;
	Y = 0;
	//
	RowLoop:
	//
	X = 0;
	while ((AT(TextureData,Texture->USize,X,Y) != 255)&&(Y < Texture->VSize))
		{
		X++;
		if (X >= Texture->USize)
			{
			X = 0;
			Y++;
			//
			if (Y >= Texture->VSize) goto Done;
			};
		};
	//
	// Scan all characters in this row:
	//
	MaxYL = 0;
	//
	while ((i<256) && (ScanFontBox (Texture,X,Y,&XL,&YL)==0))
		{
		//bug ("C%i %i,%i - %i,%i",i,X,Y,XL,YL);
		Index[i].StartU = X+1;
		Index[i].StartV = Y+1;
		Index[i].USize  = XL;
		Index[i].VSize  = YL;
		//
		X += XL + 1;
		i ++;
		if (YL>MaxYL) MaxYL = YL;
		};
	Y = Y + MaxYL + 1; /* Proceed past end of this row. */
	//
	if ((i < 256) && (Y < Texture->VSize)) goto RowLoop;
	//
	Done: // Finished scanning font
	return Font;
	//
	UNGUARD("FGlobalGfx::MakeFontFromTexture");
	};

/*------------------------------------------------------------------------------
	Frame functions
------------------------------------------------------------------------------*/

//
// Darken a rectangular area
//
void FGlobalGfx::BurnRect (UTexture *Texture,int X1,int X2,int Y1,int Y2,int Bright)
	{
	GUARD;
	BYTE			*Data = Texture->GetData();
	BYTE			*Dest1,*Dest;
	DWORD			*DDest1,*DDest;
	int				i,j;
	//
	X1 = OurMax(0,OurMin(Texture->USize,X1));
	X2 = OurMax(0,OurMin(Texture->USize,X2));
	Y1 = OurMax(0,OurMin(Texture->VSize,Y1));
	Y2 = OurMax(0,OurMin(Texture->VSize,Y2));
	//
	if (Texture->ColorBytes==1)
		{
		Dest1  = &Data [X1 + Y1*Texture->USize];
		Bright = Bright?0x00:0x40;
		for (i=Y1; i<Y2; i++)
			{
			Dest = Dest1;
			for (j=X1; j<X2; j++)
				{
				*Dest = 0x10 + Bright + (*Dest & 7) + ((*Dest & 0xf0) >> 1);
				Dest++;
				};
			Dest1 += Texture->USize;
			};
		}
	else
		{
		DDest1  = (DWORD *)&Data[4*(X1 + Y1*Texture->USize)];
		for (i=Y1; i<Y2; i++)
			{
			DDest = DDest1;
			for (j=X1; j<X2; j++)
				{
				*DDest = (Bright?0x7f7f7f7f:0) + ((*DDest & 0xfefefefe)>>1);
				DDest++;
				};
			DDest1 += Texture->USize;
			};
		};
	UNGUARD("FGlobalGfx::BurnRect");
	};

void __inline FillDWORD(DWORD *Dest,DWORD D,int n)
	{
	#ifdef ASM
	if (n>0) __asm
		{
		mov edi,[Dest]
		mov esi,[n]
		mov eax,[D]
		;
		FillLoop:
		mov [edi],eax
		add edi,4
		dec esi
		jg  FillLoop
		};
	#else
		while (n-- > 0) *Dest++ = D;
	#endif
	};

//
// Clear the screen to a default palette color.
//
void FGlobalGfx::Clearscreen (ICamera *Camera,BYTE c)
	{
	int		Size4	= (Camera->SXR * Camera->ColorBytes) >> 2;
	int		Fill;
	//
	if (Camera->ColorBytes==1)
		{
		Fill = (DWORD)c + (((DWORD)c)<<8) + (((DWORD)c)<<16) + (((DWORD)c)<<24);
		}
	else if ((Camera->ColorBytes==2) && (Camera->Caps & CC_RGB565))
		{
		int W = DefaultColors[c].HiColor565();
		Fill = (W) + (W<<16);
		}
	else if (Camera->ColorBytes==2)
		{
		int W = DefaultColors[c].HiColor555();
		Fill = (W) + (W<<16);
		}
	else
		{
		Fill = DefaultColors[c].TrueColor();
		};
	DWORD *Dest = (DWORD *)Camera->Screen;
	for (int i=0; i<Camera->SYR; i++)
		{
		FillDWORD(Dest,Fill,Size4);
		Dest += (Camera->SXStride * Camera->ColorBytes) >> 2;
		};
	};

ICamera *GSavedCamera;
void FGlobalGfx::PreRender(ICamera *Camera)
	{
	GUARD;
	if (0 && (Camera->ColorBytes==4))
		{
		GSavedCamera=(ICamera *)GMem.Get(sizeof(ICamera));
		memcpy(GSavedCamera,Camera,sizeof(ICamera));
		//
		Camera->SXStride = (((Camera->SXR+1) >> 1)+3)&~3;
		Camera->PrecomputeRenderInfo
			(
			Camera->SXStride,
			((Camera->SYR+1) >> 1)
			);
		Camera->Screen = (BYTE *)GMem.Get(Camera->SXStride * Camera->SYR * Camera->ColorBytes);
		};
	if (Smooth)
		{
		GSavedCamera=(ICamera *)GMem.Get(sizeof(ICamera));
		memcpy(GSavedCamera,Camera,sizeof(ICamera));
		Camera->SXStride = Camera->SXR * 2;
		Camera->SYR      = Camera->SYR * 2;
		Camera->PrecomputeRenderInfo(Camera->SXStride,Camera->SYR);
		Camera->Screen   = (BYTE *)GMem.Get(Camera->SXStride * Camera->SYR * Camera->ColorBytes);
		};
	UNGUARD("FGlobalGfx::PreRender");
	};

//
// Finalize the stuff in the frame based on the current camera mode.
//
void FGlobalGfx::PostRender(ICamera *Camera)
	{
	GUARD;
	int 	i,j;
	BYTE	*Dest,*Source1,*Source2;
	//
	if (0 && (Camera->ColorBytes==4))
		{
		DWORD *Dest1 = (DWORD *)GSavedCamera->Screen;
		DWORD *Src1  = (DWORD *)Camera->Screen;
		int   n      = Camera->SXR;
		int   m      = GSavedCamera->SXStride;
		for (int i=0; i<Camera->SYR; i++)
			{
			DWORD *Src  = Src1;
			DWORD *Dest = Dest1;
			for (int j=0; j<Camera->SXR; j++)
				{
				DWORD A=Src[0];
				Dest[0]=A;
				Dest[1]=A;
				Dest[m]=A;
				Dest[m+1]=A;
				/*
				DWORD A = (Src[0  ]&0xfcfcfcfc)>>2;
				DWORD B = (Src[1  ]&0xfcfcfcfc)>>2;
				DWORD C = (Src[n  ]&0xfcfcfcfc)>>2;
				DWORD D = (Src[n+1]&0xfcfcfcfc)>>2;
				//
				Dest[0  ] = A*4 + B*2 + C*2 + D*1;
				Dest[1  ] = A*2 + B*4 + C*1 + D*2;
				Dest[m  ] = A*2 + B*1 + C*4 + D*2;
				Dest[m+1] = A*1 + B*2 + C*2 + D*4;
				*/
				//
				Dest += 2;
				Src  += 1;
				};
			Src1  += Camera->SXStride;
			Dest1 += GSavedCamera->SXStride << 1;
			};
		memcpy(Camera,GSavedCamera,sizeof(ICamera));
		GMem.Release(GSavedCamera);
		};
	//
	// Experimental stuff:
	//
	if (Smooth)
		{
		if (Camera->ColorBytes==1)
			{
			Source1 = Camera->Screen;
			Source2 = Camera->Screen + Camera->SXR;
			int Stride = Camera->SXStride;
			memcpy(Camera,GSavedCamera,sizeof(ICamera));
			Dest    = Camera->Screen;
			for (i=0; i<Camera->SYR; i++)
				{
				for (j=0; j<Camera->SXR; j++)
					{
					*Dest++ = GGfx.Blenders[BLEND_Transparent]
						[
						GGfx.Blenders[BLEND_Transparent][Source1[0] + ((int)Source1[1]<<8)] +
						((int)GGfx.Blenders[BLEND_Transparent][Source2[0] + ((int)Source2[1]<<8)]<<8)
						];
					Source1 += 2;
					Source2 += 2;
					};
				Source1 += Stride;
				Source2 += Stride;
				Dest    += Camera->SXStride - Camera->SXR;
				};
			};
		};
	UNGUARD("FGlobalGfx::PostRender");
	};

/*------------------------------------------------------------------------------
	FGlobalGfx table functions
------------------------------------------------------------------------------*/

void FGlobalGfx::LookupAllTables(void)
	{
	GUARD;
	//
	DefaultTexture		= new("Default",	FIND_Existing) UTexture;
	BkgndTexture		= new("Bkgnd",		FIND_Existing) UTexture;
	BackdropTexture		= new("Backdrop",	FIND_Existing) UTexture;
	BadTexture			= new("Bad",		FIND_Existing) UTexture;
	Logo				= new("Logo",		FIND_Existing) UTexture;
	//
	DefaultPalette		= new("Palette",	FIND_Existing) UPalette;
	TrueColorPalette	= new("TruePal",	FIND_Existing) UPalette;
	HugeFont			= new("f_huge",		FIND_Existing) UFont;
	LargeFont			= new("f_large",	FIND_Existing) UFont;
	MedFont				= new("f_tech",		FIND_Existing) UFont;
	SmallFont			= new("f_small",	FIND_Existing) UFont;
	ArrowBrush			= new("Arrow",		FIND_Existing) UModel;
	RootHullBrush		= new("RootHull",	FIND_Existing) UModel;
	//
	MenuUp				= new("b_menuup",	FIND_Existing) UTexture;
	MenuDn				= new("b_menudn",	FIND_Existing) UTexture;
	CollOn				= new("b_collon",	FIND_Existing) UTexture;
	CollOff				= new("b_colloff",	FIND_Existing) UTexture;
	PlyrOn				= new("b_plyron",	FIND_Existing) UTexture;
	PlyrOff				= new("b_plyroff",	FIND_Existing) UTexture;
	LiteOn				= new("b_liteon",	FIND_Existing) UTexture;
	LiteOff				= new("b_liteoff",	FIND_Existing) UTexture;
	//
	DefaultColors		= DefaultPalette->GetData();
	TrueColors			= TrueColorPalette->GetData();
	//
	// Prevent cached resources from being relocated during file load:
	//
	DefaultPalette  ->SetFlags(RF_NoReplace); 
	TrueColorPalette->SetFlags(RF_NoReplace);
	//
	UNGUARD("FGlobalGfx::LookupAllTables");
	};

void FGlobalGfx::LookupAllLuts(void)
	{
	GUARD;
	//
	UBuffer *AverageTable=new("Average",FIND_Existing)UBuffer;
	ShadeTable					= new("Shade",		FIND_Existing) UBuffer;
	BlendTable					= new("Blend",		FIND_Existing) UBuffer;
	GhostTable					= new("Ghost",		FIND_Existing) UBuffer;
	GlowTable					= new("Glow",		FIND_Existing) UBuffer;
	HueTable					= new("Hue",		FIND_Existing) UBuffer;
	SincTable					= new("Sinc",		FIND_Existing) UBuffer;
	ShadeData					= (BYTE *)ShadeTable->GetData();
	Blenders[BLEND_None]		= NULL;
	Blenders[BLEND_Transparent]	= (BYTE *)BlendTable->GetData();
	Blenders[BLEND_Ghost]		= (BYTE *)GhostTable->GetData();
	Blenders[BLEND_Glow]		= (BYTE *)GlowTable->GetData();
	Blenders[BLEND_Average]		= (BYTE *)AverageTable->GetData();
	HueData						= (FVector *)HueTable->GetData();
	SincData					= (FLOAT *)SincTable->GetData();
	//
	ShadeTable ->SetFlags(RF_NoReplace);
	//
	UNGUARD("FGlobalGfx::LookupAllLuts");
	};

/*------------------------------------------------------------------------------
	FGlobalGfx Init & Exit
------------------------------------------------------------------------------*/

//
// Initialize the graphics engine and allocate all stuff.
// Calls appError if failure.
//
void FGlobalGfx::Init (void)
	{
	GUARD;
	FColor			Color,*C1,*C2;
	int 			ShadeWeight[32],i,j;
	//
	// Init color table using non-Windows colors
	//
	BlackColor				= 233;	// Pure non-Windows black
	WhiteColor				= 16;	// Pure non-Windows white
	MaskColor				= 0;	// Invisible color for sprites and masked textures
	//
	WorldBoxColor			= COLOR(P_BLUE,10);
	GroundPlaneHighlight	= COLOR(P_BLUE,8);
	GroundPlaneColor		= COLOR(P_BLUE,16);
	NormalColor				= COLOR(P_GREY,0);
	BrushFillColor			= COLOR(P_FIRE,0);
	BrushWireColor			= COLOR(P_RED,0);
	AddWireColor			= COLOR(P_BLUE,0);
	SubtractWireColor		= COLOR(P_WOOD,0);
	GreyWireColor			= COLOR(P_GREY,4);
	InvalidColor			= COLOR(P_GREY,4);
	BspWireColor			= COLOR(P_FLESH,6);
	BspFillColor			= COLOR(P_FLESH,16);
	SelectColor				= COLOR(P_BLUE,8);
	SelectBorderColor		= COLOR(P_FLESH,4);
	PivotColor				= COLOR(P_WOOD,0);
	ActorDotColor			= COLOR(P_BROWN,0);
	ActorWireColor			= COLOR(P_BROWN,8);
	ActorHiWireColor		= COLOR(P_BROWN,2);
	ActorFillColor			= COLOR(P_BROWN,24);
	NonSolidWireColor		= COLOR(P_GREY,4);
	SemiSolidWireColor		= COLOR(P_GREEN,4);
	NormalFontColor			= COLOR(P_FIRE,1);
	WireBackground			= COLOR(P_GREY,4);
	WireGridAxis			= COLOR(P_GREY,12);
	ActorArrowColor			= COLOR(P_RED,4);
	ScaleBoxColor			= COLOR(P_FIRE,6);
	ScaleBoxHiColor			= COLOR(P_FIRE,0);
	ZoneWireColor			= COLOR(P_BLUE,0);
	MoverColor				= COLOR(P_GREEN,2);
	//
	// Allocate global graphics array:
	//
	Graphics = new("Graphics",CREATE_Unique)UArray(256);
	//
	// Init misc
	//
	DefaultCameraFlags = SHOW_Frame | SHOW_Actors | SHOW_MovingBrushes;
	//
	// Allocate all palettes:
	//
	GammaPalette = new("Gamma",CREATE_Unique)UPalette;
	GammaColors  = GammaPalette->AllocData();
	Graphics->Add(GammaPalette);
	//
	// Gamma parameters:
	//
	NumGammaLevels  = 8;
	GammaLevel		= 4;
	//
	// Add graphics array to root:
	//
	GRes.Root->Add(Graphics);
	debug(LOG_Init,"Graphics initialized");
	//
	// Load graphics tables if present, otherwise regenerate.
	//
	GRes.AddFile ("Unreal.gfx");
	Tables = new("GfxTables",FIND_Optional)UArray;
	if (!Tables)
		{
		GApp->BeginSlowTask("Building Unreal.gfx",1,0);
		Tables = new("GfxTables",CREATE_Unique)UArray(256);
		debug (LOG_Init,"Building Unreal.gfx");
		//
		// Regenerate all graphics tables
		//
		DefaultColors  = NULL; // Required by palette importer
		DefaultPalette = new("Palette",CREATE_Replace)UPalette;
		if (!DefaultPalette->ImportFromFile(DEFAULT_PALETTE_FNAME))
			{
			appError ("Error loading default palette");
			};
		FixPalette (DefaultPalette);
		DefaultColors	= DefaultPalette->GetData();
		Tables->Add(DefaultPalette);
		//
		// Call import script to load editor stuff (marker sprites, fonts, etc):
		//
		GUARD;
		if (!GEditor) appError("Can't find graphics resources");
		GEditor->Exec("MACRO PLAY NAME=Startup FILE=" GFX_BOOTSTRAP_FNAME);
		UNGUARD("Parsing " GFX_BOOTSTRAP_FNAME);
		//
		// Reverse palette:
		//
		TrueColorPalette = new("TruePal",CREATE_Unique)UPalette;
		TrueColors       = TrueColorPalette->AllocData();
		for (i=0; i<256; i++)
			{
			TrueColors[i].Red	= DefaultColors[i].Blue;
			TrueColors[i].Green	= DefaultColors[i].Green;
			TrueColors[i].Blue	= DefaultColors[i].Red;
			};
		Tables->Add(TrueColorPalette);
		//
		// Finish & save new table to disk:
		//
		GApp->StatusUpdate ("Saving",0,0);
		GRes.SaveDependent (Tables,"Unreal.gfx",FILE_NONE);
		GApp->EndSlowTask();
		};
	LookupAllTables();
	Graphics->Add(Tables);
	//
	GRes.AddFile ("Unreal.tab");
	Luts = new("GfxLuts",FIND_Optional)UArray;
	if (!Luts)
		{
		GApp->BeginSlowTask("Building Unreal.tab",1,0);
		Luts = new("GfxLuts",CREATE_Unique)UArray(16);
		debug (LOG_Init,"Building Unreal.tab");
		//
		UBuffer *AverageTable;
		//
		ShadeTable   = new("Shade",      CREATE_Replace,RF_64KAlign)UBuffer(4*64*256,1);
		BlendTable   = new("Blend",      CREATE_Replace,RF_64KAlign)UBuffer(256*256,1);
		GhostTable   = new("Ghost",      CREATE_Replace,RF_64KAlign)UBuffer(256*256,1);
		GlowTable    = new("Glow",       CREATE_Replace,RF_64KAlign)UBuffer(256*256,1);
		AverageTable = new("Average",    CREATE_Replace,RF_64KAlign)UBuffer(256*256,1);
		HueTable	 = new("Hue",        CREATE_Replace            )UBuffer(256*sizeof(FVector),1);
		SincTable	 = new("Sinc",		 CREATE_Replace			   )UBuffer(256*sizeof(FLOAT),1);
		//
		Luts->Add(ShadeTable);
		Luts->Add(BlendTable);
		Luts->Add(GhostTable);
		Luts->Add(GlowTable);
		Luts->Add(AverageTable);
		Luts->Add(HueTable);
		Luts->Add(SincTable);
		//
		LookupAllLuts();
		//
		// Shade weight table:
		//
		ShadeWeight[31] = 0;
		for (i=1; i<32; i++)
			{
			ShadeWeight[i] = (int)(65536.0 * exp (0.95 * log ((FLOAT)(31-i)/31.0)));
			};
		//
		// Shade table:
		//
		for (i=0; i<256; i++)
			{
			GApp->StatusUpdate ("Generating shade table",i,256);
			//
			C1 = &DefaultColors[i];
			//
			FVector C;
			C.X = (FLOAT)C1->Red   / 255.0;
			C.Y = (FLOAT)C1->Green / 255.0;
			C.Z = (FLOAT)C1->Blue  / 255.0;
			//
			for (j=0; j<64; j++)
				{
				FLOAT	F	= 2.4 * (FLOAT)j/63.0;
				FVector D	= C * F;
				//
				Color.Red   = (int)OurMin(255.0 * D.X, 255.0);
				Color.Green = (int)OurMin(255.0 * D.Y, 255.0);
				Color.Blue  = (int)OurMin(255.0 * D.Z, 255.0);
				//
				ShadeData[i + j*256 + 0x0000] = DefaultPalette->BestMatch(Color);
				};
			};
		for (j=0; j<256; j++) ShadeData[j*256]=0; // Prevent mask from being remapped
		//
		// Hue table (for HSV color):
		//
		for (i=0; i<86; i++)
			{
			FLOAT F = (FLOAT)i/85.0;
			//
			HueData[i		].X = 1.0 - F; // XYZ=RGB
			HueData[i		].Y = F;
			HueData[i		].Z = 0;
			//
			HueData[i+85	].X = 0;
			HueData[i+85	].Y = 1.0 - F;
			HueData[i+85	].Z = F;
			//
			HueData[i+170	].X = F;
			HueData[i+170	].Y = 0;
			HueData[i+170	].Z = 1.0 - F;
			};
		//
		// Sinc tables:
		//
		for (int i=0; i<256; i++) // Very rough sinc filter approximation
			{
			//SincData[i] = 0.5 - 0.5 * cos(PI * i / 256.0);
			//SincData[i] = (FLOAT)i/255.0;
			SincData[i] = 0.5 * (i/255.0 + (0.5 - 0.5 * cos(PI * i / 256.0)));
			};
		//
		// Blend and ghost tables:
		//
		for (i=0; i<256; i++)
			{
			GApp->StatusUpdate ("Generating blend tables",i,256);
			C1 = &DefaultColors[i];
			for (j=0; j<256; j++)
				{
				C2 = &DefaultColors[j];
				//
				Color.Red	= (8*(int)C1->Red  +8*(int)C2->Red  ) >> 4;
				Color.Green = (8*(int)C1->Green+8*(int)C2->Green) >> 4;
				Color.Blue  = (8*(int)C1->Blue +8*(int)C2->Blue ) >> 4;
				Blenders[BLEND_Transparent][i*256+j] = DefaultPalette->BestMatchInRange(Color,j);
				//
				Color.Red	= (5*(int)C1->Red  +3*(int)C2->Red  ) >> 3;
				Color.Green = (5*(int)C1->Green+3*(int)C2->Green) >> 3;
				Color.Blue  = (5*(int)C1->Blue +3*(int)C2->Blue ) >> 3;
				Blenders[BLEND_Ghost][i*256+j] = DefaultPalette->BestMatchInRange(Color,i);
				//
				Color.Red	= OurClamp((int)C1->Red   + (int)C2->Red   - (((int)C1->Red  *(int)C2->Red  )>>8),0,255);
				Color.Green = OurClamp((int)C1->Green + (int)C2->Green - (((int)C1->Green*(int)C2->Green)>>8),0,255);
				Color.Blue  = OurClamp((int)C1->Blue  + (int)C2->Blue  - (((int)C1->Blue *(int)C2->Blue )>>8),0,255);
				Blenders[BLEND_Glow][i*256+j] = DefaultPalette->BestMatch(Color);
				//
				Color.Red	= ((int)C1->Red   + (int)C2->Red  ) >> 1;
				Color.Green = ((int)C1->Green + (int)C2->Green) >> 1;
				Color.Blue  = ((int)C1->Blue  + (int)C2->Blue ) >> 1;
				Blenders[BLEND_Average][i*256+j] = DefaultPalette->BestMatch(Color);
				};
			};
		for (i=0; i<256; i++) // Force blending tables to leave masked color as-is
			{
			Blenders[BLEND_Transparent][i    ]=0;
			Blenders[BLEND_Ghost      ][i    ]=0;
			Blenders[BLEND_Glow       ][i    ]=0;
			Blenders[BLEND_Average    ][i    ]=0;
			//
			Blenders[BLEND_Transparent][i*256]=i;
			Blenders[BLEND_Ghost      ][i*256]=i;
			Blenders[BLEND_Glow       ][i*256]=i;
			Blenders[BLEND_Average    ][i*256]=i;
			};
		GApp->StatusUpdate ("Saving",0,0);
		GRes.SaveDependent (Luts,"Unreal.tab",FILE_NONE);
		GApp->EndSlowTask();
		};
	LookupAllLuts();
	Graphics->Add(Luts);
	//
	// Init square table:
	//
	for (i=0; i<512; i++) Squares[i] = (i-256)*(i-256);
	//
	SetPalette();
	//
	UNGUARD("FGlobalGfx::Init");
	};

//
// Shut down graphics.
//
void FGlobalGfx::Exit (void)
	{
	GUARD;
	GRes.Root->Delete(Graphics);
	debug(LOG_Exit,"Graphics closed");
	UNGUARD("FGlobalGfx::Exit");
	};

/*------------------------------------------------------------------------------
	UFont implementation
------------------------------------------------------------------------------*/

void UFont::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize    = sizeof (UFont);
	Type->RecordSize    = sizeof (FFontCharacter);
	Type->Version       = 1;
	strcpy (Type->Descr,"Font");
	UNGUARD("");
	};
void UFont::InitHeader(void)
	{
	GUARD;
	Texture		= NULL;
	Max			= 0;
	Num			= 0;
	UNGUARD("");
	};
void UFont::InitData(void)
	{
	GUARD;
	Num = 0;
	UNGUARD("");
	};
void UFont::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	Callback.Resource(this,(UResource **)&Texture,0);
	UNGUARD("");
	};
AUTOREGISTER_RESOURCE(RES_Font,UFont,0xB2D90856,0xCCD211cf,0x91360000,0xC028B992);

/*------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------*/
