#ifndef VARHANDLER_HPP
#define VARHANDLER_HPP

#include <iostream>
#include <variant>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>
#include <cctype>
#include <unordered_set>

#include "../Headers/MBExceptions.hpp"
#include "../Headers/Dtypes.hpp"

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

template <typename VAR_DTYPE>
class SingleElement{
	private:
		VAR_DTYPE data;

	public:
		void assignValue(VAR_DTYPE value ){
			this->data 	= move (value);
		}

		VAR_DTYPE*
		getValue( void ){
		 	return &data;
		}
};

template <typename VAR_DTYPE>
class ArrayList{
	private:
		std::vector< 
			std::variant< 
				unique_ptr<
					SingleElement<VAR_DTYPE>>, 
					unique_ptr<ArrayList<VAR_DTYPE>
					>
				>
			> arrayList;
		
		std::vector<size_t> dimensions;	
		size_t totalElementsAllocated;
		
		static std::unique_ptr<ArrayList<VAR_DTYPE>>
		_arrayListBuilder( 
						std::vector<VALUE_TOKENS>& arrayTokenList, 
						size_t& curIndex, 
						std::queue<VAR_DTYPE>& valueQueue
					){

			auto newArrayList = make_unique< ArrayList<VAR_DTYPE> >();
			while( curIndex < arrayTokenList.size() ){
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::COMMA ){
					curIndex++;
					continue;
				}

				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_OPEN ){
					curIndex++ 	  ; 

					unique_ptr<ArrayList<VAR_DTYPE>> array = _arrayListBuilder( arrayTokenList, curIndex, valueQueue );
					newArrayList->push_ArrayList( move( array ) );
				}

				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_VALUE ){
					auto value = move ( valueQueue.front() );
					valueQueue.pop( );

					auto newVariable = make_unique<SingleElement<VAR_DTYPE>>();
					newVariable->assignValue( move( value ) );
					newArrayList->push_SingleElement( move (newVariable ));
				}

				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_CLOSE )
					return newArrayList;
				
				curIndex++;
			}
			throw InvalidSyntaxError("encounter invalid syntax in array creation");
		}

	public:

		static unique_ptr<ArrayList<VAR_DTYPE>> 
		createArray( std::vector<VALUE_TOKENS>& arrayTokenList,  size_t& currentPointer, std::queue<VAR_DTYPE>& ValueQueue ){
			unique_ptr<ArrayList> arrayResult = ArrayList::_arrayListBuilder( arrayTokenList, currentPointer, ValueQueue );	
			return arrayResult;
		}

		void push_SingleElement( unique_ptr<SingleElement<VAR_DTYPE>> singleElement ){
			this->arrayList.push_back( move( singleElement ) );
		}

		void push_ArrayList( unique_ptr<ArrayList<VAR_DTYPE>> arrayListElement ){
			this->arrayList.push_back( move( arrayListElement ) );	
		}

		std::vector< 
			std::variant< 
				unique_ptr<
					SingleElement<VAR_DTYPE>>, 
					unique_ptr<ArrayList<VAR_DTYPE>
					>
				>
			> &
		getList( void ) {
			return this->arrayList;
		}

		variant< SingleElement<VAR_DTYPE>*, ArrayList<VAR_DTYPE>* >
		getAtIndex( vector<size_t>& index, size_t start = 0 ){
			if( index.size() == 0 )
				throw InvalidSyntaxError( "No array indices is specified" );
			
			size_t current = index[ start ];

			if( current < this->arrayList.size() ){
				auto& currentData = this->arrayList[ current ];
				
				if( holds_alternative<unique_ptr<SingleElement<VAR_DTYPE>>>( currentData ) ){
					auto& currentIndexData = get<unique_ptr<SingleElement<VAR_DTYPE>>>( currentData );
					
					if( start == index.size() - 1 )
						return currentIndexData.get();

					else throw ArrayOutOfBound( "Indices reaches its limit" );
				}
				else{
					auto& currentIndexData = get<unique_ptr<ArrayList<VAR_DTYPE>>>( currentData );

					if( start == index.size() - 1 )
						return currentIndexData.get();
					
					return currentIndexData->getAtIndex( index, start + 1 );
				}
			}
			throw InvalidSyntaxError( "Invalid Syntax in Array bouding" );
		}

};

