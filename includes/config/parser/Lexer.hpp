#pragma once
#include "../config/parser/Token.hpp"

class Lexer
{
	private:
		Lexer(const std::string& input);
		
		std::string	_input;
		std::size_t	_cursor;
 		int			_line;

		void		skipWhitespaceAndComments();
		Token 		readWord();
		char		current() const;
		char		advance();
		bool		isAtEnd() const;

	public:
		std::vector<Token>	tokenize();

};