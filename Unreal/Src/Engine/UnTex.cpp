/*=============================================================================
	UnTex.cpp: Unreal texture loading/saving/processing functions.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

#define DIFFUSION_DITHER        0 /* Diffusion dither textures when rendering in 256-color */
#define BLUR_TEXTURES_ON_IMPORT 1 /* Blur textures while importing them */

/*-----------------------------------------------------------------------------
	Texture dithering
-----------------------------------------------------------------------------*/

//
// Diffusion-dither a single line of a texture:
//
static int GRedError, GGreenError, GBlueError;
void inline Disperse (BYTE *Src, BYTE *Dest,int Direction,int USize,FColor *Colors,FPalettePrecalc *Precalc,
	int FixDiffusionC,int Masked)
	{
	GUARD;
	//
	#if !DIFFUSION_DITHER
		//
		// No diffusion dithering version:
		//
		#ifdef ASM
		__asm
			{
			push	ebp
			mov		edi,[Dest]
			mov		esi,[Src]
			mov		edx,[Colors]
			mov		ecx,[Direction]
			xor		eax,eax
			mov		ebp,[USize]
			;
			RemapLoop:
			;
			mov		al,[esi]							; eax = *Src
			add		esi,ecx								; Src += Direction
			;
			mov		bl,[edx + eax*4]FColor.RemapIndex	; bl = Colors[*Src].RemapIndex
			xor		eax,eax								; Zero eax
			;
			mov		[edi],bl							; *Dest = Colors[*Src].RemapIndex
			add		edi,ecx								; Dest += Direction
			;
			dec		ebp
			jg		RemapLoop
			;
			pop		ebp
			};
		#else
		while (USize-- > 0)
			{
			*Dest = Colors[*Src].RemapIndex;
			Src		+= Direction;
			Dest	+= Direction;
			};
		#endif
	#else
		//
		// Diffusion dithering version:
		//
		// This is a bit too slow for realtime in C, but if it were hand-optimized in
		// assembly, it would be plenty fast.  The algorithm is efficient.  However,
		// as diffusion dithering only trades discretization artifacts for random color 
		// artifacts, it is currently disabled.
		//
		INT		*Squares		= &GGfx.Squares[256];
		FColor	*DefaultColors	= &GGfx.DefaultColors[0];
		while (USize-- > 0)
			{
			int iColor = *Src;
			if (iColor || !Masked)
				{
				FColor *ThisColor	= &Colors[iColor];
				int DesiredRed		= ThisColor->Red	+ GRedError;
				int DesiredGreen	= ThisColor->Green	+ GGreenError;
				int DesiredBlue		= ThisColor->Blue	+ GBlueError;
				//
				int iBestColor		= Precalc[iColor].Nearest[0];
				FColor *Color		= &DefaultColors[iBestColor];
				int BestDelta		=
					Squares[DesiredRed		- Color->Red	] +
					Squares[DesiredGreen	- Color->Green	] +
					Squares[DesiredBlue		- Color->Blue	];
				for (int i=1; i<FPalettePrecalc::NUM_NEAREST; i++)
					{
					int iTest		= Precalc[iColor].Nearest[i];
					Color			= &DefaultColors[iTest];
					int Delta		=
						Squares[DesiredRed		- Color->Red	] +
						Squares[DesiredGreen	- Color->Green	] +
						Squares[DesiredBlue		- Color->Blue	];
					if (Delta<BestDelta)
						{
						iBestColor	= iTest;
						BestDelta	= Delta;
						};
					};
				*Dest		= iBestColor;
				Color		= &GGfx.DefaultColors[iBestColor];
				GRedError	= ((DesiredRed	- Color->Red	)	* FixDiffusionC) >> 6;
				GGreenError	= ((DesiredGreen- Color->Green	)	* FixDiffusionC) >> 6;
				GBlueError	= ((DesiredBlue	- Color->Blue	)	* FixDiffusionC) >> 6;
				}
			else *Dest = 0;
			//
			Src		+= Direction;
			Dest	+= Direction;
			};
	#endif
	UNGUARD("Disperse");
	};

