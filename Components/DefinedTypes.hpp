/* MAP ITEM AND ITS HELPER CLASSES */

#ifndef DEFINEDVALUES_HPP
#define DEFINEDVALUES_HPP

#include <string>
#include <optional>
#include <memory>
#include <iostream>
#include <type_traits>

#include "Verifier/Variables.hpp"
#include "Verifier/Conditional.hpp"
#include "Verifier/Function.hpp"
#include "Verifier/loops.hpp"
#include "Verifier/InputOutput.hpp"
#include "Verifier/Instruction.hpp"

#include "AST.hpp"

#include "../Headers/Dtypes.hpp"
#include "../Headers/MBExceptions.hpp"
#include "../Headers/Tokenizer.hpp"

enum class MAPTYPE{ VARIABLE, ARRAY_PTR, FUNC_PTR };

template<typename T>
inline constexpr bool is_number_v = std::is_same_v<T, long int> || std::is_same_v<T, double> || std::is_same_v<T, bool>;

struct MapItem;
class VAR_VMAP;

struct FUNCTION_MAP_DATA{
	std::string funcName;
	size_t		argsSize;
	size_t 		bodyStartPtr;
	size_t 		bodyEndPtr;

	std::vector<std::unique_ptr<ARG_VAR_INFO>> argsInfo;
	std::pair<std::unordered_map<std::string, std::shared_ptr<MapItem>>, VAR_VMAP*> varMapCopy;
};

using ARRAY_SUPPORT_TYPES	= std::variant<VarDtype, std::shared_ptr<FUNCTION_MAP_DATA>>;
using DEEP_VALUE_DATA  		= std::variant<VarDtype, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>, std::shared_ptr<FUNCTION_MAP_DATA>>;
using REAL_AST_NODE_DATA 	= std::variant<AST_TOKENS, VarDtype, std::pair<ArrayAccessTokens, std::string>, 
							  std::pair<FunctionCallReturns, Token>, std::string>;

struct MapItem{
	MAPTYPE mapType = MAPTYPE::VARIABLE;
	std::variant<
		std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>,
		std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>,
		std::shared_ptr<FUNCTION_MAP_DATA>
	> var;

	inline void updateSingleVariable( VarDtype newValue ){
		if( this->mapType == MAPTYPE::VARIABLE ){
			auto varHolderData = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( this->var );
			if( varHolderData->isTypeArray )
				throw std::runtime_error("var is an array");
			varHolderData->value = newValue;
		}
		else throw std::runtime_error("not an single element");
	}

	inline void typeCastToInt(){
		if( mapType == MAPTYPE::VARIABLE ){
			auto VHdata = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( this->var );
			
			if( VHdata->isTypeArray )
				TypeCastError("Cannot cast array type");

			auto& vardata = std::get<VarDtype>(VHdata->value);
			
			if (auto p = std::get_if<double>(&vardata)) {
				vardata = static_cast<long>(*p);
			}
			else if (auto s = std::get_if<std::string>(&vardata)) {
            	vardata = std::stol(*s);
        	}
        	else if (auto p = std::get_if<bool>(&vardata)) {
    			vardata = static_cast<long>(*p);
			}
		}
	}

	inline void typeCastToDouble(){
		if( mapType == MAPTYPE::VARIABLE ){
			auto VHdata = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( this->var );
			
			if( VHdata->isTypeArray )
				TypeCastError("Cannot cast array type");

			auto& vardata = std::get<VarDtype>(VHdata->value);
			
			if (auto p = std::get_if<long>(&vardata)) {
				vardata = static_cast<double>(*p);
			}
			else if (auto s = std::get_if<std::string>(&vardata)) {
            	vardata = std::stod(*s);
        	}
        	else if (auto p = std::get_if<bool>(&vardata)) {
    			vardata = static_cast<double>(*p);
			}
		}
	}

	inline void typeCastToString(){
		if( mapType == MAPTYPE::VARIABLE ){
			auto VHdata = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( this->var );
			
			if( VHdata->isTypeArray )
				TypeCastError("Cannot cast array type");

			auto& vardata = std::get<VarDtype>(VHdata->value);
			
			if (auto p = std::get_if<long>(&vardata)) {
				vardata = std::to_string(*p);
			}
			else if (auto s = std::get_if<double>(&vardata)) {
            	vardata = std::to_string(*s);
        	}
        	else if (auto p = std::get_if<bool>(&vardata)) {
    			vardata = std::to_string(*p);
			}
		}
	}

	inline void typeCastToBool(){
		if( mapType == MAPTYPE::VARIABLE ){
			auto VHdata = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( this->var );
			
			if( VHdata->isTypeArray )
				TypeCastError("Cannot cast array type");

			auto& vardata = std::get<VarDtype>(VHdata->value);
			
			if (auto p = std::get_if<long>(&vardata)) {
				vardata = static_cast<bool>(*p);
			}
			else if (auto s = std::get_if<double>(&vardata)) {
            	vardata = static_cast<bool>(*s);
        	}
        	else if (auto p = std::get_if<std::string>(&vardata)) {
    			vardata = DtypeHelper::toBoolean( *p );
			}
		}
	}
};



#endif