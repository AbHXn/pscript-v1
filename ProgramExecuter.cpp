#include "Components/Headers/FunctionHandler.hpp"
#include "Components/Headers/ValueHelper.hpp"
#include "Components/Headers/BodyEncounter.hpp"

using namespace std;

optional<variant<VarDtype, shared_ptr<MapItem>>>
ProgramExecutor( const vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, FunctionHandler* prntClass, size_t endPtr ){
	size_t backUpPtr = currentPtr;

	while( currentPtr < tokens.size() ){
		backUpPtr = currentPtr;
		
		const Token& curToken = tokens[currentPtr];

		if( endPtr && currentPtr >= endPtr - 1 )
			return nullopt;

		if( curToken.token == "pidi" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				string key = prntClass->ctx->filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				prntClass->VarHandlerRunner( tokens, currentPtr, key );
			}
			catch( ... ){
				cout << "pidi Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
		}
		else if( curToken.token == "nok" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				string key = prntClass->ctx->filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				FunctionHandler newLpHander( prntClass, prntClass->runnerBody, prntClass->ctx );
				newLpHander.CondHandlerRunner( tokens, currentPtr, key );
			}
			catch( const RecoverError& err ){
				continue;
			}
			catch( ... ){
				cout << "nok Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
		}
		else if( curToken.token == "}" ){
			if( prntClass->parent == nullptr )
				throw InvalidSyntaxError( "unknown token }" );
			return nullopt;
		}
		else if( curToken.token == "para" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				string key = prntClass->ctx->filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				prntClass->IOHandlerRunner( tokens, currentPtr, key);
			}
			catch( ... ){
				cout << "para Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
		}
		else if( curToken.token == "pindi" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				prntClass->functionDefHandlerRunner( tokens, currentPtr );
				currentPtr--;
			} 
			catch( const RecoverError& err ){
				continue;
			}
			catch( ... ){
				cout << "Pindi Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
		}
		else if( curToken.token == "ittuthiri" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				string key = prntClass->ctx->filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				FunctionHandler newLpHander( prntClass, prntClass->runnerBody, prntClass->ctx );
				newLpHander.LoopHandlerRunner( tokens, currentPtr, key );
				currentPtr--;
			}
			catch( const RecoverError& err ){
				continue;
			}
			catch( ... ){
				cout << "ittuthiri Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
		}
		else{
			BodyEncounters bodyEncounter( prntClass );
			if( C_CLASS == CALLER::FUNCTION && curToken.token == "poda" && curToken.type == TOKEN_TYPE::RESERVED ){
				try{
					return bodyEncounter.getReturnedData( tokens, currentPtr );
				} 
				catch( const RecoverError& err ){
					currentPtr = backUpPtr;
					prntClass->ctx->LastRecoverErrorList = tokens[currentPtr].row;
					throw err;			
				}
				catch( ... ){
					cout << "poda statement " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
					throw;
				}
			}

			auto func = prntClass->getFromVmap( tokens[ currentPtr ].token );
			if( func && func->mapType == MAPTYPE::FUNC_PTR ){
				try{
					string key = prntClass->ctx->filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
					bodyEncounter.handleFunctionCall( func, tokens, currentPtr, key );
				}
				catch(...){
					cout << "Pindi call Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
					throw;
				}
			}
			else{
				try{
					backUpPtr = currentPtr;
					string key = prntClass->ctx->filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
					prntClass->InstructionHandlerRunner( tokens, currentPtr, key);
				}
				catch( const RecoverError& err ){
					currentPtr = backUpPtr;
					prntClass->ctx->LastRecoverErrorList = tokens[currentPtr].row;
					throw err;			
				}
				catch( ... ){
					cout << "Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
					throw;
				}
			}
		}
		currentPtr++;
	}
	return nullopt;
}


int main( int argc, char *argv[] ){
	if( argc != 2 )
		throw runtime_error("./pscript <filename>.ps");

	std::shared_ptr<Context> cContext = std::make_shared<Context>();

	cContext->filename = argv[1];
	if (cContext->filename.size() <= 3)
		throw runtime_error("Invalid file format");

	if( cContext->filename.substr(cContext->filename.size() - 3) != ".ps" )
		throw runtime_error("Invalid file format");


	getTheTokens( cContext->filename, cContext->fullTokens );

	FunctionHandler func(cContext);
	func.functionName = "MAIN";
	try{
		ProgramExecutor( func.ctx->fullTokens, func.ctx->pointer, CALLER::FUNCTION, &func );
	}
	catch( InvalidSyntaxError& err ){
		cout << err.what() << endl;
	}
	catch( const RecoverError& err ){
		throw runtime_error("Some Error occured at line: " + to_string(func.ctx->LastRecoverErrorList + 1));
	}
	return 0;
}