//
// Remap a texture using diffusion dithering.
//
void DiffusionDither(BYTE *Src, BYTE *Dest, UPalette *Palette,int USize,int VSize,
	int FixDiffusionC,int Masked)
	{
	GUARD;
	if (!Palette) appError("No palette");
	FColor			*Colors		= Palette->GetData();
	FPalettePrecalc	*Precalc	= (FPalettePrecalc *)&Colors[256];
	int				V;
	//
	if (Palette != GGfx.DefaultPalette)
		{
		GRedError=0; GGreenError=0; GBlueError=0;
		for (V=0; V<VSize; V+=2)
			{
			if (V&1) GApp->StatusUpdate ("Remapping",V,VSize);
			//
			Disperse(Src,Dest,1,USize,Colors,Precalc,FixDiffusionC,Masked);
			Dest += USize; Src += USize;
			//
			Disperse(Src+USize-1,Dest+USize-1,-1,USize,Colors,Precalc,FixDiffusionC,Masked);
			Dest += USize; Src += USize;
			};
		}
	else // No remapping necessary
		{
		for (V=0; V<VSize; V++)
			{
			memcpy (Dest,Src,USize);
			Dest += USize;
			Src  += USize;
			};
		};
	UNGUARD("DiffusionDither");
	};

/*---------------------------------------------------------------------------------------
	PCX stuff for texture and palette importers
---------------------------------------------------------------------------------------*/

//
// 128-byte header found at the beginning of a ".PCX" file.
// (Thanks to Ammon Campbell for putting this together)
//
class FPCXFileHeader
	{
	public:
	unsigned char	Manufacturer;		// Always 10
	unsigned char	Version;			// PCX file version
	unsigned char	Encoding;			// 1=run-length, 0=none
	unsigned char	BitsPerPixel;		// 1,2,4, or 8
	//
	unsigned short	XMin;				// Dimensions of the image
	unsigned short	YMin;
	unsigned short	XMax;
	unsigned short	YMax;
	//
	unsigned short	hdpi;				// Horizontal printer resolution
	unsigned short	vdpi;				// Vertical printer resolution
	//
	unsigned char	OldColorMap[48];	// Old colormap info data
	//
	unsigned char	Reserved1;			// Must be 0
	//
	unsigned char	NumPlanes;			// Number of color planes (1, 3, 4, etc)
	unsigned short	BytesPerLine;		// Number of bytes per scanline
	unsigned short	PaletteType;		// How to interpret palette: 1=color, 2=gray
	//
	unsigned short	HScreenSize;		// Horizontal monitor size
	unsigned short	VScreenSize;		// Vertical monitor size
	//
	unsigned char	Reserved2[54];		// Must be 0
	//
	};

/*---------------------------------------------------------------------------------------
	UTexture resource implementation
---------------------------------------------------------------------------------------*/

void UTexture::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UTexture);
	Type->RecordSize = 0;
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Texture");
	UNGUARD("UTexture::Register");
	};
void UTexture::InitHeader(void)
	{
	GUARD;
	//
	Class			= GClasses.TextureRes;
	Palette			= NULL;
	Microtexture	= NULL;
	FireParams		= NULL;
	FamilyName      = NAME_NONE;
	UnusedName      = NAME_NONE;
	//
	DiffuseC		= 1.0;
	SpecularC		= 0.0;
	PalDiffusionC	= 0.5;
	FrictionC		= 0.5;
	//
	FootstepSound	= NULL;
	HitSound		= NULL;
	PolyFlags		= 0;
	bNoTile			= 0;
	//
	USize			= 0;
	VSize			= 0;
	DataSize		= 0;
	ColorBytes		= 0;
	LockCount		= 0;
	CameraCaps		= 0;
	//
	MipZero.Red		= 0;
	MipZero.Green	= 0;
	MipZero.Blue	= 0;
	MipZero.RemapIndex = 0;
	//
	for (int i=0; i<MAX_MIPS; i++) MipOfs[i]=MAXDWORD;
	//
	UNGUARD("UTexture::InitHeader");
	};
void UTexture::InitData(void)
	{
	GUARD;
	mymemset (Data,0,DataSize);
	UNGUARD("UTexture::InitData");
	};
int UTexture::QuerySize(void)
	{
	GUARD;
	return DataSize;
	UNGUARD("UTexture::QuerySize");
	};
