#include "Tokenizer.hpp"
#include <unordered_map>
#include <iterator>
#include <cctype>

static const std::unordered_map<std::string, TOKEN_CONST> RESERVED = {
	{ "pindi", TOKEN_CONST::PINDI }, { "pidi", TOKEN_CONST::PIDI }, { "sheri", TOKEN_CONST::SHERI },
	{ "thettu", TOKEN_CONST::THETTU }, { "poda", TOKEN_CONST::PODA }, { "pinnava", TOKEN_CONST::PINNAVA },
	{ "theku", TOKEN_CONST::THEKU }, { "ittuthiri", TOKEN_CONST::ITTUTHIRI }, { "nok", TOKEN_CONST::NOK },
	{ "umbi", TOKEN_CONST::UMBI }, { "onnula", TOKEN_CONST::ONNULA }, { "para", TOKEN_CONST::PARA },
	{ "koode", TOKEN_CONST::KOODE }, { "um", TOKEN_CONST::UM }, { "yo", TOKEN_CONST::YO },
	{ "kootam", TOKEN_CONST::KOOTAM }, { "edukku", TOKEN_CONST::EDUKKU },
};

static inline bool isOptrChar( char c ){
	switch( c ){
		case '+': case '-': case '*': case '/': case '%': case '>': case '<': case '=': case '!': case ':': return true;
		default: return false;
	}
}

static inline bool isSpecChar( char c ){
	switch( c ){
		case '(': case ')': case '[': case ']': case '{': case '}': case ',': case '.': case ';': return true;
		default: return false;
	}
}

// a sign belongs to the number after it only where a value is expected ( start, after an operator, "(", ",", para ... )
static bool signStartsValue( const std::vector<Token>& tokens ){
	if( tokens.empty() ) return true;

	const Token& prev = tokens.back();
	switch( prev.type ){
		case TOKEN_TYPE::OPERATOR: return true;
		case TOKEN_TYPE::SPEC_CHAR: return prev.token == "(" || prev.token == "[" || prev.token == "{" || prev.token == ",";
		case TOKEN_TYPE::RESERVED:
			switch( prev.tokConst ){
				case TOKEN_CONST::PARA: case TOKEN_CONST::KOODE: case TOKEN_CONST::PODA: case TOKEN_CONST::NOK:
				case TOKEN_CONST::ITTUTHIRI: case TOKEN_CONST::UM: case TOKEN_CONST::YO: return true;
				default: return false;
			}
		default: return false;
	}
}

void getTheTokens( const std::string& filename, std::vector<Token>& Tokens ){
	std::ifstream codeFile( filename, std::ios::binary );

	if( !codeFile.is_open() )
		throw std::runtime_error( "Cannot open file: " + filename );

	const std::string src(( std::istreambuf_iterator<char>( codeFile ) ), std::istreambuf_iterator<char>());

	TOKEN_TYPE state = TOKEN_TYPE::NOTHING;
	std::string word;
	size_t row = 0, lineStart = 0, wordRow = 0, wordStart = 0;
	unsigned int tokenId = 0;
	bool comments = false, escaped = false;

	auto begin = [&]( TOKEN_TYPE type, size_t at ){
		state = type; wordRow = row; wordStart = at - lineStart;
	};
	auto flush = [&](){
		if( state == TOKEN_TYPE::NOTHING ) return;

		TOKEN_TYPE type = state;
		TOKEN_CONST tokConst = TOKEN_CONST::NOTHING;

		if( state == TOKEN_TYPE::IDENTIFIER ){
			auto it = RESERVED.find( word );
			if( it != RESERVED.end() ){
				if( word == "sheri" || word == "thettu" ) type = TOKEN_TYPE::BOOLEAN;
				else{ type = TOKEN_TYPE::RESERVED; tokConst = it->second; }
			}
		}
		Tokens.emplace_back( ++tokenId, type, std::move( word ), wordRow, wordStart );
		Tokens.back().tokConst = tokConst;
		word.clear();
		state = TOKEN_TYPE::NOTHING;
	};

	for( size_t i = 0; i < src.size(); i++ ){
		const char c = src[ i ];

		if( state == TOKEN_TYPE::STRING ){
			if( c == '"' && !escaped ){ flush(); continue; }

			escaped = ( c == '\\' ) && !escaped;
			if( c == '\n' ){ row++; lineStart = i + 1; }
			word.push_back( c );
			continue;
		}
		if( c == '?' ){
			flush();
			comments = !comments;
			continue;
		}
		if( comments ){
			if( c == '\n' ){ row++; lineStart = i + 1; }
			continue;
		}
		if( c == ' ' || c == '\t' || c == '\r' || c == '\n' ){
			flush();
			if( c == '\n' ){ row++; lineStart = i + 1; }
		}
		else if( c == '"' ){
			flush();
			begin( TOKEN_TYPE::STRING, i );
			escaped = false;
		}
		else if( isdigit( (unsigned char) c ) ){
			if( state == TOKEN_TYPE::NOTHING || state == TOKEN_TYPE::OPERATOR ){
				flush();
				begin( TOKEN_TYPE::NUMBER, i );
			}
			word.push_back( c );
		}
		else if( isalpha( (unsigned char) c ) || c == '_' ){
			if( state != TOKEN_TYPE::IDENTIFIER ){
				flush();
				begin( TOKEN_TYPE::IDENTIFIER, i );
			}
			word.push_back( c );
		}
		else if( c == '.' && state == TOKEN_TYPE::NUMBER ){
			word.push_back( c );
			state = TOKEN_TYPE::FLOATING;
		}
		else if( c == '=' && state == TOKEN_TYPE::OPERATOR && word.size() == 1 ){
			word.push_back( c );
			flush();
		}
		else if( isOptrChar( c ) ){
			flush();

			const bool isSign = c == '-' || c == '+';
			if( isSign && i + 1 < src.size() && isdigit( (unsigned char) src[ i + 1 ] ) && signStartsValue( Tokens ) )
				begin( TOKEN_TYPE::NUMBER, i );
			else
				begin( TOKEN_TYPE::OPERATOR, i );
			word.push_back( c );
		}
		else if( isSpecChar( c ) ){
			flush();
			Tokens.emplace_back( ++tokenId, TOKEN_TYPE::SPEC_CHAR, std::string( 1, c ), row, i - lineStart );
		}
		else
			throw std::runtime_error( "Invalid character '" + std::string( 1, c ) + "' at line: " + std::to_string( row + 1 ) );
	}
	if( state == TOKEN_TYPE::STRING )
		throw std::runtime_error( "Unterminated string starting at line: " + std::to_string( wordRow + 1 ) );

	flush();
}
