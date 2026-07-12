#include "TestUtils.hpp"
#include "HttpRequest.hpp"
#include <iostream>
#include <map>
#include <string>

// static bool testSimpleGetRequest()
// {
// 	Request req;
// 	req.feedStream("GET /index.html HTTP/1.1\r\nHost: localhost\r\nUser-Agent: Mozilla\r\n\r\n");

// 	ASSERT(req.parseRequestHead() == true);
// 	ASSERT(req.getMethod() == "GET");

// 	std::map<std::string, std::string> headers = req.getHeaders();
// 	ASSERT(headers.count("host") == 1);
// 	ASSERT(headers["host"] == "localhost");
// 	ASSERT(headers.count("user-agent") == 1);
// 	ASSERT(headers["user-agent"] == "Mozilla");
// 	return true;
// }

// static bool testMissingHostHeaderThrows()
// {
// 	Request req;
// 	req.feedStream("GET /index.html HTTP/1.1\r\nUser-Agent: Mozilla\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	} catch (...) {
// 		ASSERT(false);
// 	}
// 	return true;
// }

// static bool testHeaderValueTrimming()
// {
// 	Request req;
// 	req.feedStream("POST /api HTTP/1.1\r\nHost:   localhost:8080   \r\nContent-Type:  text/html; charset=utf-8  \r\n\r\n");

// 	ASSERT(req.parseRequestHead() == true);

// 	std::map<std::string, std::string> headers = req.getHeaders();
// 	ASSERT(headers["host"] == "localhost:8080");
// 	ASSERT(headers["content-type"] == "text/html; charset=utf-8");
// 	return true;
// }

// static bool testHeaderKeysAreCaseInsensitive()
// {
// 	Request req;
// 	req.feedStream("GET /a HTTP/1.1\r\nHOsT: example.com\r\nCONTENT-leNGTH: 0\r\n\r\n");
// 	// req.feedStream("GET / HTTP/1.1\r\nHOsT: example.com\r\nCONTENT-leNGTH: 0\r\n\r\n");


// 	ASSERT(req.parseRequestHead() == true);

// 	std::map<std::string, std::string> headers = req.getHeaders();
// 	ASSERT(headers.count("host") == 1);
// 	ASSERT(headers["host"] == "example.com");
// 	ASSERT(headers.count("content-length") == 1);
// 	ASSERT(headers["content-length"] == "0");
// 	return true;
// }

// static bool testDuplicateHostThrows()
// {
// 	Request req;
// 	req.feedStream("GET /a HTTP/1.1\r\nHost: localhost\r\nHost: duplicate.com\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

// static bool testDuplicateContentLengthThrows()
// {
// 	Request req;
// 	req.feedStream("POST /a HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

// static bool testInvalidMethodThrows()
// {
// 	Request req;
// 	req.feedStream("G3T /a HTTP/1.1\r\nHost: localhost\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400 || e.getStatusCode() == 405);
// 	}
// 	return true;
// }

// static bool testInvalidVersionThrows()
// {
// 	Request req;
// 	req.feedStream("GET /a HTTP/1.0\r\nHost: localhost\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400 || e.getStatusCode() == 505);
// 	}
// 	return true;
// }

// static bool testMalformedRequestLineThrows()
// {
// 	Request req;
// 	req.feedStream("GET /only-two-tokens\r\nHost: localhost1\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

// static bool testHeaderWithoutColonThrows()
// {
// 	Request req;
// 	req.feedStream("GET /a HTTP/1.1\r\nHost localhost\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

// static bool testEmptyHeaderKeyThrows()
// {
// 	Request req;
// 	req.feedStream("GET /a HTTP/1.1\r\n: value\r\nHost: localhost3\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

// static bool testEmptyHeaderValueAccepted()
// {
// 	Request req;
// 	req.feedStream("GET /a HTTP/1.1\r\nHost: localhost\r\nX-Empty:\r\n\r\n");

// 	ASSERT(req.parseRequestHead() == true);
// 	std::map<std::string, std::string> headers = req.getHeaders();
// 	ASSERT(headers.count("x-empty") == 1);
// 	ASSERT(headers["x-empty"] == "");
// 	return true;
// }

// static bool testContentLengthZero()
// {
// 	Request req;
// 	req.feedStream("POST /upload HTTP/1.1\r\nHost: localhost1\r\nContent-Length: 0\r\n\r\n");

