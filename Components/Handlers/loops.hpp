#ifndef LOOP_HPP
#define LOOP_HPP

#include <vector>
#include <string>
#include <unordered_map>

std::unordered_set<std::string> REG_LOOP_TOKENS      = { "ittuthiri", "{", "}" };
std::unordered_set<std::string> REG_LOOP_BODY_TOKENS = { "pinnava", "theku" };

enum class LOOP_TOKENS{
	LOOP_START 	,
	CONDITION 	,
	BREAK 		,
	CONTINUE 	,
	BODY_OPEN 	,
	BODY 		,
	BODY_CLOSE
};

std::unordered_map<LOOP_TOKENS, std::vector<LOOP_TOKENS>> LOOP_GRAPH = {
	{ LOOP_TOKENS::LOOP_START,  { LOOP_TOKENS::CONDITION, LOOP_TOKENS::BODY_OPEN } },
	{ LOOP_TOKENS::CONDITION,   { LOOP_TOKENS::BODY_OPEN } },
	{ LOOP_TOKENS::BODY_OPEN, 	{ LOOP_TOKENS::BODY } },
	{ LOOP_TOKENS::BODY, 		{ LOOP_TOKENS::BODY_CLOSE } }
};

bool isRegisteredLoopBodyTokens( const std::string& tok ){
	return REG_LOOP_BODY_TOKENS.find( tok ) != REG_LOOP_BODY_TOKENS.end();
}

bool isRegisteredLoopTokens( const std::string& tok ){
	return REG_LOOP_TOKENS.find( tok ) != REG_LOOP_TOKENS.end() || isRegisteredLoopBodyTokens( tok );
}

size_t getEndPointer( const std::vector<Token>& tokens, size_t startPtr ){
	int bodyOpenCount = 0;
	while( startPtr < tokens.size() ){
		if( tokens[startPtr].token == "{" )
			bodyOpenCount++;

		else if( tokens[ startPtr ].token == "}" )
			bodyOpenCount--;

		if( bodyOpenCount == 0 )
			return startPtr + 1;
		
		startPtr++;
	}
	throw InvalidSyntaxError( "forget to close '}' ? " );
}

struct LoopTokens{
	std::vector<LOOP_TOKENS> lpTokens;
	std::vector<Token> conditions;
	size_t startPtr;
	size_t endPtr;

	LoopTokens(
		std::vector<LOOP_TOKENS> lpTokens,
		std::vector<Token> conditions,
		size_t startPtr,
		size_t endPtr
	){
		this->lpTokens 	 = lpTokens;
		this->conditions = conditions;
		this->startPtr   = startPtr;
		this->endPtr 	 = endPtr;
	}
};

LoopTokens
stringToLoopTokens( const std::vector<Token>& tokens, size_t& startIndex ){
	std::vector<LOOP_TOKENS> lpTokens;
	std::vector<Token> conditions;

	while( startIndex < tokens.size() ){
		const std::string& curToken = tokens[ startIndex ].token;
		
		if( curToken == "ittuthiri" )
			lpTokens.push_back( LOOP_TOKENS::LOOP_START );
		
		else if( curToken == "{" ){
			if( conditions.empty() ){
				lpTokens.push_back( LOOP_TOKENS::CONDITION );
				conditions.push_back( Token( TOKEN_TYPE::RESERVED, "sheri", 0, 0 ) );
			}
			lpTokens.push_back( LOOP_TOKENS::BODY_OPEN );

			size_t startPtr = startIndex + 1;
			size_t endPtr 	= getEndPointer( tokens, startIndex );

			lpTokens.push_back( LOOP_TOKENS::BODY );
			lpTokens.push_back( LOOP_TOKENS::BODY_CLOSE );
			
			return LoopTokens( lpTokens, conditions, startPtr, endPtr );
		}
		else{
			while( startIndex < tokens.size() ){
				const Token& curToken = tokens[ startIndex ];
				if( isRegisteredLoopTokens( curToken.token ) ){
					startIndex--;
					break;
				}
				conditions.push_back( curToken );
				startIndex++;
			}
			lpTokens.push_back( LOOP_TOKENS::CONDITION );
		}
		startIndex++;
	}
	throw InvalidSyntaxError( "Syntax error occured in ittuthiri statement" );
}

void 
passValidLoopTokens( std::vector<LOOP_TOKENS>& tokens ){
	size_t startIndex = 0;
	LOOP_TOKENS currentStage = LOOP_TOKENS::LOOP_START;

	while( startIndex < tokens.size() ){
		LOOP_TOKENS newTok = tokens[ startIndex ];
		if( newTok == LOOP_TOKENS::BODY_CLOSE )
			return ;

		if( startIndex + 1 >= tokens.size() )
			break;

		LOOP_TOKENS nextExpected = tokens[ ++startIndex ];
		std::vector<LOOP_TOKENS>& nextExpectedTokens = LOOP_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( LOOP_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError("Invalid Instruction in ittuthiri syntax");
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter end ; token in ittuthiri" );
}

#endif