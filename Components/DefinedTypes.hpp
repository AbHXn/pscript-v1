/* MAP ITEM AND ITS HELPER CLASSES */

#ifndef DEFINEDVALUES_HPP
#define DEFINEDVALUES_HPP

#include <string>
#include <optional>
#include <memory>
#include <iostream>
#include <type_traits>

#include "Handlers/Variables.hpp"
#include "Handlers/Conditional.hpp"
#include "Handlers/Function.hpp"
#include "Handlers/loops.hpp"
#include "Handlers/InputOutput.hpp"
#include "Handlers/Instruction.hpp"

#include "AST.hpp"

#include "../Headers/Dtypes.hpp"
#include "../Headers/MBExceptions.hpp"
#include "../Headers/Tokenizer.hpp"

using namespace std;

enum class MAPTYPE{ VARIABLE, FUNCTION, ARRAY_PTR };

template<typename T>
inline constexpr bool is_number_v = std::is_same_v<T, long int> || std::is_same_v<T, double> || std::is_same_v<T, bool>;

using DEEP_VALUE_DATA  		= variant<VarDtype, ArrayList*, FUNCTION_MAP_DATA*>;
using AST_NODE_DATA 		= variant<AST_TOKENS, DEEP_VALUE_DATA>;


struct MapItem{
	MAPTYPE mapType = MAPTYPE::VARIABLE;
	variant<
		unique_ptr<VARIABLE_HOLDER>,
		unique_ptr<FUNCTION_MAP_DATA>,
		ArrayList*
	> var;

	void updateSingleVariable( VarDtype newValue ){
		if( this->mapType == MAPTYPE::VARIABLE ){
			auto& varHolderData = get<unique_ptr<VARIABLE_HOLDER>>( this->var );
			if( varHolderData->isTypeArray )
				throw runtime_error("var is an array");
			varHolderData->value = newValue;
		}
		else throw runtime_error("not an single element");
	}
};

class ValueHelper{
	public:
		static
		std::string unescapeString(const std::string& input) {
	    std::string out;
	    out.reserve(input.size());

	    for (size_t i = 0; i < input.size(); ++i) {
	        if (input[i] == '\\' && i + 1 < input.size()) {
	            char e = input[i + 1];
	            switch (e) {
	                case 'n':  out.push_back('\n'); break;
	                case 't':  out.push_back('\t'); break;
	                case 'r':  out.push_back('\r'); break;
	                case 'b':  out.push_back('\b'); break;
	                case 'f':  out.push_back('\f'); break;
	                case 'v':  out.push_back('\v'); break;
	                case '\\': out.push_back('\\'); break;
	                case '\'': out.push_back('\''); break;
	                case '0':  out.push_back('\0'); break;
	                default:
	                    out.push_back(e);
	                    break;
	            }
	            ++i; 
	        } else {
	            out.push_back(input[i]);
	        }
	    }
	    return out;
	}

	static void printVarDtype(const VarDtype& value) {
	    std::visit([](const auto& v) {
	        using T = std::decay_t<decltype(v)>;

	        if constexpr (std::is_same_v<T, bool>) {
	            std::cout << (v ? "sheri" : "thettu");
	        }
	        else if constexpr (std::is_same_v<T, std::string>) {
	            std::cout << ValueHelper::unescapeString(v);
	        }
	        else {
	            std::cout << v;
	        }
	    }, value);
	}

