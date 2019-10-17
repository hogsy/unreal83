/*=============================================================================
	UnRender.cpp: Main Unreal rendering functions and pipe

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney

	Rearrange:
		* Merge Bsp Poly and lighting info
		* Separate Bsp Node and bound info
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Testing
-----------------------------------------------------------------------------*/

int a,b;

void test(void)
{
	try
	{
		a++;
	}
	catch(...)
	{	
		b++;
	};
};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
