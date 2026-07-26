#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Http.hpp"
#include "HttpException.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"

// ============================================================================
// MACROS & HARNESS DE PRUEBAS
// ============================================================================

#define RESET "\033[0m"
#define RED   "\033[1;31m"
#define GREEN "\033[1;32m"
#define CYAN  "\033[1;36m"

#define ASSERT(condition) \
	if (!(condition)) { \
		std::cerr << RED << "FAIL: " << #condition \
				  << " (" << __FILE__ << ":" << __LINE__ << ")\n" << RESET; \
		return false; \
	}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef bool (*TestFn)();

struct Test {
	const char*	name;
	TestFn		fn;
};

inline void printTestSummary(const char* name, int passed, int failed) {
	std::cout << GREEN << name << ": " << passed << "/" << (passed + failed) << " passed" << RESET;
	if (failed > 0)
		std::cout << RED << " (" << failed << " failed)" << RESET;
	std::cout << "\n";
}

// Instancia global o helper para proveer un ServerConfig a los tests
static ServerConfig& getTestConfig() {
	static ServerConfig config;
	static bool initialized = false;

	if (!initialized) {
		// Inyectamos la location por defecto '/' para que las peticiones
		// encuentren un LocationConfig valido y no fallen con SIGSEGV (0x0).
		config.addLocation(LocationConfig::buildDefault(config));
		initialized = true;
	}
	return config;
}

// Helper para alimentar la máquina de estados de Http
static void feed(Http& h, const std::string& s) {
	if (s.empty()) {
		h.HttpRoutine(NULL, 0);
	} else {
		std::vector<char> buff(s.begin(), s.end());
		h.HttpRoutine(&buff[0], buff.size());
	}
}

// ============================================================================
// SUITE DE TESTS
// ============================================================================

static bool testSimpleGetRequest() {
	Http h(getTestConfig());
	feed(h, "GET /index.html HTTP/1.1\r\nHost: localhost\r\nUser-Agent: Mozilla\r\n\r\n");

	ASSERT(h.getStatus() == PROCESSING || h.getStatus() == FINISHED);
	ASSERT(h.getRequest().getMethod() == "GET");

	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers.count("host") == 1);
	ASSERT(headers["host"] == "localhost");
	ASSERT(headers.count("user-agent") == 1);
	ASSERT(headers["user-agent"] == "Mozilla");
	return true;
}

static bool testMissingHostHeaderThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /index.html HTTP/1.1\r\nUser-Agent: Mozilla\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testHeaderValueTrimming() {
	Http h(getTestConfig());
	feed(h, "POST /api HTTP/1.1\r\nHost:   localhost:8080   \r\nContent-Type:  text/html; charset=utf-8  \r\nContent-Length: 0\r\n\r\n");

	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["host"] == "localhost:8080");
	ASSERT(headers["content-type"] == "text/html; charset=utf-8");
	return true;
}

static bool testHeaderKeysAreCaseInsensitive() {
	Http h(getTestConfig());
	feed(h, "GET /a HTTP/1.1\r\nHOsT: example.com\r\n\r\n");
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers.count("host") == 1);
	ASSERT(headers["host"] == "example.com");
	return true;
}

static bool testDuplicateHostThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /a HTTP/1.1\r\nHost: localhost\r\nHost: duplicate.com\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testDuplicateContentLengthThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "POST /a HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testInvalidVersionThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /a HTTP/1.0\r\nHost: localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 505 || e.getStatusCode() == 400);
	}
	return true;
}

static bool testMalformedRequestLineThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /only-two-tokens\r\nHost: localhost1\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testHeaderWithoutColonThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /a HTTP/1.1\r\nHost localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testEmptyHeaderKeyThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /a HTTP/1.1\r\n: value\r\nHost: localhost3\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testEmptyHeaderValueAccepted() {
	Http h(getTestConfig());
	feed(h, "GET /a HTTP/1.1\r\nHost: localhost\r\nX-Empty:\r\n\r\n");

	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers.count("x-empty") == 1);
	ASSERT(headers["x-empty"] == "");
	return true;
}

