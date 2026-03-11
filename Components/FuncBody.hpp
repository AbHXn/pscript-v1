#ifndef FUNCBODY_HPP
#define FUNCBODY_HPP

#include "DefinedTypes.hpp"
#include "VMAP.hpp"

class LoopHandler;
class FunctionHandler;

std::vector<Token> fullTokens;
size_t 		   pointer = 0;

enum class CALLER{ LOOP, FUNCTION, CONDITIONAL };

template <typename T> std::optional<std::variant<VarDtype, std::unique_ptr<MapItem>>>
ProgramExecutor( const std::vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, T* prntClass, size_t endPtr = 0  );
DEEP_VALUE_DATA evaluate_AST_NODE( std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>& astNode, FunctionHandler* helperHandler, size_t level = 0);

using BUCKET_TYPE = std::variant<std::unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>, std::unique_ptr<FUNCTION_MAP_DATA>>;
std::vector<BUCKET_TYPE> _CACHE_VARS;

struct RESOLVER_TYPE{
	std::queue<REAL_AST_NODE_DATA> 		resolvedAstNodeData;
	std::vector<std::string> 			simpleVector;

	RESOLVER_TYPE(
		std::queue<REAL_AST_NODE_DATA> 		resolvedAstNodeData,
		std::vector<std::string> 			simpleVector
	){
		this->resolvedAstNodeData = resolvedAstNodeData;
		this->simpleVector 		  = simpleVector;
	}
};

class FunctionHandler: public VAR_VMAP {
	public:
		std::string functionName;

		VarDtype getValueFromToken( const Token& tok ){
			if( tok.type == TOKEN_TYPE::STRING )
				return VarDtype{ tok.token };
			
			else if( tok.type == TOKEN_TYPE::NUMBER )
				return VarDtype{  DtypeHelper::toLong( tok.token ) };
			
			else if( tok.type == TOKEN_TYPE::FLOATING )
				return VarDtype{ DtypeHelper::toDouble( tok.token ) };

			else if( tok.type == TOKEN_TYPE::BOOLEAN && 
					( tok.token == "sheri" || tok.token == "thettu" ) )
				return VarDtype{ DtypeHelper::toBoolean( tok.token ) };
			
			throw InvalidDTypeError("not a value");
		}

		DEEP_VALUE_DATA 
		evaluateVector( std::vector<Token>& vtr ){
			auto resolvedType 		= vectorResolver( vtr );
			auto astTokenAndData 	= stringToASTTokens( resolvedType.simpleVector );
			size_t startAST 		= 0;

			// built AST Tree
			auto newAstNode = BUILD_AST<REAL_AST_NODE_DATA, REAL_AST_NODE_DATA>( astTokenAndData, resolvedType.resolvedAstNodeData, startAST );

			if( newAstNode.has_value() ){
				auto AST_NODE = std::move( newAstNode.value() );
				return evaluate_AST_NODE( AST_NODE, this );
			}
			throw InvalidDTypeError("Failed to resolve the vector\n");
		}

		/* this function is to convert to pure vector
		resolve variables, function call, array calls etc */
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
					VarDtype value = this->getValueFromToken( tok );
					resolvedAstNodeData.push( value );
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
						// pass array access validity
						passArrayAccessToken( arrToken.tokens );

