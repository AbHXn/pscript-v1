#ifndef LOOPHANDLER_HPP
#define LOOPHANDLER_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_map>

using namespace std;

unordered_set<string> REG_LOOP_TOKENS = { "ittuthiri", "{", "}" };
unordered_set<string> REG_LOOP_BODY_TOKENS = { "pinnava", "theku" };

enum class LOOP_TOKENS{
	LOOP_START 	,
	CONDITION 	,
	BREAK 		,
	CONTINUE 	,
	BODY_OPEN 	,
	BODY 		,
	BODY_CLOSE
};

unordered_map<LOOP_TOKENS, vector<LOOP_TOKENS>> LOOP_GRAPH = {
	{ LOOP_TOKENS::LOOP_START,  { LOOP_TOKENS::CONDITION, LOOP_TOKENS::BODY_OPEN } },
	{ LOOP_TOKENS::CONDITION,   { LOOP_TOKENS::BODY_OPEN } },
	{ LOOP_TOKENS::BODY_OPEN, 	{ LOOP_TOKENS::BODY } },
	{ LOOP_TOKENS::BODY, 		{ LOOP_TOKENS::BODY_CLOSE } }
};

bool isRegisteredLoopBodyTokens( const string& tok ){
	return REG_LOOP_BODY_TOKENS.find( tok ) != REG_LOOP_BODY_TOKENS.end();
}

bool isRegisteredLoopTokens( const string& tok ){
	return REG_LOOP_TOKENS.find( tok ) != REG_LOOP_TOKENS.end() || isRegisteredLoopBodyTokens( tok );
}

size_t getEndPointer( const vector<string>& tokens, size_t startPtr ){
	int bodyOpenCount = 0;

	while( startPtr < tokens.size() ){
		if( tokens[startPtr] == "{" )
			bodyOpenCount++;

		else if( tokens[ startPtr ] == "}" )
			bodyOpenCount--;
		
		if( bodyOpenCount == 0 )
			return startPtr + 1;

		startPtr++;
	}
	throw InvalidSyntaxError( "forget to close '}' ? " );
}

pair<pair<vector<LOOP_TOKENS>, vector<string>>, pair<size_t, size_t>>
stringToLoopTokens( const vector<string>& tokens, size_t& startIndex ){
	vector<LOOP_TOKENS> lpTokens;
	vector<string> conditions;

	while( startIndex < tokens.size() ){
		string curToken = tokens[ startIndex ];

		if( curToken == "ittuthiri" ){
			lpTokens.push_back( LOOP_TOKENS::LOOP_START );
		}
		else if( curToken == "{" ){
			if( conditions.empty() ){
				lpTokens.push_back( LOOP_TOKENS::CONDITION );
				conditions.push_back("true");
			}
			
			lpTokens.push_back( LOOP_TOKENS::BODY_OPEN );
			size_t startPtr = startIndex + 1;
			size_t endPtr = getEndPointer( tokens, startIndex );

			lpTokens.push_back( LOOP_TOKENS::BODY );
			lpTokens.push_back( LOOP_TOKENS::BODY_CLOSE );

			return { { lpTokens, conditions }, { startPtr, endPtr} };
		}
		else{
			while( startIndex < tokens.size() ){
				string curToken = tokens[ startIndex ];
				if( isRegisteredLoopTokens( curToken ) ){
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
	throw InvalidSyntaxError( "Syntax error occured in loop" );
}


bool 
isValidLoopTokens( vector<LOOP_TOKENS>& tokens ){
	size_t startIndex = 0;
	LOOP_TOKENS currentStage = LOOP_TOKENS::LOOP_START;

	while( startIndex < tokens.size() ){
		LOOP_TOKENS newTok = tokens[ startIndex ];
		if( newTok == LOOP_TOKENS::BODY_CLOSE )
			return true;

		if( startIndex + 1 < tokens.size() )
			startIndex++;
		else break;

		LOOP_TOKENS nextExpected = tokens[ startIndex ];
		vector<LOOP_TOKENS>& nextExpectedTokens = LOOP_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( LOOP_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError("Invalid Instruction");
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter end ; token in para" );
}

#endif