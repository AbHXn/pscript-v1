#include <unistd.h>
#include <cstdio>
#include "Components/Headers/FunctionHandler.hpp"
#include "Components/Headers/ValueHelper.hpp"
#include "Components/Headers/BodyEncounter.hpp"

using namespace std;

FUNC_RETURN_TYPE
ProgramExecutor( const vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, FunctionHandler* prntClass, size_t endPtr ){
	Context* ctx = prntClass->ctx;

	while( currentPtr < tokens.size() ){
		if( endPtr && currentPtr >= endPtr - 1 )
			break;

		const Token& curToken = tokens[ currentPtr ];
		const size_t at = currentPtr;
		const char* label = "Line ";	// printed in front of the line number when a statement fails

		try{
			switch( curToken.type == TOKEN_TYPE::RESERVED ? curToken.tokConst : TOKEN_CONST::NOTHING ){
				case TOKEN_CONST::PIDI:
					label = "pidi Line ";
					prntClass->VarHandlerRunner( tokens, currentPtr, curToken.tokenId );
					break;

				case TOKEN_CONST::NOK: {
					label = "nok Line ";
					FunctionHandler scope( prntClass, prntClass->runnerBody, ctx );
					scope.CondHandlerRunner( tokens, currentPtr, curToken.tokenId );
					break;
				}
				case TOKEN_CONST::ITTUTHIRI: {
					label = "ittuthiri Line ";
					FunctionHandler scope( prntClass, prntClass->runnerBody, ctx );
					scope.LoopHandlerRunner( tokens, currentPtr, curToken.tokenId );
					currentPtr--;
					break;
				}
				case TOKEN_CONST::PARA:
					label = "para Line ";
					prntClass->IOHandlerRunner( tokens, currentPtr, curToken.tokenId );
					break;

				case TOKEN_CONST::PINDI:
					label = "Pindi Line ";
					prntClass->functionDefHandlerRunner( tokens, currentPtr );
					currentPtr--;
					break;

				case TOKEN_CONST::PODA:
					label = "poda statement ";
					ctx->ret = BodyEncounters( prntClass ).getReturnedData( tokens, currentPtr );
					ctx->signal = SIGNAL::RETURN;
					break;

				case TOKEN_CONST::THEKU: case TOKEN_CONST::PINNAVA:
					ctx->signal = curToken.tokConst == TOKEN_CONST::THEKU ? SIGNAL::BREAK : SIGNAL::CONTINUE;
					ctx->LastRecoverErrorList = curToken.row;
					break;

				default: {
					if( curToken.token == "}" ){
						if( prntClass->parent == nullptr )
							throw InvalidSyntaxError( "unknown token }" );
						return nullopt;
					}
					const shared_ptr<MapItem>* func = prntClass->lookup( curToken.token );

					if( func && (*func)->mapType == MAPTYPE::FUNC_PTR ){
						label = "Pindi call Line ";
						shared_ptr<MapItem> keepAlive = *func;
						BodyEncounters( prntClass ).handleFunctionCall( keepAlive, tokens, currentPtr, curToken.tokenId );
					}
					else{
						try{ prntClass->InstructionHandlerRunner( tokens, currentPtr, curToken.tokenId ); }
						catch( const RecoverError& ){ ctx->LastRecoverErrorList = curToken.row; throw; }
					}
				}
			}
		}
		catch( const RecoverError& ){ throw; }
		catch( ... ){
			cout << label << to_string( tokens[ at ].row + 1 ) << ": \n";
			throw;
		}
		if( ctx->signal != SIGNAL::NONE )
			break;

		currentPtr++;
	}
	// only a function ( or the program itself ) consumes a pending signal, blocks and loops pass it on
	if( C_CLASS == CALLER::FUNCTION && ctx->signal != SIGNAL::NONE ){
		const SIGNAL pending = ctx->signal;
		ctx->signal = SIGNAL::NONE;

		if( pending != SIGNAL::RETURN )
			throw InvalidSyntaxError( "theku / pinnava used outside of a loop ( line " + to_string( ctx->LastRecoverErrorList + 1 ) + " )" );

		FUNC_RETURN_TYPE result = std::move( ctx->ret );
		ctx->ret.reset();
		return result;
	}
	return nullopt;
}


int main( int argc, char *argv[] ){
	// Buffered output is only used when stdout is a pipe / file. On a terminal the original
	// synchronised behaviour is kept so every para shows up immediately.
	if( !isatty( fileno( stdout ) ) )
		std::ios_base::sync_with_stdio( false );

	if( argc != 2 )
		throw runtime_error("./pscript <filename>.pp");

	std::shared_ptr<Context> cContext = std::make_shared<Context>();

	cContext->filename = argv[1];
	if (cContext->filename.size() <= 3)
		throw runtime_error("Invalid file format");

	if( cContext->filename.substr(cContext->filename.size() - 3) != ".pp" )
		throw runtime_error("Invalid file format");


	getTheTokens( cContext->filename, cContext->fullTokens );
	preComputed.reserve( cContext->fullTokens.size() / 2 + 16 );

	FunctionHandler func(cContext);
	func.functionName = "MAIN";
	try{
		ProgramExecutor( func.ctx->fullTokens, func.ctx->pointer, CALLER::FUNCTION, &func );
	}
	catch( InvalidSyntaxError& err ){
		cout << err.what() << endl;
	}
	catch( const RecoverError& err ){
		cout.flush();
		throw runtime_error("Some Error occured at line: " + to_string(func.ctx->LastRecoverErrorList + 1));
	}
	catch( ... ){
		// keep everything printed so far when the program dies with an uncaught error
		cout.flush();
		throw;
	}
	cout.flush();
	return 0;
}