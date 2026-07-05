#include <iostream>

void runLexerTests(int& passed, int& failed);
void runParseConfigTests(int& passed, int& failed);

int main()
{
	int passed = 0;
	int failed = 0;

	runLexerTests(passed, failed);
	runParseConfigTests(passed, failed);

	std::cout << "\n" << passed << " passed, " << failed << " failed\n";
	return (failed > 0 ? 1 : 0);
}