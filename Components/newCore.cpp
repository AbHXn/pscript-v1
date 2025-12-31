#include "ProgramBody.hpp"

class LoopRunner;

vector<string> fullTokens;
size_t pointer = 0;

class FunctionRunner: public ProgramBody {
	public:

		optional<VALUE_DATA>
		handleBody( const vector<string>& tokens, size_t& currentPtr, size_t endPtr );

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

			else throw InvalidSyntaxError( "Syntax Error occured in pindi" );

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
					auto typeValue = DtypeHelper::getTypeAndValue( curToken );
					simpleVector.push_back("NUM");
					resolvedVector.push( typeValue.second );
				}
				catch( const InvalidDTypeError& err ){
					auto VMAPData = this->getFromVmap( curToken );

					if( VMAPData != nullptr ){
						if( VMAPData->mapType == MAPTYPE::VARIABLE ){
							VarDtype copyData = ValueHelper::getValueFromMapItem( VMAPData );
							resolvedVector.push( copyData );
							simpleVector.push_back("VAR");
						}
						else if( VMAPData->mapType == MAPTYPE::FUNCTION ){
							try{
								FunctionCallReturns pt = stringToFunctionCallTokens( tokens, x );

								if( pt.argsVector.size() == 0){
									auto data = this->runVoidFunction( VMAPData, fullTokens );
								
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
							}
							catch ( InvalidSyntaxError& err ){
								cout << err.what() << endl;
							}
						}
					}
					throw err;
				}

			}

			return { move(resolvedVector), simpleVector };
		}

		optional<VALUE_DATA>	
		runVoidFunction( MapItem* func, const vector<string>& tokens ){
			auto& funcFromMap = get<unique_ptr<FUNCTION_MAP_DATA>>( func->var );

			size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
			size_t funcEndStartPtr = funcFromMap->bodyEndPtr;

			unique_ptr<FunctionRunner> newFuncRunner = make_unique<FunctionRunner>();
			newFuncRunner->parent = this;
			return newFuncRunner->handleBody( tokens, funcBodyStartPtr, funcEndStartPtr );
			
		}

		optional<VALUE_DATA>	
		handleFunctionCall( MapItem* func, const vector<string>& tokens, size_t& currentPtr, optional<FunctionCallReturns> data = nullopt ){
			FunctionCallReturns Data = stringToFunctionCallTokens( tokens, currentPtr );
			if( data.has_value() )
				Data = data.value();

			if( isValidFuncCall( Data.callTokens ) ){
				if( Data.argsVector.size() == 0 )
					return runVoidFunction( func, tokens );

				unique_ptr<FunctionRunner> newFuncRunner = make_unique<FunctionRunner>();
				newFuncRunner->parent = this;

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
				
				return newFuncRunner->handleBody( fullTokens, funcBodyStartPtr, funcEndStartPtr );
			}
			throw InvalidSyntaxError("Invalid function call");
		}

		void VarHandlerRunner( const vector<string>& test, size_t& start ){
			auto tokens  = codeToTokens( test, start );
			auto toks 	 = tokens.first;
			auto names 	 = tokens.second;

			vector<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>> vars;

			bool isValidVar = isValidVariableSyntax<VARIABLE_HOLDER_DATA>( toks.first, vars, names.first );

			for( auto& varVerification: vars ){
				auto data = this->getFromVmap( varVerification->key );
				if( data != nullptr )
					throw VariableAlreayExists( varVerification->key );
			} 

			if( isValidVar ){
				auto valueVectors = names.second;

				queue<VARIABLE_HOLDER_DATA> finalValueQueue;

				for( auto& VarientData: valueVectors ){
					if( holds_alternative< vector<string> > ( VarientData ) ){
						auto data = get<vector<string>> ( VarientData );

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

		void handleBody( const vector<string>& tokens, size_t& currentPtr );

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
					this->handleBody( test, start );
					start = endOfNOK;
					return;
				}

			}
		}


};

