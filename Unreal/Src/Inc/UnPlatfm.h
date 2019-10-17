/*=============================================================================
	UnPlatfm.h: All generic hooks for platform-specific routines

	This structure is shared between the generic Unreal code base and
	the platform-specific routines.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_PLATFORM
#define _INC_PLATFORM

#ifndef _INC_UNPORT
#include "UnPort.h"
#endif

// Forward declarations:
class FOutputDevice;
class FGlobalPlatform;
UNREAL_API extern FGlobalPlatform *GApp;

/*------------------------------------------------------------------------------
	FOutputDevice
------------------------------------------------------------------------------*/

//
// Logging event constants for platform logging subsystem
//
enum ELogType
	{
	LOG_None		= 0,  // Nothing
	LOG_Info        = 1,  // General info & non-error response to 'Exec'
	LOG_Critical    = 2,  // Critical errors, should always print
	LOG_Win         = 3,  // Errors from Windows platform code
	LOG_Net         = 4,  // Errors/warnings from networking calls
	LOG_Ed          = 5,  // Errors/warnings in Editor interface
	LOG_Rend        = 6,  // Errors/warnings in rendering engine
	LOG_File        = 7,  // Errors/warnings during file i/o
	LOG_Audio       = 8,  // Messages about DDE conversation
	LOG_Res         = 9,  // Resource errors/warnings
	LOG_Actor       = 10, // Actor information
	LOG_Server      = 11, // Server information
	LOG_Bug         = 12, // Quick debug message
	LOG_Init        = 13, // Initializing
	LOG_Exit        = 14, // Exiting
	LOG_Bsp         = 15, // Bsp
	LOG_Trans       = 16, // Transaction system
	LOG_Cmd         = 17, // Command line
	LOG_Play        = 18, // General gameplay messages
	LOG_Chat        = 19, // A chat message during gameplay
	LOG_Whisper     = 20, // A whisper message during gameplay
	LOG_Session     = 21, // A session-related message from the server
	LOG_Client      = 22, // A gameplay message from the client
	LOG_ComeGo      = 23, // A gameplay message regarding players coming and going
	LOG_Console		= 24, // Console commands
	LOG_ExecError	= 25, // Error response to Exec
	LOG_Task		= 26, // Task manager
	LOG_Debug		= 27, // Messages to show when debugging only
	LOG_Script		= 28, // Messages to show when debugging only
	LOG_MAX			= 29, // Unused tag representing maximum log value
	};

//
// An output device.  Player consoles, the debug log, and the script
// compiler result text accumulator are output devices.
//
class UNREAL_API FOutputDevice : public FUnknown
	{
	public:
	//
	virtual void Log(ELogType MsgType, const char *Text)=0;
	virtual void Log(const char *Text);
	virtual void VARARGS Logf(ELogType MsgType, const char *Fmt,...);
	virtual void VARARGS Logf(const char *Fmt,...);
	virtual void SpawnConsoleMessage(void);
	};

/*------------------------------------------------------------------------------
	FTask
------------------------------------------------------------------------------*/

//
// Priority of a task.
//
enum ETaskPriority
	{
	PRIORITY_Realtime	= 0,	// The server
	PRIORITY_Normal		= 1,	// Normal priority
	PRIORITY_Camera		= 2,	// Only is run when other tasks are caught up
	};

//
// Bit flags affecting a task.
//
enum ETaskFlags
	{
	TASK_NoUserKill		= 1,	// Task can't be killed by user
	TASK_Paused			= 2,	// Task is paused
	};

//
// An Unreal task.
//
class UNREAL_API FTask
	{
	public:
	//
	FTask *Owner;				// Task owning this task, or NULL
	FOutputDevice *Out;			// Where to send task output
	ETaskPriority Priority;		// Priority of task
	int TaskFlags;				// Flags affecting task
	int TaskID;					// Unique number identifying task
	int TaskTickTime;			// CPU time consumed by most recent tick
	//
	virtual void TaskTick(void)=0;
	virtual void TaskExit(void)=0;
	virtual char *TaskStatus(char *Name,char *Desc)=0;
	};

/*------------------------------------------------------------------------------
	FTaskManager
------------------------------------------------------------------------------*/

