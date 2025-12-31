/* MAP ITEM AND ITS HELPER CLASSES */

#ifndef DEFINEDVALUES_HPP
#define DEFINEDVALUES_HPP

#include <string>
#include <optional>
#include <memory>
#include <iostream>
#include <type_traits>

#include "VarHandler.hpp"
#include "ConditionalHandler.hpp"
#include "IOHandler.hpp"
#include "InstructionHandler.hpp"
#include "LoopHandler.hpp"
#include "FunctionHandler.hpp"

#include "AST.hpp"
#include "Parser.hpp"

#include "../Headers/Dtypes.hpp"
#include "../Headers/MBExceptions.hpp"


using namespace std;

enum class MAPTYPE{ VARIABLE, FUNCTION };

template<typename T>
inline constexpr bool is_number_v =
    std::is_same_v<T, long int> ||
    std::is_same_v<T, double> || 
    std::is_same_v<T, bool>;

struct MapItem;

using VALUE_DATA 			= variant<VarDtype, unique_ptr<MapItem>>;
using AST_NODE_DATA 		= variant<AST_TOKENS, VALUE_DATA>;
using VARIABLE_HOLDER_DATA  = variant<
								VALUE_DATA, 
								unique_ptr<AST_NODE<AST_NODE_DATA>>
								>;

struct MapItem{
	MAPTYPE mapType = MAPTYPE::VARIABLE;
	variant<
		unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>,
		unique_ptr<FUNCTION_MAP_DATA>
	> var;

	void
	assignSingleValue( unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>> data ){
		if( holds_alternative<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>>( var ) ){
			auto& varData = get<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>>( var );
			if( varData->isTypeArray )
				throw InvalidDTypeError("Cannot assign single value to kootam");
			varData->value = move( data );
			return;
		}
		throw InvalidSyntaxError( "Assign value to variable only! " ); 
	}

	// void
	// assignArrayValue( unique_ptr<ArrayList<VARIABLE_HOLDER_DATA>> data ){
	// 	if( !var->isTypeArray )
	// 		throw InvalidSyntaxError("Cannot assign kootam value to single value");
	// 	var->value = move( data );
	// }
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
	                case '"':  out.push_back('"');  break;
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

	static
	void printVarDtype(const VarDtype& value) {
	    std::visit(
	        [](const auto& v) {
	            using T = std::decay_t<decltype(v)>;
	            if constexpr (std::is_same_v<T, bool>) {
	                std::cout << (v ? "sheri" : "thettu");
	            }
	            else if constexpr (std::is_same_v<T, std::string>) {
	                std::cout << ValueHelper::unescapeString(v);
	            }
	            else  std::cout << v;
	        },
	        value
	    );
	}

	static VarDtype 
	getValueFromVariableHolderType( VARIABLE_HOLDER_DATA& vData ){
		if( holds_alternative<VALUE_DATA>( vData ) ){
			auto& valueDataD = get<VALUE_DATA>( vData );
			return ValueHelper::getValueFromValueData( valueDataD );
		}
		else if(  holds_alternative<unique_ptr<AST_NODE<AST_NODE_DATA>>>( vData ) ){
			auto& astNode = get<unique_ptr<AST_NODE<AST_NODE_DATA>>>( vData );
			VarDtype astnodeData = ValueHelper::evaluate_AST_NODE( astNode );
			return astnodeData;
		}
		else throw ;
	}

	static VarDtype
	getValueFromValueData( VALUE_DATA& data ){
		if( holds_alternative<VarDtype>( data ) ){
			VarDtype vardtypeData = get<VarDtype>( data );
			return vardtypeData;
		}
		else{
			unique_ptr<MapItem>& mapItem = get<unique_ptr<MapItem>>( data );
			VarDtype varDtypeData = ValueHelper::getValueFromMapItem( mapItem.get() );
			return varDtypeData;
		}
	}

	static VarDtype 
	evaluate_AST_NODE( unique_ptr<AST_NODE<AST_NODE_DATA>>& astNode ){
		auto& astNodeData = astNode->AST_DATA;

		if( holds_alternative<VALUE_DATA>( astNodeData ) ){
			auto& valueData = get<VALUE_DATA>( astNodeData );

			if( holds_alternative<VarDtype>( valueData ) ){
				VarDtype varDtypeData = get<VarDtype>( valueData );
				return varDtypeData;
			}
			else{
				auto& mapData = get<unique_ptr<MapItem>>( valueData );
				VarDtype value = ValueHelper::getValueFromMapItem( mapData.get() );
				ValueHelper::printVarDtype( value );
				return value;
			}
		}
		else{
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

	static VarDtype 
	getValueFromMapItem( MapItem* mapItem ){
		auto& variableData = mapItem->var;

		if( mapItem->mapType != MAPTYPE::VARIABLE ){
			throw InvalidSyntaxError("Its not a varible");
		}

		if( holds_alternative<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>>( variableData ) ){

			auto& nextVariableData = get<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>> ( variableData );

			if( holds_alternative<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>> ( nextVariableData->value ) ){
				auto& SEdata = get<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>>( nextVariableData->value );			
			
				auto completeData = SEdata->getValue( );

				if( completeData != nullptr ){
					VarDtype data = ValueHelper::getValueFromVariableHolderType( *completeData );
					return data;
				}
			}
			throw InvalidSyntaxError( "Cannot copy an kootam variable" );
		}
		throw InvalidSyntaxError("Its not a varible");
	}

	static
	void printArray( ArrayList<VARIABLE_HOLDER_DATA>* pArrList ){
		std::vector< 
			std::variant< 
				unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>, 
				unique_ptr<ArrayList<VARIABLE_HOLDER_DATA>>
			>
		>&  arrayList = pArrList->getList( );
		
		cout << '[';
		for(int x = 0; x < arrayList.size(); x++){
			auto& data = arrayList[ x ];
			if( holds_alternative<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>> ( data ) ){
				auto& singleData = get<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>> ( data );
				auto dataT = singleData->getValue();
				VarDtype data = ValueHelper::getValueFromVariableHolderType( *dataT );
				ValueHelper::printVarDtype( data );
				if( x != arrayList.size() - 1 )
					cout << ", ";
			}
			else{
				auto& array = get<unique_ptr<ArrayList<VARIABLE_HOLDER_DATA>>> (data);
				ValueHelper::printArray( array.get() );
			}
		}
		cout << ']';
	}
};

#endif