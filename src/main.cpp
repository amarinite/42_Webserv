#include <iostream>
#include "FileUtils.hpp"
#include "Lexer.hpp"
#include "ParseConfig.hpp"
#include "ConfigValidator.hpp"
#include "ConfigException.hpp"
#include "Config.hpp"

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
		std::string source = readFile(argv[1]);

		std::vector<Token> tokens = Lexer::tokenize(source);

		ParseConfig parser(tokens);
		tree = parser.parse();

		ConfigValidator::validate(tree);

		Config config = Config::build(tree);

		delete tree;

		// ServerManager manager(config);
		// manager.run();

		std::cout << "Config loaded successfully (" << config.getServers().size()
			<< " server block(s)). ServerManager not yet implemented.\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		delete tree;
		return 1;
	}

	return 0;
}