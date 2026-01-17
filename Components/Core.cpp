#include "DefinedTypes.hpp"

using namespace std;

class LoopHandler;


class VAR_VMAP{
	private:
		queue<unique_ptr<MapItem>> funcReturnedCache;
		unordered_map<string, 
			pair<unordered_map<string, MapItem*>, VAR_VMAP*>
		> functionMapsCache;
		size_t cacheElement = 0;

	public:
		string runnerBody = "__xmain__";
		VAR_VMAP* parent = nullptr;
		unordered_map<string, unique_ptr<MapItem>> VMAP;
		unordered_map<string, MapItem*> VMAP_COPY;

		pair<unordered_map<string, MapItem*>, VAR_VMAP*>
		getFromFunctionMapsCache(const string& key){
			VAR_VMAP* cur = this;
			while( cur != nullptr ){
				auto result = cur->functionMapsCache.find(key);
				if( result != cur->functionMapsCache.end() )
					return result->second;
				cur = cur->parent;
			}
			throw runtime_error(key);
		}

		void addTofunctionMapsCache( const string& key, unordered_map<string, MapItem*> data, VAR_VMAP* parent ){
			this->functionMapsCache[ key ] = make_pair(data, parent);
		}

		unordered_map<string, MapItem*>
		getCopyOfVMAP( void ){
			unordered_map<string, MapItem*> copyData;
			VAR_VMAP* currentDir = this;
			while( currentDir && currentDir->runnerBody == this->runnerBody ){
				for( auto& value: currentDir->VMAP )
					copyData[value.first] = value.second.get();
				currentDir = currentDir->parent;
			}
			return copyData;
		}

		pair<MapItem*, VAR_VMAP*>
		getFromVmapCopy(const std::string& key) {
		    VAR_VMAP* currentEnv = this;
 		    while (currentEnv != nullptr) {
		        auto it_2 = currentEnv->VMAP_COPY.find(key);
		        if( it_2 != currentEnv->VMAP_COPY.end() )
		        	return make_pair( it_2->second, currentEnv );
 		        currentEnv = currentEnv->parent;
		    }
		    return make_pair(nullptr, nullptr);
		}

		pair<MapItem*, VAR_VMAP*>
		getFromVmap(const std::string& key) {
	        auto cur = this;
	        while( cur && cur->runnerBody == this->runnerBody ){
	        	auto it = cur->VMAP.find(key);
	       		if (it != cur->VMAP.end())
	            	return make_pair( it->second.get(), cur );
	            cur = cur->parent;
	        }
		    return getFromVmapCopy(key);
		}

		optional<unique_ptr<MapItem>>
		moveFromVmap(const std::string& key){
		    auto it = VMAP.find(key);
		    if (it == VMAP.end())
		        return std::nullopt;
		    auto result = std::move(it->second);
		    VMAP.erase(it);
		    return result;
		}

		void addToMap( string key, unique_ptr<MapItem> data ){
			this->VMAP[ key ] = move( data );
		}

		void pushCache( unique_ptr<MapItem> data ){
			this->cacheElement++;
			this->funcReturnedCache.push( move( data ) );
		}

		unique_ptr<MapItem> getCacheItem(void){
			auto data = move( this->funcReturnedCache.front() );
			this->funcReturnedCache.pop();
			this->cacheElement--;
			return move(data);
		}

		size_t getCacheSize( void ){
			return this->cacheElement;
		}

		void clearFuncRetCache( void ){
			while( !this->funcReturnedCache.empty() )
				funcReturnedCache.pop();
			this->cacheElement = 0;
		}

};

vector<Token> fullTokens;
size_t 		   pointer = 0;

enum class CALLER{ LOOP, FUNCTION, CONDITIONAL };

