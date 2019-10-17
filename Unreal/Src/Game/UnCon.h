/*=============================================================================
	UnCon.h: FCameraConsole game-specific definition

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Contains routines for: Messages, menus, status bar
=============================================================================*/

#ifndef _INC_UNCON
#define _INC_UNCON

/*------------------------------------------------------------------------------
	FCameraConsole definition
------------------------------------------------------------------------------*/

//
// Camera console. Overrides the virtual base class FVirtualCameraConsole.
// The Unreal engine only has access to the public members in
// FVirtualCameraConsole.
//
class FCameraConsole : public FVirtualCameraConsole
	{
	friend class FGame;
	//
	// Overrides:
	//
	public:
	void	Init 		(UCamera *Camera);
	void	Exit		(void);
	int		Key 		(int Key);
	int		IsTyping	(void);
	void	Message 	(int MsgType, const char *Text);
	void VARARGS Messagef(int MsgType, const char *Fmt,...);
	void	PreRender	(class ICamera *Camera);
	void	PostRender	(class ICamera *Camera,int XLeft);
	void	Log			(ELogType MsgType, const char *Text);
	int		Exec		(const char *Cmd,FOutputDevice *Out=GApp);
	void	NoteResize	(void);
	//
	// Utility functions:
	//
	void	ClearInput	(void);
	//
	// Game-specific implementation:
	//
	private:
	//
	#define CON_SHOW 0.55 /* Fraction of playfield occupied by console when shown */
	enum {MESSAGE_TIME	= 70};
	enum {MAX_BORDER    = 6};
	enum {MAX_LINES		= 64};
	enum {MAX_HISTORY	= 16};
	typedef char TEXTMSG[TEXTMSG_LENGTH];
	//
	UCamera		*Camera;				// Camera owning this console
	FCameraConsole *Old;
	int StatusRefreshPages;
	//
	int  		KeyState;				// Typing state
	int			HistoryTop;				// Top of history list
	int			HistoryCur;				// Current entry in history list
	TEXTMSG		TypedStr;				// Message the player is typing
	TEXTMSG		History[MAX_HISTORY];	// Message the player is typing
	//
	int			Scrollback;				// How many lines the console is scrolled back
	int			NumLines;				// Number of valid lines in buffer
	int			TopLine;				// Current message (loop buffer)
	int  		MsgType;				// Type of most recent message
	int  		MsgStart;				// Timer tick when the most recent message begin
	int  		MsgDuration;			// Expiration time of most recent message
	TEXTMSG 	MsgText[MAX_LINES];		// Current console message
	//
	int			BorderSize,Redraw;
	int			SXR,SYR;
	int			ConsoleLines,StatusBarLines;//obsolete:ExtraBarLines;
	int			BorderLines,BorderPixels;
	int			LogoUp;
	//
	int			QuickStats,AllStats;
	QWORD		LastTickTime;
	FLOAT		Fade;
	FLOAT		ConsolePos,ConsoleDest;
	UTexture	*StatusBar;				// Status bar picture
	UTexture	*StatusSmallBar;		// Smaller version of StatusBar.
	UTexture	*StatusWeaponLight;     // Status bar light for owned weapons.
	UTexture	*StatusSmallWeaponLight;// Smaller version of StatusWeaponLight.
    //obsolete:	UTexture	*ExtraBar;				// Extra status bar picture
	UTexture	*ConBackground;			// Player console background texture
	UTexture	*Border;				// Screen border when sized down
	UTexture	*ArmorImages[DMT_Count];// Armor images for different kinds of armor status.
    //obsolete:	UTexture    *HealthImage           ; // Image used for health status.
    UTexture    *StatusDigits[10]; // Array of digits '0'..'9', to show the status values.
    UTexture    *StatusSmallDigits[10]; // Smaller versions of StatusDigits[].
	//
	void	ShowStat	(class ICamera *Camera,int *StatYL,const char *Str);
	void	DrawStats 	(class ICamera *Camera);
	void	Tick		(ICamera *Camera,int TicksPassed);
	};

/*------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------*/
#endif // _INC_UNCON
