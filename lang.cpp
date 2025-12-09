#include <iostream>
#include <variant>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>

/************************* RUN TESTS ********************************/

using namespace std;

using VarDtype = variant<string, long int, double, bool>;
struct VARIABLE;

enum class DTYPES{
	VALUE_NOT_DEFINED, 
	STRING 			 , 
	INT 			 , 
	DOUBLE 			 , 
	BOOLEAN
};

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

queue<string> VarQueue, ValueQueue;

// variable syntax handling...
unordered_map <VARIABLE_TOKENS, vector<VARIABLE_TOKENS>> VARIABLE_DECLARE_GRAPH = {
	{ VARIABLE_TOKENS::VAR_START, 	{ VARIABLE_TOKENS::NAME } 								  },

	{ VARIABLE_TOKENS::NAME,		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::ARRAY, VARIABLE_TOKENS::VAR_ENDS }     },

	{ VARIABLE_TOKENS::ARRAY, 		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::VAR_ENDS } 		 					  },

	{ VARIABLE_TOKENS::COMMA, 		{ VARIABLE_TOKENS::NAME } 						  		  }	
};

// value syntax handling...
unordered_map <VALUE_TOKENS, vector<VALUE_TOKENS>> VALUE_ASSIGN_GRAPH = {
	{ VALUE_TOKENS::NORMAL_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END } 			},
	{ VALUE_TOKENS::COMMA, 		  { VALUE_TOKENS::NORMAL_VALUE }							}, 
	// ARRAY CASE
	{ VALUE_TOKENS::ARRAY_OPEN,  { VALUE_TOKENS::ARRAY_OPEN, VALUE_TOKENS::ARRAY_VALUE } 	},
	{ VALUE_TOKENS::ARRAY_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::ARRAY_CLOSE } 	   	},
	{ VALUE_TOKENS::COMMA, 	  	 { VALUE_TOKENS::ARRAY_VALUE, VALUE_TOKENS::ARRAY_OPEN } 	},
	{ VALUE_TOKENS::ARRAY_CLOSE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END }		   	}
};

enum class FLAGS{
	IS_TYPE_ARRAY   = 1 << 0, 
	IS_VALUE_FILLED = 1 << 1,
	IS_PRIVATE		= 1 << 2,
};

pair<vector<VARIABLE_TOKENS>, vector<VALUE_TOKENS>> codeToTokens( vector<string>& tokens, size_t& curIndexPtr ){
	int startCurPtr = curIndexPtr;

	vector<VARIABLE_TOKENS> varTokens;
	vector<VALUE_TOKENS> 	valueToken;

	bool isVariableTurn = true;
	int arrayOpenedCount = 0;

	for( ; startCurPtr < tokens.size(); ++startCurPtr ) {
		string curToken = tokens[ startCurPtr ];

		if( curToken == "pidi" ){
			varTokens.push_back( VARIABLE_TOKENS::VAR_START );
		}
		
		else if( curToken == ","){
			( isVariableTurn ) ? 
				varTokens.push_back( VARIABLE_TOKENS::COMMA )
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
			( isVariableTurn ) ? 
				varTokens.push_back( VARIABLE_TOKENS::VAR_ENDS )
				: valueToken.push_back( VALUE_TOKENS::VALUE_END );
		}
		else{
			if( isVariableTurn ){
				VarQueue.push( tokens[ startCurPtr ] );
				varTokens.push_back( VARIABLE_TOKENS::NAME );
			}else{
				ValueQueue.push( tokens[ startCurPtr ] );
				if( arrayOpenedCount > 0 )
					valueToken.push_back( VALUE_TOKENS::ARRAY_VALUE );
				else valueToken.push_back( VALUE_TOKENS::NORMAL_VALUE );
			}
		}		
	}
	return { varTokens, valueToken };
}


class SingleElement{
	private:
		VarDtype data;
		bool isValueDefined = false;
	
	public:
		DTYPES getValueDtypes( void ) 
		const {
			if( !isValueDefined )
				return DTYPES::VALUE_NOT_DEFINED;
			
			else if( holds_alternative<string>( this->data ) )
				return DTYPES::STRING;

			else if( holds_alternative<long int> ( this->data ) )
				return DTYPES::INT;

			else if( holds_alternative<double> ( this->data ) )
				return DTYPES::DOUBLE;

			else return DTYPES::BOOLEAN;
		}

		void assignValue(const VarDtype& value )
		{
			this->data 	= value;
			this->isValueDefined = true;
		}
		
		template <typename Dtype>
		optional<Dtype> getValue( void ) 
		const {
			DTYPES realType = this->getValueDtypes( );
			
			if( realType == DTYPES::VALUE_NOT_DEFINED )
				return nullopt;

			if( auto data = get_if<Dtype>( &this->data ) )
				return *data;

			return nullopt;
		}
};

class ArrayList{
	private:
		variant<vector<SingleElement>, vector<ArrayList>> arrayList;
		vector<size_t> dimensions;	
		size_t totalElementsAllocated;

