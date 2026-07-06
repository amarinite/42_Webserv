#pragma once
#include <vector>
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

class Lexer
{
	private:		
		std::string			_input;
		std::size_t			_cursor;
 		int					_line;

		void				skipWhitespaceAndComments();
		Token 				readWord();
		char				current() const;
		char				advance();
		bool				isAtEnd() const;
		std::vector<Token>	run();

	public:
		Lexer(const std::string& input);
		static std::vector<Token>	tokenize(const std::string& input);

};