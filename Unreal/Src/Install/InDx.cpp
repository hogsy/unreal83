/*=============================================================================
	InDX.cpp: Unreal DirectX install

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Based on SetupDX2.cpp from Intel
		* Expanded by Tim Sweeney
=============================================================================*/

#include <windows.h>
#include "DSetup.h"    /* From the DirectX(tm) SDK */

/*-----------------------------------------------------------------------------
	DirectX setup caller
-----------------------------------------------------------------------------*/

//
// Description:
//   This program calls the DirectXSetup function, and returns the return 
//   value from this function call.  This program interprets the return
//   code passed back from the function, and informs the user if an error
//   occurred.  If a reboot is required, the user is informed, and
//   given the option to reboot.  If the installation is successful, and
//   no errors occurred, then the program terminates silently.
//
// Arguments:   
//   Requires one argument, which specifies the full path to the DirectX(tm)
//   redistribution files.  If this argument is missing or incorrect, an error
//   message is displayed.  These error messages boxes are intended to assist
//   the ISV as this program is integrated into their SETUP utility.  Once it
//   is properly integrated, these messages will never be seen by the end user.
//
// Linking instructions:
//   This file must be linked with DSETUP.LIB, which is included in the DirectX
//   SDK.
//   
// Required DLLs (must be in same directory as the .EXE, or in the PATH):
//   DSETUP.DLL
//   DSETUP6E.DLL
//   DSETUPE.DLL
//   

/*-----------------------------------------------------------------------------
	Constants
-----------------------------------------------------------------------------*/

const char NoArgumentErrorMsg[] =
       "ERROR:  This program must be called with one argument, specifying the "\
       "directory containing the DirectX(tm) redistribution files.\n\n"\
       "Please try again.";

const char SuccessNeedRestartMsg[] = 
      "DirectX(tm) installation Successful!\n\nYour system must be restarted before "\
      "the changes will take effect.\n\nWould you like to restart now?";

const char BadWinVerMsg[] = 
      "This application requires DirectX(tm) support.\n\nThe Windows version on your "\
      "system is not supported by DirectX."; 

const char OutOfDiskSpaceMsg[] = 
      "ERROR:  The setup program ran out of disk space during installation.\nPlease "\
      "delete unnecessary files and start SETUP again.";

const char BadLocationErrorMsg[] = 
      "ERROR:  Incorrect directory location specified for the DirectX(tm) "\
      "redistribution files.\n\nLocate the REDIST directory, and specify the DIRECTX "\
      " directory underneath that.\n\n"\
      "Example --> D:\\REDIST\\DIRECTX\n\nPlease try again.";

char ErrorCodeMsg[256] = 
      "Internal Error: DirectX(tm) Setup failed.\nReturn code:  ";

char YouGottaRebootLater[] =
	  "DirectX was installed successfully!  Before you can use DirectX, you must shut "
	  "down your computer, then restart it.  You should do this after exiting Setup.";

/*-----------------------------------------------------------------------------
	WinMain
-----------------------------------------------------------------------------*/

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, int nCmdShow)
	{
	//
	// Make sure that the location of the DirectX redistribution files
	// was passed as an argument
	//
	if(*lpCmdLine == NULL)
		{
		MessageBox(hInstance, NoArgumentErrorMsg, NULL, MB_OK | MB_SETFOREGROUND);
		return -1;
		};
	//
	// Call DirectXSetup to install all DirectX components:
	//
	int returnCode = DirectXSetup(hInstance, lpCmdLine, DSETUP_DIRECTX);	
	if (returnCode==0)
		{
		return 0;  // if no reboot is required, just continue silently
		}
	else if(returnCode == 1)
		{
		//
		// Reboot is required
		//
		MessageBox(hInstance, YouGottaRebootLater, NULL, MB_OK | MB_SETFOREGROUND);
#if 0 /* Buggy Intel example code */
		int answer = MessageBox
			(
			hInstance, SuccessNeedRestartMsg,"Installation Successful", 
			MB_YESNO | MB_SETFOREGROUND
			);
		if(answer == IDYES)
			{
			//
			// Reboot the system: Set shutdown privelage and reboot.
			//
			HANDLE hToken; 
			TOKEN_PRIVILEGES tkp; 
			//
			// Get a token for this process
			//
			if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
				{
				MessageBox(hInstance, "Error in OpenProcessToken", NULL, MB_OK | MB_SETFOREGROUND);
				return 1;
				};
			//
			// Get the LUID for the shutdown privilege
			//
			LookupPrivilegeValue(NULL, TEXT("SE_SHUTDOWN_NAME"),&tkp.Privileges[0].Luid); 
			//
			tkp.PrivilegeCount = 1;  // one privilege to set
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
			//
			// Get the shutdown privilege for this process.
			//
			AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
			//
			// Cannot test the return value of AdjustTokenPrivileges
			//
			if (GetLastError() != ERROR_SUCCESS)
				{
				MessageBox(hInstance, "Error in AdjustTokenPrivileges", NULL, MB_OK | MB_SETFOREGROUND);
				return 1;
				};
			//
			// Shut down the system
			//
			if (!ExitWindowsEx(EWX_REBOOT, 0)) 
				{
				MessageBox(hInstance, "Error in ExitWindowsEx", NULL, MB_OK | MB_SETFOREGROUND);
				return 1;
				};
			return 0; // Should never get here
			};
#endif

		}
	else if(returnCode == DSETUPERR_BADWINDOWSVERSION) // Windows version does not support DirectX
		{
		MessageBox
			(
			hInstance,  BadWinVerMsg, "Error", 
            MB_ICONEXCLAMATION | MB_SETFOREGROUND
			);
		}
	else if(returnCode == DSETUPERR_OUTOFDISKSPACE)
		{
		MessageBox
			(
			hInstance, OutOfDiskSpaceMsg,
			"Error", MB_ICONEXCLAMATION | MB_SETFOREGROUND
			);
		}
	else if(returnCode == DSETUPERR_CANTFINDINF || returnCode == DSETUPERR_CANTFINDDIR)
		{
		MessageBox
			(
			hInstance, BadLocationErrorMsg,
			"Error", MB_ICONEXCLAMATION | MB_SETFOREGROUND
			);
		}
	else  
		{
		char temp[10];
		itoa(returnCode, temp, 10);
		strcat(ErrorCodeMsg, temp);
		MessageBox
			(
			hInstance, ErrorCodeMsg, "Error",MB_ICONEXCLAMATION | MB_SETFOREGROUND
			);
		};
	return returnCode; 
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