template <typename VAR_DTYPE>
struct VARIABLE_HOLDER{
	std::string key;
	bool isTypeArray;
	bool isValueAssigned;

	std::variant<
		unique_ptr<SingleElement<VAR_DTYPE>>,
		unique_ptr<ArrayList<VAR_DTYPE>>
	> value;
};

/* This function convert the varible string tokens to enum tokens */
pair<
	pair<
		vector<VARIABLE_TOKENS>, 
		vector<VALUE_TOKENS>
	>,
	pair<
		queue<string>,
		vector<variant<string, vector<string>>>
	>
> 
codeToTokens( const vector<string>& tokens, size_t& startCurPtr ){
	vector<VARIABLE_TOKENS> varTokens;
	vector<VALUE_TOKENS> 	valueToken;

	vector<
		variant<
			string, 
			vector<string>
		>
	> valueVector;
	
	queue<string> VarQueue;

	bool isVariableTurn 	= true;
	int arrayOpenedCount 	= 0;

	for( ; startCurPtr < tokens.size(); ++startCurPtr ) {
		string curToken = tokens[ startCurPtr ];

		if( curToken == "pidi" ){
			varTokens.push_back( VARIABLE_TOKENS::VAR_START );
		}
		else if( curToken == ","){
			( isVariableTurn ) ?  varTokens.push_back( VARIABLE_TOKENS::COMMA )
							   : valueToken.push_back( VALUE_TOKENS::COMMA );
		}
		else if( curToken == "kootam" ){
			varTokens.push_back( VARIABLE_TOKENS::ARRAY );
		}
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
			return { { varTokens, valueToken }, { VarQueue, valueVector } };
		}
		else{
			if( isVariableTurn ){
				VarQueue.push( tokens[ startCurPtr ] );
				varTokens.push_back( VARIABLE_TOKENS::NAME );
			}
			else{
				vector<string> curValueVector;
				int openBrack = 0;
				while( startCurPtr < tokens.size() ){
					if( tokens[ startCurPtr ] == "(" )
						openBrack++;
					else if( tokens[startCurPtr] == ")")
						openBrack--;

					if( isRegisteredVariableToken( tokens[ startCurPtr ] )  && openBrack == 0 ){
						startCurPtr--;
						break;
					}
					curValueVector.push_back( tokens[ startCurPtr++ ] );
				}

				if( curValueVector.size() > 1 )
					valueVector.push_back( curValueVector );
				else 
					valueVector.push_back( curValueVector.back() );

				if( arrayOpenedCount > 0 )
					valueToken.push_back( VALUE_TOKENS::ARRAY_VALUE );
				else 
					valueToken.push_back( VALUE_TOKENS::NORMAL_VALUE );
			}
		}	
	}
	throw InvalidSyntaxError( "; didn't hit this token, hope its an erro :) " );
}

template <typename VAR_DTYPE> bool 
isValidVariableSyntax( 
					vector<VARIABLE_TOKENS>& varTokens, 
					vector<
						unique_ptr<VARIABLE_HOLDER<VAR_DTYPE>>
					>& variableStack ,
					queue<string>& VarQueue
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
						string variableName = VarQueue.front();
						if( !DtypeHelper::isValidVariableName( variableName ) ){
							throw InvalidNameError( variableName );
						}
							
						unique_ptr<VARIABLE_HOLDER<VAR_DTYPE>> newVariable = make_unique<VARIABLE_HOLDER<VAR_DTYPE>>();
						newVariable->key = variableName;
						newVariable->isValueAssigned = false;

						VarQueue.pop( );
						variableStack.push_back( move( newVariable ));
					}
					break;
				}
				case VARIABLE_TOKENS::ARRAY: {
					if( variableStack.empty() )
						return false;
					else{
						variableStack.back()->isTypeArray = true;
					}
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
			vector<unique_ptr<VARIABLE_HOLDER<VAR_DTYPE>>>& variable,
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
					auto newArray = ArrayList<VAR_DTYPE>::createArray( 
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

					auto value = move( ValueQueue.front() );
					auto newElement = make_unique<SingleElement<VAR_DTYPE>>();
					
					newElement->assignValue( move( value ) );

					ValueQueue.pop();
					topVariable->value = move( newElement );
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

#endif