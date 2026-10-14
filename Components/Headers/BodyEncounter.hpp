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

using ArrayPtr = std::shared_ptr<ArrayList<ARRAY_SUPPORT_TYPES>>;
using FuncPtr  = std::shared_ptr<FUNCTION_MAP_DATA>;

class BodyEncounters {
	public:
		FunctionHandler* funcHandler;
		explicit BodyEncounters( FunctionHandler* funcHandler ): funcHandler( funcHandler ) {}

		std::optional<VarDtype> handleArrayProperties( const ArrayPtr& array, const ArrayAccessTokens& arrToken );
		DEEP_VALUE_DATA handleArrayCases( const ArrayPtr& arrayData, const ArrayAccessTokens& arrToken );
		VarDtype handlerVarDtypePrpperties( const DEEP_VALUE_DATA& Vdata, const ArrayAccessTokens& tok );
		DEEP_VALUE_DATA handleVarDtypeCases( const ArrayAccessTokens& arrToken, const VarDtype& varHolder );

		FUNC_RETURN_TYPE
		handleFunctionCall( std::shared_ptr<MapItem>& func, const std::vector<Token>& tokens, size_t& currentPtr, 
							const unsigned int KEY, const FunctionCallReturns* data = nullptr );

		void updateString( std::string& strToUpdate, long int index, const DEEP_VALUE_DATA& updata );
		std::vector<long int> getResolvedIndexVectors( const ArrayAccessTokens& arrToken );
		// only for token vectors that live for the whole program run ( eg: cached instruction tokens )
		const ArrayAccessTokens& getCachedArrayAccess( const std::vector<Token>& tokens, size_t& curPtr );
		void arrayUpdation( const std::vector<Token>& tokens, size_t& curPtr, DEEP_VALUE_DATA upvalue, ArrayPtr& arr );
		void typeCastRequest( InstructionTokens& InsTokensAndData, std::vector<Token>& varsAndVals );
		
		FUNC_RETURN_TYPE getReturnedData( const std::vector<Token>&tokens, size_t& currentPtr );
};

#endif
