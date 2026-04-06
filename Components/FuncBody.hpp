#ifndef FUNCBODY_HPP
#define FUNCBODY_HPP

#include "DefinedTypes.hpp"
#include "VMAP.hpp"

class FunctionHandler;

enum class CALLER{ LOOP, FUNCTION, CONDITIONAL };

std::optional<std::variant<VarDtype, std::shared_ptr<MapItem>>>
ProgramExecutor( const std::vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, FunctionHandler* prntClass, size_t endPtr = 0  );

DEEP_VALUE_DATA evaluate_AST_NODE( const std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& astNode, FunctionHandler* helperHandler, size_t level = 0);

struct RESOLVER_TYPE{
	std::queue<REAL_AST_NODE_DATA> resolvedAstNodeData;
	std::vector<std::string> simpleVector;

	RESOLVER_TYPE(
		std::queue<REAL_AST_NODE_DATA> 	resolvedAstNodeData,
		std::vector<std::string> simpleVector
	){
		this->resolvedAstNodeData = resolvedAstNodeData;
		this->simpleVector = simpleVector;
	}
};

#include "Extension.hpp"

class FunctionHandler: public VAR_VMAP {
	public:
		std::string functionName;

		FunctionHandler() = default;

		FunctionHandler( VAR_VMAP* parent, std::string runnerBody ){
			this->runnerBody = runnerBody;
			this->parent = parent;
		}

		std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> 
		getAstRootNode( std::vector<Token>& vtr ){
			auto resolvedType 		= vectorResolver( vtr );
			auto astTokenAndData 	= stringToASTTokens( resolvedType.simpleVector );
			size_t startAST 		= 0;

			auto newAstNode = BUILD_AST<REAL_AST_NODE_DATA>( astTokenAndData, resolvedType.resolvedAstNodeData, startAST );
			if( !newAstNode.has_value() )
				throw InvalidDTypeError("Failed to resolve the vector\n");

			return std::move( newAstNode.value() );
		}

		DEEP_VALUE_DATA
		getFinalValue( std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& newAstNode ){
			return evaluate_AST_NODE( newAstNode, this );
		}

		DEEP_VALUE_DATA 
		evaluateVector( std::vector<Token>& vtr ){
			auto data = this->getAstRootNode( vtr );
			return this->getFinalValue( data );
		}

