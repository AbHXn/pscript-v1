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
		// string literals are unescaped once, when the value is created from its token
		static std::string unescapeString( const std::string& in ){
			std::string out;
			out.reserve( in.size() );

			for( size_t i = 0; i < in.size(); i++ ){
				if( in[ i ] != '\\' || i + 1 >= in.size() ){ out += in[ i ]; continue; }

				switch( const char e = in[ ++i ] ){
					case 'n': out += '\n'; break;
					case 't': out += '\t'; break;
					case 'r': out += '\r'; break;
					case 'b': out += '\b'; break;
					case 'f': out += '\f'; break;
					case 'v': out += '\v'; break;
					case '0': out += '\0'; break;
					default:  out += e;		// \\ \' \" and unknown escapes keep the character itself
				}
			}
			return out;
		}

		static void printVarDtype( const VarDtype& value ){
			std::visit( []( const auto& v ){
				using T = std::decay_t<decltype( v )>;

				if constexpr( std::is_same_v<T, std::monostate> )  std::cout << "onnula";
				else if constexpr( std::is_same_v<T, bool> )       std::cout << ( v ? "sheri" : "thettu" );
				else 											   std::cout << v;
			}, value );
		}

		static void printArrayList( const std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>& array ){
			std::cout << '{';
			for( size_t i = 0; i < array->arrayList.size(); i++ ){
				if( i ) std::cout << ", ";

				const auto& elem = array->arrayList[ i ];
				if( auto* nested = std::get_if<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( &elem ) )
					printArrayList( *nested );
				else if( auto* v = std::get_if<VarDtype>( &std::get<ARRAY_SUPPORT_TYPES>( elem ) ) )
					printVarDtype( *v );
				else std::cout << "FUNC";
			}
			std::cout << '}';
		}

		static void printDEEP_VALUE_DATA( const DEEP_VALUE_DATA& data ){
			if( auto* v = std::get_if<VarDtype>( &data ) )
				printVarDtype( *v );
			else if( auto* a = std::get_if<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( &data ) )
				printArrayList( *a );
			else std::cout << "FUNC";
		}

		// whatever a map entry holds, as a general value
		static DEEP_VALUE_DATA getFinalValueFromMap( const std::shared_ptr<MapItem>& item ){
			return std::visit( []( const auto& v ) -> DEEP_VALUE_DATA {
				if constexpr( std::is_same_v<std::decay_t<decltype( v )>, std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>> )
					return std::visit( []( const auto& x ) -> DEEP_VALUE_DATA { return x; }, v->value );
				else return v;
			}, item->var );
		}

		static std::string getValueType( const DEEP_VALUE_DATA& value ){
			static const char* const names[] = { "STR", "INT", "THULA", "BOOL", "ARILA" };	// same order as VarDtype

			if( auto* v = std::get_if<VarDtype>( &value ) )
				return names[ v->index() ];

			return std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>( value ) ? "FUNC_PTR" : "ARRAY_PTR";
		}

		static VarDtype getVariableSize( const DEEP_VALUE_DATA& value ){
			const VarDtype* v = std::get_if<VarDtype>( &value );
			if( !v )
				throw InvalidSyntaxError( "Kanam property is only for single type variable" );

			if( auto* s = std::get_if<std::string>( v ) ) return (long) s->size();
			if( std::holds_alternative<double>( *v ) )    return (long) sizeof( double );
			if( std::holds_alternative<long>( *v ) )      return (long) sizeof( long );
			if( std::holds_alternative<bool>( *v ) )      return (long) sizeof( bool );
			throw InvalidSyntaxError( "Invalid Type for kanam" );
		}

		static VarDtype getValueFromToken( const Token& tok ){
			switch( tok.type ){
				case TOKEN_TYPE::STRING:   return VarDtype{ unescapeString( tok.token ) };
				case TOKEN_TYPE::NUMBER:   return VarDtype{ DtypeHelper::toLong( tok.token ) };
				case TOKEN_TYPE::FLOATING: return VarDtype{ DtypeHelper::toDouble( tok.token ) };
				case TOKEN_TYPE::BOOLEAN:  return VarDtype{ tok.token == "sheri" };
				default: throw InvalidDTypeError( "not a value" );
			}
		}

		template<typename T>
		static std::string toString( const T& v ){
			if constexpr( std::is_same_v<T, bool> )               return v ? "sheri" : "thettu";
			else if constexpr( std::is_same_v<T, std::string> )   return v;
			else if constexpr( std::is_same_v<T, std::monostate> ) return "onnula";
			else 												  return std::to_string( v );
		}

		static bool toBool( const VarDtype& value ){
			return std::visit( []( const auto& v ) -> bool {
				using T = std::decay_t<decltype( v )>;

				if constexpr( std::is_same_v<T, std::monostate> )    return false;
				else if constexpr( std::is_same_v<T, std::string> )  return !v.empty();
				else 												 return static_cast<bool>( v );
			}, value );
		}
};

#endif
