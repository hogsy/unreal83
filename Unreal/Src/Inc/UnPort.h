/*=============================================================================
	UnPort.h: All platform-specific macros and stuff to aid in porting
	between different systems.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNPORT // Prevent multiple includes
#define _INC_UNPORT

/*----------------------------------------------------------------------------
	Platforms:
		__WIN32__		Windows

	Compilers:
		__MSVC__		Microsoft Visual C++ 4.0

	Byte order:
		__INTEL__		Intel byte order
		__NONINTEL__	Non-intel byte order
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
	Ansi C libraries
----------------------------------------------------------------------------*/

#ifndef  _INC_STDIO
#include <stdio.h>
#endif

#ifndef  _INC_STDLIB
#include <stdlib.h>
#endif

#ifndef  _INC_STRING
#include <string.h>
#endif

#ifndef  _INC_MATH
#include <math.h>
#endif

#ifndef  _INC_STDARG
#include <stdarg.h>
#endif

#ifndef  _INC_MALLOC
#include <malloc.h>
#endif

#ifndef  _INC_CTYPE
#include <ctype.h>
#endif

#ifndef _INC_TYPES
#include <sys\types.h>
#endif

#if _MSC_VER
/*----------------------------------------------------------------------------
	PC Windows 95/NT: Microsoft Visual C++ 4.0
----------------------------------------------------------------------------*/

#define __MSVC__
#define __WIN32__
#define __INTEL__

//
// Undo any Windows defines
//
#ifdef _WINDOWS
	#undef BYTE
	#undef CHAR
	#undef WORD
	#undef DWORD
	#undef INT
	#undef FLOAT
	#undef TRUE
	#undef FALSE
	#undef MAXBYTE
	#undef MAXWORD
	#undef MAXDWORD
	#undef MAXINT
	#undef VOID
#endif

//
// Base datatypes
//
typedef unsigned char  		BYTE;
typedef char    			CHAR;
typedef unsigned short 		WORD;
typedef unsigned long  		DWORD;
typedef unsigned __int64	QWORD;
typedef signed char    		SBYTE;
typedef signed short   		SWORD;
typedef signed int    		INT;
typedef signed __int64 		SQWORD;
typedef signed int          BOOL;
typedef float               FLOAT;
typedef double				DOUBLE;
typedef void				VOID;
typedef int					INDEX;		// Index into a list - actors, bsp, etc.
typedef unsigned int		RESINDEX;	// Resource index into GRes table
typedef int					ANGLE_TYPE;	// An angle. Only low word matters. 65536=360 degrees.

//
// Global constants
//
enum {MAXBYTE		= 0xff};
enum {MAXWORD		= 0xffffU};
enum {MAXDWORD		= 0xffffffffUL};
enum {MAXSBYTE		= 0x7f};
enum {MAXSWORD		= 0x7fff};
enum {MAXINT		= 0x7fffffffL};
enum {INDEX_NONE	= (INDEX)MAXWORD};
enum {NAME_NONE		= INDEX_NONE};
enum {FILE_NONE		= (BYTE)~0};
enum {NAME_SIZE		= 32};
enum {RESINDEX_NONE = (RESINDEX)~0};
enum EBool {FALSE=0, TRUE=1};

//
// Optimization macros (preceeded by #pragma):
//
#define DISABLE_OPTIMIZATION optimize("",off)
#define ENABLE_OPTIMIZATION  optimize("",on)

//
// Assembly code
//
#ifdef ASM
	#define ASMVAR extern
#else
	#define ASMVAR
#endif

//
// Function type macros:
//
#define DLL_IMPORT	__declspec(dllimport)	/* Import function from DLL */
#define DLL_EXPORT  __declspec(dllexport)	/* Export function to DLL */
#define IUNKNOWN    __stdcall				/* Component object model standard call */
#define IUNKNOWNV   __cdecl					/* Component object model variable argument call */
#define VARARGS     __cdecl					/* Functions with variable arguments */

//
// API defines used so that Unreal headers work both when 
// compiling each of the particular DLL's:
//
#ifdef COMPILING_ENGINE
	#define UNREAL_API DLL_EXPORT /* Export when compiling the engine */
#else
	#define UNREAL_API DLL_IMPORT /* Import when compiling an add-on */
#endif

#ifdef COMPILING_GAME
	#define UNGAME_API DLL_EXPORT /* Export when compiling game */
#else
	#define UNGAME_API DLL_IMPORT /* Import when not compiling game */
#endif

#ifdef COMPILING_NETWORK
	#define UNNETWORK_API DLL_EXPORT /* Export when compiling network code */
#else
	#define UNNETWORK_API DLL_IMPORT /* Import when compiling network code */
#endif

#ifdef COMPILING_RENDER
	#define UNRENDER_API DLL_EXPORT /* Export when compiling rendering code */
#else
	#define UNRENDER_API DLL_IMPORT /* Import when compiling rendering code */
#endif

#ifdef COMPILING_EDITOR
	#define UNEDITOR_API DLL_EXPORT /* Export when compiling editor code */
#else
	#define UNEDITOR_API DLL_IMPORT /* Export when compiling editor code */
#endif

#ifdef _DEBUG
	#define COMPILER "Compiled with Visual C++ 4.0 Debug"
#else
	#define COMPILER "Compiled with Visual C++ 4.0"
#endif

