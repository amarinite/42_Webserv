#include "TestUtils.hpp"
#include "UriParser.hpp"
#include <iostream>


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef bool (*TestFn)();

struct Test
{
	const char*	name;
	TestFn		fn;
};

// Test 1: Comprobar una ruta relativa simple
static bool testRelativeSimplePath()
{
	t_uri uri;
	std::string input = "/index.html";
	// std::cout << CYAN << "/index.html" << RESET << std::endl;


	try {
		parseUri(uri, input);
	} catch (const std::exception& e) {
		ASSERT(false);
	}

	ASSERT(uri.has_scheme == false);
	ASSERT(uri.has_auth == false);
	ASSERT(uri.has_path == true);
	ASSERT(uri.path == "/index.html");
	ASSERT(uri.has_query == false);
	ASSERT(uri.has_frag == false);
	return true;
}

// Test 2: URI absoluta con esquema http válido y puerto numérico
static bool testAbsoluteUriWithPort()
{
	t_uri uri;
	std::string input = "http://localhost:8080/path/to/resource";
	// std::cout << CYAN << "http://localhost:8080/path/to/resource" << RESET << std::endl;


	try {
		parseUri(uri, input);
	} catch (const std::exception& e) {
		ASSERT(false);
	}

	ASSERT(uri.has_scheme == true);
	ASSERT(uri.scheme == "http");
	ASSERT(uri.has_auth == true);
	ASSERT(uri.authority == "localhost");
	ASSERT(uri.auth_port == 8080);
	ASSERT(uri.path == "/path/to/resource");
	return true;
}

// Test 3: Normalización de paths conflictivos (.. y .)
static bool testPathNormalization()
{
	t_uri uri;
	std::string input = "/static/images/.././icon.png";
	// std::cout << CYAN << "/static/images/.././icon.png" << RESET << std::endl;


	try {
		parseUri(uri, input);
	} catch (const std::exception& e) {
		ASSERT(false);
	}

	// /static/images/.././icon.png  debería resolverse a  /static/icon.png
	ASSERT(uri.path == "/static/icon.png");
	return true;
}

// Test 4: Query y Fragmento en orden correcto e incorrecto
static bool testQueryAndFragment()
{
	t_uri uri;
	std::string input = "/search?q=42#section1";
	// std::cout << CYAN << "/search?q=42#section1" << RESET << std::endl;


	try {
		parseUri(uri, input);
	} catch (const std::exception& e) {
		ASSERT(false);
	}

	ASSERT(uri.has_query == true);
	ASSERT(uri.query == "q=42");
	ASSERT(uri.has_frag == true);
	ASSERT(uri.fragment == "section1");
	return true;
}

// Test 5: Debe lanzar 400 Bad Request si el puerto tiene letras o desborda
static bool testInvalidPortThrows()
{
	t_uri uri;
	std::string input = "http://localhost:80a80/index.html";
	// std::cout << CYAN << "http://localhost:80a80/index.html" << RESET << std::endl;

	bool threw = false;

	try {
		parseUri(uri, input);
	} catch (const std::exception& e) {
		threw = true; 
	}
	ASSERT(threw == true);
	return true;
}

// Test 6: Debe lanzar 400 Bad Request ante esquemas no soportados (ej. ftp)
static bool testUnsupportedSchemeThrows()
{
	t_uri uri;
	std::string input = "ftp://localhost/file.txt";
	// std::cout << CYAN << "ftp://localhost/file.txt" << RESET << std::endl;


	bool threw = false;

	try {
		parseUri(uri, input);
	} catch (const std::exception& e) {
		threw = true;
	}

	ASSERT(threw == true);
	return true;
}

// Test 7: Caracteres prohibidos en el Path (como espacios directos sin codificar)
static bool testForbiddenCharactersInPath()
{
	t_uri uri;
	std::string input = "/bad path/index.html";
	// std::cout << CYAN << "/bad path/index.html" << RESET << std::endl;
	bool threw = false;

	try {
		parseUri(uri, input);
	} catch (const std::exception& e) {
		threw = true;
	}

	ASSERT(threw == true);
	return true;
}

void runUriTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "relative simple path parsing",      testRelativeSimplePath },
		{ "absolute URI with scheme & port",   testAbsoluteUriWithPort },
		{ "path dot-dot normalization",        testPathNormalization },
		{ "query and fragment extraction",     testQueryAndFragment },
		{ "invalid port format throws 400",    testInvalidPortThrows },
		{ "unsupported scheme throws 400",     testUnsupportedSchemeThrows },
		{ "forbidden characters throw 400",    testForbiddenCharactersInPath },
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

	printTestSummary("UriParser", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}