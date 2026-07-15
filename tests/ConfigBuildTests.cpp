#include "TestUtils.hpp"
#include "Lexer.hpp"
#include "ParseConfig.hpp"
#include "ConfigValidator.hpp"
#include "ConfigException.hpp"
#include "Config.hpp"
#include <iostream>

// Returns true if build succeeds without throwing anything.
static bool runFullPipeline(const std::string& input,
	bool& threwDuringValidate, bool& threwDuringBuild)
{
	threwDuringValidate = false;
	threwDuringBuild = false;

	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();

	try {
		ConfigValidator::validate(tree);
	}
	catch (const ConfigException&) {
		threwDuringValidate = true;
		delete tree;
		return false;
	}

	bool buildOk = true;
	try {
		Config config = Config::build(tree);
		(void)config;
	}
	catch (const std::exception&) {
		threwDuringBuild = true;
		buildOk = false;
	}

	delete tree;
	return buildOk;
}

// ============ valid config never throws in build ============

static bool testValidConfigBuildsSuccessfully()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	index index.html;\n"
		"	location / {\n"
		"		root /var/www;\n"
		"		allowed_methods GET POST;\n"
		"	}\n"
		"}\n";

	bool threwValidate, threwBuild;
	bool ok = runFullPipeline(input, threwValidate, threwBuild);
	ASSERT(ok);
	ASSERT(!threwValidate);
	ASSERT(!threwBuild);
	return true;
}

static bool testValidatedConfigNeverThrowsDuringBuild()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root ./www/site;\n"
		"	location / {\n"
		"		root ./www/site;\n"
		"	}\n"
		"}\n";

	bool threwValidate, threwBuild;
	bool ok = runFullPipeline(input, threwValidate, threwBuild);

	if (!threwValidate) {
		ASSERT(!threwBuild);
	}
	ASSERT(ok || threwValidate);
	return true;
}

static bool testInvalidConfigNeverReachesBuild()
{
	// missing listen -- should fail at validate(), never reach Config::build
	std::string input =
		"server {\n"
		"	root /var/www;\n"
		"}\n";

	bool threwValidate, threwBuild;
	bool ok = runFullPipeline(input, threwValidate, threwBuild);
	ASSERT(!ok);
	ASSERT(threwValidate);
	ASSERT(!threwBuild); // never got there
	return true;
}

// ============ multi-server end-to-end ============

static bool testMultipleServersBuildIndependently()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www/a;\n"
		"}\n"
		"server {\n"
		"	listen 8080;\n"
		"	root /var/www/b;\n"
		"}\n";

	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();

	ConfigValidator::validate(tree);
	Config config = Config::build(tree);
	delete tree;

	ASSERT(config.getServers().size() == 2);
	ASSERT(config.getServers()[0].getRoot() == "/var/www/a");
	ASSERT(config.getServers()[1].getRoot() == "/var/www/b");
	return true;
}

// ============ getServer() lookup after full build ============

static bool testGetServerFindsCorrectServerByListenAddr()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www/a;\n"
		"}\n"
		"server {\n"
		"	listen 127.0.0.1:8080;\n"
		"	root /var/www/b;\n"
		"}\n";

	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();

	ConfigValidator::validate(tree);
	Config config = Config::build(tree);
	delete tree;

	ListenAddr target;
	target.host = "127.0.0.1";
	target.port = 8080;

	const ServerConfig& found = config.getServer(target);
	ASSERT(found.getRoot() == "/var/www/b");
	return true;
}

// ============ location resolution after full build ============

static bool testGetLocationConfigResolvesLongestPrefixMatch()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	location / {\n"
		"		root /var/www;\n"
		"	}\n"
		"	location /uploads {\n"
		"		root /var/www/uploads;\n"
		"	}\n"
		"}\n";

	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();

	ConfigValidator::validate(tree);
	Config config = Config::build(tree);
	delete tree;

	ListenAddr addr;
	addr.host = "0.0.0.0";
	addr.port = 80;
	const ServerConfig& server = config.getServer(addr);

	t_uri requestUri;
	parseUri(requestUri, "/uploads/file.txt");

	const LocationConfig& loc = server.getLocationConfig(requestUri);
	ASSERT(loc.getPath().path == "/uploads");
	ASSERT(loc.getRoot() == "/var/www/uploads");
	return true;
}

static bool testGetLocationConfigFallsBackToDefaultRoot()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	location /uploads {\n"
		"		root /var/www/uploads;\n"
		"	}\n"
		"}\n";

	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();

	ConfigValidator::validate(tree);
	Config config = Config::build(tree);
	delete tree;

	ListenAddr addr;
	addr.host = "0.0.0.0";
	addr.port = 80;
	const ServerConfig& server = config.getServer(addr);

	t_uri requestUri;
	parseUri(requestUri, "/other/page.html");

	const LocationConfig& loc = server.getLocationConfig(requestUri);
	ASSERT(loc.getPath().path == "/"); // auto-generated default location
	ASSERT(loc.getRoot() == "/var/www");
	return true;
}

// ============ runner ============

void runConfigBuildTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "valid config builds successfully",              testValidConfigBuildsSuccessfully },
		{ "validated config never throws during build",    testValidatedConfigNeverThrowsDuringBuild },
		{ "invalid config never reaches build",            testInvalidConfigNeverReachesBuild },
		{ "multiple servers build independently",          testMultipleServersBuildIndependently },
		{ "getServer finds correct server by listen addr", testGetServerFindsCorrectServerByListenAddr },
		{ "getLocationConfig resolves longest prefix",     testGetLocationConfigResolvesLongestPrefixMatch },
		{ "getLocationConfig falls back to default root",  testGetLocationConfigFallsBackToDefaultRoot },
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
	printTestSummary("ConfigBuild", localPassed, localFailed);
	passed += localPassed;
	failed += localFailed;
}