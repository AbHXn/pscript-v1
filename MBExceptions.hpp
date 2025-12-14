#ifndef MBEXCEPTIONS_HPP
#define MBEXCEPTIONS_HPP

#include <string.h>
#include <stdexcept>

using namespace std;

class InvalidNameError: public runtime_error {
	public:
		explicit InvalidNameError( const string& varName ): 
			runtime_error( "Mariadik ulla variable thaade: " + varName ) {};
};

class VariableDeclarationMissing: public runtime_error{
	public:
		explicit VariableDeclarationMissing( ):
			runtime_error( "Variable illand enth property oole\n" ){};
};

class InvalidDTypeError: public runtime_error{
	public:
		explicit InvalidDTypeError( const string& msg ):
			runtime_error( "Invalid Dtype " + msg ){};
};

#endif