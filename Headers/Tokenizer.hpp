#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_set>

using namespace std;

unordered_set<string> RESERVED_KEYS = {
	"thenga", "pidi", "sheri", "thettu", "poda", "pinnava",
	"theku", "ittuthiri", "nok", "umbi", "onnula", "para", 
	"koode", "um", "yo", "kootam", "edukku"
};

unordered_set<string> SCHARS = {
	":", "(", ")", "[", "]", "{", "}", ",", ".", ";"
};

unordered_set<string> OPTR{
	"+", "-", "*", "/", "%", ">", "<", "=", "!", ":"
};

bool isReservedKey( const string& tok ){
	return RESERVED_KEYS.find( tok ) != RESERVED_KEYS.end();
}
bool isOptr( const string& tok ){
	return OPTR.find( tok ) != OPTR.end();
}
bool isSpec( const string& tok ){
	return SCHARS.find( tok ) != SCHARS.end();
}

vector<string> debug_string = { "NOTHING", "NUMBER", "FLOATING", 
								"STRING", "BOOLEAN", "IDENTIFIER", "RESERVED", 
								"SPEC_CHAR", "OPTR" };

enum class TOKEN_TYPE{
	NOTHING		,
	NUMBER 		,
	FLOATING	,
	STRING  	,
	BOOLEAN		,
	IDENTIFIER ,
	RESERVED 	,
	SPEC_CHAR 	,
	OPERATOR 
};

bool isValueType( TOKEN_TYPE type ){
	return ( type == TOKEN_TYPE::NUMBER || type == TOKEN_TYPE::STRING 
			|| type == TOKEN_TYPE::FLOATING || type == TOKEN_TYPE::BOOLEAN );
}


struct Token{
	TOKEN_TYPE type;
	string token;
	size_t row;
	size_t col;

	Token( 
		TOKEN_TYPE type,
		string token,
		size_t row,
		size_t col
	){
		this->type 	= type;
		this->token = token;
		this->row 	= row;
		this->col 	= col;
	}
};

