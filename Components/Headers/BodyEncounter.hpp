#ifndef BODYENC_HPP
#define BODYENC_HPP

#include "VMAP.hpp"
#include "ValueHelper.hpp"

#include "../Verifier/Headers/Variables.hpp"
#include "../Verifier/Headers/Conditional.hpp"
#include "../Verifier/Headers/Function.hpp"
#include "../Verifier/Headers/loops.hpp"
#include "../Verifier/Headers/InputOutput.hpp"
#include "../Verifier/Headers/Instruction.hpp"

#include "Extension.hpp"
#include "ExprResolver.hpp"
#include "FunctionHandler.hpp"

class BodyEncounters {
	public:
		FunctionHandler* funcHandler;
		BodyEncounters( FunctionHandler* funcHandler );

		std::optional<VarDtype> handleArrayProperties( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> array, ArrayAccessTokens& arrToken);
		DEEP_VALUE_DATA handleArrayCases( std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>> arrayData, ArrayAccessTokens& arrToken );
		VarDtype handlerVarDtypePrpperties( DEEP_VALUE_DATA& Vdata, ArrayAccessTokens& tok );
		DEEP_VALUE_DATA handleVarDtypeCases( ArrayAccessTokens& arrToken, DEEP_VALUE_DATA& varHolder );

		FUNC_RETURN_TYPE
		handleFunctionCall( std::shared_ptr<MapItem>& func, const std::vector<Token>& tokens, size_t& currentPtr, 
							std::string KEY, std::optional<FunctionCallReturns> data = std::nullopt );

		void updateString( std::string& strToUpdate, long int index, DEEP_VALUE_DATA updata );
		std::vector<long int> getResolvedIndexVectors( std::vector<std::vector<Token>>& indexVector );
		void arrayUpdation( const std::vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>& arr );
		void typeCastRequest( InstructionTokens& InsTokensAndData, std::vector<Token>& varsAndVals );
		
		FUNC_RETURN_TYPE getReturnedData( const std::vector<Token>&tokens, size_t& currentPtr );
};

#endif