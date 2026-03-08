#include "FuncBody.hpp"

template <typename PARENT_CLASS>
optional<variant<VarDtype, unique_ptr<MapItem>>>
ProgramExecutor( const vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, PARENT_CLASS* prntClass, size_t endPtr ){
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