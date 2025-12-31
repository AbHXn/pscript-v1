#include <string>
#include <optional>
#include <memory>
#include <iostream>
#include <type_traits>

#include "VarHandler.hpp"
#include "ConditionalHandler.hpp"
#include "IOHandler.hpp"
#include "InstructionHandler.hpp"
#include "LoopHandler.hpp"

#include "AST.hpp"
#include "Parser.hpp"

#include "../Headers/Dtypes.hpp"
#include "../Headers/MBExceptions.hpp"

enum class MAPTYPE{ VARIABLE,  ENVIRON, FUNCTION };

template<typename T>
inline constexpr bool is_number_v =
    std::is_same_v<T, long int> ||
    std::is_same_v<T, double> || 
    std::is_same_v<T, bool>;

struct MapItem;

using VALUE_DATA 			= variant<VarDtype, unique_ptr<MapItem>>;
using AST_NODE_DATA 		= variant<AST_TOKENS, VALUE_DATA>;
using VARIABLE_HOLDER_DATA  = variant<
								VALUE_DATA, 
								unique_ptr<AST_NODE<AST_NODE_DATA>>
								>;

struct MapItem{
	//MAP_ITEM_TYPE mapType;
	unique_ptr<
		VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>
	> var;

	void
	assignSingleValue( unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>> data ){
		if( var->isTypeArray )
			throw InvalidDTypeError("Cannot assign single value to kootam");
		var->value = move( data );
	}

	void
	assignArrayValue( unique_ptr<ArrayList<VARIABLE_HOLDER_DATA>> data ){
		if( !var->isTypeArray )
			throw InvalidSyntaxError("Cannot assign kootam value to single value");
		var->value = move( data );
	}
};

/*
vector<size_t> t = {0};
				auto test = arrayList->getAtIndex(t, 0);
				if( holds_alternative<SingleElement<VARIABLE_HOLDER_DATA>*>(test) ){
					SingleElement<VARIABLE_HOLDER_DATA>* dt = get<SingleElement<VARIABLE_HOLDER_DATA>*>( test );
					VarDtype valueAtIndex = ValueHelper::getValueFromVariableHolderType( *dt->getValue() );
					printVarDtype( valueAtIndex );
				}else{
					ArrayList<VARIABLE_HOLDER_DATA>*dt  = get<ArrayList<VARIABLE_HOLDER_DATA>*>(test);
					ValueHelper::printArray( dt );
				}
*/


std::string unescapeString(const std::string& input) {
    std::string out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            char e = input[i + 1];

            switch (e) {
                case 'n':  out.push_back('\n'); break;
                case 't':  out.push_back('\t'); break;
                case 'r':  out.push_back('\r'); break;
                case 'b':  out.push_back('\b'); break;
                case 'f':  out.push_back('\f'); break;
                case 'v':  out.push_back('\v'); break;
                case '\\': out.push_back('\\'); break;
                case '"':  out.push_back('"');  break;
                case '\'': out.push_back('\''); break;
                case '0':  out.push_back('\0'); break;

                default:
                    // unknown escape → keep original
                    out.push_back(e);
                    break;
            }

            ++i; // skip escape char
        } else {
            out.push_back(input[i]);
        }
    }

    return out;
}


void printVarDtype(const VarDtype& value) {
    std::visit(
        [](const auto& v) {
            using T = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<T, bool>) {
                std::cout << (v ? "sheri" : "thettu");
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << unescapeString(v);
            }
            else {
                std::cout << v;
            }
        },
        value
    );
}



class ValueHelper{
	public:
		static VarDtype 
		getValueFromVariableHolderType( VARIABLE_HOLDER_DATA& vData ){
			if( holds_alternative<VALUE_DATA>( vData ) ){
				auto& valueDataD = get<VALUE_DATA>( vData );
				return ValueHelper::getValueFromValueData( valueDataD );
			}
			else if(  holds_alternative<unique_ptr<AST_NODE<AST_NODE_DATA>>>( vData ) ){
				auto& astNode = get<unique_ptr<AST_NODE<AST_NODE_DATA>>>( vData );
				VarDtype astnodeData = ValueHelper::evaluate_AST_NODE( astNode );
				return astnodeData;
			}
			else throw ;
		}

