/*=============================================================================
	!ReadMe.txt: Unreal source distribution notes
	By: Tim Sweeney, Epic MegaGames, Inc.
	Date: 10-1-96
=============================================================================*/

This is a confidential, internal source code distribution of Unreal, and is
a trade secret of Epic MegaGames, Inc.  It is intended only for people who have
signed the appropriate non-disclosure agreements.

--------------------------------------
For the latest development information
--------------------------------------

Visit http://unreal.epicgames.com/
Please read all of the latest info there before asking a question.
Check out the list of contacts there to find the approprite person to question.

------------
This package
------------

Version:		Unreal 0.83.  See UnBuild.h for detailed version information.
Included:		Server, Engine, Render, Game, Networking, Launcher, Editor

--------------------
Development Platform
--------------------

Unreal source, Windows 95/NT, requires:
   * Visual C++ 4.0, 4.1, or 4.2
   * MASM 6.11d (from \Unreal\Src\Tools), ML.EXE must be in path

UnrealEd, Windows 95/NT, requires:
   * Visual Basic Professional or Enterprise 4.0

--------------------
Install instructions
--------------------

1. Install the corresponding game/demo version of Unreal into \Unreal.
   Run Unreal.exe and verify that it works properly.

2. Optionally install UnrealEd and any auxillary distributions into \Unreal.

3. Install this source distribution into \Unreal.
   To verify that it works, delete the .exe and .dll files in \Unreal and \Unreal\System,
   then build all of the source files here and run Unreal.exe again.

-------------------
Documentation Index
-------------------

	File			Contents
	--------------- -------------------------------------------------------
	!ReadMe.txt		This index
	Actions.txt     Description of the input system and input actions
	Amff.txt		Galaxy sound system AMFF file format docs
	Galaxy.txt		Galaxy docs
	ProjMan.txt		Important source code guidelines to be aware of
	Script.txt		UnrealScript preliminary docs & samples
	T3d.txt			Unreal brush .T3D file format docs
	TaskList.txt	List of major tasks to be done
	TimLog.txt		Tim Sweeney work log
	TimNotes.txt	Tim Sweeney misc notes (confidential)
	TimOld.txt		Tim Sweeney old work logs

----------------
Unreal.mak Index
----------------

	Project		   Description             Target			            Compatibility
	-------------  ----------------------  ---------------------------  ----------------------------
	Documentation  Documentation files	   (none)                       (none)
	Editor		   UnrealEd support files  \Unreal\System\UnEditor.dll	Ansi C++
	Engine		   All engine source code  \Unreal\System\UnEngine.dll	Ansi C++
	Game		   Game & AI routines      \Unreal\System\UnGame.dll	Ansi C++
	Network		   All network code		   \Unreal\System\UnNet.dll     Ansi C++ & Windows API
	Render		   Rendering subsystem	   \Unreal\System\UnRender.dll	Ansi C++
	Windows		   Windows-specific code   \Unreal\System\UnServer.exe	Ansi C++ & MFC & Windows API

---------------
Utils.mak Index
---------------

	Project		   Description             Target			            Compatibility
	-------------  ----------------------  ---------------------------  --------------------
	Launcher	   Unreal launcher stub	   \Unreal\Unreal.exe           Ansi C & Windows API
	Install		   Installation utilities  \Unreal\System\Install.exe   Ansi C & Windows API

---------------
Directory Index
---------------

Assuming you installed into the \Unreal\ directory, here are the 
subdirectories you'll find the Unreal source code in:

From the Unreal source distribution:

