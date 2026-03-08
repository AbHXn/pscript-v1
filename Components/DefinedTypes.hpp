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

enum class MAPTYPE{ VARIABLE, FUNCTION, ARRAY_PTR, NULL_PTR, FUNC_PTR };

template<typename T>
inline constexpr bool is_number_v = std::is_same_v<T, long int> || std::is_same_v<T, double> || std::is_same_v<T, bool>;

struct MapItem;
class VAR_VMAP;

struct FUNCTION_MAP_DATA{
	std::string funcName;
	size_t argsSize;
	std::vector<std::unique_ptr<ARG_VAR_INFO>> argsInfo;
	size_t bodyStartPtr;
	size_t bodyEndPtr;
	std::pair<std::unordered_map<std::string, MapItem*>, VAR_VMAP*> varMapCopy;

	FUNCTION_MAP_DATA() = default;
};

using ARRAY_SUPPORT_TYPES	= std::variant<VarDtype, FUNCTION_MAP_DATA*>;
using DEEP_VALUE_DATA  		= std::variant<VarDtype, ArrayList<ARRAY_SUPPORT_TYPES>*, FUNCTION_MAP_DATA*>;
using AST_NODE_DATA 		= std::variant<AST_TOKENS, DEEP_VALUE_DATA>;


struct MapItem{
	MAPTYPE mapType = MAPTYPE::VARIABLE;
	std::variant<
		VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*,
		ArrayList<ARRAY_SUPPORT_TYPES>*,
		FUNCTION_MAP_DATA*
	> var;

	void updateSingleVariable( VarDtype newValue ){
		if( this->mapType == MAPTYPE::VARIABLE ){
			auto varHolderData = std::get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( this->var );
			if( varHolderData->isTypeArray )
				throw std::runtime_error("var is an array");
			varHolderData->value = newValue;
		}
		else throw std::runtime_error("not an single element");
	}

