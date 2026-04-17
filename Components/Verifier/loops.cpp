#include "Headers/loops.hpp"

std::unordered_set<std::string_view> REG_LOOP_TOKENS      = { "ittuthiri", "{", "}" };
std::unordered_set<std::string_view> REG_LOOP_BODY_TOKENS = { "pinnava", "theku" };

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

LoopTokens stringToLoopTokens( 
		std::unordered_map<std::string, size_t>& bMap,
		const std::vector<Token>& tokens, 
		size_t& startIndex, 
		std::string& filename
){
	std::vector<LOOP_TOKENS> lpTokens;
	std::vector<Token> conditions;

	while( startIndex < tokens.size() ){
		const Token& curToken = tokens[ startIndex ];
		
		if( curToken.token == "ittuthiri" && curToken.type == TOKEN_TYPE::RESERVED )
			lpTokens.push_back( LOOP_TOKENS::LOOP_START );
		
		else if( curToken.token == "{" && curToken.type == TOKEN_TYPE::SPEC_CHAR ){
			if( conditions.empty() ){
				lpTokens.push_back( LOOP_TOKENS::CONDITION );
				conditions.push_back( Token( TOKEN_TYPE::BOOLEAN, "sheri", 0, 0 ) );
			}
			lpTokens.push_back( LOOP_TOKENS::BODY_OPEN );

			size_t startPtr = startIndex + 1;
			size_t endPtr = 0;

			std::string bKey = filename + std::to_string( curToken.row + 1 ) + "-" + std::to_string( curToken.col + 1 );

			auto bkeyD = bMap.find(bKey);
			if( bkeyD != bMap.end() )
				endPtr = bkeyD->second + 1;
			else throw InvalidSyntaxError("} failed to get closing tag");
		
			lpTokens.push_back( LOOP_TOKENS::BODY );
			lpTokens.push_back( LOOP_TOKENS::BODY_CLOSE );

			return LoopTokens( lpTokens, conditions, startPtr, endPtr );
		}
		else{
			while( startIndex < tokens.size() ){
				const Token& curCondToken = tokens[ startIndex ];
				if( isRegisteredLoopTokens( curCondToken.token ) && (
						curCondToken.type == TOKEN_TYPE::RESERVED || curCondToken.type == TOKEN_TYPE::SPEC_CHAR
					)){
					startIndex--;
					break;
				}
				conditions.push_back( curCondToken );
				startIndex++;
			}
			lpTokens.push_back( LOOP_TOKENS::CONDITION );
		}
		startIndex++;
	}

	throw InvalidSyntaxError( "Syntax error occured in ittuthiri statement" );
}

void passValidLoopTokens( std::vector<LOOP_TOKENS>& tokens ){
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

