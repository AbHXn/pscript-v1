#include "Headers/ExprResolver.hpp"
#include "Headers/FunctionHandler.hpp"

std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> 
ExprResolver::getAstRootNode( std::vector<Token>& vtr, FunctionHandler* func ){
	auto resolvedType 		= ExprResolver::vectorResolver( vtr, func );
	auto astTokenAndData 	= stringToASTTokens( resolvedType.simpleVector );
	size_t startAST 		= 0;

	auto newAstNode = BUILD_AST<REAL_AST_NODE_DATA>( astTokenAndData, resolvedType.resolvedAstNodeData, startAST );
	if( !newAstNode.has_value() )
		throw InvalidDTypeError("Failed to resolve the vector\n");
	return std::move( newAstNode.value() );
}

DEEP_VALUE_DATA
ExprResolver::getFinalValue( std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& newAstNode, FunctionHandler* func ){
	return evaluate_AST_NODE( newAstNode, func );
}

DEEP_VALUE_DATA 
ExprResolver::evaluateVector( std::vector<Token>& vtr, FunctionHandler* func ){
	auto data = ExprResolver::getAstRootNode( vtr, func );
	return ExprResolver::getFinalValue( data, func );
}

RESOLVER_TYPE
ExprResolver::vectorResolver( const std::vector<Token>& tokens, FunctionHandler* func ){
	std::queue<REAL_AST_NODE_DATA> resolvedAstNodeData;
	std::vector<std::string> 	simpleVector;

	for( size_t x = 0; x < tokens.size(); x++ ){
		const Token& tok = tokens[ x ];
		const std::string& curToken = tokens[ x ].token;

		if( !isValueType( tok.type ) && (isRegisteredASTExprTokens( curToken ) || curToken == ")" ||  curToken == "(" )){
			simpleVector.push_back( curToken );
			continue;
		}
		if( isValueType( tok.type ) ){
			resolvedAstNodeData.push( ValueHelper::getValueFromToken( tok ) );
			simpleVector.push_back("NUM");
		}
		else if( tok.type == TOKEN_TYPE::IDENTIFIER ){
			auto mainVmapData = func->getFromVmap( curToken );

			if (mainVmapData == nullptr)
				throw InvalidSyntaxError( "Unknown identifier " + curToken );

			if( mainVmapData->mapType == MAPTYPE::VARIABLE || mainVmapData->mapType == MAPTYPE::ARRAY_PTR ){
				ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );
				passArrayAccessToken( arrToken.tokens );
				resolvedAstNodeData.push( std::make_pair(arrToken, curToken) );
				simpleVector.push_back("VAR");
				x--;
			}
			else {
				FunctionCallReturns pt = stringToFunctionCallTokens( tokens, x );
				resolvedAstNodeData.push( std::make_pair(pt, tok) );
				simpleVector.push_back("CACHE");
				x--; 
			}
		}
		else if( tok.type == TOKEN_TYPE::RESERVED && curToken == "edukku" ){
			resolvedAstNodeData.push( std::string("edukku"));
			simpleVector.push_back("NUM");
		}
		else if( curToken == "{" && tok.type == TOKEN_TYPE::SPEC_CHAR ){
			size_t start_ptr = 0;
			VariableTokens arrayCreationToken = stringToVariableTokens( tokens, start_ptr, false );
			arrayCreationToken.valueTokens.push_back( VALUE_TOKENS::VALUE_END );
			passValidValueTokens( arrayCreationToken.valueTokens );

			std::queue<DEEP_VALUE_DATA> resolvedValueVector;
			for(int x = 0; x < arrayCreationToken.valueVector.size(); x++){
				auto finalValue = ExprResolver::evaluateVector( arrayCreationToken.valueVector[x], func );
				resolvedValueVector.push( finalValue );
			}
			size_t nstart_ptr = 1;
			std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> tempArray = ArrayList<ARRAY_SUPPORT_TYPES>::createArray( 
				arrayCreationToken.valueTokens, nstart_ptr, resolvedValueVector
			 );
			resolvedAstNodeData.push( tempArray );
			simpleVector.push_back("ARRAY");
			x = start_ptr;
		}
		else throw InvalidSyntaxError( "Unknown Token " + curToken );
	}
	return RESOLVER_TYPE( resolvedAstNodeData, simpleVector );
}

