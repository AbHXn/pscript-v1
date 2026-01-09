#include "DefinedTypes.hpp"

using namespace std;

class LoopHandler;

class VAR_VMAP{
	private:
		queue<unique_ptr<MapItem>> funcReturnedCache;
		size_t cacheElement = 0;

	public:
		string runnerBody = "__xmain__";
		VAR_VMAP* parent = nullptr;
		unordered_map<string, unique_ptr<MapItem>> VMAP;

		MapItem*
		getFromVmap(const std::string& key) const{
		    const VAR_VMAP* currentEnv = this;
		    while (currentEnv != nullptr) {
		        auto it = currentEnv->VMAP.find(key);
		        if (it != currentEnv->VMAP.end())
		            return it->second.get();
		        currentEnv = currentEnv->parent;
		    }
		    return nullptr;
		}

		VAR_VMAP* getPrevParent(){
			VAR_VMAP* curParent = this->parent;
			while( curParent && curParent->runnerBody == this->runnerBody )
				curParent = curParent->parent;
			return curParent;
		}

		optional<unique_ptr<MapItem>>
		moveFromVmap(const std::string& key){
		    auto it = VMAP.find(key);
		    if (it == VMAP.end())
		        return std::nullopt;
		    auto result = std::move(it->second);
		    VMAP.erase(it);   // important after move
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
				return ValueHelper::evaluate_AST_NODE( AST_NODE );
			}
			throw InvalidDTypeError("Failed to resolve the vector\n");
		}

		optional<size_t>
		handleArrayProperties( ArrayList* array, ArrayAccessTokens& arrToken){
			if( arrToken.arrProperty.propertyType == "valupam" ){
				return array->totalElementsAllocated;
			}
			return nullopt;
		}

		pair<queue<DEEP_VALUE_DATA>, vector<string>>
		vectorResolver( const vector<Token>& tokens ){
			queue<DEEP_VALUE_DATA> resolvedVector;       
			vector<string> simpleVector;

			for( size_t x = 0; x < tokens.size(); x++ ){
				const Token& tok = tokens[ x ];
				const string& curToken = tokens[ x ].token;

				if( isRegisteredASTExprTokens( curToken ) || curToken == ")" ||  curToken == "(" ){
					simpleVector.push_back( curToken );
					continue;
				}
				
				if( isValueType( tok.type ) ){
					VarDtype value = this->getValueFromToken( tok );
					simpleVector.push_back("NUM");
					resolvedVector.push( value );
				}
				
				else if( tok.type == TOKEN_TYPE::IDENTIFIER ){
					auto VMAPData = this->getFromVmap( curToken );

					if( VMAPData != nullptr ){
						// if it is variable then get its value
						if( VMAPData->mapType == MAPTYPE::VARIABLE ){
							auto& varHolder = get<unique_ptr<VARIABLE_HOLDER>>( VMAPData->var );
							if( !varHolder->isTypeArray ){
								auto& data = get<VarDtype>( varHolder->value );
								resolvedVector.push( data );
								simpleVector.push_back("VAR");
							}
							else {
								auto& arrayData = get<unique_ptr<ArrayList>>( varHolder->value );
								ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );
								x--;

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

									variant<ArrayList*, VarDtype> returnIndex = arrayData->getElementAtIndex( resolvedIndexVector, 0 );
									
									if( holds_alternative<VarDtype> ( returnIndex ) ){
										resolvedVector.push( get<VarDtype>( returnIndex ) );
										simpleVector.push_back("VAR");
									}
									else{
										ArrayList* arrList = get<ArrayList*>( returnIndex );
										if( arrToken.isTouchedArrayProperty ){
											auto data = handleArrayProperties( arrList, arrToken );
											if( data.has_value() ) {
												resolvedVector.push( (long int) data.value() );
												simpleVector.push_back("VAR");
											}
										}
										else{
											resolvedVector.push( arrList );
											simpleVector.push_back("VAR");
										}
									}
								}
								else {
									if( arrToken.isTouchedArrayProperty ){
										auto data = handleArrayProperties( arrayData.get(), arrToken );
										if( data.has_value() ) {
											resolvedVector.push( (long int) data.value() );
											simpleVector.push_back("VAR");
										}
									}
									else{
										resolvedVector.push( arrayData.get() );
										simpleVector.push_back("VAR");
									}
								}
							}
						}
						// if it is a function then call it and get the value
						else if( VMAPData->mapType == MAPTYPE::FUNCTION ){
							try{
								FunctionCallReturns pt = stringToFunctionCallTokens( tokens, x );
								auto data = this->handleFunctionCall( VMAPData, fullTokens, x, pt);
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
								x--; // stringfuncalltokens it hits then unknown token get that token back
							}
							catch ( const InvalidSyntaxError& err ){
								cout << err.what() << endl;
							}
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
		zeroArgFunction( MapItem* func, const vector<Token>& tokens, string funcName ){
			auto& funcFromMap = get<unique_ptr<FUNCTION_MAP_DATA>>( func->var );

			size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
			size_t funcEndStartPtr = funcFromMap->bodyEndPtr;

			unique_ptr<FunctionHandler> newFuncRunner = make_unique<FunctionHandler>();
			newFuncRunner->runnerBody = funcName;
			
			if( funcName == this->runnerBody )
				newFuncRunner->parent = getPrevParent();
			else newFuncRunner->parent = this;

			return ProgramExecutor( 
					tokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr 
				);
		}

		optional<variant<VarDtype, unique_ptr<MapItem>>>
		handleFunctionCall( MapItem* func, const vector<Token>& tokens, size_t& currentPtr, optional<FunctionCallReturns> data = nullopt ){
			FunctionCallReturns Data;
			if( data.has_value() )
				Data = data.value();
			else Data = stringToFunctionCallTokens( tokens, currentPtr );

			if( isValidFuncCall( Data.callTokens ) ){
				if( Data.argsVector.size() == 0 )
					return zeroArgFunction( func, tokens, Data.funcName );

				unique_ptr<FunctionHandler> newFuncRunner = make_unique<FunctionHandler>();
				newFuncRunner->runnerBody = Data.funcName;

				if( Data.funcName == this->runnerBody )
					newFuncRunner->parent = getPrevParent();
				else newFuncRunner->parent = this;

				auto& funcFromMap = get<unique_ptr<FUNCTION_MAP_DATA>>( func->var );
				vector<unique_ptr<VARIABLE_HOLDER>> vars;

				for( auto& var: funcFromMap->argsInfo ){
					unique_ptr<VARIABLE_HOLDER> data = make_unique<VARIABLE_HOLDER>( );
					data->key = var->name;
					data->isTypeArray = var->isArray;
					vars.push_back( move( data ) );
				}
				vector<Token> lhsTokens;
				lhsTokens.push_back( Token( TOKEN_TYPE::OPERATOR, "=", 0, 0 ) );
				queue<VarDtype> finalValueQueue;

				int total_comma = Data.argsVector.size() - 1;

				for( auto argSingleVec: Data.argsVector ){
					auto [ resValQueue, resToken ] = vectorResolver( argSingleVec );
					auto astTokenAndData = stringToASTTokens( resToken );

					size_t startAST = 0;
					auto newAstNode = BUILD_AST<DEEP_VALUE_DATA, AST_NODE_DATA>( 
												astTokenAndData, resValQueue, startAST 
												);
					if( !newAstNode.has_value( ) )
						return nullopt;

					DEEP_VALUE_DATA evaluationResult = ValueHelper::evaluate_AST_NODE( newAstNode.value( ) );

					if( !holds_alternative<VarDtype>( evaluationResult ) )
						throw ; /////////// HANDLER LATER;

					finalValueQueue.push( get<VarDtype>( evaluationResult ) );
					lhsTokens.push_back( Token( TOKEN_TYPE::IDENTIFIER, "NUM", 0, 0) );
					if( total_comma )
						lhsTokens.push_back( Token( TOKEN_TYPE::SPEC_CHAR, ",", 0, 0) );
					total_comma--;
				}
				lhsTokens.push_back(Token( TOKEN_TYPE::SPEC_CHAR, ";", 0, 0));

				size_t start = 0;
				VariableTokens funcVars = stringToVariableTokens( lhsTokens, start );
				
				if( isValidValueSyntax( funcVars.valueTokens, vars, finalValueQueue ) ){
					for( auto& Variable: vars ){
						string& key    = Variable->key;
						auto newMapVar = make_unique<MapItem>( );
						newMapVar->var = move( Variable );
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

			vector<unique_ptr<VARIABLE_HOLDER>> vars;
			if( isValidVariableSyntax( toks.first, vars, names.first ) ){

				for( auto& varVerification: vars ){
					auto data = this->getFromVmap( varVerification->key );	
					if( data ) throw VariableAlreayExists( varVerification->key );
				} 
				auto valueVectors = names.second;
				queue<VarDtype> finalValueQueue;

				for( auto& VarientData: valueVectors ){
					auto [ resValQueue, resToken ] = vectorResolver( VarientData );
					
					if( this->getCacheSize() > 1 ){
						this->clearFuncRetCache();
						throw InvalidSyntaxError("Invalid Dtypes in variable initialization");
					}
					if( this->getCacheSize() == 1 ){
						auto item = this->getCacheItem( );
						auto onlyVariable = move( vars.back() );

						if( item->mapType == MAPTYPE::VARIABLE ){
							auto& var = get<unique_ptr<VARIABLE_HOLDER>>( item->var );
							if( var->isTypeArray != onlyVariable->isTypeArray )
								throw InvalidSyntaxError("Variable property mismatch");
						}
						string VarName = onlyVariable->key;
						this->addToMap( VarName, move( item ) );
						return;
					}
					auto astTokenAndData = stringToASTTokens( resToken );
					size_t startAST = 0;
					auto newAstNode = BUILD_AST<DEEP_VALUE_DATA, AST_NODE_DATA>( 
														astTokenAndData, resValQueue, startAST 
													);
					if( !newAstNode.has_value() )
						return;
					DEEP_VALUE_DATA evaluationResult = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
					
					if( !holds_alternative<VarDtype>(evaluationResult) )
						throw ; // later 

					finalValueQueue.push( get<VarDtype>(evaluationResult) );
				}
				bool isValValue = isValidValueSyntax( toks.second, vars, finalValueQueue );
				if( isValValue ){
					for( auto& Variable: vars ){
						string& key    = Variable->key;
						auto newMapVar = make_unique<MapItem>( );
						newMapVar->var = move( Variable );
						this->addToMap( key, move( newMapVar ) );
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

		void arrayUpdation( const vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, ArrayList* arr ){
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

				variant<ArrayList*, VarDtype> returnIndex = arr->getElementAtIndex( resolvedIndexVector, 0 );

				if( holds_alternative<VarDtype> ( returnIndex ) )
					throw InvalidSyntaxError("Failed to index an non array type");

				ArrayList* arrList = get<ArrayList*>( returnIndex );


				if( arrList->totalElementsAllocated <= updationIndex )
					throw ArrayOutOfBound( to_string( updationIndex ) );

				auto& elementAtIndex = arrList->arrayList[ updationIndex ];
				if( holds_alternative<VarDtype>( elementAtIndex ) && holds_alternative<VarDtype>( upvalue ) ){
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
				MapItem* mapData = getFromVmap( varsAndVals[ x ].token );
				
				if( mapData == nullptr )
					throw InvalidSyntaxError(
							"Unknown token: " + varsAndVals[x].token + " at line: " + to_string( varsAndVals[x].row ) 
						);

				if( mapData->mapType != MAPTYPE::VARIABLE )
					throw InvalidSyntaxError(
						"Only variables are allowed for updation\n Error at line: " + to_string( varsAndVals[x].row )
					);

				auto& vmapvariable = get<unique_ptr<VARIABLE_HOLDER>>( mapData->var );
				
				DEEP_VALUE_DATA topValue = finalValueQueue.front();
				finalValueQueue.pop();

				if( vmapvariable->isTypeArray ){
					auto& arr = get<unique_ptr<ArrayList>>( vmapvariable->value );
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
				if( this->getFromVmap( funcTokens.funcName ) != nullptr )
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

			MapItem* func = prntClass->getFromVmap( tokens[ currentPtr ].token );
			if( func && func->mapType == MAPTYPE::FUNCTION ){
				prntClass->handleFunctionCall( func, tokens, currentPtr );
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