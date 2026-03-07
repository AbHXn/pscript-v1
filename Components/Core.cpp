#include "FuncBody.hpp"

class Conditional: public FunctionHandler{
	public:
		unique_ptr<LoopHandler> lpRunner;
		Conditional( unique_ptr<LoopHandler> lpRunner ){
			this->lpRunner = move( lpRunner );
		}
		void CondHandlerRunner( const vector<Token>& tokens, size_t& start ){
			size_t endOfNOK = 0;
			auto ctokens = stringToCondTokens( tokens, start, endOfNOK );
			isValidCondToken( ctokens.first );

			bool runTheCondition = false;

			for(int x = 0; x < ctokens.second.size(); x++){
				auto data = ctokens.second[ x ];
				DEEP_VALUE_DATA evRes = evaluateVector( data.first );

				if( holds_alternative<VarDtype>( evRes ) ){
					auto VdtypData = get<VarDtype>( evRes );

					if( holds_alternative<bool>( VdtypData ) ){
						runTheCondition = true;
						if( get<bool>( VdtypData ) ){
							start = data.second + 1;
							runTheCondition = true;
							
							try{
								ProgramExecutor( tokens, start, CALLER::CONDITIONAL, this );
								start = endOfNOK;
								return;
							} catch( const InvalidSyntaxError& err ){
								cout << err.what() << endl;
							}
						}
					}
				}
			}
			if( !runTheCondition ) 
				throw InvalidSyntaxError("Conditional Expression expects boolean values");
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
				DEEP_VALUE_DATA finalValue = evaluateVector( lpTokens.conditions );
				if( !holds_alternative<VarDtype>(finalValue) )
					throw InvalidSyntaxError( "loop condition should be boolean or blank" );

				VarDtype cdData = get<VarDtype>(finalValue);

				if( !holds_alternative<bool>(cdData) )
					throw InvalidSyntaxError( "loop condition should be boolean or blank" );

				bool runTheBodyAgain = get<bool>( cdData );

				if( runTheBodyAgain ){
					size_t bodyStart = bodyIns.first;
					try{
						ProgramExecutor( tokens, bodyStart, CALLER::LOOP, this );
						currentPtr = beginCopy;
					} 
					catch( const RecoverError& err ){
						currentPtr = bodyStart;
						const string& expTok = tokens[ bodyStart ].token;
						if( expTok == "theku" )
							break;
						else if( expTok == "pinnava" )
							currentPtr = beginCopy;
						else throw err;
					}
					catch( const InvalidSyntaxError& err ){
						cout << err.what() << endl;
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
			catch( const RecoverError& err ){
				continue;
			}
			catch( const InvalidSyntaxError& err ){
				cout << err.what() << endl;
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
			catch( const RecoverError& err ){
				continue;
			}
		}
		else{
			switch( C_CLASS ){
				case CALLER::FUNCTION: {
					if( curToken == "poda" ){
						return prntClass->getReturnedData( tokens, currentPtr );
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
					throw RecoverError();
					// throw InvalidSyntaxError(
					// 		"Encounter error in function at line:  " + 
					// 			to_string( tokens[currentPtr].row + 1) + " col " +  to_string( tokens[ currentPtr ].col )
					// 		);
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