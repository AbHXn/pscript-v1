#ifndef VMAP_HPP
#define VMAP_HPP

std::vector<std::unique_ptr<MapItem>> propHolderTemp;

class VAR_VMAP{
	private:
		std::queue<std::unique_ptr<MapItem>> funcReturnedCache;
		size_t cacheElement = 0;

	public:
		std::string runnerBody = "__xmain__";
		VAR_VMAP* parent = nullptr;
		
		std::unordered_map<std::string, std::unique_ptr<MapItem>> VMAP;
		std::unordered_map<std::string, MapItem*> VMAP_COPY;

		std::unordered_map<std::string, MapItem*>
		getDeepCopyOfVMAP( void ){
			std::unordered_map<std::string, MapItem*> copyData;
			VAR_VMAP* currentDir = this;
			
			while( currentDir ){
				for( auto& value: currentDir->VMAP )
					copyData[value.first] = value.second.get();
				
				for( auto value: currentDir->VMAP_COPY )
					copyData[value.first] = value.second;
				currentDir = currentDir->parent;
			}
			return copyData;
		}

		std::pair<MapItem*, VAR_VMAP*>
		getFromVmapCopy(const std::string& key) {
		    VAR_VMAP* currentEnv = this;
 		    
 		    while (currentEnv != nullptr) {
		        auto it_2 = currentEnv->VMAP_COPY.find(key);
		        if( it_2 != currentEnv->VMAP_COPY.end() )
		        	return std::make_pair( it_2->second, currentEnv );
 		        currentEnv = currentEnv->parent;
		    }
		    return std::make_pair(nullptr, nullptr);
		}

		std::pair<MapItem*, VAR_VMAP*>
		getFromVmap(const std::string& key) {
	        auto cur = this;
	        while( cur && cur->runnerBody == this->runnerBody ){
	        	auto it = cur->VMAP.find(key);
	       		if (it != cur->VMAP.end())
	            	return std::make_pair( it->second.get(), cur );
	            cur = cur->parent;
	        }
		    return getFromVmapCopy(key);
		}

		std::optional<std::unique_ptr<MapItem>>
		moveFromVmap(const std::string& key){
		    auto it = VMAP.find(key);

		    if (it == VMAP.end())
		        return std::nullopt;

		    auto result = std::move(it->second);
		    VMAP.erase(it);

		    return result;
		}

		void addToMap( std::string key, std::unique_ptr<MapItem> data ){
			this->VMAP[ key ] = std::move( data );
		}

		void pushCache( std::unique_ptr<MapItem> data ){
			this->cacheElement++;
			this->funcReturnedCache.push( std::move( data ) );
		}
};

#endif