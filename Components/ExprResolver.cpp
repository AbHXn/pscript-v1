#include "Headers/ExprResolver.hpp"
#include "Headers/FunctionHandler.hpp"

using Node = AST_NODE<REAL_AST_NODE_DATA>;

std::unique_ptr<Node>
ExprResolver::getAstRootNode( const std::vector<Token>& vtr, FunctionHandler* func ){
	RESOLVER_TYPE resolved = vectorResolver( vtr, func );
	auto astTokens = stringToASTTokens( resolved.simpleVector );
	size_t startAST = 0;

	auto root = BUILD_AST<REAL_AST_NODE_DATA>( astTokens, resolved.resolvedAstNodeData, startAST );
	if( !root.has_value() )
		throw InvalidDTypeError( "Failed to resolve the vector\n" );
	return std::move( root.value() );
}

DEEP_VALUE_DATA
ExprResolver::getFinalValue( const std::unique_ptr<Node>& newAstNode, FunctionHandler* func ){
	return evalNode( newAstNode.get(), func, 0 );
}

RESOLVER_TYPE
ExprResolver::vectorResolver( const std::vector<Token>& tokens, FunctionHandler* func ){
	RESOLVER_TYPE out;

	auto add = [&]( REAL_AST_NODE_DATA data, const char* kind ){
		out.resolvedAstNodeData.push( std::move( data ) );
		out.simpleVector.push_back( kind );
	};

	for( size_t x = 0; x < tokens.size(); x++ ){
		const Token& tok = tokens[ x ];
		const std::string& cur = tok.token;
		const bool isValue = isValueType( tok.type );

		if( !isValue && ( cur == "(" || cur == ")" || isRegisteredASTExprTokens( cur ) ) )
			out.simpleVector.push_back( cur );

		else if( isValue )
			add( ValueHelper::getValueFromToken( tok ), "NUM" );

		else if( tok.type == TOKEN_TYPE::RESERVED && cur == "onnula" )
			add( VarDtype{ std::monostate{} }, "NONE" );

		else if( tok.type == TOKEN_TYPE::RESERVED && cur == "edukku" )
			add( std::string( "edukku" ), "NUM" );

		else if( tok.type == TOKEN_TYPE::IDENTIFIER ){
			auto item = func->getFromVmap( cur );
			if( item == nullptr )
				throw InvalidSyntaxError( "Unknown identifier " + cur );

			if( item->mapType == MAPTYPE::FUNC_PTR ){
				add( std::make_pair( stringToFunctionCallTokens( tokens, x ), tok ), "CACHE" );
			}
			else{
				ArrayAccessTokens access = stringToArrayAccesToken( tokens, x );
				passArrayAccessToken( access.tokens );
				add( std::make_pair( std::move( access ), cur ), "VAR" );
			}
			x--;
		}
		else if( cur == "{" && tok.type == TOKEN_TYPE::SPEC_CHAR ){
			size_t endPtr = 0;
			add( std::make_pair( stringToVariableTokens( tokens, endPtr, false ), tok ), "ARRAY" );
			x = endPtr;
		}
		else throw InvalidSyntaxError( "Unknown Token " + cur );
	}
	return out;
}

// ---------------------------------------------------------------------------------
// numbers ( long / double / bool ) are computed on a small tagged union, no variant, no allocation
// ---------------------------------------------------------------------------------
struct Num{
	enum Kind : unsigned char { L, D, B } kind;
	union{ long l; double d; bool b; };

	double dbl()    const { return kind == L ? (double) l : ( kind == D ? d : (double) b ); }
	long   lng()    const { return kind == L ? l : ( kind == D ? (long) d : (long) b ); }
	bool   truthy() const { return kind == L ? l != 0 : ( kind == D ? d != 0.0 : b ); }

	static Num ofLong( long v )     { Num n; n.kind = L; n.l = v; return n; }
	static Num ofDouble( double v ) { Num n; n.kind = D; n.d = v; return n; }
	static Num ofBool( bool v )     { Num n; n.kind = B; n.b = v; return n; }
};

