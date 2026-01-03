#include "ProgramBody.hpp"

class LoopRunner;

vector<string> fullTokens;
size_t pointer = 0;

enum class CALLER{
	LOOP 		,
	FUNCTION 	,
	CONDITIONAL 
};

template <typename T>
optional<VALUE_DATA>
ProgramExecutor( const vector<string>& tokens, size_t& currentPtr, CALLER C_CLASS, T* prntClass, size_t endPtr = 0 );

class FunctionRunner: public ProgramBody {
	public:
		string functionName;

		void IOHandlerRunner( const vector<string>& tokens, size_t& start ){
			auto tokensAndData = stringToIoTokens( tokens, start );
			
			if( !isValidIoTokens( tokensAndData.first ) )
				return;

			queue<VarDtype> finalQueue;
			
			for( vector<string>&valToks: tokensAndData.second ){
				auto astTokenAndData = stringToASTTokens( valToks );
				queue<VALUE_DATA> ValueQueue;

				for( string& oData: astTokenAndData.second )
					handleUnknownToken( ValueQueue, oData );

				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, ValueQueue, startAST );
				
				if( !newAstNode.has_value() )
					return;

				VarDtype asEval = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
				finalQueue.push( asEval );
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

						VarDtype top = finalQueue.front();
						finalQueue.pop();
						ValueHelper::printVarDtype( top );
						break;
					}
					case IO_TOKENS::CONCAT:{
						cout << " ";
						if( finalQueue.empty() )
							throw InvalidSyntaxError("In koode statement");

						VarDtype top = finalQueue.front();
						finalQueue.pop();
						ValueHelper::printVarDtype( top );
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

		void
		InstructionHandlerRunner( const vector<string>& tokens, size_t& currentPtr ){
			auto InsTokensAndData = stringToInsToken( tokens, currentPtr );

			if( InsTokensAndData.second.first.size() != InsTokensAndData.second.second.size() )
				throw InvalidSyntaxError( "Invalid Instruction" );

			if( !isValidInstructionSet( InsTokensAndData.first.first ) )
				throw InvalidSyntaxError( "Invalid Instruction set" );

			queue<VarDtype> finalValueQueue;
			auto varsAndVals = InsTokensAndData.second;

			queue<MapItem*> mapVariables;
			for( string& var: varsAndVals.first ){

				MapItem* mapVar = this->getFromVmap( var );
				
				if( mapVar == nullptr )
					throw InvalidSyntaxError( "No variable named " + var );

				mapVariables.push( mapVar );
			}

			for( vector<string>& astStrToks: varsAndVals.second ){
				auto astTokenAndData = stringToASTTokens( astStrToks );
				queue<VALUE_DATA> valueQeue;

				for(string& vdata: astTokenAndData.second)
					handleUnknownToken<VALUE_DATA>( valueQeue, vdata );
				
				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, valueQeue, startAST );
				
				if( !newAstNode.has_value() )
					throw InvalidSyntaxError("Invalid expressions");

				VarDtype finalValue = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
				finalValueQueue.push( finalValue );
			}

			while( !mapVariables.empty() ){
				MapItem* var = mapVariables.front();
				VarDtype value = finalValueQueue.front();

				mapVariables.pop();
				finalValueQueue.pop();

				unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>> newData = make_unique<
							SingleElement<VARIABLE_HOLDER_DATA>>();
				
				/* 
					// IMPLEMENT +=, -= short keys here
				*/

				newData->assignValue(value);
				var->assignSingleValue( move( newData ) );
			}
		}

