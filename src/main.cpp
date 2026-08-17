#include <iostream>
#include "FileUtils.hpp"
#include "ServerManager.hpp"
#include "Lexer.hpp"
#include "ParseConfig.hpp"
#include "ConfigValidator.hpp"
#include "ConfigException.hpp"
#include "Config.hpp"
#include "Signals.hpp"

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << (argc > 0 ? argv[0] : "webserv") << " <config_file>\n";
		return 1;
	}

	Node* tree = NULL;

	try
	{
		setupSignalHandlers();
		std::string source = readFile(argv[1]);

		std::vector<Token> tokens = Lexer::tokenize(source);

		ParseConfig parser(tokens);
		tree = parser.parse();

		ConfigValidator::validate(tree);

		Config config = Config::build(tree);
		delete tree;
		tree = NULL;

		ServerManager manager(config);
		manager.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		delete tree;
		return 1;
	}

	return 0;
}
