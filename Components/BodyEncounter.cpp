#include "Headers/BodyEncounter.hpp"

BodyEncounters::BodyEncounters( FunctionHandler* funcHandler ){
	this->funcHandler = funcHandler;
}

std::optional<VarDtype>
BodyEncounters::handleArrayProperties( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> array, ArrayAccessTokens& arrToken){
	if( arrToken.arrProperty == "valupam" )
		return (long int) array->totalElementsAllocated;
	
	else if( arrToken.arrProperty == "jaadi" )
		return "KOOTAM";

	else if( arrToken.arrProperty == "irangu" )
		throw InvalidSyntaxError("irangu is for elements inside the array");
	
	return std::nullopt;
}

DEEP_VALUE_DATA
BodyEncounters::handleArrayCases( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> arrayData, ArrayAccessTokens& arrToken ){
	if( arrToken.indexVector.size() ){
		std::vector<long int> resolvedIndexVector = getResolvedIndexVectors( arrToken.indexVector );

		// ONE INDEX BEFORE TYPE / * can extend later */
		if( arrToken.isTouchedArrayProperty && arrToken.arrProperty == "irangu"){
			auto lastIndex = resolvedIndexVector.back();
			resolvedIndexVector.pop_back();
			auto returnIndex = arrayData->getElementAtIndex( resolvedIndexVector, 0 );

			if( !std::holds_alternative< std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( returnIndex ) )
				throw std::runtime_error("irangu is only applicable to kootam type");

			auto arrList = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( returnIndex );					

			auto data = ArrayList<ARRAY_SUPPORT_TYPES>::removeElementAtIndex( arrList, lastIndex );
			
			if( std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( data ) )
				return DEEP_VALUE_DATA{ std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( data ) };

			else if( std::holds_alternative<ARRAY_SUPPORT_TYPES>( data ) ){
				auto adata = std::get<ARRAY_SUPPORT_TYPES>( data );
				return std::visit( [&](auto&& cdata){ return DEEP_VALUE_DATA{ cdata }; }, adata);
			}
			else throw std::runtime_error("Invalid dtype in array");
		}

		auto returnIndex = arrayData->getElementAtIndex( resolvedIndexVector, 0 );

		if( std::holds_alternative<ARRAY_SUPPORT_TYPES> ( returnIndex ) ){
			auto spData = std::get<ARRAY_SUPPORT_TYPES>( returnIndex );
			return std::visit( [&]( auto&& data ) {
			    if (arrToken.isTouchedArrayProperty) {
			    	DEEP_VALUE_DATA dv = data;
			        return DEEP_VALUE_DATA{handleVarDefinedProperties(dv, arrToken)};
			    } 
			    return DEEP_VALUE_DATA{data};
			}, spData);
		}
		else{
			auto arrList = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( returnIndex );
			if( arrToken.isTouchedArrayProperty ){
				auto data = this->handleArrayProperties( arrList, arrToken );
				if( data.has_value() ) return data.value();
			}
			return arrList;
		}
	}
	else {
		if( arrToken.isTouchedArrayProperty ){
			auto data = this->handleArrayProperties( arrayData, arrToken );
			if( data.has_value() ) return data.value();
		}
		return arrayData;
	}
}

VarDtype 
BodyEncounters::handleVarDefinedProperties( DEEP_VALUE_DATA& Vdata, ArrayAccessTokens& tok ){
	if( tok.arrProperty == "jaadi" ){
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
	else if( tok.arrProperty == "kanam" ){
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
		}
	}
	else if( tok.arrProperty == "THA_ASCII" ){
		if( !std::holds_alternative<VarDtype>( Vdata ) )
			throw InvalidSyntaxError("THA_ASCII is only for string of size 1");

		VarDtype data = std::get<VarDtype>( Vdata );

		if( !std::holds_alternative<std::string> ( data ) )
			throw InvalidSyntaxError("THA_ASCII is only for string of size 1");

		std::string str = std::get<std::string> (data);
		if( str.size() != 1 )
			throw InvalidSyntaxError("ASCII is only for size 1");

		return (long) str[0];
	}
	else if( tok.arrProperty == "PO_ASCII" ){
		if( !std::holds_alternative<VarDtype>( Vdata ) )
			throw InvalidSyntaxError("PO_ASCII is only for INT");

		VarDtype data = std::get<VarDtype>( Vdata );

		if( !std::holds_alternative<long int> ( data ) )
			throw InvalidSyntaxError("PO_ASCII is only for INT");

		long str = std::get<long int> (data);
		return std::string(1, (char) str);
	}
	throw InvalidSyntaxError("Invalid variable property");
}

