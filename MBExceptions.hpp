#ifndef MBEXCEPTIONS_HPP
#define MBEXCEPTIONS_HPP

#include <string.h>
#include <stdexcept>

class InvalidNameError: public std::runtime_error {
	public:
		explicit InvalidNameError( const std::string& varName ): 
			runtime_error( "Mariadik ulla variable thaade: " + varName ) {};
};

class VariableDeclarationMissing: public std::runtime_error{
	public:
		explicit VariableDeclarationMissing( ):
			runtime_error( "Variable illand enth property oole\n" ){};
};

class InvalidDTypeError: public std::runtime_error{
	public:
		explicit InvalidDTypeError( const std::string& msg ):
			runtime_error( "Invalid Dtype " + msg ){};
};

#endif