		static VarDtype
		getValueFromValueData( VALUE_DATA& data ){
			if( holds_alternative<VarDtype>( data ) ){
				VarDtype vardtypeData = get<VarDtype>( data );
				return vardtypeData;
			}
			else{
				unique_ptr<MapItem>& mapItem = get<unique_ptr<MapItem>>( data );
				VarDtype varDtypeData = ValueHelper::getValueFromMapItem( mapItem.get() );
				return varDtypeData;
			}
		}

		static VarDtype 
		evaluate_AST_NODE( unique_ptr<AST_NODE<AST_NODE_DATA>>& astNode ){
			auto& astNodeData = astNode->AST_DATA;

			if( holds_alternative<VALUE_DATA>( astNodeData ) ){
				auto& valueData = get<VALUE_DATA>( astNodeData );

				if( holds_alternative<VarDtype>( valueData ) ){
					VarDtype varDtypeData = get<VarDtype>( valueData );
					return varDtypeData;
				}
				else{
					auto& mapData = get<unique_ptr<MapItem>>( valueData );
					return ValueHelper::getValueFromMapItem( mapData.get() );
				}
			}
			else{
				VarDtype leftData  = ValueHelper::evaluate_AST_NODE( astNode->left );
				VarDtype rightData = ValueHelper::evaluate_AST_NODE( astNode->right );

				return std::visit(
					[&](const auto& x, const auto& y) -> VarDtype {
						using X = std::decay_t<decltype(x)>;
						using Y = std::decay_t<decltype(y)>;

						if constexpr (is_number_v<X> && is_number_v<Y>) {
							switch (astNode->isASTTokens ? get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD) {
								case AST_TOKENS::ADD: 	return static_cast<double>(x) + static_cast<double>(y);
								case AST_TOKENS::SUB: 	return static_cast<double>(x) - static_cast<double>(y);
								case AST_TOKENS::MUL:	return static_cast<double>(x) * static_cast<double>(y);
								case AST_TOKENS::DIV: 	return static_cast<double>(x) / static_cast<double>(y);
								case AST_TOKENS::MOD:  	return static_cast<long int>(x) % static_cast<long int>(y);
								
								case AST_TOKENS::AND: 	return static_cast<bool>(x) && static_cast<bool>(y);
								case AST_TOKENS::OR: 	return static_cast<bool>(x) || static_cast<bool>(y);
								
								case AST_TOKENS::LS_THAN: 	   return static_cast<double>(x) < static_cast<double>(y);
								case AST_TOKENS::GT_THAN:  	   return static_cast<double>(x) > static_cast<double>(y);
								case AST_TOKENS::LS_THAN_EQ:   return static_cast<double>(x) <= static_cast<double>(y);
								case AST_TOKENS::GT_THAN_EQ:   return static_cast<double>(x) >= static_cast<double>(y);
								case AST_TOKENS::D_EQUAL_TO:   return static_cast<double>(x) == static_cast<double>(y);
								case AST_TOKENS::NOT_EQUAL_TO: return static_cast<double>(x) != static_cast<double>(y);

								default: throw std::runtime_error("Unknown operator for numbers");
							}
						} 
						else if constexpr ( std::is_same_v<X, std::string> && std::is_same_v<Y, std::string> ) {
							switch( astNode->isASTTokens ? get<AST_TOKENS>(astNodeData) : AST_TOKENS::ADD ) {
								case AST_TOKENS::ADD: 	return x + y;
								case AST_TOKENS::LS_THAN: 	   return x < y;
								case AST_TOKENS::GT_THAN:  	   return x > y;
								case AST_TOKENS::LS_THAN_EQ:   return x <= y;
								case AST_TOKENS::GT_THAN_EQ:   return x >= y;
								case AST_TOKENS::D_EQUAL_TO:   return x == y;
								case AST_TOKENS::NOT_EQUAL_TO: return x != y;
								default: throw InvalidSyntaxError("Invalid operator for strings");
							} 
							
						} 
						else throw InvalidSyntaxError("Invalid operation between VarDtype types");
						
					}, leftData, rightData
				);
			}
		}

