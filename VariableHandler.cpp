#include "VariableHandler.hpp"

using namespace std;

std::queue<std::string> VarQueue, ValueQueue;


std::unordered_map <VARIABLE_TOKENS, std::vector<VARIABLE_TOKENS>> VARIABLE_DECLARE_GRAPH = {
	{ VARIABLE_TOKENS::VAR_START, 	{ VARIABLE_TOKENS::NAME } 								  },

	{ VARIABLE_TOKENS::NAME,		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::ARRAY, VARIABLE_TOKENS::VAR_ENDS }     },

	{ VARIABLE_TOKENS::ARRAY, 		{ VARIABLE_TOKENS::COMMA, VARIABLE_TOKENS::VALUE_ASSIGN, 
					  				  VARIABLE_TOKENS::VAR_ENDS } 		 					  },

	{ VARIABLE_TOKENS::COMMA, 		{ VARIABLE_TOKENS::NAME } 						  		  }	
};

std::unordered_map <VALUE_TOKENS, std::vector<VALUE_TOKENS>> VALUE_ASSIGN_GRAPH = {
	{ VALUE_TOKENS::NORMAL_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END } 			},
	{ VALUE_TOKENS::ARRAY_OPEN,  { VALUE_TOKENS::ARRAY_OPEN, VALUE_TOKENS::ARRAY_VALUE } 	},
	{ VALUE_TOKENS::ARRAY_VALUE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::ARRAY_CLOSE } 	   	},
	{ VALUE_TOKENS::COMMA, 	  	 { VALUE_TOKENS::ARRAY_VALUE, VALUE_TOKENS::ARRAY_OPEN, 
								   VALUE_TOKENS::NORMAL_VALUE } 							},
	{ VALUE_TOKENS::ARRAY_CLOSE, { VALUE_TOKENS::COMMA, VALUE_TOKENS::VALUE_END }		   	}
};

pair<vector<VARIABLE_TOKENS>, vector<VALUE_TOKENS>>  
VARIABLE_HANDLER::codeToTokens( vector<string>& tokens, size_t& curIndexPtr ){
	int startCurPtr = curIndexPtr;

	vector<VARIABLE_TOKENS> varTokens;
	vector<VALUE_TOKENS> 	valueToken;

	bool isVariableTurn = true;
	int arrayOpenedCount = 0;

	for( ; startCurPtr < tokens.size(); ++startCurPtr ) {
		string curToken = tokens[ startCurPtr ];

		if( curToken == "pidi" ){
			varTokens.push_back( VARIABLE_TOKENS::VAR_START );
		}
		
		else if( curToken == ","){
			( isVariableTurn ) ? 
				varTokens.push_back( VARIABLE_TOKENS::COMMA )
				: valueToken.push_back( VALUE_TOKENS::COMMA );
		}
		
		else if( curToken == "kootam" ){
			varTokens.push_back( VARIABLE_TOKENS::ARRAY );
		}

		else if ( curToken == "{" ){
			arrayOpenedCount++;
			valueToken.push_back( VALUE_TOKENS::ARRAY_OPEN );
		}
		
		else if( curToken == "="){
			varTokens.push_back( VARIABLE_TOKENS::VALUE_ASSIGN );
			isVariableTurn = false;
		}

		else if(curToken == "}"){
			arrayOpenedCount--;
			valueToken.push_back( VALUE_TOKENS::ARRAY_CLOSE );
		}

		else if(curToken == ";"){
			( isVariableTurn ) ? 
				varTokens.push_back( VARIABLE_TOKENS::VAR_ENDS )
				: valueToken.push_back( VALUE_TOKENS::VALUE_END );
		}
		else{
			if( isVariableTurn ){
				VarQueue.push( tokens[ startCurPtr ] );
				varTokens.push_back( VARIABLE_TOKENS::NAME );
			}else{
				ValueQueue.push( tokens[ startCurPtr ] );
				if( arrayOpenedCount > 0 )
					valueToken.push_back( VALUE_TOKENS::ARRAY_VALUE );
				else valueToken.push_back( VALUE_TOKENS::NORMAL_VALUE );
			}
		}		
	}
	return { varTokens, valueToken };
}

void
SingleElement::assignType( DTYPES type ){
	this->realType = type;
}

void
SingleElement::assignValue( const VarDtype& value ){
	this->data 	= value;
	this->isValueDefined = true;
}

optional<VarDtype>
SingleElement::getValue( void ) const {
	if( realType == DTYPES::VALUE_NOT_DEFINED )
		return nullopt;
	return data;
}


