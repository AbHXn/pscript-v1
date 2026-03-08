#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <unordered_set>
#include <string>
#include <vector>
#include <unordered_map>

unordered_set<string> REG_INS_TOKEN = {
	"=", "+=", "-=", "/=", "*=", ":=", "%=", ",", ";"
};

bool isRegisteredInsTokens( const std::string& tok ){
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

std::unordered_map<INS_TOKEN, std::vector<INS_TOKEN>> INS_GRAPH = {
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

bool MainToken( const std::string& curToken, INS_TOKEN& Optr ){
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
	std::vector<INS_TOKEN> insToken;
	INS_TOKEN optr;
	std::vector<vector<Token>> rightVector;
	std::vector<Token> leftVector;

	InstructionTokens(
		std::vector<INS_TOKEN> insToken,
		INS_TOKEN optr,
		std::vector<std::vector<Token>> rightVector,
		std::vector<Token> leftVector
	){
		this->insToken 	  = insToken;
		this->optr 		  = optr;
		this->rightVector = rightVector;
		this->leftVector  = leftVector;
	}

};

InstructionTokens
stringToInsToken(const std::vector<Token>& tokens, size_t& startIndex ){
	INS_TOKEN Optr = INS_TOKEN::NOT_DEFINED;
	bool isLhs 	   = true;
	
	std::vector<std::vector<Token>> rightVector;
	std::vector<Token> 		   		leftValue;
	std::vector<INS_TOKEN> 	   		insTokens;

	while( startIndex < tokens.size() ){
		const Token& tok = tokens[ startIndex ];
		const std::string& curToken = tok.token;
			
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
				std::vector<Token> valueVector;
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

void 
passValidInstructionTokens( std::vector<INS_TOKEN>& tokens ){
	size_t startIndex = 0;
	INS_TOKEN currentStage = INS_TOKEN::LHS_VALUE;

	while( startIndex < tokens.size() ){
		INS_TOKEN newTok = tokens[ startIndex ];
		if( newTok == INS_TOKEN::INS_END )
			return ;

		if( startIndex + 1 >= tokens.size() )
			break;

		INS_TOKEN nextExpected = tokens[ ++startIndex ];
		std::vector<INS_TOKEN>& nextExpectedTokens = INS_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( INS_TOKEN nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw RecoverError(); // some instruction failure can recover
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError("Do dont encounter end ; token in para");
}

#endif