DEEP_VALUE_DATA 
BodyEncounters::handleRawVariables( ArrayAccessTokens& arrToken, DEEP_VALUE_DATA& varHolder ){
	DEEP_VALUE_DATA HandlingDtype = varHolder;
	if( arrToken.indexVector.size() > 0 ){
		if( !std::holds_alternative<VarDtype> ( varHolder ) || !std::holds_alternative<std::string> ( std::get<VarDtype>( varHolder ) ) )
			throw std::runtime_error("Indexing Invalid DType");

		if( arrToken.indexVector.size() > 1 )
			throw std::runtime_error("Indexing ND on 1D type");

		long int index = getResolvedIndexVectors( arrToken.indexVector ).back();

		std::string& stringVarHolder = std::get<std::string>( std::get<VarDtype>( varHolder ) );

		if( index >= 0 && index < stringVarHolder.size() )
			HandlingDtype =  DEEP_VALUE_DATA { std::string(1, stringVarHolder[ index ]) };
	}
	if( arrToken.isTouchedArrayProperty )
		return handleVarDefinedProperties( HandlingDtype, arrToken );	

	return HandlingDtype;
}

std::optional<std::variant<VarDtype, std::shared_ptr<MapItem>>>
BodyEncounters::handleFunctionCall( std::shared_ptr<MapItem>& func, const std::vector<Token>& tokens, size_t& currentPtr, 
					std::string KEY, std::optional<FunctionCallReturns> data
	){
	if( preComputed.find( KEY ) == preComputed.end() ){
		FunctionCallReturns Data = ( data.has_value() ) ? data.value() : stringToFunctionCallTokens( tokens, currentPtr );
		passValidFuncCallToken( Data.callTokens ); 

		ExtendedFunctionCall funCallTok;
		funCallTok.tokens = Data;
		funCallTok.endPtr = currentPtr;

		if( Data.argsVector.size() ){
			std::vector< std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> > resolvedArgs;
			int total_comma = Data.argsVector.size() - 1;

			std::vector<Token> rhsTokens;
			rhsTokens.push_back( Token( TOKEN_TYPE::OPERATOR, "=", 0, 0 ) );

			for( auto argSingleVec: Data.argsVector ){
				auto dpData = ExprResolver::getAstRootNode(argSingleVec, this->funcHandler );
				resolvedArgs.push_back( std::move( dpData ));

				rhsTokens.push_back( Token( TOKEN_TYPE::IDENTIFIER, "NUM", 0, 0) );
				if( total_comma-- )
					rhsTokens.push_back( Token( TOKEN_TYPE::SPEC_CHAR, ",", 0, 0) );
			}
			rhsTokens.push_back(Token( TOKEN_TYPE::SPEC_CHAR, ";", 0, 0));
			size_t start = 0;
			VariableTokens funcVars = stringToVariableTokens( rhsTokens, start );
			
			passValidValueTokens( funcVars.valueTokens );
			funCallTok.argsVcts = std::move( resolvedArgs );
		}
		preComputed[ KEY ] = std::move( funCallTok );
	}

	auto& variantData = preComputed[KEY];
	auto& extFuncCallToken = std::get<ExtendedFunctionCall>( variantData );
	currentPtr = extFuncCallToken.endPtr;

	// if it is zero arg function then no need to resolve arg vectors
	if( extFuncCallToken.tokens.argsVector.size() == 0 ){
		auto funcFromMap = std::get<std::shared_ptr<FUNCTION_MAP_DATA>>( func->var );

		if( funcFromMap->argsInfo.size() != 0 )
			throw std::runtime_error("pindi need args");

		FunctionHandler newFuncRunner;

		size_t funcBodyStartPtr 	= funcFromMap->bodyStartPtr + 1;
		size_t funcEndStartPtr  	= funcFromMap->bodyEndPtr;
		newFuncRunner.runnerBody 	= extFuncCallToken.tokens.funcName;
		newFuncRunner.VMAP_COPY 	= funcFromMap->varMapCopy.first;
		newFuncRunner.parent 		= funcFromMap->varMapCopy.second;

		return ProgramExecutor( tokens, funcBodyStartPtr, CALLER::FUNCTION, &newFuncRunner, funcEndStartPtr );
	}
	FunctionHandler newFuncRunner;
	newFuncRunner.runnerBody = extFuncCallToken.tokens.funcName;
				
	auto funcFromMap 		= std::get<std::shared_ptr<FUNCTION_MAP_DATA>>( func->var );
	newFuncRunner.VMAP_COPY = funcFromMap->varMapCopy.first;
	newFuncRunner.parent 	= funcFromMap->varMapCopy.second;

	std::queue<DEEP_VALUE_DATA> resolvedArgs;

	for(int x = 0; x < extFuncCallToken.argsVcts.size(); x++){
		auto& tree = extFuncCallToken.argsVcts[x];
		DEEP_VALUE_DATA value = ExprResolver::getFinalValue( tree, this->funcHandler );
		resolvedArgs.push( value );
	}

	for( auto& ArgsInfo: funcFromMap->argsInfo ){
		if( resolvedArgs.empty() )
			throw InvalidSyntaxError("No value to initialize the args in function");

		if( newFuncRunner.VMAP_COPY.find( ArgsInfo->name ) != newFuncRunner.VMAP_COPY.end() )
			throw VariableAlreayExists( ArgsInfo->name );

		DEEP_VALUE_DATA topValue = resolvedArgs.front();
		resolvedArgs.pop();

		if( std::holds_alternative<VarDtype> ( topValue ) ){
			if( ArgsInfo->isArray )
				throw InvalidSyntaxError("Argument expects kootam\n");

			// create variable
			std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = std::make_shared<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
			newVariable->key 	= ArgsInfo->name;							
			newVariable->value 	= std::get<VarDtype>( topValue );

			// add to vmap
			auto newMapVar 		= std::make_shared<MapItem>( );
			newMapVar->mapType  = MAPTYPE::VARIABLE;
			newMapVar->var 		= newVariable;

			newFuncRunner.addToMap( ArgsInfo->name, newMapVar );
		}
		else if(std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>> ( topValue )){
			// no need to create variable add to map
			if( !ArgsInfo->isArray )
				throw InvalidSyntaxError("Argument is not kootam type");

			std::string& key    = ArgsInfo->name;
			auto newMapVar		= std::make_shared<MapItem>( );
			newMapVar->mapType 	= MAPTYPE::ARRAY_PTR;

			newMapVar->var = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>(topValue) ;
			newFuncRunner.addToMap( key, newMapVar );
		}
		else if( std::holds_alternative<std::shared_ptr<FUNCTION_MAP_DATA>>( topValue ) ){
			std::string& key    = ArgsInfo->name;
			auto newMapVar 		= std::make_shared<MapItem>( );
			newMapVar->mapType 	= MAPTYPE::FUNC_PTR;
			newMapVar->var 		= std::get<std::shared_ptr<FUNCTION_MAP_DATA>>(topValue);
			
			newFuncRunner.addToMap( key, newMapVar );
		}
		else throw std::runtime_error("Type not defined");
	}
	size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
	size_t funcEndStartPtr  = funcFromMap->bodyEndPtr;
	
	return ProgramExecutor( tokens, funcBodyStartPtr, CALLER::FUNCTION, &newFuncRunner, funcEndStartPtr );
}