	 static void printArrayList(const ArrayList* array) {
        if (!array) {
            std::cout << "null";
            return;
        }

        std::cout << "[";
        for (size_t i = 0; i < array->arrayList.size(); ++i) {
            const auto& elem = array->arrayList[i];
            std::visit([&](auto&& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, VarDtype>) {
                    printVarDtype(v);
                }
                else if constexpr (std::is_same_v<T, unique_ptr<ArrayList>>){
                	printArrayList( v.get() );
                }
            }, elem);

            if (i + 1 < array->arrayList.size()) std::cout << ", ";
        }
        std::cout << "]";
    }

	static DEEP_VALUE_DATA
	getFinalValueFromMap( MapItem* mapData ){
		if( mapData->mapType == MAPTYPE::VARIABLE ){
			auto& VariableData = get<unique_ptr<VARIABLE_HOLDER>>( mapData->var );
			auto varData = ValueHelper::getDataFromVariableHolder( VariableData.get() );
			return varData;
		}
		else if( mapData->mapType == MAPTYPE::FUNCTION ){
			auto& VariableData = get<unique_ptr<FUNCTION_MAP_DATA>>( mapData->var );
			return VariableData.get();
		}
		throw runtime_error("DTYPE not defined in MAP");
	}

	static DEEP_VALUE_DATA
	getDataFromVariableHolder( VARIABLE_HOLDER* varData ){
		if( holds_alternative<VarDtype> ( varData->value ) ){
			return get<VarDtype>( varData->value );
		}
		else if( holds_alternative<unique_ptr<ArrayList>>( varData->value ) ){
			auto& arrList = get<unique_ptr<ArrayList>>( varData->value );
			return arrList.get();
		}
		throw runtime_error("DTYPE not defined in MAP");
	}

	static void
	printDEEP_VALUE_DATA( DEEP_VALUE_DATA& data ){
		if( holds_alternative<VarDtype>( data ) ){
			printVarDtype( get<VarDtype>( data ) );
		}
		else if( holds_alternative<ArrayList*>(data) ){
			auto& test = get<ArrayList*>(data);
			printArrayList( test );
		}
		else throw runtime_error("Didn't create display method for given type");
	}

	static VarDtype 
	evaluate_AST_NODE( unique_ptr<AST_NODE<AST_NODE_DATA>>& astNode ){
		auto& astNodeData = astNode->AST_DATA;

		if( holds_alternative<DEEP_VALUE_DATA>( astNodeData ) ){
			auto& valueData = get<DEEP_VALUE_DATA>( astNodeData );
			if( holds_alternative<VarDtype>( valueData ) ){
				VarDtype varDtypeData = get<VarDtype>( valueData );
				return varDtypeData;
			}
			throw InvalidSyntaxError("Invalid dtype for operation");
		}
		else{
			if( !astNode->left || !astNode->right )
				throw InvalidSyntaxError("Invalid Expression");

			VarDtype leftData  = ValueHelper::evaluate_AST_NODE( astNode->left );
			VarDtype rightData = ValueHelper::evaluate_AST_NODE( astNode->right );

			return std::visit(
				[&](const auto& x, const auto& y) -> VarDtype {
					using X = std::decay_t<decltype(x)>;
					using Y = std::decay_t<decltype(y)>;

					if constexpr (is_number_v<X> && is_number_v<Y>) {
						switch (astNode->isASTTokens ? get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD) {
							case AST_TOKENS::ADD: 	return static_cast<double>(x) + static_cast<double>(y);
							case AST_TOKENS::SUB: 	return static_cast<double>(x) - static_cast<double>(y);
							case AST_TOKENS::MUL:	return static_cast<double>(x) * static_cast<double>(y);
							case AST_TOKENS::DIV: 	return static_cast<double>(x) / static_cast<double>(y);
							case AST_TOKENS::MOD:  	return static_cast<long int>(x) % static_cast<long int>(y);
							
							case AST_TOKENS::AND: 	return static_cast<bool>(x) && static_cast<bool>(y);
							case AST_TOKENS::OR: 	return static_cast<bool>(x) || static_cast<bool>(y);
							
							case AST_TOKENS::LS_THAN: 	   return static_cast<double>(x) < static_cast<double>(y);
							case AST_TOKENS::GT_THAN:  	   return static_cast<double>(x) > static_cast<double>(y);
							case AST_TOKENS::LS_THAN_EQ:   return static_cast<double>(x) <= static_cast<double>(y);
							case AST_TOKENS::GT_THAN_EQ:   return static_cast<double>(x) >= static_cast<double>(y);
							case AST_TOKENS::D_EQUAL_TO:   return static_cast<double>(x) == static_cast<double>(y);
							case AST_TOKENS::NOT_EQUAL_TO: return static_cast<double>(x) != static_cast<double>(y);

							default: throw std::runtime_error("Unknown operator for numbers");
						}
					} 
					else if constexpr ( std::is_same_v<X, std::string> && std::is_same_v<Y, std::string> ) {
						switch( astNode->isASTTokens ? get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD ) {
							case AST_TOKENS::ADD: 	return x + y;
							case AST_TOKENS::LS_THAN: 	   return x < y;
							case AST_TOKENS::GT_THAN:  	   return x > y;
							case AST_TOKENS::LS_THAN_EQ:   return x <= y;
							case AST_TOKENS::GT_THAN_EQ:   return x >= y;
							case AST_TOKENS::D_EQUAL_TO:   return x == y;
							case AST_TOKENS::NOT_EQUAL_TO: return x != y;
							default: throw InvalidSyntaxError("Invalid operator for strings");
						} 
						
					} 
					else throw InvalidSyntaxError("Invalid operation between VarDtype types");
					
				}, leftData, rightData
			);
		}
	}
};

#endif