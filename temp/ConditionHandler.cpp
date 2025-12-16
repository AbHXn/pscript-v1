#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;


enum class CHAIN_TOKENS{
	AND 	, 
	OR 		, 
	NOT 	, 
	EXPR 	,
	GTR_THAN, 
	GTR_EQ	, 
	LST_EQ	,
	LST_THAN, 
	DB_EQ
}

enum class CONDITIONAL_TOKENS{
	NAT 		,
	BODY_OPEN 	, 
	BODY_CLOSE 	,
	IF 			, 
	ELSE_IF 	, 
	ELSE 		,
	CHAIN 		,
	BODY_INTERNAL, 
}

class ExprChain{
	private:
		vector<EXPR_TOKENS> chain;
	public:


		static vector<EXPR_TOKENS> getTokensFromString( vector<string>& Tokens, size_t& startIndex ){
			// check if its a temp value
			// check if its a variable
			// create function that returns evaluate value (varDtype)
		}
}

class ConditionalChain{
	private:
		vector<CONDITIONAL_TOKENS> chain;
	public:
		static vector<CHAIN_TOKENS> getTokensFromString( vector<string>&Tokens, size_t& startIndex ){

		}
}

unordered_map<CONDITIONAL_TOKENS, vector<CONDITIONAL_TOKENS>> CONDITIONA_GRAPH = {
	{ IF, 				{ CHAIN } 			 	},
	{ CHAIN, 			{ BODY_OPEN } 		 	},
	{ BODY_CLOSE, 		{ ELSE_IF, ELSE } 		},
	{ ELSE_IF, 			{ BODY_OPEN } 		 	},
	{ BODY_OPEN, 		{ BODY_INTERNAL} 	 	},
	{ BODY_INTERNAL,	{ BODY_CLOSE } 		 	},
	{ ELSE,				{ BODY_OPEN } 		 	}
}

queue<string> ChainBody;

vector<CONDITIONAL_TOKENS> codeToTokens( vector<string>& tokens, size_t& curIndexPtr ){
	int startCurPtr = curIndexPtr;

	vector<CONDITIONAL_TOKENS> result;
	string preStringToken = '';
	CONDITIONAL_TOKENS prevToken = CONDITIONAL_TOKENS::NAT;

	for( ; startCurPtr < tokens.size(); ++startCurPtr ){
		string curToken = tokens[ startCurPtr ];

		if( curToken == "nokada" ){
			if ( preStringToken == "ithu" )
				result.push_back( CONDITIONAL_TOKENS::ELSE_IF );
			else result.push_back( CONDITIONAL_TOKENS::IF );
		}
		
		else if( curToken == "{" ){
			result.push_back( CONDITIONAL_TOKENS::BODY_OPEN );
			result.push_back( CONDITIONAL_TOKENS::BODY_INTERNAL );
		}
		
		else if( curToken == "}" ){
			result.push_back( CONDITIONAL_TOKENS::BODY_CLOSE );
		}
		
		else if( curToken == "nokiye" && preToken == "mathi" ){
			result.push_back( CONDITIONAL_TOKENS::ELSE );
		}

		else if( prevToken == CONDITIONAL_TOKENS::IF || 
				 prevToken == CONDITIONAL_TOKENS::ELSE_IF ){
			// get conditional chain
		}
		
		else if( prev_token == CONDITIONAL_TOKENS::BODY_OPEN )
			ChainBody.push(  )
		
		prevToken = ( result.size() > 0 ) ? result.back() : prevToken;
	}

	return result;
}

bool checkIfValid( vector<CONDITIONAL_TOKENS> condTokens ){

}