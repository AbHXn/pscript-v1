#ifndef DTYPES_HPP
#define DTYPES_HPP

#include <variant>
#include <string>
#include <unordered_set>

#include "MBExceptions.hpp"

using VarDtype = std::variant<std::string, long int, double, bool>;

enum class DTYPES{ VALUE_NOT_DEFINED, STRING, INT, DOUBLE, BOOLEAN, EXPR_TYPE };

static std::unordered_set<std::string> DEFINED_TOKENS = {
	"pidi", ",", "=", ";", "kootam", "*", "-", "+", "/", "%"
};

static bool isHitsDefinedTokens( const std::string& token ) {
	if( DEFINED_TOKENS.find( token ) != DEFINED_TOKENS.end() )
		return true;
	return false;
}

class DtypeHelper{
	public:
		static bool isValidVariableName(const std::string& name){
			if( name == "sheri" || name == "thettu")
				return false;

			if (name.empty()) return false;
			if (!isalpha(name[0]) && name[0] != '_') return false;
			
		    for (size_t i = 1; i < name.size(); ++i)
				if (!isalnum(name[i]) && name[i] != '_')
					return false;
		    return true;
		}

		static std::pair<DTYPES, VarDtype> getTypeAndValue(const std::string& token){
		    if (!token.empty() && token.front() == '"'){
		        if (token.back() == '"' && token.size() >= 2)
		            return { DTYPES::STRING, token.substr(1, token.size() - 2) };
		        throw InvalidDTypeError("unterminated string literal");
		    }
		    if (token == "sheri")
		        return { DTYPES::BOOLEAN, true };
		    if (token == "thettu")
		        return { DTYPES::BOOLEAN, false };
		    
		    bool hasDot = false;
		    size_t start = (token[0] == '-' ? 1 : 0);
		    if (start == token.size())
		        throw InvalidDTypeError("invalid number");
		    for (size_t i = start; i < token.size(); ++i){
		        if (token[i] == '.'){
		            if (hasDot)
		                throw InvalidDTypeError("invalid double");
		            hasDot = true;
		        }
		        else if (!isdigit(token[i])){
		            throw InvalidDTypeError("invalid character in number");
		        }
		    }
		    try{
		        if (hasDot){
		            size_t pos;
		            double val = stod(token, &pos);
		            if (pos != token.size())
		                throw InvalidDTypeError("invalid double");
		            return { DTYPES::DOUBLE, val };
		        }
		        else{
		            size_t pos;
		            long val = stol(token, &pos);
		            if (pos != token.size())
		                throw InvalidDTypeError("invalid integer");
		            return { DTYPES::INT, val };
		        }
		    }
		    catch (const std::out_of_range&){
		        throw InvalidDTypeError("number overflow: " + token);
		    }
		    catch (const std::invalid_argument&){
		        throw InvalidDTypeError("invalid number: " + token);
		    }
		}
};

#endif