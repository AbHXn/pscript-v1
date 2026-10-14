#include "Headers/FunctionHandler.hpp"

FunctionHandler::FunctionHandler( VAR_VMAP* parent, const std::string& runnerBody, Context* ctx ){
	this->runnerBody = runnerBody;
	this->parent = parent;
	this->ctx = ctx;
}

void FunctionHandler::VarHandlerRunner( const std::vector<Token>& test, size_t& start, const unsigned int KEY ){
	EXT_TYPE* itCache = findPre( KEY );
	if( itCache == nullptr ){
		PreComputedCaching::VariableCaching( test, start, KEY, this );
		indexPre( KEY );
		itCache = findPre( KEY );
	}

	ExtendedVariableToken& tokens = std::get<ExtendedVariableToken>( (*itCache) );
	start = tokens.endPtr;

	ValueQueue resolvedValueVector;
	resolvedValueVector.reserve( tokens.valueAst.size() );

	for( size_t x = 0; x < tokens.valueAst.size(); x++ ){
		auto& dataV = tokens.valueAst[x];
		resolvedValueVector.push( ExprResolver::getFinalValue( dataV, this ) );
	}

	std::vector<VAR_INFO>& varInfos = tokens.varInfos;

	for( size_t x = 0, i = 0; x < tokens.tokens.valueTokens.size(); x++ ){
		if( i >= varInfos.size() && resolvedValueVector.empty() ) break;

		const VALUE_TOKENS curValueToken = tokens.tokens.valueTokens[x];

		if( curValueToken != VALUE_TOKENS::NORMAL_VALUE && curValueToken != \
			VALUE_TOKENS::ARRAY_VALUE && curValueToken != VALUE_TOKENS::ARRAY_OPEN )
			continue;

		if( i >= varInfos.size() && !resolvedValueVector.empty() )
			throw InvalidSyntaxError("No variable to initialize the value");

		const VAR_INFO& curVarInfo = varInfos[ i++ ];

		if( this->lookup( curVarInfo.varName ) )
			throw VariableAlreayExists( curVarInfo.varName );

		if( curValueToken == VALUE_TOKENS::NORMAL_VALUE ){
			DEEP_VALUE_DATA curValue = std::move( resolvedValueVector.front() );
			resolvedValueVector.pop();

			const bool isArrayValue = std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( curValue );

			if( curVarInfo.isTypeArray && !isArrayValue )
				throw InvalidSyntaxError("Assigning value to kootam type is invalid");

			if( !curVarInfo.isTypeArray && isArrayValue )
				throw InvalidSyntaxError("Trying to Assign kootam type to non kootam type");

			if( std::holds_alternative<VarDtype> ( curValue ) ){
				std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = std::make_shared<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
				newVariable->key 		 = curVarInfo.varName;
				newVariable->isTypeArray = false;
				newVariable->value 		 = std::move( std::get<VarDtype>( curValue ) );

				auto newMapVar 		= std::make_shared<MapItem>( );
				newMapVar->mapType 	= MAPTYPE::VARIABLE;
				newMapVar->var 	  	= std::move( newVariable );
				
				this->addToMap( curVarInfo.varName, std::move( newMapVar ) );
			}
			else if( isArrayValue ){
				const std::string& key = curVarInfo.varName;
				auto newMapVar 		= std::make_shared<MapItem>( );
				newMapVar->mapType 	= MAPTYPE::ARRAY_PTR;
				newMapVar->var 		= std::move( std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>(curValue) );
				
				this->addToMap( key, std::move( newMapVar ) );
			}
			else if( std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>( curValue ) ){
				const std::string& key = curVarInfo.varName;
				auto newMapVar 		= std::make_shared<MapItem>( );
				newMapVar->mapType 	= MAPTYPE::FUNC_PTR;
				newMapVar->var 		= std::move( std::get<std::shared_ptr<FUNCTION_MAP_DATA>>(curValue) );
				
				this->addToMap( key, std::move( newMapVar ) );
			}
			else throw InternalError("Type not defined");
		}
		else if( curValueToken == VALUE_TOKENS::ARRAY_OPEN ){
			x++;

			if( !curVarInfo.isTypeArray )
				throw InvalidSyntaxError("Expected kootam type");

			auto newArray = ArrayList<ARRAY_SUPPORT_TYPES>::createArray<DEEP_VALUE_DATA>(tokens.tokens.valueTokens,  x, resolvedValueVector );

			std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = std::make_shared<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
			newVariable->key 		 = curVarInfo.varName;
			newVariable->isTypeArray = true;
			newVariable->value 		 = std::move( newArray );

			auto newMapVar 		= std::make_shared<MapItem>( );
			newMapVar->mapType 	= MAPTYPE::VARIABLE;
			newMapVar->var 		= std::move( newVariable );

			this->addToMap( curVarInfo.varName, std::move( newMapVar ) );
		}
	}
}

