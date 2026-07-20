#include "TestUtils.hpp"
#include "Lexer.hpp"
#include "ServerConfig.hpp"
#include "ParseConfig.hpp"
#include "UriParser.hpp"
#include <iostream>

// Helper that parses a raw string and returns the first location block Node found
static Node* getFirstLocationNode(const std::string& configSnippet)
{
	std::vector<Token> tokens = Lexer::tokenize(configSnippet);
	ParseConfig parser(tokens);
	
	Node* root = parser.parse(); 
	
	std::vector<Node*> servers = getChildrenByType(root, NODE_BLOCK, "server");
	if (servers.empty()) {
		delete root;
		return NULL;
	}
	
	std::vector<Node*> locations = getChildrenByType(servers[0], NODE_BLOCK, "location");
	if (locations.empty()) {
		delete root;
		return NULL;
	}

	Node* targetLocation = locations[0];
	
	for (size_t i = 0; i < servers[0]->children.size(); ++i) {
		if (servers[0]->children[i] == targetLocation) {
			servers[0]->children.erase(servers[0]->children.begin() + i);
			break;
		}
	}
	
	delete root; 
	return targetLocation;
}

static bool testLocationConfigDirectives()
{
	ServerConfig dummyParent;

	std::string configInput = 
		"server {\n"
		"    location /downloads {\n"
		"        root /var/www;\n"
		"        index main.html;\n"
		"        allowed_methods POST GET;\n"
		"        autoindex on;\n"
		"    }\n"
		"}\n";

	Node* locationNode = getFirstLocationNode(configInput);
	ASSERT(locationNode != NULL);

	LocationConfig loc = LocationConfig::build(locationNode, dummyParent);
	
	delete locationNode;

	ASSERT(loc.getPath().path == "/downloads");
	ASSERT(loc.getRoot() == "/var/www");
	ASSERT(loc.getIndex()[0] == "main.html");
	ASSERT(loc.hasAutoindex() == true);
	
	ASSERT(loc.getAllowedMethods().size() == 2); 
	ASSERT(loc.getAllowedMethods()[0] == "POST");
	ASSERT(loc.getAllowedMethods()[1] == "GET");

	return true;
}

static bool testLocationConfigBuildDefault()
{
	// 1. Set up a real parent ServerConfig using parsing pipeline

	std::string serverConfigInput = 
		"server {\n"
		"    root /var/www/default_fallback;\n"
		"    index welcome.html;\n"
		"}\n";

	std::vector<Token> serverTokens = Lexer::tokenize(serverConfigInput);
	ParseConfig serverParser(serverTokens);
	Node* serverRootAST = serverParser.parse();
	
	std::vector<Node*> serverNodes = getChildrenByType(serverRootAST, NODE_BLOCK, "server");
	ASSERT(!serverNodes.empty());
	
	ServerConfig parentServer = ServerConfig::build(serverNodes[0]);
	delete serverRootAST;

	// 2. Execute the static factory method under test
	LocationConfig loc = LocationConfig::buildDefault(parentServer);

	// 3. Assertions
	ASSERT(loc.getPath().path == "/");
	
	ASSERT(loc.getRoot() == "/var/www/default_fallback"); 
	ASSERT(loc.getIndex()[0] == "welcome.html");
	
	ASSERT(loc.getAllowedMethods().size() == 1);
	ASSERT(loc.getAllowedMethods()[0] == "GET");

	return true;
}

static bool testLocationConfigInheritance()
{

	std::string serverConfigInput =
		"server {\n"
		"    root /var/www/main_company;\n"
		"    index home.html;\n"
		"}\n";

	std::vector<Token> serverTokens = Lexer::tokenize(serverConfigInput);
	ParseConfig serverParser(serverTokens);
	Node* serverRootAST = serverParser.parse();

	std::vector<Node*> serverNodes = getChildrenByType(serverRootAST, NODE_BLOCK, "server");
	ASSERT(!serverNodes.empty());

	ServerConfig parentServer = ServerConfig::build(serverNodes[0]);
	delete serverRootAST;

	std::string locationConfigInput =
		"server {\n"
		"    location /api {\n"
		"        allowed_methods POST;\n"
		"    }\n"
		"}\n";

	Node* locationNode = getFirstLocationNode(locationConfigInput);
	ASSERT(locationNode != NULL);

	LocationConfig loc = LocationConfig::build(locationNode, parentServer);
	delete locationNode;

	ASSERT(loc.getPath().path == "/api");
	ASSERT(loc.getRoot() == "/var/www/main_company");
	ASSERT(loc.getIndex()[0] == "home.html");
	ASSERT(loc.getAllowedMethods().size() == 1);
	ASSERT(loc.getAllowedMethods()[0] == "POST");

	return true;
}

static bool testLocationConfigBooleans()
{
	ServerConfig dummyParent;

	// --- Scenario 1: "Not Set" Case (Defaults) ---
	std::string configInputOff = 
		"server {\n"
		"    location / {\n"
		"        allowed_methods GET;\n"
		"    }\n"
		"}\n";

	Node* nodeOff = getFirstLocationNode(configInputOff);
	ASSERT(nodeOff != NULL);

	LocationConfig locOff = LocationConfig::build(nodeOff, dummyParent);
	delete nodeOff;

	ASSERT(locOff.hasAutoindex() == false);
	ASSERT(locOff.hasUploadEnabled() == false);
	ASSERT(locOff.hasCgi() == false);

	// --- Scenario 2: "Set" Case (Explicitly Enabled) ---
	std::string configInputOn = 
		"server {\n"
		"    location /cgi-bin {\n"
		"        autoindex on;\n"
		"        upload_store /var/www/uploads;\n"
		"        cgi_extension .py /usr/bin/python3;\n"
		"    }\n"
		"}\n";

	Node* nodeOn = getFirstLocationNode(configInputOn);
	ASSERT(nodeOn != NULL);

	LocationConfig locOn = LocationConfig::build(nodeOn, dummyParent);
	delete nodeOn;

	ASSERT(locOn.hasAutoindex() == true);
	ASSERT(locOn.hasUploadEnabled() == true);
	ASSERT(locOn.hasCgi() == true);

	return true;
}

void runLocationConfigTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "a normal location with each directive set",	testLocationConfigDirectives },
		{ "location inherits root and index from server parent", testLocationConfigInheritance },
		{ "default location / is created when none present", testLocationConfigBuildDefault },
		{ "booleans for autoindex, cgi, upload, are correct", testLocationConfigBooleans }

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

	printTestSummary("LocationConfig", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}
