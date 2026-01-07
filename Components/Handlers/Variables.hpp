#ifndef VARIABLES_HPP
#define VARIABLES_HPP

#include <iostream>
#include <variant>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>
#include <cctype>
#include <unordered_set>

#include "../../Headers/MBExceptions.hpp"
#include "../../Headers/Dtypes.hpp"
#include "../../Headers/Tokenizer.hpp"

using namespace std;

enum class VALUE_TOKENS{
	ARRAY_OPEN 	,
	ARRAY_VALUE ,
	NORMAL_VALUE,
	COMMA 		,
	ARRAY_CLOSE ,
	VALUE_END
};

enum class VARIABLE_TOKENS{
	VAR_START 	 ,
	NAME 	 	 ,
	COMMA 		 ,
	ARRAY 		 ,
	VALUE_ASSIGN ,
	VAR_ENDS	
};

const unordered_set<string> REGISTERED_TOKENS  = { 
		"pidi", 
		"kootam", 
		"=", 
		",", 
		";", 
		"{", 
		"}" 
};

unordered_map <VARIABLE_TOKENS, vector<VARIABLE_TOKENS>> VARIABLE_DECLARE_GRAPH = {
	{ VARIABLE_TOKENS::VAR_START, 	{ VARIABLE_TOKENS::NAME } 								  },
	{ VARIABLE_TOKENS::NAME,		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::ARRAY, VARIABLE_TOKENS::VAR_ENDS }     },
	{ VARIABLE_TOKENS::ARRAY, 		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::VAR_ENDS } 		 					  },
	{ VARIABLE_TOKENS::COMMA, 		{ VARIABLE_TOKENS::NAME } 						  		  }	
};

unordered_map <VALUE_TOKENS, vector<VALUE_TOKENS>> VALUE_ASSIGN_GRAPH = {
	{ VALUE_TOKENS::NORMAL_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END } 			},
	{ VALUE_TOKENS::ARRAY_OPEN,  { VALUE_TOKENS::ARRAY_OPEN, VALUE_TOKENS::ARRAY_VALUE } 	},
	{ VALUE_TOKENS::ARRAY_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::ARRAY_CLOSE } 	   	},
	{ VALUE_TOKENS::COMMA, 	  	 { VALUE_TOKENS::ARRAY_VALUE, VALUE_TOKENS::ARRAY_OPEN, 
								   VALUE_TOKENS::NORMAL_VALUE } 							},
	{ VALUE_TOKENS::ARRAY_CLOSE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END }		   	}
};

bool isRegisteredVariableToken( const string& token ){
	return REGISTERED_TOKENS.find( token ) != REGISTERED_TOKENS.end();
}

class ArrayList{
	public:
		std::vector< 
			std::variant< 
				VarDtype,
				unique_ptr<ArrayList>
				>
			> arrayList;
		
		std::vector<size_t> dimensions;	
		size_t totalElementsAllocated;
		
		static unique_ptr<ArrayList>
		_arrayListBuilder( 
						std::vector<VALUE_TOKENS>& arrayTokenList, 
						size_t& curIndex, 
						std::queue<VarDtype>& valueQueue
					){
			auto newArrayList = make_unique< ArrayList >();
			while( curIndex < arrayTokenList.size() ){
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::COMMA ){
					curIndex++;
					continue;
				}
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_OPEN ){
					curIndex++ 	  ; 
					unique_ptr<ArrayList> array = _arrayListBuilder( arrayTokenList, curIndex, valueQueue );
					newArrayList->push_ArrayList( move( array ) );
				}
				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_VALUE ){
					VarDtype value = move ( valueQueue.front() );
					valueQueue.pop( );
					newArrayList->push_SingleElement( value );
				}
				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_CLOSE )
					return newArrayList;
				curIndex++;
			}
			throw InvalidSyntaxError("encounter invalid syntax in array creation");
		}

		static unique_ptr<ArrayList>
		createArray( std::vector<VALUE_TOKENS>& arrayTokenList,  size_t& currentPointer, std::queue<VarDtype>& ValueQueue ){
			unique_ptr<ArrayList> arrayResult = ArrayList::_arrayListBuilder( arrayTokenList, currentPointer, ValueQueue );	
			return arrayResult;
		}
		void push_SingleElement( VarDtype singleElement ){
			this->arrayList.push_back( singleElement );
		}
		
		variant<ArrayList*, VarDtype>
		getElementAtIndex(vector<long int>& dimensions, size_t index = 0){
			auto curIndex = dimensions[ index ];
			if( curIndex >= 0 && curIndex < this->arrayList.size() ){
				auto& test = this->arrayList[ curIndex ];

				if( index == dimensions.size() - 1 ){
					if ( holds_alternative<VarDtype>( test ) )
						return get<VarDtype>(test);
					auto& finalData = get<unique_ptr<ArrayList>>(test);
					return finalData.get();
				}

				if ( holds_alternative<VarDtype>( test ) )
					throw InvalidSyntaxError("Only Array Can Access Using Index");

				auto& finalData = get<unique_ptr<ArrayList>>(test);
				return finalData->getElementAtIndex( dimensions, index+1 );
			} 
			else throw ArrayOutOfBound(to_string(curIndex));
		}

		void push_ArrayList( unique_ptr<ArrayList> arrayListElement ){
			this->arrayList.push_back( move( arrayListElement ) );	
		}
};

