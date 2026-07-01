#pragma once
#include <vector>
#include <string> 
#include "Token.hpp"

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