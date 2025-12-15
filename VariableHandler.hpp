#ifndef VARIABLEHANDLER_HPP
#define VARIABLEHANDLER_HPP

#include <iostream>
#include <variant>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>
#include <cctype>

#include "MBExceptions.hpp"
#include "Dtypes.hpp"

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

enum class FLAGS{
	IS_TYPE_ARRAY   = 1 << 0, 
	IS_VALUE_FILLED = 1 << 1,
	IS_PRIVATE		= 1 << 2,
};

std::queue<std::string> VarQueue, ValueQueue;

std::unordered_map <VARIABLE_TOKENS, std::vector<VARIABLE_TOKENS>> VARIABLE_DECLARE_GRAPH = {
	{ VARIABLE_TOKENS::VAR_START, 	{ VARIABLE_TOKENS::NAME } 								  },

	{ VARIABLE_TOKENS::NAME,		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::ARRAY, VARIABLE_TOKENS::VAR_ENDS }     },

	{ VARIABLE_TOKENS::ARRAY, 		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::VAR_ENDS } 		 					  },

	{ VARIABLE_TOKENS::COMMA, 		{ VARIABLE_TOKENS::NAME } 						  		  }	
};

std::unordered_map <VALUE_TOKENS, std::vector<VALUE_TOKENS>> VALUE_ASSIGN_GRAPH = {
	{ VALUE_TOKENS::NORMAL_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END } 			},
	{ VALUE_TOKENS::ARRAY_OPEN,  { VALUE_TOKENS::ARRAY_OPEN, VALUE_TOKENS::ARRAY_VALUE } 	},
	{ VALUE_TOKENS::ARRAY_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::ARRAY_CLOSE } 	   	},
	{ VALUE_TOKENS::COMMA, 	  	 { VALUE_TOKENS::ARRAY_VALUE, VALUE_TOKENS::ARRAY_OPEN, 
								   VALUE_TOKENS::NORMAL_VALUE } 							},
	{ VALUE_TOKENS::ARRAY_CLOSE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END }		   	}
};


class SingleElement{
	public:
		VarDtype data;
		bool isValueDefined = false;
		DTYPES realType = DTYPES::VALUE_NOT_DEFINED;
	
	public:
		void assignType( 
				DTYPES type 
				);

		void assignValue(
			const VarDtype& value 
			);

		std::optional<VarDtype> getValue( void ) const;

};

static int curly_opened = 0;
class ArrayList{
	private:
		std::variant<std::vector<SingleElement>, std::vector<ArrayList>> arrayList;
		std::vector<size_t> dimensions;	
		size_t totalElementsAllocated;


		static std::unique_ptr<ArrayList> _arrayListBuilder( 
									std::vector<VALUE_TOKENS>& arrayTokenList, 
									size_t& curIndex, 
									std::queue<std::string>& valueQueue
									); 
	public:

		static ArrayList createArray( 
				std::vector<VALUE_TOKENS>& arrayTokenList, 
				size_t& currentPointer 
				);

		void push_SingleElement( 
				std::vector<SingleElement>& singleElementList 
				);
		
		void push_ArrayList( 
				ArrayList& arrayListElement 
				);
		
		void printArray( void ) const;
};

namespace VARIABLE_HANDLER{
	std::pair<std::vector<VARIABLE_TOKENS>, std::vector<VALUE_TOKENS>> codeToTokens( 
							std::vector<std::string>& tokens, 
							size_t& curIndexPtr 
							);

	struct VARIABLE{
		std::string variableName;
		unsigned int varStatus 	;
		DTYPES variableType 	;
		std::variant<SingleElement, ArrayList> value;

		void printVariable( void );
	};


	bool isValidVariableSyntax( 
			std::vector<VARIABLE_TOKENS>& varTokens, 
			std::vector<VARIABLE>& variableStack 
			);

	bool isValidValueSyntax( 
			std::vector<VALUE_TOKENS>& valueTokens, 
			std::vector<VARIABLE>& variable 
			);

	std::unique_ptr<VARIABLE> getNewVariable( std::string varName );
};

#endif