		static unique_ptr<ArrayList> _arrayListBuilder( 
												vector<VALUE_TOKENS>& arrayTokenList, 
												size_t curIndex, 
												queue<string>& valueQueue
		) {
			auto newArrayList = make_unique< ArrayList >();
			vector<SingleElement> resultList;
			
			while( curIndex < arrayTokenList.size() ){
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::COMMA ){
					continue;
				}

				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_OPEN ){
					unique_ptr<ArrayList> array = _arrayListBuilder( arrayTokenList, curIndex + 1, valueQueue );
					newArrayList->push_ArrayList( *array );
				}

				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_VALUE ){
					string value = valueQueue.front();
					valueQueue.pop();

					auto newVariable = make_unique<SingleElement>();
					newVariable->assignValue( value );

					resultList.push_back( *newVariable );
				}

				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_CLOSE ){
					newArrayList->push_SingleElement( resultList );
					return newArrayList;
				}
			}
			return newArrayList;

		}

	public:
		static ArrayList createArray( vector<VALUE_TOKENS>& arrayTokenList, size_t& currentPointer ){
			return *_arrayListBuilder( arrayTokenList, currentPointer,  ValueQueue );	
		}

		void push_SingleElement( vector<SingleElement>& singleElementList ){
			arrayList = singleElementList;
		}

		void push_ArrayList( ArrayList& arrayListElement ){
			auto* vec = std::get_if<std::vector<ArrayList>>(&arrayList);

		    if (!vec) return;
		    vec->push_back(arrayListElement);
		}

};

struct VARIABLE{
	string variableName		;
	unsigned int varStatus 	;
	DTYPES variableType 	;
	variant<SingleElement, ArrayList> value;
};

unique_ptr<VARIABLE> getNewVariable( string varName ){
	auto newVariable = make_unique<VARIABLE>( );
	
	newVariable->variableName = varName;
	newVariable->varStatus 	  = ( unsigned int ) FLAGS::IS_PRIVATE;
	newVariable->variableType = DTYPES::VALUE_NOT_DEFINED;

	return newVariable;
}

bool isValidVariableSyntax( vector<VARIABLE_TOKENS>& varTokens, vector<VARIABLE>& variableStack ){
	if( !varTokens.size() )
		return false;

	VARIABLE_TOKENS currentStage = VARIABLE_TOKENS::VAR_START;
	size_t currentPointer		 = 0;

	while( currentStage == varTokens[ currentPointer ] ){		
		if( currentStage == VARIABLE_TOKENS::VALUE_ASSIGN )
			break;

		switch( varTokens[ currentPointer ] ){
			case VARIABLE_TOKENS::NAME: {
				if( VarQueue.empty() ){
					// raise syntax error
					return false;
				}
				else{
					auto newVariable = getNewVariable( VarQueue.front( ) );
					VarQueue.pop( );
					variableStack.push_back( *newVariable );
				}
				break;
			}

			case VARIABLE_TOKENS::ARRAY:{
				if( variableStack.empty() ){
					//raise syntax error
					return false;
				}
				else{
					VARIABLE& backStack = variableStack.back();
					backStack.varStatus |= ( unsigned int ) FLAGS::IS_TYPE_ARRAY;
				}
				break;
			}
		}

		if( currentPointer + 1 >= varTokens.size() )
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
	return currentStage == VARIABLE_TOKENS::VALUE_ASSIGN;
}

bool isValidValueSyntax( vector<VALUE_TOKENS>& valueTokens, vector<VARIABLE>& variable ){
	if( !valueTokens.size() )
		return false;

	VALUE_TOKENS currentStage = VALUE_TOKENS::NORMAL_VALUE;
	size_t currentPointer 	  = 0;
	size_t updatedVariable 	  = 0;

	if( valueTokens[ currentPointer ] == VALUE_TOKENS::ARRAY_OPEN )
		currentStage = VALUE_TOKENS::ARRAY_OPEN;

	while( valueTokens[ currentPointer ] == currentStage ){
		if( currentStage == VALUE_TOKENS::VALUE_END )
			break;

		if( updatedVariable >= variable.size() ){
			// raise exception
			return false;
		}

		switch( valueTokens[ currentPointer ] ){
			case VALUE_TOKENS::ARRAY_OPEN: {
				
				VARIABLE& topVariable = variable[ updatedVariable ];

				if( !( topVariable.varStatus & (unsigned int) FLAGS::IS_TYPE_ARRAY ) ){
					// raise exeption; given type is not an array
					return false;
				}
				
				ArrayList newArray = ArrayList::createArray( valueTokens, currentPointer );
				
				topVariable.value = newArray;				
				updatedVariable++;
				
			}
			case VALUE_TOKENS::NORMAL_VALUE:{
				VARIABLE& topVariable = variable[updatedVariable];

				if( topVariable.varStatus & (unsigned int) FLAGS::IS_TYPE_ARRAY ){
					// raise exeption; given type is an array
					return false;
				}

				if( ValueQueue.empty() ){
					// no variable found to update
					return false;
				}

				auto newElement = make_unique<SingleElement>();
				newElement->assignValue( ValueQueue.front() );

				ValueQueue.pop();

				topVariable.value = *newElement;
				updatedVariable++;
			}
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
	cout << (bool) (currentStage == VALUE_TOKENS::VALUE_END) << endl;

	return currentStage == VALUE_TOKENS::VALUE_END;
}

int main(){
	vector<string> test =  { "pidi", "test", "=", "23325.3454", ";" };
	size_t index = 0;
	auto res = codeToTokens( test, index );
	vector<VARIABLE_TOKENS>t = res.first;
	for(int x = 0; x < t.size(); x++){
		cout << (int)t[x] << endl;
	}

	cout << '\n';
	vector<VALUE_TOKENS>r = res.second;
	for(int x = 0; x < r.size(); x++){
		cout << (int)r[x] << endl;
	}

	cout << VarQueue.size() << ' ' << ValueQueue.size() << endl;

	vector<VARIABLE> var;
	size_t ind = 0;
	auto te = codeToTokens( test, ind );
	vector<VARIABLE_TOKENS> first = te.first;
	vector<VALUE_TOKENS> second = te.second;

	if( isValidVariableSyntax( first, var ) && isValidValueSyntax( second, var )){
		VARIABLE test = var[0];
		cout << test.variableName << endl;
	}

	
}