int UTexture::QueryMinSize(void)
	{
	GUARD;
	return QuerySize();
	UNGUARD("UTexture::QueryMinSize");
	};
const char *UTexture::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	FPCXFileHeader	*PCX = (FPCXFileHeader *)Buffer;
	INT				RunLength;
	BYTE  			Color,*DestEnd,*DestPtr;
	//
	// Validate:
	//
	int Length = (int)(BufferEnd-Buffer);
	if (Length < sizeof(FPCXFileHeader)) return NULL; // Doesn't contain valid header
	if (PCX->Manufacturer != 10) return NULL; // Unknown format
	if ((PCX->BitsPerPixel!=8) || (PCX->NumPlanes!=1)) return NULL; // Bad format, must have 8 bits per pixel, 1 plane
	//
	// Set texture properties, assume stuff was validated in RMSG_IMPORTSIZE.
	//
	USize		= PCX->XMax + 1 - PCX->XMin;
	VSize		= PCX->YMax + 1 - PCX->YMin;
	UBits		= FLogTwo(USize);
	VBits		= FLogTwo(VSize);
	DataSize	= USize * VSize;
	Palette	    = NULL;
	//
	Realloc();
	//
	// debugf(LOG_REND,"X=%i,%i Y=%i,%i",PCX->pcx_xmin,PCX->pcx_xmax,PCX->pcx_ymin,PCX->pcx_ymax);
	//
	DestPtr	= GetData();
	DestEnd	= DestPtr + DataSize;
	//
	Buffer += 128;
	while (DestPtr<DestEnd)
		{
		Color = *Buffer++;
		if ((Color & 0xc0) == 0xc0)
			{
			RunLength = Color & 0x3f;
			Color     = *Buffer++;
			mymemset (DestPtr,Color,OurMin(RunLength,(int)(DestEnd - DestPtr)));
			DestPtr  += RunLength;
			}
		else *DestPtr++ = Color;
		};
	if ((USize != (INT)FNextPowerOfTwo(USize)) ||
		(VSize != (INT)FNextPowerOfTwo(VSize)) ||
		(USize>1024) || (VSize>1024))
		{
		bNoTile = 1;
		};
	MipOfs[0] = 0;
	return BufferEnd;
	UNGUARD("UTexture::Import");
	};
char *UTexture::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	FColor			*Colors;
	FPCXFileHeader	*PCX;
	DWORD 			XYSize,i,Size;
	BYTE  			Color,*ScreenPtr,*BufferPtr;
	//
	// Set all PCX file header properties
	//
	PCX  = (FPCXFileHeader *)Buffer;
	Size = sizeof (FPCXFileHeader);
	mymemset (PCX,0,Size);
	//
	PCX->Manufacturer	= 10;
	PCX->Version		= 05;
	PCX->Encoding		= 1;
	PCX->BitsPerPixel	= 8;
	PCX->XMin			= 0;
	PCX->YMin			= 0;
	PCX->XMax			= USize-1;
	PCX->YMax			= VSize-1;
	PCX->hdpi			= USize;
	PCX->vdpi			= VSize;
	PCX->NumPlanes		= 1;
	PCX->BytesPerLine	= USize;
	PCX->PaletteType	= 0;
	PCX->HScreenSize	= 0;
	PCX->VScreenSize	= 0;
	//
	// Copy all RLE bytes:
	//
	XYSize    = USize*VSize;
	BufferPtr = (BYTE *)Buffer;
	ScreenPtr = &GetData()[MipOfs[0]];
	for (i=0; i<XYSize; i++)
		{
		Color = *(ScreenPtr++);
		if ((Color&0xc0)!=0xc0)
			{
			BufferPtr [Size++] = Color;					
			}
		else
			{
			BufferPtr [Size++] = 0xc1; // Run length = 1
			BufferPtr [Size++] = Color;
			};
		};
	//
	// Build palette:
	//
	BufferPtr[Size++] = 12; // Required before palette by PCX format
	//
	if (!Palette)	Colors = GGfx.DefaultPalette->GetData();
	else			Colors = Palette->GetData();
	for (i=0; i<UPalette::NUM_PAL_COLORS; i++)
		{
		BufferPtr [Size++] = Colors[i].Red;
		BufferPtr [Size++] = Colors[i].Green;
		BufferPtr [Size++] = Colors[i].Blue;
		};
	return Buffer+Size;
	UNGUARD("UTexture::Export");
	};
