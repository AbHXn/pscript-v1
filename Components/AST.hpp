#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <queue>
#include <unordered_set>

#include <cassert>

enum class AST_TOKENS { 
		NONE,
		//mathematical
		ADD, SUB, MUL, DIV, MOD,			
		// logical
		D_EQUAL_TO, GT_THAN, LS_THAN, 
		GT_THAN_EQ, LS_THAN_EQ, NOT_EQUAL_TO,
		// relational
		AND, OR
	};

enum class CODE_TOKENS{ OPEN, CLOSE, VALUE };

std::unordered_set<std::string> EXPR_CONSTANTS = {
	"+", "-", "/", "%", "*",
	">", "<", "==", "!=", "<=", ">=",
	"um", "yo"
};

std::unordered_map<AST_TOKENS, short unsigned int> PRIORITY = {
	{ AST_TOKENS::OR, 1 },
	
	{ AST_TOKENS::AND, 2 },
	
	{ AST_TOKENS::D_EQUAL_TO, 	3 },
	{ AST_TOKENS::NOT_EQUAL_TO, 3 },

	{ AST_TOKENS::LS_THAN, 		4 },
	{ AST_TOKENS::GT_THAN, 		4 },
	{ AST_TOKENS::GT_THAN_EQ, 	4 },
	{ AST_TOKENS::LS_THAN_EQ, 	4 },

	{ AST_TOKENS::ADD, 5 },
	{ AST_TOKENS::SUB, 5 },
	
	{ AST_TOKENS::MUL, 6 },
	{ AST_TOKENS::DIV, 6 },
	{ AST_TOKENS::MOD, 6 },
};

template <typename NodeType>
struct AST_NODE{
	bool isASTTokens;
	NodeType AST_DATA;
	std::unique_ptr<AST_NODE> left;
	std::unique_ptr<AST_NODE> right;
};

AST_TOKENS getExprToken( const std::string& curTokens ) {
	// mathematical 
	if 	   ( curTokens == "+" ) return AST_TOKENS::ADD;
	else if( curTokens == "-" ) return AST_TOKENS::SUB;
	else if( curTokens == "*" ) return AST_TOKENS::MUL;
	else if( curTokens == "/" ) return AST_TOKENS::DIV;
	else if( curTokens == "%" ) return AST_TOKENS::MOD;
	// relational
	else if( curTokens == "==" ) return AST_TOKENS::D_EQUAL_TO;
	else if( curTokens == ">" )  return AST_TOKENS::GT_THAN;
	else if( curTokens == "<" )  return AST_TOKENS::LS_THAN;
	else if( curTokens == ">=" ) return AST_TOKENS::GT_THAN_EQ;
	else if( curTokens == "<=" ) return AST_TOKENS::LS_THAN_EQ;
	else if( curTokens == "!=" ) return AST_TOKENS::NOT_EQUAL_TO;
	// and .. or.. 
	else if( curTokens == "um" )  return AST_TOKENS::AND;	
	else if( curTokens == "yo" )  return AST_TOKENS::OR;

	return AST_TOKENS::NONE;
}

bool isRegisteredASTExprTokens( const std::string& tok ){
	return EXPR_CONSTANTS.find( tok ) != EXPR_CONSTANTS.end();
}

std::vector<std::variant<CODE_TOKENS, AST_TOKENS>>
stringToASTTokens( const std::vector<std::string>& tokens ){
	std::vector<std::variant<CODE_TOKENS, AST_TOKENS>> resultTokens;

	for( int startIndex = 0; startIndex < tokens.size(); startIndex++ ){
		const std::string& curToken = tokens[ startIndex ];

		if( isRegisteredASTExprTokens(curToken) ){
			AST_TOKENS getToken = getExprToken( curToken );
			resultTokens.push_back( getToken );
		}
		else if( curToken == "(" ) 
			resultTokens.push_back( CODE_TOKENS::OPEN );
		else if( curToken == ")" )
			resultTokens.push_back( CODE_TOKENS::CLOSE );
		else resultTokens.push_back( CODE_TOKENS::VALUE );
	}
	return resultTokens;
} 

