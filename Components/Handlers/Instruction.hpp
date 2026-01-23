#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <unordered_set>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

unordered_set<string> REG_INS_TOKEN = {
	"=", "+=", "-=", "/=", "*=", ":=", "%=", ",", ";"
};

bool isRegisteredInsTokens( const string& tok ){
	return REG_INS_TOKEN.find( tok ) != REG_INS_TOKEN.end();
}

enum class INS_TOKEN{
	NOT_DEFINED,
	REPLACE,
	COMMA,
	ADD_REPLACE,
	TYPE_CAST  ,
	SUB_REPLACE,
	MUL_REPLACE,
	DIV_REPLACE,
	MOD_REPLACE,
	LHS_VALUE,
	RHS_VALUE,
	INS_END,
};

unordered_map<INS_TOKEN, vector<INS_TOKEN>> INS_GRAPH = {
	{ INS_TOKEN::LHS_VALUE, { INS_TOKEN::LHS_VALUE,
							  INS_TOKEN::COMMA,
							  INS_TOKEN::ADD_REPLACE, 
							  INS_TOKEN::SUB_REPLACE,
							  INS_TOKEN::MUL_REPLACE, 
							  INS_TOKEN::DIV_REPLACE, 
							  INS_TOKEN::MOD_REPLACE,
							  INS_TOKEN::TYPE_CAST	,
							  INS_TOKEN::REPLACE } },
	
	{ INS_TOKEN::ADD_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::TYPE_CAST,   { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::SUB_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::MUL_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::DIV_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::MOD_REPLACE, { INS_TOKEN::RHS_VALUE } },
	{ INS_TOKEN::REPLACE, 	  { INS_TOKEN::RHS_VALUE } },
	
	{ INS_TOKEN::RHS_VALUE, { INS_TOKEN::COMMA, INS_TOKEN::INS_END } },
	{ INS_TOKEN::COMMA, { INS_TOKEN::LHS_VALUE, INS_TOKEN::RHS_VALUE } },
};

bool MainToken( const string& curToken, INS_TOKEN& Optr ){
	if( curToken == "=" ){
		Optr = INS_TOKEN::REPLACE;
		return true;
	}
	if( curToken == "+=" ){
		Optr = INS_TOKEN::ADD_REPLACE;
		return true;
	}
	else if( curToken == "-=" ){
		Optr = INS_TOKEN::SUB_REPLACE;
		return true;
	}
	else if( curToken == "*=" ){
		Optr = INS_TOKEN::MUL_REPLACE;
		return true;
	}
	else if( curToken == "/=" ){
		Optr = INS_TOKEN::DIV_REPLACE;
		return true;
	}
	else if( curToken == "%=" ){
		Optr = INS_TOKEN::MOD_REPLACE;
		return true;
	}
	else if( curToken == ":=" ){
		Optr = INS_TOKEN::TYPE_CAST;
		return true;
	}
	return false;
}

struct InstructionTokens{
	vector<INS_TOKEN> insToken;
	INS_TOKEN optr;
	vector<vector<Token>> rightVector;
	vector<Token> leftVector;

	InstructionTokens(
		vector<INS_TOKEN> insToken,
		INS_TOKEN optr,
		vector<vector<Token>> rightVector,
		vector<Token> leftVector
	){
		this->insToken 	  = insToken;
		this->optr 		  = optr;
		this->rightVector = rightVector;
		this->leftVector  = leftVector;
	}

};

InstructionTokens
stringToInsToken(const vector<Token>& tokens, size_t& startIndex ){
	INS_TOKEN Optr = INS_TOKEN::NOT_DEFINED;
	bool isLhs = true;
	
	vector<vector<Token>> rightVector;
	vector<Token> 		   leftValue;
	vector<INS_TOKEN> 	   insTokens;

	while( startIndex < tokens.size() ){
		const Token& tok = tokens[ startIndex ];
		const string& curToken = tok.token;
			
		if( curToken == "," )
			insTokens.push_back( INS_TOKEN::COMMA );

		else if( MainToken( curToken, Optr ) ){
			isLhs = false;
			insTokens.push_back( Optr );
		}
		else if( curToken == ";" ){
			insTokens.push_back( INS_TOKEN::INS_END );
			return InstructionTokens( insTokens, Optr, rightVector, leftValue );
		}
		else{
			if( isLhs ){
				leftValue.push_back( tok );
				insTokens.push_back( INS_TOKEN::LHS_VALUE );
			}
			else{
				vector<Token> valueVector;
				int openBrackCnts = 0;
				while( startIndex < tokens.size() ){
					if( tokens[ startIndex ].token == "(" )
						openBrackCnts++;
					else if( tokens[ startIndex ].token == ")" )
						openBrackCnts--;

					if( !openBrackCnts && isRegisteredInsTokens( tokens[ startIndex ].token ) ){
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