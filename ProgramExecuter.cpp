#include "Components/FuncBody.hpp"

using namespace std;

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
			try{
				prntClass->VarHandlerRunner( tokens, currentPtr );
			}
			catch( ... ){
				cout << "Line " + to_string( tokens[backUpPtr].row ) << ": ";
				throw;
			}
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
			catch( ... ){
				cout << "Line " + to_string( tokens[backUpPtr].row ) << ": ";
				throw;
			}
		}
		else if( curToken == "}" ){
			if( prntClass->parent == nullptr )
				throw InvalidSyntaxError( "unknown token }" );
			return nullopt;
		}
		else if( curToken == "para" ){
			try{
				prntClass->IOHandlerRunner( tokens, currentPtr );
			}
			catch( ... ){
				cout << "Line " + to_string( tokens[backUpPtr].row ) << ": ";
				throw;
			}
		}
		else if( curToken == "pari" ){
			try{
				prntClass->functionDefHandlerRunner( tokens, currentPtr );
				currentPtr--;
			} 
			catch( const RecoverError& err ){
				continue;
			}
			catch( ... ){
				cout << "Line " + to_string( tokens[backUpPtr].row ) << ": ";
				throw;
			}
			
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
			catch( ... ){
				cout << "Line " + to_string( tokens[backUpPtr].row ) << ": ";
				throw;
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
				catch( const RecoverError& err ){
					currentPtr = backUpPtr;
					throw err;			
				}
				catch( ... ){
					cout << "Line " + to_string( tokens[backUpPtr].row ) << ": ";
					throw;
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
		if (filename.size() <= 3)
			throw runtime_error("Invalid file format");

		if( filename.substr(filename.size() - 3) != ".ps" )
			throw runtime_error("Invalid file format");

		getTheTokens( filename, fullTokens );

		unique_ptr<LoopHandler> lpHandler = make_unique<LoopHandler>();
		shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );

		unique_ptr<FunctionHandler> func = make_unique<FunctionHandler>( );
		func->functionName = "MAIN";
		auto dd = move(ProgramExecutor( fullTokens, pointer, CALLER::FUNCTION, func.get() ));
	}
	catch( InvalidSyntaxError& err ){
		cout << err.what() << endl;
	}
	catch( const RecoverError& err ){
		throw runtime_error("Error occured at line: " + to_string( pointer ));
	}
	return 0;
}