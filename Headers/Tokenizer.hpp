#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

enum class TOKEN_TYPE{
	NOTHING, NUMBER, FLOATING, STRING, BOOLEAN, IDENTIFIER, RESERVED, SPEC_CHAR, OPERATOR
};

// same order
enum class TOKEN_CONST {
	PINDI, PIDI, SHERI, THETTU, PODA, PINNAVA, THEKU, ITTUTHIRI, NOK, UMBI,
	ONNULA, PARA, KOODE, UM, YO, KOOTAM, EDUKKU, NOTHING
};

inline bool isValueType( TOKEN_TYPE type ){
	return type == TOKEN_TYPE::NUMBER || type == TOKEN_TYPE::STRING || type == TOKEN_TYPE::FLOATING || type == TOKEN_TYPE::BOOLEAN;
}

struct Token{
	unsigned int tokenId;
	TOKEN_TYPE   type;
	TOKEN_CONST  tokConst = TOKEN_CONST::NOTHING;
	std::string  token;
	size_t row;
	size_t col;

	Token( unsigned int tokenId, TOKEN_TYPE type, std::string token, size_t row, size_t col ):
		tokenId( tokenId ), type( type ), token( std::move( token ) ), row( row ), col( col ) {}
};

void getTheTokens( const std::string& filename, std::vector<Token>& Tokens );

#endif
