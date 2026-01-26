#ifndef HELPER_HPP
#define HELPER_HPP

#include "DefinedTypes.hpp"
#include "VMAP.hpp"

using namespace std;

class HelperClass: public VAR_VMAP {
	public:
		string functionName;

		VarDtype getValueFromToken( const Token& tok ){
			if( tok.type == TOKEN_TYPE::STRING )
				return VarDtype{ tok.token };
			
			else if( tok.type == TOKEN_TYPE::NUMBER )
				return VarDtype{  DtypeHelper::toLong( tok.token ) };
			
			else if( tok.type == TOKEN_TYPE::FLOATING )
				return VarDtype{ DtypeHelper::toDouble( tok.token ) };

			else if( tok.type == TOKEN_TYPE::BOOLEAN && 
					( tok.token == "sheri" || tok.token == "thettu" ) )
				return VarDtype{ DtypeHelper::toBoolean( tok.token ) };
			
			else throw InvalidDTypeError("not a value");
		}


		DEEP_VALUE_DATA 
		getTheResult( vector<Token>& vtr ){
			auto [resolvedQeueu, strVector] = vectorResolver( vtr );

			auto astTokenAndData = stringToASTTokens( strVector );
			size_t startAST = 0;
			auto newAstNode = BUILD_AST<DEEP_VALUE_DATA, 
								AST_NODE_DATA>( 
									astTokenAndData, resolvedQeueu, startAST 
								);

			if( newAstNode.has_value() ){
				auto AST_NODE = move( newAstNode.value() );
				if( !AST_NODE->left && !AST_NODE->right && 
						holds_alternative<DEEP_VALUE_DATA> ( AST_NODE->AST_DATA)){
					return get<DEEP_VALUE_DATA>( AST_NODE->AST_DATA );
				}
				VarDtype finalValue = ValueHelper::evaluate_AST_NODE( AST_NODE );
				return finalValue;
			}
			else throw InvalidDTypeError("Failed to resolve the vector\n");
			
		}


		pair<queue<DEEP_VALUE_DATA>, vector<string>>
		vectorResolver( const vector<Token>& tokens ){

			queue<DEEP_VALUE_DATA> resolvedVector;       
			vector<string> simpleVector;

			for( size_t x = 0; x < tokens.size(); x++ ){
				const Token& tok = tokens[ x ];
				const string& curToken = tokens[ x ].token;

				if( !isValueType(tok.type) && isRegisteredASTExprTokens( curToken ) || curToken == ")" ||  curToken == "(" ){
					simpleVector.push_back( curToken );
					continue;
				}
				
				if( isValueType( tok.type ) ){
					VarDtype value = this->getValueFromToken( tok );
					simpleVector.push_back("NUM");
					resolvedVector.push( value );
				}

				else if( tok.type == TOKEN_TYPE::IDENTIFIER ){
					auto [VMAPData, rPT] = this->getFromVmap( curToken );

					if( VMAPData != nullptr ){
						// if it is variable then get its value
						if( VMAPData->mapType == MAPTYPE::VARIABLE ){
							auto& varHolder = get<unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( VMAPData->var );
							if( !varHolder->isTypeArray ){
								ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );
								for(auto test: arrToken.)

								auto& data = get<VarDtype>( varHolder->value );
								resolvedVector.push( data );
								simpleVector.push_back("VAR");
							}
							else {
								auto& arrayData = get<unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( varHolder->value );
								ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );
								x--;
								handleArrayCases( arrayData.get(), arrToken, resolvedVector, simpleVector );
							}
						}
						// if it is a function then call it and get the value
						else if( VMAPData->mapType == MAPTYPE::FUNCTION || VMAPData->mapType == MAPTYPE::FUNC_PTR ){
							try{
								auto varHolder = (VMAPData->mapType == MAPTYPE::FUNCTION) ? \
												get<unique_ptr<FUNCTION_MAP_DATA>>( VMAPData->var ).get() : get<FUNCTION_MAP_DATA*>( VMAPData->var );

								FunctionCallReturns pt = stringToFunctionCallTokens( tokens, x );

								if( isFuncPtr( pt.callTokens ) ){
									simpleVector.push_back("FUNC_PTR");
									resolvedVector.push( varHolder );
								}
								else{
									auto data = this->handleFunctionCall( VMAPData, fullTokens, x, rPT, pt);
									if( data.has_value() ){	
										if( holds_alternative<VarDtype>( data.value() ) ){
											VarDtype returnedData = get<VarDtype>( data.value() );
											simpleVector.push_back("VAR");
											resolvedVector.push( returnedData );
										}
										else if( holds_alternative<unique_ptr<MapItem>>( data.value() ) ){
											auto returnedData = move( get<unique_ptr<MapItem>>( data.value() ) );
											DEEP_VALUE_DATA final = ValueHelper::getFinalValueFromMap( returnedData.get() );
											resolvedVector.push( final );
											this->pushCache( move( returnedData ) );
											simpleVector.push_back("CACHE");
										}
										else throw runtime_error("unknown typed pushed to queue");
									}
								}
								x--; // stringfuncalltokens it hits then unknown token get that token back
							}
							catch ( const InvalidSyntaxError& err ){
								cout << err.what() << endl;
							}
						}
						else if( VMAPData->mapType == MAPTYPE::ARRAY_PTR ){
							auto arryListPtr = get<ArrayList<ARRAY_SUPPORT_TYPES>*>( VMAPData->var );
							ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );
							x--;
							handleArrayCases( arryListPtr, arrToken, resolvedVector, simpleVector );
						}
					}
					else throw InvalidSyntaxError(
							"Unknown identifier at line: " + to_string(tok.row) + " " + curToken 
							);
				}
				else throw InvalidSyntaxError( "Unknown Token at: " +  to_string(tok.row) + " " + curToken );
			}
			return { move(resolvedVector), simpleVector };
		}

};

#endif