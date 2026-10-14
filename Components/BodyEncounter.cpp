#include "Headers/BodyEncounter.hpp"

// a stored array element ( single value / pindi ) as a general value
static DEEP_VALUE_DATA deep( const ARRAY_SUPPORT_TYPES& item ){
	return std::visit( []( const auto& x ){ return DEEP_VALUE_DATA{ x }; }, item );
}

std::optional<VarDtype>
BodyEncounters::handleArrayProperties( const ArrayPtr& array, const ArrayAccessTokens& tok ){
	if( tok.arrProperty == "valupam" )
		return (long) array->totalElementsAllocated;
	
	if( tok.arrProperty == "jaadi" )
		return std::string( "KOOTAM" );

	if( tok.arrProperty == "irangu" )
		throw InvalidSyntaxError( "irangu is for elements inside the array" );
	
	return std::nullopt;
}

DEEP_VALUE_DATA
BodyEncounters::handleArrayCases( const ArrayPtr& arrayData, const ArrayAccessTokens& tok ){
	if( tok.indexVector.empty() ){
		if( tok.isTouchedArrayProperty )
			if( auto prop = handleArrayProperties( arrayData, tok ) ) return *prop;
		return arrayData;
	}
	std::vector<long int> path = getResolvedIndexVectors( tok );

	// a[ i ][ j ]:irangu removes element j of a[ i ] and gives it back
	if( tok.isTouchedArrayProperty && tok.arrProperty == "irangu" ){
		const long int last = path.back();
		path.pop_back();

		auto holder = arrayData->getElementAtIndex( path, 0 );
		auto* list = std::get_if<ArrayPtr>( &holder );
		if( !list )
			throw InvalidSyntaxError( "irangu is only applicable to kootam type" );

		auto removed = ArrayList<ARRAY_SUPPORT_TYPES>::removeElementAtIndex( *list, last );
		if( auto* nested = std::get_if<ArrayPtr>( &removed ) )
			return *nested;
		return deep( std::get<ARRAY_SUPPORT_TYPES>( removed ) );
	}
	auto element = arrayData->getElementAtIndex( path, 0 );

	if( auto* single = std::get_if<ARRAY_SUPPORT_TYPES>( &element ) ){
		DEEP_VALUE_DATA value = deep( *single );
		return tok.isTouchedArrayProperty ? DEEP_VALUE_DATA{ handlerVarDtypePrpperties( value, tok ) } : value;
	}
	auto& list = std::get<ArrayPtr>( element );
	if( tok.isTouchedArrayProperty )
		if( auto prop = handleArrayProperties( list, tok ) ) return *prop;
	return list;
}

VarDtype 
BodyEncounters::handlerVarDtypePrpperties( const DEEP_VALUE_DATA& Vdata, const ArrayAccessTokens& tok ){
	const std::string& property = tok.arrProperty;

	if( property == "jaadi" ) return ValueHelper::getValueType( Vdata );
	if( property == "kanam" ) return ValueHelper::getVariableSize( Vdata );

	const VarDtype* data = std::get_if<VarDtype>( &Vdata );

	if( property == "THA_ASCII" ){
		const std::string* str = data ? std::get_if<std::string>( data ) : nullptr;
		if( !str )
			throw InvalidSyntaxError( "THA_ASCII is only for string of size 1" );
		if( str->size() != 1 )
			throw InvalidSyntaxError( "ASCII is only for size 1" );

		return (long) (unsigned char) (*str)[ 0 ];
	}
	if( property == "PO_ASCII" ){
		const long* code = data ? std::get_if<long>( data ) : nullptr;
		if( !code )
			throw InvalidSyntaxError( "PO_ASCII is only for INT" );

		return std::string( 1, (char) *code );
	}
	throw InvalidSyntaxError( "Invalid variable property" );
}

DEEP_VALUE_DATA 
BodyEncounters::handleVarDtypeCases( const ArrayAccessTokens& tok, const VarDtype& varHolder ){
	if( tok.indexVector.empty() )
		return tok.isTouchedArrayProperty ? DEEP_VALUE_DATA{ handlerVarDtypePrpperties( DEEP_VALUE_DATA{ varHolder }, tok ) }
										  : DEEP_VALUE_DATA{ varHolder };

	const std::string* str = std::get_if<std::string>( &varHolder );
	if( !str )
		throw InvalidSyntaxError( "Indexing Invalid DType" );

	if( tok.indexVector.size() > 1 )
		throw InvalidSyntaxError( "Indexing ND on 1D type" );

	const long int index = getResolvedIndexVectors( tok ).back();
	if( index < 0 || index >= (long int) str->size() )
		throw ArrayOutOfBound( std::to_string( index ) );	// used to hand back the whole string

	DEEP_VALUE_DATA letter{ VarDtype{ std::string( 1, (*str)[ index ] ) } };
	return tok.isTouchedArrayProperty ? DEEP_VALUE_DATA{ handlerVarDtypePrpperties( letter, tok ) } : letter;
}

