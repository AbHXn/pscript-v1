#include "Headers/Variables.hpp"

const std::unordered_set<std::string_view> REGISTERED_TOKENS  = { 
		"pidi", 
		"kootam", 
		"=", 
		",", 
		";", 
		"{", 
		"}" 
};

// variable LHS graph (variable declare)
std::unordered_map <VARIABLE_TOKENS, std::vector<VARIABLE_TOKENS>> VARIABLE_DECLARE_GRAPH = {
	{ VARIABLE_TOKENS::VAR_START, 	{ VARIABLE_TOKENS::NAME } 								  },
	{ VARIABLE_TOKENS::NAME,		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::ARRAY, VARIABLE_TOKENS::VAR_ENDS }     },
	{ VARIABLE_TOKENS::ARRAY, 		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::VAR_ENDS } 		 					  },
	{ VARIABLE_TOKENS::COMMA, 		{ VARIABLE_TOKENS::NAME } 						  		  }	
};

// variable RHS graph (value assign)
std::unordered_map <VALUE_TOKENS, std::vector<VALUE_TOKENS>> VALUE_ASSIGN_GRAPH = {
	{ VALUE_TOKENS::NORMAL_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END } 			},
	{ VALUE_TOKENS::ARRAY_OPEN,  { VALUE_TOKENS::ARRAY_OPEN, VALUE_TOKENS::ARRAY_VALUE,
								   VALUE_TOKENS::ARRAY_CLOSE } 								},
	{ VALUE_TOKENS::ARRAY_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::ARRAY_CLOSE } 	   	},
	{ VALUE_TOKENS::COMMA, 	  	 { VALUE_TOKENS::ARRAY_VALUE, VALUE_TOKENS::ARRAY_OPEN, 
								   VALUE_TOKENS::NORMAL_VALUE } 							},
	{ VALUE_TOKENS::ARRAY_CLOSE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END, 
								   VALUE_TOKENS::ARRAY_CLOSE }		   						}
};

bool isRegisteredVariableToken( const std::string& token ){
	return REGISTERED_TOKENS.find( token ) != REGISTERED_TOKENS.end();
}

VariableTokens stringToVariableTokens( const std::vector<Token>& tokens, size_t& startCurPtr, bool isVariableTurn ){
	std::vector<VARIABLE_TOKENS> 	varTokens;
	std::vector<VALUE_TOKENS> 		valueToken;
	std::vector<std::vector<Token>> valueVector;
	std::queue<Token> VarQueue;

	int arrayOpenedCount 	= 0;

	for( ; startCurPtr < tokens.size(); ++startCurPtr ) {
		const Token& tok = tokens[ startCurPtr ];
		const std::string& curToken = tok.token;

		if( curToken == "pidi" && tok.type == TOKEN_TYPE::RESERVED ){
			varTokens.push_back( VARIABLE_TOKENS::VAR_START );
		}
		else if( curToken == "," && tok.type == TOKEN_TYPE::SPEC_CHAR){
			( isVariableTurn ) ?  varTokens.push_back( VARIABLE_TOKENS::COMMA ): valueToken.push_back( VALUE_TOKENS::COMMA );
		}
		else if( curToken == "kootam" && tok.type == TOKEN_TYPE::RESERVED )
			varTokens.push_back( VARIABLE_TOKENS::ARRAY );

		else if ( curToken == "{" && tok.type == TOKEN_TYPE::SPEC_CHAR ){
			arrayOpenedCount++;
			valueToken.push_back( VALUE_TOKENS::ARRAY_OPEN );
		}
		else if( curToken == "=" && tok.type == TOKEN_TYPE::OPERATOR ){
			varTokens.push_back( VARIABLE_TOKENS::VALUE_ASSIGN );
			isVariableTurn = false;
		}
		else if(curToken == "}" && tok.type == TOKEN_TYPE::SPEC_CHAR){
			arrayOpenedCount--;
			valueToken.push_back( VALUE_TOKENS::ARRAY_CLOSE );
		}
		else if(curToken == ";" && tok.type == TOKEN_TYPE::SPEC_CHAR ){
			( isVariableTurn ) ? varTokens.push_back( VARIABLE_TOKENS::VAR_ENDS ) : valueToken.push_back( VALUE_TOKENS::VALUE_END );
			return VariableTokens( varTokens, valueToken, valueVector, VarQueue );
		}
		else{
			if( isVariableTurn ){
				VarQueue.push( tok );
				varTokens.push_back( VARIABLE_TOKENS::NAME );
			}
			else{
				std::vector<Token> curValueVector;
				int openBrack = 0;

				while( startCurPtr < tokens.size() ){
					if( tokens[ startCurPtr ].token == "(" && tokens[startCurPtr].type == TOKEN_TYPE::SPEC_CHAR )
						openBrack++;
					else if( tokens[startCurPtr].token == ")" && tokens[startCurPtr].type == TOKEN_TYPE::SPEC_CHAR )
						openBrack--;
					if( !isValueType(tokens[startCurPtr].type) && isRegisteredVariableToken( tokens[ startCurPtr ].token )  && openBrack == 0 ){
						startCurPtr--;
						break;
					}					 
					curValueVector.push_back( tokens[ startCurPtr++ ] );
				}
				( curValueVector.size() > 1 ) ? valueVector.push_back( curValueVector ): valueVector.push_back( curValueVector );
				( arrayOpenedCount > 0 ) ? valueToken.push_back( VALUE_TOKENS::ARRAY_VALUE ): valueToken.push_back( VALUE_TOKENS::NORMAL_VALUE );
			}
		}	
	}
	return VariableTokens( varTokens, valueToken, valueVector, VarQueue );
}

