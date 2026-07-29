#include "TestUtils.hpp"
#include "ParseConfig.hpp"
#include "ConfigException.hpp"
#include "Lexer.hpp"
#include "CgiHandler.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpRequest.hpp"
#include <fstream>
#include <cstdio>
#include <unistd.h>

// Builds a ServerConfig with one location containing a cgi_extension directive
// pointing at realPath as the interpreter, so access() checks in canHandleCgi
// have something real to validate against.
static ServerConfig buildCgiServerConfig(const std::string &root, const std::string &interpreterPath)
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	location / {\n"
		"		root " + root + ";\n"
		"		cgi_extension .py " + interpreterPath + ";\n"
		"	}\n"
		"}\n";
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();
	ServerConfig sc = ServerConfig::build(tree->children[0]);
	delete tree;
	return sc;
}

static void writeFile(const std::string &path, const std::string &content, int mode)
{
	std::ofstream f(path.c_str());
	f << content;
	f.close();
	chmod(path.c_str(), mode);
}

static bool testCanHandleCgi_validScript()
{
	std::string root = "/tmp/webserv_cgi_test_root";
	mkdir(root.c_str(), 0755);
	writeFile(root + "/script.py", "#!/usr/bin/env python3\nprint('ok')\n", 0755);

	ServerConfig sc = buildCgiServerConfig(root, "/usr/bin/python3");
	const LocationConfig &lc = sc.getLocations()[0];

	t_uri uri;
	uri.path = "/script.py";

	ASSERT(CgiHandler::canHandleCgi(uri, lc) == true);

	remove((root + "/script.py").c_str());
	rmdir(root.c_str());
	return true;
}