class LoopRunner: public FunctionRunner{
	public:		
		void handleBody( const vector<string>& tokens, size_t& currentPtr );
		
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
						this->handleBody( tokens, bodyStart );
						currentPtr = beginCopy;
					} 
					catch( const InvalidSyntaxError& err ){
						const string& expTok = tokens[ bodyStart - 1 ];
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


void LoopRunner::handleBody( const vector<string>& tokens, size_t& currentPtr ){
	while( currentPtr < tokens.size() ){
		if( tokens[ currentPtr ] == "pidi" ){
			this->VarHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[currentPtr] == "nok" ){
			unique_ptr<LoopRunner> lpHandler = make_unique<LoopRunner>();
			shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );
			cdHandler->parent = this;
			cdHandler->CondHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[ currentPtr ] == "}" ){
			if( this->parent == nullptr )
				throw InvalidSyntaxError( "unknown token }" );
			return ;
		}
		else if( tokens[ currentPtr ] == "para" ){
			this->IOHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[ currentPtr ] == "ittuthiri" ){
			unique_ptr<LoopRunner> newLpHander = make_unique<LoopRunner>();
			newLpHander->parent = this;
			newLpHander->LoopHandlerRunner( tokens, currentPtr );
			currentPtr--;
		}
		else{
			this->InstructionHandlerRunner( tokens, currentPtr );
		}
		currentPtr++;
	}
}

void Conditional::handleBody( const vector<string>& tokens, size_t& currentPtr ){
	while( currentPtr < tokens.size() ){
		if( tokens[ currentPtr ] == "pidi" ){
			this->VarHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[currentPtr] == "nok" ){
			try{
				unique_ptr<LoopRunner> lpHandler = make_unique<LoopRunner>();
				shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );
				cdHandler->parent = this;
				cdHandler->CondHandlerRunner( tokens, currentPtr );
			}
			catch( const InvalidSyntaxError& err ){
				throw InvalidSyntaxError("Invalid Syntax In conditional");
			}
		}
		else if( tokens[ currentPtr ] == "}" ){
			if( this->parent == nullptr )
				throw InvalidSyntaxError( "unknown token }" );
			return ;
		}
		else if( tokens[ currentPtr ] == "para" ){
			this->IOHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[ currentPtr ] == "ittuthiri" ){
			unique_ptr<LoopRunner> newLpHander = make_unique<LoopRunner>();
			newLpHander->parent = this;
			newLpHander->LoopHandlerRunner( tokens, currentPtr );
			currentPtr--;
		}
		else{
			this->InstructionHandlerRunner( tokens, currentPtr );
		}
		currentPtr++;
	}
}


optional<VALUE_DATA>
FunctionRunner::handleBody( 
								const vector<string>& tokens, 
								size_t& currentPtr, 
								size_t endPtr = 0
){
	while( currentPtr < tokens.size() ){
		if( endPtr && currentPtr >= endPtr - 1 )
			return nullopt;

		if( tokens[ currentPtr ] == "pidi" ){
			this->VarHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[currentPtr] == "nok" ){
			try{
				unique_ptr<LoopRunner> lpHandler = make_unique<LoopRunner>();
				shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );
				cdHandler->parent = this;
				cdHandler->CondHandlerRunner( tokens, currentPtr );
			}
			catch( const InvalidSyntaxError& err ){
				// function keywords;
			}
		}
		else if( tokens[ currentPtr ] == "}" ){
			if( this->parent == nullptr )
				throw InvalidSyntaxError( "unknown token }" );
			return nullopt;
		}
		else if( tokens[ currentPtr ] == "para" ){
			this->IOHandlerRunner( tokens, currentPtr );
		}
		else if( tokens[ currentPtr ] == "poda" ){
			return this->getReturnedData( tokens, currentPtr );
		}
		else if( tokens[ currentPtr ] == "pindi" ){
			this->functionDefHandlerRunner( tokens, currentPtr );
			currentPtr--;
		}
		else if( tokens[ currentPtr ] == "ittuthiri" ){
			unique_ptr<LoopRunner> newLpHander = make_unique<LoopRunner>();
			newLpHander->parent = this;
			newLpHander->LoopHandlerRunner( tokens, currentPtr );
			currentPtr--;
		}
		else{
			MapItem* func = this->getFromVmap( tokens[ currentPtr ] );
			if( func && func->mapType == MAPTYPE::FUNCTION ){
				this->handleFunctionCall( func, tokens, currentPtr );
			}
			else this->InstructionHandlerRunner( tokens, currentPtr );
			
		}
		currentPtr++;
	}
	return nullopt;
}

int main(  ){
	try{
		string filename = "test.mb";
		fullTokens = parseTheCodeToTokens( filename );

		size_t start = 0;
		unique_ptr<LoopRunner> lpHandler = make_unique<LoopRunner>();
		shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );

		unique_ptr<FunctionRunner> func = make_unique<FunctionRunner>( );
		auto dd = move(func->handleBody( fullTokens, pointer ));
	}
	catch( InvalidSyntaxError& err ){
		cout << err.what() << endl;
	}
	return 0;
}