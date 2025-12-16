#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <optional>

using namespace std;

#include "VariableHandler.hpp"
#include "IOHandler.hpp"
#include "Parser.hpp"

enum class MAP_ITEM_TYPE{
	VARIABLE, 
	ENVIRON,
	FUNCTION,
};

struct MapItem{
	MAP_ITEM_TYPE mapType;
	VARIABLE_HANDLER::VARIABLE var;
};

class Environment{
	private:
		unordered_map<string, MapItem> VMAP;

	public:
		void pushToVMAP( string key, MapItem& mapItem ){
			this->VMAP[ key ] = mapItem;
		}
};

int main( int argc, char* argv[] ){
	if( argc != 2 ){
		cout << "program file\n";
		return -1;
	}

	optional<vector<string>> optData = parseTheCodeToTokens( argv[ 1 ] );

	if( !optData.has_value() ){
		cout << "Failed to create tokens\n";
		return -1;
	}

	vector<string> datas = optData.value();

	unique_ptr<Environment> MAIN_ENVIRON = make_unique<Environment>();
	size_t curPointer = 0;

	while( curPointer < datas.size() ){
		if( datas[ curPointer ] == "pidi" ){
			auto varTokens = VARIABLE_HANDLER::codeToTokens( datas, curPointer );
			vector<VARIABLE_HANDLER::VARIABLE> varList;

			if( isValidVariableSyntax( varTokens.first, varList )
				 && isValidValueSyntax( varTokens.second, varList ) ){
				
				for(int x = 0; x < varList.size(); x++){
					unique_ptr<MapItem> mapItem = make_unique<MapItem>();

					mapItem->mapType = MAP_ITEM_TYPE::VARIABLE;
					mapItem->var = varList[ x ];
				}
			}
		}
		else if( datas[ curPointer ] == "para" ){
			
		}

		curPointer++;
	}

	return 0;
}