/*=============================================================================
	UnEditor.h: Main classes used by the Unreal editor

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNEDITOR
#define _INC_UNEDITOR
#ifdef  EDITOR /* Only include if editor subsystem is to be compiled */

#ifdef COMPILING_EDITOR
	UNEDITOR_API extern class FEditor GUnrealEditor;
#endif

/*-----------------------------------------------------------------------------
	FScan
-----------------------------------------------------------------------------*/

//
// Scanner types, for mouse click hit testing.
//
enum EEdScan				// Things editor camera scanner can look for
	{
	EDSCAN_None,			// Nothing found yet
	EDSCAN_BspNodePoly, 	// Index = iNode, A = unused
	EDSCAN_BspNodeSide,     // Index = iNode, A = side index
	EDSCAN_BspNodeVertex, 	// Index = iNode, A = vertex index
	EDSCAN_BrushPoly,		// Index = Brush ID, A = poly index
	EDSCAN_BrushSide,       // Index = Brush ID, A = poly index, B = side index
	EDSCAN_BrushVertex,     // Index = Brush ID, A = poly index, B = vertex index
	EDSCAN_Actor,			// Index = actor index
	EDSCAN_UIElement,		// Misc user interface elements, Index=element number
	EDSCAN_BrowserTex,		// Texture in texture browser
	EDSCAN_BrowserMesh,		// Mesh in mesh browser
	};

//
// Scanner parameters.
//
enum {EDSCAN_RADIUS=1  }; /* Search center plus this many pixels on all sides */
enum {EDSCAN_IGNORE=254}; /* Ignores color 254 while scanning */

//
// Scanner class, for mouse click hit testing.  Init() is called before
// a frame is rendered.  Before each object is drawn, PreScan() is called
// to remember the state of the screen under the mouse pointer.  After
// the object is drawn, PostScan() is called to see if the area under the
// mouse was changed.  By the time the frame is finished rendering from front
// to back, FScan knows what the user clicked on, if anything.
//
class UNEDITOR_API FScan // Editor camera scanner results
	{
	public:
	ICamera	*Camera;	// Remembered camera
	int		Active;  	// 1 = actively scanning, 0=we are not scanning
	int		X;			// Scan X location in screenspace (mouse click location)
	int		Y;			// Scan Y location in screenspace
	int		Type;		// Type of most recent hit, or SCAN_NONE if nothing found yet
	int  	Index;		// Index of most recent hit
	int  	A;			// PostScan A value of most recent hit
	int  	B;			// PostScan B value of most recent hit
	FVector	V;			// Any floating point vector that you want
	DWORD	Pixels [1 + 2 * EDSCAN_RADIUS][1 + 2 * EDSCAN_RADIUS];  // PreScan pixel values
	//
	// Functions:
	//
	virtual void Init		(ICamera *NewCamera);
	virtual void Exit		(void);
	virtual void PreScan  	(void);
	virtual void PostScan 	(EEdScan ScanType, int Index, int A, int B, FVector *V);
	};

/*-----------------------------------------------------------------------------
	FConstraints
-----------------------------------------------------------------------------*/

//
// General purpose movement/rotation constraints.
//
class UNEDITOR_API FConstraints
	{
	public:
	//
	// Toggles:
	//
	WORD		GridEnabled;		// Grid on/off
	WORD		RotGridEnabled;		// Rotation grid on/off
	WORD		Flags;				// Movement constraint bit flags
	WORD		SnapVertex;			// Snap to nearest vertex within SnapDist, if any
	//
	FVector		Grid;				// Movement grid
	FVector		GridBase;			// Base (origin) of movement grid
	FRotation	RotGrid;			// Rotation grid
	//
	FLOAT		SnapDist;			// Distance to check for snapping
	};

/*-----------------------------------------------------------------------------
	Enums
-----------------------------------------------------------------------------*/

//
// Quality level for rebuilding Bsp.
//
enum EBspOptimization
	{
	BSP_Lame,
	BSP_Good,
	BSP_Optimal
	};