\Unreal\						Root Unreal directory
	\Unreal\Doc\				Documentation files directory
		!ReadMe.txt				This index file
	\Unreal\Src\				Root Unreal source code directory
		\Unreal\Src\Game\		Game code for UnActors.dll
			Unreal.mak			Visual C++ project file for Unreal
		\Unreal\Src\Engine\		Game engine (client & server) for UnEngine.dll
			Unreal.cpp			Main startup file
		\Unreal\Src\Inc\		Include files directory
			Unreal.h			Main Unreal include
		\Unreal\Src\Network\	Windows networking code
		\Unreal\Src\Render\		Rendering code
		\Unreal\Src\Editor\		Unreal editor support files
		\Unreal\Src\Windows\	All windows-specific code for Unreal
		\Unreal\Src\Launcher\	Unreal launcher stub
		\Unreal\Src\Install\	Install utility
		\Unreal\Src\Listing\	VC++ generated assembly listing files
	\Unreal\EdSrc\				Root UnrealEd source code directory
		UnrealEd.vbp			Visual Basic 4.0 project file for UnrealEd.exe
	\Unreal\Classes\			Actor class descriptions
		Root.tcx				Root actor class description
		Classes.mac				UnrealEd macro to import all actor classes

From auxillary Unreal distributions:

	\Unreal\Graphics\			Ancillary game and editor graphics, compiled to \Unreal\System\Unreal.gfx
		Graphics.mac			UnrealEd macro that imports all graphics resources

	\Unreal\Models\				All 3D models and their associated texture maps referenced by Root.tcx
		Models.mac				UnrealEd macro that imports all models

-----------------------
VC++ Build instructions
-----------------------

You need to build the Unreal files in this order due to dependencies:

1. Build "Engine Files".
2. Build "Render Files".
3. Build "Editor Files", "Network Files" and "Game Files".
4. Build "Windows Files".

----------------------
Project configurations
----------------------

Release: Optimized version of Unreal for general development use.

Debug: Debug version of Unreal, for use in the VC++ debugger.  Note that the debug version is
too damn slow to be usable for non-debugging purposes.

We hardly ever use the debugger while working on Unreal.  The GUARD/UNGUARD mechanism we use
shows the calling history whenever a crash or critical error occurs, so we track down errors
from the release version.  It should also be noted that Visual C++'s debugger is highly unstable
and tracking down bugs in it is an interesting lesson in the Heisenberg principle.

-----------------
Completion status
-----------------

Completion status of major features:

Feature          %%% Notes
---------------- --- -----------------------------------------------------------
UnrealEd		 90% Texture properties, level properties, tweaking remain.
Rendering		 75% Lighting tweaking/adjustment and major optimization remain.
Scripting		 50% Compiles and parses most items; executor not started.
Net Play		 20% Low level code started, no high level code.
Moving Brushes	 80% Tweaking & extra features remain.
Particle systems 20% Totally gross, hacked code.
Input            80% Not included in build 0.76. Early (25%) version included.
Audio            60% Not included in build 0.76. Early (25%) version included.

Known problems
	Actors are only rendered in 8-bit color; invisible in higher color depths.
	16- and 24-bit color - implementation is limited.
	Actor performance problems.
	Debug version shutdown code crashes due to known memory leak.
	Unreal allocates about 200 megs of virtual memory for game/editing use regardless
		of level size, so you need lots of hard disk space free.  This will be reduced
		to a reasonable size by 0.80 (this was a programming convenience).
	No network play.
	UnrealEd texture properties - not functioning.
	UnrealEd sound and ambient sound properties - not functioning.
	Rendering and AI not fully optimized.
	Music/sound effects are limited.
	Teleporters work in same level only.
	No load game/save game.
	Player collision is working funny; you can get snagged on walls/edges sometimes.

See TimLog.txt and TimOld.txt for a day-by-day account of the details.

------------
Legal notice
------------

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret, and
	all distribution is prohibited.

	All source code, programming techniques, content, and algorithms incorporated
	herein are trade secrets of Epic MegaGames, Inc., including, but not limited to
	the following:

	* All specific texture mapping concept and algorithms
	* All texture and light filtering/dithering concepts and algoritms
	* Lattice-based rendering concept
	* Lattice light generation concept and algorithm
	* GUARD/UNGUARD call history error logging facility
	* UnrealEd interactive constructive solid geometry paradigm and algorithms
	* Realtime BSP maintenance algorithm
	* Zone and portal based occlusion algorithms
	* Unreal resource manager object-oriented load/save/link/delink algorithms

Enjoy.

-Tim