void BodyEncounters::updateString( std::string& strToUpdate, long int index, DEEP_VALUE_DATA updata ){
	if( !std::holds_alternative<VarDtype>( updata ) )
		throw std::runtime_error("Invalid string updation dtype");

	auto& dataL1 = std::get<VarDtype>( updata );
	if( !std::holds_alternative<std::string> ( dataL1 ) )
		throw std::runtime_error( "invalid string updation dtype" );

	std::string rightValue = std::get<std::string> ( dataL1 );

	( index >= 0 && index < strToUpdate.size() ) ? strToUpdate.replace(index, 1, rightValue) : throw std::runtime_error("Index limit failed");
}

std::vector<long int> BodyEncounters::getResolvedIndexVectors( std::vector<std::vector<Token>>& indexVector ){
	std::vector<long int> resolvedIndexVector;

	for( auto& vec: indexVector ){
		DEEP_VALUE_DATA val = ExprResolver::evaluateVector( vec, this->funcHandler );

		if( !std::holds_alternative<VarDtype>( val ) )
			throw InvalidSyntaxError("Array Index Expects Type Integer");
		
		VarDtype vDtypeIndex = std::get<VarDtype>( val );
		
		if( !std::holds_alternative<long int> ( vDtypeIndex ) && !std::holds_alternative<double> ( vDtypeIndex ))
			throw InvalidSyntaxError("Array Index Expects Integer");

		long int index = ( std::holds_alternative<long int>( vDtypeIndex ) ) ? std::get<long int> ( vDtypeIndex ) : (long int) std::get<double>( vDtypeIndex );
		resolvedIndexVector.push_back( index );
	}
	return resolvedIndexVector;
}