struct VARIABLE_HOLDER{
	std::string key;
	bool isTypeArray;
	bool isValueAssigned;

	std::variant<
		VarDtype,
		unique_ptr<ArrayList>
	> value;
};

struct VariableTokens{
	vector<VARIABLE_TOKENS> varTokens;
	vector<VALUE_TOKENS> 	valueTokens;
	vector<vector<Token>>	valueVector;
	queue<Token> 			VarQueue;

	// constructor
	VariableTokens(
		vector<VARIABLE_TOKENS> varTokens,
		vector<VALUE_TOKENS> 	valueTokens,
		vector<vector<Token>>	valueVector,
		queue<Token>			VarQueue
	){
		this->varTokens   = varTokens;
		this->valueTokens = valueTokens;
		this->valueVector = valueVector;
		this->VarQueue 	  = VarQueue;
	}
};

VariableTokens stringToVariableTokens( const vector<Token>& tokens, size_t& startCurPtr ){
	vector<VARIABLE_TOKENS> varTokens;
	vector<VALUE_TOKENS> 	valueToken;
	vector<vector<Token>> 	valueVector;
	queue<Token> VarQueue;

	bool isVariableTurn 	= true;
	int arrayOpenedCount 	= 0;

	for( ; startCurPtr < tokens.size(); ++startCurPtr ) {
		const Token& tok = tokens[ startCurPtr ];
		const string& curToken = tok.token;

		if( curToken == "pidi" ){
			varTokens.push_back( VARIABLE_TOKENS::VAR_START );
		}
		else if( curToken == ","){
			( isVariableTurn ) ?  varTokens.push_back( VARIABLE_TOKENS::COMMA )
							   : valueToken.push_back( VALUE_TOKENS::COMMA );
		}
		else if( curToken == "kootam" )
			varTokens.push_back( VARIABLE_TOKENS::ARRAY );

		else if ( curToken == "{" ){
			arrayOpenedCount++;
			valueToken.push_back( VALUE_TOKENS::ARRAY_OPEN );
		}
		else if( curToken == "="){
			varTokens.push_back( VARIABLE_TOKENS::VALUE_ASSIGN );
			isVariableTurn = false;
		}
		else if(curToken == "}"){
			arrayOpenedCount--;
			valueToken.push_back( VALUE_TOKENS::ARRAY_CLOSE );
		}
		else if(curToken == ";"){
			( isVariableTurn ) ? varTokens.push_back( VARIABLE_TOKENS::VAR_ENDS )
				 			   : valueToken.push_back( VALUE_TOKENS::VALUE_END );
			return VariableTokens( varTokens, valueToken, valueVector, VarQueue );
		}
		else{
			if( isVariableTurn && tok.type == TOKEN_TYPE::IDENTIFIER ){
				VarQueue.push( tok );
				varTokens.push_back( VARIABLE_TOKENS::NAME );
			}
			else{
				vector<Token> curValueVector;
				int openBrack = 0;

				while( startCurPtr < tokens.size() ){
					if( tokens[ startCurPtr ].token == "(" )
						openBrack++;
					else if( tokens[startCurPtr].token == ")")
						openBrack--;
					if( isRegisteredVariableToken( tokens[ startCurPtr ].token )  && openBrack == 0 ){
						startCurPtr--;
						break;
					}
					curValueVector.push_back( tokens[ startCurPtr++ ] );
				}
				if( curValueVector.size() > 1 )
					valueVector.push_back( curValueVector );
				else  valueVector.push_back( curValueVector );
				
				if( arrayOpenedCount > 0 )
					valueToken.push_back( VALUE_TOKENS::ARRAY_VALUE );
				else valueToken.push_back( VALUE_TOKENS::NORMAL_VALUE );
			}
		}	
	}
	throw InvalidSyntaxError( "; didn't hit this token, hope its an erro :) " );
}

