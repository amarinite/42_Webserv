#include "TestUtils.hpp"
#include "ParseConfig.hpp"
#include "ConfigValidator.hpp"
#include "ConfigException.hpp"
#include "Lexer.hpp"
#include <iostream>

// parse helper
static Node* parseInput(const std::string& input)
{
	std::vector<Token> tokens = Lexer::tokenize(input);
	ParseConfig parser(tokens);
	return parser.parse();
}

// throw checker helper
static bool validateThrows(const std::string& input)
{
	Node *tree = parseInput(input);
	bool threw = false;
	try {
		ConfigValidator::validate(tree);
	} catch (const ConfigException&) {
		threw = true;
	}
	delete tree;
	return threw;
}

// happy paths
static bool testValidMinimalConfigDoesNotThrow()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	location / {\n"
		"		root /var/www;\n"
		"	}\n"
		"}\n";
	ASSERT(!validateThrows(input));
	return true;
}

static bool testValidFullConfigDoesNotThrow()
{
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /var/www;\n"
		"	index index.html;\n"
		"	client_max_body_size 1000;\n"
		"	error_page 404 /errors/404.html;\n"
		"	location / {\n"
		"		root /var/www;\n"
		"		allowed_methods GET POST;\n"
		"		autoindex on;\n"
		"		cgi_extension .py /usr/bin/python3;\n"
		"	}\n"
		"}\n";
	ASSERT(!validateThrows(input));
	return true;
}

// ============ isNameAllowed ============

