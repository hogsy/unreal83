;================================================================================
; UnTmap.asm: Unreal texture mapper assembly code.
;
; Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
; Compiled with Visual C++ 4.0 using Tabs=4, Calling method=__fastcall
;
;	Revision history:
;		* Created by Tim Sweeney
;================================================================================

.486P
.Model FLAT, C
APP_32BIT equ 1

;--------------------------------------------------------------------------------
; Definitions
;--------------------------------------------------------------------------------
.DATA

;
; Types
;
u			TEXTEQU <>
um			TEXTEQU <>
uv			TEXTEQU <>
v			TEXTEQU <>
xxx			TEXTEQU <>

;
; Structures
;

DD_GLOBAL MACRO Var:REQ
	PUBLIC Var
	Var DD ?
ENDM

DB_GLOBAL MACRO Var:REQ
	PUBLIC Var
	Var DB ?
ENDM

ALIGN 16
;
DD_GLOBAL	TM_RectY1
DD_GLOBAL	TM_DU
DD_GLOBAL	TM_DG0
DD_GLOBAL	TM_GInc
DD_GLOBAL	TM_SavedESP
DD_GLOBAL	TM_TexAddr
DD_GLOBAL	TM_SpanIndex
DD_GLOBAL	TM_Dest
;
DD_GLOBAL	TM_RectY2
DD_GLOBAL	TM_DG
DD_GLOBAL	TM_DU0
DD_GLOBAL	TM_UInc
DD_GLOBAL	TM_AndMask
DD_GLOBAL	TM_SpanDest
DD_GLOBAL	TM_End4
DD_GLOBAL	TM_BlenderDiff
;
DD_GLOBAL	TM_RectX1
DD_GLOBAL	TM_StartLine
DD_GLOBAL	TM_GG
DD_GLOBAL	TM_DU1
DD_GLOBAL	TM_DGInc
DD_GLOBAL	TM_DestInc
DD_GLOBAL	TM_Line
DD_GLOBAL	TM_SavedEBP
;
DD_GLOBAL	TM_RectX2
DD_GLOBAL	TM_UU
DD_GLOBAL	TM_DG1
DD_GLOBAL	TM_DUInc
DD_GLOBAL	TM_ShadeAddr
DD_GLOBAL	TM_EndAddr
DD_GLOBAL	TM_EndLine
;
DD_GLOBAL	TM_DitherPtr
DB_GLOBAL	TM_PixelShift

;
; Bilinear interpolation setup:
;
DD_GLOBAL	B_IU
DD_GLOBAL	B_IUX
DD_GLOBAL	B_IUY
DD_GLOBAL	B_IUXY
;
DD_GLOBAL	B_IV
DD_GLOBAL	B_IVX
DD_GLOBAL	B_IVY
DD_GLOBAL	B_IVXY
;
DD_GLOBAL	B_IG
DD_GLOBAL	B_IGX
DD_GLOBAL	B_IGY
DD_GLOBAL	B_IGXY
;
DD_GLOBAL   B_PanU
DD_GLOBAL   B_PanV

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
	Unused13	dd ?
	Unused14	dd ?
	Unused15	dd ?
	Unused16	dd ?
	;
	RoutineOfs  dd ?
FTexLattice ENDS           

FTexLatticeQ STRUCT
	LatL		dd ?
	LatH		dd ?
	LatLX		dd ?
	LatHX		dd ?
	LatLY		dd ?
	LatHY		dd ?
	LatLXY		dd ?
	LatHXY		dd ?
	;
	SubL		dd ?
	SubH		dd ?
	SubLX		dd ?
	SubHX		dd ?
	SubLY		dd ?
	SubHY		dd ?
	SubLXY		dd ?
	SubHXY		dd ?
FTexLatticeQ ENDS

;--------------------------------------------------------------------------------
; Support
;--------------------------------------------------------------------------------
.CODE
;include Filename.inc

;--------------------------------------------------------------------------------
; Unrolled 64-bit multiplier macro from hell
;--------------------------------------------------------------------------------

;
; Here we have 64 MASM labels (with entries labeled
; Offset0 - Offset 255) which perform shifts 
; identical to the following sequence:
;
; for (i=0; i<n; i++)
;    {
;    add ebx,esp
;    adc ebp,ecx
;    };
;
; In other words, this adds the 64-bit quantity
; (esp:ecx)*n to (ebx:ebp).
;