static inline bool toNum( const VarDtype& v, Num& out ){
	if( const long* a = std::get_if<long>( &v ) )   { out = Num::ofLong( *a );   return true; }
	if( const double* a = std::get_if<double>( &v ) ){ out = Num::ofDouble( *a ); return true; }
	if( const bool* a = std::get_if<bool>( &v ) )   { out = Num::ofBool( *a );   return true; }
	return false;
}

static inline VarDtype fromNum( const Num& n ){
	return n.kind == Num::L ? VarDtype( n.l ) : ( n.kind == Num::D ? VarDtype( n.d ) : VarDtype( n.b ) );
}

template<typename T>
static inline bool compare( AST_TOKENS op, const T& x, const T& y, bool& r ){
	switch( op ){
		case AST_TOKENS::LS_THAN:       r = x <  y; return true;
		case AST_TOKENS::GT_THAN:       r = x >  y; return true;
		case AST_TOKENS::LS_THAN_EQ:    r = x <= y; return true;
		case AST_TOKENS::GT_THAN_EQ:    r = x >= y; return true;
		case AST_TOKENS::D_EQUAL_TO:    r = x == y; return true;
		case AST_TOKENS::NOT_EQUAL_TO:  r = x != y; return true;
		default: return false;
	}
}

static bool numBinary( AST_TOKENS op, const Num& a, const Num& b, Num& out ){
	bool r;
	// two ints stay exact ( no detour through double )
	if( a.kind == Num::L && b.kind == Num::L ){
		switch( op ){
			case AST_TOKENS::ADD: out = Num::ofLong( a.l + b.l ); return true;
			case AST_TOKENS::SUB: out = Num::ofLong( a.l - b.l ); return true;
			case AST_TOKENS::MUL: out = Num::ofLong( a.l * b.l ); return true;
			default: break;
		}
		if( compare( op, a.l, b.l, r ) ){ out = Num::ofBool( r ); return true; }
	}
	else{
		const double x = a.dbl(), y = b.dbl();
		switch( op ){
			case AST_TOKENS::ADD: out = Num::ofDouble( x + y ); return true;
			case AST_TOKENS::SUB: out = Num::ofDouble( x - y ); return true;
			case AST_TOKENS::MUL: out = Num::ofDouble( x * y ); return true;
			default: break;
		}
		if( compare( op, x, y, r ) ){ out = Num::ofBool( r ); return true; }
	}
	switch( op ){
		case AST_TOKENS::DIV: out = Num::ofDouble( a.dbl() / b.dbl() ); return true;
		case AST_TOKENS::MOD: {
			const long divisor = b.lng();
			if( divisor == 0 )
				throw InvalidSyntaxError( "Modulo by zero" );	// used to crash the whole process

			out = Num::ofLong( divisor == -1 ? 0 : a.lng() % divisor );	// LONG_MIN % -1 would overflow
			return true;
		}
		default: return false;
	}
}

static std::string asText( const VarDtype& v ){
	return std::visit( []( const auto& x ){ return ValueHelper::toString( x ); }, v );
}

static VarDtype binary( AST_TOKENS op, const VarDtype& l, const VarDtype& r ){
	Num a, b, res;
	if( toNum( l, a ) && toNum( r, b ) ){
		if( !numBinary( op, a, b, res ) )
			throw InvalidSyntaxError( "Unknown operator for numbers" );
		return fromNum( res );
	}
	const std::string* ls = std::get_if<std::string>( &l );
	const std::string* rs = std::get_if<std::string>( &r );

	if( ls && rs ){
		bool res2;
		if( op == AST_TOKENS::ADD ) return *ls + *rs;
		if( compare( op, *ls, *rs, res2 ) ) return res2;
		throw InvalidSyntaxError( "Invalid operator for strings" );
	}
	if( ls || rs ){
		if( op == AST_TOKENS::ADD ) return asText( l ) + asText( r );
		throw InvalidSyntaxError( "Invalid operator involving string" );
	}
	// whatever is left involves onnula
	const bool same = std::holds_alternative<std::monostate>( l ) && std::holds_alternative<std::monostate>( r );
	if( op == AST_TOKENS::D_EQUAL_TO )   return same;
	if( op == AST_TOKENS::NOT_EQUAL_TO ) return !same;
	throw InvalidSyntaxError( "Invalid operator involving onnula" );
}