static bool testContentLengthZero() {
	Http h(getTestConfig());
	feed(h, "POST /upload HTTP/1.1\r\nHost: localhost1\r\nContent-Length: 0\r\n\r\n");

	ASSERT(h.getRequest().getBody().empty());
	return true;
}

static bool testContentLengthBodyTooShortNotDone() {
	Http h(getTestConfig());
	feed(h, "POST /submit HTTP/1.1\r\nHost: localhost3\r\nContent-Length: 10\r\n\r\nHello");

	ASSERT(h.getStatus() == READING_BODY);
	return true;
}

static bool testNegativeContentLengthThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: -3\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testChunkedSingleChunkAccepted() {
	Http h(getTestConfig());
	feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");

	ASSERT(h.getRequest().getBody() == "Hello");
	return true;
}

static bool testChunkedMultipleChunksAccepted() {
	Http h(getTestConfig());
	feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n");

	ASSERT(h.getRequest().getBody() == "Hello World");
	return true;
}

static bool testChunkedInvalidHexSizeThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nZ\r\nHello\r\n0\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testBothContentLengthAndChunkedPolicy() {
	Http h(getTestConfig());
	try {
		feed(h, "POST /x HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testGetMethodWithBodyThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /index.html HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\n12345");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testSpaceInHeaderKeyThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET / HTTP/1.1\r\nHost : localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testFragmentedHeaderParsing() {
	Http h(getTestConfig());
	feed(h, "GET /index.h");
	ASSERT(h.getStatus() == READING_HEADERS);
	
	feed(h, "tml HTTP/1.1\r\nHo");
	ASSERT(h.getStatus() == READING_HEADERS);

	feed(h, "st: localhost\r\n\r\n");
	ASSERT(h.getRequest().getMethod() == "GET");
	
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["host"] == "localhost");
	return true;
}

static bool testChunkedFragmentedPayload() {
	Http h(getTestConfig());
	feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n");
	ASSERT(h.getStatus() == READING_BODY);

	feed(h, "5\r\n");
	ASSERT(h.getStatus() == READING_BODY);

	feed(h, "Hello\r\n");
	ASSERT(h.getStatus() == READING_BODY);

	feed(h, "0\r\n\r\n");
	ASSERT(h.getRequest().getBody() == "Hello");
	return true;
}

static bool testInvalidDelimiterInRequestLineThrows() {
	Http h(getTestConfig());
	try {
		feed(h, "GET /index.html HTTP/1.1\nHost: localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testMultiLineHeaderValueFoldingSupported() {
	Http h(getTestConfig());
	feed(h, "GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom: val1\r\nX-Custom: val2\r\n\r\n");
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["x-custom"] == "val1, val2");
	return true;
}

static bool testUriWithQueryStringAndFragment() {
	Http h(getTestConfig());
	feed(h, "GET /search?query=c++98&lang=es HTTP/1.1\r\nHost: localhost\r\n\r\n");
	ASSERT(h.getRequest().getPath() == "/search");
	return true;
}

static bool testContentLengthExceedsClientMaxBodySize() {
	Http h(getTestConfig());
	try {
		feed(h, "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 999999999999\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 413);
	}
	return true;
}

static bool testChunkedExceedsClientMaxBodySize() {
	Http h(getTestConfig());
	try {
		feed(h, "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n1000000\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 413);
	}
	return true;
}

static bool testCRLFDistributedAcrossPackets() {
	Http h(getTestConfig());
	feed(h, "GET / HTTP/1.1\r\nHost: localhost\r\n\r");
	ASSERT(h.getStatus() == READING_HEADERS);
	feed(h, "\n");
	ASSERT(h.getRequest().getMethod() == "GET");
	return true;
}

static bool testHeadersWithTabSpaces() {
	Http h(getTestConfig());
	feed(h, "GET / HTTP/1.1\r\nHost:\tlocalhost\r\nCustom-Header:\t  value \t \r\n\r\n");
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["host"] == "localhost");
	ASSERT(headers["custom-header"] == "value");
	return true;
}

static bool testZeroBufferBytesReadHandling() {
	Http h(getTestConfig());
	feed(h, "GET / HTTP/1.1\r\nHost: localhost\r\n");
	h.HttpRoutine(NULL, 0);
	ASSERT(h.getStatus() == READING_HEADERS);
	return true;
}

void runHttpRequestTests(int& passed, int& failed) {
	Test tests[] = {
		{ "Test 1: Simple GET request head",                   testSimpleGetRequest },
		{ "Test 2: Missing Host throws 400",                   testMissingHostHeaderThrows },
		{ "Test 3: Header value trimming",                     testHeaderValueTrimming },
		{ "Test 4: Header keys case-insensitive / GET policy", testHeaderKeysAreCaseInsensitive },
		{ "Test 5: Duplicate Host throws 400",                 testDuplicateHostThrows },
		{ "Test 6: Duplicate Content-Length throws 400",       testDuplicateContentLengthThrows },
		{ "Test 7: Invalid HTTP version throws",               testInvalidVersionThrows },
		{ "Test 8: Malformed request line throws",             testMalformedRequestLineThrows },
		{ "Test 9: Header without colon throws",               testHeaderWithoutColonThrows },
		{ "Test 10: Empty header key throws",                  testEmptyHeaderKeyThrows },
		{ "Test 11: Empty header value accepted",              testEmptyHeaderValueAccepted },
		{ "Test 12: Content-Length zero body",                 testContentLengthZero },
		{ "Test 13: Content-Length too short body => not done",testContentLengthBodyTooShortNotDone },
		{ "Test 14: Negative Content-Length throws",           testNegativeContentLengthThrows },
		{ "Test 15: Chunked single chunk accepted",            testChunkedSingleChunkAccepted },
		{ "Test 16: Chunked multiple chunks accepted",         testChunkedMultipleChunksAccepted },
		{ "Test 17: Chunked invalid size throws",              testChunkedInvalidHexSizeThrows },
		{ "Test 18: Both CL and chunked policy",               testBothContentLengthAndChunkedPolicy },
		{ "Test 19: GET method with body throws 400",          testGetMethodWithBodyThrows },
		{ "Test 20: Space in header key throws 400",           testSpaceInHeaderKeyThrows },
		{ "Test 21: Fragmented header parsing",                testFragmentedHeaderParsing },
		{ "Test 22: Chunked fragmented payload",               testChunkedFragmentedPayload },
		{ "Test 23: Invalid delimiter in request line (\\n)",  testInvalidDelimiterInRequestLineThrows },
		{ "Test 24: Multi-line header folding concatenation",  testMultiLineHeaderValueFoldingSupported },
		{ "Test 25: URI Query String stripping",               testUriWithQueryStringAndFragment },
		{ "Test 26: Payload too large (Content-Length)",       testContentLengthExceedsClientMaxBodySize },
		{ "Test 27: Payload too large (Chunked)",              testChunkedExceedsClientMaxBodySize },
		{ "Test 28: Split CRLF between network packets",       testCRLFDistributedAcrossPackets },
		{ "Test 29: Headers with tab whitespace trimming",     testHeadersWithTabSpaces },
		{ "Test 30: Zero bytes read buffer feed",              testZeroBufferBytesReadHandling }
	};

	int localPassed = 0;
	int localFailed = 0;

	for (size_t i = 0; i < ARRAY_SIZE(tests); i++) {
		if (tests[i].fn()) {
			localPassed++;
		} else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}

	printTestSummary("Http & Request Pipeline", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}