void BodyEncounters::arrayUpdation( const std::vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>& arr ){
	ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, curPtr );
	curPtr--;

	if( arrToken.isTouchedArrayProperty )
		throw InvalidSyntaxError("Using array property in updatation is not allowed");

	if( arrToken.indexVector.size() ){
		std::vector<long int> resolvedIndexVector = getResolvedIndexVectors( arrToken.indexVector );

		long int updationIndex = resolvedIndexVector.back();
		resolvedIndexVector.pop_back();

		if( updationIndex < 0 ) throw InvalidSyntaxError("Array Index Expects Unsigned Integer");

		auto returnIndex = arr->getElementAtIndex( resolvedIndexVector, 0 );

		if( std::holds_alternative<ARRAY_SUPPORT_TYPES> ( returnIndex ) ){
			auto arrData = std::get<ARRAY_SUPPORT_TYPES>( returnIndex );

			if( !std::holds_alternative<VarDtype>( arrData ) || !std::holds_alternative<std::string> ( std::get<VarDtype>( arrData ) ) )
				throw InvalidSyntaxError("Failed to index an non array type");
			
			auto& ttt = std::get<VarDtype>( arrData );
			std::string& strToUpdate = std::get<std::string>( ttt );
			updateString( strToUpdate, updationIndex, upvalue );
			return ;
		}
		auto arrList = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( returnIndex );

		if( arrList->totalElementsAllocated <= updationIndex ){
			size_t cur = arrList->totalElementsAllocated;
			for(; cur <= updationIndex; cur++)
				arrList->push_SingleElement( VarDtype{0} );
		}
		std::visit( [&]( auto&& data ){ arrList->arrayList[updationIndex] = data; }, upvalue );
	}
	else {
		if( !std::holds_alternative<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( upvalue ) )
			throw InvalidSyntaxError("Dtype mismatch in array updation");
		arr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( upvalue );
	}
}

void BodyEncounters::typeCastRequest( InstructionTokens& InsTokensAndData, std::vector<Token>& varsAndVals ){
	for( int x = 0; x < varsAndVals.size(); x++ ){
		auto mapData = this->funcHandler->getFromVmap( varsAndVals[ x ].token );

		if( x >= InsTokensAndData.rightVector.size() )
			throw InvalidSyntaxError("Typecasting error");

		std::vector<Token>castInfo = InsTokensAndData.rightVector[x];
		if( castInfo.size() != 1 )
			throw InvalidSyntaxError("Typecasting error");

		Token& top = castInfo.back();

		if( mapData == nullptr )
			throw InvalidSyntaxError("Unknown token: " + varsAndVals[x].token   );

		if( mapData->mapType != MAPTYPE::VARIABLE )
			throw InvalidSyntaxError("Only variables are allowed for TYPE_CASTING");
		
		if( top.token == "INT" )
			mapData->typeCastToInt();
		
		else if( top.token == "THULA" )
			mapData->typeCastToDouble();
		
		else if( top.token == "STR" )
			mapData->typeCastToString();
		
		else if( top.token == "BOOL" )
			mapData->typeCastToBool();
		
		else throw TypeCastError("Failed to cast");
	}
}

std::optional<std::variant<VarDtype, std::shared_ptr<MapItem>>>
BodyEncounters::getReturnedData( const std::vector<Token>&tokens, size_t& currentPtr ){
	currentPtr++;
	std::vector<Token> returnStatementData;

	while( currentPtr < tokens.size() ){
		const Token& curToken = tokens[ currentPtr++ ];
		if( curToken.token == ";" ) 
			break;
		returnStatementData.push_back( curToken );
	}

	if( returnStatementData.empty() )
		return std::nullopt;

	if( returnStatementData.size() == 1 && returnStatementData.back().type == TOKEN_TYPE::IDENTIFIER ){
		auto mapData = this->funcHandler->getFromVmap( returnStatementData.back().token );
		
		if( mapData == nullptr )
			throw InvalidSyntaxError( "No variable found " + returnStatementData.back().token + 
				"\nRemember variable should be create inside the function body" );

		return mapData;				
	}
	DEEP_VALUE_DATA res = ExprResolver::evaluateVector( returnStatementData, this->funcHandler );
	if( std::holds_alternative<VarDtype> ( res ) )
		return std::get<VarDtype>( res );
	throw InvalidSyntaxError("Invalid return statement");
}		