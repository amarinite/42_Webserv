#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cstdlib>

#include "Http.hpp"
#include "HttpException.hpp"
#include "HttpResponse.hpp"

// ============================================================================
// MACROS & MACRO-HARNESS DE PRUEBAS (C++98)
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

// Helper para alimentar la maquina de estados de Http de forma segura
static void feed(Http& h, const std::string& s) {
	if (s.empty()) {
		h.HttpRoutine(NULL, 0);
	} else {
		// Pasamos buffer no-constante compatible con la firma HttpRoutine(char*, size_t)
		std::vector<char> buff(s.begin(), s.end());
		h.HttpRoutine(&buff[0], buff.size());
	}
}

// ============================================================================
// 1. TESTS ADAPTADOS DE PETICIÓN (REQUEST & PARSING)
// ============================================================================

static bool testSimpleGetRequest() {
	Http h;
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
	Http h;
	try {
		feed(h, "GET /index.html HTTP/1.1\r\nUser-Agent: Mozilla\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testHeaderValueTrimming() {
	Http h;
	feed(h, "POST /api HTTP/1.1\r\nHost:   localhost:8080   \r\nContent-Type:  text/html; charset=utf-8  \r\nContent-Length: 0\r\n\r\n");

	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["host"] == "localhost:8080");
	ASSERT(headers["content-type"] == "text/html; charset=utf-8");
	return true;
}

static bool testHeaderKeysAreCaseInsensitive() {
	Http h;
	feed(h, "GET /a HTTP/1.1\r\nHOsT: example.com\r\n\r\n");
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers.count("host") == 1);
	ASSERT(headers["host"] == "example.com");
	return true;
}