		static VarDtype 
		getValueFromMapItem( MapItem* mapItem ){
			auto& variableData = mapItem->var->value;

			if( holds_alternative<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>> ( variableData ) ){
				auto& SEdata = get<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>>( variableData );
				
				auto completeData = SEdata->getValue( );

				if( completeData != nullptr ){
					VarDtype data = ValueHelper::getValueFromVariableHolderType( *completeData );
					return data;
				}
			}
			throw InvalidSyntaxError( "Cannot copy an kootam variable" );
		}

		static
		void printArray( ArrayList<VARIABLE_HOLDER_DATA>* pArrList ){
			std::vector< 
				std::variant< 
					unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>, 
					unique_ptr<ArrayList<VARIABLE_HOLDER_DATA>>
				>
			>&  arrayList = pArrList->getList( );
			
			cout << '[';
			for(int x = 0; x < arrayList.size(); x++){
				auto& data = arrayList[ x ];
				if( holds_alternative<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>> ( data ) ){
					auto& singleData = get<unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>>> ( data );
					auto dataT = singleData->getValue();
					VarDtype data = ValueHelper::getValueFromVariableHolderType( *dataT );
					printVarDtype( data );
					if( x != arrayList.size() - 1 )
						cout << ", ";
				}
				else{
					auto& array = get<unique_ptr<ArrayList<VARIABLE_HOLDER_DATA>>> (data);
					ValueHelper::printArray( array.get() );
				}
			}
			cout << ']';
		}
};

enum class ENV_TYPE{ NORMAL, FUNCTION, CLASS, LOOP };

class ENV_CORE{
	public:
		string envName;
		ENV_CORE* parent = nullptr;
		ENV_TYPE envType = ENV_TYPE::NORMAL;

		unordered_map<string, unique_ptr<MapItem>> VMAP;

		MapItem*
		getFromVmap(const std::string& key) const{
		    const ENV_CORE* currentEnv = this;

		    while (currentEnv != nullptr) {
		        auto it = currentEnv->VMAP.find(key);

		        if (it != currentEnv->VMAP.end())
		            return it->second.get();

		        currentEnv = currentEnv->parent;
		    }
		    return nullptr;
		}

		void addToMap( string key, unique_ptr<MapItem> data ){
			this->VMAP[ key ] = move( data );
		}

		template <typename QUEUE_TYPE>
		void handleUnknownToken( queue<QUEUE_TYPE>& ValueQueue, string& vdata ){
			try{
				auto data = DtypeHelper::getTypeAndValue( vdata );
				ValueQueue.push( data.second );
			}
			catch( InvalidDTypeError& err ){
				if( DtypeHelper::isValidVariableName ){
					auto VMAPData = this->getFromVmap( vdata );

					if( VMAPData != nullptr ){
						VarDtype copyData = ValueHelper::getValueFromMapItem( VMAPData );
						ValueQueue.push( copyData );
					}
					else throw InvalidSyntaxError( "No variable named " + vdata );
				}
				else throw InvalidSyntaxError( "Unknown token " + vdata );		
			}
		}

		void
		handleBody( vector<string>& tokens, size_t& currentPtr ){
			while( currentPtr < tokens.size() ){
				if( tokens[ currentPtr ] == "pidi" ){
					this->VarHandlerRunner( tokens, currentPtr );
				}
				else if( tokens[currentPtr] == "nok" ){
					this->CondHandlerRunner( tokens, currentPtr );
				}
				else if( tokens[ currentPtr ] == "}" ){
					if( this->parent == nullptr )
						throw InvalidSyntaxError( "unknown token }" );
					return ;
				}
				else if( tokens[ currentPtr ] == "para" ){
					this->IOHandlerRunner( tokens, currentPtr );
				}
				else if( tokens[ currentPtr ] == "ittuthiri" ){
					this->LoopHandlerRunner( tokens, currentPtr );
					currentPtr--;
				}
				else{
					this->InstructionHandlerRunner( tokens, currentPtr );
				}
				currentPtr++;
			}
		}