// ---------------------------------------------------------------------------------
// evaluation
// ---------------------------------------------------------------------------------
// the plain value of a variable that is read without index / property
static inline const VarDtype* plainValue( const ArrayAccessTokens& access, const std::string& key, FunctionHandler* h ){
	if( !access.indexVector.empty() || access.isTouchedArrayProperty )
		return nullptr;

	const std::shared_ptr<MapItem>* item = h->lookup( key );
	if( !item || (*item)->mapType != MAPTYPE::VARIABLE )
		return nullptr;

	auto& holder = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( (*item)->var );
	return holder->isTypeArray ? nullptr : &std::get<VarDtype>( holder->value );
}

// side effect free evaluation of numeric trees ( false = not purely numeric, use the general path )
static bool tryNum( AST_NODE_PTR node, FunctionHandler* h, Num& out ){
	switch( node->AST_DATA.index() ){
		case 1:
			return toNum( std::get<VarDtype>( node->AST_DATA ), out );

		case 2: {
			auto& [access, key] = std::get<std::pair<ArrayAccessTokens, std::string>>( node->AST_DATA );
			const VarDtype* v = plainValue( access, key, h );
			return v && toNum( *v, out );
		}
		case 0: {
			if( !node->left || !node->right || !node->isASTTokens )
				return false;

			const AST_TOKENS op = std::get<AST_TOKENS>( node->AST_DATA );
			Num a, b;
			if( !tryNum( node->left.get(), h, a ) )
				return false;

			if( op == AST_TOKENS::AND || op == AST_TOKENS::OR ){
				const bool left = a.truthy();
				if( left == ( op == AST_TOKENS::OR ) ){ out = Num::ofBool( left ); return true; }
				if( !tryNum( node->right.get(), h, b ) ) return false;
				out = Num::ofBool( b.truthy() );
				return true;
			}
			return tryNum( node->right.get(), h, b ) && numBinary( op, a, b, out );
		}
		default: return false;
	}
}

static VarDtype scalar( AST_NODE_PTR node, FunctionHandler* h, size_t level );

static VarDtype operation( AST_NODE_PTR node, FunctionHandler* h, size_t level ){
	if( !node->left || !node->right )
		throw InvalidSyntaxError( "Invalid Expression" );

	const AST_TOKENS op = node->isASTTokens ? std::get<AST_TOKENS>( node->AST_DATA ) : AST_TOKENS::ADD;
	VarDtype left = scalar( node->left.get(), h, level + 1 );

	if( op == AST_TOKENS::AND || op == AST_TOKENS::OR ){
		if( ValueHelper::toBool( left ) == ( op == AST_TOKENS::OR ) )
			return op == AST_TOKENS::OR;	// short circuit

		return ValueHelper::toBool( scalar( node->right.get(), h, level + 1 ) );
	}
	return binary( op, left, scalar( node->right.get(), h, level + 1 ) );
}

// operand of an operator : literal, plain variable or anything that evaluates to a single value
static VarDtype scalar( AST_NODE_PTR node, FunctionHandler* h, size_t level ){
	switch( node->AST_DATA.index() ){
		case 0: return operation( node, h, level );
		case 1: return std::get<VarDtype>( node->AST_DATA );
		case 2: {
			auto& [access, key] = std::get<std::pair<ArrayAccessTokens, std::string>>( node->AST_DATA );
			if( const VarDtype* v = plainValue( access, key, h ) ) return *v;
			break;
		}
		default: break;
	}
	return std::get<VarDtype>( ExprResolver::evalNode( node, h, level ) );
}

// only a top level expression may produce a kootam, everything inside an operation must be a single value
static inline DEEP_VALUE_DATA singleValueAtDepth( DEEP_VALUE_DATA value, size_t level ){
	if( level == 0 || std::holds_alternative<VarDtype>( value ) )
		return value;
	throw InvalidSyntaxError( "Invalid dtype for operation" );
}

