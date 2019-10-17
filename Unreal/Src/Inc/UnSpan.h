/*=============================================================================
	UnSpan.h: Span buffering functions and structures

	Copyright 1995 Epic MegaGames, Inc.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNSPAN // Prevent header from being included multiple times
#define _INC_UNSPAN

/*------------------------------------------------------------------------------------
	General span buffer related classes
------------------------------------------------------------------------------------*/

//
// Screen extents of an axis-aligned bounding box.
//
class FScreenBounds
	{
	public:
	int		Valid;
	int		MinX,MinY;
	int		MaxX,MaxY;
	FLOAT	MinZ,MaxZ;
	//
	void DebugDump(void);
	};

//
// A span buffer linked-list entry representing a free (undrawn) 
// portion of a scanline. **WARNING** Mirrored in UnRender.inc.
//
class FSpan
	{
	public:
	int		Start;			// Starting X value
	int		End;			// Ending X value
	FSpan	*Next;			// NULL = no more
	};

//
// A span buffer, which represents all free (undrawn) scanlines on
// the screen.
//
class FSpanBuffer
	{
	public:
	int		StartY;			// Starting Y value
	int		EndY;			// Last Y value + 1
	int		ValidLines;		// Number of lines at beginning (for screen)
	int		Churn;			// Number of lines added since start
	FSpan	**Index;		// Contains (EndY-StartY) units pointing to first span or NULL.
	FSpan	*List;			// Entries
	FMemPool *Mem;			// Memory pool everything is stored in
	//
	// Allocation:
	//
	void AllocIndex				(int AllocStartY, int AllocEndY, FMemPool *Mem);
	void AllocAndInitIndex		(int AllocStartY, int AllocEndY, FMemPool *Mem);
	void AllocIndexForScreen	(int SXR, int SYR, FMemPool *Mem);
	void Release				(void);
	void GetValidRange			(SWORD *ValidStartY,SWORD *ValidEndY);
	//
	// Merge/copy/alter operations:
	//
	void CopyIndexFrom			(const FSpanBuffer &Source,								  FMemPool *Mem);
	void MergeFrom				(const FSpanBuffer &Source1, const FSpanBuffer &Source2,  FMemPool *Mem);
	void MergeWith				(const FSpanBuffer &Other);
	//
	// Grabbing and updating from rasterizations:
	//
	int  CopyFromRange			(FSpanBuffer &ScreenSpanBuffer,int Y1, int Y2, FMemPool *Mem);
	int  CopyFromRaster			(FSpanBuffer &ScreenSpanBuffer,class FRasterPoly &Raster);
	int  CopyFromRasterUpdate	(FSpanBuffer &ScreenSpanBuffer,class FRasterPoly &Raster);
	//
	// Rectangle/lattice operations:
	//
	int  CalcRectFrom			(const FSpanBuffer &Source,BYTE GridXBits,BYTE GridYBits, FMemPool *Mem);
	void CalcLatticeFrom		(const FSpanBuffer &Source,								  FMemPool *Mem);
	//
	// Occlusion:
	//
	int	 BoxIsVisible           (int X1, int Y1, int X2, int Y2);
	int	 BoundIsVisible			(FScreenBounds &Bound);
	//
	// Debugging:
	//
	void AssertEmpty			(char *Name); // Assert it contains no active spans
	void AssertNotEmpty			(char *Name); // Assert it contains active spans
	void AssertValid			(char *Name); // Assert its data is ok
	void DebugDraw				(ICamera *Camera,BYTE Color);
	};

/*------------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------------*/
#endif // _INC_UNSPAN

