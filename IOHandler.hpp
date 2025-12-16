#include <vector>
#include <string>
#include <queue>
#include <iostream>

using namespace std;

enum class IOTOKENS{ DISPLAY, CONCAT, INPUT, END };

class Output{
	public:
		pair<vector<IOTOKENS>, queue<string>>
		stringToTokens( vector<string>& tokens, size_t startIndex ){
			vector<IOTOKENS> resultTokens;
			queue<string> outputContent;

			for( ; startIndex < tokens.size(); startIndex++ ){
			 	string curTokens = tokens[ startIndex ];

			 	if( curTokens == "para" )
			 		resultTokens.push_back( IOTOKENS::DISPLAY );
			 	
			 	else if( curTokens == "koode" )
					resultTokens.push_back( IOTOKENS::CONCAT ); 
			 	
			 	else if( curTokens == ";" ){
			 		resultTokens.push_back( IOTOKENS::END );
			 		return { resultTokens, outputContent }
			 	}
			 	
			 	else {
			 		// output content can be tempVal, hashValu, expr, condi
			 		outputContent.push( curTokens );
			 	}
			}
			cerr << "Forget ; ? may be a chance for invalid output\n";
			return { resultTokens, outputContent };
		}
		
		static void 
		runTheIOTOKENS( vector<IOTOKENS>& tokens, queue<string>& outputContents ){
			if( !outputContents.size() || !tokens.size() )
				return;
			if( tokens.back() != IOTOKENS::END ){
				// raise error (forget ;)
				return;
			}
			bool firstDisplay = true;
			for(int x = 0; x < tokens.size(); x++){
				// if no more contents to display then break
				if( outputContents.empty() )
					break;
				string outputContent = outputContents.front();
				outputContents.pop();
				if( tokens[ x ] == IOTOKENS::DISPLAY ){
					if( firstDisplay )
						firstDisplay = false;
					else cout << '\n';
					cout << outputContent;
				}
				else if( tokens[ x ] == IOTOKENS::CONCAT ){
					cout << ' ' << outputContent; 
				}
			}
			cout << '\n';
		}
};