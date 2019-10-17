/*=============================================================================
	UnScript.h: UnrealScript compiler

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#ifndef _INC_UNSCRIPT
#define _INC_UNSCRIPT

/*-----------------------------------------------------------------------------
	Enums (compiler, executor)
-----------------------------------------------------------------------------*/

//
// Sizes:
//
enum {MAX_NEST_LEVELS		= 16};
enum {MAX_IDENTIFIER_SIZE	= NAME_SIZE};
enum {MAX_STRING_CONST_SIZE	= 256};
enum {MAX_VARS				= UClass::MAX_CLASS_PROPERTIES};
enum {MAX_FUNCS				= 512};
enum {MAX_STACK_DEPTH		= 16};
enum {MAX_STACK_TREE		= 1024};

//
// Script compiler enums:
//
enum ETokenType				//	Types of parsable tokens
	{
	TOKEN_End				= 0x00,		// End of line
	TOKEN_Identifier		= 0x01,		// Alphanumeric identifier
	TOKEN_Symbol			= 0x02,		// Symbol
	TOKEN_Variable			= 0x03,		// A variable, only set after IsVariable()
	TOKEN_IntegerConst		= 0x04,		// Integer constant
	TOKEN_FloatConst		= 0x05,		// Floating point constant
	TOKEN_StringConst		= 0x06,		// String constant
	TOKEN_ResourceConst		= 0x07,		// A resource constant
	TOKEN_NameConst			= 0x08,		// A name constant
	TOKEN_RotationConst     = 0x09,		// A rotation constant
	TOKEN_VectorConst       = 0x0A,		// A vector constant
	TOKEN_Max				= 0x0B
	};
enum ENestType				// Code nesting type
	{
	NEST_None				=0x0000,	//  No nesting
	NEST_Class				=0x0001,	//  Class/EndClass
	NEST_State				=0x0002,	//	State/EndState
	NEST_When				=0x0003,	//	Event/EndEvent
	NEST_Function			=0x0004,	//	Function/EndFunction
	NEST_Operator			=0x0005,	//	Operator/EndOperator
	NEST_If					=0x0006,	//  If/ElseIf/EndIf
	NEST_Loop				=0x0007,	//  While/Do/Loop/Until
	NEST_Select				=0x0008,	//  Select Case
	NEST_Case				=0x0009,	//  Case
	NEST_For				=0x000A,	//  For
	NEST_Max				=0x000B
	};
enum ENestAllowBits			// Types of statements to allow within a particular nesting block
	{
	ALLOW_SlowCmd			= 0x0001,	// Allow commands that take >0 cycle time
	ALLOW_Cmd				= 0x0002,	// Allow commands that take 0 cycle time
	ALLOW_When				= 0x0004,	// Allow Event declarations at this level
	ALLOW_Function			= 0x0008,	// Allow Event declarations at this level
	ALLOW_State				= 0x0010,	// Allow State declarations at this level
	ALLOW_ElseIf			= 0x0020,	// Allow ElseIf commands at this level
	ALLOW_Until				= 0x0040,	// Allow EndWhile/Loop/Until commands at this level
	ALLOW_VarDecl			= 0x0080,	// Allow variable declarations at this level
	ALLOW_Class				= 0x0100,	// Allow class definition heading
	ALLOW_Case				= 0x0200,	// Allow 'case' statement
	ALLOW_Default			= 0x0400,	// Allow 'default' case statement
	ALLOW_Return			= 0x0800,	// Allow 'return' within a function
	};
