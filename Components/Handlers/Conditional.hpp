#ifndef CONDITIONAL_HPP
#define CONDITIONAL_HPP

#include <unordered_set>
#include <string>
#include <vector>

#include "../../Headers/MBExceptions.hpp"

using namespace std;

unordered_set<string> REG_COND_TOKENS = {
	"nok", "{", "}", "umbi",   
};

enum class COND_TOKENS{ IF, ELSE, CONDITION, BODY_OPEN, BODY_CLOSE, CHAIN, END };

bool isRegisteredCondToken( const string& tokens ){
	return REG_COND_TOKENS.find( tokens ) != REG_COND_TOKENS.end();
}

unordered_map<COND_TOKENS, vector<COND_TOKENS>> CONDITIONAL_GRAPH = {
	{ COND_TOKENS::IF, 		    { COND_TOKENS::CONDITION } },
	{ COND_TOKENS::CONDITION,   { COND_TOKENS::BODY_OPEN } },
	{ COND_TOKENS::BODY_OPEN,   { COND_TOKENS::BODY_CLOSE } },
	{ COND_TOKENS::BODY_CLOSE,  { COND_TOKENS::END, COND_TOKENS::CHAIN } },
	{ COND_TOKENS::CHAIN, 		{ COND_TOKENS::IF, COND_TOKENS::ELSE } } ,
	{ COND_TOKENS::ELSE, 		{ COND_TOKENS::BODY_OPEN } }
};

bool
isValidCondToken( vector<COND_TOKENS>& tokens ){
	COND_TOKENS curStage = COND_TOKENS::IF;
	int startIndex 	  = 0;
	bool hitElseStage = false;
	while( startIndex < tokens.size() ){
		if( tokens[ startIndex ] == COND_TOKENS::END ){
			if( startIndex < tokens.size() - 1 )
				throw InvalidSyntaxError( "Invalid Syntax encounter in nok statements" );
			return true;
		}
		if( curStage == COND_TOKENS::ELSE ){
			if( hitElseStage )
				throw InvalidSyntaxError( "Cannot have two umbi statement" );
			hitElseStage = true;
		}
		if( startIndex + 1 < tokens.size() )
			startIndex++;
		else break;
		COND_TOKENS nextExpected = tokens[ startIndex ];
		vector<COND_TOKENS>& nextExpectedTokens = CONDITIONAL_GRAPH[ curStage ];

		bool continueNext = false;
		for( COND_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext ){
			throw InvalidSyntaxError( "Invalid Token encounter in nok" );
			break;
		}
		curStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter end ; token in nok" );
}

pair<vector<COND_TOKENS>, vector<pair<vector<Token>, size_t>>>
stringToCondTokens( const vector<Token>& tokens, size_t& start, size_t& endPtr ){
	vector<pair<vector<Token>, size_t>> conditions;
	vector<COND_TOKENS> condTokens;
	size_t bodyOpenCount = 0;

	for( ; start < tokens.size(); start++ ){
		const string& curToken = tokens[ start ].token;

		if( !bodyOpenCount ){
			if( curToken == "nok" )
				condTokens.push_back( COND_TOKENS::IF );
			else if( curToken == "{" ){
				condTokens.push_back( COND_TOKENS::BODY_OPEN );
				bodyOpenCount++;
			}
			else if( curToken == ":" )
				condTokens.push_back( COND_TOKENS::CHAIN );
			else if( curToken == ";"){
				condTokens.push_back( COND_TOKENS::END );
				endPtr = start;
				return { condTokens, conditions };
			}
			else if( curToken == "umbi" ){
				condTokens.push_back( COND_TOKENS::ELSE );
				conditions.push_back( 
					{ {Token( TOKEN_TYPE::BOOLEAN, "sheri", 0, 0 )}, start + 1 } 
					);
			}
			else{
				vector<Token> condVector;
				while( start < tokens.size() ){
					if( isRegisteredCondToken( tokens[ start ].token ) ){
						start--;
						break;
					}
					condVector.push_back( tokens[ start++ ] );
				}
				conditions.push_back( { condVector, start + 1 } );
				condTokens.push_back( COND_TOKENS::CONDITION );
			}
		} 
		else if( curToken == "{" )
			bodyOpenCount++;
		else if( curToken == "}" ){
			bodyOpenCount--;
			if( !bodyOpenCount )
				condTokens.push_back( COND_TOKENS::BODY_CLOSE );
		}
	}
	throw InvalidSyntaxError("Faild to find the end of token ;");
}

#endif
