#include "Headers/Extension.hpp"

std::unordered_map<std::string, EXT_TYPE> preComputed;

namespace PreComputedCaching{
	void VariableCaching( const std::vector<Token>& test, size_t& start, std::string KEY, FunctionHandler* func ){
		VariableTokens tokens  = stringToVariableTokens( test, start );
		size_t endPtr = start;

		std::vector<std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>> astNodes;
		size_t curIndex = 0;

		for(auto testVec: tokens.valueVector){
			std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> evaluatedRes = ExprResolver::getAstRootNode( testVec, func ); 
			astNodes.push_back( std::move( evaluatedRes ));
		}
		std::vector<VAR_INFO> varInfos;
		passValidVarDeclaration( tokens.varTokens, varInfos, tokens.VarQueue );
		passValidValueTokens( tokens.valueTokens );

		ExtendedVariableToken newExtTok = ExtendedVariableToken(
			tokens, std::move( astNodes ), varInfos, endPtr
		);
		preComputed[ KEY ] = std::move( newExtTok );
	}

	void InstructionCaching( const std::vector<Token>& tokens, size_t& currentPtr, std::string KEY, FunctionHandler* func ){
		InstructionTokens InsTokensAndData = stringToInsToken( tokens, currentPtr );
		passValidInstructionTokens( InsTokensAndData.insToken );

		ExtendedInsTokens newInsToken = ExtendedInsTokens( InsTokensAndData, currentPtr);

		if( InsTokensAndData.optr == INS_TOKEN::TYPE_CAST )
			preComputed[KEY] = std::move( newInsToken ); 
		else{
			std::vector< std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>> insTreeNodes;

			for(auto& astStrToks: InsTokensAndData.rightVector){
				auto treeNode = ExprResolver::getAstRootNode( astStrToks, func );
				insTreeNodes.push_back( std::move( treeNode ) );
			}
			newInsToken.insTree = std::move( insTreeNodes );
			preComputed[ KEY ] = std::move( newInsToken );
		}
	}

	void IOCaching( const std::vector<Token>& tokens, size_t& start, std::string KEY, FunctionHandler* func){
		auto tokensAndData = stringToIoTokens( tokens, start );
		passValidIOTokens( tokensAndData.first );

		std::vector<std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>> outputInfo;

		for( std::vector<Token>&valToks: tokensAndData.second ){
			auto astNode = ExprResolver::getAstRootNode( valToks, func );
			outputInfo.push_back( std::move( astNode ) );
		}
		ExtendedIoToken newIo = ExtendedIoToken( tokensAndData, std::move( outputInfo ), start );
		preComputed[ KEY ] = std::move( newIo );
	}

	void ConditionalCaching( 
			std::unordered_map<std::string, size_t>& bMap, 
			std::string& filename,
			const std::vector<Token>& tokens, 
			size_t& start, 
			std::string KEY, 
			FunctionHandler* func 
		){
		CondReturnToken ctokens  = stringToCondTokens( bMap, tokens, start, filename );
 		passCondTokenValidation( ctokens.tokens );

		std::vector<std::pair<std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>>, size_t>> astNodes; 

		for( int x = 0; x < ctokens.conditions.size(); x++ ){
			auto curCond = ctokens.conditions[ x ];
			auto treeNode = ExprResolver::getAstRootNode( curCond.first, func );
			astNodes.push_back( std::move( make_pair( std::move( treeNode ), curCond.second ) ) );
		}

		ExtendedConditionalToken newExtTok = ExtendedConditionalToken( ctokens, std::move( astNodes ) );
		preComputed[ KEY ] = std::move( newExtTok );
	}

	void LoopCaching ( 
			std::unordered_map<std::string, size_t>& bMap, 
			std::string& filename,
			const std::vector<Token>& tokens, 
			size_t& currentPtr, 
			std::string KEY, 
			FunctionHandler* func 
		){
		LoopTokens lpTokens  = stringToLoopTokens( bMap, tokens, currentPtr, filename );
 		passValidLoopTokens( lpTokens.lpTokens );

		std::unique_ptr<AST_NODE<REAL_AST_NODE_DATA>> astNode = ExprResolver::getAstRootNode( lpTokens.conditions, func ); 
		ExtendedLoopTokens newExtTok = ExtendedLoopTokens( lpTokens, std::move( astNode ) );
		preComputed[ KEY ] = std::move( newExtTok );
	}
}
