#ifndef MBEXCEPTIONS_HPP
#define MBEXCEPTIONS_HPP

#include <string.h>
#include <stdexcept>

class InvalidNameError: public std::runtime_error {
	public:
		explicit InvalidNameError( const std::string& varName ): 
			runtime_error( "InvalidNameError: " + varName ) {};
};

class VariableDeclarationMissing: public std::runtime_error{
	public:
		explicit VariableDeclarationMissing( ):
			runtime_error( "InvalidSyntax: Single variable holds more than 2 values\n" ){};
};

class InvalidDTypeError: public std::runtime_error{
	public:
		explicit InvalidDTypeError( const std::string& msg ):
			runtime_error( "DtypeError: " + msg ){};
};

class InvalidSyntaxError: public std::runtime_error{
	public:
		explicit InvalidSyntaxError( const std::string& msg ):
			runtime_error( "InvalidSyntax: " + msg ){};
};
#endif