DEEP_VALUE_DATA 
ExprResolver::evaluate_AST_NODE( const std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& astNode, FunctionHandler* helperHandler, size_t level ){
	auto& astNodeData = astNode->AST_DATA;

	if( !std::holds_alternative<AST_TOKENS>( astNodeData ) ){
		if( std::holds_alternative<std::string>( astNodeData ) ){
			std::string test = std::get<std::string> ( astNodeData );
			if( test == "edukku" ){
				std::string inputValue; 
				getline(std::cin, inputValue);
				return inputValue;
			}
			throw;
		}

		if( std::holds_alternative<VarDtype>( astNodeData ) )
			return std::get<VarDtype>( astNodeData );
		
		BodyEncounters bodyEncounter( helperHandler );
		if( std::holds_alternative<std::pair<ArrayAccessTokens, std::string>> ( astNodeData ) ){
			auto [accessTok, mapDataKey] = std::get<std::pair<ArrayAccessTokens, std::string>>( astNodeData );
			std::shared_ptr<MapItem> mapData = helperHandler->getFromVmap( mapDataKey );

			if( mapData->mapType == MAPTYPE::VARIABLE ){
				auto varHolder = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( mapData->var );

				if( !varHolder->isTypeArray ){
					DEEP_VALUE_DATA tdata = ValueHelper::getDataFromVariableHolder( varHolder );
					auto datan = bodyEncounter.handleVarDtypeCases( accessTok,  tdata );
					if( level == 0 ) return datan;

					if( std::holds_alternative<VarDtype>( datan ) )
						return datan;		

					throw InvalidSyntaxError("Invalid dtype for operation");
				}
				else {
					auto& arrayData = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( varHolder->value );
					auto datan = bodyEncounter.handleArrayCases( arrayData, accessTok );
					if( level == 0 ) return datan;

					if( std::holds_alternative<VarDtype>( datan ) )
						return datan;

					throw InvalidSyntaxError("Invalid dtype for operation");
				}
			}
			if( mapData->mapType == MAPTYPE::ARRAY_PTR ){
				auto arryListPtr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( mapData->var );
				auto datan = bodyEncounter.handleArrayCases( arryListPtr, accessTok );
				if( level == 0) return datan;

				if( std::holds_alternative<VarDtype>( datan ) )
					return datan;

				throw InvalidSyntaxError("Invalid dtype for operation");
			}
		}
		else if( std::holds_alternative<std::pair<FunctionCallReturns, Token>>( astNodeData ) ){
			auto [funcRecToks, mapDatakey] = std::get<std::pair<FunctionCallReturns, Token>>( astNodeData );
			std::shared_ptr<MapItem> mapData = helperHandler->getFromVmap( mapDatakey.token );

			auto varHolder = std::get<std::shared_ptr<FUNCTION_MAP_DATA>>( mapData->var );
			
			if( isFuncPtr( funcRecToks.callTokens ) )
				return varHolder;

			size_t st = 0;
			std::string key = helperHandler->ctx->filename + std::to_string( mapDatakey.row + 1 ) + "-" + std::to_string( mapDatakey.col + 1 );

			auto data = bodyEncounter.handleFunctionCall( mapData, helperHandler->ctx->fullTokens, st, key, funcRecToks);
			
			if( data.has_value() ){	
				if( std::holds_alternative<VarDtype>( data.value() ) ){
					return std::get<VarDtype>(data.value());
				}
				else if( std::holds_alternative<std::shared_ptr<MapItem>>( data.value() ) ){
					auto returnedData = std::move( std::get<std::shared_ptr<MapItem>>( data.value() ) );
					DEEP_VALUE_DATA final = ValueHelper::getFinalValueFromMap( returnedData );

					if( level == 0 ) return final;
					
					if( std::holds_alternative<VarDtype>(final) )
						return final;

					throw InvalidSyntaxError("Invalid dtype for operation");
				}
				else if( std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( data.value() ) ){
					DEEP_VALUE_DATA final = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( data.value() );
					if( level == 0 ) return final;

					throw InvalidSyntaxError("Invalid dtype for operation");
				}
				else throw std::runtime_error("unknown typed pushed to queue");
			}
		}
		else if( std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( astNodeData ) ){
			auto data = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( astNodeData );
			if( level == 0 ) return data;
			throw InvalidSyntaxError("Invalid dtype for operation");
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
