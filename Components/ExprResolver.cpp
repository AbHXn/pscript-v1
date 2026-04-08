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
		else if( curToken == "{" && tok.type == TOKEN_TYPE::SPEC_CHAR )
			throw InvalidSyntaxError("Array should create in seperate pidi");
		else throw InvalidSyntaxError( "Unknown Token " + curToken );
	}
	return RESOLVER_TYPE( resolvedAstNodeData, simpleVector );
}
