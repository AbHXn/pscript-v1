#ifndef VMAP_HPP
#define VMAP_HPP

class VAR_VMAP{
	public:
		std::string runnerBody = "__xmain__";
		VAR_VMAP* parent = nullptr;
		std::unordered_map<std::string, std::shared_ptr<MapItem>> VMAP, VMAP_COPY;

		std::unordered_map<std::string, std::shared_ptr<MapItem>>
		inline getDeepCopyOfVMAP( void ){
			std::unordered_map<std::string, std::shared_ptr<MapItem>> copyData;
			VAR_VMAP* currentDir = this;
			
			while( currentDir ){
				for( auto& value: currentDir->VMAP )
					copyData[value.first] = value.second;

				for( auto value: currentDir->VMAP_COPY )
					copyData[value.first] = value.second;
				currentDir = currentDir->parent;
			}
			return copyData;
		}

		std::shared_ptr<MapItem>
		inline getFromVmapCopy(const std::string& key) {
		    VAR_VMAP* currentEnv = this;

 		    while (currentEnv != nullptr) {
		        auto it_2 = currentEnv->VMAP_COPY.find(key);
		        if( it_2 != currentEnv->VMAP_COPY.end() )
		        	return it_2->second;
 		        currentEnv = currentEnv->parent;
		    }
		    return nullptr;
		}

		std::shared_ptr<MapItem>
		inline getFromVmap(const std::string& key) {
	        auto cur = this;
	      
	        while( cur && cur->runnerBody == this->runnerBody ){
	        	auto it = cur->VMAP.find(key);
	       		if (it != cur->VMAP.end())
	            	return it->second;
	            cur = cur->parent;
	        }
		    return getFromVmapCopy(key);
		}

		inline void addToMap( std::string key, std::shared_ptr<MapItem> data ){
			this->VMAP[ key ] = data;
		}
};

#endif