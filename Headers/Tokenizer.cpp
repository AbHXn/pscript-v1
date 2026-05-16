#include "Tokenizer.hpp"

std::unordered_map<std::string, TOKEN_CONST> RESERVED = {
	{ "pindi", TOKEN_CONST::PINDI },
	{ "pidi", TOKEN_CONST::PIDI },
	{ "sheri", TOKEN_CONST::SHERI },
	{ "thettu", TOKEN_CONST::THETTU },
	{ "poda", TOKEN_CONST::PODA },
	{ "pinnava", TOKEN_CONST::PINNAVA },
	{ "theku", TOKEN_CONST::THEKU },
	{ "ittuthiri", TOKEN_CONST::ITTUTHIRI },
	{ "nok", TOKEN_CONST::NOK },
	{ "umbi", TOKEN_CONST::UMBI },
	{ "onnula", TOKEN_CONST::ONNULA },
	{ "para", TOKEN_CONST::PARA },
	{ "koode", TOKEN_CONST::KOODE },
	{ "um", TOKEN_CONST::UM },
	{ "yo", TOKEN_CONST::YO },
	{ "kootam", TOKEN_CONST::KOOTAM },
	{ "edukku", TOKEN_CONST::EDUKKU },
};

TOKEN_CONST getTokenConst( const std::string& str ){
	auto elem = RESERVED.find( str );
	if( elem != RESERVED.end() )
		return elem->second;
	return TOKEN_CONST::NOTHING;
}

std::unordered_set<std::string_view> SCHARS = {
	":", "(", ")", "[", "]", "{", "}", ",", ".", ";"
};

std::unordered_set<std::string_view> OPTR{
	"+", "-", "*", "/", "%", ">", "<", "=", "!", ":", "?"
};


bool isReservedKey(const std::string& tok){
	return RESERVED.find(tok) != RESERVED.end();
}

bool isOptr(const std::string& tok){
	return OPTR.find(tok) != OPTR.end();
}

bool isSpec(const std::string& tok){
	return SCHARS.find(tok) != SCHARS.end();
}

std::vector<std::string> debug_string = { 
	"NOTHING", "NUMBER", "FLOATING", 
	"STRING", "BOOLEAN", "IDENTIFIER", 
	"RESERVED", "SPEC_CHAR", "OPTR" 
};

bool isValueType(TOKEN_TYPE type){
	return ( type == TOKEN_TYPE::NUMBER || type == TOKEN_TYPE::STRING  || type == TOKEN_TYPE::FLOATING || type == TOKEN_TYPE::BOOLEAN );
}