template <typename T>
optional<variant<VarDtype, unique_ptr<MapItem>>>
ProgramExecutor( const vector<Token>& tokens, 
				 size_t& currentPtr, 
				 CALLER C_CLASS, T* prntClass, 
				 size_t endPtr = 0  
);

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

		optional<size_t>
		handleArrayProperties( ArrayList<ARRAY_SUPPORT_TYPES>* array, ArrayAccessTokens& arrToken){
			if( arrToken.arrProperty.propertyType == "valupam" ){
				return array->totalElementsAllocated;
			}
			return nullopt;
		}

		void handleArrayCases( 
				ArrayList<ARRAY_SUPPORT_TYPES>* arrayData, 
				ArrayAccessTokens& arrToken,
				queue<DEEP_VALUE_DATA>& resolvedVector,
				vector<string>& simpleVector
		){
			if( arrToken.indexVector.size() ){
				vector<long int> resolvedIndexVector;

				for( auto& vec: arrToken.indexVector ){
					DEEP_VALUE_DATA val = getTheResult( vec );
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
					
					resolvedIndexVector.push_back( index );
				}

				variant<ArrayList<ARRAY_SUPPORT_TYPES>*, ARRAY_SUPPORT_TYPES> returnIndex = arrayData->getElementAtIndex( resolvedIndexVector, 0 );
				
				if( holds_alternative<ARRAY_SUPPORT_TYPES> ( returnIndex ) ){
					auto spData = get<ARRAY_SUPPORT_TYPES>( returnIndex );

					visit( [&]( auto&& data ){
						resolvedVector.push( data );
						simpleVector.push_back("VAR");
					},  spData );
				}
				else{
					ArrayList<ARRAY_SUPPORT_TYPES>* arrList = get<ArrayList<ARRAY_SUPPORT_TYPES>*>( returnIndex );
					if( arrToken.isTouchedArrayProperty ){
						auto data = handleArrayProperties( arrList, arrToken );
						if( data.has_value() ) 
							resolvedVector.push( (long int) data.value() );
					}
					else resolvedVector.push( arrList );
					simpleVector.push_back("VAR");
				}
			}
			else {
				if( arrToken.isTouchedArrayProperty ){
					auto data = handleArrayProperties( arrayData, arrToken );
					if( data.has_value() )
						resolvedVector.push( (long int) data.value() );
				}
				else resolvedVector.push( arrayData );
				simpleVector.push_back("VAR");
			
			}	
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

		optional<variant<VarDtype, unique_ptr<MapItem>>>	
		zeroArgFunction( MapItem* func, const vector<Token>& tokens, string funcName, VAR_VMAP* rPT ){
			auto funcFromMap = ( func->mapType == MAPTYPE::FUNCTION ) ? get<unique_ptr<FUNCTION_MAP_DATA>>( func->var ).get() \
								: get<FUNCTION_MAP_DATA*>( func->var );

			size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
			size_t funcEndStartPtr = funcFromMap->bodyEndPtr;

			unique_ptr<FunctionHandler> newFuncRunner = make_unique<FunctionHandler>();
			newFuncRunner->runnerBody = funcName;
			auto funcMetaData = getFromFunctionMapsCache( funcName );
			newFuncRunner->VMAP_COPY = funcMetaData.first;
			newFuncRunner->parent = funcMetaData.second;

			return ProgramExecutor( 
					tokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr 
				);
		}

		optional<variant<VarDtype, unique_ptr<MapItem>>>
		handleFunctionCall( MapItem* func, const vector<Token>& tokens, size_t& currentPtr, VAR_VMAP* rPT, optional<FunctionCallReturns> data = nullopt ){
			FunctionCallReturns Data;
			if( data.has_value() )
				Data = data.value();
			else Data = stringToFunctionCallTokens( tokens, currentPtr );

			if( isValidFuncCall( Data.callTokens ) ){
				if( Data.argsVector.size() == 0 )
					return zeroArgFunction( func, tokens, Data.funcName, rPT );

				unique_ptr<FunctionHandler> newFuncRunner = make_unique<FunctionHandler>();
				newFuncRunner->runnerBody = Data.funcName;
				auto funcMetaData = getFromFunctionMapsCache(  Data.funcName );
				newFuncRunner->VMAP_COPY = funcMetaData.first;
				newFuncRunner->parent = funcMetaData.second;

				queue<DEEP_VALUE_DATA> resolvedArgs;
				
				auto funcFromMap = ( func->mapType == MAPTYPE::FUNCTION ) ? get<unique_ptr<FUNCTION_MAP_DATA>>( func->var ).get() \
								: get<FUNCTION_MAP_DATA*>( func->var );

				int total_comma = Data.argsVector.size() - 1;

				vector<Token> lhsTokens;
				lhsTokens.push_back( Token( TOKEN_TYPE::OPERATOR, "=", 0, 0 ) );

				for( auto argSingleVec: Data.argsVector ){
					DEEP_VALUE_DATA dpData = getTheResult( argSingleVec );

					resolvedArgs.push( dpData );
					lhsTokens.push_back( Token( TOKEN_TYPE::IDENTIFIER, "NUM", 0, 0) );
					if( total_comma )
						lhsTokens.push_back( Token( TOKEN_TYPE::SPEC_CHAR, ",", 0, 0) );
					total_comma--;
				}

				lhsTokens.push_back(Token( TOKEN_TYPE::SPEC_CHAR, ";", 0, 0));

				size_t start = 0;
				VariableTokens funcVars = stringToVariableTokens( lhsTokens, start );
				
				if( !isValidValueSyntax( funcVars.valueTokens ) )
					throw InvalidSyntaxError("Invalid args initalization in thenga");

				for( auto& ArgsInfo: funcFromMap->argsInfo ){
					if( resolvedArgs.empty() )
						throw InvalidSyntaxError("No value to initialized the args in thenga");
					DEEP_VALUE_DATA topValue = resolvedArgs.front();
					resolvedArgs.pop();

					if( holds_alternative<VarDtype> ( topValue ) ){
						// create variable
						unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
						newVariable->key = ArgsInfo->name;
						newVariable->isTypeArray = ArgsInfo->isArray;
						newVariable->value = get<VarDtype>( topValue );

						// add to vmap
						auto newMapVar = make_unique<MapItem>( );
						newMapVar->mapType = MAPTYPE::VARIABLE;
						newMapVar->var = move( newVariable ) ;
						newFuncRunner->addToMap( ArgsInfo->name, move( newMapVar ) );

					}
					else if(holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*> ( topValue )){
						// no need to create variable add to map
						string& key    = ArgsInfo->name;
						auto newMapVar = make_unique<MapItem>( );
						newMapVar->mapType = MAPTYPE::ARRAY_PTR;
						newMapVar->var = get<ArrayList<ARRAY_SUPPORT_TYPES>*>(topValue) ;
						newFuncRunner->addToMap( key, move( newMapVar ) );
					}

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
			VariableTokens tokens  = stringToVariableTokens( test, start );  // convert variable to tokens ( leaves entire value parts )
			auto toks 	 = make_pair(tokens.varTokens, tokens.valueTokens);  // variable tokens and value tokens
			auto names 	 = make_pair(tokens.VarQueue,  tokens.valueVector);  // values

			queue<DEEP_VALUE_DATA> resolvedValueVector;
			size_t curIndex = 0;

			for( auto valVec: toks.second ){
				if( valVec == VALUE_TOKENS::ARRAY_VALUE ||
					valVec == VALUE_TOKENS::NORMAL_VALUE ){
					auto testVec = names.second[ curIndex++ ];

					DEEP_VALUE_DATA evaluatedRes = getTheResult( testVec );
					resolvedValueVector.push( evaluatedRes );
				}
			}

			vector<VAR_INFO> varInfos;

			if( isValidVariableSyntax( toks.first, varInfos, names.first ) ){
				for( auto& varVerification: varInfos ){
					auto [data, rPT] = this->getFromVmap( varVerification.varName );	
					if( data ) throw VariableAlreayExists( varVerification.varName );
				}

				if( !isValidValueSyntax( toks.second ) )
					throw InvalidSyntaxError("Invalid Value Init\n");

				for( size_t x = 0, i = 0; x < toks.second.size(); x++ ){
					if( i >= varInfos.size() && resolvedValueVector.empty() )
						break;

					auto curValueToken = toks.second[x];

					if( curValueToken != VALUE_TOKENS::NORMAL_VALUE && 
						curValueToken != VALUE_TOKENS::ARRAY_VALUE && 
						curValueToken != VALUE_TOKENS::ARRAY_OPEN )
						continue;

					if( i < varInfos.size() && resolvedValueVector.empty() )
						throw InvalidSyntaxError(
							"more variables to initialized\n"
						);

					if( i >= varInfos.size() && !resolvedValueVector.empty() )
						throw InvalidSyntaxError("No variable to store the value\n");

					VAR_INFO curVarInfo = varInfos[ i++ ];

					if( curValueToken == VALUE_TOKENS::NORMAL_VALUE ){
						auto curValue = resolvedValueVector.front();
						resolvedValueVector.pop();

						if( curVarInfo.isTypeArray && !holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>( curValue ) )
							throw InvalidSyntaxError("Assigning value to array type is invalid");

						if( holds_alternative<VarDtype> ( curValue ) ){
							unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
							newVariable->key = curVarInfo.varName;
							newVariable->isTypeArray = false;
							newVariable->value = get<VarDtype>( curValue );

							auto newMapVar = make_unique<MapItem>( );
							newMapVar->mapType = MAPTYPE::VARIABLE;
							newMapVar->var = move( newVariable ) ;
							this->addToMap( curVarInfo.varName, move(newMapVar) );
						}
						else if(holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*> ( curValue )){
							string& key    = curVarInfo.varName;
							auto newMapVar = make_unique<MapItem>( );
							newMapVar->mapType = MAPTYPE::ARRAY_PTR;
							newMapVar->var = get<ArrayList<ARRAY_SUPPORT_TYPES>*>(curValue) ;
							this->addToMap( key, move( newMapVar ) );
						}
						else if( holds_alternative<FUNCTION_MAP_DATA*>( curValue ) ){
							string& key    = curVarInfo.varName;
							auto newMapVar = make_unique<MapItem>( );
							newMapVar->mapType = MAPTYPE::FUNC_PTR;
							newMapVar->var = get<FUNCTION_MAP_DATA*>(curValue);
							this->addToMap( key, move( newMapVar ) );

							auto funcCache = this->getCopyOfVMAP( );
							this->addTofunctionMapsCache( curVarInfo.varName, funcCache, this );
						}

					}
					else if( curValueToken == VALUE_TOKENS::ARRAY_OPEN ){
						x++;

						auto newArray = ArrayList<ARRAY_SUPPORT_TYPES>::createArray<DEEP_VALUE_DATA>( 
											toks.second,  x, resolvedValueVector );

						unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
						newVariable->key = curVarInfo.varName;
						newVariable->isTypeArray = true;
						newVariable->value = move( newArray );

						auto newMapVar = make_unique<MapItem>( );
						newMapVar->mapType = MAPTYPE::VARIABLE;
						newMapVar->var = move( newVariable ) ;
						this->addToMap( curVarInfo.varName, move(newMapVar) );
					}
				}
			}
			else throw InvalidDTypeError( "Invalid Syntax occured in variable declaration" );
		}

		void IOHandlerRunner( const vector<Token>& tokens, size_t& start ){
				auto tokensAndData = stringToIoTokens( tokens, start );
				
				if( !isValidIoTokens( tokensAndData.first ) )
					return;

				queue<DEEP_VALUE_DATA> finalQueue;
				
				for( vector<Token>&valToks: tokensAndData.second ){
					DEEP_VALUE_DATA data = getTheResult( valToks );
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

		void arrayUpdation( const vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, ArrayList<ARRAY_SUPPORT_TYPES>* arr ){
			ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, curPtr );
			curPtr--;

			if( arrToken.isTouchedArrayProperty )
				throw InvalidSyntaxError("Using array property in updatation is not allowed");

			if( arrToken.indexVector.size() ){
				vector<long int> resolvedIndexVector;

				for( auto& vec: arrToken.indexVector ){
					DEEP_VALUE_DATA val = getTheResult( vec );

					if( !holds_alternative<VarDtype>( val ) )
						throw InvalidSyntaxError("Array Index Expects Type Integer");
					
					VarDtype vDtypeIndex = get<VarDtype>( val );
					
					if( !holds_alternative<long int> ( vDtypeIndex ) && !holds_alternative<double> ( vDtypeIndex ))
						throw InvalidSyntaxError("Array Index Expects Integer");

					long int index;
					
					if( holds_alternative<long int>(vDtypeIndex) )
						index = get<long int>(vDtypeIndex);
					else index = (long int) get<double>( vDtypeIndex );
				
					resolvedIndexVector.push_back( index );
				}

				long int updationIndex = resolvedIndexVector.back();
				resolvedIndexVector.pop_back();

				if( updationIndex < 0 )
					throw InvalidSyntaxError("Array Index Expects Unsigned Integer");

				variant<ArrayList<ARRAY_SUPPORT_TYPES>*, ARRAY_SUPPORT_TYPES> returnIndex = arr->getElementAtIndex( resolvedIndexVector, 0 );

				if( holds_alternative<ARRAY_SUPPORT_TYPES> ( returnIndex ) )
					throw InvalidSyntaxError("Failed to index an non array type");

				ArrayList<ARRAY_SUPPORT_TYPES>* arrList = get<ArrayList<ARRAY_SUPPORT_TYPES>*>( returnIndex );

				if( arrList->totalElementsAllocated <= updationIndex )
					throw ArrayOutOfBound( to_string( updationIndex ) );

				auto& elementAtIndex = arrList->arrayList[ updationIndex ];

				if( holds_alternative<ARRAY_SUPPORT_TYPES>( elementAtIndex ) ){
					auto arrData = get<ARRAY_SUPPORT_TYPES>( elementAtIndex );

				 	if( holds_alternative<VarDtype>( arrData ) && holds_alternative<VarDtype>( upvalue ))
						arrList->arrayList[ updationIndex ] = get<VarDtype>( upvalue );
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

			for( vector<Token>& astStrToks: InsTokensAndData.rightVector ){
				DEEP_VALUE_DATA data = getTheResult( astStrToks );
				finalValueQueue.push( data  );
			}

			for( size_t x = 0; x < varsAndVals.size(); x++ ){
				auto [mapData, rPT] = getFromVmap( varsAndVals[ x ].token );
				
				if( mapData == nullptr )
					throw InvalidSyntaxError(
							"Unknown token: " + varsAndVals[x].token + " at line: " + to_string( varsAndVals[x].row ) 
						);

				if( mapData->mapType != MAPTYPE::VARIABLE && mapData->mapType != MAPTYPE::ARRAY_PTR )
					throw InvalidSyntaxError(
						"Only variables are allowed for updation\n Error at line: " + to_string( varsAndVals[x].row )
					);


				DEEP_VALUE_DATA topValue = finalValueQueue.front();
				finalValueQueue.pop();

				if( mapData->mapType == MAPTYPE::ARRAY_PTR ){
					auto& arr = get<ArrayList<ARRAY_SUPPORT_TYPES>*>( mapData->var );
					arrayUpdation( varsAndVals, x, topValue, arr );
					return ;
				}

				auto& vmapvariable = get<unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( mapData->var );

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

				unique_ptr<MapItem> funcMapItem = make_unique<MapItem>();
				
				funcMapItem->mapType = MAPTYPE::FUNCTION;
				funcMapItem->var 	 = move( funcMapData );

				this->addToMap( funcTokens.funcName, move( funcMapItem ) );
				auto funcCache = this->getCopyOfVMAP( );

				this->addTofunctionMapsCache( funcTokens.funcName, funcCache, this );
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
				if( mapData.has_value() )
					return  move( mapData.value() ) ;
				throw InvalidSyntaxError(
					"no variable found line:  " + to_string(returnStatementData.back().row) + " " + returnStatementData.back().token
				);
			}
			DEEP_VALUE_DATA res = getTheResult( returnStatementData );
			if( holds_alternative<VarDtype> ( res ) )
				return get<VarDtype>( res );
			throw InvalidSyntaxError("Invalid return statement");
		}
};

class Conditional: public FunctionHandler{
	public:
		unique_ptr<LoopHandler> lpRunner;
		Conditional( unique_ptr<LoopHandler> lpRunner ){
			this->lpRunner = move( lpRunner );
		}
		void CondHandlerRunner( const vector<Token>& tokens, size_t& start ){
			size_t endOfNOK = 0;
			auto ctokens = stringToCondTokens( tokens, start, endOfNOK );
			if( !isValidCondToken( ctokens.first ) ) return;

			for(int x = 0; x < ctokens.second.size(); x++){
				auto data = ctokens.second[ x ];
				DEEP_VALUE_DATA evRes = getTheResult( data.first );

				if( holds_alternative<VarDtype>( evRes ) ){
					auto VdtypData = get<VarDtype>( evRes );
					
					if( get<bool>( VdtypData ) ){
						start = data.second + 1;
						// nothing will return here so just skip
						ProgramExecutor( tokens, start, CALLER::CONDITIONAL, this );
						start = endOfNOK;
						return;
					}
				}
			}
			throw InvalidSyntaxError("Conditional Expression expects boolean values\n");
		}
};

class LoopHandler: public FunctionHandler{
	public:		
		void 
		LoopHandlerRunner ( const vector<Token>& tokens, size_t& currentPtr ){
			size_t beginCopy = currentPtr;
		 	LoopTokens lpTokens = stringToLoopTokens( tokens, currentPtr );
		 	auto bodyIns = make_pair( lpTokens.startPtr, lpTokens.endPtr );
			
			if( !isValidLoopTokens( lpTokens.lpTokens ) )
				throw InvalidSyntaxError("Invalid Syntax error in loop");

			while( true ){
				DEEP_VALUE_DATA finalValue = getTheResult( lpTokens.conditions );
				if( !holds_alternative<VarDtype>(finalValue) )
					throw InvalidSyntaxError( "loop condition should be boolean or blank" );

				VarDtype cdData = get<VarDtype>(finalValue);

				if( !holds_alternative<bool>(cdData) )
					throw InvalidSyntaxError( "loop condition should be boolean or blank" );

				bool runTheBodyAgain = get<bool>( cdData );

				if( runTheBodyAgain ){
					size_t bodyStart = bodyIns.first;
					try{
						// nothing will return here so just skip
						ProgramExecutor( tokens, bodyStart, CALLER::LOOP, this );
						currentPtr = beginCopy;
					} 
					catch( const InvalidSyntaxError& err ){
						currentPtr = bodyStart;
						const string& expTok = tokens[ bodyStart ].token;
						if( expTok == "theku" )
							break;
						else if( expTok == "pinnava" )
							currentPtr = beginCopy;
						else throw err;
					}
					return;
				}
				else break;
			}
			currentPtr = bodyIns.second;
		}
};

template <typename PARENT_CLASS>
optional<variant<VarDtype, unique_ptr<MapItem>>>
ProgramExecutor( const vector<Token>& tokens, 
				 size_t& currentPtr, 
				 CALLER C_CLASS, 
				 PARENT_CLASS* prntClass, 
				 size_t endPtr
){
	size_t backUpPtr = currentPtr;

	while( currentPtr < tokens.size() ){
		backUpPtr = currentPtr;

		const string& curToken = tokens[currentPtr].token;

		if( endPtr && currentPtr >= endPtr - 1 )
			return nullopt;

		if( curToken == "pidi" ){
			prntClass->VarHandlerRunner( tokens, currentPtr );
		}
		else if( curToken == "nok" ){
			try{
				unique_ptr<LoopHandler> lpHandler = make_unique<LoopHandler>();
				lpHandler->runnerBody = prntClass->runnerBody;
				lpHandler->parent = prntClass;
				
				shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );
				cdHandler->runnerBody = prntClass->runnerBody;
				cdHandler->parent = prntClass;
				
				cdHandler->CondHandlerRunner( tokens, currentPtr );
			}
			catch( const InvalidSyntaxError& err ){
				continue;
			}
		}
		else if( curToken == "}" ){
			if( prntClass->parent == nullptr )
				throw InvalidSyntaxError( "unknown token }" );
			return nullopt;
		}
		else if( curToken == "para" ){
			prntClass->IOHandlerRunner( tokens, currentPtr );
		}
		else if( curToken == "thenga" ){
			prntClass->functionDefHandlerRunner( tokens, currentPtr );
			currentPtr--;
		}
		else if( curToken == "ittuthiri" ){
			try{
				unique_ptr<LoopHandler> newLpHander = make_unique<LoopHandler>();
				newLpHander->parent = prntClass;
				newLpHander->runnerBody = prntClass->runnerBody;
				newLpHander->LoopHandlerRunner( tokens, currentPtr );
				currentPtr--;
			}
			catch( const InvalidSyntaxError& err ){
				continue;
			}
		}
		else{
			switch( C_CLASS ){
				case CALLER::FUNCTION: {
					if( curToken == "poda" ){
						auto test = prntClass->getReturnedData( tokens, currentPtr );
						return test;
					}
				}
				default: break;
			}
			auto [func, rPT] = prntClass->getFromVmap( tokens[ currentPtr ].token );
			if( func && ( func->mapType == MAPTYPE::FUNCTION || func->mapType == MAPTYPE::FUNC_PTR) ){
				prntClass->handleFunctionCall( func, tokens, currentPtr, rPT );
			}
			else{
				try{
					backUpPtr = currentPtr;
					prntClass->InstructionHandlerRunner( tokens, currentPtr );
				}
				catch( const InvalidSyntaxError& err ){
					currentPtr = backUpPtr;
					throw InvalidSyntaxError(
							"Encounter error in function at line:  " + 
								to_string( tokens[currentPtr].row + 1) + " col " +  to_string( tokens[ currentPtr ].col )
							);
				}
			}
		}
		currentPtr++;
	}
	return nullopt;
}

int main( int argc, char *argv[] ){
	try{
		string filename = argv[1];
		getTheTokens( filename, fullTokens );

		size_t start = 0;
		unique_ptr<LoopHandler> lpHandler = make_unique<LoopHandler>();
		shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );

		unique_ptr<FunctionHandler> func = make_unique<FunctionHandler>( );
		func->functionName = "MAIN";
		auto dd = move(ProgramExecutor( fullTokens, pointer, CALLER::FUNCTION, func.get() ));
	}
	catch( InvalidSyntaxError& err ){
		cout << err.what() << endl;
	}
	return 0;
}