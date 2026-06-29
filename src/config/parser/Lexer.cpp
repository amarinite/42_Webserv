#include "../../../includes/config/parser/Lexer.hpp"

Lexer::Lexer(const std::string& input)
	: _input(input), _cursor(0), _line(1)
{
	// TODO: no implementation
}

void Lexer::skipWhitespaceAndComments()
{
	// TODO: no implementation
}

Token Lexer::readWord()
{
	// TODO: no implementation
	return Token(TOKEN_EOF, "", _line);
}

char Lexer::current() const
{
	// TODO: no implementation
	return '\0';
}

char Lexer::advance()
{
	// TODO: no implementation
	return '\0';
}

bool Lexer::isAtEnd() const
{
	// TODO: no implementation
	return true;
}

std::vector<Token> Lexer::tokenize()
{
	// TODO: no implementation
	return {};
}