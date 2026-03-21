#ifndef EXTENSION_HPP
#define EXTENSION_HPP

enum class STATES{
	VARIABLE	,
	FUNCTION	,
	LOOP		,
	CONDITION	,
	IOTOKENS	,
	INSTRUCTON
};

struct ExtendedIoToken;
struct ExtendedVariableToken;
struct ExtendedConditionalToken;
struct ExtendedLoopTokens;
struct ExtendedInsTokens;

using EXT_TYPE = std::variant<
					ExtendedConditionalToken,
					ExtendedLoopTokens,
					ExtendedVariableToken,
					ExtendedIoToken,
					ExtendedInsTokens
					>;

struct ExtendedVariableToken{
	VariableTokens tokens;
	std::vector<
		std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>
		>
	> valueAst;
	std::vector<VAR_INFO> varInfos;
	size_t endPtr;

	ExtendedVariableToken() = default;

	ExtendedVariableToken(
		VariableTokens tokens, 
		std::vector<
			std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>
			>
		> valueAst,
		std::vector<VAR_INFO> varInfos,
		size_t endPtr
	){
		this->tokens = tokens;
		this->valueAst = std::move( valueAst );
		this->varInfos = varInfos;
		this->endPtr = endPtr;
	}
};

struct ExtendedIoToken{
	IOTokenReturn insTokens;
	std::vector<
		std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>
	> outputInfo;
	size_t endPtr;

	ExtendedIoToken() = default;

	ExtendedIoToken(
		IOTokenReturn insTokens,
		std::vector<
			std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>
		> outputInfo, 
		size_t endPtr
	){
		this->insTokens = insTokens;
		this->outputInfo = std::move( outputInfo );
		this->endPtr = endPtr;
	}
};

struct ExtendedConditionalToken{
	CondReturnToken ctokens;
	std::vector<
		std::pair<
			std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>, 
			size_t
		>
	> conditions;

	ExtendedConditionalToken() = default;

	ExtendedConditionalToken(
		CondReturnToken ctokens,
		std::vector<
			std::pair<
				std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>, 
				size_t
			>
		> conditions
	){
		this->ctokens = ctokens;
		this->conditions = std::move( conditions );
	}
};

struct ExtendedLoopTokens{
	LoopTokens lpToken;
	std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> loopAst;

	ExtendedLoopTokens() = default;
	
	ExtendedLoopTokens(
		LoopTokens lpToken,
		std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> loopAst
	){
		this->lpToken = lpToken;
		this->loopAst = std::move( loopAst );
	}
};

struct ExtendedInsTokens{
	InstructionTokens InsTokensAndData;
	std::vector<
		std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>
	> insTree;
	size_t endPtr;

	ExtendedInsTokens() = default;

	ExtendedInsTokens(
		InstructionTokens InsTokensAndData,
		size_t endPtr
	){
		this->InsTokensAndData = InsTokensAndData;
		this->endPtr = endPtr;
	}
};

std::unordered_map<std::string, EXT_TYPE> preComputed;

#endif