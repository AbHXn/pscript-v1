#ifndef DTYPES_HPP
#define DTYPES_HPP

#include <string>

using VarDtype = std::variant<std::string, long int, double, bool, std::monostate>;

class DtypeHelper{
	public:
		static long toLong(const std::string& value) {
		    try {
		    	return std::stol( value );
		    } catch (...) {
		        throw std::invalid_argument("Invalid long: " + value);
		    }
		}

		static bool toBoolean( const std::string& value ){
			if( value == "sheri" )
				return true;
			else if(value == "thettu")
				return false;
			else throw;
		}

		static double toDouble(const std::string& value){
			try{
				return std::stod( value );
			} catch( ... ){
				throw std::invalid_argument("Invalid Double: " + value);
			}
		}
};

#endif