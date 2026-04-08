#ifndef LOOP_HPP
#define LOOP_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <string_view>

#include "../../../Headers/Tokenizer.hpp"
#include "../../../Headers/MBExceptions.hpp"

extern std::unordered_set<std::string_view> REG_LOOP_TOKENS, REG_LOOP_BODY_TOKENS;

enum class LOOP_TOKENS{
	LOOP_START 	,
	CONDITION 	,
	BREAK 		,
	CONTINUE 	,
	BODY_OPEN 	,
	BODY 		,
	BODY_CLOSE
};

struct LoopTokens{
	std::vector<LOOP_TOKENS> lpTokens;
	std::vector<Token> conditions;
	size_t startPtr;
	size_t endPtr;

	LoopTokens() = default;

	LoopTokens(
		std::vector<LOOP_TOKENS> lpTokens,
		std::vector<Token> conditions,
		size_t startPtr,
		size_t endPtr
	){
		this->lpTokens 	 = lpTokens;
		this->conditions = conditions;
		this->startPtr   = startPtr;
		this->endPtr 	 = endPtr;
	}
};

LoopTokens stringToLoopTokens( const std::vector<Token>& tokens, size_t& startIndex );
void passValidLoopTokens( std::vector<LOOP_TOKENS>& tokens );
bool isRegisteredLoopBodyTokens( const std::string& tok );
bool isRegisteredLoopTokens( const std::string& tok );
size_t getEndPointer( const std::vector<Token>& tokens, size_t startPtr );

#endif