void UTexture::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	//
	Callback.Resource (this,(UResource **)&Class,0);
	Callback.Resource (this,(UResource **)&Palette,0);
	Callback.Resource (this,(UResource **)&Microtexture,0);
	Callback.Resource (this,(UResource **)&FireParams,0);
	//
	Callback.Resource (this,(UResource **)&FootstepSound,0);
	Callback.Resource (this,(UResource **)&HitSound,0);
	//
	Callback.Name     (this,&FamilyName,0);
	Callback.Name     (this,&UnusedName,0);
	//
	UNGUARD("UTexture::QueryHeaderReferences");
	};
AUTOREGISTER_RESOURCE(RES_Texture,UTexture,0xB2D90876,0xCCD211cf,0x91360000,0xC028B992);

//
// Initialize a remap table
//
void UTexture::InitRemap (BYTE *Remap)
	{
	GUARD;
	for (int i=0; i<UPalette::NUM_PAL_COLORS; i++) Remap[i] = 0;
	Remap[0]   = 0;
	Remap[255] = 255;
	UNGUARD("UTexture::InitRemap");
	};

//
// Remap a texture according to a remap table
//
void UTexture::DoRemap (BYTE *Remap)
	{
	GUARD;
	BYTE *Dest = GetData();
	for (int i=0; i<DataSize; i++) *(Dest++) = Remap[*Dest];
	UNGUARD("UTexture::DoRemap");
	};

//
// Remap a texture's palette from its current palette to
// the specified destination palette
//
void UTexture::Remap (UPalette *SourcePalette, UPalette *DestPalette)
	{
	GUARD;
	BYTE Remap[UPalette::NUM_PAL_COLORS];
	//
	// Find all best-match colors.  Remap first and last 10 (Windows) colors to mask.
	// Don't remap any colors to zero (black).
	//
	if (PolyFlags & PF_Masked)	Remap[0] = 0;
	else						Remap[0] = DestPalette->BestMatch(SourcePalette->Element(0));
	//
	for (int i=1; i<256; i++)	Remap[i] = DestPalette->BestMatch(SourcePalette->Element(i));
	//
	DoRemap (Remap);
	//
	UNGUARD("UTexture::Remap");
	};

//
// Rearrange a texture's colors to fit the additive-lighting palette
//
void UTexture::Fixup (void)
	{
	GUARD;
	BYTE Remap[UPalette::NUM_PAL_COLORS];
	int  iColor,iShade,iStart;
	//
	InitRemap(Remap);
	for (iColor=0; iColor<8; iColor++)
		{
		if (iColor==0)	iStart = 1;
		else			iStart = iColor * 32;
		//
		for (iShade=0; iShade<28; iShade++)
			{
			Remap[iStart + iShade] = 32 + iColor + (iShade<<3);
			};
		};
	DoRemap (Remap);
	UNGUARD("UTexture::Fixup");
	};

/*---------------------------------------------------------------------------------------
	Texture locking and unlocking
---------------------------------------------------------------------------------------*/

//
// Lock a texture and make sure all needed precomputed info is in place.
// Call with:
//
//    Mip = mipmap number to get, or -1 = all mipmaps.  If Mip>=0, sets Data to the
//          data pointer for the specified mipmap and leaves Mips[] undefined.
//          If Mip<0, sets Data to the data pointer for mip 0 and Mips[] to the appropriate
//          info for all mipmap levels up to MAX_MIPS.  Note that if less than MAX_MIPS exist,
//          the remaining Mips[] are filled in with the last mip's information.
//
//   If Flags|TL_Normalized, returns normalized texture data, 256-byte texture data which is
//          remapped to the palette in 256-color modes, or as-is in 16- and 24-bit modes.
//
//   If Flags|TL_Renderable, makes the texture hardware-renderable if 3D hardware is active.
//
void UTexture::Lock(ITexture *TextureInfo,ICamera *Camera,int Mip,int Flags)
	{
	GUARD;
	//
	TextureInfo->Texture = this;
	TextureInfo->Palette = Palette ? Palette : GGfx.DefaultPalette;
	//
	if (Mip>=0)	TextureInfo->Mip = Mip; // Get a particular mipmap
	else		TextureInfo->Mip = 0;	// Get all mipmaps
	//
	// Get base texture pointer:
	//
	if (Flags & TL_Remapped)
		{
		// If in 256-color mode, return data remapped to 256 colors:
		TextureInfo->Data = GetOriginalData(&TextureInfo->Mip,&TextureInfo->USize,&TextureInfo->VSize);
		}
	else
		{
		TextureInfo->Data = GetData(&TextureInfo->Mip,Camera->ColorBytes,&TextureInfo->USize,&TextureInfo->VSize);
		};
	//
	// Get colors:
	//
	TextureInfo->Colors = &TextureInfo->Palette->Element(0);
	//
	UNGUARD("UTexture::Lock");
	};