		void 
		LoopHandlerRunner ( vector<string>& tokens, size_t& currentPtr ){
			auto [ tokInfo, bodyIns ] = stringToLoopTokens( tokens, currentPtr );
			
			if( !isValidLoopTokens( tokInfo.first ) )
				throw InvalidSyntaxError("Invalid Syntax error in loop");

			while( true ){
				auto astTokenAndData = stringToASTTokens( tokInfo.second );
				queue<VALUE_DATA> valueQeue;

				for(string& vdata: astTokenAndData.second)
					handleUnknownToken<VALUE_DATA>( valueQeue, vdata );
				
				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, valueQeue, startAST );

				if( !newAstNode.has_value() )
					throw InvalidSyntaxError("Invalid expressions in loops");

				VarDtype finalValue = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
				if( !holds_alternative<bool>(finalValue) )
					throw InvalidSyntaxError( "loop condition should be boolean or blank" );

				bool runTheBodyAgain = get<bool>( finalValue );

				if( runTheBodyAgain ){
					ENV_CORE newBody;
					newBody.envType = ENV_TYPE::LOOP;
					newBody.parent = this;

					size_t bodyStart = bodyIns.first;
					newBody.handleBody( tokens, bodyStart );

					finalValue = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
					runTheBodyAgain = get<bool>( finalValue );
				}
				else break;
			}
			currentPtr = bodyIns.second;
		}

		void
		InstructionHandlerRunner( vector<string>& tokens, size_t& currentPtr ){
			auto InsTokensAndData = stringToInsToken( tokens, currentPtr );

			if( InsTokensAndData.second.first.size() != InsTokensAndData.second.second.size() )
				throw InvalidSyntaxError( "Invalid Instruction" );

			if( !isValidInstructionSet( InsTokensAndData.first.first ) )
				throw InvalidSyntaxError( "Invalid Instruction set" );

			queue<VarDtype> finalValueQueue;
			auto varsAndVals = InsTokensAndData.second;

			queue<MapItem*> mapVariables;
			for( string& var: varsAndVals.first ){
				MapItem* mapVar = this->getFromVmap( var );
				
				if( mapVar == nullptr )
					throw InvalidSyntaxError( "No variable named " + var );
				
				mapVariables.push( mapVar );
			}

			for( vector<string>& astStrToks: varsAndVals.second ){
				auto astTokenAndData = stringToASTTokens( astStrToks );
				queue<VALUE_DATA> valueQeue;

				for(string& vdata: astTokenAndData.second)
					handleUnknownToken<VALUE_DATA>( valueQeue, vdata );
				
				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, valueQeue, startAST );
				
				if( !newAstNode.has_value() )
					throw InvalidSyntaxError("Invalid expressions");

				VarDtype finalValue = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
				finalValueQueue.push( finalValue );
			}

			while( !mapVariables.empty() ){
				MapItem* var = mapVariables.front();
				VarDtype value = finalValueQueue.front();

				mapVariables.pop();
				finalValueQueue.pop();

				unique_ptr<SingleElement<VARIABLE_HOLDER_DATA>> newData = make_unique<
							SingleElement<VARIABLE_HOLDER_DATA>>();
				
				/*
					// IMPLEMENT +=, -= short keys here
				*/

				newData->assignValue(value);
				var->assignSingleValue( move( newData ) );
			}
		}

		void VarHandlerRunner( vector<string>& test, size_t& start ){
			auto tokens  = codeToTokens( test, start );
			auto toks 	 = tokens.first;
			auto names 	 = tokens.second;

			vector<unique_ptr<VARIABLE_HOLDER<VARIABLE_HOLDER_DATA>>> vars;

			bool isValidVar = isValidVariableSyntax<VARIABLE_HOLDER_DATA>( toks.first, vars, names.first );

			for( auto& varVerification: vars ){
				auto data = this->getFromVmap( varVerification->key );
				if( data != nullptr )
					throw VariableAlreayExists( varVerification->key );
			}

			if( isValidVar ){
				auto valueVectors = names.second;

				queue<VARIABLE_HOLDER_DATA> finalValueQueue;

				for( auto& VarientData: valueVectors ){
					if( holds_alternative< vector<string> > ( VarientData ) ){
						auto data = get<vector<string>> ( VarientData );

						auto astTokenAndData = stringToASTTokens( data );
						queue<VALUE_DATA> ValueQueue;

						for(string& vdata: astTokenAndData.second){
							handleUnknownToken<VALUE_DATA>( ValueQueue, vdata );
						}
						size_t startAST = 0;
						auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, ValueQueue, startAST );
						if( !newAstNode.has_value() ){
							return;
						}

						finalValueQueue.push( move( newAstNode.value() ) );
					}
					else{
						auto vdata = get<string>( VarientData );
						handleUnknownToken<VARIABLE_HOLDER_DATA>( finalValueQueue, vdata );
					}
				}

				bool isValValue = isValidValueSyntax<VARIABLE_HOLDER_DATA>( toks.second, vars, finalValueQueue );

				if( isValValue ){
					for( auto& Variable: vars ){
						string& key    = Variable->key;
						auto newMapVar = make_unique<MapItem>( );
						newMapVar->var = move( Variable );

						addToMap( key, move( newMapVar ) );
					}
				}
			}
			else throw InvalidDTypeError( "Invalid Syntax occured in variable declaration" );
		}

		size_t 
		runNewEnvFromThis( vector<string>& tokens, size_t index ){
			ENV_CORE nm;
			nm.parent = this;
			nm.handleBody( tokens, index );
			return index;
		}

		void CondHandlerRunner( vector<string>& test, size_t& start ){
			size_t endOfNOK = 0;
			auto tokens = stringToCondTokens( test, start, endOfNOK );
			
			if( !isValidCondToken( tokens.first ) )
				return;

			for(int x = 0; x < tokens.second.size(); x++){
				auto data = tokens.second[ x ];
				
				auto astTokenAndData = stringToASTTokens( data.first );
				queue<VALUE_DATA> ValueQueue;
				
				for( string& tok: astTokenAndData.second ){
					handleUnknownToken<VALUE_DATA>( ValueQueue, tok );
				}

				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, ValueQueue, startAST );
				if( !newAstNode.has_value() )
					return;

				VarDtype asEval = ValueHelper::evaluate_AST_NODE( newAstNode.value() );

				if( get<bool>(asEval) ){
					this->runNewEnvFromThis( test, data.second + 1 );
					start = endOfNOK;
					return;
				}

			}
		}

		void IOHandlerRunner( vector<string>& tokens, size_t& start ){
			auto tokensAndData = stringToIoTokens( tokens, start );
			
			if( !isValidIoTokens( tokensAndData.first ) )
				return;

			queue<VarDtype> finalQueue;
			
			for( vector<string>&valToks: tokensAndData.second ){
				auto astTokenAndData = stringToASTTokens( valToks );
				queue<VALUE_DATA> ValueQueue;

				for( string& oData: astTokenAndData.second )
					handleUnknownToken( ValueQueue, oData );

				size_t startAST = 0;
				auto newAstNode = BUILD_AST<VALUE_DATA, AST_NODE_DATA>( astTokenAndData.first, ValueQueue, startAST );
				
				if( !newAstNode.has_value() )
					return;

				VarDtype asEval = ValueHelper::evaluate_AST_NODE( newAstNode.value() );
				finalQueue.push( asEval );
			}

			bool isFirstPrint = true;
			for( IO_TOKENS tok: tokensAndData.first ){
				switch( tok ){
					case IO_TOKENS::PRINT: {
						if( !isFirstPrint )
							cout << "\n";
						else isFirstPrint = false;

						if( finalQueue.empty() )
							throw InvalidSyntaxError("In para statement");

						VarDtype top = finalQueue.front();
						finalQueue.pop();
						printVarDtype( top );
						break;
					}
					case IO_TOKENS::CONCAT:{
						cout << " ";
						if( finalQueue.empty() )
							throw InvalidSyntaxError("In koode statement");

						VarDtype top = finalQueue.front();
						finalQueue.pop();
						printVarDtype( top );
						break;
					}
					case IO_TOKENS::PRINT_VALUE:
						continue;
					
					case IO_TOKENS::END:
					default:
						return;
				}
			}
		}
};

int main(  ){
	try{
		string filename = "test.mb";
		vector<string> tokens = parseTheCodeToTokens( filename );
		ENV_CORE mn;
	
		size_t currentPtr = 0;
		mn.handleBody( tokens, currentPtr );
		// auto t = mn.getFromVmap("test");
		// cout << "=== ";
		// printVarDtype(ValueHelper::getValueFromMapItem(t));
		// cout << endl;
	}
	catch( InvalidSyntaxError& err ){
		cout << err.what() << endl;
	}
	return 0;
}