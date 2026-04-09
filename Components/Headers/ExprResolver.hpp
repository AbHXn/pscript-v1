#ifndef EXPRRESOLVER
#define EXPRRESOLVER

#include "ValueHelper.hpp"
#include "AST.hpp"
#include "../Verifier/Headers/Variables.hpp"
#include "../Verifier/Headers/Function.hpp"

#include <string>

template<typename T>
inline constexpr bool is_number_v = std::is_same_v<T, long int> || std::is_same_v<T, double> || std::is_same_v<T, bool>;

class FunctionHandler;

struct RESOLVER_TYPE{
	std::queue<REAL_AST_NODE_DATA> resolvedAstNodeData;
	std::vector<std::string> simpleVector;

	RESOLVER_TYPE() = default;

	RESOLVER_TYPE(
		std::queue<REAL_AST_NODE_DATA> 	resolvedAstNodeData,
		std::vector<std::string> simpleVector
	){
		this->resolvedAstNodeData = resolvedAstNodeData;
		this->simpleVector = simpleVector;
	}
};

class ExprResolver {
	public:
		static std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> getAstRootNode( std::vector<Token>& vtr, FunctionHandler* func );
		static DEEP_VALUE_DATA getFinalValue( std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& newAstNode, FunctionHandler* func );
		static DEEP_VALUE_DATA evaluateVector( std::vector<Token>& vtr, FunctionHandler* func );
		static RESOLVER_TYPE vectorResolver( const std::vector<Token>& tokens, FunctionHandler* func );
		static DEEP_VALUE_DATA evaluate_AST_NODE( const std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& astNode, FunctionHandler* helperHandler, size_t level = 0);

};

#endif