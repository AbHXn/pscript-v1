#include "Headers/InputOutput.hpp"

std::unordered_set<std::string_view> REGISTERED_IO_TOKENS = { "koode", "para", ";" };

bool isRegisteredIoTokens( const std::string& token ){
	return REGISTERED_IO_TOKENS.find( token ) != REGISTERED_IO_TOKENS.end();
}

std::unordered_map<IO_TOKENS, std::vector<IO_TOKENS>> IO_GRAPH = {
	{ IO_TOKENS::PRINT, 		{ IO_TOKENS::PRINT_VALUE } },
	{ IO_TOKENS::PRINT_VALUE,   { IO_TOKENS::END, IO_TOKENS::CONCAT, 
								  IO_TOKENS::PRINT } },
	{ IO_TOKENS::CONCAT, 		{ IO_TOKENS::PRINT_VALUE } },
};

void
passValidIOTokens( std::vector<IO_TOKENS>& IOTokens ){
	if( IOTokens.empty() )
		throw InvalidSyntaxError("Empty IO tokens error");

	IO_TOKENS currentStage = IO_TOKENS::PRINT;
	size_t startIndex = 0;

	while( startIndex < IOTokens.size() ){
		if( IOTokens[ startIndex ] == IO_TOKENS::END )
			return ;

		if( startIndex + 1 >= IOTokens.size() )
			break;

		IO_TOKENS nextExpected = IOTokens[ ++startIndex ];
		std::vector<IO_TOKENS>& nextExpectedTokens = IO_GRAPH[ currentStage ];

		bool continueNext = false;
		for( IO_TOKENS nextToks: nextExpectedTokens ){
			if( nextToks == nextExpected ){
				continueNext = true;
				break;
			}
		}
		if( !continueNext )
			throw InvalidSyntaxError( "Invalid Token encounter in para" );
		currentStage = nextExpected;
	}
	throw InvalidSyntaxError( "Do dont encounter end ; token in para" );
}

IOTokenReturn
stringToIoTokens( const std::vector<Token>& tokens, size_t& startIndex ){
	std::vector<IO_TOKENS> IOTokens;
	std::vector<std::vector<Token>> printValues;

	for(; startIndex < tokens.size(); startIndex++){
		const std::string& curToken = tokens[ startIndex ].token;
		if( curToken == "para" )
			IOTokens.push_back( IO_TOKENS::PRINT );
		
		else if( curToken == "koode" )
			IOTokens.push_back( IO_TOKENS::CONCAT );
		
		else if( curToken == ";" ){
			IOTokens.push_back( IO_TOKENS::END );
			return { IOTokens, printValues };
		}
		else{
			std::vector<Token> valueVector;
			while( startIndex < tokens.size() ){
				if( isRegisteredIoTokens( tokens[ startIndex ].token ) 
						&& (tokens[startIndex].type == TOKEN_TYPE::RESERVED || 
						tokens[startIndex].type == TOKEN_TYPE::SPEC_CHAR) ){
					startIndex--;
					break;
				}
				valueVector.push_back( tokens[ startIndex++ ] );
			}
			printValues.push_back( valueVector );
			IOTokens.push_back( IO_TOKENS::PRINT_VALUE );
		}
	}
	throw InvalidSyntaxError("Failed to find the end of token ;");
}