		void 
		functionDefHandlerRunner( const vector<string>&token, size_t& start ){
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

		pair<queue<VALUE_DATA>, vector<string>>
		vectorResolver( const vector<string>& tokens ){
			queue<VALUE_DATA> resolvedVector;
			vector<string> simpleVector;

			for( size_t x = 0; x < tokens.size(); x++ ){
				string curToken = tokens[ x ];

				if( isRegisteredASTExprTokens(curToken) ){
					simpleVector.push_back( curToken );
					continue;
				}
				if( curToken == ")" ||  curToken == "(" ){
					simpleVector.push_back( curToken );
					continue;
				}

				try{
					// check if it is a temp value
					auto typeValue = DtypeHelper::getTypeAndValue( curToken );
					simpleVector.push_back("NUM");
					resolvedVector.push( typeValue.second );
				}
				catch( const InvalidDTypeError& err ){
					// check if it is defined
					auto VMAPData = this->getFromVmap( curToken );

					if( VMAPData != nullptr ){
						// if it is variable then get its value
						if( VMAPData->mapType == MAPTYPE::VARIABLE ){
							VarDtype copyData = ValueHelper::getValueFromMapItem( VMAPData );
							resolvedVector.push( copyData );
							simpleVector.push_back("VAR");
						}
						// if it is a function then call it and get the value
						else if( VMAPData->mapType == MAPTYPE::FUNCTION ){
							try{
								FunctionCallReturns pt = stringToFunctionCallTokens( tokens, x );
								// if it is zero args function 
								if( pt.argsVector.size() == 0){
									auto data = this->zeroArgFunction( VMAPData, fullTokens, pt.funcName );
									if( data.has_value() ){
										simpleVector.push_back("FUNC");
										resolvedVector.push( move( data.value() ) );
									}
								}
								else{
									auto data = this->handleFunctionCall( VMAPData, fullTokens, x, pt);
									if( data.has_value() ){
										simpleVector.push_back("FUNC_MUL_ARGS");
										resolvedVector.push( move( data.value() ) );
									}
								}
								x--; // stringfuncalltokens it hits then unknown token get that token back
							}
							catch ( InvalidSyntaxError& err ){
								cout << err.what() << endl;
							}
						}
					}
					// if everything failed then its unknown token
					else throw InvalidSyntaxError( "Unknown Token " + curToken );
				}
			}
			return { move(resolvedVector), simpleVector };
		}

		optional<VALUE_DATA>	
		zeroArgFunction( MapItem* func, const vector<string>& tokens, string funcName ){
			auto& funcFromMap = get<unique_ptr<FUNCTION_MAP_DATA>>( func->var );

			size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
			size_t funcEndStartPtr = funcFromMap->bodyEndPtr;

			unique_ptr<FunctionRunner> newFuncRunner = make_unique<FunctionRunner>();
			newFuncRunner->functionName = funcName;
			
			if( funcName == this->functionName )
				newFuncRunner->parent = this->parent;
			else newFuncRunner->parent = this;

			return ProgramExecutor( tokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr );
			
		}

		optional<VALUE_DATA>	
		handleFunctionCall( MapItem* func, const vector<string>& tokens, size_t& currentPtr, optional<FunctionCallReturns> data = nullopt ){
			FunctionCallReturns Data;
			if( data.has_value() )
				Data = data.value();
			else Data = stringToFunctionCallTokens( tokens, currentPtr );

			if( isValidFuncCall( Data.callTokens ) ){
				if( Data.argsVector.size() == 0 )
					return zeroArgFunction( func, tokens, Data.funcName );

				unique_ptr<FunctionRunner> newFuncRunner = make_unique<FunctionRunner>();
				newFuncRunner->functionName = Data.funcName;
				
				if( Data.funcName == this->functionName )
					newFuncRunner->parent = this->parent;
				else newFuncRunner->parent = this;

				auto& funcFromMap = get<unique_ptr<FUNCTION_MAP_DATA>>( func->var );

				vector<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>> vars;

				for( auto& var: funcFromMap->argsInfo ){
					unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>> data = make_unique<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>();
					data->key = var->name;
					data->isTypeArray = var->isArray;
					vars.push_back( move( data ) );
				}


				vector<string> lhsTokens = {"="};
				queue<VARIABLE_HOLDER_DATA> finalValueQueue;

				int total_comma = Data.argsVector.size() - 1;

				for( auto argSingleVec: Data.argsVector ){
					auto [ resValQueue, resToken ] = vectorResolver( argSingleVec );

					auto astTokenAndData = stringToASTTokens( resToken );

					size_t startAST = 0;
					auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, resValQueue, startAST );

					if( !newAstNode.has_value() )
						return nullopt;

					finalValueQueue.push( move( newAstNode.value() ) );

					for( string t: argSingleVec )
						lhsTokens.push_back( t );

					if( total_comma )
						lhsTokens.push_back( "," );
					total_comma--;
				}
				lhsTokens.push_back(";");

				size_t start = 0;
				auto test = codeToTokens( lhsTokens, start );
				
				if( isValidValueSyntax( test.first.second, vars, finalValueQueue ) ){
					for( auto& Variable: vars ){
						string& key  = Variable->key;
						auto newMapVar = make_unique<MapItem>( );
						newMapVar->var = move( Variable );
						newFuncRunner->addToMap( key, move( newMapVar ) );
					}
				}
				size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
				size_t funcEndStartPtr = funcFromMap->bodyEndPtr;
				
				return ProgramExecutor<FunctionRunner>( fullTokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr );
			}
			throw InvalidSyntaxError("Invalid function call");
		}

