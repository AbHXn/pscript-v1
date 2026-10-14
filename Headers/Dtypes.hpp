#ifndef DTYPES_HPP
#define DTYPES_HPP

#include <string>
#include <variant>
#include <stdexcept>

#include "MBExceptions.hpp"

using VarDtype = std::variant<std::string, long int, double, bool, std::monostate>;

struct DtypeHelper{
	static long toLong( const std::string& value ){
		try{ return std::stol( value ); }
		catch( ... ){ throw TypeCastError( "Invalid long: " + value ); }
	}

	static double toDouble( const std::string& value ){
		try{ return std::stod( value ); }
		catch( ... ){ throw TypeCastError( "Invalid Double: " + value ); }
	}

	static bool toBoolean( const std::string& value ){
		if( value == "sheri" )  return true;
		if( value == "thettu" ) return false;
		throw TypeCastError( "Invalid BOOL: " + value );	// used to be a bare 'throw;' that killed the process
	}
};

#endif
