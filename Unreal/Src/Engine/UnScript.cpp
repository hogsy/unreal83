/*=============================================================================
	UnScript.cpp: UnrealScript compiler

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnScript.h"

#define PROPERTIES_DATA_SIZE	2048
#define MAX_CODE_SIZE			50000

/*-----------------------------------------------------------------------------
	Utility functions
-----------------------------------------------------------------------------*/

//
// Throw a string exception with a message:
//
void VARARGS throwf(char *Fmt,...)
	{
	char		TempStr[4096];
	va_list		ArgPtr;
	va_start	(ArgPtr,Fmt);
	vsprintf	(TempStr,Fmt,ArgPtr);
	va_end		(ArgPtr);
	throw		(TempStr);
	};

const char *IdentifierToCRemap[96] =
	{
	"Space","Not","Quote","Pound","Dollars","Mod","BitAnd",
	"LeftParen","RightParen","Times","Plus","Minus","Comma","Dash","Dot","Divide",
	"0","1","2","3","4","5","6","7",
	"8","9","Colon","Semicolon","Less","Equals","Greater","Question",
	"At","A","B","C","D","E","F","G",
	"H","I","J","K","L","M","N","O",
	"P","Q","R","S","T","U","V","W",
	"X","Y","Z","LeftBracket","Backslash","RightBracket","BitXor","Underscore",
	"Doink","a","b","c","d","e","f","g",
	"h","i","j","k","l","m","n","o",
	"p","q","r","s","t","u","v","w",
	"x","y","z","LeftBrace","BitOr","RightBrace","Squiggle","Del"
	};

void IdentifierToC(const char *Src,char *Dest)
	{
	char c;
	while (1)
		{
		c = *Src++;
		if(!c) break;
		Dest += sprintf(Dest,IdentifierToCRemap[c]);
		};
	*Dest=0;
	};

/*-----------------------------------------------------------------------------
	FToken implementation
-----------------------------------------------------------------------------*/

char *FToken::Descr(void)
	{
	static char Result[80];
	switch (Type)
		{
		case TOKEN_End:
			strcpy(Result,"<end of line>");
			break;
		case TOKEN_Identifier:
			strcpy(Result,Identifier);
			break;
		case TOKEN_Symbol:
			strcpy(Result,Identifier);
			break;
		case TOKEN_IntegerConst:
			sprintf(Result,"%i",Integer);
			break;
		case TOKEN_FloatConst:
			sprintf(Result,"%f",Float);
			break;
		case TOKEN_StringConst:
			sprintf(Result,"string \"%s\"",String);
			break;
		case TOKEN_NameConst:
			sprintf(Result,"Name(%s)",Name.Name());
			break;
		case TOKEN_VectorConst:
			sprintf(Result,"Vector(%f,%f,%f)",Vector.X,Vector.Y,Vector.Z);
			break;
		case TOKEN_RotationConst:
			sprintf(Result,"Rotation(%i,%i,%i)",Rotation.Pitch,Rotation.Yaw,Rotation.Roll);
			break;
		case TOKEN_ResourceConst:
			sprintf(Result,"%s(%s)",GRes.Types[ResType].Descr,Res->Name);
			break;
		case TOKEN_Variable:
			sprintf(Result,"Variable(%f)",Float);
			break;
		default:
			strcpy(Result,"<unknown token>");
			break;
		};
	return Result;
	};

//
// See if this token can represent a constant integer value. Returns 1 if this
// is the case (i.e. the number is an integer constant, a floating point constant,
// "True", "False", or an enumeration constant). Otherwise returns 0.
//
int FToken::GetInteger(int *i)
	{
	if		(Type==TOKEN_IntegerConst)  {*i=Integer; return 1;}
	else if (Type==TOKEN_FloatConst)    {*i=(int)Float; return 1;}
	else if (Type==TOKEN_Identifier)
		{
		// See if this is a 'True' or 'False' constant:
		if (Matches("True"))  {*i = 1; return 1;};
		if (Matches("False")) {*i = 0; return 1;};
		// See if this is an enum constant
		UEnum *Enum;
		FOR_ALL_TYPED_RES(Enum,RES_Enum,UEnum)
			{
			for (int n=0;n<Enum->Num; n++)
				{
				if (Matches(Enum->Element(n).Name())) {*i=n; return 1;};
				};
			};
		END_FOR_ALL_TYPED_RES;
		};
	return 0;
	};

//
// See if this token matches an existing global name.  If it does, returns 1 and sets
// the token's Name member to the name's ID (which is guaranteed not to be NAME_NONE).
// Otherwise, returns 0 and leaves the Name property unaffected, so it should remain
// NAME_NONE as it was set in GetToken.
//
// Since this may be called several times for a given token, it uses the LookedUpName
// member to indicate whether the lookup has already been performed.
//
int FToken::LookupName(void)
	{
	if (!LookedUpName)
		{
		LookedUpName = 1;
		if ((Type==TOKEN_Identifier)||(Type==TOKEN_Symbol)) Name.Find(Identifier);
		};
	return (Name!=NAME_NONE);
	};

/*-----------------------------------------------------------------------------
	Single-character processing
-----------------------------------------------------------------------------*/

//
// Get a single character from the input stream and return it, or 0=end.
//
char inline FScriptCompiler::GetChar(void)
	{
	PrevPos  = InputPos;
	PrevLine = InputLine;
	if (InputPos < InputSize)
		{
		char c = Input[InputPos++];
		if (c==0x0a) InputLine++;
		return c;
		}
	else
		{
		InputPos++; // So that UngetChar() works properly
		return 0;
		};
	};

//
// Unget the previous character retrieved with GetChar()
//
void inline FScriptCompiler::UngetChar(void)
	{
	InputPos  = PrevPos;
	InputLine = PrevLine;
	};

//
// Skip past all spaces and tabs in the input stream
//
char inline FScriptCompiler::GetLeadingChar(void)
	{
	char c;
	do c=GetChar(); while ((c==0x20)||(c==0x09));
	if (c=='_') // Line continuation character
		{
		SkipBlanks();
		return GetChar();
		};
	return c;
	};

//
// Skip past all spaces, tabs, cr's, linefeeds, and comments in the input stream
//
void inline FScriptCompiler::SkipBlanks(void)
	{
	char c;
	//
	Skip1: do c=GetChar(); while ((c==0x20)||(c==0x09)||(c==0x0d)||(c==0x0a));
	if (c=='\'') // Comment
		{
		do c=GetChar(); while ((c!=0x0d)&&(c!=0x0a)&&(c!=0x00));
		goto Skip1;
		};
	UngetChar();
	};

//
// Return 1 if input as a valid end-of-line character, or 0 if not.
// EOL characters are: Comment, CR, linefeed, 0 (end-of-file mark)
//
int inline FScriptCompiler::IsEOL(char c)
	{
	return ((c=='\'')||(c==0x0d)||(c==0x0a)||(c==0));
	};

/*-----------------------------------------------------------------------------
	Code emitting
-----------------------------------------------------------------------------*/

//
// Emit a constant expression into an expression residing in code.
//
void FScriptCompiler::EmitConstant(FToken &Token)
	{
	switch (Token.Type)
		{
		case TOKEN_IntegerConst:
			EmitB(EX_IntegerConst);
			EmitD(Token.Integer);
			break;
		case TOKEN_FloatConst:
			EmitB(EX_RealConst);
			EmitF(Token.Float);
			break;
		case TOKEN_StringConst:
			EmitB(EX_StringConst);
			EmitS(Token.String);
			break;
		case TOKEN_ResourceConst:
			EmitB(EX_ResourceConst);
			EmitB(Token.ResType);
			EmitD((DWORD)Token.Res);
			break;
		case TOKEN_NameConst:
			EmitB(EX_NameConst);
			EmitW(Token.Name.Index);
			break;
		case TOKEN_RotationConst:
			EmitB(EX_RotationConst);
			EmitW(Token.Rotation.Pitch);
			EmitW(Token.Rotation.Yaw);
			EmitW(Token.Rotation.Roll);
			break;
		case TOKEN_VectorConst:
			EmitB(EX_VectorConst);
			EmitF(Token.Vector.X);
			EmitF(Token.Vector.Y);
			EmitF(Token.Vector.Z);
			break;
		default:
			throwf("Internal EmitConstant token type error");
			break;
		};
	};

/*-----------------------------------------------------------------------------
	Tokenizing
-----------------------------------------------------------------------------*/

//
// Get the next token from the input stream, set *Token to it.
// Returns 1 if processed a token, 0 if end of line.
//
int FScriptCompiler::GetToken(FToken &Token)
	{
	Token.StartPos		= InputPos;
	Token.StartLine		= InputLine;
	Token.Name			= NAME_NONE;
	Token.LookedUpName	= 0;
	//
	char c=GetLeadingChar();
	if (IsEOL(c))
		{
		UngetChar();
		Token.Type = TOKEN_End;
		return 0;
		}
	else if (((c>='A')&&(c<='Z')) || ((c>='a')&&(c<='z'))) // Alphanumeric token
		{
		int Length=0;
		do  {
			Token.Identifier[Length++] = c;
			if (Length > MAX_IDENTIFIER_SIZE) throwf("Identifer length exceeds maximum of %i",MAX_IDENTIFIER_SIZE);
			c = GetChar();
			} while (((c>='A')&&(c<='Z')) || ((c>='a')&&(c<='z')) || ((c>='0')&&(c<='9')) || (c=='_'));
		UngetChar();
		Token.Identifier[Length]=0;
		//
		// See if the idenfitifier is part of a name, vector, rotation, or resource constant:
		//
		if ((!stricmp(Token.Identifier,"Name")) && MatchSymbol("(")) // This is a name constant
			{
			FToken NameToken;
			if (!GetToken(NameToken)) throwf("Missing name");
			if (!MatchSymbol(")")) throwf("Missing ')' after name");
			//
			if		(NameToken.Type==TOKEN_Identifier)	Token.Name.Add(NameToken.Identifier);
			else if (NameToken.Type==TOKEN_StringConst)	Token.Name.Add(NameToken.String);
			else										throwf("Bad name");
			//
			Token.Type			= TOKEN_NameConst;
			Token.LookedUpName	= 1;
			return 1;
			}
		if ((!stricmp(Token.Identifier,"Vector")) && MatchSymbol("(")) // This is a vector constant
			{
			if (!GetFloatConst(&Token.Vector.X)) throwf("Missing X component of vector");
			if (!MatchSymbol(","))               throwf("Missing ',' in vector");
			if (!GetFloatConst(&Token.Vector.Y)) throwf("Missing Y component of vector");
			if (!MatchSymbol(","))               throwf("Missing ',' in vector");
			if (!GetFloatConst(&Token.Vector.Z)) throwf("Missing Z component of vector");
			if (!MatchSymbol(")"))               throwf("Missing ')' in vector");
			Token.Type = TOKEN_VectorConst;
			return 1;
			}
		if ((!stricmp(Token.Identifier,"Rotation")) && MatchSymbol("(")) // This is a rotation constant
			{
			int Pitch,Yaw,Roll;
			if (!GetIntegerConst(&Pitch))          throwf("Missing Pitch component of rotation");
			if (!MatchSymbol(","))                 throwf("Missing ',' in rotation");
			if (!GetIntegerConst(&Yaw))            throwf("Missing Yaw component of rotation");
			if (!MatchSymbol(","))                 throwf("Missing ',' in vector");
			if (!GetIntegerConst(&Roll))           throwf("Missing Roll component of rotation");
			if (!MatchSymbol(")"))                 throwf("Missing ')' in vector");
			Token.Rotation.Pitch = Pitch;
			Token.Rotation.Yaw   = Yaw;
			Token.Rotation.Roll  = Roll;
			Token.Type           = TOKEN_RotationConst;
			return 1;
			};
		//
		// See if this is a resource constant:
		//
		EResourceType ResType = GRes.LookupType(Token.Identifier);
		if ((ResType!=RES_None) && (GRes.Types[ResType].TypeFlags & RTF_ScriptReferencable) &&
			MatchSymbol("(")) // This is a resource constant
			{
			FToken NameToken;
			if (!GetIdentifier(NameToken)) throwf("Missing %s name",GRes.Types[ResType].Descr);
			UResource *Res = GRes.PrivateLookup(NameToken.Identifier,ResType,FIND_Optional);
			if (Res==NULL) throwf("Can't find %s '%s'",GRes.Types[ResType].Descr,NameToken.Identifier);
			if (!MatchSymbol(")")) throwf("Missing ')' after %s name",GRes.Types[ResType].Descr);
			Token.Res     = Res;
			Token.ResType = ResType;
			Token.Type    = TOKEN_ResourceConst;
			return 1;
			};
		//
		// This is a plain old identifier:
		//
		Token.Type = TOKEN_Identifier;
		return 1;
		}
	else if ((c>='0') && (c<='9')) // Integer or floating point constant
		{
		char TempNum[MAX_IDENTIFIER_SIZE]="";
		int  IsFloat=0;
		int  Length=0;
		do  {
			if (c=='.') IsFloat=1;
			TempNum[Length++] = c;
			if (Length>=MAX_IDENTIFIER_SIZE) throwf("Number length exceeds maximum of %i ",MAX_IDENTIFIER_SIZE);
			c = GetChar();
			} while (((c>='0') && (c<='9')) || (c=='.'));
		TempNum[Length]=0;
		if (IsFloat)
			{
			Token.Float		= atof(TempNum);
			Token.Type		= TOKEN_FloatConst;
			}
		else
			{
			Token.Integer	= atoi(TempNum);
			Token.Type		= TOKEN_IntegerConst;
			};
		UngetChar();
		return 1;
		}
	else if (c=='"') // String constant
		{
		int Length=0;
		c=GetChar();
		while ((c!='"') && !IsEOL(c))
			{
			if (c=='\\')
				{
				c=GetChar();
				if (IsEOL(c)) break;
				};
			Token.Identifier[Length++] = c;
			if (Length>=MAX_STRING_CONST_SIZE) throwf("String constant exceeds maximum of %i characters",MAX_IDENTIFIER_SIZE);
			c=GetChar();
			};
		if (c!='"') throwf("Unterminated string constant");
		//
		Token.Identifier[Length]=0;
		Token.Type = TOKEN_StringConst;
		return 1;
		}
	else // Symbol
		{
		int Length=0;
		Token.Identifier[Length++] = c;
		//
		// Handle special 2-character symbols:
		//
		#define PAIR(cc,dd) ((c==cc)&&(d==dd)) /* Comparison macro for convenience */
		char d=GetChar();
		if  (
			PAIR('<','<') ||
			PAIR('>','>') ||
			PAIR('!','=') ||
			PAIR('<','>') ||
			PAIR('<','=') ||
			PAIR('>','=') ||
			PAIR(':',':') ||
			PAIR('+','+') ||
			PAIR('-','-') ||
			PAIR('+','=') ||
			PAIR('-','=') ||
			PAIR('*','=') ||
			PAIR('/','=')
			)
			{
			Token.Identifier[Length++] = d;
			}
		else UngetChar();
		#undef PAIR
		//
		Token.Identifier[Length]=0;
		Token.Type = TOKEN_Symbol;
		return 1;
		};
	return 0;
	};

