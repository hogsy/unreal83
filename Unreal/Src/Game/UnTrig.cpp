/*=============================================================================
	UnTrig.cpp: Trigger classes

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "UnGame.h"
#include "UnFActor.h" 

/*-----------------------------------------------------------------------------
	Triggers base class
-----------------------------------------------------------------------------*/

int ATriggers::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	return 0;
	UNGUARD("ATriggers::Process");
	};

/*-----------------------------------------------------------------------------
	Trigger
-----------------------------------------------------------------------------*/

int ATrigger::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
    FActor & Actor = FActor::Actor(*this);
	switch (Message.Index)
		{
		case ACTOR_Tick:
			// Does nothing
			return 1;
		case ACTOR_Touch:
			{
			if (EventName!=NAME_NONE)
				{
				Level->SendMessageEx(ACTOR_Trigger,Params,INDEX_NONE,EventName,NULL);
				};
			if (bShowMessage)
				{
				PText Msg;
				Msg.MsgType = LOG_Play;
				sprintf(Msg.Message,this->Message);
				Level->SendMessage(*(INDEX *)Params,ACTOR_TextMsg,&Msg);
				};
			if (bTriggerOnceOnly) Actor.SetActorCollision(FALSE);
			return 1;
			};
		case ACTOR_UnTouch:
			{
			if (EventName!=NAME_NONE)
				{
				PTouch TouchParams;
				TouchParams.iActor = iMe;
				Level->SendMessageEx(ACTOR_UnTrigger,&TouchParams,INDEX_NONE,EventName,NULL);
				};
			return 1;
			};
		};
	return 0;
	UNGUARD("ATrigger::Process");
	};

/*-----------------------------------------------------------------------------
	Counter
-----------------------------------------------------------------------------*/

int ACounter::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	switch (Message.Index)
		{
		case ACTOR_Tick:
			return 1;
		case ACTOR_Trigger:
			if (NumToCount>0)
				{
				PText Msg;
				Msg.MsgType = LOG_Play;
				if (--NumToCount > 0)
					{
					char Temp[80]; sprintf(Temp,"%i",NumToCount);
					if		(NumToCount==1)	sprintf(Msg.Message,CountMessage,"one");
					else if	(NumToCount==2)	sprintf(Msg.Message,CountMessage,"two");
					else if	(NumToCount==3)	sprintf(Msg.Message,CountMessage,"three");
					else if	(NumToCount==4)	sprintf(Msg.Message,CountMessage,"four");
					else if	(NumToCount==5)	sprintf(Msg.Message,CountMessage,"five");
					else if	(NumToCount==6)	sprintf(Msg.Message,CountMessage,"six");
					else					sprintf(Msg.Message,CountMessage,Temp);
					}
				else
					{
					sprintf(Msg.Message,CompleteMessage);
					if (EventName!=NAME_NONE)
						{
						Level->SendMessageEx(ACTOR_Trigger,Params,INDEX_NONE,EventName,NULL);
						};
					};
				if (bShowMessage) Level->SendMessage(*(INDEX *)Params,ACTOR_TextMsg,&Msg);
				};
			return 1;
		};
	return 0;
	UNGUARD("ACounter::Process");
	};

/*-----------------------------------------------------------------------------
	Triggers base class
-----------------------------------------------------------------------------*/

int ADispatcher::Process(ILevel *Level, FName Message, void *Params)
	{
	GUARD;
	return 0;
	UNGUARD("ADispatcher::Process");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