//
// Task manager.  Tracks all active tasks.  Tasks include
// things like: The server daemon for a particular in-play
// level; a camera; or a worker task like Ping.
//
// A task does not correspond to a Windows task or thread;
// all tasks are run cooperative from Unreal's one and only
// main thread via their Tick functions.
//
class FTaskManager
	{
	public:
	//
	virtual void Init(void)=0;
	virtual void Exit(void)=0;
	virtual int Exec(const char *Cmd,FOutputDevice *Out=(FOutputDevice *)GApp)=0;
	//
	virtual FTask *GetTaskByID(int TaskID)=0;
	virtual int AddTask(FTask *Task,FTask *Owner,FOutputDevice *Out,ETaskPriority Priority,int Flags)=0;
	virtual void KillTask(FTask *Task)=0;
	virtual void KillTasksUsing(FOutputDevice *Out)=0;
	};
UNREAL_API extern class FTaskManager *GTaskManager;

/*----------------------------------------------------------------------------
	FGlobalPlatform
----------------------------------------------------------------------------*/

//
// Platform-specific code's communication structure:
//
class FGlobalPlatform : public FOutputDevice
	{
	public:
	//
	enum {PLATFORM_VERSION=5};
	//
	// ---------
	// Variables
	// ---------
	//
	DWORD   PlatformVersion;
	DWORD	hWndLog;
	DWORD	hWndParent;
	DWORD	hWndSlowTask;
	DWORD	Debugging;			// =1 if run in debugger with debug version
	DWORD	InAppError;			// =1 if program is caught in error handler.
	DWORD	InSlowTask;			// >0 if in a slow task
	DWORD	ServerLaunched;		// =1 if server has been launched
	DWORD	ServerAlive;		// =1 if server is up and running
	DWORD	LogAlive;			// =1 if logging window is up and running
	DWORD	GuardTrap;			// =1 if error was trapped in guarded code with try/except
	DWORD   KnownButtons;		// Mouse buttons that are definitely pressed
	DWORD	LaunchWithoutLog;	// Launch with log hidden
	DWORD	MMX,PentiumPro,Extra2,Extra3; // Processor features present
	//
	BYTE	LogEventEnabled[LOG_MAX];
	const char	**LogEventNames;
	FILE	*LogFile;
	//
	// Strings:
	//
	char	StartingPath	[256];	// Starting path
	char	DataPath		[256];	// Data path, i.e. for CD-Rom play
	char	CmdLine			[512];	// Command line
	char	LogFname		[256];	// Name of log file
	char	ErrorHist		[2048];	// Error history
	//
	BYTE	KeyDown			[256];
	BYTE	KeyPressed		[256];
	//
	// ----------------------------
	// Platform-specific subsystems
	// ----------------------------
	//
	class FRenderDevice			*RenDev;
	class FCameraManager		*CameraManager;
	// Platform-dependent global audio class goes here
	// Platform-dependent global input class goes here
	// Other global platform-specific subsystem classes go here...
	//
	// ---------
	// Functions
	// ---------
	//
	// Logging:
	//
	virtual void	Log(ELogType Event,const char *Text);
	virtual void	OpenLog(const char *Fname);
	virtual void	CloseLog(void);
	//
	// Memory allocation:
	//
	virtual void	*VARARGS Malloc(int Size,const char *Fmt,...);
	virtual void	*VARARGS MallocAligned(int Size,int Alignment,const char *Fmt,...);
	virtual void	*VARARGS Realloc(void *Ptr,int NewSize,char *Fmt,...);
	virtual void	Free(void *Ptr);
	//
	// Application init & exit & critical event:
	//
	virtual void	Init(char *CmdLine,char *BaseDir);
	virtual void	Startup(void);
	virtual void	CheckAllocations(void);
	virtual void	Exit(void);
	virtual void	Error(const char *Msg);
	virtual void	VARARGS Errorf(const char *Fmt,...);
	virtual int		VARARGS DebugBoxf(char *Fmt,...);
	virtual void    CheckMachineState(void);
	virtual void	ShutdownAfterError(void);
	virtual void	DebugBreak(void);
	virtual void	VARARGS GuardMessagef(const char *Fmt,...);
	//
	// Slow task management:
	//
	virtual void		BeginSlowTask(const char *Task,int StatusWindow, int Cancelable);
	virtual void		EndSlowTask  (void);
	virtual int			StatusUpdate (const char *Str, int Numerator, int Denominator);
	virtual int VARARGS StatusUpdatef(const char *Fmt, int Numerator, int Denominator, ...);
	//
	// Misc:
	//
	virtual void	Poll(void);
	virtual QWORD	TimeMSec(void); // Millisecond timer
	virtual QWORD	TimeUSec(void); // Microsecond timer
	virtual DWORD	CreateFileMapping(const char *Name, int MaxSize, BYTE **Address);
	virtual void    CloseFileMapping(DWORD Handle);
	virtual void	*GetProcAddress(char *ModuleName,char *ProcName,int Checked);
	virtual void	SetParent(DWORD hWndParent);
	virtual void	RequestExit(void);
	virtual void	EdCallback(WORD Code,WORD Param);
	virtual void	Show(void);
	virtual void	Minimize(void);
	virtual void	Hide(void);
	virtual int		MessageBox(const char *Text,const char *Title,int YesNo); // YesNo: 1=yes/no, 0=ok
	virtual void	ResetKeyboard(void);
	virtual void	Enable(void);
	virtual void	Disable(void);
	virtual void    LaunchURL(const char *URL, const char *Extra);
	virtual int     PasswordDialog(const char *Title,const char *Prompt,char *Name,char *Password);
	virtual void	GetMemoryInfo(int *MemoryInUse,int *MemoryAvailable);
	virtual int		Exec(const char *Cmd,FOutputDevice *Out=GApp);
	virtual int		EnableFastMath(int Enable);
    // Reading/writing profile (.ini) values:
    virtual const char * DefaultProfileFileName() const; // What is the default profile file name?
    virtual const char * FactoryProfileFileName() const; // What is the factory-settings profile file name?
    virtual BOOL GetProfileBoolean // Get the boolean value for Key in Section.
    (
        const char * Section      // The name of the section
    ,   const char * Key          // The name of the key.
    ,   BOOL         DefaultValue // The default value if a valid boolean profile value is not found.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
    );
    virtual int GetProfileInteger // Get the integer value for Key in Section.
    (
        const char * Section      // The name of the section
    ,   const char * Key          // The name of the key.
    ,   int          Default      // The default value if the key is not found.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
    );
    virtual void GetProfileSection // Get all the values in a section.
    (
        const char * Section      // The name of the section
    ,   char       * Values       // Where to put Key=Value strings.
    ,   int          Size         // The size of Values.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
        // All the values in the section are put into *Values in the form
        // of "Key=Value". They are separated by null characters, and the
        // last value is followed by 2 null characters.
    );
    virtual BOOL  GetProfileValue // Get the value associated with a key.
    (
        const char * Section      // The name of the section
    ,   const char * Key          // The name of the key.
    ,   const char * Default      // The default value if the key is not found.
    ,         char * Value        // The output value.
    ,   int          Size         // The size of *Value.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
        // Notes:
        //   1. If there are no errors, and the key is found (or a default is
        //      specified), TRUE is returned. Otherwise FALSE is returned.
    );	
    virtual void PutProfileSection // Remove all values in the section and write out new values.
    (
        const char * Section      // The name of the section
    ,   const char * Values       // A list of Key=Value strings.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
        // The Key=Value pairs are separated by nulls, and the last pair is followed by 2 nulls.
    );
    virtual void PutProfileBoolean // Put a boolean value for Key into Section.
    (
        const char * Section      // The name of the section
    ,   const char * Key          // The name of the key.
    ,   BOOL         Value        // The value to put.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
    );
    virtual void PutProfileInteger // Put an integer value for Key into Section.
    (
        const char * Section      // The name of the section
    ,   const char * Key          // The name of the key.
    ,   int          Value        // The value to put.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
    );
    virtual void PutProfileValue // Change the value associated with a key in a section.
    (
        const char * Section      // The name of the section
    ,   const char * Key          // The name of the key.
    ,   const char * Value        // The value to use. 0 causes the value to be deleted from the profile.
    ,   const char * FileName = 0 // The name of the profile file. 0 to use the default.
    );	
	};