//
// Get an identifier token, return 1 if gotten, 0 if not.
//
int	FScriptCompiler::GetIdentifier(FToken &Token)
	{
	if (!GetToken(Token)) return 0;
	if (Token.Type==TOKEN_Identifier) return 1;
	UngetToken(Token);
	return 0;
	};

//
// Get a symbol token, return 1 if gotten, 0 if not.
//
int	FScriptCompiler::GetSymbol(FToken &Token)
	{
	if (!GetToken(Token)) return 0;
	if (Token.Type==TOKEN_Symbol) return 1;
	UngetToken(Token);
	return 0;
	};

//
// Get an integer constant, return 1 if gotten, 0 if not.
//
int FScriptCompiler::GetIntegerConst(int *Result)
	{
	FToken Token;
	//
	if (!GetToken(Token)) return 0;
	if (Token.Type==TOKEN_IntegerConst)
		{
		*Result = Token.Integer;
		return 1;
		};
	UngetToken(Token);
	return 0;
	};

//
// Get a real number, return 1 if gotten, 0 if not.
//
int FScriptCompiler::GetFloatConst(FLOAT *Result)
	{
	FToken Token;
	//
	if (!GetToken(Token)) return 0;
	if (Token.Type==TOKEN_IntegerConst)
		{
		*Result = (FLOAT)Token.Integer;
		return 1;
		}
	else if (Token.Type==TOKEN_FloatConst)
		{
		*Result = Token.Float;
		return 1;
		};
	UngetToken(Token);
	return 0;
	};

//
// Get a constant expression of any type.  Returns type if gotten,
// CPT_None (==0) if not.
//
int FScriptCompiler::GetConstExpr(FToken &Token, EClassPropertyType *Type)
	{
	//
	// See if a token is available:
	//
	if (!GetToken(Token)) return 0;
	//
	// Handle regular constants:
	//
	if (Token.Type==TOKEN_IntegerConst)		{*Type = CPT_Integer;		return 1;};
	if (Token.Type==TOKEN_FloatConst)		{*Type = CPT_Real;			return 1;};
	if (Token.Type==TOKEN_StringConst)		{*Type = CPT_String;		return 1;};
	if (Token.Type==TOKEN_ResourceConst)	{*Type = CPT_Resource;		return 1;};
	if (Token.Type==TOKEN_NameConst)		{*Type = CPT_Name;			return 1;};
	if (Token.Type==TOKEN_VectorConst)		{*Type = CPT_Vector;		return 1;};
	if (Token.Type==TOKEN_RotationConst)	{*Type = CPT_Rotation;		return 1;};
	//
	// See if this is a special integer constant (true, false, or enum):
	//
	if (Token.Type==TOKEN_Identifier)
		{
		int i;
		if (Token.GetInteger(&i))
			{
			Token.Type    = TOKEN_IntegerConst;
			Token.Integer = i;
			//
			*Type = CPT_Integer;
			return 1;
			};
		};
	UngetToken(Token);
	return 0;
	};

//
// Get a specific identifier and return 1 if gotten, 0 if not.
// This is used primarily for checking for required symbols during compilation.
//
int	FScriptCompiler::MatchIdentifier(char *Match)
	{
	FToken Token;
	if (!GetToken(Token)) return 0;
	if ((Token.Type==TOKEN_Identifier) && !stricmp(Token.Identifier,Match)) return 1;
	UngetToken(Token);
	return 0;
	};

int	FScriptCompiler::MatchSymbol(char *Match)
	{
	FToken Token;
	if (!GetToken(Token)) return 0;
	if ((Token.Type==TOKEN_Symbol) && !stricmp(Token.Identifier,Match)) return 1;
	UngetToken(Token);
	return 0;
	};

//
// Unget the most recently gotten token.
//
void inline FScriptCompiler::UngetToken(FToken &Token)
	{
	InputPos	= Token.StartPos;
	InputLine	= Token.StartLine;
	};

/*-----------------------------------------------------------------------------
	State & Scope
-----------------------------------------------------------------------------*/

//
// Find a stack node residing in the specified script, at the specified node
// number.  Handles the special cases of a NULL script and of a script that
// is currently being compiled.
//
FStackNode *FScriptCompiler::FindStackNode(UScript *SrcScript,INT iNode)
	{
	if		(SrcScript==NULL)	return NULL;
	else if (SrcScript==Script)	return &StackTree[iNode];
	else						return &((FStackNode *)SrcScript->GetData())[iNode];
	};

//
// Find a stack node specified by a stack node link.
//
FStackNode *FScriptCompiler::FindStackNode(FStackNodeLink *Link)
	{
	return FindStackNode(Link->Script,Link->iNode);
	};

//
// Find the parameters structure corresponding to a particular stack node.  Sets Props
// to a pointer to the first property, and iNumProps to the number of parameter and return
// value properties (which will be less than the total number of properties if the function or
// when declares local variables). 
//
// If the number of properties is zero, the Props pointer is not meaningful.  Handles the special
// cases of a null script, the current stack node of the script currently being compiled,
// and a non-current stack node of the script currently being compiled.
//
void FScriptCompiler::FindParams(UScript *SrcScript,INT iNode,FClassProperty **Props,INT *NumProps)
	{
	if (SrcScript==NULL) // Null script
		{
		//AddResultText(" -> Null script\r\n");
		*Props     = NULL;
		*NumProps = 0;
		}
	else if ((SrcScript==Script) && (&StackTree[iNode]==TopNest->Node))
		{
		//AddResultText(" -> This node\r\n");
		*Props		= TopNest->Properties;
		*NumProps	= TopNest->Node->NumProperties;
		}
	else if (SrcScript==Script)
		{
		//AddResultText(" -> This script\r\n");
		FStackNode *Node = FindStackNode(SrcScript,iNode);
		//
		*Props      = &LocalProperties[Node->PropertiesOffset];
		*NumProps   = Node->NumProperties;
		}
	else
		{
		//AddResultText(" -> Other script\r\n");
		FStackNode *Node = FindStackNode(SrcScript,iNode);
		//
		*Props      = (FClassProperty *)(&SrcScript->GetData()[Node->PropertiesOffset]);
		*NumProps   = Node->NumProperties;
		};
	while ((*NumProps>0) && !((*Props)[*NumProps-1].PropertyFlags & (CPF_Param | CPF_ReturnValue)))
		{
		(*NumProps)--;
		};
	};

void FScriptCompiler::FindParams(FStackNodeLink *Link,FClassProperty **Props,INT *NumProps)
	{
	FindParams(Link->Script,Link->iNode,Props,NumProps);
	};

/*-----------------------------------------------------------------------------
	Variable getting
-----------------------------------------------------------------------------*/

//
// See if Token is a local or global variable.  Returns 1 if it is, or 0 if it's
// not a recognized variable.  If successful, sets iVar to the index of the variable
// in its property frame, IsGlobal to 1 if global, 0 if local, and VarProperty to a
// pointer to the variable's property description.
//
int FScriptCompiler::GetVariable(FToken &Token, INT *iVar, INT *IsGlobal,
	FClassProperty **VarProperty, EClassPropertyType *Type)
	{
	if (!Token.LookupName()) return 0; // No name -> this is not a variable
	//
	// Check locals:
	//
	FClassProperty *Property = &TopNest->Properties[0];
	for (int i=0; i<TopNest->Node->NumProperties; i++)
		{
		if (Token.Name==Property->PropertyName)
			{
			*iVar			= i;
			*IsGlobal		= 0;
			*VarProperty	= Property;
			*Type			= (EClassPropertyType)Property->PropertyType;
			return 1;
			};
		Property++;
		};
	//
	// Check globals:
	//
	Property = &Class->Element(0);
	for (int i=0; i<Class->Num; i++)
		{
		if (Token.Name==Property->PropertyName)
			{
			*iVar			= i;
			*IsGlobal		= 1;
			*VarProperty	= Property;
			*Type			= (EClassPropertyType)Property->PropertyType;
			return 1;
			};
		Property++;
		};
	return 0;
	};

/*-----------------------------------------------------------------------------
	Retry management
-----------------------------------------------------------------------------*/

//
// Remember the current compilation points, both in the source being
// compiled and the object code being emitted.
//
void FScriptCompiler::InitRetry(FRetryPoint *Retry)
	{
	Retry->InputPos		= InputPos;
	Retry->InputLine	= InputLine;
	Retry->CodeTop		= CodeTop;
	};

//
// Return to a previously-saved retry point.
//
void FScriptCompiler::PerformRetry(FRetryPoint *Retry)
	{
	InputPos	= Retry->InputPos;
	InputLine	= Retry->InputLine;
	CodeTop		= Retry->CodeTop;
	};

/*-----------------------------------------------------------------------------
	Expression parser
-----------------------------------------------------------------------------*/

