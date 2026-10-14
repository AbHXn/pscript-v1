#ifndef VMAP_HPP
#define VMAP_HPP

#include "VMAPData.hpp"

class VAR_VMAP{
	public:
		std::string runnerBody = "__xmain__";
		VAR_VMAP* parent = nullptr;
		VMAP_TYPE VMAP;
		// shared (read only) snapshot of the variables visible when the function was defined
		std::shared_ptr<VMAP_TYPE> VMAP_COPY;

		VMAP_TYPE
		inline getDeepCopyOfVMAP( void ){
			VMAP_TYPE copyData;
			VAR_VMAP* currentDir = this;
			
			while( currentDir ){
				for( auto& value: currentDir->VMAP )
					copyData[value.first] = value.second;

				if( currentDir->VMAP_COPY ){
					for( auto& value: *currentDir->VMAP_COPY )
						copyData[value.first] = value.second;
				}
				currentDir = currentDir->parent;
			}
			return copyData;
		}

		// returns pointer to the stored shared_ptr ( no ref-count traffic ) or nullptr
		inline const std::shared_ptr<MapItem>*
		lookupVmapCopy(const std::string& key) {
		    VAR_VMAP* currentEnv = this;

 		    while (currentEnv != nullptr) {
		    	if( currentEnv->VMAP_COPY && !currentEnv->VMAP_COPY->empty() ){
			        auto it_2 = currentEnv->VMAP_COPY->find(key);
			        if( it_2 != currentEnv->VMAP_COPY->end() )
			        	return &it_2->second;
		        }
 		        currentEnv = currentEnv->parent;
		    }
		    return nullptr;
		}

		inline const std::shared_ptr<MapItem>*
		lookup(const std::string& key) {
	        VAR_VMAP* cur = this;
	        const size_t bodyLen = this->runnerBody.size();
	      
	        while( cur && cur->runnerBody.size() == bodyLen && cur->runnerBody == this->runnerBody ){
	        	if( !cur->VMAP.empty() ){
		        	auto it = cur->VMAP.find(key);
		       		if (it != cur->VMAP.end())
		            	return &it->second;
	            }
	            cur = cur->parent;
	        }
		    return lookupVmapCopy(key);
		}

		std::shared_ptr<MapItem>
		inline getFromVmap(const std::string& key) {
			auto p = lookup( key );
			return p ? *p : nullptr;
		}

		inline void addToMap( const std::string& key, std::shared_ptr<MapItem> data ){
			this->VMAP[ key ] = std::move( data );
		}
};

#endif