enum EExprToken				// Evaluatable expression item types
	{
	EX_Nothing				= 0x00,		// No expression (end-of-list or no-expression indicator)
	EX_LocalVariable		= 0x01,		// A variable on the stack (during function calls)
	EX_GlobalVariable		= 0x02,		// A global variable of the actore
	EX_ArrayElement			= 0x03,		// Precedes EX_LocalVariable or EX_GlobalVariable for array elements
	EX_Function				= 0x04,		// A function call with parameters
	EX_FastFunction			= 0x05,		// A prebound function call with parameters
	EX_ContextOverride		= 0x06,		// Actor reference context for next subexpression, overrides Me
	EX_IntegerConst			= 0x07,		// Integer constant
	EX_RealConst			= 0x08,		// Floating point constant
	EX_StringConst			= 0x09,		// String constant
	EX_ResourceConst		= 0x0A,		// A resource constant
	EX_NameConst			= 0x0B,		// A name constant
	EX_RotationConst		= 0x0C,		// A rotation constant
	EX_VectorConst			= 0x0D,		// A vector constant
	EX_Error				= 0x0E,		// Crash with an error when encountering this
	EX_Max					= 0x0F,		// Maximum allowable expression item
	};
enum EStackNodeFlags		// Information about a stack node
	{
	SNODE_Intrinsic			= 0x01,		// This stack node represents an intrinsic function
	SNODE_Auto				= 0x02,		// This is the automatic (default) state
	SNODE_Editable			= 0x04,		// This state should be settable in UnrealEd
	SNODE_Fast				= 0x08,		// A prebindable, non-overridable function, event, or operator
	SNODE_AutoConversion	= 0x10,		// For unary conversion operators: conversion automatically applied where needed
	};

/*-----------------------------------------------------------------------------
	Code tokens
-----------------------------------------------------------------------------*/

enum ECodeToken							// Built-in script tokens
	{
	SC_End					= 0x00,		// End of code block
	SC_Let					= 0x01,		// Assign a value to a variable
	SC_Function				= 0x02,		// A function call
	SC_For					= 0x03,		// For-Next loop
	SC_While				= 0x04,		// While/Do-Loop/Until loop
	SC_Select				= 0x05,		// Select-case
	SC_GotoCode				= 0x06,		// Goto a local address in code
	SC_GotoState			= 0x07,		// Goto a state
	SC_If					= 0x08,		// If/ElseIf/Else/EndIf
	SC_Error				= 0x09,		// Halt with an error
	SC_Test					= 0x0A,		// Test command for debugging
	SC_TestEval				= 0x0B,		// Test expression evaluator for debugging
	SC_Max					= 0x0C,		// Maximum allowable token
	};

/*-----------------------------------------------------------------------------
	FToken (compiler)
-----------------------------------------------------------------------------*/

//
// Information about a token that was just parsed.
//
class UNREAL_API FToken
	{
	public:
	//
	int			StartPos;		// Starting position in script where this token came from
	int			StartLine;		// Starting line in script
	int			LookedUpName;	// 1 if we performed a name lookup
	ETokenType	Type;			// type of token
	FName		Name;			// Valid if TOKEN_NameConst, or if not NAME_NONE
	char		*Descr(void);
	union
		{
		char	Identifier[MAX_IDENTIFIER_SIZE];	// If TOKEN_Identifier or TOKEN_Symbol
		int		Integer;							// If TOKEN_IntegerConst
		FLOAT	Float;								// If TOKEN_FloatConst
		char	String[MAX_STRING_CONST_SIZE];		// If TOKEN_StringConst
		FRotation Rotation;							// If TOKEN_RotationConst
		FVector Vector;								// If TOKEN_VectorConst
		struct										// If TOKEN_ResourceConst
			{
			EResourceType	ResType;
			UResource		*Res;
			};
		};
	int inline Matches(char *Str)
		{
		return ((Type==TOKEN_Identifier)||(Type==TOKEN_Symbol)) && !stricmp(Identifier,Str);
		};
	int inline GetFloat(FLOAT *f)
		{
		if (Type==TOKEN_FloatConst)    {*f=Float; return 1;};
		if (Type==TOKEN_IntegerConst)  {*f=(FLOAT)Integer; return 1;};
		return 0;
		};
	int inline GetString(char **s)
		{
		if (Type==TOKEN_StringConst)   {*s = String; return 1;};
		return 0;
		};
	int inline GetName(FName *n)
		{
		if (Type==TOKEN_NameConst)     {*n = Name; return 1;};
		return 0;
		};
	int inline GetVector(FVector *v)
		{
		if (Type==TOKEN_VectorConst)   {*v = Vector; return 1;};
		return 0;
		};
	int inline GetRotation(FRotation *r)
		{
		if (Type==TOKEN_RotationConst) {*r = Rotation; return 1;};
		return 0;
		};
	int inline GetResource(UResource **r,EResourceType DesiredResType)
		{
		if ((Type==TOKEN_ResourceConst) && (ResType==DesiredResType)) {*r = Res; return 1;};
		return 0;
		};
	int GetInteger(int *i);
	int LookupName(void);
	};