//
// Unlock a texture.
//
void UTexture::Unlock(ITexture *TextureInfo)
	{
	GUARD;
	//
	// Does nothing yet.
	//
	UNGUARD("UTexture::Unlock");
	};

/*---------------------------------------------------------------------------------------
	UTexture mipmap generation
---------------------------------------------------------------------------------------*/

#define DO_RGB(x) x(R); x(G); x(B); /* Macro for doing the x thing to each RGB component */

//
// Generate all mipmaps for a texture.  Call this after setting the
// texture's palette.
//
void UTexture::CreateMips(int FullMips)
	{
	GUARD;
	UPalette	*ThisPalette = Palette ? Palette : GGfx.DefaultPalette;
	FColor		*Colors		 = ThisPalette->GetData();
	FColor		*SourceColor,ResultColor;
	BYTE		*Texel,*SourceTex,*DestTex,B;
	INT			ThisUSize,ThisVSize,HalfUSize,HalfVSize,U,V,X,Y,UAnd,VAnd,Red,Green,Blue,MipLevel,ThisSize,i,n;
	static const int BoxC[4][4] =
		{
		{ 1, 2, 2, 1},
		{ 2,13,11, 2},
		{ 2,10, 9, 2},
		{ 1, 2, 2, 1}
		};
	enum {BOX_SHIFT = 6};
	enum {BOX_SUM   = 64};
	//
	// Create average color (lowest mipmap):
	//
	Red  = 0; Green = 0; Blue = 0; 
	Texel = GetData();
	for (i=0; i<DataSize; i++)
		{
		B      = *Texel++;
		Red   += Colors[B].Red;
		Green += Colors[B].Green;
		Blue  += Colors[B].Blue;
		};
	MipZero.Red			= Red/i;
	MipZero.Green		= Green/i;
	MipZero.Blue		= Blue/i;
	MipZero.RemapIndex	= GGfx.DefaultPalette->BestMatch(MipZero,0);
	//
	if (!FullMips) return;
	if (MipOfs[1] != MAXDWORD) return; // Already have mips
	//
	DataSize = 0;
	for (MipLevel=7; MipLevel>=0; MipLevel--)
		{
		ThisSize = (USize >> MipLevel)*(VSize >> MipLevel);
		//
		if (ThisSize>0) MipOfs[MipLevel] = DataSize;
		else			MipOfs[MipLevel] = MAXDWORD;
		//
		DataSize += ThisSize;
		};
	Realloc(); Texel = GetData();
	//
	// Blur:
	//
	#if BLUR_TEXTURES_ON_IMPORT
		{
		FColor *C = &ThisPalette->Element(0);
		BYTE *Temp = (BYTE *)GMem.Get(USize*VSize);
		memcpy(Temp,&Texel[0],USize*VSize);
		BYTE *D1 = &Temp[(VSize-1)*USize];
		BYTE *D2 = &Temp[0];
		BYTE *D  = &Texel[0];
		for (int vv=0; vv<VSize; vv++)
			{
			for (int uu=0; uu<USize; uu++)
				{
				int uuu=(uu+1)&(USize-1);
				FColor T;
				//
				T.Red   = ((int)C[D1[uu]].Red   + C[D1[uuu]].Red   + C[D2[uu]].Red   + C[D2[uuu]].Red  )>>2;
				T.Green = ((int)C[D1[uu]].Green + C[D1[uuu]].Green + C[D2[uu]].Green + C[D2[uuu]].Green)>>2;
				T.Blue  = ((int)C[D1[uu]].Blue  + C[D1[uuu]].Blue  + C[D2[uu]].Blue  + C[D2[uuu]].Blue )>>2;
				//
				*D++ = ThisPalette->BestMatch(T,0);
				};
			D1  = D2;
			D2 += USize;
			};
		GMem.Release(Temp);
		};
	#endif
	//
	memmove (&Texel[DataSize - USize*VSize],&Texel[0],USize*VSize);
	mymemset(&Texel[0],3,DataSize - USize*VSize);
	//
	// Build each mip from the next-larger mip
	//
	ThisUSize		= USize;
	ThisVSize		= VSize;
	//
	for (MipLevel=1; (MipLevel<MAX_MIPS) && (MipOfs[MipLevel]!=MAXDWORD); MipLevel++)
		{
		SourceTex	= &Texel[MipOfs[MipLevel-1]];
		DestTex		= &Texel[MipOfs[MipLevel]];
		//
		HalfUSize	= ThisUSize>>1;
		HalfVSize	= ThisVSize>>1;
		//
		UAnd		= ThisUSize-1;
		VAnd		= ThisVSize-1;
		//
		if (!(PolyFlags & PF_Masked)) // Simple (non masked) mipmap
			{
			for (U=0; U<HalfUSize; U++) for (V=0; V<HalfVSize; V++) 
				{
				if (U&1) GApp->StatusUpdate ("Creating mipmap",U,HalfUSize);
				//
				Red=0; Green=0; Blue=0;
				for (X=0; X<4; X++) for (Y=0; Y<4; Y++)
					{
					SourceColor = &Colors[SourceTex[((V*2+Y-1)&VAnd)*ThisUSize + ((U*2+X-1)&UAnd)]];
					Red   += BoxC[X][Y]*(int)SourceColor->Red;
					Green += BoxC[X][Y]*(int)SourceColor->Green;
					Blue  += BoxC[X][Y]*(int)SourceColor->Blue;
					};
				ResultColor.Red   = Red   >> BOX_SHIFT;
				ResultColor.Green = Green >> BOX_SHIFT;
				ResultColor.Blue  = Blue  >> BOX_SHIFT;
				//
				DestTex[V*HalfUSize+U] = ThisPalette->BestMatch(ResultColor,0);
				};
			}
		else // Masked mipmap
			{
			for (U=0; U<HalfUSize; U++) for (V=0; V<HalfVSize; V++) 
				{
				if (U&1) GApp->StatusUpdate ("Creating mipmap",U,HalfUSize);
				//
				n = 0;
				Red=0; Green=0; Blue=0;
				for (X=0; X<4; X++) for (Y=0; Y<4; Y++)
					{
					B = SourceTex[((V*2+Y-1)&VAnd)*ThisUSize + ((U*2+X-1)&UAnd)];
					if (B)
						{
						n += BoxC[X][Y];
						SourceColor = &Colors[B];
						Red   += BoxC[X][Y]*(int)SourceColor->Red;
						Green += BoxC[X][Y]*(int)SourceColor->Green;
						Blue  += BoxC[X][Y]*(int)SourceColor->Blue;
						};
					};
				if ((n*2) >= BOX_SUM) // Mostly unmasked - keep it
					{
					ResultColor.Red   = Red   / n;
					ResultColor.Green = Green / n;
					ResultColor.Blue  = Blue  / n;
					B                 = ThisPalette->BestMatch(ResultColor,0);
					}
				else B = 0; // Mostly masked - remain masked.
				//
				DestTex[V*HalfUSize+U] = B;
				};
			};
		ThisUSize = ThisUSize >> 1;
		ThisVSize = ThisVSize >> 1;
		};
	UNGUARD("UTexture::CreateMips");
	};

