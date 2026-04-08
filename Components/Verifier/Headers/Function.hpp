#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string_view>
#include <memory>

#include "../../../Headers/Tokenizer.hpp"
#include "../../../Headers/MBExceptions.hpp"

extern std::unordered_set<std::string_view> REGISTERED_FUNC_TOKEN, REGISTERED_FUNC_BODY_TOKENS;

enum class FUNC_TOKENS{
	NOTHING 	,
	FUNC_START 	,
	FUNC_NAME 	,
	ARGS_OPEN 	,
	ARGS_CLOSE 	,
	BODY_OPEN 	,
	FUNC_BODY 	,
	BODY_CLOSE 	,
	VAR_START 	,
	VAR_NAME 	,
	VAR_ARRAY 	,
	VAR_COMMA
};

// For arg Name and Type ( Single or Array )
struct ARG_VAR_INFO{
	std::string name;
	bool isArray;
};

// Func Info
struct FunctionTokenReturn {
	std::vector<FUNC_TOKENS> 			 tokens;
	std::vector<std::unique_ptr<ARG_VAR_INFO>> args;
	std::string 					 funcName;
	size_t 							 funcStartPtr;
	size_t 							 funcEndPtr;

	FunctionTokenReturn() = default;

	FunctionTokenReturn( 
		std::vector<FUNC_TOKENS> tokens,
		std::vector<std::unique_ptr<ARG_VAR_INFO>> args,
		std::string funcName,
		size_t funcStartPtr,
		size_t funcEndPtr
	){
		this->tokens 		= tokens;
		this->args 			= std::move(args);
		this->funcName 		= funcName;
		this->funcStartPtr  = funcStartPtr;
		this->funcEndPtr	= funcEndPtr;
	}
};

void passValidFuncToken( std::vector<FUNC_TOKENS>& tokens );

FunctionTokenReturn stringToFuncTokens( const std::vector<Token>&tokens, size_t& startIndex );

/* --------------------------- FUNC CALL HANDLER --------------------------------*/

enum class FUNC_CALL_TOKEN{
	NOTHING,   // 0
	FUNC_NAME, // 1
	ARG_OPEN,  // 2
	ARG_COMMA, // 3
	ARG_VALUE, // 4
	ARG_CLOSE, // 5
};

bool isFuncPtr( std::vector<FUNC_CALL_TOKEN>& callTokens );
void passValidFuncCallToken( std::vector<FUNC_CALL_TOKEN>& callTokens );

struct FunctionCallReturns{
	std::vector<FUNC_CALL_TOKEN> callTokens;
	std::vector<std::vector<Token>> argsVector;
	std::string funcName;

	FunctionCallReturns() = default;

	FunctionCallReturns( 
		std::vector<FUNC_CALL_TOKEN> callTokens,
		std::vector<std::vector<Token>> argsVector,
		std::string funcName
	){
		this->callTokens = callTokens;
		this->argsVector = argsVector;
		this->funcName 	 = funcName;
	}
};

FunctionCallReturns stringToFunctionCallTokens( const std::vector<Token>& tokens, size_t& curPtr );
#endif

