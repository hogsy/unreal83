/*=============================================================================
	UnMem.cpp: Unreal memory grabbing functions

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-----------------------------------------------------------------------------
	FMemPool implementation (not including inlines)
-----------------------------------------------------------------------------*/

//
// Allocate this memory pool's buffer.
//
void FMemPool::AllocatePool(int NewSize, const char *Name)
	{
	GUARD;
	char Descr[80]; sprintf(Descr,"MemPool(%s)",Descr);
	Start = appMallocArray(NewSize,BYTE,Descr);
	Size  = NewSize;
	InitPool();
	UNGUARD("FMemPool::AllocatePool");
	};

//
// Initialize this memory pool to its empty state.
//
void FMemPool::InitPool(void)
	{
	GUARD;
	Top = Start;
	End = Start + Size;
	UNGUARD("FMemPool::InitPool");
	};

//
// Free this memory pool's buffer.
//
void FMemPool::FreePool(void)
	{
	GUARD;
	appFree(Start);
	Start = NULL;
	Top   = NULL;
	End   = NULL;
	Size  = 0;
	UNGUARD("FMemPool::FreePool");
	};

/*-----------------------------------------------------------------------------
	Memory pool errors
-----------------------------------------------------------------------------*/

void FMemPool::OverflowError(void)
	{
	appError("FMemPool::OverflowError");
	};

void FMemPool::AccessError(void)
	{
	appError("FMemPool::AccessError");
	};

void FMemPool::SizeError(void)
	{
	appError("FMemPool::SizeError");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