AddM MACRO ; 1 cycle
v	add ebx,esp
u	adc ebp,ecx
ENDM

SubtractM MACRO
v	sub ebx,esp
u	sbb ebp,ecx
ENDM

Times8M MACRO ; 4 cycles
v	mov eax,esp
u	shl ecx,3
v
u	shr eax,29
v
u	shl esp,3
v	add ecx,eax
u
ENDM

Times16M MACRO ; 4 cycles
v	mov eax,esp
u	shl ecx,4
v
u	shr eax,28
v
u	shl esp,4
v	add ecx,eax
u
ENDM

Times32M MACRO ; 4 cycles
v	mov eax,esp
u	shl ecx,5
v
u	shr eax,27
v
u	shl esp,5
v	add ecx,eax
u
ENDM

Times64M MACRO ; 4 cycles
v	mov eax,esp
u	shl ecx,6
v
u	shr eax,26
v
u	shl esp,6
v	add ecx,eax
u
ENDM

RoutineM MACRO Number:REQ
	LabelName TEXTEQU @CatStr(Offset,Number)
	jmp esi
	ALIGN 16
LabelName:
EndM

RoutineM 0x00	;
RoutineM 0x01	; 1
	AddM
RoutineM 0x02	; 1+1
	AddM
	AddM
RoutineM 0x03	; 1+1+1
	AddM
	AddM
	AddM
RoutineM 0x04	; 1+1+1+1
	AddM
	AddM
	AddM
	AddM
RoutineM 0x05	; 1+1+1+1+1
	AddM
	AddM
	AddM
	AddM
	AddM
RoutineM 0x06	; 1+1+1+1+1+1
	AddM
	AddM
	AddM
	AddM
	AddM
	AddM
RoutineM 0x07	; -1 + 8
	SubtractM
	Times8M
	AddM
RoutineM 0x08	; 8
	Times8M
	AddM
RoutineM 0x09	; 1 + 8
	AddM
	Times8M
	AddM
RoutineM 0x0A	; 1+1 + 8
	AddM
	AddM
	Times8M
	AddM
RoutineM 0x0B	; 1+1+1 + 8
	AddM
	AddM
	AddM
	Times8M
	AddM
RoutineM 0x0C	; -1-1-1-1 + 16
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	Times16M
	AddM
RoutineM 0x0D	; -1-1-1 + 16
	SubtractM
	SubtractM
	SubtractM
	Times16M
	AddM
RoutineM 0x0E	; -1-1 + 16
	SubtractM
	SubtractM
	Times16M
	AddM
RoutineM 0x0F	; -1 + 16
	SubtractM
	Times16M
	AddM
RoutineM 0x10	; 16
	Times16M
	AddM
RoutineM 0x11	; 1 + 16
	AddM
	Times16M
	AddM
RoutineM 0x12	; 1+1 + 16
	AddM
	AddM
	Times16M
	AddM
RoutineM 0x13	; 1+1+1 + 16
	AddM
	AddM
	AddM
	Times16M
	AddM
RoutineM 0x14	; 1+1+1+1 + 16
	AddM
	AddM
	AddM
	AddM
	Times16M
	AddM
RoutineM 0x15	; 1+1+1+1+1 + 16
	AddM
	AddM
	AddM
	AddM
	AddM
	Times16M
	AddM
RoutineM 0x16	; -1-1 + 8+8+8
	SubtractM
	SubtractM
	Times8M
	AddM
	AddM
	AddM
RoutineM 0x17	; -1 + 8+8+8 
	SubtractM
	Times8M
	AddM
	AddM
	AddM
RoutineM 0x18	; 8+8+8
	Times8M
	AddM
	AddM
	AddM
RoutineM 0x19	; 1 + 8+8+8
	AddM
	Times8M
	AddM
	AddM
	AddM
RoutineM 0x1A	; 1+1 + 8+8+8
	AddM
	AddM
	Times8M
	AddM
	AddM
	AddM
RoutineM 0x1B	; 1+1+1 + 8+8+8
	AddM
	AddM
	AddM
	Times8M
	AddM
	AddM
	AddM
RoutineM 0x1C	; 1+1+1+1 + 8+8+8
	AddM
	AddM
	AddM
	AddM
	Times8M
	AddM
	AddM
	AddM