/*-----------------------------------------------------------------------------
	FStackNode (compiler, executor)
-----------------------------------------------------------------------------*/

//
// Specifies a class and a stack node index which completely determines a
// linkage to a particular stack node in a script.  This is used for linked
// lists of stack nodes, where the linked list can contain multiple nodes
// from multiple scripts.
//
class UNREAL_API FStackNodeLink
	{
	public:
	UScript	*Script;	// Script the stack node resides in, NULL=empty entry
	INT		iNode;		// Node index into class's script's stack tree
	//
	void inline Init(void)
		{
		Script = NULL;
		iNode  = 0;
		};
	void inline Set(UScript *NewScript,INT iNewNode)
		{
		Script = NewScript;
		iNode  = iNewNode;
		};
	};

//
// A tree of all possible stack levels (functions, events, code blocks) as stored
// in a script resource.  Node 0 is always defined in each script and it represents
// the root of that script's stack tree.  Except in the root script, stack nodes
// are always linked to stack nodes in other scripts' stack trees.
//
// Starting at the root of a script's stack tree, one can traverse the tree and
// find all functions and states that are accessible to actors of that class.  
// This will include functions and states defined in the script itself, as well 
// as functions and scripts defined in all parent scripts.
//
// As a script executes, execution can go back and forth between that script's code,
// and code defined in all of its parent classes.  However, only the primary script's
// stack tree is used as the sole index/reference to which overridden functions are
// where.
//
class UNREAL_API FStackNode
	{
	public:
	//
	// ChildFunctions and ChildStates are indices to first item of each type (function, state)
	// of each type.  Used in traversing the linked list when a function is called,
	// a state is set, or the script it decompiled/qureried.  0 indicates no child.
	//
	// iNext is an index to next peer scope link, or iNext==0 if either (1) this is the
	// last item in the linked list, or (2) this is element 0 in the scope table,
	// i.e. the global scope.
	//
	FStackNodeLink ChildFunctions,ChildStates,Next;
	//
	// Information about class properties in this scope:
	//
	INT			PropertiesOffset;		// Offset of property list in script's raw data, 0 if this is the global scope.
	INT			NumProperties;			// Total number of properties (params + regular variables declared on this scope level).
	INT			DataOffset;				// Offset of property data in script's raw data, 0 if this is the global scope.
	INT			DataLength;				// Number of bytes in scope data
	INT			CodeStart;				// Offset of start of code in script's code bin, or 0=no code.
	ENestType	NestType;				// Type of this scope block.
	FName		Name;					// Name if function/when, NAME_NONE otherwise.
	BYTE		StackNodeFlags;			// EStackNodeFlags
	BYTE		OperParams;				// If operator, number of parameters (1=unary, 2=binary)
	BYTE		OperType;				// If operator, data type (of all properties and return type)
	BYTE		OperPrecedence;			// Operator precedence
	//
	void Init(ENestType NestType,FName Name); // Init a node in the stack tree
	void QueryReferences(UResource *Res,FResourceCallback &Callback, DWORD ResourceContextFlags);
	};

/*-----------------------------------------------------------------------------
	FNestInfo (compiler)
-----------------------------------------------------------------------------*/

//
// Information for a particular nesting level.
//
class UNREAL_API FNestInfo 
	{
	public:
	ENestType		NestType;		// Statement that caused the nesting
	INT				Allow;			// Types of statements to allow at this nesting level
	INT				DenyIntrinsic;	// Types that were denied because the thing was intrinsic
	FStackNode		*Node;			// Node in the stack tree
	FClassProperty  *Properties;	// Temporary property list, moved during PopNest
	BYTE			*Data;			// Initialized data for properties
	BYTE			*StartCodeTop;	// Code pointer at start of nesting, for internal bug checking only
	};