unique_ptr<ArrayList>
ArrayList::_arrayListBuilder( vector<VALUE_TOKENS>& arrayTokenList, size_t& curIndex, queue<string>& valueQueue) {
	auto newArrayList = make_unique< ArrayList >();
	vector<SingleElement> resultList;

	try{

		while( curIndex < arrayTokenList.size() ){
			if( arrayTokenList[ curIndex ] == VALUE_TOKENS::COMMA ){
				curIndex++;
				continue;
			}

			if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_OPEN ){
				curIndex++; curly_opened++;

				unique_ptr<ArrayList> array = _arrayListBuilder( arrayTokenList, curIndex, valueQueue );
				newArrayList->push_ArrayList( *array );

				if( !curly_opened )
					return newArrayList;
			}

			else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_VALUE ){
				string value = valueQueue.front();
				valueQueue.pop();

				auto newVariable = make_unique<SingleElement>();
				auto valueInfo = DtypeHelper::getTypeAndValue( value );

				newVariable->assignType( valueInfo.first );						
				newVariable->assignValue( valueInfo.second );

				resultList.push_back( *newVariable );
			}

			else if( arrayTokenList[ curIndex ] == VALUE_TOKENS::ARRAY_CLOSE ){
				curly_opened--;
				if( resultList.size() > 0 )
					newArrayList->push_SingleElement( resultList );
				return newArrayList;
			}
			curIndex++;
		}
	}catch ( const InvalidDTypeError& err ){
		cout << err.what() << endl;
	}
		
	return newArrayList;
}

ArrayList 
ArrayList::createArray( vector<VALUE_TOKENS>& arrayTokenList, size_t& currentPointer ){
	unique_ptr<ArrayList> arrayResult = ArrayList::_arrayListBuilder( arrayTokenList, currentPointer,  ValueQueue );	
	return *arrayResult;
}

void 
ArrayList::push_SingleElement( vector<SingleElement>& singleElementList ){
	this->arrayList = singleElementList;
}

void 
ArrayList::push_ArrayList( ArrayList& arrayListElement ){
	auto* vec = get_if<vector<ArrayList>>( &arrayList );
    if ( !vec ){
    	vector<ArrayList> newArrayList = { arrayListElement };
    	this->arrayList = newArrayList;
    }
    else vec->push_back(arrayListElement);		    	
}

void 
ArrayList::printArray( void ) const {
	const auto* nested = get_if<vector<ArrayList>>(&this->arrayList);
	if ( nested ){
        cout << "[";
        for (size_t i = 0; i < nested->size(); ++i){
            (*nested)[i].printArray();
            if (i + 1 < nested->size())
                cout << ", ";
        }
        cout << "]";
        return ;
    }
    const auto* flat = get_if<vector<SingleElement>>(&this->arrayList);
   	if ( flat ){
        cout << "[";
        for (size_t i = 0; i < flat->size(); ++i){
            visit(
                [](const auto& value) {
                    cout << value;
                }, (*flat)[i].data );
            if (i + 1 < flat->size())
                cout << ", ";
        }
        cout << "]";
    }
}

unique_ptr<VARIABLE_HANDLER::VARIABLE> 
VARIABLE_HANDLER::getNewVariable( string varName ){
	auto newVariable = make_unique<VARIABLE_HANDLER::VARIABLE>( );
	
	newVariable->variableName = varName;
	newVariable->varStatus 	  = ( unsigned int ) FLAGS::IS_PRIVATE;
	newVariable->variableType = DTYPES::VALUE_NOT_DEFINED;

	return newVariable;
}

void 
VARIABLE_HANDLER::VARIABLE::printVariable( void ){
	variant<SingleElement, ArrayList> value = this->value;
	if( auto ptr = get_if<SingleElement>( &value ) ){		
		std::visit([](auto &val) {
       		cout << val << "\n";
    	}, ptr->data);

	}else{
		const auto ptr1 = get_if<ArrayList> (&value);
		ptr1->printArray();
	}
}