						resolvedAstNodeData.push( std::make_pair(arrToken, mainVmapData) );
						simpleVector.push_back("VAR");
						x--;
					}
					// Resolve if it is function call
					else if( mainVmapData->mapType == MAPTYPE::FUNCTION || mainVmapData->mapType == MAPTYPE::FUNC_PTR ){
						FunctionCallReturns pt = stringToFunctionCallTokens( tokens, x );

						resolvedAstNodeData.push( std::make_pair(pt, mainVmapData) );
						simpleVector.push_back("CACHE");
						x--; // stringfuncalltokens it hits then unknown token get that token back
					}
					else if( mainVmapData->mapType == MAPTYPE::ARRAY_PTR ){
						ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, x );

						resolvedAstNodeData.push( std::make_pair(arrToken, mainVmapData) );
						simpleVector.push_back("ARRAY_PTR");
						x--;
					}
				}
				else if( tok.type == TOKEN_TYPE::RESERVED && curToken == "edukku" ){
					std::string inputValue; 
					getline(std::cin, inputValue);
					resolvedAstNodeData.push( inputValue );
					simpleVector.push_back("NUM");
				}
				else throw InvalidSyntaxError( "Unknown Token " + curToken );
			}
			return RESOLVER_TYPE( resolvedAstNodeData, simpleVector );
		}

		std::optional<VarDtype>
		handleArrayProperties( ArrayList<ARRAY_SUPPORT_TYPES>* array, ArrayAccessTokens& arrToken){
			if( arrToken.arrProperty == "valupam" ){
				return (long int) array->totalElementsAllocated;
			} 
			else if( arrToken.arrProperty == "jaadi" ){
				return "KOOTAM";
			}
			return std::nullopt;
		}

		DEEP_VALUE_DATA
		handleArrayCases( ArrayList<ARRAY_SUPPORT_TYPES>* arrayData, ArrayAccessTokens& arrToken ){
			if( arrToken.indexVector.size() ){
				std::vector<long int> resolvedIndexVector;
				// Resolve the index vector to the final value
				for( auto& vec: arrToken.indexVector ){
					DEEP_VALUE_DATA val = evaluateVector( vec );
					if( !std::holds_alternative<VarDtype>( val ) )
						throw InvalidSyntaxError("Array Index Expects Positive Integer");
				
					VarDtype vDtypeIndex = std::get<VarDtype>( val );
					if( !std::holds_alternative<long int> ( vDtypeIndex ) && !std::holds_alternative<double> ( vDtypeIndex ))
						throw InvalidSyntaxError("Array Index Expects Positive Integer");

					long int index = std::holds_alternative<long int> ( vDtypeIndex ) ? std::get<long int> ( vDtypeIndex ) : std::get<double> (vDtypeIndex);
					resolvedIndexVector.push_back( index );
				}
				std::variant<ArrayList<ARRAY_SUPPORT_TYPES>*, ARRAY_SUPPORT_TYPES*> returnIndex = arrayData->getElementAtIndex( resolvedIndexVector, 0 );
				// resolve if it touch property functions (:)
				if( std::holds_alternative<ARRAY_SUPPORT_TYPES*> ( returnIndex ) ){
					auto spData = std::get<ARRAY_SUPPORT_TYPES*>( returnIndex );
					return std::visit( [&]( auto&& data ) {

					    if (arrToken.isTouchedArrayProperty) {
					    	DEEP_VALUE_DATA dv = data;
					        return DEEP_VALUE_DATA{handleVarDefinedProperties(dv, arrToken)};
					    } 
					    return DEEP_VALUE_DATA{data};
					}, *spData);
				}
				else{
					auto arrList = std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>( returnIndex );
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
					if( data.has_value() )
						return data.value();
				}
				return arrayData;
			}
			throw;
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
				else if( std::holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>( Vdata ) )
					return "ARRAY_PTR";
				
				else if( std::holds_alternative<FUNCTION_MAP_DATA*>( Vdata ) )
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

			throw InvalidSyntaxError("Invalid property");
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

				if( index >= 0 && index < stringVarHolder.size())
					HandlingDtype =  DEEP_VALUE_DATA { std::string(1, stringVarHolder[ index ]) };
			}
			if( arrToken.isTouchedArrayProperty )
				return handleVarDefinedProperties( HandlingDtype, arrToken );	

			return HandlingDtype;
		}

		std::optional<std::variant<VarDtype, std::unique_ptr<MapItem>>>
		handleFunctionCall( MapItem* func, const std::vector<Token>& tokens, size_t& currentPtr, VAR_VMAP* rPT, std::optional<FunctionCallReturns> data = std::nullopt ){
			FunctionCallReturns Data = ( data.has_value() ) ? data.value() : stringToFunctionCallTokens( tokens, currentPtr );
			
			// pass the function call validation
			passValidFuncCallToken( Data.callTokens ); 

			// if it is zero arg function then no need to resolve arg vectors
			if( Data.argsVector.size() == 0 ){
				auto funcFromMap = std::get<FUNCTION_MAP_DATA*>( func->var );

				std::unique_ptr<FunctionHandler> newFuncRunner = std::make_unique<FunctionHandler>();

				size_t funcBodyStartPtr 	= funcFromMap->bodyStartPtr + 1;
				size_t funcEndStartPtr  	= funcFromMap->bodyEndPtr;
				newFuncRunner->runnerBody 	= Data.funcName;
				newFuncRunner->VMAP_COPY 	= funcFromMap->varMapCopy.first;
				newFuncRunner->parent 		= funcFromMap->varMapCopy.second;

				return ProgramExecutor( 
					tokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr 
				);
			}
			std::unique_ptr<FunctionHandler> newFuncRunner = std::make_unique<FunctionHandler>();
			newFuncRunner->runnerBody = Data.funcName;
						
			auto funcFromMap 		 = std::get<FUNCTION_MAP_DATA*>( func->var );
			newFuncRunner->VMAP_COPY = funcFromMap->varMapCopy.first;
			newFuncRunner->parent 	 = funcFromMap->varMapCopy.second;

			std::queue<DEEP_VALUE_DATA> resolvedArgs;
			int total_comma = Data.argsVector.size() - 1;

			std::vector<Token> rhsTokens;
			rhsTokens.push_back( Token( TOKEN_TYPE::OPERATOR, "=", 0, 0 ) );

			for( auto argSingleVec: Data.argsVector ){
				DEEP_VALUE_DATA dpData = evaluateVector( argSingleVec );
				resolvedArgs.push( dpData );

				rhsTokens.push_back( Token( TOKEN_TYPE::IDENTIFIER, "NUM", 0, 0) );
				if( total_comma-- )
					rhsTokens.push_back( Token( TOKEN_TYPE::SPEC_CHAR, ",", 0, 0) );
			}
			rhsTokens.push_back(Token( TOKEN_TYPE::SPEC_CHAR, ";", 0, 0));

			size_t start = 0;
			VariableTokens funcVars = stringToVariableTokens( rhsTokens, start );
			
			// pass value syntax tokens			
			passValidValueTokens( funcVars.valueTokens );

			for( auto& ArgsInfo: funcFromMap->argsInfo ){
				if( resolvedArgs.empty() )
					throw InvalidSyntaxError("No value to initialized the args in pari");
				
				DEEP_VALUE_DATA topValue = resolvedArgs.front();
				resolvedArgs.pop();

				if( std::holds_alternative<VarDtype> ( topValue ) ){
					if( ArgsInfo->isArray )
						throw InvalidSyntaxError("Argument expects kootam\n");

					// create variable
					std::unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = std::make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
					newVariable->key 	= ArgsInfo->name;							
					newVariable->value 	= std::get<VarDtype>( topValue );

					// add to vmap
					auto newMapVar 		= std::make_unique<MapItem>( );
					newMapVar->mapType  = MAPTYPE::VARIABLE;
					newMapVar->var 		= newVariable.get();

					_CACHE_VARS.push_back( std::move( newVariable ) );
					newFuncRunner->addToMap( ArgsInfo->name, std::move( newMapVar ) );
				}
				else if(std::holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*> ( topValue )){
					// no need to create variable add to map
					if( !ArgsInfo->isArray )
						throw InvalidSyntaxError("Argument is not kootam type");

					std::string& key    = ArgsInfo->name;
					auto newMapVar		= std::make_unique<MapItem>( );
					newMapVar->mapType 	= MAPTYPE::ARRAY_PTR;

					newMapVar->var = std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>(topValue) ;
					newFuncRunner->addToMap( key, std::move( newMapVar ) );
				}
				else if( std::holds_alternative<FUNCTION_MAP_DATA*>( topValue ) ){
					std::string& key    = ArgsInfo->name;
					auto newMapVar 		= std::make_unique<MapItem>( );
					newMapVar->mapType 	= MAPTYPE::FUNC_PTR;
					newMapVar->var 		= std::get<FUNCTION_MAP_DATA*>(topValue);
					
					newFuncRunner->addToMap( key, std::move( newMapVar ) );
				}
				else throw std::runtime_error("Type not defined");
			}
			size_t funcBodyStartPtr = funcFromMap->bodyStartPtr + 1;
			size_t funcEndStartPtr  = funcFromMap->bodyEndPtr;
			
			return ProgramExecutor<FunctionHandler>( 
				fullTokens, funcBodyStartPtr, CALLER::FUNCTION, newFuncRunner.get(), funcEndStartPtr 
			);
		}

		void VarHandlerRunner( const std::vector<Token>& test, size_t& start ){
			VariableTokens tokens  = stringToVariableTokens( test, start );

			std::queue<DEEP_VALUE_DATA> resolvedValueVector;
			size_t curIndex = 0;

			for( auto valVec: tokens.valueTokens ){
				if( valVec == VALUE_TOKENS::ARRAY_VALUE || valVec == VALUE_TOKENS::NORMAL_VALUE ){
					auto testVec = tokens.valueVector[ curIndex++ ];

					DEEP_VALUE_DATA evaluatedRes = evaluateVector( testVec ); 
					resolvedValueVector.push( evaluatedRes );
				}
			}
			std::vector<VAR_INFO> varInfos;
			// pass the validity test for variable declaration
			passValidVarDeclaration( tokens.varTokens, varInfos, tokens.VarQueue );

			for( auto& varVerification: varInfos ){
				auto [data, rPT] = this->getFromVmap( varVerification.varName );	
				// check if variable is already existsed
				if( data ) throw VariableAlreayExists( varVerification.varName );
			}
			// if syntax is not correct
			passValidValueTokens( tokens.valueTokens );

			for( size_t x = 0, i = 0; x < tokens.valueTokens.size(); x++ ){
				if( i >= varInfos.size() && resolvedValueVector.empty() )
					break;

				auto curValueToken = tokens.valueTokens[x];

				if( curValueToken != VALUE_TOKENS::NORMAL_VALUE && curValueToken != \
					VALUE_TOKENS::ARRAY_VALUE && curValueToken != VALUE_TOKENS::ARRAY_OPEN )
					continue;

				if( i >= varInfos.size() && !resolvedValueVector.empty() )
					throw InvalidSyntaxError("No variable to initialize the value");

				VAR_INFO curVarInfo = varInfos[ i++ ];

				if( curValueToken == VALUE_TOKENS::NORMAL_VALUE ){
					auto curValue = resolvedValueVector.front();
					resolvedValueVector.pop();

					if( curVarInfo.isTypeArray && !std::holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>( curValue ) )
						throw InvalidSyntaxError("Assigning value to kootam type is invalid");

					if( !curVarInfo.isTypeArray && std::holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*>( curValue ))
						throw InvalidSyntaxError("Trying to Assign kootam type to non kootam type");

					if( std::holds_alternative<VarDtype> ( curValue ) ){
						std::unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = std::make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
						newVariable->key 		 = curVarInfo.varName;
						newVariable->isTypeArray = false;
						newVariable->value 		 = std::get<VarDtype>( curValue );

						auto newMapVar 		= std::make_unique<MapItem>( );
						newMapVar->mapType 	= MAPTYPE::VARIABLE;
						newMapVar->var 	  	= newVariable.get() ;
						
						_CACHE_VARS.push_back( std::move( newVariable ) );
						this->addToMap( curVarInfo.varName, std::move(newMapVar) );
					}
					else if(std::holds_alternative<ArrayList<ARRAY_SUPPORT_TYPES>*> ( curValue )){
						std::string& key    = curVarInfo.varName;
						auto newMapVar 		= std::make_unique<MapItem>( );
						newMapVar->mapType 	= MAPTYPE::ARRAY_PTR;
						newMapVar->var 		= std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>(curValue) ;
						
						this->addToMap( key, std::move( newMapVar ) );
					}
					else if( std::holds_alternative<FUNCTION_MAP_DATA*>( curValue ) ){
						std::string& key    = curVarInfo.varName;
						auto newMapVar 		= std::make_unique<MapItem>( );
						newMapVar->mapType 	= MAPTYPE::FUNC_PTR;
						newMapVar->var 		= std::get<FUNCTION_MAP_DATA*>(curValue);
						
						this->addToMap( key, std::move( newMapVar ) );
					}
					else throw std::runtime_error("Type not defined");
				}
				else if( curValueToken == VALUE_TOKENS::ARRAY_OPEN ){
					x++;

					auto newArray = ArrayList<ARRAY_SUPPORT_TYPES>::createArray<DEEP_VALUE_DATA>(tokens.valueTokens,  x, resolvedValueVector );

					std::unique_ptr<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>> newVariable = std::make_unique<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>>();
					newVariable->key 		 = curVarInfo.varName;
					newVariable->isTypeArray = true;
					newVariable->value 		 = std::move( newArray );

					auto newMapVar 		= std::make_unique<MapItem>( );
					newMapVar->mapType 	= MAPTYPE::VARIABLE;
					newMapVar->var 		= newVariable.get();

					_CACHE_VARS.push_back( std::move( newVariable ) );
					this->addToMap( curVarInfo.varName, std::move(newMapVar) );
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

		void arrayUpdation( const std::vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, ArrayList<ARRAY_SUPPORT_TYPES>* arr ){
			ArrayAccessTokens arrToken = stringToArrayAccesToken( tokens, curPtr );
			curPtr--;

			if( arrToken.isTouchedArrayProperty )
				throw InvalidSyntaxError("Using array property in updatation is not allowed");

			if( arrToken.indexVector.size() ){
				std::vector<long int> resolvedIndexVector = getResolvedIndexVectors( arrToken.indexVector );

				long int updationIndex = resolvedIndexVector.back();
				resolvedIndexVector.pop_back();

				if( updationIndex < 0 ) throw InvalidSyntaxError("Array Index Expects Unsigned Integer");

				std::variant< ArrayList<ARRAY_SUPPORT_TYPES>*, ARRAY_SUPPORT_TYPES*> returnIndex = arr->getElementAtIndex( resolvedIndexVector, 0 );

				if( std::holds_alternative<ARRAY_SUPPORT_TYPES*> ( returnIndex ) ){
					auto* arrData = std::get<ARRAY_SUPPORT_TYPES*>( returnIndex );

					if( !std::holds_alternative<VarDtype>( *arrData ) || !std::holds_alternative<std::string> ( std::get<VarDtype>( *arrData ) ) )
						throw InvalidSyntaxError("Failed to index an non array type");
					
					auto& ttt = std::get<VarDtype>( *arrData );
					std::string& strToUpdate = std::get<std::string>( ttt );
					updateString( strToUpdate, updationIndex, upvalue );
					return ;
				}
				ArrayList<ARRAY_SUPPORT_TYPES>* arrList = std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>( returnIndex );

				if( arrList->totalElementsAllocated <= updationIndex ){
					size_t cur = arrList->totalElementsAllocated;
					for(; cur <= updationIndex; cur++)
						arrList->push_SingleElement( VarDtype{0} );
				}
				std::visit( [&]( auto&& data ){ arrList->arrayList[updationIndex] = data; }, upvalue );
			}
		}

		void InstructionHandlerRunner( const std::vector<Token>& tokens, size_t& currentPtr ){
			InstructionTokens InsTokensAndData = stringToInsToken( tokens, currentPtr );

			// check instruction token validation
			passValidInstructionTokens( InsTokensAndData.insToken );

			std::queue<DEEP_VALUE_DATA> finalValueQueue;
			std::vector<Token>& varsAndVals = InsTokensAndData.leftVector;

			if( InsTokensAndData.optr == INS_TOKEN::TYPE_CAST ){
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
				return;
			}

			for( std::vector<Token>& astStrToks: InsTokensAndData.rightVector ){
				DEEP_VALUE_DATA data = evaluateVector( astStrToks );
				finalValueQueue.push( data  );
			}

			for( size_t x = 0; x < varsAndVals.size(); x++ ){
				auto [mapData, rPT] = getFromVmap( varsAndVals[ x ].token );
				
				if( mapData == nullptr )
					throw InvalidSyntaxError("Unknown token: " + varsAndVals[x].token );

				if( mapData->mapType != MAPTYPE::VARIABLE && mapData->mapType != MAPTYPE::ARRAY_PTR )
					throw InvalidSyntaxError("Only variables are allowed for updation");

				DEEP_VALUE_DATA topValue = finalValueQueue.front();
				finalValueQueue.pop();

				if( std::holds_alternative<FUNCTION_MAP_DATA*>(topValue) ){
					mapData->mapType = MAPTYPE::FUNC_PTR;
					mapData->var = std::get<FUNCTION_MAP_DATA*>(topValue);
					continue;
				}

				if( mapData->mapType == MAPTYPE::ARRAY_PTR ){
					auto& arr = std::get<ArrayList<ARRAY_SUPPORT_TYPES>*>( mapData->var );
					arrayUpdation( varsAndVals, x, topValue, arr );
					return ;
				}

				auto vmapvariable = std::get<VARIABLE_HOLDER<ARRAY_SUPPORT_TYPES>*>( mapData->var );
				if( std::holds_alternative<VarDtype> ( vmapvariable->value ) ){
					auto& varDtypeData = std::get<VarDtype>( vmapvariable->value );

					if( std::holds_alternative<std::string> ( varDtypeData ) ){
						ArrayAccessTokens arrToken = stringToArrayAccesToken( varsAndVals, x );
						x--;
						
						auto resData = getResolvedIndexVectors( arrToken.indexVector );
						
						if( resData.size() > 1 )
							throw std::runtime_error("string is a one dimensional");

						std::string& strToUpdate = std::get<std::string> ( varDtypeData );
						if( !resData.empty() ){
							long int index = resData.back();
							updateString( strToUpdate, index, topValue );
						}
						else mapData->updateSingleVariable( std::get<VarDtype>( topValue ) );
						return;
					}
				}
				if( vmapvariable->isTypeArray ){
					auto& arr = std::get<std::unique_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>>( vmapvariable->value );
					arrayUpdation( varsAndVals, x, topValue, arr.get());
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
			
			std::unique_ptr<FUNCTION_MAP_DATA> funcMapData = std::make_unique<FUNCTION_MAP_DATA>();
			funcMapData->funcName 		= funcTokens.funcName;
			funcMapData->argsSize 		= funcTokens.args.size();
			funcMapData->bodyStartPtr 	= funcTokens.funcStartPtr;
			funcMapData->bodyEndPtr 	= funcTokens.funcEndPtr;
			funcMapData->argsInfo 		= std::move( funcTokens.args );
			
			auto vmapCopy = this->getDeepCopyOfVMAP();

			funcMapData->varMapCopy				 			   = std::make_pair(vmapCopy, this->parent);
			std::unique_ptr<MapItem> funcMapItem 			   = std::make_unique<MapItem>();
			funcMapItem->mapType 				 			   = MAPTYPE::FUNCTION;
			funcMapData->varMapCopy.first[funcTokens.funcName] = funcMapItem.get();
			funcMapItem->var 	 							   = funcMapData.get();

			_CACHE_VARS.push_back( std::move(funcMapData) );					
			this->addToMap( funcTokens.funcName, std::move( funcMapItem ) );
		}

		std::optional<std::variant<VarDtype, std::unique_ptr<MapItem>>>
		getReturnedData( const std::vector<Token>&tokens, size_t& currentPtr ){
			currentPtr++;
			std::vector<Token> returnStatementData;

			while( currentPtr < tokens.size() ){
				const Token& curToken = tokens[ currentPtr ];
				if( curToken.token == ";" ) break;
				returnStatementData.push_back( curToken );
				currentPtr++;
			}

			if( returnStatementData.empty() )
				return std::nullopt;

			if( returnStatementData.size() == 1 && returnStatementData.back().type == TOKEN_TYPE::IDENTIFIER ){
				auto mapData = this->moveFromVmap( returnStatementData.back().token );
				if( mapData.has_value() ){
					for( auto& mapData: this->VMAP )
						propHolderTemp.push_back( std::move( mapData.second ) );
					return  std::move( mapData.value() ) ;
				}
				throw InvalidSyntaxError("no variable foun " + returnStatementData.back().token );
			}
			DEEP_VALUE_DATA res = evaluateVector( returnStatementData );

			if( std::holds_alternative<VarDtype> ( res ) )
				return std::get<VarDtype>( res );

			throw InvalidSyntaxError("Invalid return statement");
		}

		void IOHandlerRunner( const std::vector<Token>& tokens, size_t& start ){
				auto tokensAndData = stringToIoTokens( tokens, start );
				// pass the validation test for iotokens
				passValidIOTokens( tokensAndData.first );

				std::queue<DEEP_VALUE_DATA> finalQueue;
				
				for( std::vector<Token>&valToks: tokensAndData.second ){
					DEEP_VALUE_DATA data = evaluateVector( valToks );
					finalQueue.push(data);
				}
		
				bool isFirstPrint = true;
				for( IO_TOKENS tok: tokensAndData.first ){
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
						case IO_TOKENS::PRINT_VALUE:
							continue;
						
						case IO_TOKENS::END:
						default:
							return;
					}
				}
			}
};

class Conditional: public FunctionHandler{
	public:
		// inside condition there may be loop statements
		std::unique_ptr<LoopHandler> lpRunner;
		Conditional( std::unique_ptr<LoopHandler> lpRunner ){
			this->lpRunner = std::move( lpRunner );
		}
		void CondHandlerRunner( const std::vector<Token>& tokens, size_t& start ){
			size_t endOfNOK = 0;
			auto ctokens = stringToCondTokens( tokens, start, endOfNOK );
			// pass the validation
			passCondTokenValidation( ctokens.first );

			bool runTheCondition = false;

			for(int x = 0; x < ctokens.second.size(); x++){
				auto data = ctokens.second[ x ];
				DEEP_VALUE_DATA evRes = evaluateVector( data.first );

				if( std::holds_alternative<VarDtype>( evRes ) ){
					auto VdtypData = std::get<VarDtype>( evRes );

					// only support when condition is boolean
					if( std::holds_alternative<bool>( VdtypData ) ){
						runTheCondition = true;
						// if it is true run the statement
						if( std::get<bool>( VdtypData ) ){
							start = data.second + 1;
							runTheCondition = true;
							
							ProgramExecutor( tokens, start, CALLER::CONDITIONAL, this );
							start = endOfNOK;
							return;
						}
					}
				}
			}
			if( !runTheCondition ) 
				throw InvalidSyntaxError("Conditional Expression expects boolean values");
		}
};

class LoopHandler: public FunctionHandler{
	public:		
		void 
		LoopHandlerRunner ( const std::vector<Token>& tokens, size_t& currentPtr ){
			size_t beginCopy = currentPtr;
		 	LoopTokens lpTokens = stringToLoopTokens( tokens, currentPtr );

		 	// pass the loop validation test
		 	passValidLoopTokens( lpTokens.lpTokens );
			
			// run loop until its condition fails
			while( true ){
				DEEP_VALUE_DATA finalValue = evaluateVector( lpTokens.conditions );

				if( !std::holds_alternative<VarDtype>( finalValue ) )
					throw InvalidSyntaxError( "Loop Condition Should be Boolean or Blank" );

				VarDtype cdData = std::get<VarDtype>( finalValue );

				if( !std::holds_alternative<bool>(cdData) )
					throw InvalidSyntaxError( "loop condition should be boolean or blank" );

				bool runTheBodyAgain = std::get<bool>( cdData );
				if( !runTheBodyAgain ) break;

				size_t bodyStart = lpTokens.startPtr;
				try{
					ProgramExecutor( tokens, bodyStart, CALLER::LOOP, this );
					currentPtr = beginCopy;
					return ;
				} 
				// inside loop some statement like break, continue
				// or may be function statement 
				catch( const RecoverError& err ){
					currentPtr = bodyStart;
					const std::string& expTok = tokens[ bodyStart ].token;
					
					if( expTok == "theku" )
						break;
					else if( expTok == "pinnava" )
						currentPtr = beginCopy;
					else throw err; 
				}
			}
			currentPtr = lpTokens.endPtr;
		}
};
#endif