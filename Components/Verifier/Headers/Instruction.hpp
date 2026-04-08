#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <unordered_set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "../../../Headers/Tokenizer.hpp"
#include "../../../Headers/MBExceptions.hpp"

extern std::unordered_set<std::string_view> REG_INS_TOKEN;

bool isRegisteredInsTokens( const std::string& tok );
enum class INS_TOKEN{
	NOT_DEFINED,
	REPLACE,
	COMMA,
	ADD_REPLACE,
	TYPE_CAST  ,
	SUB_REPLACE,
	MUL_REPLACE,
	DIV_REPLACE,
	MOD_REPLACE,
	LHS_VALUE,
	RHS_VALUE,
	INS_END,
};

bool MainToken( const std::string& curToken, INS_TOKEN& Optr );

struct InstructionTokens{
	std::vector<INS_TOKEN> insToken;
	INS_TOKEN optr;
	std::vector<std::vector<Token>> rightVector;
	std::vector<Token> leftVector;

	InstructionTokens() = default;

	InstructionTokens(
		std::vector<INS_TOKEN> insToken,
		INS_TOKEN optr,
		std::vector<std::vector<Token>> rightVector,
		std::vector<Token> leftVector
	){
		this->insToken 	  = insToken;
		this->optr 		  = optr;
		this->rightVector = rightVector;
		this->leftVector  = leftVector;
	}
};

InstructionTokens stringToInsToken(const std::vector<Token>& tokens, size_t& startIndex );
void passValidInstructionTokens( std::vector<INS_TOKEN>& tokens );

#endif