RoutineM 0x1D	; -1-1-1 + 32
	SubtractM
	SubtractM
	SubtractM
	Times32M
	AddM
RoutineM 0x1E	; -1-1 + 32
	SubtractM
	SubtractM
	Times32M
	AddM
RoutineM 0x1F	; -1 + 32
	SubtractM
	Times32M
	AddM
RoutineM 0x20	; 32
	Times32M
	AddM
RoutineM 0x21	; 1 + 32
	AddM
	Times32M
	AddM
RoutineM 0x22	; 1+1 + 32
	AddM
	AddM
	Times32M
	AddM
RoutineM 0x23	; 1+1+1 + 32
	AddM
	AddM
	AddM
	Times32M
	AddM
RoutineM 0x24	; 1+1+1+1 + 32
	AddM
	AddM
	AddM
	AddM
	Times32M
	AddM
RoutineM 0x25	; 1+1+1+1+1 + 32
	AddM
	AddM
	AddM
	AddM
	AddM
	Times32M
	AddM
RoutineM 0x26	; 1+1+1+1+1+1 + 32
	AddM
	AddM
	AddM
	AddM
	AddM
	AddM
	Times32M
	AddM
RoutineM 0x27	; -1 + 8+8+8+8+8
	SubtractM
	Times8M
	AddM
	AddM
	AddM
	AddM
	AddM
RoutineM 0x28	; 8+8+8+8+8
	Times8M
	AddM
	AddM
	AddM
	AddM
	AddM
RoutineM 0x29	; 1 +8+8+8+8+8
	AddM
	Times8M
	AddM
	AddM
	AddM
	AddM
	AddM
RoutineM 0x2A	; 1+1 + 8+8+8+8+8
	AddM
	AddM
	Times8M
	AddM
	AddM
	AddM
	AddM
	AddM
RoutineM 0x2B	; 1+1+1 + 8+8+8+8+8
	AddM
	AddM
	AddM
	Times8M
	AddM
	AddM
	AddM
	AddM
	AddM
RoutineM 0x2C	; -1-1-1-1 + 16+16+16
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x2D	; -1-1-1 + 16+16+16
	SubtractM
	SubtractM
	SubtractM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x2E	; -1-1 + 16+16+16
	SubtractM
	SubtractM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x2F	; -1 + 16+16+16
	SubtractM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x30	; 16+16+16
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x31	; 1 + 16+16+16
	AddM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x32	; 1+1 + 16+16+16
	AddM
	AddM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x33	; 1+1+1 + 16+16+16
	AddM
	AddM
	AddM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x34	; 1+1+1+1 + 16+16+16
	AddM
	AddM
	AddM
	AddM
	Times16M
	AddM
	AddM
	AddM
RoutineM 0x35	; -1-1-1 -8 + 64
	SubtractM
	SubtractM
	SubtractM
	Times8M
	SubtractM
	Times8M
	AddM
RoutineM 0x36	; -1-1 -8 + 64
	SubtractM
	SubtractM
	Times8M
	SubtractM
	Times8M
	AddM
RoutineM 0x37	; -1 -8 + 64
	SubtractM
	Times8M
	SubtractM
	Times8M
	AddM
RoutineM 0x38	; -8 + 64
	Times8M
	SubtractM
	Times8M
	AddM
RoutineM 0x39	; 1 -8 + 64
	AddM
	Times8M
	SubtractM
	Times8M
	AddM
RoutineM 0x3A	; -1-1-1-1-1-1 + 64
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	Times64M
	AddM
RoutineM 0x3B	; -1-1-1-1-1 + 64
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	Times64M
	AddM
RoutineM 0x3C	; -1-1-1-1 + 64
	SubtractM
	SubtractM
	SubtractM
	SubtractM
	Times64M
	AddM
RoutineM 0x3D	; -1-1-1 + 64
	SubtractM
	SubtractM
	SubtractM
	Times64M
	AddM
RoutineM 0x3E	; -1-1 + 64
	SubtractM
	SubtractM
	Times64M
	AddM
RoutineM 0x3F	; -1 + 64
	SubtractM
	Times64M
	AddM
RoutineM Unused

.DATA
MultTable DD 0
	FORC High,<0123>
		FORC Low,<0123456789ABCDEF>
			RoutineName TEXTEQU @CatStr(@CatStr(Offset0x,High),Low)
			DD RoutineName
		ENDM
	ENDM
.CODE

;--------------------------------------------------------------------------------
; Texture span macros
;--------------------------------------------------------------------------------

;
; Lead-in for all span drawers
;

TexLeadIn MACRO ShiftCount:REQ, DitherNum
	DGLabel TEXTEQU @CatStr(TM_DG,DitherNum)
	DULabel TEXTEQU @CatStr(TM_DU,DitherNum)
	;
u	mov eax,[TM_AndMask]	; Get address mask
v	mov esp,[DGLabel]		; Get shading increment
u	and eax,ebp				; Mask the address
v	xxx
u	rol eax,ShiftCount		; Shift texel address into place
v	add ebx,esp				; Update shade value
u	mov esp,[DULabel]		; Get texture increment
v	adc ebp,esp				; Move through texture
ENDM

;
; All texture loops
;

; Normal texture loop:
TexLoop0 MACRO DestReg:REQ, ShiftCount:REQ, DitherNum
	DGLabel TEXTEQU @CatStr(TM_DG,DitherNum)
	DULabel TEXTEQU @CatStr(TM_DU,DitherNum)
	;
u	mov cl,[eax+esi]		; Get texel and put into color lookup table address
v	mov eax,[TM_AndMask]	; Get address mask
u	and eax,ebp				; Get masked, unshifted texel address
v	mov ch,bh				; Put shading value into color lookup table address
u	rol eax,ShiftCount		; Shift texel address into place
v	mov esp,[DGLabel]		; Get shading increment
u	add ebx,esp				; Update shade value
v	mov esp,[DULabel]		; Get texture increment
u	adc ebp,esp				; Move through texture
v	mov DestReg,[ecx]		; Get pixel from color lookup table
ENDM

TexStart0 MACRO
	; Do nothing
ENDM

; Masked texture loop:
TexLoop1 MACRO DestReg:REQ, ShiftCount:REQ, DitherNum
	LOCAL SkipMasked
	DGLabel TEXTEQU @CatStr(TM_DG,DitherNum)
	DULabel TEXTEQU @CatStr(TM_DU,DitherNum)
	;
u	mov cl,[eax+esi]		; Get texel and put into color lookup table address
v	mov eax,[TM_AndMask]	; Get address mask
u	and eax,ebp				; Get masked, unshifted texel address
v	mov ch,bh				; Put shading value into color lookup table address
u	rol eax,ShiftCount		; Shift texel address into place
v	mov esp,[DGLabel]		; Get shading increment
u	test cl,cl				; Is this pixel masked?
v	jz  SkipMasked			; Skip masked pixels
u	mov DestReg,[ecx]		; Get pixel from color lookup table
v	xxx
	SkipMasked:
u	add ebx,esp				; Update shade value
v	mov esp,[DULabel]		; Get texture increment
u	adc ebp,esp				; Move through texture
v	xxx
ENDM

TexStart1 MACRO
	mov edx,[edi]
ENDM

; Blended texture loop:
TexLoop2 MACRO DestReg:REQ, ShiftCount:REQ, DitherNum
	DGLabel TEXTEQU @CatStr(TM_DG,DitherNum)
	DULabel TEXTEQU @CatStr(TM_DU,DitherNum)
	;
u	mov cl,[eax+esi]		; Get texel and put into color lookup table address
v	mov eax,[TM_AndMask]	; Get address mask
u	and eax,ebp				; Get masked, unshifted texel address
v	mov ch,bh				; Put shading value into color lookup table address
u	rol eax,ShiftCount		; Shift texel address into place
v	mov esp,[DGLabel]		; Get shading increment
u	mov cl,[ecx]			; Get texel for blending
v	add ebx,esp				; Update shade value
u	mov ch,DestReg			; Get screen pixel for blending
v	mov esp,[DULabel]		; Get texture increment
u	adc ebp,esp				; Move through texture
v	mov esp,[TM_BlenderDiff]; Get blender table pointer difference
u	xxx						
v	xxx
u	mov DestReg,[ecx+esp]	; Lookup final pixel from blender
ENDM

TexStart2 MACRO
	mov edx,[edi]
ENDM

; Masked, blended texture loop:
TexLoop3 MACRO DestReg:REQ, ShiftCount:REQ, DitherNum
	LOCAL SkipMasked
	DGLabel TEXTEQU @CatStr(TM_DG,DitherNum)
	DULabel TEXTEQU @CatStr(TM_DU,DitherNum)
	;
u	mov cl,[eax+esi]		; Get texel and put into color lookup table address
v	mov eax,[TM_AndMask]	; Get address mask
u	and eax,ebp				; Get masked, unshifted texel address
v	mov ch,bh				; Put shading value into color lookup table address
u	rol eax,ShiftCount		; Shift texel address into place
v	test cl,cl				; Is this pixel masked?
u	mov cl,[ecx]			; Place screen pixel in blender
v	jz  SkipMasked			; Skip masked pixels
u	mov ch,DestReg			; Place screen pixel in blender
v	mov esp,[TM_BlenderDiff]; Get blender table pointer difference
u	xxx
v	xxx
u	mov DestReg,[ecx+esp]
	SkipMasked:
v	mov esp,[DGLabel]		; Get shading increment
u	add ebx,esp				; Update shade value
v	mov esp,[DULabel]		; Get texture increment
u	adc ebp,esp				; Move through texture
v	xxx
ENDM

TexStart3 MACRO
	mov edx,[edi]
ENDM

;
; Span setup & draw
;

TexSpanMacro MACRO DoneMacro:REQ,ShiftCount:REQ,DrawKind:REQ
	LOCAL LeadIn0,LeadIn1,LeadIn2,LeadIn3,FourLoop,LeadInTable
	LOCAL LoopOfs0,LoopOfs1,LoopOfs2,LoopOfs3,LeadOut,DoLeadIn
	TexLoop  TEXTEQU @CatStr(TexLoop,DrawKind)
	TexStart TEXTEQU @CatStr(TexStart,DrawKind)
	;
	; Set up all increments and addresses:
	; (eax, edx, esp, esi, ecx = temp)
	;
u	mov edx,[TM_Line]
v	mov esp,[TM_DitherPtr]
u	and edx,3
v	mov esi,[TM_DG]
u	shl edx,5
v	mov ecx,[TM_DU]
u	add esp,edx
v	xxx
u	xxx
v	xxx
	;
u	mov edx,[esp]
v	xxx
u	mov eax,[esp+4]
v	add edx,esi
u	adc eax,ecx
v	mov [TM_DG0],edx
u	mov edx,[esp+16]
v	mov [TM_DU0],eax
u	mov eax,[esp+20]
v	add edx,esi
u	adc eax,ecx
v	mov ecx,edi
u	mov [TM_DG1],edx
v	and ecx,3
u	mov [TM_DU1],eax
v	jnz DoLeadIn
	;
	LeadIn0:
u	mov eax,[esp+24]
v	mov esi,[TM_TexAddr]
u	add ebx,eax
v	mov edx,[esp+28]
u	adc ebp,edx
v	mov ecx,[TM_ShadeAddr]
	TexStart
	TexLeadIn ShiftCount,1
	TexLoop dl,ShiftCount,0
	TexLoop dh,ShiftCount,1
u	ror   edx,16
v	add   edi,4
	TexLoop dl,ShiftCount,0
	TexLoop dh,ShiftCount,1
u	ror   edx,16
v	mov   esp,[TM_EndAddr]
u	cmp   edi,esp
v	jl    FourLoop
u	and   esp,3
v	jnz	  LeadOut
u	mov   [edi-4],edx
v	DoneMacro
	;
	ALIGN 16
	DoLeadIn:
	jmp LeadInTable[ecx*4]
	;
	ALIGN 16
	LeadIn1:
u	sub edi,1
v	mov edx,[esp+8]
u	add ebx,edx
v	mov edx,[esp+12]
u	adc ebp,edx
v	mov edx,[edi]
	TexLeadIn ShiftCount,0
u	mov esi,[TM_TexAddr]
v	mov ecx,[TM_ShadeAddr]
	TexLoop dh,ShiftCount,1
u	ror   edx,16
v	add   edi,4
	TexLoop dl,ShiftCount,0
	TexLoop dh,ShiftCount,1
u	ror   edx,16
v	mov   esp,[TM_EndAddr]
u	cmp   edi,esp
v	jl    FourLoop
u	and   esp,3
v	jnz	  LeadOut
u	mov   [edi-4],edx
v	DoneMacro
	;
	ALIGN 16
	LeadIn2:
u	add edi,2
v	mov edx,[esp+24]
u	add ebx,edx
v	mov edx,[esp+28]
u	adc ebp,edx
u	mov edx,[edi-4]
	TexLeadIn ShiftCount,1
u	mov esi,[TM_TexAddr]
v	mov ecx,[TM_ShadeAddr]
u	ror edx,16
	TexLoop dl,ShiftCount,0
	TexLoop dh,ShiftCount,1
u	ror   edx,16
v	mov   esp,[TM_EndAddr]
u	cmp   edi,esp
v	jl    FourLoop
u	and   esp,3
v	jnz	  LeadOut
u	mov   [edi-4],edx
v	DoneMacro
	;
	ALIGN 16
	LeadIn3:
u	add edi,1
v	mov edx,[esp+8]
u	add ebx,edx
v	mov edx,[esp+12]
u	adc ebp,edx
u	mov edx,[edi-4]
u	ror edx,16
u	mov esi,[TM_TexAddr]
v	mov ecx,[TM_ShadeAddr]
	TexLeadIn ShiftCount,0
	TexLoop dh,ShiftCount,1
u	ror   edx,16
v	mov   esp,[TM_EndAddr]
u	cmp   edi,esp
v	jl    FourLoop
u	and   esp,3
v	jnz	  LeadOut
u	mov   [edi-4],edx
v	DoneMacro
	;
	ALIGN 16
FourLoop:
u	mov [edi-4],edx
v	TexStart
	TexLoop dl,ShiftCount,0
	TexLoop dh,ShiftCount,1
u	rol   edx,16
v	add   edi,4
	TexLoop dl,ShiftCount,0
	TexLoop dh,ShiftCount,1
u	ror   edx,16
v	mov   esp,[TM_EndAddr]
u	cmp   edi,esp
v	jl    FourLoop
u	and   esp,3
v	jnz	  LeadOut
u	mov   [edi-4],edx
v	DoneMacro
	;
	ALIGN 16
	LeadOut:
u	mov	eax,esp
v	mov ebx,[edi-4]
u	mov ecx,FourUnmask[eax*4]
v	xxx
u	mov esi,FourMask[eax*4]
v	and ebx,ecx
u	and edx,esi
v	xxx
u	or  edx,ebx
v	xxx
u	mov [edi-4],edx
v	DoneMacro
	;
	ALIGN 16
	LeadInTable:
	DD	LeadIn0
	DD	LeadIn1
	DD	LeadIn2
	DD	LeadIn3
ENDM

ALIGN 16
FourMask:
DD  000000000h
DD  0000000ffh
DD  00000ffffh
DD  000ffffffh

ALIGN 16
FourUnmask:
DD  0ffffffffh
DD  0ffffff00h
DD  0ffff0000h
DD  0ff000000h

;
; Define all span routines
;
FORC ShiftCount, <0123456789AB>
	FORC TexDrawKind, <0123>
		TextureProcName  TEXTEQU @CatStr(@CatStr(TextureSpan,ShiftCount),TexDrawKind)
		;
		DoneMacro MACRO
			mov esp,[TM_SavedESP]
			mov ebp,[TM_SavedEBP]
			ret
		ENDM
		;
		; Routine:
		;
		ALIGN 16
		TextureProcName PROC C PUBLIC
			mov [TM_SavedESP],esp
			mov [TM_SavedEBP],ebp
			;
			mov edi,[TM_SpanDest]
			mov ebx,[TM_GG]
			mov ebp,[TM_UU]
			TexSpanMacro <DoneMacro>,@CatStr(0,@CatStr(ShiftCount,h)),TexDrawKind
		TextureProcName ENDP
	ENDM
ENDM

;
; Table of all span routine entry points:
;
.DATA
ALIGN 16
PUBLIC TextureSpanTable
TextureSpanTable DD 0
FORC ShiftCount, <0123456789AB>
	FORC TexDrawKind, <0123>
		TextureProcName TEXTEQU @CatStr(@CatStr(TextureSpan,ShiftCount),TexDrawKind)
		DD TextureProcName
	ENDM
ENDM
.CODE

;--------------------------------------------------------------------------------
; Block texture mappers
;--------------------------------------------------------------------------------

SetupDoSpanDoneMacro MACRO DoneMacro:REQ
u	mov		esi,[TM_SpanIndex]
v	mov		ecx,[TM_Line]
u	mov		edi,[esi]
v	DoneMacro
ENDM