//
// Editor mode settings.
//
// These are also referenced by help files and by the editor client, so
// they shouldn't be changed.
//
enum EEditorMode
	{
	EM_None 			= 0,	// Gameplay, editor disabled
	EM_CameraMove		= 1,	// Move camera normally
	EM_CameraZoom		= 2,	// Move camera with acceleration
	EM_BrushFree		= 3,	// Move brush free-form
	EM_BrushMove		= 4,	// Move brush along one axis at a time
	EM_BrushRotate		= 5,	// Rotate brush
	EM_BrushSheer		= 6,	// Sheer brush
	EM_BrushScale		= 7,	// Scale brush
	EM_BrushStretch		= 8,	// Stretch brush
	EM_AddActor			= 9,	// Add actor/light
	EM_MoveActor		= 10,	// Move actor/light
	EM_TexturePan		= 11,	// Pan textures
	EM_TextureSet		= 12,	// Set textures
	EM_TextureRotate	= 13,	// Rotate textures
	EM_TextureScale		= 14,	// Scale textures
	EM_BrushWarp		= 16,	// Warp brush verts
	EM_Terraform		= 17,	// Terrain edit
	EM_BrushSnap		= 18,	// Brush snap-scale
	EM_TexView			= 19,	// Viewing textures
	EM_TexBrowser		= 20,	// Browsing textures
	EM_MeshView			= 21,	// Viewing mesh
	EM_MeshBrowser		= 22,	// Browsing mesh
	};

//
// Editor mode classes.
//
// These are also referenced by help files and by the editor client, so
// they shouldn't be changed.
//
enum EEditorModeClass
	{
	EMC_None		= 0,	// Editor disabled
	EMC_Camera		= 1,	// Moving the camera
	EMC_Brush		= 2,	// Affecting the brush
	EMC_Actor		= 3,	// Affecting actors
	EMC_Texture		= 4,	// Affecting textures
	EMC_Player		= 5,	// Player movement
	EMC_Terrain		= 6,	// Terrain editing
	};

//
// Bsp poly alignment types for polyTexAlign
//
enum ETexAlign						
	{
	TEXALIGN_Default		= 0,	// No special alignment (just derive from UV vectors)
	TEXALIGN_Floor			= 1,	// Regular floor (U,V not necessarily axis-aligned)
	TEXALIGN_WallDir		= 2,	// Grade (approximate floor), U,V X-Y axis aligned
	TEXALIGN_WallPan		= 3,	// Align as wall (V vertical, U horizontal)
	TEXALIGN_OneTile		= 4,	// Align one tile
	TEXALIGN_WallColumn		= 5,	// Align as wall on column
	};

//
// Things to set in mapSetBrush
//
enum EMapSetBrushFlags				
	{
	MSB_BrushColor	= 1,			// Set brush color
	MSB_Group		= 2,			// Set group
	MSB_PolyFlags	= 4,			// Set poly flags
	};

//
// Possible positions of a child Bsp node relative to its parent (for BspAddToNode)
//
enum ENodePlace 
	{
	NODE_Root		= 0, // Node is the Bsp root and has no parent -> Bsp[0]
	NODE_Front		= 1, // Node is in front of parent             -> Bsp[iParent].iFront
	NODE_Back		= 2, // Node is in back of parent              -> Bsp[iParent].iBack
	NODE_Plane		= 3, // Node is coplanar with parent           -> Bsp[iParent].iPlane
	};

/*-----------------------------------------------------------------------------
	FEditor definition
-----------------------------------------------------------------------------*/

typedef void (*POLY_CALLBACK)(IModel *ModelInfo, INDEX iSurf);