void getTheTokens(const std::string& filename, std::vector<Token>& Tokens){
	std::ifstream codeFile(filename);

	if(!codeFile.is_open())
		throw std::runtime_error("Cannot open file: " + filename);

	char cCode;
	std::string currentWord = "";
	TOKEN_TYPE currentState = TOKEN_TYPE::NOTHING;

	size_t lineNumber = 0, colNumber = 0;
	bool isSkip = false;
	bool comments = false;

	unsigned int tokenId = 0;

	while(codeFile.get(cCode)){
		if( cCode == '?' ){
			comments = !comments;
			continue;
		}
		if( comments ){
			if( cCode == '\n' )
				lineNumber++;
			continue;
		}

		if(cCode == '"' && currentState != TOKEN_TYPE::STRING){
			if(currentState != TOKEN_TYPE::NOTHING)
				throw std::runtime_error("Invalid token at line: " + std::to_string(lineNumber) + " Column: " + std::to_string(colNumber));
			currentState = TOKEN_TYPE::STRING;
			continue;
		}

		if(currentState == TOKEN_TYPE::STRING){
			if(cCode == '\\')
				isSkip = true;

			if(cCode == '"'){
				if(!currentWord.empty() && currentWord.back() == '\\'){
					currentWord.push_back('"');
					continue;
				}
				// currentState will be a string 
				Tokens.push_back(Token(++tokenId, currentState, currentWord, lineNumber, colNumber));
				currentWord = "";
				currentState = TOKEN_TYPE::NOTHING;
				colNumber++;
			}
			else{
				currentWord.push_back(cCode);
				isSkip = false;
			}
			continue;
		}
		if(cCode == '\n'){
			lineNumber++;
			colNumber = 0;
			continue;
		}
		/* if current char is a digit */
		if( isdigit( static_cast<unsigned char>(cCode) ) ){
		    if( currentState == TOKEN_TYPE::NUMBER ||  currentState == TOKEN_TYPE::IDENTIFIER || currentState == TOKEN_TYPE::FLOATING )
		        currentWord.push_back(cCode);
		    
		    else if( currentState == TOKEN_TYPE::NOTHING ){
		    	currentState = TOKEN_TYPE::NUMBER;
		        currentWord.push_back(cCode);
		    }
		    else{
		    	Token newToken = Token( ++tokenId, currentState,currentWord,lineNumber,colNumber );
		        if( currentState == TOKEN_TYPE::RESERVED ){
		        	TOKEN_CONST tokConst = getTokenConst( currentWord );
		      		newToken.tokConst = tokConst;
		      	}
		        Tokens.push_back( newToken );

		        currentWord = std::string( 1, cCode );
		    	currentState = TOKEN_TYPE::NUMBER;
		    }
		}

		else if( isalpha( (unsigned char)cCode ) || cCode == '_' ){
			if( currentState == TOKEN_TYPE::IDENTIFIER )
				currentWord.push_back(cCode);

			else if( currentState == TOKEN_TYPE::NOTHING ){
				currentState = TOKEN_TYPE::IDENTIFIER;
				currentWord.push_back(cCode);
			}
			else if( currentState == TOKEN_TYPE::OPERATOR ){
				Tokens.push_back( Token( ++tokenId, currentState, currentWord, lineNumber, colNumber ) );
				colNumber++;
				currentWord = std::string(1, cCode);
				currentState = TOKEN_TYPE::IDENTIFIER;
			}			
		}
		else if(cCode == ' '){
			if(!currentWord.empty() && currentState != TOKEN_TYPE::NOTHING){
				if(currentState == TOKEN_TYPE::NUMBER ||
				   currentState == TOKEN_TYPE::FLOATING ||
				   currentState == TOKEN_TYPE::STRING){

					Tokens.push_back(Token(++tokenId, currentState, currentWord, lineNumber, colNumber));
					currentWord = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber++;
					continue;
				}
				if(isReservedKey(currentWord)){
					currentState = TOKEN_TYPE::RESERVED;
					if(currentWord == "sheri" || currentWord == "thettu")
						currentState = TOKEN_TYPE::BOOLEAN;
				}
				else if(isOptr(currentWord))
					currentState = TOKEN_TYPE::OPERATOR;

				else if(isSpec(currentWord))
					currentState = TOKEN_TYPE::SPEC_CHAR;

				Token newToken = Token( ++tokenId, currentState,currentWord,lineNumber,colNumber );
		        if( currentState == TOKEN_TYPE::RESERVED ){
		        	TOKEN_CONST tokConst = getTokenConst( currentWord );
		      		newToken.tokConst = tokConst;
		      	}
		        Tokens.push_back( newToken );

				currentWord = "";
				currentState = TOKEN_TYPE::NOTHING;
				colNumber++;
			}
		}
		else{
			if(cCode == '='){
				if(isOptr(currentWord)){
					currentWord.push_back(cCode);
					Tokens.push_back(Token(++tokenId, TOKEN_TYPE::OPERATOR,currentWord,lineNumber,colNumber));
					currentWord = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber++;
				}
				else{
					if(currentState != TOKEN_TYPE::NOTHING){
						Token newToken = Token( ++tokenId, currentState,currentWord,lineNumber,colNumber );
				        if( currentState == TOKEN_TYPE::RESERVED ){
				        	TOKEN_CONST tokConst = getTokenConst( currentWord );
				      		newToken.tokConst = tokConst;
				      	}
				        Tokens.push_back( newToken );
						colNumber++;
					}
					currentState = TOKEN_TYPE::OPERATOR;
					currentWord = std::string(1, cCode);
				}
			}
			else if(isOptr(std::string(1, cCode))){
			    if((cCode == '-' || cCode == '+') && currentState == TOKEN_TYPE::NOTHING){
			        bool unary = false;

			        if(Tokens.empty())
			            unary = true;
			        else{
			            Token prev = Tokens.back();

			            if(prev.type == TOKEN_TYPE::OPERATOR)
			                unary = true;

			            else if(prev.type == TOKEN_TYPE::SPEC_CHAR &&
			               (prev.token == "(" || prev.token == "[" || prev.token == "{" || prev.token == ",")){
			                unary = true;
			            }
			            else if( prev.type == TOKEN_TYPE::RESERVED && (
			            		prev.token == "para" || prev.token == "poda") )
			            	unary = true;
			        }
			        if(unary){
			            currentState = TOKEN_TYPE::NUMBER;
			            currentWord.push_back(cCode);
			            continue;
			        }
			    }
			    if(currentState != TOKEN_TYPE::NOTHING){
					Token newToken = Token( ++tokenId, currentState,currentWord,lineNumber,colNumber );
			        if( currentState == TOKEN_TYPE::RESERVED ){
			        	TOKEN_CONST tokConst = getTokenConst( currentWord );
			      		newToken.tokConst = tokConst;
			      	}
			        Tokens.push_back( newToken );
			        currentWord.clear();
			    }

			    currentWord = std::string(1, cCode);
			    currentState = TOKEN_TYPE::OPERATOR;
			    colNumber++;
			}
			else if(cCode == '.' && currentState == TOKEN_TYPE::NUMBER){
				currentWord.push_back(cCode);
				currentState = TOKEN_TYPE::FLOATING;
			}
			else if(isSpec(std::string(1, cCode))){
				if(currentState != TOKEN_TYPE::NOTHING){
					if(isReservedKey(currentWord)){
						currentState = TOKEN_TYPE::RESERVED;
						if(currentWord == "sheri" || currentWord == "thettu")
							currentState = TOKEN_TYPE::BOOLEAN;
					}

					Token newToken = Token( ++tokenId, currentState,currentWord,lineNumber,colNumber );
			        if( currentState == TOKEN_TYPE::RESERVED ){
			        	TOKEN_CONST tokConst = getTokenConst( currentWord );
			      		newToken.tokConst = tokConst;
			      	}
			        Tokens.push_back( newToken );

					currentWord = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber++;
				}
				Tokens.push_back(Token(++tokenId, TOKEN_TYPE::SPEC_CHAR,std::string(1, cCode),lineNumber,colNumber));
				colNumber++;
			}
		}
	}
	if(currentState != TOKEN_TYPE::NOTHING)
		Tokens.push_back(Token(++tokenId, currentState, currentWord, lineNumber, colNumber));
}

