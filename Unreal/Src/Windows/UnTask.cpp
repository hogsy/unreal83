/*=============================================================================
	UnTask.cpp: Unreal task manager for Windpws

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "StdAfx.h"
#include "UnWn.h"
#include "Unreal.h"
#include "UnWnCam.h"

extern FWindowsCameraManager CameraManager;

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

//
// Unreal task manager for windows.  Manages all tasks.  Tasks are
// updated via their Tick routines in CUnrealWnApp.MessagePump().
//
class FWindowsTaskManager : public FTaskManager
	{
	public:
	//
	// FTaskManager interface:
	//
	virtual void Init(void);
	virtual void Exit(void);
	virtual int Exec(const char *Cmd,FOutputDevice *Out=GApp);
	//
	virtual FTask *GetTaskByID(int TaskID);
	virtual int AddTask(FTask *Task,FTask *Owner,FOutputDevice *Out,ETaskPriority Priority,int Flags);
	virtual void KillTask(FTask *Task);
	virtual void KillTasksUsing(FOutputDevice *Out);
	//
	// Private implementation:
	//
	int TopTask,Initialized;
	enum {MAX_TASKS=64};
	FTask *Tasks[MAX_TASKS];
	//
	FWindowsTaskManager() {Initialized=0;};
	void AssertInitialized(void) {if (!Initialized) appError("Not initialized");};
	};

FWindowsTaskManager WindowsTaskManager;
FTaskManager *TaskManager = &WindowsTaskManager;

/*-----------------------------------------------------------------------------
	Init & Exit
-----------------------------------------------------------------------------*/

//
// Initialize the task manager.
//
void FWindowsTaskManager::Init(void)
	{
	GUARD;
	//
	if (Initialized) appError("Already initialized");
	//
	for (int i=0; i<MAX_TASKS; i++) Tasks[i]=NULL;
	Initialized=1;
	TopTask=1;
	//
	UNGUARD("FWindowsTaskManager::Init");
	};

//
// Shut down the task manager.  Does not kill any yet-unkilled tasks.
// You are responsible for killing all tasks before shutting down the
// task manager.
//
void FWindowsTaskManager::Exit(void)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_TASKS; i++)
		{
		if (Tasks[i])
			{
			char Name[256],Descr[256];
			debugf(LOG_Task,"Task is still running: %s",Tasks[i]->TaskStatus(Name,Descr));
			};
		};
	Initialized=0;
	//
	UNGUARD("FWindowsTaskManager::Exit");
	};

/*-----------------------------------------------------------------------------
	Command line
-----------------------------------------------------------------------------*/

//
// Execute a command line.
//
int FWindowsTaskManager::Exec(const char *Cmd,FOutputDevice *Out)
	{
	GUARD;
	const char *Str = Cmd;
	AssertInitialized();
	//
	if (GetCMD(&Str,"STATUS") && (GetCMD(&Str,"TASK") || !Str[0]))
		{
		int n=0; for (int i=0; i<MAX_TASKS; i++) if (Tasks[i]) n++;
		//
		Out->Logf("   TASK - Ok, %i/%i tasks",n,TopTask);
		return Str[0]!=0;
		}
	else if (GetCMD(&Str,"HELP"))
		{
		Out->Log("   PS - List all tasks");
		Out->Log("   KILL - Kill a task");
		return 0;
		}
	else if (GetCMD(&Str,"PS"))
		{
		Out->Log("Unreal tasks:");
		for (int i=0; i<MAX_TASKS; i++)
			{
			if (Tasks[i])
				{
				char Name[256],Descr[256];
				Tasks[i]->TaskStatus(Name,Descr);
				Out->Logf(" %i. %s (%s) %3.1f ms",Tasks[i]->TaskID,Name,Descr[0] ? Descr : "Ok",Tasks[i]->TaskTickTime*Cyc2Msec);
				};
			};
		return 1;
		}
	else if (GetCMD(&Str,"KILL"))
		{
		int TaskID = atoi(Str);
		if (TaskID)
			{
			FTask *Task = GetTaskByID(TaskID);
			if (Task)
				{
				if (Task->TaskFlags & TASK_NoUserKill)
					{
					Out->Log("Task is not killable");
					}
				else
					{
					char Name[256],Descr[256];
					Out->Logf("Killing task: %s",Task->TaskStatus(Name,Descr));
					KillTask(Task);
					};
				};
			}
		else Out->Log("Missing or invalid task number");
		return 1;
		}
	else return 0;
	//
	return 0;
	UNGUARD("FWindowsTaskManager::Exec");
	};

/*-----------------------------------------------------------------------------
	Task management
-----------------------------------------------------------------------------*/

//
// Lookup a task by its ID and return a pointer to the task object.
// Returns 0 if the task ID doesn't correspond to a running task.
//
FTask *FWindowsTaskManager::GetTaskByID(int TaskID)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_TASKS; i++)
		{
		if (Tasks[i] && (Tasks[i]->TaskID==TaskID)) return Tasks[i];
		};
	return NULL; // Not found
	//
	UNGUARD("FWindowsTaskManager::GetTaskByID");
	};