FUNC_RETURN_TYPE
BodyEncounters::handleFunctionCall( 
					std::shared_ptr<MapItem>& func, 
					const std::vector<Token>& tokens, 
					size_t& currentPtr, 
					const unsigned int KEY, 
					const FunctionCallReturns* data
	){
	EXT_TYPE* cached = findPre( KEY );
	if( !cached ){
		FunctionCallReturns call = data ? *data : stringToFunctionCallTokens( tokens, currentPtr );
		passValidFuncCallToken( call.callTokens ); 

		ExtendedFunctionCall ext;
		ext.endPtr = currentPtr;
		for( auto& arg: call.argsVector )
			ext.argsVcts.push_back( ExprResolver::getAstRootNode( arg, funcHandler ) );
		ext.tokens = std::move( call );

		preComputed[ KEY ] = std::move( ext );
		indexPre( KEY );
		cached = findPre( KEY );
	}
	ExtendedFunctionCall& ext = std::get<ExtendedFunctionCall>( *cached );
	currentPtr = ext.endPtr;

	// keeps the pindi alive even if its variable is re-assigned while it runs
	FuncPtr def = std::get<FuncPtr>( func->var );

	if( ext.argsVcts.empty() && !def->argsInfo.empty() )
		throw InvalidSyntaxError( "pindi need args" );

	if( ext.argsVcts.size() > def->argsInfo.size() )
		throw InvalidSyntaxError( "Too many arguments for pindi " + ext.tokens.funcName );	// used to be ignored silently

	FunctionHandler runner( funcHandler->ctx );
	runner.runnerBody = ext.tokens.funcName;
	runner.VMAP_COPY  = def->varMapCopy.first;
	runner.parent 	  = def->varMapCopy.second;

	// arguments are evaluated in the caller's scope, they become variables of the new one
	for( size_t i = 0; i < def->argsInfo.size(); i++ ){
		const ARG_VAR_INFO& arg = def->argsInfo[ i ];

		if( i >= ext.argsVcts.size() )
			throw InvalidSyntaxError( "No value to initialize the args in function" );

		if( runner.VMAP_COPY->count( arg.name ) )
			throw VariableAlreayExists( arg.name );

		DEEP_VALUE_DATA value = ExprResolver::getFinalValue( ext.argsVcts[ i ], funcHandler );

		if( std::holds_alternative<VarDtype>( value ) && arg.isArray )
			throw InvalidSyntaxError( "Argument expects kootam\n" );

		if( std::holds_alternative<ArrayPtr>( value ) && !arg.isArray )
			throw InvalidSyntaxError( "Argument is not kootam type" );

		runner.addToMap( arg.name, MapItem::make( arg.name, std::move( value ) ) );
	}
	size_t bodyStart = def->bodyStartPtr + 1;
	return ProgramExecutor( tokens, bodyStart, CALLER::FUNCTION, &runner, def->bodyEndPtr );
}

void BodyEncounters::updateString( std::string& strToUpdate, long int index, const DEEP_VALUE_DATA& updata ){
	const VarDtype* value = std::get_if<VarDtype>( &updata );
	const std::string* replacement = value ? std::get_if<std::string>( value ) : nullptr;

	if( !replacement )
		throw InvalidSyntaxError( "Invalid string updation dtype" );

	if( index < 0 || index >= (long int) strToUpdate.size() )
		throw ArrayOutOfBound( std::to_string( index ) );

	strToUpdate.replace( index, 1, *replacement );
}

// index expressions are parsed once ( lazily ) and their ASTs are kept inside the access token
std::vector<long int> BodyEncounters::getResolvedIndexVectors( const ArrayAccessTokens& tok ){
	const size_t total = tok.indexVector.size();
	std::vector<long int> resolved;
	resolved.reserve( total );

	tok.indexAst.resize( total );
	for( size_t k = 0; k < total; k++ ){
		if( !tok.indexAst[ k ] )
			tok.indexAst[ k ] = keepAst( ExprResolver::getAstRootNode( tok.indexVector[ k ], funcHandler ) );

		DEEP_VALUE_DATA val = ExprResolver::evalNode( static_cast<AST_NODE_PTR>( tok.indexAst[ k ].get() ), funcHandler, 0 );

		const VarDtype* number = std::get_if<VarDtype>( &val );
		if( !number )
			throw InvalidSyntaxError( "Array Index Expects Type Integer" );

		if( auto* li = std::get_if<long int>( number ) )  resolved.push_back( *li );
		else if( auto* di = std::get_if<double>( number ) ) resolved.push_back( (long int) *di );
		else throw InvalidSyntaxError( "Array Index Expects Integer" );
	}
	return resolved;
}