void getTheTokens( const string& filename, vector<Token>& Tokens ){
	ifstream codeFile(filename);
	
	char cCode;
	string currentWord = "";
	TOKEN_TYPE currentState = TOKEN_TYPE::NOTHING;

	size_t lineNumber = 0, colNumber = 0;
	bool isSkip = false;

	while( codeFile.get(cCode) ){
		if( cCode == '"'  && currentState != TOKEN_TYPE::STRING ){
			if( currentState != TOKEN_TYPE::NOTHING )
				throw runtime_error("Invalid token at line: " + to_string(lineNumber) + 
									 " Column: " + to_string(colNumber) );
			currentState = TOKEN_TYPE::STRING;
			continue;
		}
		// string input handler
		if( currentState == TOKEN_TYPE::STRING ){
			if( cCode == '\\' )
				isSkip = true;
			else if( cCode == '"' ){
				if( currentWord.size() > 0 && currentWord[ currentWord.size()-1 ] == '\\'){
					currentWord.push_back('"');
					continue;
				}
			}
			if( cCode == '"' ){
				Tokens.push_back( Token( currentState, currentWord, lineNumber, colNumber ) );
				currentWord  = "";
				currentState = TOKEN_TYPE::NOTHING;
				colNumber += 1;
			}
			else {
				currentWord.push_back( cCode ); 
				isSkip = false;
			}
			continue;
		}
		// line checker
		if( cCode == '\n' ){
			lineNumber++;
			colNumber = 0;
			continue;
		}
		// encounter an digit
		if( isdigit( cCode ) ){
			if( currentState == TOKEN_TYPE::NUMBER || 
				currentState == TOKEN_TYPE::IDENTIFIER || 
				currentState == TOKEN_TYPE::FLOATING ){
					currentWord.push_back( cCode );
			} 
			else if( currentState == TOKEN_TYPE::NOTHING ){
				currentState = TOKEN_TYPE::NUMBER;
				currentWord.push_back( cCode );
			}
			else if( currentState == TOKEN_TYPE::OPERATOR && 
					( currentWord == "-" || currentWord == "+" ) ){
				currentState = TOKEN_TYPE::NUMBER;
				currentWord.push_back( cCode );
			}
			else throw runtime_error("Invalid token at line: " + to_string(lineNumber) + 
									 " Column: " + to_string(colNumber) );
		}
		else if( isalpha( cCode ) || cCode == '_' ){
			if( currentState == TOKEN_TYPE::IDENTIFIER )
				currentWord.push_back( cCode );
			
			else if( currentState == TOKEN_TYPE::NOTHING ){
				currentState = TOKEN_TYPE::IDENTIFIER;
				currentWord.push_back( cCode );
			}
			else if( currentState == TOKEN_TYPE::OPERATOR ){
				Tokens.push_back( Token( currentState, currentWord, lineNumber,  colNumber ) );
				colNumber++;

				currentWord = string(1, cCode);
				currentState = TOKEN_TYPE::IDENTIFIER;
			}			
		}
		else if( cCode == ' ' ){
			if( !currentWord.empty() && currentState != TOKEN_TYPE::NOTHING ){
				if( currentState == TOKEN_TYPE::NUMBER || currentState == TOKEN_TYPE::FLOATING || currentState == TOKEN_TYPE::STRING ) {
					Tokens.push_back( Token( currentState, currentWord, lineNumber,  colNumber ) );
					currentWord  = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber += 1;
					continue;
				}
				if( isReservedKey( currentWord ) ) {
					currentState = TOKEN_TYPE::RESERVED;
					if( currentWord == "sheri" || currentWord == "thettu" )
						currentState = TOKEN_TYPE::BOOLEAN;
				}
				
				else if( isOptr( currentWord ) )
					currentState = TOKEN_TYPE::OPERATOR;

				else if( isSpec( currentWord ) )
					currentState = TOKEN_TYPE::SPEC_CHAR;
				
				Tokens.push_back( Token( currentState, currentWord, lineNumber,  colNumber ) );
				currentWord  = "";
				currentState = TOKEN_TYPE::NOTHING;
				colNumber += 1;
			}
		}
		else {
			if( cCode == '=' ){
				if( isOptr( currentWord ) ){
					currentWord.push_back( cCode );
					Tokens.push_back( Token( TOKEN_TYPE::OPERATOR, currentWord, lineNumber, colNumber ) );
					currentWord  = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber += 1;
				}
				else {
					if( currentState != TOKEN_TYPE::NOTHING ){
						Tokens.push_back( Token( currentState, currentWord, lineNumber, colNumber ) );
						colNumber++;
					}
					currentState = TOKEN_TYPE::OPERATOR;
					currentWord = string(1, cCode);
				}
			}
			else if( isOptr( string(1, cCode) ) ){
				if( currentState != TOKEN_TYPE::NOTHING ){
					Tokens.push_back( Token( currentState, currentWord, lineNumber, colNumber ) );
					currentWord  = string(1, cCode);
					currentState = TOKEN_TYPE::OPERATOR;
					colNumber += 1;
				}
				else {
					currentWord.push_back( cCode );
					currentState = TOKEN_TYPE::OPERATOR;
				}
			}
			else if( cCode == '.' && currentState == TOKEN_TYPE::NUMBER ){
				currentWord.push_back( cCode );
				currentState = TOKEN_TYPE::FLOATING;
			}
			else if( isSpec( string(1, cCode) ) ){
				if( currentState != TOKEN_TYPE::NOTHING ){
					if( isReservedKey( currentWord ) ) {
						currentState = TOKEN_TYPE::RESERVED;
						if( currentWord == "sheri" || currentWord == "thettu" )
							currentState = TOKEN_TYPE::BOOLEAN;
					}
					Tokens.push_back( Token( currentState, currentWord, lineNumber, colNumber ) );
					currentWord  = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber += 1;
				}
				Tokens.push_back( Token( TOKEN_TYPE::SPEC_CHAR, string(1, cCode), lineNumber, colNumber ));
				colNumber++;
			}
		}
	}
	// for( auto x: Tokens )
	// 	cout << debug_string[ (int) x.type ] << ": " << x.token << endl;
	if( currentState != TOKEN_TYPE::NOTHING )
		Tokens.push_back( Token( currentState, currentWord, lineNumber, colNumber ) );
}

#endif