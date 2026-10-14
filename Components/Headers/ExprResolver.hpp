#ifndef EXPRRESOLVER
#define EXPRRESOLVER

#include "ValueHelper.hpp"
#include "AST.hpp"
#include "../Verifier/Headers/Variables.hpp"
#include "../Verifier/Headers/Function.hpp"

#include <string>

class FunctionHandler;

using AST_NODE_PTR = const AST_NODE<REAL_AST_NODE_DATA>*;

// parsed sub expressions are kept inside the ( shared, type erased ) token structs that describe them
inline std::shared_ptr<void> keepAst( std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> root ){
	return std::shared_ptr<void>( root.release(), []( void* p ){ delete static_cast<AST_NODE<REAL_AST_NODE_DATA>*>( p ); } );
}

struct RESOLVER_TYPE{
	std::queue<REAL_AST_NODE_DATA> resolvedAstNodeData;
	std::vector<std::string> simpleVector;
};

class ExprResolver {
	public:
		static std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> getAstRootNode( const std::vector<Token>& vtr, FunctionHandler* func );
		static DEEP_VALUE_DATA getFinalValue( const std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& newAstNode, FunctionHandler* func );
		static RESOLVER_TYPE vectorResolver( const std::vector<Token>& tokens, FunctionHandler* func );
		static DEEP_VALUE_DATA evalNode( AST_NODE_PTR astNode, FunctionHandler* helperHandler, size_t level = 0 );
};

#endif