		void VarHandlerRunner( const vector<string>& test, size_t& start ){
			auto tokens  = codeToTokens( test, start ); // convert variable to tokens ( leaves entire value parts )
			auto toks 	 = tokens.first;  				// variable tokens and value tokens
			auto names 	 = tokens.second; 				// values

			vector<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>> vars;

			// check if it is valid variable syntax by DFS ing the GRAPH
			if( isValidVariableSyntax<VARIABLE_HOLDER_DATA>( toks.first, vars, names.first ) ){
				// check if variable alread exists or created				
				for( auto& varVerification: vars ){
					auto data = this->getFromVmap( varVerification->key );	
					if( data ) throw VariableAlreayExists( varVerification->key );
				} 
				
				auto valueVectors = names.second;
				
				queue<VARIABLE_HOLDER_DATA> finalValueQueue;

				for( auto& VarientData: valueVectors ){
					// if it holds vector string then it may be an operations or function calls
					// or may be array calls
					if( holds_alternative< vector<string> > ( VarientData ) ){
						auto data = get<vector<string>> ( VarientData );

						// resolve the value to final stage, if function call return value etc..
						auto [ resValQueue, resToken ] = vectorResolver( data );

						queue<VALUE_DATA> refinedQueue; 

 						while( !resValQueue.empty() ){
							auto top = move( resValQueue.front() );

							resValQueue.pop();

							if( holds_alternative<unique_ptr<MapItem>>( top ) ){
								unique_ptr<MapItem> item = move( get<unique_ptr<MapItem>>( top ) );

								if( resToken.size() > 1 )
									throw InvalidSyntaxError("In pidi expression, token is not a variable! ");
								
								else if( vars.size() > 2 )
									throw InvalidSyntaxError("No enough value to unpack\n");

								if( vars.size() == 1 ){
									auto onlyVariable = move( vars.back() );
									string VarName = onlyVariable->key;
									this->addToMap( VarName, move( item ) );
									return;
								}
							}
							refinedQueue.push( move( top ) );
						}

						auto astTokenAndData = stringToASTTokens( resToken );

						size_t startAST = 0;
						auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, refinedQueue, startAST );

						if( !newAstNode.has_value() ){
							return;
						}

						finalValueQueue.push( move( newAstNode.value() ) );
					}
					else{
						auto vdata = get<string>( VarientData );
						handleUnknownToken<VARIABLE_HOLDER_DATA>( finalValueQueue, vdata );
					}
				}

				bool isValValue = isValidValueSyntax<VARIABLE_HOLDER_DATA>( toks.second, vars, finalValueQueue );

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

		optional<VALUE_DATA>
		getReturnedData( const vector<string>&tokens, size_t& currentPtr ){
			currentPtr++;
			vector<string> returnStatementData;

			while( currentPtr < tokens.size() ){
				string curToken = tokens[ currentPtr ];
				if( curToken == ";" ) break;
				
				returnStatementData.push_back( curToken );
				currentPtr++;
			}

			if( returnStatementData.empty() )
				return nullopt;

			if( returnStatementData.size() == 1 && DtypeHelper::isValidVariableName( returnStatementData.back() ) ){
				auto mapData = this->moveFromVmap( returnStatementData.back() );

				if( mapData.has_value() )
					return VALUE_DATA{ move( mapData.value() ) };
				
				throw InvalidSyntaxError("No Variable named " + returnStatementData.back());
			}

			auto resolvedVector = vectorResolver( returnStatementData );
			auto asTokens = stringToASTTokens( resolvedVector.second );
			
			size_t startAST = 0;
			auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( asTokens.first, resolvedVector.first, startAST );	

			if( newAstNode.has_value() ){
				VarDtype asEval = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
				return VALUE_DATA{ asEval };
			}
			throw InvalidSyntaxError( "Invalid Poda statement" );
		}
};

class Conditional: public FunctionRunner{
	public:
		unique_ptr<LoopRunner> lpRunner;

		Conditional( unique_ptr<LoopRunner> lpRunner ){
			this->lpRunner = move( lpRunner );
		}

		void CondHandlerRunner( const vector<string>& test, size_t& start ){
			size_t endOfNOK = 0;
			auto tokens = stringToCondTokens( test, start, endOfNOK );
			
			if( !isValidCondToken( tokens.first ) )
				return;

			for(int x = 0; x < tokens.second.size(); x++){
				auto data = tokens.second[ x ];
				
				auto astTokenAndData = stringToASTTokens( data.first );
				queue<VALUE_DATA> ValueQueue;
				
				for( string& tok: astTokenAndData.second ){
					try{
						this->handleUnknownToken<VALUE_DATA>( ValueQueue, tok );
					} catch(InvalidSyntaxError& err){
						cout << err.what() << endl;
					}
				}

				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, ValueQueue, startAST );
					
				if( !newAstNode.has_value() )
					throw InvalidSyntaxError("Invalid conditional expr in nok");

				VarDtype asEval = ValueHelper::evaluate_AST_NODE( newAstNode.value() );

				if( get<bool>(asEval) ){
					start = data.second + 1;
					ProgramExecutor( test, start, CALLER::CONDITIONAL, this );
					start = endOfNOK;
					return;
				}

			}
		}


};