DEEP_VALUE_DATA
ExprResolver::evalNode( AST_NODE_PTR node, FunctionHandler* h, size_t level ){
	REAL_AST_NODE_DATA& data = const_cast<REAL_AST_NODE_DATA&>( node->AST_DATA );

	switch( data.index() ){
		case 0: {
			Num n;
			if( tryNum( node, h, n ) ) return fromNum( n );
			return operation( node, h, level );
		}
		case 1: return std::get<VarDtype>( data );

		case 4: {
			if( std::get<std::string>( data ) != "edukku" )
				throw InternalError( "while resolving expr" );

			std::string line;
			std::getline( std::cin, line );
			return VarDtype( std::move( line ) );
		}
		case 2: {	// variable / kootam access
			auto& [access, key] = std::get<std::pair<ArrayAccessTokens, std::string>>( data );
			const std::shared_ptr<MapItem>* found = h->lookup( key );
			if( !found )
				throw InvalidSyntaxError( "Unknown identifier " + key );

			MapItem* item = found->get();
			BodyEncounters body( h );

			if( item->mapType == MAPTYPE::VARIABLE ){
				auto& holder = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( item->var );

				if( !holder->isTypeArray ){
					return singleValueAtDepth( body.handleVarDtypeCases( access, std::get<VarDtype>( holder->value ) ), level );
				}
				auto array = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( holder->value );	// keeps it alive
				return singleValueAtDepth( body.handleArrayCases( array, access ), level );
			}
			if( item->mapType == MAPTYPE::ARRAY_PTR ){
				auto array = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( item->var );
				return singleValueAtDepth( body.handleArrayCases( array, access ), level );
			}
			break;
		}
		case 3: {	// pindi call or pindi name used as a value
			auto& [call, nameTok] = std::get<std::pair<FunctionCallReturns, Token>>( data );
			const std::shared_ptr<MapItem>* found = h->lookup( nameTok.token );
			if( !found )
				throw InvalidSyntaxError( "Unknown identifier " + nameTok.token );

			std::shared_ptr<MapItem> item = *found;
			if( item->mapType != MAPTYPE::FUNC_PTR )
				throw InvalidSyntaxError( nameTok.token + " is not a pindi" );

			if( isFuncPtr( call.callTokens ) )
				return std::get<std::shared_ptr<FUNCTION_MAP_DATA>>( item->var );

			size_t unused = 0;
			FUNC_RETURN_TYPE result = BodyEncounters( h ).handleFunctionCall( item, h->ctx->fullTokens, unused, nameTok.tokenId, &call );
			if( !result ) break;

			if( auto* v = std::get_if<VarDtype>( &*result ) )
				return std::move( *v );

			if( auto* m = std::get_if<std::shared_ptr<MapItem>>( &*result ) )
				return singleValueAtDepth( ValueHelper::getFinalValueFromMap( *m ), level );

			return singleValueAtDepth( std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( *result ), level );
		}
		case 5: {	// { a, b, { c } } used as a value
			if( level != 0 )
				break;

			VariableTokens& literal = std::get<std::pair<VariableTokens, Token>>( data ).first;

			if( !literal.preparedAsExpr ){	// validated once, the tokens never change afterwards
				literal.valueTokens.push_back( VALUE_TOKENS::VALUE_END );
				passValidValueTokens( literal.valueTokens );
				literal.preparedAsExpr = true;
			}
			const size_t total = literal.valueVector.size();
			literal.elementAst.resize( total );

			ValueQueue values;
			values.reserve( total );
			for( size_t i = 0; i < total; i++ ){
				if( !literal.elementAst[ i ] )
					literal.elementAst[ i ] = keepAst( getAstRootNode( literal.valueVector[ i ], h ) );
				values.push( evalNode( static_cast<AST_NODE_PTR>( literal.elementAst[ i ].get() ), h, 0 ) );
			}
			size_t from = 1;
			return ArrayList<ARRAY_SUPPORT_TYPES>::createArray<DEEP_VALUE_DATA>( literal.valueTokens, from, values );
		}
		default: break;
	}
	throw InvalidSyntaxError( "Invalid dtype for operation" );
}
