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
#include "EnvironDef.hpp"

enum class MAP_ITEM_TYPE{
	VARIABLE, 
	ENVIRON,
	FUNCTION,
};

struct MapItem{
	MAP_ITEM_TYPE mapType;
	VARIABLE_HANDLER::VARIABLE var;
};


std::ostream& operator<<(std::ostream& os, const MapItem& item)
{
    item.var.printVariable();
    return os;
}

Environment<MapItem> MN;

int main( int argc, char* argv[] ){
	if( argc != 2 ){
		cerr << "Error: Didn't specify Program File\n";
		return -1;
	}
	optional<vector<string>> optData = parseTheCodeToTokens( argv[ 1 ] );

	if( !optData.has_value() ){
		cerr << "Error: Failed To Create Tokens\n";
		return -1;
	}
	vector<string> datas = optData.value();
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
					MN.pushToVMAP( varList[x].variableName, *mapItem );
				}
			}
		}
		else if( datas[ curPointer ] == "para" ){
			runIOHandler( datas, curPointer, MN );
		}
		
		curPointer++;
	}

	return 0;
}