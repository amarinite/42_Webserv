#include <iostream>

void runLexerTests(int& passed, int& failed);
void runSocketTests(int &passed, int &failed);
void runUriTests(int& passed, int& failed);

int main()
{
	int passed = 0;
	int failed = 0;

	runLexerTests(passed, failed);
	runSocketTests(passed, failed);
	runUriTests(passed, failed);

	std::cout << "\n" << passed << " passed, " << failed << " failed\n";
	return (failed > 0 ? 1 : 0);
}
