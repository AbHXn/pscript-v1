#ifndef IOHANDLER_HPP
#define IOHANDLER_HPP

#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>

#include "../Headers/MBExceptions.hpp"

using namespace std;

unordered_set<string> REGISTERED_IO_TOKENS = { "koode", "para", ";" };
enum class IO_TOKENS { PRINT, CONCAT, PRINT_VALUE, END };

bool isRegisteredIoTokens( const string& token ){
	return REGISTERED_IO_TOKENS.find( token ) != REGISTERED_IO_TOKENS.end();
}

unordered_map<IO_TOKENS, vector<IO_TOKENS>> IO_GRAPH = {
	{ IO_TOKENS::PRINT, 		{ IO_TOKENS::PRINT_VALUE } },
	{ IO_TOKENS::PRINT_VALUE,   { IO_TOKENS::END, IO_TOKENS::CONCAT, IO_TOKENS::PRINT } },
	{ IO_TOKENS::CONCAT, 		{ IO_TOKENS::PRINT_VALUE } },
};

bool
isValidIoTokens( vector<IO_TOKENS>& IOTokens ){
	if( IOTokens.empty() )
		return false;

	IO_TOKENS currentStage = IO_TOKENS::PRINT;
	size_t startIndex = 0;

	while( startIndex < IOTokens.size() ){
		if( IOTokens[ startIndex ] == IO_TOKENS::END ){
			return true;
		}
		if( startIndex + 1 < IOTokens.size() )
			startIndex++;
		else break;

		IO_TOKENS nextExpected = IOTokens[ startIndex ];
		vector<IO_TOKENS>& nextExpectedTokens = IO_GRAPH[ currentStage ];

		bool continueNext = false;
		for( IO_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError( "Invalid Token encounter in para" );
		
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter end ; token in para" );
}

pair<vector<IO_TOKENS>, vector<vector<string>>>
stringToIoTokens( const vector<string>& tokens, size_t& startIndex ){
	vector<IO_TOKENS> IOTokens;
	vector<vector<string>> printValues;

	for(; startIndex < tokens.size(); startIndex++){
		string curToken = tokens[ startIndex ];
		if( curToken == "para" ){
			IOTokens.push_back( IO_TOKENS::PRINT );
		}
		else if( curToken == "koode" ){
			IOTokens.push_back( IO_TOKENS::CONCAT );
		}
		else if( curToken == ";" ){
			IOTokens.push_back( IO_TOKENS::END );
			return { IOTokens, printValues };
		}
		else{
			vector<string> valueVector;

			while( startIndex < tokens.size() ){
				if( isRegisteredIoTokens( tokens[ startIndex ] ) ){
					startIndex--;
					break;
				}
				valueVector.push_back( tokens[ startIndex++ ] );
			}
			printValues.push_back( valueVector );
			IOTokens.push_back( IO_TOKENS::PRINT_VALUE );
		}
	}
	throw InvalidSyntaxError("Faild to find the end of token ;");
}

#endif