//
// Ed callback codes:
//
enum EUnrealEdCallbacks
	{
	EDC_None			= 0,	// Nothing
	EDC_CurTexChange	= 10,	// Change in current texture
	EDC_CurClassChange	= 11,	// Change in current actor class
	EDC_SelPolyChange	= 20,	// Poly selection set changed
	EDC_SelActorChange	= 21,	// Selected actor set changed
	EDC_SelBrushChange	= 22,	// Selected brush set changed
	EDC_RtClickTexture	= 23,	// Right clicked on a picture
	EDC_RtClickPoly		= 24,	// Right clicked on a polygon
	EDC_RtClickActor	= 25,	// Right clicked on an actor
	EDC_RtClickWindow	= 26,	// Right clicked on camera window
	EDC_ModeChange		= 40,	// Mode has changed, Param=new mode index
	EDC_BrushChange		= 41,	// Brush settings changed
	EDC_MapChange		= 42,	// Change in map, Bsp
	EDC_ActorChange		= 43,	// Change in actors
	};

//
//	Keys scan codes, stolen from windows VK_ definitions: !!
//
enum EKeyScan
	{
	K_PLUS 			= 0x6B,
	K_BACKSPACE 	= 0x08,
	K_CTRL 			= 0x11,
	K_DECIMAL 		= 0x6e,
	K_DELETE 		= 0x2e,
	K_DIVIDE 		= 0x6f,
	K_DOWN 			= 0x28,
	K_END 			= 0x23,
	K_ESCAPE 		= 0x1b,
	K_F1 			= 0x70,
	K_F2 			= 0x71,
	K_F3 			= 0x72,
	K_F4 			= 0x73,
	K_F5 			= 0x74,
	K_F6 			= 0x75,
	K_F7 			= 0x76,
	K_F8 			= 0x77,
	K_F9 			= 0x78,
	K_F10 			= 0x79,
	K_F11 			= 0x7a,
	K_F12 			= 0x7b,
	K_HOME 			= 0x24,
	K_INSERT 		= 0x2D,
	K_LBUTTON 		= 0x01,
	K_LEFT 			= 0x25,
	K_MBUTTON 		= 0x04,
	K_MULTIPLY 		= 0x6a,
	K_NUMPAD0		= 0x60,
	K_NUMPAD1 		= 0x61,
	K_NUMPAD2 		= 0x62,
	K_NUMPAD3 		= 0x63,
	K_NUMPAD4 		= 0x64,
	K_NUMPAD5 		= 0x65,
	K_NUMPAD6 		= 0x66,
	K_NUMPAD7 		= 0x67,
	K_NUMPAD8 		= 0x68,
	K_NUMPAD9 		= 0x69,
	K_PAUSE 		= 0x13,
	K_PAGEUP 		= 0x21,
	K_PAGEDOWN		= 0x22,
	K_RBUTTON 		= 0x02,
	K_ENTER 		= 0x0d,
	K_RIGHT 		= 0x27,
	K_SHIFT 		= 0x10,
	K_SPACE 		= 0x20,
	K_SUBTRACT 		= 0x6D,
	K_TAB 			= 0x09,
	K_UP 			= 0x26,
	K_MENU			= 0x12,
	K_ALT			= 0x12,
	};

