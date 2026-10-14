#ifndef DEFINEDVALUES_HPP
#define DEFINEDVALUES_HPP

#include <string>
#include <memory>

#include "../Verifier/Headers/Variables.hpp"
#include "../Verifier/Headers/Function.hpp"
#include "../../Headers/Dtypes.hpp"
#include "../../Headers/MBExceptions.hpp"

#include "AST.hpp"

enum class MAPTYPE{ VARIABLE, ARRAY_PTR, FUNC_PTR };

struct MapItem;
class VAR_VMAP;

// identifiers are short : an inlined FNV-1a is cheaper than the out-of-line std::hash<std::string>
struct FastStringHash{
	inline size_t operator()( const std::string& key ) const noexcept {
		size_t h = 1469598103934665603ULL;
		for( unsigned char c: key ){
			h ^= c;
			h *= 1099511628211ULL;
		}
		return h;
	}
};
using VMAP_TYPE = std::unordered_map<std::string, std::shared_ptr<MapItem>, FastStringHash>;

struct FUNCTION_MAP_DATA{
	std::string funcName;
	size_t		argsSize;
	size_t 		bodyStartPtr;
	size_t 		bodyEndPtr;

	std::vector<ARG_VAR_INFO> argsInfo;
	std::pair<std::shared_ptr<VMAP_TYPE>, VAR_VMAP*> varMapCopy;
};

using ARRAY_SUPPORT_TYPES	= std::variant<VarDtype, std::shared_ptr<FUNCTION_MAP_DATA>>;
using DEEP_VALUE_DATA  		= std::variant<VarDtype, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>, std::shared_ptr<FUNCTION_MAP_DATA>>;
using REAL_AST_NODE_DATA 	= std::variant<
							  AST_TOKENS, VarDtype, std::pair<ArrayAccessTokens, std::string>, 
							  std::pair<FunctionCallReturns, Token>, std::string, 
							  std::pair<VariableTokens, Token>
							  >;

// light weight FIFO (vector backed) used instead of std::queue<std::deque> on hot paths
class ValueQueue{
	private:
		std::vector<DEEP_VALUE_DATA> data;
		size_t head = 0;
	public:
		inline void reserve( size_t n ){ data.reserve( n ); }
		inline bool empty() const { return head >= data.size(); }
		inline size_t size() const { return data.size() - head; }
		inline void push( DEEP_VALUE_DATA v ){ data.push_back( std::move( v ) ); }
		inline DEEP_VALUE_DATA& front(){ return data[ head ]; }
		inline void pop(){ ++head; }
};

using FUNC_RETURN_TYPE = std::optional<std::variant<
							VarDtype, std::shared_ptr<MapItem>, 
							std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>
						 >>;

struct MapItem{
	MAPTYPE mapType = MAPTYPE::VARIABLE;
	std::variant<
		std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>,
		std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>,
		std::shared_ptr<FUNCTION_MAP_DATA>
	> var;

	// the map entry that stores a value : single value, kootam or pindi
	static std::shared_ptr<MapItem> make( const std::string& name, DEEP_VALUE_DATA&& value ){
		auto item = std::make_shared<MapItem>();

		if( auto* single = std::get_if<VarDtype>( &value ) ){
			auto holder = std::make_shared<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
			holder->key = name;
			holder->isTypeArray = false;
			holder->value = std::move( *single );
			item->var = std::move( holder );
		}
		else if( auto* array = std::get_if<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( &value ) ){
			item->mapType = MAPTYPE::ARRAY_PTR;
			item->var = std::move( *array );
		}
		else{
			item->mapType = MAPTYPE::FUNC_PTR;
			item->var = std::move( std::get<std::shared_ptr<FUNCTION_MAP_DATA>>( value ) );
		}
		return item;
	}

	inline void updateSingleVariable( VarDtype newValue ){
		if( this->mapType == MAPTYPE::VARIABLE ){
			auto& varHolderData = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( this->var );
			if( varHolderData->isTypeArray )
				throw std::runtime_error("var is an array");
			varHolderData->value = std::move( newValue );
		}
		else throw std::runtime_error("not an single element");
	}

	// := INT / THULA / STR / BOOL ( onnula stays onnula )
	inline void typeCast( const Token& tok ){
		if( this->mapType != MAPTYPE::VARIABLE )
			throw InvalidSyntaxError("Only variables are allowed for TYPE_CASTING");

		const std::string& to = tok.token;
		if( to != "INT" && to != "THULA" && to != "STR" && to != "BOOL" )
			throw TypeCastError("Failed to cast");

		auto& holder = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( this->var );
		if( holder->isTypeArray )
			throw TypeCastError("Cannot cast array type");	// used to be missing its throw

		VarDtype& v = std::get<VarDtype>( holder->value );
		const long*        l = std::get_if<long>( &v );
		const double*      d = std::get_if<double>( &v );
		const bool*        b = std::get_if<bool>( &v );
		const std::string* s = std::get_if<std::string>( &v );

		if( to == "INT" ){
			if( d ) v = (long) *d; else if( s ) v = DtypeHelper::toLong( *s ); else if( b ) v = (long) *b;
		}
		else if( to == "THULA" ){
			if( l ) v = (double) *l; else if( s ) v = DtypeHelper::toDouble( *s ); else if( b ) v = (double) *b;
		}
		else if( to == "STR" ){
			if( l ) v = std::to_string( *l ); else if( d ) v = std::to_string( *d ); else if( b ) v = std::string( *b ? "sheri" : "thettu" );
		}
		else{
			if( l ) v = (bool) *l; else if( d ) v = (bool) *d; else if( s ) v = DtypeHelper::toBoolean( *s );
		}
	}
};



#endif