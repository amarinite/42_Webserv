#include "TestUtils.hpp"
#include "Http.hpp"
#include <iostream>
#include <map>
#include <string>

static void feed(Http& h, const std::string& s) {
	h.HttpRoutine(const_cast<char*>(s.data()), s.size());
}

static bool testSimpleGetRequest()
{
	Http h;
	feed(h, "GET /index.html HTTP/1.1\r\nHost: localhost\r\nUser-Agent: Mozilla\r\n\r\n");

	ASSERT(h.getStatus() == PROCESSING);
	ASSERT(h.getRequest().getMethod() == "GET");

	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers.count("host") == 1);
	ASSERT(headers["host"] == "localhost");
	ASSERT(headers.count("user-agent") == 1);
	ASSERT(headers["user-agent"] == "Mozilla");
	return true;
}

static bool testMissingHostHeaderThrows()
{
	Http h;
	try {
		feed(h, "GET /index.html HTTP/1.1\r\nUser-Agent: Mozilla\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testHeaderValueTrimming()
{
	Http h;
	feed(h, "POST /api HTTP/1.1\r\nHost:   localhost:8080   \r\nContent-Type:  text/html; charset=utf-8  \r\nContent-Length: 0\r\n\r\n");

	ASSERT(h.getStatus() == PROCESSING);
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers["host"] == "localhost:8080");
	ASSERT(headers["content-type"] == "text/html; charset=utf-8");
	return true;
}

static bool testHeaderKeysAreCaseInsensitive()
{
	Http h;

	try {
		feed(h, "GET /a HTTP/1.1\r\nHOsT: example.com\r\nCONTENT-leNGTH: 0\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testDuplicateHostThrows()
{
	Http h;
	try {
		feed(h, "GET /a HTTP/1.1\r\nHost: localhost\r\nHost: duplicate.com\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testDuplicateContentLengthThrows()
{
	Http h;
	try {
		feed(h, "POST /a HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testInvalidMethodThrows()
{
	Http h;
	try {
		feed(h, "G3T /a HTTP/1.1\r\nHost: localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400 || e.getStatusCode() == 405);
	}
	return true;
}

static bool testInvalidVersionThrows()
{
	Http h;
	try {
		feed(h, "GET /a HTTP/1.0\r\nHost: localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400 || e.getStatusCode() == 505);
	}
	return true;
}

static bool testMalformedRequestLineThrows()
{
	Http h;
	try {
		feed(h, "GET /only-two-tokens\r\nHost: localhost1\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testHeaderWithoutColonThrows()
{
	Http h;
	try {
		feed(h, "GET /a HTTP/1.1\r\nHost localhost\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testEmptyHeaderKeyThrows()
{
	Http h;
	try {
		feed(h, "GET /a HTTP/1.1\r\n: value\r\nHost: localhost3\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testEmptyHeaderValueAccepted()
{
	Http h;
	feed(h, "GET /a HTTP/1.1\r\nHost: localhost\r\nX-Empty:\r\n\r\n");

	ASSERT(h.getStatus() == PROCESSING);
	std::map<std::string, std::string> headers = h.getRequest().getHeaders();
	ASSERT(headers.count("x-empty") == 1);
	ASSERT(headers["x-empty"] == "");
	return true;
}

static bool testContentLengthZero()
{
	Http h;
	feed(h, "POST /upload HTTP/1.1\r\nHost: localhost1\r\nContent-Length: 0\r\n\r\n");

	ASSERT(h.getStatus() == PROCESSING);
	ASSERT(h.getRequest().getBody().empty());
	return true;
}

static bool testContentLengthBodyTooShortNotDone()
{
	Http h;
	feed(h, "POST /submit HTTP/1.1\r\nHost: localhost3\r\nContent-Length: 10\r\n\r\nHello");

	ASSERT(h.getStatus() == READING_BODY);
	return true;
}

static bool testNegativeContentLengthThrows()
{
	Http h;
	try {
		feed(h, "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: -3\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testChunkedSingleChunkAccepted()
{
	Http h;
	feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");

	ASSERT(h.getStatus() == PROCESSING);
	ASSERT(h.getRequest().getBody() == "Hello");
	return true;
}

static bool testChunkedMultipleChunksAccepted()
{
	Http h;
	feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n1\r\n \r\n5\r\nWorld\r\n0\r\n\r\n");

	ASSERT(h.getStatus() == PROCESSING);
	ASSERT(h.getRequest().getBody() == "Hello World");
	return true;
}

static bool testChunkedInvalidHexSizeThrows()
{
	Http h;
	try {
		feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nZ\r\nHello\r\n0\r\n\r\n");
		ASSERT(false);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testChunkedMissingTerminatorNotDoneOrThrows400()
{
	Http h;
	try {
		feed(h, "POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n");
		ASSERT(h.getStatus() == READING_BODY);
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

static bool testBothContentLengthAndChunkedPolicy()
{
	Http h;
	try {
		feed(h, "POST /x HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");
	} catch (const HttpException& e) {
		ASSERT(e.getStatusCode() == 400);
	}
	return true;
}

void runHttpRequestTests(int& passed, int& failed)
{
	Test tests[] = {
		{ "Test 1: Simple GET request head",                   testSimpleGetRequest },
		{ "Test 2: Missing Host throws 400",                   testMissingHostHeaderThrows },
		{ "Test 3: Header value trimming",                     testHeaderValueTrimming },
		{ "Test 4: Header keys case-insensitive / GET policy", testHeaderKeysAreCaseInsensitive },
		{ "Test 5: Duplicate Host throws 400",                 testDuplicateHostThrows },
		{ "Test 6: Duplicate Content-Length throws 400",       testDuplicateContentLengthThrows },
		{ "Test 7: Invalid method throws",                     testInvalidMethodThrows },
		{ "Test 8: Invalid HTTP version throws",               testInvalidVersionThrows },
		{ "Test 9: Malformed request line throws",             testMalformedRequestLineThrows },
		{ "Test 10: Header without colon throws",              testHeaderWithoutColonThrows },
		{ "Test 11: Empty header key throws",                  testEmptyHeaderKeyThrows },
		{ "Test 12: Empty header value accepted",              testEmptyHeaderValueAccepted },
		{ "Test 13: Content-Length zero body",                 testContentLengthZero },
		{ "Test 15: Content-Length too short body => not done",testContentLengthBodyTooShortNotDone },
		{ "Test 16: Negative Content-Length throws",           testNegativeContentLengthThrows },
		{ "Test 18: Chunked single chunk accepted",            testChunkedSingleChunkAccepted },
		{ "Test 19: Chunked multiple chunks accepted",         testChunkedMultipleChunksAccepted },
		{ "Test 20: Chunked invalid size throws",              testChunkedInvalidHexSizeThrows },
		{ "Test 21: Chunked missing terminator",               testChunkedMissingTerminatorNotDoneOrThrows400 },
		{ "Test 23: Both CL and chunked policy",               testBothContentLengthAndChunkedPolicy }
	};

	int localPassed = 0;
	int localFailed = 0;

	for (int i = 0; i < (int)ARRAY_SIZE(tests); i++)
	{
		if (tests[i].fn())
			localPassed++;
		else {
			std::cout << RED << "  [FAIL] " << tests[i].name << "\n" << RESET;
			localFailed++;
		}
	}

	printTestSummary("HttpRequest", localPassed, localFailed);

	passed += localPassed;
	failed += localFailed;
}