/*-----------------------------------------------------------------------------
	Execution classes
-----------------------------------------------------------------------------*/

//
// Describes the exact state an actor is in.  If NumStates==0, the actor
// isn't in any particular state.  The array is due to the fact that actors
// can have nested states, substates, sub-substates, etc.
//
class UNREAL_API FState
	{
	enum {MAX_STATES=8};
	int		NumStates;						// Number of active states (0 to MAX_STATES-1)
	FName	StateName[MAX_STATES];			// Names of states and substates
	// Need FStackNodeLink
	};

class UNREAL_API FStackEntry
	{
	public:
	INT		iStackScope;	// Index of this stack scope into script's stack scope table
	INT		iFirstProperty;	// Index of first class property in script's class property table
	INT		LocalIP;		// Local instruction pointer for 'live' execution at this stack level
	INT		LocalTop;		// Offset of top of for/while
	};

//
// State information of a script that exists while the script is executing during a
// particular call to ::Process.  This information is tracked reentrantly, so that 
// scripts can recursively call other script code as well as C++ code.
//
// One permanent FScriptExecutionThread is stored in ARoot and is permanently tracked 
// for that script.  This is the script's main execution thread and is used for all
// non-polled, variable-time, thread-based execution.  The permanent FScriptExecutionThread
// remains active across multiple class to ::Process, which calls may return to C++ while
// script execution continues executing variable-time script instructions in the background.
//
// Multiple temorary FScriptExecutionThreads may be created for an actor during calls 
// to the actor's functions (both private functions by the actor itself, and public 
// functions by the actor or other actors).  Temporary execution threads may only 
// execute zero-time instructions, and must return to an iStack==0 state at the end
// of the call to ::Process.
//
class UNREAL_API FScriptExecutionThread
	{
	public:
	INT				iStack;						// Stack level, 0=global level
	INT				IsPermanent;				// This is the actor's permanent call state
	FStackEntry		Stack[MAX_STACK_DEPTH];		// The execution stack
	};

/*
Root class script-related properties:
	bBlockWhen
	bHoldWhen
	BlockMsg[16]?
*/

/*-----------------------------------------------------------------------------
	FRetryPoint (compiler)
-----------------------------------------------------------------------------*/

//
// A point in the script compilation state that can be set and returned to
// using InitRetry() and PerformRetry().  This is used in cases such as testing
// to see which overridden operator should be used, where code must be compiled
// and then "undone" if it was found not to match.
//
// Retries are not allowed to cross command boundaries (and thus nesting 
// boundaries).  Retries can occur across a single command or expressions and
// subexpressions within a command.
//
class FRetryPoint
	{
	public:
	int			InputPos;
	int			InputLine;
	BYTE		*CodeTop;
	};

/*-----------------------------------------------------------------------------
	FScriptCompiler (compiler)
-----------------------------------------------------------------------------*/