static bool testCanHandleCgi_noCgiExtensionConfigured()
{
	// Location with no cgi_extension at all -> hasCgi() should be false
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	location / {\n"
		"		root /tmp;\n"
		"	}\n"
		"}\n";
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	Node *tree = parser.parse();
	ServerConfig sc = ServerConfig::build(tree->children[0]);
	delete tree;

	const LocationConfig &lc = sc.getLocations()[0];
	t_uri uri;
	uri.path = "/anything.py";

	ASSERT(CgiHandler::canHandleCgi(uri, lc) == false);
	return true;
}

static bool testCanHandleCgi_extensionMismatch()
{
	std::string root = "/tmp/webserv_cgi_test_root2";
	mkdir(root.c_str(), 0755);

	ServerConfig sc = buildCgiServerConfig(root, "/usr/bin/python3");
	const LocationConfig &lc = sc.getLocations()[0];

	t_uri uri;
	uri.path = "/index.html"; // .html not registered, only .py is

	ASSERT(CgiHandler::canHandleCgi(uri, lc) == false);

	rmdir(root.c_str());
	return true;
}

static bool testCanHandleCgi_directoryRequestReturnsFalse()
{
	std::string root = "/tmp/webserv_cgi_test_root3";
	mkdir(root.c_str(), 0755);

	ServerConfig sc = buildCgiServerConfig(root, "/usr/bin/python3");
	const LocationConfig &lc = sc.getLocations()[0];

	t_uri uri;
	uri.path = "/cgi-bin/"; // no extension -> should be treated as non-CGI

	ASSERT(CgiHandler::canHandleCgi(uri, lc) == false);

	rmdir(root.c_str());
	return true;
}

static bool testCanHandleCgi_missingScriptThrows404()
{
	std::string root = "/tmp/webserv_cgi_test_root4";
	mkdir(root.c_str(), 0755);
	// no script.py actually created on disk

	ServerConfig sc = buildCgiServerConfig(root, "/usr/bin/python3");
	const LocationConfig &lc = sc.getLocations()[0];

	t_uri uri;
	uri.path = "/script.py";

	bool threw404 = false;
	try {
		CgiHandler::canHandleCgi(uri, lc);
	} catch (const HttpException &e) {
		threw404 = (e.getStatusCode() == 404);
	}
	ASSERT(threw404);

	rmdir(root.c_str());
	return true;
}

static bool testCanHandleCgi_scriptNotExecutableThrows403()
{
	std::string root = "/tmp/webserv_cgi_test_root5";
	mkdir(root.c_str(), 0755);
	writeFile(root + "/script.py", "print('ok')\n", 0644); // no +x

	ServerConfig sc = buildCgiServerConfig(root, "/usr/bin/python3");
	const LocationConfig &lc = sc.getLocations()[0];

	t_uri uri;
	uri.path = "/script.py";

	bool threw403 = false;
	try {
		CgiHandler::canHandleCgi(uri, lc);
	} catch (const HttpException &e) {
		threw403 = (e.getStatusCode() == 403);
	}
	ASSERT(threw403);

	remove((root + "/script.py").c_str());
	rmdir(root.c_str());
	return true;
}

static bool testCanHandleCgi_missingInterpreterThrows500()
{
	std::string root = "/tmp/webserv_cgi_test_root6";
	mkdir(root.c_str(), 0755);
	writeFile(root + "/script.py", "print('ok')\n", 0755);

	ServerConfig sc = buildCgiServerConfig(root, "/usr/bin/definitely_not_a_real_interpreter");
	const LocationConfig &lc = sc.getLocations()[0];

	t_uri uri;
	uri.path = "/script.py";

	bool threw500 = false;
	try {
		CgiHandler::canHandleCgi(uri, lc);
	} catch (const HttpException &e) {
		threw500 = (e.getStatusCode() == 500);
	}
	ASSERT(threw500);

	remove((root + "/script.py").c_str());
	rmdir(root.c_str());
	return true;
}

// Builds a Request by parsing raw HTTP bytes, same path Http::handleBuffer uses.
static Request buildTestRequest(const std::string &rawHttp, size_t maxBodySize)
{
	Request req(maxBodySize);
	req.setStream(rawHttp);
	req.parseRequestHead();
	req.parseRequestBody();
	return req;
}

static bool testBuildCgiEnv_containsExpectedKeys()
{
	std::string raw =
		"GET /script.py?name=world HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"X-Custom-Header: hello\r\n"
		"\r\n";
	Request req = buildTestRequest(raw, 1024 * 1024);

	ServerConfig sc = buildCgiServerConfig("/tmp", "/usr/bin/python3");

	char **envp = CgiHandler::buildCgiEnv(req, sc, "127.0.0.1");
	ASSERT(envp != NULL);

	bool foundMethod = false, foundQuery = false, foundCustomHeader = false, foundRemoteAddr = false;
	for (size_t i = 0; envp[i] != NULL; ++i) {
		std::string entry(envp[i]);
		if (entry.find("REQUEST_METHOD=GET") == 0) foundMethod = true;
		if (entry.find("QUERY_STRING=name=world") == 0) foundQuery = true;
		if (entry.find("HTTP_X_CUSTOM_HEADER=hello") == 0) foundCustomHeader = true;
		if (entry.find("REMOTE_ADDR=127.0.0.1") == 0) foundRemoteAddr = true;
	}

	CgiHandler::freeCgiEnv(envp);

	ASSERT(foundMethod);
	ASSERT(foundQuery);
	ASSERT(foundCustomHeader);
	ASSERT(foundRemoteAddr);
	return true;
}

static bool testBuildCgiEnv_freeDoesNotCrashOnNull()
{
	CgiHandler::freeCgiEnv(NULL); // should be a safe no-op
	return true;
}

void runCgiHandlerTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "canHandleCgi valid script",					testCanHandleCgi_validScript },
		{ "canHandleCgi no cgi_extension configured",	testCanHandleCgi_noCgiExtensionConfigured },
		{ "canHandleCgi extension mismatch",			testCanHandleCgi_extensionMismatch },
		{ "canHandleCgi directory request",				testCanHandleCgi_directoryRequestReturnsFalse },
		{ "canHandleCgi missing script -> 404",			testCanHandleCgi_missingScriptThrows404 },
		{ "canHandleCgi non-executable script -> 403",	testCanHandleCgi_scriptNotExecutableThrows403 },
		{ "canHandleCgi missing interpreter -> 500",	testCanHandleCgi_missingInterpreterThrows500 },
		{ "buildCgiEnv contains expected keys",			testBuildCgiEnv_containsExpectedKeys },
		{ "freeCgiEnv handles NULL safely",				testBuildCgiEnv_freeDoesNotCrashOnNull }
	};
	int localPassed = 0;
	int localFailed = 0;
	for (int i = 0; i < (int)ARRAY_SIZE(tests); i++) {
		if (tests[i].fn()) {
			localPassed++;
		} else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}
	printTestSummary("CgiHandler", localPassed, localFailed);
	passed += localPassed;
	failed += localFailed;
}