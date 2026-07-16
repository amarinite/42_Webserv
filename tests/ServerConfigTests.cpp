#include "TestUtils.hpp"
#include "Lexer.hpp"
#include "ServerConfig.hpp"
#include "ParseConfig.hpp"
#include <iostream>

// Helper that parses a raw string and returns the first server block Node found.
static Node* getFirstServerNode(const std::string& configSnippet)
{
	std::vector<Token> tokens = Lexer::tokenize(configSnippet);
	ParseConfig parser(tokens);

	Node* root = parser.parse();

	std::vector<Node*> servers = getChildrenByType(root, NODE_BLOCK, "server");
	if (servers.empty()) {
		delete root;
		return NULL;
	}

	Node* targetServer = servers[0];

	for (size_t i = 0; i < root->children.size(); ++i) {
		if (root->children[i] == targetServer) {
			root->children.erase(root->children.begin() + i);
			break;
		}
	}

	delete root;
	return targetServer;
}

// ============ basic directive dispatch ============

static bool testServerConfigDirectives()
{
	std::string input =
		"server {\n"
		"	listen 8080;\n"
		"	root /var/www;\n"
		"	index main.html;\n"
		"	client_max_body_size 2048;\n"
		"}\n";

	Node* serverNode = getFirstServerNode(input);
	ASSERT(serverNode != NULL);

	ServerConfig config = ServerConfig::build(serverNode);
	delete serverNode;

	ASSERT(config.getListen().size() == 1);
	ASSERT(config.getListen()[0].host == "0.0.0.0");
	ASSERT(config.getListen()[0].port == 8080);
	ASSERT(config.getRoot() == "/var/www");
	ASSERT(config.getIndex() == "main.html");
	ASSERT(config.getClientMaxBodySize() == 2048);

	return true;
}

// ============ defaults when directives are absent ============

static bool testServerConfigDefaults()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"}\n";

	Node* serverNode = getFirstServerNode(input);
	ASSERT(serverNode != NULL);

	ServerConfig config = ServerConfig::build(serverNode);
	delete serverNode;

	// defaults from the ServerConfig() ctor
	ASSERT(config.getIndex() == "index.html");
	ASSERT(config.getClientMaxBodySize() == 1000000);

	return true;
}

// ============ multiple listen args on one directive ============

static bool testServerConfigMultipleListenArgs()
{
	std::string input =
		"server {\n"
		"	listen 80 443 127.0.0.1:8080;\n"
		"	root /var/www;\n"
		"}\n";

	Node* serverNode = getFirstServerNode(input);
	ASSERT(serverNode != NULL);

	ServerConfig config = ServerConfig::build(serverNode);
	delete serverNode;

	ASSERT(config.getListen().size() == 3);
	ASSERT(config.getListen()[0].host == "0.0.0.0");
	ASSERT(config.getListen()[0].port == 80);
	ASSERT(config.getListen()[1].host == "0.0.0.0");
	ASSERT(config.getListen()[1].port == 443);
	ASSERT(config.getListen()[2].host == "127.0.0.1");
	ASSERT(config.getListen()[2].port == 8080);

	return true;
}

// ============ error_page map ============

static bool testServerConfigErrorPages()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	error_page 404 /errors/404.html;\n"
		"	error_page 500 /errors/500.html;\n"
		"}\n";

	Node* serverNode = getFirstServerNode(input);
	ASSERT(serverNode != NULL);

	ServerConfig config = ServerConfig::build(serverNode);
	delete serverNode;

	const std::map<int, std::string>& pages = config.getErrorPages();
	ASSERT(pages.size() == 2);

	std::map<int, std::string>::const_iterator it404 = pages.find(404);
	ASSERT(it404 != pages.end());
	ASSERT(it404->second == "/errors/404.html");

	std::map<int, std::string>::const_iterator it500 = pages.find(500);
	ASSERT(it500 != pages.end());
	ASSERT(it500->second == "/errors/500.html");

	return true;
}

// ============ explicit root location suppresses default injection ============

static bool testServerConfigExplicitRootLocationNoDefaultAdded()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	location / {\n"
		"		root /var/www/custom;\n"
		"	}\n"
		"}\n";

	Node* serverNode = getFirstServerNode(input);
	ASSERT(serverNode != NULL);

	ServerConfig config = ServerConfig::build(serverNode);
	delete serverNode;

	// only the explicit "/" location should exist -- no duplicate default
	ASSERT(config.getLocations().size() == 1);
	ASSERT(config.getLocations()[0].getPath().path == "/");
	ASSERT(config.getLocations()[0].getRoot() == "/var/www/custom");

	return true;
}

// ============ no root location -> default "/" location injected ============

static bool testServerConfigNoRootLocationInjectsDefault()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	index home.html;\n"
		"	location /uploads {\n"
		"		root /var/www/uploads;\n"
		"	}\n"
		"}\n";

	Node* serverNode = getFirstServerNode(input);
	ASSERT(serverNode != NULL);

	ServerConfig config = ServerConfig::build(serverNode);
	delete serverNode;

	ASSERT(config.getLocations().size() == 2);

	bool foundDefault = false;
	bool foundUploads = false;
	for (size_t i = 0; i < config.getLocations().size(); ++i)
	{
		const LocationConfig& loc = config.getLocations()[i];
		if (loc.getPath().path == "/")
		{
			foundDefault = true;
			ASSERT(loc.getRoot() == "/var/www");
			ASSERT(loc.getIndex() == "home.html");
		}
		if (loc.getPath().path == "/uploads")
			foundUploads = true;
	}
	ASSERT(foundDefault);
	ASSERT(foundUploads);

	return true;
}

// ============ getLocationConfig via ServerConfig directly ============

static bool testServerConfigGetLocationConfigLongestMatch()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	location / {\n"
		"		root /var/www;\n"
		"	}\n"
		"	location /api {\n"
		"		root /var/www/api;\n"
		"	}\n"
		"}\n";

	Node* serverNode = getFirstServerNode(input);
	ASSERT(serverNode != NULL);

	ServerConfig config = ServerConfig::build(serverNode);
	delete serverNode;

	t_uri requestUri;
	parseUri(requestUri, "/api/users");

	const LocationConfig& loc = config.getLocationConfig(requestUri);
	ASSERT(loc.getPath().path == "/api");
	ASSERT(loc.getRoot() == "/var/www/api");

	return true;
}

// ============ runner ============

void runServerConfigTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "a normal server with each directive set",              testServerConfigDirectives },
		{ "defaults apply when directives absent",                testServerConfigDefaults },
		{ "single listen directive with multiple args",           testServerConfigMultipleListenArgs },
		{ "error_page directives populate map correctly",         testServerConfigErrorPages },
		{ "explicit root location suppresses default injection",  testServerConfigExplicitRootLocationNoDefaultAdded },
		{ "no root location triggers default location injection", testServerConfigNoRootLocationInjectsDefault },
		{ "getLocationConfig resolves longest prefix match",      testServerConfigGetLocationConfigLongestMatch },
	};

	int localPassed = 0;
	int localFailed = 0;

	for (int i = 0; i < (int)ARRAY_SIZE(tests); i++)
	{
		bool result = false;
		try {
			result = tests[i].fn();
		} catch (const std::exception& e) {
			std::cerr << RED << "  [EXCEPTION] " << tests[i].name << ": " << e.what() << "\n" << RESET;
			result = false;
		}
		if (result) localPassed++;
		else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}

	printTestSummary("ServerConfig", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}