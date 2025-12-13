#include <iostream>
#include <variant>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>

using namespace std;

/*
   1) print complex arrays, 
   2) make this code small
   3) varible name check
   4) condition checks
   5) error handling
   6) typecasting
*/

using VarDtype = variant<string, long int, double, bool>;

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
	{ VALUE_TOKENS::ARRAY_OPEN,  { VALUE_TOKENS::ARRAY_OPEN, VALUE_TOKENS::ARRAY_VALUE } 	},
	{ VALUE_TOKENS::ARRAY_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::ARRAY_CLOSE } 	   	},
	{ VALUE_TOKENS::COMMA, 	  	 { VALUE_TOKENS::ARRAY_VALUE, VALUE_TOKENS::ARRAY_OPEN, 
								   VALUE_TOKENS::NORMAL_VALUE } 							},
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
	public:
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

		void assignValue(const VarDtype& value ){
			this->data 	= value;
			this->isValueDefined = true;
		}

		optional<VarDtype> getValue( void ) const {
			DTYPES realType = this->getValueDtypes( );
			if( realType == DTYPES::VALUE_NOT_DEFINED )
				return nullopt;
			return data;
		}
};

static int curly_opened = 0;

class ArrayList{
	public:
		variant<vector<SingleElement>, vector<ArrayList>> arrayList;
		vector<size_t> dimensions;	
		size_t totalElementsAllocated;


		static unique_ptr<ArrayList> _arrayListBuilder( 
												vector<VALUE_TOKENS>& arrayTokenList, 
												size_t& curIndex, 
												queue<string>& valueQueue
		) {

			auto newArrayList = make_unique< ArrayList >();
			vector<SingleElement> resultList;
			
			while( curIndex < arrayTokenList.size() ){
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::COMMA ){
					curIndex++;
					continue;
				}
				// pidi test kootam = {{{1, 2, 3}, {3, 4}}, {3, 4}};

				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_OPEN ){
					curIndex++; curly_opened++;

					unique_ptr<ArrayList> array = _arrayListBuilder( arrayTokenList, curIndex, valueQueue );
					newArrayList->push_ArrayList( *array );

					if( !curly_opened )
						return newArrayList;
				}

				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_VALUE ){
					string value = valueQueue.front();
					valueQueue.pop();

					auto newVariable = make_unique<SingleElement>();
					newVariable->assignValue( value );

					resultList.push_back( *newVariable );
				}

				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_CLOSE ){
					curly_opened--;
					if( resultList.size() > 0 )
						newArrayList->push_SingleElement( resultList );
					return newArrayList;
				}
				curIndex++;
			}
			return newArrayList;

		}

	public:
		static ArrayList createArray( vector<VALUE_TOKENS>& arrayTokenList, size_t& currentPointer ){
			unique_ptr<ArrayList> arrayResult = _arrayListBuilder( arrayTokenList, currentPointer,  ValueQueue );	
			return *arrayResult;
		}

		void push_SingleElement( vector<SingleElement>& singleElementList ){
			this->arrayList = singleElementList;
		}

		void push_ArrayList( ArrayList& arrayListElement ){
			auto* vec = get_if<vector<ArrayList>>( &arrayList );
		    if ( !vec ){
		    	vector<ArrayList> newArrayList = { arrayListElement };
		    	this->arrayList = newArrayList;
		    }
		    else vec->push_back(arrayListElement);		    	
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

/*=======================================================================*/

void printArray( ArrayList& array, size_t row ){
	auto arr = get_if<vector<ArrayList>> ( &array.arrayList );
	if( arr ){
		cout << "ARRAY LIST\n";
		for(ArrayList& ar: *arr)
			printArray( ar, row + 1 );
	}
	else{
		auto sarr = get_if<vector<SingleElement>> ( &array.arrayList );
		cout << "ROW: " << row << endl;
		for(SingleElement& elemData: *sarr){
			std::visit([](auto &val) {
	       		cout << val << " ";
	    	}, elemData.data);
		}
		cout << endl;
	}
}

void printVariable( VARIABLE data ){
	variant<SingleElement, ArrayList> value = data.value;
	cout << "Variable Name: " << data.variableName << endl;

	if( auto ptr = get_if<SingleElement>(&value) ){		
		cout << "Value: ";
		std::visit([](auto &val) {
       		cout << val << "\n";
    	}, ptr->data);


	}else{
		auto ptr1 = get_if<ArrayList> (&value);
		printArray( *ptr1, 0 );
	}
}

/*======================================================================*/

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
				break;
				
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
				break;
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

		if( !continueChecking ){
			cout << (int)valueTokens[currentPointer - 1] << endl;
			cout << "need " << (int)valueTokens[ currentPointer ] << endl;
			break;
		}
	}

	return currentStage == VALUE_TOKENS::VALUE_END;
}


// pidi test = {{ {1, 2, 3}, {3, 4} }, {3, 4}}

vector<vector<string>> tests = {
	{"pidi", "test1", "=", "325345", ";"},
	{"pidi", "adminva", "=", "adminvl", ";"},
	{"pidi", "test2", ",", "admin", "=", "2332", ",", "345", ";"},
	{"pidi", "test3", "kootam", "=", "{", "asg", ",", "sadg", "}", ";"},

	{ "pidi", "test", "kootam", "=", "{", "{", "{", "1", ",", "2", ",", "3", ",", "}", ",",
		"{", "3", ",", "4", "}", "}", ",", "{", "3", ",", "4", ",", "}", "}", ";" },
	{ "pidi", "admin", ",", "error", "kootam", ",", "test", "=", "345.45", ",",
		"{", "{", "1", ",", "3", "}", "}", ",", "true", ";" }

};

// pidi admin, error kootam, test = 345.45, {{1, 3}}, true;

int main(){
	int round = 1;
	for(vector<string> test: tests){
		cout << "ROUND " << round << endl;
		cout << test[1] << endl;

		while( !ValueQueue.empty() ){
			ValueQueue.pop();
		}

		while( !VarQueue.empty() ){
			VarQueue.pop();
		}

		size_t index = 0;

		auto res = codeToTokens( test, index );
		vector<VALUE_TOKENS>r = res.second;
		vector<VARIABLE> var;
		
		size_t ind = 0;
		auto te = codeToTokens( test, ind );

		vector<VARIABLE_TOKENS> first = te.first;
		vector<VALUE_TOKENS> second = te.second;

		bool a, b;

		if( (a = isValidVariableSyntax( first, var )) && (b = isValidValueSyntax( second, var ))){
			cout << "VALID" << endl;
			for(int x = 0; x < var.size(); x++)
				printVariable( var[x] );
		}else {
			if( !a ){
				cout << "INvalid variable\n";
			}else cout << "invalid value\n";
			cout << "INVALID SYNTAX\n";
		}
		cout << '\n';
		round++;
	}
	return 0;	
}
