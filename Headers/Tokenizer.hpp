#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <cctype>
#include <string_view>
#include <unordered_map>

extern std::unordered_set<std::string_view> RESERVED_KEYS;
extern std::unordered_set<std::string_view> SCHARS;
extern std::unordered_set<std::string_view> OPTR;

bool isReservedKey(const std::string& tok);
bool isOptr(const std::string& tok);
bool isSpec(const std::string& tok);

extern std::vector<std::string> debug_string;


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

// same order
enum class TOKEN_CONST {
	PINDI,
	PIDI,
	SHERI,
	THETTU,
	PODA,
	PINNAVA,
	THEKU,
	ITTUTHIRI,
	NOK,
	UMBI,
	ONNULA,
	PARA,
	KOODE,
	UM,
	YO,
	KOOTAM,
	EDUKKU,
	NOTHING
};

bool isValueType(TOKEN_TYPE type);

struct Token{
	unsigned int tokenId;
	TOKEN_TYPE   type;
	TOKEN_CONST  tokConst = TOKEN_CONST::NOTHING;
	std::string  token;
	size_t row;
	size_t col;

	Token(
		unsigned int tokenId,
		TOKEN_TYPE type,
		std::string token,
		size_t row,
		size_t col
	){
		this->tokenId = tokenId;
		this->type = type;
		this->token = token;
		this->row = row;
		this->col = col;
	}
};
void getTheTokens(const std::string& filename, std::vector<Token>& Tokens);

#endif