/*---------------------------------------------------------------------------------------
	UPalette implementation
---------------------------------------------------------------------------------------*/

//
// Resource functions:
//
void UPalette::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UPalette);
	Type->RecordSize = sizeof (FColor);
	Type->Version    = 1;
	Type->TypeFlags  = RTF_ScriptReferencable;
	strcpy (Type->Descr,"Palette");
	UNGUARD("UPalette::Register");
	};
void UPalette::InitData(void)
	{
	GUARD;
	mymemset (Data,0,QuerySize());
	UNGUARD("UPalette::InitData");
	};
int UPalette::QuerySize(void)
	{
	GUARD;
	return NUM_PAL_COLORS * (sizeof (FColor) + sizeof (FPalettePrecalc));
	UNGUARD("UPalette::QuerySize");
	};
int UPalette::QueryMinSize(void)
	{
	GUARD;
	return QuerySize();
	UNGUARD("UPalette::QueryMinSize");
	};
const char *UPalette::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	FPCXFileHeader	*PCX		 = (FPCXFileHeader *)Buffer;
	BYTE			*PCXPalette  = (BYTE *)(BufferEnd - NUM_PAL_COLORS * 3);
	int				BufferLength = (int)(BufferEnd-Buffer);
	//
	// Validate stuff in header:
	//
	if (BufferLength < sizeof(FPCXFileHeader)) return NULL; // Doesn't contain valid header and palette
	if (PCX->Manufacturer != 10) return NULL; // Unknown format
	if ((PCX->BitsPerPixel!=8) || (PCX->NumPlanes!=1)) return NULL; // Bad format, must have 8 bits per pixel, 1 plane
	//
	Realloc();
	FColor *Colors = GetData();
	for (int i=0; i<NUM_PAL_COLORS; i++)
		{
		Colors[i].Red	= *PCXPalette++;
		Colors[i].Green	= *PCXPalette++;
		Colors[i].Blue	= *PCXPalette++;
		Colors[i].Flags	= 0;
		};
	BuildBrightnessTable();
	return BufferEnd;
	UNGUARD("UPalette::Import");
	};