bool 
VARIABLE_HANDLER::isValidVariableSyntax( vector<VARIABLE_TOKENS>& varTokens, vector<VARIABLE_HANDLER::VARIABLE>& variableStack ){
	if( !varTokens.size() )
		return false;

	VARIABLE_TOKENS currentStage = VARIABLE_TOKENS::VAR_START;
	size_t currentPointer		 = 0;

	try{
		while( currentStage == varTokens[ currentPointer ] ){		
			if( currentStage == VARIABLE_TOKENS::VALUE_ASSIGN )
				break;

			switch( varTokens[ currentPointer ] ){
				case VARIABLE_TOKENS::NAME: {
					if( VarQueue.empty() )
						return false;
					else{
						string variableName = VarQueue.front();
						if( !DtypeHelper::isValidVariableName( variableName ) ){
							throw InvalidNameError( variableName );
						}
						auto newVariable = getNewVariable( VarQueue.front( ) );
						VarQueue.pop( );
						variableStack.push_back( *newVariable );
					}
					break;
				}

				case VARIABLE_TOKENS::ARRAY: {
					if( variableStack.empty() ){
						return false;
					}
					else{
						VARIABLE_HANDLER::VARIABLE& backStack = variableStack.back();
						backStack.varStatus |= ( unsigned int ) FLAGS::IS_TYPE_ARRAY;
					}
					break;
				}
				default:
					break;
			}

			if( currentPointer + 1 >= varTokens.size() )
				break;

			currentPointer++;
			vector<VARIABLE_TOKENS> nextPossibleTokens = VARIABLE_DECLARE_GRAPH[ varTokens[ currentPointer - 1 ] ];
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
		return currentStage == VARIABLE_TOKENS::VALUE_ASSIGN;
	} 
	catch ( const InvalidNameError& err ){
		cout << err.what() << endl;
		return false;
	}
}

bool 
VARIABLE_HANDLER::isValidValueSyntax( vector<VALUE_TOKENS>& valueTokens, vector<VARIABLE_HANDLER::VARIABLE>& variable ){
	if( !valueTokens.size() )
		return false;

	VALUE_TOKENS currentStage = VALUE_TOKENS::NORMAL_VALUE;
	size_t currentPointer 	  = 0;
	size_t updatedVariable 	  = 0;

	try {

		if( valueTokens[ currentPointer ] == VALUE_TOKENS::ARRAY_OPEN )
			currentStage = VALUE_TOKENS::ARRAY_OPEN;

		while( valueTokens[ currentPointer ] == currentStage ){
			if( currentStage == VALUE_TOKENS::VALUE_END )
				break;

			if( updatedVariable >= variable.size() ){
				throw VariableDeclarationMissing( );
			}

			switch( valueTokens[ currentPointer ] ){
				case VALUE_TOKENS::ARRAY_OPEN: {
					
					VARIABLE_HANDLER::VARIABLE& topVariable = variable[ updatedVariable ];

					if( !( topVariable.varStatus & (unsigned int) FLAGS::IS_TYPE_ARRAY ) ){
						// raise exeption; given type is not an array
						return false;
					}
					
					ArrayList newArray = ArrayList::createArray( valueTokens, currentPointer );
					topVariable.value = newArray;	

					updatedVariable++;
					break;
					
				}
				case VALUE_TOKENS::NORMAL_VALUE:{
					VARIABLE_HANDLER::VARIABLE& topVariable = variable[updatedVariable];

					if( topVariable.varStatus & (unsigned int) FLAGS::IS_TYPE_ARRAY ){
						// raise exeption; given type is an array					
						return false;
					}

					if( ValueQueue.empty() ){
						// no variable found to update
						return false;
					}
					string value = ValueQueue.front();

					auto newElement = make_unique<SingleElement>();
					auto typedValue = DtypeHelper::getTypeAndValue( value );

					newElement->assignType( typedValue.first );
					newElement->assignValue( typedValue.second );

					ValueQueue.pop();

					topVariable.value = *newElement;
					updatedVariable++;

					break;
				}
				default:
					break;
			}

			if( currentPointer + 1 >= valueTokens.size() )
				break;

			currentPointer++;
			bool continueChecking = false;

			vector<VALUE_TOKENS> graph = VALUE_ASSIGN_GRAPH[ valueTokens[ currentPointer - 1 ] ];

			for(int x = 0; x < graph.size(); x++ ){
				if( valueTokens[ currentPointer ] == graph[ x ] ){
					currentStage = valueTokens[ currentPointer ];
					continueChecking = true;
				}
			}

			if( !continueChecking )
				break;
		}
		while( !VarQueue.empty() ){
			VarQueue.pop();
		}
		while( !ValueQueue.empty() ){
			ValueQueue.pop();
		}
		return currentStage == VALUE_TOKENS::VALUE_END;
	}
	catch ( const VariableDeclarationMissing& err ){
		cout << err.what() << endl;
		return false;
	}
	catch( const InvalidDTypeError& err ){
		cout << err.what() << endl;
		return false;
	}
}