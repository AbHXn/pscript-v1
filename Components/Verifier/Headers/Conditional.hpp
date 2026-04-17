#ifndef CONDITIONAL_HPP
#define CONDITIONAL_HPP

#include <unordered_set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "../../../Headers/MBExceptions.hpp"
#include "../../../Headers/Tokenizer.hpp"

enum class COND_TOKENS{ IF, ELSE, CONDITION, BODY_OPEN, BODY_CLOSE, CHAIN, END };

struct CondReturnToken{
	std::vector<COND_TOKENS> tokens;
	std::vector<std::pair<std::vector<Token>, size_t>> conditions;
	size_t endOfNok;

	CondReturnToken() = default;

	CondReturnToken(
		std::vector<COND_TOKENS> tokens,
		std::vector<std::pair<std::vector<Token>, size_t>> conditions,
		size_t endOfNok
	){
		this->tokens = tokens;
		this->conditions = conditions;
		this->endOfNok = endOfNok;
	}
};

bool isRegisteredCondToken( const std::string& tokens );
void passCondTokenValidation( std::vector<COND_TOKENS>& tokens );

CondReturnToken stringToCondTokens( 
		std::unordered_map<std::string, size_t>& bMap,
		const std::vector<Token>& tokens, 
		size_t& start,
		std::string& filename 
	);

#endif 