void passValidVarDeclaration( std::vector<VARIABLE_TOKENS>& varTokens, std::vector<VAR_INFO>& variableStack , std::queue<Token>& VarQueue ){
	if( !varTokens.size() ) 
		throw std::runtime_error("Variable token is empty");

	VARIABLE_TOKENS currentStage = VARIABLE_TOKENS::VAR_START;
	size_t currentPointer = 0;
	while( currentStage == varTokens[ currentPointer ] ){		
		if( currentStage == VARIABLE_TOKENS::VALUE_ASSIGN ) 
			break;
		switch( varTokens[ currentPointer ] ){
			case VARIABLE_TOKENS::NAME: {
				if( VarQueue.empty() )
					throw std::runtime_error("variable queue is empty");
				else{
					Token variableName = VarQueue.front();
					VarQueue.pop( );
					
					if( variableName.type != TOKEN_TYPE::IDENTIFIER )
						throw InvalidSyntaxError("Invalid Variable Name");
					
					VAR_INFO newVariable( variableName.token, false );					
					variableStack.push_back( newVariable );
				}
				break;
			}
			case VARIABLE_TOKENS::ARRAY: {
				if( variableStack.empty() )
					throw InvalidSyntaxError("Array property without variable");
				variableStack.back().isTypeArray = true;
				break;
			}
			default: break;
		}
		if(currentPointer + 1 >= varTokens.size() )
			break;

		std::vector<VARIABLE_TOKENS> nextPossibleTokens = VARIABLE_DECLARE_GRAPH[ varTokens[ currentPointer++ ] ];
		
		bool continueChecking = false;
		for( int x = 0; x < nextPossibleTokens.size(); x++ ){
			if( nextPossibleTokens[ x ] == varTokens[ currentPointer ] ){
				currentStage = varTokens[ currentPointer ];
				continueChecking = true;
				break;
			}
		}
		if( !continueChecking )
			break;
	}
	if( currentStage != VARIABLE_TOKENS::VALUE_ASSIGN || currentPointer != varTokens.size() - 1 )
		throw InvalidSyntaxError( "Occures Syntax error in Variable declaration" );
}

void passValidValueTokens( std::vector<VALUE_TOKENS>& valueTokens ){
	if( !valueTokens.size() )
		throw std::runtime_error("Value tokens empty");

	VALUE_TOKENS currentStage = VALUE_TOKENS::NORMAL_VALUE;
	size_t currentPointer 	  = 0;
	size_t updatedVariable 	  = 0;
	
	if( valueTokens[ currentPointer ] == VALUE_TOKENS::ARRAY_OPEN )
		currentStage = VALUE_TOKENS::ARRAY_OPEN;
	
	while( valueTokens[ currentPointer ] == currentStage ){
		if( currentStage == VALUE_TOKENS::VALUE_END )
			return ;

		if( currentPointer + 1 >= valueTokens.size() )
			break;

		bool continueChecking = false;
		std::vector<VALUE_TOKENS> graph = VALUE_ASSIGN_GRAPH[ valueTokens[ currentPointer++ ] ];

		for(int x = 0; x < graph.size(); x++ ){
			if( valueTokens[ currentPointer ] == graph[ x ] ){
				currentStage = valueTokens[ currentPointer ];
				continueChecking = true;
			}
		}
		if( !continueChecking )
			break;
	}
	if( currentStage != VALUE_TOKENS::VALUE_END)
		throw InvalidSyntaxError("Occures Syntax error in Variable initialization");
}

