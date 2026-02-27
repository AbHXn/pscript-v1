#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>

using namespace std;

unordered_set<string> REGISTERED_FUNC_TOKEN = {
	"thenga", "(", ")", "{", "}", ",", "pidi", "kootam"
};

unordered_set<string> REGISTERED_FUNC_BODY_TOKENS = {
	"poda"
};

enum class FUNC_TOKENS{
	NOTHING,
	FUNC_START,
	FUNC_NAME,
	ARGS_OPEN,
	ARGS_CLOSE,
	BODY_OPEN,
	FUNC_BODY,
	BODY_CLOSE,
	VAR_START,
	VAR_NAME,
	VAR_ARRAY,
	VAR_COMMA
};

struct ARG_VAR_INFO{
	string name;
	bool isArray;
};

struct FunctionTokenReturn {
	vector<FUNC_TOKENS> tokens;
	vector<unique_ptr<ARG_VAR_INFO>> args;
	string funcName;
	size_t funcStartPtr;
	size_t funcEndPtr;

	FunctionTokenReturn( 
		vector<FUNC_TOKENS> tokens,
		vector<unique_ptr<ARG_VAR_INFO>> args,
		string funcName,
		size_t funcStartPtr,
		size_t funcEndPtr
	){
		this->tokens 		= tokens;
		this->args 			= move(args);
		this->funcName 		= funcName;
		this->funcStartPtr  = funcStartPtr;
		this->funcEndPtr	= funcEndPtr;
	}
};

unordered_map<FUNC_TOKENS, vector<FUNC_TOKENS>> FUNC_GRAPH = {
	{ FUNC_TOKENS::FUNC_START, 	{ FUNC_TOKENS::FUNC_NAME } 	},
	{ FUNC_TOKENS::FUNC_NAME, 	{ FUNC_TOKENS::ARGS_OPEN }  },
	{ FUNC_TOKENS::VAR_START, 	{ FUNC_TOKENS::VAR_NAME }   },
	{ FUNC_TOKENS::ARGS_OPEN,  { FUNC_TOKENS::ARGS_CLOSE, 
								  FUNC_TOKENS::VAR_START }  },
	{ FUNC_TOKENS::VAR_NAME, 	{ FUNC_TOKENS::VAR_COMMA, 
							  	  FUNC_TOKENS::ARGS_CLOSE, 
							      FUNC_TOKENS::VAR_ARRAY } 	},
    { FUNC_TOKENS::VAR_COMMA,   { FUNC_TOKENS::VAR_START } 	},
    { FUNC_TOKENS::VAR_ARRAY,   { FUNC_TOKENS::VAR_COMMA, 
    							  FUNC_TOKENS::ARGS_CLOSE } },
    { FUNC_TOKENS::ARGS_CLOSE,  { FUNC_TOKENS::BODY_OPEN } 	},
    { FUNC_TOKENS::BODY_OPEN,   { FUNC_TOKENS::FUNC_BODY }  },
    { FUNC_TOKENS::FUNC_BODY,   { FUNC_TOKENS::BODY_CLOSE } },
};

