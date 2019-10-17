/*=============================================================================
	UnGfx.h: Graphics functions

	Copyright 1995 Epic MegaGames, Inc.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNGFX // Prevent header from being included multiple times
#define _INC_UNGFX

/*------------------------------------------------------------------------------------
	Color information
------------------------------------------------------------------------------------*/

//
// Editor colors, set by FGlobalGraphics::Init.
//
enum {MAX_COLORS=64};
#define WorldBoxColor			GGfx.Colors[0 ]
#define GroundPlaneColor		GGfx.Colors[1 ]
#define GroundPlaneHighlight	GGfx.Colors[2 ]
#define NormalColor				GGfx.Colors[3 ]
#define BrushFillColor			GGfx.Colors[4 ]
#define BrushWireColor			GGfx.Colors[5 ]
#define BspWireColor			GGfx.Colors[6 ]
#define BspFillColor			GGfx.Colors[7 ]
#define PivotColor				GGfx.Colors[8 ]
#define SelectColor				GGfx.Colors[9 ]
#define CurrentColor			GGfx.Colors[10]
#define AddWireColor			GGfx.Colors[11]
#define SubtractWireColor		GGfx.Colors[12]
#define GreyWireColor			GGfx.Colors[15]
#define BrushVertexColor		GGfx.Colors[16]
#define BrushSnapColor			GGfx.Colors[17]
#define InvalidColor			GGfx.Colors[18]
#define ActorDotColor			GGfx.Colors[19]
#define ActorWireColor			GGfx.Colors[20]
#define ActorHiWireColor		GGfx.Colors[21]
#define ActorFillColor			GGfx.Colors[22]
#define BlackColor				GGfx.Colors[23]
#define WhiteColor				GGfx.Colors[24]
#define MaskColor				GGfx.Colors[25]
#define SelectBorderColor		GGfx.Colors[27]
#define SemiSolidWireColor		GGfx.Colors[28]
#define NonSolidWireColor       GGfx.Colors[30]
#define NormalFontColor         GGfx.Colors[31]
#define WireBackground			GGfx.Colors[32]
#define WireGridAxis			GGfx.Colors[35]
#define ActorArrowColor			GGfx.Colors[36]
#define ScaleBoxColor			GGfx.Colors[37]
#define ScaleBoxHiColor			GGfx.Colors[38]
#define ZoneWireColor			GGfx.Colors[39]
#define MoverColor				GGfx.Colors[40]

//
// Standard colors
//
enum EStandardColors
	{
	P_GREY		= 0,
	P_BROWN		= 1,
	P_FLESH		= 2,
	P_WOOD		= 3,
	P_GREEN		= 4,
	P_RED		= 5,
	P_BLUE		= 6,
	P_FIRE		= 7,
	};

#define BRIGHTNESS(b) (BYTE)((b)<<3)
#define COLOR(c,b)    (BYTE)((c)+((b)<<3)+32) /* c=color 0-7, b=brightness 0-27 */

#define FIRSTCOLOR 16
#define LASTCOLOR  240

/*------------------------------------------------------------------------------------
	FGlobalGfx
------------------------------------------------------------------------------------*/

enum EBlendTable
	{
	BLEND_None			= 0, // No blending
	BLEND_Transparent	= 1, // 50% transparent
	BLEND_Ghost			= 2, // 30% transparent
	BLEND_Glow			= 3, // Adds colors together and prevents overflow
	BLEND_Average		= 4, // Average
	BLEND_MAX			= 5
	};