ArrayAccessTokens stringToArrayAccesToken( const std::vector<Token>&tokens, size_t& currentPtr ){
	std::vector<std::vector<Token>> indexVector;
	std::vector<ARRAY_ACCESS> 		arrAccessTokens;
	std::string 				  	arrName;
	ARRAY_ACCESS prev = ARRAY_ACCESS::NOTHING;

	std::string propertyName;
	bool isTouchedArrayProperty = false;

	while( currentPtr < tokens.size() ){
		const Token& tok = tokens[ currentPtr ];
		const std::string& curToken = tok.token;

		if( curToken == "[" && tok.type == TOKEN_TYPE::SPEC_CHAR )
			arrAccessTokens.push_back( ARRAY_ACCESS::BRACK_OPEN );

		else if( curToken == "]" && tok.type == TOKEN_TYPE::SPEC_CHAR )
			arrAccessTokens.push_back( ARRAY_ACCESS::BRACK_CLOSE );

		else if( curToken == ":" && tok.type == TOKEN_TYPE::OPERATOR ){
			isTouchedArrayProperty = true;
			arrAccessTokens.push_back( ARRAY_ACCESS::PROPERTY_ACCESS );
		}
		else {
			if( prev == ARRAY_ACCESS::NOTHING ){
				arrName = curToken;
				arrAccessTokens.push_back( ARRAY_ACCESS::VAR_NAME );
			}
			else if( prev == ARRAY_ACCESS::BRACK_OPEN ){
				arrAccessTokens.push_back( ARRAY_ACCESS::INDEX_VECTOR );
				std::vector<Token> curIndexVec;
				
				int openCnts = 1;

				while( currentPtr < tokens.size() ){
					const Token& tok = tokens[ currentPtr ];
					if( tok.token == "[" && tok.type == TOKEN_TYPE::SPEC_CHAR ){
						openCnts++; currentPtr++;
						curIndexVec.push_back( tok );
						continue;
					}
					if( tok.token == "]" && tok.type == TOKEN_TYPE::SPEC_CHAR ){
						if( --openCnts == 0 ){
							currentPtr--;
							break;
						}
						curIndexVec.push_back( tok );
						currentPtr++;
						continue;
					}
					
					curIndexVec.push_back( tok );
					currentPtr++;
				}
				indexVector.push_back( curIndexVec );
			}
			else if( prev == ARRAY_ACCESS::PROPERTY_ACCESS ){
				propertyName = curToken;
				arrAccessTokens.push_back( ARRAY_ACCESS::PROPERTY_NAME );
			}
			else goto FINAL;
		}
		currentPtr++;
		prev = arrAccessTokens.empty() ? prev : arrAccessTokens.back();
	}
	FINAL:
		arrAccessTokens.push_back( ARRAY_ACCESS::END );
		ArrayAccessTokens newArrToken( arrAccessTokens, arrName, indexVector );
		newArrToken.isTouchedArrayProperty = isTouchedArrayProperty;

		if( isTouchedArrayProperty )
			newArrToken.setArrayProperty( propertyName );

		return newArrToken;
}

std::unordered_map<ARRAY_ACCESS, std::vector<ARRAY_ACCESS>> ARRAY_ACCESS_GRAPH = {
	{ ARRAY_ACCESS::VAR_NAME, 		  { ARRAY_ACCESS::PROPERTY_ACCESS,
										ARRAY_ACCESS::BRACK_OPEN, 
									    ARRAY_ACCESS::END } 			},
	{ ARRAY_ACCESS::BRACK_OPEN, 	  { ARRAY_ACCESS::INDEX_VECTOR }  	},
	{ ARRAY_ACCESS::INDEX_VECTOR, 	  { ARRAY_ACCESS::BRACK_CLOSE } 	},
	{ ARRAY_ACCESS::BRACK_CLOSE, 	  { ARRAY_ACCESS::END, 
										ARRAY_ACCESS::PROPERTY_ACCESS,
										ARRAY_ACCESS::BRACK_OPEN } },
	{ ARRAY_ACCESS::PROPERTY_ACCESS,  { ARRAY_ACCESS::PROPERTY_NAME }   },
	{ ARRAY_ACCESS::PROPERTY_NAME,    { ARRAY_ACCESS::END } 			},
};

void passArrayAccessToken( std::vector<ARRAY_ACCESS>& tokens ){
	size_t startIndex = 0;
	ARRAY_ACCESS currentStage = ARRAY_ACCESS::VAR_NAME;

	while( startIndex < tokens.size() ){
		ARRAY_ACCESS newTok = tokens[ startIndex ];
		
		if( newTok == ARRAY_ACCESS::END )
			return ;
		
		if( startIndex + 1 >= tokens.size() )
			break;

		ARRAY_ACCESS nextExpected = tokens[ ++startIndex ];
		std::vector<ARRAY_ACCESS>& nextExpectedTokens = ARRAY_ACCESS_GRAPH[ currentStage ];
	
		bool continueNext = false;
		for( ARRAY_ACCESS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError("Invalid access method");
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError("Failed to reach token end for array access syntax");
}

