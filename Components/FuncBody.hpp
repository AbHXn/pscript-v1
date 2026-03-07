#ifndef FUNCBODY_HPP
#define FUNCBODY_HPP

#include "DefinedTypes.hpp"
#include "VMAP.hpp"

using namespace std;

class LoopHandler;

vector<Token> fullTokens;
size_t 		   pointer = 0;

enum class CALLER{ LOOP, FUNCTION, CONDITIONAL };

template <typename T> optional<variant<VarDtype, unique_ptr<MapItem>>>
ProgramExecutor( const vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, T* prntClass, size_t endPtr = 0  );
using BUCKET_TYPE = variant<unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>,unique_ptr<FUNCTION_MAP_DATA>>;

vector<BUCKET_TYPE> _CACHE_VARS;


class FunctionHandler: public VAR_VMAP {
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
			
			throw InvalidDTypeError("not a value");
		}

		DEEP_VALUE_DATA 
		evaluateVector( vector<Token>& vtr ){
			auto [resolvedQeueu, strVector] = vectorResolver( vtr );
			auto astTokenAndData 			= stringToASTTokens( strVector );
			size_t startAST 				= 0;

			// built AST Tree
			auto newAstNode = BUILD_AST<DEEP_VALUE_DATA, AST_NODE_DATA>( astTokenAndData, resolvedQeueu, startAST );

			if( newAstNode.has_value() ){
				auto AST_NODE = move( newAstNode.value() );

				// if Tree has only one node and that holds DEEP_VALUE_DATA
				if( !AST_NODE->left && !AST_NODE->right && holds_alternative<DEEP_VALUE_DATA> ( AST_NODE->AST_DATA ))
					return get<DEEP_VALUE_DATA>( AST_NODE->AST_DATA );

				// else we will evaluate tree ( AFTER EVALUATION IT WILL BE EITHER, string, long, double, boolean )
				return ValueHelper::evaluate_AST_NODE( AST_NODE );
			}
			throw InvalidDTypeError("Failed to resolve the vector\n");
		}

		/* this function is to convert to pure vector
		resolve variables, function call, array calls etc */
		pair<queue<DEEP_VALUE_DATA>, vector<string>>
		vectorResolver( const vector<Token>& tokens ){
			queue<DEEP_VALUE_DATA> resolvedVector;       
			vector<string> 		   simpleVector;

			for( size_t x = 0; x < tokens.size(); x++ ){
				const Token& tok = tokens[ x ];
				const string& curToken = tokens[ x ].token;

				if( !isValueType( tok.type ) && isRegisteredASTExprTokens( curToken ) || curToken == ")" ||  curToken == "(" ){
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

					// if identifier is not there in VMAP
					if (VMAPData == nullptr)
						throw InvalidSyntaxError( "Unknown identifier at line: " + to_string(tok.row) + " " + curToken );

					// Resolve if it is variable
					if( VMAPData->mapType == MAPTYPE::VARIABLE ){
						auto varHolder = get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( VMAPData->var );
						
						ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );

						if( !isValidArrayAccess( arrToken.tokens ) )
							throw runtime_error("Invalid Array Access Syntax");

						if( !varHolder->isTypeArray ){
							DEEP_VALUE_DATA tdata = ValueHelper::getDataFromVariableHolder( varHolder );
							auto datan = handleRawVariables( arrToken,  tdata );
							
							resolvedVector.push( datan ); 
							simpleVector.push_back( "VAR" );
						}
						else {
							auto& arrayData = get<unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( varHolder->value );
							handleArrayCases( arrayData.get(), arrToken, resolvedVector, simpleVector );
						}
						x--;
					}
					// Resolve if it is function call
					else if( VMAPData->mapType == MAPTYPE::FUNCTION || VMAPData->mapType == MAPTYPE::FUNC_PTR ){
						try{
							auto varHolder = get<FUNCTION_MAP_DATA*>( VMAPData->var );

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
										this->pushCache( move( returnedData ) );
										
										simpleVector.push_back("CACHE");
										resolvedVector.push( final );
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
						handleArrayCases( arryListPtr, arrToken, resolvedVector, simpleVector );
						x--;
					}
				}
				else throw InvalidSyntaxError( "Unknown Token at: " +  to_string(tok.row) + " " + curToken );
			}
			return { move(resolvedVector), simpleVector };
		}

		optional<VarDtype>
		handleArrayProperties( ArrayList<ARRAY_SUPPORT_TYPES>* array, ArrayAccessTokens& arrToken){
			if( arrToken.arrProperty.propertyType == "valupam" ){
				return (long int) array->totalElementsAllocated;
			} 
			else if( arrToken.arrProperty.propertyType == "jaadi" ){
				return "KOOTAM";
			}
			return nullopt;
		}

		void handleArrayCases( ArrayList<ARRAY_SUPPORT_TYPES>* arrayData, ArrayAccessTokens& arrToken, queue<DEEP_VALUE_DATA>& resolvedVector, vector<string>& simpleVector ){
			if( arrToken.indexVector.size() ){
				vector<long int> resolvedIndexVector;

				// Resolve the index vector to the final value
				for( auto& vec: arrToken.indexVector ){
					DEEP_VALUE_DATA val = evaluateVector( vec );
					if( !holds_alternative<VarDtype>( val ) )
						throw InvalidSyntaxError("Array Index Expects Positive Integer");
				
					VarDtype vDtypeIndex = get<VarDtype>( val );
					if( !holds_alternative<long int> ( vDtypeIndex ) && !holds_alternative<double> ( vDtypeIndex ))
						throw InvalidSyntaxError("Array Index Expects Positive Integer");

					long int index = holds_alternative<long int> ( vDtypeIndex ) ? get<long int> ( vDtypeIndex ) : get<double> (vDtypeIndex);
					resolvedIndexVector.push_back( index );
				}

				variant<ArrayList<ARRAY_SUPPORT_TYPES>*, ARRAY_SUPPORT_TYPES*> returnIndex = arrayData->getElementAtIndex( resolvedIndexVector, 0 );
				// resolve if it touch property functions (:)
				if( holds_alternative<ARRAY_SUPPORT_TYPES*> ( returnIndex ) ){
					auto spData = get<ARRAY_SUPPORT_TYPES*>( returnIndex );
					std::visit( [&]( auto&& data ) {
					    if (arrToken.isTouchedArrayProperty) {
					        DEEP_VALUE_DATA dv = data;
					        resolvedVector.push( handleVarDefinedProperties(dv, arrToken) );
					    } 
					    else resolvedVector.push(data);
					    simpleVector.push_back("VAR");
					}, *spData);
				}
				else{
					auto arrList = get<ArrayList<ARRAY_SUPPORT_TYPES>*>( returnIndex );
					if( arrToken.isTouchedArrayProperty ){
						auto data = handleArrayProperties( arrList, arrToken );
						if( data.has_value() ) 
							resolvedVector.push( data.value() );
					}
					else resolvedVector.push( arrList );
					simpleVector.push_back("VAR");
				}
			}
			else {
				if( arrToken.isTouchedArrayProperty ){
					auto data = handleArrayProperties( arrayData, arrToken );
					if( data.has_value() )
						resolvedVector.push( data.value() );
				}
				else resolvedVector.push( arrayData );
				simpleVector.push_back("VAR");	
			}	
		}

		// return the size of 
		VarDtype 
		handleVarDefinedProperties( DEEP_VALUE_DATA& Vdata, ArrayAccessTokens& tok ){
			if( tok.arrProperty.propertyType == "jaadi" ){
				if( holds_alternative<VarDtype> (Vdata) ){
					auto data = get<VarDtype>( Vdata );
					if( holds_alternative<string> ( data ) )
						return "STR";
					else if( holds_alternative<double> (data) )
						return "THULA";
					else if( holds_alternative<long> (data) )
						return "INT";
					else if( holds_alternative<bool> (data) )
						return "BOOL";
					else return "ARILA";
				}
				else if( holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>( Vdata ) )
					return "ARRAY_PTR";
				else if( holds_alternative<FUNCTION_MAP_DATA*>( Vdata ) )
					return "FUNC_PTR";
				else return "ARILA";
			}
			else if( tok.arrProperty.propertyType == "kanam" ){
				if( holds_alternative<VarDtype>(Vdata) ){
					auto data = get<VarDtype>(Vdata);
					if( holds_alternative<string> ( data ) )
						return (long) get<string>(data).size();
					else if( holds_alternative<double> (data) )
						return (long) sizeof(double);
					else if( holds_alternative<long> (data) )
						return (long) sizeof(long);
					else if( holds_alternative<bool> (data) )
						return (long) sizeof(bool);
				}
			}
			throw InvalidSyntaxError("Invalid property");
		}

		DEEP_VALUE_DATA 
		handleRawVariables( ArrayAccessTokens& arrToken, DEEP_VALUE_DATA& varHolder ){
			DEEP_VALUE_DATA HandlingDtype = varHolder;
			if( arrToken.indexVector.size() ){
				if( !holds_alternative<VarDtype> ( varHolder ) 
					|| !holds_alternative<string> ( get<VarDtype>( varHolder ) ) )
					throw runtime_error("indexing invalid dtype");

				if( arrToken.indexVector.size() > 1 )
					throw runtime_error("indexing error");

				DEEP_VALUE_DATA val = evaluateVector( arrToken.indexVector.back() );
				if( !holds_alternative<VarDtype>( val ) )
					throw InvalidSyntaxError("Array Index Expects numbesdfr");
				
				VarDtype vDtypeIndex = get<VarDtype>( val );
				if( !holds_alternative<long int> ( vDtypeIndex ) &&
								 !holds_alternative<double> ( vDtypeIndex ))
					throw InvalidSyntaxError("Array Index Expects 99number");

				long int index;
				if( holds_alternative<long int>(vDtypeIndex) )
					index = get<long int>(vDtypeIndex);
				else index = (long int) get<double>( vDtypeIndex );

				string& stringVarHolder = get<string>( get<VarDtype>( varHolder ) );

				if( index >= 0 && index < stringVarHolder.size())
					HandlingDtype =  DEEP_VALUE_DATA { string(1, stringVarHolder[ index ]) };
			}

			if( arrToken.isTouchedArrayProperty )
				return handleVarDefinedProperties( HandlingDtype, arrToken );		
			
			return HandlingDtype;
		}

		optional<variant<VarDtype, unique_ptr<MapItem>>>
		handleFunctionCall( MapItem* func, const vector<Token>& tokens, size_t& currentPtr, VAR_VMAP* rPT, optional<FunctionCallReturns> data = nullopt ){
			FunctionCallReturns Data = ( data.has_value() ) ? data.value() : stringToFunctionCallTokens( tokens, currentPtr );
			
			if( isValidFuncCall( Data.callTokens ) ){
				// if it is zero arg function then no need to resolve arg vectors
				if( Data.argsVector.size() == 0 ){
					auto funcFromMap = get<FUNCTION_MAP_DATA*>( func->var );

					unique_ptr<FunctionHandler> newFuncRunner = make_unique<FunctionHandler>();

					size_t funcBodyStartPtr 	= funcFromMap->bodyStartPtr + 1;
					size_t funcEndStartPtr  	= funcFromMap->bodyEndPtr;
					newFuncRunner->runnerBody 	= Data.funcName;
					newFuncRunner->VMAP_COPY 	= funcFromMap->varMapCopy.first;
					newFuncRunner->parent 		= funcFromMap->varMapCopy.second;

					return ProgramExecutor( 
						tokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr 
					);
				}

				unique_ptr<FunctionHandler> newFuncRunner = make_unique<FunctionHandler>();
				newFuncRunner->runnerBody = Data.funcName;
							
				auto funcFromMap 		 = get<FUNCTION_MAP_DATA*>( func->var );
				newFuncRunner->VMAP_COPY = funcFromMap->varMapCopy.first;
				newFuncRunner->parent 	 = funcFromMap->varMapCopy.second;

				queue<DEEP_VALUE_DATA> resolvedArgs;
				int total_comma = Data.argsVector.size() - 1;

				vector<Token> rhsTokens;
				rhsTokens.push_back( Token( TOKEN_TYPE::OPERATOR, "=", 0, 0 ) );

				for( auto argSingleVec: Data.argsVector ){
					DEEP_VALUE_DATA dpData = evaluateVector( argSingleVec );
					resolvedArgs.push( dpData );

					rhsTokens.push_back( Token( TOKEN_TYPE::IDENTIFIER, "NUM", 0, 0) );
					if( total_comma-- )
						rhsTokens.push_back( Token( TOKEN_TYPE::SPEC_CHAR, ",", 0, 0) );
				}
				rhsTokens.push_back(Token( TOKEN_TYPE::SPEC_CHAR, ";", 0, 0));

				size_t start = 0;
				VariableTokens funcVars = stringToVariableTokens( rhsTokens, start );
				
				if( !isValidValueSyntax( funcVars.valueTokens ) )
					throw InvalidSyntaxError("Invalid args initalization in thenga");

				for( auto& ArgsInfo: funcFromMap->argsInfo ){
					if( resolvedArgs.empty() )
						throw InvalidSyntaxError("No value to initialized the args in thenga");
					
					DEEP_VALUE_DATA topValue = resolvedArgs.front();
					resolvedArgs.pop();

					if( holds_alternative<VarDtype> ( topValue ) ){
						if( ArgsInfo->isArray )
							throw InvalidSyntaxError("Argument expects kootam\n");

						// create variable
						unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
						newVariable->key 	= ArgsInfo->name;							
						newVariable->value 	= get<VarDtype>( topValue );

						// add to vmap
						auto newMapVar 		= make_unique<MapItem>( );
						newMapVar->mapType  = MAPTYPE::VARIABLE;
						newMapVar->var 		= newVariable.get();

						_CACHE_VARS.push_back( move( newVariable ) );
						newFuncRunner->addToMap( ArgsInfo->name, move( newMapVar ) );

					}
					else if(holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*> ( topValue )){
						// no need to create variable add to map
						if( !ArgsInfo->isArray )
							throw InvalidSyntaxError("Argument is not kootam type");

						string& key    		= ArgsInfo->name;
						auto newMapVar		= make_unique<MapItem>( );
						newMapVar->mapType 	= MAPTYPE::ARRAY_PTR;

						newMapVar->var = get<ArrayList<ARRAY_SUPPORT_TYPES>*>(topValue) ;
						newFuncRunner->addToMap( key, move( newMapVar ) );
					}
					else if( holds_alternative<FUNCTION_MAP_DATA*>( topValue ) ){
						string& key    		= ArgsInfo->name;
						auto newMapVar 		= make_unique<MapItem>( );
						newMapVar->mapType 	= MAPTYPE::FUNC_PTR;
						newMapVar->var 		= get<FUNCTION_MAP_DATA*>(topValue);
						
						newFuncRunner->addToMap( key, move( newMapVar ) );
					}
					else throw runtime_error("Type not defined");
				}
				size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
				size_t funcEndStartPtr  = funcFromMap->bodyEndPtr;
				
				return ProgramExecutor<FunctionHandler>( 
					fullTokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr 
				);
			}
			throw InvalidSyntaxError("Invalid function call");
		}

		void VarHandlerRunner( const vector<Token>& test, size_t& start ){
			VariableTokens tokens  = stringToVariableTokens( test, start );

			queue<DEEP_VALUE_DATA> resolvedValueVector;
			size_t curIndex = 0;

			for( auto valVec: tokens.valueTokens ){
				if( valVec == VALUE_TOKENS::ARRAY_VALUE || valVec == VALUE_TOKENS::NORMAL_VALUE ){
					auto testVec = tokens.valueVector[ curIndex++ ];

					DEEP_VALUE_DATA evaluatedRes = evaluateVector( testVec ); 
					resolvedValueVector.push( evaluatedRes );
				}
			}
			vector<VAR_INFO> varInfos;

			if( isValidVariableSyntax( tokens.varTokens, varInfos, tokens.VarQueue ) ){
				for( auto& varVerification: varInfos ){
					auto [data, rPT] = this->getFromVmap( varVerification.varName );	
					// check if variable is already existsed
					if( data ) throw VariableAlreayExists( varVerification.varName );
				}

				// if syntax is not correct
				if( !isValidValueSyntax( tokens.valueTokens ) )
					throw InvalidSyntaxError("Invalid Value Initialization Syntax");

				for( size_t x = 0, i = 0; x < tokens.valueTokens.size(); x++ ){
					if( i >= varInfos.size() && resolvedValueVector.empty() )
						break;

					auto curValueToken = tokens.valueTokens[x];

					if( curValueToken != VALUE_TOKENS::NORMAL_VALUE && curValueToken != \
						VALUE_TOKENS::ARRAY_VALUE && curValueToken != VALUE_TOKENS::ARRAY_OPEN )
						continue;

					if( i < varInfos.size() && resolvedValueVector.empty() )
						throw InvalidSyntaxError("more variables to initialize");

					if( i >= varInfos.size() && !resolvedValueVector.empty() )
						throw InvalidSyntaxError("No variable to initialize the value");

					VAR_INFO curVarInfo = varInfos[ i++ ];

					if( curValueToken == VALUE_TOKENS::NORMAL_VALUE ){
						auto curValue = resolvedValueVector.front();
						resolvedValueVector.pop();

						if( curVarInfo.isTypeArray && !holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>( curValue ) )
							throw InvalidSyntaxError("Assigning value to array type is invalid");

						if( holds_alternative<VarDtype> ( curValue ) ){
							unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
							newVariable->key 		 = curVarInfo.varName;
							newVariable->isTypeArray = false;
							newVariable->value 		 = get<VarDtype>( curValue );

							auto newMapVar 		= make_unique<MapItem>( );
							newMapVar->mapType 	= MAPTYPE::VARIABLE;
							newMapVar->var 	  	= newVariable.get() ;
							
							_CACHE_VARS.push_back( move( newVariable ) );
							this->addToMap( curVarInfo.varName, move(newMapVar) );
						}
						else if(holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*> ( curValue )){
							string& key    		= curVarInfo.varName;
							auto newMapVar 		= make_unique<MapItem>( );
							newMapVar->mapType 	= MAPTYPE::ARRAY_PTR;
							newMapVar->var 		= get<ArrayList<ARRAY_SUPPORT_TYPES>*>(curValue) ;
							
							this->addToMap( key, move( newMapVar ) );
						}
						else if( holds_alternative<FUNCTION_MAP_DATA*>( curValue ) ){
							string& key    		= curVarInfo.varName;
							auto newMapVar 		= make_unique<MapItem>( );
							newMapVar->mapType 	= MAPTYPE::FUNC_PTR;
							newMapVar->var 		= get<FUNCTION_MAP_DATA*>(curValue);
							
							this->addToMap( key, move( newMapVar ) );
						}
						else throw runtime_error("Type not defined");
					}
					else if( curValueToken == VALUE_TOKENS::ARRAY_OPEN ){
						x++;

						auto newArray = ArrayList<ARRAY_SUPPORT_TYPES>::createArray<DEEP_VALUE_DATA>(tokens.valueTokens,  x, resolvedValueVector );

						unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
						newVariable->key 		 = curVarInfo.varName;
						newVariable->isTypeArray = true;
						newVariable->value 		 = move( newArray );

						auto newMapVar 		= make_unique<MapItem>( );
						newMapVar->mapType 	= MAPTYPE::VARIABLE;
						newMapVar->var 		= newVariable.get();

						_CACHE_VARS.push_back( move( newVariable ) );
						this->addToMap( curVarInfo.varName, move(newMapVar) );
					}
				}
			}
			else throw InvalidDTypeError( "Invalid Syntax occured in variable declaration" );
		}

		void updateString( string& strToUpdate, long int index, DEEP_VALUE_DATA updata ){
			if( !holds_alternative<VarDtype>( updata ) )
				throw runtime_error("Invalid string updation dtype");

			auto& dataL1 = get<VarDtype>( updata );
			if( !holds_alternative<string> ( dataL1 ) )
				throw runtime_error( "invalid string updation dtype" );

			string rightValue = get<string> ( dataL1 );

			( index >= 0 && index < strToUpdate.size() ) ? strToUpdate.replace(index, 1, rightValue) : throw runtime_error("Index limit failed");
		}

		vector<long int> getResolvedIndexVectors( vector<vector<Token>>& indexVector ){
			vector<long int> resolvedIndexVector;

			for( auto& vec: indexVector ){
				DEEP_VALUE_DATA val = evaluateVector( vec );

				if( !holds_alternative<VarDtype>( val ) )
					throw InvalidSyntaxError("Array Index Expects Type Integer");
				
				VarDtype vDtypeIndex = get<VarDtype>( val );
				
				if( !holds_alternative<long int> ( vDtypeIndex ) && !holds_alternative<double> ( vDtypeIndex ))
					throw InvalidSyntaxError("Array Index Expects Integer");

				long int index = ( holds_alternative<long int>( vDtypeIndex ) ) ? get<long int> ( vDtypeIndex ) : (long int) get<double>( vDtypeIndex );
				resolvedIndexVector.push_back( index );
			}
			return resolvedIndexVector;
		}

		void arrayUpdation( const vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, ArrayList<ARRAY_SUPPORT_TYPES>* arr ){
			ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, curPtr );
			curPtr--;

			if( arrToken.isTouchedArrayProperty )
				throw InvalidSyntaxError("Using array property in updatation is not allowed");

			if( arrToken.indexVector.size() ){
				vector<long int> resolvedIndexVector = getResolvedIndexVectors( arrToken.indexVector );

				long int updationIndex = resolvedIndexVector.back();
				resolvedIndexVector.pop_back();

				if( updationIndex < 0 )
					throw InvalidSyntaxError("Array Index Expects Unsigned Integer");

				variant< ArrayList<ARRAY_SUPPORT_TYPES>*, ARRAY_SUPPORT_TYPES*> returnIndex = arr->getElementAtIndex( resolvedIndexVector, 0 );

				if( holds_alternative<ARRAY_SUPPORT_TYPES*> ( returnIndex ) ){
					auto* arrData = get<ARRAY_SUPPORT_TYPES*>( returnIndex );

					if( !holds_alternative<VarDtype>( *arrData ) || !holds_alternative<string> ( get<VarDtype>( *arrData ) ) )
						throw InvalidSyntaxError("Failed to index an non array type");
					
					auto& ttt = get<VarDtype>( *arrData );
					string& strToUpdate = get<string>( ttt );
					updateString( strToUpdate, updationIndex, upvalue );
					return ;
				}
				ArrayList<ARRAY_SUPPORT_TYPES>* arrList = get<ArrayList<ARRAY_SUPPORT_TYPES>*>( returnIndex );

				if( arrList->totalElementsAllocated <= updationIndex ){
					size_t cur = arrList->totalElementsAllocated;
					for(; cur <= updationIndex; cur++)
						arrList->push_SingleElement( VarDtype{0} );
				}
				auto& elementAtIndex = arrList->arrayList[ updationIndex ];

				if( holds_alternative<ARRAY_SUPPORT_TYPES>( elementAtIndex ) ){
					auto arrData = get<ARRAY_SUPPORT_TYPES>( elementAtIndex );
					visit( [&]( auto&& data ){ arrList->arrayList[updationIndex] = data; }, upvalue );
				}
				else throw InvalidDTypeError("Dtype mismatch in array updation");
			}
		}

		void InstructionHandlerRunner( const vector<Token>& tokens, size_t& currentPtr ){
			InstructionTokens InsTokensAndData = stringToInsToken( tokens, currentPtr );

			if( !isValidInstructionSet( InsTokensAndData.insToken) )
				throw InvalidSyntaxError( "Invalid Instruction set" );

			queue<DEEP_VALUE_DATA> finalValueQueue;
			vector<Token>& varsAndVals = InsTokensAndData.leftVector;

			if( InsTokensAndData.optr == INS_TOKEN::TYPE_CAST ){
				for( int x = 0; x < varsAndVals.size(); x++ ){
					auto [mapData, rPT] = getFromVmap( varsAndVals[ x ].token );

					if( x >= InsTokensAndData.rightVector.size() )
						throw InvalidSyntaxError("Typecasting error");

					vector<Token>castInfo = InsTokensAndData.rightVector[x];
					if( castInfo.size() != 1 )
						throw InvalidSyntaxError("Typecasting error");

					Token& top = castInfo.back();

					if( mapData == nullptr )
						throw InvalidSyntaxError(
								"Unknown token: " + varsAndVals[x].token + " at line: " + to_string( varsAndVals[x].row ) 
							);

					if( mapData->mapType != MAPTYPE::VARIABLE ){
						throw InvalidSyntaxError(
							"Only variables are allowed for TYPE_CASTING\n Error at line: " + to_string( varsAndVals[x].row )
						);
					}
					if( top.token == "INT" ){
						mapData->typeCastToInt();
					}
					else if( top.token == "THULA" ){
						mapData->typeCastToDouble();
					}
					else if( top.token == "STR" ){
						mapData->typeCastToString();
					}
					else throw TypeCastError("Failed to cast");
				}
				return;
			}

			for( vector<Token>& astStrToks: InsTokensAndData.rightVector ){
				DEEP_VALUE_DATA data = evaluateVector( astStrToks );
				finalValueQueue.push( data  );
			}

			for( size_t x = 0; x < varsAndVals.size(); x++ ){
				auto [mapData, rPT] = getFromVmap( varsAndVals[ x ].token );
				
				if( mapData == nullptr )
					throw InvalidSyntaxError(
							"Unknown token: " + varsAndVals[x].token + " at line: " + to_string( varsAndVals[x].row ) 
						);

				if( mapData->mapType != MAPTYPE::VARIABLE && mapData->mapType != MAPTYPE::ARRAY_PTR ){
					throw InvalidSyntaxError(
						"Only variables are allowed for updation\n Error at line: " + to_string( varsAndVals[x].row )
					);
				}

				DEEP_VALUE_DATA topValue = finalValueQueue.front();
				finalValueQueue.pop();

				if( holds_alternative<FUNCTION_MAP_DATA*>(topValue) ){
					mapData->mapType = MAPTYPE::FUNC_PTR;
					mapData->var = get<FUNCTION_MAP_DATA*>(topValue);
					continue;
				}

				if( mapData->mapType == MAPTYPE::ARRAY_PTR ){
					auto& arr = get<ArrayList<ARRAY_SUPPORT_TYPES>*>( mapData->var );
					arrayUpdation( varsAndVals, x, topValue, arr );
					return ;
				}

				auto vmapvariable = get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( mapData->var );
				if( holds_alternative<VarDtype> ( vmapvariable->value ) ){
					auto& varDtypeData = get<VarDtype>( vmapvariable->value );

					if( holds_alternative<string> ( varDtypeData ) ){
						ArrayAccessTokens arrToken = stringToArrayAccesToken( varsAndVals, x );
						x--;
						
						auto resData = getResolvedIndexVectors( arrToken.indexVector );
						
						if( resData.size() > 1 )
							throw runtime_error("string is a one dimensional");

						string& strToUpdate = get<string> ( varDtypeData );
						if( !resData.empty() ){
							long int index = resData.back();
							updateString( strToUpdate, index, topValue );
						}
						else mapData->updateSingleVariable( get<VarDtype>( topValue ) );
						return;
					}
				}
				if( vmapvariable->isTypeArray ){
					auto& arr = get<unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( vmapvariable->value );
					arrayUpdation( varsAndVals, x, topValue, arr.get());
				}
				else if( holds_alternative<VarDtype>( topValue ) )
					mapData->updateSingleVariable( get<VarDtype>( topValue ) );
			}
		}

		void 
		functionDefHandlerRunner( const vector<Token>&token, size_t& start ){
			FunctionTokenReturn funcTokens = stringToFuncTokens( token, start );

			if( isValidFunction( funcTokens.tokens ) ){ 
				if( this->getFromVmap( funcTokens.funcName ).first != nullptr )
					throw InvalidSyntaxError( funcTokens.funcName + " already defined" );
				
				unique_ptr<FUNCTION_MAP_DATA> funcMapData = make_unique<FUNCTION_MAP_DATA>();
				funcMapData->funcName 		= funcTokens.funcName;
				funcMapData->argsSize 		= funcTokens.args.size();
				funcMapData->bodyStartPtr 	= funcTokens.funcStartPtr;
				funcMapData->bodyEndPtr 	= funcTokens.funcEndPtr;
				funcMapData->argsInfo 		= move( funcTokens.args );
				
				auto vmapCopy = this->getDeepCopyOfVMAP();

				funcMapData->varMapCopy		= make_pair(vmapCopy, this->parent);

				unique_ptr<MapItem> funcMapItem = make_unique<MapItem>();
				funcMapItem->mapType = MAPTYPE::FUNCTION;
				funcMapData->varMapCopy.first[funcTokens.funcName] = funcMapItem.get();
				funcMapItem->var 	 = funcMapData.get();
				_CACHE_VARS.push_back( move(funcMapData) );					
				this->addToMap( funcTokens.funcName, move( funcMapItem ) );

			}
			else throw InvalidSyntaxError( "Syntax Error occured in thenga" );
		}

		optional<variant<VarDtype, unique_ptr<MapItem>>>
		getReturnedData( const vector<Token>&tokens, size_t& currentPtr ){
			currentPtr++;
			vector<Token> returnStatementData;

			while( currentPtr < tokens.size() ){
				const Token& curToken = tokens[ currentPtr ];
				if( curToken.token == ";" ) break;
				returnStatementData.push_back( curToken );
				currentPtr++;
			}

			if( returnStatementData.empty() )
				return nullopt;

			if( returnStatementData.size() == 1 && returnStatementData.back().type == TOKEN_TYPE::IDENTIFIER ){
				auto mapData = this->moveFromVmap( returnStatementData.back().token );
				if( mapData.has_value() ){
					for( auto& mapData: this->VMAP )
						propHolderTemp.push_back( move( mapData.second ) );
					return  move( mapData.value() ) ;
				}
				throw InvalidSyntaxError(
					"no variable found line:  " + to_string(returnStatementData.back().row) + " " + returnStatementData.back().token
				);
			}
			DEEP_VALUE_DATA res = evaluateVector( returnStatementData );

			if( holds_alternative<VarDtype> ( res ) )
				return get<VarDtype>( res );

			throw InvalidSyntaxError("Invalid return statement");
		}

		void IOHandlerRunner( const vector<Token>& tokens, size_t& start ){
				auto tokensAndData = stringToIoTokens( tokens, start );
				
				if( !isValidIoTokens( tokensAndData.first ) )
					return;

				queue<DEEP_VALUE_DATA> finalQueue;
				
				for( vector<Token>&valToks: tokensAndData.second ){
					DEEP_VALUE_DATA data = evaluateVector( valToks );
					finalQueue.push(data);
				}
		
				bool isFirstPrint = true;
				for( IO_TOKENS tok: tokensAndData.first ){
					switch( tok ){
						case IO_TOKENS::PRINT: {
							if( !isFirstPrint )
								cout << "\n";
							else isFirstPrint = false;

							if( finalQueue.empty() )
								throw InvalidSyntaxError("In para statement");

							auto top = finalQueue.front();
							finalQueue.pop();
							ValueHelper::printDEEP_VALUE_DATA( top );
							break;
						}
						case IO_TOKENS::CONCAT:{
							cout << " ";
							if( finalQueue.empty() )
								throw InvalidSyntaxError("In koode statement");

							auto top = finalQueue.front();
							finalQueue.pop();
							ValueHelper::printDEEP_VALUE_DATA( top );
							break;
						}
						case IO_TOKENS::PRINT_VALUE:
							continue;
						
						case IO_TOKENS::END:
						default:
							return;
					}
				}
			}
};

#endif