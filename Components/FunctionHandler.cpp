#include "Headers/FunctionHandler.hpp"

FunctionHandler::FunctionHandler( VAR_VMAP* parent, std::string runnerBody, std::shared_ptr<Context> ctx ){
	this->runnerBody = runnerBody;
	this->parent = parent;
	this->ctx = ctx;
}

void FunctionHandler::VarHandlerRunner( const std::vector<Token>& test, size_t& start, std::string KEY ){
	if( preComputed.find( KEY ) == preComputed.end() )
		PreComputedCaching::VariableCaching( test, start, KEY, this );
	
	auto& variationalData = preComputed[ KEY ];
	ExtendedVariableToken& tokens = std::get<ExtendedVariableToken>( variationalData );
	start = tokens.endPtr;

	std::queue<DEEP_VALUE_DATA> resolvedValueVector;

	for(int x = 0; x < tokens.valueAst.size(); x++){
		auto& dataV = tokens.valueAst[x];
		DEEP_VALUE_DATA val1 = ExprResolver::getFinalValue( dataV, this );
		resolvedValueVector.push( val1 );
	}

	std::vector<VAR_INFO>& varInfos = tokens.varInfos;

	for( size_t x = 0, i = 0; x < tokens.tokens.valueTokens.size(); x++ ){
		if( i >= varInfos.size() && resolvedValueVector.empty() ) break;

		auto curValueToken = tokens.tokens.valueTokens[x];

		if( curValueToken != VALUE_TOKENS::NORMAL_VALUE && curValueToken != \
			VALUE_TOKENS::ARRAY_VALUE && curValueToken != VALUE_TOKENS::ARRAY_OPEN )
			continue;

		if( i >= varInfos.size() && !resolvedValueVector.empty() )
			throw InvalidSyntaxError("No variable to initialize the value");

		VAR_INFO curVarInfo = varInfos[ i++ ];

		auto data = this->getFromVmap( curVarInfo.varName );	
		if( data ) throw VariableAlreayExists( curVarInfo.varName );

		if( curValueToken == VALUE_TOKENS::NORMAL_VALUE ){
			auto curValue = resolvedValueVector.front();
			resolvedValueVector.pop();

			if( curVarInfo.isTypeArray && !std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( curValue ) )
				throw InvalidSyntaxError("Assigning value to kootam type is invalid");

			if( !curVarInfo.isTypeArray && std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( curValue ))
				throw InvalidSyntaxError("Trying to Assign kootam type to non kootam type");

			if( std::holds_alternative<VarDtype> ( curValue ) ){
				std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = std::make_shared<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
				newVariable->key 		 = curVarInfo.varName;
				newVariable->isTypeArray = false;
				newVariable->value 		 = std::get<VarDtype>( curValue );

				auto newMapVar 		= std::make_shared<MapItem>( );
				newMapVar->mapType 	= MAPTYPE::VARIABLE;
				newMapVar->var 	  	= newVariable;
				
				this->addToMap( curVarInfo.varName, newMapVar );
			}
			else if(std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>> ( curValue )){
				std::string& key    = curVarInfo.varName;
				auto newMapVar 		= std::make_shared<MapItem>( );
				newMapVar->mapType 	= MAPTYPE::ARRAY_PTR;
				newMapVar->var 		= std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>(curValue) ;
				
				this->addToMap( key, newMapVar );
			}
			else if( std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>( curValue ) ){
				std::string& key    = curVarInfo.varName;
				auto newMapVar 		= std::make_shared<MapItem>( );
				newMapVar->mapType 	= MAPTYPE::FUNC_PTR;
				newMapVar->var 		= std::get<std::shared_ptr<FUNCTION_MAP_DATA>>(curValue);
				
				this->addToMap( key, newMapVar );
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
			newVariable->value 		 = newArray;

			auto newMapVar 		= std::make_shared<MapItem>( );
			newMapVar->mapType 	= MAPTYPE::VARIABLE;
			newMapVar->var 		= newVariable;

			this->addToMap( curVarInfo.varName, newMapVar );
		}
	}
}

void FunctionHandler::InstructionHandlerRunner( const std::vector<Token>& tokens, size_t& currentPtr, std::string KEY ){
	if( preComputed.find( KEY ) == preComputed.end() )
		PreComputedCaching::InstructionCaching( tokens, currentPtr, KEY, this );

	BodyEncounters bodyEncouter( this );
	auto& variationalData = preComputed[ KEY ];
	ExtendedInsTokens& insTokens = std::get<ExtendedInsTokens>( variationalData );
	currentPtr = insTokens.endPtr;

	std::queue<DEEP_VALUE_DATA> finalValueQueue;
	std::vector<Token>& varsAndVals = insTokens.InsTokensAndData.leftVector;

	if( insTokens.InsTokensAndData.optr == INS_TOKEN::TYPE_CAST ){
		bodyEncouter.typeCastRequest( insTokens.InsTokensAndData, varsAndVals );
		return;
	}
	for( auto& astStrToks: insTokens.insTree ){
		DEEP_VALUE_DATA data = ExprResolver::getFinalValue( astStrToks, this );
		finalValueQueue.push( data );
	}
	for( size_t x = 0; x < varsAndVals.size(); x++ ){
		auto mapData = getFromVmap( varsAndVals[ x ].token );
		
		if( mapData == nullptr )
			throw InvalidSyntaxError("Unknown token: " + varsAndVals[x].token );

		if( mapData->mapType != MAPTYPE::VARIABLE && mapData->mapType != MAPTYPE::ARRAY_PTR )
			throw InvalidSyntaxError("Only variables are allowed for updation");

		DEEP_VALUE_DATA topValue = finalValueQueue.front( );
		finalValueQueue.pop( );

		if( std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>( topValue ) ){
			mapData->mapType = MAPTYPE::FUNC_PTR;
			mapData->var = std::get<std::shared_ptr<FUNCTION_MAP_DATA>>(topValue);
			continue;
		}
		if( mapData->mapType == MAPTYPE::ARRAY_PTR ){
			auto& arr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( mapData->var );
			bodyEncouter.arrayUpdation( varsAndVals, x, topValue, arr );
			mapData->var = arr;
			return ;
		}
		auto vmapvariable = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( mapData->var );
		if( std::holds_alternative<VarDtype> ( vmapvariable->value ) ){
			auto& varDtypeData = std::get<VarDtype>( vmapvariable->value );

			if( std::holds_alternative<std::string> ( varDtypeData ) ){
				ArrayAccessTokens arrToken = stringToArrayAccesToken( varsAndVals, x );
				x--;

				auto resData = bodyEncouter.getResolvedIndexVectors( arrToken.indexVector );
				if( resData.size() > 1 )
					throw InvalidSyntaxError("string is a one dimensional");
				
				std::string& strToUpdate = std::get<std::string> ( varDtypeData );
				if ( !std::holds_alternative<VarDtype>( topValue ) )
					throw InvalidSyntaxError("Variable dtype mismatch mismatch");
				
				( !resData.empty() ) ? bodyEncouter.updateString( strToUpdate, (long int) resData.back(), topValue ) :
				mapData->updateSingleVariable( std::get<VarDtype>( topValue ) );
				return;
			}
		}
		if( vmapvariable->isTypeArray ){
			auto& arr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( vmapvariable->value );
			bodyEncouter.arrayUpdation( varsAndVals, x, topValue, arr);
			mapData->var = arr;
			mapData->mapType = MAPTYPE::ARRAY_PTR;
		}
		else if( std::holds_alternative<VarDtype>( topValue ) )
			mapData->updateSingleVariable( std::get<VarDtype>( topValue ) );
	}
}

void FunctionHandler::functionDefHandlerRunner( const std::vector<Token>&token, size_t& start ){
	FunctionTokenReturn funcTokens = stringToFuncTokens( this->ctx->bMap, token, start, this->ctx->filename );
	passValidFuncToken( funcTokens.tokens );

	if( this->getFromVmap( funcTokens.funcName ) != nullptr )
		throw InvalidSyntaxError( funcTokens.funcName + " already defined" );
	
	std::shared_ptr<FUNCTION_MAP_DATA> funcMapData = std::make_shared<FUNCTION_MAP_DATA>();
	funcMapData->funcName 		= funcTokens.funcName;
	funcMapData->argsSize 		= funcTokens.args.size();
	funcMapData->bodyStartPtr 	= funcTokens.funcStartPtr;
	funcMapData->bodyEndPtr 	= funcTokens.funcEndPtr;
	funcMapData->argsInfo 		= std::move( funcTokens.args );
	
	auto vmapCopy = this->getDeepCopyOfVMAP();

	funcMapData->varMapCopy				 			   = std::make_pair(vmapCopy, this->parent);
	std::shared_ptr<MapItem> funcMapItem 			   = std::make_shared<MapItem>();
	funcMapItem->mapType 				 			   = MAPTYPE::FUNC_PTR;
	funcMapData->varMapCopy.first[funcTokens.funcName] = funcMapItem;
	funcMapItem->var 	 							   = funcMapData;

	this->addToMap( funcTokens.funcName, funcMapItem );
}

void FunctionHandler::IOHandlerRunner( const std::vector<Token>& tokens, size_t& start, std::string KEY ){
	if( preComputed.find( KEY ) == preComputed.end() )
		PreComputedCaching::IOCaching( tokens, start, KEY, this );

	auto& variationalData = preComputed[ KEY ];
	ExtendedIoToken& IoTokens = std::get<ExtendedIoToken>( variationalData );
	start = IoTokens.endPtr;

	std::queue<DEEP_VALUE_DATA> finalQueue;
	
	for( auto& valTree: IoTokens.outputInfo ){
		DEEP_VALUE_DATA data = ExprResolver::getFinalValue( valTree, this );
		finalQueue.push(data);
	}
	bool isFirstPrint = true;
	for( IO_TOKENS tok: IoTokens.insTokens.first ){
		switch( tok ){
			case IO_TOKENS::PRINT: {
				if( !isFirstPrint )
					std::cout << "\n";
				else isFirstPrint = false;

				if( finalQueue.empty() )
					throw InvalidSyntaxError("In para statement");

				auto top = finalQueue.front();
				finalQueue.pop();
				ValueHelper::printDEEP_VALUE_DATA( top );
				break;
			}
			case IO_TOKENS::CONCAT:{
				std::cout << " ";
				if( finalQueue.empty() )
					throw InvalidSyntaxError("In koode statement");

				auto top = finalQueue.front();
				finalQueue.pop();
				ValueHelper::printDEEP_VALUE_DATA( top );
				break;
			}
			case IO_TOKENS::PRINT_VALUE: continue;
			case IO_TOKENS::END: default: return;
		}
	}
}

void FunctionHandler::CondHandlerRunner( const std::vector<Token>& tokens, size_t& start, std::string KEY ){
	if( preComputed.find( KEY ) == preComputed.end() )
		PreComputedCaching::ConditionalCaching(this->ctx->bMap, this->ctx->filename, tokens, start, KEY, this );

	auto& variationalData = preComputed[ KEY ];
	ExtendedConditionalToken& cTokens = std::get<ExtendedConditionalToken>( variationalData );
	start = cTokens.ctokens.endOfNok;

	bool runTheCondition = false;
	for(int x = 0; x < cTokens.conditions.size(); x++){
		auto& data = cTokens.conditions[ x ];
		DEEP_VALUE_DATA evRes = ExprResolver::getFinalValue( data.first, this );

		if( std::holds_alternative<VarDtype>( evRes ) ){
			auto VdtypData = std::get<VarDtype>( evRes );

			if( std::holds_alternative<bool>( VdtypData ) ){
				runTheCondition = true;

				if( std::get<bool>( VdtypData ) ){
					start = data.second + 1;
					ProgramExecutor( tokens, start, CALLER::CONDITIONAL, this );
					start = cTokens.ctokens.endOfNok; return;
				}
			}
		}
	}
	if( !runTheCondition ) 
		throw InvalidSyntaxError("Conditional Expression expects boolean values");
}

void FunctionHandler::LoopHandlerRunner ( const std::vector<Token>& tokens, size_t& currentPtr, std::string KEY ){
	size_t beginCopy = currentPtr;
	if( preComputed.find( KEY ) == preComputed.end() )
		PreComputedCaching::LoopCaching( this->ctx->bMap, this->ctx->filename, tokens, currentPtr, KEY, this );

	auto& variationalData = preComputed[ KEY ];
	ExtendedLoopTokens& lpTokens = std::get<ExtendedLoopTokens>( variationalData );
	currentPtr = beginCopy;

	while( currentPtr == beginCopy ){
		currentPtr = lpTokens.lpToken.endPtr;

		DEEP_VALUE_DATA finalValue = ExprResolver::getFinalValue( lpTokens.loopAst, this );
		if( !std::holds_alternative<VarDtype>( finalValue ) )
			throw InvalidSyntaxError( "Loop Condition Should be Boolean or Blank" );

		VarDtype cdData = std::get<VarDtype>( finalValue );
		if( !std::holds_alternative<bool>(cdData) )
			throw InvalidSyntaxError( "loop condition should be boolean or blank" );

		if( !std::get<bool>( cdData ) ) return ;

		size_t bodyStart = lpTokens.lpToken.startPtr;
		try{
			ProgramExecutor( tokens, bodyStart, CALLER::LOOP, this );
			currentPtr = beginCopy;
			std::unordered_map<std::string, std::shared_ptr<MapItem>>().swap(this->VMAP);
		} 
		catch( const RecoverError& err ){
			currentPtr = bodyStart;
			const Token& expTok = tokens[ bodyStart ];
			
			if( expTok.token == "theku" && expTok.type == TOKEN_TYPE::RESERVED ){
				currentPtr = lpTokens.lpToken.endPtr; 
				return ;
			}
			if( expTok.token == "pinnava" && expTok.type == TOKEN_TYPE::RESERVED ){
				currentPtr = beginCopy; 
				return ;
			}
			throw err; 
		}
	}
}