//
// Try to parse a complete function call with a name matching Token.  Returns
// 1 if a function call was successfully parsed, or 0 if no matching function
// was found.  Handles the error condition where the function was called but the
// specified parameters didn't match, or there was an error in a parameter 
// expression.
//
// The function to call must be accessible within the current scope.
//
// This also handles unary operators identically to functions, but not binary
// operators.
//
int FScriptCompiler::GetFunctionExpr(FToken &Token, EClassPropertyType *Type)
	{
	FRetryPoint Retry;
	InitRetry(&Retry);
	//
	// All possible causes of errors in the function call which don't cause an
	// immediate compiler response.  These are set when a particular function override 
	// is determined to be unacceptable.  If a correct override is later found, these error
	// causes are ignored.  If no override is found, the error causes are analyzed so that
	// a useful error message can be displayed:
	//
	int ErrTypeMismatch=0,ErrMissingParameter=0,ErrTooManyParameters=0,iErrParameter=0;
	EClassPropertyType MismatchExpecting=CPT_None;
	//
	int i,MatchedName=0,MatchedSomething=0;
	if (!Token.LookupName()) return 0; // No name -> this is not a function
	//
	for (i=NestLevel-1; i>=0; i--)
		{
		if (Nest[i].Node)
			{
			FStackNodeLink *Link = &Nest[i].Node->ChildFunctions;
			while (Link->Script)
				{
				FStackNode *Node = FindStackNode(Link);
				//
				// If the other function's name matches this one's, process it:
				//
				if (Node->Name==Token.Name)
					{
					int				NumParams;
					FClassProperty	*Params;
					FindParams(Link->Script,Link->iNode,&Params,&NumParams);
					// /* Debugging: */ AddResultText("Checking %s (%i props)\r\n",Node->Name.Name(),Node->NumProperties);
					//
					// Prevent mismatch of underlying type (Function, Operator, Event):
					//
					if (Node->NestType==NEST_Function)
						{
						if (!MatchSymbol("(")) goto NotFound;
						}
					else if (Node->NestType==NEST_Operator)
						{
						if (NumParams>2) goto NextLink; // Not a unary operator
						}
					else throwf("'%s' is an event and it doesn't return a value",Token.Identifier);
					//
					if (NumParams<=0) throwf("Function '%s' has no return type - can't call it",Token.Identifier);
					//
					MatchedName = 1;
					//
					// Emit the function call now on the assumption that the parameters are
					// correct.  If we're wrong, Retry() will back this out.
					//
					//
					if (Node->StackNodeFlags & SNODE_Fast) // Prebound, non-overridable function
						{
						EmitB(EX_FastFunction);
						EmitW(Link->iNode);
						EmitD((DWORD)Link->Script);
						}
					else // Regular function
						{
						EmitB(EX_Function);
						EmitW(Token.Name.Index);
						};
					// Parse all parameters except for return type; the parameters are emitted to 
					// code as a series of non-empty expressions terminated by an EX_Nothing expression.
					// If the parameters aren't in agreement, skip this function and it will either be 
					// picked up by an override somewhere along the way, or the "Mismatched parameter" 
					// message will be thrown after we've checked them all.  Handles the case of optional 
					// parameters not specified.
					//
					FClassProperty		*Param = &Params[0];
					EClassPropertyType	FoundType;
					int					SkippingOptional=0, First=1;
					//
					int j;
					for (j=0; j<NumParams; j++)
						{
						if (!(Param->PropertyFlags & CPF_Param)) break; // Done with list of parameters
						//
						if (!SkippingOptional)
							{
							// Get comma parameter delimiter:
							if ((!First) && !MatchSymbol(",")) // Failed to get a comma
								{
								if (Param->PropertyFlags & CPF_OptionalParam) // Ok, it was optional
									{
									EmitB(EX_Nothing);
									SkippingOptional = 1;
									}
								else // This parameter was required but not specified
									{
									ErrMissingParameter++;
									iErrParameter = j;
									goto NextLink;
									};
								};
							if (GetExpr(CPT_None,&FoundType)) // Got expression, now verify its type
								{
								if (FoundType != (EClassPropertyType)Param->PropertyType) // Type mismatch
									{
									ErrTypeMismatch++;
									iErrParameter = j;
									MismatchExpecting = Param->PropertyType;
									goto NextLink;
									};
								}
							else // Failed to get an expression
								{
								if (Param->PropertyFlags & CPF_OptionalParam) SkippingOptional = 1; // Ok
								else
									{
									ErrMissingParameter++;
									iErrParameter = j;
									goto NextLink;
									};
								};
							// Here, we successfully got an expression, matching Param.
							};
						Param++;
						First=0;
						};
					if (!SkippingOptional) EmitB(EX_Nothing);
					//
					if ((Node->NestType==NEST_Function) && !MatchSymbol(")"))
						{
						ErrTooManyParameters++;
						goto NextLink;
						};
					//
					// Make sure return value is valid:
					//
					if ((j>=NumParams) || !(Param->PropertyFlags & CPF_ReturnValue))
						{
						throwf("Function '%s' has no return type - can't call it",Token.Identifier);
						};
					*Type = (EClassPropertyType) Param->PropertyType;
					//
					return 1; // Success
					};
				//
				// Here we know that the function we checked at *Link isn't an acceptable
				// match, so we go on to the next one.
				//
				NextLink:
				PerformRetry(&Retry);
				MatchSymbol("(");
				Link = &Node->Next;
				};
			};
		};
	NotFound:
	PerformRetry(&Retry);
	if (MatchedName)
		{
		//
		// Handle error cause:
		//
		if ((ErrTypeMismatch+ErrMissingParameter)>1)
			{
			// Several errors; can't easily figure out what's what:
			throwf("Call to '%s': Mismatched parameters",Token.Identifier);
			}
		else if (ErrTypeMismatch)		throwf("Call to '%s': type mismatch in parameter %i (need '%s')",Token.Identifier,iErrParameter+1,GetPropertyTypeName(MismatchExpecting));
		else if (ErrMissingParameter)	throwf("Call to '%s': missing required parameter %i",Token.Identifier,iErrParameter+1);
		else if (ErrTooManyParameters)	throwf("Call to '%s': Too many parameters or missing ')'",Token.Identifier);
		else							throwf("Internal GetFunctionExpr inconsistency");
		};
	return 0;
	};

//
// Parse an expression.  Call with:
//
// RequiredType = the mandatory type of the expression, or CPT_None = any allowable type.
// Optional     = 1 if the expression is optional, 0 if required.
//
// Returns the exact type of the expression parsed (which must match RequiredType if 
// specified), or CPT_None if no expression was parsed.  The parsed expression has
// been emitted to the code stream and begins with one of the EExprToken tokens which
// will be EX_None if no expression was parsed.
//
int FScriptCompiler::GetExpr(EClassPropertyType RequiredType, EClassPropertyType *Type)
	{
	FToken				Token;
	FClassProperty		*VarProperty;
	EClassPropertyType	ThisType = CPT_None;
	INT					iVar,IsGlobal;
	//
	if (GetConstExpr(Token,&ThisType))
		{
		//
		// This is some kind of constant:
		//
		EmitConstant(Token);
		}
	else if (!GetToken(Token))
		{
		//
		// This is an empty expression:
		//
		ThisType = CPT_None;
		EmitB(EX_Nothing);
		}
	else if (Token.Matches("("))
		{
		//
		// Parenthesis:
		//
		if (!GetExpr(CPT_None,&ThisType))	throwf("Missing expression after '('");
		if (!MatchSymbol(")"))				throwf("Missing ')'");
		}
	else if (GetVariable(Token,&iVar,&IsGlobal,&VarProperty,&ThisType))
		{
		int IsArrayElement=0;
		//
		// This is a variable:
		//
		if (VarProperty->PropertyArrayDim)
			{
			if (MatchSymbol("("))
				{
				IsArrayElement=1;
				EmitB(EX_ArrayElement);
				}
			else if (ThisType!=CPT_String) throwf("%s is an array; expecting '('",Token.Identifier);
			};
		if	 (IsGlobal)	EmitB(EX_GlobalVariable);
		else			EmitB(EX_LocalVariable);
		//
		EmitB(ThisType);
		EmitW(VarProperty->PropertyOffset);
		//
		if (IsArrayElement)
			{
			EClassPropertyType SubscriptType;
			//
			if (!GetExpr(CPT_Integer,&SubscriptType)) throwf("Missing or invalid array index");
			if (!MatchSymbol(")")) throwf("%s is an array; expecting '('",Token.Identifier);
			};
		}
	else if (GetFunctionExpr(Token,&ThisType))
		{
		//
		// We successfully parsed a function-call expression
		//
		}
	else
		{
		//
		// This doesn't match an expression, so put it back.  It might be some kind of
		// delimiter like a comma, but whatever routine called this will error out if it's
		// not valid in its particular context:
		//
		EmitB(EX_Nothing);
		UngetToken(Token);
		};
	//
	// See if a binary operator follows the expression we just got:
	//
	if ((ThisType!=CPT_None) && (0))
		{
		//
		// We have found a valid type followed by a binary operator:
		//
		};
	//
	// Verify that what we got is what we want:
	//
	if (RequiredType && ThisType)
		{
		if (ThisType!=RequiredType) throwf("Expecting %s, found %s",GetPropertyTypeName(RequiredType),GetPropertyTypeName(ThisType));
		};
	if (ThisType)
		{
		if (Type) *Type = ThisType;
		return 1;
		}
	else return 0;
	};

/*-----------------------------------------------------------------------------
	Make
-----------------------------------------------------------------------------*/

void FScriptCompiler::InitMake(void)
	{
	GUARD;
	//
	StatementsCompiled	= 0;
	LinesCompiled		= 0;
	//
	ErrorText			= new("ScriptError",CREATE_Replace)UTextBuffer(16384);
	*ErrorText->GetData()=0;
	//
	UNGUARD("FScriptCompiler::InitMake");
	};

void FScriptCompiler::ExitMake(int Success)
	{
	GUARD;
	//
	if (Success)
		{
		if (LinesCompiled) AddResultText("Success: Compiled %i line(s), %i statement(s).\r\n",LinesCompiled,StatementsCompiled);
		else AddResultText("Success: Everything is up to date");
		}
	else
		{
		AddResultText("Script compilation failed.\r\n");
		};
	UNGUARD("FScriptCompiler::ExitMake");
	};

/*-----------------------------------------------------------------------------
	Nest management
-----------------------------------------------------------------------------*/

char *FScriptCompiler::NestTypeName(ENestType NestType)
	{
	switch (NestType)
		{
		case NEST_None:		return "Global Scope";
		case NEST_Class:	return "Class";
		case NEST_State:	return "State";
		case NEST_When:		return "When";
		case NEST_Function:	return "Function";
		case NEST_Operator:	return "Operator";
		case NEST_If:		return "If";
		case NEST_Loop:		return "Loop";
		case NEST_Select:	return "Select";
		case NEST_Case:		return "Case";
		case NEST_For:		return "For";
		default:			return "Unknown";
		};
	};

//
// Make sure that a particular kind of command is allowed on this nesting level.
// If it's not, issues a compiler error referring to the token and the current
// nesting level.
//
void FScriptCompiler::CheckAllow (FToken &Token,int AllowFlags)
	{
	if ((TopNest->Allow & AllowFlags) != AllowFlags)
		{
		if (TopNest->NestType==NEST_None)
			{
			throwf("'%s' is not allowed before the Class or Module definition",Token.Identifier);
			}
		else if (TopNest->DenyIntrinsic & AllowFlags)
			{
			throwf("'%s' is not allowed within this intrinsic '%s'",Token.Identifier,NestTypeName(TopNest->NestType));
			}
		else
			{
			throwf("'%s' is not allowed in this '%s' block",Token.Identifier,NestTypeName(TopNest->NestType));
			};
		};
	if (AllowFlags & ALLOW_Cmd)
		{
		TopNest->Allow &= ~(ALLOW_VarDecl); // Don't allow variable declarations after commands
		};
	};

