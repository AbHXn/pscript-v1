#include <iostream>
#include <vector>
#include <string>
#include <queue>

using namespace std;

enum class EXPR_TOKENS{
	ADD 	, 
	SUB 	, 
	DIV 	, 
	MULT 	,
	MOD 	,
	VAL     ,
	B_OPEN  ,
	B_CLOSE ,
};

queue<string> exprValue;

vector<EXPR_TOKENS> getTokensFromString( vector<string>& tokens, size_t currentIndex ){
	vector<EXPR_TOKENS> exprTokens;

	for( ; currentIndex < tokens.size(); currentIndex++ ){
		string currentToken = tokens[ currentIndex ];

		if( currentToken == "+" ){
			exprTokens.push_back( EXPR_TOKENS::ADD );
		}
		else if( currentToken == "-" ){
			exprTokens.push_back( EXPR_TOKENS::SUB );
		}
		else if( currentToken == "%" ){
			exprTokens.push_back( EXPR_TOKENS::MOD );
		}
		else if( currentToken == "*" ){
			exprTokens.push_back( EXPR_TOKENS::MULT );
		}
		else if( currentToken == "/" ){
			exprTokens.push_back( EXPR_TOKENS::DIV );
		}
		else if( currentToken == "(" ){
			exprTokens.push_back( EXPR_TOKENS::B_OPEN );
		}
		else if( currentToken == ")" ){
			exprTokens.push_back( EXPR_TOKENS::B_CLOSE );
		}else{
			// check if temp value or a key in hashmap ( can be result from function, or variable );
			exprValue.push( currentToken );
		}
	}
	return exprTokens;
}

class ValueInfo{
	public:
		enum Type { TEMP, HASH } ValueType;
		union {
			// Hashmap
			// ValueDtype
		}Value;
}

int main(){
	vector<string> test = { "1", "+", "3", "/", "(", "30", "-", "23", ")" };
	size_t currentIndex = 0;
	vector<EXPR_TOKENS> res = getTokensFromString( test, currentIndex );
	for(EXPR_TOKENS data: res)
		cout << (int) data << endl;
	return 0;
}