//
// Precision timing (Thanks to Erik de Nieve)
//
#ifdef ASM
	#pragma warning (disable : 4035) // legalize implied return value EAX
	inline DWORD TimeCpuCycles(void)
		{
		__asm
			{
			_emit 0x0F // RDTSC  -  Pentium+ time stamp register to EDX:EAX
			_emit 0x31 //  use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec
			};
		};
	#pragma warning (default : 4035)
    #define CYCLE_OVERHEAD 14  /* 14 cycles = RDTSC and SUB time */
#else
	inline DWORD TimeCpuCycles(void) {return 0;};
	#define CYCLE_OVERHEAD 0
#endif

#define ALWAYS_BEGINTIME(StatName) StatName -= TimeCpuCycles();
#define ALWAYS_ENDTIME(StatName)   StatName += TimeCpuCycles() - CYCLE_OVERHEAD;
UNREAL_API extern float Cyc2Msec;
//
#ifdef STATS_TIMED
    #define BEGINTIME(StatName)		ALWAYS_BEGINTIME(StatName)
    #define ENDTIME(StatName)		ALWAYS_ENDTIME(StatName)
#else
	#define BEGINTIME(StatName) /* Do nothing */
	#define ENDTIME(StatName)   /* Do nothing */
#endif

//
// Unwanted VC++ warnings to disable:
//
#pragma warning(disable : 4244) /* conversion to float, possible loss of data							*/
#pragma warning(disable : 4761) /* integral size mismatch in argument; conversion supplied				*/
#pragma warning(disable : 4699) /* creating precompiled header											*/
#pragma warning(disable : 4055) /* void pointer casting													*/
#pragma warning(disable : 4127) /* conditional 'while (1)' is constant									*/
#pragma warning(disable : 4152) /* function/data pointer conversion										*/
#pragma warning(disable : 4200) /* Zero-length array item at end of structure, a VC-specific extension	*/
#pragma warning(disable : 4100) /* unreferenced formal parameter										*/
#pragma warning(disable : 4514) /* unreferenced inline function has been removed						*/
#pragma warning(disable : 4201) /* nonstandard extension used : nameless struct/union					*/
#pragma warning(disable : 4710) /* inline function not expanded											*/
#pragma warning(disable : 4702) /* unreachable code in inline expanded function							*/

//
// VC++ warnings enabled:
//
#pragma warning(3       : 4706) /* assignment within conditional expression								*/
#pragma warning(3       : 4705) /* statement has no effect												*/
#pragma warning(3       : 4709) /* comma operator within array index expression							*/
#pragma warning(3       : 4128) /* storage-class specifier after type									*/
#pragma warning(3       : 4130) /* logical operation on address of string constant						*/
#pragma warning(3       : 4131) /* function uses old-style declaration									*/
#pragma warning(3       : 4132) /* const object should be initialized									*/

#else
/*----------------------------------------------------------------------------
	Unknown compiler
----------------------------------------------------------------------------*/

#error Unknown compiler, not supported by Unreal!
#endif

/*----------------------------------------------------------------------------
	Universal types
----------------------------------------------------------------------------*/

//
// Globally unique identifier
//
class FGUID
	{
	public:
    DWORD	Data1;
    WORD	Data2;
    WORD	Data3;
    BYTE	Data4[8];
	};

//
// Definition of FUnknown, equivalant to Ole IUnknown.
//
class UNREAL_API FUnknown
	{
	public:
    DWORD virtual IUNKNOWN AddRef(void);
    DWORD virtual IUNKNOWN Release(void);
    DWORD virtual IUNKNOWN QueryInterface(FGUID GUID, void** ppvObj);
	};

//
// Byte swap
//
#define BS_WORD(x)  (((x)>>8) | (((x)&0xff)<<8))
#define BS_DWORD(x) (((x)>>24) | (((x)&0xff0000)>>8) | (((x)&0xff00)<<8) | (((x)&0xff)<<24))
#define BS_QWORD(x) )((QWORD)BS_DWORD(x)<<32) | ((QWORD)BS_DWORD(x>>32)))

//
// Intel functions
//
#ifdef	__INTEL__
	#define	HOST_WORD(x)   ((WORD)x)
	#define HOST_SWORD(x)  ((SWORD)x)
	#define	HOST_DWORD(x)  ((DWORD)x)
	#define	HOST_QWORD(x)  ((QWORD)x)
	#define HOST_INT(x)	   ((INT)x)
	//
	#define	NET_WORD(x)		BS_WORD(x)
	#define NET_SWORD(x)	((SWORD)BS_WORD((WORD)x))
	#define	NET_DWORD(x)	BS_DWORD(x)
	#define	NET_QWORD(x)	BS_QWORD(x)
	#define NET_INT(x)		((INT)BS_WORD((DWORD)x))
#endif

//
// Non-Intel functions
//
#ifdef	__NONINTEL__
	#define	HOST_WORD(x)    BS_WORD(x)
	#define HOST_SWORD(x)   ((SWORD)BS_WORD((WORD)x))
	#define	HOST_DWORD(x)   BS_DWORD(x)
	#define	HOST_QWORD(x)   BS_QWORD(x)
	#define HOST_INT(x)	    ((INT)BS_WORD((DWORD)x))
	//
	#define	NET_WORD(x)		((WORD)x)
	#define NET_SWORD(x)	((SWORD)x)
	#define	NET_DWORD(x)	((DWORD)x)
	#define	NET_QWORD(x)	((QWORD)x)
	#define NET_INT(x)		((INT)x)
#endif

/*----------------------------------------------------------------------------
	The End
----------------------------------------------------------------------------*/
#endif // _INC_UNPORT
