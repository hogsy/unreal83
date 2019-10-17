/*=============================================================================
	UnMem.h: FMemoryPool class, ultra-fast temporary memory allocation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNMEM
#define _INC_UNMEM

/*-----------------------------------------------------------------------------
	FMemPool
-----------------------------------------------------------------------------*/

class UNREAL_API FMemPool // Simple linear-allocation memory pool
	{
	public:
	BYTE		*Start;			// Start of memory area
	BYTE		*End;			// End of memory data
	BYTE		*Top;			// Top of memory pool
	int			Size;
	//
	// Operational functions:
	//
	inline void *Get(int AllocSize) // Get 8-aligned memory and increase pool top
		{
		void *Result = (void *)(((int)Top+7)&~7);
		if (AllocSize<0) SizeError();
		Top = (BYTE *)Result + AllocSize;
		if (Top >= (Start+Size)) OverflowError();
		return Result;
		};
	inline void *GetFast(int AllocSize) // Get 8-aligned with absolutely no error checking
		{
		void *Result = (void *)(((int)Top+7)&~7);
		Top = (BYTE *)Result + AllocSize;
		return Result;
		};
	inline void *GetFast4(int AllocSize) // Get 4-aligned memory quickly
		{
		void *Result = (void *)(((int)Top+3)&~3);
		Top = (BYTE *)Result + AllocSize;
		return Result;
		};
	inline void Release(void *Mem) // Release all memory at and above Mem pointer
		{
		if ((Mem<Start)||(Mem>=End)) AccessError();
		Top = (BYTE *)Mem;
		};
	inline void *GetZeroed(int AllocSize)
		{
		void *Result = Get (AllocSize);
		mymemset(Result,0,AllocSize);
		return Result;
		};
	inline void *GetOned(int AllocSize)
		{
		void *Result = Get (AllocSize);
		mymemset(Result,(BYTE)255,AllocSize);
		return Result;
		};
	//
	// Startup/shutdown functions:
	//
	void AllocatePool(int Size, const char *Name); // Allocate the memory pool
	void InitPool(void); // Initialize or reinitialize the memory pool to empty
	void FreePool(void);
	void OverflowError(void);
	void AccessError(void);
	void SizeError(void);
	//
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNMEM

