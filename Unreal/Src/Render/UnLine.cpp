/*=============================================================================
	UnLine.cpp: INCLUDABLE C FILE for drawing lines.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	This is used to avoid the overhead of establishing a stack frame and calling
	a routine for every scanline to be drawn.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

//
//void DrawLine(const CAMERA_INFO *Camera, FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2,..)
//	{
	FLOAT	FloatFixDX,Temp;
	BYTE	*Screen;
	INT		FixDX,FixX,X,Y,DY,Count,DestX,ScreenInc,ColorBytes;
	//
	#ifdef 	ANTIALIAS
		FLOAT	FloatFixDY;
		INT		FixDY,FixY,DX;
	#endif
	//
	#ifdef  DEPTHSHADE
		FLOAT	G1,G2;
		INT		FixG1,FixG2,FixDG,FixTemp;
	#endif
	//
	GUARD;
	//
	// Bounds check
	//
	if ((X1 < 0.0         ) || (X2 < 0.0          ) ||
		(Y1 < 0.0         ) || (Y2 < 0.0          ) ||
		(X1 > Camera->FSXR) || (X2 > Camera->FSXR ) ||
		(Y1 > Camera->FSYR) || (Y2 > Camera->FSYR))
		{
		debugf(LOG_Rend,"Line: Bounds (%i,%i) (%i,%i)",X1,Y1,X2,Y2);
		return;
		};
	ColorBytes=Camera->ColorBytes;
	//
	// Depth shading
	//
	#ifdef  DEPTHSHADE
		#define DEPTHSETUP(Arclen) FixDG = (FixG2-FixG1)/Arclen;
		G1 = 65536.0 * 12 - RZ1 * 65536.0 * 100000.0; if (G1<0.0) G1=0.0;
		G2 = 65536.0 * 12 - RZ2 * 65536.0 * 100000.0; if (G2<0.0) G2=0.0;
		FixG1 = ftoi(G1);
		FixG2 = ftoi(G2);
	#else
		#define DEPTHSETUP(Arclen)
	#endif
	//
	// Arrange so that Y2 >= Y1
	//
	if (Y2 < Y1)
		{
		Temp = Y1; Y1 = Y2; Y2 = Temp;
		Temp = X1; X1 = X2; X2 = Temp;
		//
		#ifdef DEPTHSHADE
			FixTemp=FixG1; FixG1 = FixG2; FixG2 = FixTemp;
		#endif
		};
	DestX	= ftoi(X2);
	Y		= ftoi(Y1-0.5);
	DY      = ftoi(Y2-0.5)-Y;
	//
	if (DY==0) // Horizontal line
		{
		if (X2>X1)
			{
			X = ftoi(X1-0.5); Count = ftoi(X2-0.5) - X;
			}
		else
			{
			X = ftoi(X2-0.5); Count = ftoi(X1-0.5) - X;
			//
			#ifdef DEPTHSHADE
				OurExchange(FixG1,FixG2);
			#endif
			};
		#if ISDOTTED
			LineToggle = X&1;
		#endif
		if (Count>0)
			{
			GUARD;
			DEPTHSETUP(Count);
			Screen = Camera->Screen + ((X + Y*Camera->SXStride)<<SHIFT);
			//
			#if defined(ASM) && defined(ASMPIXEL)
			__asm
				{
				mov edi,[Screen]
				mov esi,[ColorBytes]
				mov ecx,[Count]
				#ifdef DEPTHSHADE
					mov eax,[FixG1]
				#else
					mov eax,[NewColor]
				#endif
				;
				LABEL2(HInner):
				L_ASMPIXEL(HInner)
				add edi,esi
				dec ecx
				jg  LABEL2(HInner)
				};
			#else
			while (Count-- > 0)
				{
				L_DRAWPIXEL(Screen); 
				Screen += ColorBytes;
				};
			#endif
			UNGUARD("CASE 1");
			};
		return;
		};
	FloatFixDX = 65536.0 * (X2-X1) / (Y2-Y1);
	FixDX      = ftoi(FloatFixDX);
	FixX       = ftoi(65536.0 * X1 + FloatFixDX * ((FLOAT)(Y+1) - Y1));
	//
	// Antialiased only:
	//
	#ifdef ANTIALIAS
	if (OurAbs(FixDX) > FIX(1))
		{
		//
		// Arrange so that X2 >= X1
		//
		if (X2 < X1)
			{
			Temp = Y1; Y1 = Y2; Y2 = Temp;
			Temp = X1; X1 = X2; X2 = Temp;
			#ifdef DEPTHSHADE
			FixTemp = FixG1; FixG1 = FixG2; FixG2 = FixTemp;
			#endif
			};
		X		  = ftoi(X1-0.5);
		DX        = ftoi(X2-0.5) - X;
		Screen	  = Camera->Screen + X * ColorBytes;
		//
		if (DX==0) return;
		//
		FloatFixDY = 65536.0 * (Y2-Y1) / (X2-X1); // X2-X1 guaranteed nonzero
		FixDY      = ftoi(FloatFixDY);
		FixY       = ftoi(65536.0 * Y1 + FloatFixDY * ((FLOAT)(X+1) - X1));
		//
		DEPTHSETUP(DX);
		//
		if (FixDY >= 0)
			{
			FixY -= FixDY;
			if (FixY < 0) {FixY += FixDY; Screen += ColorBytes<<SHIFT; DX--;};
			do {
				L_DRAWPIXEL(Screen + ((UNFIX(FixY) * Camera->SXStride)<<SHIFT));
				Screen 	+= ColorBytes;
				FixY 	+= FixDY;
				} while (--DX > 0);
			}
		else
			{
			do {
				L_DRAWPIXEL(Screen + ((UNFIX(FixY) * Camera->SXStride)<<SHIFT));
				Screen 	+= ColorBytes;
				FixY 	+= FixDY;
				} while (--DX > 0);
			};
		return;
		}
	#else
	//
	// Unantialiased only:
	//
	if (FixDX < -FIX(1)) // From -infinity to -1 (Horizontal major)
		{		   
		GUARD;
		X      = ftoi(X1-0.5);
		Screen = Camera->Screen + ((X + Y*Camera->SXStride)<<SHIFT);
		//
		#if ISDOTTED
			LineToggle = X&1;
		#endif
		//
		DEPTHSETUP(OurMax(DY,X-ftoi(X2-0.5)));
		//
		while (--DY >= 0)
			{
			Count 	 = X;
			X 		 = UNFIX(FixX);
			Count 	-= X;
			while (Count-- > 0)
				{
				Screen -= ColorBytes; 
				L_DRAWPIXEL(Screen);
				};
			Screen 	+= Camera->SXStride << SHIFT;
			FixX   	+= FixDX;
			};
		while (X-- > DestX) {Screen -= ColorBytes; L_DRAWPIXEL(Screen);};
		UNGUARD("CASE 3");
		}
	else if (FixDX > FIX(1)) // From 1 to +infinity (Horizontal major)
		{
		GUARD;
		X 	   = ftoi(X1-0.5);
		Screen = Camera->Screen + ((X + Y*Camera->SXStride)<<SHIFT);
		//
		#if ISDOTTED
			LineToggle = X&1;
		#endif
		//
		DEPTHSETUP(OurMax(DY,ftoi(X2-0.5)-X));
		//
		while (--DY >= 0)
			{
			Count 	 = X;
			X 		 = UNFIX(FixX);
			Count 	-= X;
			while (Count++ < 0)
				{
				L_DRAWPIXEL(Screen);
				Screen += ColorBytes;
				};
			Screen 	+= Camera->SXStride << SHIFT;
			FixX   	+= FixDX;
			};
		while (X++ < DestX)
			{
			L_DRAWPIXEL(Screen); Screen += ColorBytes;
			};
		UNGUARD("CASE 4");
		}
	#endif
	//
	else if (DY>0) // Vertical major
		{
		Screen = Camera->Screen + ((Y*Camera->SXStride)<<SHIFT);
		/* Antialiased only
		if (FixDX >= 0) // Fix up for from 0 to 1
			{
			FixX -= FixDX;
			if (FixX < 0) {FixX += FixDX; Screen += Camera->SXStride<<SHIFT; if (--DY<==0) break;};
			};*/
		ScreenInc = Camera->SXStride << SHIFT;
		#if ISDOTTED
			LineToggle = Y & 1;
		#endif
		//
		DEPTHSETUP(DY);
		//
		int D=DY,XX=FixX;
		GUARD;
		#if defined(ASM) && defined(ASMPIXEL)
		__asm
			{
			#ifdef DEPTHSHADE
				mov eax,[FixG1]
			#else
				mov eax,[NewColor]
			#endif
			;
			LABEL2(VInner):
			mov edx,[FixX]					; u Fixed point X location
			mov esi,[FixDX]					; v Get X increment
			add esi,edx						; u Update X increment
			mov edi,[Screen]				; v Screen destination
			sar edx,16-SHIFT				; u Unfix the location it
			mov [FixX],esi					; v Saved next X value
			mov esi,[ScreenInc]				; u Get screen increment
			and edx,0xffffffff << SHIFT		; v Mask out 1/2/4-byte color
			add esi,edi						; u Get next screen destination
			add edi,edx						; v Get destination address forthis pixel
			mov [Screen],esi				; u Save next screen destination
			;
			L_ASMPIXEL(VInner)
			;
			mov edx,[DY]
			dec edx
			mov [DY],edx
			jg LABEL2(VInner)
			};
		#else
		do  {
			L_DRAWPIXEL(Screen + (UNFIX(FixX)<<SHIFT));
			FixX 	+= FixDX;
			Screen 	+= ScreenInc;
			} while (--DY > 0);
		#endif
		UNGUARD("CASE 2");
		};
	#undef DEPTHSETUP
	UNGUARD("UnLine");
//	};
