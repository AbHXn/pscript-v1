#ifndef VALUEHELPER 
#define VALUEHELPER

#include "VMAPData.hpp"

#include "../Verifier/Headers/Variables.hpp"
#include "../Verifier/Headers/Conditional.hpp"
#include "../Verifier/Headers/Function.hpp"
#include "../Verifier/Headers/loops.hpp"
#include "../Verifier/Headers/InputOutput.hpp"
#include "../Verifier/Headers/Instruction.hpp"

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
	        } else out.push_back(input[i]);
	    }
	    return out;
	}

	static void printVarDtype(const VarDtype& value) {
		std::visit([](const auto& v) {
			using T = std::decay_t<decltype(v)>;
			if constexpr ( std::is_same_v<T, std::monostate> )
				std::cout << "onnula";

			else if constexpr (std::is_same_v<T, bool>)
				std::cout << (v ? "sheri" : "thettu");
			
			else if constexpr (std::is_same_v<T, std::string>)
				std::cout << ValueHelper::unescapeString(v);

	        else std::cout << v;
	  
	    }, value);
	}

	 static void printArrayList( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> array) {
        if (!array) {
            std::cout << "null";
            return;
        }
        std::cout << "{";
        for (size_t i = 0; i < array->arrayList.size(); ++i) {
            auto& elem = array->arrayList[i];
            if( std::holds_alternative<ARRAY_SUPPORT_TYPES>( elem ) ){
            	auto arrSpType = std::get<ARRAY_SUPPORT_TYPES>( elem );
            	if( std::holds_alternative<VarDtype>(arrSpType) ){
            		ValueHelper::printVarDtype( std::get<VarDtype>( arrSpType ) );
            	}
            	else if( std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>(arrSpType) ){
            		std::cout << "FUNC";
            	}
            	else std::cout << "Unknown";
            }
            else if( std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( elem ) ){
            	auto arrptr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( elem );
            	ValueHelper::printArrayList( arrptr );
            }
            if (i + 1 < array->arrayList.size()) std::cout << ", ";
        }
        std::cout << "}";
    }

	static DEEP_VALUE_DATA
	getFinalValueFromMap(std::shared_ptr<MapItem>& mapData) {
	    return std::visit([](auto& value) -> DEEP_VALUE_DATA {
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>) 
				return ValueHelper::getDataFromVariableHolder(value);
			
			else if constexpr (std::is_same_v<T, std::shared_ptr<FUNCTION_MAP_DATA>>) 
				return value;
			
			else if constexpr (std::is_same_v<T, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>)
				return value;
			
			else throw std::runtime_error("Unknown type in MAP");
		}, mapData->var);
	}

	static DEEP_VALUE_DATA
	getDataFromVariableHolder( std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>& varData ){
		return std::visit( [](auto& value)-> DEEP_VALUE_DATA { return value; }, varData->value );
	}

	static void
	printDEEP_VALUE_DATA(DEEP_VALUE_DATA& data) {
	    std::visit([](auto& value) {
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<T, VarDtype>)
				printVarDtype(value);

			else if constexpr (std::is_same_v<T, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>)
				printArrayList(value);
			
			else throw std::runtime_error("Unknown type");
	    }, data);
	}

	static std::string getValueType( DEEP_VALUE_DATA& Vdata ){
		if( std::holds_alternative<VarDtype> (Vdata) ){
			auto data = std::get<VarDtype>( Vdata );
			
			if( std::holds_alternative<std::string> ( data ) )
				return "STR";
			
			else if( std::holds_alternative<double> (data) )
				return "THULA";
			
			else if( std::holds_alternative<long> (data) )
				return "INT";
			
			else if( std::holds_alternative<bool> (data) )
				return "BOOL";
			
			else return "ARILA";
		}
		else if( std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( Vdata ) )
			return "ARRAY_PTR";
		
		else if( std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>( Vdata ) )
			return "FUNC_PTR";
		
		else return "ARILA";
	}

	static VarDtype getVariableSize( DEEP_VALUE_DATA& Vdata ){
		if( std::holds_alternative<VarDtype>(Vdata) ){
			auto data = std::get<VarDtype>(Vdata);
			
			if( std::holds_alternative<std::string> ( data ) )
				return (long) std::get<std::string>(data).size();
			
			else if( std::holds_alternative<double> (data) )
				return (long) sizeof(double);
			
			else if( std::holds_alternative<long> (data) )
				return (long) sizeof(long);

			else if( std::holds_alternative<bool> (data) )
				return (long) sizeof(bool);

			throw InvalidSyntaxError("Invalid Type for kanam");
		}
		throw InvalidSyntaxError("Kanam property is only for single type variable");
	}

	static VarDtype getValueFromToken( const Token& tok ){
		if( tok.type == TOKEN_TYPE::STRING )
			return VarDtype{ ValueHelper::unescapeString( tok.token ) };
		
		else if( tok.type == TOKEN_TYPE::NUMBER )
			return VarDtype{  DtypeHelper::toLong( tok.token ) };
		
		else if( tok.type == TOKEN_TYPE::FLOATING )
			return VarDtype{ DtypeHelper::toDouble( tok.token ) };

		else if( tok.type == TOKEN_TYPE::BOOLEAN && 
				( tok.token == "sheri" || tok.token == "thettu" ) )
			return VarDtype{ DtypeHelper::toBoolean( tok.token ) };
		
		throw InvalidDTypeError("not a value");
	}

	template<typename T>
	static std::string toString(T v) {
		if constexpr (std::is_same_v<T, bool>)
			return v ? "sheri" : "thettu";
	    
		else if constexpr (std::is_same_v<T, std::string>)
			return v;

		else if constexpr(std::is_same_v<T, std::monostate>)
			return "onnula";
	    
		else return std::to_string(v);
	}
	static bool toBool(const VarDtype& value){
        return std::visit([](auto&& v) -> bool {
            using T = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<T, bool>)
				return v;
            
			else if constexpr (std::is_same_v<T, long>)
				return v != 0;
            
			else if constexpr (std::is_same_v<T, double>)
				return v != 0.0;
            
			else if constexpr (std::is_same_v<T, std::string>)
				return !v.empty();
            
			else return false;
            
        }, value);
    }
};

#endif