AUTOREGISTER_RESOURCE(RES_Palette,UPalette,0xB2D90877,0xCCD211cf,0x91360000,0xC028B992);

//
// Find closest palette color matching a given RGB value:
//
BYTE UPalette::BestMatch (FColor Color,int SystemPalette)
	{
	GUARD;
	FColor		*Colors = GetData();
	FColor		*ColorPtr;
	int 		Delta,BestDelta,BestColor,TestColor;
	//
	BestDelta  		= MAXINT;
	BestColor  		= FIRSTCOLOR;
	//
	int First,Last;
	//
	if (SystemPalette)	{First=FIRSTCOLOR; Last=LASTCOLOR;}
	else				{First=1; Last=NUM_PAL_COLORS;};
	//   
	ColorPtr = &Colors [First];
	for (TestColor=First; TestColor<Last; TestColor++)
		{
		Delta =
			(
			OurSquare((int)ColorPtr->Red   - (int)Color.Red)   +
			OurSquare((int)ColorPtr->Green - (int)Color.Green) +
			OurSquare((int)ColorPtr->Blue  - (int)Color.Blue)
			);
		if (Delta < BestDelta)
			{
			BestColor = TestColor;
			BestDelta = Delta;
			};
		ColorPtr++;
		};
	return BestColor;
	UNGUARD("UPalette::BestMatch");
	};

//
// Find closest palette color matching a given RGB value that
// lies within a particular one of the 8 palette ranges:
//
BYTE UPalette::BestMatchInRange(FColor MatchColor, BYTE RangeColor)
	{
	GUARD;
	FColor		*Colors = GetData();
	FColor		*ColorPtr;
	int 		Delta,BestDelta,BestColor,TestColor;
	//
	RangeColor		= RangeColor & 7;
	ColorPtr		= &Colors[FIRSTCOLOR];
	BestDelta  		= MAXSWORD;
	BestColor  		= FIRSTCOLOR;
	//
	for (TestColor=FIRSTCOLOR; TestColor<LASTCOLOR; TestColor++)
		{
		if (RangeColor == (TestColor&7))
			{
			Delta =
				OurSquare((int)ColorPtr->Red   - (int)MatchColor.Red)   +
				OurSquare((int)ColorPtr->Green - (int)MatchColor.Green) +
				OurSquare((int)ColorPtr->Blue  - (int)MatchColor.Blue);
			if (Delta < BestDelta)
				{
				BestColor = TestColor;
				BestDelta = Delta;
				};
			};
		ColorPtr++;
		};
	return BestColor;
	UNGUARD("UPalette::BestMatchInRange");
	};