void FunctionHandler::InstructionHandlerRunner( const std::vector<Token>& tokens, size_t& currentPtr, const unsigned int KEY ){
	EXT_TYPE* itCache = findPre( KEY );
	if( itCache == nullptr ){
		PreComputedCaching::InstructionCaching( tokens, currentPtr, KEY, this );
		indexPre( KEY );
		itCache = findPre( KEY );
	}

	BodyEncounters bodyEncouter( this );
	ExtendedInsTokens& insTokens = std::get<ExtendedInsTokens>( (*itCache) );
	currentPtr = insTokens.endPtr;

	std::vector<Token>& varsAndVals = insTokens.InsTokensAndData.leftVector;

	if( insTokens.InsTokensAndData.optr == INS_TOKEN::TYPE_CAST ){
		bodyEncouter.typeCastRequest( insTokens.InsTokensAndData, varsAndVals );
		return;
	}

	// assigns one evaluated value to the variable that starts at varsAndVals[ x ]
	auto assignValue = [&]( size_t& x, DEEP_VALUE_DATA&& topValue ){
		const std::shared_ptr<MapItem>* mapPtr = this->lookup( varsAndVals[ x ].token );
		
		if( mapPtr == nullptr )
			throw InvalidSyntaxError("Unknown token: " + varsAndVals[x].token );

		MapItem* mapData = mapPtr->get();

		if( mapData->mapType != MAPTYPE::VARIABLE && mapData->mapType != MAPTYPE::ARRAY_PTR )
			throw InvalidSyntaxError("Only variables are allowed for updation");

		if( std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>( topValue ) ){
			mapData->mapType = MAPTYPE::FUNC_PTR;
			mapData->var = std::move( std::get<std::shared_ptr<FUNCTION_MAP_DATA>>(topValue) );
			return;
		}
		if( mapData->mapType == MAPTYPE::ARRAY_PTR ){
			auto& arr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( mapData->var );
			bodyEncouter.arrayUpdation( varsAndVals, x, std::move( topValue ), arr );
			return;
		}
		auto& vmapvariable = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( mapData->var );
		if( std::holds_alternative<VarDtype> ( vmapvariable->value ) ){
			auto& varDtypeData = std::get<VarDtype>( vmapvariable->value );

			if( std::holds_alternative<std::string> ( varDtypeData ) ){
				const ArrayAccessTokens& arrToken = bodyEncouter.getCachedArrayAccess( varsAndVals, x );
				x--;

				auto resData = bodyEncouter.getResolvedIndexVectors( arrToken );
				if( resData.size() > 1 )
					throw InvalidSyntaxError("string is a one dimensional");
				
				std::string& strToUpdate = std::get<std::string> ( varDtypeData );
				if ( !std::holds_alternative<VarDtype>( topValue ) )
					throw InvalidSyntaxError("Variable dtype mismatch mismatch");
				
				// no index : the whole value is replaced below
				if( !resData.empty() )
					bodyEncouter.updateString( strToUpdate, (long int) resData.back(), topValue );
			}
		}
		if( vmapvariable->isTypeArray ){
			auto& arr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( vmapvariable->value );
			bodyEncouter.arrayUpdation( varsAndVals, x, std::move( topValue ), arr);
		}
		else if( std::holds_alternative<VarDtype>( topValue ) )
			mapData->updateSingleVariable( std::move( std::get<VarDtype>( topValue ) ) );
	};

	// most common shape:  var = expr;   -> no queue needed
	if( insTokens.insTree.size() == 1 && varsAndVals.size() == 1 ){
		DEEP_VALUE_DATA single = ExprResolver::getFinalValue( insTokens.insTree[ 0 ], this );
		size_t x = 0;
		assignValue( x, std::move( single ) );
		return;
	}

	ValueQueue finalValueQueue;
	finalValueQueue.reserve( insTokens.insTree.size() );

	for( auto& astStrToks: insTokens.insTree )
		finalValueQueue.push( ExprResolver::getFinalValue( astStrToks, this ) );

	for( size_t x = 0; x < varsAndVals.size(); x++ ){
		// the variable is looked up first so the error order stays the same
		if( finalValueQueue.empty() ){
			const std::shared_ptr<MapItem>* chk = this->lookup( varsAndVals[ x ].token );
			if( chk == nullptr )
				throw InvalidSyntaxError("Unknown token: " + varsAndVals[x].token );
			throw InvalidSyntaxError("No value to assign for " + varsAndVals[x].token );
		}
		DEEP_VALUE_DATA topValue = std::move( finalValueQueue.front( ) );
		finalValueQueue.pop( );
		assignValue( x, std::move( topValue ) );
	}
}