//
// Increase the nesting level, setting the new top nesting level to
// the one specified.  If pushing a function or state and it overrides a similar
// thing declared on a 
//
void FScriptCompiler::PushNest(ENestType NestType,const char *Name)
	{
	INT iNode;
	FStackNode *PrevNode;
	FName ThisName;
	ThisName.Add(Name);
	//
	DWORD PrevAllow;
	TopNest					= &Nest[NestLevel++];
	TopNest->NestType		= NestType;
	TopNest->StartCodeTop	= CodeTop;
	//
	if (NestLevel >= MAX_NEST_LEVELS) throwf("Maximum nesting limit exceeded");
	//
	// Set up pointers:
	//
	if (NestLevel>1)
		{
		iNode					= Script->NumStackTree++;
		PrevAllow				= TopNest->Allow;
		TopNest->DenyIntrinsic	= TopNest[-1].DenyIntrinsic;
		TopNest->Node			= &StackTree[iNode];
		TopNest->Node->Init(NestType,ThisName);
		PrevNode				= TopNest[-1].Node;
		}
	else
		{
		iNode					= 0;
		PrevAllow				= 0;
		TopNest->DenyIntrinsic	= 0;
		TopNest->Node			= NULL;
		PrevNode			= NULL;
		};
	//
	// If this item has a name associated with it, see if (1) it's an override of an existing
	// item declared on a lower scope, and (2) it's a duplicate of an existing item on this scope.
	//
	if (PrevNode && (ThisName!=NAME_NONE) && (NestType!=NEST_Operator))
		{
		if ((NestType==NEST_Function) || (NestType==NEST_When)||
			(NestType==NEST_State)    || (NestType==NEST_Operator))
			{
			FStackNodeLink *Link;
			//
			if (NestType==NEST_State)	Link = &PrevNode->ChildStates;
			else						Link = &PrevNode->ChildFunctions;
			//
			while (Link->Script)
				{
				FStackNode *Node = FindStackNode(Link);
				if (Node->Name==ThisName)
					{
					if (Link->Script==Script) // This is a duplicate on this scope level:
						{
						throwf("Duplicate %s '%s'",NestTypeName(NestType),Name);
						}
					else // This is an override of a lower scope level (allowed):
						{
						//
						// We are overriding a state or a function from a parent class's script
						// at this same nesting level.  Copy the overridden stack node's child,
						// next, and state links to this node.
						//
						if (!TopNest->Node) throwf("Internal PushNest node inconsistency");
						TopNest->Node->ChildFunctions = Node->ChildFunctions;
						TopNest->Node->ChildStates    = Node->ChildStates;
						};
					};
				Link = &Node->Next;			
				};
			};
		};
	//
	// NestType specific logic:
	//
	switch (NestType)
		{
		case NEST_None:
			if (PrevNode) throwf("Internal error in PushNest None");
			TopNest->Allow = ALLOW_Class;
			break;
		case NEST_Class:
			if (ThisName==NAME_NONE) throwf("Missing Class name");
			if (PrevNode) throwf("Internal error in PushNest Class");
			TopNest->Allow = ALLOW_VarDecl + ALLOW_Function + ALLOW_State;
			break;
		case NEST_State:
			if (ThisName==NAME_NONE) throwf("Missing State name");
			if (!PrevNode) throwf("Internal error in PushNest State");
			TopNest->Allow					= ALLOW_Function + ALLOW_When + ALLOW_State + ALLOW_SlowCmd + ALLOW_Cmd;
			TopNest->Node->Next		= PrevNode->ChildStates;
			PrevNode->ChildStates.Set(Script,iNode);
			break;
		case NEST_When:
			if (ThisName==NAME_NONE) throwf("Missing When name");
			if (!PrevNode) throwf("Internal error in PushNest When");
			TopNest->Allow					= ALLOW_VarDecl + ALLOW_SlowCmd + ALLOW_Cmd;
			TopNest->Node->Next		= PrevNode->ChildFunctions;
			PrevNode->ChildFunctions.Set(Script,iNode);
			break;
		case NEST_Function:
			if (ThisName==NAME_NONE) throwf("Missing Function name");
			if (!PrevNode) throwf("Internal error in PushNest Function");
			TopNest->Allow					= ALLOW_VarDecl + ALLOW_Cmd + ALLOW_Return;
			TopNest->Node->Next		= PrevNode->ChildFunctions;
			PrevNode->ChildFunctions.Set(Script,iNode);
			break;
		case NEST_Operator:
			if (ThisName==NAME_NONE) throwf("Missing Operator name");
			if (!PrevNode) throwf("Internal error in PushNest Operator");
			TopNest->Allow					= ALLOW_VarDecl + ALLOW_Cmd + ALLOW_Return;
			TopNest->Node->Next		= PrevNode->ChildFunctions;
			PrevNode->ChildFunctions.Set(Script,iNode);
			break;
		case NEST_If:
			if (ThisName!=NAME_NONE) throwf("Internal name error in PushNest If");
			if (!PrevNode) throwf("Internal error in PushNest If");
			TopNest->Allow = ALLOW_Cmd + (PrevAllow & (ALLOW_SlowCmd|ALLOW_Return));
			break;
		case NEST_Loop:
			if (ThisName!=NAME_NONE) throwf("Internal name error in PushNest While");
			if (!PrevNode) throwf("Internal error in PushNest While");
			TopNest->Allow = ALLOW_Cmd + ALLOW_Until + (PrevAllow & (ALLOW_SlowCmd|ALLOW_Return));
			break;
		case NEST_Select:
			if (ThisName!=NAME_NONE) throwf("Internal name error in PushNest Select");
			if (!PrevNode) throwf("Internal error in PushNest Select");
			TopNest->Allow = ALLOW_Case + ALLOW_Default + (PrevAllow & (ALLOW_SlowCmd|ALLOW_Return));
			break;
		case NEST_For:
			if (ThisName!=NAME_NONE) throwf("Internal name error in PushNest For");
			if (!PrevNode) throwf("Internal error in PushNest For");
			TopNest->Allow = ALLOW_Cmd + (PrevAllow & (ALLOW_SlowCmd|ALLOW_Return));
			break;
		case NEST_Case:
			if (ThisName!=NAME_NONE) throwf("Internal name error in PushNest Case");
			if (!PrevNode) throwf("Internal error in PushNest Case");
			TopNest->Allow = ALLOW_Cmd + (PrevAllow & (ALLOW_SlowCmd||ALLOW_Return));
			break;
		default:
			throwf("Internal error in PushNest, type %i",NestType);
			break;
		};
	if (TopNest->Node)
		{
		//
		// If local variable declarations are allowed here, allocate them:
		//
		TopNest->Node->NumProperties = 0;
		//
		if ((TopNest->Allow & ALLOW_VarDecl) && (NestType != NEST_Class))
			{
			TopNest->Properties	= (FClassProperty *)Mem->Get(UClass::MAX_CLASS_PROPERTIES * sizeof(FClassProperty));
			TopNest->Data   	= (BYTE           *)Mem->Get(PROPERTIES_DATA_SIZE         * sizeof (BYTE));
			}
		else
			{
			TopNest->Properties = NULL;
			TopNest->Data       = NULL;
			};
		//
		// If code is allowed here, allocate space for it:
		//
		if (TopNest->Allow & ALLOW_Cmd)
			{
			TopNest->Node->CodeStart = (int)(CodeTop-CodeStart);
			};
		};
	};

//
// Decrease the nesting level and handle any errors that result:
//
void FScriptCompiler::PopNest(ENestType NestType,const char *Descr)
	{
	if (NestLevel<=0) throwf("Unexpected '%s' at global scope",Descr,NestTypeName(NestType));
	//
	if (TopNest->NestType!=NestType)
		{
		throwf("Unexpected '%s' in '%s' block",Descr,NestTypeName(TopNest->NestType));
		};
	if (TopNest->Properties)
		{
		//
		// Move all of this nesting level's properties and data to the script's properties
		// and data bins:
		//
		TopNest->Node->PropertiesOffset = TotalLocalProperties;
		TopNest->Node->DataOffset       = TotalLocalDataSize;
		//
		int NumProperties    = TopNest->Node->NumProperties;
		int NumPropertyBytes = NumProperties * sizeof(FClassProperty);
		//
		if (NumProperties>0)
			{
			TopNest->Node->DataLength =
				TopNest->Properties[NumProperties-1].PropertyOffset + 
				TopNest->Properties[NumProperties-1].PropertySize;
			}
		else TopNest->Node->DataLength = 0;
		//
		memcpy(&LocalProperties[TotalLocalProperties],TopNest->Properties,TopNest->Node->NumProperties * sizeof(FClassProperty));
		memcpy(&LocalData      [TotalLocalDataSize  ],TopNest->Data,      TopNest->Node->DataLength);
		//
		TotalLocalProperties += TopNest->Node->NumProperties;
		TotalLocalDataSize   += TopNest->Node->DataLength;
		//
		Mem->Release(TopNest->Properties);
		};
	if (TopNest->Allow & ALLOW_Cmd)
		{
		EmitB(SC_End); // End of code block
		}
	else if (CodeTop != TopNest->StartCodeTop) throwf("Internal nest code bytes inconsistency");
	//
	NestLevel--;
	TopNest--;
	};

/*-----------------------------------------------------------------------------
	Compiler directives
-----------------------------------------------------------------------------*/

//
// Process a compiler directive
//
void FScriptCompiler::ProcessCompilerDirective(void)
	{
	FToken Directive;
	//
	if (!GetIdentifier(Directive)) throwf("Missing compiler directive after '#'");
	//
	if		(Directive.Matches("DumpNest"))		DebugDumpNest();
	else if (Directive.Matches("Error"))		throwf("#Error directive encountered");
	else if (Directive.Matches("Decompile"))	Decompile = 1;
	else if (Directive.Matches("DecompileAll"))	Decompile = 2;
	else throwf("Unrecognized compiler directive %s",Directive.Identifier);
	};

/*-----------------------------------------------------------------------------
	Variable declaration parser
-----------------------------------------------------------------------------*/

//
// Parse one variable declaration and set up its properties in VarProperty.
// Returns pointer to the class property if success, or NULL if there was no variable 
// declaration to parse. Called during variable 'Dim' declaration and function 
// parameter declaration.
//
// If you specify a hard-coded name, that name will be used as the variable name (this
// is used for function return values, which are automatically called "ReturnValue"), and
// a default value is not allowed.
//
FClassProperty  *FScriptCompiler::GetVarDecl(int *NumPropertiesPtr, FClassProperty *PropertiesPtr, BYTE *Data,
	int NoOverrides,int NoArrays,int NoDefault,char *HardcodedName)
	{
	FClassProperty VarProperty,*Result;
	int ArraySize = 0;
	VarProperty.Init();
	//
	// Get varible name:
	//
	FToken VarToken;
	if (HardcodedName) // Hard-coded variable name, such as with return value
		{
		VarToken.Type = TOKEN_Identifier;
		strcpy(VarToken.Identifier,HardcodedName);
		}
	else if (!GetIdentifier(VarToken)) return NULL;
	//
	VarProperty.PropertyName.Add(VarToken.Identifier);
	if (VarProperty.PropertyName.IsNone()) throwf("Dim: '%s' is illegal",VarToken.Identifier);
	//
	// Verify that the variable is unique within this scope:
	//
	for (int i=0; i<*NumPropertiesPtr; i++)
		{
		if (PropertiesPtr[i].PropertyName==VarProperty.PropertyName)
			{
			if ((NumPropertiesPtr==&Class->Num) && Class->ParentClass && (i<Class->ParentClass->Num))
				{
				throwf("Dim: Variable '%s' is defined in parent class '%s'",VarToken.Identifier,Class->ParentClass->Name);
				}
			else
				{
				throwf("Dim: Variable '%s' already defined",VarToken.Identifier);
				};
			};
		};
	//
	// Get optional dimension immediately after name:
	//
	if (MatchSymbol("("))
		{
		if (NoArrays) throwf("Arrays aren't allowed in this context");
		if (!GetIntegerConst(&ArraySize)) throwf("Dim %s: Missing array size",VarToken.Identifier);
		if (ArraySize<=0) throwf("Dim %s: Illegal array size %i",VarToken.Identifier,ArraySize);
		if (!MatchSymbol(")")) throwf("Dim %s: Missing ')'",VarToken.Identifier);
		};
	//
	// Get property flag overrides:
	//
	if (!MatchIdentifier("As")) throwf("Dim %s: missing 'As'",VarToken.Identifier);
	//
	FToken PropToken;
	while (GetToken(PropToken) && VarProperty.SetFlags(PropToken.Identifier))
		{
		if (PropToken.Matches("Editable") && MatchSymbol("(")) // Get optional property editing category
			{
			FToken Category;
			if (!GetIdentifier(Category)) throwf("Missing editable category");
			VarProperty.PropertyCategory.Add(Category.Identifier);
			if (!MatchSymbol(")")) throwf("Missing ')' after editable category");
			};
		};
	UngetToken(PropToken);
	//
	if ((VarProperty.PropertyFlags & CPF_Edit) && (VarProperty.PropertyCategory==NAME_NONE))
		{
		// If editable but no category was specified, the category name is our class name
		VarProperty.PropertyCategory.Add(Class->Name);
		};
	if (NoOverrides && VarProperty.PropertyFlags) throwf ("Variable type modifiers (Const, Editable, etc) are not allowed here");
	//
	// Get variable type:
	//
	FToken VarType;
	if (!GetIdentifier(VarType)) throwf("Dim %s: Missing variable type",VarToken.Identifier);
	//
	// Get optional dimension immediately after type:
	//
	if ((!ArraySize) && MatchSymbol("("))
		{
		if (NoArrays && !VarType.Matches("String")) throwf("Arrays aren't allowed in this context");
		if (!GetIntegerConst(&ArraySize)) throwf("Dim %s: Missing array size",VarToken.Identifier);
		if (ArraySize<=0) throwf("Dim %s: Illegal array size %i",VarToken.Identifier,ArraySize);
		if (!MatchSymbol(")")) throwf("Dim %s: Missing ')'",VarToken.Identifier);
		};
	//
	// Parse variable type and dimension:
	//
	char Error[80];
	if (!VarProperty.SetType(VarType.Identifier,ArraySize,Error)) throwf("Dim %s: %s",VarToken.Identifier,Error);
	//
	// Get default value
	//
	FToken *ConstValue=NULL;
	if (MatchSymbol("="))
		{
		if (NoDefault) throwf ("You don't need an '=' here");
		if (VarProperty.PropertyFlags & CPF_Edit) throwf("Dim %s: Can't specify default value for editable properties",VarToken.Identifier);
		//
		FToken TempConstValue;
		EClassPropertyType Type;
		if (!GetConstExpr(TempConstValue,&Type))
			{
			throwf("Dim %s: No default value or bad default value",VarToken.Identifier);
			};
		ConstValue = &TempConstValue;
		VarProperty.PropertyFlags |= CPF_Initialized;
		}
	else if ((VarProperty.PropertyFlags & (CPF_Const|CPF_Edit))==CPF_Const)
		{
		throwf("Dim %s: You must specify a value for Const properties",VarToken.Identifier);
		};
	//
	// Add to properties bin:
	//
	if (*NumPropertiesPtr >= UClass::MAX_CLASS_PROPERTIES)
		{
		throwf("Dim %s: Too many variables are declared here",VarToken.Identifier);
		}
	Result = VarProperty.AddToFrame(NumPropertiesPtr,PropertiesPtr);
	if (!Result)
		{
		throwf("Dim %s: illegal variable declaration",VarToken.Identifier);
		}
	if (!VarProperty.InitPropertyData(Data,ConstValue))
		{
		throwf("Dim %s: Illegal constant value",VarToken.Identifier);
		};
	return Result;
	};