class LoopRunner: public FunctionRunner{
	public:		
		void 
		LoopHandlerRunner ( const vector<string>& tokens, size_t& currentPtr ){
			size_t beginCopy = currentPtr;
			auto [ tokInfo, bodyIns ] = stringToLoopTokens( tokens, currentPtr );
			
			if( !isValidLoopTokens( tokInfo.first ) )
				throw InvalidSyntaxError("Invalid Syntax error in loop");

			while( true ){
				auto astTokenAndData = stringToASTTokens( tokInfo.second );
				queue<VALUE_DATA> valueQeue;

				for(string& vdata: astTokenAndData.second)
					handleUnknownToken<VALUE_DATA>( valueQeue, vdata );
				
				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, valueQeue, startAST );

				if( !newAstNode.has_value() )
					throw InvalidSyntaxError("Invalid expressions in loops");

				VarDtype finalValue = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
				if( !holds_alternative<bool>(finalValue) )
					throw InvalidSyntaxError( "loop condition should be boolean or blank" );

				bool runTheBodyAgain = get<bool>( finalValue );

				if( runTheBodyAgain ){
					size_t bodyStart = bodyIns.first;
					try{
						ProgramExecutor( tokens, bodyStart, CALLER::LOOP, this );
						currentPtr = beginCopy;
					} 
					catch( const InvalidSyntaxError& err ){
						const string& expTok = tokens[ bodyStart ];
						if( expTok == "theku" )
							break;

						else if( expTok == "pinnava" )
							currentPtr = beginCopy;
						
						else {
							currentPtr = bodyStart;
							throw err;
						}
					}
					return;
				}
				else break;
				
			}
			currentPtr = bodyIns.second;
		}

};

template <typename PARENT_CLASS>
optional<VALUE_DATA>
ProgramExecutor( const vector<string>& tokens, 
				 size_t& currentPtr, 
				 CALLER C_CLASS, 
				 PARENT_CLASS* prntClass, 
				 size_t endPtr
){
	size_t backUpPtr = currentPtr;

	while( currentPtr < tokens.size() ){

		if( endPtr && currentPtr >= endPtr - 1 )
			return nullopt;

		if( tokens[ currentPtr ] == "pidi" ){
			prntClass->VarHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[currentPtr] == "nok" ){
			try{
				unique_ptr<LoopRunner> lpHandler = make_unique<LoopRunner>();
				shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );
				cdHandler->parent = prntClass;
				cdHandler->CondHandlerRunner( tokens, currentPtr );
			}
			catch( const InvalidSyntaxError& err ){
				continue;
			}
		}
		else if( tokens[ currentPtr ] == "}" ){
			if( prntClass->parent == nullptr )
				throw InvalidSyntaxError( "unknown token }" );
			return nullopt;
		}
		else if( tokens[ currentPtr ] == "para" ){
			prntClass->IOHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[ currentPtr ] == "thenga" ){
			prntClass->functionDefHandlerRunner( tokens, currentPtr );
			currentPtr--;
		}
		else if( tokens[ currentPtr ] == "ittuthiri" ){
			try{
				unique_ptr<LoopRunner> newLpHander = make_unique<LoopRunner>();
				newLpHander->parent = prntClass;
				newLpHander->LoopHandlerRunner( tokens, currentPtr );
				currentPtr--;
			}catch( const InvalidSyntaxError& err ){
				continue;
			}
		}
		else{
			switch( C_CLASS ){
				case CALLER::FUNCTION: {
					if( tokens[ currentPtr ] == "poda" )
						return prntClass->getReturnedData( tokens, currentPtr );
					}
				default: break;
			}

			MapItem* func = prntClass->getFromVmap( tokens[ currentPtr ] );
			if( func && func->mapType == MAPTYPE::FUNCTION ){
				prntClass->handleFunctionCall( func, tokens, currentPtr );
			}
			else{
				try{
					backUpPtr = currentPtr;
					prntClass->InstructionHandlerRunner( tokens, currentPtr );
				}
				catch(...){
					currentPtr = backUpPtr;
					throw InvalidSyntaxError("Encounter error in function");
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
		fullTokens = parseTheCodeToTokens( filename );

		size_t start = 0;
		unique_ptr<LoopRunner> lpHandler = make_unique<LoopRunner>();
		shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );

		unique_ptr<FunctionRunner> func = make_unique<FunctionRunner>( );
		func->functionName = "MAIN";
		auto dd = move(ProgramExecutor( fullTokens, pointer, CALLER::FUNCTION, func.get() ));
	}
	catch( InvalidSyntaxError& err ){
		cout << err.what() << endl;
	}
	return 0;
}