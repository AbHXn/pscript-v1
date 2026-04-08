#ifndef INPUTOUTPUT_HPP
#define INPUTOUTPUT_HPP

#include <unordered_set>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../../../Headers/MBExceptions.hpp"
#include "../../../Headers/Tokenizer.hpp"

extern std::unordered_set<std::string_view> REGISTERED_IO_TOKENS ;
enum class IO_TOKENS { PRINT, CONCAT, PRINT_VALUE, END };

bool isRegisteredIoTokens( const std::string& token );
using IOTokenReturn = std::pair<std::vector<IO_TOKENS>, std::vector<std::vector<Token>>>;
void passValidIOTokens( std::vector<IO_TOKENS>& IOTokens );
IOTokenReturn stringToIoTokens( const std::vector<Token>& tokens, size_t& startIndex );

#endif