		/* this function is to convert to pure vector resolve variables, function call, array calls etc */
		RESOLVER_TYPE
		vectorResolver( const std::vector<Token>& tokens ){
			std::queue<REAL_AST_NODE_DATA> resolvedAstNodeData;
			std::vector<std::string> 	simpleVector;

			for( size_t x = 0; x < tokens.size(); x++ ){
				const Token& tok = tokens[ x ];
				const std::string& curToken = tokens[ x ].token;

				if( !isValueType( tok.type ) && (isRegisteredASTExprTokens( curToken ) || curToken == ")" ||  curToken == "(" )){
					simpleVector.push_back( curToken );
					continue;
				}
				if( isValueType( tok.type ) ){
					resolvedAstNodeData.push( ValueHelper::getValueFromToken( tok ) );
					simpleVector.push_back("NUM");
				}
				else if( tok.type == TOKEN_TYPE::IDENTIFIER ){
					auto VmapData = this->getFromVmap( curToken );
					auto mainVmapData = VmapData.first;
					// if identifier is not there in VMAP
					if (mainVmapData == nullptr)
						throw InvalidSyntaxError( "Unknown identifier " + curToken );

					// Resolve if it is variable
					if( mainVmapData->mapType == MAPTYPE::VARIABLE ){
						ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );
						passArrayAccessToken( arrToken.tokens );

						resolvedAstNodeData.push( std::make_pair(arrToken, curToken) );
						simpleVector.push_back("VAR");
						x--;
					}
					// Resolve if it is function call
					else if( mainVmapData->mapType == MAPTYPE::FUNC_PTR ){
						FunctionCallReturns pt = stringToFunctionCallTokens( tokens, x );

						resolvedAstNodeData.push( std::make_pair(pt, tok) );
						simpleVector.push_back("CACHE");
						x--; 
					}
					else if( mainVmapData->mapType == MAPTYPE::ARRAY_PTR ){
						ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );

						resolvedAstNodeData.push( std::make_pair(arrToken, curToken) );
						simpleVector.push_back("ARRAY_PTR");
						x--;
					}
				}
				else if( tok.type == TOKEN_TYPE::RESERVED && curToken == "edukku" ){
					resolvedAstNodeData.push( std::string("edukku"));
					simpleVector.push_back("NUM");
				}
				else if( curToken == "{" && tok.type == TOKEN_TYPE::SPEC_CHAR )
					throw InvalidSyntaxError("Array should create in seperate pidi");
				else throw InvalidSyntaxError( "Unknown Token " + curToken );
			}
			return RESOLVER_TYPE( resolvedAstNodeData, simpleVector );
		}


		std::optional<VarDtype>
		handleArrayProperties( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> array, ArrayAccessTokens& arrToken){
			if( arrToken.arrProperty == "valupam" ){
				return (long int) array->totalElementsAllocated;
			} 
			else if( arrToken.arrProperty == "jaadi" ){
				return "KOOTAM";
			}
			else if( arrToken.arrProperty == "irangu" ){
				throw InvalidSyntaxError("irangu is for elements inside the array");
			}
			return std::nullopt;
		}

		DEEP_VALUE_DATA
		handleArrayCases( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> arrayData, ArrayAccessTokens& arrToken ){
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
						return std::visit( [&](auto&& cdata){
							return DEEP_VALUE_DATA{ cdata };
						}, adata);
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
						auto data = handleArrayProperties( arrList, arrToken );
						if( data.has_value() )  
							return data.value();
					}
					return arrList;
				}
			}
			else {
				if( arrToken.isTouchedArrayProperty ){
					auto data = handleArrayProperties( arrayData, arrToken );
					if( data.has_value() ) return data.value();
				}
				return arrayData;
			}
		}

		VarDtype 
		handleVarDefinedProperties( DEEP_VALUE_DATA& Vdata, ArrayAccessTokens& tok ){
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
		handleRawVariables( ArrayAccessTokens& arrToken, DEEP_VALUE_DATA& varHolder ){
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
		handleFunctionCall( std::shared_ptr<MapItem>& func, const std::vector<Token>& tokens, size_t& currentPtr, 
							VAR_VMAP* rPT, std::string KEY, std::optional<FunctionCallReturns> data = std::nullopt 
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
						auto dpData = getAstRootNode( argSingleVec );
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
				DEEP_VALUE_DATA value = getFinalValue( tree );
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

		void VarHandlerRunner( const std::vector<Token>& test, size_t& start, std::string KEY = nullptr ){
			if( preComputed.find( KEY ) == preComputed.end() ){
				VariableTokens tokens  = stringToVariableTokens( test, start );

				size_t endPtr = start;

				std::vector<std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>> astNodes;
				size_t curIndex = 0;

				for(auto testVec: tokens.valueVector){
					std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> evaluatedRes = getAstRootNode( testVec ); 
					astNodes.push_back( std::move( evaluatedRes ));
				}

				std::vector<VAR_INFO> varInfos;
				passValidVarDeclaration( tokens.varTokens, varInfos, tokens.VarQueue );
				passValidValueTokens( tokens.valueTokens );

				ExtendedVariableToken newExtTok = ExtendedVariableToken(
					tokens, std::move( astNodes ), varInfos, endPtr
				);
				preComputed[ KEY ] = std::move( newExtTok );
			}
			auto& variationalData = preComputed[ KEY ];
			ExtendedVariableToken& tokens = std::get<ExtendedVariableToken>( variationalData );
			start = tokens.endPtr;

			std::queue<DEEP_VALUE_DATA> resolvedValueVector;

			for(int x = 0; x < tokens.valueAst.size(); x++){
				auto& dataV = tokens.valueAst[x];
				DEEP_VALUE_DATA val1 = this->getFinalValue( dataV );
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

				auto [data, _] = this->getFromVmap( curVarInfo.varName );	
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
					else throw std::runtime_error("Type not defined");
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

		void updateString( std::string& strToUpdate, long int index, DEEP_VALUE_DATA updata ){
			if( !std::holds_alternative<VarDtype>( updata ) )
				throw std::runtime_error("Invalid string updation dtype");

			auto& dataL1 = std::get<VarDtype>( updata );
			if( !std::holds_alternative<std::string> ( dataL1 ) )
				throw std::runtime_error( "invalid string updation dtype" );

			std::string rightValue = std::get<std::string> ( dataL1 );

			( index >= 0 && index < strToUpdate.size() ) ? strToUpdate.replace(index, 1, rightValue) : throw std::runtime_error("Index limit failed");
		}

		std::vector<long int> getResolvedIndexVectors( std::vector<std::vector<Token>>& indexVector ){
			std::vector<long int> resolvedIndexVector;

			for( auto& vec: indexVector ){
				DEEP_VALUE_DATA val = evaluateVector( vec );

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

		void arrayUpdation( const std::vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>& arr ){
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

		// This function handles type casting ....
		void typeCastRequest( InstructionTokens& InsTokensAndData, std::vector<Token>& varsAndVals ){
			for( int x = 0; x < varsAndVals.size(); x++ ){
				auto [mapData, rPT] = getFromVmap( varsAndVals[ x ].token );

				if( x >= InsTokensAndData.rightVector.size() )
					throw InvalidSyntaxError("Typecasting error");

				std::vector<Token>castInfo = InsTokensAndData.rightVector[x];
				if( castInfo.size() != 1 )
					throw InvalidSyntaxError("Typecasting error");

				Token& top = castInfo.back();

				if( mapData == nullptr )
					throw InvalidSyntaxError("Unknown token: " + varsAndVals[x].token   );

				if( mapData->mapType != MAPTYPE::VARIABLE ){
					throw InvalidSyntaxError("Only variables are allowed for TYPE_CASTING");
				}
				if( top.token == "INT" ){
					mapData->typeCastToInt();
				}
				else if( top.token == "THULA" ){
					mapData->typeCastToDouble();
				}
				else if( top.token == "STR" ){
					mapData->typeCastToString();
				}
				else if( top.token == "BOOL" ){
					mapData->typeCastToBool();
				}
				else throw TypeCastError("Failed to cast");
			}
		}

		void InstructionHandlerRunner( const std::vector<Token>& tokens, size_t& currentPtr, std::string KEY ){
			if( preComputed.find( KEY ) == preComputed.end() ){
				InstructionTokens InsTokensAndData = stringToInsToken( tokens, currentPtr );
				passValidInstructionTokens( InsTokensAndData.insToken );

				ExtendedInsTokens newInsToken = ExtendedInsTokens( InsTokensAndData, currentPtr);

				if( InsTokensAndData.optr == INS_TOKEN::TYPE_CAST )
					preComputed[KEY] = std::move( newInsToken ); 
				else{
					std::vector< std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>> insTreeNodes;

					for(auto& astStrToks: InsTokensAndData.rightVector){
						auto treeNode = getAstRootNode( astStrToks );
						insTreeNodes.push_back( std::move( treeNode ) );
					}
					newInsToken.insTree = std::move( insTreeNodes );
					preComputed[ KEY ] = std::move( newInsToken );
				}
			}

			auto& variationalData = preComputed[ KEY ];
			ExtendedInsTokens& insTokens = std::get<ExtendedInsTokens>( variationalData );
			currentPtr = insTokens.endPtr;

			std::queue<DEEP_VALUE_DATA> finalValueQueue;
			std::vector<Token>& varsAndVals = insTokens.InsTokensAndData.leftVector;

			if( insTokens.InsTokensAndData.optr == INS_TOKEN::TYPE_CAST ){
				typeCastRequest( insTokens.InsTokensAndData, varsAndVals );
				return;
			}

			for( auto& astStrToks: insTokens.insTree ){
				DEEP_VALUE_DATA data = getFinalValue( astStrToks );
				finalValueQueue.push( data );
			}

			for( size_t x = 0; x < varsAndVals.size(); x++ ){
				auto [mapData, rPT] = getFromVmap( varsAndVals[ x ].token );
				
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
					arrayUpdation( varsAndVals, x, topValue, arr );
					mapData->var = arr;
					return ;
				}

				auto vmapvariable = std::get<std::shared_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>>( mapData->var );
				if( std::holds_alternative<VarDtype> ( vmapvariable->value ) ){
					auto& varDtypeData = std::get<VarDtype>( vmapvariable->value );

					if( std::holds_alternative<std::string> ( varDtypeData ) ){
						ArrayAccessTokens arrToken = stringToArrayAccesToken( varsAndVals, x );
						x--;
						
						auto resData = getResolvedIndexVectors( arrToken.indexVector );
						
						if( resData.size() > 1 )
							throw std::runtime_error("string is a one dimensional");

						std::string& strToUpdate = std::get<std::string> ( varDtypeData );

						if ( !std::holds_alternative<VarDtype>( topValue ) )
							throw std::runtime_error("Variable dtype mismatch mismatch");
						
						( !resData.empty() ) ? updateString( strToUpdate, (long int) resData.back(), topValue ) :
						mapData->updateSingleVariable( std::get<VarDtype>( topValue ) );
						return;
					}
				}
				if( vmapvariable->isTypeArray ){
					auto& arr = std::get<std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( vmapvariable->value );
					arrayUpdation( varsAndVals, x, topValue, arr);
					mapData->var = arr;
					mapData->mapType = MAPTYPE::ARRAY_PTR;
				}
				else if( std::holds_alternative<VarDtype>( topValue ) )
					mapData->updateSingleVariable( std::get<VarDtype>( topValue ) );
			}
		}

		void 
		functionDefHandlerRunner( const std::vector<Token>&token, size_t& start ){
			FunctionTokenReturn funcTokens = stringToFuncTokens( token, start );
			// pass the validation to move forward
			passValidFuncToken( funcTokens.tokens );

			if( this->getFromVmap( funcTokens.funcName ).first != nullptr )
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

		std::optional<std::variant<VarDtype, std::shared_ptr<MapItem>>>
		getReturnedData( const std::vector<Token>&tokens, size_t& currentPtr ){
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
				auto mapData = this->moveFromVmap( returnStatementData.back().token );
				
				if( !mapData.has_value() )
					throw InvalidSyntaxError( "No variable found " + returnStatementData.back().token + 
						"\nRemember variable should be create inside the function body" );

				return mapData.value();				
			}
			DEEP_VALUE_DATA res = evaluateVector( returnStatementData );
			if( std::holds_alternative<VarDtype> ( res ) )
				return std::get<VarDtype>( res );
			throw InvalidSyntaxError("Invalid return statement");
		}

		void IOHandlerRunner( const std::vector<Token>& tokens, size_t& start, std::string KEY ){
			if( preComputed.find( KEY ) == preComputed.end() ){
				auto tokensAndData = stringToIoTokens( tokens, start );
				passValidIOTokens( tokensAndData.first );

				std::vector<std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>> outputInfo;

				for( std::vector<Token>&valToks: tokensAndData.second ){
					auto astNode = getAstRootNode( valToks );
					outputInfo.push_back( std::move( astNode ) );
				}
				ExtendedIoToken newIo = ExtendedIoToken( tokensAndData, std::move( outputInfo ), start );
				preComputed[ KEY ] = std::move( newIo );
			}
			auto& variationalData = preComputed[ KEY ];
			ExtendedIoToken& IoTokens = std::get<ExtendedIoToken>( variationalData );
			start = IoTokens.endPtr;

			std::queue<DEEP_VALUE_DATA> finalQueue;
			
			for( auto& valTree: IoTokens.outputInfo ){
				DEEP_VALUE_DATA data = getFinalValue( valTree );
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
};

inline void CondHandlerRunner( const std::vector<Token>& tokens, size_t& start, std::string KEY, FunctionHandler* func ){
	if( preComputed.find( KEY ) == preComputed.end() ){
		CondReturnToken ctokens  = stringToCondTokens( tokens, start );
 		passCondTokenValidation( ctokens.tokens );

		std::vector<std::pair<std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>, size_t>> astNodes; 

		for( int x = 0; x < ctokens.conditions.size(); x++ ){
			auto curCond = ctokens.conditions[ x ];
			auto treeNode = func->getAstRootNode( curCond.first );
			astNodes.push_back( std::move( make_pair( std::move( treeNode ), curCond.second ) ) );
		}

		ExtendedConditionalToken newExtTok = ExtendedConditionalToken( ctokens, std::move( astNodes ) );
		preComputed[ KEY ] = std::move( newExtTok );
	}
	auto& variationalData = preComputed[ KEY ];
	ExtendedConditionalToken& cTokens = std::get<ExtendedConditionalToken>( variationalData );
	start = cTokens.ctokens.endOfNok;

	bool runTheCondition = false;

	for(int x = 0; x < cTokens.conditions.size(); x++){
		auto& data = cTokens.conditions[ x ];
		DEEP_VALUE_DATA evRes = func->getFinalValue( data.first );

		if( std::holds_alternative<VarDtype>( evRes ) ){
			auto VdtypData = std::get<VarDtype>( evRes );
			// only support when condition is boolean
			if( std::holds_alternative<bool>( VdtypData ) ){
				runTheCondition = true;
				// if it is true run the statement
				if( std::get<bool>( VdtypData ) ){
					start = data.second + 1;
					ProgramExecutor( tokens, start, CALLER::CONDITIONAL, func );
					start = cTokens.ctokens.endOfNok; return;
				}
			}
		}
	}
	if( !runTheCondition ) 
		throw InvalidSyntaxError("Conditional Expression expects boolean values");
}

inline void LoopHandlerRunner ( const std::vector<Token>& tokens, size_t& currentPtr, std::string KEY, FunctionHandler* func ){
	size_t beginCopy = currentPtr;

	if( preComputed.find( KEY ) == preComputed.end() ){
		LoopTokens lpTokens  = stringToLoopTokens( tokens, currentPtr );
 		passValidLoopTokens( lpTokens.lpTokens );

		std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> astNode  = func->getAstRootNode( lpTokens.conditions ); 
		ExtendedLoopTokens newExtTok = ExtendedLoopTokens( lpTokens, std::move( astNode ) );
		preComputed[ KEY ] = std::move( newExtTok );
	}
	auto& variationalData = preComputed[ KEY ];
	ExtendedLoopTokens& lpTokens = std::get<ExtendedLoopTokens>( variationalData );
	currentPtr = lpTokens.lpToken.endPtr;

	DEEP_VALUE_DATA finalValue = func->getFinalValue( lpTokens.loopAst );

	if( !std::holds_alternative<VarDtype>( finalValue ) )
		throw InvalidSyntaxError( "Loop Condition Should be Boolean or Blank" );

	VarDtype cdData = std::get<VarDtype>( finalValue );

	if( !std::holds_alternative<bool>(cdData) )
		throw InvalidSyntaxError( "loop condition should be boolean or blank" );

	bool runTheBodyAgain = std::get<bool>( cdData );
	
	if( !runTheBodyAgain ) {
		currentPtr = lpTokens.lpToken.endPtr;
		return ;
	}

	size_t bodyStart = lpTokens.lpToken.startPtr;
	try{
		ProgramExecutor( tokens, bodyStart, CALLER::LOOP, func );
		currentPtr = beginCopy;
	} 
	// inside loop some statement like break, continue or may be function statement 
	catch( const RecoverError& err ){
		currentPtr = bodyStart;
		const Token& expTok = tokens[ bodyStart ];
		
		if( expTok.token == "theku" && expTok.type == TOKEN_TYPE::RESERVED ){
			currentPtr = lpTokens.lpToken.endPtr; 
			return ;
		}
		if( expTok.token == "pinnava" && expTok.type == TOKEN_TYPE::RESERVED ){
			currentPtr = beginCopy; return ;
		}
		else throw err; 
	}
}

#endif