// 	ASSERT(req.parseRequestHead() == true);
// 	ASSERT(req.parseRequestBody() == true);
// 	ASSERT(req.getBody().empty());
// 	return true;
// }

// static bool testContentLengthBodyTooShortNotDone()
// {
// 	Request req;
// 	req.feedStream("POST /submit HTTP/1.1\r\nHost: localhost3\r\nContent-Length: 10\r\n\r\nHello");

// 	ASSERT(req.parseRequestHead() == true);
// 	ASSERT(req.parseRequestBody() == false);
// 	return true;
// }

// static bool testNegativeContentLengthThrows()
// {
// 	Request req;
// 	req.feedStream("POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: -3\r\n\r\n");

// 	try {
// 		req.parseRequestHead();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

static bool testChunkedSingleChunkAccepted()
{
	Request req;
	req.feedStream("POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");

	ASSERT(req.parseRequestHead() == true);
	ASSERT(req.parseRequestBody() == true);
	ASSERT(req.getBody() == "Hello");
	return true;
}

// static bool testChunkedMultipleChunksAccepted()
// {
// 	Request req;
// 	req.feedStream("POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n1\r\n \r\n5\r\nWorld\r\n0\r\n\r\n");

// 	ASSERT(req.parseRequestHead() == true);
// 	ASSERT(req.parseRequestBody() == true);
// 	ASSERT(req.getBody() == "Hello World");
// 	return true;
// }

// static bool testChunkedInvalidHexSizeThrows()
// {
// 	Request req;
// 	req.feedStream("POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nZ\r\nHello\r\n0\r\n\r\n");

// 	ASSERT(req.parseRequestHead() == true);
// 	try {
// 		req.parseRequestBody();
// 		ASSERT(false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

// static bool testChunkedMissingTerminatorNotDoneOrThrows400()
// {
// 	Request req;
// 	req.feedStream("POST /stream HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n");

// 	ASSERT(req.parseRequestHead() == true);
// 	try {
// 		bool done = req.parseRequestBody();
// 		ASSERT(done == false);
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

// static bool testBothContentLengthAndChunkedPolicy()
// {
// 	Request req;
// 	req.feedStream("POST /x HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n");

// 	try {
// 		bool ok = req.parseRequestHead();
// 		if (ok) {
// 			bool done = req.parseRequestBody();
// 			ASSERT(done == true || done == false);
// 		}
// 	} catch (const HttpException& e) {
// 		ASSERT(e.getStatusCode() == 400);
// 	}
// 	return true;
// }

void runHttpRequestTests(int& passed, int& failed)
{
	Test tests[] = {
	// { "Test 1: Simple GET request head",                    testSimpleGetRequest },
	// { "Test 2: Missing Host throws 400",                   testMissingHostHeaderThrows },
	// { "Test 3: Header value trimming",                     testHeaderValueTrimming },
	// { "Test 4: Header keys case-insensitive",              testHeaderKeysAreCaseInsensitive },
	// { "Test 5: Duplicate Host throws 400",                 testDuplicateHostThrows },
	// { "Test 6: Duplicate Content-Length throws 400",       testDuplicateContentLengthThrows },
	// { "Test 7: Invalid method throws",                     testInvalidMethodThrows },
	// { "Test 8: Invalid HTTP version throws",               testInvalidVersionThrows },
	// { "Test 9: Malformed request line throws",             testMalformedRequestLineThrows },
	// { "Test 10: Header without colon throws",              testHeaderWithoutColonThrows },
	// { "Test 11: Empty header key throws",                  testEmptyHeaderKeyThrows },
	// { "Test 12: Empty header value accepted",              testEmptyHeaderValueAccepted },
	// { "Test 13: Content-Length zero body",                 testContentLengthZero },
	// { "Test 15: Content-Length too short body => not done",testContentLengthBodyTooShortNotDone },
	// { "Test 16: Negative Content-Length throws",           testNegativeContentLengthThrows },
	{ "Test 18: Chunked single chunk accepted",            testChunkedSingleChunkAccepted },
	// { "Test 19: Chunked multiple chunks accepted",         testChunkedMultipleChunksAccepted },
	// { "Test 20: Chunked invalid size throws",              testChunkedInvalidHexSizeThrows },
	// { "Test 21: Chunked missing terminator",               testChunkedMissingTerminatorNotDoneOrThrows400 },
	// { "Test 23: Both CL and chunked policy",               testBothContentLengthAndChunkedPolicy }
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