/*-----------------------------------------------------------------------------
	Statement compiler
-----------------------------------------------------------------------------*/

//
// Compile one line of the script.  Returns 1 if successful, or 0 if either
// at end of file, or an error occured.
//
int FScriptCompiler::CompileStatement(void)
	{
	FToken Token;
	//
	SkipBlanks();
	if (!GetToken(Token)) return 0; // End of file
	//
	switch (Token.Type)
		{
		case TOKEN_Identifier:
		case TOKEN_Symbol:
			// Process this below
			break;
		case TOKEN_IntegerConst:
			throwf("Unexpected integer constant %i at start of line",Token.Integer);
			return 0;
		case TOKEN_FloatConst:
			throwf("Unexpected floating point constant %f at start of line",Token.Float);
			return 0;
		case TOKEN_StringConst:
			throwf("Unexpected string constant \"%s\" at start of line",Token.String);
			return 0;
		case TOKEN_ResourceConst:
			throwf("Unexpected resource constant at start of line");
			return 0;
		case TOKEN_VectorConst:
			throwf("Unexpected vector constant at start of line");
			return 0;
		case TOKEN_RotationConst:
			throwf("Unexpected rotation constant at start of line");
			return 0;
		case TOKEN_NameConst:
			throwf("Unexpected name constant at start of line");
			return 0;
		default:
			throwf("Internal compiler error, unknown token type %i",Token.Type);
			return 0;
		};
	if (Token.Matches("Class")) // Start of a class block
		{
		CheckAllow(Token,ALLOW_Class);
		//
		// Class name:
		//
		if (!GetToken(Token)) throwf("Missing class name");
		if (!Token.Matches(Class->Name)) throwf("Class must be named %s, not %s",Class->Name,Token.Identifier);
		//
		PushNest(NEST_Class,Class->Name);
		//
		// Class attributes:
		//
		int GotParent=0;
		while (GetToken(Token))
			{
			if (Token.Matches("Intrinsic"))
				{
				Class->Intrinsic=1;
				//
				// Don't allow any scripting logic in this script, just definitions:
				//
				TopNest->DenyIntrinsic  = ALLOW_SlowCmd|ALLOW_Cmd|ALLOW_When|ALLOW_State;
				TopNest->Allow         &= ~TopNest->DenyIntrinsic;
				}
			else if (Token.Matches("Expands"))
				{
				if (!GetIdentifier(Token)) throwf("Missing parent class name");
				UClass *TempClass = new(Token.Identifier,FIND_Optional)UClass;
				if (!TempClass) throwf("Parent class %s not found",Token.Identifier);
				//
				if		(Class->ParentClass==NULL) Class->ParentClass = TempClass;
				else if (Class->ParentClass!=TempClass) throwf("%s's parent class must be %s, not %s",Class->Name,Class->ParentClass->Name,TempClass->Name);
				//
				// Copy parent's stack node links if parent is scripted:
				//
				UScript *ParentScript = Class->ParentClass->Script;
				if (ParentScript)
					{
					FStackNode *ParentNode = FindStackNode(ParentScript,0);
					//
					TopNest->Node->ChildFunctions = ParentNode->ChildFunctions;
					TopNest->Node->ChildStates    = ParentNode->ChildStates;
					};
				GotParent = 1;
				}
			else throwf("Class: bad definition");
			};
		if (Class->ParentClass && !GotParent) throwf("Class: 'Expands %s' missing",Class->ParentClass->Name);
		//
		// Copy stack tree and all properties from parent class:
		//
		if (Class->ParentClass) Class->AddParentProperties();
		}
	else if (Token.Matches("Function") || Token.Matches("When") || Token.Matches("Operator"))
		{
		ENestType NestType;
		FToken Function;
		int NoDefaults;
		//
		if (!Token.Matches("Operator")) // Get function name
			{
			if (!GetIdentifier(Function)) throwf("Missing function name");
			}
		else // Get operator name or symbol
			{
			if ((!GetIdentifier(Function)) && (!GetSymbol(Function))) throwf("Missing operator name");
			};
		//
		// Make sure this is allowable here:
		//
		if (Token.Matches("Function"))
			{
			NestType		= NEST_Function;
			NoDefaults		= 0;
			CheckAllow(Token,ALLOW_Function);
			}
		else if (Token.Matches("Operator"))
			{
			NestType		= NEST_Operator;
			NoDefaults		= 1;
			CheckAllow(Token,ALLOW_Function);
			}
		else // Token.Matches("When")
			{
			NestType		= NEST_When;
			NoDefaults		= 0;
			CheckAllow(Token,ALLOW_When);
			};
		//
		// Allocate local property frame, push nesting level and verify 
		// uniqueness at this scope level:
		//
		PushNest(NestType,Function.Identifier);
		//
		// Get parameter list
		//
		if (MatchSymbol("("))
			{
			int First=1,Optional=0;
			do	{
				FClassProperty *Property = GetVarDecl
					(
					&TopNest->Node->NumProperties,
					TopNest->Properties,
					TopNest->Data,
					1,0,NoDefaults,NULL
					);
				if (!Property)
					{
					if (First) break;
					else throwf("Bad parameter variable declaration");
					};
				Property->PropertyFlags |= CPF_Param;
				if (Property->PropertyFlags & CPF_Initialized) 
					{
					Property->PropertyFlags |= CPF_OptionalParam;
					Optional=1;
					}
				else if (Optional) throwf("After an optional parameters, all other parmeters must be optional");
				//
				First=0;
				} while (MatchSymbol(","));
			if (!MatchSymbol(")")) throwf("Missing ')' after parameter list",Token.Identifier);
			}
		else if (NestType == NEST_Function) throwf("Missing '(' after function name");
		else if (NestType == NEST_Operator) throwf("Missing '(' after operator name");
		//
		// Get return type, if any:
		//
		FToken Temp;
		if (GetIdentifier(Temp))
			{
			UngetToken(Temp);
			if (Temp.Matches("As"))
				{
				if (NestType == NEST_When) throwf("'When' handlers can't return values");
				FClassProperty *Property = GetVarDecl
					(
					&TopNest->Node->NumProperties,
					TopNest->Properties,
					TopNest->Data,
					1,1,1,"ReturnValue"
					);
				if (!Property) throwf("Bad function return type");
				Property->PropertyFlags |= CPF_ReturnValue;
				};
			};
		//
		// For operators, verify that: the operator is either binary or unary, there is
		// a return value, and all parameters have the same type as the return value
		//
		if (Token.Matches("Operator"))
			{
			int n = TopNest->Node->NumProperties;
			if ((n<2)||(n>3)) throwf("Operator must have one or two parameters");
			//
			if (!(TopNest->Properties[n-1].PropertyFlags&CPF_ReturnValue)) throwf("Operator must have a return value");
			//
			if ((n==3) && (TopNest->Properties[0].PropertyType!=TopNest->Properties[1].PropertyType))
				{
				throwf("All operator parameters must be of the same type");
				};
			};
		int				NumParams1,NumParams2;
		FClassProperty	*Params1,*Params2;
		FindParams(Script,Script->NumStackTree-1,&Params1,&NumParams1);
		//
		// Handle operator information:
		//
		if (NestType==NEST_Operator)
			{
			TopNest->Node->OperParams		= NumParams1;
			TopNest->Node->OperType			= Params1[0].PropertyType;
			TopNest->Node->OperPrecedence	= 0;
			};
		//
		// Verify parameter list and return type compatibility within the function or event, if
		// any, that it overrides.
		//
		for (int i=NestLevel-1; i>=0; i--)
			{
			if (Nest[i].Node)
				{
				FStackNodeLink *Link = &Nest[i].Node->ChildFunctions;
				//
				while (Link->Script)
					{
					FStackNode *Node = FindStackNode(Link);
					//
					// If the other function's name matches this one's, process it:
					//
					if ((Node->Name==TopNest->Node->Name) && (Node!=TopNest->Node)) // Skip function we're defining now
						{
						FindParams(Link->Script,Link->iNode,&Params2,&NumParams2);
						//
						// Prevent mismatch of underlying type (Function, Operator, Event):
						//
						if (Node->NestType!=TopNest->Node->NestType)
							{
							throwf("Redefinition of '%s' mismatches '%s' with '%s'",
								Function.Identifier,NestTypeName(Node->NestType),NestTypeName(TopNest->Node->NestType));
							};
						//
						// Assure parameter and return value compatibility for functions and events:
						//
						if ((Node->NestType==NEST_Function) || (Node->NestType==NEST_When))
							{
							//
							// See if this is a valid function/operator/when override, or an illegal mismatched
							// override:
							//
							if (NumParams1!=NumParams2)
								{
								Mismatch:
								//
								if (Link->Script==Script) throwf("Redefinition of %s '%s' differs from original",
									Token.Identifier,Function.Identifier);
								else throwf ("Redefinition of %s '%s' differs from original in script '%s'",
									Token.Identifier,Function.Identifier,Link->Script->Name);
								};
							for (int i=0; i<NumParams1; i++)
								{
								const DWORD MatchFlags = CPF_Param | CPF_ReturnValue;
								//
								// Make sure that param types and vital flags match.  Don't care about names, etc.
								//
								if (Params1[i].PropertyType!=Params2[i].PropertyType) goto Mismatch;
								if ((Params1[i].PropertyFlags ^ Params2[i].PropertyFlags) & MatchFlags) goto Mismatch;
								};
							};
						// /*For debugging:*/ AddResultText("%s::%s\r\n",OtherScript->Name,Node->Name.Name());
						};
					Link = &Node->Next;
					};
				};
			};
		//
		// See if this is intrinsic:
		//
		if (MatchIdentifier("Intrinsic"))
			{
			if (!Class->Intrinsic) throwf("Scripted classes may not contain intrinsic functions");
			if (NestType==NEST_When) throwf ("Only intrinsic Functions are allowed, not Events");
			//
			TopNest->Node->StackNodeFlags |= SNODE_Intrinsic;
			}
		else if (Class->Intrinsic) throwf("Intrinsic classes may only contain intrinsic functions");
		//
		// Check for special overrides:
		//
		if (MatchIdentifier("Fast"))
			{
			//
			// This is a fast (prebinding, non-overridable) function, event, or operator.
			//
			TopNest->Node->StackNodeFlags |= SNODE_Fast;
			}
		else if (TopNest->Node->NestType==NEST_Operator)
			{
			throwf("Operators must be declared as 'Fast'");
			};
		if (MatchIdentifier("AutoConversion"))
			{
			if ((TopNest->Node->NestType!=NEST_Operator) ||
				(TopNest->Node->NumProperties != 2))
				{
				throwf("AutoConversion is only allowed for unary operators");
				};
			TopNest->Node->StackNodeFlags |= SNODE_AutoConversion;
			};
		if (Class->Intrinsic)
			{
			//
			// If intrinsic, no function body is allowed:
			//
			char PopMsg[80];
			sprintf(PopMsg,"Intrinsic function %s",Token.Identifier);
			PopNest(NestType,PopMsg);
			}
		else
			{
			//
			// Handle single-line command form with implied "EndWhen/EndFunction":
			//
			FToken NewToken;
			if (GetToken(NewToken))
				{
				UngetToken(NewToken);
				if (!CompileStatement()) throwf("Illegal commands following %s '%s'",Token.Identifier,Function.Identifier);
				char PopMsg[80];
				sprintf(PopMsg,"Single-line %s",Token.Identifier);
				PopNest(NestType,PopMsg);
				};
			};
		}
	else if (Token.Matches("EndOperator"))
		{
		PopNest(NEST_Operator,"EndOperator");
		}
	else if (Token.Matches("EndFunction"))
		{
		PopNest(NEST_Function,"EndFunction");
		}
	else if (Token.Matches("EndWhen"))
		{
		PopNest(NEST_When,"EndWhen");
		}
	else if (Token.Matches("Return"))
		{
		CheckAllow(Token,ALLOW_Return);
		}
	else if (Token.Matches("Dim")) // Variable definition
		{
		int				*NumPropertiesPtr,NoOverrides;
		FClassProperty	*PropertiesPtr;
		BYTE			*Data;
		//
		CheckAllow(Token,ALLOW_VarDecl);
		if (!TopNest->Node) throwf("Internal stack node error on Dim");
		//
		if (!TopNest->Properties) // Declaring global variables:
			{
			NumPropertiesPtr	= &Class->Num;
			PropertiesPtr	    = &Class->Element(0);
			Data				= (BYTE *)&Class->DefaultActor;
			NoOverrides			= 0;
			}
		else // Declaring local variables:
			{
			NumPropertiesPtr	= &TopNest->Node->NumProperties;
			PropertiesPtr		= TopNest->Properties;
			Data				= TopNest->Data;
			NoOverrides			= 1;
			};
		do	{
			if (!GetVarDecl(NumPropertiesPtr,PropertiesPtr,Data,NoOverrides,0,0,NULL))
				{
				throwf("Dim: Missing variable declaration");
				};
			} while (MatchSymbol(","));
		}
	else if (Token.Matches("EnumDef")) // Enumeration definition
		{
		CheckAllow(Token,ALLOW_VarDecl);
		//
		FToken EnumToken;
		if (!GetIdentifier(EnumToken)) throwf("Missing enumeration name");
		if (!MatchSymbol  ("="))       throwf("Missing '=' in enumeration");
		//
		// Parse all enums:
		//
		int   NumEnums=0;
		FToken TagTokens[255]; // Maximum enumerations allowed
		//
		while (GetIdentifier(TagTokens[NumEnums]))
			{
			for (int i=0; i<NumEnums; i++)
				{
				if (!stricmp(TagTokens[i].Identifier,TagTokens[NumEnums].Identifier))
					throwf("Duplicate enumerator %s",TagTokens[NumEnums].Identifier);
				};
			if (++NumEnums>255) throwf("Exceeded maximum of 255 enumerators");
			if (!MatchSymbol(",")) break;
			};
		if (!NumEnums) throwf("Enumeration must contain at least one enumerator");
		if (GetToken(Token)) throwf("Bad '%s' in enumeration",Token.Descr());
		//
		// Create enumeration resource:
		//
		FName TempName;
		UEnum *TempEnum = new(EnumToken.Identifier,CREATE_Replace)UEnum(NumEnums);
		for (int i=0; i<NumEnums; i++)
			{
			TempName.Add(TagTokens[i].Identifier);
			TempEnum->Add(TempName);
			};
		}
	else if (Token.Matches("State")) // State block
		{
		CheckAllow(Token,ALLOW_State);
		//
		FToken NameToken;
		if (!GetIdentifier(NameToken)) throwf("Missing state name");
		//
		PushNest(NEST_State,NameToken.Identifier);
		//
		if (MatchSymbol(","))
			{
			FToken StateOverride;
			while (GetIdentifier(StateOverride))
				{
				if (StateOverride.Matches("Auto"))
					TopNest->Node->StackNodeFlags |= SNODE_Auto;
				else if (StateOverride.Matches("Editable"))
					TopNest->Node->StackNodeFlags |= SNODE_Editable;
				else
					throwf("Bad state override '%s'",StateOverride.Identifier);
				};
			};
		}
	else if (Token.Matches("EndState")) // End of State block
		{
		PopNest(NEST_State,"EndState");
		}
	else if (Token.Matches("#")) // Compiler directive
		{
		ProcessCompilerDirective();
		}
	else if (Token.Matches("Select")) // Select Case
		{
		CheckAllow(Token,ALLOW_Cmd);
		if (!MatchIdentifier("Case")) throwf("Correct syntax is 'Select Case <expression>'");
		PushNest(NEST_Select,"");
		}
	else if (Token.Matches("EndSelect")) // EndSelect
		{
		if (TopNest->NestType==NEST_Case) PopNest(NEST_Case,"Case");
		PopNest(NEST_Select,"EndSelect");
		}
	else if (Token.Matches("Case"))
		{
		if (TopNest->NestType==NEST_Case) PopNest(NEST_Case,"Case");
		CheckAllow(Token,ALLOW_Case);
		PushNest(NEST_Case,"");
		}
	else if (Token.Matches("Default"))
		{
		if (TopNest->NestType==NEST_Case) PopNest(NEST_Case,"Case");
		CheckAllow(Token,ALLOW_Case);
		TopNest->Allow &= ~(ALLOW_Case); // Don't allow additional Cases after Default
		PushNest(NEST_Case,"");
		}
	else if (Token.Matches("Test")) // Test
		{
		CheckAllow	(Token,ALLOW_Cmd);
		EmitB(SC_Test);
		}
	else if (Token.Matches("TestEval")) // TestEval
		{
		CheckAllow	(Token,ALLOW_Cmd);
		EmitB(SC_TestEval);
		GetExpr(CPT_None,NULL);
		}
	else if (Token.Matches("If")) // If
		{
		CheckAllow	(Token,ALLOW_Cmd);
		PushNest	(NEST_If,"");
		TopNest->Allow |= ALLOW_ElseIf;
		}
	else if (Token.Matches("ElseIf")) // ElseIf
		{
		CheckAllow	(Token,ALLOW_ElseIf);
		PopNest		(NEST_If,"ElseIf");
		PushNest	(NEST_If,"");
		TopNest->Allow |= ALLOW_ElseIf;
		}
	else if (Token.Matches("Else"))
		{
		CheckAllow(Token,ALLOW_ElseIf);
		PopNest(NEST_If,"Else");
		PushNest(NEST_If,"");
		// Don't allow ElseIf here
		}
	else if (Token.Matches("EndIf"))
		{
		PopNest(NEST_If,"EndIf");
		}
	else if (Token.Matches("While"))
		{
		CheckAllow(Token,ALLOW_Cmd);
		PushNest(NEST_Loop,"");
		}
	else if (Token.Matches("Do"))
		{
		CheckAllow(Token,ALLOW_Cmd);
		PushNest(NEST_Loop,"");
		}
	else if (Token.Matches("Until"))
		{
		PopNest(NEST_Loop,"Until");
		}
	else if (Token.Matches("EndWhile") || Token.Matches("EndDo") || Token.Matches("Loop") || Token.Matches("WEnd"))
		{
		PopNest(NEST_Loop,Token.Identifier);
		}
	else if (Token.Matches("For"))
		{
		CheckAllow(Token,ALLOW_Cmd);
		PushNest(NEST_For,"");
		}
	else if (Token.Matches("Next"))
		{
		PopNest(NEST_For,"Next");
		}
	else if (Token.Matches("Goto"))
		{
		CheckAllow(Token,ALLOW_Cmd);
		}
	else throwf("Bad command %s",Token.Identifier);
	//
	// Make sure no junk is left over:
	//
	if (GetToken(Token)) throwf("Misplaced '%s'",Token.Descr());
	return 1;
	};