static bool testDuplicateHostThrows() {
	Http h;
	try {
		feed(h, "GET /a HTTP/1.1\r\nHost: localhost\r\nHost: duplicate.com\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testDuplicateContentLengthThrows() {
	Http h;
	try {
		feed(h, "POST /a HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testInvalidVersionThrows() {
	Http h;
	try {
		feed(h, "GET /a HTTP/1.0\r\nHost: localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 505 || e.getStatusCode() == 400);
	}
	return true;
}

static bool testMalformedRequestLineThrows() {
	Http h;
	try {
		feed(h, "GET /only-two-tokens\r\nHost: localhost1\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testHeaderWithoutColonThrows() {
	Http h;
	try {
		feed(h, "GET /a HTTP/1.1\r\nHost localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testEmptyHeaderKeyThrows() {
	Http h;
	try {
		feed(h, "GET /a HTTP/1.1\r\n: value\r\nHost: localhost3\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testEmptyHeaderValueAccepted() {
	Http h;
	feed(h, "GET /a HTTP/1.1\r\nHost: localhost\r\nX-Empty:\r\n\r\n");

	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers.count("x-empty") == 1);
	ASSERT(headers["x-empty"] == "");
	return true;
}

static bool testContentLengthZero() {
	Http h;
	feed(h, "POST /upload HTTP/1.1\r\nHost: localhost1\r\nContent-Length: 0\r\n\r\n");

	ASSERT(h.getRequest().getBody().empty());
	return true;
}

static bool testContentLengthBodyTooShortNotDone() {
	Http h;
	feed(h, "POST /submit HTTP/1.1\r\nHost: localhost3\r\nContent-Length: 10\r\n\r\nHello");

	ASSERT(h.getStatus() == READING_BODY);
	return true;
}

static bool testNegativeContentLengthThrows() {
	Http h;
	try {
		feed(h, "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: -3\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testChunkedSingleChunkAccepted() {
	Http h;
	feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");

	ASSERT(h.getRequest().getBody() == "Hello");
	return true;
}

static bool testChunkedMultipleChunksAccepted() {
	Http h;
	feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n");

	ASSERT(h.getRequest().getBody() == "Hello World");
	return true;
}

static bool testChunkedInvalidHexSizeThrows() {
	Http h;
	try {
		feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nZ\r\nHello\r\n0\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testBothContentLengthAndChunkedPolicy() {
	Http h;
	try {
		feed(h, "POST /x HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testGetMethodWithBodyThrows() {
	Http h;
	try {
		feed(h, "GET /index.html HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\n12345");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testSpaceInHeaderKeyThrows() {
	Http h;
	try {
		feed(h, "GET / HTTP/1.1\r\nHost : localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testFragmentedHeaderParsing() {
	Http h;
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
	Http h;
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
	Http h;
	try {
		feed(h, "GET /index.html HTTP/1.1\nHost: localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testMultiLineHeaderValueFoldingSupported() {
	Http h;
	feed(h, "GET / HTTP/1.1\r\nHost: localhost\r\nX-Custom: val1\r\nX-Custom: val2\r\n\r\n");
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["x-custom"] == "val1, val2");
	return true;
}

// ============================================================================
// 2. NUEVOS TESTS DE CASOS LÍMITE (EDGE CASES & ADVANCED FLOWS)
// ============================================================================

static bool testUriWithQueryStringAndFragment() {
	Http h;
	feed(h, "GET /search?query=c++98&lang=es HTTP/1.1\r\nHost: localhost\r\n\r\n");
	ASSERT(h.getRequest().getPath() == "/search");
	return true;
}

static bool testContentLengthExceedsClientMaxBodySize() {
	Http h; // Asumiendo clientMaxBodySize configurado por defecto
	try {
		feed(h, "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 999999999999\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 413); // Payload Too Large
	}
	return true;
}

static bool testChunkedExceedsClientMaxBodySize() {
	Http h;
	try {
		feed(h, "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n1000000\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 413);
	}
	return true;
}

static bool testCRLFDistributedAcrossPackets() {
	Http h;
	feed(h, "GET / HTTP/1.1\r\nHost: localhost\r\n\r");
	ASSERT(h.getStatus() == READING_HEADERS);
	feed(h, "\n");
	ASSERT(h.getRequest().getMethod() == "GET");
	return true;
}

static bool testHeadersWithTabSpaces() {
	Http h;
	feed(h, "GET / HTTP/1.1\r\nHost:\tlocalhost\r\nCustom-Header:\t  value \t \r\n\r\n");
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["host"] == "localhost");
	ASSERT(headers["custom-header"] == "value");
	return true;
}

static bool testZeroBufferBytesReadHandling() {
	Http h;
	feed(h, "GET / HTTP/1.1\r\nHost: localhost\r\n");
	// Simular lectura de 0 bytes (EOF o timeout) sin alterar el stream
	h.HttpRoutine(NULL, 0);
	ASSERT(h.getStatus() == READING_HEADERS);
	return true;
}

static bool testResponseRawBufferDataPersists() {
	Response res;
	res.setStatusCode("200");
	res.setMessage("OK");
	res.setResponseBody("Hello World");
	res.assignHeaders("html", "close");
	res.buildRawResponse();

	const char* raw = res.getRawData();
	size_t size = res.getRawSize();

	ASSERT(raw != NULL);
	ASSERT(size > 0);

	std::string rawStr(raw, size);
	ASSERT(rawStr.find("HTTP/1.1 200 OK\r\n") == 0);
	ASSERT(rawStr.find("Content-Length: 11\r\n") != std::string::npos);
	ASSERT(rawStr.find("\r\n\r\nHello World") != std::string::npos);
	return true;
}

static bool testResponseAllowedMethodsHeader() {
	Response res;
	res.setStatusCode("405");
	res.setMessage("Method Not Allowed");
	res.setAllowedMethodsHeader("GET, POST");
	res.buildRawResponse();

	std::string rawStr(res.getRawData(), res.getRawSize());
	ASSERT(rawStr.find("Allow: GET, POST\r\n") != std::string::npos);
	return true;
}

static bool testResponseLocationHeaderOnRedirect() {
	Response res;
	res.setStatusCode("301");
	res.setMessage("Moved Permanently");
	res.setLocationHeader("/new-path/");
	res.buildRawResponse();

	std::string rawStr(res.getRawData(), res.getRawSize());
	ASSERT(rawStr.find("Location: /new-path/\r\n") != std::string::npos);
	return true;
}

static bool testHttpExceptionErrorResponseBuilding() {
	Response res;
	HttpException ex(404, "Not Found");
	
	// Simular preparación de error
	res.setStatusCode(toStr(ex.getStatusCode()));
	res.setMessage(ex.what());
	res.setResponseBody("<h1>404 Not Found</h1>");
	res.assignHeaders("html", "close");
	res.buildRawResponse();

	std::string rawStr(res.getRawData(), res.getRawSize());
	ASSERT(rawStr.find("HTTP/1.1 404 Not Found\r\n") == 0);
	ASSERT(rawStr.find("Connection: close\r\n") != std::string::npos);
	return true;
}

// ============================================================================
// SUITE EXECUTION MAIN
// ============================================================================

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
		{ "Test 9: Header without colon throws",              testHeaderWithoutColonThrows },
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
		// Edges & Flows
		{ "Test 25: URI Query String stripping",               testUriWithQueryStringAndFragment },
		{ "Test 26: Payload too large (Content-Length)",       testContentLengthExceedsClientMaxBodySize },
		{ "Test 27: Payload too large (Chunked)",              testChunkedExceedsClientMaxBodySize },
		{ "Test 28: Split CRLF between network packets",       testCRLFDistributedAcrossPackets },
		{ "Test 29: Headers with tab whitespace trimming",     testHeadersWithTabSpaces },
		{ "Test 30: Zero bytes read buffer feed",              testZeroBufferBytesReadHandling },
		{ "Test 31: Response raw buffer serialization",        testResponseRawBufferDataPersists },
		{ "Test 32: Response Allow header (405)",              testResponseAllowedMethodsHeader },
		{ "Test 33: Response Location header (301)",           testResponseLocationHeaderOnRedirect },
		{ "Test 34: HttpException Error Response formatting",  testHttpExceptionErrorResponseBuilding }
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

int main() {
	int passed = 0;
	int failed = 0;

	std::cout << CYAN << "=== INICIANDO SUITE DE TESTS HTTP & PIPELINE ===" << RESET << "\n\n";
	runHttpRequestTests(passed, failed);
	std::cout << "\n" << CYAN << "===============================================" << RESET << "\n";

	return (failed == 0) ? 0 : 1;
}