void UPalette::Smooth (void)
	{
	GUARD;
	FColor *Colors = GetData();
	FColor *C1=&Colors[0], *C2=&Colors[1];
	//
	for (int i=1; i<256; i++)
		{
		C2->Red   = ((int)C1->Red   + (int)C2->Red  )>>1;
		C2->Green = ((int)C1->Green + (int)C2->Green)>>1;
		C2->Blue  = ((int)C1->Blue  + (int)C2->Blue )>>1;
		//
		C1++;
		C2++;
		};
	UNGUARD("UPalette::Smooth");
	};

void UPalette::BuildBrightnessTable (void)
	{
	GUARD;
	FColor			*Colors		= GetData();
	FPalettePrecalc	*Precalc	= (FPalettePrecalc *)&Colors[256];
	//
	for (int i=0; i<256; i++)
		{
		if (!(i&7)) GApp->StatusUpdate("Precalculating",i,256);
		//
		int ThisBrightness    = Colors[i].Brightness();
		Precalc[i].Brightness = ThisBrightness;
		Precalc[i].RemapIndex = 0;
		//
		if (GGfx.DefaultColors) // Skip when importing first palette
			{
			//
			// Precalculate nearest colors to aid fast diffusion ditherer:
			//
			BYTE Available[256];
			mymemset (Available,1,256);
			for (int j=0; j<FPalettePrecalc::NUM_NEAREST; j++)
				{
				int BestColor = 0;
				int BestDelta = 768;
				for (int k=FIRSTCOLOR; k<LASTCOLOR; k++)
					{
					int Delta = OurAbs(GGfx.DefaultColors[k].Brightness() - ThisBrightness);
					//int Delta = 
					//	OurAbs((int)GGfx.DefaultColors[k].Red   - (int)Element(i).Red)   +
					//	OurAbs((int)GGfx.DefaultColors[k].Green - (int)Element(i).Green) +
					//	OurAbs((int)GGfx.DefaultColors[k].Blue  - (int)Element(i).Blue);
					if ((Delta<BestDelta) && Available[k])
						{
						BestColor = k;
						BestDelta = Delta;
						};
					};
				Available [BestColor]	= 0;
				Precalc[i].Nearest[j]	= BestColor;
				};
			};
		};
	UNGUARD("UPalette::BuildBrightnessTable");
	};

//
// Build the palette remap index
//
void UPalette::BuildPaletteRemapIndex(int Masked)
	{
	GUARD;
	//
	FColor *Colors = GetData();
	//
	if (Masked || !GGfx.DefaultColors)	Colors[0].RemapIndex = 0;
	else								Colors[0].RemapIndex = GGfx.DefaultPalette->BestMatch(Colors[0]);
	//
	if (GGfx.DefaultColors) for (int i=1; i<NUM_PAL_COLORS; i++)
		{
		Colors[i].RemapIndex = GGfx.DefaultPalette->BestMatch(Colors[i]);
		}
	else for (int i=0; i<NUM_PAL_COLORS; i++)
		{
		Colors[i].RemapIndex = i;
		};
	UNGUARD("UPalette::BuildPaletteRemapIndex");
	};

//
// Sees if this palette is a duplicate of an existing palette.
// If it is, deletes this palette and returns the existing one.
// If not, returns this palette.
//
UPalette *UPalette::ReplaceWithExisting(void)
	{
	GUARD;
	//
	UPalette *TestPalette;
	FOR_ALL_TYPED_RES(TestPalette,RES_Palette,UPalette)
		{
		if (TestPalette!=this)
			{
			FColor *C1 = &Element(0);
			FColor *C2 = &TestPalette->Element(0);
			for (int i=0; i<NUM_PAL_COLORS; i++)
				{
				if ((C1->Red!=C2->Red)||(C1->Green!=C2->Green)||(C1->Blue!=C2->Blue)) goto Next;
				C1++; 
				C2++;
				};
			debugf(LOG_Ed,"Replaced palette %s with %s",Name,TestPalette->Name);
			Kill();
			return TestPalette;
			};
		Next:;
		}
	END_FOR_ALL_RES;
	//
	return this;
	//
	UNGUARD("UPalette::ReplaceWithExisting");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