//
// Graphics globals:
//
class UNREAL_API FGlobalGfx
	{
	public:
	//
	// Parameters:
	//
	DWORD	DefaultCameraFlags;	// Camera-show flags when opening a new camera
	DWORD	DefaultRendMap;		// Camera map rendering flags when opening a new camera
	//
	// Standard colors:
	//
	BYTE Colors[MAX_COLORS];
	//
	// Resources:
	//
	UArray		*Graphics;			// Graphics array resource
	UArray		*Tables;			// Stored/regenerated tables of textures & things
	UArray		*Luts;				// Lookup tables
	UTexture	*DefaultTexture;	// Default texture for untextured polygons
	UTexture	*BkgndTexture;		// Background texture for viewers
	UTexture	*BackdropTexture;	// World backdrop
	UTexture	*BadTexture;		// Invalid texture picture
	UTexture	*Logo;				// Game logo
	UPalette	*DefaultPalette;	// Palette to use at startup
	UPalette    *TrueColorPalette;	// True color-reversed palette
	UPalette	*GammaPalette;		// Gamma-corrected version of current palette
	UModel		*ArrowBrush;		// Brush that shows actor directions
	UModel		*RootHullBrush;		// Brush that encloses the world
	//
	UBuffer		*ShadeTable;		// 64K shade table
	UBuffer		*BlendTable;		// 64K blend table
	UBuffer		*GhostTable;		// 64K ghost table
	UBuffer		*GlowTable;			// 64K glow table
	UBuffer		*HueTable;			// 24-bit color 256-entry hue table
	UBuffer		*SincTable;			// Pseudo 2D sinc interpolation table
	//
	UFont		*HugeFont;
	UFont		*LargeFont;
	UFont		*MedFont;
	UFont		*SmallFont;
	//
	UTexture	*MenuUp,*MenuDn;
	UTexture	*CollOn,*CollOff;
	UTexture	*PlyrOn,*PlyrOff;
	UTexture	*LiteOn,*LiteOff;
	//
	// Gamma correction parameters:
	//
	WORD	NumGammaLevels;
	WORD	GammaLevel;
	//
	// Data:
	//
	BYTE	*ShadeData;				// 64K shade table
	BYTE	*Blenders[BLEND_MAX];	// 64K tables for blending two colors
	FLOAT	*SincData;				// 2*4*256 byte pseudo 2D sinc interpolation table
	FVector	*HueData;				// 24-bit color hue table (floating point XYZ=RGB)
	INT		Squares[512];			// Square[-256..255]
	//
	FColor *DefaultColors;
	FColor *GammaColors;
	FColor *TrueColors;
	//
	// Effects/Postprocessing
	//
	int Smooth;
	int Stretch;
	//
	// Functions:
	//
	public:
	void Init(void);
	void Exit(void);
	void Cout(UTexture *DestTexture,int X, int Y, int XSpace, UFont *Font, int Color, char C,void *Palette);
	//
	void BurnRect(UTexture *Texture,int X1,int X2,int Y1,int Y2,int Bright);
	void Clearscreen(ICamera *Camera,BYTE DefaultPalColor);
	void GammaCorrectPalette (UPalette *DestPalette, UPalette *SourcePalette);
	void SetPalette(void);
	void FixPalette(UPalette *Palette);
	UFont *MakeFontFromTexture (UTexture *Texture);
	//
	void StrLen(int *XL, int *YL, int XSpace, int YSpace, UFont *Font, const char *Text);
	void WrappedStrLen(int *XL, int *YL, int XSpace, int YSpace, UFont *Font, int Width, const char *Text);
	void VARARGS Printf(UTexture *DestTexture,int X, int Y, int XSpace, UFont *Font, int Color, const char *Fmt,...);
	void VARARGS WrappedPrintf(UTexture *DestTexture,int X, int Y, int XSpace, int YSpace, UFont *Font, int Color, int Width, int Center, const char *Fmt,...);
	//
	void PreRender(ICamera *Camera);
	void PostRender(ICamera *Camera);
	//
	void* GetPaletteTable(UTexture *CameraTexture,UPalette *Palette);
	//
	void RGBtoHSV(FVector &Result,BYTE H,BYTE S,BYTE V,int CameraColorBytes);
	//
	private:
	void LookupAllTables(void);
	void LookupAllLuts(void);
	int FGlobalGfx::ScanFontBox (UTexture *Texture,int X,int Y,int *XL,int *YL);
	};

/*------------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------------*/
#endif // _INC_UNGFX
