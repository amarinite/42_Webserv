#pragma once
#include <string>

enum TokenType
{
	TOKEN_WORD,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_SEMI,
	TOKEN_EOF
};

struct Token
{
	TokenType	type;
	std::string	value;
	int			line;

	Token(TokenType type, const std::string& value, int line) 
		: type(type), value(value), line(line) {}
};

inline std::string tokenTypeToString(TokenType t)
{
	switch (t) {
		case TOKEN_WORD:	return "WORD";
		case TOKEN_LBRACE:	return "'{'";
		case TOKEN_RBRACE:	return "'}'";
		case TOKEN_SEMI:	return "';'";
		case TOKEN_EOF:		return "EOF";
	}
	return "UNKNOWN";
}