//
// Add a newly-created task to the task list.  Assumes that you have already
// initialized the task and that it's ready to receive Tick() calls.
// This doesn't directly call any task functions.  Returns task ID, or 0 if 
// task couldn't be created, which happens only if there are more than 
// MAX_TASKS tasks active.
//
int FWindowsTaskManager::AddTask(FTask *Task,FTask *Owner,FOutputDevice *Out,ETaskPriority Priority,int Flags)
	{
	GUARD;
	AssertInitialized();
	//
	int TaskID = TopTask++;
	//
	for (int i=0; i<MAX_TASKS; i++) if (!Tasks[i]) break;
	if (i>=MAX_TASKS) return 0; 
	//
	Tasks[i]=Task;
	Task->Owner			= Owner;
	Task->Out			= Out;
	Task->Priority		= Priority;
	Task->TaskFlags		= Flags;
	Task->TaskID		= TaskID;
	//
	char Name[256],Descr[256];
	debugf(LOG_Task,"Created task: %s",Task->TaskStatus(Name,Descr));
	return TaskID;
	//
	UNGUARD("FWindowsTaskManager::AddTask");
	};

//
// Kill a task.  Calls the task's TaskExit() function.  This may be
// called at any time, such as if someone kills the task manually
// at a console, so the task's TaskExit() function must be able
// to perform all necessary cleanup.
//
void FWindowsTaskManager::KillTask(FTask *Task)
	{
	GUARD;
	AssertInitialized();
	//
	char Name[256],Descr[256];
	debugf(LOG_Task,"Destroyed task: %s",Task->TaskStatus(Name,Descr));
	//
	for (int i=0; i<MAX_TASKS; i++)
		{
		if (Tasks[i]==Task)
			{
			Task->TaskExit();
			Tasks[i]=NULL;
			return;
			};
		};
	appError("Task not found");
	//
	UNGUARD("FWindowsTaskManager::KillTask");
	};

void FWindowsTaskManager::KillTasksUsing(FOutputDevice *Out)
	{
	GUARD;
	AssertInitialized();
	//
	for (int i=0; i<MAX_TASKS; i++)
		{
		if (Tasks[i] && (Tasks[i]->Out==Out)) KillTask(Tasks[i]);
		};
	UNGUARD("FWindowsTaskManager::KillTasksUsing");
	};

/*-----------------------------------------------------------------------------
	Tasking Kernel & Windows Message Pump
-----------------------------------------------------------------------------*/

//
// Unreal's main message pump.  All windows in Unreal receive messages
// somewhere below this function on the stack.  This is tuned to keep
// the UnrealServer executing at a constant rate while rendering the screen 
// at a variable frame rate and processing all player input to achieve
// maximum responsiveness.
//
// This also doubles as Unreal's tasking kernel.
//
void CUnrealWnApp::MessagePump(void)
	{
	GUARD;
	//
	int Ticks=0;
	while (1)
		{
		MSG Msg;
		while
			(
			PeekMessage(&Msg,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE) ||
			PeekMessage(&Msg,NULL,WM_LBUTTONDOWN,WM_MBUTTONDBLCLK,PM_REMOVE) ||
			PeekMessage(&Msg,NULL,WM_QUIT,WM_QUIT,PM_REMOVE) ||
			PeekMessage(&Msg,NULL,WM_QUIT,WM_MOUSEMOVE,PM_REMOVE)
			)
			{
			if (Msg.message!=WM_QUIT) RouteMessage(&Msg);
			else return;
			//
			if (Msg.message==WM_MOUSEMOVE) break;
			};
		int GotMessage;
		if (CameraManager.FullscreenCamera || Ticks)
			{
			GotMessage = PeekMessage(&Msg,NULL,0,0,PM_REMOVE);
			}
		else
			{
			if (!GetMessage(&Msg,NULL,0,0)) return;
			GotMessage=1;
			};
		if (GotMessage)
			{
			if (Msg.message==WM_QUIT)
				{
				return;
				}
			else if (Msg.message==WM_UNREALTIMER)
				{
				int Throttle=0;
				do	{
					if (++Throttle < 6)
						{
						for (int i=0; i<WindowsTaskManager.MAX_TASKS; i++)
							{
							FTask *Task = WindowsTaskManager.Tasks[i];
							if (Task && (Task->Priority==PRIORITY_Realtime))
								{
								Task->TaskTickTime=0;
								ALWAYS_BEGINTIME(Task->TaskTickTime);
								Task->TaskTick();
								ALWAYS_ENDTIME(Task->TaskTickTime);
								};
							};
						Ticks++;
						};
					} while (PeekMessage(&Msg,NULL,WM_UNREALTIMER,WM_UNREALTIMER,PM_REMOVE));
				if (CameraManager.CursorIsCaptured()) goto DoCameras;
				}
			else RouteMessage(&Msg);
			}
		else if (Ticks && !CameraManager.CursorIsCaptured())
			{
			DoCameras:
			//
			// Ticks>=1, and it represents the number of server ticks that have
			// passed before this camera tick.
			//
			for (int i=0; i<WindowsTaskManager.MAX_TASKS; i++)
				{
				FTask *Task = WindowsTaskManager.Tasks[i];
				if (Task && (Task->Priority==PRIORITY_Camera))
					{
					Task->TaskTickTime=0;
					ALWAYS_BEGINTIME(Task->TaskTickTime);
					Task->TaskTick();
					ALWAYS_ENDTIME(Task->TaskTickTime);
					};
				};
			Ticks=0;
			};
		};
	UNGUARD("CUnrealWnApp::MessageLoop");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
