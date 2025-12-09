#include <iostream>
#include <variant>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>

using namespace std;

using VarDtype = variant<string, long int, double, bool>;


enum class DTYPES
{
	VALUE_NOT_DEFINED, 
	STRING 			 , 
	INT 			 , 
	DOUBLE 			 , 
	BOOLEAN
};

enum class VALUE_TOKENS
{
	ARRAY_OPEN 	,
	ARRAY_VALUE ,
	NORMAL_VALUE,
	COMMA 		,
	ARRAY_CLOSE ,
	VALUE_END
};

enum class VARIABLE_TOKENS
{
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
		bool isValueDefined = false;
		VarDtype data;
	
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

		unique_ptr<ArrayList> _arrayListBuilder( vector<VALUE_TOKENS>& arrayTokenList, size_t curIndex, unordered_map<VALUE_TOKENS, queue<SingleElement>>& Values ) 
		{
			auto newArrayList = make_unique< ArrayList >();
			vector<SingleElement> resultList;
			
			while( curIndex < arrayTokenList.size() ){
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::COMMA )
				{
					continue;
				}
				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_OPEN )
				{
					unique_ptr<ArrayList> array = _arrayListBuilder( arrayTokenList, curIndex + 1, Values );
					newArrayList->push_ArrayList( *array );
				}
				else if(
				 	auto value = Values.find( arrayTokenList[ curIndex ] );
				 	value != Values.end() 
				 )
				{
					auto data = value->second.front( );
					value->second.pop( );
					resultList.push_back( data );
				}
				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_CLOSE )
				{
					newArrayList->push_SingleElement( resultList );
					return newArrayList;
				}
			}
			return newArrayList;

		}

	public:
		void createArray( vector<VALUE_TOKENS>& arrayTokenList ){
			unordered_map<VALUE_TOKENS, queue<SingleElement>> values;
			_arrayListBuilder( arrayTokenList, 0,  values );	
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

int main(){
	vector<string> test =  { "pidi", "name", "kootam", "=", "{", "sdf", ",", "{", "}", "34", "}", ",", "sd", ";" };
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
}