	void typeCastToInt(){
		if( mapType == MAPTYPE::VARIABLE ){
			auto VHdata = std::get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( this->var );
			
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

	void typeCastToDouble(){
		if( mapType == MAPTYPE::VARIABLE ){
			auto VHdata = std::get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( this->var );
			
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

	void typeCastToString(){
		if( mapType == MAPTYPE::VARIABLE ){
			auto VHdata = std::get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( this->var );
			
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

	 static void printArrayList(const ArrayList<ARRAY_SUPPORT_TYPES>* array) {
        if (!array) {
            std::cout << "null";
            return;
        }

        std::cout << "[";
        for (size_t i = 0; i < array->arrayList.size(); ++i) {
            const auto& elem = array->arrayList[i];
            if( std::holds_alternative<ARRAY_SUPPORT_TYPES>( elem ) ){
            	auto arrSpType = std::get<ARRAY_SUPPORT_TYPES>( elem );
            	if( std::holds_alternative<VarDtype>(arrSpType) ){
            		ValueHelper::printVarDtype( std::get<VarDtype>( arrSpType ) );
            	}
            	else if( std::holds_alternative<FUNCTION_MAP_DATA*>(arrSpType) ){
            		std::cout << "FUNC";
            	}
            	else std::cout << "Unknown";
            }
            else if (std::holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>( elem )){
            	auto arrptr = std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>( elem );
            	ValueHelper::printArrayList( arrptr );
            }
            else if( std::holds_alternative<std::unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( elem ) ){
            	auto& arrptr = std::get<std::unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( elem );
            	ValueHelper::printArrayList( arrptr.get() );
            }
            if (i + 1 < array->arrayList.size()) std::cout << ", ";
        }
        std::cout << "]";
    }

	static DEEP_VALUE_DATA
	getFinalValueFromMap( MapItem* mapData ){
		if( mapData->mapType == MAPTYPE::VARIABLE ){
			auto VariableData = std::get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( mapData->var );
			return ValueHelper::getDataFromVariableHolder( VariableData );
		}
		else if( mapData->mapType == MAPTYPE::FUNCTION ){
			return std::get<FUNCTION_MAP_DATA*>( mapData->var );
		}
		else if( mapData->mapType == MAPTYPE::FUNC_PTR ){
			return std::get<FUNCTION_MAP_DATA*>( mapData->var );
		}
		throw std::runtime_error("DTYPE not defined in MAP");
	}

	static DEEP_VALUE_DATA
	getDataFromVariableHolder( VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>* varData ){
		if( std::holds_alternative<VarDtype> ( varData->value ) ){
			return std::get<VarDtype>( varData->value );
		}
		else if( std::holds_alternative<std::unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( varData->value ) ){
			auto& arrList = std::get<std::unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( varData->value );
			return arrList.get();
		}
		throw std::runtime_error("DTYPE not defined in MAP");
	}

	static void
	printDEEP_VALUE_DATA( DEEP_VALUE_DATA& data ){
		if( std::holds_alternative<VarDtype>( data ) ){
			printVarDtype( std::get<VarDtype>( data ) );
		}
		else if( std::holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>(data) ){
			auto& test = std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>(data);
			printArrayList( test );
		}
		else throw std::runtime_error("Didn't create display method for given type");
	}

	template<typename T>
	static std::string toString(T v) {
	    if constexpr (std::is_same_v<T, bool>)
	        return v ? "sheri" : "thettu";
	    else if constexpr (std::is_same_v<T, std::string>)
	        return v;
	    else
	        return std::to_string(v);
	}

	static VarDtype 
	evaluate_AST_NODE( std::unique_ptr<AST_NODE<AST_NODE_DATA>>& astNode ){
		auto& astNodeData = astNode->AST_DATA;

		if( std::holds_alternative<DEEP_VALUE_DATA>( astNodeData ) ){
			auto& valueData = std::get<DEEP_VALUE_DATA>( astNodeData );
			if( std::holds_alternative<VarDtype>( valueData ) ){
				return std::get<VarDtype>( valueData );
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
					    switch (astNode->isASTTokens ? std::get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD) {

						    case AST_TOKENS::ADD:
						        if constexpr (std::is_same_v<X,long> && std::is_same_v<Y,long>)
						            return x + y;
						        else
						            return static_cast<double>(x) + static_cast<double>(y);

						    case AST_TOKENS::SUB:
						        if constexpr (std::is_same_v<X,long> && std::is_same_v<Y,long>)
						            return x - y;
						        else
						            return static_cast<double>(x) - static_cast<double>(y);

						    case AST_TOKENS::MUL:
						        if constexpr (std::is_same_v<X,long> && std::is_same_v<Y,long>)
						            return x * y;
						        else
						            return static_cast<double>(x) * static_cast<double>(y);

						    case AST_TOKENS::DIV:
						        return static_cast<double>(x) / static_cast<double>(y);

						    case AST_TOKENS::MOD:
						        return static_cast<long>(x) % static_cast<long>(y);

						    case AST_TOKENS::AND:
						        return static_cast<bool>(x) && static_cast<bool>(y);

						    case AST_TOKENS::OR:
						        return static_cast<bool>(x) || static_cast<bool>(y);

						    case AST_TOKENS::LS_THAN:
						        return static_cast<double>(x) < static_cast<double>(y);

						    case AST_TOKENS::GT_THAN:
						        return static_cast<double>(x) > static_cast<double>(y);

						    case AST_TOKENS::LS_THAN_EQ:
						        return static_cast<double>(x) <= static_cast<double>(y);

						    case AST_TOKENS::GT_THAN_EQ:
						        return static_cast<double>(x) >= static_cast<double>(y);

						    case AST_TOKENS::D_EQUAL_TO:
						        return static_cast<double>(x) == static_cast<double>(y);

						    case AST_TOKENS::NOT_EQUAL_TO:
						        return static_cast<double>(x) != static_cast<double>(y);

						    default:
						        throw std::runtime_error("Unknown operator for numbers");
						}
					}
					else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>) {

					    switch (astNode->isASTTokens ? std::get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD) {

					        case AST_TOKENS::ADD:  return x + y;

					        case AST_TOKENS::LS_THAN:      return x <  y;
					        case AST_TOKENS::GT_THAN:      return x >  y;
					        case AST_TOKENS::LS_THAN_EQ:   return x <= y;
					        case AST_TOKENS::GT_THAN_EQ:   return x >= y;
					        case AST_TOKENS::D_EQUAL_TO:   return x == y;
					        case AST_TOKENS::NOT_EQUAL_TO: return x != y;

					        default:
					            throw InvalidSyntaxError("Invalid operator for strings");
					    }
					}
					else if constexpr (
					    (std::is_same_v<X, std::string> || std::is_same_v<Y, std::string>)
					) {
					    switch (astNode->isASTTokens ? std::get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD) {
					        case AST_TOKENS::ADD:
					            return toString(x) + toString(y);
					        default:
					            throw InvalidSyntaxError("Invalid operator involving string");
					    }
					} 
					else throw InvalidSyntaxError("Invalid operation between VarDtype types");
					
				}, leftData, rightData
			);
		}
	}
};

#endif