//
// Script compiler class
//
class UNREAL_API FScriptCompiler // Script compiler global variables
	{
	public:
	//
	// Variables:
	//
	UClass		*Class;						// Actor class info while compiling is happening
	UScript		*Script;					// Script we're generating
	UTextBuffer	*ErrorText;					// Error text buffer
	FMemPool	*Mem;						// Pool for temporary allocations
	//
	char		*Input;						// Input text
	int			InputSize;					// Total length of input buffer
	int			InputPos;					// Current position in text
	int			InputLine;					// Current line in text
	int			PrevPos;					// Position previous to last GetChar() call
	int			PrevLine;					// Line previous to last GetChar() call
	int			StatementsCompiled;			// Number of statements compiled
	int			LinesCompiled;				// Total number of lines compiled
	int			Decompile;					// Decompile when done?
	//
	// Stack tree:
	//
	FStackNode		*StackTree;				// Tree of all stack paths
	//
	// All local variables:
	//
	int				TotalLocalProperties;
	int				TotalLocalDataSize;
	FClassProperty	*LocalProperties;
	BYTE			*LocalData;
	//
	// Nesting:
	//
	int				NestLevel;				// Current nest level, starts at 0
	FNestInfo		*TopNest;				// Current nesting level
	FNestInfo		Nest [MAX_NEST_LEVELS];	// Information about all nesting levels
	//
	// Code:
	//
	BYTE			*CodeStart;				// Code pool during compilation;
	BYTE			*CodeTop;				// Top of code pool
	//
	// Functions:
	//
	void			InitMake		(void);
	void			ExitMake		(int Success);
	int				CompileStatement(void);
	int				Compile			(UClass *Class,FMemPool *Mem,int ActorPropertiesAreValid);
	int				GetToken		(FToken &Token);
	int				GetIdentifier	(FToken &Token);
	int				GetSymbol		(FToken &Token);
	int				GetIntegerConst	(int *Result);
	int				GetFloatConst   (FLOAT *Result);
	int				GetConstExpr	(FToken &Token,EClassPropertyType *Type);
	int				GetVariable		(FToken &Token, INT *iVar, INT *IsLocal, FClassProperty **VarProperty,EClassPropertyType *Type);
	int				GetFunctionExpr	(FToken &Token, EClassPropertyType *Type);
	int				MatchIdentifier	(char *Match);
	int				MatchSymbol		(char *Match);
	char			*NestTypeName	(ENestType NestType);
	void inline		UngetToken		(FToken &Token);
	char inline		GetChar			(void);
	char inline		GetLeadingChar	(void);
	void inline		UngetChar		(void);
	void inline		SkipBlanks		(void);
	int  inline		IsEOL			(char c);
	void VARARGS	AddResultText	(char *Fmt,...);
	void			PushNest		(ENestType NestType,const char *Name);
	void			PopNest			(ENestType NestType,const char *Descr);
	void			ProcessCompilerDirective(void);
	void			CheckAllow		(FToken &Token,int AllowFlags);
	FClassProperty  *GetVarDecl		(int *NumFrameProperties, FClassProperty *FrameProperties, BYTE *Data,
									int NoOverrides,int NoArrays,int NoDefault,char *HardcodedName);
	int				 GetExpr		(EClassPropertyType RequiredType, EClassPropertyType *Type=NULL);
	FStackNode		*FindStackNode	(UScript *Script,INT iNode);
	FStackNode		*FindStackNode	(FStackNodeLink *Link);
	void			FindParams		(UScript *Script,INT iNode,FClassProperty **Params,INT *NumParams);
	void			FindParams		(FStackNodeLink *Link,FClassProperty **Params,INT *NumParams);
	//
	void			InitRetry		(FRetryPoint *Retry);
	void			PerformRetry	(FRetryPoint *Retry);
	//
	inline void		EmitB			(BYTE  B) {*(BYTE  *)CodeTop = B; CodeTop+=sizeof(BYTE );};
	inline void		EmitW			(WORD  W) {*(WORD  *)CodeTop = W; CodeTop+=sizeof(WORD );};
	inline void		EmitD			(DWORD D) {*(DWORD *)CodeTop = D; CodeTop+=sizeof(DWORD);};
	inline void		EmitF			(FLOAT F) {*(FLOAT *)CodeTop = F; CodeTop+=sizeof(FLOAT);};
	inline void		EmitS			(char *S) {strcpy((char*)CodeTop,S); CodeTop+=strlen(S)+1;};
	void			EmitConstant	(FToken &ConstToken);
	//
	// Debugging:
	//
	void			DebugDumpToken	(FToken &Token);
	void			DebugDumpNest	(void);
	};

/*-----------------------------------------------------------------------------
	Global functions
-----------------------------------------------------------------------------*/

void VARARGS throwf(char *Fmt,...);
void IdentifierToC(const char *Src,char *Dest);

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
#endif // _INC_UNSCRIPT
