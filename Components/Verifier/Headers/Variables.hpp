#ifndef VARIABLES_HPP
#define VARIABLES_HPP

#include <iostream>
#include <variant>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <cctype>
#include <unordered_set>

#include "../../../Headers/MBExceptions.hpp"
#include "../../../Headers/Dtypes.hpp"
#include "../../../Headers/Tokenizer.hpp"

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

extern const std::unordered_set<std::string_view> REGISTERED_TOKENS;

bool isRegisteredVariableToken( const std::string& token );

struct VariableTokens{
	std::vector<VARIABLE_TOKENS> varTokens;   // LHS Token
	std::vector<VALUE_TOKENS> 	valueTokens; // RHS Token
	std::vector<std::vector<Token>>	valueVector;
	std::queue<Token> 			VarQueue;

	VariableTokens() = default;

	// constructor
	VariableTokens(
		std::vector<VARIABLE_TOKENS> varTokens,
		std::vector<VALUE_TOKENS> 	valueTokens,
		std::vector<std::vector<Token>>	valueVector,
		std::queue<Token>			VarQueue
	){
		this->varTokens   = varTokens;
		this->valueTokens = valueTokens;
		this->valueVector = valueVector;
		this->VarQueue 	  = VarQueue;
	}
};

enum class ARRAY_ACCESS{
	NOTHING 		,  // 0
	VAR_NAME 		,  // 1
	BRACK_OPEN 		,  // 2
	INDEX_VECTOR 	,  // 3
	BRACK_CLOSE	 	,  // 4
	PROPERTY_ACCESS ,  // 5
	PROPERTY_NAME	,  // 6
	END 			   // 7
};

struct ArrayAccessTokens{
	std::vector<ARRAY_ACCESS> tokens;
	std::string arrayName;
	std::vector<std::vector<Token>> indexVector;
	bool isTouchedArrayProperty = false;
	std::string arrProperty;

	ArrayAccessTokens( 
		std::vector<ARRAY_ACCESS> tokens,
		std::string arrayName,
		std::vector<std::vector<Token>> indexVector
	){
		this->tokens 	  = tokens;
		this->arrayName   = arrayName;
		this->indexVector = indexVector;
	}

	void setArrayProperty( std::string arrProp ){
		this->isTouchedArrayProperty = true;
		this->arrProperty = arrProp;
	}
};

template <typename ARRAY_SUPPORT_TYPES>
class ArrayList: public std::enable_shared_from_this<ArrayList<ARRAY_SUPPORT_TYPES>>{
	public:
		std::vector< std::variant< ARRAY_SUPPORT_TYPES, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>> arrayList;
		std::vector<size_t> dimensions;	
		size_t totalElementsAllocated = 0;
		