/*-----------------------------------------------------------------------------
	FStackNode
-----------------------------------------------------------------------------*/

//
// Init a regular node
//
void FStackNode::Init(ENestType ThisNestType,FName ThisName)
	{
	ChildFunctions.Init();
	ChildStates.Init();
	Next.Init();
	//
	PropertiesOffset		= 0;
	DataOffset				= 0;
	DataLength				= 0;
	NumProperties			= 0;
	//
	CodeStart				= 0;
	NestType				= ThisNestType;
	Name					= ThisName;
	//
	StackNodeFlags			= 0;
	OperParams				= 0;
	OperType				= CPT_None;
	OperPrecedence			= 0;
	//
	switch (ThisNestType)
		{
		case NEST_None:
			throwf("Internal: Illegal stack tree");
			break;
		case NEST_Class:
			if (ThisName==NAME_NONE) throwf("Internal: Missing name");
			break;
		case NEST_State:
			Name = ThisName;
			if (ThisName==NAME_NONE) throwf("Internal: Missing name");
			break;
		case NEST_When:
		case NEST_Function:
		case NEST_Operator:
			if (ThisName==NAME_NONE) throwf("Internal: Missing name");
			break;
		case NEST_If:
		case NEST_Loop:
		case NEST_Select:
		case NEST_Case:
		case NEST_For:
			//throwf("Not supported");
			break;
		};
	};

//
// Return all resource and name references in a stack tree node.
//
void FStackNode::QueryReferences(UResource *Res,FResourceCallback &Callback, DWORD ResourceContextFlags)
	{
	Callback.Name(Res,&Name,ResourceContextFlags);
	};

/*-----------------------------------------------------------------------------
	Main script compiling routine
-----------------------------------------------------------------------------*/