// NOTE: the token vector must stay alive and unchanged for the whole program run ( key is the token address )
const ArrayAccessTokens& BodyEncounters::getCachedArrayAccess( const std::vector<Token>& tokens, size_t& curPtr ){
	struct CachedAccess{ ArrayAccessTokens tok; size_t endPtr; };
	static std::unordered_map<const Token*, CachedAccess> accessCache;

	const Token* key = tokens.data() + curPtr;
	auto it = accessCache.find( key );

	if( it == accessCache.end() ){
		size_t endPtr = curPtr;
		ArrayAccessTokens parsed = stringToArrayAccesToken( tokens, endPtr );
		it = accessCache.emplace( key, CachedAccess{ std::move( parsed ), endPtr } ).first;
	}
	curPtr = it->second.endPtr;
	return it->second.tok;
}

void BodyEncounters::arrayUpdation( const std::vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, ArrayPtr& arr ){
	const ArrayAccessTokens& tok = getCachedArrayAccess( tokens, curPtr );
	curPtr--;

	if( tok.isTouchedArrayProperty )
		throw InvalidSyntaxError( "Using array property in updatation is not allowed" );

	// no index : the whole kootam is replaced
	if( tok.indexVector.empty() ){
		auto* list = std::get_if<ArrayPtr>( &upvalue );
		if( !list )
			throw InvalidSyntaxError( "Dtype mismatch in array updation" );

		arr = std::move( *list );
		return;
	}
	std::vector<long int> path = getResolvedIndexVectors( tok );
	const long int index = path.back();
	path.pop_back();

	if( index < 0 )
		throw InvalidSyntaxError( "Array Index Expects Unsigned Integer" );

	auto target = arr->getElementAtIndex( path, 0 );

	// a[ ... ][ index ] = "x"  where a[ ... ] is a string : change that letter
	if( auto* single = std::get_if<ARRAY_SUPPORT_TYPES>( &target ) ){
		auto* value = std::get_if<VarDtype>( single );
		auto* text = value ? std::get_if<std::string>( value ) : nullptr;
		if( !text )
			throw InvalidSyntaxError( "Failed to index an non array type" );

		updateString( *text, index, upvalue );

		// the lookup hands out a copy : store the changed string back ( the change used to be lost )
		const long int slot = path.back();
		path.pop_back();
		std::get<ArrayPtr>( arr->getElementAtIndex( path, 0 ) )->arrayList[ slot ] = std::move( *single );
		return;
	}
	auto& list = std::get<ArrayPtr>( target );

	while( list->totalElementsAllocated <= (size_t) index )
		list->push_SingleElement( VarDtype{ std::monostate{} } );

	std::visit( [&]( auto&& item ){ list->arrayList[ index ] = std::move( item ); }, upvalue );
}

void BodyEncounters::typeCastRequest( InstructionTokens& ins, std::vector<Token>& names ){
	for( size_t x = 0; x < names.size(); x++ ){
		if( x >= ins.rightVector.size() || ins.rightVector[ x ].size() != 1 )
			throw InvalidSyntaxError( "Typecasting error" );

		auto item = funcHandler->getFromVmap( names[ x ].token );
		if( item == nullptr )
			throw InvalidSyntaxError( "Unknown token: " + names[ x ].token );

		item->typeCast( ins.rightVector[ x ].back() );
	}
}

FUNC_RETURN_TYPE
BodyEncounters::getReturnedData( const std::vector<Token>&tokens, size_t& currentPtr ){
	static std::unordered_map<unsigned int, ExtendedReturnToken> returnCache;

	auto it = returnCache.find( tokens[ currentPtr ].tokenId );
	if( it == returnCache.end() ){
		ExtendedReturnToken ret;
		std::vector<Token> expr;

		size_t at = currentPtr + 1;
		for( ; at < tokens.size() && !( tokens[ at ].token == ";" && tokens[ at ].type == TOKEN_TYPE::SPEC_CHAR ); at++ )
			expr.push_back( tokens[ at ] );

		ret.endPtr = std::min( at + 1, tokens.size() );

		if( expr.size() == 1 && expr[ 0 ].type == TOKEN_TYPE::IDENTIFIER )
			ret.identifier = expr[ 0 ].token;
		else if( !expr.empty() )
			ret.ast = ExprResolver::getAstRootNode( expr, funcHandler );

		it = returnCache.emplace( tokens[ currentPtr ].tokenId, std::move( ret ) ).first;
	}
	ExtendedReturnToken& ret = it->second;
	currentPtr = ret.endPtr;

	if( !ret.identifier.empty() ){	// poda name;  hands over the variable itself ( no copy )
		auto item = funcHandler->getFromVmap( ret.identifier );
		if( item == nullptr )
			throw InvalidSyntaxError( "No variable found " + ret.identifier + "\nRemember variable should be create inside the function body" );
		return item;
	}
	if( !ret.ast )
		return std::nullopt;

	DEEP_VALUE_DATA res = ExprResolver::getFinalValue( ret.ast, funcHandler );

	if( auto* v = std::get_if<VarDtype>( &res ) )
		return std::move( *v );

	if( auto* a = std::get_if<ArrayPtr>( &res ) )
		return std::move( *a );

	throw InvalidSyntaxError( "Invalid return statement" );
}
