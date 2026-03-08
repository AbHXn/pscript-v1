#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>

std::unordered_set<std::string> REGISTERED_FUNC_TOKEN = { 
	"thenga", "(", ")", "{", "}", ",", "pidi", "kootam" 
};

std::unordered_set<std::string> REGISTERED_FUNC_BODY_TOKENS = {
	"poda"
};

enum class FUNC_TOKENS{
	NOTHING 	,
	FUNC_START 	,
	FUNC_NAME 	,
	ARGS_OPEN 	,
	ARGS_CLOSE 	,
	BODY_OPEN 	,
	FUNC_BODY 	,
	BODY_CLOSE 	,
	VAR_START 	,
	VAR_NAME 	,
	VAR_ARRAY 	,
	VAR_COMMA
};

// For arg Name and Type ( Single or Array )
struct ARG_VAR_INFO{
	std::string name;
	bool isArray;
};

// Func Info
struct FunctionTokenReturn {
	std::vector<FUNC_TOKENS> 			 tokens;
	std::vector<std::unique_ptr<ARG_VAR_INFO>> args;
	std::string 					 funcName;
	size_t 							 funcStartPtr;
	size_t 							 funcEndPtr;

	FunctionTokenReturn( 
		std::vector<FUNC_TOKENS> tokens,
		std::vector<std::unique_ptr<ARG_VAR_INFO>> args,
		std::string funcName,
		size_t funcStartPtr,
		size_t funcEndPtr
	){
		this->tokens 		= tokens;
		this->args 			= std::move(args);
		this->funcName 		= funcName;
		this->funcStartPtr  = funcStartPtr;
		this->funcEndPtr	= funcEndPtr;
	}
};
// Function syntax verifier
std::unordered_map<FUNC_TOKENS, std::vector<FUNC_TOKENS>> FUNC_GRAPH = {
	{ FUNC_TOKENS::FUNC_START, 	{ FUNC_TOKENS::FUNC_NAME } 	},
	{ FUNC_TOKENS::FUNC_NAME, 	{ FUNC_TOKENS::ARGS_OPEN }  },
	{ FUNC_TOKENS::VAR_START, 	{ FUNC_TOKENS::VAR_NAME }   },
	{ FUNC_TOKENS::ARGS_OPEN,  	{ FUNC_TOKENS::ARGS_CLOSE, 
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

void 
passValidFuncToken( std::vector<FUNC_TOKENS>& tokens ){
	size_t startIndex = 0;
	FUNC_TOKENS currentStage = FUNC_TOKENS::FUNC_START;

	while( startIndex < tokens.size() ){
		FUNC_TOKENS newTok = tokens[ startIndex ];
		
		// return if function hits its end without breaking the graph
		if( newTok == FUNC_TOKENS::BODY_CLOSE )
			return ;
		
		if( startIndex + 1 >= tokens.size() )
			break ;

		FUNC_TOKENS nextExpected = tokens[ ++startIndex ];
		std::vector<FUNC_TOKENS>& nextExpectedTokens = FUNC_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( FUNC_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		// throw error if it violate the graph rule
		if( !continueNext )
			throw InvalidSyntaxError("Breaks Function Syntax");
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter end } token in Function" );
}

FunctionTokenReturn
stringToFuncTokens( const std::vector<Token>&tokens, size_t& startIndex ){
	std::vector<FUNC_TOKENS> funcTokens;
	std::vector<std::unique_ptr<ARG_VAR_INFO>> args;
	std::string funcName;

	size_t bodyStart = 0;
	FUNC_TOKENS prev = FUNC_TOKENS::NOTHING;

	for( ; startIndex < tokens.size(); startIndex++ ){
		const std::string& curToken = tokens[ startIndex ].token;

		if( curToken == "thenga" ){
			funcTokens.push_back( FUNC_TOKENS::FUNC_START );
		}
		else if( curToken == "(" ){
			funcTokens.push_back( FUNC_TOKENS::ARGS_OPEN );
		}
		else if( curToken == ")" ){
			funcTokens.push_back( FUNC_TOKENS::ARGS_CLOSE );
		}
		else if( curToken == "pidi" ){
			funcTokens.push_back( FUNC_TOKENS::VAR_START );
		}
		else if( curToken == "," ){
			funcTokens.push_back( FUNC_TOKENS::VAR_COMMA );
		}
		else if( curToken == "{" ){
			funcTokens.push_back( FUNC_TOKENS::BODY_OPEN );
			funcTokens.push_back( FUNC_TOKENS::FUNC_BODY );
			
			bodyStart = startIndex++;
			int openBody = 1;

			while( startIndex < tokens.size() ){
				const std::string& curToken = tokens[ startIndex++ ].token;
				if( curToken == "}" ){
					if( --openBody == 0 ){
						funcTokens.push_back( FUNC_TOKENS::BODY_CLOSE );
						return FunctionTokenReturn(
							funcTokens, std::move(args), funcName, bodyStart, startIndex
						);
					}
				}
				else if( curToken == "{" )
					++openBody;
			}
		}
		else{
			if( prev == FUNC_TOKENS::FUNC_START ){
				funcName = curToken;
				funcTokens.push_back( FUNC_TOKENS::FUNC_NAME );
			}
			else if( prev == FUNC_TOKENS::VAR_START ){
				std::unique_ptr<ARG_VAR_INFO> newVar = std::make_unique<ARG_VAR_INFO>();
				newVar->name = curToken;
				funcTokens.push_back( FUNC_TOKENS::VAR_NAME );
				args.push_back( std::move(newVar) );
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
	NOTHING,   // 0
	FUNC_NAME, // 1
	ARG_OPEN,  // 2
	ARG_COMMA, // 3
	ARG_VALUE, // 4
	ARG_CLOSE, // 5
};

// function call syntax verifier
std::unordered_map<FUNC_CALL_TOKEN, std::vector<FUNC_CALL_TOKEN>> FUNC_CALL_GRAPH = {
	{ FUNC_CALL_TOKEN::FUNC_NAME, { FUNC_CALL_TOKEN::ARG_OPEN } },
	{ FUNC_CALL_TOKEN::ARG_OPEN,  { FUNC_CALL_TOKEN::ARG_VALUE, FUNC_CALL_TOKEN::ARG_CLOSE } },
	{ FUNC_CALL_TOKEN::ARG_VALUE, { FUNC_CALL_TOKEN::ARG_COMMA, FUNC_CALL_TOKEN::ARG_CLOSE } },
	{ FUNC_CALL_TOKEN::ARG_COMMA, { FUNC_CALL_TOKEN::ARG_VALUE } },
	{ FUNC_CALL_TOKEN::ARG_VALUE, { FUNC_CALL_TOKEN::ARG_CLOSE } }
};

bool isFuncPtr( std::vector<FUNC_CALL_TOKEN>& callTokens ){
	return callTokens.size() == 1 && callTokens.back() == FUNC_CALL_TOKEN::FUNC_NAME;
}

void
passValidFuncCallToken( std::vector<FUNC_CALL_TOKEN>& callTokens ){
	if( callTokens.empty() )
		throw InvalidSyntaxError("Function call token is empty");

	FUNC_CALL_TOKEN currentStage = FUNC_CALL_TOKEN::FUNC_NAME;
	size_t startIndex = 0;

	while( startIndex < callTokens.size() ){
		if( callTokens[ startIndex ] == FUNC_CALL_TOKEN::ARG_CLOSE )
			return ;
		
		if( startIndex + 1 >= callTokens.size() )
			break;

		FUNC_CALL_TOKEN nextExpected = callTokens[ ++startIndex ];
		std::vector<FUNC_CALL_TOKEN>& nextExpectedTokens = FUNC_CALL_GRAPH[ currentStage ];
		
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
	std::vector<FUNC_CALL_TOKEN> callTokens;
	std::vector<std::vector<Token>> argsVector;
	std::string funcName;

	FunctionCallReturns() = default;

	FunctionCallReturns( 
		std::vector<FUNC_CALL_TOKEN> callTokens,
		std::vector<std::vector<Token>> argsVector,
		std::string funcName
	){
		this->callTokens = callTokens;
		this->argsVector = argsVector;
		this->funcName 	 = funcName;
	}
};

FunctionCallReturns stringToFunctionCallTokens( const std::vector<Token>& tokens, size_t& curPtr ){
	std::vector<FUNC_CALL_TOKEN> ctokens;
	std::vector<std::vector<Token>> argsTokens;
	std::string funcCallName;

	FUNC_CALL_TOKEN prev = FUNC_CALL_TOKEN::NOTHING;

	for( ; curPtr < tokens.size(); curPtr++ ){
		const std::string& curToken = tokens[ curPtr ].token;

		if( prev == FUNC_CALL_TOKEN::NOTHING ){
			ctokens.push_back( FUNC_CALL_TOKEN::FUNC_NAME );
			funcCallName = curToken;
		}
		else if( curToken == "(" ){
			ctokens.push_back( FUNC_CALL_TOKEN::ARG_OPEN );
			int openCnts = 1;

			std::vector<Token> curVector;
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