//
// Compile the script associated with the specified class.
// Returns 1 if compilation was a success, 0 if any errors occured.
//
int FScriptCompiler::Compile(UClass *CompileClass,FMemPool *MemPool,int ActorPropertiesAreValid)
	{
	GUARD;
	Mem = MemPool;
	void *MemTop = Mem->Get(0);
	char *ActorPropertiesText=NULL;
	void *SavedClassData,*SavedClass;
	int  ClassDataSize,Success = 0;
	//
	// Set up:
	//
	Class			= CompileClass;
	Script			= new(Class->Name,CREATE_Replace)UScript;
	Class->Script	= Script;
	Script->Class	= Class;
	//
	// Save stuff for later recall:
	//
	ClassDataSize  = Class->QueryMinSize();
	SavedClass	   = Mem->Get(sizeof(UClass));	memcpy(SavedClass,    Class,          sizeof(UClass));
	SavedClassData = Mem->Get(ClassDataSize);	memcpy(SavedClassData,Class->GetData(),ClassDataSize);
	//
	Class->Intrinsic = 0; // Declare as non-intrinsic until declared otherwise
	//
	if (ActorPropertiesAreValid)
		{
		ActorPropertiesText = (char *)Mem->Get(65536);
		int Len=ExportActor(&Class->DefaultActor,ActorPropertiesText,NAME_NONE,0,0,CPF_Edit,NULL,0,-1,NAME_NONE);
		};
	//
	// Init stack tree:
	//
	StackTree			 = (FStackNode *)Mem->Get(MAX_STACK_TREE * sizeof(FStackNode));
	Script->NumStackTree = 0;
	//
	// Init compiler variables:
	//
	Input				= Class->ScriptText->GetData();
	InputSize			= Class->ScriptText->Num;
	InputPos			= 0;
	InputLine			= 1;
	PrevPos				= 0;
	PrevLine			= 1;
	Decompile			= 0;
	//
	// Init nesting:
	//
	NestLevel			= 0;
	TopNest				= &Nest[-1];
	PushNest(NEST_None,"");
	//
	// Init code:
	//
	CodeStart			= (BYTE *)Mem->Get(MAX_CODE_SIZE);
	CodeTop				= CodeStart+8; // 8-byte padding
	//
	// Init global properties:
	//
	Class->Num			= 0;
	//
	// Init local properties:
	//
	TotalLocalProperties	= 0;
	TotalLocalDataSize		= 0;
	LocalProperties			= (FClassProperty *)Mem->Get(16 * UClass::MAX_CLASS_PROPERTIES * sizeof(FClassProperty));
	LocalData				= (BYTE           *)Mem->Get(16 * PROPERTIES_DATA_SIZE         * sizeof(BYTE));;
	//
	// Begin:
	//
	AddResultText("Compiling %s...\r\n",Class->Name);
	GApp->StatusUpdatef("Compiling %s",0,0,Class->Name);
	try // Compile until we get an error
		{
		if (Class->ParentClass && !Class->ParentClass->Script)
			{
			throwf("'%s' can't be compiled: Parent script '%s' has errors",Class->Name,Class->ParentClass->Name);
			};
		while (CompileStatement()) StatementsCompiled++;
		if (NestLevel==0) throwf("Internal nest inconsistency");
		if (NestLevel==1) throwf("Missing 'Class' or 'Module' definition");
		if (NestLevel>2)  throwf("Unexpected end of script in '%s' block",NestTypeName(TopNest->NestType));
		//
		// Update the default actor:
		//
		Class->DefaultActor.Class = Class; // Must set before calling ImportActorProperties
		if (ActorPropertiesAreValid) ImportActorProperties(&Class->DefaultActor,ActorPropertiesText);
		Success = 1;
		//
		// Allocate the script's data, and move the state tree, property list, property data, and
		// code pool into the script's data while relocating their various offsets:
		//
		int StackTreeSize		= Script->NumStackTree * sizeof(FStackNode);
		int LocalPropertySize	= TotalLocalProperties * sizeof(FClassProperty);
		int CodeSize			= (int)(CodeTop-CodeStart);
		//
		int StackTreeOffset     = 0;
		int LocalPropertyOffset = StackTreeSize;
		int LocalDataOffset     = StackTreeSize + LocalPropertySize;
		int CodeOffset          = StackTreeSize + LocalPropertySize + TotalLocalDataSize;
		//
		Script->Length			= StackTreeSize + LocalPropertySize + TotalLocalDataSize + CodeSize;
		Script->CodeOffset		= CodeOffset;
		Script->AllocData(1);
		BYTE *Data				= Script->GetData();
		//
		//AddResultText("Props=%i, Data=%i -> %i\r\n",TotalLocalProperties,TotalLocalDataSize,Class->Script->Length);
		//
		for (int i=0; i<Script->NumStackTree; i++)
			{
			FStackNode *Node = &StackTree[i];
			int iFirstProperty = Node->PropertiesOffset;
			int iFirstData     = Node->DataOffset;
			//
			// Update and copy ith stack tree:
			//
			Node->PropertiesOffset = LocalPropertyOffset;
			Node->DataOffset       = LocalDataOffset;
			//
			memcpy(Data+StackTreeOffset,Node,sizeof(FStackNode));
			StackTreeOffset += sizeof(FStackNode);
			//
			if (Node->NumProperties)
				{
				//
				// Copy local properties from ith stack tree:
				//
				memcpy
					(
					Data + LocalPropertyOffset,
					&LocalProperties[iFirstProperty],
					Node->NumProperties * sizeof(FClassProperty)
					);
				LocalPropertyOffset += Node->NumProperties * sizeof(FClassProperty);
				//
				// Copy local data from the ith stack tree:
				//
				memcpy
					(
					Data + LocalDataOffset,
					&LocalData[iFirstData],
					Node->DataLength
					);
				LocalDataOffset += Node->DataLength;
				};
			};
		memcpy(Data+CodeOffset,CodeStart,CodeSize);
		//
		// Update "make" info:
		//
		Class->ScriptTextCRC  = Class->ScriptText->DataCRC();
		//
		if (Class->ParentClass && Class->ParentClass->ScriptText) Class->ParentTextCRC = Class->ParentClass->ScriptText->DataCRC();
		else Class->ParentTextCRC = 0;
		//
		if (Decompile)
			{
			AddResultText("Script decompilation:\r\n");
			UTextBuffer *Text = new("Decompile",CREATE_Replace)UTextBuffer(65536);
			Class->Script->Decompile(Text->GetData(),Decompile==2);
			AddResultText(Text->GetData());
			Text->Kill();
			};
		}
	catch (char *ErrorMsg) // Handle compiler error
		{
		AddResultText("Error in %s, Line %i: %s\r\n",Class->Name,InputLine,ErrorMsg);
		//
		Class->Script->Kill();
		//
		// Restore the class to its precompiled state:
		//
		memcpy(Class,           SavedClass,    sizeof(UClass));
		memcpy(Class->GetData(),SavedClassData,ClassDataSize);
		//
		// Kill compiled script resource, preventing incomplete code from being executed:
		//
		Class->Script             = NULL;
		Class->DefaultActor.Class = Class;
		//
		// Mark as bad:
		//
		Class->ScriptTextCRC  = 0;
		Class->ParentTextCRC  = 0;
		};
	LinesCompiled += InputLine;
	Mem->Release(MemTop);
	return Success;
	UNGUARD("FScriptCompiler::Compile");
	};

/*-----------------------------------------------------------------------------
	FScriptCompiler error handling
-----------------------------------------------------------------------------*/

//
// Print a formatted debugging message (same format as printf)
//
void VARARGS FScriptCompiler::AddResultText(char *Fmt,...)
	{
	GUARD;
	char 	 TempStr[4096];
	va_list  ArgPtr;
	//
	va_start (ArgPtr,Fmt);
	vsprintf (TempStr,Fmt,ArgPtr);
	va_end   (ArgPtr);
	//
	debugf(LOG_Script,TempStr);
	strcat(ErrorText->GetData(),TempStr);
	UNGUARD("FScriptCompiler::AddResultText");
	};

/*-----------------------------------------------------------------------------
	FScriptCompiler debugging
-----------------------------------------------------------------------------*/

//
// Display the compiler's complete internal nesting state:
//
void FScriptCompiler::DebugDumpNest(void)
	{
	AddResultText("Compiler nesting levels:\r\n");
	//
	FNestInfo *NestInfo = &Nest[1];
	for (int i=1; i<NestLevel; i++)
		{
		AddResultText("   Level %i: %s, Allow=%i\r\n",i,NestTypeName(NestInfo->NestType),NestInfo->Allow);
		NestInfo++;
		};
	};

void FClassProperty::DebugDump(char *Str)
	{
	sprintf(Str,"%s",PropertyName.Name());
	};

//
// Display the contents of a token structure:
//
void FScriptCompiler::DebugDumpToken(FToken &Token)
	{
	switch (Token.Type)
		{
		case TOKEN_End:
			AddResultText("TOKEN_End\r\n");
			break;
		case TOKEN_Identifier:
			AddResultText("TOKEN_Identifier: %s\r\n",Token.Identifier);
			break;
		case TOKEN_Symbol:
			AddResultText("TOKEN_Symbol: %s\r\n",Token.Identifier);
			break;
		case TOKEN_IntegerConst:
			AddResultText("TOKEN_IntegerConst: %i\r\n",Token.Integer);
			break;
		case TOKEN_FloatConst:
			AddResultText("TOKEN_FloatConst: %f\r\n",Token.Float);
			break;
		case TOKEN_StringConst:
			AddResultText("TOKEN_StringConst: <%s>\r\n",Token.String);
			break;
		case TOKEN_ResourceConst:
			AddResultText("TOKEN_ResourceConst: <%s(%s)>\r\n",GRes.Types[Token.ResType],Token.Res->Name);
			break;
		case TOKEN_VectorConst:
			AddResultText("TOKEN_VectorConst: <Vector(%f,%f,%f)>\r\n",Token.Vector.X,Token.Vector.Y,Token.Vector.Z);
			break;
		case TOKEN_RotationConst:
			AddResultText("TOKEN_RotationConst: <Rotation(%i,%i,%i)>\r\n",Token.Rotation.Pitch,Token.Rotation.Yaw,Token.Rotation.Roll);
			break;
		case TOKEN_NameConst:
			AddResultText("TOKEN_NameConst: <Name(%s)>\r\n",Token.Name.Name());
			break;
		default:
			AddResultText("TOKEN_UNKNOWN\r\n");
			break;
		};
	};

/*-----------------------------------------------------------------------------
	Global functions
-----------------------------------------------------------------------------*/

//
// Resource flags used for make:
//
enum
	{
	RF_DID_ANALYZE = RF_Temp1, // Set for classes that were analyzed (and maybe recompiled)
	};

//
// Compile the specified actor class's text script and output the
// results into its token script.
//
// Assumes:
// * Class->ScriptText contains the script text.
// * Class has been initialized as a valid resource
// * ActorPropertiesAreValid indicates whether actor properties are set to meaningful values
//
// Does not assume:
// * Class contains valid properties
//
int UNREAL_API CompileScript(UClass *Class,int ActorPropertiesAreValid)
	{
	GUARD;
	FScriptCompiler	Compiler;
	int Success;
	//
	Compiler.InitMake();
	//
	if (Class->ScriptText)	Success = Compiler.Compile(Class,&GMem,ActorPropertiesAreValid);
	else					Success = 0;
	//
	Compiler.ExitMake(Success);
	//
	return Success;
	//
	UNGUARD("CompileScript");
	};

int Make(FScriptCompiler &Compiler,UClass *Class,int MakeAll)
	{
	if (MakeAll || (Class->ScriptText->DataCRC() != Class->ScriptTextCRC) ||
		(Class->ParentClass && Class->ParentClass->ScriptText && 
		(Class->ParentClass->ScriptText->DataCRC() != Class->ParentTextCRC)))
		{
		MakeAll=1;
		if (!Compiler.Compile(Class,&GMem,1)) return 0;
		};
	UClass *TestClass;
	FOR_ALL_TYPED_RES(TestClass,RES_Class,UClass)
		{
		if ((TestClass->ParentClass==Class) && !Make(Compiler,TestClass,MakeAll)) return 0;
		}
	END_FOR_ALL_RES;
	//
	return 1; // Success
	};

//
// Make scripts hierarchically.
//
int UNREAL_API MakeScripts(UClass *Class,int MakeAll)
	{
	GUARD;
	FScriptCompiler	Compiler;
	//
	Compiler.InitMake();
	//
	if (MakeAll)	Compiler.AddResultText("Making all scripts.\r\n");	
	else			Compiler.AddResultText("Making changed scripts.\r\n");
	//
	// Go through all classes in an arbitrary order and make sure each one
	// is conditionally remade:
	//
	int Success = Make(Compiler,Class,MakeAll);
	//
	Compiler.ExitMake(Success);
	//
	return Success;
	//
	UNGUARD("MakeScripts");
	};

/*-----------------------------------------------------------------------------
	Script Resource Implementation
-----------------------------------------------------------------------------*/

void UScript::Register(FResourceType *Type)
	{
	GUARD;
	Type->HeaderSize = sizeof (UScript);
	Type->RecordSize = 1;
	Type->Version    = 1;
	strcpy (Type->Descr,"Script");
	UNGUARD("UScript::Register");
	};
void UScript::InitHeader(void)
	{
	GUARD;
	Class			= NULL;
	Length			= 0;
	NumStackTree	= 0;
	UNGUARD("UScript::InitHeader");
	};
void UScript::InitData(void)
	{
	GUARD;
	mymemset(Data,0,Length);
	UNGUARD("UScript::InitData");
	};
int UScript::QuerySize(void)
	{
	GUARD;
	return Length;
	UNGUARD("UScript::QuerySize");
	};
int UScript::QueryMinSize(void)
	{
	GUARD;
	return QuerySize();
	UNGUARD("UScript::QueryMinSize");
	};
const char *UScript::Import(const char *Buffer, const char *BufferEnd,const char *FileType)
	{
	GUARD;
	return Buffer;
	UNGUARD("UScript::Import");
	};
char *UScript::Export(char *Buffer,const char *FileType,int Indent)
	{
	GUARD;
	memcpy(Buffer,GetData(),Length);
	return Buffer;
	UNGUARD("UScript::Export");
	};
void UScript::QueryHeaderReferences(FResourceCallback &Callback)
	{
	GUARD;
	//
	Callback.Resource (this,(UResource **)&Class,0);
	//
	UNGUARD("UScript::QueryHeaderReferences");
	};
