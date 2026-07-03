#include "TestUtils.hpp"
#include "Lexer.hpp"
#include <iostream>

bool testEmptyInputProducesEOF()
{
	std::vector<Token> tokens = Lexer::tokenize("");

	ASSERT(tokens.size() == 1);
	ASSERT(tokens[0].type == TOKEN_EOF);
	return true;
}

static bool testSimpleServerBlock()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	server_name example.com;\n"
		"}\n";

	std::vector<Token> tokens = Lexer::tokenize(input);

	// server {
	ASSERT(tokens[0].type == TOKEN_WORD);
	ASSERT(tokens[0].value == "server");
	ASSERT(tokens[1].type == TOKEN_LBRACE);

	// listen 80;
	ASSERT(tokens[2].type == TOKEN_WORD);
	ASSERT(tokens[2].value == "listen");
	ASSERT(tokens[3].type == TOKEN_WORD);
	ASSERT(tokens[3].value == "80");
	ASSERT(tokens[4].type == TOKEN_SEMI);

	// server_name example.com;
	ASSERT(tokens[5].type == TOKEN_WORD);
	ASSERT(tokens[5].value == "server_name");
	ASSERT(tokens[6].type == TOKEN_WORD);
	ASSERT(tokens[6].value == "example.com");
	ASSERT(tokens[7].type == TOKEN_SEMI);

	// }
	ASSERT(tokens[8].type == TOKEN_RBRACE);

	// EOF
	ASSERT(tokens[9].type == TOKEN_EOF);

	// nothing extra
	ASSERT(tokens.size() == 10);

	return true;
}

static bool testCommentsAreIgnored()
{
	std::string input =
		"# this is a comment\n"
		"server {\n"
		"	listen 80; # inline comment\n"
		"}\n";

	std::vector<Token> tokens = Lexer::tokenize(input);

	// # this is a comment  ->  nothing
	// server {
	ASSERT(tokens[0].type == TOKEN_WORD);
	ASSERT(tokens[0].value == "server");
	ASSERT(tokens[1].type == TOKEN_LBRACE);

	// listen 80;  (# inline comment  ->  nothing)
	ASSERT(tokens[2].type == TOKEN_WORD);
	ASSERT(tokens[2].value == "listen");
	ASSERT(tokens[3].type == TOKEN_WORD);
	ASSERT(tokens[3].value == "80");
	ASSERT(tokens[4].type == TOKEN_SEMI);

	// }
	ASSERT(tokens[5].type == TOKEN_RBRACE);

	// EOF
	ASSERT(tokens[6].type == TOKEN_EOF);

	// nothing extra
	ASSERT(tokens.size() == 7);

	return true;
}

static bool testLineNumbers()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	server_name example.com;\n"
		"}\n";

	std::vector<Token> tokens = Lexer::tokenize(input);

	// server {  ->  line 1
	ASSERT(tokens[0].line == 1);
	ASSERT(tokens[1].line == 1);

	// listen 80;  ->  line 2
	ASSERT(tokens[2].line == 2);
	ASSERT(tokens[3].line == 2);
	ASSERT(tokens[4].line == 2);

	// server_name example.com;  ->  line 3
	ASSERT(tokens[5].line == 3);
	ASSERT(tokens[6].line == 3);
	ASSERT(tokens[7].line == 3);

	// }  ->  line 4
	ASSERT(tokens[8].line == 4);

	return true;
}

void runLexerTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "empty input produces EOF",	testEmptyInputProducesEOF },
		{ "simple server block",		testSimpleServerBlock },
		{ "comments are ignored",		testCommentsAreIgnored },
		{ "line numbers",				testLineNumbers },
	};

	int localPassed = 0;
	int localFailed = 0;

	for (int i = 0; i < (int)ARRAY_SIZE(tests); i++)
	{
		if (tests[i].fn()) {
			localPassed++;
		} else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}

	printTestSummary("Lexer", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}
