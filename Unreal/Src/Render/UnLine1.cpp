/*=============================================================================
	UnLine1.cpp: INCLUDABLE C FILE for drawing lines.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	This is used to avoid the overhead of establishing a stack frame and calling
	a routine for every scanline to be drawn.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

if (!Dotted) // Regular line
	{
	#define L_DRAWPIXEL(Dest)	DRAWPIXEL(Dest)
	#define L_ASMPIXEL(Tag)		ASMPIXEL
	#define ISDOTTED 0
	#define LABEL2(X) LABEL1(X)##Normal
	#include "UnLine.cpp"
	#undef  LABEL2
	#undef  ISDOTTED
	#undef  L_DRAWPIXEL
	#undef  L_ASMPIXEL
	}
else // Dotted line
	{
	int LineToggle=0;
	#define L_DRAWPIXEL(Dest)  if (LineToggle^=1) DRAWPIXEL(Dest)
	#define ISDOTTED 1
	#define L_ASMPIXEL(Tag)\
		__asm{mov edx,[LineToggle]}\
		__asm{xor edx,1}\
		__asm{je LABEL2(Tag)##Next}\
		ASMPIXEL\
		__asm{LABEL2(Tag)##Next:}\
		__asm{mov [LineToggle],edx}
	#define LABEL2(X) LABEL1(X)##Dotted
	#include "UnLine.cpp"
	#undef LABEL2
	#undef ISDOTTED
	#undef L_DRAWPIXEL
	#undef L_ASMPIXEL
	};