;
; Set up to draw without jump-in:
;
SetupDoSpanMacro MACRO ShiftCount:REQ, DrawKind:REQ, DoneMacro:REQ
u	mov		[esi],edi
v	mov		edi,[TM_Dest]
u	add		edi,edx
v	mov		[TM_SpanIndex],esi
u	add		ebx,edi
v	mov		[TM_Line],ecx
u	mov		[TM_EndAddr],ebx
	;
u	mov ebx,[TM_GG]	; G start
v	mov ebp,[TM_UU]	; U start
	;
	TexSpanMacro <SetupDoSpanDoneMacro <DoneMacro>>,ShiftCount,DrawKind
ENDM

RedoLineMacro MACRO	
u	mov		edi,[edi+8]			; Go to next span
v	mov		esp,[TM_RectX1]
u	mov		ebp,[TM_RectX2]
v	jmp		SkipEmptySpans
ENDM

;
; Set up to draw with jump-in:
;
SetupDoSpanJumpInMacro MACRO ShiftCount:REQ, DrawKind:REQ, DoneMacro:REQ
	LOCAL DoneTextureSpan,SuckLoop,Continue,Slow
	;
	LocalDoneMacro MACRO
	ENDM
	;
	; Set up to draw:
	;
u	mov		[esi],edi
v	mov		edi,[TM_Dest]
u	add		edi,edx
v	mov		[TM_SpanIndex],esi
u	add		ebx,edi
v	mov		[TM_Line],ecx
u	mov		[TM_EndAddr],ebx
	;
u	mov		ebx,[TM_GG]	; G start
v	mov		ebp,[TM_UU]	; U start
	;
	; Jump-in:
	;
u	mov esp,[TM_DG]
v	mov ecx,[TM_DU]
u	mov esi,Continue
v	jmp (MultTable+4)[edx*4]
	Continue:
	;
	TexSpanMacro <SetupDoSpanDoneMacro <DoneMacro>>,ShiftCount,DrawKind
ENDM

;
; Texture block drawer
;   esi = span index pointer
;   edi = span pointer
;
TexBlockMacro MACRO ShiftCount:REQ,DrawKind:REQ
	LOCAL Vert1,Vert2,Exit
	;
	pushad
	mov [TM_SavedESP],esp
	;
	; See if we're skipping vertical lines:
	;
	mov		edx,				[TM_StartLine]
	mov		esi,				[TM_SpanIndex]
	mov		ecx,				[TM_StartLine]
	sub		edx,				[TM_RectY1]
	jz		BlockLineLoop
	;
	; Fix up for vertical offset:
	;
	mov ebx,[TM_GG]	; G start
	mov ebp,[TM_UU]	; U start
	mov esp,[TM_GInc]
	mov ecx,[TM_UInc]
	mov esi,Vert1
	jmp (MultTable+4)[edx*4]
	;
	Vert1:
	mov [TM_GG],ebx
	mov [TM_UU],ebp
	mov ebx,[TM_DG]	; G start
	mov ebp,[TM_DU]	; U start
	mov esp,[TM_DGInc]
	mov ecx,[TM_DUInc]
	mov esi,Vert2
	jmp (MultTable+4)[edx*4]
	;
	Vert2:
	mov [TM_DG],ebx
	mov [TM_DU],ebp
	;
	mov		ecx,				[TM_StartLine]
	mov		esi,				[TM_SpanIndex]
	jmp		BlockLineLoop
	;
	; Exit:
	;
	ALIGN 16
	Exit:
u	mov esp,[TM_SavedESP]
v	popad
	ret
	;
	; Go to next line:
	;
	ALIGN 16
	NextLine:
	;
	; Update coordinates and go to next span
	;
u	mov		eax,[TM_GG]
v	mov		esp,[TM_GInc]
u	add		eax,esp
v	mov		[TM_GG],eax
	;
u	mov		ebx,[TM_UU]
v	mov		esp,[TM_UInc]
u	adc		ebx,esp
v	mov		[TM_UU],ebx
	;
u	mov		eax,[TM_DG]
v	mov		esp,[TM_DGInc]
u	add		eax,esp
v	mov		[TM_DG],eax
	;
