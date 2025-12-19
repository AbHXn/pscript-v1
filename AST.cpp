#include <iostream>
#include <variant>
#include <queue>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_set>

#include "Dtypes.hpp"
using namespace std;

enum class AST_TOKENS{ NONE, ADD, SUB, MUL, DIV, MOD, AND, OR, NOT };
enum class CODE_TOKENS{ TOKEN_AST, OPEN, CLOSE, VALUE };

constexpr string_view EXPR_CONSTANTS = "+-/%*";

struct AST_NODE{
	bool isASTTokens;
	variant<AST_TOKENS, VarDtype> data;
	unique_ptr<AST_NODE> left, right;
};

queue<variant<AST_TOKENS, string>> valueQueue;

unordered_map<AST_TOKENS, short unsigned int> PRIORITY = {
	{ AST_TOKENS::ADD, 1 },
	{ AST_TOKENS::SUB, 1 },
	{ AST_TOKENS::MUL, 2 },
	{ AST_TOKENS::DIV, 2 },
	// --------->relational later
};

AST_TOKENS getExprToken( const string& curTokens ) {
	if( curTokens == "+" )
		return AST_TOKENS::ADD;
	else if( curTokens == "-" )
		return AST_TOKENS::SUB;
	else if( curTokens == "*" )
		return AST_TOKENS::MUL;
	else if( curTokens == "/" )
		return AST_TOKENS::DIV;
	else if( curTokens == "%" )
		return AST_TOKENS::MOD;
	return AST_TOKENS::NONE;
}

optional<vector<CODE_TOKENS>> stringToTokens( vector<string>& tokens, size_t& startIndex ){
	vector<CODE_TOKENS> resultTokens;
	for( ; startIndex < tokens.size(); startIndex++ ){
		string curTokens = tokens[ startIndex ];

		if( EXPR_CONSTANTS.find( curTokens ) != string_view::npos ){
			AST_TOKENS getToken = getExprToken( curTokens );
			valueQueue.push( getToken );
			resultTokens.push_back( CODE_TOKENS::TOKEN_AST );
		}
		else if( curTokens == "{" ){
			resultTokens.push_back( CODE_TOKENS::OPEN );
		}
		else if( curTokens == "}" ){
			resultTokens.push_back( CODE_TOKENS::CLOSE );
		}else{
			//-----> check value, hmap else raise;
			valueQueue.push( curTokens );
			resultTokens.push_back( CODE_TOKENS::VALUE );
		}
	}
	return resultTokens;
} 

optional<unique_ptr<AST_NODE>> finishTheASTTree( vector<unique_ptr<AST_NODE>>&nodes, vector<unique_ptr<AST_NODE>>&optrs ){
	if( nodes.empty() || optrs.empty() )
		return nullopt;

	int nodePtr = 0, optrPtr = 1;
	unique_ptr<AST_NODE> left = move( nodes[ 0 ] );

	for( ; nodePtr < nodes.size() && optrPtr < optrs.size(); nodePtr++, optrPtr++ ){
		unique_ptr<AST_NODE> topOptr = move( optrs[ optrPtr ] );
		unique_ptr<AST_NODE> right   = move( nodes[ nodePtr ] );
		
		topOptr->left  = move( left );
		topOptr->right = move( right );
		
		left = move( topOptr );
	}
	if( nodePtr == nodes.size() && optrPtr == optrs.size() )
		return move( left );
	return nullopt;
}

optional<unique_ptr<AST_NODE>> builtASTTree( vector<CODE_TOKENS>& codeTokens, size_t& start ){
	vector<unique_ptr<AST_NODE>> nodeStack, optrStack;

	for( ; start < codeTokens.size(); start++ ){
		CODE_TOKENS currentToken = codeTokens[ start ];

		switch( currentToken ){
			case CODE_TOKENS::VALUE:{
				if( valueQueue.empty() ){
					//---------> value queue is empty return error;
					return nullopt;
				}
				auto topQueueValue = valueQueue.front();
				valueQueue.pop();
				
				unique_ptr<AST_NODE> newNode = make_unique<AST_NODE>();
				newNode->isASTTokens = false;
				newNode->data = get<string>( topQueueValue );

				nodeStack.push_back( move(newNode) );
				break;
			}
			case CODE_TOKENS::TOKEN_AST:{
				if( valueQueue.empty() ){
					//-------> value queue is empty return error;
					return nullopt;
				}

				auto topQueueValue = valueQueue.front();
				valueQueue.pop();

				AST_TOKENS curOptr = get<AST_TOKENS>( topQueueValue );

				unique_ptr<AST_NODE>newNode = make_unique<AST_NODE>();
				newNode->isASTTokens = true;
				newNode->data = curOptr;

				if( !optrStack.empty() ){
					AST_TOKENS optrFromStack = get<AST_TOKENS>( optrStack.back()->data );

					while( PRIORITY[ optrFromStack ] > PRIORITY[ curOptr ] ){
						unique_ptr<AST_NODE> topOptr = move( optrStack.back() );
						optrStack.pop_back();

						if( nodeStack.size() < 2 ){
							// ----> raise error
							return nullopt;
						}

						unique_ptr<AST_NODE> right = move(nodeStack.back());
						nodeStack.pop_back();
						unique_ptr<AST_NODE> left = move(nodeStack.back());
						nodeStack.pop_back();

						topOptr->left = move(left);
						topOptr->right = move(right);

						nodeStack.push_back( move( topOptr ) );

						if( optrStack.empty() )
							break;

						optrFromStack = get<AST_TOKENS>( optrStack.back()->data );
					}
					optrStack.push_back( move( newNode ) );
				}
				else optrStack.push_back( move( newNode ) );
				break;
			}
			case CODE_TOKENS::OPEN:{
				auto optCompleteNode = builtASTTree( codeTokens, ++start );
				
				if( !optCompleteNode.has_value() )
					return nullopt;

				unique_ptr<AST_NODE> completeNode = optCompleteNode.value();
				nodeStack.push_back( move(completeNode) );
				break;
			}
			case CODE_TOKENS::CLOSE:
				goto FINAL_STAGE;
			default:
				// raise invalid token
				break;
		}
	}
	FINAL_STAGE:
		return finishTheASTTree( nodeStack, optrStack );
}

int main(){
	vector<string> tokens = { "A", "+", "B", "*", "C" };
	size_t start = 0;
	auto ress = stringToTokens( tokens, start );
	vector<CODE_TOKENS> res = ress.value();
	for(CODE_TOKENS x: res)
		cout << (int) x << endl;\
	start = 0;
	//unique_ptr<AST_NODE> root = builtASTTree( res, start );
	//printAST( root );
	cout << "tree build\n";
	return 0;
}