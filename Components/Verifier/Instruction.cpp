#include "Headers/Instruction.hpp"

std::unordered_set<std::string_view> REG_INS_TOKEN = {
	"=", "+=", "-=", "/=", "*=", ":=", "%=", ",", ";"
};

bool isRegisteredInsTokens( const std::string& tok ){
	return REG_INS_TOKEN.find( tok ) != REG_INS_TOKEN.end();
}

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

InstructionTokens stringToInsToken(const std::vector<Token>& tokens, size_t& startIndex ){
	INS_TOKEN Optr = INS_TOKEN::NOT_DEFINED;
	bool isLhs 	   = true;
	
	std::vector<std::vector<Token>> rightVector;
	std::vector<Token> 		   		leftValue;
	std::vector<INS_TOKEN> 	   		insTokens;

	int curlyOpenCount = 0;

	while( startIndex < tokens.size() ){
		const Token& tok = tokens[ startIndex ];
		const std::string& curToken = tok.token;

		if( curToken == "," && tok.type == TOKEN_TYPE::SPEC_CHAR)
			insTokens.push_back( INS_TOKEN::COMMA );

		else if( MainToken( curToken, Optr ) ){
			isLhs = false;
			insTokens.push_back( Optr );
		}
		else if( curToken == ";" && tok.type == TOKEN_TYPE::SPEC_CHAR ){
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
				int openBrackCnts = 0, openCurlyCounts = 0;

				while( startIndex < tokens.size() ){
					const Token& curTok = tokens[ startIndex ];
					const std::string& curStrTok = curTok.token;

					if( curStrTok == "(" && curTok.type == TOKEN_TYPE::SPEC_CHAR)
						openBrackCnts++;
					else if( curStrTok == ")" && curTok.type == TOKEN_TYPE::SPEC_CHAR)
						openBrackCnts--;

					else if( curStrTok == "{" && curTok.type == TOKEN_TYPE::SPEC_CHAR )
						openCurlyCounts++;
					else if( curStrTok == "}" && curTok.type == TOKEN_TYPE::SPEC_CHAR )
						openCurlyCounts--;

					if( !openBrackCnts && !openCurlyCounts && isRegisteredInsTokens( curStrTok ) && (
						curTok.type == TOKEN_TYPE::OPERATOR || curTok.type == TOKEN_TYPE::SPEC_CHAR ) ){
						startIndex--; break;
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

void  passValidInstructionTokens( std::vector<INS_TOKEN>& tokens ){
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