void FunctionHandler::functionDefHandlerRunner( const std::vector<Token>&token, size_t& start ){
	FunctionTokenReturn funcTokens = stringToFuncTokens( token, start );
	passValidFuncToken( funcTokens.tokens );

	if( this->lookup( funcTokens.funcName ) != nullptr )
		throw InvalidSyntaxError( funcTokens.funcName + " already defined" );
	
	std::shared_ptr<FUNCTION_MAP_DATA> funcMapData = std::make_shared<FUNCTION_MAP_DATA>();
	funcMapData->funcName 		= funcTokens.funcName;
	funcMapData->argsSize 		= funcTokens.args.size();
	funcMapData->bodyStartPtr 	= funcTokens.funcStartPtr;
	funcMapData->bodyEndPtr 	= funcTokens.funcEndPtr;
	funcMapData->argsInfo 		= std::move( funcTokens.args );
	
	auto vmapCopy = std::make_shared<VMAP_TYPE>( this->getDeepCopyOfVMAP() );

	funcMapData->varMapCopy				 			   = std::make_pair( vmapCopy, this->parent );
	std::shared_ptr<MapItem> funcMapItem 			   = std::make_shared<MapItem>();
	funcMapItem->mapType 				 			   = MAPTYPE::FUNC_PTR;
	(*funcMapData->varMapCopy.first)[funcTokens.funcName] = funcMapItem;
	funcMapItem->var 	 							   = funcMapData;

	this->addToMap( funcTokens.funcName, funcMapItem );
}

void FunctionHandler::IOHandlerRunner( const std::vector<Token>& tokens, size_t& start, const unsigned int KEY ){
	EXT_TYPE* itCache = findPre( KEY );
	if( itCache == nullptr ){
		PreComputedCaching::IOCaching( tokens, start, KEY, this );
		indexPre( KEY );
		itCache = findPre( KEY );
	}

	ExtendedIoToken& IoTokens = std::get<ExtendedIoToken>( (*itCache) );
	start = IoTokens.endPtr;

	ValueQueue finalQueue;
	finalQueue.reserve( IoTokens.outputInfo.size() );
	
	for( auto& valTree: IoTokens.outputInfo )
		finalQueue.push( ExprResolver::getFinalValue( valTree, this ) );

	bool isFirstPrint = true;
	for( IO_TOKENS tok: IoTokens.insTokens.first ){
		switch( tok ){
			case IO_TOKENS::PRINT: {
				if( !isFirstPrint )
					std::cout << '\n';
				else isFirstPrint = false;

				if( finalQueue.empty() )
					throw InvalidSyntaxError("In para statement");

				ValueHelper::printDEEP_VALUE_DATA( finalQueue.front() );
				finalQueue.pop();
				break;
			}
			case IO_TOKENS::CONCAT:{
				std::cout << ' ';
				if( finalQueue.empty() )
					throw InvalidSyntaxError("In koode statement");

				ValueHelper::printDEEP_VALUE_DATA( finalQueue.front() );
				finalQueue.pop();
				break;
			}
			case IO_TOKENS::PRINT_VALUE: continue;
			case IO_TOKENS::END: default: return;
		}
	}
}

// evaluates a condition and insists on a boolean result
static inline bool conditionIsTrue( std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& tree, FunctionHandler* handler, const char* errorMessage ){
	DEEP_VALUE_DATA value = ExprResolver::getFinalValue( tree, handler );

	const VarDtype* scalar = std::get_if<VarDtype>( &value );
	const bool* flag = scalar ? std::get_if<bool>( scalar ) : nullptr;

	if( !flag )
		throw InvalidSyntaxError( errorMessage );
	return *flag;
}

void FunctionHandler::CondHandlerRunner( const std::vector<Token>& tokens, size_t& start, const unsigned int KEY ){
	EXT_TYPE* itCache = findPre( KEY );
	if( itCache == nullptr ){
		PreComputedCaching::ConditionalCaching( tokens, start, KEY, this );
		indexPre( KEY );
		itCache = findPre( KEY );
	}

	ExtendedConditionalToken& cTokens = std::get<ExtendedConditionalToken>( *itCache );
	start = cTokens.ctokens.endOfNok;

	for( auto& cond: cTokens.conditions ){
		if( !conditionIsTrue( cond.first, this, "Conditional Expression expects boolean values" ) )
			continue;

		size_t bodyPtr = cond.second + 1;
		ProgramExecutor( tokens, bodyPtr, CALLER::CONDITIONAL, this );
		return;
	}
}

void FunctionHandler::LoopHandlerRunner ( const std::vector<Token>& tokens, size_t& currentPtr, const unsigned int KEY ){
	EXT_TYPE* itCache = findPre( KEY );
	if( itCache == nullptr ){
		size_t scanPtr = currentPtr;
		PreComputedCaching::LoopCaching( tokens, scanPtr, KEY, this );
		indexPre( KEY );
		itCache = findPre( KEY );
	}

	ExtendedLoopTokens& lpTokens = std::get<ExtendedLoopTokens>( *itCache );
	currentPtr = lpTokens.lpToken.endPtr;

	while( conditionIsTrue( lpTokens.loopAst, this, "Loop Condition Should be Boolean or Blank" ) ){
		size_t bodyStart = lpTokens.lpToken.startPtr;
		ProgramExecutor( tokens, bodyStart, CALLER::LOOP, this );

		if( ctx->signal == SIGNAL::BREAK ){
			ctx->signal = SIGNAL::NONE;
			return;
		}
		if( ctx->signal == SIGNAL::CONTINUE )
			ctx->signal = SIGNAL::NONE;

		else if( ctx->signal == SIGNAL::RETURN )
			return;		// the function that owns this loop picks the value up

		this->VMAP.clear();
	}
}