//
// The global Unreal editor
//
class UNEDITOR_API FEditor
	{
	public:
	//
	enum {MACRO_TEXT_REC_SIZE=80000}; // Default macro record-buffer size
	//
	// Resources:
	//
	UArray			*EditorArray;
	UModel			*TempModel;
	UTexture		*CurrentTexture;
	//
	// Toggles:
	//
	int 			Mode;
	int 			ShowVertices;
	int 			MapEdit;
	int 			Show2DGrid;
	int			    Show3DGrid;
	//
	FLOAT			MovementSpeed;
	FConstraints	Constraints;
	UTextBuffer		*MacroRecBuffer;
	//
	// Editor camera scanner:
	//
	FScan			Scan;
	//
	// Functions:
	//
	virtual void	Init(void);
	virtual void	Exit(void);
	virtual int		Exec(const char *Cmd,FOutputDevice *Out=GApp);
	virtual void	NoteMacroCommand(const char *Cmd);
	//
	// Editor mode virtuals from UnEdCam.cpp:
	//
	virtual void	edcamSetMode			(int Mode);
	virtual int		edcamMode				(UCamera *Camera);
	virtual int		edcamModeClass			(int Mode);
	//
	// Editor CSG virtuals from UnEdCsg.cpp:
	//
	virtual UModel	*csgDuplicateBrush		(ULevel *Level,UModel *Brush, DWORD PolyFlags, BYTE ModelFlags);
	virtual UModel	*csgAddOperation		(UModel *Brush,ULevel *Level, DWORD PolyFlags, ECsgOper CSG, BYTE BrushFlags);
	virtual void	csgRebuild		 		(ULevel *Level);
	virtual void	csgInvalidateBsp		(ULevel *Level);
	virtual const char	*csgGetName 			(ECsgOper CsgOper);
	//
	// Editor EdPoly/BspSurf assocation virtuals from UnEdCsg.cpp:
	//
	virtual FPoly	*polyFindMaster			(IModel *ModelInfo, INDEX iSurf);
	virtual void    polyUpdateMaster		(IModel *ModelInfo, INDEX iSurf,int UpdateTexCoords,int UpdateBase);
	//
	// Bsp Poly search virtuals from UnEdCsg.cpp:
	//
	virtual void	polyFindByFlags 		(IModel *ModelInfo,DWORD SetBits, DWORD ClearBits, POLY_CALLBACK Callback);
	virtual void	polyFindByBrush 		(IModel *ModelInfo,UModel *Brush, INDEX BrushPoly, POLY_CALLBACK Callback);
	virtual void	polyFindByBrushGroupItem(IModel *ModelInfo,UModel *Brush, INDEX BrushPoly,FName Group, FName Item,POLY_CALLBACK Callback);
	virtual void	polySetAndClearPolyFlags(IModel *ModelInfo,DWORD SetBits, DWORD ClearBits,int SelectedOnly,int UpdateMaster);
	virtual void	polySetAndClearNodeFlags(IModel *ModelInfo,DWORD SetBits, DWORD ClearBits);
	virtual void	polySetItemNames		(IModel *ModelInfo,FName Item);
	//
	// Bsp Poly selection virtuals from UnEdCsg.cpp:
	//
	virtual void	polyResetSelection 		(IModel *ModelInfo);
	virtual void	polySelectAll 			(IModel *ModelInfo);
	virtual void	polySelectNone 			(IModel *ModelInfo);
	virtual void	polySelectMatchingGroups(IModel *ModelInfo);
	virtual void	polySelectMatchingItems	(IModel *ModelInfo);
	virtual void	polySelectCoplanars		(IModel *ModelInfo);
	virtual void	polySelectAdjacents		(IModel *ModelInfo);
	virtual void	polySelectAdjacentWalls	(IModel *ModelInfo);
	virtual void	polySelectAdjacentFloors(IModel *ModelInfo);
	virtual void	polySelectAdjacentSlants(IModel *ModelInfo);
	virtual void	polySelectMatchingBrush	(IModel *ModelInfo);
	virtual void	polySelectMatchingTexture(IModel *ModelInfo);
	virtual void	polySelectReverse 		(IModel *ModelInfo);
	virtual void	polyMemorizeSet 		(IModel *ModelInfo);
	virtual void	polyRememberSet 		(IModel *ModelInfo);
	virtual void	polyXorSet 				(IModel *ModelInfo);
	virtual void	polyUnionSet			(IModel *ModelInfo);
	virtual void	polyIntersectSet		(IModel *ModelInfo);
	//
	// Poly texturing virtuals from UnEdCsg.cpp:
	//
	virtual void	polyTexPan 				(IModel *ModelInfo,int PanU,int PanV,int Absolute);
	virtual void	polyTexScale			(IModel *ModelInfo,FLOAT UU,FLOAT UV, FLOAT VU, FLOAT VV,int Absolute);
	virtual void	polyTexAlign			(IModel *ModelInfo,ETexAlign TexAlignType,DWORD Texels);
	//
	// Map brush selection virtuals from UnEdCsg.cpp:
	//
	virtual void	mapSelectAll			(ULevel *Level);
	virtual void	mapSelectNone			(ULevel *Level);
	virtual void	mapSelectOperation		(ULevel *Level,ECsgOper CSGOper);
	virtual void	mapSelectFlags			(ULevel *Level,DWORD Flags);
	virtual void	mapSelectPrevious		(ULevel *Level);
	virtual void	mapSelectNext			(ULevel *Level);
	virtual void	mapSelectFirst 			(ULevel *Level);
	virtual void	mapSelectLast 			(ULevel *Level);
	virtual void	mapBrushGet				(ULevel *Level);
	virtual void	mapBrushPut				(ULevel *Level);
	virtual void	mapDelete				(ULevel *Level);
	virtual void	mapDuplicate			(ULevel *Level);
	virtual void	mapSendToFirst			(ULevel *Level);
	virtual void	mapSendToLast			(ULevel *Level);
	virtual void	mapSetBrush				(ULevel *Level,EMapSetBrushFlags PropertiesMask,WORD BrushColor,
		FName Group,DWORD SetPolyFlags,DWORD ClearPolyFlags);
	//
	// Editor actor virtuals from UnEdAct.cpp:
	//
	virtual void	edactMoveSelected 		(ILevel *Level, FVector *Delta, FFloatRotation *DeltaRot);
	virtual void	edactSelectAll 			(ILevel *Level);
	virtual void	edactSelectNone 		(ILevel *Level);
	virtual void	edactSelectOfClass		(ILevel *Level,UClass *Class);
	virtual void	edactDeleteSelected 	(ILevel *Level);
	virtual void	edactDuplicateSelected 	(ILevel *Level);
	virtual void	edactResetSelected		(ILevel *Level);
	virtual void	edactDeleteDependentsOf	(ILevel *Level,UClass *Class);
	//
	// Bsp virtuals from UnBsp.cpp:
	//
	virtual INDEX	bspAddVector		(IModel *ModelInfo, FVector *V, int Exact);
	virtual INDEX	bspAddPoint			(IModel *ModelInfo, FVector *V, int Exact);
	virtual int		bspNodeToFPoly		(IModel *ModelInfo, INDEX iNode, FPoly *EdPoly);
	virtual void	bspBuild			(UModel *Model, EBspOptimization Opt, int Balance, int RebuildSimplePolys);
	virtual void	bspRefresh			(IModel *ModelInfo,int ForceRefresh);
	virtual void	bspCleanup 			(IModel *ModelInfo);
	virtual void	bspBuildBounds		(IModel *ModelInfo);
	virtual void	bspBuildFPolys		(UModel *Model,int iSurfLinks);
	virtual void	bspMergeCoplanars	(UModel *Model,int RemapLinks);
	virtual int		bspBrushCSG 		(UModel *Brush, UModel *Model, DWORD PolyFlags, ECsgOper CSGOper,int RebuildBounds);
	virtual void	bspOptGeom			(UModel *Model);
	virtual void	bspValidateBrush	(UModel *Brush,int ForceValidate,int DoStatusUpdate);
	virtual void	bspBuildUniquePlanes(UModel *Model);
	virtual INDEX	bspAddNode			(IModel *ModelInfo, INDEX iParent, ENodePlace ENodePlace, DWORD NodeFlags, FPoly *EdPoly);
	//
	// Zone virtuals from UnZone.cpp:
	//
	virtual void	visBuild(ULevel *Level);
	//
	// Shadow virtuals from UnShadow.cpp:
	//
	virtual void	shadowIlluminateBsp (ULevel *Level, int Selected);
	//
	// Constraints (UnEdCnst.cpp):
	//
	virtual void	constraintInit				(FConstraints *Constraints);
	virtual int		constraintApply 			(IModel *LevelModelInfo, IModel *BrushInfo, FVector *Location, FRotation *Rotation,FConstraints *Constraints);
	virtual void	constraintFinishSnap 		(ILevel *LevelInfo,UModel *Brush);
	virtual void	constraintFinishAllSnaps	(ULevel *Level);
	//
	// Camera functions (UnEdCam.cpp):
	//
	virtual void	edcamDraw		(UCamera *Camera, int Scan);
	virtual void	edcamMove		(UCamera *Camera, BYTE Buttons, SWORD MouseX, SWORD MouseY, int Shift, int Ctrl);
	virtual int		edcamKey		(UCamera *Camera, int Key);
	virtual void	edcamClick		(UCamera *Camera, BYTE Buttons, SWORD MouseX, SWORD MouseY,int Shift, int Ctrl);
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // EDITOR
#endif // _INC_UNEDITOR
