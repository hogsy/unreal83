;================================================================================
; UnBlast.asm: Unreal texture blasting routines (really fast texture mapping)
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
; Support
;--------------------------------------------------------------------------------
include unrender.inc

;--------------------------------------------------------------------------------
; Texture lighting inner loop
;--------------------------------------------------------------------------------

;-----------;
; Registers ;
;-----------;
; eax = meshel address
; ebx = mesh address low interpolator
; edx = FTexLattice pointer
; esi = mesh address high interpolator
; edi = destination address
; ebp = fractional coord working register
; esp = working register

;---------;
; Globals ;
;---------;
.DATA
DD_GLOBAL	TLI_MeshFloat
DD_GLOBAL	TLI_Sinc
DD_GLOBAL	TLI_Mask
DD_GLOBAL	TLI_AddrMask
DD_GLOBAL	TLI_Temp
DD_GLOBAL	TLI_ProcBase
DD_GLOBAL	TLI_TopLattice
DD_GLOBAL	TLI_Dest
DD_GLOBAL	TLI_DestEnd
DD_GLOBAL	TLI_SkipIn
;
DD_GLOBAL	TLO_TopBase
DD_GLOBAL	TLO_BotBase
DD_GLOBAL	TLO_FinalDest
DD_GLOBAL	TLO_RectSpan
DD_GLOBAL	TLO_LatticeBase
DD_GLOBAL	TLO_LightInnerProc
;
SavedESP	DD ?
SavedEBP	DD ?
SavedProc	DD ?
SavedInner  DD ?
SavedOffset	DD ?

;--------------------------------------;
; Bilinear texture lighting inner loop ;
; Floating point version               ;
; 25 cycles per point, cache-friendly  ;
;--------------------------------------;
.CODE
FOR UBits,<0,1,2,3,4,5,6,7,8,9,10,11>
	LOCAL PrimeLoop,LightReLoop,LightLoopMain,FinishingUp
	;
	; All skip-in entries:
	;
	SkipIn = 32
	REPEAT 32
		@CatStr(TLI_Proc,UBits,_,%SkipIn):
		add ebx,ebp
		adc esi,eax
		SkipIn = SkipIn - 1
	ENDM
	;
	; No skip-in entry:
	;
	@CatStr(TLI_Proc,UBits,_0):
		mov		[SavedESP],esp
		mov		esp,[TLI_Sinc]			; Get sinc table address
		;
		mov		eax,[TLI_AddrMask]		; Get address mask
		mov		ebp,esi					; Get U interpolator
		;
		shr		ebp,56-UBits			; ebp = Uf
		and		eax,esi					; Mask address with texture coords
		;
		rol		eax,UBits+2				; Convert texture coords into linear address
		mov		ecx,[TLI_MeshFloat]		; Get light mesh base address
		;
		and		ebp,255					; ebp = Uf Alpha index into sinc table
		add		eax,ecx					; Now eax = base address of illumination element
		;
		lea ecx,[esi+(1 SHL (32-UBits))]
		and ecx,[TLI_AddrMask]
		rol ecx,UBits+2
		add ecx,[TLI_MeshFloat]
		;
		fld		DWORD PTR [ecx+(4 SHL UBits)]	; D
		fsub	DWORD PTR [eax+(4 SHL UBits)] ; D-C
		fld		DWORD PTR [ecx]			; B
		fsub	DWORD PTR [eax]			; B-A
		fxch
		;
		jmp		LightLoopMain
		;
		;-----------
		LightReLoop:
		;-----------
		;
		ALIGN 16
		faddp st(1),st					; AB + (CD-AB)*Beta
		;
		lea ecx,[esi+(1 SHL (32-UBits))]
		and ecx,[TLI_AddrMask]
		rol ecx,UBits+2
		add ecx,[TLI_MeshFloat]
		;
		fld		DWORD PTR [ecx+(4 SHL UBits)] ; D
		fld		DWORD PTR [ecx]			; B
		fxch
		fsub	DWORD PTR [eax+(4 SHL UBits)] ; D-C
		fxch	st(2)
		fstp	[TLI_Temp]				; Store floating-point light value to temporary location
		fsub	DWORD PTR [eax]			; B-A
		fxch
		;
		mov		ecx,[TLI_Temp]
		and		ebp,255					; ebp = Uf Alpha index into sinc table
		;
		and		ecx,03ff0h
		;
		mov		[edi-4],ecx				; Save light value
		;
		;-------------
		LightLoopMain:
		;-------------
		;
		fmul	DWORD PTR [esp+ebp*4]	; (D-C) * Alpha
		fxch
		fmul	DWORD PTR [esp+ebp*4]	; (B-A) * Alpha
		;
		mov		ecx,[edx].FTexLatticeD.SubLX
		mov		ebp,ebx					; Get V interpolator
		shr		ebp,24					; Convert v fraction into index into GSincData
		add		ebx,ecx					; Update v fractional interpolator
		;
		fadd	DWORD PTR [eax]			; A + (B-A) * Alpha = AB
		fxch
		fadd	DWORD PTR [eax+(4 SHL UBits)] ; C + (D-C) * Alpha = CD
		;
		mov		ecx,[edx].FTexLatticeD.SubHX
		mov		eax,[TLI_AddrMask]		; Get address mask for next iter
		adc		esi,ecx					; Update texture interpolator
		add		edi,4
		;
		fsub	st,st(1)				; (CD-AB)
		;
		and		eax,esi					; Mask address with texture coords
		mov		ecx,[TLI_MeshFloat]		; Get light mesh base address
		rol		eax,UBits+2				; Convert texture coords into linear address
		;
		fmul DWORD PTR [esp+ebp*4]		; (CD-AB)*Beta
		;
		mov		ebp,esi					; Get U interpolator
		add		eax,ecx					; Now eax = base address of illumination element
		shr		ebp,56-UBits			; ebp = Uf
		mov		ecx,[TLI_DestEnd]		; Get end pointer
		cmp		edi,ecx					; Are we at the end?
		jl		LightReLoop				; No, continue on with loop middle
		;
		;-----------
		FinishingUp:
		;-----------
		;
		faddp	st(1),st				; AB + (CD-AB)*Beta
		mov		esp,[SavedESP]			; Done
		;
		fstp	[TLI_Temp]				; Store floating-point light value to temporary location
		;
		mov		ecx,[TLI_Temp]
		and		ecx,03ff0h
		;
		mov		[edi-4],ecx				; Save light value
	ret
ENDM

;--------------------------------;
; Table of all bilinear encoders ;
;--------------------------------;
ALIGN 16
PUBLIC TLI_ProcTable
TLI_ProcTable LABEL DWORD
FOR UBits, <0,1,2,3,4,5,6,7,8,9,10,11>
	SkipIn = 0
	REPEAT 64
		IF SkipIn LE 32
			DD @CatStr(TLI_Proc,UBits,_,%SkipIn)
		ELSE
			DD 0
		ENDIF
		SkipIn = SkipIn + 1
	ENDM
ENDM

;--------------------------------------------------------------------------------
; Texture mapper inner loops
;--------------------------------------------------------------------------------

;---------;
; Globals ;
;---------;

.DATA

DD_GLOBAL	TMI_Shader
DD_GLOBAL	TMI_Dest
DD_GLOBAL	TMI_FinalDest
DD_GLOBAL	TMI_DiffLight
DD_GLOBAL	TMI_DiffDest
DD_GLOBAL	TMI_ProcBase
DD_GLOBAL   TMI_TopLattice
DD_GLOBAL   TMI_LatticeBase
DD_GLOBAL   TMI_NextLatticeBase
DD_GLOBAL	TMI_LineProcBase
DD_GLOBAL	TMI_RectFinalDest
DQ_GLOBAL	TMI_DTex

DD_GLOBAL	TMO_DestOrDestBitsPtr
DD_GLOBAL	TMO_Dest
DD_GLOBAL	TMO_DestBits

DD_GLOBAL	TMO_Span
DD_GLOBAL	TMO_OrigSpan
DD_GLOBAL	TMO_RectSpan
DD_GLOBAL	TMO_NextRectSpan
DD_GLOBAL	TMO_Stride

SavedFinalDest	DD 0
TexStuff		DQ 0,0

; Texture mapper varieties
; ------------------------
;
; 8P...8-bit Pentium
; 16P..16-bit Pentium
;
; 16M..16-bit MMX
; 24M..24-bit MMX
; 32M..32-bit MMX

;----------------------------------;
; 8-bit color Pentium texture loop ;
;----------------------------------;
;
; 7.25 cycles per pixel (when trilerp=0)
;
; Trilerpness
;          0  1  2  3
;        +------------
; Line 0 | 00 10 01 01
;      1 | 00 00 10 11
;
; Pixel 0: ((Line EQ 0) and (Trilerp EQ 1)) or ((Line EQ 1) and (Trilerp GE 2))
; Pixel 1: ((Line EQ 0) and (Trilerp GE 2)) or ((Line EQ 1) and (Trilerp EQ 3))
;
; eax = dest
; ebx = fractional tex coords
; ecx = pixel accumulator / scratch
; edx = lighting table
; esi = tex coords
; edi = tex incs
; esp = texture offset / scratch
; ebp = fractional tex incs
;
; Must not affect TMI_FinalDest
;
TMI_Define8P MACRO Mip:req, Line:req, Line1:req, Trilerp:req
	LOCAL SM_Dest1,SM_Dest2,SM_Dest3,SM_Dest4
	LOCAL SM_UBits1,SM_UBits2,SM_UBits3,SM_UBits4,SM_UBits5
	LOCAL SM_Ofs0Low,SM_Ofs0High ; Ofs
	LOCAL SM_Ofs1Low,SM_Ofs1High ; Delta
	LOCAL SM_Ofs2Low,SM_Ofs2High ; -Delta
	LOCAL SM_Ofs3Low,SM_Ofs3High ; Delta
	LOCAL SM_Ofs4Low,SM_Ofs4High ; -Delta
	LOCAL SM_AdjustLow1,SM_AdjustLow2
	LOCAL TexInnerLoop
	LOCAL EvenMip,OddMip
	;
	IF ((Line1 EQ 0) and (Trilerp EQ 1)) or ((Line1 EQ 1) and (Trilerp GE 2))
		EvenMip EQU 1
	ELSE
		EvenMip EQU 0
	ENDIF
	;
	IF ((Line1 EQ 0) and (Trilerp GE 2)) or ((Line1 EQ 1) and (Trilerp EQ 3))
		OddMip EQU 1
	ELSE
		OddMip EQU 0
	ENDIF
	;
	; Texture mapper inner loop:
	; edi = texture lattice pointer
	; ebp = destination
	;
	ALIGN 16
	@CatStr(TMI_Proc8P,Mip,Line,Trilerp):
	;
	; Setup, get first shading:
	;
	mov eax,[edi].FTexLatticeD.LatLX			; Low inc
	mov	[SavedESP],esp							; Save stack
	;
	mov DWORD PTR [TMI_DTex],eax
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov esp,[edi].FTexLatticeD.LatHX
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov DWORD PTR [TMI_DTex+4],esp
	;
	mov ecx,[eax+ebp]							; Get lighting value (guaranteed &0x3ffc)
	mov edx,[TMI_Shader]						; Get shading lookup table address
	;
	mov edi,[eax+ebp+4]							; Get next lighting value
	add ebx,ecx
	;
	sub edi,ebx									; Take difference
	mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
	;
	shl edi,16									; Pad so that a sar edx,18 will be like sar bx,2
	SM_Ofs0Low LABEL DWORD
	add ebx,012345678h
	;
	SM_Ofs0High LABEL DWORD
	adc esi,012345678h
	;xxx
	;
	sar edi,18									; Now have lighting increment
	and eax,esi
	;
	jmp	@CatStr(TMI_ProcInner8P,Mip,Line,Trilerp)
	;
	; Inner loop:
	;
	ALIGN 16
	TexInnerLoop:
		;
		mov [ebp-4],ecx							; Store pixels
		;
	@CatStr(TMI_ProcInner8P,Mip,Line,Trilerp):
		;
		IF EvenMip
			shr al,1
		ENDIF
		and edi,00000ffffh
		;
		SM_UBits1 LABEL BYTE
		rol eax,7
		mov	ecx,DWORD PTR [TMI_DTex]			; Get tex inc masked with 0ffff0000h
		;
		; Do 4-pixel shading & Pixel 1:
		;
		or  edi,ecx
		mov esp,DWORD PTR [TMI_DTex+4]
		;
		SM_Dest1 LABEL DWORD
		mov dl,[eax+012345678h]
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs1Low LABEL DWORD
		add ebx,012345678h
		;
		SM_Ofs1High LABEL DWORD
		adc esi,012345678h
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		;
		; -- Pixel 1 --
		;
		and eax,esi
		mov dh,bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits2 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs2Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs2High LABEL DWORD
		sbb esi,012345678h
		mov cl,[edx]
		;
		; -- Pixel 2 --
		;
		SM_Dest2 LABEL DWORD
		mov dl,[eax+012345678h]
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		;
		and eax,esi
		mov dh,bh
		;
		IF EvenMip
			shr al,1
		ENDIF
		;
		SM_UBits3 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs3Low LABEL DWORD
		add ebx,012345678h
		;
		SM_Ofs3High LABEL DWORD
		adc esi,012345678h
		mov ch,[edx]
		;
		shl ecx,16
		add ebp,4
		;
		; -- Pixel 3 --
		;
		SM_Dest3 LABEL DWORD
		mov dl,[eax+012345678h]
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		;
		and eax,esi
		mov dh,bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits4 LABEL BYTE
		rol eax,7
		;
		mov cl,[edx]
		SM_Dest4 LABEL DWORD
		mov dl,[eax+012345678h]
		;
		; -- Pixel 4 --
		;
		mov eax,[TMI_DiffLight]					; Pointer difference from [DestBase] to [LightBase]
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs4Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs4High LABEL DWORD
		sbb esi,012345678h
		mov edi,[eax+ebp+4]
		;
		SM_AdjustLow1 LABEL DWORD
		add edi,012345678h
		mov dh,bh
		;
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		sub edi,ebx								; Take difference
		;
		shl edi,16								; Pad so that a sar edx,18 will be like sar bx,2
		mov ch,[edx]
		;
		ror ecx,16
		and eax,esi
		;
		sar edi,18								; Now have lighting increment
		mov esp,[TMI_FinalDest]					; Get end pointer
		;
		cmp ebp,esp								; More to do?
		jl  TexInnerLoop						; Loop
		;
		; Done:
		;
		mov	esp,[SavedESP]
	ret
	;
	; Per-polygon self modifier macro routine for this loop:
	;
	@CatStr(TMI_Mod8PMacro,Mip,Line,Trilerp) MACRO
		;
		; Assembly proc to set up all self-modifying code offsets
		; for one mipmap of a texture map.  Call successively with
		; the appropriate info for each mipmap.  Call with:
		;
		; eax = this dither table pointer
		; ebx = this texture address mask
		; edx = mip texture address mask
		; ecx = U Bits (cl=this, ch=mip)
		; esi = texture base address
		; edi = mip base address
		;
		IF EvenMip
			mov [SM_Dest1+2],edi
			mov [SM_Dest3+2],edi
			;
			mov [SM_UBits1+2],ch
			mov [SM_UBits3+2],ch
		ELSE
			mov [SM_Dest1+2],esi
			mov [SM_Dest3+2],esi
			;
			mov [SM_UBits1+2],cl
			mov [SM_UBits3+2],cl
		ENDIF
		;
		; Pixel 1 & 3 setup:
		;
		IF OddMip
			mov [SM_Dest2+2],edi
			mov [SM_Dest4+2],edi
			;
			mov [SM_UBits2+2],ch
			mov [SM_UBits4+2],ch
		ELSE
			mov [SM_Dest2+2],esi
			mov [SM_Dest4+2],esi
			;
			mov [SM_UBits2+2],cl
			mov [SM_UBits4+2],cl
		ENDIF
		;
		mov ebx,[eax]			; Dither delta low
		mov edx,[eax+4]			; Dither delta high
		;
		mov [SM_Ofs1Low+2],ebx
		mov [SM_Ofs2Low+2],ebx
		mov [SM_Ofs3Low+2],ebx
		mov [SM_Ofs4Low+2],ebx
		;
		mov [SM_Ofs1High+2],edx
		mov [SM_Ofs2High+2],edx
		mov [SM_Ofs3High+2],edx
		mov [SM_Ofs4High+2],edx
		;
		mov ebx,[eax+8]			; Dither offset low
		mov edx,[eax+12]		; Dither offset high
		;
		mov [SM_Ofs0Low+2],ebx
		mov [SM_AdjustLow1+2],ebx
		mov [SM_Ofs0High+2],edx
	ENDM
ENDM

; Temporary PentiumPro version for testing
.DATA
GenAddr1  DD ?
GenAddr2  DD ?
GenAddr3  DD ?
GenAddr4  DD ?
GenPixels DD ?
;
TMI_Define8_PentiumProTemporary MACRO Mip:req, Line:req, Line1:req, Trilerp:req
	LOCAL SM_Dest1,SM_Dest2,SM_Dest3,SM_Dest4
	LOCAL SM_UBits1,SM_UBits2,SM_UBits3,SM_UBits4,SM_UBits5
	LOCAL SM_Ofs0Low,SM_Ofs0High ; Ofs
	LOCAL SM_Ofs1Low,SM_Ofs1High ; Delta
	LOCAL SM_Ofs2Low,SM_Ofs2High ; -Delta
	LOCAL SM_Ofs3Low,SM_Ofs3High ; Delta
	LOCAL SM_Ofs4Low,SM_Ofs4High ; -Delta
	LOCAL SM_AdjustLow1,SM_AdjustLow2
	LOCAL TexInnerLoop
	LOCAL EvenMip,OddMip
	;
	IF ((Line1 EQ 0) and (Trilerp EQ 1)) or ((Line1 EQ 1) and (Trilerp GE 2))
		EvenMip EQU 1
	ELSE
		EvenMip EQU 0
	ENDIF
	;
	IF ((Line1 EQ 0) and (Trilerp GE 2)) or ((Line1 EQ 1) and (Trilerp EQ 3))
		OddMip EQU 1
	ELSE
		OddMip EQU 0
	ENDIF
	;
	; Texture mapper skip-in:
	; edi = texture lattice pointer
	; ecx = skip-in value
	;
	; Texture mapper inner loop:
	; edi = texture lattice pointer
	; ebp = destination
	;
	ALIGN 16
	@CatStr(TMI_Proc8P,Mip,Line,Trilerp):
	;
	mov edx,[TMI_Shader]						; Get shading lookup table address
	mov [GenAddr1],edx
	mov [GenAddr2],edx
	mov [GenAddr3],edx
	mov [GenAddr4],edx
	;
	; Setup, get first shading:
	;
	mov eax,[edi].FTexLatticeD.LatLX			; Low inc
	mov	[SavedESP],esp							; Save stack
	;
	mov DWORD PTR [TMI_DTex],eax
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov esp,[edi].FTexLatticeD.LatHX
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov DWORD PTR [TMI_DTex+4],esp
	;
	mov ecx,[eax+ebp]							; Get lighting value (guaranteed &0x3ffc)
	;
	mov edi,[eax+ebp+4]							; Get next lighting value
	add ebx,ecx
	;
	sub edi,ebx									; Take difference
	mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
	;
	shl edi,16									; Pad so that a sar edx,18 will be like sar bx,2
	SM_Ofs0Low LABEL DWORD
	add ebx,012345678h
	;
	SM_Ofs0High LABEL DWORD
	adc esi,012345678h
	;xxx
	;
	sar edi,18									; Now have lighting increment
	and eax,esi
	;
	jmp	@CatStr(TMI_ProcInner8P,Mip,Line,Trilerp)
	;
	; Inner loop:
	;
	ALIGN 16
	TexInnerLoop:
		;
		mov [ebp-4],ecx							; Store pixels
		;
	@CatStr(TMI_ProcInner8P,Mip,Line,Trilerp):
		;
		IF EvenMip
			shr al,1
		ENDIF
		and edi,00000ffffh
		;
		SM_UBits1 LABEL BYTE
		rol eax,7
		mov	ecx,DWORD PTR [TMI_DTex]			; Get tex inc masked with 0ffff0000h
		;
		; Do 4-pixel shading & Pixel 1:
		;
		or  edi,ecx
		mov esp,DWORD PTR [TMI_DTex+4]
		;
		SM_Dest1 LABEL DWORD
		mov dl,[eax+012345678h]
		add ebx,edi
		mov BYTE PTR [GenAddr1],dl
		;
		adc esi,esp
		SM_Ofs1Low LABEL DWORD
		add ebx,012345678h
		;
		SM_Ofs1High LABEL DWORD
		adc esi,012345678h
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		;
		; -- Pixel 1 --
		;
		and eax,esi
		mov BYTE PTR [GenAddr1+1],bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits2 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs2Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs2High LABEL DWORD
		sbb esi,012345678h
		;
		; -- Pixel 2 --
		;
		SM_Dest2 LABEL DWORD
		mov dl,[eax+012345678h]
		mov BYTE PTR [GenAddr2],dl
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		;
		and eax,esi
		mov BYTE PTR [GenAddr2+1],bh
		;
		IF EvenMip
			shr al,1
		ENDIF
		;
		SM_UBits3 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs3Low LABEL DWORD
		add ebx,012345678h
		;
		SM_Ofs3High LABEL DWORD
		adc esi,012345678h
		;
		shl ecx,16
		add ebp,4
		;
		; -- Pixel 3 --
		;
		SM_Dest3 LABEL DWORD
		mov dl,[eax+012345678h]
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		mov BYTE PTR [GenAddr3],dl
		;
		and eax,esi
		mov BYTE PTR [GenAddr3+1],bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits4 LABEL BYTE
		rol eax,7
		;
		SM_Dest4 LABEL DWORD
		mov dl,[eax+012345678h]
		;
		; -- Pixel 4 --
		;
		mov eax,[TMI_DiffLight]					; Pointer difference from [DestBase] to [LightBase]
		add ebx,edi
		mov BYTE PTR [GenAddr4],dl
		;
		adc esi,esp
		SM_Ofs4Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs4High LABEL DWORD
		sbb esi,012345678h
		mov edi,[eax+ebp+4]
		;
		SM_AdjustLow1 LABEL DWORD
		add edi,012345678h
		mov BYTE PTR [GenAddr4+1],bh
		;
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		sub edi,ebx								; Take difference
		;
		shl edi,16								; Pad so that a sar edx,18 will be like sar bx,2
		;
		ror ecx,16
		and eax,esi
		;
		sar edi,18								; Now have lighting increment
		;
		mov esp,[GenAddr1]
		mov cl,[esp]
		mov esp,[GenAddr2]
		mov ch,[esp]
		mov BYTE PTR [GenPixels+0],cl
		mov BYTE PTR [GenPixels+1],ch
		;
		mov esp,[GenAddr3]
		mov cl,[esp]
		mov esp,[GenAddr4]
		mov ch,[esp]
		mov BYTE PTR [GenPixels+2],cl
		mov BYTE PTR [GenPixels+3],ch
		;
		mov ecx,[GenPixels]
		;
		mov esp,[TMI_FinalDest]					; Get end pointer
		;
		cmp ebp,esp								; More to do?
		jl  TexInnerLoop						; Loop
		;
		; Done:
		;
		mov	esp,[SavedESP]
	ret
	;
	; Per-polygon self modifier macro routine for this loop:
	;
	@CatStr(TMI_Mod8PMacro,Mip,Line,Trilerp) MACRO
		;
		; Assembly proc to set up all self-modifying code offsets
		; for one mipmap of a texture map.  Call successively with
		; the appropriate info for each mipmap.  Call with:
		;
		; eax = this dither table pointer
		; ebx = this texture address mask
		; edx = mip texture address mask
		; ecx = U Bits (cl=this, ch=mip)
		; esi = texture base address
		; edi = mip base address
		;
		IF EvenMip
			mov [SM_Dest1+2],edi
			mov [SM_Dest3+2],edi
			;
			mov [SM_UBits1+2],ch
			mov [SM_UBits3+2],ch
		ELSE
			mov [SM_Dest1+2],esi
			mov [SM_Dest3+2],esi
			;
			mov [SM_UBits1+2],cl
			mov [SM_UBits3+2],cl
		ENDIF
		;
		; Pixel 1 & 3 setup:
		;
		IF OddMip
			mov [SM_Dest2+2],edi
			mov [SM_Dest4+2],edi
			;
			mov [SM_UBits2+2],ch
			mov [SM_UBits4+2],ch
		ELSE
			mov [SM_Dest2+2],esi
			mov [SM_Dest4+2],esi
			;
			mov [SM_UBits2+2],cl
			mov [SM_UBits4+2],cl
		ENDIF
		;
		mov ebx,[eax]			; Dither delta low
		mov edx,[eax+4]			; Dither delta high
		;
		mov [SM_Ofs1Low+2],ebx
		mov [SM_Ofs2Low+2],ebx
		mov [SM_Ofs3Low+2],ebx
		mov [SM_Ofs4Low+2],ebx
		;
		mov [SM_Ofs1High+2],edx
		mov [SM_Ofs2High+2],edx
		mov [SM_Ofs3High+2],edx
		mov [SM_Ofs4High+2],edx
		;
		mov ebx,[eax+8]			; Dither offset low
		mov edx,[eax+12]		; Dither offset high
		;
		mov [SM_Ofs0Low+2],ebx
		mov [SM_AdjustLow1+2],ebx
		mov [SM_Ofs0High+2],edx
	ENDM
ENDM

;-----------------------------------;
; 16-bit color Pentium texture loop ;
;-----------------------------------;
;
; ? cycles per pixel (when trilerp=0)
;
; Trilerpness
;          0  1  2  3
;        +------------
; Line 0 | 00 10 01 01
;      1 | 00 00 10 11
;
; Pixel 0: ((Line EQ 0) and (Trilerp EQ 1)) or ((Line EQ 1) and (Trilerp GE 2))
; Pixel 1: ((Line EQ 0) and (Trilerp GE 2)) or ((Line EQ 1) and (Trilerp EQ 3))
;
; eax = dest
; ebx = fractional tex coords
; ecx = pixel accumulator / scratch
; edx = lighting table
; esi = tex coords
; edi = tex incs
; esp = texture offset / scratch
; ebp = fractional tex incs
;
; Must not affect TMI_FinalDest
;
TMI_Define16P MACRO Mip:req, Line:req, Line1:req, Trilerp:req
	LOCAL SM_Dest1,SM_Dest2,SM_Dest3,SM_Dest4
	LOCAL SM_UBits1,SM_UBits2,SM_UBits3,SM_UBits4,SM_UBits5
	LOCAL SM_Ofs0Low,SM_Ofs0High ; Ofs
	LOCAL SM_Ofs1Low,SM_Ofs1High ; Delta
	LOCAL SM_Ofs2Low,SM_Ofs2High ; -Delta
	LOCAL SM_Ofs3Low,SM_Ofs3High ; Delta
	LOCAL SM_Ofs4Low,SM_Ofs4High ; -Delta
	LOCAL SM_AdjustLow1,SM_AdjustLow2
	LOCAL TexInnerLoop
	LOCAL EvenMip,OddMip,TempLabel
	;
	IF ((Line1 EQ 0) and (Trilerp EQ 1)) or ((Line1 EQ 1) and (Trilerp GE 2))
		EvenMip EQU 1
	ELSE
		EvenMip EQU 0
	ENDIF
	;
	IF ((Line1 EQ 0) and (Trilerp GE 2)) or ((Line1 EQ 1) and (Trilerp EQ 3))
		OddMip EQU 1
	ELSE
		OddMip EQU 0
	ENDIF
	;
	; Texture mapper skip-in:
	; edi = texture lattice pointer
	; ecx = skip-in value
	;
	; Texture mapper inner loop:
	; edi = texture lattice pointer
	; ebp = destination
	;
	ALIGN 16
	@CatStr(TMI_Proc16P,Mip,Line,Trilerp):
	;
	; Setup, get first shading:
	;
	mov eax,[edi].FTexLatticeD.LatLX			; Low inc
	mov	[SavedESP],esp							; Save stack
	;
	mov DWORD PTR [TMI_DTex],eax
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov esp,[edi].FTexLatticeD.LatHX
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov DWORD PTR [TMI_DTex+4],esp
	;
	mov ecx,[eax+ebp]							; Get lighting value (guaranteed &0x3ffc)
	mov edx,[TMI_Shader]						; Get shader address
	;
	mov edi,[eax+ebp+4]							; Get next lighting value
	add ebx,ecx
	;
	sub edi,ebx									; Take difference
	mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
	;
	shl edi,16									; Pad so that a sar edi,18 will be like sar bx,2
	SM_Ofs0Low LABEL DWORD
	add ebx,012345678h
	;
	SM_Ofs0High LABEL DWORD
	adc esi,012345678h
	;xxx
	;
	sar edi,18									; Now have lighting increment
	and eax,esi
	;
	jmp	@CatStr(TMI_ProcInner16P,Mip,Line,Trilerp)
	;
	; Inner loop:
	;
	ALIGN 16
	TexInnerLoop:
		;
		mov ecx,[DWORD PTR TexStuff]
		mov esp,[DWORD PTR TexStuff+4]
		mov [ebp*2-8],ecx
		mov [ebp*2-4],esp
		;
	@CatStr(TMI_ProcInner16P,Mip,Line,Trilerp):
		;
		IF EvenMip
			shr al,1
		ENDIF
		and edi,00000ffffh
		;
		SM_UBits1 LABEL BYTE
		rol eax,7
		mov	ecx,DWORD PTR [TMI_DTex]			; Get tex inc masked with 0ffff0000h
		;
		; Do 4-pixel shading & Pixel 1:
		;
		or  edi,ecx
		mov esp,DWORD PTR [TMI_DTex+4]
		;
		SM_Dest1 LABEL DWORD
		mov dl,[eax+012345678h]
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs1Low LABEL DWORD
		add ebx,012345678h
		;
		SM_Ofs1High LABEL DWORD
		adc esi,012345678h
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		;
		; -- Pixel 1 --
		;
		and eax,esi
		mov dh,bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits2 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs2Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs2High LABEL DWORD
		sbb esi,012345678h
		mov cx,[edx*2]
		;
		; -- Pixel 2 --
		;
		SM_Dest2 LABEL DWORD
		mov dl,[eax+012345678h]
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		;
		and eax,esi
		mov dh,bh
		;
		IF EvenMip
			shr al,1
		ENDIF
		;
		SM_UBits3 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs3Low LABEL DWORD
		add ebx,012345678h
		;
		mov WORD PTR [TexStuff+0],cx
		mov cx,[edx*2]
		;
		SM_Ofs3High LABEL DWORD
		adc esi,012345678h
		mov WORD PTR [TexStuff+2],cx
		;
		add ebp,4
		;
		; -- Pixel 3 --
		;
		SM_Dest3 LABEL DWORD
		mov dl,[eax+012345678h]
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		;
		and eax,esi
		mov dh,bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits4 LABEL BYTE
		rol eax,7
		mov cx,[edx*2]
		;
		mov WORD PTR [TexStuff+4],cx
		SM_Dest4 LABEL DWORD
		mov dl,[eax+012345678h]
		;
		; -- Pixel 4 --
		;
		mov eax,[TMI_DiffLight]					; Pointer difference from [DestBase] to [LightBase]
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs4Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs4High LABEL DWORD
		sbb esi,012345678h
		mov edi,[eax+ebp+4]
		;
		SM_AdjustLow1 LABEL DWORD
		add edi,012345678h
		mov dh,bh
		;
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		sub edi,ebx								; Take difference
		;
		shl edi,16								; Pad so that a sar edi,18 will be like sar bx,2
		mov cx,[edx*2]
		;
		and eax,esi
		mov WORD PTR [TexStuff+6],cx
		;
		sar edi,18								; Now have lighting increment
		mov esp,[TMI_FinalDest]					; Get end pointer
		;
		cmp ebp,esp								; More to do?
		jl  TexInnerLoop						; Loop
		;
		; Done:
		;
		mov	esp,[SavedESP]
	ret
	;
	; Per-polygon self modifier macro routine for this loop:
	;
	@CatStr(TMI_Mod16PMacro,Mip,Line,Trilerp) MACRO
		;
		; Assembly proc to set up all self-modifying code offsets
		; for one mipmap of a texture map.  Call successively with
		; the appropriate info for each mipmap.  Call with:
		;
		; eax = this dither table pointer
		; ebx = this texture address mask
		; edx = mip texture address mask
		; ecx = U Bits (cl=this, ch=mip)
		; esi = texture base address
		; edi = mip base address
		;
		IF EvenMip
			mov [SM_Dest1+2],edi
			mov [SM_Dest3+2],edi
			;
			mov [SM_UBits1+2],ch
			mov [SM_UBits3+2],ch
		ELSE
			mov [SM_Dest1+2],esi
			mov [SM_Dest3+2],esi
			;
			mov [SM_UBits1+2],cl
			mov [SM_UBits3+2],cl
		ENDIF
		;
		; Pixel 1 & 3 setup:
		;
		IF OddMip
			mov [SM_Dest2+2],edi
			mov [SM_Dest4+2],edi
			;
			mov [SM_UBits2+2],ch
			mov [SM_UBits4+2],ch
		ELSE
			mov [SM_Dest2+2],esi
			mov [SM_Dest4+2],esi
			;
			mov [SM_UBits2+2],cl
			mov [SM_UBits4+2],cl
		ENDIF
		;
		mov ebx,[eax]			; Dither delta low
		mov edx,[eax+4]			; Dither delta high
		;
		mov [SM_Ofs1Low+2],ebx
		mov [SM_Ofs2Low+2],ebx
		mov [SM_Ofs3Low+2],ebx
		mov [SM_Ofs4Low+2],ebx
		;
		mov [SM_Ofs1High+2],edx
		mov [SM_Ofs2High+2],edx
		mov [SM_Ofs3High+2],edx
		mov [SM_Ofs4High+2],edx
		;
		mov ebx,[eax+8]			; Dither offset low
		mov edx,[eax+12]		; Dither offset high
		;
		mov [SM_Ofs0Low+2],ebx
		mov [SM_AdjustLow1+2],ebx
		mov [SM_Ofs0High+2],edx
	ENDM
ENDM

;-----------------------------------;
; 32-bit color Pentium texture loop ;
;-----------------------------------;
;
; ? cycles per pixel (when trilerp=0)
;
; Trilerpness
;          0  1  2  3
;        +------------
; Line 0 | 00 10 01 01
;      1 | 00 00 10 11
;
; Pixel 0: ((Line EQ 0) and (Trilerp EQ 1)) or ((Line EQ 1) and (Trilerp GE 2))
; Pixel 1: ((Line EQ 0) and (Trilerp GE 2)) or ((Line EQ 1) and (Trilerp EQ 3))
;
; eax = dest
; ebx = fractional tex coords
; ecx = pixel accumulator / scratch
; edx = lighting table
; esi = tex coords
; edi = tex incs
; esp = texture offset / scratch
; ebp = fractional tex incs
;
; Must not affect TMI_FinalDest
;
TMI_Define32P MACRO Mip:req, Line:req, Line1:req, Trilerp:req
	LOCAL SM_Dest1,SM_Dest2,SM_Dest3,SM_Dest4
	LOCAL SM_UBits1,SM_UBits2,SM_UBits3,SM_UBits4,SM_UBits5
	LOCAL SM_Ofs0Low,SM_Ofs0High ; Ofs
	LOCAL SM_Ofs1Low,SM_Ofs1High ; Delta
	LOCAL SM_Ofs2Low,SM_Ofs2High ; -Delta
	LOCAL SM_Ofs3Low,SM_Ofs3High ; Delta
	LOCAL SM_Ofs4Low,SM_Ofs4High ; -Delta
	LOCAL SM_AdjustLow1,SM_AdjustLow2
	LOCAL TexInnerLoop
	LOCAL EvenMip,OddMip,TempLabel
	;
	IF ((Line1 EQ 0) and (Trilerp EQ 1)) or ((Line1 EQ 1) and (Trilerp GE 2))
		EvenMip EQU 1
	ELSE
		EvenMip EQU 0
	ENDIF
	;
	IF ((Line1 EQ 0) and (Trilerp GE 2)) or ((Line1 EQ 1) and (Trilerp EQ 3))
		OddMip EQU 1
	ELSE
		OddMip EQU 0
	ENDIF
	;
	; Texture mapper skip-in:
	; edi = texture lattice pointer
	; ecx = skip-in value
	;
	; Texture mapper inner loop:
	; edi = texture lattice pointer
	; ebp = destination
	;
	ALIGN 16
	@CatStr(TMI_Proc32P,Mip,Line,Trilerp):
	;
	; Setup, get first shading:
	;
	mov eax,[edi].FTexLatticeD.LatLX			; Low inc
	mov	[SavedESP],esp							; Save stack
	;
	mov DWORD PTR [TMI_DTex],eax
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov esp,[edi].FTexLatticeD.LatHX
	mov eax,[TMI_DiffLight]						; Pointer difference from [DestBase] to [LightBase]
	;
	mov DWORD PTR [TMI_DTex+4],esp
	;
	mov ecx,[eax+ebp]							; Get lighting value (guaranteed &0x3ffc)
	mov edx,[TMI_Shader]						; Get shader address
	;
	mov edi,[eax+ebp+4]							; Get next lighting value
	add ebx,ecx
	;
	sub edi,ebx									; Take difference
	mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
	;
	shl edi,16									; Pad so that a sar edi,18 will be like sar bx,2
	SM_Ofs0Low LABEL DWORD
	add ebx,012345678h
	;
	SM_Ofs0High LABEL DWORD
	adc esi,012345678h
	;xxx
	;
	sar edi,18									; Now have lighting increment
	and eax,esi
	;
	jmp	@CatStr(TMI_ProcInner32P,Mip,Line,Trilerp)
	;
	; Inner loop:
	;
	ALIGN 16
	TexInnerLoop:
		;
		mov ecx,[DWORD PTR TexStuff]
		mov esp,[DWORD PTR TexStuff+4]
		;
		mov [ebp*4-16],ecx
		mov [ebp*4-12],esp
		;
		mov ecx,[DWORD PTR TexStuff+8]
		mov esp,[DWORD PTR TexStuff+12]
		;
		mov [ebp*4-8 ],ecx
		mov [ebp*4-4 ],esp
		;
	@CatStr(TMI_ProcInner32P,Mip,Line,Trilerp):
		;
		IF EvenMip
			shr al,1
		ENDIF
		and edi,00000ffffh
		;
		SM_UBits1 LABEL BYTE
		rol eax,7
		mov	ecx,DWORD PTR [TMI_DTex]			; Get tex inc masked with 0ffff0000h
		;
		; Do 4-pixel shading & Pixel 1:
		;
		or  edi,ecx
		mov esp,DWORD PTR [TMI_DTex+4]
		;
		SM_Dest1 LABEL DWORD
		mov dl,[eax+012345678h]
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs1Low LABEL DWORD
		add ebx,012345678h
		;
		SM_Ofs1High LABEL DWORD
		adc esi,012345678h
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		;
		; -- Pixel 1 --
		;
		and eax,esi
		mov dh,bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits2 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs2Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs2High LABEL DWORD
		sbb esi,012345678h
		mov ecx,[edx*4]
		;
		; -- Pixel 2 --
		;
		SM_Dest2 LABEL DWORD
		mov dl,[eax+012345678h]
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		;
		and eax,esi
		mov dh,bh
		;
		IF EvenMip
			shr al,1
		ENDIF
		;
		SM_UBits3 LABEL BYTE
		rol eax,7
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs3Low LABEL DWORD
		add ebx,012345678h
		;
		mov DWORD PTR [TexStuff+0],ecx
		mov ecx,[edx*4]
		;
		SM_Ofs3High LABEL DWORD
		adc esi,012345678h
		mov DWORD PTR [TexStuff+4],ecx
		;
		add ebp,4
		;
		; -- Pixel 3 --
		;
		SM_Dest3 LABEL DWORD
		mov dl,[eax+012345678h]
		mov eax,[@catstr(TMI_,TexBase1,Mip) + OddMip*4]
		;
		and eax,esi
		mov dh,bh
		;
		IF OddMip
			shr al,1
		ENDIF
		;
		SM_UBits4 LABEL BYTE
		rol eax,7
		mov ecx,[edx*4]
		;
		mov DWORD PTR [TexStuff+8],ecx
		SM_Dest4 LABEL DWORD
		mov dl,[eax+012345678h]
		;
		; -- Pixel 4 --
		;
		mov eax,[TMI_DiffLight]					; Pointer difference from [DestBase] to [LightBase]
		add ebx,edi
		;
		adc esi,esp
		SM_Ofs4Low LABEL DWORD
		sub ebx,012345678h
		;
		SM_Ofs4High LABEL DWORD
		sbb esi,012345678h
		mov edi,[eax+ebp+4]
		;
		SM_AdjustLow1 LABEL DWORD
		add edi,012345678h
		mov dh,bh
		;
		mov eax,[@catstr(TMI_,TexBase1,Mip) + EvenMip*4]
		sub edi,ebx								; Take difference
		;
		shl edi,16								; Pad so that a sar edi,18 will be like sar bx,2
		mov ecx,[edx*4]
		;
		and eax,esi
		mov DWORD PTR [TexStuff+12],ecx
		;
		sar edi,18								; Now have lighting increment
		mov esp,[TMI_FinalDest]					; Get end pointer
		;
		cmp ebp,esp								; More to do?
		jl  TexInnerLoop						; Loop
		;
		; Done:
		;
		mov	esp,[SavedESP]
	ret
	;
	; Per-polygon self modifier macro routine for this loop:
	;
	@CatStr(TMI_Mod32PMacro,Mip,Line,Trilerp) MACRO
		;
		; Assembly proc to set up all self-modifying code offsets
		; for one mipmap of a texture map.  Call successively with
		; the appropriate info for each mipmap.  Call with:
		;
		; eax = this dither table pointer
		; ebx = this texture address mask
		; edx = mip texture address mask
		; ecx = U Bits (cl=this, ch=mip)
		; esi = texture base address
		; edi = mip base address
		;
		IF EvenMip
			mov [SM_Dest1+2],edi
			mov [SM_Dest3+2],edi
			;
			mov [SM_UBits1+2],ch
			mov [SM_UBits3+2],ch
		ELSE
			mov [SM_Dest1+2],esi
			mov [SM_Dest3+2],esi
			;
			mov [SM_UBits1+2],cl
			mov [SM_UBits3+2],cl
		ENDIF
		;
		; Pixel 1 & 3 setup:
		;
		IF OddMip
			mov [SM_Dest2+2],edi
			mov [SM_Dest4+2],edi
			;
			mov [SM_UBits2+2],ch
			mov [SM_UBits4+2],ch
		ELSE
			mov [SM_Dest2+2],esi
			mov [SM_Dest4+2],esi
			;
			mov [SM_UBits2+2],cl
			mov [SM_UBits4+2],cl
		ENDIF
		;
		mov ebx,[eax]			; Dither delta low
		mov edx,[eax+4]			; Dither delta high
		;
		mov [SM_Ofs1Low+2],ebx
		mov [SM_Ofs2Low+2],ebx
		mov [SM_Ofs3Low+2],ebx
		mov [SM_Ofs4Low+2],ebx
		;
		mov [SM_Ofs1High+2],edx
		mov [SM_Ofs2High+2],edx
		mov [SM_Ofs3High+2],edx
		mov [SM_Ofs4High+2],edx
		;
		mov ebx,[eax+8]			; Dither offset low
		mov edx,[eax+12]		; Dither offset high
		;
		mov [SM_Ofs0Low+2],ebx
		mov [SM_AdjustLow1+2],ebx
		mov [SM_Ofs0High+2],edx
	ENDM
ENDM

;--------------------------------------------------------------------------------
; Texture data tables
;--------------------------------------------------------------------------------

.DATA
ALIGN 16
PUBLIC TMI_MipDataTable
TMI_MipDataTable LABEL DWORD
FOR Mip,<0,1,2,3,4,5,6,7>
	@catstr(TMI_,TexBase1,Mip) DD ?
	@catstr(TMI_,TexBase2,Mip) DD ?
ENDM

;--------------------------------------------------------------------------------
; Texture routine generators
;--------------------------------------------------------------------------------

;-------------------------------------;
; Generate texture mapper inner loops ;
;-------------------------------------;
SELF_MOD_SEG
FOR Variety,<8P,16P,32P>
	FOR Mip,<0,1,2,3,4,5,6,7>
		FOR Trilerp,<0,1,2,3>
			FOR Line,<0,1,2,3>
				@catstr(TMI_Define,Variety) Mip,Line,(Line MOD 2),Trilerp
			ENDM
		ENDM
	ENDM
ENDM
END_SELF_MOD_SEG

;---------------------------------------;
; Generate all texture mapper modifiers ;
;---------------------------------------;
.CODE
ALIGN 16
FOR Variety,<8P,16P,32P>
	FOR Mip,<0,1,2,3,4,5,6,7>
		FOR Line,<0,1,2,3>
			;
			@CatStr(TMI_Mod,Variety,Mip,Line):
			;
			mov [@catstr(TMI_,TexBase1,Mip)],ebx
			mov [@catstr(TMI_,TexBase2,Mip)],edx
			@CatStr(TMI_Mod,Variety,Macro,Mip,Line,0)
			@CatStr(TMI_Mod,Variety,Macro,Mip,Line,1)
			@CatStr(TMI_Mod,Variety,Macro,Mip,Line,2)
			@CatStr(TMI_Mod,Variety,Macro,Mip,Line,3)
			ret
			;
		ENDM
	ENDM
ENDM

;--------------------------------------------------------------------------------
; Tables
;--------------------------------------------------------------------------------

;-----------------------------------;
; Table of all texture mapper loops ;
;-----------------------------------;
;
.DATA
ALIGN 16
FOR Variety,<8P,16P,32P>
	PUBLIC @catstr(TMI_ProcTable,Variety)
	@catstr(TMI_ProcTable,Variety) DD 0,0
	FOR Mip, <0,1,2,3,4,5,6,7>
		FOR Trilerp,<0,1,2,3>
			FOR Line,<0,1,2,3>
				DD @CatStr(TMI_Proc,Variety,Mip,Line,Trilerp)
				DD @CatStr(TMI_ProcInner,Variety,Mip,Line,Trilerp)
			ENDM
		ENDM
	ENDM
ENDM

;---------------------------------------;
; Table of all texture mapper modifiers ;
;---------------------------------------;
.DATA
ALIGN 16
FOR Variety,<8P,16P,32P>
	PUBLIC @catstr(TMI_ModTable,Variety)
	@catstr(TMI_ModTable,Variety) DD 0
	FOR Mip, <0,1,2,3,4,5,6,7>
		FOR Line,<0,1,2,3>
			DD @CatStr(TMI_Mod,Variety,Mip,Line)
		ENDM
	ENDM
ENDM

;--------------------------------------------------------------------------------
; Texture rectangle setup loop
;--------------------------------------------------------------------------------

;------------------------;
; Difference generator   ;
; Uses esi, edi, and edx ;
;------------------------;
TRL_DiffGen MACRO var:req
	;
	latbase TEXTEQU @catstr(FTexLattice.T,var)
	;
	mov esi,[eax].latbase						; - esi=U1
	mov edi,[eax].latbase + SIZE FTexLattice	;   edi=U2
	;
	sub edi,esi									; - edi=U2-U1
	mov edx,[ebx].latbase + SIZE FTexLattice	;   edx=U4
	;
	mov [@catstr(D_,var,X)],edi					; - Save U2-U1
	sub edx,edi									;   edx=U4+U1-U2
	;
	mov edi,[ebx].latbase						; - edi=U3
	mov [@catstr(D_,var)],esi					;   Save U1
	;
	sub edx,edi									; - edx=U4+U1-U2-U3
	sub edi,esi									;   edi=U3-U1
	;
	mov [@catstr(D_,var,XY)],edx				; - Save U1-U2-U3+U4
	mov [@catstr(D_,var,Y)],edi					;   Save U3-U1
	;
ENDM

;------------------------;
; Shifter and offsetter  ;
; Uses esi,edi, edx, ebp ;
;------------------------;
TRL_ShiftGen MACRO type:req, var:req, ofs:req, shift:req, shiftlabel:req
	mov esi,[@catstr(D_,var)]
	mov ebp,ofs
	;
	mov edi,[@catstr(D_,var,X)]
	add esi,ebp
	;
	@catstr(shiftlabel,1) LABEL BYTE
	sar esi,shift
	mov edx,[@catstr(D_,var,Y)]
	;
	@catstr(shiftlabel,2) LABEL BYTE
	sar edi,shift
	mov ebp,[@catstr(D_,var,XY)]
	;
	@catstr(shiftlabel,3) LABEL BYTE
	sar edx,shift
	mov [@catstr(type,_,var)],esi
	;
	@catstr(shiftlabel,4) LABEL BYTE
	sar ebp,shift
	mov [@catstr(type,_,var,X)],edi
	;
	mov [@catstr(type,_,var,Y)],edx
	mov [@catstr(type,_,var,XY)],ebp
ENDM

;---------;
; Equates ;
;---------;

MAX_XR TEXTEQU <256>

;--------;
; Locals ;
;--------;
.DATA

T_U		DD ?
T_UX	DD ?
T_UY	DD ?
T_UXY	DD ?
T_V		DD ?
T_VX	DD ?
T_VY	DD ?
T_VXY	DD ?

L_U		DD ?
L_UX	DD ?
L_UY	DD ?
L_UXY	DD ?
L_V		DD ?
L_VX	DD ?
L_VY	DD ?
L_VXY	DD ?

D_U		DD ?
D_UX	DD ?
D_UY	DD ?
D_UXY	DD ?
D_V		DD ?
D_VX	DD ?
D_VY	DD ?
D_VXY	DD ?
D_G		DD ?
D_GX	DD ?
D_GY	DD ?
D_GXY	DD ?

Num		DD ?
LatticePtr DD ?
MipLevel DD ?
UBits	DD ?
VMask	DD ?

;---------;
; Globals ;
;---------;
.DATA

DD_Global	TRL_MipTable
DD_Global	TRL_TexBaseU
DD_Global	TRL_TexBaseV
DD_Global	TRL_LightBaseU
DD_Global	TRL_LightBaseV
DD_Global	TRL_LightVMask
DD_Global	TRL_RoutineOfsEffectBase
DD_Global	TRL_MipRef
DD_Global	TRL_BaseU
DD_Global	TRL_BaseV

Public TRL_MipPtr
TRL_MipPtr	 DD ?,?,?,?,?,?,?,?,?

DB_Global	TRL_LightMeshShift
DB_Global	TRL_LightMeshUShift

;----------------------;
; Rectangle setup loop ;
;----------------------;
SELF_MOD_SEG
	;
	ALIGN 16
	;
	;---------------------;
	; Compute rect deltas ;
	;---------------------;
	;
	; Call with:
	; edi = Lattice base pointer
	; esi = Starting rect x value
	; ebp = Ending rect x value
	;
	PUBLIC TRL_RectLoop
	TRL_RectLoop LABEL DWORD
		;
		lea edi,[edi + esi*4]	; Skip in
		sub ebp,esi				; ebp = length
		;
		;-----------------;
		; Rect inner loop ;
		;-----------------;
		;
		RectInner:
		;
		mov eax,[edi]						; Get top lattice pointer
		mov [LatticePtr],edi				; Save lattice pointer
		;
		mov [Num],ebp						; Save counter
		mov ebx,[TRL_MipTable]				; Get mipmap table base
		;
		;-----------------;
		; Get mipmap info ;
		;-----------------;
		;
		mov esi,[eax].FTexLattice.LocD		; Load mipmap Z value
		;
		shr esi,21							; Get mipmap magnitude + 2 bits of fractional mantissa
		xor edx,edx							; Prepare to compute trilerp
		;
		mov ebp,[TRL_MipRef]				; Get GBlit pointer
		xor ecx,ecx							; Prepare to compute mip
		;
		mov dl,[ebx+esi*2+1]				; Now edx = RoutineOfs
		mov cl,[ebx+esi*2]					; Now ecx = MipLevel
		;
		mov ebx,1							; ebx=1
		mov [MipLevel],ecx					; Save mip level
		;
		mov [ebp+ecx+1],bl					; Tag next mipmap as referenced so that it will be set up
		mov esi,[TRL_RoutineOfsEffectBase]	; Get effect offset
		;
		mov [ebp+ecx],bl					; Tag current mipmap as referenced so that it will be set up
		add edx,esi
		;
		mov esi,TRL_MipPtr[ecx*4]			; Now esi = mipmap info pointer
		mov ebx,[edi + 4*MAX_XR]			; Get next base pointer
		;
		mov [eax].FTexLattice.RoutineOfs,edx ; Store RoutineOfs
		xor edx,edx							; Prepare to load UBits
		;
		mov dl, [esi].FBlitMipInfo.UBits	; U bits in mipmap
		mov edi,[esi].FBlitMipInfo.VMask	; Get texture V mask
		;
		mov [UBits],edx						; Store U bits
		mov [VMask],edi						; Remember V mask
		;
		;----------------------;
		; Generate differences ;
		;----------------------;
		;
		TRL_DiffGen U
		TRL_DiffGen V
		TRL_DiffGen G
		;
		;---------------------------;
		; Shift texture differences ;
		;---------------------------;
		;
		mov ecx,[MipLevel]
		TRL_ShiftGen T,V,[TRL_TexBaseV],cl,SM_UnusedA
		add ecx,[UBits]
		TRL_ShiftGen T,U,[TRL_TexBaseU],cl,SM_UnusedB
		;
		;-------------------------;
		; Shift light differences ;
		;-------------------------;
		;
		TRL_ShiftGen L,V,[TRL_LightBaseV],7,SM_LightMeshShift
		TRL_ShiftGen L,U,[TRL_LightBaseU],7,SM_LightMeshUShift
		;
		;-----------------------;
		; Generate light deltas ;
		;-----------------------;
		;
		mov ebp,0ffffffffh
		mov esi,[TRL_LightVMask]
		xor ebp,esi
		;
		; L:
		;
		mov ebx,[L_V]
		mov edi,[D_G]
		;
		shl ebx,16
		and edi,00000ffffh
		;
		or  edi,ebx
		mov ebx,[L_U]
		;
		mov [eax].FTexLatticeD.SubL,edi
		mov edi,[L_V]
		;
		; H:
		;
		sar edi,16
		;
		shl ebx,16
		and edi,esi
		;
		and ebx,ebp
		;
		or  edi,ebx
		mov ebx,[L_VY]
		;
		mov [eax].FTexLatticeD.SubH,edi
		mov edi,[D_GY]
		;
		; LY:
		;
		SubY1 LABEL BYTE
		sar edi,7
		;
		SubY2 LABEL BYTE
		shl ebx,7
		and edi,00000ffffh
		;
		and ebx,0ffff0000h
		;
		or  edi,ebx
		mov ebx,[L_UY]
		;
		mov [eax].FTexLatticeD.SubLY,edi
		mov edi,[L_VY]
		;
		; HY:
		;
		SubY3 LABEL BYTE
		sar edi,7
		;
		SubY4 LABEL BYTE
		shl ebx,7
		and edi,esi
		;
		and ebx,ebp
		;
		or edi,ebx
		mov ebx,[L_VX]
		;
		mov [eax].FTexLatticeD.SubHY,edi
		mov edi,[D_GX]
		;
		; LX:
		;
		SubX1 LABEL BYTE
		sar edi,7
		;
		SubX2 LABEL BYTE
		shl ebx,7
		and edi,00000ffffh
		;
		and ebx,0ffff0000h
		;
		or  edi,ebx
		mov ebx,[L_UX]
		;
		mov [eax].FTexLatticeD.SubLX,edi
		mov edi,[L_VX]
		;
		; HX:
		;
		SubX3 LABEL BYTE
		sar edi,7
		;
		SubX4 LABEL BYTE
		shl ebx,7
		and edi,esi
		;
		and ebx,ebp
		;
		or edi,ebx
		mov ebx,[L_VXY]
		;
		mov [eax].FTexLatticeD.SubHX,edi
		mov edi,[D_GXY]
		;
		; LXY:
		;
		SubXY1 LABEL BYTE
		sar edi,7
		;
		SubXY2 LABEL BYTE
		shl ebx,7
		and edi,00000ffffh
		;
		and ebx,0ffff0000h
		;
		or  edi,ebx
		mov ebx,[L_UXY]
		;
		mov [eax].FTexLatticeD.SubLXY,edi
		mov edi,[L_VXY]
		;
		; HXY:
		;
		SubXY3 LABEL BYTE
		sar edi,7
		;
		SubXY4 LABEL BYTE
		shl ebx,7
		and edi,esi
		;
		and ebx,ebp
		mov ebp,0ffffffffh
		;
		or  edi,ebx
		mov esi,[VMask]
		;
		mov [eax].FTexLatticeD.SubHXY,edi
		xor ebp,esi
		;
		;-------------------------;
		; Generate texture deltas ;
		;-------------------------;
		;
		; L:
		;
		mov ebx,[T_V]
		;
		shl ebx,16
		mov edi,[T_V]
		;
		sar edi,16
		mov [eax].FTexLatticeD.LatL,ebx
		;
		; H:
		;
		mov ebx,[T_U]
		and edi,esi
		;
		shl ebx,16
		;
		and ebx,ebp
		;
		or  edi,ebx
		mov ebx,[T_VY]
		;
		LatY1 LABEL BYTE
		shl ebx,7
		mov [eax].FTexLatticeD.LatH,edi
		;
		; LY:
		;
		and ebx,0ffff0000h
		;
		mov [eax].FTexLatticeD.LatLY,ebx
		mov edi,[T_VY]
		;
		; HY:
		;
		LatY2 LABEL BYTE
		sar edi,7
		mov ebx,[T_UY]
		;
		and edi,esi
		;
		LatY3 LABEL BYTE
		shl ebx,7
		;
		and ebx,ebp
		;
		or edi,ebx
		mov ebx,[T_VX]
		;
		LatX1 LABEL BYTE
		shl ebx,7
		mov [eax].FTexLatticeD.LatHY,edi
		;
		; LX:
		;
		and ebx,0ffff0000h
		;
		mov edi,[T_VX]
		mov [eax].FTexLatticeD.LatLX,ebx
		;
		; HX:
		;
		LatX2 LABEL BYTE
		sar edi,7
		mov ebx,[T_UX]
		;
		LatX3 LABEL BYTE
		shl ebx,7
		and edi,esi
		;
		and ebx,ebp
		;
		or edi,ebx
		mov ebx,[T_VXY]
		;
		LatXY1 LABEL BYTE
		shl ebx,7
		mov [eax].FTexLatticeD.LatHX,edi
		;
		; LXY:
		;
		and ebx,0ffff0000h
		;
		mov edi,[T_VXY]
		mov [eax].FTexLatticeD.LatLXY,ebx
		;
		; HXY:
		;
		LatXY2 LABEL BYTE
		sar edi,7
		mov ebx,[T_UXY]
		;
		LatXY3 LABEL BYTE
		shl ebx,7
		and edi,esi
		;
		and ebx,ebp
		mov ebp,[Num]
		;
		or  edi,ebx
		;
		mov [eax].FTexLatticeD.LatHXY,edi
		mov edi,[LatticePtr]
		;
		;-----------;
		; Next rect ;
		;-----------;
		;
		add edi,4
		;
		dec ebp
		jg  RectInner
	ret
	;
	;---------------------------;
	; Self-modifying code setup ;
	;---------------------------;
	;
	; Call with:
	;
	; al = GBlit.LatticeXBits
	; bl = GBlit.LatticeYBits
	; cl = GBlit.SubXBits
	; dl = GBlit.SubYBits
	;
	TRL_SelfModRect PROC
		;
		; Sublattice:
		;
		mov [SubX1+2],cl	; cl = SubXBits
		add cl,16			; cl = 16+SubXBits
		mov [SubX3+2],cl
		neg cl				; cl = -16-SubXBits
		add cl,32			; cl = 16-SubXBits
		mov [SubX2+2],cl
		mov [SubX4+2],cl
		;
		mov [SubY1+2],dl	; dl = SubYBits
		add dl,16			; dl = 16+SubYBits
		mov [SubY3+2],dl
		neg dl				; dl = -16-SubYBits
		add dl,32			; dl = 16-SubYBits
		mov [SubY2+2],dl
		mov [SubY4+2],dl
		;
		add cl,dl			; cl = 32-SubXBits-SubYBits
		sub cl,16			; cl = 16-SubXBits-SubYBits
		mov [SubXY2+2],cl
		mov [SubXY4+2],cl
		neg cl				; cl = -16+SubXBits+SubYBits
		sub cl,16			; cl = SubXBits+SubYBits
		mov [SubXY1+2],cl
		add cl,16			; cl = 16+SubXBits+SubYBits
		mov [SubXY3+2],cl
		;
		; Lattice:
		;
		add al,16			; al = 16+LatticeXBits
		mov [LatX2+2],al
		neg al				; al = -16-LatticeXBits
		add al,32			; al = 16-LatticeXBits
		mov [LatX1+2],al
		mov [LatX3+2],al
		;
		add bl,16			; bl = 16+LatticeYBits
		mov [LatY2+2],bl
		neg bl				; bl = -16-LatticeYBits
		add bl,32			; bl = 16-LatticeYBits
		mov [LatY1+2],bl
		mov [LatY3+2],bl
		;
		add al,bl			; al = 32-LatticeXBits-LatticeYBits
		sub al,16			; al = 16-LatticeXBits-LatticeYBits
		mov [LatXY1+2],al
		mov [LatXY3+2],al
		neg al				; al = -16+LatticeXBits+LatticeYBits
		mov [LatXY2+2],al
		;
		; Light mesh shifting:
		;
		mov al,[TRL_LightMeshShift]
		mov bl,[TRL_LightMeshUShift]
		;
		mov [SM_LightMeshShift1+2],al
		mov [SM_LightMeshShift2+2],al
		mov [SM_LightMeshShift3+2],al
		mov [SM_LightMeshShift4+2],al
		;
		mov [SM_LightMeshUShift1+2],bl
		mov [SM_LightMeshUShift2+2],bl
		mov [SM_LightMeshUShift3+2],bl
		mov [SM_LightMeshUShift4+2],bl
		;
		ret
	TRL_SelfModRect ENDP
END_SELF_MOD_SEG

;--------------------------------------------------------------------------------
; Texture lighting mid loop
;--------------------------------------------------------------------------------

DD_GLOBAL TLM_GBlitInterX
DB_GLOBAL TLM_GBlitInterXBits2

PUBLIC TLM_8P_Unlit
TLM_8P_Unlit:
	mov edi,[TLI_Dest]
	mov esi,[TLI_DestEnd]
	mov eax,01800h
	;
	UnlitFillLoop:
	mov [edi   ],eax
	mov [edi+4 ],eax
	mov [edi+8 ],eax
	mov [edi+12],eax
	add edi,16
	cmp edi,esi
	jl  UnlitFillLoop
ret

PUBLIC TLM_8P_Lit
TLM_8P_Lit:
	pushad
	;
	mov edx,[TLI_TopLattice]				; edx = successful lattice pointer
	mov ebx,[TLI_SkipIn]					; ebx = updated skip-in value
	;
	; Make T a valid rect:
	;
	test edx,edx							; Is LatticeBase[0] null?
	jz Bad1									; Yes, must find better lattice
	mov ecx,[edx].FTexLatticeD.RoutineOfs	; Get routine offset
	test ecx,ecx							; Is routine offset 0?
	jnz LatticeOk							; No, so lattice is ok - branch usually taken!
	;
	Bad1:									; LatticeBase[0] is null, must get eax = LatticeBase
	mov eax,[TLI_Dest]						; eax = TLI_Dest
	mov ecx,[TLO_BotBase]					; ecx = TLO_BotBase
	sub eax,ecx								; eax = TLI_Dest - TLO_BotBase
	mov cl,TLM_GBlitInterXBits2				; cl  = GBlit.InterXBits + 2
	shr eax,cl								; eax = (TLI_Dest - TLO_BotBase) >> GBlit.InterXBits
	mov ecx,[TLO_LatticeBase]				; ecx = TLO_LatticeBase
	;
	mov edx,[ecx + eax*4 - 4]				; T = LatticeBase[-1]
	mov ebx,TLM_GBlitInterX					; SkipIn = GBlit.InterX
	;
	test edx,edx							; Is LatticeBase[-1] null?
	jz Bad2									; Yes, must update
	mov edi,[edx].FTexLatticeD.RoutineOfs	; Get routine offset
	test edi,edi							; Is routine offset 0?
	jnz LatticeOk							; No, so lattice is ok
	;
	Bad2:									; LatticeBase[-1] is null, so check LatticeBase[-MAX_XR]
	mov edx,[ecx + eax*4 - MAX_XR*4]		; T = LatticeBase[-MAX_XR]
	xor ebx,ebx								; SkipIn = 0
	test edx,edx							; Is LatticeBase[-MAX_XR] null?
	jz Bad3									; Yes, must update
	mov edi,[edx].FTexLatticeD.RoutineOfs	; Get routine offset
	test edi,edi							; Is routine offset 0?
	jnz LatticeOk							; No, so lattice is ok
	;
	Bad3:
	mov edx,[ecx + eax*4 - MAX_XR*4 - 4]	; T = LatticeBase[-MAX_XR-1]
	mov ebx,TLM_GBlitInterX					; SkipIn = GBlit.InterX
	;
	ALIGN 16
	LatticeOk:
	;
	; Call inner loop with optional skip-in:
	;
	shl		ebx,2						; ebx = SkipIn * 4
	mov		ecx,[TLI_ProcBase]			; Snag skip-in value
	;
	mov		edi,[TLI_Dest]				; Get destrination
	add		ecx,ebx						; ecx = address of function pointer
	;
	mov		ebx,[edx].FTexLatticeD.SubL	; Get low increment
	mov		esi,[edx].FTexLatticeD.SubH	; Get high increment
	;
	mov		ebp,[edx].FTexLatticeD.SubLX ; Only needed when skipping in
	mov		eax,[edx].FTexLatticeD.SubHX ; Only needed when skipping in
	;
	call	DWORD PTR [ecx] ;jmp
	popad
ret

;--------------------------------------------------------------------------------
; SubRect outer loop macro
;--------------------------------------------------------------------------------

.DATA
DD_GLOBAL	TRO_Y
DD_GLOBAL	TRO_ThisLightBase
DD_GLOBAL	TRO_SubRectEndY
DD_GLOBAL	TRO_SpanIndex
DD_GLOBAL	TRO_OuterProc
SpanStart	DD ?
SpanEnd		DD ?
SpanNext	DD ?

;
; Call with:
;    eax = TRO_Y
;
.CODE
TRO_OuterMacro MACRO Variety:req
	@catstr(TRO_Outer,Variety) PROC
	;
	SkipInMacro				TEXTEQU @catstr(TMI_,Variety,_SkipIn)
	SkipInAdjacentMacro		TEXTEQU @catstr(TMI_,Variety,_SkipInAdjacent)
	NoSkipInMacro			TEXTEQU @catstr(TMI_,Variety,_NoSkipIn)
	WriteFinalMacro			TEXTEQU @catstr(TMI_,Variety,_WriteFinal)
	WriteFinalClippedMacro	TEXTEQU @catstr(TMI_,Variety,_WriteFinalClipped)
	;
	; =======================================
	; Loop through all lines in this subrect:
	; =======================================
	;
	SubYLoop:
	;
	mov		ebx,3						; ebx = 3
	mov		esi,[TMI_ProcBase]			; esi = TMI_ProcBase
	;
	and		ebx,eax						; ebx = Y & 3
	inc		eax							; eax = Y + 1
	;
	mov		ecx,[TRO_ThisLightBase]		; ecx = ThisLightBase
	mov		[TRO_Y],eax					; Store Y+1
	;
	lea		edx,[esi + ebx*8]			; edx = &TMI_ProcBase[(Y&3)*2]
	mov		esi,[TMO_DestOrDestBitsPtr] ; esi = TMO_DestOrDestBitsPtr
	;
	mov		[TMI_LineProcBase],edx		; TMI_LineProcBase = &TMI_ProcBase[(Y&3)*2]
	mov		edx,[TMO_Dest]				; edx = TMO_Dest
	;
	mov		[esi],edx					; *TMO_DestOrDestBitsPtr = TMO_Dest
	mov		esi,[TMO_DestBits]			; esi = TMI_DestBits
	;
	mov		eax,[ecx]					; eax = *ThisLightBase
	sub		edx,esi						; edx = TMO_Dest - TMO_DestBits
	;
	mov		edi,[TRO_SpanIndex]			; edi = SpanIndex
	sub		eax,esi						; eax = *ThisLightBase - TMO_DestBits
	;
	mov		[TMI_DiffDest],edx			; TMI_DiffDest	= (int)TMO_Dest - (int)TMO_DestBits;
	mov		esi,[edi]					; edx = *SpanIndex
	;
	add		edi,4						; edi = SpanIndex+1
	mov		[TMI_DiffLight],eax			; TMI_DiffLight	= (int)*ThisLightBase - (int)TMO_DestBits;
	;
	mov		[TRO_SpanIndex],edi			; SpanIndex = SpanIndex+1
	;
	; ==============
	; Draw all spans
	; ==============
	;
	test esi,esi						; Are any spans active?
	jz   UpdateRects					; No, finished with spans
	;
	; Outer span loop:
	;
	SpanLoop:
	mov  eax,[esi].FSpan.SpanStart		; eax = SpanStart
	mov  ebx,[esi].FSpan.SpanEnd		; ebx = SpanEnd
	;
	mov  edx,[esi].FSpan.SpanNext		; edx = SpanNext
	mov  [SpanEnd],ebx					; Save end
	;
	mov  cl,[GBlit].LatticeXBits		; cl = GBlit.LatticeXBits
	dec  ebx							; ebx = SpanEnd-1
	;
	mov  ebp,eax						; ebp = Span start
	mov  [SpanStart],eax				; Save start
	;
	xor  ebx,eax						; ebx = SpanStart ^ (SpanEnd-1)
	mov  esi,[TMI_LatticeBase]			; esi = TMI_LatticeBase
	;
	shr  eax,cl							; eax = SpanStart >> GBlit.LatticeXBits
	mov  [TMO_Span],edx					; Save next span
	;
	mov  ecx,[GBlit].LatticeXNotMask	; ebx = GBlit.LatticeXNotMask
	mov  edi,[esi + eax*4]				; edi = TMI_LatticeBase[SpanStart >> GBlit.LatticeXBits]
	;
	mov  [TMI_TopLattice],edi			; TMI_TopLattice = TMI_LatticeBase[SpanStart >> GBlit.LatticeXBits]
	;
	test ebx,ecx						; Are we drawing multiple rects?
	jz DrawSingleRect					; No, just draw a single rect
	;
	; ===================
	; Draw multiple rects
	; ===================
	;
	; Draw first, left-clipped rectspan:
	;
	mov  ebx,[GBlit].LatticeXNotMask		; ebx = GBlit.LatticeXNotMask
	mov  eax,ebp							; eax = SpanStart
	;
	and  eax,ebx							; eax = SpanStart & GBlit.LatticeXNotMask
	mov  ebx,[TMO_DestBits]					; ebx = TMO_DestBits
	;
	add  eax,ebx							; eax = (SpanStart & GBlit.LatticeXNotMask) + TMO_DestBits
	mov  ebx,[GBlit].LatticeX				; ebx = GBlit.LatticeX
	;
	add  eax,ebx							; eax = (SpanStart & GBlit.LatticeXNotMask) + TMO_DestBits + GBlit.LatticeX
	mov  edi,[TMI_TopLattice]				; Get top lattice pointer
	;
	mov  [TMI_FinalDest],eax				; Store TMI_FinalDest
	mov  eax,size FTexLattice				; Get lattice size
	;
	add  eax,edi							; Get next lattice address
	mov  ecx,[SpanStart]					; Get starting value
	;
	mov  [TMI_TopLattice],eax				; Store next lattice address
	mov  eax,[TMO_DestBits]					; Prepare to compute dest
	;
	mov  ebp,ecx							; Prepare to compute dest
	mov  ebx,[TMI_LineProcBase]				; Now ebx = proc table base
	;
	add  ebp,eax							; Now ebp = dest
	mov  esi,[GBlit].LatticeXMask4			; Get skip-in mask
	;
	mov  eax,[edi].FTexLattice.RoutineOfs	; Now eax = Routine offset
	and  ecx,esi							; Now ebx = skip-in value
	;
	SkipInMacro 1, WriteFinalMacro
	;
	; Draw middle, unclipped rects:
	;
	; Center rect span setup:
	;
	mov  eax,[TMO_DestBits]					; eax = TMO_DestBits
	mov  edx,[SpanEnd]						; edx = SpanEnd
	;
	add  eax,edx							; eax = TMO_DestBits + SpanEnd
	mov  edx,[GBlit].LatticeX				; edx = GBlit.LatticeX
	;
	sub  eax,edx							; eax = TMO_DestBits + SpanEnd - GBlit.LatticeX
	mov  edi,[TMI_TopLattice]				; Get top lattice pointer
	;
	cmp  ebp,eax							; Any center rects to draw?
	jge  SkipCenter							; No, skip
	;
	mov  [TMI_RectFinalDest],eax			; Save final dest used in loop compare
	;
	; Center rect span loop:
	;
	CenterRectSpanLoop:
		;
		mov  ecx,[TMI_LineProcBase]				; Now ecx = proc table base
		mov  esi,[GBlit].LatticeX				; Get dest increment
		;
		mov  eax,[edi].FTexLattice.RoutineOfs	; Now eax = Routine offset
		add	 esi,ebp							; Now esi = final dest
		;
		mov [TMI_FinalDest],esi					; Store final dest
		mov [TMI_TopLattice],edi				; Store lattice pointer
		;
		NoSkipInMacro 2, WriteFinalMacro
		;
		; Next center rect span:
		;
		mov  edi,[TMI_TopLattice]				; Get top lattice pointer
		mov  eax,[TMI_RectFinalDest]			; Get end
		;
		add  edi,SIZE FTexLattice				; Go to next lattice
		;
		cmp  ebp,eax							; Done yet?
	jl   CenterRectSpanLoop
	;
	mov [TMI_TopLattice],edi
	;
	SkipCenter:
	;
	; Draw last, right-clipped rectspan:
	;
	mov edi,[TMI_TopLattice]				; Get top lattice pointer
	mov ecx,[TMI_LineProcBase]				; Now ecx = proc table base
	;
	mov edx,[TMO_DestBits]					; TMI_FinalDest = TMO_DestBits + SpanEnd + 3
	mov ebx,[SpanEnd]						; Get end
	;
	mov eax,[edi].FTexLattice.RoutineOfs	; Now eax = Routine offset
	add ebx,edx								; Now ebx = updated TMI_FinalDest
	;
	mov [TMI_FinalDest],ebx					; Save updated FinalDest
	;
	NoSkipInMacro 3, WriteFinalClippedMacro
	;
	; ==================
	; Draw a single rect
	; ==================
	;
	; Called with: ebx = SpanStart ^ (SpanEnd-1)
	;
	ALIGN 16
	DrawSingleRect:
	;
	mov edi,[TMI_TopLattice]				; Get top lattice pointer
	mov ecx,ebp								; Get span start
	;
	test ebx,0fffffffch						; (SpanStart ^ (SpanEnd-1)) & ~3 -> Are SpanStart and SpanEnd in same cell
	je  DrawAdjacent						; Handle adjacent 4-clipping when needed
	;
	; Single doubly-clipped span, separated
	;
	mov  esi,[TMO_DestBits]					; Calc TMO_DestBits + SpanStart
	mov  edx,[SpanEnd]						; Get adjusted end
	;
	mov  eax,[edi].FTexLattice.RoutineOfs	; Now eax = Routine offset
	add  edx,esi							; Now edx = TMI_FinalDest = TMO_DestBits + SpanEnd
	;
	add  ebp,esi							; Now ebp = dest = TMO_DestBits + SpanStart
	mov  esi,[GBlit].LatticeXMask4			; Get skip-in mask
	;
	mov  ebx,[TMI_LineProcBase]				; Now ebx = proc table base
	mov	 [TMI_FinalDest],edx				; Store final dest
	;
	and	 ecx,esi							; Clip skip-in value to lattice
	;
	SkipInMacro 4, WriteFinalClippedMacro
	;
	; Single doubly-clipped span, adjacent
	;
	DrawAdjacent:
	;
	mov  esi,[TMO_DestBits]					; esi = TMO_DestBits
	;
	add  ebp,esi							; ebp = TMO_DestBits + SpanStart
	mov  eax,[edi].FTexLattice.RoutineOfs	; Now eax = Routine offset
	;
	mov  ebx,[TMI_LineProcBase]				; Now ebx = skip-in proc table base
	mov  esi,[GBlit].LatticeXMask			; Get skip-in mask
	;
	mov  [TMI_FinalDest],ebp				; Store final dest
	and  ecx,esi							; Now ecx = skip-in value
	;
	SkipInAdjacentMacro
	;
	; Next span:
	;
	NextSpan:
	;
	mov esi,[TMO_Span]
	;
	test esi,esi
	jnz SpanLoop
	;
	; ================
	; Update all rects
	; ================
	;
	UpdateRects:
	;
	; esi = RectSpan pointer
	; edi = LatticeBase pointer
	; ebp = Rect counter
	; eax, ebx, ecx, edx = temp
	;
	mov esi,[TMO_RectSpan]		; Get RectSpan pointer
	mov ebx,[TMI_LatticeBase]	; Get lattice base address
	;
	test esi,esi				; Empty RectSpan?
	jz Done						; Yes, exit
	;
	; Rect span loop:
	;
	RectSpanLoop:
	;
	mov eax,[esi].FSpan.SpanStart	; Get start
	mov ebp,[esi].FSpan.SpanEnd		; Get end
	;
	sub ebp,eax						; Now ebp = rect counter
	;
	mov edi,[ebx + eax*4]			; Get lattice pointer
	;
	; Rect loop:
	;
	RectLoop:
		;
		; Update values:
		;
		mov eax,[edi].FTexLatticeD.LatL		; Get Low
		mov ecx,[edi].FTexLatticeD.LatLY	; Get Low x-inc
		;
		mov ebx,[edi].FTexLatticeD.LatH		; Get High
		add eax,ecx							; Update low
		;
		mov edx,[edi].FTexLatticeD.LatHY	; Get High x-inc
		mov [edi].FTexLatticeD.LatL,eax		; Save Low
		;
		adc ebx,edx							; Update high
		mov eax,[edi].FTexLatticeD.LatLX	; Get Low inc
		;
		mov [edi].FTexLatticeD.LatH,ebx		; Save High
		mov ecx,[edi].FTexLatticeD.LatLXY	; Get Low x-inc
		;
		; Update incs:
		;
		mov ebx,[edi].FTexLatticeD.LatHX	; Get High
		add eax,ecx							; Update low
		;
		mov edx,[edi].FTexLatticeD.LatHXY	; Get High x-inc
		mov [edi].FTexLatticeD.LatLX,eax	; Save Low
		;
		adc ebx,edx							; Update high
		;
		mov [edi].FTexLatticeD.LatHX,ebx	; Save High
		add edi,SIZE FTexLattice			; Go to next lattice
		;
		; Next rect:
		;
		dec ebp
	jg RectLoop
	;
	; Next span:
	;
	mov esi,[esi].FSpan.SpanNext	; Get next span pointer
	mov ebx,[TMI_LatticeBase]		; Get lattice base address again
	;
	test esi,esi					; Past last span?
	jnz  RectSpanLoop				; Proceed with next rect span
	;
	Done:
	;
	; Next line in this subrect:
	;
	mov edx,[TMO_Dest]			; edx = TMO_Dest
	mov	ebx,[TRO_ThisLightBase]	; ebx = ThisLightBase
	;
	mov	ecx,[TMO_Stride]		; ecx = TMO_Stride
	add	ebx,4					; ebx = ThisLightBase + 4
	;
	add edx,ecx					; edx = TMO_Dest + TMO_Stride
	mov eax,[TRO_Y]				; eax = Y
	;
	mov [TRO_ThisLightBase],ebx	; ThisLightBase = ThisLightBase + 4
	mov ebx,[TRO_SubRectEndY]	; ebx = SubRectEndY
	;
	mov [TMO_Dest],edx			; TMO_Dest = TMO_Dest + TMO_Stride
	;
	cmp eax,ebx					; Y < SubRectEndY?
	jl  SubYLoop				; Loop
	;
	ret
	;
	@catstr(TRO_Outer,Variety) ENDP
ENDM

;--------------------------------------------------------------------------------
; Mode-specific subrect clipper macro
;--------------------------------------------------------------------------------

;-------------------------;
; General clipper support ;
;-------------------------;

TMI_P_SkipInMath MACRO
	;
	; Perform skip-in math:
	;
	mov		esi,DWORD PTR [ebx+eax*8+4]		; Get entry procedure pointer
	mov		ebx,DWORD PTR [ebx+eax*8]		; Get inner loop procedure pointer
	;
	mov		[SavedInner],esi				; Save procedure pointer
	mov		[SavedProc],ebx					; Save procedure pointer
	;
	mov		esi,[edi].FTexLatticeD.LatH		; High tex
	mov		ebx,[edi].FTexLatticeD.LatL		; Low tex
	;
	mov		eax,[edi].FTexLatticeD.LatHX	; High inc
	;
	mul		ecx								; Multiply by skip-in value
	;
	add		esi,eax							; Update high
	mov		eax,[edi].FTexLatticeD.LatLX	; Get increment low
	;
	mul		ecx								; Multiply by skip-value
	;
	mov		ecx,3							; ecx = destination mask
	add		ebx,eax							; Update low
	;
	adc		esi,edx							; Update high
	and		ecx,ebp							; ecx = destination offset
	;
	mov		[SavedESP],esp					; Save stack
	and		ebp,0fffffffch					; 4-align destination
ENDM

;------------------------;
; 8-bit Pentium clippers ;
;------------------------;

TMI_8P_NoSkipIn MACRO tag:req, writefinalmacro:req
	;
	mov		esi,[edi].FTexLatticeD.LatH	; High tex
	mov		[SavedESP],esp				; Save stack pointer
	;
	mov		ebx,[edi].FTexLatticeD.LatL	; Low tex
	call	DWORD PTR [ecx+eax*8]		; Call span texture mapper
	;
	writefinalmacro						; Write final bytes
ENDM

TMI_8P_PreSkip MACRO
	mov		eax,[TMI_FinalDest]
	;
	mov		[SavedFinalDest],eax
	mov		[TMI_FinalDest],ebp
	;
	call	[SavedProc]					; Call procedure pointer
ENDM

TMI_8P_PostSkip MACRO tag:req, writefinalmacro:req
	mov		ecx,[SavedFinalDest]
	;
	mov		[TMI_FinalDest],ecx
	cmp		ebp,ecx
	;
	jge		@catstr(PostSkip,tag)
	;
	call	[SavedInner]
	;
	writefinalmacro
	;
	jmp		@catstr(PostSkip,tag)
ENDM

TMI_8P_SkipIn MACRO tag:req, writefinalmacro:req
	Local SkipTable,Skip0,Skip1,Skip2,Skip3
	;
	; Perform skip-in math:
	;
	TMI_P_SkipInMath
	;
	jmp		SkipTable[ecx*4]				; Call custom skip function
	;
	; Call tmapper to draw first 4 pixels:
	;
	Skip1:
	TMI_8P_PreSkip
	mov		[ebp-3],ch
	shr		ecx,16
	mov		[ebp-2],cx
	TMI_8P_PostSkip tag, writefinalmacro
	;
	Skip2:
	TMI_8P_PreSkip
	shr		ecx,16
	mov		[ebp-2],cx
	TMI_8P_PostSkip tag, writefinalmacro
	;
	Skip3:
	TMI_8P_PreSkip
	shr		ecx,16
	mov		[ebp-1],ch
	TMI_8P_PostSkip tag, writefinalmacro
	;
	; Call tmapper to draw remainder of line:
	;
	SkipTable LABEL DWORD
	DD Skip0
	DD Skip1
	DD Skip2
	DD Skip3
	;
	Skip0:
	call [SavedProc]
	writefinalmacro
	;
	@catstr(PostSkip,tag):
ENDM

TMI_8P_SkipInAdjacent MACRO
	Local Adj00,Adj01,Adj02,Adj03,Adj10,Adj11,Adj12,Adj13
	Local Adj20,Adj21,Adj22,Adj23,Adj30,Adj31,Adj32,Adj33
	Local Done,AdjacentTable
	;
	TMI_P_SkipInMath						; Perform skip-in math
	;
	mov		[SavedOffset],ecx				; SavedOffset = SpanStart & 3
	call	[SavedProc]						; Call procedure pointer
	;
	mov		eax,[SavedOffset]				; eax = SpanStart & 3
	mov		ebx,[SpanEnd]					; ebx = SpanEnd
	;
	rol		eax,4							; eax = (SpanStart & 3) * 16
	and		ebx,3							; ebx = (SpanEnd & 3)
	;
	mov		edx,ecx							; edx = Pixels
	;
	shr		edx,16							; edx = Shifted pixels
	jmp		AdjacentTable[eax+ebx*4]
	;
	Adj00:
	mov [ebp-4],ecx
	jmp NextSpan
	;
	Adj01:
	mov [ebp-4],cl
	jmp NextSpan
	;
	Adj02:
	mov [ebp-4],cx
	jmp NextSpan
	;
	Adj03:
	mov [ebp-4],cx
	mov [ebp-2],dl
	jmp NextSpan
	;
	Adj10:
	mov [ebp-3],ch
	mov [ebp-2],dx
	jmp NextSpan
	;
	Adj11:
	jmp NextSpan
	;
	Adj12:
	mov [ebp-3],ch
	jmp NextSpan
	;
	Adj13:
	mov [ebp-3],ch
	mov [ebp-2],dl
	jmp NextSpan
	;
	Adj20:
	mov [ebp-2],dx
	jmp NextSpan
	;
	Adj21:
	jmp NextSpan
	;
	Adj22:
	jmp NextSpan
	;
	Adj23:
	mov [ebp-2],dl
	jmp NextSpan
	;
	Adj30:
	mov [ebp-1],dh
	jmp NextSpan
	;
	Adj31:
	jmp NextSpan
	;
	Adj32:
	jmp NextSpan
	;
	Adj33:
	jmp NextSpan
	;
	AdjacentTable LABEL DWORD
	DD	Adj00,Adj01,Adj02,Adj03
	DD	Adj10,Adj11,Adj12,Adj13
	DD	Adj20,Adj21,Adj22,Adj23
	DD	Adj30,Adj31,Adj32,Adj33
ENDM

TMI_8P_WriteFinal MACRO
	mov [ebp-4],ecx
ENDM

TMI_8P_WriteFinalClipped MACRO
	LOCAL EndSkipTable,EndSkip0,EndSkip1,EndSkip2,EndSkip3
	;
	mov eax,[SpanEnd]
	mov edx,ecx
	;
	shr ecx,16
	and eax,3
	;
	jmp EndSkipTable[eax*4]
	;
	EndSkip0:
	mov [ebp-4],edx
	jmp NextSpan
	;
	EndSkip1:
	mov [ebp-4],dl
	jmp NextSpan
	;
	EndSkip2:
	mov [ebp-4],dx
	jmp NextSpan
	;
	EndSkip3:
	mov [ebp-4],dx
	mov [ebp-2],cl
	jmp NextSpan
	;
	EndSkipTable LABEL DWORD
	DD EndSkip0
	DD EndSkip1
	DD EndSkip2
	DD EndSkip3
ENDM

;-------------------------;
; 16-bit Pentium clippers ;
;-------------------------;

TMI_16P_NoSkipIn MACRO tag:req, writefinalmacro:req
	;
	mov		esi,[edi].FTexLatticeD.LatH	; High tex
	mov		[SavedESP],esp				; Save stack pointer
	;
	mov		ebx,[edi].FTexLatticeD.LatL	; Low tex
	call	DWORD PTR [ecx+eax*8]		; Call span texture mapper
	;
	writefinalmacro						; Write final bytes
ENDM

TMI_16P_PreSkip MACRO
	mov		eax,[TMI_FinalDest]
	;
	mov		[SavedFinalDest],eax
	mov		[TMI_FinalDest],ebp
	;
	call	[SavedProc]					; Call procedure pointer
ENDM

TMI_16P_PostSkip MACRO tag:req, writefinalmacro:req
	mov		ecx,[SavedFinalDest]
	;
	mov		[TMI_FinalDest],ecx
	cmp		ebp,ecx
	;
	jge		@catstr(PostSkip,tag)
	;
	call	[SavedInner]
	;
	writefinalmacro
	;
	jmp		@catstr(PostSkip,tag)
ENDM

TMI_16P_SkipIn MACRO tag:req, writefinalmacro:req
	Local SkipTable,Skip0,Skip1,Skip2,Skip3
	;
	; Perform skip-in math:
	;
	TMI_P_SkipInMath
	;
	jmp		SkipTable[ecx*4]				; Call custom skip function
	;
	; Call tmapper to draw first 4 pixels:
	;
	Skip1:
	TMI_16P_PreSkip
	mov			cx,WORD PTR [TexStuff+2]
	mov			[ebp*2-6],cx
	mov			ecx,DWORD PTR [TexStuff+4]
	mov			[ebp*2-4],ecx
	TMI_16P_PostSkip tag, writefinalmacro
	;
	Skip2:
	TMI_16P_PreSkip
	mov			ecx,DWORD PTR [TexStuff+4]
	mov			[ebp*2-4],ecx
	TMI_16P_PostSkip tag, writefinalmacro
	;
	Skip3:
	TMI_16P_PreSkip
	mov			[ebp*2-2],cx
	TMI_16P_PostSkip tag, writefinalmacro
	;
	; Call tmapper to draw remainder of line:
	;
	SkipTable LABEL DWORD
	DD Skip0
	DD Skip1
	DD Skip2
	DD Skip3
	;
	Skip0:
	call [SavedProc]
	writefinalmacro
	;
	@catstr(PostSkip,tag):
ENDM

TMI_16P_SkipInAdjacent MACRO
	Local Adj00,Adj01,Adj02,Adj03,Adj10,Adj11,Adj12,Adj13
	Local Adj20,Adj21,Adj22,Adj23,Adj30,Adj31,Adj32,Adj33
	Local Done,AdjacentTable
	;
	TMI_P_SkipInMath						; Perform skip-in math
	;
	mov		[SavedOffset],ecx				; SavedOffset = SpanStart & 3
	call	[SavedProc]						; Call procedure pointer
	;
	mov		eax,[SavedOffset]				; eax = SpanStart & 3
	mov		ebx,[SpanEnd]					; ebx = SpanEnd
	;
	rol		eax,4							; eax = (SpanStart & 3) * 16
	and		ebx,3							; ebx = (SpanEnd & 3)
	;
	mov		edx,ecx							; edx = Pixels
	;
	shr		edx,16							; edx = Shifted pixels
	jmp		AdjacentTable[eax+ebx*4]
	;
	Adj00:
	fld QWORD PTR [TexStuff]
	fstp QWORD PTR [ebp*2-8]
	jmp NextSpan
	;
	Adj01:
	mov cx,WORD PTR [TexStuff]
	mov WORD PTR [ebp*2-8],cx
	jmp NextSpan
	;
	Adj02:
	mov ecx,DWORD PTR [TexStuff]
	mov DWORD PTR [ebp*2-8],ecx
	jmp NextSpan
	;
	Adj03:
	mov ecx,DWORD PTR [TexStuff]
	mov DWORD PTR [ebp*2-8],ecx
	mov cx,WORD PTR [TexStuff+4]
	mov WORD PTR [ebp*2-4],cx
	jmp NextSpan
	;
	Adj10:
	mov cx,WORD PTR [TexStuff+2]
	mov WORD PTR [ebp*2-6],cx
	mov ecx,DWORD PTR [TexStuff+4]
	mov DWORD PTR [ebp*2-4],ecx
	jmp NextSpan
	;
	Adj11:
	jmp NextSpan
	;
	Adj12:
	mov cx,WORD PTR [TexStuff+2]
	mov WORD PTR [ebp*2-6],cx
	jmp NextSpan
	;
	Adj13:
	mov cx,WORD PTR [TexStuff+2]
	mov WORD PTR [ebp*2-6],cx
	mov cx,WORD PTR [TexStuff+4]
	mov WORD PTR [ebp*2-4],cx
	jmp NextSpan
	;
	Adj20:
	mov ecx,DWORD PTR [TexStuff+4]
	mov DWORD PTR [ebp*2-4],ecx
	jmp NextSpan
	;
	Adj21:
	jmp NextSpan
	;
	Adj22:
	jmp NextSpan
	;
	Adj23:
	mov cx,WORD PTR [TexStuff+4]
	mov WORD PTR [ebp*2-4],cx
	jmp NextSpan
	;
	Adj30:
	mov cx,WORD PTR [TexStuff+6]
	mov WORD PTR [ebp*2-2],cx
	jmp NextSpan
	;
	Adj31:
	jmp NextSpan
	;
	Adj32:
	jmp NextSpan
	;
	Adj33:
	jmp NextSpan
	;
	AdjacentTable LABEL DWORD
	DD	Adj00,Adj01,Adj02,Adj03
	DD	Adj10,Adj11,Adj12,Adj13
	DD	Adj20,Adj21,Adj22,Adj23
	DD	Adj30,Adj31,Adj32,Adj33
ENDM

TMI_16P_WriteFinal MACRO
	fld QWORD PTR [TexStuff]
	fstp QWORD PTR [ebp*2-8]
ENDM

TMI_16P_WriteFinalClipped MACRO
	LOCAL EndSkipTable,EndSkip0,EndSkip1,EndSkip2,EndSkip3
	;
	mov eax,[SpanEnd]
	;
	and eax,3
	;
	jmp EndSkipTable[eax*4]
	;
	EndSkip0:
	fld QWORD PTR [TexStuff]
	fstp QWORD PTR [ebp*2-8]
	jmp NextSpan
	;
	EndSkip1:
	mov cx,WORD PTR [TexStuff]
	mov [ebp*2-8],cx
	jmp NextSpan
	;
	EndSkip2:
	mov ecx,DWORD PTR [TexStuff]
	mov [ebp*2-8],ecx
	jmp NextSpan
	;
	EndSkip3:
	mov ecx,DWORD PTR [TexStuff]
	mov [ebp*2-8],ecx
	mov cx,WORD PTR [TexStuff+4]
	mov [ebp*2-4],cx
	jmp NextSpan
	;
	EndSkipTable LABEL DWORD
	DD EndSkip0
	DD EndSkip1
	DD EndSkip2
	DD EndSkip3
ENDM

;-------------------------;
; 32-bit Pentium clippers ;
;-------------------------;

TMI_32P_NoSkipIn MACRO tag:req, writefinalmacro:req
	;
	mov		esi,[edi].FTexLatticeD.LatH	; High tex
	mov		[SavedESP],esp				; Save stack pointer
	;
	mov		ebx,[edi].FTexLatticeD.LatL	; Low tex
	call	DWORD PTR [ecx+eax*8]		; Call span texture mapper
	;
	writefinalmacro						; Write final bytes
ENDM

TMI_32P_PreSkip MACRO
	mov		eax,[TMI_FinalDest]
	;
	mov		[SavedFinalDest],eax
	mov		[TMI_FinalDest],ebp
	;
	call	[SavedProc]					; Call procedure pointer
ENDM

TMI_32P_PostSkip MACRO tag:req, writefinalmacro:req
	mov		ecx,[SavedFinalDest]
	;
	mov		[TMI_FinalDest],ecx
	cmp		ebp,ecx
	;
	jge		@catstr(PostSkip,tag)
	;
	call	[SavedInner]
	;
	writefinalmacro
	;
	jmp		@catstr(PostSkip,tag)
ENDM

TMI_32P_SkipIn MACRO tag:req, writefinalmacro:req
	Local SkipTable,Skip0,Skip1,Skip2,Skip3
	;
	; Perform skip-in math:
	;
	TMI_P_SkipInMath
	;
	jmp		SkipTable[ecx*4]				; Call custom skip function
	;
	; Call tmapper to draw first 4 pixels:
	;
	Skip1:
	TMI_32P_PreSkip
		mov			ecx,DWORD PTR [TexStuff+4]
		mov			[ebp*4-12],ecx
		mov			ecx,DWORD PTR [TexStuff+8]
		mov			[ebp*4-8],ecx
		mov			ecx,DWORD PTR [TexStuff+12]
		mov			[ebp*4-4],ecx
	TMI_32P_PostSkip tag, writefinalmacro
	;
	Skip2:
	TMI_32P_PreSkip
		mov			ecx,DWORD PTR [TexStuff+8]
		mov			[ebp*4-8],ecx
		mov			ecx,DWORD PTR [TexStuff+12]
		mov			[ebp*4-4],ecx
	TMI_32P_PostSkip tag, writefinalmacro
	;
	Skip3:
	TMI_32P_PreSkip
		mov			[ebp*4-4],ecx
	TMI_32P_PostSkip tag, writefinalmacro
	;
	; Call tmapper to draw remainder of line:
	;
	SkipTable LABEL DWORD
	DD Skip0
	DD Skip1
	DD Skip2
	DD Skip3
	;
	Skip0:
	call [SavedProc]
	writefinalmacro
	;
	@catstr(PostSkip,tag):
ENDM

TMI_32P_SkipInAdjacent MACRO
	Local Adj00,Adj01,Adj02,Adj03,Adj10,Adj11,Adj12,Adj13
	Local Adj20,Adj21,Adj22,Adj23,Adj30,Adj31,Adj32,Adj33
	Local Done,AdjacentTable
	;
	TMI_P_SkipInMath						; Perform skip-in math
	;
	mov		[SavedOffset],ecx				; SavedOffset = SpanStart & 3
	call	[SavedProc]						; Call procedure pointer
	;
	mov		eax,[SavedOffset]				; eax = SpanStart & 3
	mov		ebx,[SpanEnd]					; ebx = SpanEnd
	;
	rol		eax,4							; eax = (SpanStart & 3) * 16
	and		ebx,3							; ebx = (SpanEnd & 3)
	;
	mov		edx,ecx							; edx = Pixels
	;
	shr		edx,16							; edx = Shifted pixels
	jmp		AdjacentTable[eax+ebx*4]
	;
	Adj00:
		mov  ecx,DWORD PTR [TexStuff]
		mov  DWORD PTR [ebp*4-16],ecx
		mov  ecx,DWORD PTR [TexStuff+4]
		mov  DWORD PTR [ebp*4-12],ecx
		mov  ecx,DWORD PTR [TexStuff+8]
		mov  DWORD PTR [ebp*4-8],ecx
		mov  ecx,DWORD PTR [TexStuff+12]
		mov  DWORD PTR [ebp*4-4],ecx
	jmp NextSpan
	;
	Adj01:
		mov ecx,DWORD PTR [TexStuff]
		mov DWORD PTR [ebp*4-16],ecx
	jmp NextSpan
	;
	Adj02:
		mov  ecx,DWORD PTR [TexStuff]
		mov  DWORD PTR [ebp*4-16],ecx
		mov  ecx,DWORD PTR [TexStuff+4]
		mov  DWORD PTR [ebp*4-12],ecx
	jmp NextSpan
	;
	Adj03:
		mov  ecx,DWORD PTR [TexStuff]
		mov  DWORD PTR [ebp*4-16],ecx
		mov  ecx,DWORD PTR [TexStuff+4]
		mov  DWORD PTR [ebp*4-12],ecx
		mov  ecx,DWORD PTR [TexStuff+8]
		mov  DWORD PTR [ebp*4-8],ecx
	jmp NextSpan
	;
	Adj10:
		mov  ecx,DWORD PTR [TexStuff+4]
		mov  DWORD PTR [ebp*4-12],ecx
		mov  ecx,DWORD PTR [TexStuff+8]
		mov  DWORD PTR [ebp*4-8],ecx
		mov  ecx,DWORD PTR [TexStuff+12]
		mov  DWORD PTR [ebp*4-4],ecx
	jmp NextSpan
	;
	Adj11:
	jmp NextSpan
	;
	Adj12:
		mov ecx,DWORD PTR [TexStuff+4]
		mov DWORD PTR [ebp*4-12],ecx
	jmp NextSpan
	;
	Adj13:
		mov ecx,DWORD PTR [TexStuff+4]
		mov DWORD PTR [ebp*4-12],ecx
		mov ecx,DWORD PTR [TexStuff+8]
		mov DWORD PTR [ebp*4-8],ecx
	jmp NextSpan
	;
	Adj20:
		mov ecx,DWORD PTR [TexStuff+8]
		mov DWORD PTR [ebp*4-8],ecx
		mov ecx,DWORD PTR [TexStuff+12]
		mov DWORD PTR [ebp*4-4],ecx
	jmp NextSpan
	;
	Adj21:
	jmp NextSpan
	;
	Adj22:
	jmp NextSpan
	;
	Adj23:
		mov ecx,DWORD PTR [TexStuff+8]
		mov DWORD PTR [ebp*4-8],ecx
	jmp NextSpan
	;
	Adj30:
		mov ecx,DWORD PTR [TexStuff+12]
		mov DWORD PTR [ebp*4-4],ecx
	jmp NextSpan
	;
	Adj31:
	jmp NextSpan
	;
	Adj32:
	jmp NextSpan
	;
	Adj33:
	jmp NextSpan
	;
	AdjacentTable LABEL DWORD
	DD	Adj00,Adj01,Adj02,Adj03
	DD	Adj10,Adj11,Adj12,Adj13
	DD	Adj20,Adj21,Adj22,Adj23
	DD	Adj30,Adj31,Adj32,Adj33
ENDM

TMI_32P_WriteFinal MACRO
	mov ecx,DWORD PTR [TexStuff]
	mov [ebp*4-16],ecx
	;
	mov ecx,DWORD PTR [TexStuff+4]
	mov [ebp*4-12],ecx
	;
	mov ecx,DWORD PTR [TexStuff+8]
	mov [ebp*4-8],ecx
	;
	mov ecx,DWORD PTR [TexStuff+12]
	mov [ebp*4-4],ecx
ENDM

TMI_32P_WriteFinalClipped MACRO
	LOCAL EndSkipTable,EndSkip0,EndSkip1,EndSkip2,EndSkip3
	;
	mov eax,[SpanEnd]
	;
	and eax,3
	;
	jmp EndSkipTable[eax*4]
	;
	EndSkip0:
		mov ecx,DWORD PTR [TexStuff]
		mov [ebp*4-16],ecx
		;
		mov ecx,DWORD PTR [TexStuff+4]
		mov [ebp*4-12],ecx
		;
		mov ecx,DWORD PTR [TexStuff+8]
		mov [ebp*4-8],ecx
		;
		mov ecx,DWORD PTR [TexStuff+12]
		mov [ebp*4-4],ecx
	jmp NextSpan
	;
	EndSkip1:
		mov ecx,DWORD PTR [TexStuff]
		mov [ebp*4-16],ecx
	jmp NextSpan
	;
	EndSkip2:
		mov ecx,DWORD PTR [TexStuff]
		mov [ebp*4-16],ecx
		;
		mov ecx,DWORD PTR [TexStuff+4]
		mov [ebp*4-12],ecx
	jmp NextSpan
	;
	EndSkip3:
		mov ecx,DWORD PTR [TexStuff]
		mov [ebp*4-16],ecx
		;
		mov ecx,DWORD PTR [TexStuff+4]
		mov [ebp*4-12],ecx
		;
		mov ecx,DWORD PTR [TexStuff+8]
		mov [ebp*4-8],ecx
	jmp NextSpan
	;
	EndSkipTable LABEL DWORD
	DD EndSkip0
	DD EndSkip1
	DD EndSkip2
	DD EndSkip3
ENDM

;--------------------------------------------------------------------------------
; Lighting outer loop
;--------------------------------------------------------------------------------

;---------------------;
; Lighting outer loop ;
;---------------------;
;
; Call with: esi = span pointer
;
LightOuter PROC
	;
	mov		cl,[GBlit.InterXBits]			; cl  = GBlit.InterXBits
	;
	test	esi,esi							; Empty span?
	jz		UpdateRects						; Yes, skip it
	;
	;-----------;
	; Span loop ;
	;-----------;
	SpanLoop:
	;
	; Set up:
	;
	mov		edi,[TLO_BotBase]				; edi = TLO_BotBase
	mov		ebx,[esi].FSpan.SpanEnd				; ebx = Span->End
	;
	mov		eax,[esi].FSpan.SpanStart		; eax = Span->Start
	mov		edx,[esi].FSpan.SpanNext		; edx = Span->Next
	;
	lea		ebx,[edi+ebx*4]					; ebx = TLO_BotBase + Span->End
	mov		[SpanNext],edx					; SpanNext = Span->Next
	;
	lea		edi,[edi+eax*4]					; edi = TLO_BotBase + Span->Start
	mov		edx,[GBlit].InterXMask			; edx = GBlit.InterXMask
	;
	mov		[TLO_FinalDest],ebx				; TLO_FinalDest = TLO_BotBase + Span->End
	and		edx,eax							; edx = Span->Start & GBlit.InterXMask
	;
	shr		eax,cl							; eax = Span->Start >> GBlit.InterXBits
	mov		ebx,[TLO_LatticeBase]			; ebx = TLO_LatticeBase
	;
	mov		[TLI_Dest],edi					; TLI_Dest = TLO_BotBase + Span->Start
	mov		edi,[GBlit].InterXMask			; edi = GBlit.InterXMask
	;
	mov		[TLI_SkipIn],edx				; TLI_SkipIn = Span->Start & GBlit.InterXMask
	mov		edx,[ebx + eax*4]				; edx = TLO_LatticeBase[Span->Start >> GBlit.InterXBits]
	;
	mov	[TLI_TopLattice],edx ;!!
	;
	; Check whether we're drawing multiple rectspans:
	;
	mov		ebx,[esi].FSpan.SpanEnd			; ebx = Span->End
	mov		eax,[esi].FSpan.SpanStart		; eax = Span->Start
	;
	dec		ebx								; ebx = Span->End - 1
	mov		ecx,[GBlit].InterXNotMask		; ecx = GBlit.InterXNotMask
	;
	xor		ebx,eax							; ebx = Span->Start ^ (Span->End - 1)
	mov		edi,[GBlit].InterX				; edi = GBlit.InterX
	;
	and		ebx,ecx							; ebx = (Span->Start ^ (Span->End - 1)) & GBlit.InterXNotMask
	and		eax,ecx							; eax = Span->Start & GBlit.InterXNotMask
	;
	test	ebx,ebx							; Drawing multiple rectspans?
	jz		DrawLast						; No, just draw one
	;
	; Prepare to draw multiple rectspans:
	;
	add		eax,edi							; eax = GBlit.InterX + (Span->Start & GBlit.InterXNotMask)
	mov		edi,[TLO_BotBase]				; edi = TLO_BotBase
	;
	;
	lea		eax,[edi+eax*4]					; eax = TLO_BotBase + GBlit.InterX + (Span->Start & GBlit.InterXNotMask)
	;
	mov		[TLI_DestEnd],eax				; TLI_DestEnd = TLO_BotBase + GBlit.InterX + (Span->Start & GBlit.InterXNotMask)
	call	[TLO_LightInnerProc]			; Perform lighting
	;
	; Prepare to draw middle rectspans:
	;
	mov		edx,[TLI_TopLattice]			; edx = TLI_TopLattice
	xor		ecx,ecx							; ecx = 0
	;
	mov		eax,[TLI_DestEnd]				; eax = TLI_DestEnd
	mov		ebx,[GBlit].InterX				; ebx = GBlit.InterX
	;
	;
	mov		[TLI_SkipIn],ecx				; TLI_SkipIn = 0
	lea		ebx,[eax+ebx*4]					; ebx = TLI_DestEnd + GBlit.InterX
	;
	add		edx,SIZE FTexLattice			; edx = TLI_TopLattice + 1
	mov		[TLI_Dest],eax					; TLI_Dest = TLI_DestEnd
	;
	mov		[TLI_DestEnd],ebx				; TLI_DestEnd = TLI_DestEnd + GBlit.InterX
	mov		eax,[TLO_FinalDest]				; eax = TLO_FinalDest
	;
	mov		[TLI_TopLattice],edx			; TLI_TopLattice = TLI_TopLattice + 1
	;
	cmp		ebx,eax							; TLI_DestEnd < TLO_FinalDest?
	jge		DrawLast						; No, draw last rectspan
	;
	; Draw middle rectspans:
	;
	MiddleLoop:
	;
	call	[TLO_LightInnerProc]			; Perform lighting
	;
	mov		ecx,[TLI_DestEnd]				; ecx = TLI_DestEnd
	mov		ebx,[GBlit].InterX				; ebx = GBlit.InterX
	;
	mov		edx,[TLI_TopLattice]			; edx = TLI_TopLattice
	lea		ebx,[ecx+ebx*4]					; ebx = TLI_DestEnd + GBlit.InterX
	;
	add		edx,SIZE FTexLattice			; TLI_TopLattice = TLI_TopLattice + 1
	mov		[TLI_Dest],ecx					; TLI_Dest = TLI_DestEnd
	;
	mov		[TLI_DestEnd],ebx				; TLI_DestEnd = TLI_DestEnd + GBlit.InterX
	mov		eax,[TLO_FinalDest]				; ecx = TLO_FinalDest
	;
	mov		[TLI_TopLattice],edx			; edx = TLI_TopLattice
	;
	cmp		ebx,eax							; TLI_DestEnd < TLO_FinalDest?
	jl		MiddleLoop						; Yes, keep drawing middle rectspans
	;
	; Draw the last rectspan:
	;
	DrawLast:
	;
	mov		eax,[TLO_FinalDest]
	;
	mov		[TLI_DestEnd],eax				; TLI_DestEnd = TLO_FinalDest
	call	[TLO_LightInnerProc]			; Perform lighting
	;
	; Go to next span:
	;
	mov		esi,[SpanNext]					; Get next span
	mov		cl,[GBlit.InterXBits]			; cl  = GBlit.InterXBits
	;
	test	esi,esi							; At last span?
	jnz		SpanLoop						; No, keep looping
	;
	;------------------;
	; Update all rects ;
	;------------------;
	UpdateRects:
	;
	; esi = RectSpan pointer
	; edi = LatticeBase pointer
	; ebp = Rect counter
	; eax, ebx, ecx, edx = temp
	;
	mov esi,[TLO_RectSpan]			; Get RectSpan pointer
	mov ebx,[TLO_LatticeBase]		; Get lattice base address
	;
	test esi,esi					; Empty RectSpan?
	jz Done							; Yes, exit
	;
	; Rect span loop:
	;
	RectSpanLoop:
	;
	mov eax,[esi].FSpan.SpanStart	; Get start
	mov ebp,[esi].FSpan.SpanEnd		; Get end
	;
	sub ebp,eax						; Now ebp = rect counter
	;
	mov edi,[ebx + eax*4]			; Get lattice pointer
	;
	; Rect loop:
	;
	RectLoop:
		;
		; Update values:
		;
		mov eax,[edi].FTexLatticeD.SubL		; Get Low
		mov ebx,[edi].FTexLatticeD.SubH		; Get High
		;
		mov ecx,[edi].FTexLatticeD.SubLY	; Get Low x-inc
		mov edx,[edi].FTexLatticeD.SubHY	; Get High x-inc
		;
		add eax,ecx							; Update low
		;
		adc ebx,edx							; Update high
		mov [edi].FTexLatticeD.SubL,eax		; Save Low
		;
		mov [edi].FTexLatticeD.SubH,ebx		; Save High
		;
		; Update incs:
		;
		mov eax,[edi].FTexLatticeD.SubLX	; Get Low
		mov ebx,[edi].FTexLatticeD.SubHX	; Get High
		;
		mov ecx,[edi].FTexLatticeD.SubLXY	; Get Low x-inc
		mov edx,[edi].FTexLatticeD.SubHXY	; Get High x-inc
		;
		add eax,ecx							; Update low
		;
		adc ebx,edx							; Update high
		mov [edi].FTexLatticeD.SubLX,eax	; Save Low
		;
		mov [edi].FTexLatticeD.SubHX,ebx	; Save High
		add edi,SIZE FTexLattice			; Go to next lattice
		;
		; Next rect:
		;
		dec ebp
	jg RectLoop
	;
	; Next span:
	;
	mov esi,[esi].FSpan.SpanNext	; Get next span pointer
	mov ebx,[TLO_LatticeBase]		; Get lattice base address again
	;
	test esi,esi					; Past last span?
	jnz  RectSpanLoop				; Proceed with next rect span
	;
	Done:
	ret
LightOuter ENDP

;--------------------------------------------------------------------------------
; Light interpolators
;--------------------------------------------------------------------------------

;---------------------;
; Light interpolators ;
;---------------------;
;
; Call with:
; esi = SubRectSpan pointer (must not be NULL)
;
; Variables:
; ebp = end
; edi = current position
; eax = TLO_TopBase
; ebx = TLO_BotBase
; ecx,edx,esi = averagers

; 4-line Pentium version
LightVInterpolate_8P_4 PROC
	;
	mov eax,[TLO_TopBase]			; Get top pointer
	mov ebx,[TLO_BotBase]			; Get bottom pointer
	;
	; Main span loop:
	;
	SpanLoop:
	mov edi,[esi].FSpan.SpanStart	; Now edi=start
	mov ecx,[esi].FSpan.SpanNext	; Get next pointer
	mov ebp,[esi].FSpan.SpanEnd		; Now ebp=end
	mov [SpanNext],ecx				; Save next span
	;
	; Prime:
	;
	mov	ecx,[eax + edi*4]
	mov	edx,[ebx + edi*4]
	;
	sub edx,ecx
	;
	shr edx,2
	;
	and edx,0ffffh
	;
	add ecx,edx
	;
	; Averaging loop:
	;
	AverageLoop:
	;
	mov GLight[edi*4 + LIGHT_XR*4],ecx
	add ecx,edx
	;
	mov GLight[edi*4 + LIGHT_XR*8],ecx
	add ecx,edx
	;
	mov	edx,[ebx + edi*4 + 4]
	mov GLight[edi*4 + LIGHT_XR*12],ecx
	;
	mov	ecx,[eax + edi*4 + 4]
	;
	sub edx,ecx
	inc edi
	;
	sar edx,2
	cmp edi,ebp
	;
	lea ecx,[ecx+edx]
	jle AverageLoop
	;
	; Next span:
	;
	mov  esi,[SpanNext]			; Get next-span pointer
	test esi,esi				; Done yet?
	jnz  SpanLoop				; Continue
	;
	ret
LightVInterpolate_8P_4 ENDP

; 2-line Pentium version
LightVInterpolate_8P_2 PROC
	;
	mov eax,[TLO_TopBase]			; Get top pointer
	mov ebx,[TLO_BotBase]			; Get bottom pointer
	;
	; Main span loop:
	;
	SpanLoop:
	mov edi,[esi].FSpan.SpanStart	; Now edi=start
	mov ebp,[esi].FSpan.SpanEnd		; Now ebp=end
	;
	; Averaging prime:
	;
	mov ecx,[eax + edi*4]
	mov edx,[ebx + edi*4]
	add ecx,edx
	;
	; Averaging loop:
	;
	AverageLoop:
	;
	shr ecx,1
	mov edx,[ebx + edi*4 + 4]
	;
	mov GLight[edi*4 + LIGHT_XR*4],ecx
	mov ecx,[eax + edi*4 + 4]
	;
	add ecx,edx
	mov edx,[ebx + edi*4 + 8]
	;
	shr ecx,1
	;
	mov GLight[edi*4 + LIGHT_XR*4 + 4],ecx
	mov ecx,[eax + edi*4 + 8]
	;
	add ecx,edx
	add edi,2
	;
	cmp edi,ebp
	jl  AverageLoop
	;
	shr ecx,1
	;
	mov GLight[edi*4 + LIGHT_XR*4],ecx
	;
	; Next span:
	;
	mov  esi,[esi].FSpan.SpanNext	; Get next-span pointer
	test esi,esi					; Done yet?
	jnz  SpanLoop					; Continue
	;
	ret
LightVInterpolate_8P_2 ENDP

; 1-line Pentium version
LightVInterpolate_8P_1 PROC
	ret
LightVInterpolate_8P_1 ENDP

;--------------------------------------------------------------------------------
; Outer loop generators
;--------------------------------------------------------------------------------

;----------------------------------;
; Generate all texture outer loops ;
;----------------------------------;

.CODE
FOR Variety,<8P,16P,32P>
	TRO_OuterMacro Variety
ENDM

;--------------------------------------------------------------------------------
; The End
;--------------------------------------------------------------------------------
END