		template <typename DEEP_VALUE>
		static std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>
		_arrayListBuilder( std::vector<VALUE_TOKENS>& arrayTokenList, size_t& curIndex, std::queue<DEEP_VALUE>& valueQueue ){
			auto newArrayList = std::make_shared< ArrayList<ARRAY_SUPPORT_TYPES> >();
			while( curIndex < arrayTokenList.size() ){
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::COMMA ){
					curIndex++;
					continue;
				}
				if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_OPEN ){
					curIndex++ 	  ; 
					std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> array = _arrayListBuilder( arrayTokenList, curIndex, valueQueue );
					newArrayList->push_ArrayList( array );
				}
				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_VALUE ){
					DEEP_VALUE value = valueQueue.front();
					valueQueue.pop( );
					newArrayList->push_SingleElement( value );
				}
				else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_CLOSE )
					return newArrayList;
				curIndex++;
			}
			throw InvalidSyntaxError("encounter invalid syntax in array creation");
		}

		template <typename DEEP_VALUE>
		static std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>
		createArray( std::vector<VALUE_TOKENS>& arrayTokenList,  size_t& currentPointer, std::queue<DEEP_VALUE>& ValueQueue ){
			std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> arrayResult = ArrayList<ARRAY_SUPPORT_TYPES>::_arrayListBuilder( arrayTokenList, currentPointer, ValueQueue );	
			return arrayResult;
		}

		template <typename DEEP_VALUE>
		void push_SingleElement( DEEP_VALUE singleElement ){
			this->totalElementsAllocated++;
			visit( [&]( auto&& data ){ this->arrayList.push_back( data ); }, singleElement );
		}
		
		std::variant<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>, ARRAY_SUPPORT_TYPES>
		getElementAtIndex(std::vector<long int>& dimensions, size_t index = 0){
			if( dimensions.empty() )
				return this->shared_from_this();

			auto curIndex = dimensions[ index ];
			if( curIndex >= 0 && curIndex < this->arrayList.size() ){
				auto& test = this->arrayList[ curIndex ];

				if( index == dimensions.size() - 1 ){
					if ( std::holds_alternative<ARRAY_SUPPORT_TYPES>( test ) )
						return std::get<ARRAY_SUPPORT_TYPES>( test );

					else if( std::holds_alternative< std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> >(test) )
						return std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( test );
					
					else throw std::runtime_error("Unkown Dtype");
				}

				if ( std::holds_alternative<ARRAY_SUPPORT_TYPES>( test ) ){
					auto data = std::get<ARRAY_SUPPORT_TYPES>( test );

					if( !std::holds_alternative<VarDtype>( data ) || index != dimensions.size() - 2 )
						throw InvalidSyntaxError("Only Array Can Access Using Index");
					
					auto vData = std::get<VarDtype>( data );

					if( !std::holds_alternative<std::string>( vData ) )
						throw InvalidSyntaxError("Only Array Can Access Using Index");
					
					auto stringData = std::get<std::string>( vData );
					auto ch = std::string(1, stringData[dimensions.back()]);
					return ARRAY_SUPPORT_TYPES{ VarDtype(ch) };
				}

				if( std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>(test) ){
					auto finalData = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>(test);
					return finalData->getElementAtIndex( dimensions, index+1 );
				}
				else throw std::runtime_error("Unkown Dtype");
			} 
			else throw ArrayOutOfBound(std::to_string(curIndex));
		}

		static std::variant< ARRAY_SUPPORT_TYPES, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>
		removeElementAtIndex(std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>& arrList, long int index){
			if( index >= 0 && arrList->arrayList.size() > index ){
				auto cur = arrList->arrayList[index];
				arrList->arrayList.erase( arrList->arrayList.begin() + index );
				arrList->totalElementsAllocated--;
				return cur;
			}
			else throw ArrayOutOfBound(std::to_string(index));
		}

		void push_ArrayList( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> arrayListElement ){
			this->totalElementsAllocated++;
			this->arrayList.push_back( arrayListElement );	
		}
};

template <typename ARRAY_SUPPORT_TYPES>
struct VARIABLE_HOLDER{
	std::string key;
	bool isTypeArray;
	std::variant<VarDtype,std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>> value;
};
struct VAR_INFO{
	std::string varName;
	bool isTypeArray;

	VAR_INFO( std::string varName, bool isTypeArray ){
		this->varName 	  = varName;
		this->isTypeArray = isTypeArray;
	}
};

VariableTokens stringToVariableTokens( const std::vector<Token>& tokens, size_t& startCurPtr, bool isVariableTurn = true );
void passValidVarDeclaration( std::vector<VARIABLE_TOKENS>& varTokens, std::vector<VAR_INFO>& variableStack , std::queue<Token>& VarQueue );
void passValidValueTokens( std::vector<VALUE_TOKENS>& valueTokens );
ArrayAccessTokens stringToArrayAccesToken( const std::vector<Token>&tokens, size_t& currentPtr );
void passArrayAccessToken( std::vector<ARRAY_ACCESS>& tokens );

#endif