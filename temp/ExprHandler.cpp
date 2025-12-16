#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <unordered_map>
#include <type_traits>

#include "Dtypes.hpp"

using namespace std;

struct HMAP{

};

struct TempVar{
	DTYPES 	 varType;
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

Value VarDtypeOperation( const Value& left, const Value& right, EXPR optr ){
	VarDtype leftValue, rightValue;
	DTYPES leftType, rightType;

	if( left.ValueType == VALUE_TYPE::TEMP_VAULE ){
		leftValue = left.CurValue.TempValue->TempValue;
		leftType  = left.CurValue.TempValue->varType;
	}

	else if( left.ValueType == VALUE_TYPE::VMAP_VALUE ){
		// get Map value
	}else {
		// raise error
	}

	if( right.ValueType == VALUE_TYPE::TEMP_VAULE ){
		rightValue = right.CurValue.TempValue->TempValue;
		rightType  = right.CurValue.TempValue->varType;
	}

	else if( right.ValueType == VALUE_TYPE::VMAP_VALUE ){
		// get Map value
	}else{
		// Raise error
	}

	return visit([leftValue, leftType, rightValue, rightType]( VarDtype& lft, VarDtype& rht) -> Value {
		unique_ptr<Value> newValue = make_unique<Value>();		
		newValue->ValueType = VALUE_TYPE::TEMP_VAULE;
		unique_ptr<TempVar> tempValue = make_unique<TempVar>();

		if ( is_arithmetic_v<leftValue> && is_arithmetic_v<rightValue> ){
			switch( optr ){
				case EXPR::ADD:
					tempValue->TempValue = leftValue + rightValue;
					break;
				case EXPR::SUB:
					tempValue->TempValue = leftValue + rightValue;
					break;
				case EXPR::DIV:
					if( rightValue == 0 ){
						// raise div by zero error
					}
					tempValue->TempValue = leftValue / rightValue;
					break;
				case EXPR::MUL:
					tempValue->TempValue = leftValue * rightValue;
					break;
				case EXPR::MOD:
					tempValue->TempValue = leftValue % rightValue;
					break;
				default:
					// raise error;
					break;
			}
			if( leftType == DTYPES::DOUBLE || rightType == DTYPES::DOUBLE )
				tempValue->varType = DTYPES::DOUBLE;

			else if( leftType == DTYPES::INT || rightType == DTYPES::INT )
				tempValue->varType = DTYPES::INT;

			else tempValue->varType = DTYPES::BOOL;
		}
		else if( is_same_v<leftValue, string> && is_same_v<rightValue, string> ){
			tempValue->TempValue = leftValue + rightValue;
			tempValue->varType = DTYPES::STRING;
		}
		else{
			// raise invalid dtypes
		}
		newValue->CurValue.TempValue = tempValue.get();
		return *newValue;
	}, leftValue, rightValue);
}

class ExprHandler{
	public:
		vector<Value> exprChain;

		VarDtype evaluateTheChain( void ){
			vector<Value> evaluationStack;

		}

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
				newValue->ValueType = VALUE_TYPE::EXPR_OPTR;

				if( curToken == "+" )
					newValue->CurValue.Optr = EXPR::ADD;
				
				else if( curToken == "-" )
					newValue->CurValue.Optr = EXPR::SUB;
				
				else if( curToken == "*" )
					newValue->CurValue.Optr = EXPR::MUL;
				
				else if( curToken == "/" )
					newValue->CurValue.Optr = EXPR::DIV;
				
				else if( curToken == "(" )
					newValue->CurValue.Optr = EXPR::OPEN;
				
				else if( curToken == ")" )
					newValue->CurValue.Optr = EXPR::CLOSE;
				
				else if( curToken == "%" )
					newValue->CurValue.Optr = EXPR::MOD;
				
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
							//********************* GET FROM VMAP
						}
					}
				}
				exprChain.push_back( *newValue );
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