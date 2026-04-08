#include "Headers/AST.hpp"

std::unordered_set<std::string> EXPR_CONSTANTS = {
	"+", "-", "/", "%", "*",
	">", "<", "==", "!=", "<=", ">=",
	"um", "yo"
};

std::unordered_map<AST_TOKENS, short unsigned int> PRIORITY = {
	{ AST_TOKENS::OR, 1 },
	
	{ AST_TOKENS::AND, 2 },
	
	{ AST_TOKENS::D_EQUAL_TO, 	3 },
	{ AST_TOKENS::NOT_EQUAL_TO, 3 },

	{ AST_TOKENS::LS_THAN, 		4 },
	{ AST_TOKENS::GT_THAN, 		4 },
	{ AST_TOKENS::GT_THAN_EQ, 	4 },
	{ AST_TOKENS::LS_THAN_EQ, 	4 },

	{ AST_TOKENS::ADD, 5 },
	{ AST_TOKENS::SUB, 5 },
	
	{ AST_TOKENS::MUL, 6 },
	{ AST_TOKENS::DIV, 6 },
	{ AST_TOKENS::MOD, 6 },
};


AST_TOKENS getExprToken( const std::string& curTokens ) {
	// mathematical 
	if 	   ( curTokens == "+" ) return AST_TOKENS::ADD;
	else if( curTokens == "-" ) return AST_TOKENS::SUB;
	else if( curTokens == "*" ) return AST_TOKENS::MUL;
	else if( curTokens == "/" ) return AST_TOKENS::DIV;
	else if( curTokens == "%" ) return AST_TOKENS::MOD;
	// relational
	else if( curTokens == "==" ) return AST_TOKENS::D_EQUAL_TO;
	else if( curTokens == ">" )  return AST_TOKENS::GT_THAN;
	else if( curTokens == "<" )  return AST_TOKENS::LS_THAN;
	else if( curTokens == ">=" ) return AST_TOKENS::GT_THAN_EQ;
	else if( curTokens == "<=" ) return AST_TOKENS::LS_THAN_EQ;
	else if( curTokens == "!=" ) return AST_TOKENS::NOT_EQUAL_TO;
	// and .. or.. 
	else if( curTokens == "um" )  return AST_TOKENS::AND;	
	else if( curTokens == "yo" )  return AST_TOKENS::OR;

	return AST_TOKENS::NONE;
}

bool isRegisteredASTExprTokens( const std::string& tok ){
	return EXPR_CONSTANTS.find( tok ) != EXPR_CONSTANTS.end();
}

std::vector<std::variant<CODE_TOKENS, AST_TOKENS>>
stringToASTTokens( const std::vector<std::string>& tokens ){
	std::vector<std::variant<CODE_TOKENS, AST_TOKENS>> resultTokens;

	for( int startIndex = 0; startIndex < tokens.size(); startIndex++ ){
		const std::string& curToken = tokens[ startIndex ];

		if( isRegisteredASTExprTokens(curToken) ){
			AST_TOKENS getToken = getExprToken( curToken );
			resultTokens.push_back( getToken );
		}
		else if( curToken == "(" ) 
			resultTokens.push_back( CODE_TOKENS::OPEN );
		else if( curToken == ")" )
			resultTokens.push_back( CODE_TOKENS::CLOSE );
		else resultTokens.push_back( CODE_TOKENS::VALUE );
	}
	return resultTokens;
} 
