#ifndef FUNCBODY_HPP
#define FUNCBODY_HPP

#include "VMAP.hpp"
#include "ValueHelper.hpp"
#include "../Verifier/Headers/Variables.hpp"
#include "../Verifier/Headers/Conditional.hpp"
#include "../Verifier/Headers/Function.hpp"
#include "../Verifier/Headers/loops.hpp"
#include "../Verifier/Headers/InputOutput.hpp"
#include "../Verifier/Headers/Instruction.hpp"
#include "Extension.hpp"
#include "BodyEncounter.hpp"

enum class CALLER{ LOOP, FUNCTION, CONDITIONAL };

struct Context{
	size_t LastRecoverErrorList = 0;
	std::vector<Token> fullTokens;
	size_t pointer = 0;
	std::string filename;
};

class FunctionHandler: public VAR_VMAP {
	public:
		std::string functionName;
		std::shared_ptr<Context> ctx;

		FunctionHandler( std::shared_ptr<Context> ctx ): ctx(ctx) {};
		FunctionHandler( VAR_VMAP* parent, std::string runnerBody, std::shared_ptr<Context> ctx );

		void VarHandlerRunner( const std::vector<Token>& test, size_t& start, std::string KEY );
		void InstructionHandlerRunner( const std::vector<Token>& tokens, size_t& currentPtr, std::string KEY );
		void functionDefHandlerRunner( const std::vector<Token>&token, size_t& start );
		void IOHandlerRunner( const std::vector<Token>& tokens, size_t& start, std::string KEY );
		void CondHandlerRunner( const std::vector<Token>& tokens, size_t& start, std::string KEY );
		void LoopHandlerRunner ( const std::vector<Token>& tokens, size_t& currentPtr, std::string KEY );
};

FUNC_RETURN_TYPE
ProgramExecutor( const std::vector<Token>& tokens, size_t& currentPtr, CALLER C_CLASS, FunctionHandler* prntClass, size_t endPtr = 0  );


#endif