static bool testUnknownBlockThrows()
{
	std::string input = "server { listen 80; root /a; foo {} }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testUnknownDirectiveThrows()
{
	std::string input = "server { listen 80; root /a; bogus_directive x; }\n";
	ASSERT(validateThrows(input));
	return true;
}

// ============ isContextCorrect ============

static bool testLocationOutsideServerThrows()
{
	std::string input = "location / { root /a; }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testDirectiveOutsideServerThrows()
{
	std::string input = "listen 80;\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testServerInsideServerThrows()
{
	std::string input =
		"server {\n"
		"	listen 80;\n root /a;\n"
		"	server { listen 81; }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testServerInsideLocationThrows()
{
	std::string input =
		"server {\n"
		"	listen 80;\n root /a;\n"
		"	location / {\n"
		"		root /a;\n"
		"		server { listen 81; }\n"
		"	}\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

// ============ isBlockEmpty ============

static bool testEmptyServerBlockThrows()
{
	std::string input = "server {}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testEmptyLocationBlockThrows()
{
	std::string input =
		"server {\n listen 80;\n root /a;\n location / {}\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

// ============ hasNoDuplicateDirectives ============

static bool testDuplicateNonRepeatableDirectiveThrows()
{
	// root is not repeatable
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /a;\n"
		"	root /b;\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testDuplicateRepeatableDirectiveDoesNotThrow()
{
	// error_page is repeatable
	std::string input =
		"server {\n"
		"	listen 80;\n"
		"	root /a;\n"
		"	error_page 404 /404.html;\n"
		"	error_page 500 /500.html;\n"
		"}\n";
	ASSERT(!validateThrows(input));
	return true;
}

// ============ isListenSet / isRootSet / hasNoArgs ============

static bool testMissingListenThrows()
{
	std::string input = "server {\n root /a;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testMissingRootThrows()
{
	std::string input = "server {\n listen 80;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testServerWithArgsThrows()
{
	std::string input = "server extra_arg {\n listen 80;\n root /a;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

// ============ hasOneRoutePath ============

static bool testLocationMissingPathThrows()
{
	// location with 0 args never reaches '{' cleanly via parser,
	// but a location with 2 args should be caught by the validator
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / extra {\n root /a;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testLocationInvalidPathThrows()
{
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location \"bad path\" {\n root /a;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

// ============ hasDuplicateListenAddr ============

static bool testDuplicateListenSameServerThrows()
{
	std::string input =
		"server {\n listen 80;\n listen 80;\n root /a;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testDuplicateListenAcrossServersThrows()
{
	// seenAddrs is shared across the whole validate() call
	std::string input =
		"server {\n listen 80;\n root /a;\n }\n"
		"server {\n listen 80;\n root /b;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testDifferentListenAddrsDoNotThrow()
{
	std::string input =
		"server {\n listen 80;\n root /a;\n }\n"
		"server {\n listen 8080;\n root /b;\n }\n";
	ASSERT(!validateThrows(input));
	return true;
}

// ============ per-directive validators ============

static bool testListenInvalidFormatThrows()
{
	std::string input = "server {\n listen bad:::1;\n root /a;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testListenPortOutOfRangeThrows()
{
	std::string input = "server {\n listen 999999;\n root /a;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testErrorPageWrongArgCountThrows()
{
	std::string input = "server {\n listen 80;\n root /a;\n error_page 404;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testErrorPageInvalidCodeThrows()
{
	std::string input = "server {\n listen 80;\n root /a;\n error_page 999 /404.html;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testErrorPageInvalidPathThrows()
{
	// path doesn't start with '/' -- catches bug #1 (args[1] vs args[0])
	std::string input = "server {\n listen 80;\n root /a;\n error_page 404 not-a-path;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testClientMaxBodySizeInvalidThrows()
{
	std::string input = "server {\n listen 80;\n root /a;\n client_max_body_size abc;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testRootInvalidFirstCharThrows()
{
	std::string input = "server {\n listen 80;\n root not-a-path;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testAllowedMethodsInvalidMethodThrows()
{
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / {\n root /a;\n allowed_methods GET PATCH;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testAllowedMethodsTooManyArgsThrows()
{
	// rule is maxArgs 3; GET POST DELETE is exactly 3, a 4th should fail
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / {\n root /a;\n allowed_methods GET POST DELETE GET;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testRedirectInvalidUriThrows()
{
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / {\n root /a;\n redirect \"bad uri\";\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testAutoindexInvalidValueThrows()
{
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / {\n root /a;\n autoindex maybe;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testUploadStoreInvalidThrows()
{
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / {\n root /a;\n upload_store not-a-path;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testCgiExtensionInvalidExtensionThrows()
{
	// missing leading '.' on extension
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / {\n root /a;\n cgi_extension py /usr/bin/python3;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testCgiExtensionInvalidPathThrows()
{
	// valid extension, but path doesn't start with '/', '.', or '~' --
	// this is the case bug #2 currently lets slip through silently
	std::string input =
		"server {\n listen 80;\n root /a;\n"
		"	location / {\n root /a;\n cgi_extension .py not-a-path;\n }\n"
		"}\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testTooFewArgsThrows()
{
	std::string input = "server {\n listen 80;\n root /a;\n client_max_body_size;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

static bool testTooManyArgsThrows()
{
	std::string input = "server {\n listen 80;\n root /a b;\n }\n";
	ASSERT(validateThrows(input));
	return true;
}

// ============ runner ============

void runConfigValidatorTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "valid minimal config",               testValidMinimalConfigDoesNotThrow },
		{ "valid full config",                  testValidFullConfigDoesNotThrow },
		{ "unknown block throws",               testUnknownBlockThrows },
		{ "unknown directive throws",           testUnknownDirectiveThrows },
		{ "location outside server throws",     testLocationOutsideServerThrows },
		{ "directive outside server throws",    testDirectiveOutsideServerThrows },
		{ "server inside server throws",        testServerInsideServerThrows },
		{ "server inside location throws",      testServerInsideLocationThrows },
		{ "empty server block throws",          testEmptyServerBlockThrows },
		{ "empty location block throws",        testEmptyLocationBlockThrows },
		{ "duplicate non-repeatable throws",    testDuplicateNonRepeatableDirectiveThrows },
		{ "duplicate repeatable ok",            testDuplicateRepeatableDirectiveDoesNotThrow },
		{ "missing listen throws",              testMissingListenThrows },
		{ "missing root throws",                testMissingRootThrows },
		{ "server with args throws",            testServerWithArgsThrows },
		{ "location missing path throws",       testLocationMissingPathThrows },
		{ "location invalid path throws",       testLocationInvalidPathThrows },
		{ "duplicate listen same server",       testDuplicateListenSameServerThrows },
		{ "duplicate listen across servers",    testDuplicateListenAcrossServersThrows },
		{ "different listen addrs ok",          testDifferentListenAddrsDoNotThrow },
		{ "listen invalid format throws",       testListenInvalidFormatThrows },
		{ "listen port out of range throws",    testListenPortOutOfRangeThrows },
		{ "error_page wrong arg count throws",  testErrorPageWrongArgCountThrows },
		{ "error_page invalid code throws",     testErrorPageInvalidCodeThrows },
		{ "error_page invalid path throws",     testErrorPageInvalidPathThrows },
		{ "client_max_body_size invalid",       testClientMaxBodySizeInvalidThrows },
		{ "root invalid first char throws",     testRootInvalidFirstCharThrows },
		{ "allowed_methods invalid throws",     testAllowedMethodsInvalidMethodThrows },
		{ "allowed_methods too many throws",    testAllowedMethodsTooManyArgsThrows },
		{ "redirect invalid uri throws",        testRedirectInvalidUriThrows },
		{ "autoindex invalid value throws",     testAutoindexInvalidValueThrows },
		{ "upload_store invalid throws",        testUploadStoreInvalidThrows },
		{ "cgi_extension invalid ext throws",   testCgiExtensionInvalidExtensionThrows },
		{ "cgi_extension invalid path throws",  testCgiExtensionInvalidPathThrows },
		{ "too few args throws",                testTooFewArgsThrows },
		{ "too many args throws",               testTooManyArgsThrows },
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
	printTestSummary("ConfigValidator", localPassed, localFailed);
	passed += localPassed;
	failed += localFailed;
}