template <typename AST_NODE_TYPE>
std::optional<std::unique_ptr<AST_NODE<AST_NODE_TYPE>>> 
finishTheASTTree( std::vector<std::unique_ptr<AST_NODE<AST_NODE_TYPE>>>&nodes, std::vector<std::unique_ptr<AST_NODE<AST_NODE_TYPE>>>&optrs ){
	while( !optrs.empty() ){
		std::unique_ptr<AST_NODE<AST_NODE_TYPE>> topOptr = std::move( optrs.back() );
		optrs.pop_back();
	
		if( nodes.size() < 2 ) return std::nullopt;

		std::unique_ptr<AST_NODE<AST_NODE_TYPE>> right = std::move( nodes.back() );
		nodes.pop_back();
		std::unique_ptr<AST_NODE<AST_NODE_TYPE>> left = std::move( nodes.back() );
		nodes.pop_back();

		topOptr->left 	= std::move( left );
		topOptr->right  = std::move( right);
		nodes.push_back( std::move( topOptr ) );
	}
	if( !nodes.size() ) return std::nullopt;
	return std::move( nodes.back() );
}

// AST_TREE BUILDER
template <typename VAR_HOLD_DATA, typename AST_NODE_DATA>
std::optional<std::unique_ptr<AST_NODE<AST_NODE_DATA>>>
BUILD_AST( std::vector<std::variant<CODE_TOKENS, AST_TOKENS>>& codeTokens, std::queue<VAR_HOLD_DATA>& valueQueue, size_t& start ){	
	std::vector<std::unique_ptr<AST_NODE<AST_NODE_DATA>>> nodeStack, optrStack;

	for(; start < codeTokens.size(); start++){
		auto currentVariantToken = codeTokens[ start ];
		
		if( std::holds_alternative<CODE_TOKENS>( currentVariantToken ) ){
			CODE_TOKENS currentToken = std::get<CODE_TOKENS>( currentVariantToken );

			switch( currentToken ){
				case CODE_TOKENS::VALUE: {
					if( valueQueue.empty() )
						return std::nullopt;
					
					auto topQueueValue = std::move( valueQueue.front() );
					valueQueue.pop();
					std::unique_ptr<AST_NODE<AST_NODE_DATA>> newNode = std::make_unique<AST_NODE<AST_NODE_DATA>>();
					newNode->isASTTokens = false;
					newNode->AST_DATA = std::move( topQueueValue );
					nodeStack.push_back( std::move( newNode ) );
					break;
				}
				case CODE_TOKENS::OPEN: {
					++start;
					auto optCompleteNode = BUILD_AST<VAR_HOLD_DATA, AST_NODE_DATA>( codeTokens, valueQueue, start);
					if( !optCompleteNode.has_value() )
						return std::nullopt;
					std::unique_ptr<AST_NODE<AST_NODE_DATA>> completeNode = std::move( optCompleteNode.value() );
					nodeStack.push_back( std::move( completeNode ) );
					break;
				}
				case CODE_TOKENS::CLOSE:
					goto FINAL_STAGE;
				default:
					break;
			} // switch end
		}
		else{
			AST_TOKENS curOptr = std::get<AST_TOKENS>( currentVariantToken );
			std::unique_ptr< AST_NODE<AST_NODE_DATA>> newNode = std::make_unique<AST_NODE<AST_NODE_DATA>>( );
			newNode->isASTTokens = true;
			newNode->AST_DATA = curOptr;

			if( !optrStack.empty() ){
				AST_TOKENS optrFromStack = std::get<AST_TOKENS>( optrStack.back()->AST_DATA );
				while( PRIORITY[ optrFromStack ] >= PRIORITY[ curOptr ] ){
					std::unique_ptr<AST_NODE<AST_NODE_DATA>> topOptr = std::move( optrStack.back() );
					optrStack.pop_back();
					if( nodeStack.size() < 2 ) return std::nullopt;
					
					std::unique_ptr<AST_NODE<AST_NODE_DATA>> right = std::move( nodeStack.back() );
					nodeStack.pop_back();
					std::unique_ptr<AST_NODE<AST_NODE_DATA>> left = std::move( nodeStack.back() );
					nodeStack.pop_back();
					topOptr->left  = std::move( left );
					topOptr->right = std::move( right );
					nodeStack.push_back( std::move( topOptr ) );

					if( optrStack.empty() ) break;
					
					optrFromStack = std::get<AST_TOKENS>( optrStack.back()->AST_DATA );
				}
			}
			optrStack.push_back( std::move( newNode ) );
		}
	}
	FINAL_STAGE:
		return finishTheASTTree( nodeStack, optrStack );
}

#endif