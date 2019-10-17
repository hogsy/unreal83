;================================================================================
; UnRender.inc: Unreal rendering engine MASM include file
;
; Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
; Compiled with MASM 6.11d
;
;	Revision history:
;		* Created by Tim Sweeney
;================================================================================

;---------;
; Equates ;
;---------;
u					TEXTEQU <>
um					TEXTEQU <>
uv					TEXTEQU <>
v					TEXTEQU <>
xxx					TEXTEQU <>
SELF_MOD_SEG		TEXTEQU <SelfModifyingCode SEGMENT PARA PRIVATE>
END_SELF_MOD_SEG	TEXTEQU <SelfModifyingCode ENDS>

LIGHT_XR			EQU 512
LIGHT_X_TOGGLE		EQU LIGHT_XR*4

;--------;
; Macros ;
;--------;

DD_GLOBAL MACRO Var:REQ
	PUBLIC Var
	Var DD ?
ENDM

DB_GLOBAL MACRO Var:REQ
	PUBLIC Var
	Var DB ?
ENDM

DQ_GLOBAL MACRO Var:REQ
	PUBLIC Var
	Var DQ ?
ENDM

;------------;
; Structures ;
;------------;

FTexLattice STRUCT
	Unused1		dd ?
	Unused2		dd ?
	Unused3		dd ?
	Unused4		dd ?
	Unused5		dd ?
	Unused6		dd ?
	Unused7		dd ?
	Unused8		dd ?
	;
	TU			dd ?
	TV			dd ?
	TG			dd ?
	Unused12	dd ?
	;
	LocX		dd ?
	LocY		dd ?
	LocZ		dd ?
	LocD		dd ?
	;
	RoutineOfs  dd ?
	AlignPad	dd ?
FTexLattice ENDS           

FTexLatticeD STRUCT
	LatL		dd ?
	LatH		dd ?
	LatLY		dd ?
	LatHY		dd ?
	LatLX		dd ?
	LatHX		dd ?
	LatLXY		dd ?
	LatHXY		dd ?
	;
	SubL		dd ?
	SubH		dd ?
	SubLY		dd ?
	SubHY		dd ?
	SubLX		dd ?
	SubHX		dd ?
	SubLXY		dd ?
	SubHXY		dd ?
	;
	RoutineOfs	dd ?
FTexLatticeD ENDS

FTexLatticeQ STRUCT
	LatQ		dd ?
	LatQY		dd ?
	LatQX		dd ?
	LatQXY		dd ?
	;
	SubQ		dd ?
	SubQY		dd ?
	SubQX		dd ?
	SubQXY		dd ?
	;
	RoutineOfs	dd ?
FTexLatticeQ ENDS

FBlitMipInfo STRUCT
	VMask		dd ?
	AndMask		dd ?
	Data		dd ?
	Dither		dd ?
	MipLevel	db ?
	UBits		db ?
	VBits		db ?
	Pad			db ?
FBlitMipInfo ENDS

FSpan STRUCT
	SpanStart	dd ?
	SpanEnd		dd ?
	SpanNext	dd ?
FSpan ENDS           

FBlit STRUCT
	LatticeX		dd ?
	LatticeY		dd ?
	SubX			dd ?
	SubY			dd ?
	InterX			dd ?
	InterY			dd ?
	;
	LatticeXMask	dd ?
	LatticeXNotMask	dd ?
	LatticeXMask4	dd ?
	InterXMask		dd ?
	InterXNotMask	dd ?
	;
	LatticeXBits	db ?
	LatticeYBits	db ?
	SubXBits		db ?
	SubYBits		db ?
	InterXBits		db ?
	InterYBits		db ?
	;
	dq ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
	dq ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
	dq ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
	dq ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
FBlit ENDS

;---------;
; Globals ;
;---------;

extrn GBlit : FBlit
extrn GLight : DWORD

;-------------------------------------------------------------------------------;
; The End
;-------------------------------------------------------------------------------;
