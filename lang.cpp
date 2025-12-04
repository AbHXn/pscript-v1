#include <iostream>
#include <variant>
using namespace std;

enum class DTYPES{
	VALUE_NOT_DEFINED, STRING, INT, DOUBLE, BOOLEAN
};

class SingleElement{
	private:
		bool isValueDefined = false;
		using VarDtype = variant<string, long int, double, bool>;
		VarDtype data;
	
	public:
		DTYPES getValueDtypes( void ) const {
			if( !isValueDefined )
				return DTYPES::VALUE_NOT_DEFINED;

			else if( hold_alternative<string>( this->data ) )
				return DTYPES::STRING;

			else if( hold_alternative<long int> ( this->data ) )
				return DTYPES::INT;

			else if( hold_alternative<double> ( this->data ) )
				return DTYPES::DOUBLE;

			else return DTYPES::BOOLEAN;

		}

		void assignValue(const VarDtype& value ){
			this->data 	= value;
			this->isValueDefined = true;
		}

		template <typename Dtype>
		optional<Dtype> getValue( void ){
			DTYPES realType = this->getValueDtypes( );
			
			if( realType == VALUE_NOT_DEFINED )
				return nullopt;

			if( auto data = get_if<Dtype>( &this->data ) )
				return *data;

			return nullopt;
		}
}

template <typename Dtype>
class Array{
	private:
		size_t elementSize;
		vector<ArrayElement<Dtype>> dims;
	public:

}

template <typename dtype>
struct Variable{
	string key;
	variant<dtype, ArrayType<dtype>> data;
}