bool 
isValidVariableSyntax( 
					vector<VARIABLE_TOKENS>& varTokens, 
					vector<
						unique_ptr<VARIABLE_HOLDER>
					>& variableStack ,
					queue<Token>& VarQueue
	){
	if( !varTokens.size() ) return false;

	VARIABLE_TOKENS currentStage = VARIABLE_TOKENS::VAR_START;
	size_t currentPointer = 0;
	try{
		while( currentStage == varTokens[ currentPointer ] ){		
			if( currentStage == VARIABLE_TOKENS::VALUE_ASSIGN ) 
				break;
			switch( varTokens[ currentPointer ] ){
				case VARIABLE_TOKENS::NAME: {
					if( VarQueue.empty() )
						return false;
					else{
						Token variableName = VarQueue.front();
						if( variableName.type != TOKEN_TYPE::IDENTIFIER )
							throw InvalidSyntaxError(
								"Invalid Variable Name at line:  " + to_string(variableName.row) );
						unique_ptr<VARIABLE_HOLDER> newVariable = make_unique<VARIABLE_HOLDER>();
						newVariable->key = variableName.token;
						newVariable->isValueAssigned = false;
						VarQueue.pop( );
						variableStack.push_back( move( newVariable ));
					}
					break;
				}
				case VARIABLE_TOKENS::ARRAY: {
					if( variableStack.empty() )
						return false;
					else variableStack.back()->isTypeArray = true;
					break;
				}
				default: break;
			}
			if(currentPointer + 1 >= varTokens.size() )
				break;

			currentPointer++;
			vector<VARIABLE_TOKENS> nextPossibleTokens = VARIABLE_DECLARE_GRAPH[ varTokens[ currentPointer - 1 ] ];
			bool continueChecking = false;
			for( int x = 0; x < nextPossibleTokens.size(); x++ ){
				if( nextPossibleTokens[ x ] == varTokens[ currentPointer ] ){
					currentStage = varTokens[ currentPointer ];
					continueChecking = true;
					break;
				}
			}
			if( !continueChecking )
				break;
		}
		if( currentStage != VARIABLE_TOKENS::VALUE_ASSIGN || currentPointer != varTokens.size() - 1 )
			throw InvalidSyntaxError( "Occures Syntax error in Variable declaration" );
		return true;
	} 
	catch ( const InvalidNameError& err ){
		cout << err.what() << endl;
		return false;
	}
	catch ( const InvalidSyntaxError& err ){
		cout << err.what() << endl;
		return false;
	}
}

template <typename VAR_DTYPE> bool 
isValidValueSyntax( 
			vector<VALUE_TOKENS>& valueTokens, 
			vector<unique_ptr<VARIABLE_HOLDER>>& variable,
			queue<VAR_DTYPE>& ValueQueue

){
	if( !valueTokens.size() )
		return false;

	VALUE_TOKENS currentStage = VALUE_TOKENS::NORMAL_VALUE;
	size_t currentPointer 	  = 0;
	size_t updatedVariable 	  = 0;
	
	try {
		if( valueTokens[ currentPointer ] == VALUE_TOKENS::ARRAY_OPEN )
			currentStage = VALUE_TOKENS::ARRAY_OPEN;
		
		while( valueTokens[ currentPointer ] == currentStage ){
			if( currentStage == VALUE_TOKENS::VALUE_END )
				break;

			if( updatedVariable >= variable.size() )
				throw VariableDeclarationMissing( );
			
			switch( valueTokens[ currentPointer ] ){
				case VALUE_TOKENS::ARRAY_OPEN: {		
					auto& topVariable = variable[ updatedVariable ];
					if( topVariable->isTypeArray == false )
						throw InvalidDTypeError( "Variable expected is not kootam" );
					currentPointer++;
					auto newArray = ArrayList::createArray( 
						valueTokens, 
						currentPointer,
						ValueQueue 
					);
					topVariable->value = move( newArray );	
					updatedVariable++;
					break;
				}
				case VALUE_TOKENS::NORMAL_VALUE:{
					auto& topVariable = variable[updatedVariable];
					if( topVariable->isTypeArray )
						throw InvalidDTypeError( "Variable expected is kootam" );	
					if( ValueQueue.empty() )
						return false;
					auto value = ValueQueue.front();
					ValueQueue.pop();
					topVariable->value = value;
					updatedVariable++;
					break;
				}
				default:
					break;
			}
			if( currentPointer + 1 >= valueTokens.size() )
				break;

			currentPointer++;
			bool continueChecking = false;
			vector<VALUE_TOKENS> graph = VALUE_ASSIGN_GRAPH[ valueTokens[ currentPointer - 1 ] ];
			for(int x = 0; x < graph.size(); x++ ){
				if( valueTokens[ currentPointer ] == graph[ x ] ){
					currentStage = valueTokens[ currentPointer ];
					continueChecking = true;
				}
			}
			if( !continueChecking )
				break;
		}
		if( currentStage != VALUE_TOKENS::VALUE_END)
			throw InvalidSyntaxError("Occures Syntax error in Variable initialization");
		return true;
	}
	catch ( const VariableDeclarationMissing& err ){
		cout << err.what() << endl;
		return false;
	}
	catch( const InvalidDTypeError& err ){
		cout << err.what() << endl;
		return false;
	}
	catch( const InvalidSyntaxError& err ){
		cout << err.what() << endl;
		return false;
	}
}

