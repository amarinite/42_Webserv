#include "TestUtils.hpp"
#include "ParseConfig.hpp"
#include "ConfigException.hpp"
#include "Lexer.hpp"
#include <iostream>

static bool testValidSmallTree()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	location / {\n"
		"		root /var/www/html;\n"
		"		error_page 404 500 502 /error.html;\n"
		"	}\n"
		"}\n";
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();

	// synthetic root
	ASSERT(tree->type == NODE_BLOCK);
	ASSERT(tree->context == MAIN_CTXT);
	ASSERT(tree->children.size() == 1);

	// server block
	Node *server = tree->children[0];
	ASSERT(server->type == NODE_BLOCK);
	ASSERT(server->name == "server");
	ASSERT(server->context == MAIN_CTXT);
	ASSERT(server->children.size() == 2);

	// listen directive
	Node *listen = server->children[0];
	ASSERT(listen->type == NODE_DIR);
	ASSERT(listen->name == "listen");
	ASSERT(listen->context == SERVER_CTXT);
	ASSERT(listen->args.size() == 1);
	ASSERT(listen->args[0] == "80");

	// location block
	Node *location = server->children[1];
	ASSERT(location->type == NODE_BLOCK);
	ASSERT(location->name == "location");
	ASSERT(location->context == SERVER_CTXT);
	ASSERT(location->args.size() == 1);
	ASSERT(location->args[0] == "/");
	ASSERT(location->children.size() == 2);

	// root directive
	Node *root = location->children[0];
	ASSERT(root->type == NODE_DIR);
	ASSERT(root->name == "root");
	ASSERT(root->context == LOCATION_CTXT);
	ASSERT(root->args.size() == 1);
	ASSERT(root->args[0] == "/var/www/html");

	// error_page directive (multiple args)
	Node *errorPage = location->children[1];
	ASSERT(errorPage->type == NODE_DIR);
	ASSERT(errorPage->name == "error_page");
	ASSERT(errorPage->context == LOCATION_CTXT);
	ASSERT(errorPage->args.size() == 4);
	ASSERT(errorPage->args[0] == "404");
	ASSERT(errorPage->args[1] == "500");
	ASSERT(errorPage->args[2] == "502");
	ASSERT(errorPage->args[3] == "/error.html");

	delete tree;
	return true;
}

static bool testMultipleServerBlocks()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"}\n"
		"server {\n"
		"	listen 443;\n"
		"}\n";
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();

	ASSERT(tree->children.size() == 2);
	ASSERT(tree->children[0]->name == "server");
	ASSERT(tree->children[0]->children[0]->args[0] == "80");
	ASSERT(tree->children[1]->name == "server");
	ASSERT(tree->children[1]->children[0]->args[0] == "443");

	delete tree;
	return true;
}

static bool testDoubleSeparatorThrows()
{
	// '{' immediately followed by ';' with no name in between
	std::string input =
		"server {\n"
		"	;\n"
		"}\n";
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	bool threw = false;
	try {
		Node *tree = parser.parse();
		delete tree;
	} catch (const ConfigException&) {
		threw = true;
	}
	ASSERT(threw);
	return true;
}

static bool testUnclosedBraceThrows()
{
	std::string input =
		"server {\n"
		"	listen 80;\n";
		// missing closing '}'
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	bool threw = false;
	try {
		Node *tree = parser.parse();
		delete tree;
	} catch (const ConfigException&) {
		threw = true;
	}
	ASSERT(threw);
	return true;
}

static bool testMissingSemicolonThrows()
{
	std::string input =
		"server {\n"
		"	listen 80\n"   // no ';'
		"}\n";
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	bool threw = false;
	try {
		Node *tree = parser.parse();
		delete tree;
	} catch (const ConfigException&) {
		threw = true;
	}
	ASSERT(threw);
	return true;
}

void runParseConfigTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "valid small tree",			testValidSmallTree },
		{ "multiple server blocks",		testMultipleServerBlocks },
		{ "double separator throws",	testDoubleSeparatorThrows },
		{ "unclosed brace throws",		testUnclosedBraceThrows },
		{ "missing semicolon throws",	testMissingSemicolonThrows }
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
	printTestSummary("ParseConfig", localPassed, localFailed);
	passed += localPassed;
	failed += localFailed;
}