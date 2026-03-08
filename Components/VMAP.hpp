#ifndef VMAP_HPP
#define VMAP_HPP

vector<unique_ptr<MapItem>> propHolderTemp;

class VAR_VMAP{
	private:
		queue<unique_ptr<MapItem>> funcReturnedCache;
		size_t cacheElement = 0;

	public:
		string runnerBody = "__xmain__";
		VAR_VMAP* parent = nullptr;
		
		unordered_map< string, unique_ptr<MapItem>> VMAP;
		unordered_map<string, MapItem*> VMAP_COPY;

		unordered_map<string, MapItem*>
		getDeepCopyOfVMAP( void ){
			unordered_map<string, MapItem*> copyData;
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

		pair<MapItem*, VAR_VMAP*>
		getFromVmapCopy(const std::string& key) {
		    VAR_VMAP* currentEnv = this;
 		    
 		    while (currentEnv != nullptr) {
		        auto it_2 = currentEnv->VMAP_COPY.find(key);
		        if( it_2 != currentEnv->VMAP_COPY.end() )
		        	return make_pair( it_2->second, currentEnv );
 		        currentEnv = currentEnv->parent;
		    }
		    return make_pair(nullptr, nullptr);
		}

		pair<MapItem*, VAR_VMAP*>
		getFromVmap(const std::string& key) {
	        auto cur = this;
	        while( cur && cur->runnerBody == this->runnerBody ){
	        	auto it = cur->VMAP.find(key);
	       		if (it != cur->VMAP.end())
	            	return make_pair( it->second.get(), cur );
	            cur = cur->parent;
	        }
		    return getFromVmapCopy(key);
		}

		optional<unique_ptr<MapItem>>
		moveFromVmap(const std::string& key){
		    auto it = VMAP.find(key);

		    if (it == VMAP.end())
		        return std::nullopt;

		    auto result = std::move(it->second);
		    VMAP.erase(it);

		    if( result->mapType == MAPTYPE::FUNCTION )
		    	for( auto& test: this->VMAP )
		    		propHolderTemp.push_back( move(test.second) );
		    return result;
		}

		void addToMap( string key, unique_ptr<MapItem> data ){
			this->VMAP[ key ] = move( data );
		}

		void pushCache( unique_ptr<MapItem> data ){
			this->cacheElement++;
			this->funcReturnedCache.push( move( data ) );
		}
};

#endif