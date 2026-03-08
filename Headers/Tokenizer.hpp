#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <cctype>

std::unordered_set<std::string> RESERVED_KEYS = {
	"thenga", "pidi", "sheri", "thettu", "poda", "pinnava",
	"theku", "ittuthiri", "nok", "umbi", "onnula", "para", 
	"koode", "um", "yo", "kootam", "edukku"
};

std::unordered_set<std::string> SCHARS = {
	":", "(", ")", "[", "]", "{", "}", ",", ".", ";"
};

std::unordered_set<std::string> OPTR{
	"+", "-", "*", "/", "%", ">", "<", "=", "!", ":"
};

bool isReservedKey(const std::string& tok){
	return RESERVED_KEYS.find(tok) != RESERVED_KEYS.end();
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

enum class TOKEN_TYPE{
	NOTHING,
	NUMBER,
	FLOATING,
	STRING,
	BOOLEAN,
	IDENTIFIER,
	RESERVED,
	SPEC_CHAR,
	OPERATOR
};

bool isValueType(TOKEN_TYPE type){
	return ( type == TOKEN_TYPE::NUMBER || type == TOKEN_TYPE::STRING 
			|| type == TOKEN_TYPE::FLOATING || type == TOKEN_TYPE::BOOLEAN );
}

struct Token{
	TOKEN_TYPE type;
	std::string token;
	size_t row;
	size_t col;

	Token(
		TOKEN_TYPE type,
		std::string token,
		size_t row,
		size_t col
	){
		this->type = type;
		this->token = token;
		this->row = row;
		this->col = col;
	}
};

void getTheTokens(const std::string& filename, std::vector<Token>& Tokens){

	std::ifstream codeFile(filename);

	if(!codeFile.is_open())
		throw std::runtime_error("Cannot open file: " + filename);

	char cCode;
	std::string currentWord = "";
	TOKEN_TYPE currentState = TOKEN_TYPE::NOTHING;

	size_t lineNumber = 0, colNumber = 0;
	bool isSkip = false;

	while(codeFile.get(cCode)){
		if(cCode == '"' && currentState != TOKEN_TYPE::STRING){
			if(currentState != TOKEN_TYPE::NOTHING)
				throw std::runtime_error("Invalid token at line: " + std::to_string(lineNumber) + " Column: " + std::to_string(colNumber));

			currentState = TOKEN_TYPE::STRING;
			continue;
		}
		if(currentState == TOKEN_TYPE::STRING){
			if(cCode == '\\')
				isSkip = true;

			else if(cCode == '"'){
				if(!currentWord.empty() && currentWord.back() == '\\'){
					currentWord.push_back('"');
					continue;
				}
			}
			if(cCode == '"'){
				Tokens.push_back(Token(currentState, currentWord, lineNumber, colNumber));
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
		if(isdigit((unsigned char)cCode)){
			if(currentState == TOKEN_TYPE::NUMBER || currentState == TOKEN_TYPE::IDENTIFIER || currentState == TOKEN_TYPE::FLOATING){
				currentWord.push_back(cCode);
			} 
			else if(currentState == TOKEN_TYPE::NOTHING){
				currentState = TOKEN_TYPE::NUMBER;
				currentWord.push_back(cCode);
			}
			else if(currentState == TOKEN_TYPE::OPERATOR && (currentWord == "-" || currentWord == "+")){
				currentState = TOKEN_TYPE::NUMBER;
				currentWord.push_back(cCode);
			}
			else throw std::runtime_error("Invalid token at line: " + std::to_string(lineNumber) + " Column: " + std::to_string(colNumber));
		}
		else if(isalpha((unsigned char)cCode) || cCode == '_'){
			if(currentState == TOKEN_TYPE::IDENTIFIER)
				currentWord.push_back(cCode);

			else if(currentState == TOKEN_TYPE::NOTHING){
				currentState = TOKEN_TYPE::IDENTIFIER;
				currentWord.push_back(cCode);
			}
			else if(currentState == TOKEN_TYPE::OPERATOR){
				Tokens.push_back(Token(currentState, currentWord, lineNumber, colNumber));
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

					Tokens.push_back(Token(currentState, currentWord, lineNumber, colNumber));
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

				Tokens.push_back(Token(currentState, currentWord, lineNumber, colNumber));

				currentWord = "";
				currentState = TOKEN_TYPE::NOTHING;
				colNumber++;
			}
		}
		else{
			if(cCode == '='){
				if(isOptr(currentWord)){
					currentWord.push_back(cCode);
					Tokens.push_back(Token(TOKEN_TYPE::OPERATOR,currentWord,lineNumber,colNumber));
					currentWord = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber++;
				}
				else{
					if(currentState != TOKEN_TYPE::NOTHING){
						Tokens.push_back(Token(currentState,currentWord,lineNumber,colNumber));
						colNumber++;
					}
					currentState = TOKEN_TYPE::OPERATOR;
					currentWord = std::string(1, cCode);
				}
			}
			else if(isOptr(std::string(1, cCode))){
				if(currentState != TOKEN_TYPE::NOTHING){
					Tokens.push_back(Token(currentState,currentWord,lineNumber,colNumber));
					currentWord = std::string(1, cCode);
					currentState = TOKEN_TYPE::OPERATOR;
					colNumber++;
				}
				else{
					currentWord.push_back(cCode);
					currentState = TOKEN_TYPE::OPERATOR;
				}
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
					Tokens.push_back(Token(currentState,currentWord,lineNumber,colNumber));
					currentWord = "";
					currentState = TOKEN_TYPE::NOTHING;
					colNumber++;
				}
				Tokens.push_back(Token(TOKEN_TYPE::SPEC_CHAR,std::string(1, cCode),lineNumber,colNumber));
				colNumber++;
			}
		}
	}
	if(currentState != TOKEN_TYPE::NOTHING)
		Tokens.push_back(Token(currentState, currentWord, lineNumber, colNumber));
}

#endif
