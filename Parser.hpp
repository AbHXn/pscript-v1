#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <optional>
#include <string_view>

using namespace std;

constexpr string_view specChars = "(){}[]*+=-/%;,";

optional<vector<string>> parseTheCodeToTokens( string filename ){
	ifstream codeFile(filename);

	if( !codeFile ){
		cerr << "Failed to open file\n";
		return nullopt;
	}

	char codeChar;
	string word = "";

	bool isStrinInputing = false;
	vector<string> completeStringTokens;

	while( codeFile.get( codeChar ) ){
		if( codeChar == '"' ){
			word += codeChar;
			isStrinInputing = !isStrinInputing;

			if( !isStrinInputing && word.size() > 0 ){
				completeStringTokens.push_back( word );
				word = "";
			}
			continue;
		}

		if( !isStrinInputing ){
			if( isspace( (unsigned char) codeChar ) ){
				if( word.size() > 0 )
					completeStringTokens.push_back( word );	
				word = "";
				continue;
			}
			
			bool continueToNextToken = false;
			
			for(int x = 0; x < specChars.size(); x++){
				if( specChars[ x ] == codeChar ){
					if( !isStrinInputing && word.size() > 0 ){
						completeStringTokens.push_back( word );
						word = "";
					}

					string toStr( 1, codeChar );
					completeStringTokens.push_back( toStr );
					continueToNextToken = true;
					break;
				}
			}
			if( continueToNextToken )
				continue;
			word += codeChar;
		}
		else word += codeChar;
	}

	if( word.size() )
		completeStringTokens.push_back( word );

	codeFile.close();
	return completeStringTokens;
}