bool 
isValidFunction( vector<FUNC_TOKENS>& tokens ){
	size_t startIndex = 0;
	FUNC_TOKENS currentStage = FUNC_TOKENS::FUNC_START;

	while( startIndex < tokens.size() ){
		FUNC_TOKENS newTok = tokens[ startIndex ];
		if( newTok == FUNC_TOKENS::BODY_CLOSE )
			return true;
		if( startIndex + 1 < tokens.size() )
			startIndex++;
		else break;

		FUNC_TOKENS nextExpected = tokens[ startIndex ];
		vector<FUNC_TOKENS>& nextExpectedTokens = FUNC_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( FUNC_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError("Invalid Syntax");
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter end } token in thenga" );
}

FunctionTokenReturn
stringToFuncTokens( const vector<Token>&tokens, size_t& startIndex ){
	vector<FUNC_TOKENS> funcTokens;
	FUNC_TOKENS prev = FUNC_TOKENS::NOTHING;
	string funcName;
	vector<unique_ptr<ARG_VAR_INFO>> args;
	size_t bodyStart = 0;

	for( ; startIndex < tokens.size(); startIndex++ ){
		const string& curToken = tokens[ startIndex ].token;

		if( curToken == "thenga" )
			funcTokens.push_back( FUNC_TOKENS::FUNC_START );
		else if( curToken == "(" )
			funcTokens.push_back( FUNC_TOKENS::ARGS_OPEN );
		else if( curToken == ")" )
			funcTokens.push_back( FUNC_TOKENS::ARGS_CLOSE );
		else if( curToken == "pidi" )
			funcTokens.push_back( FUNC_TOKENS::VAR_START );
		else if( curToken == "," )
			funcTokens.push_back( FUNC_TOKENS::VAR_COMMA );
		else if( curToken == "{" ){
			funcTokens.push_back( FUNC_TOKENS::BODY_OPEN );
			funcTokens.push_back( FUNC_TOKENS::FUNC_BODY );
			bodyStart = startIndex;
			startIndex++;
			
			int open = 1;
			while( startIndex < tokens.size() ){
				const string& curToken = tokens[ startIndex++ ].token;
				if( curToken == "}" ){
					open--;
					if( !open ){
						funcTokens.push_back( FUNC_TOKENS::BODY_CLOSE );
						return FunctionTokenReturn(funcTokens, move(args), funcName, bodyStart,startIndex);
					}
				}
				else if( curToken == "{" )
					++open;
			}
		}
		else{
			if( prev == FUNC_TOKENS::FUNC_START ){
				funcName = curToken;
				funcTokens.push_back( FUNC_TOKENS::FUNC_NAME );
			}
			else if( prev == FUNC_TOKENS::VAR_START ){
				unique_ptr<ARG_VAR_INFO> newVar = make_unique<ARG_VAR_INFO>();
				newVar->name = curToken;
				funcTokens.push_back( FUNC_TOKENS::VAR_NAME );
				args.push_back( move(newVar) );
			}
			else if( curToken == "kootam" ){
				if( args.empty() )
					throw InvalidSyntaxError(" kootam keyword misuse ");
				auto& backArg = args.back();
				backArg->isArray = true;
			}
			else throw InvalidSyntaxError("Unknown token " + curToken );
		}
		prev = !funcTokens.empty() ? funcTokens.back() : prev;
	}
	throw InvalidSyntaxError("Invalid syntax error in thenga declaration");
}

/* --------------------------- FUNC CALL HANDLER --------------------------------*/

enum class FUNC_CALL_TOKEN{
	NOTHING, // 0
	FUNC_NAME, // 1
	ARG_OPEN, // 2
	ARG_COMMA, // 3
	ARG_VALUE, // 4
	ARG_CLOSE, // 5
};

unordered_map<FUNC_CALL_TOKEN, vector<FUNC_CALL_TOKEN>> FUNC_CALL_GRAPH = {
	{ FUNC_CALL_TOKEN::FUNC_NAME, { FUNC_CALL_TOKEN::ARG_OPEN } },
	{ FUNC_CALL_TOKEN::ARG_OPEN,  { FUNC_CALL_TOKEN::ARG_VALUE, FUNC_CALL_TOKEN::ARG_CLOSE } },
	{ FUNC_CALL_TOKEN::ARG_VALUE, { FUNC_CALL_TOKEN::ARG_COMMA, FUNC_CALL_TOKEN::ARG_CLOSE } },
	{ FUNC_CALL_TOKEN::ARG_COMMA, { FUNC_CALL_TOKEN::ARG_VALUE } },
	{ FUNC_CALL_TOKEN::ARG_VALUE, { FUNC_CALL_TOKEN::ARG_CLOSE } }
};

bool isFuncPtr( vector<FUNC_CALL_TOKEN>& callTokens ){
	return callTokens.size() == 1 && callTokens.back() == FUNC_CALL_TOKEN::FUNC_NAME;
}

bool
isValidFuncCall( vector<FUNC_CALL_TOKEN>& callTokens ){
	if( callTokens.empty() )
		return false;

	FUNC_CALL_TOKEN currentStage = FUNC_CALL_TOKEN::FUNC_NAME;
	size_t startIndex = 0;

	while( startIndex < callTokens.size() ){
		if( callTokens[ startIndex ] == FUNC_CALL_TOKEN::ARG_CLOSE )
			return true;
		if( startIndex + 1 < callTokens.size() )
			startIndex++;
		else break;
		FUNC_CALL_TOKEN nextExpected = callTokens[ startIndex ];
		vector<FUNC_CALL_TOKEN>& nextExpectedTokens = FUNC_CALL_GRAPH[ currentStage ];
		bool continueNext = false;
		for( FUNC_CALL_TOKEN nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError( "Invalid Token encounter in thenga call" );
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter ) token in thenga call" );
}

struct FunctionCallReturns{
	vector<FUNC_CALL_TOKEN> callTokens;
	vector<vector<Token>> argsVector;
	string funcName;

	FunctionCallReturns() = default;

	FunctionCallReturns( 
		vector<FUNC_CALL_TOKEN> callTokens,
		vector<vector<Token>> argsVector,
		string funcName
	){
		this->callTokens = callTokens;
		this->argsVector = argsVector;
		this->funcName 	 = funcName;
	}
};

FunctionCallReturns stringToFunctionCallTokens( const vector<Token>& tokens, size_t& curPtr ){
	vector<FUNC_CALL_TOKEN> ctokens;
	vector<vector<Token>> argsTokens;
	string funcCallName;

	FUNC_CALL_TOKEN prev = FUNC_CALL_TOKEN::NOTHING;

	for( ; curPtr < tokens.size(); curPtr++ ){
		const string& curToken = tokens[ curPtr ].token;

		if( prev == FUNC_CALL_TOKEN::NOTHING ){
			ctokens.push_back( FUNC_CALL_TOKEN::FUNC_NAME );
			funcCallName = curToken;
		}
		else if( curToken == "(" ){
			ctokens.push_back( FUNC_CALL_TOKEN::ARG_OPEN );
			int openCnts = 1;

			vector<Token> curVector;
			curPtr++;

			while( curPtr < tokens.size() ){
				const Token& curToken = tokens[ curPtr ];
				curPtr++;

				if( curToken.token == "(" )
					openCnts++;
				else if( curToken.token == ")" )
					openCnts--;

				if( curToken.token == "," && openCnts == 1 ){
					argsTokens.push_back( curVector );
					ctokens.push_back( FUNC_CALL_TOKEN::ARG_VALUE );
					curVector.clear();
					ctokens.push_back( FUNC_CALL_TOKEN::ARG_COMMA );
					continue;
				}
				if( !openCnts ){
					if( !curVector.empty() ){
						ctokens.push_back( FUNC_CALL_TOKEN::ARG_VALUE );
						argsTokens.push_back( curVector );
					}
					ctokens.push_back( FUNC_CALL_TOKEN::ARG_CLOSE );
					return FunctionCallReturns( ctokens, argsTokens, funcCallName );
				}
				curVector.push_back( curToken );
			}
		}
		prev = ctokens.back();
	}
	return FunctionCallReturns( ctokens, argsTokens, funcCallName );
}

#endif