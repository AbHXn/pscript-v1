#ifndef MBEXCEPTIONS_HPP
#define MBEXCEPTIONS_HPP

#include <string>
#include <stdexcept>

// every error is a runtime_error whose message starts with its own prefix
#define MB_ERROR( Name, Prefix ) \
	class Name: public std::runtime_error { \
		public: explicit Name( const std::string& msg ): std::runtime_error( Prefix + msg ) {} \
	};

MB_ERROR( VariableAlreayExists, "VariableAlreayExists: Recreation of " )
MB_ERROR( InternalError, 		"InternalError: " )
MB_ERROR( InvalidDTypeError, 	"DtypeError: " )
MB_ERROR( InvalidSyntaxError, 	"InvalidSyntax: " )
MB_ERROR( ArrayOutOfBound, 		"ArrayOutOfBound: " )
MB_ERROR( TypeCastError, 		"TypeCastError: " )

// thrown by a statement that is not valid on its own, the caller may still recover from it
class RecoverError: public std::runtime_error {
	public: RecoverError(): std::runtime_error( "RECOVER!" ) {}
};

#endif
