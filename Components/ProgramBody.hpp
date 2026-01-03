#ifndef PROGRAMBODY_HPP
#define PROGRAMBODY_HPP

#include "DefinedValues.hpp"

using namespace std;

class FunctionRunner;

class ProgramBody{
	public:
		unique_ptr<FunctionRunner> funcRunner;

		string envName = "ENV234$*";
		ProgramBody* parent = nullptr;

		unordered_map<string, unique_ptr<MapItem>> VMAP;

		MapItem*
		getFromVmap(const std::string& key) const{
		    const ProgramBody* currentEnv = this;

		    while (currentEnv != nullptr) {
		        auto it = currentEnv->VMAP.find(key);

		        if (it != currentEnv->VMAP.end())
		            return it->second.get();

		        currentEnv = currentEnv->parent;
		    }
		    return nullptr;
		}

		optional<unique_ptr<MapItem>>
		moveFromVmap(const std::string& key){
		    auto it = VMAP.find(key);
		    if (it == VMAP.end())
		        return std::nullopt;

		    auto result = std::move(it->second);
		    VMAP.erase(it);   // important after move

		    return result;
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
						return;
					}
					else throw InvalidSyntaxError( "No variable named " + vdata );
				}
				else throw InvalidSyntaxError( "Unknown token " + vdata );		
			}
		}
};

#endif