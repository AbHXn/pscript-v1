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

using namespace std;

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

unordered_set<string> EXPR_CONSTANTS = {
	"+", "-", "/", "%", "*",
	">", "<", "==", "!=", "<=", ">=",
	"um", "yo"
};

unordered_set<string> REG_AST_TOKEN = {
	"+", "-", "/", "%", "*",
	">", "<", "==", "!=", "<=", ">=",
	"um", "yo", ")", "("
};

unordered_map<AST_TOKENS, short unsigned int> PRIORITY = {
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
	unique_ptr<AST_NODE> left;
	unique_ptr<AST_NODE> right;
};

AST_TOKENS getExprToken( const string& curTokens ) {
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

bool isRegisteredASTTokens( const string& tok ){
	return REG_AST_TOKEN.find( tok ) != REG_AST_TOKEN.end();
}

bool isRegisteredASTExprTokens( const string& tok ){
	return EXPR_CONSTANTS.find( tok ) != EXPR_CONSTANTS.end();
}

vector<variant<CODE_TOKENS, AST_TOKENS>>
stringToASTTokens( const vector<string>& tokens ){
	vector<variant<CODE_TOKENS, AST_TOKENS>> resultTokens;

	for( int startIndex = 0; startIndex < tokens.size(); startIndex++ ){
		const string& curToken = tokens[ startIndex ];

		if( EXPR_CONSTANTS.find( curToken ) != EXPR_CONSTANTS.end() ){
			AST_TOKENS getToken = getExprToken( curToken );
			resultTokens.push_back( getToken );
		}
		else if( curToken == "(" ){
			resultTokens.push_back( CODE_TOKENS::OPEN );
		}
		else if( curToken == ")" ){
			resultTokens.push_back( CODE_TOKENS::CLOSE );
		}
		else resultTokens.push_back( CODE_TOKENS::VALUE );
	}
	return resultTokens;
} 

template <typename AST_NODE_TYPE>
optional<unique_ptr<AST_NODE<AST_NODE_TYPE>>> 
finishTheASTTree( vector<unique_ptr<AST_NODE<AST_NODE_TYPE>>>&nodes, vector<unique_ptr<AST_NODE<AST_NODE_TYPE>>>&optrs ){
	while( !optrs.empty() ){
		unique_ptr<AST_NODE<AST_NODE_TYPE>> topOptr = move( optrs.back() );
		optrs.pop_back();
	
		if( nodes.size() < 2 ) return nullopt;

		unique_ptr<AST_NODE<AST_NODE_TYPE>> right = move( nodes.back() );
		nodes.pop_back();
		unique_ptr<AST_NODE<AST_NODE_TYPE>> left = move( nodes.back() );
		nodes.pop_back();

		topOptr->left 	= move( left );
		topOptr->right  = move( right);
		nodes.push_back( move( topOptr ) );
	}
	if( !nodes.size() ) return nullopt;
	return move( nodes.back() );
}

// AST_TREE BUILDER
template <typename VAR_HOLD_DATA, typename AST_NODE_DATA>
optional<unique_ptr<AST_NODE<AST_NODE_DATA>>>
BUILD_AST( vector<variant<CODE_TOKENS, AST_TOKENS>>& codeTokens, queue<VAR_HOLD_DATA>& valueQueue, size_t& start ){	
	vector<unique_ptr<AST_NODE<AST_NODE_DATA>>> nodeStack, optrStack;

	for(; start < codeTokens.size(); start++){
		auto currentVariantToken = codeTokens[ start ];
		
		if( holds_alternative<CODE_TOKENS>( currentVariantToken ) ){
			CODE_TOKENS currentToken = get<CODE_TOKENS>( currentVariantToken );

			switch( currentToken ){
				case CODE_TOKENS::VALUE: {
					if( valueQueue.empty() )
						return nullopt;
					
					auto topQueueValue = move( valueQueue.front() );
					valueQueue.pop();
					unique_ptr<AST_NODE<AST_NODE_DATA>> newNode = make_unique<AST_NODE<AST_NODE_DATA>>();
					newNode->isASTTokens = false;
					newNode->AST_DATA = move( topQueueValue );
					nodeStack.push_back( move( newNode ) );
					break;
				}
				case CODE_TOKENS::OPEN: {
					++start;
					auto optCompleteNode = BUILD_AST<VAR_HOLD_DATA, AST_NODE_DATA>( codeTokens, valueQueue, start);
					if( !optCompleteNode.has_value() )
						return nullopt;
					unique_ptr<AST_NODE<AST_NODE_DATA>> completeNode = move(optCompleteNode.value());
					nodeStack.push_back( move(completeNode) );
					break;
				}
				case CODE_TOKENS::CLOSE:
					goto FINAL_STAGE;
				default:
					break;
			} // switch end
		}
		else{
			AST_TOKENS curOptr = get<AST_TOKENS>( currentVariantToken );
			unique_ptr< AST_NODE<AST_NODE_DATA>> newNode = make_unique<AST_NODE<AST_NODE_DATA>>( );
			newNode->isASTTokens = true;
			newNode->AST_DATA = curOptr;

			if( !optrStack.empty() ){
				AST_TOKENS optrFromStack = get<AST_TOKENS>( optrStack.back()->AST_DATA );
				while( PRIORITY[ optrFromStack ] >= PRIORITY[ curOptr ] ){
					unique_ptr<AST_NODE<AST_NODE_DATA>> topOptr = move( optrStack.back() );
					optrStack.pop_back();
					if( nodeStack.size() < 2 ) return nullopt;
					
					unique_ptr<AST_NODE<AST_NODE_DATA>> right = move( nodeStack.back() );
					nodeStack.pop_back();
					unique_ptr<AST_NODE<AST_NODE_DATA>> left = move( nodeStack.back() );
					nodeStack.pop_back();
					topOptr->left  = move(left);
					topOptr->right = move(right);
					nodeStack.push_back( move( topOptr ) );

					if( optrStack.empty() ) break;
					
					optrFromStack = get<AST_TOKENS>( optrStack.back()->AST_DATA );
				}
			}
			optrStack.push_back( move( newNode ) );
		}
	}
	FINAL_STAGE:
		return finishTheASTTree( nodeStack, optrStack );
}

#endif