void UScript::QueryDataReferences(FResourceCallback &Callback)
	{
	GUARD;
	//
	FStackNode *StackTree = (FStackNode *)GetData();
	for (int i=0; i<NumStackTree; i++)
		{
		StackTree[i].QueryReferences(this,Callback,0);
		};
	UNGUARD("UScript::QueryDataReferences");
	};
AUTOREGISTER_RESOURCE(RES_Script,UScript,0xB2D90873,0xCCD211cf,0x91360000,0xC028B992);

/*-----------------------------------------------------------------------------
	UScript Custom Implementation
-----------------------------------------------------------------------------*/

//
// Decompile a subexpression:
//
char *DecompileExpr(UScript *Script,INT iTree,char *Str,BYTE **ThisCode)
	{
	GUARD;
	BYTE *Code = *ThisCode;
	//
	EExprToken E = (EExprToken) *Code++;
	switch (E)
		{
		case EX_Nothing:
			Str += sprintf(Str,"Nothing");
			break;
		case EX_LocalVariable:
		case EX_GlobalVariable:
		case EX_ArrayElement:
			{
			int IsArrayElement = 0;
			if (E==EX_ArrayElement)	
				{
				IsArrayElement = 1;
				E = (EExprToken) *Code++;
				if ((E!=EX_LocalVariable)&&(E!=EX_GlobalVariable)) appError("ArrayElement inconsistency");
				};
			EClassPropertyType Type = (EClassPropertyType) *Code++;
			WORD Offset = *(WORD *)Code; Code += 2;
			Str += sprintf(Str,"[%s:%s+%i",(E==EX_GlobalVariable)?"G":"L",GetPropertyTypeName(Type),Offset);
			//
			if (IsArrayElement)
				{
				Str += sprintf(Str,"(");
				Str  = DecompileExpr(Script,iTree,Str,&Code);
				Str += sprintf(Str,")");
				};
			Str += sprintf(Str,"]");
			};
			break;
		case EX_Function:
		case EX_FastFunction:
			{
			if (E==EX_FastFunction)
				{
				int     iNode         = *(WORD     *)Code; Code += 2;
				UScript *FuncScript   = *(UScript **)Code; Code += 4;
				FStackNode *FuncNode  = &((FStackNode *)FuncScript->GetData())[iNode];
				Str                  += sprintf(Str,"%s[%s.%i](",FuncNode->Name.Name(),FuncScript->Name,iNode);
				}
			else // E==EX_Function
				{
				FName Name  = *(FName *)Code; Code += sizeof(FName);
				Str        += sprintf(Str,"%s[?](",Name.Name());
				};
			while (*Code != EX_Nothing)
				{
				Str = DecompileExpr(Script,iTree,Str,&Code);
				if (*Code!=EX_Nothing) Str += sprintf(Str,",");
				};
			Str += sprintf(Str,")");
			}
			break;
		case EX_IntegerConst:
			Str  += sprintf(Str,"%i",*(INT *)Code);
			Code += sizeof(INT);
			break;
		case EX_RealConst:
			Str  += sprintf(Str,"%f",*(FLOAT *)Code);
			Code += sizeof(FLOAT);
			break;
		case EX_StringConst:
			Str  += sprintf(Str,"\"%s\"",(char *)Code);
			Code += strlen((char *)Code)+1;
			break;
		case EX_ResourceConst:
			Str += sprintf(Str,"%s(%s)",
				GRes.Types[*(BYTE *)Code].Descr,
				(*(UResource **)(Code+1))->Name);
			Code += sizeof(BYTE) + sizeof(UResource *);
			break;
		case EX_NameConst:
			Str += sprintf(Str,"%s",((FName *)Code)->Name());
			Code += sizeof(FName);
			break;
		case EX_RotationConst:
			{
			FRotation *R = (FRotation *)Code;
			Code += sizeof(FRotation);
			//
			Str += sprintf(Str,"Rotation(%i,%i,%i)",R->Pitch,R->Yaw,R->Roll);
			};
			break;
		case EX_VectorConst:
			{
			FVector *V = (FVector *)Code;
			Code += sizeof(FVector);
			//
			Str += sprintf(Str,"Vector(%f,%f,%f)",V->X,V->Y,V->Z);
			};
			break;
		case EX_Error:
			appErrorf("Encountered EX_Error",E);
			break;
		default:
			appErrorf("Unrecognized expr token %i",E);
			break;
		};
	*ThisCode = Code;
	return Str;
	//
	UNGUARD("DecompileExpr");
	};


//
// Decompile everything at a particular stack tree node, including
// variable declarations, functions/events, states, and code.
//
char *DecompileStackNode(UScript *Script,INT iTree,char *Str,int Indent,int ParentLinks)
	{
	GUARD;
	//
	UClass		*Class  = Script->Class;
	FStackNode	*Node	= &((FStackNode *)Script->GetData())[iTree];
	//
	// Decompile peer stack nodes first, since they exist in the linked list in
	// reverse order:
	//
	if (ParentLinks ? (Node->Next.Script!=NULL) : (Node->Next.Script==Script))
		{
		Str = DecompileStackNode (Node->Next.Script,Node->Next.iNode,Str,Indent,ParentLinks);
		};
	//
	// Figure out parameters/variables:
	//
	FClassProperty *Properties;
	BYTE *Data;
	int iStart,iEnd,ThisIndent;
	//
	if (iTree==0) // Global variables
		{
		if (Class->ParentClass)	iStart = Class->ParentClass->Num;
		else					iStart = 0;
		//
		iEnd		= Class->Num;
		Properties	= &Class->Element(0);
		Data		= (BYTE *)&Class->DefaultActor;
		ThisIndent  = Indent;
		}
	else 
		{
		iStart		= 0;
		iEnd		= Node->NumProperties;
		//
		if (Node->NumProperties>0)
			{
			Properties	= (FClassProperty *)(&Script->GetData()[Node->PropertiesOffset]);
			Data		= &Script->GetData()[Node->DataOffset];
			ThisIndent  = Indent+4;
			};
		};
	//
	// Command that caused the nesting:
	//
	switch (Node->NestType)
		{
		case NEST_None:
			Str += sprintf(Str,"%s#Error, empty stack node",spc(Indent));
			break;
		case NEST_Class:
			// Class <name> [Expands <parent>]:
			Str += sprintf(Str,"%sClass %s",spc(Indent),Class->Name);
			if (Class->ParentClass) Str += sprintf(Str," Expands %s",Class->ParentClass->Name);
			if (Class->Intrinsic)   Str += sprintf(Str," Intrinsic");
			Str += sprintf(Str,"\r\n");
			break;
		case NEST_State:
			// State <name> [,[Auto] [Editable]]
			Str += sprintf(Str,"%sState %s\r\n",spc(Indent),Node->Name.Name());
			break;
		case NEST_When:
		case NEST_Function:
		case NEST_Operator:
			{
			int First=1;
			Str += sprintf(Str,"%s",spc(Indent));
			//
			if		(Node->NestType==NEST_Function)	Str+=sprintf(Str,"Function ");
			else if (Node->NestType==NEST_Operator)	Str+=sprintf(Str,"Operator ");
			else									Str+=sprintf(Str,"When ");
			//
			Str += sprintf(Str,"%s(",Node->Name.Name());
			while ((iStart<iEnd) && (Properties[iStart].PropertyFlags & CPF_Param))
				{
				if (!First) Str += sprintf(Str,", ");
				Str = Properties[iStart++].ExportTCX(Str,Data);
				First=0;
				};
			Str += sprintf(Str,")");
			if ((iStart<iEnd) && (Properties[iStart].PropertyFlags & CPF_ReturnValue))
				{
				Str += sprintf(Str," ");
				Str = Properties[iStart++].ExportTCX(Str,Data);
				};
			Str += sprintf(Str,"\r\n");
			break;
			};
		case NEST_If:
			Str += sprintf(Str,"%sIf ?\r\n",spc(Indent));
			break;
		case NEST_Loop:
			Str += sprintf(Str,"%sWhile ?\r\n",spc(Indent));
			break;
		case NEST_Select:
			Str += sprintf(Str,"%sSelect Case ?\r\n",spc(Indent));
			break;
		case NEST_Case:
			Str += sprintf(Str,"%sCase ?\r\n",spc(Indent));
			break;
		case NEST_For:
			Str += sprintf(Str,"%sFor ?=? to ?\r\n",spc(Indent));
			break;
		default:
			Str += sprintf(Str,"%s#Error: unknown nest type\r\n",spc(Indent));
			break;
		};
	//
	// Variable declarations at this stack node:
	//
	if ((iTree==0) || (Node->NumProperties>0))
		{
		while (iStart < iEnd)
			{
			Str += sprintf(Str,"%s",spc(ThisIndent));
			Str  = Properties[iStart].ExportTCX(Str,Data);
			Str += sprintf(Str,"\r\n");
			//
			iStart++;
			};
		};
	//
	// Code at this stack node:
	//
	if (Node->CodeStart)
		{
		BYTE *Code = &Script->GetData()[Script->CodeOffset + Node->CodeStart];
		while (1)
			{
			BYTE B = *Code++;
			switch(B)
				{
				case SC_End:
					Str += sprintf(Str,"%s   SC_End\r\n",spc(Indent));
					goto Done;
				case SC_Let:
				case SC_Function:
				case SC_For:
				case SC_While:
				case SC_Select:
				case SC_GotoCode:
				case SC_GotoState:
				case SC_If:
				case SC_Error:
					Str += sprintf(Str,"%s   SC_Unimplemented\r\n",spc(Indent));
				case SC_Test:
					Str += sprintf(Str,"%s   SC_Test\r\n",spc(Indent));
					break;
				case SC_TestEval:
					Str += sprintf(Str,"%s   SC_TestEval(",spc(Indent));
					Str  = DecompileExpr(Script,iTree,Str,&Code);
					Str += sprintf(Str,")\r\n");
					break;
				default:
					throwf("Illegal code token");
					break;
				};
			};
		Done:;
		};
	//
	// Child functions and states:
	//
	if (ParentLinks ? (Node->ChildFunctions.Script!=NULL) : (Node->ChildFunctions.Script==Script))
		{
		Str = DecompileStackNode (Node->ChildFunctions.Script,Node->ChildFunctions.iNode,Str,Indent+4,ParentLinks);
		};
	if (ParentLinks ? (Node->ChildStates.Script!=NULL) : (Node->ChildStates.Script==Script))
		{
		Str = DecompileStackNode (Node->ChildStates.Script,Node->ChildStates.iNode,Str,Indent+4,ParentLinks);
		};
	//
	// Command that ended the nesting:
	//
	switch (Node->NestType)
		{
		case NEST_None:		Str += sprintf(Str,"%s#EndError\r\n",	spc(Indent)); break;
		case NEST_Class:	Str += sprintf(Str,"%sEndClass\r\n",	spc(Indent)); break;
		case NEST_State:	Str += sprintf(Str,"%sEndState\r\n",	spc(Indent)); break;
		case NEST_When:		Str += sprintf(Str,"%sEndWhen\r\n",		spc(Indent)); break;
		case NEST_Function:	Str += sprintf(Str,"%sEndFunction\r\n",	spc(Indent)); break;
		case NEST_Operator:	Str += sprintf(Str,"%sEndFunction\r\n",	spc(Indent)); break;
		case NEST_If:		Str += sprintf(Str,"%sEndIf\r\n",		spc(Indent)); break;
		case NEST_Loop:		Str += sprintf(Str,"%sEndWhile\r\n",	spc(Indent)); break;
		case NEST_Select:	Str += sprintf(Str,"%sEndSelect\r\n",	spc(Indent)); break;
		case NEST_Case:		Str += sprintf(Str,"%sEndCase\r\n",		spc(Indent)); break;
		case NEST_For:		Str += sprintf(Str,"%sNext\r\n",		spc(Indent)); break;
		default:			Str += sprintf(Str,"%s#EndError\r\n",	spc(Indent)); break;
		};
	return Str;
	//
	UNGUARD("DecompileStackNode");
	};

//
// Decompile a script
//
char *UScript::Decompile(char *Str,int ParentLinks)
	{
	GUARD;
	//
	Str = DecompileStackNode(this,0,Str,0,ParentLinks);
	//
	return Str;
	UNGUARD("UScript::Decompile");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
