#include "Components/FuncBody.hpp"

using namespace std;

size_t LastRecoverErrorList = 0;
string filename;

template <typename PARENT_CLASS>
optional<variant<VarDtype, unique_ptr<MapItem>>>
ProgramExecutor( const vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, PARENT_CLASS* prntClass, size_t endPtr ){
	size_t backUpPtr = currentPtr;

	while( currentPtr < tokens.size() ){
		backUpPtr = currentPtr;
		
		const Token& curToken = tokens[currentPtr];

		if( endPtr && currentPtr >= endPtr - 1 )
			return nullopt;

		if( curToken.token == "pidi" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				string key = filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				prntClass->VarHandlerRunner( tokens, currentPtr, key );
			}
			catch( ... ){
				cout << "pidi Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
		}
		else if( curToken.token == "nok" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				string key = filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				unique_ptr<LoopHandler> lpHandler = make_unique<LoopHandler>();
				lpHandler->runnerBody = prntClass->runnerBody;
				lpHandler->parent = prntClass;
				
				shared_ptr<Conditional> cdHandler = make_unique<Conditional>( move( lpHandler ) );
				cdHandler->runnerBody = prntClass->runnerBody;
				cdHandler->parent = prntClass;
				
				cdHandler->CondHandlerRunner( tokens, currentPtr, key );
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
				string key = filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				prntClass->IOHandlerRunner( tokens, currentPtr, key);
			}
			catch( ... ){
				cout << "para Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
		}
		else if( curToken.token == "pari" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				prntClass->functionDefHandlerRunner( tokens, currentPtr );
				currentPtr--;
			} 
			catch( const RecoverError& err ){
				continue;
			}
			catch( ... ){
				cout << "pari Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
				throw;
			}
			
		}
		else if( curToken.token == "ittuthiri" && curToken.type == TOKEN_TYPE::RESERVED ){
			try{
				string key = filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
				unique_ptr<LoopHandler> newLpHander = make_unique<LoopHandler>();
				newLpHander->parent = prntClass;
				newLpHander->runnerBody = prntClass->runnerBody;
				newLpHander->LoopHandlerRunner( tokens, currentPtr, key );
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
			switch( C_CLASS ){
				case CALLER::FUNCTION: {
					if( curToken.token == "poda" && curToken.type == TOKEN_TYPE::RESERVED ){
						return prntClass->getReturnedData( tokens, currentPtr );
					}
				}
				default: break;
			}
			auto [func, rPT] = prntClass->getFromVmap( tokens[ currentPtr ].token );
			if( func && ( func->mapType == MAPTYPE::FUNCTION || func->mapType == MAPTYPE::FUNC_PTR) ){
				try{
					string key = filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
					prntClass->handleFunctionCall( func, tokens, currentPtr, rPT, key );
				}
				catch(...){
					cout << "pari call Line " + to_string( tokens[backUpPtr].row + 1 ) << ": \n";
					throw;
				}
			}
			else{
				try{
					backUpPtr = currentPtr;
					string key = filename + to_string( tokens[ currentPtr ].row + 1 ) + "-" + to_string( tokens[ currentPtr ].col + 1 );
					prntClass->InstructionHandlerRunner( tokens, currentPtr, key);
				}
				catch( const RecoverError& err ){
					currentPtr = backUpPtr;
					LastRecoverErrorList = tokens[currentPtr].row;
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

DEEP_VALUE_DATA 
evaluate_AST_NODE( const std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& astNode, FunctionHandler* helperHandler, size_t level ){
	auto& astNodeData = astNode->AST_DATA;

	if( !std::holds_alternative<AST_TOKENS>( astNodeData ) ){
		if( std::holds_alternative<VarDtype>( astNodeData ) ){
			return std::get<VarDtype>( astNodeData );
		}
		else if( std::holds_alternative<std::pair<ArrayAccessTokens, string>> ( astNodeData ) ){
			auto [accessTok, mapDataKey] = std::get<std::pair<ArrayAccessTokens, string>>( astNodeData );
			MapItem* mapData = helperHandler->getFromVmap( mapDataKey ).first;

			if( mapData->mapType == MAPTYPE::VARIABLE ){
				auto varHolder = std::get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( mapData->var );

				if( !varHolder->isTypeArray ){
					DEEP_VALUE_DATA tdata = ValueHelper::getDataFromVariableHolder( varHolder );
					auto datan = helperHandler->handleRawVariables( accessTok,  tdata );
					if( level == 0 ) return datan;

					if( std::holds_alternative<VarDtype>( datan ) )
						return datan;		

					throw InvalidSyntaxError("Invalid dtype for operation");
				}
				else {
					auto& arrayData = std::get<std::unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( varHolder->value );
					auto datan = helperHandler->handleArrayCases( arrayData.get(), accessTok );
					if( level == 0 ) return datan;

					if( std::holds_alternative<VarDtype>( datan ) )
						return datan;

					throw InvalidSyntaxError("Invalid dtype for operation");
				}
			}
			if( mapData->mapType == MAPTYPE::ARRAY_PTR ){
				auto arryListPtr = std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>( mapData->var );
				auto datan = helperHandler->handleArrayCases( arryListPtr, accessTok );
				if( level == 0) return datan;

				if( std::holds_alternative<VarDtype>( datan ) )
					return datan;

				throw InvalidSyntaxError("Invalid dtype for operation");
			}

		}
		else if( std::holds_alternative<std::pair<FunctionCallReturns, Token>>( astNodeData ) ){
			auto [funcRecToks, mapDatakey] = std::get<std::pair<FunctionCallReturns, Token>>( astNodeData );
			MapItem* mapData = helperHandler->getFromVmap( mapDatakey.token ).first;

			auto varHolder = std::get<FUNCTION_MAP_DATA*>( mapData->var );
			
			if( isFuncPtr( funcRecToks.callTokens ) )
				return varHolder;

			size_t st = 0;
			string key = filename + to_string( mapDatakey.row + 1 ) + "-" + to_string( mapDatakey.col + 1 );

			auto data = helperHandler->handleFunctionCall( mapData, fullTokens, st, nullptr, key, funcRecToks);
			
			if( data.has_value() ){	
				if( std::holds_alternative<VarDtype>( data.value() ) ){
					return std::get<VarDtype>(data.value());
				}
				else if( std::holds_alternative<std::unique_ptr<MapItem>>( data.value() ) ){
					auto returnedData = std::move( std::get<std::unique_ptr<MapItem>>( data.value() ) );
					DEEP_VALUE_DATA final = ValueHelper::getFinalValueFromMap( returnedData.get() );
					helperHandler->pushCache( std::move( returnedData ) );

					if( level == 0 ) return final;
					
					if( std::holds_alternative<VarDtype>(final) )
						return final;

					throw InvalidSyntaxError("Invalid dtype for operation");
				}
				else throw std::runtime_error("unknown typed pushed to queue");
			}
		}
		throw InvalidSyntaxError("Invalid dtype for operation");
	}
	else{
		if( !astNode->left || !astNode->right ) 
			throw InvalidSyntaxError("Invalid Expression");
		VarDtype leftData  = std::get<VarDtype>(evaluate_AST_NODE(astNode->left, helperHandler, level + 1));
		auto op = astNode->isASTTokens ? std::get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD;

		if (op == AST_TOKENS::AND){
		    if (!ValueHelper::toBool(leftData))
		        return false;
		    VarDtype rightData = std::get<VarDtype>(evaluate_AST_NODE(astNode->right, helperHandler, level + 1));
		    return ValueHelper::toBool(rightData);
		}

		if (op == AST_TOKENS::OR){
		    if (ValueHelper::toBool(leftData))
		        return true;
		    VarDtype rightData = std::get<VarDtype>(evaluate_AST_NODE(astNode->right, helperHandler, level + 1));
		    return ValueHelper::toBool(rightData);
		}
		VarDtype rightData = std::get<VarDtype>(evaluate_AST_NODE(astNode->right, helperHandler, level + 1));

		return std::visit(
		    [&](const auto& x, const auto& y) -> VarDtype {

		        using X = std::decay_t<decltype(x)>;
		        using Y = std::decay_t<decltype(y)>;

		        if constexpr (is_number_v<X> && is_number_v<Y>){
		            switch (op){
		                case AST_TOKENS::ADD:
		                    if constexpr (std::is_same_v<X,long> && std::is_same_v<Y,long>)
		                        return x + y;
		                    else
		                        return static_cast<double>(x) + static_cast<double>(y);
		                case AST_TOKENS::SUB:
		                    if constexpr (std::is_same_v<X,long> && std::is_same_v<Y,long>)
		                        return x - y;
		                    else
		                        return static_cast<double>(x) - static_cast<double>(y);
		                case AST_TOKENS::MUL:
		                    if constexpr (std::is_same_v<X,long> && std::is_same_v<Y,long>)
		                        return x * y;
		                    else
		                        return static_cast<double>(x) * static_cast<double>(y);
		                case AST_TOKENS::DIV:
		                    return static_cast<double>(x) / static_cast<double>(y);
		                case AST_TOKENS::MOD:
		                    return static_cast<long>(x) % static_cast<long>(y);
		                case AST_TOKENS::LS_THAN:
		                    return static_cast<double>(x) < static_cast<double>(y);
		                case AST_TOKENS::GT_THAN:
		                    return static_cast<double>(x) > static_cast<double>(y);
		                case AST_TOKENS::LS_THAN_EQ:
		                    return static_cast<double>(x) <= static_cast<double>(y);
		                case AST_TOKENS::GT_THAN_EQ:
		                    return static_cast<double>(x) >= static_cast<double>(y);
		                case AST_TOKENS::D_EQUAL_TO:
		                    return static_cast<double>(x) == static_cast<double>(y);
		                case AST_TOKENS::NOT_EQUAL_TO:
		                    return static_cast<double>(x) != static_cast<double>(y);
		                default:
		                    throw std::runtime_error("Unknown operator for numbers");
		            }
		        }
		        else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>){
		            switch (op){
		                case AST_TOKENS::ADD:  return x + y;
		                case AST_TOKENS::LS_THAN:      return x <  y;
		                case AST_TOKENS::GT_THAN:      return x >  y;
		                case AST_TOKENS::LS_THAN_EQ:   return x <= y;
		                case AST_TOKENS::GT_THAN_EQ:   return x >= y;
		                case AST_TOKENS::D_EQUAL_TO:   return x == y;
		                case AST_TOKENS::NOT_EQUAL_TO: return x != y;
		                default:
		                    throw InvalidSyntaxError("Invalid operator for strings");
		            }
		        }
		        else if constexpr (std::is_same_v<X, std::string> || std::is_same_v<Y, std::string> ){
		            if (op == AST_TOKENS::ADD)
		                return ValueHelper::toString(x) + ValueHelper::toString(y);
		            throw InvalidSyntaxError("Invalid operator involving string");
		        }
		        else throw InvalidSyntaxError("Invalid operation between VarDtype types");
		    }, leftData, rightData
		);
	}
}

int main( int argc, char *argv[] ){
	try{
		filename = argv[1];
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
		throw runtime_error("Some Error occured at line: " + to_string(LastRecoverErrorList + 1));
	}
	return 0;
}