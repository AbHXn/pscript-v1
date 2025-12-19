#include <vector>
#include <string>
#include <queue>
#include <iostream>
#include <variant>
#include <utility>
#include <optional>

#include "Dtypes.hpp"
#include "MBExceptions.hpp"
#include "EnvironDef.hpp"

using namespace std;

enum class IOTOKENS { DISPLAY, CONCAT, INPUT, END };

template <typename VMAPDTYPE>
struct DISPLAY_CONTENT {
    bool IS_VMAP_ITEM;
    std::variant<VMAPDTYPE, std::string> display;
};

class Output {
	public:
	    template <typename Vtype>
	    optional<pair<vector<IOTOKENS>, queue<DISPLAY_CONTENT<Vtype>>>>
	    stringToTokens( vector<string>& tokens, size_t& startIndex, Environment<Vtype>& env) {
	        try	{
		        vector<IOTOKENS> resultTokens;
		        queue<DISPLAY_CONTENT<Vtype>> outputContent;

		        for (; startIndex < tokens.size(); startIndex++) {
		            string curTokens = tokens[startIndex];
		            if (curTokens == "para")
		                resultTokens.push_back(IOTOKENS::DISPLAY);
		            else if (curTokens == "koode")
		                resultTokens.push_back(IOTOKENS::CONCAT);
		            else if (curTokens == ";") {
		                resultTokens.push_back(IOTOKENS::END);
		                return make_pair(resultTokens, outputContent);
		            }
		            else {
		                try {
		                    DtypeHelper::getTypeAndValue(curTokens);
		                    DISPLAY_CONTENT<Vtype> dContent;
		                    dContent.IS_VMAP_ITEM = false;
		                    dContent.display = curTokens;
		                    outputContent.push(dContent);
		                }
		                catch (InvalidDTypeError&) {
		                    auto vmapData = env.getFromVmap(curTokens);
		                    if (vmapData.has_value()) {
		                        DISPLAY_CONTENT<Vtype> dContent;
		                        dContent.IS_VMAP_ITEM = true;
		                        dContent.display = vmapData.value();
		                        outputContent.push(dContent);
		                    }
		                    else {
		                    	throw InvalidSyntaxError(" Error in output format ");
		                    }
		                }
		            }
		        }
		    }
	        catch (InvalidSyntaxError& err){
	        	cout << err.what() << endl;
	        }
	        return nullopt;
	    }
	    template <typename Vtype>
	    void runTheIOTOKENS( vector<IOTOKENS>& tokens, queue<DISPLAY_CONTENT<Vtype>>& outputContents ){
	        if (tokens.empty() || outputContents.empty()) return;
	        if (tokens.back() != IOTOKENS::END) return;

	        bool firstHit = false;
	        for (size_t i = 0; i < tokens.size(); ++i) {
	            if (outputContents.empty()) break;
	            auto content = outputContents.front();
	            outputContents.pop();
	            if (tokens[i] == IOTOKENS::DISPLAY) {
	            	if( !firstHit )
	            		firstHit = true;
	            	else cout << '\n';
	                std::visit([](auto&& value) { cout << value; }, content.display);
	            }
	            else if (tokens[i] == IOTOKENS::CONCAT) {
	            	cout <<' ';
	                std::visit([](auto&& value) { cout << value; }, content.display);
	            }
	        }
	        cout << '\n';
	    }
};

template <typename Vtype>
bool runIOHandler( vector<string>& tokens, size_t& startIndex, Environment<Vtype>& env ) {
    Output Displayer;
    auto displayContents = Displayer.stringToTokens(tokens, startIndex, env);
   	if( displayContents.has_value() ){
    	Displayer.runTheIOTOKENS(displayContents.value().first, displayContents.value().second);
   		return true;
   	}
    else return false;
}
