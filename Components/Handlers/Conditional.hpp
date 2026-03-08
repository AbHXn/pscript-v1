#ifndef CONDITIONAL_HPP
#define CONDITIONAL_HPP

#include <unordered_set>
#include <string>
#include <vector>

#include "../../Headers/MBExceptions.hpp"

std::unordered_set<std::string> REG_COND_TOKENS = { "nok", "{", "}", "umbi" };

enum class COND_TOKENS{ IF, ELSE, CONDITION, BODY_OPEN, BODY_CLOSE, CHAIN, END };

bool isRegisteredCondToken( const std::string& tokens ){
	return REG_COND_TOKENS.find( tokens ) != REG_COND_TOKENS.end();
}
// syntax verifier graph
std::unordered_map<COND_TOKENS, std::vector<COND_TOKENS>> CONDITIONAL_GRAPH = {
	{ COND_TOKENS::IF, 		    { COND_TOKENS::CONDITION } },
	{ COND_TOKENS::CONDITION,   { COND_TOKENS::BODY_OPEN } },
	{ COND_TOKENS::BODY_OPEN,   { COND_TOKENS::BODY_CLOSE } },
	{ COND_TOKENS::BODY_CLOSE,  { COND_TOKENS::END, COND_TOKENS::CHAIN } },
	{ COND_TOKENS::CHAIN, 		{ COND_TOKENS::IF, COND_TOKENS::ELSE } } ,
	{ COND_TOKENS::ELSE, 		{ COND_TOKENS::BODY_OPEN } }
};

void 
passCondTokenValidation( std::vector<COND_TOKENS>& tokens ){
	COND_TOKENS curStage = COND_TOKENS::IF;
	int startIndex 	  = 0;
	bool hitElseStage = false;

	while( startIndex < tokens.size() ){
		if( tokens[ startIndex ] == COND_TOKENS::END ){
			// checking if end tokens hits bofore finishing the entire tokens
			if( startIndex < tokens.size() - 1 )
				throw InvalidSyntaxError( "Invalid Syntax encounter in nok statements" );
			return ;
		}
		// checking else hits more than one 
		if( curStage == COND_TOKENS::ELSE ){
			if( hitElseStage )
				throw InvalidSyntaxError( "Cannot have two umbi statement" );
			hitElseStage = true;
		}
		// check if no more tokens to process 
		if( startIndex + 1 >= tokens.size() )
			break;

		COND_TOKENS nextExpected = tokens[ ++startIndex ];
		std::vector<COND_TOKENS>& nextExpectedTokens = CONDITIONAL_GRAPH[ curStage ];

		bool continueNext = false;
		for( COND_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError( "Invalid Token encounter in nok" );
		// move to next token
		curStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter ; in nok statement" );
}

std::pair<std::vector<COND_TOKENS>, std::vector<std::pair<std::vector<Token>, size_t>>>
stringToCondTokens( const std::vector<Token>& tokens, size_t& start, size_t& endPtr ){
	std::vector<std::pair<std::vector<Token>, size_t>> conditions;
	std::vector<COND_TOKENS> condTokens;
	size_t bodyOpenCount = 0;

	for( ; start < tokens.size(); start++ ){
		const std::string& curToken = tokens[ start ].token;
		// if not inside a conditional body
		if( !bodyOpenCount ){
			if( curToken == ";" ){
				condTokens.push_back( COND_TOKENS::END );
				endPtr = start;
				return { condTokens, conditions };
			}	
			if( curToken == "nok" ){
				condTokens.push_back( COND_TOKENS::IF );
			}
			else if( curToken == "{" ){
				condTokens.push_back( COND_TOKENS::BODY_OPEN );
				bodyOpenCount++;
			}
			else if( curToken == ":" ){
				condTokens.push_back( COND_TOKENS::CHAIN );
			}
			else if( curToken == "umbi" ){
				condTokens.push_back( COND_TOKENS::ELSE );
				// else is always true
				conditions.push_back( { {Token( TOKEN_TYPE::BOOLEAN, "sheri", 0, 0 )}, start + 1 } );
			}
			else{
				std::vector<Token> condVector;
				while( start < tokens.size() ){
					if( isRegisteredCondToken( tokens[ start ].token ) ){
						start--; break;
					}
					condVector.push_back( tokens[ start++ ] );
				}
				conditions.push_back( { condVector, start + 1 } );
				condTokens.push_back( COND_TOKENS::CONDITION );
			}
		} 
		else if( curToken == "{" ){
			bodyOpenCount++;
			continue;
		}
		else if( curToken == "}" ){
			if( --bodyOpenCount == 0 ) 
				condTokens.push_back( COND_TOKENS::BODY_CLOSE );
		}
	}
	throw InvalidSyntaxError("Forgot ; in nok statement?");
}

#endif 
