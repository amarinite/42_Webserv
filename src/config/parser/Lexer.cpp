#include "Lexer.hpp"

Lexer::Lexer(const std::string& input)
	: _input(input), _cursor(0), _line(1)
{
}

void Lexer::skipWhitespaceAndComments()
{
	while (true)
	{
		while (std::isspace(static_cast<unsigned char>(current())))
			advance();
		if (current() == '#')
			while (current() != '\n' && current() != '\0')
				advance();
		else
			break;
	}
}

static bool isWordChar(char c)
{
	return !std::isspace(static_cast<unsigned char>(c)) 
		&& c != '{' && c != '}' && c != ';' && c != '\0';
}

Token Lexer::readWord()
{
	std::string word;
	while (isWordChar(current()))
		word.push_back(advance());
	return Token(TOKEN_WORD, word, _line);
}

char Lexer::current() const
{
	if (isAtEnd())
		return '\0';
	return (_input[_cursor]);
}

char Lexer::advance()
{
	if (isAtEnd())
		return '\0';
	if (_input[_cursor] == '\n')
		_line++;
	return (_input[_cursor++]);
}

bool Lexer::isAtEnd() const
{
	return (_cursor >= _input.length());
}

std::vector<Token> Lexer::run()
{
	std::vector<Token> tokens;
	
	while (!isAtEnd())
	{
		skipWhitespaceAndComments();
		if (isAtEnd())
			break;
		switch (current())
		{
			case '{':
				tokens.push_back(Token(TOKEN_LBRACE, "{", _line));
				advance();
				break;
			case '}':
				tokens.push_back(Token(TOKEN_RBRACE, "}", _line));
				advance();
				break;
			case ';':
				tokens.push_back(Token(TOKEN_SEMI, ";", _line));
				advance();
				break;
			default:
				tokens.push_back(readWord());
		}
	}
	tokens.push_back(Token(TOKEN_EOF, "", _line));
	return tokens;
}

std::vector<Token> Lexer::tokenize(const std::string& input)
{
	Lexer lexer(input);
    return lexer.run();
}