#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <unordered_map>

#include "Dtypes.hpp"

using namespace std;

struct HMAP{

};

struct TempVar{
	DTYPES varType;
	VarDtype TempValue;
};

vector<string> debug_expr = { "+", "-", "*", "/", "%", "(", ")" };

enum class EXPR 		{ ADD, SUB, MUL, DIV, MOD, OPEN, CLOSE };
enum class VALUE_TYPE 	{ TEMP_VAULE, VMAP_VALUE, EXPR_OPTR  };

struct Value{
	VALUE_TYPE ValueType;
	union {
		HMAP*	 MapValue;
		TempVar* TempValue;
		EXPR 	 Optr;
	} CurValue;
};

unordered_map<EXPR, short unsigned int> BODMAS = {
	{ EXPR::SUB, 	1 },
	{ EXPR::ADD, 	1 },
	{ EXPR::MUL, 	3 },
	{ EXPR::DIV, 	3 },
	{ EXPR::OPEN,	5 }
};

class ExprHandler{

	public:
		vector<Value> exprChain;

		vector<Value> convertToPostFix( size_t& startIndex ){
			vector<Value> postfixExpr, exprStack;
			
			for( ; startIndex < this->exprChain.size(); startIndex++ ){
				Value& exprChainValue = exprChain[ startIndex ];

				if( exprChainValue.ValueType == VALUE_TYPE::EXPR_OPTR ){

					if( exprChainValue.CurValue.Optr == EXPR::CLOSE )
						break;

					if( exprChainValue.CurValue.Optr == EXPR::OPEN){
						vector<Value> res = convertToPostFix( ++startIndex );
						postfixExpr.insert( postfixExpr.end(), res.begin(), res.end() );
					}
					else{
						EXPR curExpr = exprChainValue.CurValue.Optr;
						
						if( exprStack.empty() )
							exprStack.push_back( exprChainValue );
						else{
							Value stackExprValue = exprStack.back();
							
							if( BODMAS[ stackExprValue.CurValue.Optr ] >= BODMAS[ curExpr ] ){
								exprStack.pop_back( );
								exprStack.push_back( exprChainValue );
								postfixExpr.push_back( stackExprValue );
							}
							else exprStack.push_back( exprChainValue );
						}
					}
				}
				else postfixExpr.push_back( exprChainValue );
			}

			if( !exprStack.empty() )
				postfixExpr.insert( postfixExpr.end(), exprStack.rbegin(), exprStack.rend() );
			
			return postfixExpr;
		}

		void createChainFromStringToken( vector<string>& chain, size_t& startIndex ){
			for( ; startIndex < chain.size(); startIndex++ ){
				string curToken = chain[ startIndex ];
				unique_ptr<Value> newValue = make_unique<Value>();

				if( curToken == "+" )
				{
					newValue->ValueType 	= VALUE_TYPE::EXPR_OPTR;
					newValue->CurValue.Optr = EXPR::ADD;
					exprChain.push_back( *newValue );
				}
				else if( curToken == "-" )
				{
					newValue->ValueType 	= VALUE_TYPE::EXPR_OPTR;
					newValue->CurValue.Optr = EXPR::SUB;
					exprChain.push_back( *newValue );
				}
				else if( curToken == "*" )
				{
					newValue->ValueType 	= VALUE_TYPE::EXPR_OPTR;
					newValue->CurValue.Optr = EXPR::MUL;
					exprChain.push_back( *newValue );
				}
				else if( curToken == "/" )
				{
					newValue->ValueType 	= VALUE_TYPE::EXPR_OPTR;
					newValue->CurValue.Optr = EXPR::DIV;
					exprChain.push_back( *newValue );
				}
				else if( curToken == "(" )
				{
					newValue->ValueType 	= VALUE_TYPE::EXPR_OPTR;
					newValue->CurValue.Optr = EXPR::OPEN;
					exprChain.push_back( *newValue );
				}
				else if( curToken == ")" )
				{
					newValue->ValueType 	= VALUE_TYPE::EXPR_OPTR;
					newValue->CurValue.Optr = EXPR::CLOSE;
					exprChain.push_back( *newValue );
				}
				else if( curToken == "%" )
				{
					newValue->ValueType 	= VALUE_TYPE::EXPR_OPTR;
					newValue->CurValue.Optr = EXPR::MOD;
					exprChain.push_back( *newValue );
				}
				else{
					try{
						auto result = DtypeHelper::getTypeAndValue( curToken );
						newValue->ValueType = VALUE_TYPE::TEMP_VAULE;
						
						unique_ptr<TempVar> tempData = make_unique<TempVar>( );
						tempData->varType  = result.first;

						newValue->CurValue.TempValue = tempData.get( );
						exprChain.push_back( *newValue );
					}
					catch ( const InvalidDTypeError& err ){
						if( DtypeHelper::isValidVariableName( curToken ) ) {
							cout << "In Vmap\n";
							//********************* GET FROM VMAP
						}
					}
				}
			}
		}		
};

int main(){
	vector<string> test = { "1", "+", "3", "/", "(", "30", "-", "23", "+", "34", ")" };
	size_t currentIndex = 0;
	ExprHandler* n = new ExprHandler();
	n->createChainFromStringToken( test, currentIndex );
	size_t startIndex = 0;
	vector<Value> postFix = n->convertToPostFix( startIndex );
	for(Value data: postFix){
		
		if( data.ValueType == VALUE_TYPE::EXPR_OPTR ){
			cout << debug_expr[(int)data.CurValue.Optr] << endl;
		}else cout << (int) data.ValueType << endl;

	}
	return 0;
}