#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <memory>

#include "Dtypes.hpp"

using namespace std;

struct HMAP{

};
struct TempVar{
	DTYPES varType;
	VarDtype TempValue;
};

enum class EXPR{ ADD, SUB, MUL, DIV, MOD, OPEN, CLOSE };
enum class VALUE_TYPE { TEMP_VAULE, VMAP_VALUE, EXPR_OPTR  };

struct Value{
	VALUE_TYPE ValueType;
	union {
		HMAP*	 MapValue;
		TempVar* TempValue;
		EXPR 	 Optr;
	} CurValue;
};

class ExprHandler{
	public:
		vector<Value> exprChain;

		void convertToPostFix( void ){

		}

		VarDtype Evaluate( void ){
			
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
	vector<string> test = { "1", "+", "3", "/", "(", "30", "-", "23", "+", "admin", ")" };
	size_t currentIndex = 0;
	ExprHandler* n = new ExprHandler();
	n->createChainFromStringToken( test, currentIndex );
	for(Value data: n->exprChain)
		cout << (int) data.ValueType << endl;
	return 0;
}