//
// Help messages sent to main window:
//
enum EMainWindowCmds
	{
	IDC_HELP_ABOUT		= 32783,
	IDC_HELP_ORDER		= 32790,
	IDC_HELP_ORDERNOW	= 32791,
	IDC_HELP_TOPICS		= 32782,
	IDC_HELP_WEB		= 32792,
	};

/*----------------------------------------------------------------------------
	GUARD mechanism
----------------------------------------------------------------------------*/

//
// GUARD and UNGUARD macros, which are used to display the calling
// stack after appError's and GPF's for debuggerless debugging.
//
extern UNREAL_API FGlobalPlatform *GApp;
extern UNREAL_API void **GLock,*GDummy[16];
inline void VARARGS DO_NOTHINGF(...);

#if defined(DEBUG) || defined(NO_GUARD)
	#define GUARD {
	#define UNGUARD_BEGIN }if(0){
	#define UNGUARD_END };
	#define UNGUARD_MSGF DO_NOTHINGF
	#define UNGUARD(s) };
#else
	#define GUARD			try{
	#define UNGUARD_BEGIN	}catch(...){
	#define UNGUARD_END		GApp->GuardTrap=1; *(char*)NULL=0;};
	#define UNGUARD_MSGF	GApp->GuardMessagef
	#define UNGUARD(s)		}catch(...){UNGUARD_MSGF(s); GApp->GuardTrap=1; *(char*)NULL=0; throw;};
#endif

#ifdef PARANOID
	#define SLOW_GUARD GUARD
	#define SLOW_UNGUARD(s) UNGUARD(s)
#else
	#define SLOW_GUARD
	#define SLOW_UNGUARD(s)
#endif

/*-----------------------------------------------------------------------------
	Convenience macros
-----------------------------------------------------------------------------*/

#define debug				GApp->Log
#define debugf				GApp->Logf
#define appError			GApp->Error
#define appErrorf			GApp->Errorf
#define appMalloc			GApp->Malloc
#define appMallocAligned	GApp->MallocAligned
#define appRealloc			GApp->Realloc
#define appFree				GApp->Free
#define appMallocArray(elements,type,descr) (type *)GApp->Malloc((elements)*sizeof(type),descr)

/*----------------------------------------------------------------------------
	The End
----------------------------------------------------------------------------*/
#endif /* _INC_PLATFORM */
