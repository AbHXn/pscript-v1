#ifndef INSTRUCTIONHANDLER_HPP
#define INSTRUCTIONHANDLER_HPP

#include <unordered_set>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

unordered_set<string> REG_INS_TOKEN = {
	"=", "+=", "-=", "/=", "*=", "%=", ",", ";"
};

bool isRegisteredInsTokens( const string& tok ){
	return REG_INS_TOKEN.find( tok ) != REG_INS_TOKEN.end();
}

enum class INS_TOKEN{
	NOT_DEFINED,
	REPLACE,
	COMMA,
	ADD_REPLACE,
	SUB_REPLACE,
	MUL_REPLACE,
	DIV_REPLACE,
	MOD_REPLACE,
	LHS_VALUE,
	RHS_VALUE,
	INS_END,
};

unordered_map<INS_TOKEN, vector<INS_TOKEN>> INS_GRAPH = {
	{ INS_TOKEN::LHS_VALUE, { INS_TOKEN::COMMA, 
							  INS_TOKEN::INS_END, 
							  INS_TOKEN::ADD_REPLACE, 
							  INS_TOKEN::SUB_REPLACE,
							  INS_TOKEN::MUL_REPLACE, 
							  INS_TOKEN::DIV_REPLACE, 
							  INS_TOKEN::MOD_REPLACE,
							  INS_TOKEN::REPLACE } },
	
	{ INS_TOKEN::ADD_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::SUB_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::MUL_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::DIV_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::MOD_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::REPLACE, 	  { INS_TOKEN::RHS_VALUE } },
	
	{ INS_TOKEN::RHS_VALUE, { INS_TOKEN::COMMA, INS_TOKEN::INS_END } },
	{ INS_TOKEN::COMMA, { INS_TOKEN::LHS_VALUE, INS_TOKEN::RHS_VALUE } },
};

bool MainToken( string curToken, INS_TOKEN& Optr, string nextToken ){
	if( nextToken != "=" )
		return false;

	if( curToken == "+" ){
		Optr = INS_TOKEN::ADD_REPLACE;
		return true;
	}

	else if( curToken == "-" ){
		Optr = INS_TOKEN::SUB_REPLACE;
		return true;
	}

	else if( curToken == "*" ){
		Optr = INS_TOKEN::MUL_REPLACE;
		return true;
	}

	else if( curToken == "/" ){
		Optr = INS_TOKEN::DIV_REPLACE;
		return true;
	}

	else if( curToken == "%" ){
		Optr = INS_TOKEN::MOD_REPLACE;
		return true;
	}
	return false;
}

pair<
	pair< vector<INS_TOKEN>,  INS_TOKEN >,
	pair< vector<string>, vector<vector<string>>>
>
stringToInsToken(const vector<string>& tokens, size_t& startIndex ){
	INS_TOKEN Optr = INS_TOKEN::NOT_DEFINED;
	bool isLhs = true;
	
	vector<vector<string>> rightVector;
	vector<string> 		   leftValue;
	vector<INS_TOKEN> 	   insTokens;

	while( startIndex < tokens.size() ){
		string curToken = tokens[ startIndex ];
		
		if( curToken == "," )
			insTokens.push_back( INS_TOKEN::COMMA );

		else if( startIndex + 1 < tokens.size() && MainToken( curToken, Optr, tokens[startIndex + 1] ) ){
			isLhs = false;
			insTokens.push_back( Optr );
			startIndex++;
		}
		else if( curToken == "=" ){
			isLhs = false;
			insTokens.push_back( INS_TOKEN::REPLACE );
			Optr = INS_TOKEN::REPLACE;
		}
		
		else if( curToken == ";" ){
			insTokens.push_back( INS_TOKEN::INS_END );
			return { { insTokens, Optr }, { leftValue, rightVector } };
		}

		else{
			if( isLhs ){
				leftValue.push_back( curToken );
				insTokens.push_back( INS_TOKEN::LHS_VALUE );
			}
			else{
				vector<string> valueVector;

				while( startIndex < tokens.size() ){
					if( isRegisteredInsTokens( tokens[ startIndex ] ) ){
						startIndex--;
						break;
					}
					valueVector.push_back( tokens[ startIndex++ ] );
				}
				rightVector.push_back( valueVector );
				insTokens.push_back( INS_TOKEN::RHS_VALUE );
			}
		}
		startIndex++;

	}
	throw InvalidSyntaxError( "Do dont encounter end ; token in para" );
}

bool 
isValidInstructionSet( vector<INS_TOKEN>& tokens ){
	size_t startIndex = 0;
	INS_TOKEN currentStage = INS_TOKEN::LHS_VALUE;

	while( startIndex < tokens.size() ){
		INS_TOKEN newTok = tokens[ startIndex ];
		if( newTok == INS_TOKEN::INS_END )
			return true;

		if( startIndex + 1 < tokens.size() )
			startIndex++;
		else break;

		INS_TOKEN nextExpected = tokens[ startIndex ];
		vector<INS_TOKEN>& nextExpectedTokens = INS_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( INS_TOKEN nextToks: nextExpectedTokens ){
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