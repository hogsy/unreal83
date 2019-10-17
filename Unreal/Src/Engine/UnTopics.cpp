/*=============================================================================
	UnTopics.cpp: FTopicTable implementation

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	What's happening: This is how pieces of information are exchanged between
	the editor app and the server.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"

/*-------------------------------------------------------------------------------
	FTopicTable init/exit/register
-------------------------------------------------------------------------------*/

//
// Initialize the topic table by clearing out its linked list.
// Only initializes on the very first call to it, because the topic
// table init function may be called several times during startup.
//
void FGlobalTopicTable::Init (void)
	{
	GUARD;
	static int Started=0;
	if (!Started)
		{
		Started			= 1;
		FirstHandler	= NULL;
		};
	UNGUARD("FGlobalTopicTable::Init");
	};

//
// Shut down the topic table by clearing freeing its linked list.
//
void FGlobalTopicTable::Exit (void)
	{
	GUARD;
	FirstHandler = NULL;
	UNGUARD("FGlobalTopicTable::Exit");
	};

//
// WARNING! This routine is called in the resource type constructors before
// anything has been initialized and before the code has actually begun
// executing.  Anything done here must be 100% safe independent of startup order.
//
void FGlobalTopicTable::Register(const char *TopicName,FTopicHandler *Handler)
	{
	static int Started=0;
	if (!Started)
		{
		Init();
		Started = 1;
		};
	if (TopicName && *TopicName && (strlen(TopicName)<NAME_SIZE))
		{
		//
		// Create a new TopicInfo entry and insert it in the linked list of topics
		//
		strcpy(Handler->TopicName,TopicName);
		Handler->Next			= FirstHandler;
		FirstHandler			= Handler;
		};
	};

/*-------------------------------------------------------------------------------
	FTopicTable Find/Get/Set
-------------------------------------------------------------------------------*/

//
// Find a named topic in the topic table, and return a pointer
// to its handler, or NULL if not found.
//
FTopicHandler *FGlobalTopicTable::Find (const char *TopicName)
	{
	FTopicHandler *Handler = FirstHandler;
	while (Handler)
		{
		if (stricmp(TopicName,Handler->TopicName)==0) return Handler;
		Handler = Handler ->Next;
		};
	return NULL;
	};

void FGlobalTopicTable::Get (ULevel *Level, const char *Topic, const char *Item, char *Data)
	{
	*Data = 0; // Handlers expect data to default to an empty response
	FTopicHandler *Handler = Find(Topic);
	if (!Level) Level = GServer.Levels->Element(0);
	if (Handler) Handler->Get(Level,Topic,Item,Data);
	};

void FGlobalTopicTable::Set (ULevel *Level, const char *Topic, const char *Item, const char *Value)
	{
	FTopicHandler *Handler = Find(Topic);
	if (!Level) Level = GServer.Levels->Element(0);
	if (Handler) Handler->Set(Level,Topic,Item,Value);
	};

/*-------------------------------------------------------------------------------
	The End
-------------------------------------------------------------------------------*/
