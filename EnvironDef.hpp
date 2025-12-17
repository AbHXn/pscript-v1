#ifndef UNIVERSALTYPES_HPP
#define UNIVERSALTYPES_HPP

#include <string>
#include <optional>

enum class ENV_TYPE{
	TYPE_LOOP 	 ,
	TYPE_FUNCTION,
	TYPE_CLASS
};


template <typename VmapDtype>
class Environment{
	private:
		std::string 	envName;
		Environment* 	parent;
		ENV_TYPE 		envType;
		unordered_map<string, VmapDtype> VMAP;
	public:
		void pushToVMAP( const std::string& key, VmapDtype& mapItem ){
			this->VMAP[ key ] = mapItem;
		}
		ENV_TYPE getEnvType( void ) const {
			return this->envType;
		}
		optional<VmapDtype> getFromVmap( const std::string key ){
			Environment* currentEnv = this;
			while( currentEnv != nullptr ){
				auto VMAPData = this->VMAP.find( key );
				if( VMAPData != VMAP.end() )
					return VMAPData->second;
				currentEnv = currentEnv->parent;
			}
			return nullopt;
		}
		void debug_vmap_print_keys( void ) const{
			for(auto data: this->VMAP)
				cout << data.first << '\n';
			cout << endl;
		}
};

#endif