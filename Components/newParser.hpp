#ifndef NEWPARSER_HPP
#define NEWPARSER_HPP

#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>

using namespace std;

bool isIdentifierStart(char c) {
	return isalpha(c) || c == '_';
}

bool isIdentifierChar(char c) {
	return isalnum(c) || c == '_';
}

bool isNumberChar(char c) {
	return isdigit(c) || c == '.';
}

vector<string>
parseTheCodeToTokens(const string& filename) {
	ifstream file(filename);

	if (!file) {
		cerr << filename << " not found\n";
		return {};
	}

	vector<string> tokens;
	string current;
	char c;

	while (file.get(c)) {

		if (isspace(c)) {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}
			continue;
		}

		if (c == '"') {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}

			string str;
			str += '"';

			while (file.get(c)) {
				if (c == '\\') {
					if (!file.get(c))
						break;

					switch (c) {
						case 'n':	str += '\n';	break;
						case 't':	str += '\t';	break;
						case '"':	str += '"';		break;
						case '\\':	str += '\\';	break;
						default:	str += c;		break;
					}
					continue;
				}

				if (c == '"') {
					str += '"';
					break;
				}

				str += c;
			}

			tokens.push_back(str);
			continue;
		}

		if (isIdentifierStart(c)) {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}

			current += c;
			while (file.peek() != EOF && isIdentifierChar(file.peek()))
				current += file.get();

			tokens.push_back(current);
			current.clear();
			continue;
		}

		if (isdigit(c)) {
			if (!current.empty()) {
				tokens.push_back(current);
				current.clear();
			}

			current += c;
			while (file.peek() != EOF && isNumberChar(file.peek()))
				current += file.get();

			tokens.push_back(current);
			current.clear();
			continue;
		}

		if (!current.empty()) {
			tokens.push_back(current);
			current.clear();
		}

		string sym;
		sym += c;
		tokens.push_back(sym);
	}

	if (!current.empty())
		tokens.push_back(current);

	return tokens;
}

#endif