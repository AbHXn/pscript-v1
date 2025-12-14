#ifndef DTYPES_HPP
#define DTYPES_HPP

using VarDtype = variant<string, long int, double, bool>;

enum class DTYPES{
	VALUE_NOT_DEFINED, 
	STRING 			 , 
	INT 			 , 
	DOUBLE 			 , 
	BOOLEAN
};

class DtypeHelper{
	public:
		static bool isValidVariableName(const string& name){
			if (name.empty())
				return false;
			
			if (!isalpha(name[0]) && name[0] != '_')
				return false;

		    for (size_t i = 1; i < name.size(); ++i)
				if (!isalnum(name[i]) && name[i] != '_')
					return false;
		    return true;
		}

		static pair<DTYPES, VarDtype> getTypeAndValue(const string& token){
		    if (!token.empty() && token.front() == '"'){
		        if (token.back() == '"' && token.size() >= 2)
		            return { DTYPES::STRING, token.substr(1, token.size() - 2) };
		        throw InvalidDTypeError("unterminated string literal");
		    }
		    if (token == "true")
		        return { DTYPES::BOOLEAN, true };
		    if (token == "false")
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
		    catch (const out_of_range&){
		        throw InvalidDTypeError("number overflow: " + token);
		    }
		    catch (const invalid_argument&){
		        throw InvalidDTypeError("invalid number: " + token);
		    }
		}
};

#endif