enum class ARRAY_ACCESS{
	NOTHING 		,
	VAR_NAME 		,
	BRACK_OPEN 		,
	INDEX_VECTOR 	,
	BRACK_CLOSE	 	,
	END
};

struct ArrayAccessTokens{
	vector<ARRAY_ACCESS> tokens;
	string arrayName;
	vector<vector<Token>> indexVector;

	ArrayAccessTokens( 
		vector<ARRAY_ACCESS> tokens,
		string arrayName,
		vector<vector<Token>> indexVector
	){
		this->tokens 	  = tokens;
		this->arrayName   = arrayName;
		this->indexVector = indexVector;
	}
};

ArrayAccessTokens
stringToArrayAccesToken( const vector<Token>&tokens, size_t& currentPtr ){
	vector<vector<Token>> indexVector;
	vector<ARRAY_ACCESS>  arrAccessTokens;
	string 				  arrName;
	ARRAY_ACCESS prev = ARRAY_ACCESS::NOTHING;

	while( currentPtr < tokens.size() ){
		const Token& tok = tokens[ currentPtr ];
		const string& curToken = tok.token;

		if( curToken == "[" )
			arrAccessTokens.push_back( ARRAY_ACCESS::BRACK_OPEN );

		else if( curToken == "]" )
			arrAccessTokens.push_back( ARRAY_ACCESS::BRACK_CLOSE );

		else {
			if( prev == ARRAY_ACCESS::NOTHING ){
				arrName = curToken;
				arrAccessTokens.push_back( ARRAY_ACCESS::VAR_NAME );
			}
			else if( prev == ARRAY_ACCESS::BRACK_OPEN ){
				arrAccessTokens.push_back( ARRAY_ACCESS::INDEX_VECTOR );
				vector<Token> curIndexVec;
				
				int openCnts = 1;

				while( currentPtr < tokens.size() ){
					const Token& tok = tokens[ currentPtr ];
					if( tok.token == "[" ){
						openCnts++;
						currentPtr++;
						continue;
					}
					if( tok.token == "]" ){
						openCnts--;
						if( openCnts == 0 ){
							currentPtr--;
							break;
						}
						currentPtr++;
						continue;
					}
					
					curIndexVec.push_back( tok );
					currentPtr++;
				}
				indexVector.push_back( curIndexVec );
			}
			else {
				arrAccessTokens.push_back( ARRAY_ACCESS::END );
				return ArrayAccessTokens( arrAccessTokens, arrName, indexVector );
			}
		}
		currentPtr++;
		prev = arrAccessTokens.empty() ? prev : arrAccessTokens.back();
	}
	arrAccessTokens.push_back( ARRAY_ACCESS::END );
	return ArrayAccessTokens( arrAccessTokens, arrName, indexVector );
}

unordered_map<ARRAY_ACCESS, vector<ARRAY_ACCESS>> ARRAY_ACCESS_GRAPH = {
	{ ARRAY_ACCESS::VAR_NAME, 		{ ARRAY_ACCESS::BRACK_OPEN, 
									  ARRAY_ACCESS::END } 			},
	{ ARRAY_ACCESS::BRACK_OPEN, 	{ ARRAY_ACCESS::INDEX_VECTOR }  },
	{ ARRAY_ACCESS::INDEX_VECTOR, 	{ ARRAY_ACCESS::BRACK_CLOSE } 	},
	{ ARRAY_ACCESS::BRACK_CLOSE, 	{ ARRAY_ACCESS::END } 			}
};

bool 
isValidArrayAccess( vector<ARRAY_ACCESS>& tokens ){
	size_t startIndex = 0;
	ARRAY_ACCESS currentStage = ARRAY_ACCESS::VAR_NAME;

	while( startIndex < tokens.size() ){
		ARRAY_ACCESS newTok = tokens[ startIndex ];
		if( newTok == ARRAY_ACCESS::END )
			return true;
		if( startIndex + 1 < tokens.size() )
			startIndex++;
		else break;

		ARRAY_ACCESS nextExpected = tokens[ startIndex ];
		vector<ARRAY_ACCESS>& nextExpectedTokens = ARRAY_ACCESS_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( ARRAY_ACCESS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError("Invalid Array Access");
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Occurs error in array accessing" );
}

#endif