u	mov		ebx,[TM_DU]
v	mov		esp,[TM_DUInc]
u	adc		ebx,esp
v	mov		[TM_DU],ebx
	;
u	mov		eax,[TM_Dest]		; Dest
v	mov		esp,[TM_DestInc]
u	add		eax,esp
v	mov		[esi],edi			; Update span index
	;
u	mov		[TM_Dest],eax
v	add		esi,4				; Go to next line
u	mov     edx,[TM_EndLine]
v	inc		ecx
u	cmp		ecx,edx
v	jge		Exit
	;
	; Block line loop:
	;
	BlockLineLoop:
u	mov		esp,[TM_RectX1]
v	mov		edi,[esi]			; edi = span pointer
u	mov		ebp,[TM_RectX2]
v	xxx
	;
	SkipEmptySpans:
u	cmp		edi,0				; Done with line?
v	jz		NextLine			; Do next, usually not taken
u   mov     edx,edi				; save span pointer
v   mov     eax,[edi]           ; eax = start value
u	mov		ebx,[edi+4]			; ebx = end value
v	cmp		ebx,esp				; If End <= TM_RectX1, keep searching
u	mov		edi,[edi+8]			; Go to next span
v	jle		SkipEmptySpans		; Usually not taken
	;
	; Check start:
	;
u	mov     edi,edx				; Back up so we process this span again
v	xor		edx,edx				; Offset = 0
u	cmp		eax,ebp				; If SpanStart >= TM_RectX2, don't process this line yet
v	jge		NextLine			; Usually not taken
	;
	; Now set up esp=SpanStartX, ebx=SpanEndX
	;
u	sub		eax,esp				; if SpanStart > TM_RectX1, start is clipped
v	jg		StartClipped		; Usually not taken
	;
	; ------------------
	; Start is unclipped
	; ------------------
	;
u	cmp		ebx,ebp				; If SpanEnd < TM_RectX2, Do shortened end-clipped section
v	jle		EndClipped			; Usually not taken
u	mov		ebx,ebp
v	sub		ebx,esp				; Find length
	;
	; Start=Unclipped, End=Unclipped
	;
	SetupDoSpanMacro ShiftCount,DrawKind,<jmp NextLine>
	;
	; Start=Unclipped, End=Clipped
	;
	ALIGN 16
	EndClipped:
u	sub		ebx,esp				; Find length
	SetupDoSpanMacro ShiftCount,DrawKind,<RedoLineMacro>
	;
	; ----------------
	; Start is clipped
	; ----------------
	;
	ALIGN 16
	StartClipped:
u	add		esp,eax				; Now eax=positive offset, esp=SpanStartX
v	mov		edx,eax
u	cmp		ebx,ebp				; If SpanEnd > TM_RectX2, Do shortened section
v	jle		BothClipped
	;
	; Start=Clipped, End=Unclipped
	;
u	mov		ebx,ebp
v	sub		ebx,esp				; Find length
	SetupDoSpanJumpInMacro ShiftCount,DrawKind,<jmp NextLine>
	;
	; Start=Clipped, End=Clipped
	;
	ALIGN 16
	BothClipped:
u	sub		ebx,esp				; Find length
v	jle		NextLine			; Skip zero-length spans (sometimes taken)
	SetupDoSpanJumpInMacro ShiftCount,DrawKind,<RedoLineMacro>
ENDM

;
; Define all block routines
;
FORC ShiftCount, <0123456789AB>
	FORC TexDrawKind, <0123>
		TextureProcName TEXTEQU @CatStr(@CatStr(TextureBlock,ShiftCount),TexDrawKind)
		;
		; Routine:
		;
		ALIGN 16
		TextureProcName PROC C PUBLIC
			TexBlockMacro @CatStr(0,@CatStr(ShiftCount,h)),TexDrawKind
		TextureProcName ENDP
	ENDM
ENDM

;
; Table of all block routine entry points:
;
.DATA
ALIGN 16
PUBLIC TextureBlockTable
TextureBlockTable DD 0
FORC ShiftCount, <0123456789AB>
	FORC TexDrawKind, <0123>
		TextureProcName TEXTEQU @CatStr(@CatStr(TextureBlock,ShiftCount),TexDrawKind)
		DD TextureProcName
	ENDM
ENDM
.CODE

;--------------------------------------------------------------------------------
; The End
;--------------------------------------------------------------------------------
END
